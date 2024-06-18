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
/// https://github.com/RealTimeChris/jsonifier
/// Feb 20, 2023
#pragma once

#include <jsonifier/StringView.hpp>
#include <jsonifier/StaticVector.hpp>
#include <jsonifier/Reflection.hpp>
#include <jsonifier/Derailleur.hpp>
#include <jsonifier/Error.hpp>
#include <jsonifier/Tuple.hpp>
#include <jsonifier/Config.hpp>
#include <algorithm>
#include <numeric>
#include <span>

namespace jsonifier_internal {

	struct xoshiro256 {
		std::array<uint64_t, 4> state{};

		constexpr xoshiro256() {
			auto x = 7185499250578500046ull >> 12ull;
			x ^= x << 25ull;
			x ^= x >> 27ull;
			uint64_t s = x * 0x2545F4914F6CDD1Dull;
			for (auto& x: state) {
				x = splitmix64(s);
			}
		}

		constexpr uint64_t operator()() {
			const uint64_t result = rotl(state[1] * 5, 7) * 9;

			const uint64_t t = state[1] << 17;

			state[2] ^= state[0];
			state[3] ^= state[1];
			state[1] ^= state[2];
			state[0] ^= state[3];

			state[2] ^= t;

			state[3] = rotl(state[3], 45);

			return result;
		}

	  private:
		constexpr uint64_t rotl(const uint64_t x, int k) const {
			return (x << k) | (x >> (64 - k));
		}

		constexpr uint64_t splitmix64(uint64_t& seed) const {
			uint64_t result = seed += 0x9E3779B97F4A7C15;
			result			= (result ^ (result >> 30)) * 0xBF58476D1CE4E5B9;
			result			= (result ^ (result >> 27)) * 0x94D049BB133111EB;
			return result ^ (result >> 31);
		}
	};

	template<uint64_t length> struct set_simd {
		using type = std::conditional_t<length >= 64, simd_int_512, std::conditional_t<length >= 32, simd_int_256, std::conditional_t<length >= 16, simd_int_128, simd_int_128>>>;
	};

	template<uint64_t length> struct set_integer {
		using type = std::conditional_t<length >= 64, uint64_t, std::conditional_t<length >= 32, uint32_t, std::conditional_t<length >= 16, uint16_t, uint8_t>>>;
	};

	template<uint64_t N> constexpr uint64_t setSimdWidth() {
		return N >= 64 && bytesPerStep >= 64 ? 64 : N >= 32 && bytesPerStep >= 32 ? 32 : N >= 16 && bytesPerStep >= 16 ? 16 : N >= 8 ? 8 : 8;
	};

	template<uint64_t length> using set_simd_t = set_simd<length>::type;

	template<uint64_t length> using set_integer_t = set_integer<length>::type;

	constexpr double scalingFactorTable[]{ 0.10f, 0.20f, 0.30f, 0.40f, 0.50f, 0.60f, 0.70f, 0.80f, 0.90f, 1.0f };

	constexpr uint64_t simdSetMaxSizes[]{ 1, 2, 4, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096 };

	template<typename value_type> constexpr bool contains(const value_type* hashData, value_type byteToCheckFor, uint64_t size) {
		for (uint64_t x = 0; x < size; ++x) {
			if (hashData[x] == byteToCheckFor) {
				return true;
			}
		}
		return false;
	}

	template<uint64_t size, uint64_t N> constexpr uint64_t getMaxSizeIndex(const uint64_t (&maxSizes)[N]) {
		for (uint64_t x = 0; x < std::size(maxSizes); ++x) {
			if (size <= maxSizes[x]) {
				return x;
			}
		}
		return std::size(maxSizes) - 1;
	}

	constexpr std::array<uint64_t, 256> broadcastedHashes{ [] {
		std::array<uint64_t, 256> returnValues{};
		for (uint64_t x = 0; x < 256; ++x) {
			returnValues[x] = 0x0101010101010101ULL * static_cast<uint8_t>(x);
		}
		return returnValues;
	}() };

