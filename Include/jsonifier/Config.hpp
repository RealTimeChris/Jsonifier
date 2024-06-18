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

#include <jsonifier/TypeEntities.hpp>
#include <functional>
#include <iostream>
#include <sstream>
#include <cassert>
#include <cstring>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cfloat>
#include <bitset>
#include <atomic>
#include <array>

#if !defined(__GNUC__)
	#pragma warning(disable : 4710)
	#pragma warning(disable : 4711)
	#pragma warning(disable : 4371)
	#pragma warning(disable : 4514)
	#pragma warning(disable : 4625)
	#pragma warning(disable : 4706)
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

#if defined(macintosh) || defined(Macintosh) || (defined(__APPLE__) && defined(__MACH__))
	#define JSONIFIER_MAC 1
#elif defined(linux) || defined(__linux) || defined(__linux__) || defined(__gnu_linux__)
	#define JSONIFIER_LINUX 1
#elif defined(WIN32) || defined(_WIN32) || defined(_WIN64)
	#define JSONIFIER_WIN 1
#endif

#if defined(__clang__) && defined(NDEBUG) && !defined(JSONIFIER_INLINE)
	#define JSONIFIER_INLINE inline __attribute__((always_inline))
#elif !defined(JSONIFIER_INLINE)
	#define JSONIFIER_INLINE inline
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

union __m128x;

#if JSONIFIER_CHECK_FOR_INSTRUCTION(JSONIFIER_ANY_AVX)

using simd_int_128 = __m128i;
using simd_int_256 = __m256i;
using simd_int_512 = __m512i;

	#if JSONIFIER_CHECK_FOR_INSTRUCTION(JSONIFIER_AVX512)
using simd_int_t = __m512i;
constexpr uint64_t bitsPerStep{ 512 };
using string_parsing_type = uint64_t;
	#elif JSONIFIER_CHECK_FOR_INSTRUCTION(JSONIFIER_AVX2)
using simd_int_t = __m256i;
constexpr uint64_t bitsPerStep{ 256 };
using string_parsing_type = uint32_t;
	#elif JSONIFIER_CHECK_FOR_INSTRUCTION(JSONIFIER_AVX)
using simd_int_t = __m128i;
constexpr uint64_t bitsPerStep{ 128 };
using string_parsing_type = uint16_t;
	#endif
#elif JSONIFIER_CHECK_FOR_INSTRUCTION(JSONIFIER_NEON)

	#include <arm_neon.h>

using simd_int_128 = uint8x16_t;
using simd_int_256 = uint32_t;
using simd_int_512 = uint64_t;

using simd_int_t = uint8x16_t;
constexpr uint64_t bitsPerStep{ 128 };
using string_parsing_type = uint16_t;
#else
union __m128x {
	#if JSONIFIER_WIN
	int8_t m128x_int8[16]{};
	int16_t m128x_int16[8];
	int32_t m128x_int32[4];
	int64_t m128x_int64[2];
	uint8_t m128x_uint8[16];
	int16_t m128x_uint16[8];
	int32_t m128x_uint32[4];
	uint64_t m128x_uint64[2];
	#else
	int64_t m128x_int64[2];
	int32_t m128x_int32[4];
	int16_t m128x_int16[8];
	int8_t m128x_int8[16]{};
	uint64_t m128x_uint64[2];
	int32_t m128x_uint32[4];
	int16_t m128x_uint16[8];
	uint8_t m128x_uint8[16];
	#endif
};
using simd_int_128 = __m128x;
using simd_int_256 = uint32_t;
using simd_int_512 = uint64_t;

using simd_int_t = __m128x;
constexpr uint64_t bitsPerStep{ 128 };
using string_parsing_type = uint16_t;
#endif

constexpr uint64_t bytesPerStep{ bitsPerStep / 8 };
constexpr uint64_t sixtyFourBitsPerStep{ bitsPerStep / 64 };
constexpr uint64_t stridesPerStep{ bitsPerStep / bytesPerStep };

using string_view_ptr	= const char*;
using structural_index	= const char*;
using string_buffer_ptr = char*;

template<typename value_type>
concept simd_int_512_type = std::is_same_v<simd_int_512, jsonifier::concepts::unwrap_t<value_type>>;
template<typename value_type>
concept simd_int_256_type = std::is_same_v<simd_int_256, jsonifier::concepts::unwrap_t<value_type>>;
template<typename value_type>
concept simd_int_128_type = std::is_same_v<simd_int_128, jsonifier::concepts::unwrap_t<value_type>>;
template<typename value_type>
concept simd_int_type = std::is_same_v<simd_int_t, jsonifier::concepts::unwrap_t<value_type>>;

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

#include <jsonifier/ISA/AVX.hpp>
#include <jsonifier/ISA/Lzcount.hpp>
#include <jsonifier/ISA/Popcount.hpp>
#include <jsonifier/ISA/Bmi2.hpp>
#include <jsonifier/ISA/Bmi.hpp>
#include <jsonifier/ISA/CollectIndices.hpp>
#include <jsonifier/ISA/CompareValues.hpp>
#include <jsonifier/ISA/Fallback.hpp>

