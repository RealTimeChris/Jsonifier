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
#pragma once

#if !defined(__GNUC__)
	#pragma warning(disable : 4710)
	#pragma warning(disable : 4711)
	#pragma warning(disable : 4371)
	#pragma warning(disable : 4514)
	#pragma warning(disable : 4625)
	#pragma warning(disable : 4706)
	#pragma warning(disable : 4820)
	#pragma warning(disable : 5045)
	#pragma warning(disable : 5246)
#endif

#if defined(__clang__) && defined(NDEBUG) && !defined(JSONIFIER_INLINE)
	#define JSONIFIER_INLINE inline __attribute__((always_inline))
#elif !defined(JSONIFIER_INLINE)
	#define JSONIFIER_INLINE inline
#endif

#if !defined(JSONIFIER_CPU_INSTRUCTIONS)
	#define JSONIFIER_CPU_INSTRUCTIONS 0
#endif

#if !defined JSONIFIER_ALIGN
	#define JSONIFIER_ALIGN alignas(BytesPerStep)
#endif

#if !defined(JSONIFIER_CHECK_FOR_INSTRUCTION)
	#define JSONIFIER_CHECK_FOR_INSTRUCTION(x) (JSONIFIER_CPU_INSTRUCTIONS & x)
#endif

#if !defined(JSONIFIER_CHECK_FOR_AVX)
	#define JSONIFIER_CHECK_FOR_AVX(x) (JSONIFIER_CPU_INSTRUCTIONS >= x)
#endif

#if !defined(JSONIFIER_POPCNT)
	#define JSONIFIER_POPCNT (1 << 0)
#endif
#if !defined(JSONIFIER_LZCNT)
	#define JSONIFIER_LZCNT (1 << 1)
#endif
#if !defined(JSONIFIER_BMI)
	#define JSONIFIER_BMI (1 << 2)
#endif
#if !defined(JSONIFIER_BMI2)
	#define JSONIFIER_BMI2 (1 << 3)
#endif
#if !defined(JSONIFIER_AVX)
	#define JSONIFIER_AVX (1 << 4)
#endif
#if !defined(JSONIFIER_AVX2)
	#define JSONIFIER_AVX2 (1 << 5)
#endif
#if !defined(JSONIFIER_AVX512)
	#define JSONIFIER_AVX512 (1 << 6)
#endif

#if !defined(JSONIFIER_ANY)
	#define JSONIFIER_ANY (JSONIFIER_AVX | JSONIFIER_AVX2 | JSONIFIER_AVX512 | JSONIFIER_POPCNT | JSONIFIER_BMI | JSONIFIER_BMI2 | JSONIFIER_LZCNT)
#endif

#if !defined(JSONIFIER_ANY_AVX)
	#define JSONIFIER_ANY_AVX (JSONIFIER_AVX | JSONIFIER_AVX2 | JSONIFIER_AVX512)
#endif

#include <jsonifier/TypeEntities.hpp>
#include <iostream>
#include <concepts>
#include <sstream>
#include <cstring>
#include <cstdint>
#include <bitset>
#include <array>

#if JSONIFIER_CHECK_FOR_INSTRUCTION(JSONIFIER_ANY)

	#include <immintrin.h>

#endif
#if JSONIFIER_CHECK_FOR_INSTRUCTION(JSONIFIER_ANY_AVX)

using simd_int_512 = __m512i;
using simd_int_256 = __m256i;
using simd_int_128 = __m128i;

#if JSONIFIER_CHECK_FOR_INSTRUCTION(JSONIFIER_AVX512)

constexpr uint64_t BitsPerStep{ 512 };
using string_parsing_type = uint64_t;
using simd_int_t		  = simd_int_512;
template<typename value_type>
concept simd_int_type = std::is_same_v<simd_int_t, jsonifier::concepts::unwrap_t<value_type>>;

	#elif JSONIFIER_CHECK_FOR_INSTRUCTION(JSONIFIER_AVX2)

constexpr uint64_t BitsPerStep{ 256 };
using string_parsing_type = uint32_t;
using simd_int_t		  = simd_int_256;
template<typename value_type>
concept simd_int_type = std::is_same_v<simd_int_t, jsonifier::concepts::unwrap_t<value_type>>;

	#elif JSONIFIER_CHECK_FOR_INSTRUCTION(JSONIFIER_AVX)

