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
#include <jsonifier/Base.hpp>
#include <algorithm>
#include <numeric>
#include <span>

namespace jsonifier_internal {

	struct naive_prng final {
		uint64_t x = 7185499250578500046;
		constexpr uint64_t operator()() noexcept {
			x ^= x >> 12;
			x ^= x << 25;
			x ^= x >> 27;
			return x * 0x2545F4914F6CDD1DULL;
		}
	};

	template<uint64_t length> struct set_simd {
		using type =
			std::conditional_t<(length >= 64) && (BytesPerStep == 64), simd_int_512, std::conditional_t<(length >= 32) && (BytesPerStep == 32), simd_int_256, simd_int_128>>;
	};

	template<uint64_t length> struct set_integer {
		using type = std::conditional_t<(length >= 64) && (BytesPerStep == 64), uint64_t, std::conditional_t<(length >= 32) && (BytesPerStep == 32), uint32_t, uint16_t>>;
	};

	template<uint64_t length> using set_simd_t = set_simd<length>::type;

	template<uint64_t length> using set_integer_t = set_integer<length>::type;

	template<uint64_t length> constexpr uint64_t setSimdLength() {
		return (length >= 64) && (BytesPerStep == 64) ? 64 : (length >= 32) && (BytesPerStep == 32) ? 32 : 16;
	};

	template<uint64_t length> struct fit_unsigned {
		using type = std::conditional_t<length <= std::numeric_limits<uint8_t>::max(), uint8_t,
			std::conditional_t<length <= std::numeric_limits<uint16_t>::max(), uint16_t,
				std::conditional_t<length <= std::numeric_limits<uint32_t>::max(), uint32_t, std::conditional_t<length <= std::numeric_limits<uint64_t>::max(), uint64_t, void>>>>;
	};

	template<uint64_t length> using fit_unsigned_t = typename fit_unsigned<length>::type;

	template<typename value_type> constexpr value_type abs(value_type value) {
		return value < 0 ? -value : value;
	}

	template<uint64_t value01> constexpr bool isItCloseEnough() {
		return abs(value01 % setSimdLength<value01>()) <= 1;
	}

