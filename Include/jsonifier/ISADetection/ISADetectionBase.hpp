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
	#pragma warning(disable : 4251)
	#pragma warning(disable : 4371)
	#pragma warning(disable : 4514)
	#pragma warning(disable : 4623)
	#pragma warning(disable : 4625)
	#pragma warning(disable : 4626)
	#pragma warning(disable : 4820)
	#pragma warning(disable : 5267)
	#pragma warning(disable : 5026)
	#pragma warning(disable : 5027)
	#pragma warning(disable : 5045)
	#pragma warning(disable : 5246)
#endif

#if !defined(JSONIFIER_CPU_INSTRUCTIONS)
	#define JSONIFIER_CPU_INSTRUCTIONS 0
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
#include <jsonifier/CTimeArray.hpp>
#include <iostream>
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

typedef union __m128x {
	int8_t m128x_int8[16];
	int16_t m128x_int16[8];
	int32_t m128x_int32[4];
	int64_t m128x_int64[2];
	uint8_t m128x_uint8[16];
	int16_t m128x_uint16[8];
	int32_t m128x_uint32[4];
	uint64_t m128x_uint64[2];
} __m128x;

using simd_int_128 = __m128x;

#endif

#if JSONIFIER_CHECK_FOR_INSTRUCTION(JSONIFIER_AVX512)

constexpr uint64_t BitsPerStep{ 512 };
constexpr uint64_t BytesPerStep{ BitsPerStep / 8 };
constexpr uint64_t SixtyFourBitsPerStep{ BitsPerStep / 64 };
constexpr uint64_t StridesPerStep{ BitsPerStep / BytesPerStep };
using string_parsing_type = uint64_t;
using simd_int_t		  = simd_int_512;

#elif JSONIFIER_CHECK_FOR_INSTRUCTION(JSONIFIER_AVX2)

constexpr uint64_t BitsPerStep{ 256 };
constexpr uint64_t BytesPerStep{ BitsPerStep / 8 };
constexpr uint64_t SixtyFourBitsPerStep{ BitsPerStep / 64 };
constexpr uint64_t StridesPerStep{ BitsPerStep / BytesPerStep };
using string_parsing_type = uint32_t;
using simd_int_t		  = simd_int_256;

#elif JSONIFIER_CHECK_FOR_INSTRUCTION(JSONIFIER_AVX)

constexpr uint64_t BitsPerStep{ 128 };
constexpr uint64_t BytesPerStep{ BitsPerStep / 8 };
constexpr uint64_t SixtyFourBitsPerStep{ BitsPerStep / 64 };
constexpr uint64_t StridesPerStep{ BitsPerStep / BytesPerStep };
using string_parsing_type = uint16_t;
using simd_int_t		  = simd_int_128;

#else

constexpr uint64_t BitsPerStep{ 128 };
constexpr uint64_t BytesPerStep{ BitsPerStep / 8 };
constexpr uint64_t SixtyFourBitsPerStep{ BitsPerStep / 64 };
constexpr uint64_t StridesPerStep{ BitsPerStep / BytesPerStep };
using string_parsing_type = uint16_t;
using simd_int_t		  = simd_int_128;

#endif

namespace jsonifier_internal {

	template<typename return_type, typename char_type, typename value_type, size_t N, size_t... Indices>
	constexpr auto createArray(ctime_array<value_type, N> newArray, std::index_sequence<Indices...>) {
		return return_type{ static_cast<char_type>(newArray[Indices % newArray.size()])... };
	}

	template<typename return_type, typename char_type, typename value_type, size_t M> constexpr auto createArray(ctime_array<value_type, M> newArray) {
		return createArray<return_type, char_type>(newArray, std::make_index_sequence<sizeof(return_type) / sizeof(value_type)>());
	}

	inline void printBits(uint64_t values, const std::string& valuesTitle);

	template<typename simd_type> inline std::string printBits(const simd_type& value) noexcept;

	template<typename simd_type> inline const simd_type& printBits(const simd_type& value, const std::string& valuesTitle) noexcept;

	using string_view_ptr	= const uint8_t*;
	using structural_index	= string_view_ptr;
	using string_buffer_ptr = uint8_t*;

#if JSONIFIER_CHECK_FOR_AVX(JSONIFIER_AVX)

	template<typename value_type>
	concept simd_int_128_t = std::same_as<jsonifier::concepts::unwrap<value_type>, simd_int_128>;

