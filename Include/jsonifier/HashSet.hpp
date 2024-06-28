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

#include <jsonifier/StaticVector.hpp>
#include <jsonifier/StringView.hpp>
#include <jsonifier/Reflection.hpp>
#include <jsonifier/Derailleur.hpp>
#include <jsonifier/Config.hpp>
#include <jsonifier/Tuple.hpp>
#include <jsonifier/Error.hpp>
#include <algorithm>
#include <numeric>
#include <span>

namespace jsonifier_internal {

	struct xoshiro256 {
		std::array<size_t, 4> state{};

		constexpr xoshiro256() {
			auto x = 7185499250578500046ull >> 12ull;
			x ^= x << 25ull;
			x ^= x >> 27ull;
			size_t s = x * 0x2545F4914F6CDD1Dull;
			for (auto& y: state) {
				y = splitmix64(s);
			}
		}

		constexpr size_t operator()() {
			const size_t result = rotl(state[1] * 5ull, 7ull) * 9ull;

			const size_t t = state[1] << 17ull;

			state[2ull] ^= state[0ull];
			state[3ull] ^= state[1ull];
			state[1ull] ^= state[2ull];
			state[0ull] ^= state[3ull];

			state[2ull] ^= t;

			state[3ull] = rotl(state[3ull], 45ull);

			return result;
		}

	  protected:
		constexpr size_t rotl(const size_t x, size_t k) const {
			return (x << k) | (x >> (64ull - k));
		}

		constexpr size_t splitmix64(size_t& seed) const {
			size_t result = seed += 0x9E3779B97F4A7C15ull;
			result		  = (result ^ (result >> 30ull)) * 0xBF58476D1CE4E5B9ull;
			result		  = (result ^ (result >> 27ull)) * 0x94D049BB133111EBull;
			return result ^ (result >> 31ull);
		}
	};

	constexpr double scalingFactorTable[]{  0.20f, 0.30f, 0.40f, 0.50f, 0.60f, 0.70f, 0.80f, 0.90f, 1.0f };

	template<typename value_type01, typename value_type02> constexpr bool contains(const value_type01* hashData, value_type02 byteToCheckFor, size_t size) {
		for (size_t x = 0; x < size; ++x) {
			if (hashData[x] == byteToCheckFor) {
				return true;
			}
		}
		return false;
	}

	template<size_t size, size_t length> constexpr size_t getMaxSizeIndex(const size_t (&maxSizes)[length]) {
		for (size_t x = 0; x < std::size(maxSizes); ++x) {
			if (size <= maxSizes[x]) {
				return x;
			}
		}
		return std::size(maxSizes) - 1;
	}

	struct set_construction_values {
		size_t stringScalingFactorIndex{ std::numeric_limits<size_t>::max() };
		size_t maxSizeIndex{ std::numeric_limits<size_t>::max() };
		bool success{};
		size_t seed{};
	};

	template<size_t length> struct set_simd {
		using type = std::conditional_t<length >= 64 && bytesPerStep >= 64, simd_int_512, std::conditional_t<length >= 32 && bytesPerStep >= 32, simd_int_256, simd_int_128>>;
	};

	template<size_t length> struct set_integer {
		using type = size_t;
	};

	template<size_t length> constexpr size_t setSimdWidth() {
		return length >= 64 && bytesPerStep >= 64 ? 64 : length >= 32 && bytesPerStep >= 32 ? 32 : 16;
	};

	template<size_t length> using set_simd_t = set_simd<length>::type;

	template<size_t length> using set_integer_t = set_integer<length>::type;

	template<size_t sizeOne, size_t sizeTwo>
	constexpr set_construction_values collectOptimalConstructionValues(std::array<std::array<set_construction_values, sizeOne>, sizeTwo> indexTable) {
		set_construction_values returnValues{};
		for (size_t x = 0; x < sizeTwo; ++x) {
			for (size_t y = 0; y < sizeOne; ++y) {
				if (indexTable[x][y].success &&
					((returnValues.stringScalingFactorIndex > indexTable[x][y].stringScalingFactorIndex) ||
						(returnValues.stringScalingFactorIndex == indexTable[x][y].stringScalingFactorIndex && returnValues.maxSizeIndex > indexTable[x][y].maxSizeIndex))) {
					returnValues = indexTable[x][y];
				}
			}
		}
		return returnValues;
	}

