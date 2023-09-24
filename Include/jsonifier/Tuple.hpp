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

#include <jsonifier/Concepts.hpp>
#include <jsonifier/RawArray.hpp>
#include <jsonifier/Pair.hpp>
#include <type_traits>
#include <concepts>

namespace jsonifier_internal {

	namespace tuplet {

		template<typename value_type> using identity_t = value_type;

		template<typename value_type> using type_t = typename value_type::type;

		template<size_t N> using tag_range = std::make_index_sequence<N>;

		template<typename tup> using BaseListT = typename ref_unwrap<tup>::base_list;

		template<typename... value_type> struct tuple;

		template<typename... value_type> struct type_list {};

		template<typename... l, typename... r> constexpr auto operator+(type_list<l...>, type_list<r...>) {
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
			inline static value_type declElem(Tag<I>);
			using type = value_type;

			value_type value;

			constexpr decltype(auto) operator[](Tag<I>) & {
				return (value);
			}

			constexpr decltype(auto) operator[](Tag<I>) const& {
				return (value);
			}

			constexpr decltype(auto) operator[](Tag<I>) && {
				return (std::move(*this).value);
			}
			auto operator<=>(tuple_elem const&) const = default;
			bool operator==(tuple_elem const&) const  = default;

			constexpr auto operator<=>(tuple_elem const& other) const noexcept(noexcept(value <=> other.value))
				requires(std::is_reference_v<value_type> && ordered<value_type>)
			{
				return value <=> other.value;
			}

			constexpr bool operator==(tuple_elem const& other) const noexcept(noexcept(value == other.value))
				requires(std::is_reference_v<value_type> && equality_comparable<value_type>)
			{
				return value == other.value;
			}
		};

		template<typename a, typename... value_type> struct get_tuple_base;

		template<size_t... I, typename... value_type> struct get_tuple_base<std::index_sequence<I...>, value_type...> {
			using type = type_map<tuple_elem<I, value_type>...>;
		};

		template<typename f, typename value_type, typename... bases> constexpr decltype(auto) applyImpl(f&& fNew, value_type&& object, type_list<bases...>) {
			return static_cast<f&&>(fNew)(static_cast<value_type&&>(object).identity_t<bases>::value...);
		}
		template<char... D> constexpr size_t sizetFromDigits() {
			static_assert((('0' <= D && D <= '9') && ...), "Must be integral literal");
			size_t num = 0;
			return ((num = num * 10 + (D - '0')), ..., num);
		}
		template<typename First, typename> using first_t = First;

		template<typename value_type, typename... Q> constexpr auto repeatType(type_list<Q...>) {
			return type_list<first_t<value_type, Q>...>{};
		}
		template<typename... outer> constexpr auto getOuterBases(type_list<outer...>) {
			return (repeatType<outer>(BaseListT<type_t<outer>>{}) + ...);
		}
		template<typename... outer> constexpr auto getInnerBases(type_list<outer...>) {
			return (BaseListT<type_t<outer>>{} + ...);
		}

		template<typename value_type, typename... outer, typename... inner> constexpr auto catImpl(value_type tup, type_list<outer...>, type_list<inner...>)
			-> tuple<type_t<inner>...> {
			return { static_cast<type_t<outer>&&>(tup.identity_t<outer>::value).identity_t<inner>::value... };
		}

		template<typename... value_type> using tuple_base_t = typename get_tuple_base<tag_range<sizeof...(value_type)>, value_type...>::type;

