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

	constexpr std::array<int64_t, 24> expectedInt64Values{ 0, 1, -1, 42, -42, 123456789, -123456789, 2147483647, -2147483648, std::numeric_limits<int64_t>::max(),
		std::numeric_limits<int64_t>::min(), 0, 1, -1, 3, -2, 123, -789, 100000, -2000, 31400000000LL, 0, 500, 1 };

	template<bool passTest = true, typename value_type>
	auto runTest(value_type& expectedValue, const std::string_view& testName, const std::string& dataToParse, jsonifier::jsonifier_core<>& parser) noexcept {
		std::cout << testName << " Input: " << dataToParse.substr(1, dataToParse.size() - 2) << std::endl;
		std::vector<int64_t> data;
		auto result = parser.parseJson(data, dataToParse);
		if constexpr (passTest) {
			if (result && parser.getErrors().size() == 0) {
				if (data.size() == 1 && data[0] == expectedValue) {
					std::cout << testName << " Succeeded - Output: " << data[0] << std::endl;
					std::cout << testName << " Succeeded - Expected Output: " << expectedValue << std::endl;
					return;
				}
			}
			if (data.size() == 1) {
				std::cout << testName << " Failed - Output: " << data[0] << std::endl;
			}
			std::cout << testName << " Failed - Expected Output: " << expectedValue << std::endl;
			for (auto& value: parser.getErrors()) {
				std::cout << "Jsonifier Error: " << value << std::endl;
			}
		} else {
			if (!result) {
				std::cout << testName << " Succeeded - Output: " << data[0] << std::endl;
				std::cout << testName << " Succeeded - Expected Output: " << expectedValue.substr(1, expectedValue.size() - 2) << std::endl;
			} else {
				if (data.size() == 1) {
					std::cout << testName << " Failed - Output: " << data[0] << std::endl;
				}
				std::cout << testName << " Failed - Expected Output: " << expectedValue.substr(1, expectedValue.size() - 2) << std::endl;
				for (auto& value: parser.getErrors()) {
					std::cout << "Jsonifier Error: " << value << std::endl;
				}
			}
		}
		return;
	}

	bool intTests() noexcept {
		std::cout << "Int Tests: " << std::endl;
		std::string filePath01{ testPath };
		filePath01 += "/IntValidation/passTests.json";
		auto file = bnch_swt::file_loader::loadFile(filePath01);
		std::vector<std::string> passTests{};
		jsonifier::jsonifier_core parser{};
		parser.parseJson(passTests, file);
		for (size_t x = 0; x < passTests.size(); ++x) {
			runTest(expectedInt64Values[x], "Integer-Pass-Test " + std::to_string(x + 1), passTests[x], parser);
		}
		std::string filePath02{ testPath };
		filePath02 += "/IntValidation/failTests.json";
		file = bnch_swt::file_loader::loadFile(filePath02);
		std::vector<std::string> failTests{};
		parser.parseJson(failTests, file);
		for (size_t x = 0; x < failTests.size(); ++x) {
			runTest<false>(failTests[x], "Integer-Fail-Test " + std::to_string(x + 1), failTests[x], parser);
		}
		return true;
	}

}