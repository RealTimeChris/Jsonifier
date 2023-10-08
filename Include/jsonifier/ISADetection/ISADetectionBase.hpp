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
#include <source_location>
#include <iostream>
#include <sstream>
#include <cstring>
#include <cstdint>
#include <string>
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

jsonifier_constexpr uint64_t BitsPerStep{ 512 };
jsonifier_constexpr uint64_t BytesPerStep{ BitsPerStep / 8 };
jsonifier_constexpr uint64_t SixtyFourBitsPerStep{ BitsPerStep / 64 };
jsonifier_constexpr uint64_t StridesPerStep{ BitsPerStep / BytesPerStep };
using string_parsing_type = uint64_t;
using simd_int_t		  = simd_int_512;

#elif JSONIFIER_CHECK_FOR_INSTRUCTION(JSONIFIER_AVX2)

jsonifier_constexpr uint64_t BitsPerStep{ 256 };
jsonifier_constexpr uint64_t BytesPerStep{ BitsPerStep / 8 };
jsonifier_constexpr uint64_t SixtyFourBitsPerStep{ BitsPerStep / 64 };
jsonifier_constexpr uint64_t StridesPerStep{ BitsPerStep / BytesPerStep };
using string_parsing_type = uint32_t;
using simd_int_t		  = simd_int_256;

#elif JSONIFIER_CHECK_FOR_INSTRUCTION(JSONIFIER_AVX)

jsonifier_constexpr uint64_t BitsPerStep{ 128 };
jsonifier_constexpr uint64_t BytesPerStep{ BitsPerStep / 8 };
jsonifier_constexpr uint64_t SixtyFourBitsPerStep{ BitsPerStep / 64 };
jsonifier_constexpr uint64_t StridesPerStep{ BitsPerStep / BytesPerStep };
using string_parsing_type = uint16_t;
using simd_int_t		  = simd_int_128;

#else

jsonifier_constexpr uint64_t BitsPerStep{ 128 };
jsonifier_constexpr uint64_t BytesPerStep{ BitsPerStep / 8 };
jsonifier_constexpr uint64_t SixtyFourBitsPerStep{ BitsPerStep / 64 };
jsonifier_constexpr uint64_t StridesPerStep{ BitsPerStep / BytesPerStep };
using string_parsing_type = uint16_t;
using simd_int_t		  = simd_int_128;

#endif

namespace jsonifier_internal {

	jsonifier_inline static void printBits(uint64_t values, const std::string& valuesTitle);

	template<typename simd_type> jsonifier_inline static std::string printBits(const simd_type& value) noexcept;

	template<typename simd_type> jsonifier_inline static const simd_type& printBits(const simd_type& value, const std::string& valuesTitle) noexcept;

	using string_view_ptr	= const uint8_t*;
	using structural_index	= string_view_ptr;
	using string_buffer_ptr = uint8_t*;

#if JSONIFIER_CHECK_FOR_INSTRUCTION(JSONIFIER_ANY_AVX)

	template<typename value_type>
	concept simd_int_128_t = std::same_as<jsonifier::concepts::unwrap<value_type>, simd_int_128>;

	template<typename value_type>
	concept simd_int_256_t = std::same_as<jsonifier::concepts::unwrap<value_type>, simd_int_256>;

	template<typename value_type>
	concept simd_int_512_t = std::same_as<jsonifier::concepts::unwrap<value_type>, simd_int_512>;

	template<typename value_type>
	concept simd_float_128_t = std::same_as<jsonifier::concepts::unwrap<value_type>, simd_float_128>;

	template<typename value_type>
	concept simd_float_256_t = std::same_as<jsonifier::concepts::unwrap<value_type>, simd_float_256>;

	template<typename value_type>
	concept simd_float_512_t = std::same_as<jsonifier::concepts::unwrap<value_type>, simd_float_512>;