namespace jsonifier_internal {

	template<typename value_type01, typename value_type02> constexpr value_type01 max(value_type01 value1, value_type02 value2) {
		return value1 > static_cast<value_type01>(value2) ? value1 : static_cast<value_type01>(value2);
	}

	template<jsonifier::concepts::unsigned_type value_type> void printBits(value_type values, const std::string& valuesTitle) {
		std::cout << valuesTitle;
		std::cout << std::bitset<sizeof(value_type) * 8>{ values };
		std::cout << std::endl;
	}

	template<simd_int_type simd_type> const simd_type& printBits(const simd_type& value, const std::string& valuesTitle) noexcept {
		JSONIFIER_ALIGN uint8_t values[sizeof(simd_type)]{};
		std::stringstream theStream{};
		store(value, values);
		std::cout << valuesTitle;
		for (string_parsing_type x = 0; x < sizeof(simd_type); ++x) {
			for (string_parsing_type y = 0; y < 8; ++y) {
				std::cout << std::bitset<1>{ static_cast<uint64_t>(*(values + x)) >> y };
			}
		}
		std::cout << std::endl;
		return value;
	}

	JSONIFIER_INLINE std::string printBits(bool value) noexcept {
		std::stringstream theStream{};
		theStream << std::boolalpha << value << std::endl;
		return theStream.str();
	}

	template<typename simd_type> JSONIFIER_INLINE std::string printBits(const simd_type& value) noexcept {
		JSONIFIER_ALIGN uint8_t values[sizeof(simd_type)]{};
		std::stringstream theStream{};
		store(value, values);
		for (uint64_t x = 0; x < bytesPerStep; ++x) {
			for (uint64_t y = 0; y < 8; ++y) {
				theStream << std::bitset<1>{ static_cast<uint64_t>(*(values + x)) >> y };
			}
		}
		theStream << std::endl;
		return theStream.str();
	}

	template<jsonifier::concepts::time_type value_type> class stop_watch {
	  public:
		using hr_clock = std::chrono::high_resolution_clock;

		JSONIFIER_INLINE stop_watch(uint64_t newTime) {
			totalNumberOfTimeUnits.store(value_type{ newTime }, std::memory_order_release);
		}

		JSONIFIER_INLINE stop_watch(value_type newTime) {
			totalNumberOfTimeUnits.store(newTime, std::memory_order_release);
		}

		JSONIFIER_INLINE stop_watch& operator=(stop_watch&& other) {
			if (this != &other) [[likely]] {
				totalNumberOfTimeUnits.store(other.totalNumberOfTimeUnits.load(std::memory_order_acquire), std::memory_order_release);
				startTimeInTimeUnits.store(other.startTimeInTimeUnits.load(std::memory_order_acquire), std::memory_order_release);
			}
			return *this;
		}

		JSONIFIER_INLINE stop_watch(stop_watch&& other) {
			*this = std::move(other);
		}

		JSONIFIER_INLINE stop_watch& operator=(const stop_watch& other) {
			if (this != &other) [[likely]] {
				totalNumberOfTimeUnits.store(other.totalNumberOfTimeUnits.load(std::memory_order_acquire), std::memory_order_release);
				startTimeInTimeUnits.store(other.startTimeInTimeUnits.load(std::memory_order_acquire), std::memory_order_release);
			}
			return *this;
		}

		JSONIFIER_INLINE stop_watch(const stop_watch& other) {
			*this = other;
		}

		JSONIFIER_INLINE bool hasTimeElapsed() {
			if (std::chrono::duration_cast<value_type>(hr_clock::now().time_since_epoch()) - startTimeInTimeUnits.load(std::memory_order_acquire) >=
				totalNumberOfTimeUnits.load(std::memory_order_acquire)) [[likely]] {
				return true;
			} else {
				return false;
			}
		}

		JSONIFIER_INLINE void reset(value_type newTimeValue = value_type{}) {
			if (newTimeValue != value_type{}) [[likely]] {
				totalNumberOfTimeUnits.store(newTimeValue, std::memory_order_release);
				startTimeInTimeUnits.store(std::chrono::duration_cast<value_type>(hr_clock::now().time_since_epoch()), std::memory_order_release);
			} else {
				startTimeInTimeUnits.store(std::chrono::duration_cast<value_type>(hr_clock::now().time_since_epoch()), std::memory_order_release);
			}
		}

		JSONIFIER_INLINE value_type getTotalWaitTime() const {
			return totalNumberOfTimeUnits.load(std::memory_order_acquire);
		}

		JSONIFIER_INLINE value_type totalTimeElapsed() {
			return std::chrono::duration_cast<value_type>(hr_clock::now().time_since_epoch()) - startTimeInTimeUnits.load(std::memory_order_acquire);
		}

	  protected:
		std::atomic<value_type> totalNumberOfTimeUnits{};
		std::atomic<value_type> startTimeInTimeUnits{};
	};

	template<jsonifier::concepts::time_type value_type> stop_watch(value_type) -> stop_watch<value_type>;
}