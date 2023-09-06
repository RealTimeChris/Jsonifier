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
/// Feb 20, 2023
#pragma once

#include <jsonifier/StringView.hpp>
#include <jsonifier/RawVector.hpp>
#include <jsonifier/Expected.hpp>
#include <jsonifier/RawArray.hpp>
#include <jsonifier/Tuple.hpp>
#include <jsonifier/Base.hpp>
#include <algorithm>
#include <stdexcept>
#include <vector>
#include <span>

namespace JsonifierInternal {

	template<typename ValueType> constexpr uint64_t toUint64(const ValueType* bytes, const uint64_t n) noexcept {
		if (std::is_constant_evaluated()) {
			uint64_t res{};
			for (uint64_t x = 0; x < n; ++x) {
				res |= static_cast<uint64_t>(bytes[x]) << (8 * x);
			}
			return res;
		} else {
			uint64_t res{};
			std::memcpy(&res, bytes, n);
			return res;
		}
	}

	template<uint64_t n, typename ValueType> constexpr uint64_t toUint64N(const ValueType* bytes) noexcept {
		static_assert(n <= 8);
		if (std::is_constant_evaluated()) {
			uint64_t res{};
			for (uint64_t x = 0; x < n; ++x) {
				res |= static_cast<uint64_t>(bytes[x]) << (8 * x);
			}
			return res;
		} else {
			uint64_t res{};
			std::memcpy(&res, bytes, n);
			return res;
		}
	}

	struct StringCompareHelper {
		template<typename T0, typename T1> constexpr bool operator()(T0&& lhs, T1&& rhs) const noexcept {
			if (std::is_constant_evaluated()) {
				return stringConstCompare(std::forward<T0>(lhs), std::forward<T1>(rhs));
			} else {
				return JsonifierCoreInternal::compare(lhs.data(), rhs.data(), rhs.size());
			}
		}
	};

	template<typename StringT> constexpr uint64_t fnv1aHash(const StringT& value) {
		uint64_t d = 5381;
		for (const auto& c: value)
			d = d * 33 + static_cast<uint64_t>(c);
		return d;
	}

	// https://en.wikipedia.org/wiki/Fowler%E2%80%93Noll%E2%80%93Vo_hash_function
	// With the lowest bits removed, based on experimental setup.
	template<typename StringT> constexpr uint64_t fnv1aHash(const StringT& value, uint64_t seed) {
		uint64_t d = (0x811c9dc5 ^ seed) * static_cast<uint64_t>(0x01000193);
		for (const auto& c: value)
			d = (d ^ static_cast<uint64_t>(c)) * static_cast<uint64_t>(0x01000193);
		return d >> 8;
	}

	template<typename ValueType> struct Hash<Jsonifier::StringViewBase<ValueType>> {
		constexpr uint64_t operator()(Jsonifier::StringViewBase<char> value) const noexcept {
			return fnv1aHash(value);
		}

		constexpr uint64_t operator()(Jsonifier::StringViewBase<char> value, uint64_t seed) const noexcept {
			return fnv1aHash(value, seed);
		}

		constexpr uint64_t operator()(Jsonifier::StringViewBase<uint8_t> value) const noexcept {
			return fnv1aHash(value);
		}

		constexpr uint64_t operator()(Jsonifier::StringViewBase<uint8_t> value, uint64_t seed) const noexcept {
			return fnv1aHash(value, seed);
		}
	};

	constexpr auto nextHighestPowerOfTwo(uint64_t v) {
		// https://graphics.stanford.edu/~seander/bithacks.html#RoundUpPowerOf2
		constexpr auto tripCount = std::numeric_limits<decltype(v)>::digits;
		v--;
		for (uint64_t x = 1; x < tripCount; x <<= 1)
			v |= v >> x;
		++v;
		return v;
	}

	template<typename ValueType> constexpr auto log(ValueType v) {
		uint64_t n = 0;
		while (v > 1) {
			n += 1;
			v >>= 1;
		}
		return n;
	}

	constexpr uint64_t bitWeight(uint64_t n) {
		return static_cast<uint64_t>(n <= 8ull) * sizeof(uint32_t) + (static_cast<uint64_t>(n <= 8ull) * sizeof(uint32_t)) +
			(static_cast<uint64_t>(n <= 8ull) * sizeof(unsigned long long)) + (n <= 128ull);
	}

	template<uint64_t n> inline unsigned long long selectUintLeast(std::integral_constant<uint64_t, n>) {
		static_assert(n < 2, "Unsupported type size.");
		return {};
	}

