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
		template<typename source_type, typename dest_type> jsonifier_inline static backslash_and_quote copyAndFind(const source_type* source, dest_type* destString) {
			simd_int_t values(gatherValuesU<simd_int_t>(source));
			simd_base::store(values, destString);
			backslash_and_quote returnData{};
			returnData.bsBits	 = { simd_base::cmpeq(values, simd_base::backslashes) };
			returnData.quoteBits = { simd_base::cmpeq(values, simd_base::quotes) };
			return returnData;
		}

		jsonifier_inline bool hasQuoteFirst() {
			return ((bsBits - 1) & quoteBits) != 0;
		}

		jsonifier_inline bool hasBackslash() {
			return ((quoteBits - 1) & bsBits) != 0;
		}

		jsonifier_inline string_parsing_type quoteIndex() {
			return tzcnt(quoteBits);
		}

		jsonifier_inline string_parsing_type backslashIndex() {
			return tzcnt(bsBits);
		}

	  protected:
		string_parsing_type quoteBits{};
		string_parsing_type bsBits{};
	};

	template<typename value_type> jsonifier_inline static uint32_t hexToU32NoCheck(const value_type* source) {
		uint32_t v1 = digitToVal32<uint32_t>[630 + source[0]];
		uint32_t v2 = digitToVal32<uint32_t>[420 + source[1]];
		uint32_t v3 = digitToVal32<uint32_t>[210 + source[2]];
		uint32_t v4 = digitToVal32<uint32_t>[0 + source[3]];
		return v1 | v2 | v3 | v4;
	}

	template<typename value_type> jsonifier_inline static uint32_t codePointToUtf8(uint32_t codePoint, value_type* c) {
		if (codePoint <= 0x7F) {
			c[0] = value_type(codePoint);
			return 1;
		}
		uint32_t leading_zeros = lzcnt(codePoint);

		if (leading_zeros >= 11) {
			uint32_t pattern = pdep(0x3F00U, codePoint);
			pattern |= 0xC0ull;
			c[0] = static_cast<value_type>(pattern >> 8);
			c[1] = static_cast<value_type>(pattern & 0xFFu);
			return 2;
		} else if (leading_zeros >= 16) {
			uint32_t pattern = pdep(0x0F0800U, codePoint);
			pattern |= 0xE0ull;
			c[0] = static_cast<value_type>(pattern >> 16);
			c[1] = static_cast<value_type>(pattern >> 8);
			c[2] = static_cast<value_type>(pattern & 0xFFu);
			return 3;
		} else if (leading_zeros >= 21) {
			uint32_t pattern = pdep(0x01020400U, codePoint);
			pattern |= 0xF0ull;
			c[0] = static_cast<value_type>(pattern >> 24);
			c[1] = static_cast<value_type>(pattern >> 16);
			c[2] = static_cast<value_type>(pattern >> 8);
			c[3] = static_cast<value_type>(pattern & 0xFFu);
			return 4;
		}
		return 0;
	}

	template<typename value_type01, typename value_type02>
	jsonifier_inline bool handleUnicodeCodePoint(value_type01* srcPtr, value_type02* dstPtr) {
		static jsonifier_constexpr uint32_t subCodePoint = 0xfffd;
		static jsonifier_constexpr uint8_t backslash{ '\\' };
		static jsonifier_constexpr uint8_t u{ 'u' };
		uint32_t codePoint						   = hexToU32NoCheck(*srcPtr + 2);
		*srcPtr += 6;
		if (codePoint >= 0xd800 && codePoint < 0xdc00) {
			value_type01 srcData = *srcPtr;
			if (((srcData[0] << 8) | srcData[1]) != ((static_cast<std::remove_pointer_t<value_type01>>(backslash) << 8) | static_cast<std::remove_pointer_t<value_type01>>(u))) {
				codePoint = subCodePoint;
			} else {
				uint32_t codePoint02 = hexToU32NoCheck(srcData + 2);
				uint32_t lowBit		 = codePoint02 - 0xdc00;
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
		size_t offset = codePointToUtf8(codePoint, *dstPtr);
		*dstPtr += offset;
		return offset > 0;
	}

	template<typename value_type01, typename value_type02> jsonifier_inline value_type02* parseString(const value_type01* source, value_type02* dest) {
		while (1) {
			backslash_and_quote bsQuote = backslash_and_quote::copyAndFind(source, dest);
			if (bsQuote.hasQuoteFirst()) {
				return dest + bsQuote.quoteIndex();
			}
			if (bsQuote.hasBackslash()) {
				auto bsDist		   = bsQuote.backslashIndex();
				uint8_t escapeChar = source[bsDist + 1];
				if (escapeChar == 'u') {
					source += bsDist;
					dest += bsDist;
					if (!handleUnicodeCodePoint(&source, &dest)) {
						return nullptr;
					}
				} else {
					uint8_t escape_result = escapeMap<value_type01>[escapeChar];
					if (escape_result == 0u) {
						return nullptr;
					}
					dest[bsDist] = escape_result;
					source += bsDist + 2;
					dest += bsDist + 1;
				}
			} else {
				source += BytesPerStep;
				dest += BytesPerStep;
			}
		}
		return nullptr;
	}

	template<typename value_type01, typename value_type02>
	jsonifier_inline uint64_t serializeString(const value_type01* source, value_type02* dest, uint64_t length, uint64_t& index) {
		uint64_t newIndex{};
		while (length >= BytesPerStep) {
			string_parsing_type valuesNew{};
			simd_base::convertEscapeablesToSimdBase(valuesNew, gatherValuesU<simd_int_t>(source));
			if (valuesNew != 0) {
				for (uint64_t x = 0; x < BytesPerStep; ++x) {
					if (valuesNew & (1 << x)) {
						dest[index++] = '\\';
						dest[index++] = escapeTable<value_type01>()[source[x]];
					} else {
						dest[index++] = source[x];
					}
				}
			} else {
				std::memcpy(dest + index, source, BytesPerStep);
				index += BytesPerStep;
			}
			newIndex += BytesPerStep;
			length -= BytesPerStep;
			source += BytesPerStep;
		}

		return newIndex;
	}

	jsonifier_inline bool parseBool(string_view_ptr json) {
		uint8_t valueNew[5]{ "true" };
		return std::memcmp(valueNew, json, 4) == 0;
	}
}// namespace jsonifier_internal
