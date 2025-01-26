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

#include <jsonifier/ISA/JsonifierCPUInstructions.hpp>
#include <jsonifier/ISA/CTimeSimdTypes.hpp>

#if JSONIFIER_CHECK_FOR_INSTRUCTION(JSONIFIER_ANY_AVX)

	#include <immintrin.h>

using jsonifier_simd_int_128 = __m128i;
using jsonifier_simd_int_256 = __m256i;
using jsonifier_simd_int_512 = __m512i;

	#if JSONIFIER_CHECK_FOR_INSTRUCTION(JSONIFIER_AVX512)
using jsonifier_simd_int_t = __m512i;
static constexpr size_t bitsPerStep{ 512 };
using jsonifier_string_parsing_type = size_t;
	#elif JSONIFIER_CHECK_FOR_INSTRUCTION(JSONIFIER_AVX2)
using jsonifier_simd_int_t = __m256i;
static constexpr size_t bitsPerStep{ 256 };
using jsonifier_string_parsing_type = uint32_t;
	#elif JSONIFIER_CHECK_FOR_INSTRUCTION(JSONIFIER_AVX)
using jsonifier_simd_int_t = __m128i;
static constexpr size_t bitsPerStep{ 128 };
using jsonifier_string_parsing_type = uint16_t;
	#endif
#elif JSONIFIER_CHECK_FOR_INSTRUCTION(JSONIFIER_NEON)

	#include <arm_neon.h>

using jsonifier_simd_int_128 = uint8x16_t;
using jsonifier_simd_int_256 = uint32_t;
using jsonifier_simd_int_512 = size_t;

using jsonifier_simd_int_t = uint8x16_t;
static constexpr size_t bitsPerStep{ 128 };
using jsonifier_string_parsing_type = uint16_t;
#else
using jsonifier_simd_int_128 = simd::__m128x;
using jsonifier_simd_int_256 = uint32_t;
using jsonifier_simd_int_512 = size_t;

using jsonifier_simd_int_t = simd::__m128x;
static constexpr size_t bitsPerStep{ 128 };
using jsonifier_string_parsing_type = uint16_t;
#endif

static constexpr size_t bytesPerStep{ bitsPerStep / 8 };
static constexpr size_t sixtyFourBitsPerStep{ bitsPerStep / 64 };
static constexpr size_t stridesPerStep{ bitsPerStep / bytesPerStep };

constexpr uint16_t packValues2(const char* values) {
	return static_cast<uint16_t>(static_cast<uint16_t>(values[0]) | static_cast<uint16_t>(values[1]) << 8);
}

constexpr uint32_t packValues3(const char* values) {
	return static_cast<uint32_t>(static_cast<uint32_t>(values[0]) | static_cast<uint32_t>(values[1]) << 8 | static_cast<uint32_t>(values[2]) << 16);
}

constexpr uint32_t packValues4(const char* values) {
	return static_cast<uint32_t>(
		static_cast<uint32_t>(values[0]) | static_cast<uint32_t>(values[1]) << 8 | static_cast<uint32_t>(values[2]) << 16 | static_cast<uint32_t>(values[3]) << 24);
}

constexpr uint64_t packValues5(const char* values) {
	return static_cast<uint64_t>(static_cast<uint64_t>(values[0]) | static_cast<uint64_t>(values[1]) << 8 | static_cast<uint64_t>(values[2]) << 16 |
		static_cast<uint64_t>(values[3]) << 24 | static_cast<uint64_t>(values[4]) << 32);
}

static constexpr auto falseV{ packValues5("false") };
static constexpr auto trueV{ packValues4("true") };
static constexpr auto nullV{ packValues4("null") };
alignas(2) static constexpr char backslash{ '\\' };
alignas(2) static constexpr char newline{ '\n' };
alignas(2) static constexpr char lBracket{ '[' };
alignas(2) static constexpr char rBracket{ ']' };
alignas(2) static constexpr char lBrace{ '{' };
alignas(2) static constexpr char rBrace{ '}' };
alignas(2) static constexpr char colon{ ':' };
alignas(2) static constexpr char comma{ ',' };
alignas(2) static constexpr char quote{ '"' };
alignas(2) static constexpr char n{ 'n' };

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

void jsonifierPrefetchImpl(const void* ptr) noexcept {
#if defined(JSONIFIER_MAC) && defined(__arm64__)
	__builtin_prefetch(ptr, 0, 0);
#elif defined(JSONIFIER_MSVC) || defined(JSONIFIER_GNUCXX) || defined(JSONIFIER_CLANG)
	_mm_prefetch(static_cast<string_view_ptr>(ptr), _MM_HINT_T0);
#else
	#error "Compiler or architecture not supported for prefetching"
#endif
}