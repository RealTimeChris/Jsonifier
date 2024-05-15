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

#include <jsonifier/ISA/Popcount.hpp>
#include <jsonifier/ISA/Lzcount.hpp>
#include <jsonifier/ISA/Bmi.hpp>
#include <jsonifier/ISA/Bmi2.hpp>
#include <jsonifier/ISA/Neon.hpp>
#include <jsonifier/ISA/AVX.hpp>
#include <jsonifier/ISA/AVX2.hpp>
#include <jsonifier/ISA/AVX512.hpp>
#include <jsonifier/ISA/Fallback.hpp>
#include <jsonifier/ISA/CollectIndices.hpp>
#include <jsonifier/ISA/SimdCommon.hpp>
#include <atomic>

template<jsonifier::concepts::unsigned_type value_type> void simd_internal::printBits(value_type values, const std::string& valuesTitle) {
	std::cout << valuesTitle;
	std::cout << std::bitset<sizeof(value_type) * 8>{ values };
	std::cout << std::endl;
}

template<simd_int_type simd_type> const simd_type& simd_internal::printBits(const simd_type& value, const std::string& valuesTitle) noexcept {
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

namespace jsonifier_internal {

	template<typename value_type01, typename value_type02> constexpr value_type01 max(value_type01 value1, value_type02 value2) {
		return static_cast<value_type01>(value1 > value2 ? value1 : value2);
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
		for (uint64_t x = 0; x < BytesPerStep; ++x) {
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