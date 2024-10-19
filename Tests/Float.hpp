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

namespace float_validation_tests {

	auto testValues = std::unordered_map<std::string_view, double>{ { "test1.json", 0.0 }, { "test2.json", -0.0 }, { "test3.json", 1.0 }, { "test4.json", -1.0 },
		{ "test5.json", 1.5 }, { "test6.json", -1.5 }, { "test7.json", 3.1416 }, { "test8.json", 1E10 }, { "test9.json", 1e10 }, { "test10.json", 1E+10 }, { "test11.json", 1E-10 },
		{ "test12.json", -1E10 }, { "test13.json", -1e10 }, { "test14.json", -1E+10 }, { "test15.json", -1E-10 }, { "test16.json", 1.234E+10 }, { "test17.json", 1.234E-10 },
		{ "test18.json", 1.79769e+308 }, { "test19.json", 2.22507e-308 }, { "test20.json", -1.79769e+308 }, { "test21.json", -2.22507e-308 },
		{ "test22.json", 4.9406564584124654e-324 }, { "test23.json", 2.2250738585072009e-308 }, { "test24.json", 2.2250738585072014e-308 },
		{ "test25.json", 1.7976931348623157e+308 }, { "test26.json", 0.0 }, { "test27.json", 18446744073709551616.0 }, { "test28.json", -9223372036854775808.0 },
		{ "test29.json", 0.9868011474609375 }, { "test30.json", 123e34 }, { "test31.json", 45913141877270640000.0 }, { "test32.json", 2.2250738585072011e-308 },
		{ "test33.json", 1e-00011111111111 }, { "test34.json", -1e-00011111111111 }, { "test35.json", 1e-214748363 }, { "test36.json", 1e-214748364 },
		{ "test37.json", 1e-21474836311 }, { "test38.json", 0.017976931348623157e+310 }, { "test39.json", 2.2250738585072012e-308 },
		{ "test40.json", 2.22507385850720113605740979670913197593481954635164564e-308 }, { "test41.json", 2.22507385850720113605740979670913197593481954635164565e-308 },
		{ "test42.json", 0.999999999999999944488848768742172978818416595458984375 }, { "test43.json", 0.999999999999999944488848768742172978818416595458984374 },
		{ "test44.json", 0.999999999999999944488848768742172978818416595458984376 }, { "test45.json", 1.00000000000000011102230246251565404236316680908203125 },
		{ "test46.json", 1.00000000000000011102230246251565404236316680908203124 }, { "test47.json", 1.00000000000000011102230246251565404236316680908203126 },
		{ "test48.json", 72057594037927928.0 }, { "test49.json", 72057594037927936.0 }, { "test50.json", 72057594037927932.0 }, { "test51.json", 7205759403792793199999e-5 },
		{ "test52.json", 7205759403792793200001e-5 }, { "test53.json", 9223372036854774784.0 }, { "test54.json", 9223372036854775808.0 }, { "test55.json", 9223372036854775296.0 },
		{ "test56.json", 922337203685477529599999e-5 }, { "test57.json", 922337203685477529600001e-5 }, { "test58.json", 10141204801825834086073718800384.0 },
		{ "test59.json", 10141204801825835211973625643008.0 }, { "test60.json", 10141204801825834649023672221696.0 }, { "test61.json", 1014120480182583464902367222169599999e-5 },
		{ "test62.json", 1014120480182583464902367222169600001e-5 }, { "test63.json", 5708990770823838890407843763683279797179383808.0 },
		{ "test64.json", 5708990770823839524233143877797980545530986496.0 }, { "test65.json", 5708990770823839207320493820740630171355185152.0 },
		{ "test66.json", 5708990770823839207320493820740630171355185151999e-3 }, { "test67.json", 5708990770823839207320493820740630171355185152001e-3 }, { "test68.json", 1e+308 },
		{ "test69.json", 2.22507e-308 } };

