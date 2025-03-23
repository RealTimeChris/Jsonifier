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

template<> struct jsonifier::core<test_struct> {
	using value_type				 = test_struct;
	static constexpr auto parseValue = createValue<&value_type::testUint, &value_type::testBool, &value_type::testString, &value_type::testInt, &value_type::testDouble>();
};

template<> struct jsonifier::core<partial_test_struct> {
	using value_type				 = partial_test_struct;
	static constexpr auto parseValue = createValue<&value_type::testBool, &value_type::testString>();
};

template<typename value_type_new> struct jsonifier::core<partial_test<value_type_new>> {
	using value_type				 = partial_test<value_type_new>;
	static constexpr auto parseValue = createValue<&value_type::m>();
};

template<typename value_type_new> struct jsonifier::core<test_generator<value_type_new>> {
	using value_type				 = test_generator<value_type_new>;
	static constexpr auto parseValue = createValue<&value_type::a, &value_type::b, &value_type::c, &value_type::d, &value_type::e, &value_type::f, &value_type::g, &value_type::h,
		&value_type::i, &value_type::j, &value_type::k, &value_type::l, &value_type::m, &value_type::n, &value_type::o, &value_type::p, &value_type::q, &value_type::r,
		&value_type::s, &value_type::t, &value_type::u, &value_type::v, &value_type::w, &value_type::x, &value_type::y, &value_type::z>();
};

template<typename value_type_new> struct jsonifier::core<test<value_type_new>> {
	using value_type				 = test<value_type_new>;
	static constexpr auto parseValue = createValue<&value_type::a, &value_type::b, &value_type::c, &value_type::d, &value_type::e, &value_type::f, &value_type::g, &value_type::h,
		&value_type::i, &value_type::j, &value_type::k, &value_type::l, &value_type::m, &value_type::n, &value_type::o, &value_type::p, &value_type::q, &value_type::r,
		&value_type::s, &value_type::t, &value_type::u, &value_type::v, &value_type::w, &value_type::x, &value_type::y, &value_type::z>();
};

namespace bounds_tests {

	void boundsTests() {
		std::cout << "Starting bounds-truncation test.\n";
		jsonifier::jsonifier_core<> parser{};
		test_generator<test_struct> tests{};
		partial_test<partial_test_struct> seed{};
		std::string buf;
		parser.serializeJson(tests, buf);
		parser.parseJson(seed, buf);
		seed.m.resize(5);
		seed.s.resize(3);

		auto runTruncation = [&](std::string s, auto&& parseFn, const char* label) {
			if (!parser.validateJson(s)) {
				throw std::runtime_error{ std::string{ label } + ": baseline failed validation" };
			}
			s.pop_back();
			while (!s.empty()) {
				test<test_struct> sink{};
				parseFn(sink, s);
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
			[&](auto& d, const auto& src) {
				parser.parseJson<jsonifier::parse_options{ .minified = true }>(d, src);
			},
			"minified");

		std::string pretty;
		parser.serializeJson<jsonifier::serialize_options{ .prettify = true }>(seed, pretty);
		runTruncation(
			pretty,
			[&](auto& d, const auto& src) {
				parser.parseJson(d, src);
			},
			"prettified");

		std::cout << "Finished bounds-truncation test.\n";
	}

}