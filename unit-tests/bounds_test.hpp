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
#pragma once

#include "common.hpp"

namespace bounds_tests {

	inline void sliceStringBySize(std::string& input) {
		if (input.empty()) {
			return;
		}

		const size_t size			  = input.size();
		static constexpr size_t tenKb = 10 * 1024;

		if (size > tenKb) {
			input.resize(input.size() * 9 / 10);
		} else if (size >= 2000) {
			input.resize(input.size() / 2);
		} else {
			input.pop_back();
		}
	}

	template<rt_ut::string_literal testNameNew, typename test_data_type, bool prettified, bool partial, bool knownOrder> inline static void boundsTestsImpl() {
		static constexpr rt_ut::string_literal testName{ testNameNew };
		static constexpr rt_ut::string_literal testNameRtUt{ testNameNew + ", " + testTypePartial<partial> + testTypeKnownOrder<knownOrder> };
		auto dataToParse = file_handle::get(basePath.operator std::string() + "/json/" + testName.operator std::string() + ".json");
		jsonifier::jsonifier_core<> parser{};
		rt_ut::unit_test<testNameRtUt, true>::run(
			[&](std::string s) {
				test_data_type jsonifierValue;
				parser.parseJson<jsonifier::parse_options{ .partialRead = partial, .knownOrder = knownOrder, .minified = !prettified, .validateUtf8 = true }>(jsonifierValue, s);
				if (parser.getErrors().size()) {
					return false;
				}
				s.pop_back();
				while (!s.empty()) {
					test_data_type jsonifierValueLocal;
					parser.parseJson<jsonifier::parse_options{ .partialRead = partial, .knownOrder = knownOrder, .minified = !prettified, .validateUtf8 = true }>(
						jsonifierValueLocal, s);
					if (!parser.getErrors().size()) {
						return false;
					}
					parser.getErrors().clear();
					sliceStringBySize(s);
				}
				return true;
			},
			dataToParse);
	}

	template<bool partial, bool knownOrder> inline static void boundsTestsImpl() {
		std::cout << "Starting Bounds-Truncation Test, " << testTypePartial<partial> << testTypeKnownOrder<knownOrder> << ": " << std::endl;
		boundsTestsImpl<"Abc (In Order) Partial Test (Minified)", abc_in_order_partial_test, false, partial, knownOrder>();
		boundsTestsImpl<"Abc (In Order) Partial Test (Prettified)", abc_in_order_partial_test, true, partial, knownOrder>();
		boundsTestsImpl<"Abc (In Order) Test (Minified)", abc_in_order_test, false, partial, knownOrder>();
		boundsTestsImpl<"Abc (In Order) Test (Prettified)", abc_in_order_test, true, partial, knownOrder>();
		boundsTestsImpl<"Abc (Out of Order) Partial Test (Minified)", abc_out_of_order_partial_test, false, partial, knownOrder>();
		boundsTestsImpl<"Abc (Out of Order) Partial Test (Prettified)", abc_out_of_order_partial_test, true, partial, knownOrder>();
		boundsTestsImpl<"Abc (Out of Order) Test (Minified)", abc_out_of_order_test, false, partial, knownOrder>();
		boundsTestsImpl<"Abc (Out of Order) Test (Prettified)", abc_out_of_order_test, true, partial, knownOrder>();
		boundsTestsImpl<"Apache Builds Test (Minified)", apache_builds_message, false, partial, knownOrder>();
		boundsTestsImpl<"Apache Builds Test (Prettified)", apache_builds_message, true, partial, knownOrder>();
		boundsTestsImpl<"Canada Test (Minified)", canada_message, false, partial, knownOrder>();
		boundsTestsImpl<"Canada Test (Prettified)", canada_message, true, partial, knownOrder>();
		boundsTestsImpl<"CitmCatalog Test (Minified)", citm_catalog_message, false, partial, knownOrder>();
		boundsTestsImpl<"CitmCatalog Test (Prettified)", citm_catalog_message, true, partial, knownOrder>();
		boundsTestsImpl<"Discord Test (Minified)", discord_message, false, partial, knownOrder>();
		boundsTestsImpl<"Discord Test (Prettified)", discord_message, true, partial, knownOrder>();
		boundsTestsImpl<"Github Events Test (Minified)", github_events_message, false, partial, knownOrder>();
		boundsTestsImpl<"Github Events Test (Prettified)", github_events_message, true, partial, knownOrder>();
		boundsTestsImpl<"Google Maps Response Test (Minified)", google_maps_response_message, false, partial, knownOrder>();
		boundsTestsImpl<"Google Maps Response Test (Prettified)", google_maps_response_message, true, partial, knownOrder>();
		boundsTestsImpl<"Instruments Test (Minified)", instruments_message, false, partial, knownOrder>();
		boundsTestsImpl<"Instruments Test (Prettified)", instruments_message, true, partial, knownOrder>();
		boundsTestsImpl<"Marine IK Test (Minified)", marine_ik, false, partial, knownOrder>();
		boundsTestsImpl<"Marine IK Test (Prettified)", marine_ik, true, partial, knownOrder>();
		boundsTestsImpl<"Mesh Test (Minified)", mesh_message, false, partial, knownOrder>();
		boundsTestsImpl<"Mesh Test (Prettified)", mesh_message, true, partial, knownOrder>();
		boundsTestsImpl<"Random Test (Minified)", random_message, false, partial, knownOrder>();
		boundsTestsImpl<"Random Test (Prettified)", random_message, true, partial, knownOrder>();
		boundsTestsImpl<"Twitter Partial Test (Minified)", twitter_partial_message, false, partial, knownOrder>();
		boundsTestsImpl<"Twitter Partial Test (Prettified)", twitter_partial_message, true, partial, knownOrder>();
		boundsTestsImpl<"Twitter Test (Minified)", twitter_message, false, partial, knownOrder>();
		boundsTestsImpl<"Twitter Test (Prettified)", twitter_message, true, partial, knownOrder>();
	}

	inline static void boundsTests() {
		boundsTestsImpl<false, false>();
		boundsTestsImpl<false, true>();
		boundsTestsImpl<true, false>();
		boundsTestsImpl<true, true>();
	}

}