	template<typename key_type, typename value_type, uint64_t actualCount, size_t storageSizeNew> struct simd_set : public fnv1a_hash {
		static constexpr uint64_t bucketSize  = setSimdWidth<actualCount>();
		static constexpr uint64_t storageSize = storageSizeNew > bucketSize ? storageSizeNew : setSimdWidth<actualCount>();
		static constexpr uint64_t numGroups	  = storageSize > bucketSize ? storageSize / bucketSize : 1;
		using hasher						  = fnv1a_hash;
		using simd_type						  = set_simd_t<bucketSize>;
		using integer_type					  = set_integer_t<bucketSize>;
		JSONIFIER_ALIGN std::array<uint8_t, storageSize> controlBytes{};
		JSONIFIER_ALIGN std::array<value_type, storageSize> items{};
		JSONIFIER_ALIGN std::array<uint64_t, storageSize> hashes{};
		JSONIFIER_ALIGN uint64_t stringScalingFactorIndex{};
		JSONIFIER_ALIGN uint64_t seed{};

		constexpr simd_set() noexcept = default;

		constexpr decltype(auto) begin() const noexcept {
			return items.data();
		}

		constexpr decltype(auto) end() const noexcept {
			return items.data() + storageSize;
		}

		constexpr auto size() const noexcept {
			return actualCount;
		}

		template<typename key_type_new> constexpr decltype(auto) find(key_type_new&& key) const noexcept {
			JSONIFIER_ALIGN const auto hash =
				hasher::operator()(key.data(), static_cast<uint64_t>(static_cast<double>(key.size()) * scalingFactorTable[stringScalingFactorIndex]), seed);
			JSONIFIER_ALIGN const auto resultIndex = ((hash >> 7) % numGroups) * bucketSize;
			if (std::is_constant_evaluated()) {
				JSONIFIER_ALIGN const auto adjustedIndex = constMatch(controlBytes.data() + resultIndex, static_cast<uint8_t>(hash)) % bucketSize + resultIndex;
				return (hashes[adjustedIndex] == hash) ? items.data() + adjustedIndex : end();
			} else {
				JSONIFIER_ALIGN const auto adjustedIndex = nonConstMatch(controlBytes.data() + resultIndex, static_cast<uint8_t>(hash)) % bucketSize + resultIndex;
				return (hashes[adjustedIndex] == hash) ? items.data() + adjustedIndex : end();
			}
		}

	  protected:
		constexpr integer_type tzcnt(integer_type value) const {
			integer_type count{};
			while ((value & 1) == 0 && value != 0) {
				value >>= 1;
				++count;
			}
			return count;
		}

		template<uint64_t nNew = bucketSize> constexpr std::enable_if_t<(nNew >= 16), uint64_t> nonConstMatch(const uint8_t* hashData, uint8_t hash) const {
			return simd_internal::tzcnt(simd_internal::opCmpEq(simd_internal::gatherValue<simd_type>(hash), simd_internal::gatherValues<simd_type>(hashData)));
		}

		template<uint64_t nNew = bucketSize> constexpr std::enable_if_t<(nNew < 16), uint64_t> nonConstMatch(const uint8_t* hashData, uint8_t hash) const {
			uint64_t broadcastedHash = broadcastedHashes[hash];
			uint64_t data{};
			memcpy(&data, hashData, sizeof(data));
			uint64_t comparison				  = (data ^ broadcastedHash);
			constexpr uint64_t comparisonMask = repeatByte<0x80, uint64_t>();
			uint64_t mask					  = ((comparison - 0x0101010101010101ULL) & ~comparison & comparisonMask) >> 7;
			return simd_internal::tzcnt(mask) >> 3;
		}

		constexpr integer_type constMatch(const uint8_t* hashData, uint8_t hash) const {
			integer_type mask = 0;
			for (uint64_t i = 0; i < bucketSize; ++i) {
				if (hashData[i] == hash) {
					mask |= (1 << i);
				}
			}
			return tzcnt(mask);
		}
	};

	template<uint64_t N, typename key_type, typename value_type> using simd_set_variant = std::variant<simd_set<key_type, value_type, N, 1>, simd_set<key_type, value_type, N, 2>,
		simd_set<key_type, value_type, N, 4>, simd_set<key_type, value_type, N, 8>, simd_set<key_type, value_type, N, 16>, simd_set<key_type, value_type, N, 32>,
		simd_set<key_type, value_type, N, 64>, simd_set<key_type, value_type, N, 128>, simd_set<key_type, value_type, N, 256>, simd_set<key_type, value_type, N, 512>,
		simd_set<key_type, value_type, N, 1024>, simd_set<key_type, value_type, N, 2048>, simd_set<key_type, value_type, N, 4096>>;

	template<typename key_type, typename value_type, size_t N> constexpr auto constructSimdSet(const std::array<std::pair<key_type, value_type>, N>& pairsNew)
		-> simd_set_variant<N, key_type, value_type> {
		xoshiro256 prng{};
		return constructSimdSetHelper<key_type, value_type, N, getMaxSizeIndex<N>(simdSetMaxSizes)>(pairsNew, prng);
	}

