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

#if !defined __GNUC__
	#pragma warning(push)
	#pragma warning(disable : 4244)
#endif

#include <jsonifier/Core.hpp>
#include <jsonifier/Error.hpp>
#include <jsonifier/Pair.hpp>
#include <jsonifier/RawJsonData.hpp>
#include <jsonifier/StringView.hpp>
#include <jsonifier/Tuple.hpp>

#include <bitset>
#include <chrono>
#include <concepts>
#include <functional>
#include <immintrin.h>
#include <iostream>
#include <map>
#include <span>
#include <string_view>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <variant>
#include <vector>

namespace JsonifierInternal {

	class Serializer;

	template<typename ValueType> struct always_false : std::false_type {};

	template<typename ValueType> constexpr bool always_false_v = always_false<ValueType>::value;

	template<typename ValueType> void printTypeInCompilationError(ValueType&&) noexcept {
		static_assert(always_false_v<ValueType>, "Compilation failed because you failed to specialize the Core<> template for the following class:");
	}

	template<typename ValueType = void> struct Hash {
		static_assert(std::is_integral<ValueType>::value || std::is_enum<ValueType>::value, "Hash only supports integral types, specialize for other types.");

		constexpr std::size_t operator()(ValueType const& value, std::size_t seed) const {
			std::size_t key = seed ^ static_cast<std::size_t>(value);
			key				= (~key) + (key << 21);
			key				= key ^ (key >> 24);
			key				= (key + (key << 3)) + (key << 8);
			key				= key ^ (key >> 14);
			key				= (key + (key << 2)) + (key << 4);
			key				= key ^ (key >> 28);
			key				= key + (key << 31);
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
	concept VectorSubscriptable = requires(ValueType value, uint64_t index) {
		{ value[index] } -> std::same_as<typename std::remove_reference_t<ValueType>::reference>;
	} || requires(ValueType value, uint64_t index) {
		{ value[index] } -> std::same_as<typename std::remove_reference_t<ValueType>::const_reference>;
	};

	template<typename ValueType>
	concept PairT = requires(ValueType value) {
		{ value.first } -> std::same_as<typename ValueType::first_type&>;
		{ value.second } -> std::same_as<typename ValueType::second_type&>;
	};

	template<typename ValueType>
	concept BoolT = std::same_as<RefUnwrap<ValueType>, bool>;

	template<typename ValueType>
	concept IntegerT = std::integral<ValueType>;

	template<typename ValueType>
	concept FloatT = std::floating_point<std::decay_t<ValueType>> && !BoolT<std::decay_t<ValueType>>;

	template<typename ValueType>
	concept SignedT = std::signed_integral<std::decay_t<ValueType>> && !BoolT<std::decay_t<ValueType>>;

	template<typename ValueType>
	concept UnsignedT = std::unsigned_integral<std::decay_t<ValueType>> && !BoolT<std::decay_t<ValueType>> && !SignedT<ValueType>;

	template<typename ValueType>
	concept NumT = FloatT<ValueType> || UnsignedT<ValueType> || SignedT<ValueType>;

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

	template<typename... Args> constexpr bool FalseV = FalseT<Args...>::value;

	template<std::size_t strLength, typename ValueType> class StringLiteral {
	  public:
		static constexpr std::size_t sizeVal = (strLength > 0) ? (strLength - 1) : 0;

		constexpr StringLiteral() noexcept = default;

		constexpr StringLiteral(const ValueType (&str)[strLength]) {
			std::copy(str, str + strLength, string);
		}

		constexpr const ValueType* data() const {
			return string;
		}

		constexpr const ValueType* begin() const {
			return string;
		}

		constexpr const ValueType* end() const {
			return string + sizeVal;
		}

		constexpr std::size_t size() const {
			return sizeVal;
		}

		constexpr const Jsonifier::StringView stringView() const {
			return { string, sizeVal };
		}

		ValueType string[strLength];
	};

	template<std::size_t Count> constexpr auto stringLiteralFromView(const Jsonifier::StringViewBase<char>& str) {
		constexpr StringLiteral<Count + 1, char> string{};
		std::copy_n(str.data(), str.size(), string.string);
		*(string.string + Count) = '\0';
		return string;
	}

	template<StringLiteral str> struct CharsImpl {
		static constexpr Jsonifier::StringView value{ str.string, JsonifierInternal::CharTraits<char>::length(str.string) };
	};

	template<StringLiteral str> constexpr Jsonifier::StringView Chars = CharsImpl<str>::value;

	template<typename ValueType>
	concept StdTupleT = IsSpecializationV<ValueType, Tuplet::Tuple>::value || IsSpecializationV<ValueType, Pair>::value;

	template<typename = void, std::size_t... Indices> constexpr auto indexer(std::index_sequence<Indices...>) {
		return [](auto&& f) noexcept -> decltype(auto) {
			return decltype(f)(f)(std::integral_constant<std::size_t, Indices>{}...);
		};
	}

	template<std::size_t... Is> struct Sequence {};
	template<std::size_t Count, std::size_t... Is> struct GenSeq : GenSeq<Count - 1, Count - 1, Is...> {};
	template<std::size_t... Is> struct GenSeq<0, Is...> : Sequence<Is...> {};

	template<std::size_t N1, std::size_t... I1, std::size_t N2, std::size_t... I2>
	constexpr std::array<const char, N1 + N2 - 1> concat(const char (&a1)[N1], const char (&a2)[N2], Sequence<I1...>, Sequence<I2...>) {
		return { { a1[I1]..., a2[I2]... } };
	}

	template<typename ValueType01, std::size_t size01, const std::array<ValueType01, size01>& string01, typename ValueType02, std::size_t size02>
	constexpr auto concatArrays(const ValueType02 (&string02)[size02]) {
		std::array<char, string01.size() + size02> returnArray{};
		std::copy(string01.data(), string01.data() + string01.size(), returnArray.data());
		std::copy(string02, string02 + size02, returnArray.data() + string01.size());
		return returnArray;
	}

	template<typename ValueType01, const Jsonifier::StringViewBase<ValueType01>& string01, typename ValueType02, std::size_t size>
	constexpr auto concatArrays(const ValueType02 (&string02)[size]) {
		std::array<char, string01.size() + size> returnArray{};
		std::copy(string01.data(), string01.data() + string01.size(), returnArray.data());
		std::copy(string02, string02 + size, returnArray.data() + string01.size());
		return returnArray;
	}

	template<typename ValueType01, const Jsonifier::StringViewBase<ValueType01>& string01, typename ValueType02, const Jsonifier::StringViewBase<ValueType02>& string02>
	constexpr auto concatArrays() {
		std::array<char, string01.size() + string02.size()> returnArray{};
		std::copy(string01.data(), string01.data() + string01.size(), returnArray.data());
		std::copy(string02.data(), string02.data() + string02.size(), returnArray.data() + string01.size());
		return returnArray;
	}

	template<std::size_t n> constexpr auto indexer() {
		return indexer(std::make_index_sequence<n>{});
	}

	template<std::size_t n, typename Func> constexpr auto forEach(Func&& f) {
		return indexer<n>()([&](auto&&... i) {
			(std::forward<std::remove_cvref_t<Func>>(f)(i), ...);
		});
	}

	template<RawArray newArr> struct MakeStatic {
		static constexpr auto value = newArr;
	};

	template<const Jsonifier::StringView&... Strings> constexpr Jsonifier::StringView join() {
		constexpr auto joinedArr = []() {
			constexpr std::size_t len = (Strings.size() + ... + 0);
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
}// namespace JsonifierInternal

namespace Jsonifier {

	template<typename ValueType> struct Array {
		ValueType parseValue;
	};

	template<typename ValueType> Array(ValueType) -> Array<ValueType>;

	template<typename ValueType> struct Object {
		ValueType parseValue;
	};

	template<typename ValueType> Object(ValueType) -> Object<ValueType>;

	constexpr auto array(auto&&... args) {
		return Array{ JsonifierInternal::Tuplet::copyTuple(args...) };
	}

	constexpr auto object(auto&&... args) {
		if constexpr (sizeof...(args) == 0) {
			return Object{ JsonifierInternal::Tuplet::Tuple{} };
		} else {
			return Object{ JsonifierInternal::GroupBuilder<JsonifierInternal::RefUnwrap<decltype(JsonifierInternal::Tuplet::copyTuple(args...))>>::op(
				JsonifierInternal::Tuplet::copyTuple(args...)) };
		}
	}

}// namespace Jsonifier

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
	concept TimeT = std::same_as<ValueType, std::chrono::nanoseconds> || std::same_as<ValueType, std::chrono::microseconds> || std::same_as<ValueType, std::chrono::milliseconds> ||
		std::same_as<ValueType, std::chrono::minutes> || std::same_as<ValueType, std::chrono::hours> || std::same_as<ValueType, std::chrono::days>;

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
	concept ArrayT =
		( !ComplexT<ValueType> && !MapT<ValueType> && VectorSubscriptable<ValueType> && HasData<ValueType> && HasEmplaceBack<ValueType> )&&!JsonifierArrayT<ValueType> &&
			!TupleT<ValueType> && !HasSubstr<ValueType> && requires(ValueType data) { typename ValueType::value_type; } ||
		IsSpecializationV<ValueType, Jsonifier::Vector>::value;

	template<typename ValueType>
	concept RawArrayT =
		( ( !ComplexT<ValueType> && !MapT<ValueType> && VectorSubscriptable<ValueType> && HasData<ValueType> && !HasEmplaceBack<ValueType> )&&!JsonifierArrayT<ValueType> &&
			!TupleT<ValueType> && !HasSubstr<ValueType> && !HasResize<ValueType> ) ||
		std::is_array_v<ValueType>;

	template<typename ValueType>
	concept VectorLike = HasResize<ValueType> && VectorSubscriptable<ValueType> && HasData<ValueType>;

	template<typename ValueType>
	concept CoreType = IsSpecializationV<Jsonifier::Core<ValueType>, Jsonifier::Core>::value;

	template<TimeT TimeType> class StopWatch {
	  public:
		inline StopWatch() = default;

		using HRClock = std::chrono::high_resolution_clock;

		inline StopWatch& operator=(StopWatch&& other) noexcept {
			maxNumberOfTimeUnits.store(other.maxNumberOfTimeUnits.load(std::memory_order_acquire), std::memory_order_release);
			startTime.store(other.startTime.load(std::memory_order_acquire), std::memory_order_release);
			return *this;
		}

		inline StopWatch(StopWatch&& other) noexcept {
			*this = std::move(other);
		}

		StopWatch& operator=(const StopWatch& data) = delete;
		StopWatch(const StopWatch& other)			= delete;

		inline StopWatch(uint64_t maxNumberOfTimeUnitsNew) {
			maxNumberOfTimeUnits.store(TimeType{ maxNumberOfTimeUnitsNew }, std::memory_order_release);
		}

		inline StopWatch(TimeType maxNumberOfTimeUnitsNew) {
			maxNumberOfTimeUnits.store(maxNumberOfTimeUnitsNew, std::memory_order_release);
		}

		inline TimeType totalTimePassed() const {
			return std::chrono::duration_cast<TimeType>(HRClock::now().time_since_epoch()) - startTime.load(std::memory_order_acquire);
		}

		inline TimeType getTotalWaitTime() const {
			return maxNumberOfTimeUnits.load(std::memory_order_acquire);
		}

		inline bool hasTimePassed() const {
			if (std::chrono::duration_cast<TimeType>(HRClock::now().time_since_epoch()) - startTime.load(std::memory_order_acquire) >=
				maxNumberOfTimeUnits.load(std::memory_order_acquire)) {
				return true;
			} else {
				return false;
			}
		}

		inline void resetTimer() {
			startTime.store(std::chrono::duration_cast<TimeType>(HRClock::now().time_since_epoch()), std::memory_order_release);
		}

	  protected:
		std::atomic<TimeType> maxNumberOfTimeUnits{ TimeType{ 0 } };
		std::atomic<TimeType> startTime{ TimeType{ 0 } };
	};

	template<TimeT TimeType> StopWatch(TimeType) -> StopWatch<TimeType>;
}// namespace JsonifierInternal