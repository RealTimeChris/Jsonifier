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

namespace Jsonifier {

	inline constexpr uint64_t toUint64(const char* bytes, const size_t n) noexcept {
		if (std::is_constant_evaluated()) {
			uint64_t res{};
			for (size_t x = 0; x < n; ++x) {
				res |= static_cast<uint64_t>(bytes[x]) << (8 * x);
			}
			return res;
		} else if (n == 8) {
			return *reinterpret_cast<const uint64_t*>(bytes);
		} else {
			uint64_t res;
			std::memcpy(&res, bytes, n);
			return res;
		}
	}

	template<size_t N> inline constexpr uint64_t toUint64N(const char* bytes) noexcept {
		static_assert(N <= 8);
		if (std::is_constant_evaluated()) {
			uint64_t res{};
			for (size_t x = 0; x < N; ++x) {
				res |= static_cast<uint64_t>(bytes[x]) << (8 * x);
			}
			return res;
		} else if constexpr (N == 8) {
			return *reinterpret_cast<const uint64_t*>(bytes);
		} else {
			uint64_t res;
			std::memcpy(&res, bytes, N);
			return res;
		}
	}

	struct StringCompareHelper final {
		template<typename T0, typename T1> inline constexpr bool operator()(T0&& lhs, T1&& rhs) const noexcept {
			return stringConstCompare(std::forward<T0>(lhs), std::forward<T1>(rhs));
		}
	};

	template<typename StringT> inline constexpr size_t fnv1aHash(const StringT& value) {
		size_t d = 5381;
		for (const auto& c: value)
			d = d * 33 + static_cast<size_t>(c);
		return d;
	}

	// https://en.wikipedia.org/wiki/Fowler%E2%80%93Noll%E2%80%93Vo_hash_function
	// With the lowest bits removed, based on experimental setup.
	template<typename StringT> inline constexpr size_t fnv1aHash(const StringT& value, size_t seed) {
		size_t d = (0x811c9dc5 ^ seed) * static_cast<size_t>(0x01000193);
		for (const auto& c: value)
			d = (d ^ static_cast<size_t>(c)) * static_cast<size_t>(0x01000193);
		return d >> 8;
	}

	template<> struct Hash<StringView> {
		inline constexpr size_t operator()(StringView value) const noexcept {
			return fnv1aHash(value);
		}

		inline constexpr size_t operator()(StringView value, size_t seed) const noexcept {
			return fnv1aHash(value, seed);
		}
	};

	inline constexpr auto nextHighestPowerOfTwo(size_t v) {
		// https://graphics.stanford.edu/~seander/bithacks.html#RoundUpPowerOf2
		constexpr auto trip_count = std::numeric_limits<decltype(v)>::digits;
		v--;
		for (size_t x = 1; x < trip_count; x <<= 1)
			v |= v >> x;
		++v;
		return v;
	}

	template<typename OTy> inline constexpr auto log(OTy v) {
		size_t n = 0;
		while (v > 1) {
			n += 1;
			v >>= 1;
		}
		return n;
	}

	inline constexpr size_t bitWeight(size_t n) {
		return static_cast<size_t>(n <= 8ull) * sizeof(uint32_t) + (static_cast<size_t>(n <= 8ull) * sizeof(uint32_t)) +
			(static_cast<size_t>(n <= 8ull) * sizeof(unsigned long long)) + (n <= 128ull);
	}

	template<size_t N> inline unsigned long long selectUintLeast(std::integral_constant<size_t, N>) {
		static_assert(N < 2, "Unsupported type size.");
		return {};
	}

	template<size_t N> using SelectUintLeastT = decltype(selectUintLeast(std::integral_constant<size_t, bitWeight(N)>()));

	template<typename OTy> inline constexpr void primitiveSwap(OTy& a, OTy& b) {
		auto tmp = a;
		a = b;
		b = tmp;
	}

	template<typename OTy, typename U> inline constexpr void primitiveSwap(std::pair<OTy, U>& a, std::pair<OTy, U>& b) {
		primitiveSwap(a.first, b.first);
		primitiveSwap(a.second, b.second);
	}

	template<typename... Tys, size_t... Is>
	inline constexpr void primitiveSwap(std::tuple<Tys...>& a, std::tuple<Tys...>& b, std::index_sequence<Is...>) {
		using swallow = int[];
		( void )swallow{ (primitiveSwap(std::get<Is>(a), std::get<Is>(b)), 0)... };
	}

	template<typename... Tys> inline constexpr void primitiveSwap(std::tuple<Tys...>& a, std::tuple<Tys...>& b) {
		primitiveSwap(a, b, std::make_index_sequence<sizeof...(Tys)>());
	}

