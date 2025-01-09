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

	constexpr jsonifier::internal::array<std::string_view, 9> stringViews01 = { "\"\"", "\"Hello\"", "\"Hello\\nWorld\"", "\"Hello\u0000World\"", "\"\\\"\\\\/\\b\\f\\n\\r\\t\"",
		"\"\\u0024\"",
		"\"\\u00A2\"", "\"\\u20AC\"", "\"\\uD834\\uDD1E\"" };

	constexpr jsonifier::internal::array<std::string_view, 9> stringViews02 = { "", "Hello", "Hello\nWorld", "Hello\0World", "\"\\/\b\f\n\r\t", "\x24", "\xC2\xA2", "\xE2\x82\xAC",
		"\xF0\x9D\x84\x9E" };

	auto runTest(const std::string_view& testName, const std::string_view& dataToParse, const std::string_view& valueToCompare, jsonifier::jsonifier_core<>& parser) noexcept {
		std::cout << testName << " Input: " << dataToParse << std::endl;
		std::string data{};
		if (parser.parseJson(data, dataToParse) && parser.getErrors().size() == 0) {
			if (data == valueToCompare) {
				std::cout << testName << " Succeeded - Output: " << data << std::endl;
				std::cout << testName << " Succeeded - Expected Output: " << valueToCompare << std::endl;
			} else {
				std::cout << testName << " Failed - Output: " << data << std::endl;
				std::cout << testName << " Failed - Expected Output: " << valueToCompare<< std::endl;
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
		std::cout << "String Tests: " << std::endl;
		for (size_t x = 0; x < std::size(stringViews01); ++x) {
			runTest("String Test #" + std::to_string(x), stringViews01[x], stringViews02[x], parser);
		}
		return true;
	}

}
