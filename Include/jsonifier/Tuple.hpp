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
	License along with this library; if not, Write to the Free Software
	Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301
	USA
*/
/// NOTE: Most of the code in this header was sampled heavily from Glaze library: https://github.com/stephenberry/glaze
/// https://github.com/RealTimeChris/Jsonifier
/// Feb 3, 2023
#pragma once

#include <jsonifier/RawArray.hpp>
#include <jsonifier/Pair.hpp>
#include <type_traits>
#include <concepts>

namespace JsonifierInternal {

	template<typename OTy> using RefUnwrap = std::decay_t<std::remove_const_t<OTy>>;

	namespace Tuplet {
		template<typename OTy> using IdentityT = OTy;

		template<typename OTy> using TypeT = typename OTy::type;

		template<size_t I> using Tag = std::integral_constant<size_t, I>;

		template<size_t I> inline constexpr Tag<I> TagV{};

		template<size_t N> using TagRange = std::make_index_sequence<N>;

		template<typename OTy, typename U>
		concept SameAs = std::same_as<OTy, U> && std::same_as<U, OTy>;

		template<typename OTy, typename U>
		concept OtherThan = !std::same_as<RefUnwrap<OTy>, U>;

		template<typename Tup> using BaseListT = typename RefUnwrap<Tup>::base_list;

		template<typename Tuple>
		concept BaseListTuple = requires() { typename RefUnwrap<Tuple>::base_list; };

		template<typename OTy>
		concept Stateless = std::is_empty_v<RefUnwrap<OTy>>;

		template<typename OTy>
		concept Indexable = Stateless<OTy> || requires(OTy value) { value[Tag<0>()]; };

		template<typename U, typename OTy>
		concept AssignableTo = requires(U u, OTy object) { object = u; };

		template<typename OTy>
		concept Ordered = requires(OTy const& object) {
			{ object <=> object };
		};
		template<typename OTy>
		concept EqualityComparable = requires(OTy const& object) {
			{ object == object } -> SameAs<bool>;
		};

		template<typename... OTy> struct Tuple;

		template<typename... OTy> struct TypeList {};

		template<typename... Ls, typename... Rs> inline constexpr auto operator+(TypeList<Ls...>, TypeList<Rs...>) {
			return TypeList<Ls..., Rs...>{};
		}

		template<typename... Bases> struct TypeMap : Bases... {
			using base_list = TypeList<Bases...>;
			using Bases::operator[]...;
			using Bases::declElem...;
			auto operator<=>(TypeMap const&) const = default;
			bool operator==(TypeMap const&) const = default;
		};

		template<size_t I, typename OTy> struct TupleElem {
			inline static OTy declElem(Tag<I>);
			using type = OTy;

			OTy value;

			inline constexpr decltype(auto) operator[](Tag<I>) & {
				return (value);
			}

			inline constexpr decltype(auto) operator[](Tag<I>) const& {
				return (value);
			}

			inline constexpr decltype(auto) operator[](Tag<I>) && {
				return (std::move(*this).value);
			}
			auto operator<=>(TupleElem const&) const = default;
			bool operator==(TupleElem const&) const = default;

			inline constexpr auto operator<=>(TupleElem const& other) const noexcept(noexcept(value <=> other.value))
				requires(std::is_reference_v<OTy> && Ordered<OTy>)
			{
				return value <=> other.value;
			}

			inline constexpr bool operator==(TupleElem const& other) const noexcept(noexcept(value == other.value))
				requires(std::is_reference_v<OTy> && EqualityComparable<OTy>)
			{
				return value == other.value;
			}
		};

		template<typename A, typename... OTy> struct GetTupleBase;

		template<size_t... I, typename... OTy> struct GetTupleBase<std::index_sequence<I...>, OTy...> {
			using type = TypeMap<TupleElem<I, OTy>...>;
		};

		template<typename F, typename OTy, typename... Bases> inline constexpr decltype(auto) applyImpl(F&& f, OTy&& object, TypeList<Bases...>) {
			return static_cast<F&&>(f)(static_cast<OTy&&>(object).IdentityT<Bases>::value...);
		}
		template<char... D> inline constexpr size_t sizetFromDigits() {
			static_assert((('0' <= D && D <= '9') && ...), "Must be integral literal");
			size_t num = 0;
			return ((num = num * 10 + (D - '0')), ..., num);
		}
		template<typename First, typename> using first_t = First;

