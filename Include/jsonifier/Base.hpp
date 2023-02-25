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
/// https://github.com/RealTimeChris/Jsonifier
/// Feb 3, 2023
#pragma once

#pragma warning(push)
#pragma warning(disable : 4244)
#pragma warning(disable : 4251)

#include <jsonifier/StringView.hpp>
#include <jsonifier/Core.hpp>
#include <jsonifier/Tuple.hpp>

#include <source_location>
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

#ifdef max
	#undef max
#endif
#ifdef min
	#undef min
#endif

namespace Jsonifier {

	class SimdIteratorCore;
	class Serializer;
	class StringView;
	class String;
	class Parser;

	template<typename OTy = void> struct Hash {
		static_assert(std::is_integral<OTy>::value || std::is_enum<OTy>::value, "only supports integral types, specialize for other types");

		inline constexpr std::size_t operator()(OTy const& value, std::size_t seed) const {
			std::size_t key = seed ^ static_cast<std::size_t>(value);
			key = (~key) + (key << 21);// key = (key << 21) - key - 1;
			key = key ^ (key >> 24);
			key = (key + (key << 3)) + (key << 8);// key * 265
			key = key ^ (key >> 14);
			key = (key + (key << 2)) + (key << 4);// key * 21
			key = key ^ (key >> 28);
			key = key + (key << 31);
			return key;
		}
	};

	template<> struct Hash<void> {
		template<typename OTy> inline constexpr std::size_t operator()(OTy const& value, std::size_t seed) const {
			return Hash<OTy>{}(value, seed);
		}
	};

	// From:
	// https://stackoverflow.com/questions/16337610/how-to-know-if-a-type-is-a-specialization-of-stdvector
	template<typename Test, template<typename...> class Ref> struct IsSpecializationV : std::false_type {};

	template<template<typename...> class Ref, typename... Args> struct IsSpecializationV<Ref<Args...>, Ref> : std::true_type {};

	template<typename OTy>
	concept Range = requires(OTy& t) {
		typename OTy::value_type;
		requires !std::same_as<void, decltype(t.begin())>;
		requires !std::same_as<void, decltype(t.end())>;
	};

	template<class OTy>
	concept ComplexT = JsonifierT<std::decay_t<OTy>>;

	template<typename OTy>
	concept TupleT = requires(OTy t) {
		std::tuple_size<OTy>::value;
		Tuplet::get<0>(t);
	}
	&&!ComplexT<OTy> && !Range<OTy>;

	template<typename OTy>
	concept MapSubscriptable = requires(OTy container) {
		{ container[std::declval<typename OTy::key_type>()] } -> std::same_as<typename OTy::mapped_type&>;
	};

	template<typename OTy>
	concept PairT = requires(OTy pair) {
		{ pair.first } -> std::same_as<typename OTy::first_type&>;
		{ pair.second } -> std::same_as<typename OTy::second_type&>;
	};

	template<class OTy>
	concept CharT = std::same_as < std::decay_t<OTy>,
	char > || std::same_as<std::decay_t<OTy>, char16_t> || std::same_as<std::decay_t<OTy>, char32_t> || std::same_as<std::decay_t<OTy>, char>;

	template<class OTy>
	concept BoolT = std::same_as < std::decay_t<OTy>,
	bool > || std::same_as<std::decay_t<OTy>, std::vector<bool>::reference>;

	template<class OTy>
	concept IntT = std::integral<std::decay_t<OTy>> && !CharT<std::decay_t<OTy>> && !BoolT<OTy>;

	template<class OTy>
	concept NumT = std::floating_point<std::decay_t<OTy>> || IntT<OTy>;

	template<class OTy>
	concept StringT = !ComplexT<OTy> && !std::same_as<std::nullptr_t, OTy> && std::convertible_to<std::decay_t<OTy>, StringView>;

	template<typename OTy> using IteratorT = decltype(std::begin(std::declval<OTy&>()));

	template<Range R> using RangeValueT = std::iter_value_t<IteratorT<R>>;

	template<typename OTy>
	concept MapT = !ComplexT<OTy> && !StringT<OTy> && Range<OTy> && PairT<RangeValueT<OTy>> && MapSubscriptable<OTy>;

