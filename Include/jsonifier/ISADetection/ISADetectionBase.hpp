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
	#pragma warning(disable : 4251)
	#pragma warning(disable : 4371)
	#pragma warning(disable : 4514)
	#pragma warning(disable : 4623)
	#pragma warning(disable : 4625)
	#pragma warning(disable : 4626)
	#pragma warning(disable : 4820)
	#pragma warning(disable : 5027)
	#pragma warning(disable : 5246)
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

#if JSONIFIER_CHECK_FOR_INSTRUCTION(JSONIFIER_AVX) || JSONIFIER_CHECK_FOR_INSTRUCTION(JSONIFIER_AVX2) || JSONIFIER_CHECK_FOR_INSTRUCTION(JSONIFIER_AVX512)
	#include <immintrin.h>

using avx_int_512	= __m512i;
using avx_int_256	= __m256i;
using avx_int_128	= __m128i;
using avx_float_512 = __m512;
using avx_float_256 = __m256;
using avx_float_128 = __m128;

#else

struct __m128x {
	uint64_t values[2]{};
};

using avx_int_128 = __m128x;

#endif

#include <jsonifier/TypeEntities.hpp>
#include <source_location>
#include <iostream>
#include <cstring>
#include <cstdint>
#include <bitset>

namespace jsonifier_internal {

#if JSONIFIER_CHECK_FOR_INSTRUCTION(JSONIFIER_AVX512)

	constexpr uint64_t BitPerStep{ 512 };
	constexpr uint64_t BytesPerStep{ BitPerStep / 8 };
	constexpr uint64_t JsonifierAlignment{ BytesPerStep };
	constexpr uint64_t SixtyFourBitsPerStep{ BitPerStep / 64 };
	constexpr uint64_t StridesPerStep{ BitPerStep / BytesPerStep };
	using string_parsing_type = uint64_t;

#elif JSONIFIER_CHECK_FOR_INSTRUCTION(JSONIFIER_AVX2)

	constexpr uint64_t BitPerStep{ 256 };
	constexpr uint64_t BytesPerStep{ BitPerStep / 8 };
	constexpr uint64_t JsonifierAlignment{ BytesPerStep };
	constexpr uint64_t SixtyFourBitsPerStep{ BitPerStep / 64 };
	constexpr uint64_t StridesPerStep{ BitPerStep / BytesPerStep };
	using string_parsing_type = uint32_t;

#elif JSONIFIER_CHECK_FOR_INSTRUCTION(JSONIFIER_AVX)

	constexpr uint64_t BitPerStep{ 128 };
	constexpr uint64_t BytesPerStep{ BitPerStep / 8 };
	constexpr uint64_t JsonifierAlignment{ BytesPerStep };
	constexpr uint64_t SixtyFourBitsPerStep{ BitPerStep / 64 };
	constexpr uint64_t StridesPerStep{ BitPerStep / BytesPerStep };
	using string_parsing_type = uint16_t;

#else

	constexpr uint64_t BitPerStep{ 128 };
	constexpr uint64_t BytesPerStep{ BitPerStep / 8 };
	constexpr uint64_t JsonifierAlignment{ BytesPerStep };
	constexpr uint64_t SixtyFourBitsPerStep{ BitPerStep / 64 };
	constexpr uint64_t StridesPerStep{ BitPerStep / BytesPerStep };
	using string_parsing_type = uint16_t;

#endif

	template<uint64_t BitPerStep> struct simd_base_internal {};

	using simd_base			= simd_base_internal<BitPerStep>;
	using string_view_ptr	= const uint8_t*;
	using structural_index	= string_view_ptr;
	using string_buffer_ptr = uint8_t*;

};