	template<typename key_type, typename value_type, size_t N> struct simd_map : public jsonifier_internal::fnv1a_hash {
		static constexpr uint64_t storageSize = roundUpToMultiple<setSimdLength<N>(), uint64_t>(N);
		static constexpr uint64_t bucketSize  = ((N >= setSimdLength<N>()) ? setSimdLength<N>() : N);
		static constexpr uint64_t numGroups	  = storageSize > bucketSize ? storageSize / bucketSize : 1;
		uint64_t seed{};
		using hasher	   = jsonifier_internal::fnv1a_hash;
		using simd_type	   = set_simd_t<storageSize>;
		using integer_type = set_integer_t<storageSize>;
		JSONIFIER_ALIGN std::pair<key_type, value_type> items[storageSize]{};
		JSONIFIER_ALIGN uint8_t controlBytes[storageSize]{};
		JSONIFIER_ALIGN uint64_t hashes[storageSize]{};

		constexpr auto begin() const noexcept {
			return items;
		}

		constexpr auto end() const noexcept {
			return items + storageSize;
		}

		constexpr auto size() const noexcept {
			return N;
		}

		template<typename key_type_new> constexpr auto find(key_type_new&& key) const noexcept {
			const auto hash		   = hasher::operator()(key.data(), key.size(), seed);
			const auto hashNew	   = hash >> 7;
			const size_t groupPos  = hashNew % numGroups;
			const auto resultIndex = match(controlBytes + groupPos * bucketSize, static_cast<uint8_t>(hash));

			return (hashes[groupPos * bucketSize + resultIndex] >> 7) == hashNew ? items + groupPos * bucketSize + resultIndex : end();
		}

		constexpr simd_map(const std::array<std::pair<key_type, value_type>, N>& pairs) : items{}, hashes{} {
			if constexpr (N == 0) {
				return;
			}

			size_t bucketSizes[numGroups]{};
			seed = 1;
			bool failed{};

			do {
				std::fill_n(items, storageSize, std::pair<key_type, value_type>{});
				std::fill_n(controlBytes, storageSize, 0);
				std::fill_n(bucketSizes, numGroups, 0);
				std::fill_n(hashes, storageSize, 0);

				failed = false;
				for (size_t i = 0; i < N; ++i) {
					const auto hash			 = hasher::operator()(pairs[i].first.data(), pairs[i].first.size(), seed);
					const auto groupPos		 = (hash >> 7) % numGroups;
					const auto bucketSizeNew = bucketSizes[groupPos]++;
					const auto ctrlByte		 = static_cast<uint8_t>(hash);

					if (bucketSizeNew >= bucketSize || doesItContainIt(controlBytes + groupPos * bucketSize, ctrlByte)) {
						failed				  = true;
						bucketSizes[groupPos] = 0;
						++seed;
						break;
					}
					controlBytes[groupPos * bucketSize + bucketSizeNew] = ctrlByte;
					hashes[groupPos * bucketSize + bucketSizeNew]		= hash;
					items[groupPos * bucketSize + bucketSizeNew]		= pairs[i];
				}
			} while (failed);
		}

	  protected:
		constexpr bool doesItContainIt(const uint8_t* hashData, uint8_t byteToCheckFor) const {
			for (uint64_t x = 0; x < bucketSize; ++x) {
				if (hashData[x] == byteToCheckFor) {
					return true;
				}
			}
			return false;
		}

		constexpr uint8_t tzcnt(integer_type value) const {
			uint8_t count{};
			while ((value & 1) == 0 && value != 0) {
				value >>= 1;
				++count;
			}
			return count;
		}

		constexpr uint8_t constMatch(const uint8_t* hashData, uint8_t hash) const {
			uint32_t mask = 0;
			for (int32_t i = 0; i < bucketSize; ++i) {
				if (hashData[i] == hash) {
					mask |= (1 << i);
				}
			}
			return tzcnt(mask);
		}

		constexpr uint8_t nonConstMatch(const uint8_t* hashData, uint8_t hash) const {
			return simd_internal::tzcnt(simd_internal::opCmpEq(simd_internal::gatherValue<simd_type>(hash), simd_internal::gatherValues<simd_type>(hashData)));
		}

		constexpr uint8_t match(const uint8_t* hashData, uint8_t hash) const {
			return std::is_constant_evaluated() ? constMatch(hashData, hash) : nonConstMatch(hashData, hash);
		}
	};

	template<const jsonifier::string_view& lhs> inline constexpr bool compareSv(const jsonifier::string_view rhs) noexcept {
		constexpr auto N = lhs.size();
		return (N == rhs.size()) && compare<N>(lhs.data(), rhs.data());
	}

	inline constexpr bool compareSv(const jsonifier::string_view lhs, const jsonifier::string_view rhs) noexcept {
		if (std::is_constant_evaluated()) {
			return lhs == rhs;
		} else {
			return (lhs.size() == rhs.size()) && compare(lhs.data(), rhs.data(), lhs.size());
		}
	}

	constexpr size_t naiveMapMaxSize = 32;

	struct naive_map_desc {
		size_t N{};
		uint64_t seed{};
		size_t bucketSize{};
		size_t minLength = (std::numeric_limits<size_t>::max)();
		size_t maxLength{};
	};