	template<typename OTy>
	concept ArrayT = (!ComplexT<OTy> && !StringT<OTy> && !MapT<OTy> && Range<OTy>);

	template<typename OTy> struct Array { OTy value; };

	template<typename OTy> Array(OTy) -> Array<OTy>;

	template<typename OTy> struct Object { OTy value; };

	template<typename OTy> Object(OTy) -> Object<OTy>;

	inline constexpr auto size(const char* S) noexcept {
		int i = 0;
		while (*S != 0) {
			++i;
			++S;
		}
		return i;
	}

	template<class... Args> struct FalseT : std::false_type {};
	namespace detail {
		struct aggressive_unicorn_type;// Do not unleash
	}
	template<> struct FalseT<detail::aggressive_unicorn_type> : std::true_type {};

	template<class... Args> inline constexpr bool FalseV = FalseT<Args...>::value;

	template<size_t N> struct StringLiteral {
		static constexpr size_t size = (N > 0) ? (N - 1) : 0;

		inline constexpr StringLiteral() = default;

		inline constexpr StringLiteral(const char (&str)[N]) {
			std::copy_n(str, N, value);
		}

		char value[N];
		inline constexpr const char* end() const noexcept {
			return value + size;
		}

		inline constexpr const StringView stringView() const noexcept {
			return { value, size };
		}
	};

	template<size_t N> inline constexpr auto stringLiteralFromView(StringView str) {
		StringLiteral<N + 1> sl{};
		std::copy_n(str.data(), str.size(), sl.value);
		*(sl.value + N) = '\0';
		return sl;
	}

	template<size_t N> inline constexpr size_t length(char const (&)[N]) noexcept {
		return N;
	}

	template<typename OTy>
	concept IsStdTuple = IsSpecializationV<OTy, std::tuple>::value || IsSpecializationV<OTy, std::pair>::value;

	template<typename OTy, std::size_t N> class RawVector {
		OTy data[N] = {};// zero-initialization for scalar type OTy, default-initialized otherwise
		std::size_t dsize = 0;

	  public:
		// Container typdefs
		using value_type = OTy;
		using reference = value_type&;
		using const_reference = const value_type&;
		using pointer = value_type*;
		using const_pointer = const value_type*;
		using iterator = pointer;
		using const_iterator = const_pointer;
		using size_type = std::size_t;
		using difference_type = std::ptrdiff_t;

		// Constructors
		inline constexpr RawVector(void) = default;
		inline constexpr RawVector(size_type count, const auto& value) : dsize(count) {
			for (std::size_t i = 0; i < N; ++i)
				data[i] = value;
		}

		// Iterators
		inline constexpr iterator begin() noexcept {
			return data;
		}
		inline constexpr iterator end() noexcept {
			return data + dsize;
		}

		// Capacity
		inline constexpr size_type size() const {
			return dsize;
		}

		// Element access
		inline constexpr reference operator[](std::size_t index) {
			return data[index];
		}
		inline constexpr const_reference operator[](std::size_t index) const {
			return data[index];
		}

		inline constexpr reference back() {
			return data[dsize - 1];
		}
		inline constexpr const_reference back() const {
			return data[dsize - 1];
		}

		// Modifiers
		inline constexpr void push_back(const OTy& a) {
			data[dsize++] = a;
		}
		inline constexpr void push_back(OTy&& a) {
			data[dsize++] = std::move(a);
		}
		inline constexpr void pop_back() {
			--dsize;
		}

		inline constexpr void clear() {
			dsize = 0;
		}
	};

