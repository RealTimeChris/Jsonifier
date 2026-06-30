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

	THE SOFTWARE IS PROVIDED "AS IS", partial, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
	INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
	PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE
	FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
	OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
	DEALINGS IN THE SOFTWARE.
*/
/// https://github.com/RealTimeChris/jsonifier
#pragma once

#include "common.hpp"

#include <jsonifier>
#include <filesystem>
#include <fstream>

struct pass03 {
	std::unordered_map<std::string, std::string> json_test_pattern_pass3{};
};

using pass01 = jsonifier::raw_json_data;

namespace jsonifier {
	template<> struct core<pass03> {
		using value_type				 = pass03;
		static constexpr auto parseValue = createValue<makeJsonEntity<&value_type::json_test_pattern_pass3, "JSON Test Pattern pass3">()>();
	};
}

using pass02 = jsonifier::raw_json_data;

namespace conformance_tests {	

	template<rt_ut::string_literal testNameNew, bool partial, typename test_type, jsonifier::internal::parse_status parse_error = jsonifier::internal::parse_status{}>
	inline static void runConformanceTest(const std::string& dataToParse, jsonifier::jsonifier_core<>& parser) {
		static constexpr rt_ut::string_literal testTypePartial{ [] {
			if constexpr (partial) {
				return rt_ut::string_literal{ " Partial-Reading: Enabled" };
			} else {
				return rt_ut::string_literal{ " Partial-Reading: Disabled" };
			}
		}() };
		static constexpr rt_ut::string_literal testName{ testNameNew + testTypePartial };
		test_type valueNew{};
		std::cout << "Input String: " << dataToParse << std::endl;
		parser.parseJson<jsonifier::parse_options{ .partialRead = partial, .knownOrder = false }>(valueNew, dataToParse);
		rt_ut::unit_test<testName, true>::template assert_eq<parse_error>([&]() {
			auto& errors = parser.getErrors();
			for (auto& value: errors) {
				std::cout << "Error: " << value << std::endl;
			}
			if (errors.size() > 0) {
				return errors[0].operator jsonifier::internal::parse_status();
			} else {
				return jsonifier::internal::parse_status::Success;
			}
		});
	}

