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

#include <jsonifier-incl/simd/jsonifier_cpu_instructions.hpp>
#include <jsonifier-incl/core/config.hpp>
#include <cstdint>

#if JSONIFIER_CHECK_FOR_INSTRUCTION(JSONIFIER_ANY_AVX)

	#include <immintrin.h>

namespace jsonifier {

	using jsonifier_simd_int_128 = __m128i;
	using jsonifier_simd_int_256 = __m256i;
	using jsonifier_simd_int_512 = __m512i;

	#if JSONIFIER_CHECK_FOR_INSTRUCTION(JSONIFIER_AVX512)
	using jsonifier_simd_int_t			= __m512i;
	using jsonifier_string_parsing_type = uint64_t;
	inline static constexpr uint64_t bitsPerStep{ 512 };
	#elif JSONIFIER_CHECK_FOR_INSTRUCTION(JSONIFIER_AVX2)
	using jsonifier_simd_int_t			= __m256i;
	using jsonifier_string_parsing_type = uint32_t;
	inline static constexpr uint64_t bitsPerStep{ 256 };
	#elif JSONIFIER_CHECK_FOR_INSTRUCTION(JSONIFIER_AVX)
	using jsonifier_simd_int_t			= __m128i;
	using jsonifier_string_parsing_type = uint16_t;
	inline static constexpr uint64_t bitsPerStep{ 128 };
	#endif

#elif JSONIFIER_CHECK_FOR_INSTRUCTION(JSONIFIER_NEON)

	#include <arm_neon.h>

namespace jsonifier {

	using jsonifier_simd_int_128 = uint8x16_t;
	using jsonifier_simd_int_256 = uint32_t;
	using jsonifier_simd_int_512 = uint64_t;

	using jsonifier_simd_int_t			= uint8x16_t;
	using jsonifier_string_parsing_type = uint16_t;
	inline static constexpr uint64_t bitsPerStep{ 128 };

#else

namespace jsonifier {

	using jsonifier_simd_int_128 = jsonifier::simd::simd_x<simd::simd_classes::x_128>;
	using jsonifier_simd_int_256 = uint32_t;
	using jsonifier_simd_int_512 = uint64_t;

	using jsonifier_simd_int_t			= jsonifier::simd::simd_x<simd::simd_classes::x_128>;
	using jsonifier_string_parsing_type = uint16_t;
	inline constexpr uint64_t bitsPerStep{ 128 };

#endif

	inline constexpr uint64_t bytesPerStep{ bitsPerStep / 8 };
	inline constexpr uint64_t stridesPerStep{ bitsPerStep / bytesPerStep };
	inline constexpr uint64_t registersPerSixtyFourBits{ 64 / bytesPerStep };

#if JSONIFIER_ARCH_ARM64
	inline constexpr uint64_t simdBlocksPerStep{ 4 };
#else
	inline constexpr uint64_t simdBlocksPerStep{ bitsPerStep / 64 };
#endif

	template<typename value_type>
	concept simd_int_512_type = sizeof(value_type) == 64;
	template<typename value_type>
	concept simd_int_256_type = sizeof(value_type) == 32;
	template<typename value_type>
	concept simd_int_128_type = sizeof(value_type) == 16;

	struct simd_array {
		using size_type = uint64_t;
		jsonifier_simd_int_t values[registersPerSixtyFourBits]{};

		template<uint64_t index_new> JSONIFIER_INLINE constexpr void assign_value(jsonifier_simd_int_t value) noexcept {
			constexpr uint64_t index{ index_new % registersPerSixtyFourBits };
			values[index] = value;
		}

		template<uint64_t index_new> JSONIFIER_INLINE constexpr jsonifier_simd_int_t get_value() const noexcept {
			constexpr uint64_t index{ index_new % registersPerSixtyFourBits };
			return values[index];
		}
	};

	using string_view_ptr	   = const char*;
	using structural_index_ptr = uint32_t*;
	using string_buffer_ptr	   = char*;

	[[maybe_unused]] JSONIFIER_INLINE static void jsonifierPrefetchImpl(const void* ptr) noexcept {
#if JSONIFIER_PLATFORM_MAC && defined(__arm64__)
		__builtin_prefetch(ptr, 0, 0);
#elif JSONIFIER_COMPILER_MSVC || JSONIFIER_COMPILER_GCC || JSONIFIER_COMPILER_CLANG
		_mm_prefetch(static_cast<string_view_ptr>(ptr), _MM_HINT_T0);
#else
	#error "Compiler or architecture not supported for prefetching"
#endif
	}

#if JSONIFIER_COMPILER_CLANG
	#if JSONIFIER_ARCH_ARM64
	static constexpr uint64_t jsonifier_tape_max = 24;
	static constexpr uint64_t jsonifier_tape_step = 4;
	#else
	static constexpr uint64_t jsonifier_tape_max  = 32;
	static constexpr uint64_t jsonifier_tape_step = 4;
	#endif
#elif JSONIFIER_COMPILER_GCC
	#if JSONIFIER_ARCH_ARM64
	static constexpr uint64_t jsonifier_tape_max = 32;
	static constexpr uint64_t jsonifier_tape_step = 2;
	#else
	static constexpr uint64_t jsonifier_tape_max  = 16;
	static constexpr uint64_t jsonifier_tape_step = 8;
	#endif
#elif JSONIFIER_COMPILER_MSVC
	static constexpr uint64_t jsonifier_tape_max = 24;
	static constexpr uint64_t jsonifier_tape_step = 4;
#else
static constexpr uint64_t jsonifier_tape_max = 24;
static constexpr uint64_t jsonifier_tape_step = 4;
#endif

	static_assert(jsonifier_tape_max % jsonifier_tape_step == 0, "jsonifier_tape_max must be a multiple of jsonifier_tape_step");

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

	inline static constexpr uint16_t packValues2(string_view_ptr values) {
		return static_cast<uint16_t>(static_cast<uint16_t>(values[0]) | static_cast<uint16_t>(values[1]) << 8);
	}

	inline static constexpr uint32_t packValues3(string_view_ptr values) {
		return static_cast<uint32_t>(static_cast<uint32_t>(values[0]) | static_cast<uint32_t>(values[1]) << 8 | static_cast<uint32_t>(values[2]) << 16);
	}

	inline static constexpr uint32_t packValues4(string_view_ptr values) {
		return static_cast<uint32_t>(
			static_cast<uint32_t>(values[0]) | static_cast<uint32_t>(values[1]) << 8 | static_cast<uint32_t>(values[2]) << 16 | static_cast<uint32_t>(values[3]) << 24);
	}

	inline static constexpr uint64_t packValues5(string_view_ptr values) {
		return static_cast<uint64_t>(static_cast<uint64_t>(values[0]) | static_cast<uint64_t>(values[1]) << 8 | static_cast<uint64_t>(values[2]) << 16 |
			static_cast<uint64_t>(values[3]) << 24 | static_cast<uint64_t>(values[4]) << 32);
	}
}