		template<typename OTy, typename... Q> inline constexpr auto repeatType(TypeList<Q...>) {
			return TypeList<first_t<OTy, Q>...>{};
		}
		template<typename... Outer> inline constexpr auto getOuterBases(TypeList<Outer...>) {
			return (repeatType<Outer>(BaseListT<TypeT<Outer>>{}) + ...);
		}
		template<typename... Outer> inline constexpr auto getInnerBases(TypeList<Outer...>) {
			return (BaseListT<TypeT<Outer>>{} + ...);
		}

		template<typename OTy, typename... Outer, typename... Inner> inline constexpr auto catImpl(OTy tup, TypeList<Outer...>, TypeList<Inner...>)
			-> Tuple<TypeT<Inner>...> {
			return { static_cast<TypeT<Outer>&&>(tup.IdentityT<Outer>::value).IdentityT<Inner>::value... };
		}

		template<typename... OTy> using TupleBaseT = typename GetTupleBase<TagRange<sizeof...(OTy)>, OTy...>::type;

		template<typename... OTy> struct Tuple : TupleBaseT<OTy...> {
			inline static constexpr size_t N = sizeof...(OTy);
			using super = TupleBaseT<OTy...>;
			using super::operator[];
			using base_list = typename super::base_list;
			using element_list = TypeList<OTy...>;
			using super::declElem;

			template<OtherThan<Tuple> U> inline constexpr auto& operator=(U&& tup) {
				using tuple2 = RefUnwrap<U>;
				if (BaseListTuple<tuple2>) {
					eqImpl(static_cast<U&&>(tup), base_list(), typename tuple2::base_list());
				} else {
					eqImpl(static_cast<U&&>(tup), TagRange<N>());
				}
				return *this;
			}

			auto operator<=>(Tuple const&) const = default;
			bool operator==(Tuple const&) const = default;

			template<typename F> inline constexpr auto map(F&& func) & {
				return mapImpl(base_list(), static_cast<F&&>(func));
			}

			template<typename F> inline constexpr auto map(F&& func) const& {
				return mapImpl(base_list(), static_cast<F&&>(func));
			}

			template<typename F> inline constexpr auto map(F&& func) && {
				return static_cast<Tuple&&>(*this).mapImpl(base_list(), static_cast<F&&>(func));
			}

		  protected:
			template<typename U, typename... B1, typename... B2> inline constexpr void eqImpl(U&& u, TypeList<B1...>, TypeList<B2...>) {
				// See:
				// https://developercommunity.visualstudio.com/object/fold-expressions-unreliable-in-171-with-c20/1676476
				(void(B1::value = static_cast<U&&>(u).B2::value), ...);
			}

			template<typename U, size_t... I> inline constexpr void eqImpl(U&& u, std::index_sequence<I...>) {
				(void(TupleElem<I, OTy>::value = get<I>(static_cast<U&&>(u))), ...);
			}

			template<typename F, typename... B>
			inline constexpr auto mapImpl(TypeList<B...>, F&& func) & -> Tuple<UnwrapRefDecayT<decltype(func(B::value))>...> {
				return { func(B::value)... };
			}

			template<typename F, typename... B>
			inline constexpr auto mapImpl(TypeList<B...>, F&& func) const& -> Tuple<UnwrapRefDecayT<decltype(func(B::value))>...> {
				return { func(B::value)... };
			}

			template<typename F, typename... B>
			inline constexpr auto mapImpl(TypeList<B...>, F&& func) && -> Tuple<UnwrapRefDecayT<decltype(func(static_cast<OTy&&>(B::value)))>...> {
				return { func(static_cast<OTy&&>(B::value))... };
			}
		};

