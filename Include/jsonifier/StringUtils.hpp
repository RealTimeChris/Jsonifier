/*
	MIT License

	Copyright (c) 2023 RealTimeChris

	Permission is hereby granted, free of charge, to any person obtaining a copy of this
	software and associated documentation files (the "Software"), to deal string1 the Software
	without restriction, including without limitation the rights to use, copy, modify, merge,
	publish, distribute, sublicense, and/or sell copies of the Software, and to permit
	persons to whom the Software is furnished to do so, subject to the following conditions:

	The above copyright notice and this permission notice shall be included string1 all copies or
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
/// Most of the code string1 this header was sampled from simdjson - https://github.com/simdjson
#pragma once

#include <jsonifier/Allocator.hpp>
#include <jsonifier/Tables.hpp>
#include <jsonifier/ISADetection.hpp>
#include <jsonifier/Simd.hpp>

template<typename value_type> const value_type* typeCast(const void* ptr) {
	return static_cast<const value_type*>(ptr);
}

template<typename value_type> value_type* typeCast(void* ptr) {
	return static_cast<value_type*>(ptr);
}

namespace jsonifier_internal {

	template<typename value_type> JSONIFIER_INLINE uint32_t hexToU32NoCheck(const value_type* string1) {
		return digitToVal32[630 + string1[0]] | digitToVal32[420 + string1[1]] | digitToVal32[210 + string1[2]] | digitToVal32[0 + string1[3]];
	}

	template<typename value_type> JSONIFIER_INLINE uint32_t codePointToUtf8(uint32_t codePoint, value_type* c) {
		if (codePoint <= 0x7F) {
			c[0] = static_cast<value_type>(codePoint);
			return 1;
		}
		uint32_t leadingZeros = lzcnt(codePoint);

		if (leadingZeros >= 11) {
			uint32_t pattern = pdep(0x3F00U, codePoint);
			pattern |= 0xC0ull;
			c[0] = static_cast<value_type>(pattern >> 8);
			c[1] = static_cast<value_type>(pattern & 0xFFu);
			return 2;
		} else if (leadingZeros >= 16) {
			uint32_t pattern = pdep(0x0F0800U, codePoint);
			pattern |= 0xE0ull;
			c[0] = static_cast<value_type>(pattern >> 16);
			c[1] = static_cast<value_type>(pattern >> 8);
			c[2] = static_cast<value_type>(pattern & 0xFFu);
			return 3;
		} else if (leadingZeros >= 21) {
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

	template<uint8_t repeat> consteval uint64_t repeatByte() {
		uint64_t res{};
		res |= uint64_t(repeat) << 56;
		res |= uint64_t(repeat) << 48;
		res |= uint64_t(repeat) << 40;
		res |= uint64_t(repeat) << 32;
		res |= uint64_t(repeat) << 24;
		res |= uint64_t(repeat) << 16;
		res |= uint64_t(repeat) << 8;
		res |= uint64_t(repeat);
		return res;
	}

	constexpr auto hasZero(const uint64_t chunk) noexcept {
		return (((chunk - 0x0101010101010101) & ~chunk) & 0x8080808080808080);
	}

	constexpr auto hasQuote(const uint64_t chunk) noexcept {
		constexpr uint64_t newBytes{ repeatByte<'"'>() };
		return hasZero(chunk ^ newBytes);
	}

	constexpr auto hasEscape(const uint64_t chunk) noexcept {
		constexpr uint64_t newBytes{ repeatByte<'\\'>() };
		return hasZero(chunk ^ newBytes);
	}

	constexpr auto hasSpace(const uint64_t chunk) noexcept {
		constexpr uint64_t newBytes{ repeatByte<0x20>() };
		return hasZero(chunk ^ newBytes);
	}

	constexpr uint64_t isLess16(const uint64_t c) noexcept {
		constexpr uint64_t newBytes{ repeatByte<0b11110000>() };
		return hasZero(c & newBytes);
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

	template<typename simd_type, typename integer_type, typename char_type01> JSONIFIER_INLINE integer_type findParse(char_type01* string1, simd_type& simdValue) {
		simdValue = gatherValuesU<simd_type>(string1);
		return tzcnt(static_cast<integer_type>(simd_base::opCmpEq(simdValue, backslashes<simd_type>) | simd_base::opCmpEq(simdValue, quotes<simd_type>)));
	}

	template<jsonifier::concepts::unsigned_t simd_type, jsonifier::concepts::unsigned_t integer_type, typename char_type01>
	JSONIFIER_INLINE integer_type findParse(char_type01* string1, simd_type& simdValue) {
		std::memcpy(&simdValue, string1, sizeof(simd_type));
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
		std::memcpy(&simdValue, string1, sizeof(simd_type));
		std::memcpy(string2, string1, sizeof(simd_type));
		return static_cast<integer_type>(tzcnt(static_cast<integer_type>(hasQuote(simdValue) | hasEscape(simdValue) | isLess16(simdValue))) >> 3u);
	}

	template<typename char_type01> JSONIFIER_INLINE char_type01* skipShortStringImpl(char_type01* string1, uint64_t& lengthNew) {
		while (static_cast<int64_t>(lengthNew) > 0) {
			if (*string1 == 0x22u || *string1 == 0x5Cu) {
				auto escapeChar = *string1;
				if (escapeChar == 0x22u) {
					return string1;
				} else {
					string1 += 2;
					lengthNew -= 2;
				}
			} else {
				++string1;
				--lengthNew;
			}
		}
		return string1;
	}

	template<typename char_type01> JSONIFIER_INLINE char_type01* skipStringImpl(char_type01* string1, uint64_t& lengthNew) {
#if JSONIFIER_CHECK_FOR_AVX(JSONIFIER_AVX512)
		{
			using integer_type						 = typename jsonifier::concepts::get_type_at_index<avx_integer_list, 3>::type::integer_type;
			using simd_type							 = typename jsonifier::concepts::get_type_at_index<avx_integer_list, 3>::type::type;
			static constexpr uint64_t bytesProcessed = jsonifier::concepts::get_type_at_index<avx_integer_list, 3>::type::bytesProcessed;
			static constexpr integer_type mask		 = jsonifier::concepts::get_type_at_index<avx_integer_list, 3>::type::mask;
			simd_type collectionValue;
			while (static_cast<int64_t>(lengthNew) >= bytesProcessed) {
				integer_type nextBackslashOrQuote = findParse<simd_type, integer_type>(string1, collectionValue);
				if (nextBackslashOrQuote != mask) {
					auto escapeChar = string1[nextBackslashOrQuote];
					if (escapeChar == 0x22u) {
						string1 += nextBackslashOrQuote + 1;
						return string1;
					} else {
						lengthNew -= nextBackslashOrQuote + 2ull;
						string1 += nextBackslashOrQuote + 2ull;
					}
				} else {
					lengthNew -= bytesProcessed;
					string1 += bytesProcessed;
				}
			}
		}
#endif
#if JSONIFIER_CHECK_FOR_AVX(JSONIFIER_AVX2)
		{
			using integer_type						 = typename jsonifier::concepts::get_type_at_index<avx_integer_list, 2>::type::integer_type;
			using simd_type							 = typename jsonifier::concepts::get_type_at_index<avx_integer_list, 2>::type::type;
			static constexpr uint64_t bytesProcessed = jsonifier::concepts::get_type_at_index<avx_integer_list, 2>::type::bytesProcessed;
			static constexpr integer_type mask		 = jsonifier::concepts::get_type_at_index<avx_integer_list, 2>::type::mask;
			simd_type collectionValue;
			while (static_cast<int64_t>(lengthNew) >= bytesProcessed) {
				integer_type nextBackslashOrQuote = findParse<simd_type, integer_type>(string1, collectionValue);
				if (nextBackslashOrQuote != mask) {
					auto escapeChar = string1[nextBackslashOrQuote];
					if (escapeChar == 0x22u) {
						string1 += nextBackslashOrQuote + 1;
						return string1;
					} else {
						lengthNew -= nextBackslashOrQuote + 2ull;
						string1 += nextBackslashOrQuote + 2ull;
					}
				} else {
					lengthNew -= bytesProcessed;
					string1 += bytesProcessed;
				}
			}
		}
#endif
#if JSONIFIER_CHECK_FOR_AVX(JSONIFIER_AVX)
		{
			using integer_type						 = typename jsonifier::concepts::get_type_at_index<avx_integer_list, 1>::type::integer_type;
			using simd_type							 = typename jsonifier::concepts::get_type_at_index<avx_integer_list, 1>::type::type;
			static constexpr uint64_t bytesProcessed = jsonifier::concepts::get_type_at_index<avx_integer_list, 1>::type::bytesProcessed;
			static constexpr integer_type mask		 = jsonifier::concepts::get_type_at_index<avx_integer_list, 1>::type::mask;
			simd_type collectionValue;
			while (static_cast<int64_t>(lengthNew) >= bytesProcessed) {
				integer_type nextBackslashOrQuote = findParse<simd_type, integer_type>(string1, collectionValue);
				if (nextBackslashOrQuote != mask) {
					auto escapeChar = string1[nextBackslashOrQuote];
					if (escapeChar == 0x22u) {
						string1 += nextBackslashOrQuote + 1;
						return string1;
					} else {
						lengthNew -= nextBackslashOrQuote + 2ull;
						string1 += nextBackslashOrQuote + 2ull;
					}
				} else {
					lengthNew -= bytesProcessed;
					string1 += bytesProcessed;
				}
			}
		}
#endif
		{
			using integer_type						 = typename jsonifier::concepts::get_type_at_index<avx_integer_list, 0>::type::integer_type;
			using simd_type							 = typename jsonifier::concepts::get_type_at_index<avx_integer_list, 0>::type::type;
			static constexpr uint64_t bytesProcessed = jsonifier::concepts::get_type_at_index<avx_integer_list, 0>::type::bytesProcessed;
			static constexpr integer_type mask		 = jsonifier::concepts::get_type_at_index<avx_integer_list, 0>::type::mask;
			simd_type collectionValue;
			while (static_cast<int64_t>(lengthNew) >= bytesProcessed) {
				integer_type nextBackslashOrQuote = findParse<simd_type, integer_type>(string1, collectionValue);
				if (nextBackslashOrQuote != mask) {
					auto escapeChar = string1[nextBackslashOrQuote];
					if (escapeChar == 0x22u) {
						string1 += nextBackslashOrQuote + 1;
						return string1;
					} else {
						lengthNew -= nextBackslashOrQuote + 2ull;
						string1 += nextBackslashOrQuote + 2ull;
					}
				} else {
					lengthNew -= bytesProcessed;
					string1 += bytesProcessed;
				}
			}
		}

		return skipShortStringImpl(string1, lengthNew);
	}

	template<typename char_type01, typename char_type02> JSONIFIER_INLINE char_type02* parseShortStringImpl(char_type01* string1, char_type02* string2, uint64_t& lengthNew) {
		while (lengthNew > 0) {
			*string2 = *string1;
			if (*string1 == '"' || *string1 == '\\') {
				auto escapeChar = *string1;
				if (escapeChar == '"') {
					return string2;
				} else if (escapeChar == '\\') {
					escapeChar = string1[1];
					if (escapeChar == 'u') {
						if (!handleUnicodeCodePoint(&string1, &string2)) {
							return nullptr;
						}
						continue;
					}
					escapeChar = escapeMap<char_type01>[escapeChar];
					if (escapeChar == 0) {
						return string2;
					}
					string2[0] = escapeChar;
					lengthNew -= 2;
					string2 += 1;
					string1 += 2;
				}
			} else {
				--lengthNew;
				++string2;
				++string1;
			}
		}
		return string2;
	}

	template<typename char_type01, typename char_type02> JSONIFIER_INLINE char_type02* parseStringImpl(char_type01* string1, char_type02* string2, uint64_t& lengthNew) {
#if JSONIFIER_CHECK_FOR_AVX(JSONIFIER_AVX512)
		{
			using integer_type						 = typename jsonifier::concepts::get_type_at_index<avx_integer_list, 3>::type::integer_type;
			using simd_type							 = typename jsonifier::concepts::get_type_at_index<avx_integer_list, 3>::type::type;
			static constexpr uint64_t bytesProcessed = jsonifier::concepts::get_type_at_index<avx_integer_list, 3>::type::bytesProcessed;
			static constexpr integer_type mask		 = jsonifier::concepts::get_type_at_index<avx_integer_list, 3>::type::mask;
			simd_type collectionValue;
			while (static_cast<int64_t>(lengthNew) >= bytesProcessed) {
				integer_type nextBackslashOrQuote = copyAndFindParse<simd_type, integer_type>(string1, string2, collectionValue);
				if (nextBackslashOrQuote != mask) {
					auto escapeChar = string1[nextBackslashOrQuote];
					if (escapeChar == 0x22u) {
						string1 += nextBackslashOrQuote;
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
						escapeChar = escapeMap<char_type01>[escapeChar];
						if (escapeChar == 0u) {
							return static_cast<char_type02*>(nullptr);
						}
						string2[nextBackslashOrQuote] = static_cast<char_type02>(escapeChar);
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
		}
#endif
#if JSONIFIER_CHECK_FOR_AVX(JSONIFIER_AVX2)
		{
			using integer_type						 = typename jsonifier::concepts::get_type_at_index<avx_integer_list, 2>::type::integer_type;
			using simd_type							 = typename jsonifier::concepts::get_type_at_index<avx_integer_list, 2>::type::type;
			static constexpr uint64_t bytesProcessed = jsonifier::concepts::get_type_at_index<avx_integer_list, 2>::type::bytesProcessed;
			static constexpr integer_type mask		 = jsonifier::concepts::get_type_at_index<avx_integer_list, 2>::type::mask;
			simd_type collectionValue;
			while (static_cast<int64_t>(lengthNew) >= bytesProcessed) {
				integer_type nextBackslashOrQuote = copyAndFindParse<simd_type, integer_type>(string1, string2, collectionValue);
				if (nextBackslashOrQuote != mask) {
					auto escapeChar = string1[nextBackslashOrQuote];
					if (escapeChar == 0x22u) {
						string1 += nextBackslashOrQuote;
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
						escapeChar = escapeMap<char_type01>[escapeChar];
						if (escapeChar == 0u) {
							return static_cast<char_type02*>(nullptr);
						}
						string2[nextBackslashOrQuote] = static_cast<char_type02>(escapeChar);
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
		}
#endif
#if JSONIFIER_CHECK_FOR_AVX(JSONIFIER_AVX)
		{
			using integer_type						 = typename jsonifier::concepts::get_type_at_index<avx_integer_list, 1>::type::integer_type;
			using simd_type							 = typename jsonifier::concepts::get_type_at_index<avx_integer_list, 1>::type::type;
			static constexpr uint64_t bytesProcessed = jsonifier::concepts::get_type_at_index<avx_integer_list, 1>::type::bytesProcessed;
			static constexpr integer_type mask		 = jsonifier::concepts::get_type_at_index<avx_integer_list, 1>::type::mask;
			simd_type collectionValue;
			while (static_cast<int64_t>(lengthNew) >= bytesProcessed) {
				integer_type nextBackslashOrQuote = copyAndFindParse<simd_type, integer_type>(string1, string2, collectionValue);
				if (nextBackslashOrQuote != mask) {
					auto escapeChar = string1[nextBackslashOrQuote];
					if (escapeChar == 0x22u) {
						string1 += nextBackslashOrQuote;
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
						escapeChar = escapeMap<char_type01>[escapeChar];
						if (escapeChar == 0u) {
							return static_cast<char_type02*>(nullptr);
						}
						string2[nextBackslashOrQuote] = static_cast<char_type02>(escapeChar);
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
		}
#endif
		{
			using integer_type						 = typename jsonifier::concepts::get_type_at_index<avx_integer_list, 0>::type::integer_type;
			using simd_type							 = typename jsonifier::concepts::get_type_at_index<avx_integer_list, 0>::type::type;
			static constexpr uint64_t bytesProcessed = jsonifier::concepts::get_type_at_index<avx_integer_list, 0>::type::bytesProcessed;
			static constexpr integer_type mask		 = jsonifier::concepts::get_type_at_index<avx_integer_list, 0>::type::mask;
			simd_type collectionValue;
			while (static_cast<int64_t>(lengthNew) >= bytesProcessed) {
				integer_type nextBackslashOrQuote = copyAndFindParse<simd_type, integer_type>(string1, string2, collectionValue);
				if (nextBackslashOrQuote != mask) {
					auto escapeChar = string1[nextBackslashOrQuote];
					if (escapeChar == 0x22u) {
						string1 += nextBackslashOrQuote;
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
						escapeChar = escapeMap<char_type01>[escapeChar];
						if (escapeChar == 0u) {
							return static_cast<char_type02*>(nullptr);
						}
						string2[nextBackslashOrQuote] = static_cast<char_type02>(escapeChar);
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
		}
		return parseShortStringImpl(string1, string2, lengthNew);
	}

	template<typename char_type01, typename char_type02>
	JSONIFIER_INLINE void serializeShortStringImpl(char_type01* string1, char_type02* string2, uint64_t lengthNew, uint64_t& indexNew) {
		const auto* const e = string1 + lengthNew;
		for (; string1 < e; ++string1) {
			if (const auto escapeChar = escapeTable[uint8_t(*string1)]; escapeChar) [[likely]] {
				std::memcpy(string2, &escapeChar, 2);
				string2 += 2;
				indexNew += 2;
			} else {
				*string2 = *string1;
				++indexNew;
				++string2;
			}
		}
	}

	template<typename char_type01, typename char_type02>
	JSONIFIER_INLINE void serializeStringImpl(char_type01* string1, char_type02* string2, uint64_t lengthNew, uint64_t& indexNew) {
#if JSONIFIER_CHECK_FOR_AVX(JSONIFIER_AVX512)
		{
			using integer_type						 = typename jsonifier::concepts::get_type_at_index<avx_integer_list, 3>::type::integer_type;
			using simd_type							 = typename jsonifier::concepts::get_type_at_index<avx_integer_list, 3>::type::type;
			static constexpr uint64_t bytesProcessed = jsonifier::concepts::get_type_at_index<avx_integer_list, 3>::type::bytesProcessed;
			static constexpr integer_type mask		 = jsonifier::concepts::get_type_at_index<avx_integer_list, 3>::type::mask;
			simd_type collectionValue;
			while (static_cast<int64_t>(lengthNew) >= bytesProcessed) {
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
		}
#endif
#if JSONIFIER_CHECK_FOR_AVX(JSONIFIER_AVX2)
		{
			using integer_type						 = typename jsonifier::concepts::get_type_at_index<avx_integer_list, 2>::type::integer_type;
			using simd_type							 = typename jsonifier::concepts::get_type_at_index<avx_integer_list, 2>::type::type;
			static constexpr uint64_t bytesProcessed = jsonifier::concepts::get_type_at_index<avx_integer_list, 2>::type::bytesProcessed;
			static constexpr integer_type mask		 = jsonifier::concepts::get_type_at_index<avx_integer_list, 2>::type::mask;
			simd_type collectionValue;
			while (static_cast<int64_t>(lengthNew) >= bytesProcessed) {
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
		}
#endif
#if JSONIFIER_CHECK_FOR_AVX(JSONIFIER_AVX)
		{
			using integer_type						 = typename jsonifier::concepts::get_type_at_index<avx_integer_list, 1>::type::integer_type;
			using simd_type							 = typename jsonifier::concepts::get_type_at_index<avx_integer_list, 1>::type::type;
			static constexpr uint64_t bytesProcessed = jsonifier::concepts::get_type_at_index<avx_integer_list, 1>::type::bytesProcessed;
			static constexpr integer_type mask		 = jsonifier::concepts::get_type_at_index<avx_integer_list, 1>::type::mask;
			simd_type collectionValue;
			while (static_cast<int64_t>(lengthNew) >= bytesProcessed) {
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
		}
#endif
		{
			using integer_type						 = typename jsonifier::concepts::get_type_at_index<avx_integer_list, 0>::type::integer_type;
			using simd_type							 = typename jsonifier::concepts::get_type_at_index<avx_integer_list, 0>::type::type;
			static constexpr uint64_t bytesProcessed = jsonifier::concepts::get_type_at_index<avx_integer_list, 0>::type::bytesProcessed;
			static constexpr integer_type mask		 = jsonifier::concepts::get_type_at_index<avx_integer_list, 0>::type::mask;
			simd_type collectionValue;
			while (static_cast<int64_t>(lengthNew) >= bytesProcessed) {
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
		}
		return serializeShortStringImpl(string1, string2, lengthNew, indexNew);
	}

	template<typename char_type> JSONIFIER_INLINE uint64_t parseBool(bool& value, char_type* json) {
		uint8_t valueNew00[5]{ "true" };
		uint8_t valueNew01[6]{ "false" };
		if (compare<4>(valueNew00, json)) {
			value = true;
			return 4;
		} else if (compare<5>(valueNew01, json)) {
			value = false;
			return 5;
		} else {
			return 0;
		}
	}

	template<typename char_type> JSONIFIER_INLINE uint64_t parseNull(char_type* json) {
		uint8_t valueNew00[5]{ "null" };
		if (compare<4>(valueNew00, json)) {
			return 4;
		} else {
			return 0;
		}
	}
}// namespace jsonifier_internal