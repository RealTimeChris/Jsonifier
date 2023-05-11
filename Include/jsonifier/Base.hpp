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

#include <jsonifier/RawJsonData.hpp>
#include <jsonifier/StringView.hpp>
#include <jsonifier/Tuple.hpp>
#include <jsonifier/Error.hpp>
#include <jsonifier/Core.hpp>
#include <jsonifier/Pair.hpp>

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

namespace JsonifierInternal {

	class Serializer;

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
	concept ComplexT = JsonifierT<RefUnwrap<OTy>>;

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
	concept CharT = std::same_as<RefUnwrap<OTy>, char> || std::same_as<RefUnwrap<OTy>, char16_t> || std::same_as<RefUnwrap<OTy>, char32_t> ||
		std::same_as<RefUnwrap<OTy>, char8_t>;

	template<typename OTy>
	concept BoolT = std::same_as<RefUnwrap<OTy>, bool>;

	template<typename OTy>
	concept IntT = std::integral<RefUnwrap<OTy>> && !CharT<RefUnwrap<OTy>> && !BoolT<OTy>;

	template<typename OTy>
	concept NumT = std::floating_point<RefUnwrap<OTy>> || IntT<OTy>;

	template<typename OTy>
	concept StringT = HasSubstr<std::remove_const_t<OTy>> && HasData<std::remove_const_t<OTy>> && HasSize<std::remove_const_t<OTy>> &&
		!std::same_as<char, std::remove_const_t<OTy>>;

	template<typename OTy>
	concept MapT = !ComplexT<OTy> && !StringT<OTy> && MapSubscriptable<OTy>;

	template<typename OTy>
	concept EmplaceBackable = requires(OTy value) {
		{ value.emplace_back(std::declval<typename OTy::value_type&>()) };
	};

	template<typename T>
	concept UniquePtrT = std::is_same_v<T, std::unique_ptr<typename T::element_type>>;

	template<typename Container>
	concept HasFind = requires(Container c, const typename Container::key_type& val) {
		{ c.find(val) };
	};

	template<typename OTy>
	concept HasExcludedKeys = requires(OTy value) {
		{ value.excludedKeys };
	};

	template<typename OTy>
	concept NullT = std::same_as<OTy, std::nullptr_t>;

	template<typename... Args> struct FalseT : std::false_type {};

	struct RandomCoreType;

	template<> struct FalseT<RandomCoreType> : std::true_type {};

	template<typename... Args> inline constexpr bool FalseV = FalseT<Args...>::value;

	template<size_t n> inline constexpr size_t length(const char (&)[n]) noexcept {
		return n;
	}

	template<size_t strLength> class StringLiteral {
	  public:
		static constexpr size_t sizeVal = (strLength > 0) ? (strLength - 1) : 0;

		inline constexpr StringLiteral() noexcept = default;

		inline constexpr StringLiteral(const char (&str)[strLength]) {
			std::copy_n(str, strLength, string);
		}

		inline constexpr const char* data() const noexcept {
			return string;
		}

		inline constexpr const char* begin() const noexcept {
			return string;
		}

		inline constexpr const char* end() const noexcept {
			return string + sizeVal;
		}

		inline constexpr size_t size() const noexcept {
			return sizeVal;
		}

		inline constexpr const Jsonifier::StringView stringView() const noexcept {
			return { string, sizeVal };
		}

		char string[strLength];
	};

	template<StringLiteral str> struct CharsImpl {
		static constexpr Jsonifier::StringView value{ str.string, length(str.string) - 1 };
	};

	template<StringLiteral str> inline constexpr Jsonifier::StringView Chars = CharsImpl<str>::value;

	template<typename OTy>
	concept StdTupleT = IsSpecializationV<OTy, Tuplet::Tuple>::value || IsSpecializationV<OTy, Pair>::value;

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

	template<RawArray newArr> struct MakeStatic {
		static constexpr auto value = newArr;
	};

	struct StringViewTwo {
		size_t sizeVal{};
		const char* str{};

		constexpr StringViewTwo(const Jsonifier::StringView& stringNew) {
			sizeVal = stringNew.size();
			str = stringNew.data();
		}

		constexpr const char* begin() const noexcept {
			return str;
		}

		constexpr const char* end() const noexcept {
			return str + sizeVal;
		}