		template<typename... value_type> struct tuple : tuple_base_t<value_type...> {
			static constexpr size_t N = sizeof...(value_type);
			using super				  = tuple_base_t<value_type...>;
			using super::operator[];
			using base_list	   = typename super::base_list;
			using element_list = type_list<value_type...>;
			using super::declElem;

			template<other_than<tuple> u> constexpr auto& operator=(u&& tup) {
				using tuple2 = ref_unwrap<u>;
				if (base_list_tuple<tuple2>) {
					eqImpl(static_cast<u&&>(tup), base_list(), typename tuple2::base_list());
				} else {
					eqImpl(static_cast<u&&>(tup), tag_range<N>());
				}
				return *this;
			}

			auto operator<=>(tuple const&) const = default;
			bool operator==(tuple const&) const	 = default;

			template<typename f> constexpr auto map(f&& func) & {
				return mapImpl(base_list(), static_cast<f&&>(func));
			}

			template<typename f> constexpr auto map(f&& func) const& {
				return mapImpl(base_list(), static_cast<f&&>(func));
			}

			template<typename f> constexpr auto map(f&& func) && {
				return static_cast<tuple&&>(*this).mapImpl(base_list(), static_cast<f&&>(func));
			}

		  protected:
			template<typename u, typename... b1, typename... b2> constexpr void eqImpl(u&& uNew, type_list<b1...>, type_list<b2...>) {
				// See:
				// https://developercommunity.visualstudio.com/object/fold-expressions-unreliable-in-171-with-c20/1676476
				(void(b1::value = static_cast<u&&>(uNew).b2::value), ...);
			}

			template<typename u, size_t... I> constexpr void eqImpl(u&& uNew, std::index_sequence<I...>) {
				(void(tuple_elem<I, value_type>::value = get<I>(static_cast<u&&>(uNew))), ...);
			}

			template<typename f, typename... b> constexpr auto mapImpl(type_list<b...>, f&& func) & -> tuple<unwrap_ref_decay_t<decltype(func(b::value))>...> {
				return { func(b::value)... };
			}

			template<typename f, typename... b> constexpr auto mapImpl(type_list<b...>, f&& func) const& -> tuple<unwrap_ref_decay_t<decltype(func(b::value))>...> {
				return { func(b::value)... };
			}

			template<typename f, typename... b>
			constexpr auto mapImpl(type_list<b...>, f&& func) && -> tuple<unwrap_ref_decay_t<decltype(func(static_cast<value_type&&>(b::value)))>...> {
				return { func(static_cast<value_type&&>(b::value))... };
			}
		};

		template<> struct tuple<> : tuple_base_t<> {
			static constexpr size_t N = 0;
			using super				  = tuple_base_t<>;
			using base_list			  = type_list<>;
			using element_list		  = type_list<>;

			template<other_than<tuple> u>
				requires stateless<u>
			constexpr auto& operator=(u&&) noexcept {
				return *this;
			}

			auto operator<=>(tuple const&) const = default;
			bool operator==(tuple const&) const	 = default;

			template<typename f> constexpr void forEach(f&&) const noexcept {
			}

			template<typename f> constexpr bool any(f&&) const noexcept {
				return false;
			}

			template<typename f> constexpr bool all(f&&) const noexcept {
				return true;
			}

			template<typename f> constexpr auto map(f&&) const noexcept {
				return tuple{};
			}
		};

		template<typename... value_types> tuple(value_types...) -> tuple<unwrap_ref_decay_t<value_types>...>;

		template<size_t I, indexable tup> constexpr decltype(auto) get(tup&& tupNew) {
			return static_cast<tup&&>(tupNew)[Tag<I>()];
		}

		template<typename... value_type> constexpr tuple<value_type&...> tie(value_type&... object) {
			return { object... };
		}

		template<typename f, base_list_tuple tup> constexpr decltype(auto) apply(f&& func, tup&& tupNew) {
			return applyImpl(static_cast<f&&>(func), static_cast<tup&&>(tupNew), typename ref_unwrap<tup>::base_list());
		}
		template<typename f, typename a, typename b> constexpr decltype(auto) apply(f&& func, pair<a, b>& pair) {
			return static_cast<f&&>(func)(pair.first, pair.second);
		}
		template<typename f, typename a, typename b> constexpr decltype(auto) apply(f&& func, pair<a, b> const& pair) {
			return static_cast<f&&>(func)(pair.first, pair.second);
		}
		template<typename f, typename a, typename b> constexpr decltype(auto) apply(f&& func, pair<a, b>&& pair) {
			return static_cast<f&&>(func)(std::move(pair).first, std::move(pair).second);
		}

		template<base_list_tuple... value_type> constexpr auto tupleCat(value_type&&... ts) {
			if constexpr (sizeof...(value_type) == 0) {
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
				using big_tuple = tuple<ref_unwrap<value_type>...>;
#endif
				using outer_bases	 = BaseListT<big_tuple>;
				constexpr auto outer = getOuterBases(outer_bases{});
				constexpr auto inner = getInnerBases(outer_bases{});
				return catImpl(big_tuple{ static_cast<value_type&&>(ts)... }, outer, inner);
			}
		}

		template<typename... value_types> constexpr auto makeTuple(value_types&&... args) {
			return tuple<unwrap_ref_decay_t<value_types>...>{ static_cast<value_types&&>(args)... };
		}

		template<typename... value_types> constexpr auto copyTuple(value_types... args) {
			return tuple<value_types...>{ args... };
		}

		template<typename... value_type> constexpr auto forwardAstuple(value_type&&... a) noexcept {
			return tuple<value_type&&...>{ static_cast<value_type&&>(a)... };
		}
	}

	namespace tuplet::literals {
		template<char... D> constexpr auto operator""_tag() noexcept -> Tag<sizetFromDigits<D...>()> {
			return {};
		}
	}
}