	template<simd_int_128_t simd_type, typename char_type> jsonifier_inline simd_type gatherValues(char_type* str) {
		return _mm_load_si128(reinterpret_cast<const __m128i*>(str));
	}

	template<simd_int_256_t simd_type, typename char_type> jsonifier_inline simd_type gatherValues(char_type* str) {
		return _mm256_load_si256(reinterpret_cast<const __m256i*>(str));
	}

	template<simd_int_512_t simd_type, typename char_type> jsonifier_inline simd_type gatherValues(char_type* str) {
		return _mm512_load_si512(str);
	}

	template<simd_int_128_t simd_type, typename char_type> jsonifier_inline simd_type gatherValuesU(char_type* str) {
		return _mm_loadu_si128(reinterpret_cast<const __m128i*>(str));
	}

	template<simd_int_256_t simd_type, typename char_type> jsonifier_inline simd_type gatherValuesU(char_type* str) {
		return _mm256_loadu_si256(reinterpret_cast<const __m256i*>(str));
	}

	template<simd_int_512_t simd_type, typename char_type> jsonifier_inline simd_type gatherValuesU(char_type* str) {
		return _mm512_loadu_si512(str);
	}

	template<simd_float_128_t simd_type, jsonifier::concepts::float_t char_type> jsonifier_inline simd_type gatherValues(char_type* str) {
		return _mm_loadu_pd(str);
	}

	template<simd_float_256_t simd_type, jsonifier::concepts::float_t char_type> jsonifier_inline simd_type gatherValues(char_type* str) {
		return _mm256_loadu_pd(str);
	}

	template<simd_float_512_t simd_type, jsonifier::concepts::float_t char_type> jsonifier_inline simd_type gatherValues(char_type* str) {
		return _mm512_loadu_pd(str);
	}

	template<typename value_type> jsonifier_inline void storeu(const simd_int_128& value, value_type* storageLocation) {
		_mm_storeu_si128(reinterpret_cast<__m128i*>(storageLocation), value);
	}

	template<typename value_type> jsonifier_inline void storeu(const simd_int_256& value, value_type* storageLocation) {
		_mm256_storeu_si256(reinterpret_cast<__m256i*>(storageLocation), value);
	}

	template<typename value_type> jsonifier_inline void storeu(const simd_int_512& value, value_type* storageLocation) {
		_mm512_storeu_si512(storageLocation, value);
	}

	template<typename value_type> jsonifier_inline void store(const simd_int_128& value, value_type* storageLocation) {
		_mm_store_si128(reinterpret_cast<__m128i*>(storageLocation), value);
	}

	template<typename value_type> jsonifier_inline void store(const simd_int_256& value, value_type* storageLocation) {
		_mm256_store_si256(reinterpret_cast<__m256i*>(storageLocation), value);
	}

	template<typename value_type> jsonifier_inline void store(const simd_int_512& value, value_type* storageLocation) {
		_mm512_store_si512(storageLocation, value);
	}

	template<simd_int_128_t return_type> jsonifier_constexpr return_type simdFromTable(std::array<uint8_t, 16> arrayNew01) {
	#if !defined(_WIN32)
		int64_t newArray[sizeof(return_type) / sizeof(uint64_t)]{};
		for (uint64_t x = 0; x < sizeof(return_type) / sizeof(uint64_t); ++x) {
			newArray[x] |= static_cast<int64_t>(arrayNew01[((x * 8) + 7)]) << 56;
			newArray[x] |= static_cast<int64_t>(arrayNew01[((x * 8) + 6)]) << 48;
			newArray[x] |= static_cast<int64_t>(arrayNew01[((x * 8) + 5)]) << 40;
			newArray[x] |= static_cast<int64_t>(arrayNew01[((x * 8) + 4)]) << 32;
			newArray[x] |= static_cast<int64_t>(arrayNew01[((x * 8) + 3)]) << 24;
			newArray[x] |= static_cast<int64_t>(arrayNew01[((x * 8) + 2)]) << 16;
			newArray[x] |= static_cast<int64_t>(arrayNew01[((x * 8) + 1)]) << 8;
			newArray[x] |= static_cast<int64_t>(arrayNew01[((x * 8) + 0)]);
		}
		return_type returnValue{ newArray[0], newArray[1] };
	#else
		return_type returnValue{};
		for (uint64_t x = 0; x < sizeof(return_type); ++x) {
			returnValue.m128i_u8[x] = arrayNew01[x];
		}
	#endif
		return returnValue;
	}

