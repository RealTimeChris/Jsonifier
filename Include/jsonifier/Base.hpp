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
	License along with this library; if not, write to the Free Software
	Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301
	USA
*/
/// https://github.com/RealTimeChris/Jsonifier
/// Feb 3, 2023
#pragma once

#pragma warning(push)
#pragma warning(disable : 4244)
#pragma warning(disable : 4251)

#include <jsonifier/StringView.hpp>
#include <jsonifier/Tuple.hpp>
#include <jsonifier/Error.hpp>
#include <jsonifier/Core.hpp>

#include <unordered_map>
#include <immintrin.h>
#include <string_view>
#include <type_traits>
#include <functional>
#include <concepts>
#include <iostream>
#include <variant>
#include <vector>
#include <chrono>
#include <bitset>
#include <span>
#include <map>

namespace Jsonifier {

	class StructuralIterator;
	class Serializer;
	class StringView;
	class String;
	class Parser;

	template<typename OTy> inline void swapF(OTy& object01, OTy& object02) {
		OTy temp = std::move(object01);
		object01 = std::move(object02);
		object02 = std::move(temp);
	}

	template<typename OTy = void> struct Hash {
		static_assert(std::is_integral<OTy>::value || std::is_enum<OTy>::value, "Hash only supports integral types, specialize for other types.");

		inline constexpr size_t operator()(OTy const& value, size_t seed) const {
			size_t key = seed ^ static_cast<size_t>(value);
			key = (~key) + (key << 21);
			key = key ^ (key >> 24);
			key = (key + (key << 3)) + (key << 8);
			key = key ^ (key >> 14);
			key = (key + (key << 2)) + (key << 4);
			key = key ^ (key >> 28);
			key = key + (key << 31);
			return key;
		}
	};

	template<> struct Hash<void> {
		template<typename OTy> inline constexpr size_t operator()(OTy const& value, size_t seed) const {
			return Hash<OTy>{}(value, seed);
		}
	};

	// From:
	// https://stackoverflow.com/questions/16337610/how-to-know-if-a-type-is-a-specialization-of-stdvector
	template<typename Test, template<typename...> class Ref> struct IsSpecializationV : std::false_type {};

	template<template<typename...> class Ref, typename... Args> struct IsSpecializationV<Ref<Args...>, Ref> : std::true_type {};

	template<typename OTy>
	concept Range = requires(OTy& value) {
		typename OTy::value_type;
		requires !std::same_as<void, decltype(value.begin())>;
		requires !std::same_as<void, decltype(value.end())>;
	};

	template<typename OTy>
	concept ComplexT = JsonifierT<std::decay_t<OTy>>;

	template<typename OTy>
	concept TupleT = requires(OTy value) {
		std::tuple_size<OTy>::value;
		Tuplet::get<0>(value);
	} && !ComplexT<OTy> && !Range<OTy>;

	template<typename OTy>
	concept MapSubscriptable = requires(OTy value) {
		{ value[std::declval<typename OTy::key_type>()] } -> std::same_as<typename OTy::mapped_type&>;
	};

	template<typename OTy>
	concept VectorSubscriptable = requires(OTy value) {
		{ value[size_t{}] } -> std::same_as<typename OTy::value_type&>;
	};

	template<typename OTy>
	concept PairT = requires(OTy value) {
		{ value.first } -> std::same_as<typename OTy::first_type&>;
		{ value.second } -> std::same_as<typename OTy::second_type&>;
	};

	template<typename OTy>
	concept HasSize = requires(OTy value) {
		{ value.size() } -> std::same_as<size_t>;
	};

	template<typename OTy>
	concept HasData = requires(OTy value) {
		{ value.data() };
	};

	template<typename OTy>
	concept HasSubstr = requires(OTy value, const size_t pos, size_t len) {
		{ value.substr(pos, len) } -> std::same_as<OTy>;
	};

	template<typename OTy>
	concept CharT = std::same_as<std::decay_t<OTy>, char> || std::same_as<std::decay_t<OTy>, char16_t> || std::same_as<std::decay_t<OTy>, char32_t> ||
		std::same_as<std::decay_t<OTy>, char8_t>;

