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
/// Feb 3, 2023
#pragma once

#include <jsonifier/TypeEntities.hpp>

namespace jsonifier_internal {

	constexpr std::array<json_structural_type, 256> asciiClassesMap{ [] {
		std::array<json_structural_type, 256> returnValues{};
		for (uint64_t x = 0; x < 256; ++x) {
			returnValues[x] = json_structural_type::Unset;
		}
		returnValues['"'] = json_structural_type::String;
		returnValues['t'] = json_structural_type::Bool;
		returnValues['f'] = json_structural_type::Bool;
		returnValues['n'] = json_structural_type::Null;
		returnValues['0'] = json_structural_type::Number;
		returnValues['1'] = json_structural_type::Number;
		returnValues['2'] = json_structural_type::Number;
		returnValues['3'] = json_structural_type::Number;
		returnValues['4'] = json_structural_type::Number;
		returnValues['5'] = json_structural_type::Number;
		returnValues['6'] = json_structural_type::Number;
		returnValues['7'] = json_structural_type::Number;
		returnValues['8'] = json_structural_type::Number;
		returnValues['9'] = json_structural_type::Number;
		returnValues['-'] = json_structural_type::Number;
		returnValues['['] = json_structural_type::Array_Start;
		returnValues[']'] = json_structural_type::Array_End;
		returnValues['{'] = json_structural_type::Object_Start;
		returnValues['}'] = json_structural_type::Object_End;
		returnValues[':'] = json_structural_type::Colon;
		returnValues[','] = json_structural_type::Comma;
		return returnValues;
	}() };

}// namespace jsonifier_internal