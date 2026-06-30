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

#include "parsing_tests.hpp"
#include "conformance.hpp"
#include "bounds_test.hpp"
#include "unit_tests.hpp"
#include "round_trip.hpp"
#include "string.hpp"
#include "float.hpp"
#include "uint.hpp"
#include "int.hpp"

namespace tests {

	inline static void testFunction() {
		parsing_tests::parsingTests();
		conformance_tests::conformanceTests();
		//bounds_tests::boundsTests();
		//round_trip_tests::roundTripTests();
		//string_validation_tests::stringTests();
		//float_validation_tests::floatTests();
		//uint_validation_tests::uintTests();
		//int_validation_tests::intTests();
		//unit_tests::unitTests();
	};

}
