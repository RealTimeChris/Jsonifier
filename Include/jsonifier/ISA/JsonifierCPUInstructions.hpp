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
/// Updated: Sep 3, 2024
#pragma once

#if defined(_MSC_VER) && defined(__AVX2__)
	#ifndef __POPCNT__
		#define __POPCNT__ 1
	#endif
	#ifndef __LZCNT__
		#define __LZCNT__ 1
	#endif
	#ifndef __BMI__
		#define __BMI__ 1
	#endif
#endif

#if !defined(JSONIFIER_CHECK_FOR_INSTRUCTION)
	#define JSONIFIER_CHECK_FOR_INSTRUCTION(x) (JSONIFIER_CPU_INSTRUCTIONS & x)
#endif

#if defined(__POPCNT__)
	#define JSONIFIER_POPCNT 1
#else
	#define JSONIFIER_POPCNT 0
#endif
#if defined(__LZCNT__)
	#define JSONIFIER_LZCNT 1
#else
	#define JSONIFIER_LZCNT 0
#endif
#if defined(__BMI__)
	#define JSONIFIER_BMI 1
#else
	#define JSONIFIER_BMI 0
#endif
#if defined(__aarch64__) || defined(_M_ARM64) || defined(_M_ARM64EC)
	#define JSONIFIER_NEON 1
#else
	#define JSONIFIER_NEON 0
#endif
#if defined(__AVX__)
	#define JSONIFIER_AVX 1
#else
	#define JSONIFIER_AVX 0
#endif
#if defined(__AVX2__)
	#define JSONIFIER_AVX2 1
#else
	#define JSONIFIER_AVX2 0
#endif
#if defined(__AVX512__)
	#define JSONIFIER_AVX512 1
#else
	#define JSONIFIER_AVX512 0
#endif

#if !defined(JSONIFIER_CPU_INSTRUCTIONS)
	#define JSONIFIER_CPU_INSTRUCTIONS (JSONIFIER_POPCNT | JSONIFIER_LZCNT | JSONIFIER_BMI | JSONIFIER_NEON | JSONIFIER_AVX | JSONIFIER_AVX2 | JSONIFIER_AVX512)
#endif

#if !defined(JSONIFIER_ANY)
	#define JSONIFIER_ANY (JSONIFIER_AVX | JSONIFIER_AVX2 | JSONIFIER_AVX512 | JSONIFIER_POPCNT | JSONIFIER_BMI | JSONIFIER_LZCNT)
#endif

#if !defined(JSONIFIER_ANY_AVX)
	#define JSONIFIER_ANY_AVX (JSONIFIER_AVX | JSONIFIER_AVX2 | JSONIFIER_AVX512)
#endif
