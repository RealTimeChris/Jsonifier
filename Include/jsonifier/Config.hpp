/*
	MIT License

	Copyright (c) 2024 RealTimeChris

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

#include <jsonifier/ISA/JsonifierCPUInstructions.hpp>
#include <cstdint>
#include <atomic>

#if defined(__clang__) || (defined(__GNUC__) && defined(__llvm__))
	#define JSONIFIER_CLANG 1
#elif defined(_MSC_VER)
	#pragma warning(disable : 4820)
	#pragma warning(disable : 4371)
	#pragma warning(disable : 4324)
	#define JSONIFIER_MSVC 1
#elif defined(__GNUC__) && !defined(__clang__)
	#define JSONIFIER_GNUCXX 1
#endif

#if defined(JSONIFIER_MSVC)
	#define JSONIFIER_VISUAL_STUDIO 1
	#if defined(JSONIFIER_CLANG)
		#define JSONIFIER_CLANG_VISUAL_STUDIO 1
	#else
		#define JSONIFIER_REGULAR_VISUAL_STUDIO 1
	#endif
#endif

#define JSONIFIER_GCC_VERSION (__GNUC__ * 100 + __GNUC_MINOR__)

#if defined(macintosh) || defined(Macintosh) || (defined(__APPLE__) && defined(__MACH__))
	#define JSONIFIER_MAC 1
#elif defined(linux) || defined(__linux) || defined(__linux__) || defined(__gnu_linux__)
	#define JSONIFIER_LINUX 1
#elif defined(WIN32) || defined(_WIN32) || defined(_WIN64)
	#define JSONIFIER_WIN 1
#endif

#if defined(__has_builtin)
	#define JSONIFIER_HAS_BUILTIN(x) __has_builtin(x)
#else
	#define JSONIFIER_HAS_BUILTIN(x) 0
#endif

#if !defined(JSONIFIER_LIKELY)
	#define JSONIFIER_LIKELY(...) __VA_ARGS__ [[likely]]
#endif

#if !defined(JSONIFIER_UNLIKELY)
	#define JSONIFIER_UNLIKELY(...) __VA_ARGS__ [[unlikely]]
#endif

#if defined(__cpp_inline_variables) && __cpp_inline_variables >= 201606L
	#define JSONIFIER_HAS_INLINE_VARIABLE 1
#elif __cplusplus >= 201703L
	#define JSONIFIER_HAS_INLINE_VARIABLE 1
#elif defined(JSONIFIER_MSVC) && JSONIFIER_MSVC >= 1912 && _MSVC_LANG >= 201703L
	#define JSONIFIER_HAS_INLINE_VARIABLE 1
#else
	#define JSONIFIER_HAS_INLINE_VARIABLE 0
#endif

#if JSONIFIER_HAS_INLINE_VARIABLE
	#define JSONIFIER_ALWAYS_INLINE_VARIABLE inline constexpr
#else
	#define JSONIFIER_ALWAYS_INLINE_VARIABLE static constexpr
#endif

#if defined(NDEBUG)
	#if defined(JSONIFIER_MSVC)
		#define JSONIFIER_NO_INLINE __declspec(noinline)
		#define JSONIFIER_FLATTEN inline [[msvc::flatten]]
		#define JSONIFIER_ALWAYS_INLINE [[msvc::forceinline]] inline
		#define JSONIFIER_MAYBE_ALWAYS_INLINE [[msvc::forceinline]] inline
		#define JSONIFIER_INLINE inline
	#elif defined(JSONIFIER_CLANG)
		#define JSONIFIER_NO_INLINE __attribute__((__noinline__))
		#define JSONIFIER_FLATTEN inline __attribute__((flatten))
		#define JSONIFIER_ALWAYS_INLINE inline __attribute__((always_inline))
		#define JSONIFIER_MAYBE_ALWAYS_INLINE inline __attribute__((always_inline))
		#define JSONIFIER_INLINE inline
	#elif defined(JSONIFIER_GNUCXX)
		#define JSONIFIER_NO_INLINE __attribute__((noinline))
		#define JSONIFIER_FLATTEN inline __attribute__((flatten))
		#define JSONIFIER_MAYBE_ALWAYS_INLINE inline
		#define JSONIFIER_ALWAYS_INLINE inline __attribute__((always_inline))
		#define JSONIFIER_INLINE inline
	#endif
#else
	#define JSONIFIER_NO_INLINE
	#define JSONIFIER_FLATTEN
	#define JSONIFIER_ALWAYS_INLINE inline
	#define JSONIFIER_MAYBE_ALWAYS_INLINE inline
	#define JSONIFIER_INLINE inline
#endif

#if !defined JSONIFIER_ALIGN
	#define JSONIFIER_ALIGN alignas(bytesPerStep)
#endif

#if defined(JSONIFIER_ANY)
	#include <immintrin.h>

#endif

JSONIFIER_ALWAYS_INLINE constexpr int32_t mmShuffle(int32_t fp3, int32_t fp2, int32_t fp1, int32_t fp0) noexcept {
	return ((fp3 & 0x3) << 6) | ((fp2 & 0x3) << 4) | ((fp1 & 0x3) << 2) | (fp0 & 0x3);
}

#include <jsonifier/ISA/CTimeSimdTypes.hpp>

#if defined(JSONIFIER_ANY_AVX)

using jsonifier_simd_int_128 = __m128i;
using jsonifier_simd_int_256 = __m256i;
using jsonifier_simd_int_512 = __m512i;

	#if defined(JSONIFIER_AVX512)
using jsonifier_simd_int_t = __m512i;
static constexpr size_t bitsPerStep{ 512 };
using jsonifier_string_parsing_type = size_t;
using jsonifier_simd_fb_type		= jsonifier_internal::__m512x;
	   #elif defined(JSONIFIER_AVX2)
using jsonifier_simd_int_t = __m256i;
static constexpr size_t bitsPerStep{ 256 };
using jsonifier_string_parsing_type = uint32_t;
using jsonifier_simd_fb_type		= jsonifier_internal::__m256x;
	   #elif defined(JSONIFIER_AVX)
using jsonifier_simd_int_t = __m128i;
static constexpr size_t bitsPerStep{ 128 };
using jsonifier_string_parsing_type = uint16_t;
using jsonifier_simd_fb_type		= jsonifier_internal::__m128x;
	   #endif
#elif defined(JSONIFIER_NEON)

	#include <arm_neon.h>

using jsonifier_simd_int_128 = uint8x16_t;
using jsonifier_simd_int_256 = uint32_t;
using jsonifier_simd_int_512 = size_t;

using jsonifier_simd_int_t = uint8x16_t;
static constexpr size_t bitsPerStep{ 128 };
using jsonifier_string_parsing_type = uint16_t;
using jsonifier_simd_fb_type		= jsonifier_internal::__m128x;
#else
using jsonifier_simd_int_128 = jsonifier_internal::__m128x;
using jsonifier_simd_int_256 = uint32_t;
using jsonifier_simd_int_512 = size_t;

using jsonifier_simd_int_t = jsonifier_internal::__m128x;
static constexpr size_t bitsPerStep{ 128 };
using jsonifier_string_parsing_type = uint16_t;
using jsonifier_simd_fb_type		= jsonifier_internal::__m128x;
#endif

static constexpr size_t bytesPerStep{ bitsPerStep / 8 };
static constexpr size_t sixtyFourBitsPerStep{ bitsPerStep / 64 };
static constexpr size_t stridesPerStep{ bitsPerStep / bytesPerStep };

using string_view_ptr	= const char*;
using structural_index	= const char*;
using string_buffer_ptr = char*;

#if defined(__APPLE__) && defined(__arm64__)
	#define JSONIFIER_PREFETCH(ptr) __builtin_prefetch(ptr, 0, 0);
#elif defined(JSONIFIER_MSVC)
	#include <intrin.h>
	#define JSONIFIER_PREFETCH(ptr) _mm_prefetch(static_cast<const char*>(ptr), _MM_HINT_T0);
#elif defined(JSONIFIER_GNUCXX) || defined(JSONIFIER_CLANG)
	#include <xmmintrin.h>
	#define JSONIFIER_PREFETCH(ptr) _mm_prefetch(static_cast<const char*>(ptr), _MM_HINT_T0);
#else
	#error "Compiler or architecture not supported for prefetching"
#endif

JSONIFIER_ALWAYS_INLINE void jsonifierPrefetchImpl(const void* ptr) noexcept {
	JSONIFIER_PREFETCH(ptr)
}

inline std::atomic_uint64_t sectionInstanceCount{};
inline std::atomic_uint64_t coreInstanceCount{};