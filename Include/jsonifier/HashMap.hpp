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
/// Note: Some of the code in this header was sampled from Glaze library: https://github.com/StephenBerry/Glaze
/// https://github.com/RealTimeChris/jsonifier
/// Feb 20, 2023
#pragma once

#include <jsonifier/TypeEntities.hpp>
#include <jsonifier/Hash.hpp>
#include <jsonifier/StringView.hpp>
#include <jsonifier/Reflection.hpp>
#include <jsonifier/StringUtils.hpp>
#include <jsonifier/Tuple.hpp>
#include <algorithm>
#include <numeric>
#include <utility>

namespace jsonifier_internal {

	template<typename value_type, size_t size> std::ostream& operator<<(std::ostream& os, const std::array<value_type, size>& values) {
		os << "[";
		for (size_t x = 0; x < size; ++x) {
			os << values[x];
			if (x < size - 1) {
				os << ",";
			}
		}
		os << "]";
		return os;
	}

	struct tuple_reference {
		jsonifier::string_view key{};
		size_t oldIndex{};
	};

	struct tuple_references {
		const tuple_reference* rootPtr{};
		size_t count{};
	};

	template<typename value_type>
	concept has_view = requires(unwrap_t<value_type> value) { value.view(); };

	template<size_t maxIndex, size_t currentIndex = 0, typename tuple_type>
	constexpr auto collectTupleRefsImpl(const tuple_type& tuple, std::array<tuple_reference, maxIndex>& tupleRefs) {
		if constexpr (currentIndex < maxIndex) {
			auto potentialKey = std::get<currentIndex>(tuple);
			if constexpr (has_view<decltype(potentialKey)>) {
				tupleRefs[currentIndex].key = potentialKey.view();
			}
			tupleRefs[currentIndex].oldIndex = currentIndex;
			return collectTupleRefsImpl<maxIndex, currentIndex + 1>(tuple, tupleRefs);
		}
		return tupleRefs;
	}

	template<typename tuple_type> constexpr auto collectTupleRefs(const tuple_type& tuple) {
		constexpr auto tupleSize = std::tuple_size_v<tuple_type>;
		std::array<tuple_reference, tupleSize> tupleRefs{};
		return collectTupleRefsImpl<tupleSize>(tuple, tupleRefs);
	}

	template<size_t size> constexpr auto sortTupleRefsByFirstByte(const std::array<tuple_reference, size>& tupleRefs) {
		std::array<tuple_reference, size> returnValues{ tupleRefs };
		std::sort(returnValues.begin(), returnValues.end(), [](const tuple_reference& lhs, const tuple_reference& rhs) {
			return lhs.key[0] < rhs.key[0];
		});
		return returnValues;
	}

	template<size_t size> constexpr auto sortTupleRefsByLength(const std::array<tuple_reference, size>& tupleRefs) {
		std::array<tuple_reference, size> returnValues{ tupleRefs };
		std::sort(returnValues.begin(), returnValues.end(), [](const tuple_reference& lhs, const tuple_reference& rhs) {
			return lhs.key.size() < rhs.key.size();
		});
		return returnValues;
	}

	template<size_t size> constexpr auto consolidateTupleRefs(const std::array<tuple_reference, size>& tupleRefs) {
		tuple_references returnValues{};
		returnValues.rootPtr = &tupleRefs[0];
		returnValues.count	 = size;
		return returnValues;
	}

	template<typename value_type> inline constexpr auto tupleRefs{ collectTupleRefs(jsonifier::concepts::coreV<value_type>) };
	template<typename value_type> inline constexpr auto sortedTupleReferencesByLength{ sortTupleRefsByLength(tupleRefs<value_type>) };
	template<typename value_type> inline constexpr auto tupleReferencesByLength{ consolidateTupleRefs(sortedTupleReferencesByLength<value_type>) };
	template<typename value_type> inline constexpr auto sortedTupleReferencesByFirstByte{ sortTupleRefsByFirstByte(tupleRefs<value_type>) };
	template<typename value_type> inline constexpr auto tupleReferencesByFirstByte{ consolidateTupleRefs(sortedTupleReferencesByFirstByte<value_type>) };

