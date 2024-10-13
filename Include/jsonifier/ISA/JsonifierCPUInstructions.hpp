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

#undef JSONIFIER_CPU_INSTRUCTIONS
#define JSONIFIER_CPU_INSTRUCTIONS 38

#if (JSONIFIER_CPU_INSTRUCTIONS & (1 << 0))
	#define JSONIFIER_POPCNT 
#endif
#if (JSONIFIER_CPU_INSTRUCTIONS & (1 << 1))
	#define JSONIFIER_LZCNT
#endif
#if (JSONIFIER_CPU_INSTRUCTIONS & (1 << 2))
	#define JSONIFIER_BMI
#endif
#if (JSONIFIER_CPU_INSTRUCTIONS & (1 << 3))
	#define JSONIFIER_NEON
#endif
#if (JSONIFIER_CPU_INSTRUCTIONS & (1 << 4))
	#define JSONIFIER_AVX
#endif
#if (JSONIFIER_CPU_INSTRUCTIONS & (1 << 5))
	#define JSONIFIER_AVX
	#define JSONIFIER_AVX2
#endif
#if (JSONIFIER_CPU_INSTRUCTIONS & (1 << 6))
	#define JSONIFIER_AVX
	#define JSONIFIER_AVX2
	#define JSONIFIER_AVX512
#endif

#if (defined(JSONIFIER_AVX) | defined(JSONIFIER_AVX2) || defined(JSONIFIER_AVX512) || defined(JSONIFIER_POPCNT) || defined(JSONIFIER_BMI) || defined(JSONIFIER_LZCNT))
	#define JSONIFIER_ANY 
#endif

#if (defined(JSONIFIER_AVX) || defined(JSONIFIER_AVX2) || defined(JSONIFIER_AVX512))
	#define JSONIFIER_ANY_AVX
#endif