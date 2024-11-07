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

#include <type_traits>
#include <cstdint>
#include <utility>

namespace jsonifier_internal {

	template<typename... types> struct tuple {
		constexpr tuple() noexcept = default;
	};

	template<typename first_type, typename... types> struct tuple<first_type, types...> {
		constexpr tuple() noexcept = default;
		template<typename first_type_new, typename... types_new> constexpr tuple(first_type_new&& argOne, types_new&&... args)
			: restVals{ std::forward<types_new>(args)... }, val{ std::forward<first_type_new>(argOne) } {};
		tuple<types...> restVals{};
		first_type val{};
	};

	template<typename first_type> struct tuple<first_type> {
		constexpr tuple() noexcept = default;
		template<typename first_type_new> constexpr tuple(first_type_new&& argOne) : val{ std::forward<first_type_new>(argOne) } {
		}
		first_type val{};
	};

	template<typename... types> tuple(types&&...) -> tuple<types...>;
	template<typename... types, typename first_type> tuple(types&&..., first_type&&) -> tuple<types..., first_type>;
	template<typename first_type> tuple(first_type&&) -> tuple<first_type>;

	template<size_t index, typename tuple_type>
		requires(index == 0)
	JSONIFIER_ALWAYS_INLINE static constexpr auto& get(tuple_type&& t) {
		return std::forward<tuple_type>(t).val;
	};

	template<size_t index, typename tuple_type> JSONIFIER_ALWAYS_INLINE static constexpr auto& get(tuple_type&& t) {
		return get<index - 1>(std::forward<tuple_type>(t).restVals);
	};

	template<typename... types> struct tuple_size;

	template<typename... types> struct tuple_size<tuple<types...>> {
		static constexpr size_t value = sizeof...(types);
	};

	template<typename... types> struct tuple_size<std::tuple<types...>> {
		static constexpr size_t value = sizeof...(types);
	};

	template<typename value_type> constexpr size_t tuple_size_v = tuple_size<std::remove_const_t<value_type>>::value;

	template<typename... types> constexpr auto makeTuple(types&&... args) {
		return tuple<std::remove_cvref_t<types>...>{ std::forward<types>(args)... };
	}

	template<typename... tuples> struct concat_tuples;

	template<typename... types1, typename... types2> struct concat_tuples<tuple<types1...>, tuple<types2...>> {
		using type = tuple<types1..., types2...>;
	};

	template<typename tuple1, typename tuple2, typename... rest> struct concat_tuples<tuple1, tuple2, rest...> {
		using type = typename concat_tuples<typename concat_tuples<tuple1, tuple2>::type, rest...>::type;
	};

	template<typename... tuples> using concat_tuples_t = typename concat_tuples<tuples...>::type;

	template<typename tuple_type01, size_t... indices1, typename tuple_type02, size_t... indices2>
	constexpr auto tupleCatImpl(tuple_type01&& tuple1, std::index_sequence<indices1...>, tuple_type02&& tuple2, std::index_sequence<indices2...>) noexcept {
		return concat_tuples_t<std::remove_cvref_t<tuple_type01>, std::remove_cvref_t<tuple_type02>>{ get<indices1>(tuple1)..., get<indices2>(tuple2)... };
	}

	template<typename tuple_type01, typename tuple_type02> constexpr auto tupleCat(tuple_type01&& tuple1, tuple_type02&& tuple2) noexcept {
		return tupleCatImpl(std::forward<tuple_type01>(tuple1), std::make_index_sequence<tuple_size_v<std::remove_cvref_t<tuple_type01>>>{}, std::forward<tuple_type02>(tuple2),
			std::make_index_sequence<tuple_size_v<std::remove_cvref_t<tuple_type02>>>{});
	}
}