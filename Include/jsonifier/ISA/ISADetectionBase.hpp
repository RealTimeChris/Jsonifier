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

using simd_int_512	 = __m512i;
using simd_int_256	 = __m256i;
using simd_int_128	 = __m128i;
using simd_float_512 = __m512d;
using simd_float_256 = __m256d;
using simd_float_128 = __m128d;

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
using simd_int_512 = int64_t;
using simd_int_256 = int32_t;

#endif

#if JSONIFIER_CHECK_FOR_INSTRUCTION(JSONIFIER_AVX512)

constexpr uint64_t BitsPerStep{ 512 };
using string_parsing_type = uint64_t;
using simd_int_t		  = simd_int_512;
template<typename value_type>
concept simd_int_type = std::same_as<simd_int_t, jsonifier::concepts::unwrap<value_type>>;

#elif JSONIFIER_CHECK_FOR_INSTRUCTION(JSONIFIER_AVX2)

constexpr uint64_t BitsPerStep{ 256 };
using string_parsing_type = uint32_t;
using simd_int_t		  = simd_int_256;
template<typename value_type>
concept simd_int_type = std::same_as<simd_int_t, jsonifier::concepts::unwrap<value_type>>;

#else

constexpr uint64_t BitsPerStep{ 128 };
using string_parsing_type = uint16_t;
using simd_int_t		  = simd_int_128;
template<typename value_type>
concept simd_int_type = std::same_as<simd_int_t, jsonifier::concepts::unwrap<value_type>>;

#endif

constexpr uint64_t BytesPerStep{ BitsPerStep / 8 };
constexpr uint64_t SixtyFourBitsPerStep{ BitsPerStep / 64 };
constexpr uint64_t StridesPerStep{ BitsPerStep / BytesPerStep };

namespace jsonifier_internal {

	template<typename return_type, typename value_type, size_t N, size_t... indices>
	constexpr return_type createArray(const value_type (&newArray)[N], std::index_sequence<indices...>) {
		return return_type{ newArray[indices % N]... };
	}

	template<typename return_type> constexpr return_type simdFromValue(uint8_t value) {
#if defined(_WIN32)
		constexpr uint64_t valueSize{ sizeof(char) };
		char newArray[16]{};
		std::fill(newArray, newArray + 16, value);
#else
		constexpr uint64_t valueSize{ sizeof(uint64_t) };
		int64_t newArray[16 / sizeof(int64_t)]{};
		for (size_t x = 0; x < 16; ++x) {
			newArray[x / sizeof(int64_t)] |= static_cast<int64_t>(value) << ((x % 8) * 8);
		}
#endif
		return_type returnValue{ createArray<return_type>(newArray, std::make_index_sequence<sizeof(return_type) / valueSize>{}) };
		return returnValue;
	}

