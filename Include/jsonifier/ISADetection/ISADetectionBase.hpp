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

#if !defined(jsonifier_inline)
	#define jsonifier_inline inline
#endif

#if !defined(jsonifier_constexpr)
	#define jsonifier_constexpr constexpr
#endif

#ifndef JSONIFIER_CPU_INSTRUCTIONS
	#define JSONIFIER_CPU_INSTRUCTIONS 0
#endif

#ifndef JSONIFIER_CHECK_FOR_INSTRUCTION
	#define JSONIFIER_CHECK_FOR_INSTRUCTION(x) (JSONIFIER_CPU_INSTRUCTIONS & x)
#endif

#ifndef JSONIFIER_POPCNT
	#define JSONIFIER_POPCNT (1 << 0)
#endif
#ifndef JSONIFIER_LZCNT
	#define JSONIFIER_LZCNT (1 << 1)
#endif
#ifndef JSONIFIER_BMI
	#define JSONIFIER_BMI (1 << 2)
#endif
#ifndef JSONIFIER_BMI2
	#define JSONIFIER_BMI2 (1 << 3)
#endif
#ifndef JSONIFIER_AVX
	#define JSONIFIER_AVX (1 << 4)
#endif
#ifndef JSONIFIER_AVX2
	#define JSONIFIER_AVX2 (1 << 5)
#endif
#ifndef JSONIFIER_AVX512
	#define JSONIFIER_AVX512 (1 << 6)
#endif

#include <jsonifier/TypeEntities.hpp>
#include <source_location>
#include <iostream>
#include <cstring>
#include <cstdint>
#include <bitset>

#if JSONIFIER_CHECK_FOR_INSTRUCTION(JSONIFIER_AVX512)

constexpr uint64_t BitsPerStep{ 512 };
constexpr uint64_t BytesPerStep{ BitsPerStep / 8 };
constexpr uint64_t SixtyFourBitsPerStep{ BitsPerStep / 64 };
constexpr uint64_t StridesPerStep{ BitsPerStep / BytesPerStep };
using string_parsing_type = uint64_t;

#elif JSONIFIER_CHECK_FOR_INSTRUCTION(JSONIFIER_AVX2)

constexpr uint64_t BitsPerStep{ 256 };
constexpr uint64_t BytesPerStep{ BitsPerStep / 8 };
constexpr uint64_t SixtyFourBitsPerStep{ BitsPerStep / 64 };
constexpr uint64_t StridesPerStep{ BitsPerStep / BytesPerStep };
using string_parsing_type = uint32_t;

#elif JSONIFIER_CHECK_FOR_INSTRUCTION(JSONIFIER_AVX)

constexpr uint64_t BitsPerStep{ 128 };
constexpr uint64_t BytesPerStep{ BitsPerStep / 8 };
constexpr uint64_t SixtyFourBitsPerStep{ BitsPerStep / 64 };
constexpr uint64_t StridesPerStep{ BitsPerStep / BytesPerStep };
using string_parsing_type = uint16_t;

#else

constexpr uint64_t BitsPerStep{ 128 };
constexpr uint64_t BytesPerStep{ BitsPerStep / 8 };
constexpr uint64_t SixtyFourBitsPerStep{ BitsPerStep / 64 };
constexpr uint64_t StridesPerStep{ BitsPerStep / BytesPerStep };
using string_parsing_type = uint16_t;

#endif

#if JSONIFIER_CHECK_FOR_INSTRUCTION(JSONIFIER_AVX) || JSONIFIER_CHECK_FOR_INSTRUCTION(JSONIFIER_AVX2) || JSONIFIER_CHECK_FOR_INSTRUCTION(JSONIFIER_AVX512)
	#include <immintrin.h>

using simd_int_512	 = __m512i;
using simd_int_256	 = __m256i;
using simd_int_128	 = __m128i;
using simd_float_512 = __m512d;
using simd_float_256 = __m256d;
using simd_float_128 = __m128d;

#else

typedef union alignas(BytesPerStep) __m128x {
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

namespace jsonifier_internal {

	template<uint64_t multiple> jsonifier_inline uint64_t roundUpToMultiple(uint64_t num) {
		uint64_t remainder = num % multiple;
		return remainder == 0 ? num : num + (multiple - remainder);
	}

	template<uint64_t BitsPerStep> class simd_base_internal {};

	using simd_base			= simd_base_internal<BitsPerStep>;
	using string_view_ptr	= const uint8_t*;
	using structural_index	= string_view_ptr;
	using string_buffer_ptr = uint8_t*;

};