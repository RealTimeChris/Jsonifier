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

#include "apache-builds.hpp"
#include "canada.hpp"
#include "citm_catalog.hpp"
#include "discord.hpp"
#include "github_events.hpp"
#include "google_maps_response.hpp"
#include "instruments.hpp"
#include "marine_ik.hpp"
#include "mesh.hpp"
#include "random.hpp"
#include "semanticscholar.hpp"
#include "twitter.hpp"

namespace parsing_tests {

	enum class test_types {
		parse_serialize,
		minify,
		prettify,
		validate,
	};

	template<test_types test_type, rt_ut::string_literal testNameNew, typename value_type, bool prettified, bool partial, bool knownOrder, bool printSerialization = false>
	inline static void parsingTests(jsonifier::jsonifier_core<>& parser) {
		static constexpr rt_ut::string_literal testName{ testNameNew };
		static constexpr rt_ut::string_literal testNameRtUt{ testNameNew + ", " + testTypePartial<partial> + testTypeKnownOrder<knownOrder> };
		value_type glz_value;
		auto dataToParse = file_handle::get(basePath.operator std::string() + "/json/" + testName.operator std::string() + ".json");
		std::string serializedJson{};
		value_type jsonifier_value;
		rt_ut::unit_test<testNameRtUt, true>::run([&]() {
			if constexpr (test_type == test_types::parse_serialize) {
				parser.parseJson<jsonifier::parse_options{ .partialRead = partial, .knownOrder = knownOrder, .minified = !prettified, .validateUtf8 = true }>(jsonifier_value,
					dataToParse);
				parser.template serializeJson<jsonifier::serialize_options{ .prettify = prettified }>(jsonifier_value, serializedJson);
			} else if constexpr (test_type == test_types::minify) {
				parser.minifyJson(dataToParse, jsonifier_value);
			} else if constexpr (test_type == test_types::prettify) {
				parser.prettifyJson(dataToParse, jsonifier_value);
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

	template<bool partial, bool knownOrder> inline static void parsingTestsImpl([[maybe_unused]] jsonifier::jsonifier_core<>& parser) {
		std::cout << "Parsing Tests, " << testTypePartial<partial> << testTypeKnownOrder<knownOrder> << ": " << std::endl;
		parsingTests<test_types::parse_serialize, "Abc (In Order) Partial Test (Minified)", abc_in_order_partial_test, false, partial, knownOrder, true>(parser);
		parsingTests<test_types::parse_serialize, "Abc (In Order) Partial Test (Prettified)", abc_in_order_partial_test, true, partial, knownOrder, true>(parser);
		parsingTests<test_types::parse_serialize, "Abc (In Order) Test (Minified)", abc_in_order_test, false, partial, knownOrder>(parser);
		parsingTests<test_types::parse_serialize, "Abc (In Order) Test (Prettified)", abc_in_order_test, true, partial, knownOrder>(parser);
		parsingTests<test_types::parse_serialize, "Abc (Out of Order) Partial Test (Minified)", abc_out_of_order_partial_test, false, partial, knownOrder, true>(parser);
		parsingTests<test_types::parse_serialize, "Abc (Out of Order) Partial Test (Prettified)", abc_out_of_order_partial_test, true, partial, knownOrder, true>(parser);
		parsingTests<test_types::parse_serialize, "Abc (Out of Order) Test (Minified)", abc_out_of_order_test, false, partial, knownOrder>(parser);
		parsingTests<test_types::parse_serialize, "Abc (Out of Order) Test (Prettified)", abc_out_of_order_test, true, partial, knownOrder>(parser);
		parsingTests<test_types::parse_serialize, "Apache Builds Test (Minified)", apache_builds_message, false, partial, knownOrder>(parser);
		parsingTests<test_types::parse_serialize, "Apache Builds Test (Prettified)", apache_builds_message, true, partial, knownOrder>(parser);
		parsingTests<test_types::parse_serialize, "Canada Test (Minified)", canada_message, false, partial, knownOrder>(parser);
		parsingTests<test_types::parse_serialize, "Canada Test (Prettified)", canada_message, true, partial, knownOrder>(parser);
		parsingTests<test_types::parse_serialize, "CitmCatalog Test (Minified)", citm_catalog_message, false, partial, knownOrder>(parser);
		parsingTests<test_types::parse_serialize, "CitmCatalog Test (Prettified)", citm_catalog_message, true, partial, knownOrder>(parser);
		parsingTests<test_types::parse_serialize, "Discord Test (Minified)", discord_message, false, partial, knownOrder>(parser);
		parsingTests<test_types::parse_serialize, "Discord Test (Prettified)", discord_message, true, partial, knownOrder>(parser);
		parsingTests<test_types::parse_serialize, "Github Events Test (Minified)", github_events_message, false, partial, knownOrder>(parser);
		parsingTests<test_types::parse_serialize, "Github Events Test (Prettified)", github_events_message, true, partial, knownOrder>(parser);
		parsingTests<test_types::parse_serialize, "Google Maps Response Test (Minified)", google_maps_response_message, false, partial, knownOrder>(parser);
		parsingTests<test_types::parse_serialize, "Google Maps Response Test (Prettified)", google_maps_response_message, true, partial, knownOrder>(parser);
		parsingTests<test_types::parse_serialize, "Instruments Test (Minified)", instruments_message, false, partial, knownOrder>(parser);
		parsingTests<test_types::parse_serialize, "Instruments Test (Prettified)", instruments_message, true, partial, knownOrder>(parser);
		parsingTests<test_types::parse_serialize, "Marine IK Test (Minified)", marine_ik, false, partial, knownOrder>(parser);
		parsingTests<test_types::parse_serialize, "Marine IK Test (Prettified)", marine_ik, true, partial, knownOrder>(parser);
		parsingTests<test_types::parse_serialize, "Mesh Test (Minified)", mesh_message, false, partial, knownOrder>(parser);
		parsingTests<test_types::parse_serialize, "Mesh Test (Prettified)", mesh_message, true, partial, knownOrder>(parser);
		parsingTests<test_types::parse_serialize, "Random Test (Minified)", random_message, false, partial, knownOrder>(parser);
		parsingTests<test_types::parse_serialize, "Random Test (Prettified)", random_message, true, partial, knownOrder>(parser);
		parsingTests<test_types::parse_serialize, "Semanticscholar Corpus Test (Minified)", semantic_scholar_message, false, partial, knownOrder>(parser);
		parsingTests<test_types::parse_serialize, "Semanticscholar Corpus Test (Prettified)", semantic_scholar_message, true, partial, knownOrder>(parser);
		parsingTests<test_types::parse_serialize, "Twitter Partial Test (Minified)", twitter_partial_message, false, partial, knownOrder, true>(parser);
		parsingTests<test_types::parse_serialize, "Twitter Partial Test (Prettified)", twitter_partial_message, true, partial, knownOrder, true>(parser);
		parsingTests<test_types::parse_serialize, "Twitter Test (Minified)", twitter_message, false, partial, knownOrder>(parser);
		parsingTests<test_types::parse_serialize, "Twitter Test (Prettified)", twitter_message, true, partial, knownOrder>(parser);
	}

	inline static void parsingTests() {
		jsonifier::jsonifier_core<> parser{};
		parsingTestsImpl<false, false>(parser);
		parsingTestsImpl<false, true>(parser);
		parsingTestsImpl<true, false>(parser);
		parsingTestsImpl<true, true>(parser);
		parsingTests<test_types::minify, "Minify Test", std::string, false, false, false>(parser);
		parsingTests<test_types::prettify, "Prettify Test", std::string, false, false, false>(parser);
		parsingTests<test_types::validate, "Validate Test", std::string, false, false, false>(parser);
	}

}