constexpr uint64_t BitsPerStep{ 128 };
using string_parsing_type = uint16_t;
using simd_int_t		  = simd_int_128;
template<typename value_type>
concept simd_int_type = std::is_same_v<simd_int_t, jsonifier::concepts::unwrap_t<value_type>>;

	#endif

#else

union __m128x {
	#if defined(_WIN32)
	int8_t m128x_int8[16]{};
	int16_t m128x_int16[8];
	int32_t m128x_int32[4];
	int64_t m128x_int64[2];
	uint8_t m128x_uint8[16];
	int16_t m128x_uint16[8];
	int32_t m128x_uint32[4];
	uint64_t m128x_uint64[2];
	#else
	int64_t m128x_int64[2];
	int8_t m128x_int8[16]{};
	int16_t m128x_int16[8];
	int32_t m128x_int32[4];
	uint8_t m128x_uint8[16];
	int16_t m128x_uint16[8];
	int32_t m128x_uint32[4];
	uint64_t m128x_uint64[2];
	#endif
};

using simd_int_128 = __m128x;
using simd_int_256 = int32_t;
using simd_int_512 = int64_t;

constexpr uint64_t BitsPerStep{ 128 };
using string_parsing_type = uint16_t;
using simd_int_t		  = simd_int_128;
template<typename value_type>
concept simd_int_type = std::is_same_v<simd_int_t, jsonifier::concepts::unwrap_t<value_type>>;

template<simd_int_type simd_int_t01, size_t... indices> JSONIFIER_INLINE string_parsing_type _mm128_movemask_epi8(simd_int_t01&& a, std::index_sequence<indices...>&&) {
	string_parsing_type mask{ 0 };
	((mask |= (a.m128x_int8[indices] & 0x80) ? (1 << indices) : 0), ...);
	return mask;
}

template<simd_int_type simd_int_t01, simd_int_type simd_int_t02> JSONIFIER_INLINE simd_int_t _mm128_or_si128(simd_int_t01&& valOne, simd_int_t02&& valTwo) {
	simd_int_t value{};
	memcpy(value.m128x_uint64, valOne.m128x_uint64, sizeof(value));
	value.m128x_uint64[0] |= valTwo.m128x_uint64[0];
	value.m128x_uint64[1] |= valTwo.m128x_uint64[1];
	return value;
}

template<simd_int_type simd_int_t01, simd_int_type simd_int_t02> JSONIFIER_INLINE simd_int_t _mm128_and_si128(simd_int_t01&& valOne, simd_int_t02&& valTwo) {
	simd_int_t value{};
	memcpy(value.m128x_uint64, valOne.m128x_uint64, sizeof(value));
	value.m128x_uint64[0] &= valTwo.m128x_uint64[0];
	value.m128x_uint64[1] &= valTwo.m128x_uint64[1];
	return value;
}

template<simd_int_type simd_int_t01, simd_int_type simd_int_t02> JSONIFIER_INLINE simd_int_t _mm128_andnot_si128(simd_int_t01&& valOne, simd_int_t02&& valTwo) {
	simd_int_t value{};
	memcpy(value.m128x_uint64, valTwo.m128x_uint64, sizeof(value));
	value.m128x_uint64[0] &= ~valOne.m128x_uint64[0];
	value.m128x_uint64[1] &= ~valOne.m128x_uint64[1];
	return value;
}

template<simd_int_type simd_int_t01, simd_int_type simd_int_t02> JSONIFIER_INLINE simd_int_t _mm128_xor_si128(simd_int_t01&& valOne, simd_int_t02&& valTwo) {
	simd_int_t value{};
	memcpy(value.m128x_uint64, valOne.m128x_uint64, sizeof(value));
	value.m128x_uint64[0] ^= valTwo.m128x_uint64[0];
	value.m128x_uint64[1] ^= valTwo.m128x_uint64[1];
	return value;
}

