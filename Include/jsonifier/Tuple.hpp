/*
	MIT License

	Copyright (c) 2023 RealTimeChris

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

namespace jsonifier_internal {

	template<uint64_t currentIndex, uint64_t maxIndex, typename tuple_type, typename arg_type01, typename arg_type02, typename... arg_types>
	constexpr auto generateInterleavedTuple(const tuple_type& newTuple, const arg_type01& arg01, const arg_type02& arg02, const arg_types&... args) noexcept {
		if constexpr (std::tuple_size_v<tuple_type> > 0) {
			if constexpr (currentIndex < maxIndex - 2) {
				auto newerPair	= makeDataMemberAuto(arg01, arg02);
				auto newerTuple = std::tuple_cat(newTuple, std::make_tuple(newerPair));
				return generateInterleavedTuple<currentIndex + 2, maxIndex>(newerTuple, args...);
			} else {
				auto newerPair	= makeDataMemberAuto(arg01, arg02);
				auto newerTuple = std::tuple_cat(newTuple, std::make_tuple(newerPair));
				return newerTuple;
			}
		} else {
			if constexpr (currentIndex < maxIndex - 2) {
				auto newerPair	= makeDataMemberAuto(arg01, arg02);
				auto newerTuple = std::make_tuple(newerPair);
				return generateInterleavedTuple<currentIndex + 2, maxIndex>(newerTuple, args...);
			} else {
				auto newerPair	= makeDataMemberAuto(arg01, arg02);
				auto newerTuple = std::make_tuple(newerPair);
				return newerTuple;
			}
		}
	}

	template<typename member_type, typename value_type> JSONIFIER_ALWAYS_INLINE constexpr decltype(auto) getMember(member_type&& member_ptr, value_type&& value) {
		using value_type02 = unwrap_t<member_type>;
		if constexpr (std::is_member_object_pointer_v<value_type02>) {
			return std::forward<value_type>(value).*std::forward<member_type>(member_ptr);
		} else if constexpr (std::is_pointer_v<value_type02>) {
			return *std::forward<member_type>(member_ptr);
		} else {
			return std::forward<member_type>(member_ptr);
		}
	}

	template<auto member_ptr, typename value_type> JSONIFIER_ALWAYS_INLINE constexpr decltype(auto) getMember(value_type&& value) {
		using value_type02 = unwrap_t<decltype(member_ptr)>;
		if constexpr (std::is_member_object_pointer_v<value_type02>) {
			return std::forward<value_type>(value).*member_ptr;
		} else if constexpr (std::is_pointer_v<value_type02>) {
			return *member_ptr;
		} else {
			return member_ptr;
		}
	}
}

namespace jsonifier {

	template<typename... arg_types> constexpr auto createValue(arg_types&&... args) noexcept {
		if constexpr (sizeof...(arg_types) > 0 && sizeof...(arg_types) % 2 == 0) {
			return value{ jsonifier_internal::generateInterleavedTuple<0, sizeof...(arg_types)>(std::make_tuple(), std::forward<arg_types>(args)...) };
		} else if constexpr (sizeof...(arg_types) > 1 && (sizeof...(arg_types) % 2) != 0) {
			static_assert(sizeof...(arg_types) % 2 == 0, "Sorry, but please pass the correct amount of arguments to createValue()");
		} else if constexpr (sizeof...(arg_types) == 1) {
			static_assert(std::is_member_pointer_v<arg_types...>, "Sorry but please only pass a memberPtr if there is only one argument to createValue().");
			return scalar_value{ std::make_tuple(std::forward<arg_types>(args)...) };
		} else {
			return value{ concepts::empty{} };
		}
	}

	/*
	* Function to create a reflected value from member pointers
	*/
	template<auto... values> constexpr auto createValue() noexcept {
		// Check if there are member pointers specified
		if constexpr (sizeof...(values) > 0) {
			// Create a tuple with the specified member pointers
			constexpr auto newTuple = std::make_tuple(values...);

			// Get the names of the specified member pointers
			constexpr auto memberNames = jsonifier_internal::getNames<values...>();

			// Generate an interleaved tuple of member names and values
			return value{ jsonifier_internal::generateInterleavedTuple(newTuple, memberNames) };
		} else {
			// If no member pointers are specified, create a reflected value with an empty placeholder
			return value{ concepts::empty{} };
		}
	}

}