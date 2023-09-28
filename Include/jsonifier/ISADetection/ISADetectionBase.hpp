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

#define JSONIFIER_CHECK_FOR_INSTRUCTION(x) (JSONIFIER_CPU_INSTRUCTIONS & x)

#define JSONIFIER_POPCNT (1 << 0)
#define JSONIFIER_LZCNT (1 << 1)
#define JSONIFIER_BMI (1 << 2)
#define JSONIFIER_AVX (1 << 3)
#define JSONIFIER_AVX2 (1 << 4)
#define JSONIFIER_AVX512 (1 << 5)

#if JSONIFIER_CHECK_FOR_INSTRUCTION(JSONIFIER_AVX512)
	#define ALIGNMENT 64
#elif JSONIFIER_CHECK_FOR_INSTRUCTION(JSONIFIER_AVX2)
	#define ALIGNMENT 32
#elif JSONIFIER_CHECK_FOR_INSTRUCTION(JSONIFIER_AVX)
	#define ALIGNMENT 16
#else
	#define ALIGNMENT 16
#endif

#if JSONIFIER_CHECK_FOR_INSTRUCTION(JSONIFIER_POPCNT) || JSONIFIER_CHECK_FOR_INSTRUCTION(JSONIFIER_LZCNT) || JSONIFIER_CHECK_FOR_INSTRUCTION(JSONIFIER_BMI) || JSONIFIER_CHECK_FOR_INSTRUCTION(JSONIFIER_AVX) || \
	JSONIFIER_CHECK_FOR_INSTRUCTION(JSONIFIER_AVX2) || JSONIFIER_CHECK_FOR_INSTRUCTION(JSONIFIER_AVX512)
	#include <immintrin.h>
#endif

#include <jsonifier/Concepts.hpp>
#include <source_location>
#include <iostream>
#include <cstring>
#include <bitset>

namespace jsonifier_internal {

	using string_view_ptr	= const uint8_t*;
	using structural_index	= string_view_ptr;
	using string_buffer_ptr = uint8_t*;

	template<uint64_t StepSize> struct simd_base {};

};