	template<typename value_type, size_t... indices> JSONIFIER_ALWAYS_INLINE constexpr auto createNewTupleImpl(std::index_sequence<indices...>) noexcept {
		return std::make_tuple(std::get<sortedTupleReferencesByLength<value_type>[indices].oldIndex>(jsonifier::concepts::coreV<value_type>)...);
	}

	template<typename value_type> JSONIFIER_ALWAYS_INLINE constexpr auto createNewTuple() noexcept {
		constexpr auto& tupleRefs = sortedTupleReferencesByLength<value_type>;
		return createNewTupleImpl<value_type>(std::make_index_sequence<tupleRefs.size()>{});
	}

	template<typename value_type> inline constexpr auto coreTupleV{ createNewTuple<unwrap_t<value_type>>() };

	template<typename value_type> using core_tuple_t = decltype(coreTupleV<value_type>);

	template<typename value_type01, typename value_type02>
	JSONIFIER_ALWAYS_INLINE constexpr bool contains(const value_type01* hashData, value_type02 byteToCheckFor, size_t size) noexcept {
		for (size_t x = 0; x < size; ++x) {
			if (hashData[x] == byteToCheckFor) {
				return true;
			}
		}
		return false;
	}

	template<size_t length> struct map_simd {
		using type = std::conditional_t<length >= 64 && bytesPerStep >= 64, jsonifier_simd_int_512,
			std::conditional_t<length >= 32 && bytesPerStep >= 32, jsonifier_simd_int_256, jsonifier_simd_int_128>>;
	};

	template<size_t length> using map_simd_t = map_simd<length>::type;

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

	JSONIFIER_ALWAYS_INLINE static constexpr size_t setSimdWidth(size_t length) noexcept {
		return length >= 64ull && bytesPerStep >= 64ull ? 64ull : length >= 32ull && bytesPerStep >= 32ull ? 32ull : 16ull;
	}

	struct key_stats_t {
		size_t minLength{ (std::numeric_limits<size_t>::max)() };
		size_t uniqueIndex{};
		size_t maxLength{};
	};