template<simd_int_type simd_int_t01, simd_int_type simd_int_t02, size_t... indices>
JSONIFIER_INLINE simd_int_t _mm128_cmpeq_epi8(simd_int_t01&& a, simd_int_t02&& b, std::index_sequence<indices...>&&) {
	simd_int_t result{};
	((result.m128x_int8[indices] = (a.m128x_int8[indices] == b.m128x_int8[indices]) ? 0xFF : 0), ...);
	return result;
}

JSONIFIER_INLINE bool _mm128_testz_si128(simd_int_t& valOne, simd_int_t& valTwo) {
	valOne.m128x_uint64[0] &= valTwo.m128x_uint64[0];
	valOne.m128x_uint64[1] &= valTwo.m128x_uint64[1];
	return valOne.m128x_uint64[0] == 0 && valOne.m128x_uint64[1] == 0;
}

JSONIFIER_INLINE simd_int_t _mm128_set_epi64x(uint64_t argOne, uint64_t argTwo) {
	simd_int_t returnValue{};
	std::memcpy(&returnValue.m128x_uint64[0], &argTwo, sizeof(uint64_t));
	std::memcpy(&returnValue.m128x_uint64[1], &argOne, sizeof(uint64_t));
	return returnValue;
}

JSONIFIER_INLINE simd_int_t _mm128_set1_epi64x(uint64_t argOne) {
	simd_int_t returnValue{};
	std::memcpy(&returnValue.m128x_uint64[0], &argOne, sizeof(uint64_t));
	std::memcpy(&returnValue.m128x_uint64[1], &argOne, sizeof(uint64_t));
	return returnValue;
}

template<simd_int_type simd_int_t01, simd_int_type simd_int_t02, size_t... indices>
JSONIFIER_INLINE simd_int_t _mm128_shuffle_epi8(simd_int_t01&& a, simd_int_t02&& b, std::index_sequence<indices...>) {
	simd_int_t result{};
	size_t index{};
	(((index = b.m128x_uint8[indices] & 0x0F), (result.m128x_uint8[indices] = a.m128x_uint8[index])), ...);
	return result;
}

#endif

constexpr uint64_t BytesPerStep{ BitsPerStep / 8 };
constexpr uint64_t SixtyFourBitsPerStep{ BitsPerStep / 64 };
constexpr uint64_t StridesPerStep{ BitsPerStep / BytesPerStep };

using avx_list = jsonifier::concepts::type_list<jsonifier::concepts::type_holder<16, simd_int_128, uint16_t, std::numeric_limits<uint16_t>::max()>,
	jsonifier::concepts::type_holder<32, simd_int_256, uint32_t, std::numeric_limits<uint32_t>::max()>,
	jsonifier::concepts::type_holder<64, simd_int_512, uint64_t, std::numeric_limits<uint64_t>::max()>>;

using avx_integer_list = jsonifier::concepts::type_list<jsonifier::concepts::type_holder<8, uint64_t, uint64_t, 8>, jsonifier::concepts::type_holder<16, simd_int_128, uint16_t, 16>,
	jsonifier::concepts::type_holder<32, simd_int_256, uint32_t, 32>, jsonifier::concepts::type_holder<64, simd_int_512, uint64_t, 64>>;

namespace jsonifier_internal {

	template<typename return_type, typename value_type, uint64_t N, size_t... indices>
	constexpr return_type createArray(const value_type (&newArray)[N], std::index_sequence<indices...>) {
		return return_type{ newArray[indices % N]... };
	}

	template<typename return_type> constexpr return_type simdFromValue(uint8_t value) {
#if defined(_WIN32)
		constexpr uint64_t valueSize{ sizeof(char) };
		char newArray[16]{};
		std::fill(newArray, newArray + 16, static_cast<char>(value));
#else
		constexpr uint64_t valueSize{ sizeof(uint64_t) };
		int64_t newArray[16 / sizeof(int64_t)]{};
		for (uint64_t x = 0; x < 16; ++x) {
			newArray[x / sizeof(int64_t)] |= static_cast<int64_t>(value) << ((x % 8) * 8);
		}
#endif
		return_type returnValue{ createArray<return_type>(newArray, std::make_index_sequence<sizeof(return_type) / valueSize>{}) };
		return returnValue;
	}

