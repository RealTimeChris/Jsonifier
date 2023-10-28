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
/// https://github.com/RealTimeChris/jsonifier
/// Feb 20, 2023
#pragma once

#include <jsonifier/StringView.hpp>
#include <jsonifier/CTimeArray.hpp>
#include <jsonifier/CTimeVector.hpp>
#include <jsonifier/Expected.hpp>
#include <jsonifier/Error.hpp>
#include <jsonifier/Tuple.hpp>
#include <jsonifier/Base02.hpp>
#include <algorithm>
#include <span>

namespace jsonifier_internal {

	template<typename value_type> constexpr uint64_t toUint64(const value_type* bytes, const uint64_t n) {
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

	template<uint64_t n, typename value_type> constexpr uint64_t toUint64N(const value_type* bytes) {
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

	struct string_compare_helper {
		template<typename T0, typename T1> jsonifier_constexpr bool operator()(T0&& lhs, T1&& rhs) const {
			if (std::is_constant_evaluated()) {
				return stringConstCompare(std::forward<T0>(lhs), std::forward<T1>(rhs));
			} else {
				if (lhs.size() == rhs.size()) {
					return jsonifier_core_internal::compare(lhs.data(), rhs.data(), rhs.size());
				}
				return false;
			}
		}
	};

	template<typename value_type> struct hash<std::basic_string<value_type>> {
		constexpr uint64_t operator()(std::basic_string<uint8_t> value) const {
			return fnv1aHash(value);
		}

		constexpr uint64_t operator()(std::basic_string<uint8_t> value, uint64_t seed) const {
			return fnv1aHash(value, seed);
		}

		constexpr uint64_t operator()(std::basic_string<char> value) const {
			return fnv1aHash(value);
		}

		constexpr uint64_t operator()(std::basic_string<char> value, uint64_t seed) const {
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

	template<typename value_type> constexpr auto log(value_type v) {
		uint64_t n = 0;
		while (v > 1) {
			n += 1;
			v >>= 1;
		}
		return n;
	}

	constexpr uint64_t bitWeight(uint64_t n) {
		return static_cast<uint64_t>(n <= 8ULL) * sizeof(uint32_t) + (static_cast<uint64_t>(n <= 8ULL) * sizeof(uint32_t)) +
			(static_cast<uint64_t>(n <= 8ULL) * sizeof(unsigned long long)) + (n <= 128ULL);
	}

	template<uint64_t n> jsonifier_inline unsigned long long selectUintLeast(std::integral_constant<uint64_t, n>) {
		static_assert(n < 2, "Unsupported type size.");
		return {};
	}

	template<uint64_t n> using SelectUintLeastT = decltype(selectUintLeast(std::integral_constant<uint64_t, bitWeight(n)>()));

	template<typename value_type> jsonifier_constexpr void primitiveSwap(value_type&& a, value_type&& b) {
		auto tmp = std::move(a);
		a		 = std::move(b);
		b		 = std::move(tmp);
	}

	template<typename value_type, typename u> jsonifier_constexpr void primitiveSwap(pair<value_type, u>&& a, pair<value_type, u>&& b) {
		primitiveSwap(std::forward<pair<value_type, u>>(a.first), std::forward<pair<value_type, u>>(b.first));
		primitiveSwap(std::forward<pair<value_type, u>>(a.second), std::forward<pair<value_type, u>>(b.second));
	}

	template<typename... Tys, uint64_t... Is> jsonifier_constexpr void primitiveSwap(tuplet::tuple<Tys...>&& a, tuplet::tuple<Tys...>&& b, std::index_sequence<Is...>) {
		using swallow = int32_t[];
		( void )swallow{ (primitiveSwap(std::forward<Tys...>(tuplet::get<Is>(a)), std::forward<Tys...>(tuplet::get<Is>(b))), 0)... };
	}

	template<typename... Tys> jsonifier_constexpr void primitiveSwap(tuplet::tuple<Tys...>& a, tuplet::tuple<Tys...>& b) {
		primitiveSwap(a, b, std::make_index_sequence<sizeof...(Tys)>());
	}

	template<typename iterator, typename compare> jsonifier_constexpr iterator partition(iterator left, iterator right, const compare& compareNew) {
		auto pivot = left + (right - left) / 2;
		auto value = *pivot;
		primitiveSwap(*right, *pivot);
		for (auto it = left; 0 < right - it; ++it) {
			if (compareNew(*it, value)) {
				primitiveSwap(*it, *left);
				++left;
			}
		}
		primitiveSwap(*right, *left);
		return left;
	}

	template<typename iterator, typename compare> jsonifier_constexpr void quicksort(iterator left, iterator right, const compare& compareNew) {
		while (0 < right - left) {
			auto new_pivot = partition(left, right, compareNew);
			quicksort(left, new_pivot, compareNew);
			left = new_pivot + 1;
		}
	}

	template<typename value_type, uint64_t n, typename compare> jsonifier_constexpr ctime_array<value_type, n> quicksort(ctime_array<value_type, n> const& array, const compare& compareNew) {
		ctime_array<value_type, n> res = array;
		quicksort(res.begin(), res.end() - 1, compareNew);
		return res;
	}

	template<typename input_iter_1, typename input_iter_2> jsonifier_constexpr bool equal(input_iter_1 first1, input_iter_1 last1, input_iter_2 first2) {
		for (; first1 != last1; ++first1, ++first2) {
			if (!(*first1 == *first2)) {
				return false;
			}
		}
		return true;
	}

	template<typename uint_type, uint_type multiplier, uint_type increment, uint_type modulus> class linear_congruential_engine {
		static_assert(std::is_unsigned<uint_type>::value, "uint_type must be an unsigned integral type.");
		template<typename value_type> static jsonifier_constexpr uint_type modulo(value_type val, std::integral_constant<uint_type, 0>) {
			return static_cast<uint_type>(val);
		}
		template<typename value_type, uint_type m2> static jsonifier_constexpr uint_type modulo(value_type val, std::integral_constant<uint_type, m2>) {
			return static_cast<uint_type>(val % m2);
		}

	  public:
		using result_type						  = uint_type;
		static constexpr result_type default_seed	  = 1u;
		jsonifier_inline linear_congruential_engine()		  = default;
		jsonifier_constexpr linear_congruential_engine(result_type S) {
			seed(S);
		}
		jsonifier_inline void seed(result_type S = default_seed) {
			state_ = S;
		}

		jsonifier_constexpr result_type operator()() {
			using uint_least_t = SelectUintLeastT<log(multiplier) + log(modulus) + 4>;
			uint_least_t tmp   = static_cast<uint_least_t>(multiplier) * state_ + increment;

			state_ = modulo(tmp, std::integral_constant<uint_type, modulus>());
			return state_;
		}

		jsonifier_constexpr void discard(unsigned long long n) {
			while (n--)
				operator()();
		}

		jsonifier_constexpr bool operator==(linear_congruential_engine const& other) const {
			return state_ == other.state_;
		}

	  protected:
		result_type state_ = default_seed;
	};

	using default_prg_t = linear_congruential_engine<std::uint_fast32_t, 48271, 0, 2147483647>;

	// https://en.wikipedia.org/wiki/Fowler�Noll�Vo_hash_function
	// http://www.isthe.com/chongo/tech/comp/fnv/index.html#FNV-param
	static constexpr uint64_t fnv64Prime	   = 1099511628211;
	static constexpr uint64_t fnv64OffsetBasis = 0xcbf29ce484222325;

	template<typename hash_type> struct xsm1 {};

	template<> struct xsm1<uint64_t> {
		template<typename value_type> constexpr uint64_t operator()(value_type&& value, const uint64_t seed) {
			uint64_t h	 = (fnv64OffsetBasis ^ seed) * fnv64Prime;
			const auto n = value.size();

			if (n < 8) {
				const auto shift = 64 - 8 * n;
				h ^= toUint64(value.data(), n) << shift;
				h ^= h >> 33;
				h *= fnv64Prime;
				return h;
			}
			using string_type							 = std::unwrap_ref_decay_t<value_type>;
			const typename string_type::value_type* d0	 = value.data();
			const typename string_type::value_type* end7 = value.data() + n - 7;
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

	template<> struct xsm1<uint32_t> {
		jsonifier_constexpr uint32_t operator()(auto&& value, const uint32_t seed) {
			uint64_t hash = xsm1<uint64_t>{}(value, seed);
			return hash >> 32;
		}
	};

	jsonifier_constexpr bool contains(auto&& data, auto&& val) {
		const auto n = data.size();
		for (uint64_t x = 0; x < n; ++x) {
			if (data[x] == val) {
				return true;
			}
		}
		return false;
	}

	template<uint64_t n> constexpr auto naiveBucketSize() {
		return n < 8 ? 2 * n : 4 * n;
	}

	template<uint64_t n> jsonifier_constexpr uint32_t naivePerfectHash(auto&& keys) {
		static_assert(n <= 20);
		constexpr uint64_t m = naiveBucketSize<n>();
		ctime_array<uint64_t, n> hashes{};
		ctime_array<uint64_t, n> buckets{};


		default_prg_t gen{};
		for (uint64_t x = 0; x < 1024; ++x) {
			uint32_t seed  = gen();
			uint64_t index = 0;
			for (const auto& key: keys) {
				const auto hash = xsm1<uint32_t>{}(key, seed);
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

		return std::numeric_limits<uint32_t>::max();
	}

	template<typename Value, uint64_t n> struct naive_map {
		static_assert(n <= 20);
		static constexpr uint64_t m = naiveBucketSize<n>();
		uint32_t seed{};
		ctime_array<pair<jsonifier::string_view, Value>, n> items{};
		ctime_array<uint32_t, n * 1> hashes{};
		ctime_array<uint8_t, m> table{};

		jsonifier_constexpr decltype(auto) begin() const {
			return items.begin();
		}

		jsonifier_constexpr decltype(auto) end() const {
			return items.end();
		}

		jsonifier_constexpr expected<std::reference_wrapper<Value>, error_code> at(auto&& key) const {
			const auto hash	 = xsm1<uint32_t>{}(key, seed);
			const auto index = table[hash % m];
			const auto& item = items[index];
			if (hashes[index] != hash) [[unlikely]] {
				return unexpected(error_code::Unknown_Key);
			}
			return item.second;
		}

		jsonifier_constexpr decltype(auto) find(auto&& key) const {
			const auto hash	 = xsm1<uint32_t>{}(key, seed);
			const auto index = table[hash % m];
			if (hashes[index] != hash) [[unlikely]]
				return items.end();
			return items.begin() + index;
		}
	};

	template<typename value_type, uint64_t n> constexpr auto makeNaiveMap(std::initializer_list<pair<jsonifier::string_view, value_type>> pairs) {
		static_assert(n <= 20);
		if (pairs.size() != n) {
			throw std::runtime_error{ "pairs.size() != n" };
		}
		naive_map<value_type, n> ht{};
		constexpr uint64_t m = naiveBucketSize<n>();

		ctime_array<jsonifier::string_view, n> keys{};
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
			const auto hash	   = xsm1<uint32_t>{}(keys[x], ht.seed);
			ht.hashes[x]	   = hash;
			ht.table[hash % m] = static_cast<unsigned char>(x);
		}

		return ht;
	}

	struct single_char_hash_desc {
		uint64_t n{};
		bool valid{};
		uint8_t min_diff{};
		uint8_t front{};
		uint8_t back{};
		bool is_front_hash = true;
		uint8_t padding[3]{};
	};

	template<uint64_t n, bool IsFrontHash = true> jsonifier_constexpr single_char_hash_desc singleCharHash(const ctime_array<jsonifier::string_view, n>& v) {
		if constexpr (n > 255) {
			return {};
		}

		ctime_array<uint8_t, n> hashes;
		for (uint64_t x = 0; x < n; ++x) {
			if (v[x].size() == 0) {
				return {};
			}
			if constexpr (IsFrontHash) {
				hashes[x] = static_cast<unsigned char>(v[x][0]);
			} else {
				hashes[x] = static_cast<unsigned char>(v[x].back());
			}
		}

		std::sort(hashes.begin(), hashes.end());

		uint8_t min_diff = std::numeric_limits<uint8_t>::max();
		for (uint64_t x = 0; x < n - 1; ++x) {
			if ((hashes[x + 1] - hashes[x]) < min_diff) {
				min_diff = static_cast<uint8_t>(hashes[x + 1] - hashes[x]);
			}
		}

		return single_char_hash_desc{ n, min_diff > 0, min_diff, hashes.front(), hashes.back(), IsFrontHash };
	}

	template<typename value_type, single_char_hash_desc D> struct single_char_map {
		static constexpr auto n = D.n;
		static_assert(n < 256);
		ctime_array<pair<jsonifier::string_view, value_type>, n> items{};
		static constexpr uint64_t N_table = static_cast<uint64_t>(D.back) - D.front + 1;
		ctime_array<uint8_t, N_table> table{};

		jsonifier_inline decltype(auto) begin() const {
			return items.begin();
		}
		jsonifier_inline decltype(auto) end() const {
			return items.end();
		}

		jsonifier_constexpr expected<std::reference_wrapper<value_type>, error_code> at(auto&& key) const {
			if (key.size() == 0) [[unlikely]] {
				return unexpected(error_code::Unknown_Key);
			}

			if constexpr (D.is_front_hash) {
				const auto k = static_cast<uint64_t>(key[0] - D.front);
				if (k >= N_table) [[unlikely]] {
					return unexpected(error_code::Unknown_Key);
				}
				const auto index = table[k];
				const auto& item = items[index];
				if (!stringConstCompare(item.first, key)) [[unlikely]] {
					return unexpected(error_code::Unknown_Key);
				}
				return item.second;
			} else {
				const auto k = static_cast<uint64_t>(key.back() - D.front);
				if (k >= N_table) [[unlikely]] {
					return unexpected(error_code::Unknown_Key);
				}
				const auto index = table[k];
				const auto& item = items[index];
				if (!stringConstCompare(item.first, key)) [[unlikely]] {
					return unexpected(error_code::Unknown_Key);
				}
				return item.second;
			}
		}

		jsonifier_constexpr decltype(auto) find(auto&& key) const {
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

	template<typename value_type, single_char_hash_desc D> constexpr auto makeSingleCharMap(std::initializer_list<pair<jsonifier::string_view, value_type>> pairs) {
		static_assert(D.n < 256);
		single_char_map<value_type, D> ht{};

		uint8_t x = 0;
		for (const auto& pair: pairs) {
			ht.items[x] = pair;
			if constexpr (D.is_front_hash) {
				ht.table[static_cast<uint64_t>(pair.first[0] - D.front)] = x;
			} else {
				ht.table[static_cast<uint64_t>(pair.first.back() - D.front)] = x;
			}
			++x;
		}

		return ht;
	}

	template<typename value_type, const jsonifier::string_view& S> struct single_item {
		ctime_array<pair<jsonifier::string_view, value_type>, 1> items{};

		jsonifier_constexpr decltype(auto) end() const {
			return items.end();
		}

		jsonifier_constexpr decltype(auto) find(auto&& key) const {
			if (stringConstCompare(S, key)) [[likely]] {
				return items.begin();
			} else [[unlikely]] {
				return items.end();
			}
		}
	};

	template<typename value_type, const jsonifier::string_view& string01, const jsonifier::string_view& string02> struct double_item {
		ctime_array<pair<jsonifier::string_view, value_type>, 2> items{};

		static constexpr auto s0 = string01;
		static constexpr auto s1 = string02;

		jsonifier_constexpr decltype(auto) end() const {
			return items.end();
		}

		static constexpr bool sameSize = s0.size() == s1.size();

		jsonifier_constexpr decltype(auto) find(auto&& key) const {
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
	// https://stackoverflow.com/questions/55941964/how-to-filter-duplicate-types-from-tuple-c
	template<typename value_type, typename... value_types> struct unique {
		using type = value_type;
	};

	template<template<typename...> class value_type, typename... value_types, typename u, typename... Us> struct unique<value_type<value_types...>, u, Us...>
		: std::conditional_t<(std::same_as<u, value_types> || ...), unique<value_type<value_types...>, Us...>, unique<value_type<value_types..., u>, Us...>> {};

	template<typename value_type> struct tuple_variant;

	template<typename... value_types> struct tuple_variant<tuplet::tuple<value_types...>> : unique<std::variant<>, value_types...> {};

	template<typename value_type> struct tuple_ptr_variant;

	template<typename... value_types> struct tuple_ptr_variant<std::tuple<value_types...>> : unique<std::variant<>, std::add_pointer_t<value_types>...> {};

	template<typename... value_types> struct tuple_ptr_variant<tuplet::tuple<value_types...>> : unique<std::variant<>, std::add_pointer_t<value_types>...> {};

	template<typename... value_types> struct tuple_ptr_variant<pair<value_types...>> : unique<std::variant<>, std::add_pointer_t<value_types>...> {};

	template<typename tuple, typename = std::make_index_sequence<std::tuple_size<tuple>::value>> struct value_tuple_variant;

	template<typename tuple, size_t... I> struct value_tuple_variant<tuple, std::index_sequence<I...>> {
		using type = typename tuple_variant<decltype(tuplet::tupleCat(std::declval<tuplet::tuple<std::tuple_element_t<1, std::tuple_element_t<I, tuple>>>>()...))>::type;
	};

	template<typename tuple> using value_tuple_variant_t = typename value_tuple_variant<tuple>::type;

	struct bucket_size_compare {
		template<typename b> bool jsonifier_constexpr operator()(b const& b0, b const& b1) const {
			return b0.size() > b1.size();
		}
	};

	template<uint64_t m> struct pmh_buckets {
		static constexpr auto bucket_max = 2 * (1u << (log(m) / 2));

		using bucket_t = ctime_vector<uint64_t, bucket_max>;
		ctime_array<bucket_t, m> buckets;
		uint64_t seed;

		struct bucket_ref {
			unsigned hash;
			const bucket_t* values;
			using value_type	 = typename bucket_t::value_type;
			using const_iterator = typename bucket_t::const_iterator;

			constexpr auto size() const {
				return values->size();
			}

			jsonifier_constexpr const auto& operator[](uint64_t idx) const {
				return (*values)[idx];
			}

			constexpr auto begin() const {
				return values->begin();
			}

			constexpr auto end() const {
				return values->end();
			}
		};

		template<size_t... Is> ctime_array<bucket_ref, m> jsonifier_constexpr makeBucketRefs(std::index_sequence<Is...>) const {
			return { { bucket_ref{ Is, &buckets[Is] }... } };
		}

		ctime_array<bucket_ref, m> jsonifier_constexpr getSortedBuckets() const {
			ctime_array<bucket_ref, m> result{ makeBucketRefs(std::make_index_sequence<m>()) };
			quicksort(result.begin(), result.end() - 1, bucket_size_compare{});
			return result;
		}
	};

	template<size_t m, typename Item, size_t n, typename key> pmh_buckets<m> jsonifier_constexpr makePmhBuckets(const ctime_array<Item, n>& items, key const& keyNew, default_prg_t& prg) {
		using result_t = pmh_buckets<m>;
		using hasher   = hash<jsonifier::string_view>;
		result_t result{};
		bool rejected = false;
		while (true) {
			for (auto& b: result.buckets) {
				b.clear();
			}
			result.seed = prg();
			rejected	= false;
			for (uint64_t x = 0; x < n; ++x) {
				auto& bucket = result.buckets[hasher{}(keyNew(items[x]), static_cast<uint64_t>(result.seed)) % m];
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

	template<typename value_type, uint64_t n> jsonifier_constexpr bool allDifferentFrom(ctime_vector<value_type, n>& data, value_type& a) {
		for (uint64_t x = 0; x < data.size(); ++x)
			if (data[x] == a) [[unlikely]] {
				return false;
			}
		return true;
	}

	struct seed_or_index {
		using value_type = uint64_t;

	  protected:
		static constexpr value_type MinusOne = std::numeric_limits<value_type>::max();
		static constexpr value_type HighBit			   = ~(MinusOne >> 1);

		value_type val = 0;

	  public:
		jsonifier_constexpr value_type value() const {
			return val;
		}

		jsonifier_constexpr bool isSeed() const {
			return val & HighBit;
		}

		jsonifier_constexpr seed_or_index(bool isSeed, value_type value) : val(isSeed ? (value | HighBit) : (value & ~HighBit)) {
		}

		jsonifier_constexpr seed_or_index() = default;
	};

	template<uint64_t m> struct pmh_tables {
		jsonifier_constexpr pmh_tables() = default;
		jsonifier_constexpr pmh_tables(uint64_t firstSeedNew, ctime_array<seed_or_index, m> firstTableNew, ctime_array<uint64_t, m> secondTableNew) {
			firstSeed	= firstSeedNew;
			firstTable	= firstTableNew;
			secondTable = secondTableNew;
		}
		uint64_t firstSeed;
		ctime_array<seed_or_index, m> firstTable;
		ctime_array<uint64_t, m> secondTable;

		template<typename KeyType> using hasher = hash<KeyType>;

		template<typename value_type> jsonifier_constexpr const hasher<value_type> getHasher() const {
			return hasher<value_type>{};
		}

		template<typename KeyType> constexpr uint64_t lookup(const KeyType& key) const {
			auto const d = firstTable[getHasher<KeyType>()(key, static_cast<uint64_t>(firstSeed)) % m];
			if (!d.isSeed()) [[unlikely]] {
				return static_cast<uint64_t>(d.value());
			} else [[likely]] {
				return secondTable[getHasher<KeyType>()(key, static_cast<uint64_t>(d.value())) % m];
			}
		}
	};

	template<size_t m, typename Item, size_t n, typename key> pmh_tables<m> jsonifier_constexpr makePmhTables(const ctime_array<Item, n>& items, key const& keyNew, default_prg_t prg) {
		auto step_one = makePmhBuckets<m>(items, keyNew, prg);
		auto buckets  = step_one.getSortedBuckets();

		ctime_array<seed_or_index, m> G;
		using hasher = hash<jsonifier::string_view_base<uint8_t>>;

		constexpr uint64_t UNUSED = std::numeric_limits<uint64_t>::max();
		ctime_array<uint64_t, m> H;
		H.fill(UNUSED);

		for (const auto& bucket: buckets) {
			auto const bsize = bucket.size();

			if (bsize == 1) {
				G[bucket.hash] = { false, static_cast<uint64_t>(bucket[0]) };
			} else if (bsize > 1) {
				seed_or_index d{ true, prg() };
				ctime_vector<uint64_t, decltype(step_one)::bucket_max> bucket_slots;

				while (bucket_slots.size() < bsize) {
					auto slot = hasher{}(keyNew(items[bucket[bucket_slots.size()]]), d.value()) % m;

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

	struct get_key {
		template<typename KV> constexpr auto const& operator()(KV const& kv) const {
			return kv.first;
		}
	};

	template<typename key, typename Value, size_t n>
	class unordered_map : protected string_compare_helper, protected ctime_array<pair<key, Value>, n>, protected pmh_tables<nextHighestPowerOfTwo(n) * (n < 32 ? 2 : 1)> {
	  public:
		static constexpr auto storageSize = nextHighestPowerOfTwo(n) * (n < 32 ? 2 : 1);
		using container_type			  = ctime_array<pair<key, Value>, n>;
		using tables_type				  = pmh_tables<storageSize>;

		using key_type		  = key;
		using mapped_type	  = Value;
		using value_type	  = typename container_type::value_type;
		using size_type		  = typename container_type::size_type;
		using difference_type = typename container_type::difference_type;
		using key_equal		  = string_compare_helper;
		using reference		  = typename container_type::reference;
		using const_reference = typename container_type::const_reference;
		using pointer		  = typename container_type::pointer;
		using const_pointer	  = typename container_type::const_pointer;
		using iterator		  = typename container_type::iterator;
		using const_iterator  = typename container_type::const_iterator;

		jsonifier_constexpr unordered_map() = default;

		jsonifier_constexpr unordered_map(container_type items) : container_type{ items }, tables_type{ makePmhTables<storageSize>(items, get_key{}, default_prg_t{}) } {
		}

		jsonifier_constexpr unordered_map(std::initializer_list<value_type> items) : unordered_map{ container_type{ items } } {
			constexpr_assert(items.size() == n, "Inconsistent initializer_list size and type size argument.");
		}

		jsonifier_constexpr const_iterator begin() const {
			return container_type::begin();
		}

		jsonifier_constexpr const_iterator end() const {
			return container_type::end();
		}

		template<typename key_type_new> jsonifier_constexpr const_iterator find(const key_type_new& keyNew) const {
			auto& kv = lookup(keyNew);
			if (keyEq()(kv.first, keyNew)) {
				return &kv;
			} else {
				return end();
			}
		}

	  protected:
		template<typename key_type_new> jsonifier_constexpr const auto& lookup(const key_type_new& keyNew) const {
			return container_type::operator[](tables_type::lookup(keyNew));
		}

		jsonifier_constexpr const key_equal& keyEq() const {
			return *this;
		}
	};

	template<typename value_type, typename u, uint64_t n> constexpr auto makeUnorderedMap(pair<value_type, u> const (&items)[n]) {
		return unordered_map<value_type, u, n>{ items };
	}

	template<typename value_type, uint64_t I> struct core_sv {
		static constexpr jsonifier::string_view value = tuplet::get<0>(tuplet::get<I>(jsonifier::concepts::coreV<value_type>));
	};

	template<typename value_type, size_t... I> constexpr auto makeMapImpl(std::index_sequence<I...>) {
		using value_t		= value_tuple_variant_t<jsonifier::concepts::core_t<value_type>>;
		constexpr auto n	= std::tuple_size_v<jsonifier::concepts::core_t<value_type>>;
		constexpr auto size = sizeof...(I);
		static_assert(size == n);
		auto naiveOrNormalHash = [&] {
			if constexpr (size <= 20) {
				return makeNaiveMap<value_t, n>(
					{ pair<jsonifier::string_view, value_t>(jsonifier::string_view(tuplet::get<0>(tuplet::get<I>(jsonifier::concepts::coreV<value_type>))),
						tuplet::get<1>(tuplet::get<I>(jsonifier::concepts::coreV<value_type>)))... });
			} else {
				return makeUnorderedMap<jsonifier::string_view, value_t, n>(
					{ pair<jsonifier::string_view, value_t>(jsonifier::string_view(tuplet::get<0>(tuplet::get<I>(jsonifier::concepts::coreV<value_type>))),
						tuplet::get<1>(tuplet::get<I>(jsonifier::concepts::coreV<value_type>)))... });
			}
		};

		constexpr bool n128 = n < 128;
		if constexpr (n == 0) {
			static_assert(falseV<value_type>, "Empty object in jsonifier::core.");
		} else if constexpr (n == 1) {
			return single_item<value_t, core_sv<value_type, I>::value...>{ ctime_array<pair<jsonifier::string_view, value_t>, n>{
				pair<jsonifier::string_view, value_t>(jsonifier::string_view(tuplet::get<0>(tuplet::get<I>(jsonifier::concepts::coreV<value_type>))),
					tuplet::get<1>(tuplet::get<I>(jsonifier::concepts::coreV<value_type>)))... } };
		} else if constexpr (n == 2) {
			return double_item<value_t, core_sv<value_type, I>::value...>{ ctime_array<pair<jsonifier::string_view, value_t>, n>{
				pair<jsonifier::string_view, value_t>(jsonifier::string_view(tuplet::get<0>(tuplet::get<I>(jsonifier::concepts::coreV<value_type>))),
					tuplet::get<1>(tuplet::get<I>(jsonifier::concepts::coreV<value_type>)))... } };
		} else if constexpr (n128) {
			constexpr auto frontDesc =
				singleCharHash<n>(ctime_array<jsonifier::string_view, n>{ jsonifier::string_view{ tuplet::get<0>(tuplet::get<I>(jsonifier::concepts::coreV<value_type>)) }... });

			if constexpr (frontDesc.valid) {
				return makeSingleCharMap<value_t, frontDesc>(
					{ pair<jsonifier::string_view, value_t>(jsonifier::string_view(tuplet::get<0>(tuplet::get<I>(jsonifier::concepts::coreV<value_type>))),
						tuplet::get<1>(tuplet::get<I>(jsonifier::concepts::coreV<value_type>)))... });
			} else {
				constexpr auto backDesc = singleCharHash<n, false>(
					ctime_array<jsonifier::string_view, n>{ jsonifier::string_view{ tuplet::get<0>(tuplet::get<I>(jsonifier::concepts::coreV<value_type>)) }... });

				if constexpr (backDesc.valid) {
					return makeSingleCharMap<value_t, backDesc>(
						{ pair<jsonifier::string_view, value_t>(jsonifier::string_view(tuplet::get<0>(tuplet::get<I>(jsonifier::concepts::coreV<value_type>))),
							tuplet::get<1>(tuplet::get<I>(jsonifier::concepts::coreV<value_type>)))... });
				} else {
					return naiveOrNormalHash();
				}
			}
		} else {
			return naiveOrNormalHash();
		}
	}

	template<typename value_type> constexpr auto makeMap() {
		constexpr auto indices = std::make_index_sequence<std::tuple_size_v<jsonifier::concepts::core_t<value_type>>>{};
		return makeMapImpl<std::unwrap_ref_decay_t<value_type>>(indices);
	}
}