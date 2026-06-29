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

	template<test_types test_type, rt_ut::string_literal testNameNew, rt_ut::string_literal testTypePartialNew, typename value_type, bool prettified, bool partial, bool knownOder,
		bool printSerialization = false>
	inline static void parsing_tests(jsonifier::jsonifier_core<>& parser) {
		static constexpr rt_ut::string_literal testName{ testNameNew };
		static constexpr rt_ut::string_literal testNameRtUt{ testNameNew + ", " + testTypePartialNew };
		auto dataToParse = file_handle::get(basePath.operator std::string() + "/json/" + testName.operator std::string() + ".json");
		value_type value;
		rt_ut::unit_test<testNameRtUt, true>::run([&]() {
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

	template<bool knownOrder, bool partialRead> inline static void parsing_tests_impl() {
		static constexpr rt_ut::string_literal test_type_partial{ [] {
			if constexpr (partialRead) {
				return rt_ut::string_literal{ "Partial-Reading: Enabled" };
			} else {
				return rt_ut::string_literal{ "Partial-Reading: Disabled" };
			}
		}() };
		jsonifier::jsonifier_core<> parser{};
		parsing_tests<test_types::parse_serialize, "Abc Partial Test (Minified)", test_type_partial, abc_partial_test<abc_partial_test_struct>, false, partialRead, knownOrder>(
			parser);
		parsing_tests<test_types::parse_serialize, "Abc Partial Test (Prettified)", test_type_partial, abc_partial_test<abc_partial_test_struct>, true, partialRead, knownOrder>(
			parser);
		parsing_tests<test_types::parse_serialize, "Abc (Out of Order) Test (Minified)", test_type_partial, abc_test<abc_test_struct>, false, partialRead, knownOrder>(parser);
		parsing_tests<test_types::parse_serialize, "Abc (Out of Order) Test (Prettified)", test_type_partial, abc_test<abc_test_struct>, true, partialRead, knownOrder>(parser);
		parsing_tests<test_types::parse_serialize, "Apache Builds Test (Minified)", test_type_partial, apache_builds_message, false, partialRead, knownOrder>(parser);
		parsing_tests<test_types::parse_serialize, "Apache Builds Test (Prettified)", test_type_partial, apache_builds_message, true, partialRead, knownOrder>(parser);
		parsing_tests<test_types::parse_serialize, "Discord Test (Minified)", test_type_partial, discord_message, false, partialRead, knownOrder>(parser);
		parsing_tests<test_types::parse_serialize, "Discord Test (Prettified)", test_type_partial, discord_message, true, partialRead, knownOrder>(parser);
		parsing_tests<test_types::parse_serialize, "Github Events Test (Minified)", test_type_partial, github_events_message, false, partialRead, knownOrder>(parser);
		parsing_tests<test_types::parse_serialize, "Github Events Test (Prettified)", test_type_partial, github_events_message, true, partialRead, knownOrder>(parser);
		parsing_tests<test_types::parse_serialize, "Google Maps Response Test (Minified)", test_type_partial, google_maps_response_message, false, partialRead, knownOrder>(parser);
		parsing_tests<test_types::parse_serialize, "Google Maps Response Test (Prettified)", test_type_partial, google_maps_response_message, true, partialRead, knownOrder>(
			parser);
		parsing_tests<test_types::parse_serialize, "Canada Test (Minified)", test_type_partial, canada_message, false, partialRead, knownOrder>(parser);
		parsing_tests<test_types::parse_serialize, "Canada Test (Prettified)", test_type_partial, canada_message, true, partialRead, knownOrder>(parser);
		parsing_tests<test_types::parse_serialize, "CitmCatalog Test (Minified)", test_type_partial, citm_catalog_message, false, partialRead, knownOrder>(parser);
		parsing_tests<test_types::parse_serialize, "CitmCatalog Test (Prettified)", test_type_partial, citm_catalog_message, true, partialRead, knownOrder>(parser);
		parsing_tests<test_types::parse_serialize, "Instruments Test (Minified)", test_type_partial, instruments_message, false, partialRead, knownOrder>(parser);
		parsing_tests<test_types::parse_serialize, "Instruments Test (Prettified)", test_type_partial, instruments_message, true, partialRead, knownOrder>(parser);
		parsing_tests<test_types::parse_serialize, "Marine IK Test (Minified)", test_type_partial, marine_ik, false, partialRead, knownOrder>(parser);
		parsing_tests<test_types::parse_serialize, "Marine IK Test (Prettified)", test_type_partial, marine_ik, true, partialRead, knownOrder>(parser);
		parsing_tests<test_types::parse_serialize, "Mesh Test (Minified)", test_type_partial, mesh_message, false, partialRead, knownOrder>(parser);
		parsing_tests<test_types::parse_serialize, "Mesh Test (Prettified)", test_type_partial, mesh_message, true, partialRead, knownOrder>(parser);
		parsing_tests<test_types::parse_serialize, "Random Test (Minified)", test_type_partial, random_message, false, partialRead, knownOrder>(parser);
		parsing_tests<test_types::parse_serialize, "Random Test (Prettified)", test_type_partial, random_message, true, partialRead, knownOrder>(parser);
		parsing_tests<test_types::parse_serialize, "Twitter Test (Minified)", test_type_partial, twitter_message, false, partialRead, knownOrder>(parser);
		parsing_tests<test_types::parse_serialize, "Twitter Test (Prettified)", test_type_partial, twitter_message, true, partialRead, knownOrder>(parser);
		parsing_tests<test_types::parse_serialize, "Twitter Partial Test (Minified)", test_type_partial, twitter_partial_message, false, partialRead, knownOrder>(parser);
		parsing_tests<test_types::parse_serialize, "Twitter Partial Test (Prettified)", test_type_partial, twitter_partial_message, true, partialRead, knownOrder>(parser);
		parsing_tests<test_types::minify, "Minify Test", test_type_partial, std::string, false, partialRead, knownOrder>(parser);
		parsing_tests<test_types::prettify, "Prettify Test", test_type_partial, std::string, false, partialRead, knownOrder>(parser);
		parsing_tests<test_types::validate, "Validate Test", test_type_partial, std::string, false, partialRead, knownOrder>(parser);
	}

	inline static void parsing_tests() {
		parsing_tests_impl<true, true>();
		parsing_tests_impl<false, true>();
		parsing_tests_impl<true, false>();
		parsing_tests_impl<false, false>();
	}

}
