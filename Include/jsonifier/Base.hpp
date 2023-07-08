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

	template<typename ValueType = void> struct Hash {
		static_assert(std::is_integral<ValueType>::value || std::is_enum<ValueType>::value,
			"Hash only supports integral types, specialize for other types.");

		inline constexpr size_t operator()(ValueType const& value, size_t seed) const {
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
	concept MapSubscriptable = requires(ValueType value) {
		{ value[std::declval<typename ValueType::key_type>()] } -> std::same_as<typename ValueType::mapped_type&>;
	};

	template<typename ValueType>
	concept VectorSubscriptable = requires(ValueType value) {
		{ value[size_t{}] } -> std::same_as<typename ValueType::value_type&>;
	};

	template<typename ValueType>
	concept PairT = requires(ValueType value) {
		{ value.first } -> std::same_as<typename ValueType::first_type&>;
		{ value.second } -> std::same_as<typename ValueType::second_type&>;
	};

	template<typename ValueType>
	concept HasSize = requires(ValueType value) {
		{ value.size() } -> std::same_as<size_t>;
	};

	template<typename ValueType>
	concept HasData = requires(ValueType value) {
		{ value.data() };
	};

	template<typename ValueType>
	concept HasSubstr = requires(ValueType value, const size_t pos, size_t len) {
		{ value.substr(pos, len) } -> std::same_as<ValueType>;
	};

	template<typename ValueType>
	concept CharT = std::same_as<RefUnwrap<ValueType>, char> || std::same_as<RefUnwrap<ValueType>, char16_t> ||
		std::same_as<RefUnwrap<ValueType>, char32_t> || std::same_as<RefUnwrap<ValueType>, char8_t>;

	template<typename ValueType>
	concept BoolT = std::same_as<RefUnwrap<ValueType>, bool>;

	template<typename ValueType>
	concept IntT = std::integral<RefUnwrap<ValueType>> && !CharT<RefUnwrap<ValueType>> && !BoolT<ValueType>;

	template<typename ValueType>
	concept NumT = std::floating_point<RefUnwrap<ValueType>> || IntT<ValueType>;

	template<typename ValueType>
	concept StringT = HasSubstr<std::remove_const_t<ValueType>> && HasData<std::remove_const_t<ValueType>> &&
		HasSize<std::remove_const_t<ValueType>> && !std::same_as<char, std::remove_const_t<ValueType>>;

	template<typename ValueType>
	concept MapT = !ComplexT<ValueType> && !StringT<ValueType> && MapSubscriptable<ValueType>;

	template<typename ValueType>
	concept EmplaceBackable = requires(ValueType value) {
		{ value.emplace_back(std::declval<typename ValueType::value_type&>()) };
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

	template<size_t strLength> class StringLiteral {
	  public:
		static constexpr size_t sizeVal = (strLength > 0) ? (strLength - 1) : 0;

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

		inline constexpr size_t size() const noexcept {
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

	template<const Jsonifier::StringView&... Strings> inline constexpr Jsonifier::StringView join() {
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

	template<const Jsonifier::StringView&... Strings> constexpr auto JoinV = join<Strings...>();

	inline decltype(auto) getMember(auto&& value, auto& member_ptr) {
		return value.*member_ptr;
	}

	template<typename ValueType, typename mptr_t> using MemberT = decltype(getMember(std::declval<ValueType>(), std::declval<RefUnwrap<mptr_t>&>()));

	template<typename ValueType>
	concept Emplaceable = requires(ValueType value) {
		{ value.emplace(std::declval<typename ValueType::value_type>()) };
	};

	template<typename ValueType>
	concept PushBackable = requires(ValueType value) {
		{ value.push_back(std::declval<typename ValueType::value_type>()) };
	};

	template<typename ValueType>
	concept Resizeable = requires(ValueType value) { value.resize(0); };
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
		if constexpr (Resizeable<ValueType>) {
			return buffer.data();
		} else {
			return buffer;
		}
	}

	template<typename ValueType>
	concept VectorT = ( !ComplexT<ValueType> && !MapT<ValueType> && VectorSubscriptable<ValueType> && HasData<ValueType> &&
						  EmplaceBackable<ValueType> )&&!JsonifierArrayT<ValueType> &&
		!TupleT<ValueType> && !HasSubstr<ValueType>;

	template<typename ValueType>
	concept RawArrayT = ( !ComplexT<ValueType> && !MapT<ValueType> && VectorSubscriptable<ValueType> && HasData<ValueType> &&
							!EmplaceBackable<ValueType> )&&!JsonifierArrayT<ValueType> &&
		!TupleT<ValueType> && !HasSubstr<ValueType> && !Resizeable<ValueType>;

	template<typename ValueType>
	concept VectorLike = Resizeable<ValueType> && VectorSubscriptable<ValueType> && HasData<ValueType>;

	template<typename ValueType>
	concept CoreType = IsSpecializationV<Jsonifier::Core<ValueType>, Jsonifier::Core>::value;

	template<TimeT TimeType> class StopWatch {
	  public:
		using HRClock = std::chrono::high_resolution_clock;

		inline StopWatch() = delete;

		inline StopWatch<TimeType>& operator=(StopWatch<TimeType>&& data) noexcept {
			maxNumberOfTimeUnits.store(data.maxNumberOfTimeUnits.load());
			startTime.store(data.startTime.load());
			return *this;
		}

		inline StopWatch(StopWatch<TimeType>&& other) noexcept = default;

		inline StopWatch(TimeType maxNumberOfMsNew) noexcept {
			maxNumberOfTimeUnits.store(maxNumberOfMsNew);
			resetTimer();
		}

		inline StopWatch(int64_t maxNumberOfMsNew) noexcept {
			maxNumberOfTimeUnits.store(TimeType{ maxNumberOfMsNew });
			resetTimer();
		}

		inline TimeType totalTimePassed() noexcept {
			return std::chrono::duration_cast<TimeType>(HRClock::now().time_since_epoch()) - startTime.load();
		}

		inline TimeType getTotalWaitTime() noexcept {
			return maxNumberOfTimeUnits.load();
		}

		inline bool hasTimePassed() noexcept {
			if (std::chrono::duration_cast<TimeType>(HRClock::now().time_since_epoch()) - startTime.load() >= maxNumberOfTimeUnits.load()) {
				return true;
			} else {
				return false;
			}
		}

		inline void resetTimer() noexcept {
			startTime.store(std::chrono::duration_cast<TimeType>(HRClock::now().time_since_epoch()));
		}

	  protected:
		std::atomic<TimeType> maxNumberOfTimeUnits{ TimeType{ 0 } };
		std::atomic<TimeType> startTime{ TimeType{ 0 } };
	};
}
