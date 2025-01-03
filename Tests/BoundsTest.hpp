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

#include "Common.hpp"

#include <jsonifier/Index.hpp>
#include <filesystem>
#include <fstream>

namespace bounds_tests {

	bool boundsTests() noexcept {
		jsonifier::jsonifier_core<> parser{};
		test_generator<test_struct> tests{};
		partial_test<partial_test_struct> newTests{};
		std::string testString{};
		parser.serializeJson(tests, testString);
		parser.parseJson(newTests, testString);
		newTests.m.resize(1);
		newTests.s.resize(0);
		parser.serializeJson(newTests, testString);
		while (testString.size() > 0) {
			test<test_struct> newData{};
			parser.parseJson<jsonifier::parse_options{ .minified = true }>(newData, testString);
			testString.resize(testString.size() - 1);
		}
		parser.serializeJson<jsonifier::serialize_options{ .prettify = true }>(newTests, testString);
		while (testString.size() > 0) {
			test<test_struct> newData{};
			parser.parseJson(newData, testString);
			testString.resize(testString.size() - 1);
		}
		return true;
	}

}