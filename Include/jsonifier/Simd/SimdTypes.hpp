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

#include <jsonifier/Simd/JsonifierCPUInstructions.hpp>
#include <jsonifier/Simd/CTimeSimdTypes.hpp>

#if JSONIFIER_CHECK_FOR_INSTRUCTION(JSONIFIER_ANY_AVX)

	#include <immintrin.h>

namespace jsonifier {

	using jsonifier_simd_int_128 = __m128i;
	using jsonifier_simd_int_256 = __m256i;
	using jsonifier_simd_int_512 = __m512i;

	#if JSONIFIER_CHECK_FOR_INSTRUCTION(JSONIFIER_AVX512)
	using jsonifier_simd_int_t			= __m512i;
	using jsonifier_string_parsing_type = size_t;
	inline static constexpr size_t bitsPerStep{ 512 };
	#elif JSONIFIER_CHECK_FOR_INSTRUCTION(JSONIFIER_AVX2)
	using jsonifier_simd_int_t			= __m256i;
	using jsonifier_string_parsing_type = uint32_t;
	inline static constexpr size_t bitsPerStep{ 256 };
	#elif JSONIFIER_CHECK_FOR_INSTRUCTION(JSONIFIER_AVX)
	using jsonifier_simd_int_t			= __m128i;
	using jsonifier_string_parsing_type = uint16_t;
	inline static constexpr size_t bitsPerStep{ 128 };
	#endif

#elif JSONIFIER_CHECK_FOR_INSTRUCTION(JSONIFIER_NEON)

	#include <arm_neon.h>

namespace jsonifier {

	using jsonifier_simd_int_128 = uint8x16_t;
	using jsonifier_simd_int_256 = uint32_t;
	using jsonifier_simd_int_512 = size_t;

	using jsonifier_simd_int_t			= uint8x16_t;
	using jsonifier_string_parsing_type = uint16_t;
	inline static constexpr size_t bitsPerStep{ 128 };

#else

namespace jsonifier {

	using jsonifier_simd_int_128 = jsonifier::simd::__m128x;
	using jsonifier_simd_int_256 = uint32_t;
	using jsonifier_simd_int_512 = size_t;

	using jsonifier_simd_int_t			= jsonifier::simd::__m128x;
	using jsonifier_string_parsing_type = uint16_t;
	inline constexpr size_t bitsPerStep{ 128 };

#endif

	inline constexpr size_t bytesPerStep{ bitsPerStep / 8 };
	inline constexpr size_t sixtyFourBitsPerStep{ bitsPerStep / 64 };
	inline constexpr size_t stridesPerStep{ bitsPerStep / bytesPerStep };

	inline static constexpr uint16_t packValues2(const char* values) {
		return static_cast<uint16_t>(static_cast<uint16_t>(values[0]) | static_cast<uint16_t>(values[1]) << 8);
	}

	inline static constexpr uint32_t packValues3(const char* values) {
		return static_cast<uint32_t>(static_cast<uint32_t>(values[0]) | static_cast<uint32_t>(values[1]) << 8 | static_cast<uint32_t>(values[2]) << 16);
	}

	inline static constexpr uint32_t packValues4(const char* values) {
		return static_cast<uint32_t>(
			static_cast<uint32_t>(values[0]) | static_cast<uint32_t>(values[1]) << 8 | static_cast<uint32_t>(values[2]) << 16 | static_cast<uint32_t>(values[3]) << 24);
	}

	inline static constexpr uint64_t packValues5(const char* values) {
		return static_cast<uint64_t>(static_cast<uint64_t>(values[0]) | static_cast<uint64_t>(values[1]) << 8 | static_cast<uint64_t>(values[2]) << 16 |
			static_cast<uint64_t>(values[3]) << 24 | static_cast<uint64_t>(values[4]) << 32);
	}

	JSONIFIER_ALIGN(2) inline constexpr char backslash{ '\\' };
	JSONIFIER_ALIGN(2) inline constexpr char newline{ '\n' };
	JSONIFIER_ALIGN(2) inline constexpr char lBracket{ '[' };
	JSONIFIER_ALIGN(2) inline constexpr char rBracket{ ']' };
	JSONIFIER_ALIGN(2) inline constexpr char lBrace{ '{' };
	JSONIFIER_ALIGN(2) inline constexpr char rBrace{ '}' };
	JSONIFIER_ALIGN(2) inline constexpr char colon{ ':' };
	JSONIFIER_ALIGN(2) inline constexpr char comma{ ',' };
	JSONIFIER_ALIGN(2) inline constexpr char quote{ '"' };
	JSONIFIER_ALIGN(2) inline constexpr char n{ 'n' };

	using string_view_ptr	= const char*;
	using structural_index	= string_view_ptr;
	using string_buffer_ptr = char*;

	template<typename value_type>
	concept simd_int_512_type = std::is_same_v<jsonifier_simd_int_512, jsonifier::internal::remove_cvref_t<value_type>>;
	template<typename value_type>
	concept simd_int_256_type = std::is_same_v<jsonifier_simd_int_256, jsonifier::internal::remove_cvref_t<value_type>>;
	template<typename value_type>
	concept simd_int_128_type = std::is_same_v<jsonifier_simd_int_128, jsonifier::internal::remove_cvref_t<value_type>>;
	template<typename value_type>
	concept simd_int_type = std::is_same_v<jsonifier_simd_int_t, jsonifier::internal::remove_cvref_t<value_type>>;

	JSONIFIER_INLINE static void jsonifierPrefetchImpl(const void* ptr) noexcept {
#if JSONIFIER_PLATFORM_MAC && defined(__arm64__)
		__builtin_prefetch(ptr, 0, 0);
#elif JSONIFIER_COMPILER_MSVC || JSONIFIER_COMPILER_GCC || JSONIFIER_COMPILER_CLANG
		_mm_prefetch(static_cast<string_view_ptr>(ptr), _MM_HINT_T0);
#else
	#error "Compiler or architecture not supported for prefetching"
#endif
	}
}