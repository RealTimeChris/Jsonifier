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
/// https://github.com/RealTimeChris/Json-Performance
#pragma once

#include "all_tests.hpp"

namespace tests {

	enum class test_types {
		parse_serialize,
		minify,
		prettify,
		validate,
	};

	template<test_types test_type, rt_ut::string_literal testName, typename value_type, bool prettified, bool partial, bool knownOder, bool printSerialization = false>
	inline static void parsing_tests(jsonifier::jsonifier_core<>& parser) {
		auto dataToParse = file_handle::get(basePath.operator std::string() + "/json/" + testName.operator std::string() + ".json");
		value_type value;
		rt_ut::unit_test<testName, true>::run([&]() {
			if constexpr (test_type == test_types::parse_serialize) {
				parser.parseJson<jsonifier::parse_options{ .partialRead = partial, .knownOrder = knownOder, .minified = !prettified }>(value, dataToParse);
				[[maybe_unused]] auto result = parser.template serializeJson<jsonifier::serialize_options{ .prettify = prettified }, value_type>(value);
				if constexpr (printSerialization) {
					std::cout << result << std::endl;
				}
			} else if constexpr (test_type == test_types::minify) {
				parser.minifyJson(dataToParse, value);
			} else if constexpr (test_type == test_types::prettify) {
				parser.prettifyJson(dataToParse, value);
			} else if constexpr (test_type == test_types::validate) {
				parser.validateJson(dataToParse);
			}
			if (auto& errors = parser.getErrors(); errors.size() > 0) {
				for (auto& error_value: errors) {
					std::cout << "Jsonifier Error: " << error_value << std::endl;
				}
				return false;
			} else {
				return true;
			}
		});
	};

	template<bool knownOrder> inline static void parsing_tests_impl() {
		jsonifier::jsonifier_core<> parser{};
		parsing_tests<test_types::parse_serialize, "Abc Partial Test (Minified)", abc_partial_test<abc_partial_test_struct>, false, true, knownOrder, true>(parser);
		/*
		parsing_tests<test_types::parse_serialize, "Abc Partial Test (Prettified)", abc_partial_test<abc_partial_test_struct>, true, true, knownOrder, true>(parser);

		parsing_tests<test_types::parse_serialize, "Abc (Out of Order) Test (Minified)", abc_test<abc_test_struct>, false, false, knownOrder>(parser);
		parsing_tests<test_types::parse_serialize, "Abc (Out of Order) Test (Prettified)", abc_test<abc_test_struct>, true, false, knownOrder>(parser);
		parsing_tests<test_types::parse_serialize, "Apache Builds Test (Minified)", apache_builds_message, false, false, knownOrder>(parser);
		parsing_tests<test_types::parse_serialize, "Apache Builds Test (Prettified)", apache_builds_message, true, false, knownOrder>(parser);
		parsing_tests<test_types::parse_serialize, "Discord Test (Minified)", discord_message, false, false, knownOrder>(parser);
		parsing_tests<test_types::parse_serialize, "Discord Test (Prettified)", discord_message, true, false, knownOrder>(parser);
		parsing_tests<test_types::parse_serialize, "Github Events Test (Minified)", github_events_message, false, false, knownOrder>(parser);
		parsing_tests<test_types::parse_serialize, "Github Events Test (Prettified)", github_events_message, true, false, knownOrder>(parser);
		parsing_tests<test_types::parse_serialize, "Google Maps Response Test (Minified)", google_maps_response_message, false, false, knownOrder>(parser);
		parsing_tests<test_types::parse_serialize, "Google Maps Response Test (Prettified)", google_maps_response_message, true, false, knownOrder>(parser);
		parsing_tests<test_types::parse_serialize, "Canada Test (Minified)", canada_message, false, false, knownOrder>(parser); 
		parsing_tests<test_types::parse_serialize, "Canada Test (Prettified)", canada_message, true, false, knownOrder>(parser);
		parsing_tests<test_types::parse_serialize, "CitmCatalog Test (Minified)", citm_catalog_message, false, false, knownOrder>(parser);
		parsing_tests<test_types::parse_serialize, "CitmCatalog Test (Prettified)", citm_catalog_message, true, false, knownOrder>(parser);
		parsing_tests<test_types::parse_serialize, "Instruments Test (Minified)", instruments_message, false, false, knownOrder>(parser);
		parsing_tests<test_types::parse_serialize, "Instruments Test (Prettified)", instruments_message, true, false, knownOrder>(parser);
		parsing_tests<test_types::parse_serialize, "Marine IK Test (Minified)", marine_ik, false, false, knownOrder>(parser);
		parsing_tests<test_types::parse_serialize, "Marine IK Test (Prettified)", marine_ik, true, false, knownOrder>(parser);
		parsing_tests<test_types::parse_serialize, "Mesh Test (Minified)", mesh_message, false, false, knownOrder>(parser);
		parsing_tests<test_types::parse_serialize, "Mesh Test (Prettified)", mesh_message, true, false, knownOrder>(parser);
		parsing_tests<test_types::parse_serialize, "Random Test (Minified)", random_message, false, false, knownOrder>(parser);
		parsing_tests<test_types::parse_serialize, "Random Test (Prettified)", random_message, true, false, knownOrder>(parser);
		parsing_tests<test_types::parse_serialize, "Twitter Test (Minified)", twitter_message, false, false, knownOrder>(parser);
		parsing_tests<test_types::parse_serialize, "Twitter Test (Prettified)", twitter_message, true, false, knownOrder>(parser);
		parsing_tests<test_types::parse_serialize, "Twitter Partial Test (Minified)", twitter_partial_message, false, true, knownOrder, true>(parser);
		parsing_tests<test_types::parse_serialize, "Twitter Partial Test (Prettified)", twitter_partial_message, true, true, knownOrder, true>(parser);
		parsing_tests<test_types::minify, "Minify Test", std::string, false, false, knownOrder>(parser);
		parsing_tests<test_types::prettify, "Prettify Test", std::string, false, false, knownOrder>(parser);
		parsing_tests<test_types::validate, "Validate Test", std::string, false, false, knownOrder>(parser);*/
	}

	inline static void parsing_tests() {
		parsing_tests_impl<true>();
		parsing_tests_impl<false>();
	}

}
