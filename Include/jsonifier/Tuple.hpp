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
/// NOTE: Most of the code in this header was sampled from Glaze library: https://github.com/stephenberry/glaze
/// https://github.com/RealTimeChris/jsonifier
/// Feb 3, 2023
#pragma once

#include <jsonifier/TypeEntities.hpp>
#include <jsonifier/CTimeArray.hpp>
#include <jsonifier/Pair.hpp>

namespace jsonifier_internal {

	template<typename... value_types> jsonifier_constexpr auto copyTuple(value_types... args) {
		return std::tuple<value_types...>{ args... };
	}

	template<uint64_t index, uint64_t indexLimit> jsonifier_constexpr void shrinkIndexArrayHelper(auto& arrayNew01, auto& arrayNew00) {
		if jsonifier_constexpr (index < indexLimit) {
			arrayNew01[index] = arrayNew00[index];
			shrinkIndexArrayHelper<index + 1, indexLimit>(arrayNew01, arrayNew00);
		}
	}

	template<size_t size> jsonifier_constexpr auto shrinkIndexArray(auto& arrayNew01) {
		ctime_array<size_t, size> res{};
		shrinkIndexArrayHelper<0, size>(res, arrayNew01);
		return res;
	}

	template<typename tuple_t> jsonifier_constexpr auto filter() {
		jsonifier_constexpr auto n = std::tuple_size_v<tuple_t>;
		ctime_array<size_t, n> indices{};
		size_t x = 0;
		forEach<n>([&](auto I) {
			using value_type = jsonifier::concepts::unwrap<std::tuple_element_t<I, tuple_t>>;
			if jsonifier_constexpr (!std::convertible_to<value_type, jsonifier::string_view>) {
				indices[x++] = I - 1;
			}
		});
		return std::make_pair(indices, x);
	}
  
	template<uint64_t index, uint64_t indexLimit> jsonifier_constexpr void groupSizesHelper(auto& diffs, auto& indices) {
		if jsonifier_constexpr (index < indexLimit) {
			diffs[index] = indices[index + 1] - indices[index];
			groupSizesHelper<index + 1, indexLimit>(diffs, indices);
		}
	}

	template<size_t nGroups> jsonifier_constexpr auto groupSizes(const ctime_array<size_t, nGroups>& indices, size_t n_total) {
		ctime_array<size_t, nGroups> diffs;
		groupSizesHelper<0, nGroups - 1>(diffs, indices);
		diffs[nGroups - 1] = n_total - indices[nGroups - 1];
		return diffs;
	}

	template<size_t Start, typename tuple_t, size_t... Is> jsonifier_constexpr auto makeGroup(tuple_t&& object, std::index_sequence<Is...>) {
		auto get_elem = [&](auto x) {
			jsonifier_constexpr auto I = decltype(x)::value;
			if jsonifier_constexpr (I == 1) {
				return get<Start + I>(object);
			} else {
				return jsonifier::string_view(get<Start + I>(object));
			}
		};
		auto r = copyTuple(get_elem(std::integral_constant<size_t, Is>{})...);
		return r;
	}

	template<auto& GroupStartArr, auto& GroupSizeArr, typename tuple_t, size_t... GroupNumber>
	jsonifier_constexpr auto makeGroupsImpl(tuple_t&& object, std::index_sequence<GroupNumber...>) {
		return copyTuple(
			makeGroup<GroupStartArr[jsonifier::concepts::Tag<GroupNumber>()]>(object, std::make_index_sequence<GroupSizeArr[jsonifier::concepts::Tag<GroupNumber>()]>{})...);
	}

	template<typename tuple_t> jsonifier_constexpr auto makeGroupsHelper() {
		jsonifier_constexpr auto size = std::tuple_size_v<tuple_t>;

		jsonifier_constexpr auto filtered = filter<tuple_t>();
		jsonifier_constexpr auto starts	  = shrinkIndexArray<filtered.second>(filtered.first);
		jsonifier_constexpr auto sizes	  = groupSizes(starts, size);

		return std::tuple(starts, sizes);
	}

	template<typename tuple_t> struct GroupBuilder {
		static jsonifier_constexpr auto h	   = makeGroupsHelper<tuple_t>();
		static jsonifier_constexpr auto starts = get<0>(h);
		static jsonifier_constexpr auto sizes  = get<1>(h);

		static jsonifier_constexpr auto op(tuple_t&& object) {
			jsonifier_constexpr auto nGroups = starts.maxSize();
			return makeGroupsImpl<starts, sizes>(std::forward<tuple_t>(object), std::make_index_sequence<nGroups>{});
		}
	};

}