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
/// Feb 20, 2023
#pragma once

#include <type_traits>

namespace jsonifier_internal {

	template<typename first_type_new, typename second_type_new> class pair {
	  public:
		using first_type  = first_type_new;
		using second_type = second_type_new;

		first_type first;
		second_type second;

		constexpr pair() noexcept = default;

		template<typename first_type_newer, typename second_type_newer> constexpr pair(first_type_newer&& firstNew, second_type_newer&& secondNew)
			: first{ std::forward<first_type_newer>(firstNew) }, second{ std::forward<second_type_newer>(secondNew) } {};

		template<typename = std::enable_if_t<!std::same_as<first_type, second_type>>> constexpr pair(first_type&& value) : first{ std::forward<first_type>(value) }, second{} {};
		constexpr pair(second_type&& value) : first{}, second{ std::forward<second_type>(value) } {};

		template<typename = std::enable_if_t<!std::same_as<first_type, second_type>>> constexpr pair(const first_type& value) : first{ value }, second{} {};
		constexpr pair(const second_type& value) : first{}, second{ value } {};

		constexpr bool operator==(const pair& other) const {
			return first == other.first && second == other.second;
		}
	};

	template<class value_type01, class value_type02> constexpr pair<jsonifier::concepts::unwrap<value_type01>, jsonifier::concepts::unwrap<value_type02>> makePair(
		value_type01&& _Val1, value_type02&& _Val2) noexcept(std::is_nothrow_constructible_v<jsonifier::concepts::unwrap<value_type01>, value_type01>&&
			std::is_nothrow_constructible_v<jsonifier::concepts::unwrap<value_type02>, value_type02>) {
		using pair_type = pair<jsonifier::concepts::unwrap<value_type01>, jsonifier::concepts::unwrap<value_type02>>;
		return pair_type(std::forward<value_type01>(_Val1), std::forward<value_type02>(_Val2));
	}

	template<typename a, typename b> pair(a, b) -> pair<std::unwrap_ref_decay<a>, std::unwrap_ref_decay<b>>;
}
