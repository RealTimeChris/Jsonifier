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

	template<typename value_type> using identity_t = value_type;

	template<typename value_type> using type_t = typename value_type::type;

	template<size_t size> using tag_range = std::make_index_sequence<size>;

	template<typename tup> using BaseListT = typename jsonifier::concepts::unwrap<tup>::base_list;

	template<typename... value_type> struct tuple;

	template<typename... value_type> struct type_list {};

	template<typename... l, typename... r> jsonifier_constexpr auto operator+(type_list<l...>, type_list<r...>) {
		return type_list<l..., r...>{};
	}

	template<typename... bases> struct type_map : bases... {
		using base_list = type_list<bases...>;
		using bases::operator[]...;
		using bases::declElem...;
		auto operator<=>(type_map const&) const = default;
		bool operator==(type_map const&) const	= default;
	};

	template<size_t I, typename value_type> struct tuple_elem {
		jsonifier_inline static value_type declElem(jsonifier::concepts::Tag<I>);
		using type = value_type;

		value_type value;

		jsonifier_constexpr decltype(auto) operator[](jsonifier::concepts::Tag<I>) & {
			return (value);
		}

		jsonifier_constexpr decltype(auto) operator[](jsonifier::concepts::Tag<I>) const& {
			return (value);
		}

		jsonifier_constexpr decltype(auto) operator[](jsonifier::concepts::Tag<I>) && {
			return (std::move(*this).value);
		}
		auto operator<=>(tuple_elem const&) const = default;
		bool operator==(tuple_elem const&) const  = default;

		jsonifier_constexpr auto operator<=>(tuple_elem const& other) const noexcept(noexcept(value <=> other.value))
			requires(std::is_reference_v<value_type> && jsonifier::concepts::ordered<value_type>)
		{
			return value <=> other.value;
		}

		jsonifier_constexpr bool operator==(tuple_elem const& other) const noexcept(noexcept(value == other.value))
			requires(std::is_reference_v<value_type> && jsonifier::concepts::equality_comparable<value_type>)
		{
			return value == other.value;
		}
	};

	template<typename a, typename... value_type> struct get_tuple_base;

	template<size_t... I, typename... value_type> struct get_tuple_base<std::index_sequence<I...>, value_type...> {
		using type = type_map<tuple_elem<I, value_type>...>;
	};

	template<typename f, typename value_type, typename... bases> jsonifier_constexpr decltype(auto) applyImpl(f&& fNew, value_type&& object, type_list<bases...>) {
		return static_cast<f&&>(fNew)(static_cast<value_type&&>(object).identity_t<bases>::value...);
	}
	template<char... D> jsonifier_constexpr size_t sizetFromDigits() {
		static_assert(((0x30u <= D && D <= 0x39u) && ...), "Must be integral literal");
		size_t num = 0;
		return ((num = num * 10 + (D - 0x30u)), ..., num);
	}
	template<typename First, typename> using first_t = First;

	template<typename value_type, typename... Q> jsonifier_constexpr auto repeatType(type_list<Q...>) {
		return type_list<first_t<value_type, Q>...>{};
	}
	template<typename... outer> jsonifier_constexpr auto getOuterBases(type_list<outer...>) {
		return (repeatType<outer>(BaseListT<type_t<outer>>{}) + ...);
	}
	template<typename... outer> jsonifier_constexpr auto getInnerBases(type_list<outer...>) {
		return (BaseListT<type_t<outer>>{} + ...);
	}

	template<typename value_type, typename... outer, typename... inner> jsonifier_constexpr auto catImpl(value_type tup, type_list<outer...>, type_list<inner...>)
		-> tuple<type_t<inner>...> {
		return { static_cast<type_t<outer>&&>(tup.identity_t<outer>::value).identity_t<inner>::value... };
	}

	template<typename... value_type> using tuple_base_t = typename get_tuple_base<tag_range<sizeof...(value_type)>, value_type...>::type;

	template<typename... value_type> struct tuple : tuple_base_t<value_type...> {
		static jsonifier_constexpr size_t size = sizeof...(value_type);
		using super							   = tuple_base_t<value_type...>;
		using super::operator[];
		using base_list	   = typename super::base_list;
		using element_list = type_list<value_type...>;
		using super::declElem;

		template<jsonifier::concepts::other_than<tuple> u> jsonifier_constexpr auto& operator=(u&& tup) {
			using tuple2 = jsonifier::concepts::unwrap<u>;
			if (jsonifier::concepts::base_list_tuple<tuple2>) {
				eqImpl(static_cast<u&&>(tup), base_list(), typename tuple2::base_list());
			} else {
				eqImpl(static_cast<u&&>(tup), tag_range<size>());
			}
			return *this;
		}

		auto operator<=>(tuple const&) const = default;
		bool operator==(tuple const&) const	 = default;

		template<typename f> jsonifier_constexpr auto map(f&& func) & {
			return mapImpl(base_list(), static_cast<f&&>(func));
		}

		template<typename f> jsonifier_constexpr auto map(f&& func) const& {
			return mapImpl(base_list(), static_cast<f&&>(func));
		}

		template<typename f> jsonifier_constexpr auto map(f&& func) && {
			return static_cast<tuple&&>(*this).mapImpl(base_list(), static_cast<f&&>(func));
		}

	  protected:
		template<typename u, typename... b1, typename... b2> jsonifier_constexpr void eqImpl(u&& uNew, type_list<b1...>, type_list<b2...>) {
			// See:
			// https://developercommunity.visualstudio.com/object/fold-expressions-unreliable-in-171-with-c20/1676476
			(void(b1::value = static_cast<u&&>(uNew).b2::value), ...);
		}

		template<typename u, size_t... I> jsonifier_constexpr void eqImpl(u&& uNew, std::index_sequence<I...>) {
			(void(tuple_elem<I, value_type>::value = get<I>(static_cast<u&&>(uNew))), ...);
		}

		template<typename f, typename... b> jsonifier_constexpr auto mapImpl(type_list<b...>, f&& func) & -> tuple<jsonifier::concepts::unwrap<decltype(func(b::value))>...> {
			return { func(b::value)... };
		}

		template<typename f, typename... b> jsonifier_constexpr auto mapImpl(type_list<b...>, f&& func) const& -> tuple<jsonifier::concepts::unwrap<decltype(func(b::value))>...> {
			return { func(b::value)... };
		}

		template<typename f, typename... b>
		jsonifier_constexpr auto mapImpl(type_list<b...>, f&& func) && -> tuple<jsonifier::concepts::unwrap<decltype(func(static_cast<value_type&&>(b::value)))>...> {
			return { func(static_cast<value_type&&>(b::value))... };
		}
	};

	template<> struct tuple<> : tuple_base_t<> {
		static jsonifier_constexpr size_t size = 0;
		using super							   = tuple_base_t<>;
		using base_list						   = type_list<>;
		using element_list					   = type_list<>;

		template<jsonifier::concepts::other_than<tuple> u>
			requires jsonifier::concepts::stateless<u>
		jsonifier_constexpr auto& operator=(u&&) {
			return *this;
		}

		auto operator<=>(tuple const&) const = default;
		bool operator==(tuple const&) const	 = default;

		template<typename f> jsonifier_constexpr void forEach(f&&) const {
		}

		template<typename f> jsonifier_constexpr bool any(f&&) const {
			return false;
		}

		template<typename f> jsonifier_constexpr bool all(f&&) const {
			return true;
		}

		template<typename f> jsonifier_constexpr auto map(f&&) const {
			return tuple{};
		}
	};

	template<typename... value_types> tuple(value_types...) -> tuple<jsonifier::concepts::unwrap<value_types>...>;

	template<size_t I, jsonifier::concepts::indexable tup> jsonifier_constexpr decltype(auto) get(tup&& tupNew) {
		return static_cast<tup&&>(tupNew)[jsonifier::concepts::Tag<I>()];
	}

	template<typename... value_type> jsonifier_constexpr tuple<value_type&...> tie(value_type&... object) {
		return { object... };
	}

	template<typename f, jsonifier::concepts::base_list_tuple tup> jsonifier_constexpr decltype(auto) apply(f&& func, tup&& tupNew) {
		return applyImpl(static_cast<f&&>(func), static_cast<tup&&>(tupNew), typename jsonifier::concepts::unwrap<tup>::base_list());
	}
	template<typename f, typename a, typename b> jsonifier_constexpr decltype(auto) apply(f&& func, pair<a, b>& pair) {
		return static_cast<f&&>(func)(pair.first, pair.second);
	}
	template<typename f, typename a, typename b> jsonifier_constexpr decltype(auto) apply(f&& func, pair<a, b> const& pair) {
		return static_cast<f&&>(func)(pair.first, pair.second);
	}
	template<typename f, typename a, typename b> jsonifier_constexpr decltype(auto) apply(f&& func, pair<a, b>&& pair) {
		return static_cast<f&&>(func)(std::move(pair).first, std::move(pair).second);
	}

	template<jsonifier::concepts::base_list_tuple... value_type> jsonifier_constexpr auto tupleCat(value_type&&... ts) {
		if jsonifier_constexpr (sizeof...(value_type) == 0) {
			return tuple<>();
		} else {
#if !defined(TUPLET_CAT_BY_FORWARDING_TUPLE)
	#if defined(__clang__)
		#define TUPLET_CAT_BY_FORWARDING_TUPLE 0
	#else
		#define TUPLET_CAT_BY_FORWARDING_TUPLE 1
	#endif
#endif
#if TUPLET_CAT_BY_FORWARDING_TUPLE
			using big_tuple = tuple<value_type&&...>;
#else
			using big_tuple = tuple<jsonifier::concepts::unwrap<value_type>...>;
#endif
			using outer_bases			   = BaseListT<big_tuple>;
			jsonifier_constexpr auto outer = getOuterBases(outer_bases{});
			jsonifier_constexpr auto inner = getInnerBases(outer_bases{});
			return catImpl(big_tuple{ static_cast<value_type&&>(ts)... }, outer, inner);
		}
	}

	template<typename... value_types> jsonifier_constexpr auto makeTuple(value_types&&... args) {
		return tuple<jsonifier::concepts::unwrap<value_types>...>{ static_cast<value_types&&>(args)... };
	}

	template<typename... value_types> jsonifier_constexpr auto copyTuple(value_types... args) {
		return tuple<value_types...>{ args... };
	}

	template<typename... value_type> jsonifier_constexpr auto forwardAstuple(value_type&&... a) {
		return tuple<value_type&&...>{ static_cast<value_type&&>(a)... };
	}

	namespace literals {
		template<char... D> jsonifier_constexpr auto operator""_tag() -> jsonifier::concepts::Tag<sizetFromDigits<D...>()> {
			return {};
		}
	}
}