	template<simd_int_128_t return_type> jsonifier_constexpr return_type simdFromValue(uint8_t value) {
	#if !defined(_WIN32)
		int64_t newArray[sizeof(return_type) / sizeof(uint64_t)]{};
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
		return_type returnValue{ newArray[0], newArray[1] };
	#else
		return_type returnValue{};
		for (uint64_t x = 0; x < sizeof(return_type); ++x) {
			returnValue.m128i_u8[x] = value;
		}
	#endif
		return returnValue;
	}

	template<simd_int_256_t return_type> jsonifier_constexpr return_type simdFromTable(std::array<uint8_t, 16> arrayNew01) {
	#if !defined(_WIN32)
		int64_t newArray[sizeof(return_type) / sizeof(uint64_t)]{};
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
		return_type returnValue{ newArray[0], newArray[1], newArray[2], newArray[3] };
	#else
		return_type returnValue{};
		for (uint64_t x = 0; x < sizeof(return_type); ++x) {
			returnValue.m256i_u8[x] = arrayNew01[x % 16];
		}
	#endif
		return returnValue;
	}

	template<simd_int_256_t return_type> jsonifier_constexpr return_type simdFromValue(uint8_t value) {
	#if !defined(_WIN32)
		int64_t newArray[sizeof(return_type) / sizeof(uint64_t)]{};
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
		return_type returnValue{ newArray[0], newArray[1], newArray[2], newArray[3] };
	#else
		return_type returnValue{};
		for (uint64_t x = 0; x < sizeof(return_type); ++x) {
			returnValue.m256i_u8[x] = value;
		}
	#endif
		return returnValue;
	}

	template<simd_int_512_t return_type> jsonifier_constexpr return_type simdFromTable(std::array<uint8_t, 16> arrayNew01) {
	#if !defined(_WIN32)
		int64_t newArray[sizeof(return_type) / sizeof(uint64_t)]{};
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
		return_type returnValue{ newArray[0], newArray[1], newArray[2], newArray[3], newArray[4], newArray[5], newArray[6], newArray[7] };
	#else
		return_type returnValue{};
		for (uint64_t x = 0; x < sizeof(return_type); ++x) {
			returnValue.m512i_u8[x] = arrayNew01[x % 16];
		}
	#endif
		return returnValue;
	}

	template<simd_int_512_t return_type> jsonifier_constexpr return_type simdFromValue(uint8_t value) {
	#if !defined(_WIN32)
		int64_t newArray[sizeof(return_type) / sizeof(uint64_t)]{};
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
		return_type returnValue{ newArray[0], newArray[1], newArray[2], newArray[3], newArray[4], newArray[5], newArray[6], newArray[7] };
	#else
		return_type returnValue{};
		for (uint64_t x = 0; x < sizeof(return_type); ++x) {
			returnValue.m512i_u8[x] = value;
		}
	#endif
		return returnValue;
	}

#else

	template<typename value_type>
	concept simd_int_128_t = std::same_as<jsonifier::concepts::unwrap<value_type>, simd_int_128>;

	template<simd_int_128_t simd_type, typename char_type> jsonifier_inline simd_type gatherValues(char_type* str) {
		simd_type returnValue{};
		std::memcpy(&returnValue, str, sizeof(simd_type));
		return returnValue;
	}

	template<simd_int_128_t simd_type, typename char_type> jsonifier_inline simd_type gatherValuesU(char_type* str) {
		simd_type returnValue{};
		std::memcpy(&returnValue, str, sizeof(simd_type));
		return returnValue;
	}