	template<uint64_t n> using SelectUintLeastT = decltype(selectUintLeast(std::integral_constant<uint64_t, bitWeight(n)>()));

	template<typename ValueType> constexpr void primitiveSwap(ValueType&& a, ValueType&& b) {
		auto tmp = std::move(a);
		a		 = std::move(b);
		b		 = std::move(tmp);
	}

	template<typename ValueType, typename U> constexpr void primitiveSwap(Pair<ValueType, U>&& a, Pair<ValueType, U>&& b) {
		primitiveSwap(std::forward<Pair<ValueType, U>>(a.first), std::forward<Pair<ValueType, U>>(b.first));
		primitiveSwap(std::forward<Pair<ValueType, U>>(a.second), std::forward<Pair<ValueType, U>>(b.second));
	}

	template<typename... Tys, uint64_t... Is> constexpr void primitiveSwap(Tuplet::Tuple<Tys...>&& a, Tuplet::Tuple<Tys...>&& b, std::index_sequence<Is...>) {
		using swallow = int32_t[];
		( void )swallow{ (primitiveSwap(std::forward<Tys...>(Tuplet::get<Is>(a)), std::forward<Tys...>(Tuplet::get<Is>(b))), 0)... };
	}

	template<typename... Tys> constexpr void primitiveSwap(Tuplet::Tuple<Tys...>& a, Tuplet::Tuple<Tys...>& b) {
		primitiveSwap(a, b, std::make_index_sequence<sizeof...(Tys)>());
	}

	template<typename Iterator, typename Compare> constexpr Iterator partition(Iterator left, Iterator right, const Compare& compare) {
		auto pivot = left + (right - left) / 2;
		auto value = *pivot;
		primitiveSwap(*right, *pivot);
		for (auto it = left; 0 < right - it; ++it) {
			if (compare(*it, value)) {
				primitiveSwap(*it, *left);
				++left;
			}
		}
		primitiveSwap(*right, *left);
		return left;
	}

	template<typename Iterator, typename Compare> constexpr void quicksort(Iterator left, Iterator right, const Compare& compare) {
		while (0 < right - left) {
			auto new_pivot = partition(left, right, compare);
			quicksort(left, new_pivot, compare);
			left = new_pivot + 1;
		}
	}

	template<typename ValueType, uint64_t n, typename Compare> constexpr RawArray<ValueType, n> quicksort(RawArray<ValueType, n> const& array, const Compare& compare) {
		RawArray<ValueType, n> res = array;
		quicksort(res.begin(), res.end() - 1, compare);
		return res;
	}

	template<typename InputIt1, typename InputIt2> constexpr bool equal(InputIt1 first1, InputIt1 last1, InputIt2 first2) {
		for (; first1 != last1; ++first1, ++first2) {
			if (!(*first1 == *first2)) {
				return false;
			}
		}
		return true;
	}

	template<typename UintType, UintType a, UintType c, UintType m> class LinearCongruentialEngine {
		static_assert(std::is_unsigned<UintType>::value, "UintType must be an unsigned integral type.");

		template<typename ValueType> static constexpr UintType modulo(ValueType val, std::integral_constant<UintType, 0>) {
			return static_cast<UintType>(val);
		}

		template<typename ValueType, UintType m2> static constexpr UintType modulo(ValueType val, std::integral_constant<UintType, m2>) {
			return static_cast<UintType>(val % m2);
		}

	  public:
		using ResultType						 = UintType;
		static constexpr ResultType multiplier	 = a;
		static constexpr ResultType increment	 = c;
		static constexpr ResultType modulus		 = m;
		static constexpr ResultType default_seed = 1u;

		inline LinearCongruentialEngine() noexcept = default;
		constexpr LinearCongruentialEngine(ResultType S) {
			seed(S);
		}

		inline void seed(ResultType S = default_seed) {
			state_ = S;
		}

		constexpr ResultType operator()() {
			using uint_least_t = SelectUintLeastT<log(a) + log(m) + 4>;
			uint_least_t tmp   = static_cast<uint_least_t>(multiplier) * state_ + increment;

			state_ = modulo(tmp, std::integral_constant<UintType, modulus>());
			return state_;
		}

		constexpr void discard(unsigned long long n) {
			while (n--)
				operator()();
		}

		constexpr bool operator==(LinearCongruentialEngine const& other) const {
			return state_ == other.state_;
		}

	  protected:
		ResultType state_ = default_seed;
	};