namespace std {

	template<typename... value_type> struct tuple_size<jsonifier_internal::tuple<value_type...>> : std::integral_constant<size_t, sizeof...(value_type)> {};

	template<size_t I, typename... value_type> struct tuple_element<I, jsonifier_internal::tuple<value_type...>> {
		using type = decltype(jsonifier_internal::tuple<value_type...>::declElem(jsonifier::concepts::Tag<I>()));
	};

	template<typename a, typename b> struct tuple_size<jsonifier_internal::pair<a, b>> : std::integral_constant<size_t, 2> {};

	template<size_t I, typename a, typename b> struct tuple_element<I, jsonifier_internal::pair<a, b>> {
		static_assert(I < 2, "pair only has 2 elements");
		using type = std::conditional_t<I == 0, a, b>;
	};

};

namespace jsonifier_internal {

	template<typename tuple, size_t... Is> auto tupleSplit(tuple&& tupleNew) {
		static jsonifier_constexpr auto size = std::tuple_size_v<tuple>;
		static jsonifier_constexpr auto is	 = std::make_index_sequence<size / 2>{};
		return std::make_pair(tupleSplitImpl<0>(tupleNew, is), tupleSplitImpl<1>(tupleNew, is));
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

	template<typename tuple> jsonifier_constexpr auto filter() {
		jsonifier_constexpr auto n = std::tuple_size_v<tuple>;
		ctime_array<size_t, n> indices{};
		size_t x = 0;
		forEach<n>([&](auto I) {
			using value_type = jsonifier::concepts::unwrap<std::tuple_element_t<I, tuple>>;
			if jsonifier_constexpr (!std::convertible_to<value_type, jsonifier::string_view>) {
				indices[x++] = I - 1;
			}
		});
		return std::make_pair(indices, x);
	}

	template<typename Func, typename tuple> jsonifier_constexpr auto mapTuple(Func&& f, tuple&& tupleNew) {
		jsonifier_constexpr auto size = std::tuple_size_v<jsonifier::concepts::unwrap<tuple>>;
		return mapTuple(f, tupleNew, std::make_index_sequence<size>{});
	}

	template<uint64_t index, uint64_t indexLimit> jsonifier_constexpr void groupSizesHelper(auto& diffs, auto& indices) {
		if jsonifier_constexpr (index < indexLimit) {
			diffs[index] = indices[index + 1] - indices[index];
			groupSizesHelper<index + 1, indexLimit>(diffs, indices);
		}
	}

	template<size_t n_groups> jsonifier_constexpr auto groupSizes(const ctime_array<size_t, n_groups>& indices, size_t n_total) {
		ctime_array<size_t, n_groups> diffs;
		groupSizesHelper<0, n_groups - 1>(diffs, indices);
		diffs[n_groups - 1] = n_total - indices[n_groups - 1];
		return diffs;
	}

	template<size_t Start, typename tuple, size_t... Is> jsonifier_constexpr auto makeGroup(tuple&& object, std::index_sequence<Is...>) {
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

	template<auto& GroupStartArr, auto& GroupSizeArr, typename tuple, size_t... GroupNumber>
	jsonifier_constexpr auto makeGroupsImpl(tuple&& object, std::index_sequence<GroupNumber...>) {
		return copyTuple(makeGroup<get<GroupNumber>(GroupStartArr)>(object, std::make_index_sequence<get<GroupNumber>(GroupSizeArr)>{})...);
	}

	template<typename tuple> jsonifier_constexpr auto makeGroupsHelper() {
		jsonifier_constexpr auto size = std::tuple_size_v<tuple>;

		jsonifier_constexpr auto filtered = filter<tuple>();
		jsonifier_constexpr auto starts	  = shrinkIndexArray<filtered.second>(filtered.first);
		jsonifier_constexpr auto sizes	  = groupSizes(starts, size);

		return makeTuple(starts, sizes);
	}

	template<typename tuple> struct GroupBuilder {
		static jsonifier_constexpr auto h	   = makeGroupsHelper<tuple>();
		static jsonifier_constexpr auto starts = get<0>(h);
		static jsonifier_constexpr auto sizes  = get<1>(h);

		static jsonifier_constexpr auto op(tuple&& object) {
			jsonifier_constexpr auto n_groups = starts.maxSize();
			return makeGroupsImpl<starts, sizes>(std::forward<tuple>(object), std::make_index_sequence<n_groups>{});
		}
	};

}