	template<typename value_type> jsonifier_inline void storeu(const simd_int_t& value, value_type* storageLocation) {
		std::memcpy(storageLocation, &value, sizeof(simd_int_t));
	}

	template<typename value_type> jsonifier_inline void store(const simd_int_t& value, value_type* storageLocation) {
		std::memcpy(storageLocation, &value, sizeof(simd_int_t));
	}

	template<simd_int_128_t return_type> jsonifier_constexpr return_type simdFromTable(const uint8_t arrayNew01[sizeof(simd_int_128)]) {
		simd_int_128 returnValue{};
		for (uint64_t x = 0; x < sizeof(simd_int_128); ++x) {
			returnValue.m128x_uint8[x] = arrayNew01[x];
		}
		return returnValue;
	}

	template<simd_int_128_t return_type> jsonifier_constexpr return_type simdFromValue(uint8_t value) {
		simd_int_128 returnValue{};
		for (uint64_t x = 0; x < sizeof(simd_int_128); ++x) {
			returnValue.m128x_uint8[x] = value;
		}
		return returnValue;
	}

#endif

	class simd_base {
	  public:
		static jsonifier_constexpr std::array<uint8_t, 16> whitespaceArray{ 0x20u, 0x64u, 0x64u, 0x64u, 0x11u, 0x64u, 0x71u, 0x02u, 0x64u, 0x09u, 0x0Au, 0x70u, 0x64u, 0x0Du, 0x64u,
			0x64u };
		static jsonifier_constexpr std::array<uint8_t, 16> opArray{ 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x3Au, 0x7Bu, 0x2Cu, 0x7Du, 0x00u,
			0x00u };
		static jsonifier_constexpr std::array<uint8_t, 16> escapeableArray01{ 0x00u, 0x00u, 0x22u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x08u, 0x09u, 0x0Au, 0x00u, 0x0Cu, 0x0Du,
			0x00u, 0x00u };
		static jsonifier_constexpr std::array<uint8_t, 16> escapeableArray02{ 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x5Cu, 0x00u,
			0x00u, 0x00u };
		static jsonifier_constexpr simd_int_t whitespaceTable{ simdFromTable<simd_int_t>(whitespaceArray) };
		static jsonifier_constexpr simd_int_t opTableVal{ simdFromTable<simd_int_t>(opArray) };
		static jsonifier_constexpr simd_int_t backslashes{ simdFromValue<simd_int_t>(0x5Cu) };
		static jsonifier_constexpr simd_int_t quotes{ simdFromValue<simd_int_t>(0x22u) };
		static jsonifier_constexpr simd_int_t chars{ simdFromValue<simd_int_t>(0x20u) };

		jsonifier_inline static string_parsing_type cmpeq(const simd_int_t& other, const simd_int_t& value);

		jsonifier_inline static simd_int_t bitAndNot(const simd_int_t& value, const simd_int_t& other);

		jsonifier_inline static simd_int_t shuffle(const simd_int_t& value, const simd_int_t& other);

		jsonifier_inline static simd_int_t opOr(const simd_int_t& other, const simd_int_t& value);

		jsonifier_inline static simd_int_t opAnd(const simd_int_t& other, const simd_int_t& value);

		jsonifier_inline static simd_int_t opXor(const simd_int_t& other, const simd_int_t& value);

		jsonifier_inline static simd_int_t setLSB(const simd_int_t& value, bool valueNew);

		jsonifier_inline static simd_int_t opNot(const simd_int_t& value);

		jsonifier_inline static bool getMSB(const simd_int_t& value);

		jsonifier_inline static bool opBool(const simd_int_t& value);

		jsonifier_inline static simd_int_t reset();

