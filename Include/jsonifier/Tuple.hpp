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
/// https://github.com/RealTimeChris/Jsonifier
/// Feb 3, 2023
#pragma once

#include <jsonifier/RawArray.hpp>
#include <jsonifier/Pair.hpp>
#include <type_traits>
#include <concepts>

namespace JsonifierInternal {

	template<typename ValueType> using RefUnwrap = std::decay_t<std::remove_const_t<ValueType>>;

	namespace Tuplet {
		template<typename ValueType> using IdentityT = ValueType;

		template<typename ValueType> using TypeT = typename ValueType::type;

		template<size_t Index> using Tag = std::integral_constant<size_t, Index>;

		template<size_t Index> constexpr Tag<Index> TagV{};

		template<size_t Count> using TagRange = std::make_index_sequence<Count>;

		template<typename ValueType, typename U>
		concept SameAs = std::same_as<ValueType, U> && std::same_as<U, ValueType>;

		template<typename ValueType, typename U>
		concept OtherThan = !std::same_as<RefUnwrap<ValueType>, U>;

		template<typename Tup> using BaseListT = typename RefUnwrap<Tup>::base_list;

		template<typename Tuple>
		concept BaseListTuple = requires() { typename RefUnwrap<Tuple>::base_list; };

		template<typename ValueType>
		concept Stateless = std::is_empty_v<RefUnwrap<ValueType>>;

		template<typename ValueType>
		concept Indexable = Stateless<ValueType> || requires(ValueType value) { value[Tag<0>()]; };

		template<typename U, typename ValueType>
		concept AssignableTo = requires(U u, ValueType object) { object = u; };

		template<typename ValueType>
		concept Ordered = requires(ValueType const& object) {
			{ object <=> object };
		};
		template<typename ValueType>
		concept EqualityComparable = requires(ValueType const& object) {
			{ object == object } -> SameAs<bool>;
		};

		template<typename... ValueType> struct Tuple;

		template<typename... ValueType> struct TypeList {};

		template<typename... Ls, typename... Rs> constexpr auto operator+(TypeList<Ls...>, TypeList<Rs...>) {
			return TypeList<Ls..., Rs...>{};
		}

		template<typename... Bases> struct TypeMap : Bases... {
			using base_list = TypeList<Bases...>;
			using Bases::operator[]...;
			using Bases::declElem...;
			auto operator<=>(TypeMap const&) const = default;
			bool operator==(TypeMap const&) const = default;
		};

		template<size_t Index, typename ValueType> struct TupleElem {
			inline static ValueType declElem(Tag<Index>);
			using type = ValueType;

			ValueType value;

			constexpr decltype(auto) operator[](Tag<Index>) & {
				return (value);
			}

			constexpr decltype(auto) operator[](Tag<Index>) const& {
				return (value);
			}

			constexpr decltype(auto) operator[](Tag<Index>) && {
				return (std::move(*this).value);
			}
			auto operator<=>(TupleElem const&) const = default;
			bool operator==(TupleElem const&) const = default;

			constexpr auto operator<=>(TupleElem const& other) const noexcept(noexcept(value <=> other.value))
				requires(std::is_reference_v<ValueType> && Ordered<ValueType>)
			{
				return value <=> other.value;
			}

			constexpr bool operator==(TupleElem const& other) const noexcept(noexcept(value == other.value))
				requires(std::is_reference_v<ValueType> && EqualityComparable<ValueType>)
			{
				return value == other.value;
			}
		};

		template<typename A, typename... ValueType> struct GetTupleBase;

		template<size_t... Index, typename... ValueType> struct GetTupleBase<std::index_sequence<Index...>, ValueType...> {
			using type = TypeMap<TupleElem<Index, ValueType>...>;
		};

		template<typename F, typename ValueType, typename... Bases>
		constexpr decltype(auto) applyImpl(F&& f, ValueType&& object, TypeList<Bases...>) {
			return static_cast<F&&>(f)(static_cast<ValueType&&>(object).IdentityT<Bases>::value...);
		}
		template<char... D> constexpr size_t sizetFromDigits() {
			static_assert((('0' <= D && D <= '9') && ...), "Must be integral literal");
			size_t num = 0;
			return ((num = num * 10 + (D - '0')), ..., num);
		}
		template<typename First, typename> using first_t = First;

