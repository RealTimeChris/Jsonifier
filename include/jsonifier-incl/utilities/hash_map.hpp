/*
	MIT License

	Copyright (c) 2024 RealTimeChris

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

#include <jsonifier-incl/core/core.hpp>
#include <jsonifier-incl/utilities/hash.hpp>
#include <jsonifier-incl/utilities/string_view.hpp>
#include <jsonifier-incl/containers/array.hpp>
#include <jsonifier-incl/utilities/reflection.hpp>
#include <jsonifier-incl/containers/tuple.hpp>
#include <algorithm>
#include <numeric>
#include <utility>

namespace std {

	template<jsonifier::concepts::string_t string_type> struct hash<string_type> : public std::hash<std::string_view> {
		uint64_t operator()(const string_type& stringNew) const noexcept {
			return std::hash<std::string_view>::operator()(std::string_view{ stringNew });
		}
	};
}

namespace jsonifier::internal {

	template<typename value_type, uint64_t size> std::ostream& operator<<(std::ostream& os, const array<value_type, size>& values) {
		os << "[";
		for (uint64_t x = 0; x < size; ++x) {
			os << values[x];
			if (x < size - 1) {
				os << ",";
			}
		}
		os << "]";
		return os;
	}

	template<typename value_type01, typename value_type02> inline static constexpr bool contains(const value_type01* hashData, value_type02 byteToCheckFor, uint64_t size) noexcept {
		for (uint64_t x = 0; x < size; ++x) {
			if (hashData[x] == byteToCheckFor) {
				return true;
			}
		}
		return false;
	}

	template<uint64_t length> struct map_simd {
		using type = jsonifier::internal::conditional_t<length >= 64 && bytesPerStep >= 64, avx_type_wrapper<avx_type::m512>,
			jsonifier::internal::conditional_t<length >= 32 && bytesPerStep >= 32, avx_type_wrapper<avx_type::m256>, avx_type_wrapper<avx_type::m128>>>;
	};

	template<uint64_t length> using map_simd_t = typename map_simd<length>::type::type;

	enum class hash_map_type {
		unset						= 0,
		empty						= 1,
		single_element				= 2,
		double_element				= 3,
		triple_element				= 4,
		single_byte					= 5,
		first_byte_and_unique_index = 6,
		unique_byte_and_length		= 7,
		unique_per_length			= 8,
		simd_full_length			= 9,
	};

	inline static constexpr uint64_t setSimdWidth(uint64_t length) noexcept {
		return length >= 64ull && bytesPerStep >= 64ull ? 64ull : length >= 32ull && bytesPerStep >= 32ull ? 32ull : 16ull;
	}

	struct key_stats_t {
		uint64_t minLength{ (std::numeric_limits<uint64_t>::max)() };
		uint64_t uniqueIndex{};
		uint64_t maxLength{};
	};

	inline static constexpr uint64_t findUniqueColumnIndex(const tuple_references& tupleRefsRaw, uint64_t maxIndex, uint64_t startingIndex = 0) noexcept {
		constexpr uint64_t alphabetSize = 256;
		string_view key{};
		for (uint64_t index = startingIndex; index < maxIndex; ++index) {
			array<bool, alphabetSize> seen{};
			bool allDifferent = true;

			for (uint64_t x = 0; x < tupleRefsRaw.count; ++x) {
				key				  = tupleRefsRaw.rootPtr[x].key;
				const char c	  = key[index];
				uint8_t charIndex = static_cast<uint8_t>(c);

				if (seen[charIndex]) {
					allDifferent = false;
					break;
				}
				seen[charIndex] = true;
			}

			if (allDifferent) {
				return index;
			}
		}

		return std::numeric_limits<uint64_t>::max();
	}

	struct hash_map_construction_data {
		using simd_type = map_simd_t<2048>;
		array<uint16_t, 2048 / setSimdWidth(2048)> bucketSizes{};
		JSONIFIER_ALIGN(64) array<uint8_t, 2049ULL> controlBytes {};
		array<uint8_t, 256ULL> uniqueIndices{};
		array<uint16_t, 2049ULL> indices{};
		uint64_t bucketSize{ setSimdWidth(2048) };
		uint64_t numGroups{ 2048 / bucketSize };
		ct_key_hasher hasher{};
		hash_map_type type{};
		uint64_t uniqueIndex{};
		char firstChar{};

		constexpr hash_map_construction_data() noexcept = default;
	};

	struct empty_data {
		constexpr empty_data(const hash_map_construction_data& newData) noexcept : type{ newData.type } {}
		hash_map_type type{};
	};

	struct single_element_data {
		inline static constexpr uint64_t storageSize{ 1 };
		constexpr single_element_data(const hash_map_construction_data& newData) noexcept : type{ newData.type } {}
		hash_map_type type{};
	};

	struct double_element_data {
		inline static constexpr uint64_t storageSize{ 2 };
		constexpr double_element_data(const hash_map_construction_data& newData) noexcept : uniqueIndex{ newData.uniqueIndex }, type{ newData.type } {}
		uint64_t uniqueIndex{};
		hash_map_type type{};
	};

	struct triple_element_data {
		inline static constexpr uint64_t storageSize{ 3 };
		constexpr triple_element_data(const hash_map_construction_data& newData) noexcept
			: uniqueIndex{ newData.uniqueIndex }, type{ newData.type }, firstChar{ newData.firstChar }, seed{ newData.hasher.seed } {}
		uint64_t uniqueIndex{};
		hash_map_type type{};
		char firstChar{};
		uint64_t seed{};
	};

	struct single_byte_data {
		inline static constexpr uint64_t storageSize{ 256 };
		constexpr single_byte_data(const hash_map_construction_data& newData) noexcept
			: uniqueIndices{ newData.uniqueIndices }, uniqueIndex{ newData.uniqueIndex }, type{ newData.type } {}
		array<uint8_t, 256ULL> uniqueIndices{};
		uint64_t uniqueIndex{};
		hash_map_type type{};
	};

	struct first_byte_and_unique_index_data {
		inline static constexpr uint64_t storageSize{ 256 };
		constexpr first_byte_and_unique_index_data(const hash_map_construction_data& newData) noexcept : uniqueIndices{ newData.uniqueIndices }, type{ newData.type } {}
		array<uint8_t, 256ULL> uniqueIndices{};
		hash_map_type type{};
	};

	struct unique_byte_and_length_data {
		inline static constexpr uint64_t storageSize{ 2048 };
		constexpr unique_byte_and_length_data(const hash_map_construction_data& newData) noexcept
			: indices{ newData.indices }, uniqueIndex{ newData.uniqueIndex }, type{ newData.type } {}
		array<uint16_t, 2049ULL> indices{};
		uint64_t uniqueIndex{};
		hash_map_type type{};
	};

	struct unique_per_length_data {
		inline static constexpr uint64_t storageSize{ 256 };
		constexpr unique_per_length_data(const hash_map_construction_data& newData) noexcept : uniqueIndices{ newData.uniqueIndices }, type{ newData.type } {}
		array<uint8_t, 256ULL> uniqueIndices{};
		hash_map_type type{};
	};

	struct simd_full_length_data {
		inline static constexpr uint64_t storageSize{ 2048 };
		constexpr simd_full_length_data(const hash_map_construction_data& newData) noexcept
			: controlBytes{ newData.controlBytes }, bucketSize{ newData.bucketSize }, numGroups{ newData.numGroups }, indices{ newData.indices },
			  uniqueIndex{ newData.uniqueIndex }, type{ newData.type }, seed{ newData.hasher.seed } {}
		JSONIFIER_ALIGN(64) array<uint8_t, storageSize + 1ULL> controlBytes {};
		char padding01[bytesPerStep - ((storageSize + 1) % 8)]{};
		uint64_t bucketSize{ setSimdWidth(storageSize) };
		uint64_t numGroups{ storageSize / bucketSize };
		array<uint16_t, storageSize + 1> indices{};
		uint64_t uniqueIndex{};
		hash_map_type type{};
		uint64_t seed{};
	};

	struct string_lengths : public tuple_references {
		uint64_t length{};
	};

	inline static constexpr uint64_t countUniqueLengths(const tuple_references& tupleRefsRaw) {
		array<uint64_t, 256ULL> stringLengths{};
		uint64_t returnValue{};
		for (uint64_t x = 0; x < tupleRefsRaw.count; ++x) {
			++stringLengths[tupleRefsRaw.rootPtr[x].key.size()];
		}
		for (uint64_t x = 0; x < 256; ++x) {
			if (stringLengths[x] > 0) {
				++returnValue;
			}
		}
		return returnValue;
	}

	template<uint64_t stringLengthCount> inline static constexpr array<string_lengths, stringLengthCount> collectLengths(const tuple_references& values) {
		array<uint64_t, 256> lengths{};
		array<string_lengths, stringLengthCount> valuesNew{};
		uint64_t currentIndex{};
		for (uint64_t x = 0; x < values.count; ++x) {
			auto& newRef = values.rootPtr[x];
			if (lengths[newRef.key.size()] == 0) {
				++lengths[newRef.key.size()];
				string_lengths tupleRefsRaw{};
				tupleRefsRaw.rootPtr	= &newRef;
				tupleRefsRaw.length		= newRef.key.size();
				valuesNew[currentIndex] = tupleRefsRaw;
				++valuesNew[currentIndex].count;
				++currentIndex;
			} else {
				for (auto& value: valuesNew) {
					if (value.length == newRef.key.size()) {
						++value.count;
					}
				}
			}
		}
		return valuesNew;
	}

	inline static constexpr uint64_t countFirstBytes(const tuple_references& tupleRefsRaw) {
		array<bool, 256ULL> stringLengths{};
		uint64_t returnValue = 0;
		for (uint64_t x = 0; x < tupleRefsRaw.count; ++x) {
			if (!tupleRefsRaw.rootPtr[x].key.empty()) {
				uint8_t firstByte = static_cast<uint8_t>(tupleRefsRaw.rootPtr[x].key[0]);
				if (!stringLengths[firstByte]) {
					++returnValue;
					stringLengths[firstByte] = true;
				}
			}
		}
		return returnValue;
	}

	struct first_bytes : public tuple_references {
		char value{};
	};

	template<uint64_t firstByteCount> inline static constexpr array<first_bytes, firstByteCount> collectFirstBytes(const tuple_references& values) {
		array<uint64_t, 256ULL> lengths{};
		array<first_bytes, firstByteCount> valuesNew{};
		uint64_t currentIndex{};
		for (uint64_t x = 0; x < values.count; ++x) {
			if (lengths[static_cast<uint8_t>(values.rootPtr[x].key[0])] == 0) {
				++lengths[static_cast<uint8_t>(values.rootPtr[x].key[0])];
				first_bytes tupleRefsRaw{};
				tupleRefsRaw.rootPtr	= &values.rootPtr[x];
				tupleRefsRaw.value		= values.rootPtr[x].key[0];
				valuesNew[currentIndex] = tupleRefsRaw;
				++valuesNew[currentIndex].count;
				++currentIndex;
			} else {
				for (auto& value: valuesNew) {
					if (value.value == values.rootPtr[x].key[0]) {
						++value.count;
					}
				}
			}
		}
		return valuesNew;
	}

	inline static constexpr key_stats_t keyStatsImpl(const tuple_references& tupleRefsRaw) noexcept {
		key_stats_t stats{};
		for (uint64_t x = 0; x < tupleRefsRaw.count; ++x) {
			const string_view& key{ tupleRefsRaw.rootPtr[x].key };
			auto num{ key.size() };
			if (num > stats.maxLength) {
				stats.maxLength = num;
			}
			if (num < stats.minLength) {
				stats.minLength = num;
			}
		}
		stats.uniqueIndex = findUniqueColumnIndex(tupleRefsRaw, stats.minLength);
		return stats;
	}

	template<uint64_t size> inline static constexpr array<key_stats_t, size> keyStats(const array<first_bytes, size>& tupleRefsRaw) noexcept {
		array<key_stats_t, size> returnValues{};
		for (uint64_t x = 0; x < size; ++x) {
			returnValues[x] = keyStatsImpl(static_cast<const tuple_references&>(tupleRefsRaw[x]));
		}
		return returnValues;
	}

	template<uint64_t size> inline static constexpr array<key_stats_t, size> keyStats(const array<string_lengths, size>& tupleRefsRaw) noexcept {
		array<key_stats_t, size> returnValues{};
		for (uint64_t x = 0; x < size; ++x) {
			returnValues[x] = keyStatsImpl(static_cast<const tuple_references&>(tupleRefsRaw[x]));
		}
		return returnValues;
	}

	template<typename value_type> inline static constexpr auto keyStatsVal = keyStatsImpl(tupleReferences<value_type>);

	template<typename value_type>
	inline static constexpr void collectSimdFullLengthHashMapData(hash_map_construction_data& returnValues, const tuple_references& pairsNew) noexcept {
		bool collided{};
		for (uint64_t w = keyStatsVal<value_type>.minLength; w < keyStatsVal<value_type>.maxLength; ++w) {
			returnValues.uniqueIndex = w;
			for (uint64_t x = 0; x < 2; ++x) {
				returnValues.controlBytes.fill(std::numeric_limits<uint8_t>::max());
				returnValues.indices.fill(static_cast<uint16_t>(returnValues.indices.size() - 1));
				returnValues.hasher.updateSeed();
				collided = false;
				for (uint64_t y = 0; y < pairsNew.count; ++y) {
					const auto keyLength	 = returnValues.uniqueIndex > pairsNew.rootPtr[y].key.size() ? pairsNew.rootPtr[y].key.size() : returnValues.uniqueIndex;
					const auto hash			 = returnValues.hasher.hashKeyCt(pairsNew.rootPtr[y].key.data(), keyLength);
					const auto groupPos		 = (hash >> 8) % returnValues.numGroups;
					const auto ctrlByte		 = static_cast<uint8_t>(hash);
					const auto bucketSizeNew = returnValues.bucketSizes[groupPos]++;
					const auto slot			 = ((groupPos * returnValues.bucketSize) + bucketSizeNew);

					if (bucketSizeNew >= returnValues.bucketSize || returnValues.indices[slot] != returnValues.indices.size() - 1 ||
						contains(returnValues.controlBytes.data() + groupPos * returnValues.bucketSize, ctrlByte, returnValues.bucketSize)) {
						returnValues.bucketSizes.fill(0);
						collided = true;
						break;
					}
					returnValues.controlBytes[slot] = ctrlByte;
					returnValues.indices[slot]		= pairsNew.rootPtr[y].oldIndex;
				}
				if (!collided) {
					break;
				}
			}
			if (!collided) {
				break;
			}
		}
		if (collided) {
			returnValues.type		 = hash_map_type::unset;
			returnValues.uniqueIndex = std::numeric_limits<uint64_t>::max();
		} else {
			returnValues.type = hash_map_type::simd_full_length;
		}
		return;
	}

	template<typename value_type> inline static constexpr void collectUniquePerLengthHashMapData(hash_map_construction_data& returnValues, const tuple_references& pairsNew) {
		constexpr auto uniqueLengthCount = countUniqueLengths(tupleReferences<value_type>);
		constexpr auto results			 = collectLengths<uniqueLengthCount>(tupleReferences<value_type>);
		constexpr auto keyStatsValNew	 = keyStats(results);
		returnValues.uniqueIndices.fill(static_cast<uint8_t>(returnValues.uniqueIndices.size() - 1));
		bool fallback = false;
		for (uint64_t x = 0; x < uniqueLengthCount; ++x) {
			auto uniqueIndex = findUniqueColumnIndex(results[x], keyStatsValNew[x].minLength);
			if (uniqueIndex == std::numeric_limits<uint64_t>::max()) {
				fallback = true;
				break;
			}
			returnValues.uniqueIndices[results[x].length] = static_cast<uint8_t>(uniqueIndex);
		}
		if (fallback) {
			collectSimdFullLengthHashMapData<value_type>(returnValues, pairsNew);
		} else {
			returnValues.type = hash_map_type::unique_per_length;
		}
		return;
	}

	template<typename value_type>
	inline static constexpr void collectUniqueByteAndLengthHashMapData(hash_map_construction_data& returnValues, const tuple_references& pairsNew) noexcept {
		bool collided{ true };
		while (returnValues.uniqueIndex < keyStatsVal<value_type>.minLength) {
			returnValues.indices.fill(static_cast<uint16_t>(returnValues.indices.size() - 1));
			collided = false;
			for (uint64_t x = 0; x < pairsNew.count; ++x) {
				const auto hash = static_cast<uint64_t>(pairsNew.rootPtr[x].key[returnValues.uniqueIndex]) ^ static_cast<uint64_t>(pairsNew.rootPtr[x].key.size());
				const auto slot = hash % 2048;
				if (returnValues.indices[slot] != returnValues.indices.size() - 1) {
					collided = true;
					break;
				}
				returnValues.indices[slot] = pairsNew.rootPtr[x].oldIndex;
			}
			if (!collided) {
				break;
			}
			++returnValues.uniqueIndex;
		}
		if (collided) {
			collectUniquePerLengthHashMapData<value_type>(returnValues, pairsNew);
		} else {
			returnValues.type = hash_map_type::unique_byte_and_length;
		}
		return;
	}

	template<typename value_type> inline static constexpr bool canUseFirstByteAndUniqueIndex() noexcept {
		constexpr auto& refs = tupleReferences<value_type>;
		array<uint64_t, 256> bucketCounts{};
		for (uint64_t x = 0; x < refs.count; ++x) {
			if (refs.rootPtr[x].key.empty())
				return false;
			++bucketCounts[static_cast<uint8_t>(refs.rootPtr[x].key[0])];
		}
		for (uint64_t fb = 0; fb < 256; ++fb) {
			if (bucketCounts[fb] == 0)
				continue;
			uint64_t bucketKeys[64]{};
			uint64_t bIdx	  = 0;
			uint64_t minLen = (std::numeric_limits<uint64_t>::max)();
			for (uint64_t x = 0; x < refs.count && bIdx < 64; ++x) {
				if (static_cast<uint8_t>(refs.rootPtr[x].key[0]) == fb) {
					bucketKeys[bIdx++] = x;
					if (refs.rootPtr[x].key.size() < minLen)
						minLen = refs.rootPtr[x].key.size();
				}
			}
			bool found = false;
			for (uint64_t col = 0; col < minLen; ++col) {
				array<bool, 256> seen{};
				bool allDifferent = true;
				for (uint64_t b = 0; b < bIdx; ++b) {
					const uint8_t c = static_cast<uint8_t>(refs.rootPtr[bucketKeys[b]].key[col]);
					if (seen[c]) {
						allDifferent = false;
						break;
					}
					seen[c] = true;
				}
				if (allDifferent) {
					found = true;
					break;
				}
			}
			if (!found)
				return false;
		}
		return true;
	}

	template<typename value_type>
	inline static constexpr void collectFirstByteAndUniqueIndexHashMapData(hash_map_construction_data& returnValues, const tuple_references& pairsNew) {
		if constexpr (canUseFirstByteAndUniqueIndex<value_type>()) {
			constexpr auto keyStatsValNewer		= keyStatsImpl(tupleReferencesByFirstByte<value_type>);
			constexpr auto uniqueFirstByteCount = countFirstBytes(tupleReferencesByFirstByte<value_type>);
			constexpr auto results				= collectFirstBytes<uniqueFirstByteCount>(tupleReferencesByFirstByte<value_type>);
			constexpr auto keyStatsValNew		= keyStats(results);
			returnValues.uniqueIndices.fill(static_cast<uint8_t>(returnValues.uniqueIndices.size() - 1));
			bool fallback = false;
			if (keyStatsValNewer.maxLength < 256) {
				for (uint64_t x = 0; x < uniqueFirstByteCount; ++x) {
					auto uniqueIndex = findUniqueColumnIndex(results[x], keyStatsValNew[x].minLength);
					if (uniqueIndex == std::numeric_limits<uint64_t>::max()) {
						fallback = true;
						break;
					}
					returnValues.uniqueIndices[static_cast<uint8_t>(results[x].rootPtr[0].key[0])] = static_cast<uint8_t>(uniqueIndex);
				}
			} else {
				fallback = true;
			}
			if (fallback) {
				collectUniqueByteAndLengthHashMapData<value_type>(returnValues, pairsNew);
			} else {
				returnValues.type = hash_map_type::first_byte_and_unique_index;
			}
		} else {
			collectUniqueByteAndLengthHashMapData<value_type>(returnValues, pairsNew);
		}
		return;
	}

	/// Sampled from Stephen Berry and his library, Glaze library: https://github.com/StephenBerry/Glaze
	template<typename value_type>
	inline static constexpr void collectSingleByteHashMapData(hash_map_construction_data& returnValues, const tuple_references& pairsNew) noexcept {
		returnValues.uniqueIndex = keyStatsVal<value_type>.uniqueIndex;
		if (returnValues.uniqueIndex != std::numeric_limits<uint64_t>::max()) {
			returnValues.uniqueIndices.fill(static_cast<uint8_t>(returnValues.uniqueIndices.size() - 1));
			for (uint64_t x = 0; x < pairsNew.count; ++x) {
				auto& newRef					 = pairsNew.rootPtr[pairsNew.rootPtr[x].oldIndex];
				const auto slot					 = static_cast<uint8_t>(newRef.key.data()[returnValues.uniqueIndex]);
				returnValues.uniqueIndices[slot] = static_cast<uint8_t>(newRef.oldIndex);
			}
			returnValues.type = hash_map_type::single_byte;
		} else {
			collectFirstByteAndUniqueIndexHashMapData<value_type>(returnValues, pairsNew);
		}
		return;
	}

	/// Sampled from Stephen Berry and his library, Glaze library: https://github.com/StephenBerry/Glaze
	template<typename value_type>
	inline static constexpr void collectTripleElementHashMapData(hash_map_construction_data& returnValues, const tuple_references& pairsNew) noexcept {
		returnValues.uniqueIndex = keyStatsVal<value_type>.uniqueIndex;
		bool collided{ true };
		while (returnValues.uniqueIndex != std::numeric_limits<uint64_t>::max()) {
			returnValues.firstChar = static_cast<char>(static_cast<uint8_t>(pairsNew.rootPtr[0].key[returnValues.uniqueIndex]));
			const auto mix1		   = static_cast<uint8_t>(pairsNew.rootPtr[1].key[returnValues.uniqueIndex]) ^ returnValues.firstChar;
			const auto mix2		   = static_cast<uint8_t>(pairsNew.rootPtr[2].key[returnValues.uniqueIndex]) ^ returnValues.firstChar;
			for (uint64_t x = 0; x < 4; ++x) {
				uint8_t hash0 = static_cast<uint8_t>(0 & 3);
				uint8_t hash1 = static_cast<uint8_t>((mix1 * static_cast<uint8_t>(returnValues.hasher.seed)) & 3);
				uint8_t hash2 = static_cast<uint8_t>((mix2 * static_cast<uint8_t>(returnValues.hasher.seed)) & 3);

				if (hash0 == 2 && hash1 == 1 && hash2 == 0) {
					collided = false;
					break;
				} else {
					returnValues.hasher.updateSeed();
				}
			}
			if (!collided) {
				break;
			}
			returnValues.uniqueIndex = findUniqueColumnIndex(pairsNew, keyStatsVal<value_type>.minLength, returnValues.uniqueIndex + 1);
		}
		if (collided) {
			collectSingleByteHashMapData<value_type>(returnValues, pairsNew);
		} else {
			returnValues.type = hash_map_type::triple_element;
		}
		return;
	}

	template<typename value_type>
	inline static constexpr void collectDoubleElementHashMapData(hash_map_construction_data& returnValues, const tuple_references& pairsNew) noexcept {
		returnValues.uniqueIndex = keyStatsVal<value_type>.uniqueIndex;
		bool collided{ true };
		while (returnValues.uniqueIndex != std::numeric_limits<uint64_t>::max()) {
			if ((static_cast<uint64_t>(pairsNew.rootPtr[pairsNew.rootPtr[0].oldIndex].key[returnValues.uniqueIndex]) & 1ull) == 0ull &&
				(static_cast<uint64_t>(pairsNew.rootPtr[pairsNew.rootPtr[1].oldIndex].key[returnValues.uniqueIndex]) & 1ull) == 1ull) {
				collided = false;
				break;
			}
			returnValues.uniqueIndex = findUniqueColumnIndex(pairsNew, keyStatsVal<value_type>.minLength, returnValues.uniqueIndex + 1);
		}
		if (collided) {
			collectSingleByteHashMapData<value_type>(returnValues, pairsNew);
		} else {
			returnValues.type = hash_map_type::double_element;
		}
		return;
	}

	template<typename value_type> inline static constexpr hash_map_construction_data collectMapConstructionDataImpl() noexcept {
		hash_map_construction_data returnValues{};
		if constexpr (tupleReferences<value_type>.count == 0) {
			returnValues.type = hash_map_type::empty;
			return returnValues;
		} else if constexpr (tupleReferences<value_type>.count == 1) {
			returnValues.type = hash_map_type::single_element;
			return returnValues;
		} else {
			if constexpr (keyStatsVal<value_type>.uniqueIndex != std::numeric_limits<uint64_t>::max()) {
				if constexpr (tupleReferences<value_type>.count == 2) {
					collectDoubleElementHashMapData<value_type>(returnValues, tupleReferences<value_type>);
					return returnValues;
				} else if constexpr (tupleReferences<value_type>.count == 3) {
					collectTripleElementHashMapData<value_type>(returnValues, tupleReferences<value_type>);
					return returnValues;
				} else {
					collectSingleByteHashMapData<value_type>(returnValues, tupleReferences<value_type>);
					return returnValues;
				}
			} else {
				collectFirstByteAndUniqueIndexHashMapData<value_type>(returnValues, tupleReferences<value_type>);
				return returnValues;
			}
		}
	}

	template<typename value_type> inline static constexpr decltype(auto) collectMapConstructionData() noexcept {
		constexpr auto constructionData = collectMapConstructionDataImpl<value_type>();
		if constexpr (constructionData.type == hash_map_type::single_element) {
			return single_element_data{ constructionData };
		} else if constexpr (constructionData.type == hash_map_type::double_element) {
			return double_element_data{ constructionData };
		} else if constexpr (constructionData.type == hash_map_type::triple_element) {
			return triple_element_data{ constructionData };
		} else if constexpr (constructionData.type == hash_map_type::single_byte) {
			return single_byte_data{ constructionData };
		} else if constexpr (constructionData.type == hash_map_type::first_byte_and_unique_index) {
			return first_byte_and_unique_index_data{ constructionData };
		} else if constexpr (constructionData.type == hash_map_type::unique_byte_and_length) {
			return unique_byte_and_length_data{ constructionData };
		} else if constexpr (constructionData.type == hash_map_type::unique_per_length) {
			return unique_per_length_data{ constructionData };
		} else if constexpr (constructionData.type == hash_map_type::simd_full_length) {
			return simd_full_length_data{ constructionData };
		} else {
			static_assert(constructionData.type != hash_map_type::unset, "Failed to construct that hashmap!");
		}
	}

	template<uint64_t keyMaxLength>
	static constexpr array<uint8_t, (keyMaxLength + 1) * 256> generateMappingsForLengths(const tuple_references& keys, const array<uint8_t, 256>& uniqueIndices) noexcept {
		array<uint8_t, (keyMaxLength + 1) * 256> mappings{};
		std::fill(mappings.data(), mappings.data() + mappings.size(), static_cast<uint8_t>(mappings.size() - 1));

		for (uint64_t x = 0; x < keys.count; ++x) {
			const auto& key = keys.rootPtr[x].key;

			uint8_t uniqueIndex = uniqueIndices[key.size()];

			if (uniqueIndex != 255 && uniqueIndex < key.size()) {
				uint8_t keyChar		= static_cast<uint8_t>(key[uniqueIndex]);
				uint64_t flatIndex	= key.size() * 256 + keyChar;
				mappings[flatIndex] = keys.rootPtr[x].oldIndex;
			}
		}

		return mappings;
	}

	template<uint64_t firstCharCount> constexpr char getMaxFirstByte(const array<first_bytes, firstCharCount>& keys) noexcept {
		char returnValue{};
		for (uint64_t x = 0; x < firstCharCount; ++x) {
			if (keys[x].value > returnValue) {
				returnValue = keys[x].value;
			}
		}
		return returnValue;
	}

	template<typename value_type> struct first_byte_arrays {
		static constexpr auto& refs = tupleReferences<value_type>;
		array<uint8_t, 256> uniqueIndexByFirstByte{};
		array<uint8_t, 256 * 256> indexByFirstByteAndChar{};
	};

	template<typename value_type> inline static constexpr first_byte_arrays<value_type> buildFirstByteArrays() noexcept {
		constexpr auto& refs = tupleReferences<value_type>;
		first_byte_arrays<value_type> result{};
		result.uniqueIndexByFirstByte.fill(255);
		result.indexByFirstByteAndChar.fill(static_cast<uint8_t>(result.indexByFirstByteAndChar.size() - 1));

		array<uint64_t, 256> bucketIndices{};
		array<array<uint64_t, 64>, 256> keysByFirstByte{};

		for (uint64_t x = 0; x < refs.count; ++x) {
			const auto& key = refs.rootPtr[x].key;
			if (key.empty())
				continue;
			const uint8_t fb = static_cast<uint8_t>(key[0]);
			if (bucketIndices[fb] < 64) {
				keysByFirstByte[fb][bucketIndices[fb]++] = x;
			}
		}

		for (uint64_t fb = 0; fb < 256; ++fb) {
			const uint64_t bucketCount = bucketIndices[fb];
			if (bucketCount == 0)
				continue;

			uint64_t minLenInBucket = (std::numeric_limits<uint64_t>::max)();
			for (uint64_t b = 0; b < bucketCount; ++b) {
				const uint64_t len = refs.rootPtr[keysByFirstByte[fb][b]].key.size();
				if (len < minLenInBucket)
					minLenInBucket = len;
			}

			uint64_t chosenColumn = std::numeric_limits<uint64_t>::max();
			for (uint64_t col = 0; col < minLenInBucket; ++col) {
				array<bool, 256> seen{};
				bool allDifferent = true;
				for (uint64_t b = 0; b < bucketCount; ++b) {
					const auto& key = refs.rootPtr[keysByFirstByte[fb][b]].key;
					const uint8_t c = static_cast<uint8_t>(key[col]);
					if (seen[c]) {
						allDifferent = false;
						break;
					}
					seen[c] = true;
				}
				if (allDifferent) {
					chosenColumn = col;
					break;
				}
			}

			if (chosenColumn == std::numeric_limits<uint64_t>::max()) {
				break;
			}

			result.uniqueIndexByFirstByte[fb] = static_cast<uint8_t>(chosenColumn);
			for (uint64_t b = 0; b < bucketCount; ++b) {
				const auto& ref								  = refs.rootPtr[keysByFirstByte[fb][b]];
				const uint8_t c								  = static_cast<uint8_t>(ref.key[chosenColumn]);
				result.indexByFirstByteAndChar[(fb << 8) | c] = static_cast<uint8_t>(ref.oldIndex);
			}
		}
		return result;
	}

	template<typename value_type> inline static constexpr auto hashData = collectMapConstructionData<value_type>();

	template<typename value_type, typename iterator_newer> struct hash_map {
		inline static constexpr auto subAmount01{ []() constexpr {
			return ((keyStatsVal<value_type>.maxLength - keyStatsVal<value_type>.minLength) >= bytesPerStep) ? keyStatsVal<value_type>.minLength : 0;
		}() };

		inline static constexpr auto subAmount02{ []() constexpr {
			return ((keyStatsVal<value_type>.maxLength - keyStatsVal<value_type>.minLength) >= bytesPerStep)
				? (keyStatsVal<value_type>.maxLength - keyStatsVal<value_type>.minLength + 2)
				: (keyStatsVal<value_type>.maxLength + 2);
		}() };

		JSONIFIER_INLINE static uint64_t findIndex(iterator_newer iter, iterator_newer end) noexcept {
			static constexpr auto checkForEnd = [](const auto& iterNew, const auto& endNew, const auto distance) {
				return (iterNew + distance) < endNew;
			};
			if constexpr (hashData<value_type>.type == hash_map_type::single_element) {
				return 0ull;
			} else if constexpr (hashData<value_type>.type == hash_map_type::double_element) {
				if JSONIFIER_LIKELY (checkForEnd(iter, end, hashData<value_type>.uniqueIndex)) {
					return static_cast<uint64_t>(iter[static_cast<uint8_t>(hashData<value_type>.uniqueIndex)]) & 1u;
				}
				return hashData<value_type>.storageSize;
			} else if constexpr (hashData<value_type>.type == hash_map_type::triple_element) {
				if JSONIFIER_LIKELY (checkForEnd(iter, end, hashData<value_type>.uniqueIndex)) {
					return (static_cast<uint8_t>(iter[hashData<value_type>.uniqueIndex] ^ hashData<value_type>.firstChar) * hashData<value_type>.seed) & 3u;
				}
				return hashData<value_type>.storageSize;
			} else if constexpr (hashData<value_type>.type == hash_map_type::single_byte) {
				if JSONIFIER_LIKELY (checkForEnd(iter, end, hashData<value_type>.uniqueIndex)) {
					return hashData<value_type>.uniqueIndices[static_cast<uint8_t>(iter[static_cast<uint8_t>(hashData<value_type>.uniqueIndex)])];
				}
				return hashData<value_type>.storageSize;
			} else if constexpr (hashData<value_type>.type == hash_map_type::first_byte_and_unique_index) {
				static constexpr auto arrays = buildFirstByteArrays<value_type>();
				const uint8_t firstByte		 = static_cast<uint8_t>(iter[0]);
				const uint8_t uniqueIdx		 = arrays.uniqueIndexByFirstByte[firstByte];
				if JSONIFIER_LIKELY (uniqueIdx != 255 && checkForEnd(iter, end, uniqueIdx)) {
					const uint8_t keyChar = static_cast<uint8_t>(iter[uniqueIdx]);
					return arrays.indexByFirstByteAndChar[(static_cast<uint64_t>(firstByte) << 8) | static_cast<uint64_t>(keyChar)];
				}
				return hashData<value_type>.storageSize;
			} else if constexpr (hashData<value_type>.type == hash_map_type::unique_byte_and_length) {
				static constexpr uint64_t storageMask = hashData<value_type>.storageSize - 1;
				const auto newPtr					= char_comparison<'"', jsonifier::internal::remove_cvref_t<decltype(*iter)>>::memchar(iter + subAmount01, subAmount02);
				if JSONIFIER_LIKELY (newPtr) {
					const uint64_t length = static_cast<uint64_t>(newPtr - iter);
					if JSONIFIER_LIKELY (checkForEnd(iter, end, hashData<value_type>.uniqueIndex)) {
						const uint64_t combinedKey = static_cast<uint64_t>(iter[hashData<value_type>.uniqueIndex]) ^ length;
						return hashData<value_type>.indices[combinedKey & storageMask];
					}
				}
				return hashData<value_type>.storageSize;
			} else if constexpr (hashData<value_type>.type == hash_map_type::unique_per_length) {
				static constexpr auto mappings =
					generateMappingsForLengths<keyStatsVal<value_type>.maxLength>(tupleReferencesByLength<value_type>, hashData<value_type>.uniqueIndices);
				const auto newPtr = char_comparison<'"', jsonifier::internal::remove_cvref_t<decltype(*iter)>>::memchar(iter + subAmount01, subAmount02);
				if JSONIFIER_LIKELY (newPtr) {
					const uint64_t length			= static_cast<uint64_t>(newPtr - iter);
					const uint64_t localUniqueIdx = hashData<value_type>.uniqueIndices[length];
					if JSONIFIER_LIKELY (localUniqueIdx != 255 && checkForEnd(iter, end, localUniqueIdx)) {
						return static_cast<uint64_t>(mappings[(length << 8) | static_cast<uint64_t>(iter[localUniqueIdx])]);
					}
				}
				return hashData<value_type>.storageSize;
			} else if constexpr (hashData<value_type>.type == hash_map_type::simd_full_length) {
				using simd_type = map_simd_t<hashData<value_type>.storageSize>;
				static constexpr rt_key_hasher<hashData<value_type>.seed> hasher{};
				static constexpr auto sizeMask{ hashData<value_type>.numGroups - 1u };
				static constexpr auto ctrlBytesPtr{ hashData<value_type>.controlBytes.data() };
				const auto newPtr = char_comparison<'"', jsonifier::internal::remove_cvref_t<decltype(*iter)>>::memchar(iter + subAmount01, subAmount02);
				if JSONIFIER_LIKELY (newPtr) {
					uint64_t length = static_cast<uint64_t>(newPtr - iter);
					length		  = (hashData<value_type>.uniqueIndex > length) ? length : hashData<value_type>.uniqueIndex;
					if JSONIFIER_LIKELY (checkForEnd(iter, end, length)) {
						const auto hash			 = hasher.hashKeyRt(iter, length);
						const uint64_t group		 = (hash >> 8) & (sizeMask);
						const uint64_t resultIndex = group * hashData<value_type>.bucketSize;
						const uint64_t matches{ simd::opCmpEq(simd::gatherValue<simd_type>(static_cast<uint8_t>(hash)),
							simd::gatherValues<simd_type>(ctrlBytesPtr + resultIndex)) };
						const uint64_t tz = simd::postCmpTzcnt(matches);
						return hashData<value_type>.indices[resultIndex + tz];
					}
				}
				return hashData<value_type>.storageSize;
			} else if constexpr (hashData<value_type>.type == hash_map_type::empty) {
				return hashData<value_type>.storageSize;
			}
		}
	};
};