	template<typename return_type> constexpr return_type simdFromTable(const std::array<uint8_t,16> valuesNew01) {
#if defined(_WIN32)
		constexpr uint64_t valueSize{ sizeof(char) };
		char newArray[16]{};
		std::copy(valuesNew01.data(), valuesNew01.data() + std::size(newArray), newArray);
#else
		constexpr uint64_t valueSize{ sizeof(uint64_t) };
		int64_t newArray[16 / sizeof(int64_t)]{};
		for (uint64_t x = 0; x < 16; ++x) {
			newArray[x / sizeof(int64_t)] |= static_cast<int64_t>(valuesNew01[x % 16]) << ((x % 8) * 8);
		}
#endif
		return_type returnValue{ createArray<return_type>(newArray, std::make_index_sequence<sizeof(return_type) / valueSize>{}) };
		return returnValue;
	}

	template<typename simd_type> JSONIFIER_INLINE const simd_type& printBits(const simd_type& value, const std::string& valuesTitle) noexcept;

	template<typename simd_type> JSONIFIER_INLINE std::string printBits(const simd_type& value) noexcept;

	JSONIFIER_INLINE void printBits(uint64_t values, const std::string& valuesTitle);

	using string_view_ptr	= const uint8_t*;
	using structural_index	= uint32_t;
	using string_buffer_ptr = uint8_t*;

	template<typename value_type>
	concept simd_int_128_t = std::is_same_v<jsonifier::concepts::unwrap_t<value_type>, simd_int_128>;

	template<typename value_type>
	concept simd_int_256_t = std::is_same_v<jsonifier::concepts::unwrap_t<value_type>, simd_int_256>;

	template<typename value_type>
	concept simd_int_512_t = std::is_same_v<jsonifier::concepts::unwrap_t<value_type>, simd_int_512>;

	template<jsonifier::concepts::unsigned_t simd_int_type_new, typename char_type> JSONIFIER_INLINE simd_int_type_new gatherValuesU(char_type* str) {
		simd_int_type_new returnValue{};
		std::memcpy(&returnValue, str, sizeof(simd_int_type_new));
		return returnValue;
	}

#if JSONIFIER_CHECK_FOR_AVX(JSONIFIER_AVX)

	template<simd_int_128_t simd_int_type_new, typename char_type> JSONIFIER_INLINE simd_int_type_new gatherValues(char_type* str) {
		return _mm_load_si128(reinterpret_cast<const __m128i*>(str));
	}

	template<simd_int_128_t simd_int_type_new, typename char_type> JSONIFIER_INLINE simd_int_type_new gatherValuesU(char_type* str) {
		return _mm_loadu_si128(reinterpret_cast<const __m128i*>(str));
	}

	template<simd_int_128_t simd_int_type_new, typename char_type> JSONIFIER_INLINE simd_int_type_new gatherValue(char_type str) {
		return _mm_set1_epi8(str);
	}

	template<simd_int_128_t simd_int_type_new, typename char_type> JSONIFIER_INLINE void store(const simd_int_type_new& value, char_type* storageLocation) {
		_mm_store_si128(reinterpret_cast<__m128i*>(storageLocation), value);
	}

	#if JSONIFIER_CHECK_FOR_AVX(JSONIFIER_AVX2)

	template<simd_int_256_t simd_int_type_new, typename char_type> JSONIFIER_INLINE simd_int_type_new gatherValues(char_type* str) {
		return _mm256_load_si256(reinterpret_cast<const __m256i*>(str));
	}

	template<simd_int_256_t simd_int_type_new, typename char_type> JSONIFIER_INLINE simd_int_type_new gatherValuesU(char_type* str) {
		return _mm256_loadu_si256(reinterpret_cast<const __m256i*>(str));
	}

	template<simd_int_256_t simd_int_type_new, typename char_type> JSONIFIER_INLINE simd_int_type_new gatherValue(char_type str) {
		return _mm256_set1_epi8(str);
	}

	template<simd_int_256_t simd_int_type_new, typename char_type> JSONIFIER_INLINE void store(const simd_int_type_new& value, char_type* storageLocation) {
		_mm256_store_si256(reinterpret_cast<__m256i*>(storageLocation), value);
	}

