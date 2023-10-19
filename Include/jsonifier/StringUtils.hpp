/*
	MIT License

	Copyright (c) 2023 RealTimeChris

	Permission is hereby granted, free of charge, to any person obtaining a copy of this
	software and associated documentation files (the "Software"), to deal in the Software
	without restriction, including without limitation the rights to use, copy, modify, merge,
	publish, distribute, sublicense, and/or sell copies of the Software, and to permit
	persons to whom the Software is furnished to do so, subject to the following conditions:

	The above copyright notice and this permission notice shall be included in all copies or
	substantial portions of the Software.

	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
	INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
	PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE
	FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
	OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
	DEALINGS IN THE SOFTWARE.
*/
/// https://github.com/RealTimeChris/jsonifier
/// Feb 3, 2023
/// Most of the code in this header was sampled from simdjson - https://github.com/simdjson
#pragma once

#include <jsonifier/Allocator.hpp>
#include <jsonifier/Tables.hpp>
#include <jsonifier/Base.hpp>
#include <jsonifier/Simd.hpp>

namespace jsonifier_internal {

	class backslash_and_quote {
	  public:
		static constexpr string_parsing_type bytesProcessed = BytesPerStep;

		template<typename source_type, typename dest_type> inline static backslash_and_quote copyAndFind(const source_type* source, dest_type* destString) {
			avx_int_t values(gatherValues<avx_int_t>(source));
			store(values, destString);
			backslash_and_quote returnData{};
			returnData.bsBits	 = { cmpEq(values, '\\') };
			returnData.quoteBits = { cmpEq(values, '\"') };
			return returnData;
		}

		inline bool hasQuoteFirst() {
			return ((bsBits - 1) & quoteBits) != 0;
		}

		inline bool hasBackslash() {
			return ((quoteBits - 1) & bsBits) != 0;
		}

		inline string_parsing_type quoteIndex() {
			return tzcnt(quoteBits);
		}

		inline string_parsing_type backslashIndex() {
			return tzcnt(bsBits);
		}

	  protected:
		string_parsing_type quoteBits{};
		string_parsing_type bsBits{};
	};

	template<typename value_type> inline static uint32_t hexToU32NoCheck(const value_type* source) {
		uint32_t v1 = digitToVal32[630 + source[0]];
		uint32_t v2 = digitToVal32[420 + source[1]];
		uint32_t v3 = digitToVal32[210 + source[2]];
		uint32_t v4 = digitToVal32[0 + source[3]];
		return v1 | v2 | v3 | v4;
	}

	inline static uint32_t codePointToUtf8(uint32_t codePoint, uint8_t* c) {
		if (codePoint <= 0x7F) {
			c[0] = uint8_t(codePoint);
			return 1;
		}
		int32_t leading_zeros = lzcnt(codePoint);

		if (leading_zeros >= 11) {
			uint32_t pattern = pdep(0x3F00U, codePoint);
			pattern |= 0xC0ULL;
			c[0] = static_cast<uint8_t>(pattern >> 8);
			c[1] = static_cast<uint8_t>(pattern & 0xFF);
			return 2;
		} else if (leading_zeros >= 16) {
			uint32_t pattern = pdep(0x0F0800U, codePoint);
			pattern |= 0xE0ULL;
			c[0] = static_cast<uint8_t>(pattern >> 16);
			c[1] = static_cast<uint8_t>(pattern >> 8);
			c[2] = static_cast<uint8_t>(pattern & 0xFF);
			return 3;
		} else if (leading_zeros >= 21) {
			uint32_t pattern = pdep(0x01020400U, codePoint);
			pattern |= 0xF0ULL;
			c[0] = static_cast<uint8_t>(pattern >> 24);
			c[1] = static_cast<uint8_t>(pattern >> 16);
			c[2] = static_cast<uint8_t>(pattern >> 8);
			c[3] = static_cast<uint8_t>(pattern & 0xFF);
			return 4;
		}
		return 0;
	}

	template<typename value_type01, typename value_type02> inline static bool handleUnicodeCodePoint(value_type01* srcPtr, value_type02* dstPtr) {
		constexpr uint32_t subCodePoint = 0xfffd;
		constexpr auto backslash{ static_cast<uint8_t>('\\') };
		constexpr auto u{ static_cast<uint8_t>('u') };
		uint32_t codePoint = hexToU32NoCheck(*srcPtr + 2);
		*srcPtr += 6;
		if (codePoint >= 0xd800 && codePoint < 0xdc00) {
			value_type01 srcData = *srcPtr;
			if (((srcData[0] << 8) | srcData[1]) != (backslash << 8 | u)) {
				codePoint = subCodePoint;
			} else {
				uint32_t codePoint2 = hexToU32NoCheck(srcData + 2);
				uint32_t lowBit		= codePoint2 - 0xdc00;
				if (lowBit >> 10) {
					codePoint = subCodePoint;
				} else {
					codePoint = (((codePoint - 0xd800) << 10) | lowBit) + 0x10000;
					*srcPtr += 6;
				}
			}
		} else if (codePoint >= 0xdc00 && codePoint <= 0xdfff) {
			codePoint = subCodePoint;
		}
		uint64_t offset = codePointToUtf8(codePoint, *dstPtr);
		*dstPtr += offset;
		return offset > 0;
	}

	template<typename source_type, typename dest_type> inline static dest_type* parseString(const source_type* source, dest_type* destString) {
		while (true) {
			auto bsQuote = backslash_and_quote::copyAndFind(source, destString);
			if (bsQuote.hasQuoteFirst()) {
				return destString + bsQuote.quoteIndex();
			}
			if (bsQuote.hasBackslash()) {
				auto bsDist			   = bsQuote.backslashIndex();
				source_type escapeChar = source[bsDist + 1];
				if (escapeChar == 'u') {
					source += bsDist;
					destString += bsDist;
					handleUnicodeCodePoint(&source, &destString);
				} else {
					uint8_t escapeResult = escapeMap<uint8_t>[escapeChar];
					if (escapeResult == 0u) {
						return nullptr;
					}
					destString[bsDist] = escapeResult;
					destString += bsDist + 1ULL;
					source += bsDist + 2ULL;
				}
			} else {
				source += backslash_and_quote::bytesProcessed;
				destString += backslash_and_quote::bytesProcessed;
			}
		}
		return nullptr;
	}

	inline bool parseBool(string_view_ptr json) {
		uint8_t valueNew[5]{ "true" };
		return std::memcmp(valueNew, json, 4) == 0;
	}
}// namespace jsonifier_internal
