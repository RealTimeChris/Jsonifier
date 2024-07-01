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
			for (size_t x = 0; x < 4; ++x) {
				state[x] = splitmix64(s);
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

	constexpr double scalingFactorTable[]{ 0.20f, 0.30f, 0.40f, 0.50f, 0.60f, 0.70f, 0.80f, 0.90f, 1.0f };

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

	constexpr size_t simdHashSetMaxSizes[]{ 16, 32, 64, 128, 256, 512, 1024 };

	struct set_construction_values {
		size_t maxStringLength{ std::numeric_limits<size_t>::min() };
		size_t minStringLength{ std::numeric_limits<size_t>::max() };
		size_t shortestSuccess{ std::numeric_limits<size_t>::max() };
		size_t maxSizeIndex{};
		size_t stringLength{};
		bool success{};
		size_t seed{};

		constexpr set_construction_values(const set_construction_values& other) {
			maxStringLength = other.maxStringLength;
			minStringLength = other.minStringLength;
			shortestSuccess = other.shortestSuccess;
			maxSizeIndex	= other.maxSizeIndex;
			stringLength	= other.stringLength;
			success			= other.success;
			seed			= other.seed;
		}

		constexpr set_construction_values() noexcept = default;

		constexpr set_construction_values(size_t maxStringLengthNew, size_t minStringLengthNew, size_t shortestSuccessNew, size_t maxSizeIndexNew, size_t stringLengthNew,
			bool successNew, size_t seedNew) {
			if (successNew && stringLength < shortestSuccessNew) {
				shortestSuccess = stringLength;
			}
			maxStringLength = maxStringLengthNew;
			minStringLength = minStringLengthNew;
			shortestSuccess = shortestSuccessNew;
			maxSizeIndex	= maxSizeIndexNew;
			stringLength	= stringLengthNew;
			success			= successNew;
			seed			= seedNew;
		}

		template<size_t actualSize> static constexpr set_construction_values constructValues(size_t minLength, size_t maxLength) {
			set_construction_values returnValues{};
			returnValues.minStringLength = minLength;
			returnValues.maxStringLength = maxLength;
			returnValues.maxSizeIndex	 = getMaxSizeIndex<actualSize>(simdHashSetMaxSizes);
			returnValues.stringLength	 = minLength;
			return returnValues;
		}

		constexpr set_construction_values incrementSizeIndex(bool successNew = false) const {
			set_construction_values returnValues{ *this };
			++returnValues.maxSizeIndex;
			returnValues.stringLength = minStringLength;
			return returnValues;
		}

		constexpr set_construction_values incrementStringLength(bool successNew = false) const {
			set_construction_values returnValues{ *this };
			++returnValues.stringLength;
			return returnValues;
		}
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

	template<size_t sizeOne> constexpr set_construction_values collectOptimalConstructionValues(static_vector<set_construction_values, sizeOne> indexTable) {
		static_vector<set_construction_values, sizeOne> newIndexTable{};
		set_construction_values returnValues{};
		for (size_t x = 0; x < sizeOne; ++x) {
			if (indexTable[x].success) {
				newIndexTable.emplace_back(indexTable[x]);
			}
		}
		std::sort(newIndexTable.begin(), newIndexTable.end(), [](set_construction_values lhs, set_construction_values rhs) {
			return lhs.stringLength < rhs.stringLength && lhs.maxSizeIndex < rhs.maxSizeIndex;
		});
		returnValues = newIndexTable[0];
		return returnValues;
	}

	JSONIFIER_INLINE constexpr bool compareSvConst(const jsonifier::string_view& lhs, const jsonifier::string_view rhs) noexcept {
		return lhs == rhs;
	}

	JSONIFIER_INLINE constexpr bool compareSvNonConst(const jsonifier::string_view& lhs, const jsonifier::string_view rhs) noexcept {
		return lhs.size() == rhs.size() && compare(lhs.data(), rhs.data(), rhs.size());
	}

	template<typename key_type, typename value_type, size_t actualCount, size_t storageSize> struct simd_hash_set : public key_hasher {
		static constexpr size_t bucketSize = setSimdWidth<actualCount>();
		static constexpr size_t numGroups  = storageSize > bucketSize ? storageSize / bucketSize : 1;
		using simd_type					   = set_simd_t<bucketSize>;
		using integer_type				   = set_integer_t<bucketSize>;
		JSONIFIER_ALIGN std::pair<jsonifier::string_view, value_type> items[storageSize]{};
		JSONIFIER_ALIGN uint8_t controlBytes[storageSize]{};
		JSONIFIER_ALIGN size_t stringLength{};

		constexpr simd_hash_set() noexcept = default;

		JSONIFIER_INLINE constexpr decltype(auto) begin() const noexcept {
			return &items->second;
		}

		JSONIFIER_INLINE constexpr decltype(auto) end() const noexcept {
			return &(items + storageSize)->second;
		}

		JSONIFIER_INLINE constexpr auto size() const noexcept {
			return actualCount;
		}

		template<typename key_type_new> JSONIFIER_INLINE constexpr decltype(auto) find(key_type_new&& key) const noexcept {
			if (!std::is_constant_evaluated()) {
				JSONIFIER_ALIGN const auto hash		   = hashKeyRt(key.data(), key.size() > stringLength ? stringLength : key.size());
				JSONIFIER_ALIGN const auto resultIndex = ((hash >> 7) % numGroups) * bucketSize;
				prefetchInternal(controlBytes + resultIndex);
				JSONIFIER_ALIGN const auto finalIndex = (simd_internal::tzcnt(simd_internal::opCmpEq(simd_internal::gatherValue<simd_type>(static_cast<uint8_t>(hash)),
															 simd_internal::gatherValues<simd_type>(controlBytes + resultIndex))) %
																bucketSize +
															resultIndex) %
					storageSize;
				return LIKELY(compareSvNonConst(items[finalIndex].first, key)) ? &(items + finalIndex)->second : end();
			} else {
				JSONIFIER_ALIGN const auto hash		   = hashKeyCt(key.data(), key.size() > stringLength ? stringLength : key.size());
				JSONIFIER_ALIGN const auto resultIndex = ((hash >> 7) % numGroups) * bucketSize;
				JSONIFIER_ALIGN const auto finalIndex  = (constMatch(controlBytes + resultIndex, static_cast<uint8_t>(hash)) % bucketSize + resultIndex) % storageSize;
				return LIKELY(compareSvConst(items[finalIndex].first, key)) ? &(items + finalIndex)->second : end();
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
	constexpr auto constructSimdHashSetFinal(const std::array<std::pair<key_type, value_type>, actualCount>& pairsNew, set_construction_values constructionValues)
		-> set_variant<16, actualCount, key_type, value_type, simd_hash_set> {
		constexpr size_t bucketSize = setSimdWidth<actualCount>();
		constexpr size_t numGroups	= storageSize > bucketSize ? storageSize / bucketSize : 1;
		simd_hash_set<key_type, value_type, actualCount, storageSize> simdHashSetNew{};
		simdHashSetNew.setSeed(constructionValues.seed);
		simdHashSetNew.stringLength = constructionValues.stringLength;
		std::array<size_t, numGroups> bucketSizes{};
		for (size_t x = 0; x < actualCount; ++x) {
			const auto hash =
				simdHashSetNew.hashKeyCt(pairsNew[x].first.data(), pairsNew[x].first.size() > simdHashSetNew.stringLength ? simdHashSetNew.stringLength : pairsNew[x].first.size());
			const auto groupPos				  = (hash >> 7) % numGroups;
			const auto ctrlByte				  = static_cast<uint8_t>(hash);
			const auto bucketSizeNew		  = ++bucketSizes[groupPos];
			const auto slot					  = (groupPos * bucketSize) + bucketSizeNew;
			simdHashSetNew.items[slot]		  = pairsNew[x];
			simdHashSetNew.controlBytes[slot] = ctrlByte;
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

	template<set_construction_values constructionValuesNew, typename key_type, typename value_type, size_t actualCount, size_t retries = 0>
	constexpr auto constructSimdHashSet(const std::array<std::pair<key_type, value_type>, actualCount>& pairsNew, xoshiro256 prng = xoshiro256{},
		static_vector<set_construction_values, (constructionValuesNew.maxStringLength - constructionValuesNew.minStringLength) * std::size(simdHashSetMaxSizes)>
			constructionValues =
				static_vector<set_construction_values, (constructionValuesNew.maxStringLength - constructionValuesNew.minStringLength) * std::size(simdHashSetMaxSizes)>{})
		-> set_variant<16, actualCount, key_type, value_type, simd_hash_set> {
		constexpr size_t bucketSize	 = setSimdWidth<actualCount>();
		constexpr size_t storageSize = simdHashSetMaxSizes[constructionValuesNew.maxSizeIndex];
		constexpr size_t numGroups	 = storageSize > bucketSize ? storageSize / bucketSize : 1;
		auto seed					 = prng();
		uint8_t controlBytes[storageSize]{};
		size_t bucketSizes[numGroups]{};
		size_t slots[storageSize]{};
		std::fill(slots, slots + storageSize, std::numeric_limits<size_t>::max());
		key_hasher hasherNew{ seed };
		for (size_t x = 0; x < actualCount; ++x) {
			const auto hash			 = hasherNew.hashKeyCt(pairsNew[x].first.data(),
				 pairsNew[x].first.size() > constructionValuesNew.stringLength ? constructionValuesNew.stringLength : pairsNew[x].first.size());
			const auto groupPos		 = (hash >> 7) % numGroups;
			const auto ctrlByte		 = static_cast<uint8_t>(hash);
			const auto bucketSizeNew = ++bucketSizes[groupPos];
			const auto slot			 = ((groupPos * bucketSize) + bucketSizeNew) % storageSize;

			if (bucketSizeNew >= bucketSize || contains(slots + groupPos * bucketSize, slot, bucketSize) || contains(controlBytes + groupPos * bucketSize, ctrlByte, bucketSize)) {
				if constexpr (constructionValuesNew.stringLength < constructionValuesNew.maxStringLength - 1) {
					if constexpr (retries <= 2) {
						return constructSimdHashSet<constructionValuesNew, key_type, value_type, actualCount, retries + 1>(pairsNew, prng, constructionValues);
					}
					return constructSimdHashSet<constructionValuesNew.incrementStringLength(), key_type, value_type, actualCount, retries>(pairsNew, prng, constructionValues);
				} else if constexpr (constructionValuesNew.maxSizeIndex < std::size(simdHashSetMaxSizes) - 1) {
					if constexpr (retries <= 2) {
						return constructSimdHashSet<constructionValuesNew, key_type, value_type, actualCount, retries + 1>(pairsNew, prng, constructionValues);
					}
					return constructSimdHashSet<constructionValuesNew.incrementSizeIndex(), key_type, value_type, actualCount, retries>(pairsNew, prng, constructionValues);
				} else {
					auto resultValues = collectOptimalConstructionValues(constructionValues);
					if (!resultValues.success) {
						throw std::runtime_error{ "Failed to construct that set." };
					}
					auto newSet = constructSimdHashSetFinalPtrs<key_type, value_type, actualCount>[resultValues.maxSizeIndex](pairsNew, resultValues);
					return { newSet };
				}
			}

			controlBytes[slot] = ctrlByte;
			slots[x]		   = slot;
		}

		constructionValues.emplace_back(set_construction_values{ constructionValuesNew.maxStringLength, constructionValuesNew.minStringLength,
			constructionValuesNew.shortestSuccess, constructionValuesNew.maxSizeIndex, constructionValuesNew.stringLength, true, seed });
		if constexpr (constructionValuesNew.stringLength < constructionValuesNew.maxStringLength - 1) {
			return constructSimdHashSet<constructionValuesNew.incrementStringLength(), key_type, value_type, actualCount, retries>(pairsNew, prng, constructionValues);
		} else if constexpr (constructionValuesNew.maxSizeIndex < std::size(simdHashSetMaxSizes) - 1) {
			return constructSimdHashSet<constructionValuesNew.incrementSizeIndex(), key_type, value_type, actualCount, retries>(pairsNew, prng, constructionValues);
		} else {
			auto resultValues = collectOptimalConstructionValues(constructionValues);
			auto newSet		  = constructSimdHashSetFinalPtrs<key_type, value_type, actualCount>[resultValues.maxSizeIndex](pairsNew, resultValues);
			return { newSet };
		}
		return {};
	}

	template<const jsonifier::string_view& S, bool CheckSize = true> JSONIFIER_INLINE constexpr bool cxStringCmp(const jsonifier::string_view key) noexcept {
		if (!std::is_constant_evaluated()) {
			constexpr auto size = S.size();
			if constexpr (CheckSize) {
				return (size == key.size()) && compare<size>(S.data(), key.data());
			} else {
				return compare<size>(S.data(), key.data());
			}
		} else {
			return key == S;
		}
	}

	template<const jsonifier::string_view& lhs> JSONIFIER_INLINE constexpr bool compareSv(const jsonifier::string_view rhs) noexcept {
		if (!std::is_constant_evaluated()) {
			constexpr auto size = lhs.size();
			return (size == rhs.size()) && compare<size>(lhs.data(), rhs.data());
		} else {
			return lhs == rhs;
		}
	}

	JSONIFIER_INLINE constexpr bool compareSv(const jsonifier::string_view& lhs, const jsonifier::string_view rhs) noexcept {
		if (!std::is_constant_evaluated()) {
			return (lhs.size() == rhs.size()) && compare(lhs.data(), rhs.data(), lhs.size());
		} else {
			return lhs == rhs;
		}
	}

	template<typename value_type, const jsonifier::string_view& S> struct micro_set1 {
		value_type items[1]{};

		JSONIFIER_INLINE constexpr decltype(auto) begin() const noexcept {
			return static_cast<const value_type*>(items);
		}

		JSONIFIER_INLINE constexpr decltype(auto) end() const noexcept {
			return static_cast<const value_type*>(items + 1);
		}

		template<typename key_type> JSONIFIER_INLINE constexpr decltype(auto) find(key_type&& key) const noexcept {
			if (compareSv<S>(key)) [[likely]] {
				return static_cast<const value_type*>(items);
			} else [[unlikely]] {
				return static_cast<const value_type*>(items + 1);
			}
		}
	};

	template<typename value_type, const jsonifier::string_view& S0, const jsonifier::string_view& S1> struct micro_set2 {
		value_type items[2]{};

		static constexpr bool sameSize	 = S0.size() == S1.size();
		static constexpr bool checkSize = !sameSize;

		JSONIFIER_INLINE constexpr decltype(auto) begin() const noexcept {
			return static_cast<const value_type*>(items);
		}

		JSONIFIER_INLINE constexpr decltype(auto) end() const noexcept {
			return static_cast<const value_type*>(items + 2);
		}

		template<typename key_type> JSONIFIER_INLINE constexpr decltype(auto) find(key_type&& key) const noexcept {
			if constexpr (sameSize) {
				constexpr auto n = S0.size();
				if (key.size() != n) {
					return static_cast<const value_type*>(items + 2);
				}
			}

			if (cxStringCmp<S0, checkSize>(key)) {
				return static_cast<const value_type*>(items);
			} else if (cxStringCmp<S1, checkSize>(key)) {
				return static_cast<const value_type*>(items) + 1;
			} else [[unlikely]] {
				return static_cast<const value_type*>(items + 2);
			}
		}
	};

	struct single_char_hash_desc {
		size_t N{};
		bool valid{};
		uint8_t minDiff{};
		uint8_t front{};
		uint8_t back{};
		bool isFrontHash = true;
		bool isSumHash   = false;
	};

	struct single_char_hash_opts {
		bool isFrontHash = true;
		bool isSumHash   = false;
	};

	template<size_t N, single_char_hash_opts options = single_char_hash_opts{}>
		requires(N < 256)
	JSONIFIER_INLINE constexpr single_char_hash_desc single_char_hash(const std::array<jsonifier::string_view, N>& v) noexcept {
		std::array<uint8_t, N> hashes;
		for (size_t i = 0; i < N; ++i) {
			if (v[i].size() == 0) {
				return {};
			}
			if constexpr (options.isFrontHash) {
				if constexpr (options.isSumHash) {
					hashes[i] = uint8_t(v[i][0]) + uint8_t(v[i].size());
				} else {
					hashes[i] = uint8_t(v[i][0]);
				}
			} else {
				hashes[i] = uint8_t(v[i].back());
			}
		}

		std::sort(hashes.begin(), hashes.end());

		uint8_t minDiff = (std::numeric_limits<uint8_t>::max)();
		for (size_t i = 0; i < N - 1; ++i) {
			const auto diff = uint8_t(hashes[i + 1] - hashes[i]);
			if (diff == 0) {
				return {};
			}
			if (diff < minDiff) {
				minDiff = diff;
			}
		}

		return single_char_hash_desc{ N, minDiff > 0, minDiff, hashes.front(), hashes.back(), options.isFrontHash, options.isSumHash };
	}

	template<class value_type, single_char_hash_desc D>
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

			const auto k = [&]() -> uint8_t {
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
			const auto index = table[k];
			const auto& item = items[index];
			if (!compareSv(item.first, key))
				return items.end();
			return items.begin() + index;
		}
	};

	template<class value_type, single_char_hash_desc D>
		requires(D.N < 256)
	constexpr auto make_single_char_map(std::initializer_list<std::pair<jsonifier::string_view, value_type>> pairs) {
		constexpr auto N = D.N;
		if (pairs.size() != N) {
			std::abort();
		}
		single_char_map<value_type, D> ht{};

		uint8_t i = 0;
		for (const auto& pair: pairs) {
			ht.items[i]		= pair;
			const auto& key = pair.first;
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
		constexpr auto keyStatsVal{ keyStats<value_type>() };
		if constexpr (n == 0) {
			return nullptr;
		} else if constexpr (n == 1) {
			return micro_set1<value_t, core_sv<value_type, I>::value...>{ getValue<value_type, I>()... };
		} else if constexpr (n == 2) {
			return micro_set2<value_t, core_sv<value_type, I>::value...>{ getValue<value_type, I>()... };
		}  else {
			constexpr auto difference = keyStatsVal.maxLength - keyStatsVal.minLength;
			constexpr auto setConValues{ set_construction_values::constructValues<n>(keyStatsVal.minLength, difference > 0 ? keyStatsVal.maxLength : keyStatsVal.maxLength + 1) };
			constexpr auto setNew{ constructSimdHashSet<setConValues, jsonifier::string_view, value_t, n>({ keyValue<value_type, I>()... }) };
			constexpr auto newIndex = setNew.index();
			return std::get<newIndex>(setNew);
		}
	}

	template<typename value_type> constexpr auto makeSet() {
		constexpr auto indices = std::make_index_sequence<std::tuple_size_v<jsonifier::concepts::core_t<value_type>>>{};
		return makeSetImpl<jsonifier::concepts::decay_keep_volatile_t<value_type>>(indices);
	}
}