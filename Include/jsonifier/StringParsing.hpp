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
	License along with this library; if not, Write to the Free Software
	Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301
	USA
*/
/// https://github.com/RealTimeChris/Jsonifier
/// Feb 3, 2023
/// Most of the code in this header was copied from Simdjson - https://github.com/simdjson
#pragma once

#include <jsonifier/Base.hpp>
#include <jsonifier/Tables.hpp>
#include <jsonifier/Simd.hpp>

namespace Jsonifier {

	struct SimdBase128;
	struct SimdBase256;
	template<typename SimdBase> class BackslashAndQuote;

	template<> class BackslashAndQuote<SimdBase128> {
	  public:
		static const uint32_t bytesProcessed = 16;

		BackslashAndQuote<SimdBase128> inline static copyAndFind(StringViewPtr source, StringBufferPtr dest) {
			SimdBase128 values(reinterpret_cast<StringViewPtr>(source));
			values.store(dest);
			BackslashAndQuote returnData{};
			returnData.bsBits = static_cast<uint16_t>((values == '\\').toBitMask());
			returnData.quoteBits = static_cast<uint16_t>((values == '\"').toBitMask());
			return returnData;
		}

		inline bool hasQuoteFirst() {
			return ((bsBits - 1) & quoteBits) != 0;
		}

		inline bool hasBackslash() {
			return ((quoteBits - 1) & bsBits) != 0;
		}

		inline int16_t quoteIndex() {
			return _tzcnt_u16(quoteBits);
		}

		inline int16_t backslashIndex() {
			return _tzcnt_u16(bsBits);
		}

	  protected:
		uint16_t quoteBits{};
		uint16_t bsBits{};
	};

	template<> class BackslashAndQuote<SimdBase256> {
	  public:
		static const uint32_t bytesProcessed = 32;

		BackslashAndQuote<SimdBase256> inline static copyAndFind(StringViewPtr source, StringBufferPtr dest) {
			SimdBase256 values(reinterpret_cast<StringViewPtr>(source));
			values.store(dest);
			BackslashAndQuote returnData{};
			returnData.bsBits = static_cast<uint32_t>((values == '\\').toBitMask());
			returnData.quoteBits = static_cast<uint32_t>((values == '\"').toBitMask());
			return returnData;
		}

		inline bool hasQuoteFirst() {
			return ((bsBits - 1) & quoteBits) != 0;
		}

		inline bool hasBackslash() {
			return ((quoteBits - 1) & bsBits) != 0;
		}

		inline int32_t quoteIndex() {
			return _tzcnt_u32(quoteBits);
		}

		inline int32_t backslashIndex() {
			return _tzcnt_u32(bsBits);
		}

	  protected:
		uint32_t quoteBits{};
		uint32_t bsBits{};
	};

	inline static uint32_t stringToUint32(StringViewPtr str) {
		uint32_t val{ *reinterpret_cast<const uint32_t*>(str) };
		return val;
	}

	inline static uint32_t str4ncmp(StringViewPtr source, const char* atom) {
		return stringToUint32(reinterpret_cast<StringViewPtr>(source)) ^ stringToUint32(reinterpret_cast<StringViewPtr>(atom));
	}

	inline static constexpr uint32_t isStructuralOrWhitespace(uint8_t c) {
		return structuralOrWhitespace[c];
	}

	inline static constexpr uint32_t isNotStructuralOrWhitespace(uint8_t c) {
		return structuralOrWhitespaceNegated[c];
	}

	inline static size_t codePointToUtf8(uint32_t codePoint, StringBufferPtr c) {
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

	inline static uint32_t hexToU32NoCheck(StringViewPtr src) {
		uint32_t v1 = digitToVal32[630 + src[0]];
		uint32_t v2 = digitToVal32[420 + src[1]];
		uint32_t v3 = digitToVal32[210 + src[2]];
		uint32_t v4 = digitToVal32[0 + src[3]];
		return v1 | v2 | v3 | v4;
	}

	inline static bool handleUnicodeCodePoint(const uint8_t** srcPtr, uint8_t** dstPtr, bool allowReplacement) {
		constexpr uint32_t substitutionCodePoint = 0xfffd;
		uint32_t codePoint = hexToU32NoCheck(*srcPtr + 2);
		*srcPtr += 6;
		if (codePoint >= 0xd800 && codePoint < 0xdc00) {
			const uint8_t* srcData = *srcPtr;
			if (((srcData[0] << 8) | srcData[1]) != ((static_cast<uint8_t>('\\') << 8) | static_cast<uint8_t>('u'))) {
				if (!allowReplacement) {
					return false;
				}
				codePoint = substitutionCodePoint;
			} else {
				uint32_t codePoint2 = hexToU32NoCheck(srcData + 2);
				uint32_t lowBit = codePoint2 - 0xdc00;
				if (lowBit >> 10) {
					if (!allowReplacement) {
						return false;
					}
					codePoint = substitutionCodePoint;
				} else {
					codePoint = (((codePoint - 0xd800) << 10) | lowBit) + 0x10000;
					*srcPtr += 6;
				}
			}
		} else if (codePoint >= 0xdc00 && codePoint <= 0xdfff) {
			if (!allowReplacement) {
				return false;
			}
			codePoint = substitutionCodePoint;
		}
		size_t offset = codePointToUtf8(codePoint, *dstPtr);
		*dstPtr += offset;
		return offset > 0;
	}

	inline static StringBufferPtr parseString(StringViewPtr source, StringBufferPtr dest, size_t length) {
		while (length > 0) {
			BackslashAndQuote<SimdBase256> bsQuote = BackslashAndQuote<SimdBase256>::copyAndFind(source, dest);
			if (bsQuote.hasQuoteFirst()) {
				return dest + bsQuote.quoteIndex();
			}
			if (bsQuote.hasBackslash()) {
				uint32_t bsDist = bsQuote.backslashIndex();
				uint8_t escapeChar = source[bsDist + 1];
				if (escapeChar == 'u') {
					source += bsDist;
					dest += bsDist;
					if (!handleUnicodeCodePoint(&source, &dest, false)) {
						return dest;
					}
				} else {
					uint8_t escapeResult = escapeMap[escapeChar];
					if (escapeResult == 0u) {
						return dest;
					}
					dest[bsDist] = escapeResult;
					source += bsDist + 2ull;
					dest += bsDist + 1ull;
				}
			} else {
				source += BackslashAndQuote<SimdBase256>::bytesProcessed;
				dest += BackslashAndQuote<SimdBase256>::bytesProcessed;
			}
			length -= BackslashAndQuote<SimdBase256>::bytesProcessed;
		}
		return dest;
	}

	inline static bool parseBool(StringViewPtr json) noexcept {
		uint32_t notTrue = str4ncmp(json, "true");
		uint32_t notFalse = str4ncmp(json, "fals") | (json[4] ^ 'e');
		bool error = (notTrue && notFalse) || isNotStructuralOrWhitespace(json[notTrue ? 5 : 4]);
		if (error) {
			return {};
		}
		return !notTrue;
	}
}