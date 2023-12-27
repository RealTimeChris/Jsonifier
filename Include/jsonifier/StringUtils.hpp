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
#include <jsonifier/ISADetection.hpp>
#include <jsonifier/Simd.hpp>

namespace jsonifier_internal {

	template<typename value_type> JSONIFIER_INLINE uint32_t hexToU32NoCheck(const value_type* string1) {
		return digitToVal32[630 + string1[0]] | digitToVal32[420 + string1[1]] | digitToVal32[210 + string1[2]] | digitToVal32[0 + string1[3]];
	}

	template<typename value_type> JSONIFIER_INLINE uint32_t codePointToUtf8(uint32_t codePoint, value_type* c) {
		if (codePoint <= 0x7F) {
			c[0] = static_cast<value_type>(codePoint);
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
				codePoint02			 = codePoint02 - 0xdc00;
				if (codePoint02 >> 10) {
					codePoint = subCodePoint;
				} else {
					codePoint = (((codePoint - 0xd800) << 10) | codePoint02) + 0x10000;
					*srcPtr += 6;
				}
			}
		} else if (codePoint >= 0xdc00 && codePoint <= 0xdfff) {
			codePoint = subCodePoint;
		}
		uint32_t offset = codePointToUtf8(codePoint, *dstPtr);
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
	JSONIFIER_INLINE integer_type copyAndFindParse(char_type01* string1, char_type02* string2, simd_type& simdValue) {
		simdValue = gatherValuesU<simd_type>(string1);
		std::memcpy(string2, string1, sizeof(simd_type));
		return tzcnt(static_cast<integer_type>(simd_base::opCmpEq(simdValue, backslashes<simd_type>) | simd_base::opCmpEq(simdValue, quotes<simd_type>)));
	}

	template<jsonifier::concepts::unsigned_t simd_type, jsonifier::concepts::unsigned_t integer_type, typename char_type01, typename char_type02>
	JSONIFIER_INLINE integer_type copyAndFindParse(char_type01* string1, char_type02* string2, simd_type& simdValue) {
		std::memcpy(&simdValue, string1, sizeof(simd_type));
		std::memcpy(string2, string1, sizeof(simd_type));
		return static_cast<integer_type>(tzcnt(static_cast<integer_type>(hasQuote(simdValue) | hasEscape(simdValue))) >> 3u);
	}

	template<typename simd_type, typename integer_type, typename char_type01, typename char_type02>
	JSONIFIER_INLINE integer_type copyAndFindSerialize(char_type01* string1, char_type02* string2, simd_type& simdValue) {
		simdValue = gatherValuesU<simd_type>(string1);
		std::memcpy(string2, string1, sizeof(simd_type));
		return tzcnt(static_cast<integer_type>(simd_base::opCmpEq(simd_base::opShuffle(escapeableTable00<simd_type>, simdValue), simdValue) |
			simd_base::opCmpEq(simd_base::opShuffle(escapeableTable01<simd_type>, simdValue), simdValue)));
	}

	template<jsonifier::concepts::unsigned_t simd_type, jsonifier::concepts::unsigned_t integer_type, typename char_type01, typename char_type02>
	JSONIFIER_INLINE integer_type copyAndFindSerialize(char_type01* string1, char_type02* string2, simd_type& simdValue) {
		std::memcpy(&simdValue, string1, sizeof(integer_type));
		std::memcpy(string2, string1, sizeof(integer_type));
		return static_cast<integer_type>(tzcnt(static_cast<integer_type>(hasQuote(simdValue) | hasEscape(simdValue) | isLess16(simdValue))) >> 3u);
	}

	template<typename char_type01, typename char_type02> JSONIFIER_INLINE char_type02* parseString(const char_type01* string1, char_type02* string2, uint64_t lengthNew);