	template<typename Iterator, typename Compare> inline constexpr Iterator partition(Iterator left, Iterator right, const Compare& compare) {
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

	template<typename Iterator, typename Compare> inline constexpr void quicksort(Iterator left, Iterator right, const Compare& compare) {
		while (0 < right - left) {
			auto new_pivot = partition(left, right, compare);
			quicksort(left, new_pivot, compare);
			left = new_pivot + 1;
		}
	}

	template<typename OTy, size_t N, typename Compare>
	inline constexpr RawArray<OTy, N> quicksort(RawArray<OTy, N> const& array, const Compare& compare) {
		RawArray<OTy, N> res = array;
		quicksort(res.begin(), res.end() - 1, compare);
		return res;
	}

	template<typename OTy, typename Compare> struct LowerBound {
		OTy const& value;
		const Compare& compareisonVal;
		inline constexpr LowerBound(OTy const& value, const Compare& compare) : value(value), compareisonVal(compare) {
		}

		template<typename ForwardIt> inline constexpr ForwardIt doItFast(ForwardIt first, std::integral_constant<size_t, 0>) {
			return first;
		}

		template<typename ForwardIt, size_t N> inline constexpr ForwardIt doItFast(ForwardIt first, std::integral_constant<size_t, N>) {
			constexpr auto step = N / 2;
			static_assert(N / 2 == N - N / 2 - 1, "Power of two minus 1.");
			auto it = first + step;
			auto next_it = compareisonVal(*it, value) ? it + 1 : first;
			return doItFast(next_it, std::integral_constant<size_t, N / 2>{});
		}

		template<typename ForwardIt, size_t N>
		inline constexpr ForwardIt doitfirst(ForwardIt first, std::integral_constant<size_t, N>, std::integral_constant<bool, true>) {
			return doItFast(first, std::integral_constant<size_t, N>{});
		}

		template<typename ForwardIt, size_t N>
		inline constexpr ForwardIt doitfirst(ForwardIt first, std::integral_constant<size_t, N>, std::integral_constant<bool, false>) {
			constexpr auto next_power = nextHighestPowerOfTwo(N);
			constexpr auto next_start = next_power / 2 - 1;
			auto it = first + next_start;
			if (compareisonVal(*it, value)) {
				constexpr auto next = N - next_start - 1;
				return doitfirst(it + 1, std::integral_constant<size_t, next>{},
					std::integral_constant<bool, nextHighestPowerOfTwo(next) - 1 == next>{});
			} else
				return doItFast(first, std::integral_constant<size_t, next_start>{});
		}

		template<typename ForwardIt>
		inline constexpr ForwardIt doitfirst(ForwardIt first, std::integral_constant<size_t, 1>, std::integral_constant<bool, false>) {
			return doItFast(first, std::integral_constant<size_t, 1>{});
		}
	};

	template<typename InputIt1, typename InputIt2> inline constexpr bool equal(InputIt1 first1, InputIt1 last1, InputIt2 first2) {
		for (; first1 != last1; ++first1, ++first2) {
			if (!(*first1 == *first2)) {
				return false;
			}
		}
		return true;
	}

	template<typename UintType, UintType a, UintType c, UintType m> class LinearCongruentialEngine {
		static_assert(std::is_unsigned<UintType>::value, "UintType must be an unsigned integral type.");

		template<typename OTy> inline static constexpr UintType modulo(OTy val, std::integral_constant<UintType, 0>) {
			return static_cast<UintType>(val);
		}

		template<typename OTy, UintType M> inline static constexpr UintType modulo(OTy val, std::integral_constant<UintType, M>) {
			return static_cast<UintType>(val % M);
		}

	  public:
		using ResultType = UintType;
		static constexpr ResultType multiplier = a;
		static constexpr ResultType increment = c;
		static constexpr ResultType modulus = m;
		static constexpr ResultType default_seed = 1u;

		inline LinearCongruentialEngine() noexcept = default;
		inline constexpr LinearCongruentialEngine(ResultType S) {
			seed(S);
		}

		inline void seed(ResultType S = default_seed) {
			state_ = S;
		}

		inline constexpr ResultType operator()() {
			using uint_least_t = SelectUintLeastT<log(a) + log(m) + 4>;
			uint_least_t tmp = static_cast<uint_least_t>(multiplier) * state_ + increment;

			state_ = modulo(tmp, std::integral_constant<UintType, modulus>());
			return state_;
		}

		inline constexpr void discard(unsigned long long n) {
			while (n--)
				operator()();
		}

		inline constexpr friend bool operator==(LinearCongruentialEngine const& self, LinearCongruentialEngine const& other) {
			return self.state_ == other.state_;
		}

		inline constexpr friend bool operator!=(LinearCongruentialEngine const& self, LinearCongruentialEngine const& other) {
			return !(self == other);
		}

	  protected:
		ResultType state_ = default_seed;
	};

	using MinStdRand = LinearCongruentialEngine<std::uint_fast32_t, 48271, 0, 2147483647>;

	using DefaultPrgT = MinStdRand;

	// https://en.wikipedia.org/wiki/Fowler�Noll�Vo_hash_function
	// http://www.isthe.com/chongo/tech/comp/fnv/index.html#FNV-param
	inline static constexpr uint64_t fnv64_prime = 1099511628211;
	inline static constexpr uint64_t fnv64_offset_basis = 0xcbf29ce484222325;

	template<typename HashType> struct Xsm1 {};

	template<> struct Xsm1<uint64_t> {
		inline constexpr uint64_t operator()(auto&& value, const uint64_t seed) noexcept {
			uint64_t h = (fnv64_offset_basis ^ seed) * fnv64_prime;
			const auto n = value.size();

			if (n < 8) {
				const auto shift = 64 - 8 * n;
				h ^= toUint64(value.data(), n) << shift;
				h ^= h >> 33;
				h *= fnv64_prime;
				return h;
			}

			const char* d0 = value.data();
			const char* end7 = value.data() + n - 7;
			for (; d0 < end7; d0 += 8) {
				h ^= toUint64N<8>(d0);
				h ^= h >> 33;
				h *= fnv64_prime;
			}

			const uint64_t nm8 = n - 8;
			h ^= toUint64N<8>(value.data() + nm8);
			h ^= h >> 33;
			h *= fnv64_prime;
			return h;
		}
	};

	template<> struct Xsm1<uint32_t> {
		inline constexpr uint32_t operator()(auto&& value, const uint32_t seed) noexcept {
			uint64_t hash = Xsm1<uint64_t>{}(value, seed);
			return hash >> 32;
		}
	};

	inline constexpr bool contains(auto&& data, auto&& val) noexcept {
		const auto n = data.size();
		for (size_t x = 0; x < n; ++x) {
			if (data[x] == val) {
				return true;
			}
		}
		return false;
	}

	template<size_t N> inline constexpr auto naiveBucketSize() noexcept {
		return N < 8 ? 2 * N : 4 * N;
	}

	template<size_t N> inline constexpr uint32_t naivePerfectHash(auto&& keys) noexcept {
		static_assert(N <= 20);
		constexpr size_t m = naiveBucketSize<N>();
		std::array<size_t, N> hashes{};
		std::array<size_t, N> buckets{};

		auto hash_alg = Xsm1<uint32_t>{};

		DefaultPrgT gen{};
		for (size_t x = 0; x < 1024; ++x) {
			uint32_t seed = gen();
			size_t index = 0;
			for (const auto& key: keys) {
				const auto hash = hash_alg(key, seed);
				if (contains(std::span{ hashes.data(), index }, hash))
					break;
				hashes[index] = hash;

				auto bucket = hash % m;
				if (contains(std::span{ buckets.data(), index }, bucket))
					break;
				buckets[index] = bucket;

				++index;
			}

			if (index == N)
				return seed;
		}

		return (std::numeric_limits<uint32_t>::max)();
	}

	template<typename Value, size_t N> struct NaiveMap {
		static_assert(N <= 20);
		static constexpr size_t m = naiveBucketSize<N>();
		uint32_t seed{};
		std::array<std::pair<StringView, Value>, N> items{};
		std::array<uint32_t, N * 1> hashes{};
		std::array<uint8_t, m> table{};

		inline constexpr decltype(auto) begin() const noexcept {
			return items.begin();
		}

		inline constexpr decltype(auto) end() const noexcept {
			return items.end();
		}

		inline constexpr Expected<std::reference_wrapper<Value>, ErrorCode> at(auto&& key) const noexcept {
			const auto hash = Xsm1<uint32_t>{}(key, seed);
			const auto index = table[hash % m];
			const auto& item = items[index];
			if (hashes[index] != hash) [[unlikely]] {
				return Unexpected(ErrorCode::Unknown_Key);
			}
			return item.second;
		}

		inline constexpr decltype(auto) find(auto&& key) const noexcept {
			const auto hash = Xsm1<uint32_t>{}(key, seed);
			const auto index = table[hash % m];
			if (hashes[index] != hash) [[unlikely]]
				return items.end();
			return items.begin() + index;
		}
	};

	template<typename OTy, size_t N> inline constexpr auto makeNaiveMap(std::initializer_list<std::pair<StringView, OTy>> pairs) {
		static_assert(N <= 20);
		if (pairs.size() != N) {
			throw std::runtime_error{ "pairs.size() != N" };
		}
		NaiveMap<OTy, N> ht{};
		constexpr size_t m = naiveBucketSize<N>();

		std::array<StringView, N> keys{};
		size_t x = 0;
		for (const auto& pair: pairs) {
			ht.items[x] = pair;
			keys[x] = pair.first;
			++x;
		}
		ht.seed = naivePerfectHash<N>(keys);
		if (ht.seed == std::numeric_limits<uint32_t>::max()) {
			throw std::runtime_error{ "Unable to find perfect hash." };
		}

		for (x = 0; x < N; ++x) {
			const auto hash = Xsm1<uint32_t>{}(keys[x], ht.seed);
			ht.hashes[x] = hash;
			ht.table[hash % m] = static_cast<uint8_t>(x);
		}

		return ht;
	}

	struct SingleCharHashDesc {
		size_t N{};
		bool valid{};
		uint8_t min_diff{};
		uint8_t front{};
		uint8_t back{};
		bool is_front_hash = true;
	};

	template<size_t N, bool IsFrontHash = true> inline constexpr SingleCharHashDesc singleCharHash(const std::array<StringView, N>& v) noexcept {
		if constexpr (N > 255) {
			return {};
		}

		std::array<uint8_t, N> hashes;
		for (size_t x = 0; x < N; ++x) {
			if (v[x].size() == 0) {
				return {};
			}
			if constexpr (IsFrontHash) {
				hashes[x] = static_cast<uint8_t>(v[x][0]);
			} else {
				hashes[x] = static_cast<uint8_t>(v[x].back());
			}
		}

		std::sort(hashes.begin(), hashes.end());

		uint8_t min_diff = (std::numeric_limits<uint8_t>::max)();
		for (size_t x = 0; x < N - 1; ++x) {
			if ((hashes[x + 1] - hashes[x]) < min_diff) {
				min_diff = hashes[x + 1] - hashes[x];
			}
		}

		return SingleCharHashDesc{ N, min_diff > 0, min_diff, hashes.front(), hashes.back(), IsFrontHash };
	}

	template<typename OTy, SingleCharHashDesc D> struct SingleCharMap {
		static constexpr auto N = D.N;
		static_assert(N < 256);
		std::array<std::pair<StringView, OTy>, N> items{};
		static constexpr size_t N_table = D.back - D.front + 1;
		std::array<uint8_t, N_table> table{};

		inline constexpr decltype(auto) begin() const noexcept {
			return items.begin();
		}
		inline constexpr decltype(auto) end() const noexcept {
			return items.end();
		}

		inline constexpr Expected<std::reference_wrapper<OTy>, ErrorCode> at(auto&& key) const noexcept {
			if (key.size() == 0) [[unlikely]] {
				return Unexpected(ErrorCode::Unknown_Key);
			}

			if constexpr (D.is_front_hash) {
				const auto k = static_cast<uint8_t>(static_cast<uint8_t>(key[0]) - D.front);
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
				const auto k = static_cast<uint8_t>(static_cast<uint8_t>(key.back()) - D.front);
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

		inline constexpr decltype(auto) find(auto&& key) const noexcept {
			if (key.size() == 0) [[unlikely]] {
				return items.end();
			}

			if constexpr (D.is_front_hash) {
				const auto k = static_cast<uint8_t>(static_cast<uint8_t>(key[0]) - D.front);
				if (k >= static_cast<uint8_t>(N_table)) [[unlikely]] {
					return items.end();
				}
				const auto index = table[k];
				const auto& item = items[index];
				if (!stringConstCompare(item.first, key)) [[unlikely]]
					return items.end();
				return items.begin() + index;
			} else {
				const auto k = static_cast<uint8_t>(static_cast<uint8_t>(key.back()) - D.front);
				if (k >= static_cast<uint8_t>(N_table)) [[unlikely]] {
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

	template<typename OTy, SingleCharHashDesc D> inline constexpr auto makeSingleCharMap(std::initializer_list<std::pair<StringView, OTy>> pairs) {
		constexpr auto N = D.N;
		static_assert(N < 256);
		SingleCharMap<OTy, D> ht{};

		uint8_t x = 0;
		for (const auto& pair: pairs) {
			ht.items[x] = pair;
			if constexpr (D.is_front_hash) {
				ht.table[static_cast<uint8_t>(pair.first[0]) - D.front] = x;
			} else {
				ht.table[static_cast<uint8_t>(pair.first.back()) - D.front] = x;
			}
			++x;
		}

		return ht;
	}

	template<typename OTy, const StringView& S> struct SingleItem {
		std::array<std::pair<StringView, OTy>, 1> items{};

		static constexpr auto SNew = S;

		inline constexpr decltype(auto) end() const noexcept {
			return items.end();
		}

		inline constexpr decltype(auto) find(auto&& key) const noexcept {
			if (cxStringCompare<StringView, S>(key)) [[likely]] {
				return items.begin();
			} else [[unlikely]] {
				return items.end();
			}
		}
	};

	template<typename OTy, const StringView& S0, const StringView& S1> struct DoubleItem {
		std::array<std::pair<StringView, OTy>, 2> items{};

		static constexpr auto s0 = S0;
		static constexpr auto s1 = S1;

		inline constexpr decltype(auto) end() const noexcept {
			return items.end();
		}

		static constexpr bool sameSize = s0.size() == s1.size();

		inline constexpr decltype(auto) find(auto&& key) const noexcept {
			if constexpr (sameSize) {
				constexpr auto n = s0.size();
				if (key.size() != n) {
					return items.end();
				}
			}
			if (cxStringCompare<StringView, s0>(key)) {
				return items.begin();
			} else if (cxStringCompare<StringView, s1>(key)) {
				return items.begin() + 1;
			} else [[unlikely]] {
				return items.end();
			}
		}
	};

	// from
	// https://stackoverflow.com/questions/55941964/how-to-filter-duplicate-types-from-Tuple-c
	template<typename OTy, typename... OTys> struct Unique {
		using type = OTy;
	};

	template<template<typename...> class OTy, typename... OTys, typename U, typename... Us> struct Unique<OTy<OTys...>, U, Us...>
		: std::conditional_t<(std::same_as<U, OTys> || ...), Unique<OTy<OTys...>, Us...>, Unique<OTy<OTys..., U>, Us...>> {};

	template<typename OTy> struct TupleVariant;

	template<typename... OTys> struct TupleVariant<Tuplet::Tuple<OTys...>> : Unique<std::variant<>, OTys...> {};

	template<typename OTy> struct TuplePtrVariant;

	template<typename... OTys> struct TuplePtrVariant<Tuplet::Tuple<OTys...>> : Unique<std::variant<>, std::add_pointer_t<OTys>...> {};

	template<typename... OTys> struct TuplePtrVariant<std::tuple<OTys...>> : Unique<std::variant<>, std::add_pointer_t<OTys>...> {};

	template<typename... OTys> struct TuplePtrVariant<std::pair<OTys...>> : Unique<std::variant<>, std::add_pointer_t<OTys>...> {};

	template<typename Tuple, typename = std::make_index_sequence<std::tuple_size<Tuple>::value>> struct ValueTupleVariant;

	template<typename Tuple, size_t... I> struct ValueTupleVariant<Tuple, std::index_sequence<I...>> {
		using type = typename TupleVariant<decltype(Tuplet::tupleCat(
			std::declval<Tuplet::Tuple<std::tuple_element_t<1, std::tuple_element_t<I, Tuple>>>>()...))>::type;
	};

	template<typename Tuple> using ValueTupleVariantT = typename ValueTupleVariant<Tuple>::type;

	struct BucketSizeCompare {
		template<typename B> bool inline constexpr operator()(B const& b0, B const& b1) const noexcept {
			return b0.size() > b1.size();
		}
	};

	template<size_t M> struct PmhBuckets {
		inline static constexpr auto bucket_max = 2 * (1u << (log(M) / 2));

		using bucket_t = RawVector<size_t, bucket_max>;
		RawArray<bucket_t, M> buckets;
		uint64_t seed;

		struct BucketRef {
			unsigned hash;
			const bucket_t* ptr;
			using value_type = typename bucket_t::value_type;
			using const_iterator = typename bucket_t::const_iterator;

			inline constexpr auto size() const noexcept {
				return ptr->size();
			}

			inline constexpr const auto& operator[](size_t idx) const noexcept {
				return (*ptr)[idx];
			}

			inline constexpr auto begin() const noexcept {
				return ptr->begin();
			}

			inline constexpr auto end() const noexcept {
				return ptr->end();
			}
		};

		template<size_t... Is> RawArray<BucketRef, M> inline constexpr makeBucketRefs(std::index_sequence<Is...>) const noexcept {
			return { { BucketRef{ Is, &buckets[Is] }... } };
		}

		RawArray<BucketRef, M> inline constexpr getSortedBuckets() const noexcept {
			RawArray<BucketRef, M> result{ makeBucketRefs(std::make_index_sequence<M>()) };
			quicksort(result.begin(), result.end() - 1, BucketSizeCompare{});
			return result;
		}
	};

	template<size_t M, typename Item, size_t N, typename Hash, typename Key>
	PmhBuckets<M> inline constexpr makePmhBuckets(const RawArray<Item, N>& items, Hash const& hash, Key const& key, DefaultPrgT& prg) {
		using result_t = PmhBuckets<M>;
		result_t result{};
		bool rejected = false;
		while (1) {
			for (auto& b: result.buckets) {
				b.clear();
			}
			result.seed = prg();
			rejected = false;
			for (size_t x = 0; x < N; ++x) {
				auto& bucket = result.buckets[hash(key(items[x]), static_cast<size_t>(result.seed)) % M];
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

	template<typename OTy, size_t N> inline constexpr bool allDifferentFrom(RawVector<OTy, N>& data, OTy& a) {
		for (size_t x = 0; x < data.size(); ++x)
			if (data[x] == a) [[unlikely]] {
				return false;
			}
		return true;
	}

	struct SeedOrIndex {
		using value_type = uint64_t;

	  protected:
		static constexpr value_type MINUS_ONE = (std::numeric_limits<value_type>::max)();
		static constexpr value_type HIGH_BIT = ~(MINUS_ONE >> 1);

		value_type val = 0;

	  public:
		inline constexpr value_type value() const noexcept {
			return val;
		}

		inline constexpr bool isSeed() const noexcept {
			return val & HIGH_BIT;
		}

		inline constexpr SeedOrIndex(bool isSeed, value_type value) : val(isSeed ? (value | HIGH_BIT) : (value & ~HIGH_BIT)) {
		}

		inline constexpr SeedOrIndex() noexcept = default;
		inline constexpr SeedOrIndex(const SeedOrIndex&) noexcept = default;
		inline constexpr SeedOrIndex& operator=(const SeedOrIndex&) noexcept = default;
	};

	template<size_t M> struct PmhTables {
		uint64_t firstSeed;
		RawArray<SeedOrIndex, M> firstTable;
		RawArray<size_t, M> secondTable;
		Hash<StringView> hash;

		template<typename KeyType> inline constexpr size_t lookup(const KeyType& key) const noexcept {
			return lookup(key, hash);
		}

		template<typename KeyType> inline constexpr size_t lookup(const KeyType& key, const Hash<StringView>& hasher) const noexcept {
			auto const d = firstTable[hasher(key, static_cast<size_t>(firstSeed)) % M];
			if (!d.isSeed()) [[unlikely]] {
				return static_cast<size_t>(d.value());
			} else [[likely]] {
				return secondTable[hasher(key, static_cast<size_t>(d.value())) % M];
			}
		}
	};

	template<size_t M, typename Item, size_t N, typename Key>
	PmhTables<M> inline constexpr makePmhTables(const RawArray<Item, N>& items, Hash<StringView> const& hash, Key const& key, DefaultPrgT prg) {
		auto step_one = makePmhBuckets<M>(items, hash, key, prg);
		auto buckets = step_one.getSortedBuckets();

		RawArray<SeedOrIndex, M> G;

		constexpr size_t UNUSED = (std::numeric_limits<size_t>::max)();
		RawArray<size_t, M> H;
		H.fill(UNUSED);

		for (const auto& bucket: buckets) {
			auto const bsize = bucket.size();

			if (bsize == 1) {
				G[bucket.hash] = { false, static_cast<uint64_t>(bucket[0]) };
			} else if (bsize > 1) {
				SeedOrIndex d{ true, prg() };
				RawVector<size_t, decltype(step_one)::bucket_max> bucket_slots;

				while (bucket_slots.size() < bsize) {
					auto slot = hash(key(items[bucket[bucket_slots.size()]]), d.value()) % M;

					if (H[slot] != UNUSED || !allDifferentFrom(bucket_slots, slot)) {
						bucket_slots.clear();
						d = { true, prg() };
						continue;
					}

					bucket_slots.push_back(slot);
				}

				G[bucket.hash] = d;
				for (size_t x = 0; x < bsize; ++x)
					H[bucket_slots[x]] = bucket[x];
			}
		}

		for (size_t x = 0; x < M; ++x)
			if (H[x] == UNUSED)
				H[x] = 0;

		return { step_one.seed, G, H, hash };
	}

	struct GetKey {
		template<typename KV> inline constexpr auto const& operator()(KV const& kv) const noexcept {
			return kv.first;
		}
	};

	template<typename Key, typename Value, size_t N> class UnorderedMap {
		inline static constexpr size_t storage_size = nextHighestPowerOfTwo(N) * (N < 32 ? 2 : 1);
		using container_type = RawArray<std::pair<Key, Value>, N>;
		using tables_type = PmhTables<storage_size>;

		StringCompareHelper const equal;
		container_type items;
		tables_type tables;

	  public:
		using Self = UnorderedMap<Key, Value, N>;
		using key_type = Key;
		using mapped_type = Value;
		using value_type = typename container_type::value_type;
		using size_type = typename container_type::size_type;
		using difference_type = typename container_type::difference_type;
		using hasher = Hash<key_type>;
		using key_equal = StringCompareHelper;
		using reference = typename container_type::reference;
		using const_reference = typename container_type::const_reference;
		using pointer = typename container_type::pointer;
		using const_pointer = typename container_type::const_pointer;
		using iterator = typename container_type::iterator;
		using const_iterator = typename container_type::const_iterator;

	  public:
		inline constexpr UnorderedMap() noexcept = default;
		inline constexpr UnorderedMap& operator=(UnorderedMap&&) noexcept = default;
		inline constexpr UnorderedMap(UnorderedMap&&) noexcept = default;
		inline constexpr UnorderedMap& operator=(const UnorderedMap&) noexcept = default;
		inline constexpr UnorderedMap(const UnorderedMap&) noexcept = default;

		inline constexpr UnorderedMap(container_type items, hasher const& hash, StringCompareHelper const& equalNew)
			: equal{ equalNew }, items{ items }, tables{ makePmhTables<storage_size>(items, hash, GetKey{}, DefaultPrgT{}) } {
		}

		inline explicit constexpr UnorderedMap(container_type items) : UnorderedMap{ items, hasher{}, StringCompareHelper{} } {
		}

		inline constexpr UnorderedMap(std::initializer_list<value_type> items, hasher const& hash, StringCompareHelper const& equalNew)
			: UnorderedMap{ container_type{ items }, hash, equalNew } {
			constexpr_assert(items.size() == N, "Inconsistent initializer_list size and type size argument.");
		};

		inline constexpr UnorderedMap(std::initializer_list<value_type> items) : UnorderedMap{ items, hasher{}, StringCompareHelper{} } {
		}

		inline constexpr iterator begin() {
			return items.begin();
		}

		inline constexpr iterator end() {
			return items.end();
		}

		inline constexpr const_iterator begin() const noexcept {
			return items.begin();
		}

		inline constexpr const_iterator end() const noexcept {
			return items.end();
		}

		inline constexpr bool empty() const noexcept {
			return !N;
		}

		inline constexpr size_type size() const noexcept {
			return N;
		}

		inline constexpr size_type maxSize() const noexcept {
			return N;
		}

		inline constexpr size_t count(const key_type& key, const hasher& hash, const key_equal& equal) const noexcept {
			const auto& kv = lookup(key, hash);
			return equal(kv.first, key);
		}

		inline constexpr size_t count(const key_type& key) const noexcept {
			return count(key, hashFunction(), keyEq());
		}

		inline constexpr const_iterator find(const key_type& key, const hasher& hash, const key_equal& equal) const noexcept {
			return findImpl(*this, key, hash, equal);
		}

		inline constexpr iterator find(const key_type& key, const hasher& hash, const key_equal& equal) {
			return findImpl(*this, key, hash, equal);
		}

		inline constexpr const_iterator find(const key_type& key) const noexcept {
			return find(key, hashFunction(), keyEq());
		}

		inline constexpr iterator find(const key_type& key) {
			return find(key, hashFunction(), keyEq());
		}

		inline constexpr std::pair<const_iterator, const_iterator> equalRange(const key_type& key, const hasher& hash,
			const key_equal& equal) const noexcept {
			return equalRangeImpl(*this, key, hash, equal);
		}

		inline constexpr std::pair<iterator, iterator> equalRange(const key_type& key, const hasher& hash, const key_equal& equal) {
			return equalRangeImpl(*this, key, hash, equal);
		}

		inline constexpr std::pair<const_iterator, const_iterator> equalRange(const key_type& key) const noexcept {
			return equalRange(key, hashFunction(), keyEq());
		}

		inline constexpr std::pair<iterator, iterator> equalRange(const key_type& key) {
			return equalRange(key, hashFunction(), keyEq());
		}

		inline constexpr size_t bucketCount() const noexcept {
			return storage_size;
		}

		inline constexpr size_t maxBucketCount() const noexcept {
			return storage_size;
		}

		inline constexpr const hasher& hashFunction() const noexcept {
			return tables.hash;
		}

		inline constexpr const key_equal& keyEq() const noexcept {
			return equal;
		}

		inline constexpr size_t tableLookup(const key_type& key) const noexcept {
			return tables.lookup(key, hashFunction());
		}

		inline constexpr auto& unsafeValueAccess(const size_t x) {
			return items[x].second;
		}

	  protected:
		template<typename OTy> inline static constexpr auto findImpl(OTy&& self, const key_type& key, const hasher& hash, const key_equal& equal) {
			auto& kv = self.lookup(key, hash);
			if (equal(kv.first, key))
				return &kv;
			else
				return self.items.end();
		}

		template<typename OTy>
		inline static constexpr auto equalRangeImpl(OTy&& self, const key_type& key, const hasher& hash, const key_equal& equal) {
			auto& kv = self.lookup(key, hash);
			using kv_ptr = decltype(&kv);
			if (equal(kv.first, key))
				return std::pair<kv_ptr, kv_ptr>{ &kv, &kv + 1 };
			else
				return std::pair<kv_ptr, kv_ptr>{ self.items.end(), self.items.end() };
		}

		template<typename OTy> inline static constexpr auto& lookupImpl(OTy&& self, const key_type& key, const hasher& hash) {
			return self.items[self.tables.lookup(key, hash)];
		}

		inline constexpr const auto& lookup(const key_type& key, const hasher& hash) const noexcept {
			return lookupImpl(*this, key, hash);
		}

		inline constexpr auto& lookup(const key_type& key, const hasher& hash) {
			return lookupImpl(*this, key, hash);
		}
	};

	template<typename OTy, typename U, size_t N> inline constexpr auto makeUnorderedMap(std::pair<OTy, U> const (&items)[N],
		const Hash<OTy>& hash = Hash<OTy>{}, const StringCompareHelper& equal = StringCompareHelper{}) {
		return UnorderedMap<OTy, U, N>{ items, hash, equal };
	}

	template<typename OTy, size_t I> struct CoreSV {
		inline static constexpr StringView value = Tuplet::get<0>(Tuplet::get<I>(CoreV<OTy>));
	};

	template<typename OTy, size_t... I> inline constexpr auto makeMapImpl(std::index_sequence<I...>) {
		using value_t = ValueTupleVariantT<CoreT<OTy>>;
		constexpr auto n = std::tuple_size_v<CoreT<OTy>>;
		constexpr auto size = sizeof...(I);
		static_assert(size == n);
		auto naiveOrNormalHash = [&] {
			if constexpr (size <= 20) {
				return makeNaiveMap<value_t, n>({ std::make_pair<StringView, value_t>(StringView(Tuplet::get<0>(Tuplet::get<I>(CoreV<OTy>))),
					Tuplet::get<1>(Tuplet::get<I>(CoreV<OTy>)))... });
			} else {
				return makeUnorderedMap<StringView, value_t, n>({ std::make_pair<StringView, value_t>(
					StringView(Tuplet::get<0>(Tuplet::get<I>(CoreV<OTy>))), Tuplet::get<1>(Tuplet::get<I>(CoreV<OTy>)))... });
			}
		};

		constexpr bool n128 = n < 128;
		if constexpr (n == 0) {
			static_assert(FalseV<OTy>, "Empty object in Jsonifier::Core.");
		} else if constexpr (n == 1) {
			return SingleItem<value_t, CoreSV<OTy, I>::value...>{ std::make_pair<StringView, value_t>(
				StringView(Tuplet::get<0>(Tuplet::get<I>(CoreV<OTy>))), Tuplet::get<1>(Tuplet::get<I>(CoreV<OTy>)))... };
		} else if constexpr (n == 2) {
			return DoubleItem<value_t, CoreSV<OTy, I>::value...>{ std::make_pair<StringView, value_t>(
				StringView(Tuplet::get<0>(Tuplet::get<I>(CoreV<OTy>))), Tuplet::get<1>(Tuplet::get<I>(CoreV<OTy>)))... };
		} else if constexpr (n128) {
			constexpr auto frontDesc = singleCharHash<n>(std::array<StringView, n>{ StringView{ Tuplet::get<0>(Tuplet::get<I>(CoreV<OTy>)) }... });

			if constexpr (frontDesc.valid) {
				return makeSingleCharMap<value_t, frontDesc>({ std::make_pair<StringView, value_t>(
					StringView(Tuplet::get<0>(Tuplet::get<I>(CoreV<OTy>))), Tuplet::get<1>(Tuplet::get<I>(CoreV<OTy>)))... });
			} else {
				constexpr auto backDesc =
					singleCharHash<n, false>(std::array<StringView, n>{ StringView{ Tuplet::get<0>(Tuplet::get<I>(CoreV<OTy>)) }... });

				if constexpr (backDesc.valid) {
					return makeSingleCharMap<value_t, backDesc>({ std::make_pair<StringView, value_t>(
						StringView(Tuplet::get<0>(Tuplet::get<I>(CoreV<OTy>))), Tuplet::get<1>(Tuplet::get<I>(CoreV<OTy>)))... });
				} else {
					return naiveOrNormalHash();
				}
			}
		} else {
			return naiveOrNormalHash();
		}
	}

	template<typename OTy> inline constexpr auto makeMap() {
		constexpr auto indices = std::make_index_sequence<std::tuple_size_v<CoreT<OTy>>>{};
		return makeMapImpl<std::decay_t<OTy>>(indices);
	}
}