		template<typename simd_type>
		jsonifier_inline static std::enable_if_t<!std::same_as<simd_type, simd_int_t>, uint16_t> cmpeq(const simd_type& value, const simd_type& other) {
			return static_cast<uint16_t>(_mm_movemask_epi8(_mm_cmpeq_epi8(value, other)));
		}

		template<typename simd_type>
		jsonifier_inline static std::enable_if_t<!std::same_as<simd_type, simd_int_t>, simd_int_128> shuffle(const simd_type& value, const simd_type& other) {
			return _mm_shuffle_epi8(value, other);
		}

		jsonifier_inline static uint64_t subborrow_u64(uint8_t carryIn, uint64_t a, uint64_t b, uint8_t& borrowOut) {
			borrowOut = (a < b + carryIn);
			return a - b - carryIn;
		}

		jsonifier_inline static simd_int_t opSub(const simd_int_t& other, const simd_int_t& value) {
			alignas(BytesPerStep) uint64_t valuesNew01[SixtyFourBitsPerStep]{};
			alignas(BytesPerStep) long long unsigned int valuesNew02[SixtyFourBitsPerStep]{};
			uint8_t carryInFlag{};
			store(other, valuesNew01);
			store(value, valuesNew02);
			for (uint64_t x = 0; x < SixtyFourBitsPerStep; ++x) {
				valuesNew02[x] = subborrow_u64(carryInFlag, valuesNew01[x], valuesNew02[x], carryInFlag);
			}
			return gatherValues<simd_int_t>(valuesNew02);
		}

		jsonifier_inline static void collectWhitespaceAsSimdBase(simd_int_t& value, const simd_int_t valuesNew[StridesPerStep]) {
			alignas(BytesPerStep) string_parsing_type arrayNew[StridesPerStep]{};
			arrayNew[0] = cmpeq(shuffle(whitespaceTable, valuesNew[0]), valuesNew[0]);
			arrayNew[1] = cmpeq(shuffle(whitespaceTable, valuesNew[1]), valuesNew[1]);
			arrayNew[2] = cmpeq(shuffle(whitespaceTable, valuesNew[2]), valuesNew[2]);
			arrayNew[3] = cmpeq(shuffle(whitespaceTable, valuesNew[3]), valuesNew[3]);
			arrayNew[4] = cmpeq(shuffle(whitespaceTable, valuesNew[4]), valuesNew[4]);
			arrayNew[5] = cmpeq(shuffle(whitespaceTable, valuesNew[5]), valuesNew[5]);
			arrayNew[6] = cmpeq(shuffle(whitespaceTable, valuesNew[6]), valuesNew[6]);
			arrayNew[7] = cmpeq(shuffle(whitespaceTable, valuesNew[7]), valuesNew[7]);
			value		= gatherValues<simd_int_t>(arrayNew);
		}

		jsonifier_inline static void collectBackslashesAsSimdBase(simd_int_t& value, const simd_int_t valuesNew[StridesPerStep]) {
			alignas(BytesPerStep) string_parsing_type arrayNew[StridesPerStep]{};
			arrayNew[0] = cmpeq(valuesNew[0], backslashes);
			arrayNew[1] = cmpeq(valuesNew[1], backslashes);
			arrayNew[2] = cmpeq(valuesNew[2], backslashes);
			arrayNew[3] = cmpeq(valuesNew[3], backslashes);
			arrayNew[4] = cmpeq(valuesNew[4], backslashes);
			arrayNew[5] = cmpeq(valuesNew[5], backslashes);
			arrayNew[6] = cmpeq(valuesNew[6], backslashes);
			arrayNew[7] = cmpeq(valuesNew[7], backslashes);
			value		= gatherValues<simd_int_t>(arrayNew);
		}

