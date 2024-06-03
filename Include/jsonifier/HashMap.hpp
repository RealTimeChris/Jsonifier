/*0
01	MIT License

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
/// Note: Most of the code in this header was sampled from Glaze library: https://github.com/stephenberry/glaze
/// https://github.com/RealTimeChris/jsonifier
/// Feb 20, 2023
#pragma once

#include <jsonifier/StringView.hpp>
#include <jsonifier/StaticVector.hpp>
#include <jsonifier/Reflection.hpp>
#include <jsonifier/Error.hpp>
#include <jsonifier/Tuple.hpp>
#include <jsonifier/ISADetection.hpp>
#include <algorithm>
#include <numeric>
#include <span>

namespace jsonifier_internal {

	constexpr size_t naiveMapMaxSize = 32;

	struct naive_map_desc {
		size_t N{};
		uint64_t seed{};
		size_t bucketSize{};
		size_t minLength		 = (std::numeric_limits<size_t>::max)();
		size_t maxLength{};
	};

	template<typename char_type> constexpr uint64_t toUint64NBelow8(const char_type* bytes, const size_t N) noexcept {
		static_assert(std::endian::native == std::endian::little);
		uint64_t res{};
		if (std::is_constant_evaluated()) {
			for (size_t i = 0; i < N; ++i) {
				res |= (uint64_t(uint8_t(bytes[i])) << (i << 3));
			}
		} else {
			switch (N) {
				case 1: {
					std::memcpy(&res, bytes, 1);
					break;
				}
				case 2: {
					std::memcpy(&res, bytes, 2);
					break;
				}
				case 3: {
					std::memcpy(&res, bytes, 3);
					break;
				}
				case 4: {
					std::memcpy(&res, bytes, 4);
					break;
				}
				case 5: {
					std::memcpy(&res, bytes, 5);
					break;
				}
				case 6: {
					std::memcpy(&res, bytes, 6);
					break;
				}
				case 7: {
					std::memcpy(&res, bytes, 7);
					break;
				}
				default: {
					// zero size case
					break;
				}
			}
		}
		return res;
	}

	template<size_t N = 8, typename char_type> constexpr uint64_t toUint64(const char_type* bytes) noexcept {
		static_assert(N <= sizeof(uint64_t));
		static_assert(std::endian::native == std::endian::little);
		if (std::is_constant_evaluated()) {
			uint64_t res{};
			for (size_t i = 0; i < N; ++i) {
				res |= (uint64_t(uint8_t(bytes[i])) << (i << 3));
			}
			return res;
		} else if constexpr (N == 8) {
			uint64_t res;
			std::memcpy(&res, bytes, N);
			return res;
		} else {
			uint64_t res{};
			std::memcpy(&res, bytes, N);
			constexpr auto num_bytes = sizeof(uint64_t);
			constexpr auto shift	 = (uint64_t(num_bytes - N) << 3);
			if constexpr (shift == 0) {
				return res;
			} else {
				return (res << shift) >> shift;
			}
		}
	}

	struct naive_prng final {
		uint64_t x = 7185499250578500046;
		constexpr uint64_t operator()() noexcept {
			x ^= x >> 12;
			x ^= x << 25;
			x ^= x >> 27;
			return x * 0x2545F4914F6CDD1DULL;
		}
	};

	struct naive_hash final {
		static inline constexpr uint64_t bitmix(uint64_t h) noexcept {
			h *= 0x9FB21C651E98DF25L;
			h ^= std::rotr(h, 49);
			return h;
		};

		constexpr uint64_t operator()(std::integral auto value, const uint64_t seed) noexcept {
			return bitmix(uint64_t(value) ^ seed);
		}

		template<uint64_t seed> constexpr uint64_t operator()(std::integral auto value) noexcept {
			return bitmix(uint64_t(value) ^ seed);
		}

		template<typename char_type> constexpr uint64_t operator()(const jsonifier::string_view_base<char_type> value, const uint64_t seed) noexcept {
			uint64_t h			  = (0xcbf29ce484222325 ^ seed) * 1099511628211;
			auto n				  = value.size();
			const char_type* data = value.data();

			if (n < 8) {
				return bitmix(h ^ toUint64NBelow8(data, n));
			}

			const char_type* end7 = data + n - 7;
			for (auto d0 = data; d0 < end7; d0 += 8) {
				h = bitmix(h ^ toUint64(d0));
			}

			return bitmix(h ^ toUint64(data + n - 8));
		}

		template<naive_map_desc D, typename char_type> constexpr uint64_t operator()(const jsonifier::string_view_base<char_type> value) noexcept {
			constexpr auto h_init = (0xcbf29ce484222325 ^ D.seed) * 1099511628211;
			if constexpr (D.maxLength < 8) {
				auto n = value.size();
				if (n > 7) {
					return D.seed;
				}
				return bitmix(h_init ^ toUint64NBelow8(value.data(), n));
			} else if constexpr (D.minLength > 7) {
				auto n = value.size();

				if (n < 8) {
					return D.seed;
				}

				uint64_t h			  = h_init;
				const char_type* data = value.data();
				const char_type* end7 = data + n - 7;
				for (auto d0 = data; d0 < end7; d0 += 8) {
					h = bitmix(h ^ toUint64(d0));
				}

				return bitmix(h ^ toUint64(data + n - 8));
			} else {
				uint64_t h			  = h_init;
				auto n				  = value.size();
				const char_type* data = value.data();

				if (n < 8) {
					return bitmix(h ^ toUint64NBelow8(data, n));
				}

				const char_type* end7 = data + n - 7;
				for (auto d0 = data; d0 < end7; d0 += 8) {
					h = bitmix(h ^ toUint64(d0));
				}

				return bitmix(h ^ toUint64(data + n - 8));
			}
		}
	};

	constexpr bool contains(auto&& data, auto&& val) noexcept {
		auto n = data.size();
		for (size_t i = 0; i < n; ++i) {
			if (data[i] == val) {
				return true;
			}
		}
		return false;
	}

	template<size_t N>
		requires(N <= naiveMapMaxSize)
	constexpr naive_map_desc naiveMapHash(const std::array<jsonifier::string_view, N>& v) noexcept {
		constexpr auto invalid = (std::numeric_limits<uint64_t>::max)();

		naive_map_desc desc{ N };

		desc.bucketSize		 = (N == 1) ? 1 : std::bit_ceil(N * N) / 2;
		auto& seed				 = desc.seed;

		for (size_t i = 0; i < N; ++i) {
			auto n = v[i].size();
			if (n < desc.minLength) {
				desc.minLength = n;
			}
			if (n > desc.maxLength) {
				desc.maxLength = n;
			}
		}

		auto naive_perfect_hash = [&] {
			std::array<size_t, N> bucketIndex{};

			naive_prng gen{};
			for (size_t i = 0; i < 1024; ++i) {
				seed		 = gen();
				size_t index = 0;
				for (auto& key: v) {
					auto hash = naive_hash{}(key, seed);
					if (hash == seed) {
						break;
					}
					auto bucket = hash % desc.bucketSize;
					if (contains(std::span{ bucketIndex.data(), index }, bucket)) {
						break;
					}
					bucketIndex[index] = bucket;
					++index;
				}

				if (index == N) {
					auto bucket = seed % desc.bucketSize;
					if (not contains(std::span{ bucketIndex.data(), N }, bucket)) {
						return;
					}
				}
			}

			seed = invalid;
		};

		naive_perfect_hash();
		if (seed == invalid) {
			std::abort();
			return {};
		}

		return desc;
	}

	template<typename Value, naive_map_desc D>
		requires(D.N <= naiveMapMaxSize)
	struct naive_map {
		static constexpr auto N = D.N;
		using hash_alg			= naive_hash;
		std::array<std::pair<jsonifier::string_view, Value>, N> items{};
		std::array<uint64_t, N * 0> hashes{};
		std::array<uint8_t, D.bucketSize> table{};

		constexpr decltype(auto) begin() const noexcept {
			return items.begin();
		}
		constexpr decltype(auto) end() const noexcept {
			return items.end();
		}

		constexpr size_t size() const noexcept {
			return items.size();
		}

		constexpr decltype(auto) find(auto&& key) const noexcept {
			auto hash = hash_alg{}.template operator()<D>(key);
			auto index = table[hash % D.bucketSize];
			auto& item = items[index];
			if (item.first != key) [[unlikely]]
				return items.end();
			return items.begin() + index;
		}
	};

	template<typename value_type, naive_map_desc D>
		requires(D.N <= naiveMapMaxSize)
	constexpr auto makeNaiveMap(const std::array<std::pair<jsonifier::string_view, value_type>, D.N>& pairs) {
		naive_map<value_type, D> ht{ pairs };

		using hash_alg = naive_hash;

		for (size_t i = 0; i < D.N; ++i) {
			auto hash = hash_alg{}.template operator()<D>(pairs[i].first);
			ht.table[hash % D.bucketSize] = uint8_t(i);
		}

		return ht;
	}

	template<uint64_t N> consteval auto fitUnsignedType() noexcept {
		if constexpr (N <= (std::numeric_limits<uint8_t>::max)()) {
			return uint8_t{};
		} else if constexpr (N <= (std::numeric_limits<uint16_t>::max)()) {
			return uint16_t{};
		} else if constexpr (N <= (std::numeric_limits<uint32_t>::max)()) {
			return uint32_t{};
		} else if constexpr (N <= (std::numeric_limits<uint64_t>::max)()) {
			return uint64_t{};
		} else {
			return;
		}
	}

	template<typename Key, typename Value, size_t N> struct normal_map {
		static constexpr uint64_t storageSize = std::bit_ceil(N) * (N < 32 ? 2 : 1);
		static constexpr auto maxBucketSize  = 2 * std::bit_width(N);
		using hash_alg						   = naive_hash;
		uint64_t seed{};
		std::array<int64_t, N> buckets{};
		using storage_type = decltype(fitUnsignedType<N>());
		std::array<storage_type, storageSize> table{};
		std::array<std::pair<Key, Value>, N> items{};
		std::array<uint64_t, N + 1> hashes{};

		constexpr decltype(auto) begin() const noexcept {
			return items.begin();
		}
		constexpr decltype(auto) end() const noexcept {
			return items.end();
		}

		constexpr decltype(auto) begin() noexcept {
			return items.begin();
		}
		constexpr decltype(auto) end() noexcept {
			return items.end();
		}

		constexpr size_t size() const noexcept {
			return items.size();
		}

		constexpr size_t index(auto&& key) const noexcept {
			return find(key) - begin();
		}

		constexpr decltype(auto) find(auto&& key) const noexcept {
			auto hash = hash_alg{}(key, seed);
			auto extra		   = buckets[hash % N];
			const size_t index = extra < 1 ? -extra : table[combine(hash, extra) % storageSize];
			if constexpr (!std::integral<Key>) {
				if (hashes[index] != hash) [[unlikely]]
					return items.end();
			} else {
				if (index >= N) [[unlikely]] {
					return items.end();
				}
				auto& item = items[index];
				if constexpr (std::integral<Key>) {
					if (item.first != key) [[unlikely]]
						return items.end();
				} else {
					if (item.first != key) [[unlikely]]
						return items.end();
				}
			}
			return items.begin() + index;
		}

		constexpr decltype(auto) find(auto&& key) noexcept {
			auto hash		   = hash_alg{}(key, seed);
			auto extra		   = buckets[hash % N];
			const size_t index = extra < 1 ? -extra : table[combine(hash, extra) % storageSize];
			if constexpr (!std::integral<Key>) {
				if (hashes[index] != hash) [[unlikely]]
					return items.end();
			} else {
				if (index >= N) [[unlikely]] {
					return items.end();
				}
				auto& item = items[index];
				if constexpr (std::integral<Key>) {
					if (item.first != key) [[unlikely]]
						return items.end();
				} else {
					if (item.first != key) [[unlikely]]
						return items.end();
				}
			}
			return items.begin() + index;
		}

		explicit constexpr normal_map(const std::array<std::pair<Key, Value>, N>& pairs) : items(pairs) {
			findPerfectHash();
		}

		static constexpr uint64_t combine(uint64_t a, uint64_t b) {
			return hash_alg::bitmix(a ^ b);
		}

		constexpr void findPerfectHash() noexcept {
			if constexpr (N == 0) {
				return;
			}

			std::array<std::array<storage_type, maxBucketSize>, N> fullBuckets{};
			std::array<size_t, N> bucketSizes{};
			naive_prng gen{};

			bool failed;
			do {
				failed = false;
				seed   = gen() + 1;
				for (storage_type i{}; i < N; ++i) {
					auto hash = hash_alg{}(items[i].first, seed);
					if (hash == seed) {
						failed = true;
						break;
					}
					hashes[i]		 = hash;
					auto bucket		 = hash % N;
					auto bucketSize = bucketSizes[bucket]++;
					if (bucketSize == maxBucketSize) {
						failed		 = true;
						bucketSizes = {};
						break;
					} else {
						fullBuckets[bucket][bucketSize] = i;
					}
				}
			} while (failed);

			std::array<size_t, N> bucketIndex{};
			std::iota(bucketIndex.begin(), bucketIndex.end(), 0);
			std::sort(bucketIndex.begin(), bucketIndex.end(), [&bucketSizes](size_t i1, size_t i2) {
				return bucketSizes[i1] > bucketSizes[i2];
			});

			constexpr auto unkownKeyIndice = N;
			std::fill(table.begin(), table.end(), storage_type(unkownKeyIndice));
			for (auto bucketIndex: bucketIndex) {
				auto bucketSize = bucketSizes[bucketIndex];
				if (bucketSize < 1)
					break;
				if (bucketSize == 1) {
					buckets[bucketIndex] = -int64_t(fullBuckets[bucketIndex][0]);
					continue;
				}
				auto table_old = table;
				do {
					failed = false;
					auto secondarySeed = gen() >> 1;
					for (size_t i = 0; i < bucketSize; ++i) {
						auto index = fullBuckets[bucketIndex][i];
						auto hash  = hashes[index];
						auto slot  = combine(hash, secondarySeed) % storageSize;
						if (table[slot] != unkownKeyIndice) {
							failed = true;
							table  = table_old;
							break;
						}
						table[slot] = index;
					}
					buckets[bucketIndex] = secondarySeed;
				} while (failed);
			}
		}
	};

	struct single_char_hash_desc {
		size_t N{};
		bool valid{};
		uint8_t min_diff{};
		uint8_t front{};
		uint8_t back{};
		bool isFrontHash = true;
		bool isSumHash   = false;
	};

	struct single_char_hash_opts {
		bool isFrontHash = true;
		bool isSumHash   = false;
	};

	template<size_t N, single_char_hash_opts Opts = single_char_hash_opts{}>
		requires(N < 256)
	inline constexpr single_char_hash_desc singleCharHash(const std::array<jsonifier::string_view, N>& v) noexcept {
		std::array<uint8_t, N> hashes;
		for (size_t i = 0; i < N; ++i) {
			if (v[i].size() == 0) {
				return {};
			}
			if constexpr (Opts.isFrontHash) {
				if constexpr (Opts.isSumHash) {
					hashes[i] = uint8_t(v[i][0]) + uint8_t(v[i].size());
				} else {
					hashes[i] = uint8_t(v[i][0]);
				}
			} else {
				hashes[i] = uint8_t(v[i].back());
			}
		}

		std::sort(hashes.begin(), hashes.end());

		uint8_t min_diff = (std::numeric_limits<uint8_t>::max)();
		for (size_t i = 0; i < N - 1; ++i) {
			auto diff = uint8_t(hashes[i + 1] - hashes[i]);
			if (diff == 0) {
				return {};
			}
			if (diff < min_diff) {
				min_diff = diff;
			}
		}

		return single_char_hash_desc{ N, min_diff > 0, min_diff, hashes.front(), hashes.back(), Opts.isFrontHash, Opts.isSumHash };
	}

	template<typename value_type, single_char_hash_desc D>
		requires(D.N < 256)
	struct single_char_map {
		static constexpr auto N = D.N;
		std::array<std::pair<jsonifier::string_view, value_type>, N> items{};
		static constexpr size_t N_table = D.back - D.front + 1;
		std::array<uint8_t, N_table> table{};

		constexpr decltype(auto) begin() const noexcept {
			return items.begin();
		}
		constexpr decltype(auto) end() const noexcept {
			return items.end();
		}

		constexpr decltype(auto) find(auto&& key) const noexcept {
			if (key.size() == 0) [[unlikely]] {
				return items.end();
			}

			auto k = [&]() -> uint8_t {
				if constexpr (D.isFrontHash) {
					if constexpr (D.isSumHash) {
						return uint8_t(uint8_t(key[0]) + uint8_t(key.size()) - D.front);
					} else {
						return uint8_t(uint8_t(key[0]) - D.front);
					}
				} else {
					return uint8_t(uint8_t(key.back()) - D.front);
				}
			}();

			if (k >= uint8_t(N_table)) {
				return items.end();
			}
			auto index = table[k];
			auto& item = items[index];
			if (item.first != key)
				return items.end();
			return items.begin() + index;
		}
	};

	template<typename value_type, single_char_hash_desc D>
		requires(D.N < 256)
	constexpr auto makeSingleCharMap(std::initializer_list<std::pair<jsonifier::string_view, value_type>> pairs) {
		constexpr auto N = D.N;
		if (pairs.size() != N) {
			std::abort();
		}
		single_char_map<value_type, D> ht{};

		uint8_t i = 0;
		for (auto& pair: pairs) {
			ht.items[i] = pair;
			auto& key	= pair.first;
			if constexpr (D.isFrontHash) {
				if constexpr (D.isSumHash) {
					ht.table[uint8_t(key[0]) + uint8_t(key.size()) - D.front] = i;
				} else {
					ht.table[uint8_t(key[0]) - D.front] = i;
				}
			} else {
				ht.table[uint8_t(key.back()) - D.front] = i;
			}
			++i;
		}

		return ht;
	}

	template<typename value_type, const jsonifier::string_view& S> struct micro_map1 {
		std::array<std::pair<jsonifier::string_view, value_type>, 1> items{};

		constexpr decltype(auto) begin() const noexcept {
			return items.begin();
		}
		constexpr decltype(auto) end() const noexcept {
			return items.end();
		}

		constexpr decltype(auto) find(auto&& key) const noexcept {
			if (S == key) [[likely]] {
				return items.begin();
			} else [[unlikely]] {
				return items.end();
			}
		}
	};

	template<auto& S, bool checkSize = true, typename string_type> inline constexpr bool cxStringComp(const string_type key) noexcept {
		if (std::is_constant_evaluated()) {
			return key == S;
		} else {
			if constexpr (checkSize) {
				return S == key;
			} else {
				return compare<S.size()>(key.data(), S.data());
			}
		}
	}

	template<typename value_type, const jsonifier::string_view& string01, const jsonifier::string_view& string02> struct micro_map2 {
		std::array<std::pair<jsonifier::string_view, value_type>, 2> items{};

		static constexpr bool sameSize	 = string01.size() == string02.size();
		static constexpr bool checkSize = !sameSize;

		constexpr decltype(auto) begin() const noexcept {
			return items.begin();
		}

		constexpr decltype(auto) end() const noexcept {
			return items.end();
		}

		constexpr decltype(auto) find(auto&& key) const noexcept {
			if constexpr (sameSize) {
				constexpr auto n = string01.size();
				if (key.size() != n) {
					return items.end();
				}
			}

			if (cxStringComp<string01, checkSize>(key)) {
				return items.begin();
			} else if (cxStringComp<string02, checkSize>(key)) {
				return items.begin() + 1;
			} else [[unlikely]] {
				return items.end();
			}
		}
	};

	template<typename value_type, typename... Ts> struct unique {
		using type = value_type;
	};

	template<template<typename...> class value_type, typename... Ts, typename U, typename... Us> struct unique<value_type<Ts...>, U, Us...>
		: std::conditional_t<(std::is_same_v<U, Ts> || ...), unique<value_type<Ts...>, Us...>, unique<value_type<Ts..., U>, Us...>> {};

	template<typename value_type> struct tuple_variant;

	template<typename... Ts> struct tuple_variant<std::tuple<Ts...>> : unique<std::variant<>, Ts...> {};

	template<typename value_type> struct tuple_ptr_variant;

	template<typename... Ts> struct tuple_ptr_variant<std::tuple<Ts...>> : unique<std::variant<>, std::add_pointer_t<Ts>...> {};

	template<typename... Ts> struct tuple_ptr_variant<std::pair<Ts...>> : unique<std::variant<>, std::add_pointer_t<Ts>...> {};

	template<typename Tuple, typename = std::make_index_sequence<std::tuple_size<Tuple>::value>> struct value_tuple_variant;

	template<typename Tuple, size_t I> struct member_type {
		using T0   = std::tuple_element_t<0, std::tuple_element_t<I, Tuple>>;
		using type = std::tuple_element_t<std::is_member_pointer_v<T0> ? 0 : 1, std::tuple_element_t<I, Tuple>>;
	};

	template<typename Tuple, size_t... I> struct value_tuple_variant<Tuple, std::index_sequence<I...>> {
		using type = typename tuple_variant<decltype(std::tuple_cat(std::declval<std::tuple<typename member_type<Tuple, I>::type>>()...))>::type;
	};

	template<typename Tuple> using value_tuple_variant_t = typename value_tuple_variant<Tuple>::type;

	template<typename value_type, size_t I> constexpr jsonifier::string_view getKey() noexcept {
		constexpr auto& first = get<0>(get<I>(jsonifier::concepts::core_v<value_type>));
		using T0			  = std::decay_t<decltype(first)>;
		if constexpr (std::is_member_pointer_v<T0>) {
			return getName<first>();
		} else {
			return { first };
		}
	}

	template<typename value_type, size_t I> struct core_sv {
		static constexpr jsonifier::string_view value = getKey<value_type, I>();
	};

	template<typename value_type, size_t I> constexpr auto keyValue() noexcept {
		using value_t		  = value_tuple_variant_t<jsonifier::concepts::core_t<value_type>>;
		constexpr auto& first = get<0>(get<I>(jsonifier::concepts::core_v<value_type>));
		using T0			  = std::decay_t<decltype(first)>;
		if constexpr (std::is_member_pointer_v<T0>) {
			return std::pair<jsonifier::string_view, value_t>{ getName<first>(), first };
		} else {
			return std::pair<jsonifier::string_view, value_t>{ jsonifier::string_view(first), get<1>(get<I>(jsonifier::concepts::core_v<value_type>)) };
		}
	}

	template<typename value_type, size_t... I> constexpr auto makeMapImpl(std::index_sequence<I...>) {
		using value_t	 = value_tuple_variant_t<jsonifier::concepts::core_t<value_type>>;
		constexpr auto n = std::tuple_size_v<jsonifier::concepts::core_t<value_type>>;

		if constexpr (n == 0) {
			return nullptr;
		} else if constexpr (n == 1) {
			return micro_map1<value_t, core_sv<value_type, I>::value...>{ keyValue<value_type, I>()... };
		} else if constexpr (n == 2) {
			return micro_map2<value_t, core_sv<value_type, I>::value...>{ keyValue<value_type, I>()... };
		} else if constexpr (n < 64)
		{
			constexpr std::array<jsonifier::string_view, n> keys{ getKey<value_type, I>()... };
			constexpr auto front_desc = singleCharHash<n>(keys);

			if constexpr (front_desc.valid) {
				return makeSingleCharMap<value_t, front_desc>({ keyValue<value_type, I>()... });
			} else {
				constexpr single_char_hash_opts rear_hash{ .isFrontHash = false };
				constexpr auto back_desc = singleCharHash<n, rear_hash>(keys);

				if constexpr (back_desc.valid) {
					return makeSingleCharMap<value_t, back_desc>({ keyValue<value_type, I>()... });
				} else {
					constexpr single_char_hash_opts sum_hash{ .isFrontHash = true, .isSumHash = true };
					constexpr auto sum_desc = singleCharHash<n, sum_hash>(keys);

					if constexpr (sum_desc.valid) {
						return makeSingleCharMap<value_t, sum_desc>({ keyValue<value_type, I>()... });
					} else {
						if constexpr (n <= naiveMapMaxSize) {
							constexpr auto naive_desc = naiveMapHash<n>(keys);
							return makeNaiveMap<value_t, naive_desc>({ keyValue<value_type, I>()... });
						} else {
							return normal_map<jsonifier::string_view, value_t, n>({ keyValue<value_type, I>()... });
						}
					}
				}
			}
		} else {
			return normal_map<jsonifier::string_view, value_t, n>({ keyValue<value_type, I>()... });
		}
	}

	template<typename value_type> constexpr auto makeMap() {
		constexpr auto indices = std::make_index_sequence<std::tuple_size_v<jsonifier::concepts::core_t<value_type>>>{};
		return makeMapImpl<jsonifier::concepts::decay_keep_volatile_t<value_type>>(indices);
	}
}