		template<> struct Tuple<> : TupleBaseT<> {
			inline static constexpr size_t N = 0;
			using super = TupleBaseT<>;
			using base_list = TypeList<>;
			using element_list = TypeList<>;

			template<OtherThan<Tuple> U>
				requires Stateless<U>
			inline constexpr auto& operator=(U&&) noexcept {
				return *this;
			}

			inline constexpr auto& assign() noexcept {
				return *this;
			}

			auto operator<=>(Tuple const&) const = default;
			bool operator==(Tuple const&) const = default;

			template<typename F> inline constexpr void forEach(F&&) const noexcept {
			}

			template<typename F> inline constexpr bool any(F&&) const noexcept {
				return false;
			}

			template<typename F> inline constexpr bool all(F&&) const noexcept {
				return true;
			}

			template<typename F> inline constexpr auto map(F&&) const noexcept {
				return Tuple{};
			}
		};

		template<typename... OTys> Tuple(OTys...) -> Tuple<UnwrapRefDecayT<OTys>...>;

		template<size_t I, Indexable Tup> inline constexpr decltype(auto) get(Tup&& tup) {
			return static_cast<Tup&&>(tup)[Tag<I>()];
		}

		template<typename... OTy> inline constexpr Tuple<OTy&...> tie(OTy&... object) {
			return { object... };
		}

		template<typename F, BaseListTuple Tup> inline constexpr decltype(auto) apply(F&& func, Tup&& tup) {
			return applyImpl(static_cast<F&&>(func), static_cast<Tup&&>(tup), typename RefUnwrap<Tup>::base_list());
		}
		template<typename F, typename A, typename B> inline constexpr decltype(auto) apply(F&& func, Pair<A, B>& Pair) {
			return static_cast<F&&>(func)(Pair.first, Pair.second);
		}
		template<typename F, typename A, typename B> inline constexpr decltype(auto) apply(F&& func, Pair<A, B> const& Pair) {
			return static_cast<F&&>(func)(Pair.first, Pair.second);
		}
		template<typename F, typename A, typename B> inline constexpr decltype(auto) apply(F&& func, Pair<A, B>&& Pair) {
			return static_cast<F&&>(func)(std::move(Pair).first, std::move(Pair).second);
		}

		template<BaseListTuple... OTy> inline constexpr auto tupleCat(OTy&&... ts) {
			if constexpr (sizeof...(OTy) == 0) {
				return Tuple<>();
			} else {
#if !defined(TUPLET_CAT_BY_FORWARDING_TUPLE)
	#if defined(__clang__)
		#define TUPLET_CAT_BY_FORWARDING_TUPLE 0
	#else
		#define TUPLET_CAT_BY_FORWARDING_TUPLE 1
	#endif
#endif
#if TUPLET_CAT_BY_FORWARDING_TUPLE
				using big_tuple = Tuple<OTy&&...>;
#else
				using big_tuple = Tuple<RefUnwrap<OTy>...>;
#endif
				using outer_bases = BaseListT<big_tuple>;
				constexpr auto outer = getOuterBases(outer_bases{});
				constexpr auto inner = getInnerBases(outer_bases{});
				return catImpl(big_tuple{ static_cast<OTy&&>(ts)... }, outer, inner);
			}
		}

		template<typename... OTys> inline constexpr auto makeTuple(OTys&&... args) {
			return Tuple<UnwrapRefDecayT<OTys>...>{ static_cast<OTys&&>(args)... };
		}

		template<typename... OTys> inline constexpr auto copyTuple(OTys... args) {
			return Tuple<OTys...>{ args... };
		}

		template<typename... OTy> inline constexpr auto forwardAsTuple(OTy&&... a) noexcept {
			return Tuple<OTy&&...>{ static_cast<OTy&&>(a)... };
		}
	}

	namespace Tuplet::literals {
		template<char... D> inline constexpr auto operator""_tag() noexcept -> Tag<sizetFromDigits<D...>()> {
			return {};
		}
	}
}

namespace std {
	template<typename... OTy> struct tuple_size<JsonifierInternal::Tuplet::Tuple<OTy...>> : std::integral_constant<size_t, sizeof...(OTy)> {};

	template<size_t I, typename... OTy> struct tuple_element<I, JsonifierInternal::Tuplet::Tuple<OTy...>> {
		using type = decltype(JsonifierInternal::Tuplet::Tuple<OTy...>::declElem(JsonifierInternal::Tuplet::Tag<I>()));
	};