	template<typename value_type>
	concept simd_float_128_t = std::same_as<jsonifier::concepts::unwrap<value_type>, simd_float_128>;

	template<simd_int_128_t simd_type, typename char_type> inline simd_type gatherValues(char_type* str) {
		return _mm_load_si128(reinterpret_cast<const __m128i*>(str));
	}

	template<simd_int_128_t simd_type, typename char_type> inline simd_type gatherValuesU(char_type* str) {
		return _mm_loadu_si128(reinterpret_cast<const __m128i*>(str));
	}

	template<simd_float_128_t simd_type, jsonifier::concepts::float_t char_type> inline simd_type gatherValues(char_type* str) {
		return _mm_loadu_pd(str);
	}

	template<typename value_type> inline void storeu(const simd_int_128& value, value_type* storageLocation) {
		_mm_storeu_si128(reinterpret_cast<__m128i*>(storageLocation), value);
	}

	template<typename value_type> inline void store(const simd_int_128& value, value_type* storageLocation) {
		_mm_store_si128(reinterpret_cast<__m128i*>(storageLocation), value);
	}

	template<typename return_type> constexpr return_type simdFromValue(uint8_t value) {
	#if !defined(_WIN32)
		ctime_array<int64_t, sizeof(return_type) / sizeof(uint64_t)> newArray{};
		for (uint64_t x = 0; x < sizeof(return_type) / sizeof(uint64_t); ++x) {
			newArray[x] |= static_cast<int64_t>(value) << 56;
			newArray[x] |= static_cast<int64_t>(value) << 48;
			newArray[x] |= static_cast<int64_t>(value) << 40;
			newArray[x] |= static_cast<int64_t>(value) << 32;
			newArray[x] |= static_cast<int64_t>(value) << 24;
			newArray[x] |= static_cast<int64_t>(value) << 16;
			newArray[x] |= static_cast<int64_t>(value) << 8;
			newArray[x] |= static_cast<int64_t>(value);
		}
		return_type returnValue{ createArray<return_type, int64_t>(newArray) };
	#else
		ctime_array<uint8_t, 16> newArray{};
		for (uint64_t x = 0; x < 16; ++x) {
			newArray[x] = value;
		}
		return_type returnValue{ createArray<return_type, char>(newArray) };
	#endif
		return returnValue;
	}

	template<typename return_type> constexpr return_type simdFromTable(ctime_array<uint8_t, 16> arrayNew01) {
	#if !defined(_WIN32)
		ctime_array<int64_t, sizeof(return_type) / sizeof(uint64_t)> newArray{};
		for (uint64_t x = 0; x < sizeof(return_type) / sizeof(uint64_t); ++x) {
			newArray[x] |= static_cast<int64_t>(arrayNew01[((x * 8) + 7) % 16]) << 56;
			newArray[x] |= static_cast<int64_t>(arrayNew01[((x * 8) + 6) % 16]) << 48;
			newArray[x] |= static_cast<int64_t>(arrayNew01[((x * 8) + 5) % 16]) << 40;
			newArray[x] |= static_cast<int64_t>(arrayNew01[((x * 8) + 4) % 16]) << 32;
			newArray[x] |= static_cast<int64_t>(arrayNew01[((x * 8) + 3) % 16]) << 24;
			newArray[x] |= static_cast<int64_t>(arrayNew01[((x * 8) + 2) % 16]) << 16;
			newArray[x] |= static_cast<int64_t>(arrayNew01[((x * 8) + 1) % 16]) << 8;
			newArray[x] |= static_cast<int64_t>(arrayNew01[((x * 8) + 0) % 16]);
		}
		return_type returnValue{ createArray<return_type, int64_t>(newArray) };
	#else
		return_type returnValue{ createArray<return_type, char>(arrayNew01) };
	#endif
		return returnValue;
	}

	#if JSONIFIER_CHECK_FOR_AVX(JSONIFIER_AVX2)

	template<typename value_type>
	concept simd_int_256_t = std::same_as<jsonifier::concepts::unwrap<value_type>, simd_int_256>;

	template<typename value_type>
	concept simd_float_256_t = std::same_as<jsonifier::concepts::unwrap<value_type>, simd_float_256>;

	template<simd_int_256_t simd_type, typename char_type> inline simd_type gatherValues(char_type* str) {
		return _mm256_load_si256(reinterpret_cast<const __m256i*>(str));
	}

