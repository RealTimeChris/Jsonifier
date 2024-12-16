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

#include <jsonifier/Reflection.hpp>

namespace jsonifier {

	template<auto testPtr, jsonifier_internal::string_literal nameNew> constexpr auto make_json_entity() {
		constexpr jsonifier_internal::string_literal name{ nameNew };
		return jsonifier_internal::makeJsonEntityAuto(jsonifier_internal::make_static<name>::value.view(), testPtr);
	}

	template<auto testPtr> constexpr auto make_json_entity() {
		return jsonifier_internal::makeJsonEntityAuto(jsonifier_internal::getName<testPtr>(), testPtr);
	}

}

namespace jsonifier_internal {

	template<typename value_type>
	concept convertible_to_json_entity = requires(std::remove_cvref_t<value_type> value) {
		{ static_cast<json_entity<typename decltype(json_entity{ value })::member_type, typename decltype(json_entity{ value })::class_type>>(value) };
	};
}

namespace jsonifier {
	
	template<auto... values> constexpr auto createValue() noexcept {
		static_assert((jsonifier_internal::convertible_to_json_entity<decltype(values)> && ...),
			"Sorry, but all arguments passed to createValue should be convertible to or a json_entity.");
		constexpr jsonifier_internal::tuple newTuple01{ jsonifier_internal::makeTuple(jsonifier_internal::makeJsonEntityAuto<values>()...) };
		return jsonifier::value{ newTuple01 };
	}

}