		#if JSONIFIER_CHECK_FOR_AVX(JSONIFIER_AVX512)

	template<simd_int_512_t simd_int_type_new, typename char_type> JSONIFIER_INLINE simd_int_type_new gatherValues(char_type* str) {
		return _mm512_load_si512(str);
	}

	template<simd_int_512_t simd_int_type_new, typename char_type> JSONIFIER_INLINE simd_int_type_new gatherValuesU(char_type* str) {
		return _mm512_loadu_si512(reinterpret_cast<const __m512i*>(str));
	}

	template<simd_int_512_t simd_int_type_new, typename char_type> JSONIFIER_INLINE simd_int_type_new gatherValue(char_type str) {
		return _mm512_set1_epi8(str);
	}

	template<simd_int_512_t simd_int_type_new, typename char_type> JSONIFIER_INLINE void store(const simd_int_type_new& value, char_type* storageLocation) {
		_mm512_store_si512(storageLocation, value);
	}

		#endif

	#endif

#else

	template<simd_int_type simd_int_type_new, typename char_type> JSONIFIER_INLINE simd_int_type_new gatherValues(char_type* str) {
		simd_int_t returnValue{};
		std::memcpy(&returnValue, str, sizeof(simd_int_t));
		return returnValue;
	}

	template<simd_int_type simd_int_type_new, typename char_type> JSONIFIER_INLINE simd_int_type_new gatherValuesU(char_type* str) {
		simd_int_t returnValue{};
		std::memcpy(&returnValue, str, sizeof(simd_int_t));
		return returnValue;
	}

	template<simd_int_type simd_int_type_new, typename char_type> JSONIFIER_INLINE simd_int_type_new gatherValue(char_type str) {
		simd_int_t returnValue{};
		std::memset(&returnValue, str, sizeof(simd_int_t));
		return returnValue;
	}

	template<typename char_type> JSONIFIER_INLINE void store(const simd_int_t& value, char_type* storageLocation) {
		std::memcpy(storageLocation, &value, sizeof(simd_int_t));
	}

#endif

	constexpr std::array<uint8_t, 16> escapeableArray00{ 0x00u, 0x00u, 0x22u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x08u, 0x00u, 0x00u, 0x00u, 0x0Cu, 0x0Du, 0x00u, 0x00u };
	constexpr std::array<uint8_t, 16> escapeableArray01{ 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x09u, 0x0Au, 0x00u, 0x5Cu, 0x00u, 0x00u, 0x00u };
	constexpr std::array<uint8_t, 16> whitespaceArray{ 0x20u, 0x64u, 0x64u, 0x64u, 0x11u, 0x64u, 0x71u, 0x02u, 0x64u, 0x09u, 0x0Au, 0x70u, 0x64u, 0x0Du, 0x64u, 0x64u };
	constexpr std::array<uint8_t, 16> opArray{ 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x3Au, 0x7Bu, 0x2Cu, 0x7Du, 0x00u, 0x00u };
	template<typename simd_type> constexpr simd_type escapeableTable00{ simdFromTable<simd_type>(escapeableArray00) };
	template<typename simd_type> constexpr simd_type escapeableTable01{ simdFromTable<simd_type>(escapeableArray01) };
	template<typename simd_type> constexpr simd_type whitespaceTable{ simdFromTable<simd_type>(whitespaceArray) };
	template<typename simd_type> constexpr simd_type backslashes{ simdFromValue<simd_type>(0x5Cu) };
	template<typename simd_type> constexpr simd_type opTable{ simdFromTable<simd_type>(opArray) };
	template<typename simd_type> constexpr simd_type quotes{ simdFromValue<simd_type>(0x22u) };
	template<typename simd_type> constexpr simd_type chars{ simdFromValue<simd_type>(0x20u) };

	class simd_base {
	  public:

		template<simd_int_type simd_int_t01, simd_int_type simd_int_t02> JSONIFIER_INLINE static simd_int_t opAndNot(simd_int_t01&& value, simd_int_t02&& other);

		template<simd_int_type simd_int_t01, simd_int_type simd_int_t02> JSONIFIER_INLINE static simd_int_t opAnd(simd_int_t01&& value, simd_int_t02&& other);

