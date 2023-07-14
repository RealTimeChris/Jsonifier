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
/// https://github.com/RealTimeChris/Jsonifier
/// Feb 3, 2023
#pragma once

#ifndef __GNUC__
	#pragma warning(push)
	#pragma warning(disable : 4244)
#endif

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
#include <utility>
#include <vector>
#include <chrono>
#include <bitset>
#include <span>
#include <map>

namespace JsonifierInternal {

	class Serializer;

	template<typename ValueType = void> struct Hash {
		static_assert(std::is_integral<ValueType>::value || std::is_enum<ValueType>::value,
			"Hash only supports integral types, specialize for other types.");

		inline constexpr uint64_t operator()(ValueType const& value, uint64_t seed) const {
			uint64_t key = seed ^ static_cast<uint64_t>(value);
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

	template<typename ValueType>
	concept Range = requires(ValueType& value) {
		typename ValueType::value_type;
		requires !std::same_as<void, decltype(value.begin())>;
		requires !std::same_as<void, decltype(value.end())>;
	};

	template<typename ValueType>
	concept ComplexT = JsonifierT<RefUnwrap<ValueType>>;

	template<typename ValueType>
	concept TupleT = requires(ValueType value) {
		std::tuple_size<ValueType>::value;
		Tuplet::get<0>(value);
	} && !ComplexT<ValueType> && !Range<ValueType>;

	template<typename ValueType>
	concept MapSubscriptable = requires(ValueType value, typename ValueType::key_type keyNew) {
		{ value[keyNew] } -> std::same_as<typename ValueType::mapped_type&>;
	};

	template<typename ValueType>
	concept VectorSubscriptable = requires(ValueType value) {
		{ value[uint64_t{}] } -> std::same_as<typename ValueType::value_type&>;
	};

	template<typename ValueType>
	concept PairT = requires(ValueType value) {
		{ value.first } -> std::same_as<typename ValueType::first_type&>;
		{ value.second } -> std::same_as<typename ValueType::second_type&>;
	};

	template<typename ValueType>
	concept HasSize = requires(ValueType value) {
		{ value.size() } -> std::same_as<uint64_t>;
	};

	template<typename ValueType>
	concept HasData = requires(ValueType value) {
		{ value.data() };
	};

	template<typename ValueType>
	concept HasSubstr = requires(ValueType value, const uint64_t pos, uint64_t len) {
		{ value.substr(pos, len) } -> std::same_as<ValueType>;
	};

	template<typename ValueType>
	concept CharT = std::same_as<RefUnwrap<ValueType>, char> || std::same_as<RefUnwrap<ValueType>, char16_t> ||
		std::same_as<RefUnwrap<ValueType>, char32_t> || std::same_as<RefUnwrap<ValueType>, char8_t>;

	template<typename ValueType>
	concept BoolT = std::same_as<RefUnwrap<ValueType>, bool>;

	template<typename ValueType>
	concept FloatT = std::floating_point<std::decay_t<ValueType>> && !BoolT<std::decay_t<ValueType>>;

	template<typename ValueType>
	concept SignedT = std::signed_integral<std::decay_t<ValueType>> && !BoolT<std::decay_t<ValueType>>;

	template<typename ValueType>
	concept UnsignedT = std::unsigned_integral<std::decay_t<ValueType>> && !BoolT<std::decay_t<ValueType>> && !SignedT<ValueType>;

	template<typename ValueType>
	concept NumT = FloatT<ValueType> || UnsignedT<ValueType> || SignedT<ValueType>;

	template<typename ValueType>
	concept StringT = HasSubstr<std::remove_const_t<ValueType>> && HasData<std::remove_const_t<ValueType>> &&
			HasSize<std::remove_const_t<ValueType>> && !std::same_as<char, std::remove_const_t<ValueType>> ||
		std::derived_from<ValueType, std::string>;

	template<typename ValueType>
	concept MapT = requires(ValueType data) {
		typename ValueType::mapped_type;
		typename ValueType::key_type;
	} && Range<ValueType> && MapSubscriptable<ValueType> && !StringT<ValueType> && !ComplexT<ValueType>;

	template<typename ValueType>
	concept HasEmplaceBack = requires(ValueType data, typename ValueType::value_type&& valueNew) {
		{ data.emplace_back(valueNew) } -> std::same_as<typename ValueType::value_type&>;
	};

	template<typename ValueType>
	concept UniquePtrT = requires(ValueType other) {
		typename ValueType::element_type;
		typename ValueType::deleter_type;
		{ other.release() } -> std::same_as<typename ValueType::element_type*>;
	};

	template<typename Container>
	concept HasFind = requires(Container c, const typename Container::key_type& val) {
		{ c.find(val) };
	};

	template<typename ValueType>
	concept HasExcludedKeys = requires(ValueType value) {
		{ value.excludedKeys };
	};

	template<typename ValueType>
	concept NullT = std::same_as<ValueType, std::nullptr_t>;

	template<typename... Args> struct FalseT : std::false_type {};

	struct RandomCoreType;

	template<> struct FalseT<RandomCoreType> : std::true_type {};

	template<typename... Args> inline constexpr bool FalseV = FalseT<Args...>::value;

	template<uint64_t strLength> class StringLiteral {
	  public:
		static constexpr uint64_t sizeVal = (strLength > 0) ? (strLength - 1) : 0;

		inline constexpr StringLiteral() noexcept = default;

		inline constexpr StringLiteral(const char (&str)[strLength]) {
			std::copy(str, str + strLength, string);
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

		inline constexpr uint64_t size() const noexcept {
			return sizeVal;
		}

		inline constexpr const Jsonifier::StringView stringView() const noexcept {
			return { string, sizeVal };
		}

		char string[strLength];
	};

	template<StringLiteral str> struct CharsImpl {
		static constexpr Jsonifier::StringView value{ str.string, std::char_traits<char>::length(str.string) };
	};

	template<StringLiteral str> inline constexpr Jsonifier::StringView Chars = CharsImpl<str>::value;

	template<typename ValueType>
	concept StdTupleT = IsSpecializationV<ValueType, Tuplet::Tuple>::value || IsSpecializationV<ValueType, Pair>::value;

	template<class = void, std::size_t... Is> inline constexpr auto indexer(std::index_sequence<Is...>) noexcept {
		return [](auto&& f) noexcept -> decltype(auto) {
			return decltype(f)(f)(std::integral_constant<std::size_t, Is>{}...);
		};
	}

	template<uint64_t n> inline constexpr auto indexer() {
		return indexer(std::make_index_sequence<n>{});
	}

	template<uint64_t n, typename Func> inline constexpr auto forEach(Func&& f) {
		return indexer<n>()([&](auto&&... i) {
			(std::forward<Func>(f)(i), ...);
		});
	}

	template<RawArray newArr> struct MakeStatic {
		static constexpr auto value = newArr;
	};

	template<const Jsonifier::StringView&... Strings> inline constexpr Jsonifier::StringView join() {
		constexpr auto joinedArr = []() {
			constexpr uint64_t len = (Strings.size() + ... + 0);
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

	template<const Jsonifier::StringView&... Strings> constexpr auto JoinV = join<Strings...>();

	inline decltype(auto) getMember(auto&& value, auto& member_ptr) {
		using ValueType = std::decay_t<decltype(member_ptr)>;
		if constexpr (std::is_member_object_pointer_v<ValueType>) {
			return value.*member_ptr;
		} else if constexpr (std::is_member_function_pointer_v<ValueType>) {
			return member_ptr;
		} else if constexpr (std::invocable<ValueType, decltype(value)>) {
			return std::invoke(member_ptr, value);
		} else if constexpr (std::is_pointer_v<ValueType>) {
			return *member_ptr;
		} else {
			return member_ptr;
		}
	}

	template<typename ValueType, typename mptr_t> using MemberT = decltype(getMember(std::declval<ValueType>(), std::declval<RefUnwrap<mptr_t>&>()));

	template<typename ValueType>
	concept HasResize = requires(ValueType value) { value.resize(0); };
}

namespace Jsonifier {

	template<typename ValueType> struct Array {
		ValueType parseValue;
	};

	template<typename ValueType> Array(ValueType) -> Array<ValueType>;

	template<typename ValueType> struct Object {
		ValueType parseValue;
	};

	template<typename ValueType> Object(ValueType) -> Object<ValueType>;

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

	template<typename ValueType>
	concept RawJsonT = std::same_as<ValueType, Jsonifier::RawJsonData> && !StringT<ValueType>;

	template<typename ValueType>
	concept JsonifierArrayT = JsonifierT<ValueType> && IsSpecializationV<CoreWrapperT<ValueType>, Jsonifier::Array>::value;

	template<typename ValueType>
	concept JsonifierObjectT = JsonifierT<ValueType> && IsSpecializationV<CoreWrapperT<ValueType>, Jsonifier::Object>::value;

	template<typename ValueType>
	concept ArrayTupleT = JsonifierArrayT<ValueType> || TupleT<RefUnwrap<ValueType>>;

	template<typename ValueType>
	concept ObjectT = MapT<ValueType> || JsonifierObjectT<ValueType>;

	template<typename ValueType>
	concept TimeT = std::same_as<ValueType, std::chrono::nanoseconds> || std::same_as<ValueType, std::chrono::microseconds> ||
		std::same_as<ValueType, std::chrono::milliseconds> || std::same_as<ValueType, std::chrono::minutes> ||
		std::same_as<ValueType, std::chrono::hours> || std::same_as<ValueType, std::chrono::days>;

	template<typename ValueType>
	concept EnumT = std::is_enum<ValueType>::value;

	template<typename ValueType> inline auto dataPtr(ValueType& buffer) {
		if constexpr (HasResize<ValueType>) {
			return buffer.data();
		} else {
			return buffer;
		}
	}

	template<typename ValueType>
	concept ArrayT = ( !ComplexT<ValueType> && !MapT<ValueType> && VectorSubscriptable<ValueType> && HasData<ValueType> &&
						 HasEmplaceBack<ValueType> )&&!JsonifierArrayT<ValueType> &&
		!TupleT<ValueType> && !HasSubstr<ValueType> && requires(ValueType data) { typename ValueType::value_type; };

	template<typename ValueType>
	concept RawArrayT = ( !ComplexT<ValueType> && !MapT<ValueType> && VectorSubscriptable<ValueType> && HasData<ValueType> &&
							!HasEmplaceBack<ValueType> )&&!JsonifierArrayT<ValueType> &&
		!TupleT<ValueType> && !HasSubstr<ValueType> && !HasResize<ValueType>;

	template<typename ValueType>
	concept VectorLike = HasResize<ValueType> && VectorSubscriptable<ValueType> && HasData<ValueType>;

	template<typename ValueType>
	concept CoreType = IsSpecializationV<Jsonifier::Core<ValueType>, Jsonifier::Core>::value;

	template<TimeT TimeType> class StopWatch {
	  public:
		inline StopWatch() = default;

		using HRClock = std::chrono::high_resolution_clock;

		inline StopWatch& operator=(StopWatch&& data) noexcept {
			maxNumberOfTimeUnits.store(data.maxNumberOfTimeUnits.load());
			startTime.store(data.startTime.load());
			return *this;
		}

		inline StopWatch(StopWatch&& other) noexcept {
			*this = std::move(other);
		}

		StopWatch& operator=(const StopWatch& data) = delete;
		StopWatch(const StopWatch& other) = delete;

		inline StopWatch(TimeType maxNumberOfTimeUnitsNew) {
			maxNumberOfTimeUnits.store(maxNumberOfTimeUnitsNew);
			resetTimer();
		}

		inline TimeType totalTimePassed() {
			return std::chrono::duration_cast<TimeType>(HRClock::now().time_since_epoch()) - startTime.load();
		}

		inline TimeType getTotalWaitTime() {
			return maxNumberOfTimeUnits.load();
		}

		inline bool hasTimePassed() {
			if (std::chrono::duration_cast<TimeType>(HRClock::now().time_since_epoch()) - startTime.load() >= maxNumberOfTimeUnits.load()) {
				return true;
			} else {
				return false;
			}
		}

		inline void resetTimer() {
			startTime.store(std::chrono::duration_cast<TimeType>(HRClock::now().time_since_epoch()));
		}

	  protected:
		std::atomic<TimeType> maxNumberOfTimeUnits{ TimeType{ 0 } };
		std::atomic<TimeType> startTime{ TimeType{ 0 } };
	};

	template<TimeT TimeType> StopWatch(TimeType) -> StopWatch<TimeType>;
}