namespace std {

	template<typename... value_type> struct tuple_size<jsonifier_internal::tuplet::tuple<value_type...>> : std::integral_constant<size_t, sizeof...(value_type)> {};

	template<size_t I, typename... value_type> struct tuple_element<I, jsonifier_internal::tuplet::tuple<value_type...>> {
		using type = decltype(jsonifier_internal::tuplet::tuple<value_type...>::declElem(jsonifier_internal::Tag<I>()));
	};

	template<typename a, typename b> struct tuple_size<jsonifier_internal::pair<a, b>> : std::integral_constant<size_t, 2> {};

	template<size_t I, typename a, typename b> struct tuple_element<I, jsonifier_internal::pair<a, b>> {
		static_assert(I < 2, "tuplet::pair only has 2 elements");
		using type = std::conditional_t<I == 0, a, b>;
	};

};

namespace jsonifier_internal {

	template<typename tuple, size_t... Is> auto tupleSplit(tuple&& tupleNew) {
		static constexpr auto N	 = std::tuple_size_v<tuple>;
		static constexpr auto is = std::make_index_sequence<N / 2>{};
		return std::make_pair(tupleSplitImpl<0>(tupleNew, is), tupleSplitImpl<1>(tupleNew, is));
	}

	template<size_t N> constexpr auto shrinkIndexArray(auto& arrayNew) {
		raw_array<size_t, N> res{};
		for (size_t x = 0; x < N; ++x) {
			res[x] = arrayNew[x];
		}
		return res;
	}

	template<typename tuple> constexpr auto filter() {
		constexpr auto n = std::tuple_size_v<tuple>;
		raw_array<size_t, n> indices{};
		size_t x = 0;
		forEach<n>([&](auto I) {
			using value_type = ref_unwrap<std::tuple_element_t<I, tuple>>;
			if constexpr (!std::convertible_to<value_type, jsonifier::string_view>) {
				indices[x++] = I - 1;
			}
		});
		return std::make_pair(indices, x);
	}

	template<typename Func, typename tuple> constexpr auto mapTuple(Func&& f, tuple&& tupleNew) {
		constexpr auto N = std::tuple_size_v<ref_unwrap<tuple>>;
		return mapTuple(f, tupleNew, std::make_index_sequence<N>{});
	}

	template<size_t n_groups> constexpr auto groupSizes(const raw_array<size_t, n_groups>& indices, size_t n_total) {
		raw_array<size_t, n_groups> diffs;

		for (size_t x = 0; x < n_groups - 1; ++x) {
			diffs[x] = indices[x + 1] - indices[x];
		}
		diffs[n_groups - 1] = n_total - indices[n_groups - 1];
		return diffs;
	}

	template<size_t Start, typename tuple, size_t... Is> constexpr auto makeGroup(tuple&& object, std::index_sequence<Is...>) {
		auto get_elem = [&](auto x) {
			constexpr auto I = decltype(x)::value;
			if constexpr (I == 1) {
				return tuplet::get<Start + I>(object);
			} else {
				return jsonifier::string_view(tuplet::get<Start + I>(object));
			}
		};
		auto r = tuplet::copyTuple(get_elem(std::integral_constant<size_t, Is>{})...);
		return r;
	}

	template<auto& GroupStartArr, auto& GroupSizeArr, typename tuple, size_t... GroupNumber> constexpr auto makeGroupsImpl(tuple&& object, std::index_sequence<GroupNumber...>) {
		return tuplet::copyTuple(makeGroup<tuplet::get<GroupNumber>(GroupStartArr)>(object, std::make_index_sequence<tuplet::get<GroupNumber>(GroupSizeArr)>{})...);
	}

	template<typename tuple> constexpr auto makeGroupsHelper() {
		constexpr auto N = std::tuple_size_v<tuple>;

		constexpr auto filtered = filter<tuple>();
		constexpr auto starts	= shrinkIndexArray<filtered.second>(filtered.first);
		constexpr auto sizes	= groupSizes(starts, N);

		return tuplet::makeTuple(starts, sizes);
	}

	template<typename tuple> struct GroupBuilder {
		static constexpr auto h		 = makeGroupsHelper<tuple>();
		static constexpr auto starts = tuplet::get<0>(h);
		static constexpr auto sizes	 = tuplet::get<1>(h);

		static constexpr auto op(tuple&& object) {
			constexpr auto n_groups = starts.size();
			return makeGroupsImpl<starts, sizes>(std::forward<tuple>(object), std::make_index_sequence<n_groups>{});
		}
	};
	
}