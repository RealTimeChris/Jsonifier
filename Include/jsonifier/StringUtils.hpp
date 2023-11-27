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

	template<typename value_type> JSONIFIER_INLINE static uint32_t hexToU32NoCheck(const value_type* string1) {
		uint32_t v1 = digitToVal32[630 + string1[0]];
		uint32_t v2 = digitToVal32[420 + string1[1]];
		uint32_t v3 = digitToVal32[210 + string1[2]];
		uint32_t v4 = digitToVal32[0 + string1[3]];
		return v1 | v2 | v3 | v4;
	}

	template<typename value_type> JSONIFIER_INLINE static uint32_t codePointToUtf8(uint32_t codePoint, value_type* c) {
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

	template<typename value_type01, typename value_type02> JSONIFIER_INLINE bool handleUnicodeCodePoint(value_type01* srcPtr, value_type02* dstPtr) {
		static constexpr uint32_t subCodePoint = 0xfffd;
		uint32_t codePoint					   = hexToU32NoCheck(*srcPtr + 2);
		*srcPtr += 6;
		if (codePoint >= 0xd800 && codePoint < 0xdc00) {
			value_type01 srcData = *srcPtr;
			if (((srcData[0] << 8) | srcData[1]) != ((static_cast<std::remove_pointer_t<value_type01>>(0x5Cu) << 8) | static_cast<std::remove_pointer_t<value_type01>>(0x75u))) {
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

	template<typename value_type> constexpr value_type hasZero(value_type chunk) noexcept {
		return static_cast<value_type>((((chunk - static_cast<value_type>(0x0101010101010101)) & ~chunk) & static_cast<value_type>(0x8080808080808080)));
	}

	template<typename value_type> constexpr value_type hasQuote(value_type chunk) noexcept {
		return static_cast<value_type>(hasZero(chunk ^ static_cast<value_type>(0b0010001000100010001000100010001000100010001000100010001000100010)));
	}

	template<typename value_type> constexpr value_type hasEscape(value_type chunk) noexcept {
		return static_cast<value_type>(hasZero(chunk ^ static_cast<value_type>(0b0101110001011100010111000101110001011100010111000101110001011100)));
	}

	template<typename value_type> constexpr value_type isLess16(value_type c) noexcept {
		return static_cast<value_type>(hasZero(c & static_cast<value_type>(0b1111000011110000111100001111000011110000111100001111000011110000)));
	}

	template<typename simd_type, typename integer_type, typename char_type01, typename char_type02>
	JSONIFIER_INLINE static integer_type copyAndFindParse(char_type01* string1, char_type02* string2) {
		const simd_type values{ gatherValuesU<simd_type>(string1) };
		std::memcpy(string2, string1, sizeof(simd_type));
		return tzcnt(static_cast<integer_type>(simd_base::cmpeq(values, backslashes<simd_type>) | simd_base::cmpeq(values, quotes<simd_type>)));
	}

	template<jsonifier::concepts::unsigned_t simd_type, jsonifier::concepts::unsigned_t integer_type, typename char_type01, typename char_type02>
	JSONIFIER_INLINE static integer_type copyAndFindParse(char_type01* string1, char_type02* string2) {
		integer_type newValue{};
		std::memcpy(string2, string1, sizeof(simd_type));
		std::memcpy(&newValue, string1, sizeof(simd_type));
		return static_cast<integer_type>(tzcnt(static_cast<integer_type>(hasQuote(newValue) | hasEscape(newValue))) >> 3u);
	}

	template<typename simd_type, typename integer_type, typename char_type01, typename char_type02>
	JSONIFIER_INLINE static integer_type copyAndFindSerialize(char_type01* string1, char_type02* string2) {
		const simd_type values{ gatherValuesU<simd_type>(string1) };
		std::memcpy(string2, string1, sizeof(simd_type));
		return tzcnt(static_cast<integer_type>(simd_base::cmpeq(simd_base::opShuffle(escapeableTable00<simd_type>, values), values) |
			simd_base::cmpeq(simd_base::opShuffle(escapeableTable01<simd_type>, values), values)));
	}

	template<jsonifier::concepts::unsigned_t simd_type, jsonifier::concepts::unsigned_t integer_type, typename char_type01, typename char_type02>
	JSONIFIER_INLINE static integer_type copyAndFindSerialize(char_type01* string1, char_type02* string2) {
		integer_type newValue{};
		std::memcpy(string2, string1, sizeof(integer_type));
		std::memcpy(&newValue, string1, sizeof(integer_type));
		return static_cast<integer_type>(tzcnt(static_cast<integer_type>(hasQuote(newValue) | hasEscape(newValue) | isLess16(newValue))) >> 3u);
	}

	template<size_t index = 0, typename char_type01, typename char_type02>
	JSONIFIER_INLINE char_type02* parseString(const char_type01* string1, char_type02* string2, uint64_t lengthNew) {
		using simd_type							 = typename jsonifier::concepts::get_type_at_index<avx_integer_list, index>::type::type;
		using integer_type						 = typename jsonifier::concepts::get_type_at_index<avx_integer_list, index>::type::integer_type;
		static constexpr uint64_t bytesProcessed = jsonifier::concepts::get_type_at_index<avx_integer_list, index>::type::bytesProcessed;
		static constexpr integer_type mask		 = jsonifier::concepts::get_type_at_index<avx_integer_list, index>::type::mask;
		while (static_cast<int64_t>(lengthNew) > 0) {
			if constexpr (index < avx_integer_list::size - 1) {
				if (lengthNew < bytesProcessed / 2) {
					return parseString<index + 1>(string1, string2, lengthNew);
				}
			}
			integer_type nextBackslashOrQuote = copyAndFindParse<simd_type, integer_type>(string1, string2);
			if (nextBackslashOrQuote != mask) {
				auto escapeChar = string1[nextBackslashOrQuote];
				if (escapeChar == 0x22u) {
					return string2 + nextBackslashOrQuote;
				} else if (escapeChar == 0x5Cu) {
					escapeChar = string1[nextBackslashOrQuote + 1];
					if (escapeChar == 0x75u) {
						lengthNew -= nextBackslashOrQuote;
						string1 += nextBackslashOrQuote;
						string2 += nextBackslashOrQuote;
						if (!handleUnicodeCodePoint(&string1, &string2)) {
							return nullptr;
						}
						continue;
					}
					escapeChar = escapeMap[escapeChar];
					if (escapeChar == 0u) {
						return static_cast<char_type02*>(nullptr);
					}
					string2[nextBackslashOrQuote] = escapeChar;
					lengthNew -= nextBackslashOrQuote + 2ull;
					string2 += nextBackslashOrQuote + 1ull;
					string1 += nextBackslashOrQuote + 2ull;
				}
			} else {
				lengthNew -= bytesProcessed;
				string2 += bytesProcessed;
				string1 += bytesProcessed;
			}
		}
		return string2;
	}

	template<size_t index = 0, typename char_type01, typename char_type02>
	JSONIFIER_INLINE void serializeString(const char_type01* string1, char_type02* string2, uint64_t lengthNew, uint64_t& indexNew) {
		using simd_type							 = typename jsonifier::concepts::get_type_at_index<avx_integer_list, index>::type::type;
		using integer_type						 = typename jsonifier::concepts::get_type_at_index<avx_integer_list, index>::type::integer_type;
		static constexpr uint64_t bytesProcessed = jsonifier::concepts::get_type_at_index<avx_integer_list, index>::type::bytesProcessed;
		static constexpr integer_type mask		 = jsonifier::concepts::get_type_at_index<avx_integer_list, index>::type::mask;
		while (static_cast<int64_t>(lengthNew) > 0) {
			if constexpr (index < avx_integer_list::size - 1) {
				if (lengthNew < bytesProcessed / 2) {
					return serializeString<index + 1>(string1, string2, lengthNew, indexNew);
				}
			}
			integer_type nextEscapeable = copyAndFindSerialize<simd_type, integer_type>(string1, string2);
			if (nextEscapeable != mask) {
				auto escapeResult = escapeTable[string1[nextEscapeable]];
				if (escapeResult == 0u) {
					indexNew += nextEscapeable;
					return;
				}
				lengthNew -= nextEscapeable;
				indexNew += nextEscapeable;
				string1 += nextEscapeable;
				string2 += nextEscapeable;
				std::memcpy(string2, &escapeResult, 2);
				indexNew += 2ULL;
				string2 += 2ULL;
				--lengthNew;
				++string1;
			} else {
				lengthNew -= bytesProcessed;
				indexNew += bytesProcessed;
				string1 += bytesProcessed;
				string2 += bytesProcessed;
			}
		}
	}

	JSONIFIER_INLINE bool parseBool(string_view_ptr json) {
		uint8_t valueNew[5]{ "true" };
		return std::memcmp(valueNew, json, 4) == 0;
	}
}// namespace jsonifier_internal