		jsonifier_inline static void collectStructuralsAsSimdBase(simd_int_t& value, const simd_int_t valuesNew[StridesPerStep]) {
			alignas(BytesPerStep) string_parsing_type arrayNew[StridesPerStep]{};
			arrayNew[0] = cmpeq(shuffle(opTableVal, valuesNew[0]), (opOr(valuesNew[0], chars)));
			arrayNew[1] = cmpeq(shuffle(opTableVal, valuesNew[1]), (opOr(valuesNew[1], chars)));
			arrayNew[2] = cmpeq(shuffle(opTableVal, valuesNew[2]), (opOr(valuesNew[2], chars)));
			arrayNew[3] = cmpeq(shuffle(opTableVal, valuesNew[3]), (opOr(valuesNew[3], chars)));
			arrayNew[4] = cmpeq(shuffle(opTableVal, valuesNew[4]), (opOr(valuesNew[4], chars)));
			arrayNew[5] = cmpeq(shuffle(opTableVal, valuesNew[5]), (opOr(valuesNew[5], chars)));
			arrayNew[6] = cmpeq(shuffle(opTableVal, valuesNew[6]), (opOr(valuesNew[6], chars)));
			arrayNew[7] = cmpeq(shuffle(opTableVal, valuesNew[7]), (opOr(valuesNew[7], chars)));
			value		= gatherValues<simd_int_t>(arrayNew);
		}

		jsonifier_inline static void collectQuotesAsSimdBase(simd_int_t& value, const simd_int_t valuesNew[StridesPerStep]) {
			alignas(BytesPerStep) string_parsing_type arrayNew[StridesPerStep]{};
			arrayNew[0] = cmpeq(valuesNew[0], quotes);
			arrayNew[1] = cmpeq(valuesNew[1], quotes);
			arrayNew[2] = cmpeq(valuesNew[2], quotes);
			arrayNew[3] = cmpeq(valuesNew[3], quotes);
			arrayNew[4] = cmpeq(valuesNew[4], quotes);
			arrayNew[5] = cmpeq(valuesNew[5], quotes);
			arrayNew[6] = cmpeq(valuesNew[6], quotes);
			arrayNew[7] = cmpeq(valuesNew[7], quotes);
			value		= gatherValues<simd_int_t>(arrayNew);
		}

		jsonifier_inline static int64_t prefixXor(int64_t prevInString) {
			prevInString ^= prevInString << 1;
			prevInString ^= prevInString << 2;
			prevInString ^= prevInString << 4;
			prevInString ^= prevInString << 8;
			prevInString ^= prevInString << 16;
			prevInString ^= prevInString << 32;
			return prevInString;
		}

		jsonifier_inline static simd_int_t carrylessMultiplication(const simd_int_t& value, bool& prevInString) {
			alignas(BytesPerStep) int64_t values[SixtyFourBitsPerStep]{};
			uint64_t newValue{};
			if (prevInString) {
				newValue = std::numeric_limits<uint64_t>::max();
			}
			store(value, values);
			for (uint64_t x = 0; x < SixtyFourBitsPerStep; ++x) {
				values[x] = prefixXor(values[x]) ^ static_cast<int64_t>(newValue);
				newValue  = uint64_t(values[x] >> 63);
			}
			prevInString = (newValue >> 63) & 1;
			return gatherValues<simd_int_t>(values);
		}

		template<uint64_t amount> jsonifier_inline static simd_int_t shl(const simd_int_t& value) {
			alignas(BytesPerStep) uint64_t newArray00[SixtyFourBitsPerStep]{};
			alignas(BytesPerStep) uint64_t newArray01[SixtyFourBitsPerStep]{};
			store(value, newArray00);
			newArray01[0] = (newArray00[0] << amount);
			for (uint64_t x = 1; x < SixtyFourBitsPerStep; ++x) {
				newArray01[x] = (newArray00[x] << amount) | (newArray00[x - 1] >> (64 - amount));
			}
			return gatherValues<simd_int_t>(newArray01);
		}

		jsonifier_inline static simd_int_t follows(const simd_int_t& value, bool& overflow) {
			simd_int_t result = shl<1>(value);
			result			  = setLSB(result, overflow);
			overflow		  = getMSB(value);
			return result;
		}
	};

}