	template<simd_int_256_t simd_type, typename char_type> inline simd_type gatherValuesU(char_type* str) {
		return _mm256_loadu_si256(reinterpret_cast<const __m256i*>(str));
	}

	template<simd_float_256_t simd_type, jsonifier::concepts::float_t char_type> inline simd_type gatherValues(char_type* str) {
		return _mm256_loadu_pd(str);
	}

	template<typename value_type> inline void storeu(const simd_int_256& value, value_type* storageLocation) {
		_mm256_storeu_si256(reinterpret_cast<__m256i*>(storageLocation), value);
	}

	template<typename value_type> inline void store(const simd_int_256& value, value_type* storageLocation) {
		_mm256_store_si256(reinterpret_cast<__m256i*>(storageLocation), value);
	}

		#if JSONIFIER_CHECK_FOR_AVX(JSONIFIER_AVX512)

	template<typename value_type>
	concept simd_int_512_t = std::same_as<jsonifier::concepts::unwrap<value_type>, simd_int_512>;

	template<typename value_type>
	concept simd_float_512_t = std::same_as<jsonifier::concepts::unwrap<value_type>, simd_float_512>;

	template<simd_int_512_t simd_type, typename char_type> inline simd_type gatherValues(char_type* str) {
		return _mm512_load_si512(str);
	}

	template<simd_int_512_t simd_type, typename char_type> inline simd_type gatherValuesU(char_type* str) {
		return _mm512_loadu_si512(str);
	}

	template<simd_float_512_t simd_type, jsonifier::concepts::float_t char_type> inline simd_type gatherValues(char_type* str) {
		return _mm512_loadu_pd(str);
	}

	template<typename value_type> inline void storeu(const simd_int_512& value, value_type* storageLocation) {
		_mm512_storeu_si512(storageLocation, value);
	}

	template<typename value_type> inline void store(const simd_int_512& value, value_type* storageLocation) {
		_mm512_store_si512(storageLocation, value);
	}

		#endif

	#endif

#else
	 template<typename value_type>
	 concept simd_int_128_t = std::same_as<jsonifier::concepts::unwrap<value_type>, simd_int_128>;

	 template<simd_int_128_t simd_type, typename char_type> inline simd_type gatherValues(char_type* str) {
		 simd_type returnValue{};
		 std::memcpy(&returnValue, str, sizeof(simd_type));
		 return returnValue;
	 }

	 template<simd_int_128_t simd_type, typename char_type> inline simd_type gatherValuesU(char_type* str) {
		 simd_type returnValue{};
		 std::memcpy(&returnValue, str, sizeof(simd_type));
		 return returnValue;
	 }

	 template<typename value_type> inline void storeu(const simd_int_t& value, value_type* storageLocation) {
		 std::memcpy(storageLocation, &value, sizeof(simd_int_t));
	 }

	 template<typename value_type> inline void store(const simd_int_t& value, value_type* storageLocation) {
		 std::memcpy(storageLocation, &value, sizeof(simd_int_t));
	 }

	 template<simd_int_128_t return_type> constexpr return_type simdFromTable(ctime_array<uint8_t, 16> arrayNew01) {
		 simd_int_128 returnValue{};
		 for (uint64_t x = 0; x < sizeof(simd_int_128); ++x) {
			 returnValue.m128x_uint8[x] = arrayNew01[x];
		 }
		 return returnValue;
	 }

	 template<simd_int_128_t return_type> constexpr return_type simdFromValue(uint8_t value) {
		 simd_int_128 returnValue{};
		 for (uint64_t x = 0; x < sizeof(simd_int_128); ++x) {
			 returnValue.m128x_uint8[x] = value;
		 }
		 return returnValue;
	 }

#endif