		template<typename ValueType, typename... Q> constexpr auto repeatType(TypeList<Q...>) {
			return TypeList<first_t<ValueType, Q>...>{};
		}
		template<typename... Outer> constexpr auto getOuterBases(TypeList<Outer...>) {
			return (repeatType<Outer>(BaseListT<TypeT<Outer>>{}) + ...);
		}
		template<typename... Outer> constexpr auto getInnerBases(TypeList<Outer...>) {
			return (BaseListT<TypeT<Outer>>{} + ...);
		}

		template<typename ValueType, typename... Outer, typename... Inner>
		constexpr auto catImpl(ValueType tup, TypeList<Outer...>, TypeList<Inner...>) -> Tuple<TypeT<Inner>...> {
			return { static_cast<TypeT<Outer>&&>(tup.IdentityT<Outer>::value).IdentityT<Inner>::value... };
		}

		template<typename... ValueType> using TupleBaseT = typename GetTupleBase<TagRange<sizeof...(ValueType)>, ValueType...>::type;

		template<typename... ValueType> struct Tuple : TupleBaseT<ValueType...> {
			static constexpr size_t Count = sizeof...(ValueType);
			using super = TupleBaseT<ValueType...>;
			using super::operator[];
			using base_list = typename super::base_list;
			using element_list = TypeList<ValueType...>;
			using super::declElem;

			template<OtherThan<Tuple> U> constexpr auto& operator=(U&& tup) {
				using tuple2 = RefUnwrap<U>;
				if (BaseListTuple<tuple2>) {
					eqImpl(static_cast<U&&>(tup), base_list(), typename tuple2::base_list());
				} else {
					eqImpl(static_cast<U&&>(tup), TagRange<Count>());
				}
				return *this;
			}

			auto operator<=>(Tuple const&) const = default;
			bool operator==(Tuple const&) const = default;

			template<typename F> constexpr auto map(F&& func) & {
				return mapImpl(base_list(), static_cast<F&&>(func));
			}

			template<typename F> constexpr auto map(F&& func) const& {
				return mapImpl(base_list(), static_cast<F&&>(func));
			}

			template<typename F> constexpr auto map(F&& func) && {
				return static_cast<Tuple&&>(*this).mapImpl(base_list(), static_cast<F&&>(func));
			}

		  protected:
			template<typename U, typename... B1, typename... B2> constexpr void eqImpl(U&& u, TypeList<B1...>, TypeList<B2...>) {
				// See:
				// https://developercommunity.visualstudio.com/object/fold-expressions-unreliable-in-171-with-c20/1676476
				(void(B1::value = static_cast<U&&>(u).B2::value), ...);
			}

			template<typename U, size_t... Index> constexpr void eqImpl(U&& u, std::index_sequence<Index...>) {
				(void(TupleElem<Index, ValueType>::value = get<Index>(static_cast<U&&>(u))), ...);
			}

			template<typename F, typename... B>
			constexpr auto mapImpl(TypeList<B...>, F&& func) & -> Tuple<UnwrapRefDecayT<decltype(func(B::value))>...> {
				return { func(B::value)... };
			}

			template<typename F, typename... B>
			constexpr auto mapImpl(TypeList<B...>, F&& func) const& -> Tuple<UnwrapRefDecayT<decltype(func(B::value))>...> {
				return { func(B::value)... };
			}

			template<typename F, typename... B>
			constexpr auto mapImpl(TypeList<B...>, F&& func) && -> Tuple<UnwrapRefDecayT<decltype(func(static_cast<ValueType&&>(B::value)))>...> {
				return { func(static_cast<ValueType&&>(B::value))... };
			}
		};

