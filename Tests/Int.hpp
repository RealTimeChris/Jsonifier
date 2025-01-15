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
#pragma once

#include "Common.hpp"

#include <jsonifier/Index.hpp>
#include <filesystem>
#include <fstream>

namespace int_validation_tests {

	constexpr std::array<std::string_view, 24> stringViews01{ "0", "1", "-1", "42", "-42", "123456789", "-123456789", "2147483647", "-2147483648", "9223372036854775807",
		"-9223372036854775808", "0.0", "1.5", "-1.5", "3.14159", "-2.71828", "123.456", "-789.012", "1e5", "-2e3", "3.14e10", "-4.2e-1", "5E2", "10000000e-7" };

	constexpr std::array<int64_t, 24> int64Values{ 0, 1, -1, 42, -42, 123456789, -123456789, 2147483647, -2147483648, 9223372036854775807LL, -9223372036854775808LL, 0, 1, -1, 3,
		-2, 123, -789, 100000, -2000, 31400000000LL, 0, 500, 1 };

	constexpr std::array<std::string_view, 11> stringViews02{ "9223372036854775808", "-9223372036854775809", "-", "1.2.3", "1e", "1e+", "1e-", "\"abc\"", "true", "null", "{}" };

	bool intTests() noexcept {
		jsonifier::jsonifier_core parser{};
		std::cout << "Int Tests: " << std::endl;
		for (size_t x = 0; x < std::size(stringViews01); ++x) {
			runTest<true>("Int Pass Test #" + std::to_string(x), stringViews01[x], int64Values[x], parser);
		}
		for (size_t x = 0; x < std::size(stringViews02); ++x) {
			runTest<false>("Int Fail Test #" + std::to_string(x), stringViews02[x], 0, parser);
		}
		return true;
	}

}