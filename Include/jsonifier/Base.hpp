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

#include <jsonifier/TypeEntities.hpp>
#include <functional>
#include <cassert>
#include <cstring>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cfloat>
#include <array>

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

#if defined(__clang__)
	#define JSONIFIER_CLANG 1
#elif defined(__GNUC__) && defined(__llvm__)
	#define JSONIFIER_CLANG 1
#elif defined(__APPLE__) && defined(__clang__)
	#define JSONIFIER_CLANG 1
#elif defined(_MSC_VER)
	#define JSONIFIER_MSVC 1
#elif defined(__GNUC__) && !defined(__clang__)
	#define JSONIFIER_GNUCXX 1
#endif

#if defined(macintosh) || defined(Macintosh) || (defined(__APPLE__) && defined(__MACH__))
	#define JSONIFIER_MAC 1
#elif defined(linux) || defined(__linux) || defined(__linux__) || defined(__gnu_linux__)
	#define JSONIFIER_LINUX 1
#elif defined(WIN32) || defined(_WIN32) || defined(_WIN64)
	#define JSONIFIER_WIN 1
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
#if !defined(JSONIFIER_NEON)
	#define JSONIFIER_NEON (1 << 4)
#endif
#if !defined(JSONIFIER_AVX)
	#define JSONIFIER_AVX (1 << 5)
#endif
#if !defined(JSONIFIER_AVX2)
	#define JSONIFIER_AVX2 (1 << 6)
#endif
#if !defined(JSONIFIER_AVX512)
	#define JSONIFIER_AVX512 (1 << 7)
#endif

#if !defined(JSONIFIER_ANY)
	#define JSONIFIER_ANY (JSONIFIER_AVX | JSONIFIER_AVX2 | JSONIFIER_AVX512 | JSONIFIER_POPCNT | JSONIFIER_BMI | JSONIFIER_BMI2 | JSONIFIER_LZCNT)
#endif

#if !defined(JSONIFIER_ANY_AVX)
	#define JSONIFIER_ANY_AVX (JSONIFIER_AVX | JSONIFIER_AVX2 | JSONIFIER_AVX512)
#endif

#if defined(_MSC_VER)
	#define JSONIFIER_VISUAL_STUDIO 1
	#if defined(__clang__)
		#define JSONIFIER_CLANG_VISUAL_STUDIO 1
	#else
		#define JSONIFIER_REGULAR_VISUAL_STUDIO 1
	#endif
#endif

#if JSONIFIER_CHECK_FOR_INSTRUCTION(JSONIFIER_ANY)

	#include <immintrin.h>

#endif

union __m128x;

#if JSONIFIER_CHECK_FOR_INSTRUCTION(JSONIFIER_ANY_AVX)

using simd_int_128 = __m128i;
using simd_int_256 = __m256i;
using simd_int_512 = __m512i;

	#if JSONIFIER_CHECK_FOR_INSTRUCTION(JSONIFIER_AVX512)
using simd_int_t = __m512i;
constexpr uint64_t BitsPerStep{ 512 };
using string_parsing_type = uint64_t;
	#elif JSONIFIER_CHECK_FOR_INSTRUCTION(JSONIFIER_AVX2)
using simd_int_t = __m256i;
constexpr uint64_t BitsPerStep{ 256 };
using string_parsing_type = uint32_t;
	#elif JSONIFIER_CHECK_FOR_INSTRUCTION(JSONIFIER_AVX)
using simd_int_t = __m128i;
constexpr uint64_t BitsPerStep{ 128 };
using string_parsing_type = uint16_t;
	#endif
#elif JSONIFIER_CHECK_FOR_INSTRUCTION(JSONIFIER_NEON)

	#include <arm_neon.h>

	#if __CHAR_UNSIGNED__
		#define CHAR_TYPE uint8_t
		#define vreinterpretq_x8_x16 vreinterpretq_u8_u16
		#define vdupq_n_x8 vdupq_n_u8
		#define vld1q_x16 vld1q_u16
		#define vpaddq_x8 vpaddq_u8
		#define vld1q_x8 vld1q_u8
		#define vceqq_x8 vceqq_u8
		#define vst1q_x8 vst1q_u8
	#else
		#define CHAR_TYPE int8_t
		#define vreinterpretq_x8_x16 vreinterpretq_s8_s16
		#define vdupq_n_x8 vdupq_n_s8
		#define vld1q_x16 vld1q_s16
		#define vpaddq_x8 vpaddq_s8
		#define vld1q_x8 vld1q_s8
		#define vceqq_x8 vceqq_s8
		#define vst1q_x8 vst1q_s8
	#endif


using simd_int_128 = uint8x16_t;
using simd_int_256 = uint32_t;
using simd_int_512 = uint64_t;

using simd_int_t = uint8x16_t;
constexpr uint64_t BitsPerStep{ 128 };
using string_parsing_type = uint16_t;
#else
union __m128x {
	#if JSONIFIER_WIN
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
	int32_t m128x_int32[4];
	int16_t m128x_int16[8];
	int8_t m128x_int8[16]{};
	uint64_t m128x_uint64[2];
	int32_t m128x_uint32[4];
	int16_t m128x_uint16[8];
	uint8_t m128x_uint8[16];
	#endif
};
using simd_int_128 = __m128x;
using simd_int_256 = uint32_t;
using simd_int_512 = uint64_t;

using simd_int_t = __m128x;
constexpr uint64_t BitsPerStep{ 128 };
using string_parsing_type = uint16_t;
#endif

constexpr uint64_t BytesPerStep{ BitsPerStep / 8 };
constexpr uint64_t SixtyFourBitsPerStep{ BitsPerStep / 64 };
constexpr uint64_t StridesPerStep{ BitsPerStep / BytesPerStep };

using string_view_ptr	= const char*;
using structural_index	= const char*;
using string_buffer_ptr = char*;

template<typename value_type>
concept simd_int_512_type = std::is_same_v<simd_int_512, jsonifier::concepts::unwrap_t<value_type>>;
template<typename value_type>
concept simd_int_256_type = std::is_same_v<simd_int_256, jsonifier::concepts::unwrap_t<value_type>>;
template<typename value_type>
concept simd_int_128_type = std::is_same_v<simd_int_128, jsonifier::concepts::unwrap_t<value_type>>;
template<typename value_type>
concept simd_int_type = std::is_same_v<simd_int_t, jsonifier::concepts::unwrap_t<value_type>>;