	template<bool partial>
	inline static void conformanceTestsImpl() {
		jsonifier::jsonifier_core<> parser{};
		std::unordered_map<std::string, test_base> jsonTests{};
		processFilesInFolder(jsonTests, "/jsonchecker");
		std::cout << "Conformance Fail Tests: " << std::endl;
		runConformanceTest<"fail02.json", partial, std::vector<std::string>, jsonifier::internal::parse_status::Missing_Array_End>(jsonTests["fail02.json"].fileContents,
			parser);
		runConformanceTest<"fail03.json", partial, std::unordered_map<std::string, std::string>, jsonifier::internal::parse_status::Missing_String_Start>(
			jsonTests["fail03.json"].fileContents, parser);
		runConformanceTest<"fail04.json", partial, std::vector<std::string>, jsonifier::internal::parse_status::Missing_Array_End>(jsonTests["fail04.json"].fileContents, parser);
		runConformanceTest<"fail05.json", partial, std::vector<std::string>, jsonifier::internal::parse_status::Missing_String_Start>(jsonTests["fail05.json"].fileContents, parser);
		runConformanceTest<"fail06.json", partial, std::vector<std::string>, jsonifier::internal::parse_status::Missing_String_Start>(jsonTests["fail06.json"].fileContents, parser);
		runConformanceTest<"fail07.json", partial, std::vector<std::string>, jsonifier::internal::parse_status::Unfinished_Input>(jsonTests["fail07.json"].fileContents, parser);
		runConformanceTest<"fail08.json", partial, std::vector<std::string>, jsonifier::internal::parse_status::Unfinished_Input>(jsonTests["fail08.json"].fileContents, parser);
		runConformanceTest<"fail09.json", partial, std::unordered_map<std::string, bool>, jsonifier::internal::parse_status::Missing_String_Start>(
			jsonTests["fail09.json"].fileContents, parser);
		runConformanceTest<"fail10.json", partial, std::unordered_map<std::string, bool>, jsonifier::internal::parse_status::Unfinished_Input>(jsonTests["fail10.json"].fileContents,
			parser);
		runConformanceTest<"fail11.json", partial, std::unordered_map<std::string, int32_t>, jsonifier::internal::parse_status::Missing_Comma>(jsonTests["fail11.json"].fileContents,
			parser);
		runConformanceTest<"fail12.json", partial, std::unordered_map<std::string, std::string>, jsonifier::internal::parse_status::Missing_String_Start>(
			jsonTests["fail12.json"].fileContents, parser);
		runConformanceTest<"fail13.json", partial, std::unordered_map<std::string, int32_t>, jsonifier::internal::parse_status::Invalid_Number_Value>(
			jsonTests["fail13.json"].fileContents, parser);
		runConformanceTest<"fail14.json", partial, std::unordered_map<std::string, int32_t>, jsonifier::internal::parse_status::Invalid_Number_Value>(
			jsonTests["fail14.json"].fileContents, parser);
		runConformanceTest<"fail15.json", partial, std::vector<std::string>, jsonifier::internal::parse_status::Invalid_String_Characters>(jsonTests["fail15.json"].fileContents,
			parser);
		runConformanceTest<"fail16.json", partial, std::vector<std::string>, jsonifier::internal::parse_status::Missing_String_Start>(jsonTests["fail16.json"].fileContents, parser);
		runConformanceTest<"fail17.json", partial, std::vector<std::string>, jsonifier::internal::parse_status::Invalid_String_Characters>(jsonTests["fail17.json"].fileContents,
			parser);
		runConformanceTest<"fail19.json", partial, std::unordered_map<std::string, std::string>, jsonifier::internal::parse_status::Missing_Colon>(
			jsonTests["fail19.json"].fileContents, parser);
		runConformanceTest<"fail20.json", partial, std::unordered_map<std::string, std::string>, jsonifier::internal::parse_status::Missing_String_Start>(
			jsonTests["fail20.json"].fileContents, parser);
		runConformanceTest<"fail21.json", partial, std::unordered_map<std::string, std::string>, jsonifier::internal::parse_status::Missing_Colon>(
			jsonTests["fail21.json"].fileContents, parser);
		runConformanceTest<"fail22.json", partial, std::vector<bool>, jsonifier::internal::parse_status::Invalid_Bool_Value>(jsonTests["fail22.json"].fileContents, parser);
		runConformanceTest<"fail23.json", partial, std::vector<std::string>, jsonifier::internal::parse_status::Missing_String_Start>(jsonTests["fail23.json"].fileContents, parser);
		runConformanceTest<"fail24.json", partial, std::vector<std::string>, jsonifier::internal::parse_status::Missing_String_Start>(jsonTests["fail24.json"].fileContents, parser);
		runConformanceTest<"fail25.json", partial, std::vector<std::string>, jsonifier::internal::parse_status::Invalid_String_Characters>(jsonTests["fail25.json"].fileContents,
			parser);
		runConformanceTest<"fail26.json", partial, std::vector<std::string>, jsonifier::internal::parse_status::Invalid_String_Characters>(jsonTests["fail26.json"].fileContents,
			parser);
		runConformanceTest<"fail27.json", partial, std::vector<std::string>, jsonifier::internal::parse_status::Invalid_String_Characters>(jsonTests["fail27.json"].fileContents,
			parser);
		runConformanceTest<"fail28.json", partial, std::vector<std::string>, jsonifier::internal::parse_status::Invalid_String_Characters>(jsonTests["fail28.json"].fileContents,
			parser);
		runConformanceTest<"fail29.json", partial, std::vector<double>, jsonifier::internal::parse_status::Invalid_Number_Value>(jsonTests["fail29.json"].fileContents, parser);
		runConformanceTest<"fail30.json", partial, std::vector<double>, jsonifier::internal::parse_status::Invalid_Number_Value>(jsonTests["fail30.json"].fileContents, parser);
		runConformanceTest<"fail31.json", partial, std::vector<double>, jsonifier::internal::parse_status::Invalid_Number_Value>(jsonTests["fail31.json"].fileContents, parser);
		runConformanceTest<"fail32.json", partial, std::unordered_map<std::string, bool>, jsonifier::internal::parse_status::Missing_String_Start>(
			jsonTests["fail32.json"].fileContents, parser);
		runConformanceTest<"fail33.json", partial, std::vector<std::string>, jsonifier::internal::parse_status::Missing_Array_End>(jsonTests["fail33.json"].fileContents, parser);
		runConformanceTest<"fail34.json", partial, std::vector<std::string>, jsonifier::internal::parse_status::Success>(jsonTests["fail34.json"].fileContents, parser);
		runConformanceTest<"fail35.json", partial, std::unordered_map<std::string, std::string>, jsonifier::internal::parse_status::Missing_String_Start>(
			jsonTests["fail35.json"].fileContents, parser);
		runConformanceTest<"fail36.json", partial, std::vector<std::string>, jsonifier::internal::parse_status::Invalid_String_Characters>(jsonTests["fail36.json"].fileContents,
			parser);
		runConformanceTest<"fail37.json", partial, std::vector<int32_t>, jsonifier::internal::parse_status::Invalid_Number_Value>(jsonTests["fail37.json"].fileContents, parser);
		runConformanceTest<"fail38.json", partial, std::vector<int32_t>, jsonifier::internal::parse_status::Missing_Array_End>(jsonTests["fail38.json"].fileContents, parser);
		runConformanceTest<"fail42.json", partial, std::unordered_map<std::string, std::string>, jsonifier::internal::parse_status::Missing_String_Start>(
			jsonTests["fail42.json"].fileContents, parser);
		runConformanceTest<"fail43.json", partial, std::vector<double>, jsonifier::internal::parse_status::Invalid_Number_Value>(jsonTests["fail43.json"].fileContents, parser);
		runConformanceTest<"fail44.json", partial, std::vector<double>, jsonifier::internal::parse_status::Invalid_Number_Value>(jsonTests["fail44.json"].fileContents, parser);
		runConformanceTest<"fail45.json", partial, std::vector<double>, jsonifier::internal::parse_status::Invalid_Number_Value>(jsonTests["fail45.json"].fileContents, parser);
		runConformanceTest<"fail46.json", partial, std::vector<double>, jsonifier::internal::parse_status::Invalid_Number_Value>(jsonTests["fail46.json"].fileContents, parser);
		runConformanceTest<"fail47.json", partial, std::vector<double>, jsonifier::internal::parse_status::Invalid_Number_Value>(jsonTests["fail47.json"].fileContents, parser);
		runConformanceTest<"fail48.json", partial, std::vector<double>, jsonifier::internal::parse_status::Invalid_Number_Value>(jsonTests["fail48.json"].fileContents, parser);
		runConformanceTest<"fail49.json", partial, std::vector<double>, jsonifier::internal::parse_status::Invalid_Number_Value>(jsonTests["fail49.json"].fileContents, parser);
		runConformanceTest<"fail50.json", partial, std::vector<double>, jsonifier::internal::parse_status::Invalid_Number_Value>(jsonTests["fail50.json"].fileContents, parser);
		runConformanceTest<"fail51.json", partial, std::vector<std::string>, jsonifier::internal::parse_status::Unfinished_Input>(jsonTests["fail51.json"].fileContents, parser);
		runConformanceTest<"fail52.json", partial, std::vector<std::string>, jsonifier::internal::parse_status::Missing_String_Start>(jsonTests["fail52.json"].fileContents, parser);
		runConformanceTest<"fail53.json", partial, std::unordered_map<std::string, std::string>, jsonifier::internal::parse_status::Unfinished_Input>(
			jsonTests["fail53.json"].fileContents, parser);
		runConformanceTest<"fail54.json", partial, std::vector<std::string>, jsonifier::internal::parse_status::Missing_String_Start>(jsonTests["fail54.json"].fileContents, parser);
		runConformanceTest<"fail55.json", partial, std::vector<int32_t>, jsonifier::internal::parse_status::Missing_Array_End>(jsonTests["fail55.json"].fileContents, parser);
		runConformanceTest<"fail56.json", partial, std::vector<std::string>, jsonifier::internal::parse_status::Missing_Array_End>(jsonTests["fail56.json"].fileContents, parser);
		runConformanceTest<"fail57.json", partial, std::unordered_map<std::string, std::string>, jsonifier::internal::parse_status::Invalid_String_Characters>(
			jsonTests["fail57.json"].fileContents, parser);
		runConformanceTest<"fail58.json", partial, std::unordered_map<std::string, std::string>, jsonifier::internal::parse_status::Invalid_String_Characters>(
			jsonTests["fail58.json"].fileContents, parser);
		runConformanceTest<"fail59.json", partial, std::unordered_map<std::string, std::string>, jsonifier::internal::parse_status::Invalid_String_Characters>(
			jsonTests["fail59.json"].fileContents, parser);
		runConformanceTest<"fail60.json", partial, std::vector<double>, jsonifier::internal::parse_status::Invalid_Number_Value>(jsonTests["fail60.json"].fileContents, parser);
		runConformanceTest<"fail61.json", partial, std::unordered_map<std::string, std::string>, jsonifier::internal::parse_status::Invalid_String_Characters>(
			jsonTests["fail61.json"].fileContents, parser);
		runConformanceTest<"fail62.json", partial, std::unordered_map<std::string, std::string>, jsonifier::internal::parse_status::Invalid_String_Characters>(
			jsonTests["fail62.json"].fileContents, parser);
		runConformanceTest<"fail63.json", partial, std::string, jsonifier::internal::parse_status::Invalid_String_Characters>(jsonTests["fail63.json"].fileContents, parser);
		runConformanceTest<"fail64.json", partial, std::string, jsonifier::internal::parse_status::Invalid_String_Characters>(jsonTests["fail64.json"].fileContents, parser);
		runConformanceTest<"fail65.json", partial, bool, jsonifier::internal::parse_status::Invalid_Bool_Value>(jsonTests["fail65.json"].fileContents, parser);
		runConformanceTest<"fail66.json", partial, int32_t, jsonifier::internal::parse_status::Unfinished_Input>(jsonTests["fail66.json"].fileContents, parser);
		runConformanceTest<"fail67.json", partial, int32_t, jsonifier::internal::parse_status::Invalid_Number_Value>(jsonTests["fail67.json"].fileContents, parser);
		runConformanceTest<"fail68.json", partial, int32_t, jsonifier::internal::parse_status::Unfinished_Input>(jsonTests["fail68.json"].fileContents, parser);
		runConformanceTest<"fail69.json", partial, bool, jsonifier::internal::parse_status::Unfinished_Input>(jsonTests["fail69.json"].fileContents, parser);
		runConformanceTest<"fail70.json", partial, std::string, jsonifier::internal::parse_status::No_Input>(jsonTests["fail70.json"].fileContents, parser);
		runConformanceTest<"fail71.json", partial, std::string, jsonifier::internal::parse_status::Success>(jsonTests["fail71.json"].fileContents, parser);
		runConformanceTest<"fail72.json", partial, std::vector<std::string>, jsonifier::internal::parse_status::Unfinished_Input>(jsonTests["fail72.json"].fileContents, parser);
		runConformanceTest<"fail73.json", partial, double, jsonifier::internal::parse_status::Invalid_Number_Value>(jsonTests["fail73.json"].fileContents, parser);
		runConformanceTest<"fail74.json", partial, std::vector<int32_t>, jsonifier::internal::parse_status::Invalid_Number_Value>(jsonTests["fail74.json"].fileContents, parser);
		runConformanceTest<"fail75.json", partial, bool, jsonifier::internal::parse_status::Invalid_Bool_Value>(jsonTests["fail75.json"].fileContents, parser);
		runConformanceTest<"fail76.json", partial, std::unordered_map<std::string, std::string>, jsonifier::internal::parse_status::Invalid_String_Characters>(
			jsonTests["fail76.json"].fileContents, parser);
		runConformanceTest<"fail77.json", partial, std::unordered_map<std::string, std::string>, jsonifier::internal::parse_status::Invalid_String_Characters>(
			jsonTests["fail77.json"].fileContents, parser);
		runConformanceTest<"fail78.json", partial, std::vector<int32_t>, jsonifier::internal::parse_status::Invalid_Number_Value>(jsonTests["fail78.json"].fileContents, parser);
		runConformanceTest<"fail79.json", partial, std::vector<int32_t>, jsonifier::internal::parse_status::Missing_Array_End>(jsonTests["fail79.json"].fileContents, parser);
		runConformanceTest<"fail80.json", partial, std::unordered_map<std::string, double>, jsonifier::internal::parse_status::Unexpected_String_End>(
			jsonTests["fail80.json"].fileContents, parser);
		runConformanceTest<"fail81.json", partial, std::string, jsonifier::internal::parse_status::Unfinished_Input>(jsonTests["fail81.json"].fileContents, parser);
		runConformanceTest<"fail82.json", partial, std::string, jsonifier::internal::parse_status::Missing_String_Start>(jsonTests["fail82.json"].fileContents, parser);
		std::cout << "Conformance Pass Tests: " << std::endl;
		runConformanceTest<"pass01.json", partial, jsonifier::raw_json_data, jsonifier::internal::parse_status::Success>(jsonTests["pass01.json"].fileContents, parser);
		runConformanceTest<"pass02.json", partial, jsonifier::raw_json_data, jsonifier::internal::parse_status::Success>(jsonTests["pass02.json"].fileContents, parser);
		runConformanceTest<"pass03.json", partial, jsonifier::raw_json_data, jsonifier::internal::parse_status::Success>(jsonTests["pass03.json"].fileContents, parser);
		runConformanceTest<"pass04.json", partial, std::vector<double>, jsonifier::internal::parse_status::Success>(jsonTests["pass04.json"].fileContents, parser);
		runConformanceTest<"pass05.json", partial, double, jsonifier::internal::parse_status::Success>(jsonTests["pass05.json"].fileContents, parser);
		runConformanceTest<"pass06.json", partial, bool, jsonifier::internal::parse_status::Success>(jsonTests["pass06.json"].fileContents, parser);
		runConformanceTest<"pass07.json", partial, jsonifier::raw_json_data, jsonifier::internal::parse_status::Success>(jsonTests["pass07.json"].fileContents, parser);
		runConformanceTest<"pass08.json", partial, int32_t, jsonifier::internal::parse_status::Success>(jsonTests["pass08.json"].fileContents, parser);
		runConformanceTest<"pass09.json", partial, bool, jsonifier::internal::parse_status::Success>(jsonTests["pass09.json"].fileContents, parser);
		runConformanceTest<"pass10.json", partial, std::string, jsonifier::internal::parse_status::Success>(jsonTests["pass10.json"].fileContents, parser);
		runConformanceTest<"pass11.json", partial, uint64_t, jsonifier::internal::parse_status::Success>(jsonTests["pass11.json"].fileContents, parser);
		runConformanceTest<"pass12.json", partial, uint64_t, jsonifier::internal::parse_status::Success>(jsonTests["pass12.json"].fileContents, parser);
		runConformanceTest<"pass13.json", partial, int32_t, jsonifier::internal::parse_status::Success>(jsonTests["pass13.json"].fileContents, parser);
		runConformanceTest<"pass14.json", partial, std::unordered_map<std::string, int32_t>, jsonifier::internal::parse_status::Success>(jsonTests["pass14.json"].fileContents,
			parser);
		runConformanceTest<"pass15.json", partial, std::vector<double>, jsonifier::internal::parse_status::Success>(jsonTests["pass15.json"].fileContents, parser);
		runConformanceTest<"pass16.json", partial, int32_t, jsonifier::internal::parse_status::Success>(jsonTests["pass16.json"].fileContents, parser);
		runConformanceTest<"pass17.json", partial, std::vector<double>, jsonifier::internal::parse_status::Success>(jsonTests["pass17.json"].fileContents, parser);
		runConformanceTest<"pass18.json", partial, std::vector<jsonifier::raw_json_data>, jsonifier::internal::parse_status::Success>(jsonTests["pass18.json"].fileContents, parser);
		runConformanceTest<"pass19.json", partial, int32_t, jsonifier::internal::parse_status::Success>(jsonTests["pass19.json"].fileContents, parser);
		runConformanceTest<"pass20.json", partial, double, jsonifier::internal::parse_status::Success>(jsonTests["pass20.json"].fileContents, parser);
		runConformanceTest<"pass21.json", partial, uint64_t, jsonifier::internal::parse_status::Success>(jsonTests["pass21.json"].fileContents, parser);
		runConformanceTest<"pass22.json", partial, uint64_t, jsonifier::internal::parse_status::Success>(jsonTests["pass22.json"].fileContents, parser);
		runConformanceTest<"pass23.json", partial, double, jsonifier::internal::parse_status::Success>(jsonTests["pass23.json"].fileContents, parser);
		runConformanceTest<"pass24.json", partial, double, jsonifier::internal::parse_status::Success>(jsonTests["pass24.json"].fileContents, parser);
		runConformanceTest<"pass25.json", partial, double, jsonifier::internal::parse_status::Success>(jsonTests["pass25.json"].fileContents, parser);
		runConformanceTest<"pass26.json", partial, double, jsonifier::internal::parse_status::Success>(jsonTests["pass26.json"].fileContents, parser);
		runConformanceTest<"pass27.json", partial, double, jsonifier::internal::parse_status::Success>(jsonTests["pass27.json"].fileContents, parser);
	}

	inline static void conformanceTests() {
		conformanceTestsImpl<false>();
		conformanceTestsImpl<true>();
	}

}
