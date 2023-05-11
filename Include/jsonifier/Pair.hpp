/*
	Jsonifier - For parsing and serializing Json - very rapidly.
	Copyright (C) 2023 Chris M. (RealTimeChris)

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	This library is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
	Lesser General Public License for more details.

	You should have received a copy of the GNU Lesser General Public
	License along with this library; if not, Serialize to the Free Software
	Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301
	USA
*/
/// https://github.com/RealTimeChris/Jsonifier
/// Feb 20, 2023
#pragma once

#include <type_traits>
#include <iterator>

namespace JsonifierInternal {

	template<typename FOTy, typename SOTy> class Pair {
	  public:
		using first_type = FOTy;
		using second_type = SOTy;

		FOTy first{};
		SOTy second{};

		inline constexpr Pair() noexcept = default;

		inline constexpr Pair(FOTy&& firstNew, SOTy&& secondNew) {
			first = std::forward<FOTy>(firstNew);
			second = std::forward<SOTy>(secondNew);
		}

		inline constexpr Pair(const FOTy& firstNew, const SOTy& secondNew) {
			first = firstNew;
			second = secondNew;
		}

		inline constexpr void swap(Pair<FOTy, SOTy>& other) noexcept {
			swapF(first, other.first);
			swapF(second, other.second);
		}

		inline constexpr bool operator==(const Pair& other) const noexcept {
			return first == other.first && second == other.second;
		}
	};

	template<typename OTy> using UnwrapRefDecayT = typename std::unwrap_ref_decay<OTy>::type;

	template<typename A, typename B> Pair(A, B) -> Pair<UnwrapRefDecayT<A>, UnwrapRefDecayT<B>>;
}
