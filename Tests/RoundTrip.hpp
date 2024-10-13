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

#include <unordered_map>
#include <vector>
#include <string>

namespace round_trip_tests {

	struct Obj2 {
		std::string foo;
	};

	struct Obj3 {
		std::unique_ptr<int32_t> a;
		std::string* foo;
	};
}

template<> struct jsonifier::core<round_trip_tests::Obj2> {
	using value_type = round_trip_tests::Obj2;
	static constexpr auto parseValue = createValue<&value_type::foo>();
};

template<> struct jsonifier::core<round_trip_tests::Obj3> {
	using value_type				 = round_trip_tests::Obj3;
	static constexpr auto parseValue = createValue<&value_type::a, &value_type::foo>();
};

namespace round_trip_tests {

	template<typename test_type>
	test_type runTest(const std::string& testName, const std::string& dataToParse, jsonifier::jsonifier_core<>& parser) noexcept {
		std::cout << "Running Test: " << testName << std::endl;
		test_type valueNew{};
		std::string newString{};
		if (parser.parseJson<jsonifier::parse_options{ .knownOrder = true }>(valueNew, dataToParse)) {
			parser.serializeJson(valueNew, newString);
			if (parser.getErrors().size() == 0) {
				std::cout << "Test: " << testName << " = Succeeded - Output: " << newString << std::endl;
			} else {
				std::cout << "Test: " << testName << " = Failed - Output: " << newString << std::endl;
				for (auto& value: parser.getErrors()) {
					std::cout << "Jsonifier Error: " << value << std::endl;
				}
			}
		} else {
			std::cout << "Test: " << testName << " = Failed - Output: " << newString << std::endl;
			for (auto& value: parser.getErrors()) {
				std::cout << "Jsonifier Error: " << value << std::endl;
			}
		}
		return valueNew;
	}

	bool roundTripTests() noexcept {
		jsonifier::jsonifier_core parser{};
		std::unordered_map<std::string, test_base> jsonTests{};
		processFilesInFolder(jsonTests, "RoundTrip");
		std::cout << "RoundTrip Tests: " << std::endl;
		runTest<std::vector<int32_t*>>("roundtrip01.json", jsonTests["roundtrip01.json"].fileContents, parser);
		runTest<std::vector<bool>>("roundtrip02.json", jsonTests["roundtrip02.json"].fileContents, parser);
		runTest<std::vector<bool>>("roundtrip03.json", jsonTests["roundtrip03.json"].fileContents, parser);
		runTest<std::vector<int32_t>>("roundtrip04.json", jsonTests["roundtrip04.json"].fileContents, parser);
		runTest<std::vector<std::string>>("roundtrip05.json", jsonTests["roundtrip05.json"].fileContents, parser);
		runTest<std::vector<int32_t>>("roundtrip06.json", jsonTests["roundtrip06.json"].fileContents, parser);
		runTest<std::unordered_map<std::string, std::string>>("roundtrip07.json", jsonTests["roundtrip07.json"].fileContents, parser);
		runTest<std::vector<int32_t>>("roundtrip08.json", jsonTests["roundtrip08.json"].fileContents, parser);
		runTest<Obj2>("roundtrip09.json", jsonTests["roundtrip09.json"].fileContents, parser);
		runTest<Obj3>("roundtrip10.json", jsonTests["roundtrip10.json"].fileContents, parser);
		runTest<std::vector<int32_t>>("roundtrip11.json", jsonTests["roundtrip11.json"].fileContents, parser);
		runTest<std::vector<int32_t>>("roundtrip12.json", jsonTests["roundtrip12.json"].fileContents, parser);
		runTest<std::vector<int64_t>>("roundtrip13.json", jsonTests["roundtrip13.json"].fileContents, parser);
		runTest<std::vector<int64_t>>("roundtrip14.json", jsonTests["roundtrip14.json"].fileContents, parser);
		runTest<std::vector<int32_t>>("roundtrip15.json", jsonTests["roundtrip15.json"].fileContents, parser);
		runTest<std::vector<int32_t>>("roundtrip16.json", jsonTests["roundtrip16.json"].fileContents, parser);
		runTest<std::vector<int64_t>>("roundtrip17.json", jsonTests["roundtrip17.json"].fileContents, parser);
		runTest<std::vector<int64_t>>("roundtrip18.json", jsonTests["roundtrip18.json"].fileContents, parser);
		runTest<std::vector<int64_t>>("roundtrip19.json", jsonTests["roundtrip19.json"].fileContents, parser);
		runTest<std::vector<double>>("roundtrip20.json", jsonTests["roundtrip20.json"].fileContents, parser);
		runTest<std::vector<double>>("roundtrip21.json", jsonTests["roundtrip21.json"].fileContents, parser);
		runTest<std::vector<double>>("roundtrip22.json", jsonTests["roundtrip22.json"].fileContents, parser);
		runTest<std::vector<double>>("roundtrip23.json", jsonTests["roundtrip23.json"].fileContents, parser);
		runTest<std::vector<double>>("roundtrip24.json", jsonTests["roundtrip24.json"].fileContents, parser);
		runTest<std::vector<double>>("roundtrip25.json", jsonTests["roundtrip25.json"].fileContents, parser);
		runTest<std::vector<double>>("roundtrip26.json", jsonTests["roundtrip26.json"].fileContents, parser);
		runTest<std::vector<double>>("roundtrip27.json", jsonTests["roundtrip27.json"].fileContents, parser);

		return true;
	}
}