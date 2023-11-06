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

#include <jsonifier/ISADetection/Popcount.hpp>
#include <jsonifier/ISADetection/Lzcount.hpp>
#include <jsonifier/ISADetection/Bmi.hpp>
#include <jsonifier/ISADetection/Bmi2.hpp>
#include <jsonifier/ISADetection/AVX.hpp>
#include <jsonifier/ISADetection/AVX2.hpp>
#include <jsonifier/ISADetection/AVX512.hpp>
#include <jsonifier/ISADetection/Fallback.hpp>

namespace jsonifier_internal {

	inline static void printBits(uint64_t values, const std::string& valuesTitle) {
		std::cout << valuesTitle;
		std::cout << std::bitset<64>{ values };
		std::cout << std::endl;
	}

	template<typename simd_type> inline static const simd_type& printBits(const simd_type& value, const std::string& valuesTitle) noexcept {
		alignas(BytesPerStep) uint8_t values[sizeof(simd_type)]{};
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

	inline static std::string printBits(bool value) noexcept {
		std::stringstream theStream{};
		theStream << std::boolalpha << value << std::endl;
		return theStream.str();
	}

	template<typename simd_type> inline static std::string printBits(const simd_type& value) noexcept {
		alignas(BytesPerStep) uint8_t values[sizeof(simd_type)]{};
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
}