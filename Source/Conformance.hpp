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

namespace conformance_tests {

	template<typename test_type> test_type runTest(const std::string& testName, const std::string& dataToParse, jsonifier::jsonifier_core<>& parser, bool doWeFail = true) noexcept {
		std::cout << "Running Test: " << testName << std::endl;
		test_type valueNew{};
		auto result = parser.parseJson<jsonifier::parse_options{ .knownOrder = true }>(valueNew, dataToParse);
		if ((parser.getErrors().size() == 0 && result) && !doWeFail) {
			std::cout << "Test: " << testName << " = Succeeded 01" << std::endl;
		} else if (!result && doWeFail) {
			std::cout << "Test: " << testName << " = Succeeded 02" << std::endl;
			for (auto& value: parser.getErrors()) {
				std::cout << "Jsonifier Error: " << value << std::endl;
			}
		} else {
			std::cout << "Test: " << testName << " = Failed" << std::endl;
			for (auto& value: parser.getErrors()) {
				std::cout << "Jsonifier Error: " << value << std::endl;
			}
		}
		return valueNew;
	}

	bool conformanceTests() noexcept {
		jsonifier::jsonifier_core parser{};
		std::unordered_map<std::string, test_base> jsonTests{};
		processFilesInFolder(jsonTests, "/ConformanceTests");
		std::cout << "Conformance Tests: " << std::endl;
		runTest<std::unordered_map<std::string, std::string>>("fail02.json", jsonTests["fail02.json"].fileContents, parser);
		runTest<std::unordered_map<std::string, std::string>>("fail03.json", jsonTests["fail03.json"].fileContents, parser);
		runTest<std::vector<std::string>>("fail04.json", jsonTests["fail04.json"].fileContents, parser);
		runTest<std::vector<std::string>>("fail05.json", jsonTests["fail05.json"].fileContents, parser);
		runTest<std::vector<std::string>>("fail06.json", jsonTests["fail06.json"].fileContents, parser);
		runTest<std::vector<std::string>>("fail07.json", jsonTests["fail07.json"].fileContents, parser);
		runTest<std::vector<std::string>>("fail08.json", jsonTests["fail08.json"].fileContents, parser);
		runTest<std::unordered_map<std::string, bool>>("fail09.json", jsonTests["fail09.json"].fileContents, parser);
		runTest<std::unordered_map<std::string, bool>>("fail10.json", jsonTests["fail10.json"].fileContents, parser);
		runTest<std::unordered_map<std::string, int32_t>>("fail11.json", jsonTests["fail11.json"].fileContents, parser);
		runTest<std::unordered_map<std::string, std::string>>("fail12.json", jsonTests["fail12.json"].fileContents, parser);
		runTest<std::unordered_map<std::string, int32_t>>("fail13.json", jsonTests["fail13.json"].fileContents, parser);
		runTest<std::unordered_map<std::string, int32_t>>("fail14.json", jsonTests["fail14.json"].fileContents, parser);
		runTest<std::vector<std::string>>("fail15.json", jsonTests["fail15.json"].fileContents, parser);
		runTest<std::vector<std::string>>("fail16.json", jsonTests["fail16.json"].fileContents, parser);
		runTest<std::vector<std::string>>("fail17.json", jsonTests["fail17.json"].fileContents, parser);
		runTest<std::unordered_map<std::string, int32_t*>>("fail19.json", jsonTests["fail19.json"].fileContents, parser);
		runTest<std::unordered_map<std::string, int32_t*>>("fail20.json", jsonTests["fail20.json"].fileContents, parser);
		runTest<std::unordered_map<std::string, int32_t*>>("fail21.json", jsonTests["fail21.json"].fileContents, parser);
		runTest<std::vector<std::string>>("fail22.json", jsonTests["fail22.json"].fileContents, parser);
		runTest<std::vector<std::string>>("fail23.json", jsonTests["fail23.json"].fileContents, parser);
		runTest<std::vector<std::string>>("fail24.json", jsonTests["fail24.json"].fileContents, parser);
		runTest<std::vector<std::string>>("fail25.json", jsonTests["fail25.json"].fileContents, parser);
		runTest<std::vector<std::string>>("fail26.json", jsonTests["fail26.json"].fileContents, parser);
		runTest<std::vector<std::string>>("fail27.json", jsonTests["fail27.json"].fileContents, parser);
		runTest<std::vector<std::string>>("fail28.json", jsonTests["fail28.json"].fileContents, parser);
		runTest<std::vector<double>>("fail29.json", jsonTests["fail29.json"].fileContents, parser);
		runTest<std::vector<double>>("fail30.json", jsonTests["fail30.json"].fileContents, parser);
		runTest<std::vector<double>>("fail31.json", jsonTests["fail31.json"].fileContents, parser);
		runTest<std::unordered_map<std::string, bool>>("fail32.json", jsonTests["fail32.json"].fileContents, parser);
		runTest<std::vector<std::string>>("fail33.json", jsonTests["fail33.json"].fileContents, parser);
		return true;
	}

}