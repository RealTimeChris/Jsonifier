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
/// https://github.com/RealTimeChris/Jsonifier
/// Feb 20, 2023
#pragma once

#include <type_traits>
#include <iterator>

namespace JsonifierInternal {

	template<typename FirstType, typename SecondType> class Pair {
	  public:
		using first_type = FirstType;
		using second_type = SecondType;

		FirstType first{};
		SecondType second{};

		constexpr Pair() noexcept = default;

		constexpr Pair(FirstType&& firstNew, SecondType&& secondNew) {
			first = std::forward<FirstType>(firstNew);
			second = std::forward<SecondType>(secondNew);
		}

		constexpr Pair(const FirstType& firstNew, const SecondType& secondNew) {
			first = firstNew;
			second = secondNew;
		}

		constexpr void swap(Pair<FirstType, SecondType>& other) noexcept {
			swapF(first, other.first);
			swapF(second, other.second);
		}

		constexpr bool operator==(const Pair& other) const noexcept {
			return first == other.first && second == other.second;
		}
	};

	template<typename ValueType> using UnwrapRefDecayT = typename std::unwrap_ref_decay<ValueType>::type;

	template<typename A, typename B> Pair(A, B) -> Pair<UnwrapRefDecayT<A>, UnwrapRefDecayT<B>>;
}