	using MinStdRand = LinearCongruentialEngine<std::uint_fast32_t, 48271, 0, 2147483647>;

	using DefaultPrgT = MinStdRand;

	// https://en.wikipedia.org/wiki/Fowler�Noll�Vo_hash_function
	// http://www.isthe.com/chongo/tech/comp/fnv/index.html#FNV-param
	static constexpr uint64_t fnv64Prime	   = 1099511628211;
	static constexpr uint64_t fnv64OffsetBasis = 0xcbf29ce484222325;

	template<typename HashType> struct Xsm1 {};

	template<> struct Xsm1<uint64_t> {
		template<typename ValueType> constexpr uint64_t operator()(ValueType&& value, const uint64_t seed) noexcept {
			uint64_t h	 = (fnv64OffsetBasis ^ seed) * fnv64Prime;
			const auto n = value.size();

			if (n < 8) {
				const auto shift = 64 - 8 * n;
				h ^= toUint64(value.data(), n) << shift;
				h ^= h >> 33;
				h *= fnv64Prime;
				return h;
			}
			using StringType							= RefUnwrap<ValueType>;
			const typename StringType::value_type* d0	= value.data();
			const typename StringType::value_type* end7 = value.data() + n - 7;
			for (; d0 < end7; d0 += 8) {
				h ^= toUint64N<8>(d0);
				h ^= h >> 33;
				h *= fnv64Prime;
			}

			const uint64_t nm8 = n - 8;
			h ^= toUint64N<8>(value.data() + nm8);
			h ^= h >> 33;
			h *= fnv64Prime;
			return h;
		}
	};

	template<> struct Xsm1<uint32_t> {
		constexpr uint32_t operator()(auto&& value, const uint32_t seed) noexcept {
			uint64_t hash = Xsm1<uint64_t>{}(value, seed);
			return hash >> 32;
		}
	};

	constexpr bool contains(auto&& data, auto&& val) noexcept {
		const auto n = data.size();
		for (uint64_t x = 0; x < n; ++x) {
			if (data[x] == val) {
				return true;
			}
		}
		return false;
	}

	template<uint64_t n> constexpr auto naiveBucketSize() noexcept {
		return n < 8 ? 2 * n : 4 * n;
	}

	template<uint64_t n> constexpr uint32_t naivePerfectHash(auto&& keys) noexcept {
		static_assert(n <= 20);
		constexpr uint64_t m = naiveBucketSize<n>();
		RawArray<uint64_t, n> hashes{};
		RawArray<uint64_t, n> buckets{};


		DefaultPrgT gen{};
		for (uint64_t x = 0; x < 1024; ++x) {
			uint32_t seed  = gen();
			uint64_t index = 0;
			for (const auto& key: keys) {
				const auto hash = Xsm1<uint32_t>{}(key, seed);
				if (contains(std::span{ hashes.data(), index }, hash))
					break;
				hashes[index] = hash;

				auto bucket = hash % m;
				if (contains(std::span{ buckets.data(), index }, bucket))
					break;
				buckets[index] = bucket;

				++index;
			}

			if (index == n)
				return seed;
		}

		return (std::numeric_limits<uint32_t>::max)();
	}

	template<typename Value, uint64_t n> struct NaiveMap {
		static_assert(n <= 20);
		static constexpr uint64_t m = naiveBucketSize<n>();
		uint32_t seed{};
		RawArray<Pair<Jsonifier::StringView, Value>, n> items{};
		RawArray<uint32_t, n * 1> hashes{};
		RawArray<uint8_t, m> table{};

		constexpr decltype(auto) begin() const noexcept {
			return items.begin();
		}

		constexpr decltype(auto) end() const noexcept {
			return items.end();
		}

		constexpr Expected<std::reference_wrapper<Value>, ErrorCode> at(auto&& key) const noexcept {
			const auto hash	 = Xsm1<uint32_t>{}(key, seed);
			const auto index = table[hash % m];
			const auto& item = items[index];
			if (hashes[index] != hash) [[unlikely]] {
				return Unexpected(ErrorCode::Unknown_Key);
			}
			return item.second;
		}

		constexpr decltype(auto) find(auto&& key) const noexcept {
			const auto hash	 = Xsm1<uint32_t>{}(key, seed);
			const auto index = table[hash % m];
			if (hashes[index] != hash) [[unlikely]]
				return items.end();
			return items.begin() + index;
		}
	};

