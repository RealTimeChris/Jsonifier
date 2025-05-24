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
	static constexpr auto parseValue = createValue<&value_type::m, &value_type::s>();
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

	bool boundsTests() noexcept {
		jsonifier::jsonifier_core<> parser{};
		test_generator<test_struct> tests{};
		partial_test<partial_test_struct> newTests{};
		std::string testString{};
		parser.serializeJson(tests, testString);
		parser.parseJson(newTests, testString);
		newTests.m.resize(5);
		newTests.s.resize(3);
		parser.serializeJson(newTests, testString);
		std::cout << "TEST STRING LENGTH: " << testString.size() << std::endl;
		if (!parser.validateJson(testString)) {
			throw std::runtime_error{ "Failed to pass validation!" };
		}
		testString.resize(testString.size() - 1);
		while (testString.size() > 0) {
			test<test_struct> newData{};
			parser.parseJson<jsonifier::parse_options{ .minified = true }>(newData, testString);
			if (parser.validateJson(testString)) {
				throw std::runtime_error{ "Failed to fail validation!" };
			}
			testString.resize(testString.size() - 1);
		}
		parser.serializeJson<jsonifier::serialize_options{ .prettify = true }>(newTests, testString);
		if (!parser.validateJson(testString)) {
			throw std::runtime_error{ "Failed to pass validation!" };
		}
		testString.resize(testString.size() - 1);
		while (testString.size() > 0) {
			test<test_struct> newData{};
			parser.parseJson(newData, testString);
			if (parser.validateJson(testString)) {
				throw std::runtime_error{ "Failed to fail validation!" };
			}
			testString.resize(testString.size() - 1);
		}
		std::cout << "Succesfully passed the bounds test." << std::endl;
		return true;
	}

	uint64_t getUniquelyModifiedIndex(std::vector<bool>& alreadyTakenIndices) {
		uint64_t newIndex = test_generator<uint64_t>::generate<uint64_t>() % alreadyTakenIndices.size();
		while (alreadyTakenIndices[newIndex % alreadyTakenIndices.size()] && newIndex % alreadyTakenIndices.size() < alreadyTakenIndices.size() - 1) {
			++newIndex;
		}
		alreadyTakenIndices[newIndex % alreadyTakenIndices.size()] = true;
		return newIndex % alreadyTakenIndices.size();
	}

	bool mutationTests() noexcept {
		jsonifier::jsonifier_core<> parser{};
		test_generator<test_struct> tests{};
		partial_test<partial_test_struct> newTests{};
		std::string testString{};
		parser.serializeJson(tests, testString);
		parser.parseJson(newTests, testString);
		newTests.m.resize(5);
		newTests.s.resize(3);
		parser.serializeJson(newTests, testString);
		std::cout << "TEST STRING LENGTH: " << testString.size() << std::endl;
		if (!parser.validateJson(testString)) {
			throw std::runtime_error{ "Failed to pass validation!" };
		}
		testString.resize(testString.size() - 1);
		std::vector<bool> alreadyTakenIndices{};
		alreadyTakenIndices.resize(testString.size());
		size_t newIndex		 = getUniquelyModifiedIndex(alreadyTakenIndices);
		testString[newIndex] = static_cast<char>(test_generator<char>::generate<int64_t>());
		for (size_t index = 0; index < testString.size(); ++index) {
			test<test_struct> newData{};
			parser.parseJson<jsonifier::parse_options{ .minified = true }>(newData, testString);
			if (parser.validateJson(testString)) {
				throw std::runtime_error{ "Failed to fail validation!" };
			}
			size_t newIndex											  = getUniquelyModifiedIndex(alreadyTakenIndices);
			testString[newIndex] = static_cast<char>(test_generator<char>::generate<int64_t>());
		}
		for (auto& value: parser.getErrors()) {
			std::cout << "Jsonifier Error: " << value << std::endl;
		}
		parser.serializeJson<jsonifier::serialize_options{ .prettify = true }>(newTests, testString);
		if (!parser.validateJson(testString)) {
			throw std::runtime_error{ "Failed to pass validation!" };
		}
		testString.resize(testString.size() - 1);
		alreadyTakenIndices = {};
		alreadyTakenIndices.resize(testString.size());
		for (size_t index = 0; index < testString.size(); ++index) {
			test<test_struct> newData{};
			parser.parseJson(newData, testString);
			if (parser.validateJson(testString)) {
				throw std::runtime_error{ "Failed to fail validation!" };
			}
			testString[getUniquelyModifiedIndex(alreadyTakenIndices)] = static_cast<char>(test_generator<char>::generate<int64_t>());
		}
		for (auto& value: parser.getErrors()) {
			std::cout << "Jsonifier Error: " << value << std::endl;
		}
		std::cout << "Succesfully passed the mutation test." << std::endl;
		return true;
	}

}