	template<uint64_t index = 0, typename char_type01, typename char_type02>
	JSONIFIER_INLINE char_type02* parseStringImpl(const char_type01* string1, char_type02* string2, uint64_t lengthNew) {
		using integer_type						 = typename jsonifier::concepts::get_type_at_index<avx_integer_list, index>::type::integer_type;
		using simd_type							 = typename jsonifier::concepts::get_type_at_index<avx_integer_list, index>::type::type;
		static constexpr uint64_t bytesProcessed = jsonifier::concepts::get_type_at_index<avx_integer_list, index>::type::bytesProcessed;
		static constexpr integer_type mask		 = jsonifier::concepts::get_type_at_index<avx_integer_list, index>::type::mask;
		simd_type collectionValue{};
		while (lengthNew >= bytesProcessed) {
			integer_type nextBackslashOrQuote = copyAndFindParse<simd_type, integer_type>(string1, string2, collectionValue);
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
							return static_cast<char_type02*>(nullptr);
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
				} else {
					lengthNew -= bytesProcessed;
					string2 += bytesProcessed;
					string1 += bytesProcessed;
				}
			} else {
				lengthNew -= bytesProcessed;
				string2 += bytesProcessed;
				string1 += bytesProcessed;
			}
		}
		if (static_cast<int64_t>(lengthNew) > 0) {
			return parseString(string1, string2, lengthNew);
		}
		return string2;
	}

	template<typename char_type01, typename char_type02> JSONIFIER_INLINE char_type02* parseShortStringImpl(const char_type01* string1, char_type02* string2, uint64_t lengthNew) {
		while (static_cast<int64_t>(lengthNew) > 0) {
			*string2 = *string1;
			if (*string1 == 0x22u || *string1 == 0x5Cu) {
				auto escapeChar = *string1;
				if (escapeChar == 0x22u) {
					return string2;
				} else if (escapeChar == 0x5Cu) {
					escapeChar = string1[1];
					if (escapeChar == 0x75u) {
						if (!handleUnicodeCodePoint(&string1, &string2)) {
							return nullptr;
						}
						continue;
					}
					escapeChar = escapeMap[escapeChar];
					if (escapeChar == 0) {
						return nullptr;
					}
					string2[0] = escapeChar;
					string2 += 1;
					string1 += 2;
					lengthNew -= 2;
				}
			} else {
				++string2;
				++string1;
				--lengthNew;
			}
		}
		return string2;
	}

	template<typename char_type01, typename char_type02> JSONIFIER_INLINE char_type02* parseString(const char_type01* string1, char_type02* string2, uint64_t lengthNew) {
		if constexpr (avx_integer_list::size >= 4) {
			if (static_cast<int64_t>(lengthNew) >= 64) {
				return parseStringImpl<0>(string1, string2, lengthNew);
			} else if (static_cast<int64_t>(lengthNew) >= 32) {
				return parseStringImpl<1>(string1, string2, lengthNew);
			} else if (static_cast<int64_t>(lengthNew) >= 16) {
				return parseStringImpl<2>(string1, string2, lengthNew);
			} else if (static_cast<int64_t>(lengthNew) >= 8) {
				return parseStringImpl<3>(string1, string2, lengthNew);
			} else {
				return parseShortStringImpl(string1, string2, lengthNew);
			}
		} else if constexpr (avx_integer_list::size >= 3) {
			if (static_cast<int64_t>(lengthNew) >= 32) {
				return parseStringImpl<0>(string1, string2, lengthNew);
			} else if (static_cast<int64_t>(lengthNew) >= 16) {
				return parseStringImpl<1>(string1, string2, lengthNew);
			} else if (static_cast<int64_t>(lengthNew) >= 8) {
				return parseStringImpl<2>(string1, string2, lengthNew);
			} else {
				return parseShortStringImpl(string1, string2, lengthNew);
			}
		} else {
			if (static_cast<int64_t>(lengthNew) >= 16) {
				return parseStringImpl<0>(string1, string2, lengthNew);
			} else if (static_cast<int64_t>(lengthNew) >= 8) {
				return parseStringImpl<1>(string1, string2, lengthNew);
			} else {
				return parseShortStringImpl(string1, string2, lengthNew);
			}
		}
	}

	template<typename char_type01, typename char_type02>
	JSONIFIER_INLINE void serializeString(const char_type01* string1, char_type02* string2, uint64_t lengthNew, uint64_t& indexNew);

	template<uint64_t index = 0, typename char_type01, typename char_type02>
	JSONIFIER_INLINE void serializeStringImpl(const char_type01* string1, char_type02* string2, uint64_t lengthNew, uint64_t& indexNew) {
		using integer_type						 = typename jsonifier::concepts::get_type_at_index<avx_integer_list, index>::type::integer_type;
		using simd_type							 = typename jsonifier::concepts::get_type_at_index<avx_integer_list, index>::type::type;
		static constexpr uint64_t bytesProcessed = jsonifier::concepts::get_type_at_index<avx_integer_list, index>::type::bytesProcessed;
		static constexpr integer_type mask		 = jsonifier::concepts::get_type_at_index<avx_integer_list, index>::type::mask;
		simd_type collectionValue{};
		while (lengthNew >= bytesProcessed) {
			integer_type nextEscapeable = copyAndFindSerialize<simd_type, integer_type>(string1, string2, collectionValue);
			if (nextEscapeable != mask) {
				auto escapeResult = escapeTable[static_cast<uint64_t>(string1[nextEscapeable])];
				if (escapeResult != 0u) [[likely]] {
					lengthNew -= nextEscapeable;
					indexNew += nextEscapeable;
					string2 += nextEscapeable;
					string1 += nextEscapeable;
					std::memcpy(string2, &escapeResult, 2);
					indexNew += 2ULL;
					string2 += 2ULL;
					--lengthNew;
					++string1;
				} else {
					indexNew += nextEscapeable;
					return;
				}
			} else {
				lengthNew -= bytesProcessed;
				indexNew += bytesProcessed;
				string2 += bytesProcessed;
				string1 += bytesProcessed;
			}
		}
		if (static_cast<int64_t>(lengthNew) > 0) {
			return serializeString(string1, string2, lengthNew, indexNew);
		}
	}

	template<typename char_type01, typename char_type02>
	JSONIFIER_INLINE void serializeShortStringImpl(const char_type01* string1, char_type02* string2, uint64_t lengthNew, uint64_t& indexNew) {
		const auto* const e = string1 + lengthNew;
		for (; string1 < e; ++string1) {
			if (const auto escaped = escapeTable[uint8_t(*string1)]; escaped) [[likely]] {
				std::memcpy(string2, &escaped, 2);
				string2 += 2;
				indexNew += 2;
			} else {
				std::memcpy(string2, string1, 1);
				++indexNew;
				++string2;
			}
		}
	}

	template<typename char_type01, typename char_type02>
	JSONIFIER_INLINE void serializeString(const char_type01* string1, char_type02* string2, uint64_t lengthNew, uint64_t& indexNew) {
		if constexpr (avx_integer_list::size >= 4) {
			if (static_cast<int64_t>(lengthNew) >= 64) {
				return serializeStringImpl<0>(string1, string2, lengthNew, indexNew);
			} else if (static_cast<int64_t>(lengthNew) >= 32) {
				return serializeStringImpl<1>(string1, string2, lengthNew, indexNew);
			} else if (static_cast<int64_t>(lengthNew) >= 16) {
				return serializeStringImpl<2>(string1, string2, lengthNew, indexNew);
			} else if (static_cast<int64_t>(lengthNew) >= 8) {
				return serializeStringImpl<3>(string1, string2, lengthNew, indexNew);
			} else {
				return serializeShortStringImpl(string1, string2, lengthNew, indexNew);
			}
		} else if constexpr (avx_integer_list::size >= 3) {
			if (static_cast<int64_t>(lengthNew) >= 32) {
				return serializeStringImpl<0>(string1, string2, lengthNew, indexNew);
			} else if (static_cast<int64_t>(lengthNew) >= 16) {
				return serializeStringImpl<1>(string1, string2, lengthNew, indexNew);
			} else if (static_cast<int64_t>(lengthNew) >= 8) {
				return serializeStringImpl<2>(string1, string2, lengthNew, indexNew);
			} else {
				return serializeShortStringImpl(string1, string2, lengthNew, indexNew);
			}
		} else {
			if (static_cast<int64_t>(lengthNew) >= 16) {
				return serializeStringImpl<0>(string1, string2, lengthNew, indexNew);
			} else if (static_cast<int64_t>(lengthNew) >= 8) {
				return serializeStringImpl<1>(string1, string2, lengthNew, indexNew);
			} else {
				return serializeShortStringImpl(string1, string2, lengthNew, indexNew);
			}
		}
	}

	JSONIFIER_INLINE bool parseBool(bool& value, string_view_ptr json) {
		uint8_t valueNew00[5]{ "true" };
		uint8_t valueNew01[6]{ "false" };
		if (compareShort(valueNew00, json, 4)) {
			value = true;
			return true;
		} else if (compareShort(valueNew01, json, 5)) {
			value = false;
			return true;
		} else {
			return false;
		}
	}

	JSONIFIER_INLINE bool parseNull(string_view_ptr json) {
		uint8_t valueNew00[5]{ "null" };
		if (compareShort(valueNew00, json, 4)) {
			return true;
		} else {
			return false;
		}
	}
}// namespace jsonifier_internal