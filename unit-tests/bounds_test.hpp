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

#include <jsonifier>
#include <filesystem>
#include <fstream>

namespace bounds_tests {

	inline static void boundsTests() {
		std::cout << "Starting bounds-truncation test.\n";
		jsonifier::jsonifier_core<> parser{};
		abc_test_generator<abc_test_struct> tests{};
		abc_partial_test<abc_partial_test_struct> seed{};
		std::string buf;
		parser.serializeJson(tests, buf);
		parser.parseJson(seed, buf);
		seed.z.resize(5);
		seed.a.resize(3);

		auto runTruncation = [&](std::string s, auto&& parseFn, const char* label) {
			if (!parser.validateJson(s)) {
				throw std::runtime_error{ std::string{ label } + ": baseline failed validation" };
			}
			s.pop_back();
			while (!s.empty()) {
				abc_test<abc_test_struct> sink{};
				parseFn(sink, s, label);
				if (parser.validateJson(s)) {
					throw std::runtime_error{ std::string{ label } + ": truncated input validated at size=" + std::to_string(s.size()) };
				}
				s.pop_back();
			}
		};

		std::string minified;
		parser.serializeJson(seed, minified);
		runTruncation(
			minified,
			[&](auto& d, const auto& src, const char* label) {
				parser.parseJson<jsonifier::parse_options{ .minified = true }>(d, src);
				if (parser.getErrors().size() == 0) {
					throw std::runtime_error{ std::string{ label } + ": truncated input validated at size=" + std::to_string(src.size()) };
				}
			},
			"minified");

		std::string pretty;
		parser.serializeJson<jsonifier::serialize_options{ .prettify = true }>(seed, pretty);
		runTruncation(
			pretty,
			[&](auto& d, const auto& src, const char* label) {
				parser.parseJson(d, src);
				if (parser.getErrors().size() == 0) {
					throw std::runtime_error{ std::string{ label } + ": truncated input validated at size=" + std::to_string(src.size()) };
				}
			},
			"prettified");

		std::cout << "Finished bounds-truncation test.\n";
	}

}