	template<typename OTy, std::size_t N> class RawArray {
		OTy data_[N] = {};// zero-initialization for scalar type OTy, default-initialized otherwise

		template<std::size_t M, std::size_t... I> inline constexpr RawArray(OTy const (&init)[M], std::index_sequence<I...>) : data_{ init[I]... } {
		}
		template<typename Iter, std::size_t... I> inline constexpr RawArray(Iter iter, std::index_sequence<I...>) : data_{ (( void )I, *iter++)... } {
		}

	  public:
		// Container typdefs
		using value_type = OTy;
		using reference = value_type&;
		using const_reference = const value_type&;
		using pointer = value_type*;
		using const_pointer = const value_type*;
		using iterator = pointer;
		using const_iterator = const_pointer;
		using reverse_iterator = std::reverse_iterator<iterator>;
		using const_reverse_iterator = std::reverse_iterator<const_iterator>;
		using size_type = std::size_t;
		using difference_type = std::ptrdiff_t;

		// Constructors
		inline constexpr RawArray(void) = default;
		template<std::size_t M> inline constexpr RawArray(OTy const (&init)[M]) : RawArray(init, std::make_index_sequence<N>()) {
			static_assert(M >= N, "Cannot initialize a RawArray with an smaller array");
		}
		template<std::size_t M> inline constexpr RawArray(std::array<OTy, M> const& init) : RawArray(&init[0], std::make_index_sequence<N>()) {
			static_assert(M >= N, "Cannot initialize a RawArray with an smaller array");
		}
		inline constexpr RawArray(std::initializer_list<OTy> init) : RawArray(init.begin(), std::make_index_sequence<N>()) {
		}

		// Iterators
		inline constexpr iterator begin() noexcept {
			return data_;
		}
		inline constexpr const_iterator begin() const noexcept {
			return data_;
		}
		inline constexpr const_iterator cbegin() const noexcept {
			return data_;
		}
		inline constexpr iterator end() noexcept {
			return data_ + N;
		}
		inline constexpr const_iterator end() const noexcept {
			return data_ + N;
		}
		inline constexpr const_iterator cend() const noexcept {
			return data_ + N;
		}

		inline constexpr reverse_iterator rbegin() noexcept {
			return reverse_iterator(end());
		}
		inline constexpr const_reverse_iterator rbegin() const noexcept {
			return const_reverse_iterator(end());
		}
		inline constexpr const_reverse_iterator crbegin() const noexcept {
			return const_reverse_iterator(end());
		}
		inline constexpr reverse_iterator rend() noexcept {
			return reverse_iterator(begin());
		}
		inline constexpr const_reverse_iterator rend() const noexcept {
			return const_reverse_iterator(begin());
		}
		inline constexpr const_reverse_iterator crend() const noexcept {
			return const_reverse_iterator(begin());
		}

		// Capacity
		inline constexpr size_type size() const {
			return N;
		}
		inline constexpr size_type maxSize() const {
			return N;
		}

		// Element access
		inline constexpr reference operator[](std::size_t index) {
			return data_[index];
		}
		inline constexpr const_reference operator[](std::size_t index) const {
			return data_[index];
		}

		inline constexpr reference at(std::size_t index) noexcept {
			if (index > N) {
				std::abort();
			}
			return data_[index];
		}
		inline constexpr const_reference at(std::size_t index) const noexcept {
			if (index > N) {
				std::abort();
			}
			return data_[index];
		}

		inline constexpr reference front() {
			return data_[0];
		}
		inline constexpr const_reference front() const {
			return data_[0];
		}

		inline constexpr reference back() {
			return data_[N - 1];
		}
		inline constexpr const_reference back() const {
			return data_[N - 1];
		}

		inline constexpr value_type* data() noexcept {
			return data_;
		}
		inline constexpr const value_type* data() const noexcept {
			return data_;
		}

		// Modifiers
		inline constexpr void fill(const value_type& val) {
			for (std::size_t i = 0; i < N; ++i)
				data_[i] = val;
		}
	};
	template<typename OTy> class RawArray<OTy, 0> {
	  public:
		// Container typdefs
		using value_type = OTy;
		using reference = value_type&;
		using const_reference = const value_type&;
		using pointer = value_type*;
		using const_pointer = const value_type*;
		using iterator = pointer;
		using const_iterator = const_pointer;
		using reverse_iterator = std::reverse_iterator<iterator>;
		using const_reverse_iterator = std::reverse_iterator<const_iterator>;
		using size_type = std::size_t;
		using difference_type = std::ptrdiff_t;

		// Constructors
		inline constexpr RawArray(void) = default;
	};


	template<class = void, std::size_t... Is> constexpr auto indexer(std::index_sequence<Is...>) {
		return [](auto&& f) -> decltype(auto) {
			return decltype(f)(f)(std::integral_constant<std::size_t, Is>{}...);
		};
	}

	// takes a number N
	// returns a function object that, when passed a function object f
	// passes it compile-time values from 0 to N-1 inclusive.
	template<size_t N> constexpr auto indexer() {
		return indexer(std::make_index_sequence<N>{});
	}