	template<typename return_type> constexpr return_type simdFromTable(const uint8_t (&valuesNew01)[16]) {
#if defined(_WIN32)
		constexpr uint64_t valueSize{ sizeof(char) };
		char newArray[16]{};
		std::copy(valuesNew01, valuesNew01 + std::size(newArray), newArray);
#else
		constexpr uint64_t valueSize{ sizeof(uint64_t) };
		int64_t newArray[16 / sizeof(int64_t)]{};
		for (size_t x = 0; x < 16; ++x) {
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
	using structural_index	= string_view_ptr;
	using string_buffer_ptr = uint8_t*;

	template<typename value_type>
	concept simd_int_128_t = std::same_as<jsonifier::concepts::unwrap<value_type>, simd_int_128>;

	template<typename value_type>
	concept simd_int_256_t = std::same_as<jsonifier::concepts::unwrap<value_type>, simd_int_256>;

	template<typename value_type>
	concept simd_int_512_t = std::same_as<jsonifier::concepts::unwrap<value_type>, simd_int_512>;

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

	static constexpr uint8_t escapeableArray00[]{ 0x00u, 0x00u, 0x22u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x08u, 0x00u, 0x00u, 0x00u, 0x0Cu, 0x0Du, 0x00u, 0x00u };
	static constexpr uint8_t escapeableArray01[]{ 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x09u, 0x0Au, 0x00u, 0x5Cu, 0x00u, 0x00u, 0x00u };
	static constexpr uint8_t whitespaceArray[]{ 0x20u, 0x64u, 0x64u, 0x64u, 0x11u, 0x64u, 0x71u, 0x02u, 0x64u, 0x09u, 0x0Au, 0x70u, 0x64u, 0x0Du, 0x64u, 0x64u };
	static constexpr uint8_t opArray[]{ 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x3Au, 0x7Bu, 0x2Cu, 0x7Du, 0x00u, 0x00u };
	template<typename simd_type> static constexpr simd_type escapeableTable00{ simdFromTable<simd_type>(escapeableArray00) };
	template<typename simd_type> static constexpr simd_type escapeableTable01{ simdFromTable<simd_type>(escapeableArray01) };
	template<typename simd_type> static constexpr simd_type whitespaceTable{ simdFromTable<simd_type>(whitespaceArray) };
	template<typename simd_type> static constexpr simd_type backslashes{ simdFromValue<simd_type>(0x5Cu) };
	template<typename simd_type> static constexpr simd_type opTable{ simdFromTable<simd_type>(opArray) };
	template<typename simd_type> static constexpr simd_type quotes{ simdFromValue<simd_type>(0x22u) };
	template<typename simd_type> static constexpr simd_type chars{ simdFromValue<simd_type>(0x20u) };

	class simd_base {
	  public:
		template<simd_int_type simd_int_t01, simd_int_type simd_int_t02> JSONIFIER_INLINE static string_parsing_type cmpeq(simd_int_t01&& value, simd_int_t02&& other);

		template<simd_int_type simd_int_t01, simd_int_type simd_int_t02> JSONIFIER_INLINE static simd_int_t opShuffle(simd_int_t01&& value, simd_int_t02&& other);

		template<simd_int_type simd_int_t01, simd_int_type simd_int_t02> JSONIFIER_INLINE static simd_int_t opAndNot(simd_int_t01&& value, simd_int_t02&& other);

		template<simd_int_type simd_int_t01, simd_int_type simd_int_t02> JSONIFIER_INLINE static simd_int_t opAnd(simd_int_t01&& value, simd_int_t02&& other);

		template<simd_int_type simd_int_t01, simd_int_type simd_int_t02> JSONIFIER_INLINE static simd_int_t opXor(simd_int_t01&& value, simd_int_t02&& other);

		template<simd_int_type simd_int_t01, simd_int_type simd_int_t02> JSONIFIER_INLINE static simd_int_t opOr(simd_int_t01&& value, simd_int_t02&& other);

		template<simd_int_type simd_int_t01> JSONIFIER_INLINE static simd_int_t setLSB(simd_int_t01&& value, bool valueNew);

		template<simd_int_type simd_int_t01> JSONIFIER_INLINE static simd_int_t opNot(simd_int_t01&& value);

		template<simd_int_type simd_int_t01> JSONIFIER_INLINE static bool getMSB(simd_int_t01&& value);

		template<simd_int_type simd_int_t01> JSONIFIER_INLINE static bool opBool(simd_int_t01&& value);

		JSONIFIER_INLINE static simd_int_t reset();

		template<simd_int_128_t simd_int_t01, simd_int_128_t simd_int_t02>
		JSONIFIER_INLINE static std::enable_if_t<!std::same_as<jsonifier::concepts::unwrap<simd_int_t01>, simd_int_t>, simd_int_128> opShuffle(simd_int_t01&& value,
			simd_int_t02&& other) {
			return _mm_shuffle_epi8(std::forward<simd_int_t01>(value), std::forward<simd_int_t02>(other));
		}

		template<simd_int_128_t simd_int_t01, simd_int_128_t simd_int_t02>
		JSONIFIER_INLINE static std::enable_if_t<!std::same_as<jsonifier::concepts::unwrap<simd_int_t01>, simd_int_t>, uint16_t> cmpeq(simd_int_t01&& value, simd_int_t02&& other) {
			return static_cast<uint16_t>(_mm_movemask_epi8(_mm_cmpeq_epi8(value, other)));
		}

		template<simd_int_256_t simd_int_t01, simd_int_256_t simd_int_t02>
		JSONIFIER_INLINE static std::enable_if_t<!std::same_as<jsonifier::concepts::unwrap<simd_int_t01>, simd_int_t>, simd_int_256> opShuffle(simd_int_t01&& value,
			simd_int_t02&& other) {
			return _mm256_shuffle_epi8(std::forward<simd_int_t01>(value), std::forward<simd_int_t02>(other));
		}

		template<simd_int_256_t simd_int_t01, simd_int_256_t simd_int_t02>
		JSONIFIER_INLINE static std::enable_if_t<!std::same_as<jsonifier::concepts::unwrap<simd_int_t01>, simd_int_t>, uint32_t> cmpeq(simd_int_t01&& value, simd_int_t02&& other) {
			return static_cast<uint32_t>(_mm256_movemask_epi8(_mm256_cmpeq_epi8(value, other)));
		}

		template<size_t... indices> JSONIFIER_INLINE static simd_int_t collectStructuralsAsSimdBase(const simd_int_t (&values)[StridesPerStep], std::index_sequence<indices...>) {
			JSONIFIER_ALIGN string_parsing_type valuesNew[StridesPerStep]{};
			((valuesNew[indices] = cmpeq(opShuffle(opTable<simd_int_t>, values[indices]), opOr(chars<simd_int_t>, values[indices]))), ...);
			return gatherValues<simd_int_t>(valuesNew);
		}

		template<size_t... indices> JSONIFIER_INLINE static simd_int_t collectWhitespaceAsSimdBase(const simd_int_t (&values)[StridesPerStep], std::index_sequence<indices...>) {
			JSONIFIER_ALIGN string_parsing_type valuesNew[StridesPerStep]{};
			((valuesNew[indices] = cmpeq(opShuffle(whitespaceTable<simd_int_t>, values[indices]), values[indices])), ...);
			return gatherValues<simd_int_t>(valuesNew);
		}

		template<size_t... indices> JSONIFIER_INLINE static simd_int_t collectBackslashesAsSimdBase(const simd_int_t (&values)[StridesPerStep], std::index_sequence<indices...>) {
			JSONIFIER_ALIGN string_parsing_type valuesNew[StridesPerStep]{};
			((valuesNew[indices] = cmpeq(backslashes<simd_int_t>, values[indices])), ...);
			return gatherValues<simd_int_t>(valuesNew);
		}

		template<size_t... indices> JSONIFIER_INLINE static simd_int_t collectQuotesAsSimdBase(const simd_int_t (&values)[StridesPerStep], std::index_sequence<indices...>) {
			JSONIFIER_ALIGN string_parsing_type valuesNew[StridesPerStep]{};
			((valuesNew[indices] = cmpeq(quotes<simd_int_t>, values[indices])), ...);
			return gatherValues<simd_int_t>(valuesNew);
		}

		static constexpr size_t power(size_t base, size_t exponent) {
			size_t result = 1;
			while (exponent > 0) {
				if (exponent & 1) {
					result *= base;
				}
				base *= base;
				exponent >>= 1;
			}
			return result;
		}

		template<size_t index> JSONIFIER_INLINE static size_t prefixXorImpl(size_t prevInString) {
			prevInString ^= prevInString << index;
			return prevInString;
		}

		template<size_t... indices> JSONIFIER_INLINE static size_t prefixXor(size_t prevInString, std::index_sequence<indices...>) {
			((prevInString = prefixXorImpl<power(2, indices)>(prevInString)), ...);
			return prevInString;
		}

		template<simd_int_type simd_int_t01, size_t... indices>
		JSONIFIER_INLINE static simd_int_t carrylessMultiplication(simd_int_t01&& value, int64_t& prevInString, std::index_sequence<indices...>) {
			JSONIFIER_ALIGN uint64_t values[SixtyFourBitsPerStep]{};
			store(value, values);
			((values[indices] = prefixXor(values[indices], std::make_index_sequence<6>{}) ^ prevInString, prevInString = static_cast<int64_t>(values[indices]) >> 63), ...);
			return gatherValues<simd_int_t>(values);
		}

		template<simd_int_type simd_int_t01, size_t... indices>
		JSONIFIER_INLINE static simd_int_t opSub(simd_int_t01&& value, simd_int_t01&& other, std::index_sequence<indices...>) {
			JSONIFIER_ALIGN uint64_t values01[SixtyFourBitsPerStep]{};
			JSONIFIER_ALIGN uint64_t values02[SixtyFourBitsPerStep]{};
			store(value, values01);
			store(other, values02);
			bool carryInNew{};
			((values02[indices] = values01[indices] - values02[indices] - static_cast<uint64_t>(carryInNew), carryInNew = values02[indices] > values01[indices]), ...);
			return gatherValues<simd_int_t>(values02);
		}

		template<uint64_t amount, size_t... indices, simd_int_type simd_int_t01> JSONIFIER_INLINE static simd_int_t shl(simd_int_t01&& value, std::index_sequence<indices...>) {
			JSONIFIER_ALIGN uint64_t values01[SixtyFourBitsPerStep]{};
			JSONIFIER_ALIGN uint64_t values02[SixtyFourBitsPerStep]{};
			static constexpr uint64_t shiftAmount{ 64 - amount };
			store(value, values01);
			values02[0] = values01[0] << amount;
			((values02[indices + 1] = values01[indices + 1] << amount | values01[indices] >> (shiftAmount)), ...);
			return gatherValues<simd_int_t>(values02);
		}

		template<simd_int_type simd_int_t01> JSONIFIER_INLINE static simd_int_t follows(simd_int_t01&& value, bool& overflow) {
			bool oldOverflow = overflow;
			overflow		 = getMSB(value);
			return setLSB(shl<1>(value, std::make_index_sequence<SixtyFourBitsPerStep - 1>{}), oldOverflow);
		}
	};

}