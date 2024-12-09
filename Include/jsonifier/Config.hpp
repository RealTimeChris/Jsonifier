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

#include <cstdint>
#include <atomic>

#if defined(__clang__) || (defined(__GNUC__) && defined(__llvm__))
	#define JSONIFIER_CLANG 1
#elif defined(_MSC_VER)
	#pragma warning(disable : 4820)
	#pragma warning(disable : 4371)
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

#if (defined(__x86_64__) || defined(_M_AMD64)) && !defined(_M_ARM64EC)
	#define JSONIFIER_IS_X86_64 1
#else
	#define JSONIFIER_IS_ARM64 1
#endif

#define JSONIFIER_GCC_VERSION (__GNUC__ * 100 + __GNUC_MINOR__)

#if defined(macintosh) || defined(Macintosh) || (defined(__APPLE__) && defined(__MACH__)) || defined(TARGET_OS_MAC)
	#define JSONIFIER_MAC 1
#elif defined(linux) || defined(__linux) || defined(__linux__) || defined(__gnu_linux__)
	#define JSONIFIER_LINUX 1
#elif defined(WIN32) || defined(_WIN32) || defined(_WIN64)
	#define JSONIFIER_WIN 1
#else
	#error "Undetected platform."
#endif

#if defined(__has_builtin)
	#define JSONIFIER_HAS_BUILTIN(x) __has_builtin(x)
#else
	#define JSONIFIER_HAS_BUILTIN(x) 0
#endif

#if !defined(JSONIFIER_LIKELY)
	#define JSONIFIER_LIKELY(...) (__VA_ARGS__) [[likely]]
#endif

#if !defined(JSONIFIER_UNLIKELY)
	#define JSONIFIER_UNLIKELY(...) (__VA_ARGS__) [[unlikely]]
#endif

#if !defined(JSONIFIER_ELSE_UNLIKELY)
	#define JSONIFIER_ELSE_UNLIKELY(...) __VA_ARGS__ [[unlikely]]
#endif

#if defined(JSONIFIER_GNUCXX) || defined(JSONIFIER_CLANG)
	#define JSONIFIER_ASSUME(x) \
		do { \
			if (!(x)) \
				__builtin_unreachable(); \
		} while (0)
#elif defined(JSONIFIER_MSVC)
	#include <intrin.h>
	#define JSONIFIER_ASSUME(x) __assume(x)
#else
	#define JSONIFIER_ASSUME(x) (( void )0)
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
		#define JSONIFIER_ALWAYS_NO_INLINE [[msvc::noinline]]
		#define JSONIFIER_ALWAYS_INLINE [[msvc::forceinline]] inline
		#define JSONIFIER_NON_GCC_ALWAYS_INLINE [[msvc::forceinline]] inline
		#define JSONIFIER_CLANG_ALWAYS_INLINE inline
		#define JSONIFIER_INLINE inline
	#elif defined(JSONIFIER_CLANG)
		#define JSONIFIER_ALWAYS_NO_INLINE __attribute__((noinline))
		#define JSONIFIER_ALWAYS_INLINE inline __attribute__((always_inline))
		#define JSONIFIER_NON_GCC_ALWAYS_INLINE inline __attribute__((always_inline))
		#define JSONIFIER_CLANG_ALWAYS_INLINE inline __attribute__((always_inline))
		#define JSONIFIER_INLINE inline
	#elif defined(JSONIFIER_GNUCXX)
		#define JSONIFIER_ALWAYS_NO_INLINE __attribute__((noinline))
		#define JSONIFIER_ALWAYS_INLINE inline __attribute__((always_inline))
		#define JSONIFIER_NON_GCC_ALWAYS_INLINE inline
		#define JSONIFIER_CLANG_ALWAYS_INLINE inline
		#define JSONIFIER_INLINE inline
	#endif
#else
	#define JSONIFIER_ALWAYS_NO_INLINE
	#define JSONIFIER_ALWAYS_INLINE inline
	#define JSONIFIER_NON_GCC_ALWAYS_INLINE inline
	#define JSONIFIER_CLANG_ALWAYS_INLINE inline
	#define JSONIFIER_INLINE inline
#endif

#if !defined JSONIFIER_ALIGN
	#define JSONIFIER_ALIGN alignas(bytesPerStep)
#endif