	constexpr size_t simdHashSetMaxSizes[]{ 16, 32, 64, 128, 256, 512, 1024 };

	template<typename key_type, typename value_type, size_t actualCount, size_t storageSize> struct simd_hash_set {
		static constexpr size_t bucketSize = setSimdWidth<actualCount>();
		static constexpr size_t numGroups  = storageSize > bucketSize ? storageSize / bucketSize : 1;
		using simd_type					   = set_simd_t<bucketSize>;
		using integer_type				   = set_integer_t<bucketSize>;
		JSONIFIER_ALIGN std::array<size_t, storageSize> hashes{ { static_cast<uint64_t>(-1) } };
		JSONIFIER_ALIGN std::array<uint8_t, storageSize> controlBytes{};
		JSONIFIER_ALIGN std::array<value_type, storageSize> items{};
		JSONIFIER_ALIGN double stringScalingFactor{};
		JSONIFIER_ALIGN key_hasher hasher{};

		constexpr simd_hash_set() noexcept = default;

		JSONIFIER_INLINE constexpr decltype(auto) begin() const noexcept {
			return items.data();
		}

		JSONIFIER_INLINE constexpr decltype(auto) end() const noexcept {
			return items.data() + storageSize;
		}

		JSONIFIER_INLINE constexpr auto size() const noexcept {
			return actualCount;
		}

		template<typename key_type_new> JSONIFIER_INLINE constexpr decltype(auto) find(key_type_new&& key) const noexcept {
			if (!std::is_constant_evaluated()) {
				JSONIFIER_ALIGN const auto hash		   = hasher.hashKeyRt(key.data(), static_cast<size_t>(static_cast<double>(key.size()) * stringScalingFactor));
				JSONIFIER_ALIGN const auto resultIndex = ((hash >> 7) % numGroups) * bucketSize;
				prefetchInternal(controlBytes.data() + resultIndex);
				JSONIFIER_ALIGN const auto finalIndex = simd_internal::tzcnt(simd_internal::opCmpEq(simd_internal::gatherValue<simd_type>(static_cast<uint8_t>(hash)),
															   simd_internal::gatherValues<simd_type>(controlBytes.data() + resultIndex))) %
						bucketSize +
					resultIndex;
				return hashes[finalIndex] == hash ? items.data() + finalIndex : end();
			} else {
				JSONIFIER_ALIGN const auto hash			 = hasher.hashKeyCt(key.data(), static_cast<size_t>(static_cast<double>(key.size()) * stringScalingFactor));
				JSONIFIER_ALIGN const auto resultIndex = ((hash >> 7) % numGroups) * bucketSize;
				JSONIFIER_ALIGN const auto finalIndex	 = constMatch(controlBytes.data() + resultIndex, static_cast<uint8_t>(hash)) % bucketSize + resultIndex;
				return hashes[finalIndex] == hash ? items.data() + finalIndex : end();
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

		constexpr integer_type constMatch(const uint8_t* hashData, uint8_t hash) const {
			integer_type mask = 0;
			for (size_t x = 0; x < bucketSize; ++x) {
				if (hashData[x] == hash) {
					mask |= (1ull << x);
				}
			}
			return tzcnt(mask);
		}
	};

	template<size_t startingValue, size_t actualCount, typename key_type, typename value_type, template<typename, typename, size_t, size_t> typename set_type> using set_variant =
		std::variant<set_type<key_type, value_type, actualCount, startingValue>, set_type<key_type, value_type, actualCount, startingValue * 2ull>,
			set_type<key_type, value_type, actualCount, startingValue * 4ull>, set_type<key_type, value_type, actualCount, startingValue * 8ull>,
			set_type<key_type, value_type, actualCount, startingValue * 16ull>, set_type<key_type, value_type, actualCount, startingValue * 32ull>,
			set_type<key_type, value_type, actualCount, startingValue * 64ull>>;

	template<typename key_type, typename value_type, size_t actualCount, size_t storageSize>
	constexpr auto constructSimdHashSetFinal(const std::array<std::pair<key_type, value_type>, actualCount>& pairsNew, uint64_t seed, size_t stringScalingFactorIndex)
		-> set_variant<16, actualCount, key_type, value_type, simd_hash_set> {
		constexpr size_t bucketSize = setSimdWidth<actualCount>();
		constexpr size_t numGroups	= storageSize > bucketSize ? storageSize / bucketSize : 1;
		simd_hash_set<key_type, value_type, actualCount, storageSize> simdHashSetNew{};
		simdHashSetNew.hasher			   = seed;
		simdHashSetNew.stringScalingFactor = scalingFactorTable[stringScalingFactorIndex];
		std::array<size_t, numGroups> bucketSizes{};
		for (size_t x = 0; x < actualCount; ++x) {
			const auto hash =
				simdHashSetNew.hasher.hashKeyCt(pairsNew[x].first.data(), static_cast<size_t>(static_cast<double>(pairsNew[x].first.size()) * simdHashSetNew.stringScalingFactor));
			const auto groupPos				  = (hash >> 7) % numGroups;
			const auto ctrlByte				  = static_cast<uint8_t>(hash);
			const auto bucketSizeNew		  = ++bucketSizes[groupPos];
			const auto slot					  = (groupPos * bucketSize) + bucketSizeNew;
			simdHashSetNew.items[slot]		  = pairsNew[x].second;
			simdHashSetNew.controlBytes[slot] = ctrlByte;
			simdHashSetNew.hashes[slot]		  = hash;
		}

		return set_variant<16, actualCount, key_type, value_type, simd_hash_set>{ simd_hash_set<key_type, value_type, actualCount, storageSize>(simdHashSetNew) };
	}

	template<typename key_type, typename value_type, size_t storageSize, size_t actualCount> using construct_simd_hash_set_function_ptr =
		decltype(&constructSimdHashSetFinal<key_type, value_type, storageSize, 16ull>);

	template<typename key_type, typename value_type, size_t actualCount>
	constexpr construct_simd_hash_set_function_ptr<key_type, value_type, actualCount, 16ull> constructSimdHashSetFinalPtrs[7] = {
		&constructSimdHashSetFinal<key_type, value_type, actualCount, 16ull>, &constructSimdHashSetFinal<key_type, value_type, actualCount, 32ull>,
		&constructSimdHashSetFinal<key_type, value_type, actualCount, 64ull>, &constructSimdHashSetFinal<key_type, value_type, actualCount, 128ull>,
		&constructSimdHashSetFinal<key_type, value_type, actualCount, 256ull>, &constructSimdHashSetFinal<key_type, value_type, actualCount, 512ull>,
		&constructSimdHashSetFinal<key_type, value_type, actualCount, 1024ull>
	};

	template<size_t maxSizeIndex, size_t stringScalingFactorIndex, typename key_type, typename value_type, size_t actualCount>
	constexpr auto constructSimdHashSet(const std::array<std::pair<key_type, value_type>, actualCount>& pairsNew, xoshiro256 prng = xoshiro256{},
		std::array<std::array<set_construction_values, std::size(scalingFactorTable)>, std::size(simdHashSetMaxSizes)> constructionValues =
			std::array<std::array<set_construction_values, std::size(scalingFactorTable)>, std::size(simdHashSetMaxSizes)>{})
		-> set_variant<16, actualCount, key_type, value_type, simd_hash_set> {
		constexpr size_t bucketSize	 = setSimdWidth<actualCount>();
		constexpr size_t storageSize = simdHashSetMaxSizes[maxSizeIndex];
		constexpr size_t numGroups	 = storageSize / bucketSize;
		auto seed					 = prng();
		std::array<uint8_t, storageSize> controlBytes{};
		std::array<size_t, numGroups> bucketSizes{};
		std::array<size_t, storageSize> slots{};
		key_hasher hasherNew{ seed };
		for (size_t x = 0; x < actualCount; ++x) {
			const auto hash			 = hasherNew.hashKeyCt(pairsNew[x].first.data(),
						 static_cast<size_t>(static_cast<double>(pairsNew[x].first.size()) * scalingFactorTable[stringScalingFactorIndex]));
			const auto groupPos		 = (hash >> 7) % numGroups;
			const auto ctrlByte		 = static_cast<uint8_t>(hash);
			const auto bucketSizeNew = ++bucketSizes[groupPos];
			const auto slot			 = (groupPos * bucketSize) + bucketSizeNew;

			if (bucketSizeNew >= bucketSize || contains(slots.data() + groupPos * bucketSize, slot, bucketSize) ||
				contains(controlBytes.data() + groupPos * bucketSize, ctrlByte, bucketSize)) {
				constructionValues[maxSizeIndex][stringScalingFactorIndex] = set_construction_values{ stringScalingFactorIndex, maxSizeIndex, false, seed };
				if constexpr (stringScalingFactorIndex < std::size(scalingFactorTable) - 1) {
					return constructSimdHashSet<maxSizeIndex, stringScalingFactorIndex + 1, key_type, value_type>(pairsNew, prng, constructionValues);
				} else if constexpr (maxSizeIndex < std::size(simdHashSetMaxSizes) - 1) {
					return constructSimdHashSet<maxSizeIndex + 1, 0, key_type, value_type>(pairsNew, prng, constructionValues);
				} else {
					auto resultValues = collectOptimalConstructionValues(constructionValues);
					auto newSet		  = constructSimdHashSetFinalPtrs<key_type, value_type, actualCount>[resultValues.maxSizeIndex](pairsNew, resultValues.seed,
						  resultValues.stringScalingFactorIndex);
					return { newSet };
				}
			}

			controlBytes[slot] = ctrlByte;
			slots[x]		   = slot;
		}

		constructionValues[maxSizeIndex][stringScalingFactorIndex] = set_construction_values{ stringScalingFactorIndex, maxSizeIndex, true, seed };
		if constexpr (stringScalingFactorIndex < std::size(scalingFactorTable) - 1) {
			return constructSimdHashSet<maxSizeIndex, stringScalingFactorIndex + 1, key_type, value_type>(pairsNew, prng, constructionValues);
		} else if constexpr (maxSizeIndex < std::size(simdHashSetMaxSizes) - 1) {
			return constructSimdHashSet<maxSizeIndex + 1, 0, key_type, value_type>(pairsNew, prng, constructionValues);
		} else {
			auto resultValues = collectOptimalConstructionValues(constructionValues);
			auto newSet =
				constructSimdHashSetFinalPtrs<key_type, value_type, actualCount>[resultValues.maxSizeIndex](pairsNew, resultValues.seed, resultValues.stringScalingFactorIndex);
			return { newSet };
		}
	}

	constexpr size_t serialHashSetMaxSizes[]{ 2, 4, 8, 16, 32, 64, 128 };

	template<typename key_type, typename value_type, size_t actualCount, size_t storageSize> struct serial_hash_set {
		JSONIFIER_ALIGN std::array<size_t, storageSize> hashes{ { static_cast<uint64_t>(-1) } };
		JSONIFIER_ALIGN std::array<value_type, storageSize> items{};
		JSONIFIER_ALIGN double stringScalingFactor{};
		JSONIFIER_ALIGN key_hasher hasher{};

		constexpr serial_hash_set() noexcept = default;

		JSONIFIER_INLINE constexpr decltype(auto) begin() const noexcept {
			return items.data();
		}

		JSONIFIER_INLINE constexpr decltype(auto) end() const noexcept {
			return items.data() + storageSize;
		}

		JSONIFIER_INLINE constexpr auto size() const noexcept {
			return actualCount;
		}

		template<typename key_type_new> JSONIFIER_INLINE constexpr decltype(auto) find(key_type_new&& key) const noexcept {
			if (!std::is_constant_evaluated()) {
				JSONIFIER_ALIGN const auto hash		  = hasher.hashKeyRt(key.data(), static_cast<size_t>(static_cast<double>(key.size()) * stringScalingFactor));
				JSONIFIER_ALIGN const auto finalIndex = hash % storageSize;
				return hashes[finalIndex] == hash ? items.data() + finalIndex : end();
			} else {
				JSONIFIER_ALIGN const auto hash		  = hasher.hashKeyCt(key.data(), static_cast<size_t>(static_cast<double>(key.size()) * stringScalingFactor));
				JSONIFIER_ALIGN const auto finalIndex = hash % storageSize;
				return hashes[finalIndex] == hash ? items.data() + finalIndex : end();
			}
		}
	};

	template<typename key_type, typename value_type, size_t actualCount, size_t storageSize>
	constexpr auto constructSerialHashSetFinal(const std::array<std::pair<key_type, value_type>, actualCount>& pairsNew, uint64_t seed, size_t stringScalingFactorIndex)
		-> set_variant<2, actualCount, key_type, value_type, serial_hash_set> {
		serial_hash_set<key_type, value_type, actualCount, storageSize> serialHashSetNew{};
		serialHashSetNew.hasher				 = seed;
		serialHashSetNew.stringScalingFactor = scalingFactorTable[stringScalingFactorIndex];
		for (size_t x = 0; x < actualCount; ++x) {
			const auto hash						= serialHashSetNew.hasher.hashKeyCt(pairsNew[x].first.data(),
									static_cast<size_t>(static_cast<double>(pairsNew[x].first.size()) * serialHashSetNew.stringScalingFactor));
			const auto finalIndex				= hash % storageSize;
			serialHashSetNew.items[finalIndex]	= pairsNew[x].second;
			serialHashSetNew.hashes[finalIndex] = hash;
		}

		return set_variant<2, actualCount, key_type, value_type, serial_hash_set>{ serial_hash_set<key_type, value_type, actualCount, storageSize>(serialHashSetNew) };
	}

	template<typename key_type, typename value_type, size_t storageSize, size_t actualCount> using construct_serial_hash_set_function_ptr =
		decltype(&constructSerialHashSetFinal<key_type, value_type, storageSize, 2ull>);

	template<typename key_type, typename value_type, size_t actualCount>
	constexpr construct_serial_hash_set_function_ptr<key_type, value_type, actualCount, 2ull> constructSerialHashSetFinalPtrs[7] = {
		&constructSerialHashSetFinal<key_type, value_type, actualCount, 2ull>, &constructSerialHashSetFinal<key_type, value_type, actualCount, 4ull>,
		&constructSerialHashSetFinal<key_type, value_type, actualCount, 8ull>, &constructSerialHashSetFinal<key_type, value_type, actualCount, 16ull>,
		&constructSerialHashSetFinal<key_type, value_type, actualCount, 32ull>, &constructSerialHashSetFinal<key_type, value_type, actualCount, 64ull>,
		&constructSerialHashSetFinal<key_type, value_type, actualCount, 128ull>
	};

	template<size_t maxSizeIndex, size_t stringScalingFactorIndex, typename key_type, typename value_type, size_t actualCount>
	constexpr auto constructSerialHashSet(const std::array<std::pair<key_type, value_type>, actualCount>& pairsNew, xoshiro256 prng = xoshiro256{},
		std::array<std::array<set_construction_values, std::size(scalingFactorTable)>, std::size(serialHashSetMaxSizes)> constructionValues =
			std::array<std::array<set_construction_values, std::size(scalingFactorTable)>, std::size(serialHashSetMaxSizes)>{})
		-> set_variant<2, actualCount, key_type, value_type, serial_hash_set> {
		constexpr size_t storageSize = serialHashSetMaxSizes[maxSizeIndex];
		auto seed					 = prng();
		std::array<size_t, storageSize> slots{};
		key_hasher hasherNew{ seed };
		for (size_t x = 0; x < actualCount; ++x) {
			const auto hash		  = hasherNew.hashKeyCt(pairsNew[x].first.data(),
					  static_cast<size_t>(static_cast<double>(pairsNew[x].first.size()) * scalingFactorTable[stringScalingFactorIndex]));
			const auto finalIndex = hash % storageSize;

			if (contains(slots.data(), finalIndex, slots.size())) {
				constructionValues[maxSizeIndex][stringScalingFactorIndex] = set_construction_values{ stringScalingFactorIndex, maxSizeIndex, false, seed };
				if constexpr (stringScalingFactorIndex < std::size(scalingFactorTable) - 1) {
					return constructSerialHashSet<maxSizeIndex, stringScalingFactorIndex + 1, key_type, value_type>(pairsNew, prng, constructionValues);
				} else if constexpr (maxSizeIndex < std::size(serialHashSetMaxSizes) - 1) {
					return constructSerialHashSet<maxSizeIndex + 1, 0, key_type, value_type>(pairsNew, prng, constructionValues);
				} else {
					auto resultValues = collectOptimalConstructionValues(constructionValues);
					if (resultValues.maxSizeIndex < std::size(constructSerialHashSetFinalPtrs<key_type, value_type, actualCount>)) {
						auto newSet = constructSerialHashSetFinalPtrs<key_type, value_type, actualCount>[resultValues.maxSizeIndex](pairsNew, resultValues.seed,
							resultValues.stringScalingFactorIndex);
						return { newSet };
					} else {
						return {};
					}
				}
			}

			slots[x] = finalIndex;
		}

		constructionValues[maxSizeIndex][stringScalingFactorIndex] = set_construction_values{ stringScalingFactorIndex, maxSizeIndex, true, seed };
		if constexpr (stringScalingFactorIndex < std::size(scalingFactorTable) - 1) {
			return constructSerialHashSet<maxSizeIndex, stringScalingFactorIndex + 1, key_type, value_type>(pairsNew, prng, constructionValues);
		} else if constexpr (maxSizeIndex < std::size(serialHashSetMaxSizes) - 1) {
			return constructSerialHashSet<maxSizeIndex + 1, 0, key_type, value_type>(pairsNew, prng, constructionValues);
		} else {
			auto resultValues = collectOptimalConstructionValues(constructionValues);
			auto newSet =
				constructSerialHashSetFinalPtrs<key_type, value_type, actualCount>[resultValues.maxSizeIndex](pairsNew, resultValues.seed, resultValues.stringScalingFactorIndex);
			return { newSet };
		}
	}

	template<const jsonifier::string_view& S, bool CheckSize = true> JSONIFIER_INLINE constexpr bool cxStringCmp(const jsonifier::string_view key) noexcept {
		if (std::is_constant_evaluated()) {
			return key == S;
		} else {
			constexpr auto size = S.size();
			if constexpr (CheckSize) {
				return (size == key.size()) && compare<size>(S.data(), key.data());
			} else {
				return compare<size>(S.data(), key.data());
			}
		}
	}

	template<const jsonifier::string_view& lhs> inline constexpr bool compareSv(const jsonifier::string_view rhs) noexcept {
		if (std::is_constant_evaluated()) {
			return lhs == rhs;
		} else {
			constexpr auto N = lhs.size();
			return (N == rhs.size()) && compare<N>(lhs.data(), rhs.data());
		}
	}

	template<typename value_type, const jsonifier::string_view& S> struct micro_set1 {
		std::array<value_type, 1> items{};

		JSONIFIER_INLINE constexpr decltype(auto) begin() const noexcept {
			return items.begin();
		}

		JSONIFIER_INLINE constexpr decltype(auto) end() const noexcept {
			return items.end();
		}

		template<typename key_type> JSONIFIER_INLINE constexpr decltype(auto) find(key_type&& key) const noexcept {
			if (compareSv<S>(key)) [[likely]] {
				return items.begin();
			} else [[unlikely]] {
				return items.end();
			}
		}
	};

	template<typename value_type, const jsonifier::string_view& S0, const jsonifier::string_view& S1> struct micro_set2 {
		std::array<value_type, 2> items{};

		static constexpr bool sameSize	 = S0.size() == S1.size();
		static constexpr bool checkSize = !sameSize;

		JSONIFIER_INLINE constexpr decltype(auto) begin() const noexcept {
			return items.begin();
		}

		JSONIFIER_INLINE constexpr decltype(auto) end() const noexcept {
			return items.end();
		}

		template<typename key_type> JSONIFIER_INLINE constexpr decltype(auto) find(key_type&& key) const noexcept {
			if constexpr (sameSize) {
				constexpr auto n = S0.size();
				if (key.size() != n) {
					return items.end();
				}
			}

			if (cxStringCmp<S0, checkSize>(key)) {
				return items.begin();
			} else if (cxStringCmp<S1, checkSize>(key)) {
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
		} else if constexpr (n < 16) {
			constexpr auto setNew{ constructSerialHashSet<getMaxSizeIndex<n>(simdHashSetMaxSizes), 0, jsonifier::string_view, value_t, n>({ keyValue<value_type, I>()... }) };
			constexpr auto newIndex = setNew.index();
			return std::get<newIndex>(setNew);
		} else {
			constexpr auto setNew{ constructSimdHashSet<getMaxSizeIndex<n>(simdHashSetMaxSizes), 0, jsonifier::string_view, value_t, n>({ keyValue<value_type, I>()... }) };
			constexpr auto newIndex = setNew.index();
			return std::get<newIndex>(setNew);
		}
	}

	template<typename value_type> constexpr auto makeSet() {
		constexpr auto indices = std::make_index_sequence<std::tuple_size_v<jsonifier::concepts::core_t<value_type>>>{};
		return makeSetImpl<jsonifier::concepts::decay_keep_volatile_t<value_type>>(indices);
	}
}