	template<typename key_type, typename value_type, size_t bucketSize, size_t storageSize, uint64_t N>
	constexpr auto constructSimdSetHelperFinal(const std::array<std::pair<key_type, value_type>, N>& pairsNew, uint64_t seed, uint64_t maxSizeIndex,
		uint64_t stringScalingFactorIndex) -> simd_set_variant<N, key_type, value_type>;

	template<typename key_type, typename value_type, size_t bucketSize, size_t storageSize, uint64_t N> using construct_simd_set_function_ptr =
		decltype(&constructSimdSetHelperFinal<key_type, value_type, bucketSize, storageSize, N>);

	template<typename key_type, typename value_type, uint64_t N>
	constexpr construct_simd_set_function_ptr<key_type, value_type, setSimdWidth<N>(), 1, N> constructSimdSetHelperPtrs[13] = {
		&constructSimdSetHelperFinal<key_type, value_type, setSimdWidth<N>(), 2, N>, &constructSimdSetHelperFinal<key_type, value_type, setSimdWidth<N>(), 4, N>,
		&constructSimdSetHelperFinal<key_type, value_type, setSimdWidth<N>(), 8, N>, &constructSimdSetHelperFinal<key_type, value_type, setSimdWidth<N>(), 16, N>,
		&constructSimdSetHelperFinal<key_type, value_type, setSimdWidth<N>(), 32, N>, &constructSimdSetHelperFinal<key_type, value_type, setSimdWidth<N>(), 64, N>,
		&constructSimdSetHelperFinal<key_type, value_type, setSimdWidth<N>(), 128, N>, &constructSimdSetHelperFinal<key_type, value_type, setSimdWidth<N>(), 256, N>,
		&constructSimdSetHelperFinal<key_type, value_type, setSimdWidth<N>(), 512, N>, &constructSimdSetHelperFinal<key_type, value_type, setSimdWidth<N>(), 1024, N>,
		&constructSimdSetHelperFinal<key_type, value_type, setSimdWidth<N>(), 2048, N>, &constructSimdSetHelperFinal<key_type, value_type, setSimdWidth<N>(), 4096, N>
	};

	template<typename key_type, typename value_type, size_t bucketSize, size_t storageSize, uint64_t N>
	constexpr auto constructSimdSetHelperFinal(const std::array<std::pair<key_type, value_type>, N>& pairsNew, uint64_t seed, uint64_t maxSizeIndex,
		uint64_t stringScalingFactorIndex) -> simd_set_variant<N, key_type, value_type> {
		constexpr uint64_t numGroups = storageSize > bucketSize ? storageSize / bucketSize : 1;
		auto simdSetNew				 = simd_set<key_type, value_type, N, storageSize>{};
		std::array<uint64_t, storageSize> slots{};
		std::array<uint64_t, numGroups> bucketSizes{};
		for (size_t i = 0; i < N; ++i) {
			const auto hash			 = fnv1a_hash{}.operator()(pairsNew[i].first.data(),
				 static_cast<uint64_t>(static_cast<double>(pairsNew[i].first.size()) * scalingFactorTable[stringScalingFactorIndex]), seed);
			const auto groupPos		 = (hash >> 7) % numGroups;
			const auto ctrlByte		 = static_cast<uint8_t>(hash);
			const auto bucketSizeNew = ++bucketSizes[groupPos];
			const auto slot			 = (groupPos * simdSetNew.bucketSize) + bucketSizeNew;

			simdSetNew.items[slot]		  = pairsNew[i].second;
			simdSetNew.controlBytes[slot] = ctrlByte;
			slots[i]					  = slot;
			simdSetNew.hashes[slot]		  = hash;
		}
		simdSetNew.stringScalingFactorIndex = stringScalingFactorIndex;
		simdSetNew.seed						= seed;

		return simd_set_variant<N, key_type, value_type>{ simdSetNew };
	}

	struct simd_set_construction_values {
		uint64_t stringScalingFactorIndex{ std::numeric_limits<uint64_t>::max() };
		uint64_t maxSizeIndex{ std::numeric_limits<uint64_t>::max() };
		uint64_t seed{};
		bool success{};
	};

	template<size_t N> constexpr simd_set_construction_values collectShortestStringScalingFactorIndex(std::array<simd_set_construction_values, N> arrayOfValues) {
		simd_set_construction_values returnValues{};
		for (uint64_t x = 0; x < N; ++x) {
			if (returnValues.stringScalingFactorIndex > arrayOfValues[x].stringScalingFactorIndex && arrayOfValues[x].success &&
				returnValues.maxSizeIndex > arrayOfValues[x].maxSizeIndex) {
				returnValues = arrayOfValues[x];
			}
		}
		return returnValues;
	}

