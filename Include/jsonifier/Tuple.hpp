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

namespace jsonifier_internal {

	template<typename tuple_t> struct group_builder {
	  public:
		static constexpr auto op(tuple_t&& object) {
			return makeGroupsImpl<starts, sizes>(std::forward<tuple_t>(object), std::make_index_sequence<nGroups>{});
		}

	  protected:
		template<size_t... indices> static constexpr auto shrinkIndexArray(auto& valuesNew01, std::index_sequence<indices...>) {
			std::array<size_t, sizeof...(indices)> res{};
			((res[indices] = valuesNew01[indices]), ...);
			return res;
		}

		static constexpr auto filter() {
			constexpr auto n = std::tuple_size_v<tuple_t>;
			std::array<size_t, n> indices{};
			size_t x = 0;
			forEach<n>([&](auto index) {
				using value_type = jsonifier::concepts::unwrap<std::tuple_element_t<index, tuple_t>>;
				if constexpr (!std::convertible_to<value_type, jsonifier::string_view>) {
					indices[x++] = index - 1;
				}
			});
			return std::make_pair(indices, x);
		}

		template<size_t... indicesNew> static constexpr void groupSizesHelper(auto& diffs, auto& indices, std::index_sequence<indicesNew...>) {
			((diffs[indicesNew] = indices[indicesNew + 1] - indices[indicesNew]), ...);
		}

		template<size_t nGroups> static constexpr auto groupSizes(const std::array<size_t, nGroups>& indices, size_t nTotal) {
			std::array<size_t, nGroups> diffs;
			groupSizesHelper(diffs, indices, std::make_index_sequence<nGroups - 1>{});
			diffs[nGroups - 1] = nTotal - indices[nGroups - 1];
			return diffs;
		}

		template<size_t start, size_t... indices> static constexpr auto makeGroup(const tuple_t& t, std::index_sequence<indices...>) {
			auto getElem = [&](auto x) {
				constexpr auto value = decltype(x)::value;
				using type			 = decltype(std::get<start + value>(t));
				if constexpr (value == 0 || std::convertible_to<type, jsonifier::string_view>) {
					return jsonifier::string_view(std::get<start + value>(t));
				} else {
					return std::get<start + value>(t);
				}
			};
			return std::make_tuple(getElem(std::integral_constant<size_t, indices>{})...);
		}

		template<auto& GroupStartArr, auto& GroupSizeArr, size_t... GroupNumber> static constexpr auto makeGroupsImpl(tuple_t&& object, std::index_sequence<GroupNumber...>) {
			auto newTuple{ std::forward<tuple_t>(object) };
			return std::make_tuple(
				makeGroup<GroupStartArr[jsonifier::concepts::tag<GroupNumber>()]>(newTuple, std::make_index_sequence<GroupSizeArr[jsonifier::concepts::tag<GroupNumber>()]>{})...);
		}

		static constexpr auto makeGroupsHelper() {
			constexpr auto nNew		 = std::tuple_size_v<tuple_t>;
			constexpr auto filtered	 = filter();
			constexpr auto startsNew = shrinkIndexArray(filtered.first, std::make_index_sequence<filtered.second>{});
			constexpr auto sizesNew	 = groupSizes(startsNew, nNew);
			return std::make_tuple(startsNew, sizesNew);
		}

		static constexpr auto h		  = makeGroupsHelper();
		static constexpr auto starts  = get<0>(h);
		static constexpr auto sizes	  = get<1>(h);
		static constexpr auto nGroups = starts.size();
	};

}