	auto runTest(const std::string_view& testName, const std::string& dataToParse, jsonifier::jsonifier_core<>& parser) noexcept {
		std::cout << testName << " Input: " << dataToParse << std::endl;
		std::vector<double> data;
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

	bool floatTests() noexcept {
		jsonifier::jsonifier_core parser{};
		std::unordered_map<std::string, test_base> jsonTests{};
		processFilesInFolder(jsonTests, "FloatValidation");
		std::cout << "Float Tests: " << std::endl;
		runTest("test1.json", jsonTests["test1.json"].fileContents, parser);
		runTest("test2.json", jsonTests["test2.json"].fileContents, parser);
		runTest("test3.json", jsonTests["test3.json"].fileContents, parser);
		runTest("test4.json", jsonTests["test4.json"].fileContents, parser);
		runTest("test5.json", jsonTests["test5.json"].fileContents, parser);
		runTest("test6.json", jsonTests["test6.json"].fileContents, parser);
		runTest("test7.json", jsonTests["test7.json"].fileContents, parser);
		runTest("test8.json", jsonTests["test8.json"].fileContents, parser);
		runTest("test9.json", jsonTests["test9.json"].fileContents, parser);
		runTest("test10.json", jsonTests["test10.json"].fileContents, parser);
		runTest("test11.json", jsonTests["test11.json"].fileContents, parser);
		runTest("test12.json", jsonTests["test12.json"].fileContents, parser);
		runTest("test13.json", jsonTests["test13.json"].fileContents, parser);
		runTest("test14.json", jsonTests["test14.json"].fileContents, parser);
		runTest("test15.json", jsonTests["test15.json"].fileContents, parser);
		runTest("test16.json", jsonTests["test16.json"].fileContents, parser);
		runTest("test17.json", jsonTests["test17.json"].fileContents, parser);
		runTest("test18.json", jsonTests["test18.json"].fileContents, parser);
		runTest("test19.json", jsonTests["test19.json"].fileContents, parser);
		runTest("test20.json", jsonTests["test20.json"].fileContents, parser);
		runTest("test21.json", jsonTests["test21.json"].fileContents, parser);
		runTest("test22.json", jsonTests["test22.json"].fileContents, parser);
		runTest("test23.json", jsonTests["test23.json"].fileContents, parser);
		runTest("test24.json", jsonTests["test24.json"].fileContents, parser);
		runTest("test25.json", jsonTests["test25.json"].fileContents, parser);
		runTest("test26.json", jsonTests["test26.json"].fileContents, parser);
		runTest("test27.json", jsonTests["test27.json"].fileContents, parser);
		runTest("test28.json", jsonTests["test28.json"].fileContents, parser);
		runTest("test29.json", jsonTests["test29.json"].fileContents, parser);
		runTest("test30.json", jsonTests["test30.json"].fileContents, parser);
		runTest("test31.json", jsonTests["test31.json"].fileContents, parser);
		runTest("test32.json", jsonTests["test32.json"].fileContents, parser);
		runTest("test33.json", jsonTests["test33.json"].fileContents, parser);
		runTest("test34.json", jsonTests["test34.json"].fileContents, parser);
		runTest("test35.json", jsonTests["test35.json"].fileContents, parser);
		runTest("test36.json", jsonTests["test36.json"].fileContents, parser);
		runTest("test37.json", jsonTests["test37.json"].fileContents, parser);
		runTest("test38.json", jsonTests["test38.json"].fileContents, parser);
		runTest("test39.json", jsonTests["test39.json"].fileContents, parser);
		runTest("test40.json", jsonTests["test40.json"].fileContents, parser);
		runTest("test41.json", jsonTests["test41.json"].fileContents, parser);
		runTest("test42.json", jsonTests["test42.json"].fileContents, parser);
		runTest("test43.json", jsonTests["test43.json"].fileContents, parser);
		runTest("test44.json", jsonTests["test44.json"].fileContents, parser);
		runTest("test45.json", jsonTests["test45.json"].fileContents, parser);
		runTest("test46.json", jsonTests["test46.json"].fileContents, parser);
		runTest("test47.json", jsonTests["test47.json"].fileContents, parser);
		runTest("test48.json", jsonTests["test48.json"].fileContents, parser);
		runTest("test49.json", jsonTests["test49.json"].fileContents, parser);
		runTest("test50.json", jsonTests["test50.json"].fileContents, parser);
		runTest("test51.json", jsonTests["test51.json"].fileContents, parser);
		runTest("test52.json", jsonTests["test52.json"].fileContents, parser);
		runTest("test53.json", jsonTests["test53.json"].fileContents, parser);
		runTest("test54.json", jsonTests["test54.json"].fileContents, parser);
		runTest("test55.json", jsonTests["test55.json"].fileContents, parser);
		runTest("test56.json", jsonTests["test56.json"].fileContents, parser);
		runTest("test57.json", jsonTests["test57.json"].fileContents, parser);
		runTest("test58.json", jsonTests["test58.json"].fileContents, parser);
		runTest("test59.json", jsonTests["test59.json"].fileContents, parser);
		runTest("test60.json", jsonTests["test60.json"].fileContents, parser);
		runTest("test61.json", jsonTests["test61.json"].fileContents, parser);
		runTest("test62.json", jsonTests["test62.json"].fileContents, parser);
		runTest("test63.json", jsonTests["test63.json"].fileContents, parser);
		runTest("test64.json", jsonTests["test64.json"].fileContents, parser);
		runTest("test65.json", jsonTests["test65.json"].fileContents, parser);
		runTest("test66.json", jsonTests["test66.json"].fileContents, parser);
		runTest("test67.json", jsonTests["test67.json"].fileContents, parser);
		runTest("test68.json", jsonTests["test68.json"].fileContents, parser);
		runTest("test69.json", jsonTests["test69.json"].fileContents, parser);
		return true;
	}

}