	template<size_t N, class Func> constexpr auto forEach(Func&& f) {
		return indexer<N>()([&](auto&&... i) {
			(std::forward<Func>(f)(i), ...);
		});
	}

	template<const StringView&... Strs> struct Join {
		// Join all strings into a single std::array of chars
		inline static constexpr auto impl() noexcept {
			// This local copy to a Tuple and avoiding of parameter pack expansion is needed to avoid MSVC internal
			// compiler errors
			constexpr size_t len = (Strs.size() + ... + 0);
			std::array<char, len + 1> arr{};
			auto append = [i = 0, &arr](const auto& S) mutable {
				for (auto c: S)
					arr[i++] = c;
			};
			(append(Strs), ...);
			arr[len] = 0;
			return arr;
		}

		static constexpr auto arr = impl();// Give the joined string inline static storage
		static constexpr StringView value{ arr.data(), arr.size() - 1 };
	};
	// Helper to get the value out
	template<const StringView&... Strs> static constexpr auto JoinV = Join<Strs...>::value;

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

	template<size_t N, size_t I = 0, typename Func, typename Value> inline constexpr void forEachValue(Func&& f, Value&& v) {
		if constexpr (I != N) {
			forEachValue<N, I + 1>(f, f(std::integral_constant<size_t, I>{}, v));
		}
	}

	template<typename OTy>
	concept EmplaceBackable = requires(OTy container) {
		{ container.emplace_back() } -> std::same_as<typename OTy::reference>;
	};

	template<class OTy, class mptr_t> using MemberT = decltype(getMember(std::declval<OTy>(), std::declval<std::decay_t<mptr_t>&>()));

	template<class OTy>
	concept NullableT = !ComplexT<OTy> && !StringT<OTy> && requires(OTy t) {
		bool(t);
		{ *t };
	};

	template<size_t... Is> struct seq {};
	template<size_t N, size_t... Is> struct gen_seq : gen_seq<N - 1, N - 1, Is...> {};
	template<size_t... Is> struct gen_seq<0, Is...> : seq<Is...> {};

	template<size_t N1, size_t... I1, size_t N2, size_t... I2>
	constexpr std::array<char const, N1 + N2 - 1> concat(char const (&a1)[N1], char const (&a2)[N2], seq<I1...>, seq<I2...>) {
		return { { a1[I1]..., a2[I2]... } };
	}

	template<size_t N1, size_t N2> constexpr std::array<char const, N1 + N2 - 1> concatArrays(char const (&a1)[N1], char const (&a2)[N2]) {
		return concat(a1, a2, gen_seq<N1 - 1>{}, gen_seq<N2>{});
	}

	template<typename OTy>
	concept HasSize = requires(OTy container) {
		{ container.size() } -> std::same_as<size_t>;
	};

	template<typename OTy>
	concept HasData = requires(OTy container) {
		{ container.data() };
	};

	template<typename OTy>
	concept Emplaceable = requires(OTy container) {
		{ container.emplace(std::declval<typename OTy::value_type>()) };
	};

	template<typename OTy>
	concept PushBackable = requires(OTy container) {
		{ container.push_back(std::declval<typename OTy::value_type>()) };
	};

	template<typename OTy>
	concept Resizeable = requires(OTy container) {
		container.resize(0);
	};

	template<auto& GroupStartArr, auto& GroupSizeArr, typename Tuple, size_t... GroupNumber>
	inline constexpr auto makeGroupsImpl(Tuple&& t, std::index_sequence<GroupNumber...>) {
		return Tuplet::copyTuple(makeGroup<get<GroupNumber>(GroupStartArr)>(t, std::make_index_sequence<std::get<GroupNumber>(GroupSizeArr)>{})...);
	}

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

	template<class OTy>
	concept JsonifierArrayT = JsonifierT<OTy> && IsSpecializationV<CoreWrapperT<OTy>, Array>::value;

	template<class OTy>
	concept JsonifierObjectT = JsonifierT<OTy> && IsSpecializationV<CoreWrapperT<OTy>, Object>::value;

	template<class OTy>
	concept JsonifierValueT = JsonifierT<OTy> && !(JsonifierArrayT<OTy> || JsonifierObjectT<OTy>);