	template<typename ValueType, uint64_t n> constexpr auto makeNaiveMap(std::initializer_list<Pair<Jsonifier::StringView, ValueType>> pairs) {
		static_assert(n <= 20);
		if (pairs.size() != n) {
			throw std::runtime_error{ "pairs.size() != n" };
		}
		NaiveMap<ValueType, n> ht{};
		constexpr uint64_t m = naiveBucketSize<n>();

		RawArray<Jsonifier::StringView, n> keys{};
		uint64_t x = 0;
		for (const auto& pair: pairs) {
			ht.items[x] = pair;
			keys[x]		= pair.first;
			++x;
		}
		ht.seed = naivePerfectHash<n>(keys);
		if (ht.seed == std::numeric_limits<uint32_t>::max()) {
			throw std::runtime_error{ "Unable to find perfect hash." };
		}

		for (x = 0; x < n; ++x) {
			const auto hash	   = Xsm1<uint32_t>{}(keys[x], ht.seed);
			ht.hashes[x]	   = hash;
			ht.table[hash % m] = x;
		}

		return ht;
	}

	struct SingleCharHashDesc {
		uint64_t n{};
		bool valid{};
		uint8_t min_diff{};
		uint8_t front{};
		uint8_t back{};
		bool is_front_hash = true;
	};

	template<uint64_t n, bool IsFrontHash = true> constexpr SingleCharHashDesc singleCharHash(const RawArray<Jsonifier::StringView, n>& v) noexcept {
		if constexpr (n > 255) {
			return {};
		}

		RawArray<uint8_t, n> hashes;
		for (uint64_t x = 0; x < n; ++x) {
			if (v[x].size() == 0) {
				return {};
			}
			if constexpr (IsFrontHash) {
				hashes[x] = v[x][0];
			} else {
				hashes[x] = v[x].back();
			}
		}

		std::sort(hashes.begin(), hashes.end());

		uint8_t min_diff = (std::numeric_limits<uint8_t>::max)();
		for (uint64_t x = 0; x < n - 1; ++x) {
			if ((hashes[x + 1] - hashes[x]) < min_diff) {
				min_diff = hashes[x + 1] - hashes[x];
			}
		}

		return SingleCharHashDesc{ n, min_diff > 0, min_diff, hashes.front(), hashes.back(), IsFrontHash };
	}

	template<typename ValueType, SingleCharHashDesc D> struct SingleCharMap {
		static constexpr auto n = D.n;
		static_assert(n < 256);
		RawArray<Pair<Jsonifier::StringView, ValueType>, n> items{};
		static constexpr uint64_t N_table = D.back - D.front + 1;
		RawArray<uint8_t, N_table> table{};

		constexpr decltype(auto) begin() const noexcept {
			return items.begin();
		}
		constexpr decltype(auto) end() const noexcept {
			return items.end();
		}

		constexpr Expected<std::reference_wrapper<ValueType>, ErrorCode> at(auto&& key) const noexcept {
			if (key.size() == 0) [[unlikely]] {
				return Unexpected(ErrorCode::Unknown_Key);
			}

			if constexpr (D.is_front_hash) {
				const auto k = static_cast<uint64_t>(key[0] - D.front);
				if (k >= N_table) [[unlikely]] {
					return Unexpected(ErrorCode::Unknown_Key);
				}
				const auto index = table[k];
				const auto& item = items[index];
				if (!stringConstCompare(item.first, key)) [[unlikely]] {
					return Unexpected(ErrorCode::Unknown_Key);
				}
				return item.second;
			} else {
				const auto k = static_cast<uint64_t>(key.back() - D.front);
				if (k >= N_table) [[unlikely]] {
					return Unexpected(ErrorCode::Unknown_Key);
				}
				const auto index = table[k];
				const auto& item = items[index];
				if (!stringConstCompare(item.first, key)) [[unlikely]] {
					return Unexpected(ErrorCode::Unknown_Key);
				}
				return item.second;
			}
		}

		constexpr decltype(auto) find(auto&& key) const noexcept {
			if (key.size() == 0) [[unlikely]] {
				return items.end();
			}

			if constexpr (D.is_front_hash) {
				const auto k = static_cast<uint64_t>(key[0] - D.front);
				if (k >= N_table) [[unlikely]] {
					return items.end();
				}
				const auto index = table[k];
				const auto& item = items[index];
				if (!stringConstCompare(item.first, key)) [[unlikely]]
					return items.end();
				return items.begin() + index;
			} else {
				const auto k = static_cast<uint64_t>(key.back() - D.front);
				if (k >= N_table) [[unlikely]] {
					return items.end();
				}
				const auto index = table[k];
				const auto& item = items[index];
				if (!stringConstCompare(item.first, key)) [[unlikely]]
					return items.end();
				return items.begin() + index;
			}
		}
	};

