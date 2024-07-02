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
		size_t state[4]{};

		constexpr xoshiro256() {
			auto x = 7185499250578500046ull >> 12ull;
			x ^= x << 25ull;
			x ^= x >> 27ull;
			size_t s = x * 0x2545F4914F6CDD1Dull;
			for (uint64_t y = 0; y < 4; ++y) {
				state[y] = splitmix64(s);
			}
		}

		constexpr size_t operator()() {
			const size_t result = rotl(state[1ull] * 5ull, 7ull) * 9ull;

			const size_t t = state[1ull] << 17ull;

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

	constexpr size_t simdHashMapMaxSizes[]{ 16, 32, 64, 128, 256, 512, 1024 };
	constexpr size_t minimalCharHashMapMaxSizes[]{ 256 };

	struct map_construction_values {
		size_t stringLength{ std::numeric_limits<size_t>::max() };
		size_t maxSizeIndex{};
		bool success{};
		size_t seed{};
	};

	template<size_t length> struct map_simd {
		using type = std::conditional_t<length >= 64 && bytesPerStep >= 64, simd_int_512, std::conditional_t<length >= 32 && bytesPerStep >= 32, simd_int_256, simd_int_128>>;
	};

	template<size_t length> constexpr size_t setSimdWidth() {
		return length >= 64 && bytesPerStep >= 64 ? 64 : length >= 32 && bytesPerStep >= 32 ? 32 : 16;
	}

	template<size_t length> using map_simd_t = map_simd<length>::type;

	template<typename string_type01, typename string_type02> JSONIFIER_INLINE constexpr bool compareSvConst(const string_type01& lhs, const string_type02& rhs) noexcept {
		return lhs == rhs;
	}

	template<typename string_type01, typename string_type02> JSONIFIER_INLINE constexpr bool compareSvNonConst(const string_type01& lhs, const string_type02& rhs) noexcept {
		return lhs.size() == rhs.size() && compare(lhs.data(), rhs.data(), lhs.size());
	}

	template<typename key_type_new, typename value_type_new, size_t actualCount, size_t storageSizeNew> struct simd_hash_map : public key_hasher {
		using simd_type		= map_simd_t<storageSizeNew>;
		using key_type		= key_type_new;
		using value_type	= value_type_new;
		using const_pointer = const value_type*;
		using size_type		= uint64_t;
		using control_type	= uint8_t;
		static constexpr size_type storageSize{ storageSizeNew };
		static constexpr size_type bucketSize = setSimdWidth<storageSize>();
		static constexpr size_type numGroups  = storageSize > bucketSize ? storageSize / bucketSize : 1;
		JSONIFIER_ALIGN std::pair<key_type, value_type> items[storageSize + 1]{};
		JSONIFIER_ALIGN control_type controlBytes[storageSize]{};
		JSONIFIER_ALIGN size_type stringLength{};

		constexpr simd_hash_map() noexcept = default;

		JSONIFIER_INLINE constexpr const_pointer begin() const noexcept {
			return &items->second;
		}

		JSONIFIER_INLINE constexpr const_pointer end() const noexcept {
			return &(items + storageSize)->second;
		}

		JSONIFIER_INLINE constexpr auto size() const noexcept {
			return actualCount;
		}

		template<typename key_type_newer> JSONIFIER_INLINE constexpr const_pointer find(key_type_newer&& key) const noexcept {
			if (!std::is_constant_evaluated()) {
				JSONIFIER_ALIGN const auto keySize	   = key.size();
				JSONIFIER_ALIGN const auto hash		   = hashKeyRt(key.data(), keySize > stringLength ? stringLength : keySize);
				JSONIFIER_ALIGN const auto resultIndex = ((hash >> 7) % numGroups) * bucketSize;
				JSONIFIER_ALIGN const auto finalIndex  = (simd_internal::tzcnt(simd_internal::opCmpEq(simd_internal::gatherValue<simd_type>(static_cast<control_type>(hash)),
															  simd_internal::gatherValues<simd_type>(controlBytes + resultIndex))) %
																 bucketSize +
															 resultIndex) %
					storageSize;
				return LIKELY(compareSvNonConst(items[finalIndex].first, key)) ? &(items + finalIndex)->second : end();
			} else {
				JSONIFIER_ALIGN const auto keySize	   = key.size();
				JSONIFIER_ALIGN const auto hash		   = hashKeyCt(key.data(), keySize > stringLength ? stringLength : keySize);
				JSONIFIER_ALIGN const auto resultIndex = ((hash >> 7) % numGroups) * bucketSize;
				JSONIFIER_ALIGN const auto finalIndex  = (constMatch(controlBytes + resultIndex, static_cast<control_type>(hash)) % bucketSize + resultIndex) % storageSize;
				return LIKELY(compareSvConst(items[finalIndex].first, key)) ? &(items + finalIndex)->second : end();
			}
		}

	  protected:
		constexpr size_type tzcnt(size_type value) const {
			size_type count{};
			while ((value & 1) == 0 && value != 0) {
				value >>= 1;
				++count;
			}
			return count;
		}

		constexpr size_type constMatch(const control_type* hashData, control_type hash) const {
			size_type mask = 0;
			for (size_type x = 0; x < bucketSize; ++x) {
				if (hashData[x] == hash) {
					mask |= (1ull << x);
				}
			}
			return tzcnt(mask);
		}
	};

	template<size_t startingValue, size_t actualCount, typename key_type, typename value_type, template<typename, typename, size_t, size_t> typename map_type> using map_variant =
		std::variant<map_type<key_type, value_type, actualCount, startingValue>, map_type<key_type, value_type, actualCount, startingValue * 2ull>,
			map_type<key_type, value_type, actualCount, startingValue * 4ull>, map_type<key_type, value_type, actualCount, startingValue * 8ull>,
			map_type<key_type, value_type, actualCount, startingValue * 16ull>, map_type<key_type, value_type, actualCount, startingValue * 32ull>,
			map_type<key_type, value_type, actualCount, startingValue * 64ull>>;

	template<typename key_type, typename value_type, size_t actualCount, size_t storageSize>
	constexpr auto constructSimdHashMapFinal(const std::array<std::pair<key_type, value_type>, actualCount>& pairsNew, map_construction_values constructionValues)
		-> map_variant<16, actualCount, key_type, value_type, simd_hash_map> {
		constexpr size_t bucketSize = setSimdWidth<storageSize>();
		constexpr size_t numGroups	= storageSize > bucketSize ? storageSize / bucketSize : 1;
		simd_hash_map<key_type, value_type, actualCount, storageSize> simdHashMapNew{};
		simdHashMapNew.setSeed(constructionValues.seed);
		simdHashMapNew.stringLength = constructionValues.stringLength;
		std::array<size_t, numGroups> bucketSizes{};
		for (size_t x = 0; x < actualCount; ++x) {
			const auto keySize				  = pairsNew[x].first.size();
			const auto stringLengthNew		  = simdHashMapNew.stringLength > keySize ? keySize : simdHashMapNew.stringLength;
			const auto hash					  = simdHashMapNew.hashKeyCt(pairsNew[x].first.data(), stringLengthNew);
			const auto groupPos				  = (hash >> 7) % numGroups;
			const auto ctrlByte				  = static_cast<uint8_t>(hash);
			const auto bucketSizeNew		  = ++bucketSizes[groupPos];
			const auto slot					  = ((groupPos * bucketSize) + bucketSizeNew) % storageSize;
			simdHashMapNew.items[slot]		  = pairsNew[x];
			simdHashMapNew.controlBytes[slot] = ctrlByte;
		}

		return map_variant<16, actualCount, key_type, value_type, simd_hash_map>{ simd_hash_map<key_type, value_type, actualCount, storageSize>(simdHashMapNew) };
	}

	template<typename key_type, typename value_type, size_t actualCount> using construct_simd_hash_map_function_ptr =
		decltype(&constructSimdHashMapFinal<key_type, value_type, actualCount, 16ull>);

	template<typename key_type, typename value_type, size_t actualCount>
	constexpr construct_simd_hash_map_function_ptr<key_type, value_type, actualCount> constructSimdHashMapFinalPtrs[7] = {
		&constructSimdHashMapFinal<key_type, value_type, actualCount, 16ull>, &constructSimdHashMapFinal<key_type, value_type, actualCount, 32ull>,
		&constructSimdHashMapFinal<key_type, value_type, actualCount, 64ull>, &constructSimdHashMapFinal<key_type, value_type, actualCount, 128ull>,
		&constructSimdHashMapFinal<key_type, value_type, actualCount, 256ull>, &constructSimdHashMapFinal<key_type, value_type, actualCount, 512ull>,
		&constructSimdHashMapFinal<key_type, value_type, actualCount, 1024ull>
	};

	template<typename key_type, typename value_type, size_t actualCount>
	constexpr key_stats_t getKeyStats(const std::array<std::pair<key_type, value_type>, actualCount>& pairsNew) {
		uint64_t minLength{ std::numeric_limits<uint64_t>::max() };
		uint64_t maxLength{ std::numeric_limits<uint64_t>::min() };
		for (uint64_t x = 0; x < actualCount; ++x) {
			const auto keySize = pairsNew[x].first.size();
			if (keySize < minLength) {
				minLength = keySize;
			}
			if (keySize > maxLength) {
				maxLength = keySize;
			}
		}
		if (minLength == maxLength && minLength > 0) {
			++maxLength;
			--minLength;
		}
		return { minLength, maxLength - minLength, maxLength };
	}

	template<uint64_t maxSizeIndex, typename key_type, typename value_type, size_t actualCount>
	constexpr auto constructSimdHashMapHelper(const std::array<std::pair<key_type, value_type>, actualCount>& pairsNew, xoshiro256 prng, key_stats_t& keyStatsVal) {
		constexpr size_t bucketSize		   = setSimdWidth<simdHashMapMaxSizes[maxSizeIndex]>();
		constexpr size_t storageSize	   = simdHashMapMaxSizes[maxSizeIndex];
		constexpr size_t numGroups		   = storageSize > bucketSize ? storageSize / bucketSize : 1;
		auto constructForGivenStringLength = [&](uint64_t stringLength) mutable -> map_construction_values {
			uint8_t controlBytes[storageSize]{};
			size_t bucketSizes[numGroups]{};
			size_t slots[storageSize]{};
			key_hasher hasherNew{};
			bool collided{};
			size_t seed{};
			std::fill(slots, slots + storageSize, std::numeric_limits<uint64_t>::max());
			for (uint64_t x = 0; x < 3; ++x) {
				seed = prng();
				hasherNew.setSeed(seed);
				collided = false;
				for (size_t y = 0; y < actualCount; ++y) {
					const auto keySize		 = pairsNew[y].first.size();
					const auto hash			 = hasherNew.hashKeyCt(pairsNew[y].first.data(), keySize > stringLength ? stringLength : keySize);
					const auto groupPos		 = (hash >> 7) % numGroups;
					const auto ctrlByte		 = static_cast<uint8_t>(hash);
					const auto bucketSizeNew = ++bucketSizes[groupPos];
					const auto slot			 = ((groupPos * bucketSize) + bucketSizeNew) % storageSize;

					if (bucketSizeNew >= bucketSize || contains(slots + groupPos * bucketSize, slot, bucketSize) ||
						contains(controlBytes + groupPos * bucketSize, ctrlByte, bucketSize)) {
						std::fill(slots, slots + storageSize, std::numeric_limits<uint64_t>::max());
						std::fill(controlBytes, controlBytes + storageSize, 0);
						std::fill(bucketSizes, bucketSizes + numGroups, 0);
						collided = true;
						break;
					}

					controlBytes[slot] = ctrlByte;
					slots[y]		   = slot;
				}
				if (!collided) {
					break;
				}
			}
			map_construction_values returnValues{};
			if (collided) {
				returnValues.success = false;
				return returnValues;
			}
			returnValues.maxSizeIndex = maxSizeIndex;
			returnValues.seed		  = seed;
			returnValues.stringLength = stringLength;
			returnValues.success	  = true;
			return returnValues;
		};
		map_construction_values bestValues{};
		if constexpr (maxSizeIndex < std::size(simdHashMapMaxSizes) - 1) {
			auto newValues = constructSimdHashMapHelper<maxSizeIndex + 1, key_type, value_type, actualCount>(pairsNew, prng, keyStatsVal);
			if (newValues.success && newValues.stringLength < bestValues.stringLength) {
				bestValues = newValues;
			}
		}
		for (uint64_t x = keyStatsVal.maxLength; x >= keyStatsVal.minLength; --x) {
			auto newValues = constructForGivenStringLength(x);
			if (newValues.success && newValues.stringLength <= bestValues.stringLength) {
				bestValues			  = newValues;
				keyStatsVal.maxLength = x;
			} else {
				break;
			}
		}
		return bestValues;
	}

	template<typename key_type, typename value_type, size_t actualCount>
	constexpr auto constructSimdHashMap(const std::array<std::pair<key_type, value_type>, actualCount>& pairsNew) {
		auto keyStatsVal = getKeyStats(pairsNew);
		auto constructionValues =
			constructSimdHashMapHelper<getMaxSizeIndex<actualCount>(simdHashMapMaxSizes), key_type, value_type, actualCount>(pairsNew, xoshiro256{}, keyStatsVal);
		return constructSimdHashMapFinalPtrs<key_type, value_type, actualCount>[constructionValues.maxSizeIndex](pairsNew, constructionValues);
	}

	template<typename key_type_new, typename value_type_new, size_t actualCount, size_t storageSize> struct minimal_char_hash_map : public key_hasher {
		using key_type					   = key_type_new;
		using value_type				   = value_type_new;
		using const_pointer				   = const value_type*;
		using size_type					   = uint64_t;
		static constexpr size_t bucketSize = 16;
		static constexpr size_t numGroups  = storageSize > bucketSize ? storageSize / bucketSize : 1;
		JSONIFIER_ALIGN std::pair<key_type, value_type> items[storageSize + 1]{};
		JSONIFIER_ALIGN uint64_t stringLength{};

		constexpr minimal_char_hash_map() noexcept = default;

		JSONIFIER_INLINE constexpr const_pointer begin() const noexcept {
			return &items->second;
		}

		JSONIFIER_INLINE constexpr const_pointer end() const noexcept {
			return &(items + storageSize)->second;
		}

		JSONIFIER_INLINE constexpr auto size() const noexcept {
			return actualCount;
		}

		template<typename key_type_newer> JSONIFIER_INLINE constexpr const_pointer find(key_type_newer&& key) const noexcept {
			if (!std::is_constant_evaluated()) {
				JSONIFIER_ALIGN const auto keySize		   = key.size();
				JSONIFIER_ALIGN const auto stringLengthNew = keySize > stringLength ? stringLength : keySize;
				JSONIFIER_ALIGN const auto hash			   = hashKeyRtSingle(key.data(), 1) + (key.data()[stringLengthNew - 1] * keySize);
				JSONIFIER_ALIGN const auto groupPos		   = hash % numGroups;
				JSONIFIER_ALIGN const auto ctrlByte		   = static_cast<uint8_t>(hash);
				JSONIFIER_ALIGN const auto finalIndex	   = ((groupPos * bucketSize) + ctrlByte) % storageSize;
				return LIKELY(compareSvNonConst(items[finalIndex].first, key)) ? &(items + finalIndex)->second : end();
			} else {
				JSONIFIER_ALIGN const auto keySize		   = key.size();
				JSONIFIER_ALIGN const auto stringLengthNew = keySize > stringLength ? stringLength : keySize;
				JSONIFIER_ALIGN const auto hash			   = hashKeyCtSingle(key.data(), 1) + (key.data()[stringLengthNew - 1] * keySize);
				JSONIFIER_ALIGN const auto groupPos		   = hash % numGroups;
				JSONIFIER_ALIGN const auto ctrlByte		   = static_cast<uint8_t>(hash);
				JSONIFIER_ALIGN const auto finalIndex	   = ((groupPos * bucketSize) + ctrlByte) % storageSize;
				return LIKELY(compareSvConst(items[finalIndex].first, key)) ? &(items + finalIndex)->second : end();
			}
		}

	  protected:
		constexpr uint64_t tzcnt(uint64_t value) const {
			uint64_t count{};
			while ((value & 1) == 0 && value != 0) {
				value >>= 1;
				++count;
			}
			return count;
		}

		constexpr uint64_t constMatch(const uint8_t* hashData, uint8_t hash) const {
			uint64_t mask = 0;
			for (size_t x = 0; x < bucketSize; ++x) {
				if (hashData[x] == hash) {
					mask |= (1ull << x);
				}
			}
			return tzcnt(mask);
		}
	};

	template<typename key_type, typename value_type, size_t actualCount, size_t storageSize>
	constexpr auto constructMinimalCharHashMapFinal(const std::array<std::pair<key_type, value_type>, actualCount>& pairsNew, map_construction_values constructionValues)
		-> map_variant<256ull, actualCount, key_type, value_type, minimal_char_hash_map> {
		constexpr size_t bucketSize = 16;
		constexpr size_t numGroups	= storageSize > bucketSize ? storageSize / bucketSize : 1;
		minimal_char_hash_map<key_type, value_type, actualCount, storageSize> minimalCharHashMapNew{};
		minimalCharHashMapNew.setSeed(constructionValues.seed);
		minimalCharHashMapNew.stringLength = constructionValues.stringLength;
		for (size_t x = 0; x < actualCount; ++x) {
			const auto keySize				  = pairsNew[x].first.size();
			const auto stringLengthNew		  = keySize > minimalCharHashMapNew.stringLength ? minimalCharHashMapNew.stringLength : keySize;
			const auto hash					  = minimalCharHashMapNew.hashKeyCtSingle(pairsNew[x].first.data(), 1) + (pairsNew[x].first.data()[stringLengthNew - 1] * keySize);
			const auto groupPos				  = hash % numGroups;
			const auto ctrlByte				  = static_cast<uint8_t>(hash);
			const auto slot					  = ((groupPos * bucketSize) + ctrlByte) % storageSize;
			minimalCharHashMapNew.items[slot] = pairsNew[x];
		}

		return map_variant<256ull, actualCount, key_type, value_type, minimal_char_hash_map>{ minimal_char_hash_map<key_type, value_type, actualCount, storageSize>(
			minimalCharHashMapNew) };
	}

	template<typename key_type, typename value_type, size_t actualCount> using construct_minimal_char_hash_map_function_ptr =
		decltype(&constructMinimalCharHashMapFinal<key_type, value_type, actualCount, 256ull>);

	template<typename key_type, typename value_type, size_t actualCount>
	constexpr construct_minimal_char_hash_map_function_ptr<key_type, value_type, actualCount> constructMinimalCharHashMapFinalPtrs[7] = {
		&constructMinimalCharHashMapFinal<key_type, value_type, actualCount, 256ull>
	};

	template<uint64_t maxSizeIndex, typename key_type, typename value_type, size_t actualCount>
	constexpr auto constructMinimalCharHashMapHelper(const std::array<std::pair<key_type, value_type>, actualCount>& pairsNew, xoshiro256 prng, key_stats_t& keyStatsVal) {
		constexpr size_t bucketSize		   = 16;
		constexpr size_t storageSize	   = minimalCharHashMapMaxSizes[maxSizeIndex];
		constexpr size_t numGroups		   = storageSize > bucketSize ? storageSize / bucketSize : 1;
		auto constructForGivenStringLength = [&](uint64_t stringLength) mutable -> map_construction_values {
			uint8_t controlBytes[storageSize]{};
			size_t bucketSizes[numGroups]{};
			size_t slots[storageSize]{};
			key_hasher hasherNew{};
			bool collided{};
			size_t seed{};
			std::fill(controlBytes, controlBytes + storageSize, std::numeric_limits<uint8_t>::max());
			std::fill(slots, slots + storageSize, std::numeric_limits<uint64_t>::max());
			for (uint64_t x = 0; x < 3; ++x) {
				seed = prng();
				hasherNew.setSeed(seed);
				for (size_t y = 0; y < actualCount; ++y) {
					collided				   = false;
					const auto keySize		   = pairsNew[y].first.size();
					const auto stringLengthNew = keySize > stringLength ? stringLength : keySize;
					const auto hash			   = hasherNew.hashKeyCtSingle(pairsNew[y].first.data(), 1) + (pairsNew[y].first.data()[stringLengthNew - 1] * keySize);
					const auto groupPos		   = hash % numGroups;
					const auto bucketSizeNew   = ++bucketSizes[groupPos];
					const auto ctrlByte		   = static_cast<uint8_t>(hash);
					const auto slot			   = ((groupPos * bucketSize) + ctrlByte) % storageSize;

					if (bucketSizeNew >= bucketSize || contains(slots, slot, storageSize) || contains(controlBytes, ctrlByte, storageSize)) {
						std::fill(controlBytes, controlBytes + storageSize, std::numeric_limits<uint8_t>::max());
						std::fill(slots, slots + storageSize, std::numeric_limits<uint64_t>::max());
						std::fill(bucketSizes, bucketSizes + numGroups, 0);
						collided = true;
						break;
					}
					slots[y] = slot;
				}
				if (!collided) {
					break;
				}
			}
			map_construction_values returnValues{};
			if (collided) {
				returnValues.success = false;
				return returnValues;
			}
			returnValues.maxSizeIndex = maxSizeIndex;
			returnValues.seed		  = seed;
			returnValues.stringLength = stringLength;
			returnValues.success	  = true;
			return returnValues;
		};
		map_construction_values bestValues{};

		if constexpr (maxSizeIndex < std::size(minimalCharHashMapMaxSizes) - 1) {
			auto newValues = constructMinimalCharHashMapHelper<maxSizeIndex + 1, key_type, value_type, actualCount>(pairsNew, prng, keyStatsVal);
			if (newValues.success && newValues.stringLength <= bestValues.stringLength) {
				bestValues = newValues;
			}
		}
		for (uint64_t x = keyStatsVal.maxLength; x >= keyStatsVal.minLength; --x) {
			auto newValues = constructForGivenStringLength(x);
			if (newValues.success && newValues.stringLength <= bestValues.stringLength) {
				bestValues			  = newValues;
				keyStatsVal.maxLength = x;
			} else {
				break;
			}
		}
		return bestValues;
	}

	template<typename key_type, typename value_type, size_t actualCount>
	constexpr auto constructMinimalCharHashMap(const std::array<std::pair<key_type, value_type>, actualCount>& pairsNew) {
		auto keyStatsVal = getKeyStats(pairsNew);
		auto constructionValues =
			constructMinimalCharHashMapHelper<getMaxSizeIndex<actualCount>(minimalCharHashMapMaxSizes), key_type, value_type, actualCount>(pairsNew, xoshiro256{}, keyStatsVal);
		return constructMinimalCharHashMapFinalPtrs<key_type, value_type, actualCount>[constructionValues.maxSizeIndex](pairsNew, constructionValues);
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

	template<typename value_type, const auto& S> struct micro_map1 {
		value_type items[1]{};

		JSONIFIER_INLINE constexpr const value_type* begin() const noexcept {
			return items;
		}

		JSONIFIER_INLINE constexpr const value_type* end() const noexcept {
			return items + 1;
		}

		template<typename key_type> JSONIFIER_INLINE constexpr const value_type* find(key_type&& key) const noexcept {
			if (compareSv<S>(key)) [[likely]] {
				return items;
			} else [[unlikely]] {
				return items + 1;
			}
		}
	};

	template<typename value_type, const auto& S0, const auto& S1> struct micro_map2 {
		value_type items[2]{};

		static constexpr bool sameSize	= S0.size() == S1.size();
		static constexpr bool checkSize = !sameSize;

		JSONIFIER_INLINE constexpr const value_type* begin() const noexcept {
			return items;
		}

		JSONIFIER_INLINE constexpr const value_type* end() const noexcept {
			return items + 2;
		}

		template<typename key_type> JSONIFIER_INLINE constexpr const value_type* find(key_type&& key) const noexcept {
			if constexpr (sameSize) {
				constexpr auto n = S0.size();
				if (key.size() != n) {
					return items + 2;
				}
			}

			if (cxStringCmp<S0, checkSize>(key)) {
				return items;
			} else if (cxStringCmp<S1, checkSize>(key)) {
				return items + 1;
			} else [[unlikely]] {
				return items + 2;
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

	template<typename tuple_type, typename = std::make_index_sequence<std::tuple_size<tuple_type>::value>> struct value_tuple_variant;

	template<typename tuple_type, size_t I> struct member_type {
		using T0   = std::tuple_element_t<0, std::tuple_element_t<I, tuple_type>>;
		using type = std::tuple_element_t<std::is_member_pointer_v<T0> ? 0 : 1, std::tuple_element_t<I, tuple_type>>;
	};

	template<typename tuple_type, size_t... I> struct value_tuple_variant<tuple_type, std::index_sequence<I...>> {
		using type = typename tuple_variant<decltype(std::tuple_cat(std::declval<std::tuple<typename member_type<tuple_type, I>::type>>()...))>::type;
	};

	template<typename tuple_type> using value_tuple_variant_t = typename value_tuple_variant<tuple_type>::type;

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

	template<typename value_type, size_t... I> constexpr auto makeMapImpl(std::index_sequence<I...>) {
		using value_t	 = value_tuple_variant_t<jsonifier::concepts::core_t<value_type>>;
		constexpr auto n = std::tuple_size_v<jsonifier::concepts::core_t<value_type>>;
		if constexpr (n == 0) {
			return nullptr;
		} else if constexpr (n == 1) {
			return micro_map1<value_t, core_sv<value_type, I>::value...>{ getValue<value_type, I>()... };
		} else if constexpr (n == 2) {
			return micro_map2<value_t, core_sv<value_type, I>::value...>{ getValue<value_type, I>()... };
		} else if constexpr (n < 16) {
			constexpr auto mapNew{ constructMinimalCharHashMap<jsonifier::string_view, value_t, n>({ keyValue<value_type, I>()... }) };
			constexpr auto newIndex = mapNew.index();
			constexpr auto newMap	= std::get<newIndex>(mapNew);
			if constexpr (newMap.stringLength == std::numeric_limits<uint64_t>::max()) {
				constexpr auto mapNewer{ constructSimdHashMap<jsonifier::string_view, value_t, n>({ keyValue<value_type, I>()... }) };
				constexpr auto newIndexer = mapNewer.index();
				return std::get<newIndexer>(mapNewer);
			} else {
				return newMap;
			}
		} else {
			constexpr auto mapNew{ constructSimdHashMap<jsonifier::string_view, value_t, n>({ keyValue<value_type, I>()... }) };
			constexpr auto newIndex = mapNew.index();
			return std::get<newIndex>(mapNew);
		}
	}

	template<typename value_type> constexpr auto makeMap() {
		constexpr auto indices = std::make_index_sequence<std::tuple_size_v<jsonifier::concepts::core_t<value_type>>>{};
		return makeMapImpl<jsonifier::concepts::decay_keep_volatile_t<value_type>>(indices);
	}
}