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
/// Note: Some of the code in this header was sampled from Glaze library: https://github.com/StephenBerry/Glaze
/// https://github.com/RealTimeChris/jsonifier
/// Feb 20, 2023
#pragma once

#include <jsonifier/Core.hpp>
#include <jsonifier/Hash.hpp>
#include <jsonifier/StringView.hpp>
#include <jsonifier/Array.hpp>
#include <jsonifier/Reflection.hpp>
#include <jsonifier/Tuple.hpp>
#include <algorithm>
#include <numeric>
#include <utility>

namespace jsonifier_internal {

	template<typename value_type, size_t size> std::ostream& operator<<(std::ostream& os, const array<value_type, size>& values) {
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

	template<typename value_type01, typename value_type02> constexpr bool contains(const value_type01* hashData, value_type02 byteToCheckFor, size_t size) noexcept {
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
		single_element				= 3,
		double_element				= 4,
		triple_element				= 5,
		single_byte					= 6,
		first_byte_and_unique_index = 7,
		unique_byte_and_length		= 8,
		unique_per_length			= 9,
		simd_full_length			= 10,
	};

	static constexpr size_t setSimdWidth(size_t length) noexcept {
		return length >= 64ull && bytesPerStep >= 64ull ? 64ull : length >= 32ull && bytesPerStep >= 32ull ? 32ull : 16ull;
	}

	struct key_stats_t {
		size_t minLength{ (std::numeric_limits<size_t>::max)() };
		size_t uniqueIndex{};
		size_t maxLength{};
	};

	static constexpr size_t findUniqueColumnIndex(const tuple_references& tupleRefs, size_t maxIndex, size_t startingIndex = 0) noexcept {
		constexpr size_t alphabetSize = 256;
		jsonifier::string_view key{};
		for (size_t index = startingIndex; index < maxIndex; ++index) {
			array<bool, alphabetSize> seen{};
			bool allDifferent = true;

			for (size_t x = 0; x < tupleRefs.count; ++x) {
				key				  = tupleRefs.rootPtr[x].key;
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

		return std::numeric_limits<size_t>::max();
	}

	template<typename value_type> struct hash_map_construction_data {
		using simd_type = map_simd_t<2048>;
		array<size_t, 2048 / setSimdWidth(2048)> bucketSizes{};
		JSONIFIER_ALIGN array<uint8_t, 2049> controlBytes{};
		array<uint8_t, 256> uniqueIndices{};
		array<size_t, 2049> indices{};
		size_t bucketSize{ setSimdWidth(2048) };
		size_t numGroups{ 2048 / bucketSize };
		array<size_t, 256> jsonTypeIndices{};
		ct_key_hasher hasher{};
		hash_map_type type{};
		size_t uniqueIndex{};
		char firstChar{};

		constexpr hash_map_construction_data() noexcept = default;
	};

	template<typename value_type> struct empty_data {
		constexpr empty_data(const hash_map_construction_data<value_type>& newData) noexcept : type{ newData.type } {};
		hash_map_type type{};
	};

	template<typename value_type> struct unique_types_data {
		static constexpr size_t storageSize{ 6 };
		constexpr unique_types_data(const hash_map_construction_data<value_type>& newData) noexcept : jsonTypeIndices{ newData.jsonTypeIndices }, type{ newData.type } {};
		array<size_t, 256> jsonTypeIndices{};
		hash_map_type type{};
	};

	template<typename value_type> struct single_element_data {
		static constexpr size_t storageSize{ 1 };
		constexpr single_element_data(const hash_map_construction_data<value_type>& newData) noexcept : type{ newData.type } {};
		hash_map_type type{};
	};

	template<typename value_type> struct double_element_data {
		static constexpr size_t storageSize{ 2 };
		constexpr double_element_data(const hash_map_construction_data<value_type>& newData) noexcept : uniqueIndex{ newData.uniqueIndex }, type{ newData.type } {};
		size_t uniqueIndex{};
		hash_map_type type{};
	};

	template<typename value_type> struct triple_element_data {
		static constexpr size_t storageSize{ 3 };
		constexpr triple_element_data(const hash_map_construction_data<value_type>& newData) noexcept
			: uniqueIndex{ newData.uniqueIndex }, type{ newData.type }, firstChar{ newData.firstChar }, seed{ newData.hasher.seed } {};
		size_t uniqueIndex{};
		hash_map_type type{};
		char firstChar{};
		size_t seed{};
	};

	template<typename value_type> struct single_byte_data {
		static constexpr size_t storageSize{ 256 };
		constexpr single_byte_data(const hash_map_construction_data<value_type>& newData) noexcept
			: uniqueIndices{ newData.uniqueIndices }, uniqueIndex{ newData.uniqueIndex }, type{ newData.type } {};
		JSONIFIER_ALIGN array<uint8_t, 256> uniqueIndices{};
		size_t uniqueIndex{};
		hash_map_type type{};
	};

	template<typename value_type> struct first_byte_and_unique_index_data {
		static constexpr size_t storageSize{ 256 };
		constexpr first_byte_and_unique_index_data(const hash_map_construction_data<value_type>& newData) noexcept
			: uniqueIndices{ newData.uniqueIndices }, type{ newData.type } {};
		JSONIFIER_ALIGN array<uint8_t, 256> uniqueIndices{};
		hash_map_type type{};
	};

	template<typename value_type> struct unique_byte_and_length_data {
		static constexpr size_t storageSize{ 2048 };
		constexpr unique_byte_and_length_data(const hash_map_construction_data<value_type>& newData) noexcept
			: indices{ newData.indices }, uniqueIndex{ newData.uniqueIndex }, type{ newData.type } {};
		JSONIFIER_ALIGN array<size_t, 2049> indices{};
		size_t uniqueIndex{};
		hash_map_type type{};
	};

	template<typename value_type> struct unique_per_length_data {
		static constexpr size_t storageSize{ 256 };
		constexpr unique_per_length_data(const hash_map_construction_data<value_type>& newData) noexcept : uniqueIndices{ newData.uniqueIndices }, type{ newData.type } {};
		JSONIFIER_ALIGN array<uint8_t, 256> uniqueIndices{};
		hash_map_type type{};
	};

	template<typename value_type> struct simd_full_length_data {
		static constexpr size_t storageSize{ 2048 };
		constexpr simd_full_length_data(const hash_map_construction_data<value_type>& newData) noexcept
			: controlBytes{ newData.controlBytes }, indices{ newData.indices }, bucketSize{ newData.bucketSize }, numGroups{ newData.numGroups },
			  uniqueIndex{ newData.uniqueIndex }, type{ newData.type }, seed{ newData.hasher.seed } {};
		JSONIFIER_ALIGN array<uint8_t, 2049> controlBytes{};
		JSONIFIER_ALIGN array<size_t, 2049> indices{};
		size_t bucketSize{ setSimdWidth(2048) };
		size_t numGroups{ 2048 / bucketSize };
		size_t uniqueIndex{};
		hash_map_type type{};
		size_t seed{};
	};

	struct string_lengths : public tuple_references {
		size_t length{};
	};

	constexpr auto countUniqueLengths(const tuple_references& tupleRefs) {
		array<size_t, 256> stringLengths{};
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
		array<size_t, 256> lengths{};
		array<string_lengths, stringLengthCount> valuesNew{};
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
		array<bool, 256> stringLengths{};
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
		array<size_t, 256> lengths{};
		array<first_bytes, firstByteCount> valuesNew{};
		size_t currentIndex{};
		for (uint64_t x = 0; x < values.count; ++x) {
			if (lengths[static_cast<uint8_t>(values.rootPtr[x].key[0])] == 0) {
				++lengths[static_cast<uint8_t>(values.rootPtr[x].key[0])];
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

	constexpr auto keyStatsImpl(const tuple_references& tupleRefs) noexcept {
		key_stats_t stats{};
		for (size_t x = 0; x < tupleRefs.count; ++x) {
			const jsonifier::string_view& key{ tupleRefs.rootPtr[x].key };
			auto num{ key.size() };
			if (num > stats.maxLength) {
				stats.maxLength = num;
			}
			if (num < stats.minLength) {
				stats.minLength = num;
			}
		}
		stats.uniqueIndex = findUniqueColumnIndex(tupleRefs, stats.minLength);
		return stats;
	}

	template<typename value_type> constexpr auto keyLengths = collectLengths<countUniqueLengths(tupleReferencesByLength<value_type>)>(tupleReferencesByLength<value_type>);

	template<size_t size> constexpr auto keyStats(const array<first_bytes, size>& tupleRefs) noexcept {
		array<key_stats_t, size> returnValues{};
		for (uint64_t x = 0; x < size; ++x) {
			returnValues[x] = keyStatsImpl(static_cast<const tuple_references&>(tupleRefs[x]));
		}
		return returnValues;
	}

	template<size_t size> constexpr auto keyStats(const array<string_lengths, size>& tupleRefs) noexcept {
		array<key_stats_t, size> returnValues{};
		for (uint64_t x = 0; x < size; ++x) {
			returnValues[x] = keyStatsImpl(static_cast<const tuple_references&>(tupleRefs[x]));
		}
		return returnValues;
	}

	template<typename value_type> constexpr auto keyStatsVal = keyStatsImpl(tupleReferencesByFirstByte<value_type>);

	template<typename value_type> constexpr auto collectSimdFullLengthHashMapData(const tuple_references& pairsNew) noexcept {
		hash_map_construction_data<value_type> returnValues{};
		bool collided{};
		for (size_t w = keyStatsVal<value_type>.minLength; w < keyStatsVal<value_type>.maxLength; ++w) {
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
					returnValues.indices[slot]		= y;
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
			returnValues.type = hash_map_type::simd_full_length;
			return returnValues;
		}
	}

	template<typename value_type> constexpr auto collectUniquePerLengthHashMapData(const tuple_references& pairsNew) {
		constexpr auto uniqueLengthCount = countUniqueLengths(tupleReferencesByLength<value_type>);
		constexpr auto results			 = collectLengths<uniqueLengthCount>(tupleReferencesByLength<value_type>);
		constexpr auto keyStatsValNew	 = keyStats(results);
		hash_map_construction_data<value_type> returnValues{};
		returnValues.uniqueIndices.fill(static_cast<uint8_t>(returnValues.uniqueIndices.size() - 1));
		for (size_t x = 0; x < uniqueLengthCount; ++x) {
			auto uniqueIndex = findUniqueColumnIndex(results[x], keyStatsValNew[x].minLength);
			if (uniqueIndex == std::numeric_limits<size_t>::max()) {
				return collectSimdFullLengthHashMapData<value_type>(pairsNew);
			} else {
				returnValues.uniqueIndices[results[x].length] = static_cast<uint8_t>(uniqueIndex);
			}
		}
		returnValues.type = hash_map_type::unique_per_length;
		return returnValues;
	}

	template<typename value_type> constexpr auto collectUniqueByteAndLengthHashMapData(const tuple_references& pairsNew) noexcept {
		hash_map_construction_data<value_type> returnValues{};
		bool collided{ true };
		while (returnValues.uniqueIndex < keyStatsVal<value_type>.minLength) {
			returnValues.indices.fill(returnValues.indices.size() - 1);
			collided = false;
			for (size_t x = 0; x < pairsNew.count; ++x) {
				const auto hash = pairsNew.rootPtr[x].key[returnValues.uniqueIndex] ^ pairsNew.rootPtr[x].key.size();
				const auto slot = hash % 2048;
				if (returnValues.indices[slot] != returnValues.indices.size() - 1) {
					collided = true;
					break;
				}
				returnValues.indices[slot] = x;
			}
			if (!collided) {
				break;
			}
			++returnValues.uniqueIndex;
		}
		if (collided) {
			return collectUniquePerLengthHashMapData<value_type>(pairsNew);
		} else {
			returnValues.type = hash_map_type::unique_byte_and_length;
			return returnValues;
		}
	}

	template<typename value_type> constexpr auto collectFirstByteAndUniqueIndexHashMapData(const tuple_references& pairsNew) {
		constexpr auto keyStatsValNewer		= keyStatsImpl(tupleReferencesByFirstByte<value_type>);
		constexpr auto uniqueFirstByteCount = countFirstBytes(tupleReferencesByFirstByte<value_type>);
		constexpr auto results				= collectFirstBytes<uniqueFirstByteCount>(tupleReferencesByFirstByte<value_type>);
		constexpr auto keyStatsValNew		= keyStats(results);
		hash_map_construction_data<value_type> returnValues{};
		returnValues.uniqueIndices.fill(static_cast<uint8_t>(returnValues.uniqueIndices.size() - 1));
		if (keyStatsValNewer.maxLength < 256) {
			for (size_t x = 0; x < uniqueFirstByteCount; ++x) {
				auto uniqueIndex = findUniqueColumnIndex(results[x], keyStatsValNew[x].minLength);
				if (uniqueIndex == std::numeric_limits<size_t>::max()) {
					return collectUniqueByteAndLengthHashMapData<value_type>(pairsNew);
				} else {
					returnValues.uniqueIndices[static_cast<uint8_t>(results[x].rootPtr[0].key[0])] = static_cast<uint8_t>(uniqueIndex);
				}
			}
		} else {
			return collectUniqueByteAndLengthHashMapData<value_type>(pairsNew);
		}
		returnValues.type = hash_map_type::first_byte_and_unique_index;
		return returnValues;
	}

	template<typename value_type> constexpr auto collectSingleByteHashMapData(const tuple_references& pairsNew) noexcept {
		hash_map_construction_data<value_type> returnValues{};
		returnValues.uniqueIndex = keyStatsVal<value_type>.uniqueIndex;
		if (returnValues.uniqueIndex != std::numeric_limits<size_t>::max()) {
			returnValues.uniqueIndices.fill(static_cast<uint8_t>(returnValues.uniqueIndices.size() - 1));
			for (size_t x = 0; x < pairsNew.count; ++x) {
				const auto slot					 = static_cast<uint8_t>(pairsNew.rootPtr[x].key.data()[returnValues.uniqueIndex]);
				returnValues.uniqueIndices[slot] = static_cast<uint8_t>(x);
			}
			returnValues.type = hash_map_type::single_byte;
			return returnValues;
		} else {
			return collectFirstByteAndUniqueIndexHashMapData<value_type>(pairsNew);
		}
	}

	template<typename value_type, size_t currentIndex, size_t maxIndex> constexpr bool areTypesUnique(array<size_t, 7> types = array<size_t, 7>{}) {
		if constexpr (currentIndex < maxIndex) {
			auto type = get<currentIndex>(jsonifier::concepts::coreV<value_type>).type;
			++types[static_cast<size_t>(type)];
			return areTypesUnique<value_type, currentIndex + 1, maxIndex>(types);
		} else {
			for (size_t x = 0; x < types.size(); ++x) {
				if (types[x] > 1) {
					return false;
				}
			}
			return true;
		}
	}

	template<typename value_type> constexpr auto collectTripleElementHashMapData(const tuple_references& pairsNew) noexcept {
		hash_map_construction_data<value_type> returnValues{};
		returnValues.uniqueIndex = keyStatsVal<value_type>.uniqueIndex;
		bool collided{ true };
		while (returnValues.uniqueIndex != std::numeric_limits<size_t>::max()) {
			returnValues.firstChar = static_cast<char>(static_cast<uint8_t>(pairsNew.rootPtr[0].key[returnValues.uniqueIndex]));
			const auto mix1		   = static_cast<uint8_t>(pairsNew.rootPtr[1].key[returnValues.uniqueIndex]) ^ returnValues.firstChar;
			const auto mix2		   = static_cast<uint8_t>(pairsNew.rootPtr[2].key[returnValues.uniqueIndex]) ^ returnValues.firstChar;
			for (size_t x = 0; x < 4; ++x) {
				uint8_t hash0 = 0 & 3;
				uint8_t hash1 = (mix1 * returnValues.hasher.seed) & 3;
				uint8_t hash2 = (mix2 * returnValues.hasher.seed) & 3;

				if (hash0 == 0 && hash1 == 1 && hash2 == 2) {
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
			return collectSingleByteHashMapData<value_type>(pairsNew);
		}
		returnValues.type = hash_map_type::triple_element;
		return returnValues;
	}

	template<typename value_type> constexpr auto collectDoubleElementHashMapData(const tuple_references& pairsNew) noexcept {
		hash_map_construction_data<value_type> returnValues{};
		returnValues.uniqueIndex = keyStatsVal<value_type>.uniqueIndex;
		bool collided{ true };
		while (returnValues.uniqueIndex != std::numeric_limits<size_t>::max()) {
			if ((pairsNew.rootPtr[0].key[returnValues.uniqueIndex] & 1u) == 0 && (pairsNew.rootPtr[1].key[returnValues.uniqueIndex] & 1u) == 1u) {
				collided = false;
				break;
			}
			returnValues.uniqueIndex = findUniqueColumnIndex(pairsNew, keyStatsVal<value_type>.minLength, returnValues.uniqueIndex + 1);
		}
		if (collided) {
			return collectSingleByteHashMapData<value_type>(pairsNew);
		}
		returnValues.type = hash_map_type::double_element;
		return returnValues;
	}

	template<typename value_type> constexpr auto collectMapConstructionDataImpl() noexcept {
		if constexpr (tupleReferencesByLength<value_type>.count == 0) {
			hash_map_construction_data<value_type> returnValues{};
			returnValues.type = hash_map_type::empty;
			return returnValues;
		} else if constexpr (tupleReferencesByLength<value_type>.count == 1) {
			hash_map_construction_data<value_type> returnValues{};
			returnValues.type = hash_map_type::single_element;
			return returnValues;
		} else {
			if constexpr (keyStatsVal<value_type>.uniqueIndex != std::numeric_limits<size_t>::max()) {
				if constexpr (tupleReferencesByLength<value_type>.count == 2) {
					return collectDoubleElementHashMapData<value_type>(tupleReferencesByLength<value_type>);
				} else if constexpr (tupleReferencesByLength<value_type>.count == 3) {
					return collectTripleElementHashMapData<value_type>(tupleReferencesByLength<value_type>);
				} else {
					return collectSingleByteHashMapData<value_type>(tupleReferencesByLength<value_type>);
				}
			} else {
				return collectFirstByteAndUniqueIndexHashMapData<value_type>(tupleReferencesByLength<value_type>);
			}
		}
	}

	template<typename value_type> constexpr auto collectMapConstructionData() noexcept {
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

	template<size_t keyMaxLength> constexpr auto generateMappingsForLengths(const tuple_references& keys, const array<uint8_t, 256>& uniqueIndices) noexcept {
		array<size_t, (keyMaxLength + 1) * 256> mappings{};

		mappings.fill(static_cast<size_t>(-1));

		for (size_t x = 0; x < keys.count; ++x) {
			const auto& key = keys.rootPtr[x].key;

			uint8_t uniqueIndex = uniqueIndices[key.size()];

			if (uniqueIndex != 255 && uniqueIndex < key.size()) {
				uint8_t keyChar		= static_cast<uint8_t>(key[uniqueIndex]);
				size_t flatIndex	= key.size() * 256 + keyChar;
				mappings[flatIndex] = x;
			}
		}

		return mappings;
	}

	template<size_t firstCharCount>
	constexpr auto generateMappingsForFirstBytes(const array<first_bytes, firstCharCount>& keys, const array<uint8_t, 256>& uniqueIndices) noexcept {
		constexpr size_t flattenedSize = 256ull * 256ull;
		array<size_t, flattenedSize> flattenedMappings{};
		flattenedMappings.fill(flattenedMappings.size() - 1);

		for (size_t x = 0; x < firstCharCount; ++x) {
			const auto& key	   = keys[x].rootPtr[0].key;
			uint8_t firstByte  = static_cast<uint8_t>(key[0]);
			size_t uniqueIndex = uniqueIndices[firstByte];

			for (size_t y = 0; y < keys[x].count; ++y) {
				const auto& keyNew = keys[x].rootPtr[y].key;
				if (uniqueIndex < keyNew.size()) {
					uint8_t keyChar					= static_cast<uint8_t>(keyNew[uniqueIndex]);
					size_t flattenedIdx				= firstByte * 256ull + keyChar;
					flattenedMappings[flattenedIdx] = keys[x].rootPtr[y].oldIndex;
				}
			}
		}

		return flattenedMappings;
	}

#if !defined(NDEBUG)
	inline std::unordered_map<std::string, uint32_t> types{};
#endif

	template<typename value_type> static constexpr auto hashData = collectMapConstructionData<std::remove_cvref_t<value_type>>();

	template<typename value_type, typename iterator_newer> struct hash_map {
		static constexpr auto subAmount01{ []() constexpr {
			return ((keyStatsVal<value_type>.maxLength - keyStatsVal<value_type>.minLength) >= bytesPerStep) ? keyStatsVal<value_type>.minLength : 0;
		}() };

		static constexpr auto subAmount02{ []() constexpr {
			return ((keyStatsVal<value_type>.maxLength - keyStatsVal<value_type>.minLength) >= bytesPerStep)
				? (keyStatsVal<value_type>.maxLength - keyStatsVal<value_type>.minLength + 2)
				: (keyStatsVal<value_type>.maxLength + 2);
		}() };

		JSONIFIER_ALWAYS_INLINE static size_t findIndex(iterator_newer iter, iterator_newer end) noexcept {
			static constexpr auto checkForEnd = [](const auto& iter, const auto& end, const auto distance) {
				return (iter + distance) < end;
			};
#if !defined(NDEBUG)
			if (!types.contains(typeid(value_type).name())) {
				types[typeid(value_type).name()] = static_cast<uint32_t>(hashData<value_type>.type);
			}
#endif
			if constexpr (hashData<value_type>.type == hash_map_type::single_element) {
				return *(iter + keyStatsVal<value_type>.maxLength) == '"' ? 0ull : 1ull;
			} else if constexpr (hashData<value_type>.type == hash_map_type::double_element) {
				if JSONIFIER_LIKELY (checkForEnd(iter, end, hashData<value_type>.uniqueIndex)) {
					return iter[static_cast<uint8_t>(hashData<value_type>.uniqueIndex)] & 1u;
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
				static constexpr auto uniqueFirstByteCount{ countFirstBytes(tupleReferencesByFirstByte<value_type>) };
				static constexpr auto mappings{ generateMappingsForFirstBytes(collectFirstBytes<uniqueFirstByteCount>(tupleReferencesByFirstByte<value_type>),
					hashData<value_type>.uniqueIndices) };
				const uint8_t firstByte = static_cast<uint8_t>(iter[0]);
				const uint8_t uniqueIdx = hashData<value_type>.uniqueIndices[firstByte];
				if JSONIFIER_LIKELY (checkForEnd(iter, end, uniqueIdx)) {
					const uint8_t keyChar	  = static_cast<uint8_t>(iter[uniqueIdx]);
					const size_t flattenedIdx = (static_cast<size_t>(firstByte) << 8) | static_cast<size_t>(keyChar);
					return mappings[flattenedIdx];
				}
				return hashData<value_type>.storageSize;
			} else if constexpr (hashData<value_type>.type == hash_map_type::unique_byte_and_length) {
				static constexpr size_t storageMask = hashData<value_type>.storageSize - 1;
				const auto newPtr					= char_comparison<'"', std::remove_cvref_t<decltype(*iter)>>::memchar(iter + subAmount01, subAmount02);
				if JSONIFIER_LIKELY (newPtr) {
					const size_t length = static_cast<size_t>(newPtr - iter);
					if JSONIFIER_LIKELY (checkForEnd(iter, end, hashData<value_type>.uniqueIndex)) {
						const size_t combinedKey = iter[hashData<value_type>.uniqueIndex] ^ length;
						return hashData<value_type>.indices[combinedKey & storageMask];
					}
				}
				return hashData<value_type>.storageSize;
			} else if constexpr (hashData<value_type>.type == hash_map_type::unique_per_length) {
				static constexpr auto mappings =
					generateMappingsForLengths<keyStatsVal<value_type>.maxLength>(tupleReferencesByLength<value_type>, hashData<value_type>.uniqueIndices);
				const auto newPtr = char_comparison<'"', std::remove_cvref_t<decltype(*iter)>>::memchar(iter + subAmount01, subAmount02);
				if JSONIFIER_LIKELY (newPtr) {
					const size_t length			= static_cast<size_t>(newPtr - iter);
					const size_t localUniqueIdx = hashData<value_type>.uniqueIndices[length];
					if JSONIFIER_LIKELY (localUniqueIdx != 255 && checkForEnd(iter, end, localUniqueIdx)) {
						return mappings[(length << 8) | iter[localUniqueIdx]];
					}
				}
				return hashData<value_type>.storageSize;
			} else if constexpr (hashData<value_type>.type == hash_map_type::simd_full_length) {
				using simd_type = map_simd_t<hashData<value_type>.storageSize>;
				static constexpr rt_key_hasher<hashData<value_type>.seed> hasher{};
				static constexpr auto sizeMask{ hashData<value_type>.numGroups - 1u };
				static constexpr auto ctrlBytesPtr{ hashData<value_type>.controlBytes.data() };
				const auto newPtr = char_comparison<'"', std::remove_cvref_t<decltype(*iter)>>::memchar(iter + subAmount01, subAmount02);
				if JSONIFIER_LIKELY (newPtr) {
					size_t length = static_cast<size_t>(newPtr - iter);
					length		  = (hashData<value_type>.uniqueIndex > length) ? length : hashData<value_type>.uniqueIndex;
					if JSONIFIER_LIKELY (checkForEnd(iter, end, length)) {
						const auto hash			 = hasher.hashKeyRt(iter, length);
						const size_t group		 = (hash >> 8) & (sizeMask);
						const size_t resultIndex = group * hashData<value_type>.bucketSize;
						uint64_t matches;
#if JSONIFIER_CHECK_FOR_INSTRUCTION(JSONIFIER_NEON)
						uint8x8_t dup				   = vdup_n_u8(hash);
						auto mask					   = vceq_u8(ctrlBytesPtr + resultIndex, dup);
						static constexpr uint64_t msbs = 0x8080808080808080ULL;
						matches						   = vget_lane_u64(vreinterpret_u64_u8(mask), 0) & msbs;
#else
						matches = simd_internal::opCmpEq(simd_internal::gatherValue<simd_type>(static_cast<uint8_t>(hash)),
							simd_internal::gatherValues<simd_type>(ctrlBytesPtr + resultIndex));
#endif
						const size_t tz = simd_internal::postCmpTzcnt(matches);
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
