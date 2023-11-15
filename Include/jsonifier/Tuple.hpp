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
#include <jsonifier/Pair.hpp>

namespace jsonifier_internal {

	template<typename tuple_t> struct GroupBuilder {
	  public:
		static constexpr auto op(tuple_t&& object) {
			return makeGroupsImpl<starts, sizes>(std::forward<tuple_t>(object), std::make_index_sequence<nGroups>{});
		}

	  protected:
		template<uint64_t index, uint64_t indexLimit> static constexpr void shrinkIndexArrayHelper(auto& arrayNew01, auto& arrayNew00) {
			if constexpr (index < indexLimit) {
				arrayNew01[index] = arrayNew00[index];
				shrinkIndexArrayHelper<index + 1, indexLimit>(arrayNew01, arrayNew00);
			}
		}

		template<size_t size> static constexpr auto shrinkIndexArray(auto& arrayNew01) {
			ctime_array<size_t, size> res{};
			shrinkIndexArrayHelper<0, size>(res, arrayNew01);
			return res;
		}

		static constexpr auto filter() {
			constexpr auto n = std::tuple_size_v<tuple_t>;
			ctime_array<size_t, n> indices{};
			size_t x = 0;
			forEach<n>([&](auto index) {
				using value_type = jsonifier::concepts::unwrap<std::tuple_element_t<index, tuple_t>>;
				if constexpr (!std::convertible_to<value_type, jsonifier::string_view>) {
					indices[x++] = index - 1;
				}
			});
			return makePair(indices, x);
		}

		template<uint64_t index, uint64_t indexLimit> static constexpr void groupSizesHelper(auto& diffs, auto& indices) {
			if constexpr (index < indexLimit) {
				diffs[index] = indices[index + 1] - indices[index];
				groupSizesHelper<index + 1, indexLimit>(diffs, indices);
			}
		}

		template<size_t nGroups> static constexpr auto groupSizes(const ctime_array<size_t, nGroups>& indices, size_t n_total) {
			ctime_array<size_t, nGroups> diffs;
			groupSizesHelper<0, nGroups - 1>(diffs, indices);
			diffs[nGroups - 1] = n_total - indices[nGroups - 1];
			return diffs;
		}

		template<size_t start, size_t... indices> static constexpr auto makeGroup(tuple_t&& t, std::index_sequence<indices...>) {
			auto getElem = [&](auto i) {
				constexpr auto x = decltype(i)::value;
				using type		 = decltype(std::get<start + x>(t));
				if constexpr (x == 0 || std::convertible_to<type, jsonifier::string_view>) {
					return jsonifier::string_view(std::get<start + x>(t));
				} else {
					return std::get<start + x>(t);
				}
			};
			return std::make_tuple(getElem(std::integral_constant<size_t, indices>{})...);
		}

		template<auto& GroupStartArr, auto& GroupSizeArr, size_t... GroupNumber> static constexpr auto makeGroupsImpl(tuple_t&& object, std::index_sequence<GroupNumber...>) {
			return std::make_tuple(makeGroup<GroupStartArr[jsonifier::concepts::tag<GroupNumber>()]>(std::forward<tuple_t>(object),
				std::make_index_sequence<GroupSizeArr[jsonifier::concepts::tag<GroupNumber>()]>{})...);
		}

		static constexpr auto makeGroupsHelper() {
			constexpr auto nNew		 = std::tuple_size_v<tuple_t>;
			constexpr auto filtered	 = filter();
			constexpr auto startsNew = shrinkIndexArray<filtered.second>(filtered.first);
			constexpr auto sizesNew	 = groupSizes(startsNew, nNew);
			return std::make_tuple(startsNew, sizesNew);
		}

		static constexpr auto h		  = makeGroupsHelper();
		static constexpr auto starts  = get<0>(h);
		static constexpr auto sizes	  = get<1>(h);
		static constexpr auto nGroups = starts.size();
	};

}