	class simd_base {
	  public:
		static constexpr ctime_array<uint8_t, 16> whitespaceArray{ 0x20u, 0x64u, 0x64u, 0x64u, 0x11u, 0x64u, 0x71u, 0x02u, 0x64u, 0x09u, 0x0Au, 0x70u, 0x64u, 0x0Du, 0x64u, 0x64u };
		static constexpr ctime_array<uint8_t, 16> opArray{ 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x3Au, 0x7Bu, 0x2Cu, 0x7Du, 0x00u, 0x00u };
		static constexpr ctime_array<uint8_t, 16> escapeableArray01{ 0x00u, 0x00u, 0x22u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x08u, 0x09u, 0x0Au, 0x00u, 0x0Cu, 0x0Du, 0x00u,
			0x00u };
		static constexpr ctime_array<uint8_t, 16> escapeableArray02{ 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x5Cu, 0x00u, 0x00u,
			0x00u };
		static constexpr simd_int_t whitespaceTable{ simdFromTable<simd_int_t>(whitespaceArray) };
		static constexpr simd_int_t opTableVal{ simdFromTable<simd_int_t>(opArray) };
		static constexpr simd_int_t backslashes{ simdFromValue<simd_int_t>(0x5Cu) };
		static constexpr simd_int_t quotes{ simdFromValue<simd_int_t>(0x22u) };
		static constexpr simd_int_t chars{ simdFromValue<simd_int_t>(0x20u) };

		inline static string_parsing_type cmpeq(const simd_int_t& other, const simd_int_t& value);

		inline static simd_int_t bitAndNot(const simd_int_t& value, const simd_int_t& other);

		inline static simd_int_t shuffle(const simd_int_t& value, const simd_int_t& other);

		inline static simd_int_t opAnd(const simd_int_t& other, const simd_int_t& value);

		inline static simd_int_t opXor(const simd_int_t& other, const simd_int_t& value);

		inline static simd_int_t opOr(const simd_int_t& other, const simd_int_t& value);

		inline static simd_int_t setLSB(const simd_int_t& value, bool values);

		inline static simd_int_t opNot(const simd_int_t& value);

		inline static bool getMSB(const simd_int_t& value);

		inline static bool opBool(const simd_int_t& value);

		inline static simd_int_t reset();

		template<typename simd_type> inline static std::enable_if_t<!std::same_as<simd_type, simd_int_t>, uint16_t> cmpeq(const simd_type& value, const simd_type& other) {
			return static_cast<uint16_t>(_mm_movemask_epi8(_mm_cmpeq_epi8(value, other)));
		}

		template<typename simd_type> inline static std::enable_if_t<!std::same_as<simd_type, simd_int_t>, simd_int_128> shuffle(const simd_type& value, const simd_type& other) {
			return _mm_shuffle_epi8(value, other);
		}

		template<uint64_t index> inline static simd_int_t opSubHelper(uint64_t* values01, uint64_t* values02, uint8_t carryInNew) {
			if constexpr (index < SixtyFourBitsPerStep) {
				values02[index] = values01[index] - values02[index] - static_cast<uint64_t>(carryInNew);
				carryInNew		= (values02[index] > values01[index]);
				return opSubHelper<index + 1>(values01, values02, carryInNew);
			} else {
				return gatherValues<simd_int_t>(values02);
			}
		}

		inline static simd_int_t opSub(const simd_int_t& value, const simd_int_t& other) {
			alignas(BytesPerStep) uint64_t values01[SixtyFourBitsPerStep]{};
			alignas(BytesPerStep) uint64_t values02[SixtyFourBitsPerStep]{};
			store(value, values01);
			store(other, values02);
			uint8_t carryInCurr{};
			return opSubHelper<0>(values01, values02, carryInCurr);
		}

		template<uint64_t index = 0>
		inline static void collectWhitespaceAsSimdBaseHelper(string_parsing_type arrayNew[StridesPerStep], simd_int_t& value, const simd_int_t values[StridesPerStep]) {
			if constexpr (index < StridesPerStep) {
				arrayNew[index] = cmpeq(shuffle(whitespaceTable, values[index]), values[index]);
				return collectWhitespaceAsSimdBaseHelper<index + 1>(arrayNew, value, values);
			}
		}

		inline static void collectWhitespaceAsSimdBase(simd_int_t& value, const simd_int_t values[StridesPerStep]) {
			alignas(BytesPerStep) string_parsing_type arrayNew[StridesPerStep]{};
			collectWhitespaceAsSimdBaseHelper<0>(arrayNew, value, values);
			value = gatherValues<simd_int_t>(arrayNew);
		}

		template<uint64_t index = 0>
		inline static void collectBackslashesAsSimdBaseHelper(string_parsing_type arrayNew[StridesPerStep], simd_int_t& value, const simd_int_t values[StridesPerStep]) {
			if constexpr (index < StridesPerStep) {
				arrayNew[index] = cmpeq(values[index], backslashes);
				return collectBackslashesAsSimdBaseHelper<index + 1>(arrayNew, value, values);
			}
		}

