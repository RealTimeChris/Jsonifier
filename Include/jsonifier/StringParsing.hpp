/*
	Jsonifier - For parsing and serializing Json - very rapidly.
	Copyright (C) 2023 Chris M. (RealTimeChris)

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	This library is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
	Lesser General Public License for more details.

	You should have received a copy of the GNU Lesser General Public
	License along with this library; if not, write to the Free Software
	Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301
	USA
*/
/// https://github.com/RealTimeChris/Jsonifier
/// Feb 3, 2023
#pragma once

#include <jsonifier/Base.hpp>
#include <jsonifier/Tables.hpp>

namespace Jsonifier {

	class Jsonifier_Dll SimdBase256;

	template<typename SimdBase256> class BackslashAndQuote {
	  public:
		static const uint32_t bytesProcessed = 32;

		BackslashAndQuote<SimdBase256> __forceinline static copyAndFind(InStringPtr src, OutStringPtr dst) {
			SimdBase256 values(reinterpret_cast<const uint8_t*>(src));
			values.store(dst);
			BackslashAndQuote returnData{};
			returnData.bsBits = static_cast<uint32_t>((values == '\\').toBitMask());
			returnData.quoteBits = static_cast<uint32_t>((values == '"').toBitMask());
			return returnData;
		}

		__forceinline bool hasQuoteFirst() {
			return ((bsBits - 1) & quoteBits) != 0;
		}

		__forceinline bool hasBackslash() {
			return ((quoteBits - 1) & bsBits) != 0;
		}

		__forceinline int32_t quoteIndex() {
			return _tzcnt_u32(quoteBits);
		}

		__forceinline int32_t backslashIndex() {
			return _tzcnt_u32(bsBits);
		}

	  protected:
		uint32_t quoteBits{};
		uint32_t bsBits{};
	};

	__forceinline uint32_t stringToUint32(InStringPtr str) {
		uint32_t val{ *reinterpret_cast<const uint32_t*>(str) };
		return val;
	}

	__forceinline uint32_t str4ncmp(InStringPtr src, const char* atom) {
		return stringToUint32(reinterpret_cast<InStringPtr>(src)) ^ stringToUint32(reinterpret_cast<InStringPtr>(atom));
	}

	__forceinline constexpr uint32_t isStructuralOrWhitespace(uint8_t c) {
		return structuralOrWhitespace[c];
	}

	__forceinline constexpr uint32_t isNotStructuralOrWhitespace(uint8_t c) {
		return structuralOrWhitespaceNegated[c];
	}

	__forceinline uint32_t hexToU32Nocheck(InStringPtr src) {
		uint32_t v1 = digitToVal32[630 + src[0]];
		uint32_t v2 = digitToVal32[420 + src[1]];
		uint32_t v3 = digitToVal32[210 + src[2]];
		uint32_t v4 = digitToVal32[0 + src[3]];
		return v1 | v2 | v3 | v4;
	}

	__forceinline size_t codePointToUtf8(uint32_t cp, OutStringPtr c) {
		if (cp <= 0x7F) {
			c[0] = uint8_t(cp);
			return 1;
		}
		if (cp <= 0x7FF) {
			c[0] = uint8_t((cp >> 6) + 192);
			c[1] = uint8_t((cp & 63) + 128);
			return 2;
		} else if (cp <= 0xFFFF) {
			c[0] = uint8_t((cp >> 12) + 224);
			c[1] = uint8_t(((cp >> 6) & 63) + 128);
			c[2] = uint8_t((cp & 63) + 128);
			return 3;
		} else if (cp <= 0x10FFFF) {
			c[0] = uint8_t((cp >> 18) + 240);
			c[1] = uint8_t(((cp >> 12) & 63) + 128);
			c[2] = uint8_t(((cp >> 6) & 63) + 128);
			c[3] = uint8_t((cp & 63) + 128);
			return 4;
		}
		return 0;
	}

	__forceinline bool handleUnicodeCodepoint(InStringPtr* srcPtr, OutStringPtr* dstPtr) {
		uint32_t codePoint = hexToU32Nocheck(*srcPtr + 2);
		*srcPtr += 6;
		if (codePoint >= 0xd800 && codePoint < 0xdc00) {
			InStringPtr srcData = *srcPtr;
			if (((srcData[0] << 8) | srcData[1]) != ((static_cast<uint8_t>('\\') << 8) | static_cast<uint8_t>('u'))) {
				return false;
			}
			uint32_t codePoint2 = hexToU32Nocheck(srcData + 2);
			uint32_t lowBit = codePoint2 - 0xdc00;
			if (lowBit >> 10) {
				return false;
			}

			codePoint = (((codePoint - 0xd800) << 10) | lowBit) + 0x10000;
			*srcPtr += 6;
		} else if (codePoint >= 0xdc00 && codePoint <= 0xdfff) {
			return false;
		}
		size_t offset = codePointToUtf8(codePoint, *dstPtr);
		*dstPtr += offset;
		return offset > 0;
	}

	__forceinline OutStringPtr parseString(InStringPtr src, OutStringPtr dst) {
		while (1) {
			auto bsQuote = BackslashAndQuote<SimdBase256>::copyAndFind(src, dst);
			if (bsQuote.hasQuoteFirst()) {
				return dst + bsQuote.quoteIndex();
			}
			if (bsQuote.hasBackslash()) {
				auto bsDist = bsQuote.backslashIndex();
				uint8_t escapeChar = src[bsDist + 1];
				if (escapeChar == 'u') {
					src += bsDist;
					dst += bsDist;
					if (!handleUnicodeCodepoint(&src, &dst)) {
						return nullptr;
					}
				} else {
					uint8_t escapeResult = escapeMap[escapeChar];
					if (escapeResult == 0u) {
						return nullptr;
					}
					dst[bsDist] = escapeResult;
					src += bsDist + 2ull;
					dst += bsDist + 1ull;
				}
			} else {
				src += BackslashAndQuote<SimdBase256>::bytesProcessed;
				dst += BackslashAndQuote<SimdBase256>::bytesProcessed;
			}
		}
		return nullptr;
	}
}