	JSONIFIER_ALWAYS_INLINE static constexpr size_t findUniqueColumnIndex(const tuple_references& tupleRefs, size_t maxIndex, size_t startingIndex = 0) noexcept {
		constexpr size_t alphabetSize = 256;
		jsonifier::string_view key{};
		for (size_t index = startingIndex; index < maxIndex; ++index) {
			std::array<bool, alphabetSize> seen{};
			bool allDifferent = true;

			for (size_t x = 0; x < tupleRefs.count; ++x) {
				key				 = tupleRefs.rootPtr[x].key;
				const char c	 = key[index];
				size_t charIndex = static_cast<const unsigned char>(c);

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

		return std::numeric_limits<size_t>::max();
	}

	template<typename value_type> struct hash_map_construction_data {
		using simd_type = map_simd_t<2048>;
		std::array<size_t, 2048 / setSimdWidth(2048)> bucketSizes{};
		JSONIFIER_ALIGN std::array<uint8_t, 2049> controlBytes{};
		JSONIFIER_ALIGN std::array<uint8_t, 256> uniqueIndices{};
		JSONIFIER_ALIGN std::array<size_t, 2049> indices{};
		size_t bucketSize{ setSimdWidth(2048) };
		size_t numGroups{ 2048 / bucketSize };
		size_t storageSize{ 2048 };
		key_stats_t keyStatsVal{};
		ct_key_hasher hasher{};
		hash_map_type type{};
		size_t uniqueIndex{};

		JSONIFIER_ALWAYS_INLINE constexpr hash_map_construction_data() noexcept = default;
	};

	struct string_lengths : public tuple_references {
		size_t length{};
	};

	constexpr auto countUniqueLengths(const tuple_references& tupleRefs) {
		std::array<size_t, 256> stringLengths{};
		size_t returnValue{};
		for (size_t x = 0; x < tupleRefs.count; ++x) {
			++stringLengths[tupleRefs.rootPtr[x].key.size()];
		}
		for (size_t x = 0; x < 256; ++x) {
			if (stringLengths[x] > 0) {
				++returnValue;
			}
		}
		return returnValue;
	}

	template<size_t stringLengthCount> constexpr auto collectLengths(const tuple_references& values) {
		std::array<size_t, 256> lengths{};
		std::array<string_lengths, stringLengthCount> valuesNew{};
		size_t currentIndex{};
		for (uint64_t x = 0; x < values.count; ++x) {
			if (lengths[values.rootPtr[x].key.size()] == 0) {
				++lengths[values.rootPtr[x].key.size()];
				string_lengths tupleRefs{};
				tupleRefs.rootPtr		= &values.rootPtr[x];
				tupleRefs.length		= values.rootPtr[x].key.size();
				valuesNew[currentIndex] = tupleRefs;
				++valuesNew[currentIndex].count;
				++currentIndex;
			} else {
				for (auto& value: valuesNew) {
					if (value.length == values.rootPtr[x].key.size()) {
						++value.count;
					}
				}
			}
		}
		return valuesNew;
	}

	constexpr auto countFirstBytes(const tuple_references& tupleRefs) {
		std::array<bool, 256> stringLengths{};
		size_t returnValue = 0;
		for (size_t x = 0; x < tupleRefs.count; ++x) {
			if (!tupleRefs.rootPtr[x].key.empty()) {
				uint8_t firstByte = static_cast<uint8_t>(tupleRefs.rootPtr[x].key[0]);
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

	template<size_t firstByteCount> constexpr auto collectFirstBytes(const tuple_references& values) {
		std::array<size_t, 256> lengths{};
		std::array<first_bytes, firstByteCount> valuesNew{};
		size_t currentIndex{};
		for (uint64_t x = 0; x < values.count; ++x) {
			if (lengths[values.rootPtr[x].key[0]] == 0) {
				++lengths[values.rootPtr[x].key[0]];
				first_bytes tupleRefs{};
				tupleRefs.rootPtr		= &values.rootPtr[x];
				tupleRefs.value			= values.rootPtr[x].key[0];
				valuesNew[currentIndex] = tupleRefs;
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

	JSONIFIER_ALWAYS_INLINE constexpr auto keyStatsImpl(const tuple_references& tupleRefs) noexcept {
		key_stats_t stats{};
		for (size_t x = 0; x < tupleRefs.count; ++x) {
			const jsonifier::string_view& key{ tupleRefs.rootPtr[x].key };
			auto n{ key.size() };
			if (n > stats.maxLength) {
				stats.maxLength = n;
			}
			if (n < stats.minLength) {
				stats.minLength = n;
			}
		}
		stats.uniqueIndex = findUniqueColumnIndex(tupleRefs, stats.minLength);
		return stats;
	}

	template<size_t size> JSONIFIER_ALWAYS_INLINE constexpr auto keyStats(const std::array<first_bytes, size>& tupleRefs) noexcept {
		std::array<key_stats_t, size> returnValues{};
		for (uint64_t x = 0; x < size; ++x) {
			returnValues[x] = keyStatsImpl(static_cast<const tuple_references&>(tupleRefs[x]));
		}
		return returnValues;
	}

	template<size_t size> JSONIFIER_ALWAYS_INLINE constexpr auto keyStats(const std::array<string_lengths, size>& tupleRefs) noexcept {
		std::array<key_stats_t, size> returnValues{};
		for (uint64_t x = 0; x < size; ++x) {
			returnValues[x] = keyStatsImpl(static_cast<const tuple_references&>(tupleRefs[x]));
		}
		return returnValues;
	}

	template<typename value_type>
	JSONIFIER_ALWAYS_INLINE constexpr auto collectSimdFullLengthHashMapData(const tuple_references& pairsNew, const key_stats_t& keyStatsVal) noexcept {
		hash_map_construction_data<value_type> returnValues{};
		bool collided{};
		for (size_t w = 0; w < keyStatsVal.maxLength; ++w) {
			returnValues.uniqueIndex = w;
			for (size_t x = 0; x < 2; ++x) {
				returnValues.controlBytes.fill(std::numeric_limits<uint8_t>::max());
				returnValues.indices.fill(returnValues.indices.size() - 1);
				returnValues.hasher.updateSeed();
				collided = false;
				for (size_t y = 0; y < pairsNew.count; ++y) {
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
			returnValues.uniqueIndex = std::numeric_limits<size_t>::max();
			return returnValues;
		} else {
			returnValues.keyStatsVal = keyStatsVal;
			returnValues.type		 = hash_map_type::simd_full_length;
			returnValues.storageSize = 2048;
			return returnValues;
		}
	}

	template<typename value_type> constexpr auto collectUniquePerLengthHashMapData(const tuple_references& pairsNew, const key_stats_t& keyStatsVal) {
		constexpr auto uniqueLengthCount = countUniqueLengths(tupleReferencesByLength<value_type>);
		constexpr auto results			 = collectLengths<uniqueLengthCount>(tupleReferencesByLength<value_type>);
		constexpr auto keyStatsValNew	 = keyStats(results);
		hash_map_construction_data<value_type> returnValues{};
		returnValues.uniqueIndices.fill(returnValues.uniqueIndices.size() - 1);
		for (size_t x = 0; x < uniqueLengthCount; ++x) {
			auto uniqueIndex = findUniqueColumnIndex(results[x], keyStatsValNew[x].minLength);
			if (uniqueIndex == std::numeric_limits<size_t>::max()) {
				return collectSimdFullLengthHashMapData<value_type>(pairsNew, keyStatsVal);
			} else {
				returnValues.uniqueIndices[results[x].length] = static_cast<uint8_t>(uniqueIndex);
			}
		}
		returnValues.keyStatsVal = keyStatsVal;
		returnValues.type		 = hash_map_type::unique_per_length;
		returnValues.storageSize = 256;
		return returnValues;
	}

	template<typename value_type>
	JSONIFIER_ALWAYS_INLINE constexpr auto collectUniqueByteAndLengthHashMapData(const tuple_references& pairsNew, const key_stats_t& keyStatsVal) noexcept {
		hash_map_construction_data<value_type> returnValues{};
		bool collided{ true };
		while (returnValues.uniqueIndex < keyStatsVal.minLength) {
			returnValues.indices.fill(returnValues.indices.size() - 1);
			collided = false;
			for (size_t x = 0; x < pairsNew.count; ++x) {
				const auto hash = pairsNew.rootPtr[x].key[returnValues.uniqueIndex] ^ pairsNew.rootPtr[x].key.size();
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
			return collectUniquePerLengthHashMapData<value_type>(pairsNew, keyStatsVal);
		} else {
			returnValues.keyStatsVal = keyStatsVal;
			returnValues.type		 = hash_map_type::unique_byte_and_length;
			returnValues.storageSize = 2048;
			return returnValues;
		}
	}

	template<typename value_type> constexpr auto collectFirstByteAndUniqueIndexHashMapData(const tuple_references& pairsNew) {
		constexpr auto keyStatsVal			= keyStatsImpl(tupleReferencesByFirstByte<value_type>);
		constexpr auto uniqueFirstByteCount = countFirstBytes(tupleReferencesByFirstByte<value_type>);
		constexpr auto results				= collectFirstBytes<uniqueFirstByteCount>(tupleReferencesByFirstByte<value_type>);
		constexpr auto keyStatsValNew		= keyStats(results);
		hash_map_construction_data<value_type> returnValues{};
		returnValues.uniqueIndices.fill(returnValues.uniqueIndices.size() - 1);
		if (keyStatsVal.maxLength < 256) {
			for (size_t x = 0; x < uniqueFirstByteCount; ++x) {
				auto uniqueIndex = findUniqueColumnIndex(results[x], keyStatsValNew[x].minLength);
				if (uniqueIndex == std::numeric_limits<size_t>::max()) {
					return collectUniqueByteAndLengthHashMapData<value_type>(pairsNew, keyStatsVal);
				} else {
					returnValues.uniqueIndices[results[x].rootPtr[0].key[0]] = uniqueIndex;
				}
			}
		} else {
			return collectUniqueByteAndLengthHashMapData<value_type>(pairsNew, keyStatsVal);
		}
		returnValues.keyStatsVal = keyStatsVal;
		returnValues.type		 = hash_map_type::first_byte_and_unique_index;
		returnValues.storageSize = 256;
		return returnValues;
	}

	template<typename value_type> JSONIFIER_ALWAYS_INLINE constexpr auto collectSingleByteHashMapData(const tuple_references& pairsNew, const key_stats_t& keyStatsVal) noexcept {
		hash_map_construction_data<value_type> returnValues{};
		returnValues.uniqueIndex = keyStatsVal.uniqueIndex;
		if (returnValues.uniqueIndex != std::numeric_limits<size_t>::max()) {
			returnValues.uniqueIndices.fill(returnValues.uniqueIndices.size() - 1);
			for (size_t x = 0; x < pairsNew.count; ++x) {
				const auto slot					 = pairsNew.rootPtr[x].key.data()[returnValues.uniqueIndex];
				returnValues.uniqueIndices[slot] = pairsNew.rootPtr[x].oldIndex;
			}
			returnValues.keyStatsVal = keyStatsVal;
			returnValues.type		 = hash_map_type::single_byte;
			returnValues.storageSize = 256;
			return returnValues;
		} else {
			return collectFirstByteAndUniqueIndexHashMapData<value_type>(pairsNew);
		}
	}

	template<typename value_type>
	JSONIFIER_ALWAYS_INLINE constexpr auto collectTripleElementHashMapData(const tuple_references& pairsNew, const key_stats_t& keyStatsVal) noexcept {
		hash_map_construction_data<value_type> returnValues{};
		returnValues.uniqueIndex = keyStatsVal.uniqueIndex;
		bool collided{ true };
		while (returnValues.uniqueIndex != std::numeric_limits<size_t>::max()) {
			const auto first = static_cast<uint8_t>(pairsNew.rootPtr[0].key[returnValues.uniqueIndex]);
			const auto mix1	 = static_cast<uint8_t>(pairsNew.rootPtr[1].key[returnValues.uniqueIndex]) ^ first;
			const auto mix2	 = static_cast<uint8_t>(pairsNew.rootPtr[2].key[returnValues.uniqueIndex]) ^ first;
			for (size_t x = 0; x < 4; ++x) {
				uint8_t hash0 = first & 3;
				uint8_t hash1 = (mix1 * returnValues.hasher.seed) & 3;
				uint8_t hash2 = (mix2 * returnValues.hasher.seed) & 3;

				if (hash0 == pairsNew.rootPtr[0].oldIndex && hash1 == pairsNew.rootPtr[1].oldIndex && hash2 == pairsNew.rootPtr[2].oldIndex) {
					collided = false;
					break;
				} else {
					returnValues.hasher.updateSeed();
				}
			}
			if (!collided) {
				break;
			}
			returnValues.uniqueIndex = findUniqueColumnIndex(pairsNew, keyStatsVal.minLength, returnValues.uniqueIndex + 1);
		}
		if (collided) {
			return collectSingleByteHashMapData<value_type>(pairsNew, keyStatsVal);
		}
		returnValues.keyStatsVal = keyStatsVal;
		returnValues.type		 = hash_map_type::triple_element;
		return returnValues;
	}

	template<typename value_type>
	JSONIFIER_ALWAYS_INLINE constexpr auto collectDoubleElementHashMapData(const tuple_references& pairsNew, const key_stats_t& keyStatsVal) noexcept {
		hash_map_construction_data<value_type> returnValues{};
		returnValues.uniqueIndex = keyStatsVal.uniqueIndex;
		bool collided{ true };
		while (returnValues.uniqueIndex != std::numeric_limits<size_t>::max()) {
			if ((pairsNew.rootPtr[0].key[returnValues.uniqueIndex] & 1) == pairsNew.rootPtr[0].oldIndex &&
				(pairsNew.rootPtr[1].key[returnValues.uniqueIndex] & 1) == pairsNew.rootPtr[1].oldIndex) {
				collided = false;
				break;
			}
			returnValues.uniqueIndex = findUniqueColumnIndex(pairsNew, keyStatsVal.minLength, returnValues.uniqueIndex + 1);
		}
		if (collided) {
			return collectSingleByteHashMapData<value_type>(pairsNew, keyStatsVal);
		}
		returnValues.keyStatsVal = keyStatsVal;
		returnValues.type		 = hash_map_type::double_element;
		return returnValues;
	}

	template<typename value_type> JSONIFIER_ALWAYS_INLINE constexpr auto collectMapConstructionData() noexcept {
		constexpr auto keyStatsVal = keyStatsImpl(tupleReferencesByFirstByte<value_type>);
		if constexpr (tupleReferencesByLength<value_type>.count == 0) {
			hash_map_construction_data<value_type> returnValues{};
			returnValues.type = hash_map_type::empty;
			return returnValues;
		} else if constexpr (tupleReferencesByLength<value_type>.count == 1) {
			hash_map_construction_data<value_type> returnValues{};
			returnValues.type = hash_map_type::single_element;
			return returnValues;
		} else {
			if constexpr (keyStatsVal.uniqueIndex != std::numeric_limits<size_t>::max()) {
				if constexpr (tupleReferencesByLength<value_type>.count == 2) {
					return collectDoubleElementHashMapData<value_type>(tupleReferencesByLength<value_type>, keyStatsVal);
				} else if constexpr (tupleReferencesByLength<value_type>.count == 3) {
					return collectTripleElementHashMapData<value_type>(tupleReferencesByLength<value_type>, keyStatsVal);
				} else {
					return collectSingleByteHashMapData<value_type>(tupleReferencesByLength<value_type>, keyStatsVal);
				}
			} else {
				return collectFirstByteAndUniqueIndexHashMapData<value_type>(tupleReferencesByLength<value_type>);
			}
		}
	}

	template<size_t keyMaxLength> constexpr auto generateMappingsForLengths(const tuple_references& keys, const std::array<uint8_t, 256>& uniqueIndices) {
		std::array<size_t, (keyMaxLength + 1) * 256> mappings{};

		mappings.fill(static_cast<size_t>(-1));

		for (size_t x = 0; x < keys.count; ++x) {
			const auto& key = keys.rootPtr[x].key;

			uint8_t uniqueIndex = uniqueIndices[key.size()];

			if (uniqueIndex != 255 && uniqueIndex < key.size()) {
				uint8_t keyChar		= static_cast<uint8_t>(key[uniqueIndex]);
				size_t flatIndex	= key.size() * 256 + keyChar;
				mappings[flatIndex] = keys.rootPtr[x].oldIndex;
			}
		}

		return mappings;
	}

	template<size_t firstCharCount>
	constexpr auto generateMappingsForFirstBytes(const std::array<first_bytes, firstCharCount>& keys, const std::array<uint8_t, 256>& uniqueIndices) {
		constexpr size_t flattenedSize = 256 * 256;
		std::array<size_t, flattenedSize> flattenedMappings{};
		flattenedMappings.fill(flattenedMappings.size() - 1);

		size_t currentIndex{};

		for (size_t x = 0; x < firstCharCount; ++x) {
			const auto& key	   = keys[x].rootPtr[0].key;
			uint8_t firstByte  = static_cast<uint8_t>(key[0]);
			size_t uniqueIndex = uniqueIndices[firstByte];

			for (size_t y = 0; y < keys[x].count; ++y) {
				const auto& keyNew = keys[x].rootPtr[y].key;
				if (uniqueIndex < keyNew.size()) {
					uint8_t keyChar					= static_cast<uint8_t>(keyNew[uniqueIndex]);
					size_t flattenedIdx				= firstByte * 256 + keyChar;
					flattenedMappings[flattenedIdx] = keys[x].rootPtr[y].oldIndex;
					++currentIndex;
				}
			}
		}

		return flattenedMappings;
	}

#if !defined(NDEBUG)
	std::unordered_map<std::string, int32_t> types{};
#endif

	template<typename value_type, typename iterator_newer> struct hash_map {
		static constexpr auto hashData = collectMapConstructionData<unwrap_t<value_type>>();
		static constexpr auto subAmount01{ []() constexpr {
			return ((hashData.keyStatsVal.maxLength - hashData.keyStatsVal.minLength) >= bytesPerStep) ? hashData.keyStatsVal.minLength : 0;
		}() };

		static constexpr auto subAmount02{ []() constexpr {
			return ((hashData.keyStatsVal.maxLength - hashData.keyStatsVal.minLength) >= bytesPerStep) ? (hashData.keyStatsVal.maxLength - hashData.keyStatsVal.minLength + 2)
																									   : (hashData.keyStatsVal.maxLength + 2);
		}() };

		JSONIFIER_ALWAYS_INLINE static size_t findIndex(iterator_newer& iter, iterator_newer& end) noexcept {
#if !defined(NDEBUG)
			if (!types.contains(typeid(value_type).name())) {
				types[typeid(value_type).name()] = ( int32_t )hashData.type;
			}
#endif
			if constexpr (hashData.type == hash_map_type::single_element) {
				return 0;
			} else if constexpr (hashData.type == hash_map_type::double_element) {
				return iter[hashData.uniqueIndex] & 1;
			} else if constexpr (hashData.type == hash_map_type::triple_element) {
				static constexpr auto firstChar = std::get<0>(jsonifier::concepts::coreV<value_type>).view()[hashData.uniqueIndex];
				return (uint8_t(iter[hashData.uniqueIndex] ^ firstChar) * hashData.hasher.seed) & 3;
			} else if constexpr (hashData.type == hash_map_type::single_byte) {
				return hashData.uniqueIndices[iter[hashData.uniqueIndex]];
			} else if constexpr (hashData.type == hash_map_type::first_byte_and_unique_index) {
				static constexpr auto uniqueFirstByteCount{ countFirstBytes(tupleReferencesByFirstByte<value_type>) };
				static constexpr auto mappings{ generateMappingsForFirstBytes(collectFirstBytes<uniqueFirstByteCount>(tupleReferencesByFirstByte<value_type>),
					hashData.uniqueIndices) };
				const uint8_t firstByte	  = static_cast<uint8_t>(iter[0]);
				const size_t uniqueIndex  = hashData.uniqueIndices[firstByte];
				const uint8_t keyChar	  = static_cast<uint8_t>(iter[uniqueIndex]);
				const size_t flattenedIdx = firstByte * 256 + keyChar;
				return mappings[flattenedIdx];
			} else if constexpr (hashData.type == hash_map_type::unique_byte_and_length) {
				const auto newPtr = char_comparison<'"', unwrap_t<decltype(*iter)>>::memchar(iter + subAmount01, subAmount02);
				if (newPtr) [[likely]] {
					return hashData.indices[(iter[hashData.uniqueIndex] ^ static_cast<size_t>(newPtr - (iter))) & (hashData.storageSize - 1)];
				}
				return hashData.storageSize;
			} else if constexpr (hashData.type == hash_map_type::unique_per_length) {
				static constexpr auto mappings = generateMappingsForLengths<hashData.keyStatsVal.maxLength>(tupleReferencesByLength<value_type>, hashData.uniqueIndices);
				const auto newPtr			   = char_comparison<'"', unwrap_t<decltype(*iter)>>::memchar(iter + subAmount01, subAmount02);
				if (newPtr) [[likely]] {
					const auto length			= static_cast<size_t>(newPtr - (iter));
					const auto localUniqueIndex = hashData.uniqueIndices[length];
					if (localUniqueIndex == 255) {
						return hashData.storageSize;
					}

					return mappings[(length * 256) + iter[localUniqueIndex]];
				}
				return hashData.storageSize;
			} else if constexpr (hashData.type == hash_map_type::simd_full_length) {
				using simd_type = typename unwrap_t<decltype(hashData)>::simd_type;
				static constexpr rt_key_hasher<hashData.hasher.seed> hasher{};
				const auto newPtr = char_comparison<'"', unwrap_t<decltype(*iter)>>::memchar(iter + subAmount01, subAmount02);
				if (newPtr) [[likely]] {
					size_t length		   = static_cast<size_t>(newPtr - (iter));
					length				   = hashData.uniqueIndex > length ? length : hashData.uniqueIndex;
					const auto hash		   = hasher.hashKeyRt(iter, length);
					const auto resultIndex = ((hash >> 8) & (hashData.numGroups - 1)) * hashData.bucketSize;
					return hashData.indices[(simd_internal::tzcnt(simd_internal::opCmpEq(simd_internal::gatherValue<simd_type>(static_cast<uint8_t>(hash)),
												 simd_internal::gatherValues<simd_type>(hashData.controlBytes.data() + resultIndex))) +
						resultIndex)];
				}
				return hashData.storageSize;
			} else if constexpr (hashData.type == hash_map_type::empty) {
				return 0;
			} else {
				static_assert(hashData.type != hash_map_type::unset, "Sorry, but we failed to construct this hash-map!");
				return false;
			}
		}
	};

}