	template<typename OTy>
	concept IsTimeType =
		std::same_as<OTy, std::chrono::nanoseconds> || std::same_as<OTy, std::chrono::microseconds> || std::same_as<OTy, std::chrono::milliseconds> ||
		std::same_as<OTy, std::chrono::minutes> || std::same_as<OTy, std::chrono::hours> || std::same_as<OTy, std::chrono::days>;

	template<typename OTy>
	concept IsConvertibleToSerializer = std::convertible_to<OTy, Serializer>;

	template<typename OTy>
	concept IsEnum = std::is_enum<OTy>::value;

	enum ErrorCode {
		Success = 0,
		Empty = 1,
		Parse_Error = 2,
		String_Error = 3,
		Mem_Alloc_Error = 4,
		Invalid_Number = 5,
		Incorrect_Type = 6,
		Uninitialized = 7,
		Out_Of_Bounds = 8,
		No_Such_JsonDataPtr = 9,
		Number_Error = 10,
		No_String_Error = 11,
		Type_Error = 12,
		No_Such_Entity = 13
	};

	inline std::ostream& operator<<(std::ostream& out, ErrorCode type) {
		switch (type) {
			case Success:
				out << "Success";
				break;
			case Empty:
				out << "Empty";
				break;
			case Parse_Error:
				out << "Parse Error";
				break;
			case String_Error:
				out << "String Error";
				break;
			case Mem_Alloc_Error:
				out << "Memory Allocation Error";
				break;
			case Invalid_Number:
				out << "Invalid Number Error";
				break;
			case Incorrect_Type:
				out << "Incorrect Type Error";
				break;
			case Uninitialized:
				out << "Uninitialized Error";
				break;
			case Out_Of_Bounds:
				out << "Out of Bounds Error";
				break;
			case No_Such_JsonDataPtr:
				out << "No Such JsonData Error";
				break;
			case Number_Error:
				out << "Number Error";
				break;
			case No_String_Error:
				out << "No String Error";
				break;
			case Type_Error:
				out << "Type Error";
				break;
		}
		return out;
	}

	enum class JsonType : uint16_t {
		Unset = 1 << 0,
		Object = 1 << 1,
		Array = 1 << 2,
		String = 1 << 3,
		Float = 1 << 4,
		Uint64 = 1 << 5,
		Int64 = 1 << 6,
		Bool = 1 << 7,
		Null = 1 << 8,
		Number = Float | Uint64 | Int64,
	};

	inline std::ostream& operator<<(std::ostream& out, JsonType type) {
		switch (type) {
			case JsonType::Object:
				out << "Object";
				break;
			case JsonType::Array:
				out << "Array";
				break;
			case JsonType::Number:
				out << "Number";
				break;
			case JsonType::String:
				out << "String";
				break;
			case JsonType::Bool:
				out << "Bool";
				break;
			case JsonType::Null:
				out << "Null";
				break;
			default:
				out << "Unset";
				break;
		}
		return out;
	}

	inline JsonType getType(auto&& iterator) noexcept {
		JsonType type{};
		switch (**iterator) {
			case '{': {
				type = JsonType::Object;
				break;
			}
			case '[': {
				type = JsonType::Array;
				break;
			}
			case '"': {
				type = JsonType::String;
				break;
			}
			case 't':
			case 'f': {
				type = JsonType::Bool;
				break;
			}
			case 'n': {
				type = JsonType::Null;
				break;
			}
			case '-':
			case '0':
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
			case '9': {
				type = JsonType::Number;
				break;
			}
			default: {
				type = JsonType::Unset;
				break;
			}
		}
		return type;
	}

	struct JsonifierException : public std::runtime_error, public std::string {
		JsonifierException(const String&, std::source_location = std::source_location::current()) noexcept;
	};

	template<IsTimeType TTy> class StopWatch {
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

		inline void printTimePassed(StringView stringView) noexcept {
			int64_t timePassed{ this->totalTimePassed().size() };
			this->resetTimer();
		}

		inline void resetTimer() {
			startTime.store(std::chrono::duration_cast<TTy>(HRClock::now().time_since_epoch()));
		}

	  protected:
		std::atomic<TTy> maxNumberOfMs{ TTy{ 0 } };
		std::atomic<TTy> startTime{ TTy{ 0 } };
	};
}