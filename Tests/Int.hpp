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
#include "Common.hpp"

#include <jsonifier/Index.hpp>
#include <filesystem>
#include <fstream>

namespace int_validation_tests {

	std::unordered_map<std::string_view, int64_t> passTestValues = { { "passTest01.json", 0 }, { "passTest02.json", 12345 }, { "passTest03.json", 9223372036854775807 },
		{ "passTest04.json", -9223372036854775808 }, { "passTest05.json", 123 }, { "passTest06.json", 1234567890123456789 }, { "passTest07.json", 999999999999999999 }, { "passTest08.json", -1000000000000000 },
		{ "passTest09.json", 456789123456789 }, { "passTest10.json", -5000 } };

	std::unordered_map<std::string_view, std::string_view> failTestValues = { { "failTest01.json", "18446744073709551616" }, { "failTest02.json", "-9223372036854775809" },
		{ "failTest03.json", "0123" }, { "failTest04.json", "\"12345\"" }, { "failTest05.json", "null" }, { "failTest06.json", "256.0" }, { "failTest07.json", "12a34" }, { "failTest08.json", "12345," },
		{ "failTest09.json", "\"NotANumber\"" }, { "failTest10.json", "NaN" } };

	auto runTest(const std::string_view testName, std::string& dataToParse, jsonifier::jsonifier_core<>& parser, bool passTest = false) noexcept {
		std::cout << testName << " Input: " << dataToParse << std::endl;
		std::vector<int64_t> data;
		auto result = parser.parseJson(data, dataToParse.data());
		if (result && parser.getErrors().size() == 0 && passTest) {
			if (data.size() == 1) {
				std::cout << testName << " Succeeded - Output: " << data[0] << std::endl;
				std::cout << testName << " Succeeded - Expected Output: " << passTestValues[testName] << std::endl;
			} else {
				std::cout << testName << " Failed." << std::endl;
				for (auto& value: parser.getErrors()) {
					std::cout << "Jsonifier Error: " << value << std::endl;
				}
			}
		} else if (!result && parser.getErrors().size() != 0 && !passTest) {
			std::cout << testName << " Succeeded - Expected Output: " << failTestValues[testName] << std::endl;
		} else {
			std::cout << testName << " Failed." << std::endl;
		}
		return true;
	}

	bool intTests() noexcept {
		jsonifier::jsonifier_core parser{};
		std::unordered_map<std::string, test_base> jsonTests{};
		processFilesInFolder(jsonTests, "IntValidation");
		std::cout << "Int Tests: " << std::endl;
		runTest("passTest01.json", jsonTests["passTest01.json"].fileContents, parser, true);
		runTest("passTest02.json", jsonTests["passTest02.json"].fileContents, parser, true);
		runTest("passTest03.json", jsonTests["passTest03.json"].fileContents, parser, true);
		runTest("passTest04.json", jsonTests["passTest04.json"].fileContents, parser, true);
		runTest("passTest05.json", jsonTests["passTest05.json"].fileContents, parser, true);
		runTest("passTest06.json", jsonTests["passTest06.json"].fileContents, parser, true);
		runTest("passTest07.json", jsonTests["passTest07.json"].fileContents, parser, true);
		runTest("passTest08.json", jsonTests["passTest08.json"].fileContents, parser, true);
		runTest("passTest09.json", jsonTests["passTest09.json"].fileContents, parser, true);
		runTest("passTest10.json", jsonTests["passTest10.json"].fileContents, parser, true);
		runTest("failTest01.json", jsonTests["failTest01.json"].fileContents, parser);
		runTest("failTest02.json", jsonTests["failTest02.json"].fileContents, parser);
		runTest("failTest03.json", jsonTests["failTest03.json"].fileContents, parser);
		runTest("failTest04.json", jsonTests["failTest04.json"].fileContents, parser);
		runTest("failTest05.json", jsonTests["failTest05.json"].fileContents, parser);
		runTest("failTest06.json", jsonTests["failTest06.json"].fileContents, parser);
		runTest("failTest07.json", jsonTests["failTest07.json"].fileContents, parser);
		runTest("failTest08.json", jsonTests["failTest08.json"].fileContents, parser);
		runTest("failTest09.json", jsonTests["failTest09.json"].fileContents, parser);
		runTest("failTest10.json", jsonTests["failTest10.json"].fileContents, parser);
		return true;
	}

}