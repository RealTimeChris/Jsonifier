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

namespace jsonifier_internal {

	template<typename FirstType, typename SecondType> class pair {
	  public:
		using first_type  = FirstType;
		using second_type = SecondType;

		first_type first;
		second_type second;

		constexpr pair() = default;

		template<typename FirstTypeNew, typename SecondTypeNew> constexpr pair(FirstTypeNew&& firstNew, SecondTypeNew&& secondNew)
			: first{ std::forward<FirstTypeNew>(firstNew) }, second{ std::forward<SecondTypeNew>(secondNew) } {
		}

		template<typename FirstTypeNew>
			requires(std::same_as<first_type, FirstTypeNew>)
		constexpr pair(FirstTypeNew&& firstNew) : first{ std::forward<FirstTypeNew>(firstNew) } {
		}

		template<typename SecondTypeNew>
			requires(std::same_as<second_type, SecondTypeNew>)
		constexpr pair(SecondTypeNew&& firstNew) : second{ std::forward<SecondTypeNew>(firstNew) } {
		}

		constexpr bool operator==(const pair& other) const {
			return first == other.first && second == other.second;
		}
	};

	template<typename a, typename b> pair(a, b) -> pair<std::unwrap_ref_decay<a>, std::unwrap_ref_decay<b>>;
}
