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

	constexpr jsonifier::internal::array parseStringViews01{ "0", "1", "42", "123456789", "2147483647", "18446744073709551615", "0.0", "1.5", "3.14159", "123.456", "1e5", "2e3",
		"3.14e10", "4.2e-1", "5E2", "10000000e-7", "5", "99", "123", "999", "1234", "9999", "12345", "99999", "123456", "999999", "1234567", "9999999", "12345678", "99999999",
		"123456789", "999999999", "1234567890", "9999999999", "12345678901", "99999999999", "123456789012", "999999999999", "1234567890123", "9999999999999", "12345678901234",
		"99999999999999", "123456789012345", "999999999999999", "1234567890123456", "9999999999999999", "12345678901234567", "99999999999999999", "123456789012345678",
		"999999999999999999", "1234567890123456789", "9999999999999999999", "12345678901234567890", "18446744073709551615" };

	constexpr jsonifier::internal::array serializeStringViews01{ "0", "1", "42", "123456789", "2147483647", "18446744073709551615", "0", "1", "3", "123", "100000", "2000",
		"31400000000", "0", "500", "1", "5", "99", "123", "999", "1234", "9999", "12345", "99999", "123456", "999999", "1234567", "9999999", "12345678", "99999999", "123456789",
		"999999999", "1234567890", "9999999999", "12345678901", "99999999999", "123456789012", "999999999999", "1234567890123", "9999999999999", "12345678901234", "99999999999999",
		"123456789012345", "999999999999999", "1234567890123456", "9999999999999999", "12345678901234567", "99999999999999999", "123456789012345678", "999999999999999999",
		"1234567890123456789", "9999999999999999999", "12345678901234567890", "18446744073709551615" };

	constexpr jsonifier::internal::array uint64Values{ 0ULL, 1ULL, 42ULL, 123456789ULL, 2147483647ULL, 18446744073709551615ULL, 0ULL, 1ULL, 3ULL, 123ULL, 100000ULL, 2000ULL,
		31400000000ULL, 0ULL, 500ULL, 1ULL, 5ULL, 99ULL, 123ULL, 999ULL, 1234ULL, 9999ULL, 12345ULL, 99999ULL, 123456ULL, 999999ULL, 1234567ULL, 9999999ULL, 12345678ULL,
		99999999ULL, 123456789ULL, 999999999ULL, 1234567890ULL, 9999999999ULL, 12345678901ULL, 99999999999ULL, 123456789012ULL, 999999999999ULL, 1234567890123ULL, 9999999999999ULL,
		12345678901234ULL, 99999999999999ULL, 123456789012345ULL, 999999999999999ULL, 1234567890123456ULL, 9999999999999999ULL, 12345678901234567ULL, 99999999999999999ULL,
		123456789012345678ULL, 999999999999999999ULL, 1234567890123456789ULL, 9999999999999999999ULL, 12345678901234567890ULL, 18446744073709551615ULL };

	constexpr jsonifier::internal::array<std::string_view, 11> stringViews02{ "18446744073709551616", "-9223372036854775809", "-", "1.2.3", "1e", "1e+", "1e-", "\"abc\"", "true",
		"null", "{}" };

	bool uintTests() noexcept {
		jsonifier::jsonifier_core parser{};
		std::cout << "Uint Tests: " << std::endl;
		for (size_t x = 0; x < std::size(parseStringViews01); ++x) {
			runTestParse<true>("Uint Pass Test #" + std::to_string(x), parseStringViews01[x], uint64Values[x], parser);
			runTestSerialize<true>("Uint Pass Test #" + std::to_string(x), uint64Values[x], serializeStringViews01[x], parser);
		}
		for (size_t x = 0; x < std::size(stringViews02); ++x) {
			//runTest<false>("Uint Fail Test #" + std::to_string(x), stringViews02[x], 0, parser);
		}
		return true;
	}

}