		template<> struct Tuple<> : TupleBaseT<> {
			static constexpr size_t Count = 0;
			using super = TupleBaseT<>;
			using base_list = TypeList<>;
			using element_list = TypeList<>;

			template<OtherThan<Tuple> U>
				requires Stateless<U>
			constexpr auto& operator=(U&&) {
				return *this;
			}

			constexpr auto& assign() {
				return *this;
			}

			auto operator<=>(Tuple const&) const = default;
			bool operator==(Tuple const&) const = default;

			template<typename F> constexpr void forEach(F&&) const {
			}

			template<typename F> constexpr bool any(F&&) const {
				return false;
			}

			template<typename F> constexpr bool all(F&&) const {
				return true;
			}

			template<typename F> constexpr auto map(F&&) const {
				return Tuple{};
			}
		};

		template<typename... OTys> Tuple(OTys...) -> Tuple<UnwrapRefDecayT<OTys>...>;

		template<size_t Index, Indexable Tup> constexpr decltype(auto) get(Tup&& tup) {
			return static_cast<Tup&&>(tup)[Tag<Index>()];
		}

		template<typename... ValueType> constexpr Tuple<ValueType&...> tie(ValueType&... object) {
			return { object... };
		}

		template<typename F, BaseListTuple Tup> constexpr decltype(auto) apply(F&& func, Tup&& tup) {
			return applyImpl(static_cast<F&&>(func), static_cast<Tup&&>(tup), typename RefUnwrap<Tup>::base_list());
		}
		template<typename F, typename A, typename B> constexpr decltype(auto) apply(F&& func, Pair<A, B>& Pair) {
			return static_cast<F&&>(func)(Pair.first, Pair.second);
		}
		template<typename F, typename A, typename B> constexpr decltype(auto) apply(F&& func, Pair<A, B> const& Pair) {
			return static_cast<F&&>(func)(Pair.first, Pair.second);
		}
		template<typename F, typename A, typename B> constexpr decltype(auto) apply(F&& func, Pair<A, B>&& Pair) {
			return static_cast<F&&>(func)(std::move(Pair).first, std::move(Pair).second);
		}

		template<BaseListTuple... ValueType> constexpr auto tupleCat(ValueType&&... ts) {
			if constexpr (sizeof...(ValueType) == 0) {
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
				using big_tuple = Tuple<ValueType&&...>;
#else
				using big_tuple = Tuple<RefUnwrap<ValueType>...>;
#endif
				using outer_bases = BaseListT<big_tuple>;
				constexpr auto outer = getOuterBases(outer_bases{});
				constexpr auto inner = getInnerBases(outer_bases{});
				return catImpl(big_tuple{ static_cast<ValueType&&>(ts)... }, outer, inner);
			}
		}

		template<typename... OTys> constexpr auto makeTuple(OTys&&... args) {
			return Tuple<UnwrapRefDecayT<OTys>...>{ static_cast<OTys&&>(args)... };
		}

		template<typename... OTys> constexpr auto copyTuple(OTys... args) {
			return Tuple<OTys...>{ args... };
		}

		template<typename... ValueType> constexpr auto forwardAsTuple(ValueType&&... a) {
			return Tuple<ValueType&&...>{ static_cast<ValueType&&>(a)... };
		}
	}

	namespace Tuplet::literals {
		template<char... D> constexpr auto operator""_tag() noexcept -> Tag<sizetFromDigits<D...>()> {
			return {};
		}
	}
}

namespace std {

	template<typename... ValueType> struct tuple_size<JsonifierInternal::Tuplet::Tuple<ValueType...>>
		: std::integral_constant<size_t, sizeof...(ValueType)> {};

	template<size_t Index, typename... ValueType> struct tuple_element<Index, JsonifierInternal::Tuplet::Tuple<ValueType...>> {
		using type = decltype(JsonifierInternal::Tuplet::Tuple<ValueType...>::declElem(JsonifierInternal::Tuplet::Tag<Index>()));
	};

	template<typename A, typename B> struct tuple_size<JsonifierInternal::Pair<A, B>> : std::integral_constant<size_t, 2> {};

