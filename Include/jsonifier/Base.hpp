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
#include <jsonifier/Core.hpp>
#include <jsonifier/Error.hpp>

#include <unordered_map>
#include <immintrin.h>
#include <string_view>
#include <type_traits>
#include <functional>
#include <concepts>
#include <memory>
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

#define JsonifierType(...) \
	[] { \
		constexpr auto map = object(__VA_ARGS__); \
		return map; \
	}()

namespace Jsonifier {

	class SimdIteratorCore;
	class Serializer;
	class StringView;
	class String;
	class Parser;

	template<typename OTy = void> struct Hash {
		static_assert(std::is_integral<OTy>::value || std::is_enum<OTy>::value, "only supports integral types, specialize for other types");

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
	concept Range = requires(OTy& t) {
		typename OTy::value_type;
		requires !std::same_as<void, decltype(t.begin())>;
		requires !std::same_as<void, decltype(t.end())>;
	};

	template<typename OTy>
	concept ComplexT = JsonifierT<std::decay_t<OTy>>;

	template<typename OTy>
	concept TupleT = requires(OTy t) {
		std::tuple_size<OTy>::value;
		Tuplet::get<0>(t);
	} && !ComplexT<OTy> && !Range<OTy>;

	template<typename OTy>
	concept MapSubscriptable = requires(OTy container) {
		{ container[std::declval<typename OTy::key_type>()] } -> std::same_as<typename OTy::mapped_type&>;
	};

	template<typename OTy>
	concept PairT = requires(OTy Pair) {
		{ Pair.first } -> std::same_as<typename OTy::first_type&>;
		{ Pair.second } -> std::same_as<typename OTy::second_type&>;
	};

	template<typename OTy>
	concept CharT = std::same_as<std::decay_t<OTy>, char> || std::same_as<std::decay_t<OTy>, char16_t> || std::same_as<std::decay_t<OTy>, char32_t> ||
		std::same_as<std::decay_t<OTy>, char>;

	template<typename OTy>
	concept BoolT = std::same_as<std::decay_t<OTy>, bool> || std::same_as<std::decay_t<OTy>, std::vector<bool>::reference>;

	template<typename OTy>
	concept IntT = std::integral<std::decay_t<OTy>> && !CharT<std::decay_t<OTy>> && !BoolT<OTy>;

	template<typename OTy>
	concept NumT = std::floating_point<std::decay_t<OTy>> || IntT<OTy>;

	template<typename OTy>
	concept StringT = !ComplexT<OTy> && !std::same_as<std::nullptr_t, OTy> &&
		(std::same_as<OTy, std::string_view> || std::same_as<OTy, std::string> || std::same_as<OTy, String> || std::same_as<OTy, StringView>);

	template<typename OTy> using IteratorT = decltype(std::begin(std::declval<OTy&>()));

	template<Range R> using RangeValueT = std::iter_value_t<IteratorT<R>>;

	template<typename OTy>
	concept MapT = !ComplexT<OTy> && !StringT<OTy> && Range<OTy> && PairT<RangeValueT<OTy>> && MapSubscriptable<OTy>;

	template<typename OTy>
	concept ArrayT = (!ComplexT<OTy> && !StringT<OTy> && !MapT<OTy> && Range<OTy>);

	template<typename OTy> struct IsUniquePtr : std::false_type {};

	template<typename OTy, typename D> struct IsUniquePtr<std::unique_ptr<OTy, D>> : std::true_type {};

	template<typename OTy> inline constexpr bool IsUniquePtrV = IsUniquePtr<OTy>::value;

	template<typename OTy> struct Array {
		OTy value;
	};

	template<typename OTy> Array(OTy) -> Array<OTy>;

	template<typename OTy> struct Object {
		OTy value;
	};

	template<typename OTy> Object(OTy) -> Object<OTy>;

	inline constexpr auto size(const char* S) noexcept {
		int i = 0;
		while (*S != 0) {
			++i;
			++S;
		}
		return i;
	}

	template<typename OTy>
	concept Nullable = std::same_as<OTy, nullptr_t>;

	template<typename... Args> struct FalseT : std::false_type {};

	struct aggressive_unicorn_type;

	template<> struct FalseT<aggressive_unicorn_type> : std::true_type {};

	template<typename... Args> inline constexpr bool FalseV = FalseT<Args...>::value;

	template<size_t N> struct StringLiteral {
		static constexpr size_t size = (N > 0) ? (N - 1) : 0;

		constexpr StringLiteral() = default;

		constexpr StringLiteral(const char (&str)[N]) {
			std::copy_n(str, N, value);
		}

		char value[N];
		constexpr const char* end() const noexcept {
			return value + size;
		}

		constexpr const std::string_view sv() const noexcept {
			return { value, size };
		}
	};

	template<size_t N> constexpr auto stringLiteralFromView(std::string_view str) {
		StringLiteral<N + 1> sl{};
		std::copy_n(str.data(), str.size(), sl.value);
		*(sl.value + N) = '\0';
		return sl;
	}

	template<size_t N> constexpr size_t length(const char (&)[N]) noexcept {
		return N;
	}

	template<StringLiteral Str> struct CharsImpl {
		static constexpr std::string_view value{ Str.value, length(Str.value) - 1 };
	};

	template<StringLiteral Str> inline constexpr std::string_view Chars = CharsImpl<Str>::value;

	template<typename OTy>
	concept IsStdTuple = IsSpecializationV<OTy, std::tuple>::value || IsSpecializationV<OTy, std::pair>::value;

	template<typename OTy, size_t N> class RawVector {
		OTy data[N] = {};
		size_t dsize = 0;

	  public:
		using value_type = OTy;
		using reference = value_type&;
		using const_reference = const value_type&;
		using pointer = value_type*;
		using const_pointer = const value_type*;
		using iterator = pointer;
		using const_iterator = const_pointer;
		using size_type = size_t;
		using difference_type = std::ptrdiff_t;

		inline constexpr RawVector(void) = default;
		inline constexpr RawVector(size_type count, const auto& value) : dsize(count) {
			for (size_t i = 0; i < N; ++i)
				data[i] = value;
		}

		inline constexpr iterator begin() noexcept {
			return data;
		}

		inline constexpr iterator end() noexcept {
			return data + dsize;
		}

		inline constexpr size_type size() const {
			return dsize;
		}

		inline constexpr reference operator[](size_t index) {
			return data[index];
		}

		inline constexpr const_reference operator[](size_t index) const {
			return data[index];
		}

		inline constexpr reference back() {
			return data[dsize - 1];
		}

		inline constexpr const_reference back() const {
			return data[dsize - 1];
		}

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

	template<typename OTy, size_t N> class RawArray {
		OTy data_[N] = {};

		template<size_t M, size_t... I> inline constexpr RawArray(OTy const (&init)[M], std::index_sequence<I...>) : data_{ init[I]... } {
		}
		template<typename Iter, size_t... I> inline constexpr RawArray(Iter iter, std::index_sequence<I...>) : data_{ (( void )I, *iter++)... } {
		}

	  public:
		using value_type = OTy;
		using reference = value_type&;
		using const_reference = const value_type&;
		using pointer = value_type*;
		using const_pointer = const value_type*;
		using iterator = pointer;
		using const_iterator = const_pointer;
		using reverse_iterator = std::reverse_iterator<iterator>;
		using const_reverse_iterator = std::reverse_iterator<const_iterator>;
		using size_type = size_t;
		using difference_type = std::ptrdiff_t;

		inline constexpr RawArray(void) = default;
		template<size_t M> inline constexpr RawArray(OTy const (&init)[M]) : RawArray(init, std::make_index_sequence<N>()) {
			static_assert(M >= N);
		}
		template<size_t M> inline constexpr RawArray(std::array<OTy, M> const& init) : RawArray(&init[0], std::make_index_sequence<N>()) {
			static_assert(M >= N);
		}

		inline constexpr RawArray(std::initializer_list<OTy> init) : RawArray(init.begin(), std::make_index_sequence<N>()) {
		}

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

		inline constexpr size_type size() const {
			return N;
		}

		inline constexpr size_type maxSize() const {
			return N;
		}

		inline constexpr reference operator[](size_t index) {
			return data_[index];
		}

		inline constexpr const_reference operator[](size_t index) const {
			return data_[index];
		}

		inline constexpr reference at(size_t index) noexcept {
			if (index > N) {
				std::abort();
			}
			return data_[index];
		}

		inline constexpr const_reference at(size_t index) const noexcept {
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

		inline constexpr void fill(const value_type& val) {
			for (size_t i = 0; i < N; ++i)
				data_[i] = val;
		}
	};
	template<typename OTy> class RawArray<OTy, 0> {
	  public:
		using value_type = OTy;
		using reference = value_type&;
		using const_reference = const value_type&;
		using pointer = value_type*;
		using const_pointer = const value_type*;
		using iterator = pointer;
		using const_iterator = const_pointer;
		using reverse_iterator = std::reverse_iterator<iterator>;
		using const_reverse_iterator = std::reverse_iterator<const_iterator>;
		using size_type = size_t;
		using difference_type = std::ptrdiff_t;

		inline constexpr RawArray(void) = default;
	};

	template<typename = void, size_t... Is> constexpr auto indexer(std::index_sequence<Is...>) {
		return [](auto&& f) -> decltype(auto) {
			return decltype(f)(f)(std::integral_constant<size_t, Is>{}...);
		};
	}

	template<size_t N> constexpr auto indexer() {
		return indexer(std::make_index_sequence<N>{});
	}

	template<size_t N, typename Func> constexpr auto forEach(Func&& f) {
		return indexer<N>()([&](auto&&... i) {
			(std::forward<Func>(f)(i), ...);
		});
	}

	template<typename OTy>
	concept IsVariant = IsSpecializationV<OTy, std::variant>::value;

	namespace detail {
		template<IsVariant OTy> inline constexpr auto runtimeVariantMap() {
			constexpr auto N = std::variant_size_v<OTy>;
			std::array<OTy, N> ret{};
			forEach<N>([&](auto I) {
				ret[I] = std::variant_alternative_t<I, OTy>{};
			});
			return ret;
		}
	}
#ifdef _MSC_VER
	struct SView {
		const char* start{};
		size_t n{};
		constexpr SView(std::string_view sv) : start(sv.data()), n(sv.size()) {
		}
		constexpr auto data() const {
			return start;
		}
		constexpr auto begin() const {
			return data();
		}
		constexpr auto end() const {
			return data() + n;
		}
		constexpr auto size() const {
			return n;
		}
	};
	template<SView... Strs>
#else
	template<const std::string_view&... Strs>
#endif
	struct Join {
		static constexpr auto impl() noexcept {
			constexpr size_t len = (Strs.size() + ... + 0);
			std::array<char, len + 1> arr{};
			auto append = [i = 0, &arr](const auto& s) mutable {
				for (auto c: s)
					arr[i++] = c;
			};
			(append(Strs), ...);
			arr[len] = 0;
			return arr;
		}

		static constexpr auto arr = impl();
		static constexpr std::string_view value{ arr.data(), arr.size() - 1 };
	};

#ifdef _MSC_VER
	template<SView... Strs>
#else
	template<const std::string_view&... Strs>
#endif
	static constexpr auto JoinV = Join<Strs...>::value;

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

	template<typename OTy, typename mptr_t> using MemberT = decltype(getMember(std::declval<OTy>(), std::declval<std::decay_t<mptr_t>&>()));

	template<typename OTy>
	concept NullableT = !ComplexT<OTy> && !StringT<OTy> && requires(OTy t) {
		bool(t);
		{ *t };
	};


	template<size_t... Is> struct Sequence {};
	template<size_t N, size_t... Is> struct GenerateSequence : GenerateSequence<N - 1, N - 1, Is...> {};
	template<size_t... Is> struct GenerateSequence<0, Is...> : Sequence<Is...> {};

	template<size_t N1, size_t... I1, size_t N2, size_t... I2>
	constexpr std::array<char const, N1 + N2 - 1> concat(char const (&a1)[N1], char const (&a2)[N2], Sequence<I1...>, Sequence<I2...>) {
		return { { a1[I1]..., a2[I2]... } };
	}

	template<size_t N1, size_t N2> constexpr std::array<char const, N1 + N2 - 1> concatArrays(char const (&a1)[N1], char const (&a2)[N2]) {
		return concat(a1, a2, GenerateSequence<N1 - 1>{}, GenerateSequence<N2>{});
	}

	template<typename OTy, typename... U>
	concept IsAnyOf = (std::same_as<OTy, U> || ...);

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
	concept Resizeable = requires(OTy container) { container.resize(0); };

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
	concept IsTimeType =
		std::same_as<OTy, std::chrono::nanoseconds> || std::same_as<OTy, std::chrono::microseconds> || std::same_as<OTy, std::chrono::milliseconds> ||
		std::same_as<OTy, std::chrono::minutes> || std::same_as<OTy, std::chrono::hours> || std::same_as<OTy, std::chrono::days>;

	template<typename OTy>
	concept IsConvertibleToSerializer = std::convertible_to<OTy, Serializer>;

	template<typename OTy>
	concept IsEnum = std::is_enum<OTy>::value;

	template<typename OTy> inline auto dataPtr(OTy& buffer) {
		if constexpr (Resizeable<OTy>) {
			return buffer.data();
		} else {
			return buffer;
		}
	}

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

		inline void printTimePassed(std::string_view stringView) noexcept {
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