		template<simd_int_type simd_int_t01, simd_int_type simd_int_t02> JSONIFIER_INLINE static simd_int_t opXor(simd_int_t01&& value, simd_int_t02&& other);

		template<simd_int_type simd_int_t01, simd_int_type simd_int_t02> JSONIFIER_INLINE static simd_int_t opOr(simd_int_t01&& value, simd_int_t02&& other);

		template<simd_int_type simd_int_t01> JSONIFIER_INLINE static simd_int_t opSetLSB(simd_int_t01&& value, bool valueNew);

		template<simd_int_type simd_int_t01> JSONIFIER_INLINE static simd_int_t opNot(simd_int_t01&& value);

		template<simd_int_type simd_int_t01> JSONIFIER_INLINE static bool opGetMSB(simd_int_t01&& value);

		template<simd_int_type simd_int_t01> JSONIFIER_INLINE static bool opBool(simd_int_t01&& value);

		JSONIFIER_INLINE static simd_int_t reset();

		template<simd_int_128_t simd_int_t01, simd_int_128_t simd_int_t02> JSONIFIER_INLINE static uint16_t opCmpEq(simd_int_t01&& value, simd_int_t02&& other) {
#if JSONIFIER_CHECK_FOR_INSTRUCTION(JSONIFIER_ANY_AVX)
			return static_cast<uint16_t>(_mm_movemask_epi8(_mm_cmpeq_epi8(std::forward<simd_int_t01>(value), std::forward<simd_int_t02>(other))));
#else
			return static_cast<uint16_t>(_mm128_movemask_epi8(
				_mm128_cmpeq_epi8(std::forward<simd_int_t01>(value), std::forward<simd_int_t02>(other), std::make_index_sequence<16>{}), std::make_index_sequence<16>{}));
#endif
		}

		template<simd_int_256_t simd_int_t01, simd_int_256_t simd_int_t02> JSONIFIER_INLINE static uint32_t opCmpEq(simd_int_t01&& value, simd_int_t02&& other) {
			return static_cast<uint32_t>(_mm256_movemask_epi8(_mm256_cmpeq_epi8(std::forward<simd_int_t01>(value), std::forward<simd_int_t02>(other))));
		}

		template<simd_int_512_t simd_int_t01, simd_int_512_t simd_int_t02> JSONIFIER_INLINE static uint64_t opCmpEq(simd_int_t01&& value, simd_int_t02&& other) {
			return static_cast<uint64_t>(_mm512_movepi8_mask(std::forward<simd_int_t01>(value), std::forward<simd_int_t02>(other)));
		}

		template<simd_int_128_t simd_int_t01, simd_int_128_t simd_int_t02> JSONIFIER_INLINE static simd_int_128 opShuffle(simd_int_t01&& value, simd_int_t02&& other) {
#if JSONIFIER_CHECK_FOR_INSTRUCTION(JSONIFIER_ANY_AVX)
			return _mm_shuffle_epi8(std::forward<simd_int_t01>(value), std::forward<simd_int_t02>(other));
#else
			return _mm128_shuffle_epi8(std::forward<simd_int_t01>(value), std::forward<simd_int_t02>(other), std::make_index_sequence<16>{});
#endif
		}

		template<simd_int_256_t simd_int_t01, simd_int_256_t simd_int_t02> JSONIFIER_INLINE static simd_int_256 opShuffle(simd_int_t01&& value, simd_int_t02&& other) {
			return _mm256_shuffle_epi8(std::forward<simd_int_t01>(value), std::forward<simd_int_t02>(other));
		}

		template<simd_int_512_t simd_int_t01, simd_int_512_t simd_int_t02> JSONIFIER_INLINE static simd_int_512 opShuffle(simd_int_t01&& value, simd_int_t02&& other) {
			return _mm512_shuffle_epi8(std::forward<simd_int_t01>(value), std::forward<simd_int_t02>(other));
		}