	template<typename ValueType, SingleCharHashDesc D> constexpr auto makeSingleCharMap(std::initializer_list<Pair<Jsonifier::StringView, ValueType>> pairs) {
		static_assert(D.n < 256);
		SingleCharMap<ValueType, D> ht{};

		uint8_t x = 0;
		for (const auto& pair: pairs) {
			ht.items[x] = pair;
			if constexpr (D.is_front_hash) {
				ht.table[pair.first[0] - D.front] = x;
			} else {
				ht.table[pair.first.back() - D.front] = x;
			}
			++x;
		}

		return ht;
	}

	template<typename ValueType, const Jsonifier::StringView& S> struct SingleItem {
		RawArray<Pair<Jsonifier::StringView, ValueType>, 1> items{};

		constexpr decltype(auto) end() const noexcept {
			return items.end();
		}

		constexpr decltype(auto) find(auto&& key) const noexcept {
			if (stringConstCompare(S, key)) [[likely]] {
				return items.begin();
			} else [[unlikely]] {
				return items.end();
			}
		}
	};

	template<typename ValueType, const Jsonifier::StringView& string01, const Jsonifier::StringView& string02> struct DoubleItem {
		RawArray<Pair<Jsonifier::StringView, ValueType>, 2> items{};

		static constexpr auto s0 = string01;
		static constexpr auto s1 = string02;

		constexpr decltype(auto) end() const noexcept {
			return items.end();
		}

		static constexpr bool sameSize = s0.size() == s1.size();

		constexpr decltype(auto) find(auto&& key) const noexcept {
			if constexpr (sameSize) {
				constexpr auto n = s0.size();
				if (key.size() != n) {
					return items.end();
				}
			}
			if (stringConstCompare(string01, key)) {
				return items.begin();
			} else if (stringConstCompare(string02, key)) {
				return items.begin() + 1;
			} else [[unlikely]] {
				return items.end();
			}
		}
	};

	// from
	// https://stackoverflow.com/questions/55941964/how-to-filter-duplicate-types-from-Tuple-c
	template<typename ValueType, typename... ValueTypes> struct Unique {
		using type = ValueType;
	};

	template<template<typename...> class ValueType, typename... ValueTypes, typename U, typename... Us> struct Unique<ValueType<ValueTypes...>, U, Us...>
		: std::conditional_t<(std::same_as<U, ValueTypes> || ...), Unique<ValueType<ValueTypes...>, Us...>, Unique<ValueType<ValueTypes..., U>, Us...>> {};

	template<typename ValueType> struct TupleVariant;

	template<typename... ValueTypes> struct TupleVariant<Tuplet::Tuple<ValueTypes...>> : Unique<std::variant<>, ValueTypes...> {};

	template<typename ValueType> struct TuplePtrVariant;

	template<typename... ValueTypes> struct TuplePtrVariant<std::tuple<ValueTypes...>> : Unique<std::variant<>, std::add_pointer_t<ValueTypes>...> {};

	template<typename... ValueTypes> struct TuplePtrVariant<Tuplet::Tuple<ValueTypes...>> : Unique<std::variant<>, std::add_pointer_t<ValueTypes>...> {};

	template<typename... ValueTypes> struct TuplePtrVariant<Pair<ValueTypes...>> : Unique<std::variant<>, std::add_pointer_t<ValueTypes>...> {};

	template<typename Tuple, typename = std::make_index_sequence<std::tuple_size<Tuple>::value>> struct ValueTupleVariant;

	template<typename Tuple, size_t... I> struct ValueTupleVariant<Tuple, std::index_sequence<I...>> {
		using type = typename TupleVariant<decltype(Tuplet::tupleCat(std::declval<Tuplet::Tuple<std::tuple_element_t<1, std::tuple_element_t<I, Tuple>>>>()...))>::type;
	};

	template<typename Tuple> using ValueTupleVariantT = typename ValueTupleVariant<Tuple>::type;

	struct BucketSizeCompare {
		template<typename B> bool constexpr operator()(B const& b0, B const& b1) const noexcept {
			return b0.size() > b1.size();
		}
	};

