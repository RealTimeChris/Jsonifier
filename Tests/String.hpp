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

#include <jsonifier/Index.hpp>
#include <filesystem>
#include <fstream>

namespace string_validation_tests {

	constexpr jsonifier::internal::array<std::string_view, 9> stringViews01 = { "\"\"", "\"Hello\"", "\"Hello\\nWorld\"", "\"Hello\u0000World\"", "\"\\\"\\\\/\\b\\f\\n\\r\\t\"",
		"\"\\u0024\"", "\"\\u00A2\"", "\"\\u20AC\"", "\"\\uD834\\uDD1E\"" };

	constexpr jsonifier::internal::array<std::string_view, 9> stringValues = { "", "Hello", "Hello\nWorld", "Hello\0World", "\"\\/\b\f\n\r\t", "\x24", "\xC2\xA2", "\xE2\x82\xAC",
		"\xF0\x9D\x84\x9E" };

	template<rt_ut::string_literal test_suite, const auto& pass_tests, const auto& pass_values> struct pass_string_tests_runner {
		template<uint64_t index> static void impl(jsonifier::jsonifier_core<>& parser) {
			rt_ut::unit_test<test_suite + rt_ut::string_literal{ rt_ut::create_string_literal<pass_tests[index].size()>(pass_tests[index].data()) }, true>::assert_eq(
				pass_values[index], [&]() {
					static std::string data{};
					parser.parseJson(data, pass_tests[index]);
					for (auto& value: parser.getErrors()) {
						std::cout << "Jsonifier Error: " << value << std::endl;
					}
					return static_cast<std::string_view>(data);
				});
		}
	};

	void stringTests() {
		pass_test_runner<"String Pass Tests: ", stringViews01, stringValues, pass_string_tests_runner, std::make_integer_sequence<uint64_t, stringViews01.size()>>::impl();
		return;
	}

}
