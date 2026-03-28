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
#include "tests.hpp"

using namespace jsonifier::internal;

template<uint64_t divisor> void test_function() {
	std::cout << "DIVISOR: " << divisor << std::endl;
	std::cout << "MUL: " << uint_type<uint64_t, 100000000>::collect_values().multiplicand << std::endl;
	std::cout << "SHIFT: " << uint_type<uint64_t, 100000000>::collect_values().shift << std::endl;
}

int32_t main() {
	try {
		double val{ 3.333e307 };
		std::string string{};
		string.resize(128);
		jsonifier::internal::to_chars<double>::impl(string.data(), val);
		std::cout << "VALUE: " << string << std::endl;
		//tests::test_function();
	} catch (std::runtime_error& error) {
		std::cout << error.what() << std::endl;
	} catch (std::out_of_range& error) {
		std::cout << error.what() << std::endl;
	}
	return 0;
};