	template<typename key_type, typename value_type, size_t N, uint64_t maxSizeIndex, uint64_t stringScalingFactorIndex = 0>
	constexpr auto constructSimdSetHelper(const std::array<std::pair<key_type, value_type>, N>& pairsNew, xoshiro256 prng,
		std::array<simd_set_construction_values, std::size(simdSetMaxSizes) * std::size(scalingFactorTable)> constructionValues =
			std::array<simd_set_construction_values, std::size(simdSetMaxSizes) * std::size(scalingFactorTable)>{}) -> simd_set_variant<N, key_type, value_type> {
		if constexpr (maxSizeIndex < std::size(simdSetMaxSizes)) {
			constexpr uint64_t bucketSize  = setSimdWidth<N>();
			constexpr uint64_t storageSize = simdSetMaxSizes[maxSizeIndex] > bucketSize ? simdSetMaxSizes[maxSizeIndex] : bucketSize;
			constexpr uint64_t numGroups   = storageSize > bucketSize ? storageSize / bucketSize : 1;
			auto seed					   = prng();
			std::array<uint8_t, storageSize> controlBytes{};
			std::array<uint64_t, numGroups> bucketSizes{};
			std::array<uint64_t, storageSize> slots{};

			for (size_t i = 0; i < N; ++i) {
				const auto hash			 = fnv1a_hash{}.operator()(pairsNew[i].first.data(),
					 static_cast<uint64_t>(static_cast<double>(pairsNew[i].first.size()) * scalingFactorTable[stringScalingFactorIndex]), seed);
				const auto groupPos		 = (hash >> 7) % numGroups;
				const auto ctrlByte		 = static_cast<uint8_t>(hash);
				const auto bucketSizeNew = ++bucketSizes[groupPos];
				const auto slot			 = (groupPos * bucketSize) + bucketSizeNew;

				if (bucketSizeNew >= bucketSize || contains(slots.data() + groupPos * bucketSize, slot, bucketSize) ||
					contains(controlBytes.data() + groupPos * bucketSize, ctrlByte, bucketSize)) {
					constructionValues[maxSizeIndex * std::size(scalingFactorTable) + stringScalingFactorIndex].stringScalingFactorIndex = stringScalingFactorIndex;
					constructionValues[maxSizeIndex * std::size(scalingFactorTable) + stringScalingFactorIndex].maxSizeIndex			 = maxSizeIndex;
					constructionValues[maxSizeIndex * std::size(scalingFactorTable) + stringScalingFactorIndex].seed					 = seed;
					constructionValues[maxSizeIndex * std::size(scalingFactorTable) + stringScalingFactorIndex].success					 = false;
					if constexpr (stringScalingFactorIndex < std::size(scalingFactorTable) - 1) {
						return constructSimdSetHelper<key_type, value_type, N, maxSizeIndex, stringScalingFactorIndex + 1>(pairsNew, prng, constructionValues);
					} else {
						return constructSimdSetHelper<key_type, value_type, N, maxSizeIndex + 1>(pairsNew, prng, constructionValues);
					}
				}

				controlBytes[slot] = ctrlByte;
				slots[i]		   = slot;
			}
			constructionValues[maxSizeIndex * std::size(scalingFactorTable) + stringScalingFactorIndex].stringScalingFactorIndex = stringScalingFactorIndex;
			constructionValues[maxSizeIndex * std::size(scalingFactorTable) + stringScalingFactorIndex].maxSizeIndex			 = maxSizeIndex;
			constructionValues[maxSizeIndex * std::size(scalingFactorTable) + stringScalingFactorIndex].seed					 = seed;
			constructionValues[maxSizeIndex * std::size(scalingFactorTable) + stringScalingFactorIndex].success					 = true;
			if constexpr (maxSizeIndex < std::size(simdSetMaxSizes)) {
				return constructSimdSetHelper<key_type, value_type, N, maxSizeIndex + 1>(pairsNew, prng, constructionValues);
			} else {
				return constructSimdSetHelper<key_type, value_type, N, maxSizeIndex, stringScalingFactorIndex>(pairsNew, prng, constructionValues);
			}
		} else {
			auto resultValues = collectShortestStringScalingFactorIndex(constructionValues);
			auto newSet		  = constructSimdSetHelperPtrs<key_type, value_type, N>[resultValues.maxSizeIndex](pairsNew, resultValues.seed, resultValues.maxSizeIndex,
				  resultValues.stringScalingFactorIndex);
			return { newSet };
		}
	}

