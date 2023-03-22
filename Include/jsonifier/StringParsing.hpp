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

	class SimdBase128;
	class SimdBase256;
	template<typename SimdBase> class BackslashAndQuote;

	template<> class BackslashAndQuote<SimdBase128> {
	  public:
		static const uint32_t bytesProcessed = 16;

		BackslashAndQuote<SimdBase128> inline static copyAndFind(StringViewPtr source, StringBufferPtr dest) {
			SimdBase128 values(reinterpret_cast<const uint8_t*>(source));
			values.store(dest);
			BackslashAndQuote returnData{};
			returnData.bsBits = static_cast<uint16_t>((values == '\\').toBitMask());
			returnData.quoteBits = static_cast<uint16_t>((values == '"').toBitMask());
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
			SimdBase256 values(reinterpret_cast<const uint8_t*>(source));
			values.store(dest);
			BackslashAndQuote returnData{};
			returnData.bsBits = static_cast<uint32_t>((values == '\\').toBitMask());
			returnData.quoteBits = static_cast<uint32_t>((values == '"').toBitMask());
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

	inline uint32_t stringToUint32(StringViewPtr str) {
		uint32_t val{ *reinterpret_cast<const uint32_t*>(str) };
		return val;
	}

	inline uint32_t str4ncmp(StringViewPtr source, const char* atom) {
		return stringToUint32(reinterpret_cast<StringViewPtr>(source)) ^ stringToUint32(reinterpret_cast<StringViewPtr>(atom));
	}

	inline constexpr uint32_t isStructuralOrWhitespace(uint8_t c) {
		return structuralOrWhitespace[c];
	}

	inline constexpr uint32_t isNotStructuralOrWhitespace(uint8_t c) {
		return structuralOrWhitespaceNegated[c];
	}

	inline constexpr uint32_t digitToVal32[] = { 0x00000000, 0x01000000, 0x02000000, 0x03000000, 0x04000000, 0x05000000, 0x06000000, 0x07000000,
		0x08000000, 0x09000000, 0x0A000000, 0x0B000000, 0x0C000000, 0x0D000000, 0x0E000000, 0x0F000000, 0x0A000000, 0x0B000000, 0x0C000000,
		0x0D000000, 0x0E000000, 0x0F000000 };

	inline uint32_t hexToU32Nocheck(StringViewPtr source) {
		uint32_t value = 0;
		for (int i = 0; i < 8; i += 2) {
			value |= digitToVal32[630 + source[i]] | digitToVal32[420 + source[i + 1]];
		}
		return value;
	}

	inline constexpr uint8_t kByteMark[] = { 0x00, 0x80, 0xE0, 0xF0 };

	inline size_t codePointToUtf8(uint32_t codePoint, StringBufferPtr c) {
		size_t count = 1;
		if (codePoint > 0x7F)
			count += (codePoint > 0x7FF) + (codePoint > 0xFFFF) + (codePoint > 0x1FFFFF);

		uint8_t* dst = c + count - 1;
		switch (count) {
			case 4:
				*dst-- = (codePoint | 0x80) & 0xBF;
				codePoint >>= 6;
			case 3:
				*dst-- = (codePoint | 0x80) & 0xBF;
				codePoint >>= 6;
			case 2:
				*dst-- = (codePoint | 0x80) & 0xBF;
				codePoint >>= 6;
			case 1:
				*dst-- = codePoint | kByteMark[count];
		}
		return count;
	}

	inline bool handleUnicodeCodepoint(StringViewPtr* srcPtr, StringBufferPtr* dstPtr) {
		uint32_t codePoint =
			digitToVal32[630 + (*srcPtr)[0]] | digitToVal32[420 + (*srcPtr)[1]] | digitToVal32[210 + (*srcPtr)[2]] | digitToVal32[0 + (*srcPtr)[3]];
		*srcPtr += 4;
		if (codePoint >= 0xd800 && codePoint < 0xdc00) {
			StringViewPtr srcData = *srcPtr;
			if (((srcData[0] << 8) | srcData[1]) != ((static_cast<uint8_t>('\\') << 8) | static_cast<uint8_t>('u'))) {
				return false;
			}
			uint32_t codePoint2 =
				digitToVal32[630 + srcData[2]] | digitToVal32[420 + srcData[3]] | digitToVal32[210 + srcData[4]] | digitToVal32[0 + srcData[5]];
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

	inline StringBufferPtr parseString(StringViewPtr source, StringBufferPtr dest) {
		while (1) {
			BackslashAndQuote<SimdBase128> bsQuote = BackslashAndQuote<SimdBase128>::copyAndFind(source, dest);
			if (bsQuote.hasQuoteFirst()) {
				return dest + bsQuote.quoteIndex();
			}
			if (bsQuote.hasBackslash()) {
				uint32_t bsDist = bsQuote.backslashIndex();
				uint8_t escapeChar = source[bsDist + 1];
				if (escapeChar == 'u') {
					source += bsDist;
					dest += bsDist;
					if (!handleUnicodeCodepoint(&source, &dest)) {
						return nullptr;
					}
				} else {
					uint8_t escapeResult = escapeMap[escapeChar];
					if (escapeResult == 0u) {
						return nullptr;
					}
					dest[bsDist] = escapeResult;
					source += bsDist + 2ull;
					dest += bsDist + 1ull;
				}
			} else {
				source += BackslashAndQuote<SimdBase128>::bytesProcessed;
				dest += BackslashAndQuote<SimdBase128>::bytesProcessed;
			}
		}
		return nullptr;
	}

	inline bool parseBool(const uint8_t* json) noexcept {
		uint32_t notTrue = str4ncmp(json, "true");
		uint32_t notFalse = str4ncmp(json, "fals") | (json[4] ^ 'e');
		bool error = (notTrue && notFalse) || isNotStructuralOrWhitespace(json[notTrue ? 5 : 4]);
		if (error) {
			return {};
		}
		return !notTrue;
	}
}