		constexpr size_t size() const noexcept {
			return sizeVal;
		}
	};

#ifdef _MSC_VER
	template<StringViewTwo... Strings>
#else
	template<const Jsonifier::StringView&... Strings>
#endif
	inline constexpr Jsonifier::StringView Join() {
		constexpr auto joinedArr = []() {
			constexpr size_t len = (Strings.size() + ... + 0);
			RawArray<char, len + 1> arr{};
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
	template<StringViewTwo... Strings>
#else
	template<const Jsonifier::StringView&... Strings>
#endif
	constexpr auto JoinV = Join<Strings...>();

	template<size_t strLength> inline constexpr auto stringLiteralFromView(Jsonifier::StringView str) {
		StringLiteral<strLength + 1> sl{};
		std::copy_n(str.data(), str.size(), sl.str);
		*(sl.str + strLength) = '\0';
		return sl;
	}

	inline decltype(auto) getMember(auto&& value, auto& member_ptr) {
		using VTy = RefUnwrap<decltype(member_ptr)>;
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

	template<typename OTy, typename mptr_t> using MemberT = decltype(getMember(std::declval<OTy>(), std::declval<RefUnwrap<mptr_t>&>()));

	template<size_t... Is> struct Sequence {};
	template<size_t n, size_t... Is> struct GenerateSequence : GenerateSequence<n - 1, n - 1, Is...> {};
	template<size_t... Is> struct GenerateSequence<0, Is...> : Sequence<Is...> {};

	template<size_t N1, size_t... I1, size_t N2, size_t... I2>
	inline constexpr RawArray<char const, N1 + N2 - 1> concat(char const (&a1)[N1], char const (&a2)[N2], Sequence<I1...>, Sequence<I2...>) {
		return { { a1[I1]..., a2[I2]... } };
	}

	template<size_t N1, size_t N2> inline constexpr RawArray<char const, N1 + N2 - 1> concatArrays(char const (&a1)[N1], char const (&a2)[N2]) {
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
}

namespace Jsonifier {

	template<typename OTy> struct Array {
		OTy parseValue;
	};

	template<typename OTy> Array(OTy) -> Array<OTy>;

	template<typename OTy> struct Object {
		OTy parseValue;
	};

	template<typename OTy> Object(OTy) -> Object<OTy>;

	inline constexpr auto array(auto&&... args) {
		return Array{ JsonifierInternal::Tuplet::copyTuple(args...) };
	}

	inline constexpr auto object(auto&&... args) {
		if constexpr (sizeof...(args) == 0) {
			return Object{ JsonifierInternal::Tuplet::Tuple{} };
		} else {
			return Object{ JsonifierInternal::GroupBuilder<JsonifierInternal::RefUnwrap<decltype(JsonifierInternal::Tuplet::copyTuple(args...))>>::op(
				JsonifierInternal::Tuplet::copyTuple(args...)) };
		}
	}

}

namespace JsonifierInternal {

	template<typename OTy>
	concept RawJsonT = std::same_as<OTy, Jsonifier::RawJsonData> && !StringT<OTy>;

	template<typename OTy>
	concept JsonifierArrayT = JsonifierT<OTy> && IsSpecializationV<CoreWrapperT<OTy>, Jsonifier::Array>::value;

	template<typename OTy>
	concept JsonifierObjectT = JsonifierT<OTy> && IsSpecializationV<CoreWrapperT<OTy>, Jsonifier::Object>::value;

	template<typename OTy>
	concept JsonifierValueT = JsonifierT<OTy> && !(JsonifierArrayT<OTy> || JsonifierObjectT<OTy>);

	template<typename OTy>
	concept ArrayTupleT = JsonifierArrayT<OTy> || TupleT<RefUnwrap<OTy>>;

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
	concept VectorT = ( !ComplexT<OTy> && !MapT<OTy> && VectorSubscriptable<OTy> && HasData<OTy> && EmplaceBackable<OTy> )&&!JsonifierArrayT<OTy> &&
		!TupleT<OTy> && !HasSubstr<OTy>;

	template<typename OTy>
	concept RawArrayT =
		( !ComplexT<OTy> && !MapT<OTy> && VectorSubscriptable<OTy> && HasData<OTy> && !EmplaceBackable<OTy> )&&!JsonifierArrayT<OTy> &&
		!TupleT<OTy> && !HasSubstr<OTy> && !Resizeable<OTy>;

	template<typename OTy>
	concept VectorLike = Resizeable<OTy> && VectorSubscriptable<OTy> && HasData<OTy>;

	template<typename OTy>
	concept CoreType = IsSpecializationV<Jsonifier::Core<OTy>, Jsonifier::Core>::value;

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