	template<const jsonifier::string_view& S, bool CheckSize = true> inline constexpr bool cxStringCmp(const jsonifier::string_view key) noexcept {
		if (std::is_constant_evaluated()) {
			return key == S;
		} else {
			if constexpr (CheckSize) {
				return (S.size() == key.size()) && compare<S.size()>(S.data(), key.data());
			} else {
				return compare<S.size()>(S.data(), key.data());
			}
		}
	}

	template<typename value_type, const jsonifier::string_view& S> struct micro_set1 {
		std::array<value_type, 1> items{};

		constexpr decltype(auto) begin() const noexcept {
			return items.data();
		}
		constexpr decltype(auto) end() const noexcept {
			return items.data() + items.size();
		}

		constexpr decltype(auto) find(auto&& key) const noexcept {
			if (cxStringCmp<S, true>(key)) [[likely]] {
				return items.data();
			} else [[unlikely]] {
				return items.data() + items.size();
			}
		}
	};

	template<typename value_type, const jsonifier::string_view& S0, const jsonifier::string_view& S1> struct micro_set2 {
		std::array<value_type, 2> items{};

		static constexpr bool sameSize	= S0.size() == S1.size();
		static constexpr bool checkSize = !sameSize;

		constexpr decltype(auto) begin() const noexcept {
			return items.data();
		}
		constexpr decltype(auto) end() const noexcept {
			return items.data() + items.size();
		}

		constexpr decltype(auto) find(auto&& key) const noexcept {
			if constexpr (sameSize) {
				constexpr auto n = S0.size();
				if (key.size() != n) {
					return items.data() + items.size();
				}
			}

			if (cxStringCmp<S0, checkSize>(key)) {
				return items.data();
			} else if (cxStringCmp<S1, checkSize>(key)) {
				return items.data() + 1;
			} else [[unlikely]] {
				return items.data() + items.size();
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

	template<typename value_type, size_t I> struct core_sv {
		static constexpr jsonifier::string_view value = getKey<value_type, I>();
	};

	template<typename value_type, size_t I> constexpr auto keyValue() noexcept {
		using value_t		  = value_tuple_variant_t<jsonifier::concepts::core_t<value_type>>;
		constexpr auto& first = std::get<0>(std::get<I>(jsonifier::concepts::core_v<value_type>));
		using T0			  = jsonifier::concepts::unwrap_t<decltype(first)>;
		if constexpr (std::is_member_pointer_v<T0>) {
			return std::pair<jsonifier::string_view, value_t>{ getName<first>(), first };
		} else {
			return std::pair<jsonifier::string_view, value_t>{ jsonifier::string_view(first), std::get<1>(std::get<I>(jsonifier::concepts::core_v<value_type>)) };
		}
	}

	template<typename value_type, size_t I> constexpr auto getValue() noexcept {
		constexpr auto& first = std::get<0>(std::get<I>(jsonifier::concepts::core_v<value_type>));
		using T0			  = jsonifier::concepts::unwrap_t<decltype(first)>;
		if constexpr (std::is_member_pointer_v<T0>) {
			return first;
		} else {
			return std::get<1>(std::get<I>(jsonifier::concepts::core_v<value_type>));
		}
	}

	template<typename value_type, size_t... I> constexpr auto makeSetImpl(std::index_sequence<I...>) {
		using value_t	 = value_tuple_variant_t<jsonifier::concepts::core_t<value_type>>;
		constexpr auto n = std::tuple_size_v<jsonifier::concepts::core_t<value_type>>;

		if constexpr (n == 0) {
			return nullptr;
		} else if constexpr (n == 1) {
			return micro_set1<value_t, core_sv<value_type, I>::value...>{ getValue<value_type, I>()... };
		} else if constexpr (n == 2) {
			return micro_set2<value_t, core_sv<value_type, I>::value...>{ getValue<value_type, I>()... };
		} else {
			constexpr auto setNew{ constructSimdSet<jsonifier::string_view, value_t, n>({ keyValue<value_type, I>()... }) };
			constexpr auto newIndex = setNew.index();
			return std::get<newIndex>(setNew);
		}
	}

	template<typename value_type> constexpr auto makeSet() {
		constexpr auto indices = std::make_index_sequence<std::tuple_size_v<jsonifier::concepts::core_t<value_type>>>{};
		return makeSetImpl<jsonifier::concepts::decay_keep_volatile_t<value_type>>(indices);
	}
}