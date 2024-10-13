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

namespace string_validation_tests {

	std::unordered_map<std::string_view, std::string> testValues = { { "test1.json", "" }, { "test2.json", "Hello" }, { "test3.json", "Hello\nWorld" },
		{ "test5.json", "Hello\u0000World" }, { "test6.json", "\"\\/\b\f\n\r\t" }, { "test8.json", "\u0024" }, { "test9.json", "\u00A2" }, { "test10.json", "\u20AC" },
		{ "test11.json", "\U0001D11E" } };

	auto runTest(const std::string_view& testName, const std::string& dataToParse, jsonifier::jsonifier_core<>& parser) noexcept {
		std::cout << testName << " Input: " << dataToParse << std::endl;
		std::vector<std::string> data;
		if (parser.parseJson(data, dataToParse) && parser.getErrors().size() == 0) {
			if (data.size() == 1) {
				std::cout << testName << " Succeeded - Output: " << data[0] << std::endl;
				std::cout << testName << " Succeeded - Expected Output: " << testValues[testName] << std::endl;
			} else {
				std::cout << testName << " Failed." << std::endl;
			}
		} else {
			std::cout << testName << " Failed." << std::endl;
			for (auto& value: parser.getErrors()) {
				std::cout << "Jsonifier Error: " << value << std::endl;
			}
		}
		return true;
	}

	bool stringTests() noexcept {
		jsonifier::jsonifier_core parser{};
		std::unordered_map<std::string, test_base> jsonTests{};
		processFilesInFolder(jsonTests, "StringValidation");
		std::cout << "String Tests: " << std::endl;
		runTest("test1.json", jsonTests["test1.json"].fileContents, parser);
		runTest("test2.json", jsonTests["test2.json"].fileContents, parser);
		runTest("test3.json", jsonTests["test3.json"].fileContents, parser);
		runTest("test5.json", jsonTests["test5.json"].fileContents, parser);
		runTest("test6.json", jsonTests["test6.json"].fileContents, parser);
		runTest("test8.json", jsonTests["test8.json"].fileContents, parser);
		runTest("test9.json", jsonTests["test9.json"].fileContents, parser);
		runTest("test10.json", jsonTests["test10.json"].fileContents, parser);
		runTest("test11.json", jsonTests["test11.json"].fileContents, parser);
		return true;
	}

}