	template<size_t Index, typename A, typename B> struct tuple_element<Index, JsonifierInternal::Pair<A, B>> {
		static_assert(Index < 2, "tuplet::pair only has 2 elements");
		using type = std::conditional_t<Index == 0, A, B>;
	};

};

namespace JsonifierInternal {

	template<typename Tuple, size_t... Indices> auto tupleSplit(Tuple&& tuple) {
		static constexpr auto Count = std::tuple_size_v<Tuple>;
		static constexpr auto is = std::make_index_sequence<Count / 2>{};
		return std::make_pair(tupleSplitImpl<0>(tuple, is), tupleSplitImpl<1>(tuple, is));
	}

	template<size_t Count> constexpr auto shrinkIndexArray(auto& arrayNew) {
		RawArray<size_t, Count> res{};
		for (size_t x = 0; x < Count; ++x) {
			res[x] = arrayNew[x];
		}
		return res;
	}

	template<typename Tuple> constexpr auto filter() {
		constexpr auto n = std::tuple_size_v<Tuple>;
		RawArray<size_t, n> indices{};
		size_t x = 0;
		forEach<n>([&](auto Index) {
			using ValueType = RefUnwrap<std::tuple_element_t<Index, Tuple>>;
			if constexpr (!std::convertible_to<ValueType, Jsonifier::StringView>) {
				indices[x++] = Index - 1;
			}
		});
		return std::make_pair(indices, x);
	}

	template<typename Func, typename Tuple> constexpr auto mapTuple(Func&& f, Tuple&& tuple) {
		constexpr auto Count = std::tuple_size_v<RefUnwrap<Tuple>>;
		return mapTuple(f, tuple, std::make_index_sequence<Count>{});
	}

	template<size_t n_groups> constexpr auto groupSizes(const RawArray<size_t, n_groups>& indices, size_t n_total) {
		RawArray<size_t, n_groups> diffs;

		for (size_t x = 0; x < n_groups - 1; ++x) {
			diffs[x] = indices[x + 1] - indices[x];
		}
		diffs[n_groups - 1] = n_total - indices[n_groups - 1];
		return diffs;
	}

	template<size_t Start, class Tuple, size_t... Indices> constexpr auto makeGroup(Tuple&& t, std::index_sequence<Indices...>) {
		auto getElem = [&](auto i) {
			constexpr auto Index = decltype(i)::value;
			using type = decltype(Tuplet::get<Start + Index>(t));
			if constexpr (Index == 0 || std::convertible_to<type, Jsonifier::StringViewBase<char>>) {
				return Jsonifier::StringViewBase<char>(Tuplet::get<Start + Index>(t));
			} else {
				return Tuplet::get<Start + Index>(t);
			}
		};
		auto r = Tuplet::copyTuple(getElem(std::integral_constant<size_t, Indices>{})...);
		return r;
	}

	template<auto& GroupStartArr, auto& GroupSizeArr, typename Tuple, size_t... GroupNumber>
	constexpr auto makeGroupsImpl(Tuple&& object, std::index_sequence<GroupNumber...>) {
		return Tuplet::copyTuple(
			makeGroup<Tuplet::get<GroupNumber>(GroupStartArr)>(object, std::make_index_sequence<Tuplet::get<GroupNumber>(GroupSizeArr)>{})...);
	}

	template<typename Tuple> constexpr auto makeGroupsHelper() {
		constexpr auto Count = std::tuple_size_v<Tuple>;

		constexpr auto filtered = filter<Tuple>();
		constexpr auto starts = shrinkIndexArray<filtered.second>(filtered.first);
		constexpr auto sizes = groupSizes(starts, Count);

		return Tuplet::makeTuple(starts, sizes);
	}

	template<typename Tuple> struct GroupBuilder {
		static constexpr auto h = makeGroupsHelper<Tuple>();
		static constexpr auto starts = Tuplet::get<0>(h);
		static constexpr auto sizes = Tuplet::get<1>(h);

		static constexpr auto op(Tuple&& object) {
			constexpr auto n_groups = starts.size();
			return makeGroupsImpl<starts, sizes>(std::forward<Tuple>(object), std::make_index_sequence<n_groups>{});
		}
	};


}
