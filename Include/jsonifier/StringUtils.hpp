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
		template<typename value_type01, typename value_type02> inline static uint16_t copyAndFind(const value_type01* source, value_type02* dest) {
			auto values = gatherValuesU<simd_int_128>(source);
			storeu(values, dest);
			uint16_t returnData{};
			returnData = simd_base::cmpeq(values, backslashes128);
			returnData |= simd_base::cmpeq(values, quotes128);
			return returnData;
		}

	  protected:
		static constexpr simd_int_128 backslashes128{ simdFromValue<simd_int_128>(0x5Cu) };
		static constexpr simd_int_128 quotes128{ simdFromValue<simd_int_128>(0x22u) };
	};

	class escapeable {
	  public:
		template<typename value_type01, typename value_type02> inline static uint16_t copyAndFind(const value_type01* source, value_type02* dest) {
			simd_int_128 values(gatherValuesU<simd_int_128>(source));
			storeu(values, dest);
			uint16_t returnData{};
			returnData = simd_base::cmpeq(simd_base::shuffle(escapeableChars02, values), values);
			returnData |= simd_base::cmpeq(simd_base::shuffle(escapeableChars03, values), values);
			return returnData;
		}

	  protected:
		static constexpr std::array<uint8_t, 16> escapeableChars00{ 0x00u, 0x00u, 0x22u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x08u, 0x00u, 0x00u, 0x00u, 0x0Cu, 0x0Du, 0x00u,
			0x00u };
		static constexpr std::array<uint8_t, 16> escapeableChars01{ 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x09u, 0x0Au, 0x00u, 0x5Cu, 0x00u, 0x00u,
			0x00u };
		static constexpr simd_int_128 escapeableChars02{ simdFromTable<simd_int_128>(escapeableChars00) };
		static constexpr simd_int_128 escapeableChars03{ simdFromTable<simd_int_128>(escapeableChars01) };
	};

	template<typename value_type> inline static uint32_t hexToU32NoCheck(const value_type* source) {
		uint32_t v1 = digitToVal32<uint32_t>[630 + source[0]];
		uint32_t v2 = digitToVal32<uint32_t>[420 + source[1]];
		uint32_t v3 = digitToVal32<uint32_t>[210 + source[2]];
		uint32_t v4 = digitToVal32<uint32_t>[0 + source[3]];
		return v1 | v2 | v3 | v4;
	}

	template<typename value_type> inline static uint32_t codePointToUtf8(uint32_t codePoint, value_type* c) {
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

	template<typename value_type01, typename value_type02> inline bool handleUnicodeCodePoint(value_type01* srcPtr, value_type02* dstPtr) {
		static constexpr uint32_t subCodePoint = 0xfffd;
		static constexpr uint8_t backslash{ '\\' };
		static constexpr uint8_t u{ 'u' };
		uint32_t codePoint = hexToU32NoCheck(*srcPtr + 2);
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

	template<typename value_type01, typename value_type02> inline static value_type02* parseString(const value_type01* source, value_type02* dest, uint64_t lengthNew) {
		while (static_cast<int64_t>(lengthNew) >= static_cast<int64_t>(0)) {
			auto bsDist = backslash_and_quote::copyAndFind(source, dest);
			if (bsDist != 0) {
				bsDist					= tzcnt(bsDist);
				value_type01 escapeChar = source[bsDist];
				if (escapeChar == '"') {
					return dest + bsDist;
				}
				escapeChar = source[bsDist + 1];
				if (escapeChar == 'u') {
					source += bsDist;
					dest += bsDist;
					handleUnicodeCodePoint(&source, &dest);
				} else {
					uint8_t escapeResult = escapeMap<uint8_t>[escapeChar];
					if (escapeResult == 0u) {
						return nullptr;
					}
					dest[bsDist] = escapeResult;
					dest += bsDist + 1ULL;
					source += bsDist + 2ULL;
				}
			} else {
				dest += 16;
				source += 16;
			}
		}
		return nullptr;
	}

	template<typename value_type01, typename value_type02> inline value_type01* serializeString(value_type01* source, value_type02* dest, uint64_t lengthNew, uint64_t& indexNew) {
		while (static_cast<int64_t>(lengthNew) >= static_cast<int64_t>(16)) {
			auto nextEscapeable = escapeable::copyAndFind(source, dest);
			if (nextEscapeable != 0) {
				nextEscapeable		 = tzcnt(nextEscapeable);
				uint8_t escapeResult = escapeableChars<uint8_t>[static_cast<uint8_t>(source[nextEscapeable])];
				if (escapeResult == 0u) {
					return source;
				}
				dest[nextEscapeable]							   = '\\';
				dest[static_cast<uint64_t>(nextEscapeable) + 1ULL] = escapeResult;
				dest += static_cast<uint64_t>(nextEscapeable) + 2ULL;
				indexNew += static_cast<uint64_t>(nextEscapeable) + 2ULL;
				lengthNew -= (nextEscapeable + 1ULL);
				source += nextEscapeable + 1ULL;
			} else {
				lengthNew -= 16ULL;
				indexNew += 16ULL;
				source += 16ULL;
				dest += 16ULL;
			}
		}
		return source;
	}

	inline bool parseBool(string_view_ptr json) {
		uint8_t valueNew[5]{ "true" };
		return std::memcmp(valueNew, json, 4) == 0;
	}
}// namespace jsonifier_internal
