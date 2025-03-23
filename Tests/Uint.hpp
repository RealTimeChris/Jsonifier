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

namespace uint_validation_tests {

	constexpr jsonifier::internal::array<std::string_view, 16> stringViews01{ "0", "1", "42", "123456789", "2147483647", "18446744073709551615", "0.0", "1.5", "3.14159", "123.456",
		"1e5", "2e3", "3.14e10", "4.2e-1", "5E2", "10000000e-7" };

	constexpr jsonifier::internal::array<uint64_t, 16> uint64Values{ 0, 1, 42, 123456789, 2147483647, 18446744073709551615ULL, 0, 1, 3, 123, 100000, 2000, 31400000000ULL, 0, 500,
		1 };

	constexpr jsonifier::internal::array<std::string_view, 11> stringViews02{ "18446744073709551616", "-9223372036854775809", "-", "1.2.3", "1e", "1e+", "1e-", "\"abc\"", "true",
		"null", "{}" };

	void uintTests() {
		pass_test_runner<"Uint Pass Tests: ", stringViews01, uint64Values, pass_tests_runner, std::make_integer_sequence<uint64_t, stringViews01.size()>>::impl();
		fail_test_runner<"Uint Fail Tests: ", stringViews02, uint64_t{ 0ULL }, fail_tests_runner, std::make_integer_sequence<uint64_t, stringViews02.size()>>::impl();
		return;
	}

}