	template<typename OTy>
	concept BoolT = std::same_as<std::decay_t<OTy>, bool> || std::same_as<std::decay_t<OTy>, std::vector<bool>::reference>;

	template<typename OTy>
	concept IntT = std::integral<std::decay_t<OTy>> && !CharT<std::decay_t<OTy>> && !BoolT<OTy>;

	template<typename OTy>
	concept NumT = std::floating_point<std::decay_t<OTy>> || IntT<OTy>;

	template<typename OTy>
	concept StringT = HasSubstr<OTy> && HasData<OTy> && HasSize<OTy> && !std::same_as<char, OTy>;

	template<typename OTy> using IteratorT = decltype(std::begin(std::declval<OTy&>()));

	template<typename OTy>
	concept MapT = !ComplexT<OTy> && !StringT<OTy> && MapSubscriptable<OTy>;

	template<typename OTy>
	concept EmplaceBackable = requires(OTy value) {
		{ value.emplace_back(std::declval<typename OTy::value_type&>()) };
	};

	template<typename T>
	concept UniquePtrT = std::is_same_v<T, std::unique_ptr<typename T::element_type>>;

	template<typename OTy> struct Array {
		OTy parseValue;
	};

	template<typename Container>
	concept Findable = requires(Container c, const typename Container::key_type& val) {
		{ c.find(val) } -> std::same_as<typename Container::const_iterator>;
	};

	template<typename OTy>
	concept HasExcludedKeys = requires(OTy value) {
		{ value.excludedKeys };
	};

	template<typename OTy> Array(OTy) -> Array<OTy>;

	template<typename OTy> struct Object {
		OTy parseValue;
	};

	template<typename OTy> Object(OTy) -> Object<OTy>;

	template<typename OTy>
	concept Nullable = std::same_as<OTy, std::nullptr_t>;

	template<typename... Args> struct FalseT : std::false_type {};

	struct RandomCoreType;

	template<> struct FalseT<RandomCoreType> : std::true_type {};

	template<typename... Args> inline constexpr bool FalseV = FalseT<Args...>::value;

	template<size_t n> inline constexpr size_t length(const char (&)[n]) noexcept {
		return n;
	}

	template<StringLiteral Str> struct CharsImpl {
		static constexpr StringView value{ Str.string, length(Str.string) - 1 };
	};

	template<StringLiteral Str> inline constexpr StringView Chars = CharsImpl<Str>::value;


	template<typename OTy>
	concept StdTupleT = IsSpecializationV<OTy, std::tuple>::value || IsSpecializationV<OTy, std::pair>::value;

	template<typename = void, size_t... Is> inline constexpr auto indexer(std::index_sequence<Is...>) {
		return [](auto&& f) -> decltype(auto) {
			return decltype(f)(f)(std::integral_constant<size_t, Is>{}...);
		};
	}

	template<size_t n> inline constexpr auto indexer() {
		return indexer(std::make_index_sequence<n>());
	}

	template<size_t n, typename Func> inline constexpr auto forEach(Func&& f) {
		return indexer<n>()([&](auto&&... x) {
			(std::forward<Func>(f)(x), ...);
		});
	}

	template<typename OTy>
	concept IsVariant = IsSpecializationV<OTy, std::variant>::value;

	namespace detail {
		template<IsVariant OTy> inline constexpr auto runtimeVariantMap() {
			constexpr auto n = std::variant_size_v<OTy>;
			std::array<OTy, n> ret{};
			forEach<n>([&](auto x) {
				ret[x] = std::variant_alternative_t<x, OTy>{};
			});
			return ret;
		}
	}

	template<std::array newArr> struct MakeStatic {
		static constexpr auto value = newArr;
	};

	struct StringViewTwo {
		size_t sizeVal{};
		const char* string{};

		constexpr StringViewTwo(const StringView& stringNew) {
			sizeVal = stringNew.size();
			string = stringNew.data();
		}

		constexpr const char* begin() const noexcept {
			return string;
		}