	template<typename A, typename B> struct tuple_size<JsonifierInternal::Pair<A, B>> : std::integral_constant<size_t, 2> {};

	template<size_t I, typename A, typename B> struct tuple_element<I, JsonifierInternal::Pair<A, B>> {
		static_assert(I < 2, "tuplet::pair only has 2 elements");
		using type = std::conditional_t<I == 0, A, B>;
	};

};

namespace JsonifierInternal {

	template<typename Tuple, size_t... Is> auto tupleSplit(Tuple&& tuple) {
		static constexpr auto N = std::tuple_size_v<Tuple>;
		static constexpr auto is = std::make_index_sequence<N / 2>{};
		return std::make_pair(tupleSplitImpl<0>(tuple, is), tupleSplitImpl<1>(tuple, is));
	}

	template<size_t N> inline constexpr auto shrinkIndexArray(auto& arrayNew) {
		RawArray<size_t, N> res{};
		for (size_t x = 0; x < N; ++x) {
			res[x] = arrayNew[x];
		}
		return res;
	}

	template<typename Tuple> inline constexpr auto filter() {
		constexpr auto n = std::tuple_size_v<Tuple>;
		RawArray<size_t, n> indices{};
		size_t x = 0;
		forEach<n>([&](auto I) {
			using VTy = RefUnwrap<std::tuple_element_t<I, Tuple>>;
			if constexpr (!std::convertible_to<VTy, Jsonifier::StringView>) {
				indices[x++] = I - 1;
			}
		});
		return std::make_pair(indices, x);
	}

	template<typename Func, typename Tuple> inline constexpr auto mapTuple(Func&& f, Tuple&& tuple) {
		constexpr auto N = std::tuple_size_v<RefUnwrap<Tuple>>;
		return mapTuple(f, tuple, std::make_index_sequence<N>{});
	}

	template<size_t n_groups> inline constexpr auto groupSizes(const RawArray<size_t, n_groups>& indices, size_t n_total) {
		RawArray<size_t, n_groups> diffs;

		for (size_t x = 0; x < n_groups - 1; ++x) {
			diffs[x] = indices[x + 1] - indices[x];
		}
		diffs[n_groups - 1] = n_total - indices[n_groups - 1];
		return diffs;
	}

	template<size_t Start, typename Tuple, size_t... Is> inline constexpr auto makeGroup(Tuple&& object, std::index_sequence<Is...>) {
		auto get_elem = [&](auto x) {
			constexpr auto I = decltype(x)::value;
			if constexpr (I == 1) {
				return Tuplet::get<Start + I>(object);
			} else {
				return Jsonifier::StringView(Tuplet::get<Start + I>(object));
			}
		};
		auto r = Tuplet::copyTuple(get_elem(std::integral_constant<size_t, Is>{})...);
		return r;
	}

	template<auto& GroupStartArr, auto& GroupSizeArr, typename Tuple, size_t... GroupNumber>
	inline constexpr auto makeGroupsImpl(Tuple&& object, std::index_sequence<GroupNumber...>) {
		return Tuplet::copyTuple(
			makeGroup<Tuplet::get<GroupNumber>(GroupStartArr)>(object, std::make_index_sequence<Tuplet::get<GroupNumber>(GroupSizeArr)>{})...);
	}

	template<typename Tuple> inline constexpr auto makeGroupsHelper() {
		constexpr auto N = std::tuple_size_v<Tuple>;

		constexpr auto filtered = filter<Tuple>();
		constexpr auto starts = shrinkIndexArray<filtered.second>(filtered.first);
		constexpr auto sizes = groupSizes(starts, N);

		return Tuplet::makeTuple(starts, sizes);
	}

	template<typename Tuple> struct GroupBuilder {
		static constexpr auto h = makeGroupsHelper<Tuple>();
		static constexpr auto starts = Tuplet::get<0>(h);
		static constexpr auto sizes = Tuplet::get<1>(h);

		inline static constexpr auto op(Tuple&& object) {
			constexpr auto n_groups = starts.size();
			return makeGroupsImpl<starts, sizes>(std::forward<Tuple>(object), std::make_index_sequence<n_groups>{});
		}
	};

}
