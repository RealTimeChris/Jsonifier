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
/// https://github.com/RealTimeChris/Jsonifier
/// Feb 3, 2023
/// Most of the code in this header was sampled fromSimdjson - https://github.com/simdjson
#pragma once

#include <jsonifier/Base.hpp>
#include <jsonifier/Tables.hpp>
#include <jsonifier/Simd.hpp>
#include <codecvt>

namespace JsonifierInternal {

	template<typename SimdBase> class BackslashAndQuote;

	template<> class BackslashAndQuote<SimdBaseReal> {
	  public:
		static const uint32_t bytesProcessed = BytesPerStep;

		BackslashAndQuote<SimdBaseReal> inline static copyAndFind(StringViewPtr source, StringBufferPtr dest) {
			SimdBaseReal values(source);
			values.store(dest);
			BackslashAndQuote returnData{};
			returnData.bsBits = { values == uint8_t{ '\\' } };
			returnData.quoteBits = { values == uint8_t{ '\"' } };
			return returnData;
		}

		inline bool hasQuoteFirst() {
			return ((bsBits - 1) & quoteBits) != 0;
		}

		inline bool hasBackslash() {
			return ((quoteBits - 1) & bsBits) != 0;
		}

		inline StringParsingType quoteIndex() {
			return tzCount(quoteBits);
		}

		inline StringParsingType backslashIndex() {
			return tzCount(bsBits);
		}

	  protected:
		StringParsingType quoteBits{};
		StringParsingType bsBits{};
	};

	inline static uint32_t hexToU32NoCheck(StringViewPtr source) {
		uint32_t v1 = digitToVal32[630ull + source[0]];
		uint32_t v2 = digitToVal32[420ull + source[1]];
		uint32_t v3 = digitToVal32[210ull + source[2]];
		uint32_t v4 = digitToVal32[0ull + source[3]];
		return v1 | v2 | v3 | v4;
	}

	inline size_t codePointToUtf8(uint32_t codePoint, StringBufferPtr c) {
		if (codePoint <= 0x7F) {
			c[0] = uint8_t(codePoint);
			return 1;
		}
		if (codePoint <= 0x7FF) {
			c[0] = uint8_t((codePoint >> 6) + 192);
			c[1] = uint8_t((codePoint & 63) + 128);
			return 2;
		} else if (codePoint <= 0xFFFF) {
			c[0] = uint8_t((codePoint >> 12) + 224);
			c[1] = uint8_t(((codePoint >> 6) & 63) + 128);
			c[2] = uint8_t((codePoint & 63) + 128);
			return 3;
		} else if (codePoint <= 0x10FFFF) {
			c[0] = uint8_t((codePoint >> 18) + 240);
			c[1] = uint8_t(((codePoint >> 12) & 63) + 128);
			c[2] = uint8_t(((codePoint >> 6) & 63) + 128);
			c[3] = uint8_t((codePoint & 63) + 128);
			return 4;
		}
		return 0;
	}

	inline bool handleUnicodeCodePoint(StringViewPtr* srcPtr, StringBufferPtr* dstPtr) {
		constexpr uint32_t subCodePoint = 0xfffd;
		uint32_t codePoint = hexToU32NoCheck(*srcPtr + 2);
		*srcPtr += 6;
		if (codePoint >= 0xd800 && codePoint < 0xdc00) {
			const uint8_t* srcData = *srcPtr;
			if (((srcData[0] << 8) | srcData[1]) != ((static_cast<uint8_t>('\\') << 8) | static_cast<uint8_t>('u'))) {
				codePoint = subCodePoint;
			} else {
				uint32_t codePoint2 = hexToU32NoCheck(srcData + 2);
				uint32_t lowBit = codePoint2 - 0xdc00;
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

	inline static StringBufferPtr parseString(StringViewPtr source, StringBufferPtr dest) {
		while (true) {
			auto bsQuote = BackslashAndQuote<SimdBaseReal>::copyAndFind(source, dest);
			if (bsQuote.hasQuoteFirst()) {
				return dest + bsQuote.quoteIndex();
			}
			if (bsQuote.hasBackslash()) {
				auto bsDist = bsQuote.backslashIndex();
				uint8_t escape_char = source[bsDist + 1];
				if (escape_char == 'u') {
					source += bsDist;
					dest += bsDist;
					if (!handleUnicodeCodePoint(&source, &dest)) {
						return nullptr;
					}
				} else {
					uint8_t escapeResult = escapeMap[escape_char];
					if (escapeResult == 0u) {
						return nullptr;
					}
					dest[bsDist] = escapeResult;
					source += bsDist + 2ull;
					dest += bsDist + 1ull;
				}
			} else {
				source += BackslashAndQuote<SimdBaseReal>::bytesProcessed;
				dest += BackslashAndQuote<SimdBaseReal>::bytesProcessed;
			}
		}
		return nullptr;
	}

	inline static bool parseBool(StringViewPtr json) noexcept {
		uint8_t valueNew[5]{ "true" };
		return std::memcmp(valueNew, json, 5) == 0;
	}
}