		JSONIFIER_INLINE static simd_int_t collectStructuralsAsSimdBase(const simd_int_t (&values)[StridesPerStep]) {
			JSONIFIER_ALIGN string_parsing_type valuesNew[StridesPerStep]{};
			valuesNew[0] = opCmpEq(opShuffle(opTable<simd_int_t>, values[0]), opOr(chars<simd_int_t>, values[0]));
			valuesNew[1] = opCmpEq(opShuffle(opTable<simd_int_t>, values[1]), opOr(chars<simd_int_t>, values[1]));
			valuesNew[2] = opCmpEq(opShuffle(opTable<simd_int_t>, values[2]), opOr(chars<simd_int_t>, values[2]));
			valuesNew[3] = opCmpEq(opShuffle(opTable<simd_int_t>, values[3]), opOr(chars<simd_int_t>, values[3]));
			valuesNew[4] = opCmpEq(opShuffle(opTable<simd_int_t>, values[4]), opOr(chars<simd_int_t>, values[4]));
			valuesNew[5] = opCmpEq(opShuffle(opTable<simd_int_t>, values[5]), opOr(chars<simd_int_t>, values[5]));
			valuesNew[6] = opCmpEq(opShuffle(opTable<simd_int_t>, values[6]), opOr(chars<simd_int_t>, values[6]));
			valuesNew[7] = opCmpEq(opShuffle(opTable<simd_int_t>, values[7]), opOr(chars<simd_int_t>, values[7]));
			return gatherValues<simd_int_t>(valuesNew);
		}

		JSONIFIER_INLINE static simd_int_t collectWhitespaceAsSimdBase(const simd_int_t (&values)[StridesPerStep]) {
			JSONIFIER_ALIGN string_parsing_type valuesNew[StridesPerStep]{};
			valuesNew[0] = opCmpEq(opShuffle(whitespaceTable<simd_int_t>, values[0]), values[0]);
			valuesNew[1] = opCmpEq(opShuffle(whitespaceTable<simd_int_t>, values[1]), values[1]);
			valuesNew[2] = opCmpEq(opShuffle(whitespaceTable<simd_int_t>, values[2]), values[2]);
			valuesNew[3] = opCmpEq(opShuffle(whitespaceTable<simd_int_t>, values[3]), values[3]);
			valuesNew[4] = opCmpEq(opShuffle(whitespaceTable<simd_int_t>, values[4]), values[4]);
			valuesNew[5] = opCmpEq(opShuffle(whitespaceTable<simd_int_t>, values[5]), values[5]);
			valuesNew[6] = opCmpEq(opShuffle(whitespaceTable<simd_int_t>, values[6]), values[6]);
			valuesNew[7] = opCmpEq(opShuffle(whitespaceTable<simd_int_t>, values[7]), values[7]);
			return gatherValues<simd_int_t>(valuesNew);
		}

		JSONIFIER_INLINE static simd_int_t collectBackslashesAsSimdBase(const simd_int_t (&values)[StridesPerStep]) {
			JSONIFIER_ALIGN string_parsing_type valuesNew[StridesPerStep]{};
			valuesNew[0] = opCmpEq(backslashes<simd_int_t>, values[0]);
			valuesNew[1] = opCmpEq(backslashes<simd_int_t>, values[1]);
			valuesNew[2] = opCmpEq(backslashes<simd_int_t>, values[2]);
			valuesNew[3] = opCmpEq(backslashes<simd_int_t>, values[3]);
			valuesNew[4] = opCmpEq(backslashes<simd_int_t>, values[4]);
			valuesNew[5] = opCmpEq(backslashes<simd_int_t>, values[5]);
			valuesNew[6] = opCmpEq(backslashes<simd_int_t>, values[6]);
			valuesNew[7] = opCmpEq(backslashes<simd_int_t>, values[7]);
			return gatherValues<simd_int_t>(valuesNew);
		}