	template<uint64_t m> struct PmhBuckets {
		static constexpr auto bucket_max = 2 * (1u << (log(m) / 2));

		using bucket_t = RawVector<uint64_t, bucket_max>;
		RawArray<bucket_t, m> buckets;
		uint64_t seed;

		struct BucketRef {
			unsigned hash;
			const bucket_t* values;
			using value_type	 = typename bucket_t::value_type;
			using const_iterator = typename bucket_t::const_iterator;

			constexpr auto size() const noexcept {
				return values->size();
			}

			constexpr const auto& operator[](uint64_t idx) const noexcept {
				return (*values)[idx];
			}

			constexpr auto begin() const noexcept {
				return values->begin();
			}

			constexpr auto end() const noexcept {
				return values->end();
			}
		};

		template<size_t... Is> RawArray<BucketRef, m> constexpr makeBucketRefs(std::index_sequence<Is...>) const noexcept {
			return { { BucketRef{ Is, &buckets[Is] }... } };
		}

		RawArray<BucketRef, m> constexpr getSortedBuckets() const noexcept {
			RawArray<BucketRef, m> result{ makeBucketRefs(std::make_index_sequence<m>()) };
			quicksort(result.begin(), result.end() - 1, BucketSizeCompare{});
			return result;
		}
	};

	template<size_t m, typename Item, size_t n, typename Key>
	PmhBuckets<m> constexpr makePmhBuckets(const RawArray<Item, n>& items, Key const& key, DefaultPrgT& prg) {
		using result_t = PmhBuckets<m>;
		using hasher = Hash<Jsonifier::StringView>;
		result_t result{};
		bool rejected = false;
		while (true) {
			for (auto& b: result.buckets) {
				b.clear();
			}
			result.seed = prg();
			rejected	= false;
			for (uint64_t x = 0; x < n; ++x) {
				auto& bucket = result.buckets[hasher{}(key(items[x]), static_cast<uint64_t>(result.seed)) % m];
				if (bucket.size() >= result_t::bucket_max) [[unlikely]] {
					rejected = true;
					break;
				}
				bucket.push_back(x);
			}
			if (!rejected) [[likely]] {
				return result;
			}
		}
	}

	template<typename ValueType, uint64_t n> constexpr bool allDifferentFrom(RawVector<ValueType, n>& data, ValueType& a) {
		for (uint64_t x = 0; x < data.size(); ++x)
			if (data[x] == a) [[unlikely]] {
				return false;
			}
		return true;
	}

	struct SeedOrIndex {
		using value_type = uint64_t;

	  protected:
		static constexpr value_type MinusOne = (std::numeric_limits<value_type>::max)();
		static constexpr value_type HighBit	 = ~(MinusOne >> 1);

		value_type val = 0;

	  public:
		constexpr value_type value() const noexcept {
			return val;
		}

		constexpr bool isSeed() const noexcept {
			return val & HighBit;
		}

		constexpr SeedOrIndex(bool isSeed, value_type value) : val(isSeed ? (value | HighBit) : (value & ~HighBit)) {
		}

		constexpr SeedOrIndex() noexcept								 = default;
		constexpr SeedOrIndex(const SeedOrIndex&) noexcept			 = default;
		constexpr SeedOrIndex& operator=(const SeedOrIndex&) noexcept = default;
	};

	template<uint64_t m> struct PmhTables : protected Hash<Jsonifier::StringViewBase<uint8_t>> {
		constexpr PmhTables() noexcept = default;
		constexpr PmhTables(uint64_t firstSeedNew, RawArray<SeedOrIndex, m> firstTableNew, RawArray<uint64_t, m> secondTableNew) {
			firstSeed = firstSeedNew;
			firstTable	= firstTableNew;
			secondTable = secondTableNew;
		}
		uint64_t firstSeed;
		RawArray<SeedOrIndex, m> firstTable;
		RawArray<uint64_t, m> secondTable;
		using hasher = Hash<Jsonifier::StringViewBase<uint8_t>>;

		constexpr const hasher& getHasher() const {
			return *this;
		}

		template<typename KeyType> constexpr uint64_t lookup(const KeyType& key) const noexcept {
			auto const d = firstTable[getHasher()(key, static_cast<uint64_t>(firstSeed)) % m];
			if (!d.isSeed()) [[unlikely]] {
				return static_cast<uint64_t>(d.value());
			} else [[likely]] {
				return secondTable[getHasher()(key, static_cast<uint64_t>(d.value())) % m];
			}
		}
	};