		inline static void collectBackslashesAsSimdBase(simd_int_t& value, const simd_int_t values[StridesPerStep]) {
			alignas(BytesPerStep) string_parsing_type arrayNew[StridesPerStep]{};
			collectBackslashesAsSimdBaseHelper<0>(arrayNew, value, values);
			value = gatherValues<simd_int_t>(arrayNew);
		}

		template<uint64_t index = 0>
		inline static void collectStructuralsAsSimdBaseHelper(string_parsing_type arrayNew[StridesPerStep], simd_int_t& value, const simd_int_t values[StridesPerStep]) {
			if constexpr (index < StridesPerStep) {
				arrayNew[index] = cmpeq(shuffle(opTableVal, values[index]), (opOr(values[index], chars)));
				return collectStructuralsAsSimdBaseHelper<index + 1>(arrayNew, value, values);
			}
		}

		inline static void collectStructuralsAsSimdBase(simd_int_t& value, const simd_int_t values[StridesPerStep]) {
			alignas(BytesPerStep) string_parsing_type arrayNew[StridesPerStep]{};
			collectStructuralsAsSimdBaseHelper<0>(arrayNew, value, values);
			value = gatherValues<simd_int_t>(arrayNew);
		}

		template<uint64_t index = 0>
		inline static void collectQuotesAsSimdBaseHelper(string_parsing_type arrayNew[StridesPerStep], simd_int_t& value, const simd_int_t values[StridesPerStep]) {
			if constexpr (index < StridesPerStep) {
				arrayNew[index] = cmpeq(values[index], quotes);
				return collectQuotesAsSimdBaseHelper<index + 1>(arrayNew, value, values);
			}
		}

		inline static void collectQuotesAsSimdBase(simd_int_t& value, const simd_int_t values[StridesPerStep]) {
			alignas(BytesPerStep) string_parsing_type arrayNew[StridesPerStep]{};
			collectQuotesAsSimdBaseHelper<0>(arrayNew, value, values);
			value = gatherValues<simd_int_t>(arrayNew);
		}

		inline static int64_t prefixXor(int64_t prevInString) {
			prevInString ^= prevInString << 1;
			prevInString ^= prevInString << 2;
			prevInString ^= prevInString << 4;
			prevInString ^= prevInString << 8;
			prevInString ^= prevInString << 16;
			prevInString ^= prevInString << 32;
			return prevInString;
		}

		template<uint64_t index> inline static simd_int_t clmulHelper(int64_t* values, uint64_t& newValue, bool& prevInString) {
			if constexpr (index < SixtyFourBitsPerStep) {
				values[index] = prefixXor(values[index]) ^ static_cast<int64_t>(newValue);
				newValue	  = uint64_t(values[index] >> 63);
				return clmulHelper<index + 1>(values, newValue, prevInString);
			} else {
				prevInString = (newValue >> 63) & 1;
				return gatherValues<simd_int_t>(values);
			}
		}

		inline static simd_int_t carrylessMultiplication(const simd_int_t& value, bool& prevInString) {
			alignas(BytesPerStep) int64_t values[SixtyFourBitsPerStep]{};
			uint64_t newValue{};
			if (prevInString) {
				newValue = std::numeric_limits<uint64_t>::max();
			}
			store(value, values);
			return clmulHelper<0>(values, newValue, prevInString);
		}

		template<uint64_t amount, uint64_t index> inline static simd_int_t shlHelper(uint64_t* newArray00, uint64_t* newArray01) {
			if constexpr (index < SixtyFourBitsPerStep) {
				newArray01[index] = (newArray00[index] << amount) | (newArray00[index - 1] >> (64 - amount));
				return shlHelper<amount, index + 1>(newArray00, newArray01);
			} else {
				return gatherValues<simd_int_t>(newArray01);
			}
		}

		template<uint64_t amount> inline static simd_int_t shl(const simd_int_t& value) {
			alignas(BytesPerStep) uint64_t newArray00[SixtyFourBitsPerStep]{};
			alignas(BytesPerStep) uint64_t newArray01[SixtyFourBitsPerStep]{};
			store(value, newArray00);
			newArray01[0] = (newArray00[0] << amount);
			return shlHelper<amount, 1>(newArray00, newArray01);
		}

		inline static simd_int_t follows(const simd_int_t& value, bool& overflow) {
			auto oldOverflow = overflow;
			overflow		 = getMSB(value);
			return setLSB(shl<1>(value), oldOverflow);
		}
	};

}