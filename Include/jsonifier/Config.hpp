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

#include <cstdint>

#if !defined(__GNUC__)
	#pragma warning(disable : 4371)
	#pragma warning(disable : 4514)
	#pragma warning(disable : 4625)
	#pragma warning(disable : 4706)
	#pragma warning(disable : 4710)
	#pragma warning(disable : 4711)
	#pragma warning(disable : 4820)
	#pragma warning(disable : 5045)
	#pragma warning(disable : 5246)
#endif

#if defined(__clang__) || (defined(__GNUC__) && defined(__llvm__)) || (defined(__APPLE__) && defined(__clang__))
	#define JSONIFIER_CLANG 1
#elif defined(_MSC_VER)
	#define JSONIFIER_MSVC 1
#elif defined(__GNUC__) && !defined(__clang__)
	#define JSONIFIER_GNUCXX 1
#endif

#if defined(__has_builtin)
	#define JSONIFIER_HAS_BUILTIN(x) __has_builtin(x)
#else
	#define JSONIFIER_HAS_BUILTIN(x) 0
#endif

#define JSONIFIER_GCC_VERSION (__GNUC__ * 100 + __GNUC_MINOR__)

#if defined(macintosh) || defined(Macintosh) || (defined(__APPLE__) && defined(__MACH__))
	#define JSONIFIER_MAC 1
#elif defined(linux) || defined(__linux) || defined(__linux__) || defined(__gnu_linux__)
	#define JSONIFIER_LINUX 1
#elif defined(WIN32) || defined(_WIN32) || defined(_WIN64)
	#define JSONIFIER_WIN 1
#endif

#if defined(JSONIFIER_GNUCXX) || defined(JSONIFIER_CLANG)
	#define LIKELY(x) (__builtin_expect(!!(x), 1))
	#define UNLIKELY(x) (__builtin_expect(!!(x), 0))
	#define ASSUME(x) \
		do { \
			if (!(x)) \
				__builtin_unreachable(); \
		} while (0)
#elif defined(JSONIFIER_MSVC)
	#include <intrin.h>
	#define LIKELY(x) (x)
	#define UNLIKELY(x) (x)
	#define ASSUME(x) __assume(x)
#else
	#define LIKELY(x) (x)
	#define UNLIKELY(x) (x)
	#define ASSUME(x) (( void )0)
#endif

#if defined(JSONIFIER_MSVC)
	#define JSONIFIER_NO_INLINE __declspec(noinline)
	#define JSONIFIER_INLINE inline
#elif defined(JSONIFIER_CLANG)
	#define JSONIFIER_NO_INLINE __attribute__((__noinline__))
	#define JSONIFIER_INLINE inline __attribute__((always_inline))
#elif defined(JSONIFIER_GNUCXX)
	#define JSONIFIER_NO_INLINE __attribute__((noinline))
	#define JSONIFIER_INLINE inline
#else
	#define JSONIFIER_INLINE inline
	#define JSONIFIER_NO_INLINE
#endif

#if !defined(JSONIFIER_CPU_INSTRUCTIONS)
	#define JSONIFIER_CPU_INSTRUCTIONS 0
#endif

#if !defined JSONIFIER_ALIGN
	#define JSONIFIER_ALIGN alignas(bytesPerStep)
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

#if defined(JSONIFIER_MSVC)
	#define JSONIFIER_VISUAL_STUDIO 1
	#if defined(JSONIFIER_CLANG)
		#define JSONIFIER_CLANG_VISUAL_STUDIO 1
	#else
		#define JSONIFIER_REGULAR_VISUAL_STUDIO 1
	#endif
#endif

#if JSONIFIER_CHECK_FOR_INSTRUCTION(JSONIFIER_ANY)

	#include <immintrin.h>

#endif

constexpr int mmShuffle(int fp3, int fp2, int fp1, int fp0) {
	return ((fp3 & 0x3) << 6) | ((fp2 & 0x3) << 4) | ((fp1 & 0x3) << 2) | (fp0 & 0x3);
}

#include <jsonifier/ISA/CTimeSimdTypes.hpp>

#if JSONIFIER_CHECK_FOR_INSTRUCTION(JSONIFIER_ANY_AVX)

using simd_int_128 = __m128i;
using simd_int_256 = __m256i;
using simd_int_512 = __m512i;

	#if JSONIFIER_CHECK_FOR_INSTRUCTION(JSONIFIER_AVX512)
using simd_int_t = __m512i;
constexpr uint64_t bitsPerStep{ 512 };
using string_parsing_type = uint64_t;
using simd_fb_type		  = jsonifier_internal::__m512x;
	   #elif JSONIFIER_CHECK_FOR_INSTRUCTION(JSONIFIER_AVX2)
using simd_int_t = __m256i;
constexpr uint64_t bitsPerStep{ 256 };
using string_parsing_type = uint32_t;
using simd_fb_type		  = jsonifier_internal::__m256x;
	   #elif JSONIFIER_CHECK_FOR_INSTRUCTION(JSONIFIER_AVX)
using simd_int_t = __m128i;
constexpr uint64_t bitsPerStep{ 128 };
using string_parsing_type = uint16_t;
using simd_fb_type		  = jsonifier_internal::__m128x;
	   #endif
#elif JSONIFIER_CHECK_FOR_INSTRUCTION(JSONIFIER_NEON)

	#include <arm_neon.h>

using simd_int_128 = uint8x16_t;
using simd_int_256 = uint32_t;
using simd_int_512 = uint64_t;

using simd_int_t = uint8x16_t;
constexpr uint64_t bitsPerStep{ 128 };
using string_parsing_type = uint16_t;
using simd_fb_type		  = jsonifier_internal::__m128x;
#else
using simd_int_128 = jsonifier_internal::__m128x;
using simd_int_256 = uint32_t;
using simd_int_512 = uint64_t;

using simd_int_t = jsonifier_internal::__m128x;
constexpr uint64_t bitsPerStep{ 128 };
using string_parsing_type = uint16_t;
using simd_fb_type		  = jsonifier_internal::__m128x;
#endif

constexpr uint64_t bytesPerStep{ bitsPerStep / 8 };
constexpr uint64_t sixtyFourBitsPerStep{ bitsPerStep / 64 };
constexpr uint64_t stridesPerStep{ bitsPerStep / bytesPerStep };

using string_view_ptr	= const char*;
using structural_index	= const char*;
using string_buffer_ptr = char*;

#if defined(__APPLE__) && defined(__arm64__)
	#define PREFETCH(ptr) __builtin_prefetch(ptr, 0, 0);
#elif defined(JSONIFIER_MSVC)
	#include <intrin.h>
	#define PREFETCH(ptr) _mm_prefetch(static_cast<const char*>(ptr), _MM_HINT_T0);
#elif defined(JSONIFIER_GNUCXX) || defined(JSONIFIER_CLANG)
	#include <xmmintrin.h>
	#define PREFETCH(ptr) _mm_prefetch(static_cast<const char*>(ptr), _MM_HINT_T0);
#else
	#error "Compiler or architecture not supported for prefetching"
#endif

JSONIFIER_INLINE void prefetchInternal(const void* ptr) {
	PREFETCH(ptr)
}