		JSONIFIER_INLINE static simd_int_t collectQuotesAsSimdBase(const simd_int_t (&values)[StridesPerStep]) {
			JSONIFIER_ALIGN string_parsing_type valuesNew[StridesPerStep]{};
			valuesNew[0] = opCmpEq(quotes<simd_int_t>, values[0]);
			valuesNew[1] = opCmpEq(quotes<simd_int_t>, values[1]);
			valuesNew[2] = opCmpEq(quotes<simd_int_t>, values[2]);
			valuesNew[3] = opCmpEq(quotes<simd_int_t>, values[3]);
			valuesNew[4] = opCmpEq(quotes<simd_int_t>, values[4]);
			valuesNew[5] = opCmpEq(quotes<simd_int_t>, values[5]);
			valuesNew[6] = opCmpEq(quotes<simd_int_t>, values[6]);
			valuesNew[7] = opCmpEq(quotes<simd_int_t>, values[7]);
			return gatherValues<simd_int_t>(valuesNew);
		}

		JSONIFIER_INLINE static uint64_t prefixXor(uint64_t prevInString) {
			prevInString ^= prevInString << 1;
			prevInString ^= prevInString << 2;
			prevInString ^= prevInString << 4;
			prevInString ^= prevInString << 8;
			prevInString ^= prevInString << 16;
			prevInString ^= prevInString << 32;
			return prevInString;
		}

		template<uint64_t index = 0> JSONIFIER_INLINE static simd_int_t opClMulImpl(uint64_t* values, int64_t& prevInString) {
			if constexpr (index < SixtyFourBitsPerStep) {
				values[index] = prefixXor(values[index]) ^ prevInString, prevInString = static_cast<int64_t>(values[index]) >> 63;
				return opClMulImpl<index + 1>(values, prevInString);

			} else {
				return gatherValues<simd_int_t>(values);
			}
		}

		template<simd_int_type simd_int_t01> JSONIFIER_INLINE static simd_int_t opClMul(simd_int_t01&& value, int64_t& prevInString) {
			JSONIFIER_ALIGN uint64_t values[SixtyFourBitsPerStep]{};
			store(value, values);
			return opClMulImpl(values, prevInString);
		}

		template<uint64_t index = 0> JSONIFIER_INLINE static simd_int_t opSubImpl(uint64_t* values, bool& carryInNew) {
			if constexpr (index < SixtyFourBitsPerStep) {
				values[index + SixtyFourBitsPerStep] = values[index] - values[index + SixtyFourBitsPerStep] - static_cast<uint64_t>(carryInNew);
				carryInNew							 = values[index + SixtyFourBitsPerStep] > values[index];
				return opSubImpl<index + 1>(values, carryInNew);
			} else {
				return gatherValues<simd_int_t>(values + SixtyFourBitsPerStep);
			}

		}

		template<simd_int_type simd_int_t01> JSONIFIER_INLINE static simd_int_t opSub(simd_int_t01&& value, simd_int_t01&& other) {
			JSONIFIER_ALIGN uint64_t values[SixtyFourBitsPerStep * 2]{};
			store(value, values);
			store(other, values + SixtyFourBitsPerStep);
			bool carryInNew{};
			return opSubImpl(values, carryInNew);
		}

		template<uint64_t index = 1, uint64_t amount> JSONIFIER_INLINE static simd_int_t opShlImpl(uint64_t* values) {
			if constexpr (index < SixtyFourBitsPerStep) {
				static constexpr uint64_t shiftAmount{ 64 - amount };
				values[index + SixtyFourBitsPerStep] = values[index] << amount | values[index - 1] >> (shiftAmount);
				return opShlImpl<index + 1, amount>(values);
			} else {
				return gatherValues<simd_int_t>(values + SixtyFourBitsPerStep);
			}
		}

		template<uint64_t amount, simd_int_type simd_int_t01> JSONIFIER_INLINE static simd_int_t opShl(simd_int_t01&& value) {
			JSONIFIER_ALIGN uint64_t values[SixtyFourBitsPerStep * 2]{};
			static constexpr uint64_t shiftAmount{ 64 - amount };
			store(value, values);
			values[SixtyFourBitsPerStep] = values[0] << amount;
			return opShlImpl<1, amount>(values);
		}

		template<simd_int_type simd_int_t01> JSONIFIER_INLINE static simd_int_t opFollows(simd_int_t01&& value, bool& overflow) {
			bool oldOverflow = overflow;
			overflow		 = opGetMSB(value);
			return opSetLSB(opShl<1>(value), oldOverflow);
		}

	  protected:
		simd_int_t value{};
	};

}