		constexpr const char* end() const noexcept {
			return string + sizeVal;
		}

		constexpr size_t size() const noexcept{
			return sizeVal;
		}

	};

#ifdef _MSC_VER
	template<StringViewTwo... Strings> inline constexpr StringView Join() {
#else
	template<const StringView&... Strings> inline constexpr StringView Join() {
#endif
		constexpr auto joinedArr = []() {
			constexpr size_t len = (Strings.size() + ... + 0);
			std::array<char, len + 1> arr{};
			auto append = [i = 0, &arr](const auto& s) mutable {
				for (auto c: s)
					arr[i++] = c;
			};
			(append(Strings), ...);
			arr[len] = 0;
			return arr;
		}();
		auto& staticArr = MakeStatic<joinedArr>::value;
		return { staticArr.data(), staticArr.size() - 1 };
	}

#ifdef _MSC_VER
	template<StringViewTwo... Strings> constexpr auto JoinV = Join<Strings...>();
#else
	template<const StringView&... Strings> constexpr auto JoinV = Join<Strings...>();
#endif

	template<size_t strLength> inline constexpr auto stringLiteralFromView(StringView str) {
		StringLiteral<strLength + 1> sl{};
		std::copy_n(str.data(), str.size(), sl.string);
		*(sl.string + strLength) = '\0';
		return sl;
	}

	inline decltype(auto) getMember(auto&& value, auto& member_ptr) {
		using VTy = std::decay_t<decltype(member_ptr)>;
		if constexpr (std::is_member_object_pointer_v<VTy>) {
			return value.*member_ptr;
		} else if constexpr (std::is_member_function_pointer_v<VTy>) {
			return member_ptr;
		} else if constexpr (std::invocable<decltype(member_ptr), decltype(value)>) {
			return std::invoke(member_ptr, value);
		} else {
			return member_ptr;
		}
	}

	template<size_t n, size_t x = 0, typename Func, typename Value> inline constexpr void forEachValue(Func&& f, Value&& v) {
		if constexpr (x != n) {
			forEachValue<n, x + 1>(f, f(std::integral_constant<size_t, x>{}, v));
		}
	}

	template<typename OTy, typename mptr_t> using MemberT = decltype(getMember(std::declval<OTy>(), std::declval<std::decay_t<mptr_t>&>()));

	template<typename OTy>
	concept NullableT = !ComplexT<OTy> && !StringT<OTy> && requires(OTy value) {
		bool(value);
		{ *value };
	};


	template<size_t... Is> struct Sequence {};
	template<size_t n, size_t... Is> struct GenerateSequence : GenerateSequence<n - 1, n - 1, Is...> {};
	template<size_t... Is> struct GenerateSequence<0, Is...> : Sequence<Is...> {};

	template<size_t N1, size_t... I1, size_t N2, size_t... I2>
	inline constexpr std::array<char const, N1 + N2 - 1> concat(char const (&a1)[N1], char const (&a2)[N2], Sequence<I1...>, Sequence<I2...>) {
		return { { a1[I1]..., a2[I2]... } };
	}

	template<size_t N1, size_t N2> inline constexpr std::array<char const, N1 + N2 - 1> concatArrays(char const (&a1)[N1], char const (&a2)[N2]) {
		return concat(a1, a2, GenerateSequence<N1 - 1>{}, GenerateSequence<N2>{});
	}

	template<typename OTy, typename... U>
	concept IsAnyOf = (std::same_as<OTy, U> || ...);

	template<typename OTy>
	concept Emplaceable = requires(OTy value) {
		{ value.emplace(std::declval<typename OTy::value_type>()) };
	};

	template<typename OTy>
	concept PushBackable = requires(OTy value) {
		{ value.push_back(std::declval<typename OTy::value_type>()) };
	};

	template<typename OTy>
	concept Resizeable = requires(OTy value) { value.resize(0); };

	inline constexpr auto array(auto&&... args) {
		return Array{ Tuplet::copyTuple(args...) };
	}

	inline constexpr auto object(auto&&... args) {
		if constexpr (sizeof...(args) == 0) {
			return Object{ Tuplet::Tuple{} };
		} else {
			return Object{ GroupBuilder<std::decay_t<decltype(Tuplet::copyTuple(args...))>>::op(Tuplet::copyTuple(args...)) };
		}
	}

	template<typename OTy>
	concept JsonifierArrayT = JsonifierT<OTy> && IsSpecializationV<CoreWrapperT<OTy>, Array>::value;

	template<typename OTy>
	concept JsonifierObjectT = JsonifierT<OTy> && IsSpecializationV<CoreWrapperT<OTy>, Object>::value;

	template<typename OTy>
	concept JsonifierValueT = JsonifierT<OTy> && !(JsonifierArrayT<OTy> || JsonifierObjectT<OTy>);

	template<typename OTy>
	concept ArrayTupleT = JsonifierArrayT<OTy> || TupleT<std::decay_t<OTy>>;

	template<typename OTy>
	concept ObjectT = MapT<OTy> || JsonifierObjectT<OTy>;

	template<typename OTy>
	concept IsTimeType =
		std::same_as<OTy, std::chrono::nanoseconds> || std::same_as<OTy, std::chrono::microseconds> || std::same_as<OTy, std::chrono::milliseconds> ||
		std::same_as<OTy, std::chrono::minutes> || std::same_as<OTy, std::chrono::hours> || std::same_as<OTy, std::chrono::days>;

	template<typename OTy>
	concept IsConvertibleToSerializer = std::convertible_to<OTy, Serializer>;

	template<typename OTy>
	concept EnumT = std::is_enum<OTy>::value;

	template<typename OTy> inline auto dataPtr(OTy& buffer) {
		if constexpr (Resizeable<OTy>) {
			return buffer.data();
		} else {
			return buffer;
		}
	}

	template<typename OTy>
	concept VectorT = ( !ComplexT<OTy> && !StringT<OTy> && !MapT<OTy> && VectorSubscriptable<OTy> && HasData<OTy> &&
						  EmplaceBackable<OTy> )&&!JsonifierArrayT<OTy> &&
		!TupleT<OTy>;

	template<typename OTy>
	concept RawArrayT =
		!VectorT<OTy> && ( !EmplaceBackable<OTy> || !Resizeable<OTy> )&&!Emplaceable<OTy> && HasSize<OTy> && !StringT<OTy> && Range<OTy>;

	template<typename OTy>
	concept VectorLike = Resizeable<OTy> && VectorSubscriptable<OTy> && HasData<OTy>;

	template<typename OTy>
	concept CoreType = IsSpecializationV<Core<OTy>, Core>::value;

	template<typename TTy> class StopWatch {
	  public:
		using HRClock = std::chrono::high_resolution_clock;

		inline StopWatch() = delete;

		inline StopWatch<TTy>& operator=(StopWatch<TTy>&& data) {
			maxNumberOfMs.store(data.maxNumberOfMs.load());
			startTime.store(data.startTime.load());
			return *this;
		}

		inline StopWatch(StopWatch<TTy>&& data) {
			*this = std::move(data);
		}

		inline StopWatch(TTy maxNumberOfMsNew) {
			maxNumberOfMs.store(maxNumberOfMsNew);
			startTime.store(std::chrono::duration_cast<TTy>(HRClock::now().time_since_epoch()));
		}

		inline TTy totalTimePassed() {
			return std::chrono::duration_cast<TTy>(HRClock::now().time_since_epoch()) - startTime.load();
		}

		inline TTy getTotalWaitTime() {
			return maxNumberOfMs.load();
		}

		inline bool hasTimePassed() {
			if (std::chrono::duration_cast<TTy>(HRClock::now().time_since_epoch()) - startTime.load() >= maxNumberOfMs.load()) {
				return true;
			} else {
				return false;
			}
		}

		inline void resetTimer() {
			startTime.store(std::chrono::duration_cast<TTy>(HRClock::now().time_since_epoch()));
		}

	  protected:
		std::atomic<TTy> maxNumberOfMs{ TTy{ 0 } };
		std::atomic<TTy> startTime{ TTy{ 0 } };
	};
}