	constexpr uint64_t toUint64NBelow8(const char* bytes, const size_t N) noexcept {
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
					break;
				}
			}
		}
		return res;
	}

	template<size_t N = 8> constexpr uint64_t toUint64(const char* bytes) noexcept {
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
			constexpr auto numBytes = sizeof(uint64_t);
			constexpr auto shift	= (uint64_t(numBytes - N) << 3);
			if constexpr (shift == 0) {
				return res;
			} else {
				return (res << shift) >> shift;
			}
		}
	}

	struct naive_hash {
		inline constexpr uint64_t bitmix(uint64_t h) const noexcept {
			h ^= (h >> 33);
			h *= 0xff51afd7ed558ccdL;
			h ^= (h >> 33);
			h *= 0xc4ceb9fe1a85ec53L;
			h ^= (h >> 33);
			return h;
		};

		constexpr uint64_t operator()(std::integral auto value, const uint64_t seed) const noexcept {
			return bitmix(uint64_t(value) ^ seed);
		}

		template<uint64_t seed> constexpr uint64_t operator()(std::integral auto value) const noexcept {
			return bitmix(uint64_t(value) ^ seed);
		}

		constexpr uint64_t operator()(const jsonifier::string_view value, const uint64_t seed) const noexcept {
			uint64_t h		 = (0xcbf29ce484222325 ^ seed) * 1099511628211;
			const auto n	 = value.size();
			const char* data = value.data();

			if (n < 8) {
				return bitmix(h ^ toUint64NBelow8(data, n));
			}

			const char* end7 = data + n - 7;
			for (auto d0 = data; d0 < end7; d0 += 8) {
				h = bitmix(h ^ toUint64(d0));
			}
			return bitmix(h ^ toUint64(data + n - 8));
		}

		template<naive_map_desc D> constexpr uint64_t operator()(const jsonifier::string_view value) const noexcept {
			constexpr auto hInit = (0xcbf29ce484222325 ^ D.seed) * 1099511628211;
			if constexpr (D.maxLength < 8) {
				const auto n = value.size();
				if (n > 7) {
					return D.seed;
				}
				return bitmix(hInit ^ toUint64NBelow8(value.data(), n));
			} else if constexpr (D.minLength > 7) {
				const auto n = value.size();

				if (n < 8) {
					return D.seed;
				}

				uint64_t h		 = hInit;
				const char* data = value.data();
				const char* end7 = data + n - 7;
				for (auto d0 = data; d0 < end7; d0 += 8) {
					h = bitmix(h ^ toUint64(d0));
				}
				return bitmix(h ^ toUint64(data + n - 8));
			} else {
				uint64_t h		 = hInit;
				const auto n	 = value.size();
				const char* data = value.data();

				if (n < 8) {
					return bitmix(h ^ toUint64NBelow8(data, n));
				}

				const char* end7 = data + n - 7;
				for (auto d0 = data; d0 < end7; d0 += 8) {
					h = bitmix(h ^ toUint64(d0));
				}
				return bitmix(h ^ toUint64(data + n - 8));
			}
		}
	};

	constexpr bool contains(auto&& data, auto&& val) noexcept {
		const auto n = data.size();
		for (size_t i = 0; i < n; ++i) {
			if (data[i] == val) {
				return true;
			}
		}
		return false;
	}

	template<size_t N> constexpr naive_map_desc makeMapHash(const std::array<jsonifier::string_view, N>& v) noexcept {
		constexpr auto invalid = (std::numeric_limits<uint64_t>::max)();

		naive_map_desc desc{ N };
		desc.bucketSize = (N == 1) ? 1 : std::bit_ceil(N * N) / 2;
		auto& seed		= desc.seed;

		for (size_t i = 0; i < N; ++i) {
			const auto n = v[i].size();
			if (n < desc.minLength) {
				desc.minLength = n;
			}
			if (n > desc.maxLength) {
				desc.maxLength = n;
			}
		}

		auto naivePerfectHash = [&] {
			std::array<size_t, N> bucket_index{};

			naive_prng gen{};
			for (size_t i = 0; i < 1024; ++i) {
				seed		 = gen();
				size_t index = 0;
				for (const auto& key: v) {
					const auto hash = naive_hash{}(key, seed);
					if (hash == seed) {
						break;
					}
					const auto bucket = hash % desc.bucketSize;
					if (contains(std::span{ bucket_index.data(), index }, bucket)) {
						break;
					}
					bucket_index[index] = bucket;
					++index;
				}

				if (index == N) {
					const auto bucket = seed % desc.bucketSize;
					if (not contains(std::span{ bucket_index.data(), N }, bucket)) {
						return;
					}
				}
			}

			seed = invalid;
		};

		naivePerfectHash();
		if (seed == invalid) {
			std::abort();
			return {};
		}

		return desc;
	}

	template<typename value_type, naive_map_desc D> struct naive_map : public naive_hash {
		static constexpr auto N = D.N;
		using hash_alg			= naive_hash;
		std::array<std::pair<jsonifier::string_view, value_type>, N> items{};
		std::array<uint8_t, D.bucketSize> controlBytes{};
		std::array<uint64_t, N> hashes{};

		constexpr naive_map(const std::array<std::pair<jsonifier::string_view, value_type>, D.N>& pairs) : items{ pairs } {};

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
			const auto hash	 = hash_alg::operator()<D>(key);
			const auto index = controlBytes[hash % D.bucketSize];
			if (hashes[index] != hash) [[unlikely]]
				return items.end();
			return items.begin() + index;
		}
	};

	template<typename value_type, naive_map_desc D> constexpr auto makeNaiveMap(const std::array<std::pair<jsonifier::string_view, value_type>, D.N>& pairs) {
		naive_map<value_type, D> ht{ pairs };

		using hash_alg = naive_hash;

		for (size_t i = 0; i < D.N; ++i) {
			const auto hash						 = hash_alg{}.template operator()<D>(pairs[i].first);
			ht.hashes[i]						 = hash;
			ht.controlBytes[hash % D.bucketSize] = uint8_t(i);
		}

		return ht;
	}

	template<const jsonifier::string_view& S, bool CheckSize = true> inline constexpr bool cxStringCmp(const jsonifier::string_view key) noexcept {
		if (std::is_constant_evaluated()) {
			return key == S;
		} else {
			if constexpr (CheckSize) {
				return compareSv<S>(key);
			} else {
				return compare<S.size()>(key.data(), S.data());
			}
		}
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

	template<typename value_type, const jsonifier::string_view& S0, const jsonifier::string_view& S1> struct micro_map2 {
		std::array<std::pair<jsonifier::string_view, value_type>, 2> items{};

		static constexpr bool same_size	 = S0.size() == S1.size();
		static constexpr bool check_size = !same_size;

		constexpr decltype(auto) begin() const noexcept {
			return items.begin();
		}
		constexpr decltype(auto) end() const noexcept {
			return items.end();
		}

		constexpr decltype(auto) find(auto&& key) const noexcept {
			if constexpr (same_size) {
				constexpr auto n = S0.size();
				if (key.size() != n) {
					return items.end();
				}
			}

			if (cxStringCmp<S0, check_size>(key)) {
				return items.begin();
			} else if (cxStringCmp<S1, check_size>(key)) {
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
		constexpr auto& first = std::get<0>(std::get<I>(jsonifier::concepts::core_v<value_type>));
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
		constexpr auto& first = std::get<0>(std::get<I>(jsonifier::concepts::core_v<value_type>));
		using T0			  = std::decay_t<decltype(first)>;
		if constexpr (std::is_member_pointer_v<T0>) {
			return std::pair<jsonifier::string_view, value_t>{ getName<first>(), first };
		} else {
			return std::pair<jsonifier::string_view, value_t>{ jsonifier::string_view(first), std::get<1>(std::get<I>(jsonifier::concepts::core_v<value_type>)) };
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
		} else if constexpr (isItCloseEnough<n>()) {
			return simd_map<jsonifier::string_view, value_t, n>({ keyValue<value_type, I>()... });
		} else {
			constexpr std::array<jsonifier::string_view, n> keys{ getKey<value_type, I>()... };

			constexpr auto naive_desc = makeMapHash<n>(keys);
			return makeNaiveMap<value_t, naive_desc>(std::array{ keyValue<value_type, I>()... });
		}
	}

	template<typename value_type> constexpr auto makeMap() {
		constexpr auto indices = std::make_index_sequence<std::tuple_size_v<jsonifier::concepts::core_t<value_type>>>{};
		return makeMapImpl<jsonifier::concepts::decay_keep_volatile_t<value_type>>(indices);
	}
}