	template<size_t m, typename Item, size_t n, typename Key>
	PmhTables<m> constexpr makePmhTables(const RawArray<Item, n>& items, Key const& key, DefaultPrgT prg) {
		auto step_one = makePmhBuckets<m>(items, key, prg);
		auto buckets  = step_one.getSortedBuckets();

		RawArray<SeedOrIndex, m> G;
		using hasher = Hash<Jsonifier::StringViewBase<uint8_t>>;

		constexpr uint64_t UNUSED = (std::numeric_limits<uint64_t>::max)();
		RawArray<uint64_t, m> H;
		H.fill(UNUSED);

		for (const auto& bucket: buckets) {
			auto const bsize = bucket.size();

			if (bsize == 1) {
				G[bucket.hash] = { false, static_cast<uint64_t>(bucket[0]) };
			} else if (bsize > 1) {
				SeedOrIndex d{ true, prg() };
				RawVector<uint64_t, decltype(step_one)::bucket_max> bucket_slots;

				while (bucket_slots.size() < bsize) {
					auto slot = hasher{}(key(items[bucket[bucket_slots.size()]]), d.value()) % m;

					if (H[slot] != UNUSED || !allDifferentFrom(bucket_slots, slot)) {
						bucket_slots.clear();
						d = { true, prg() };
						continue;
					}

					bucket_slots.push_back(slot);
				}

				G[bucket.hash] = d;
				for (uint64_t x = 0; x < bsize; ++x)
					H[bucket_slots[x]] = bucket[x];
			}
		}

		for (uint64_t x = 0; x < m; ++x)
			if (H[x] == UNUSED)
				H[x] = 0;

		return { step_one.seed, G, H };
	}

	struct GetKey {
		template<typename KV> constexpr auto const& operator()(KV const& kv) const noexcept {
			return kv.first;
		}
	};

	template<typename Key, typename Value, size_t n> class UnorderedMap : protected StringCompareHelper,
																		  protected RawArray<Pair<Key, Value>, n>,
																		  protected PmhTables<nextHighestPowerOfTwo(n) * (n < 32 ? 2 : 1)> {
	  public:
		static constexpr auto storageSize = nextHighestPowerOfTwo(n) * (n < 32 ? 2 : 1);
		using container_type			   = RawArray<Pair<Key, Value>, n>;
		using tables_type				   = PmhTables<storageSize>;

		using Self			  = UnorderedMap<Key, Value, n>;
		using key_type		  = Key;
		using mapped_type	  = Value;
		using value_type	  = typename container_type::value_type;
		using size_type		  = typename container_type::size_type;
		using difference_type = typename container_type::difference_type;
		using hasher		  = Hash<key_type>;
		using key_equal		  = StringCompareHelper;
		using reference		  = typename container_type::reference;
		using const_reference = typename container_type::const_reference;
		using pointer		  = typename container_type::pointer;
		using const_pointer	  = typename container_type::const_pointer;
		using iterator		  = typename container_type::iterator;
		using const_iterator  = typename container_type::const_iterator;

		constexpr UnorderedMap() noexcept							   = default;
		constexpr UnorderedMap& operator=(UnorderedMap&&) noexcept	   = default;
		constexpr UnorderedMap(UnorderedMap&&) noexcept				   = default;
		constexpr UnorderedMap& operator=(const UnorderedMap&) noexcept = default;
		constexpr UnorderedMap(const UnorderedMap&) noexcept			   = default;

		constexpr UnorderedMap(container_type items) : container_type{ items }, tables_type{ makePmhTables<storageSize>(items, GetKey{}, DefaultPrgT{}) } {}

		constexpr UnorderedMap(std::initializer_list<value_type> items) : UnorderedMap{ container_type{ items } } {
			constexpr_assert(items.size() == n, "Inconsistent initializer_list size and type size argument.");
		}

		constexpr iterator begin() {
			return container_type::begin();
		}

		constexpr iterator end() {
			return container_type::end();
		}

		constexpr const_iterator begin() const noexcept {
			return container_type::begin();
		}

		constexpr const_iterator end() const noexcept {
			return container_type::end();
		}

		constexpr size_type size() const noexcept {
			return n;
		}

		template<typename key_type_new> constexpr const_iterator find(const key_type_new& key) const noexcept {
			auto& kv = lookup(key);
			if (keyEq()(kv.first, key)) {
				return &kv;
			} else {
				return end();
			}
		}

		template<typename key_type_new> constexpr iterator find(const key_type_new& key) {
			auto& kv = lookup(key);
			if (keyEq()(kv.first, key)) {
				return &kv;
			} else {
				return end();
			}
		}

	  protected:
		template<typename key_type_new> constexpr const auto& lookup(const key_type_new& key) const noexcept {
			return container_type::operator[](tables_type::lookup(key));
		}

		template<typename key_type_new> constexpr auto& lookup(const key_type_new& key) {
			return container_type::operator[](tables_type::lookup(key));
		}

		constexpr const key_equal& keyEq() const noexcept {
			return *this;
		}
	};

	template<typename ValueType, typename U, uint64_t n>
	constexpr auto makeUnorderedMap(Pair<ValueType, U> const (&items)[n]) {
		return UnorderedMap<ValueType, U, n>{ items };
	}

	template<typename ValueType, uint64_t I> struct CoreSV {
		static constexpr Jsonifier::StringView value = Tuplet::get<0>(Tuplet::get<I>(CoreV<ValueType>));
	};

	template<typename ValueType, size_t... I> constexpr auto makeMapImpl(std::index_sequence<I...>) {
		using value_t		= ValueTupleVariantT<CoreT<ValueType>>;
		constexpr auto n	= std::tuple_size_v<CoreT<ValueType>>;
		constexpr auto size = sizeof...(I);
		static_assert(size == n);
		auto naiveOrNormalHash = [&] {
			if constexpr (size <= 20) {
				return makeNaiveMap<value_t, n>({ Pair<Jsonifier::StringView, value_t>(Jsonifier::StringView(Tuplet::get<0>(Tuplet::get<I>(CoreV<ValueType>))),
					Tuplet::get<1>(Tuplet::get<I>(CoreV<ValueType>)))... });
			} else {
				return makeUnorderedMap<Jsonifier::StringView, value_t, n>({ Pair<Jsonifier::StringView, value_t>(
					Jsonifier::StringView(Tuplet::get<0>(Tuplet::get<I>(CoreV<ValueType>))), Tuplet::get<1>(Tuplet::get<I>(CoreV<ValueType>)))... });
			}
		};

		constexpr bool n128 = n < 128;
		if constexpr (n == 0) {
			static_assert(FalseV<ValueType>, "Empty object in Jsonifier::Core.");
		} else if constexpr (n == 1) {
			return SingleItem<value_t, CoreSV<ValueType, I>::value...>{ RawArray<Pair<Jsonifier::StringView, value_t>, n>{ Pair<Jsonifier::StringView, value_t>(
				Jsonifier::StringView(Tuplet::get<0>(Tuplet::get<I>(CoreV<ValueType>))), Tuplet::get<1>(Tuplet::get<I>(CoreV<ValueType>)))... } };
		} else if constexpr (n == 2) {
			return DoubleItem<value_t, CoreSV<ValueType, I>::value...>{ RawArray<Pair<Jsonifier::StringView, value_t>, n>{ Pair<Jsonifier::StringView, value_t>(
				Jsonifier::StringView(Tuplet::get<0>(Tuplet::get<I>(CoreV<ValueType>))), Tuplet::get<1>(Tuplet::get<I>(CoreV<ValueType>)))... } };
		} else if constexpr (n128) {
			constexpr auto frontDesc = singleCharHash<n>(RawArray<Jsonifier::StringView, n>{ Jsonifier::StringView{ Tuplet::get<0>(Tuplet::get<I>(CoreV<ValueType>)) }... });

			if constexpr (frontDesc.valid) {
				return makeSingleCharMap<value_t, frontDesc>({ Pair<Jsonifier::StringView, value_t>(Jsonifier::StringView(Tuplet::get<0>(Tuplet::get<I>(CoreV<ValueType>))),
					Tuplet::get<1>(Tuplet::get<I>(CoreV<ValueType>)))... });
			} else {
				constexpr auto backDesc =
					singleCharHash<n, false>(RawArray<Jsonifier::StringView, n>{ Jsonifier::StringView{ Tuplet::get<0>(Tuplet::get<I>(CoreV<ValueType>)) }... });

				if constexpr (backDesc.valid) {
					return makeSingleCharMap<value_t, backDesc>({ Pair<Jsonifier::StringView, value_t>(Jsonifier::StringView(Tuplet::get<0>(Tuplet::get<I>(CoreV<ValueType>))),
						Tuplet::get<1>(Tuplet::get<I>(CoreV<ValueType>)))... });
				} else {
					return naiveOrNormalHash();
				}
			}
		} else {
			return naiveOrNormalHash();
		}
	}

	template<typename ValueType> constexpr auto makeMap() {
		constexpr auto indices = std::make_index_sequence<std::tuple_size_v<CoreT<ValueType>>>{};
		return makeMapImpl<RefUnwrap<ValueType>>(indices);
	}
}