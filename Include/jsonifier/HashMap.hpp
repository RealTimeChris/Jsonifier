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
	concept has_view = requires(std::remove_cvref_t<value_type> value) { value.view(); };

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

	template<typename value_type> JSONIFIER_ALWAYS_INLINE_VARIABLE auto tupleRefs{ collectTupleRefs(jsonifier::concepts::coreV<value_type>) };
	template<typename value_type> JSONIFIER_ALWAYS_INLINE_VARIABLE auto sortedTupleReferencesByLength{ sortTupleRefsByLength(tupleRefs<value_type>) };
	template<typename value_type> JSONIFIER_ALWAYS_INLINE_VARIABLE auto tupleReferencesByLength{ consolidateTupleRefs(sortedTupleReferencesByLength<value_type>) };

	template<typename value_type, size_t... indices> JSONIFIER_ALWAYS_INLINE constexpr auto createNewTupleImpl(std::index_sequence<indices...>) noexcept {
		return std::make_tuple(std::get<sortedTupleReferencesByLength<value_type>[indices].oldIndex>(jsonifier::concepts::coreV<value_type>)...);
	}

	template<typename value_type> JSONIFIER_ALWAYS_INLINE constexpr auto createNewTuple() noexcept {
		constexpr auto& tupleRefs = sortedTupleReferencesByLength<value_type>;
		return createNewTupleImpl<value_type>(std::make_index_sequence<tupleRefs.size()>{});
	}

	template<typename value_type> JSONIFIER_ALWAYS_INLINE_VARIABLE auto coreTupleV{ createNewTuple<std::remove_cvref_t<value_type>>() };

	template<typename value_type> JSONIFIER_ALWAYS_INLINE_VARIABLE auto tupleRefsByLength{ collectTupleRefs(coreTupleV<value_type>) };
	template<typename value_type> JSONIFIER_ALWAYS_INLINE_VARIABLE auto sortedTupleReferencesByFirstByte{ sortTupleRefsByFirstByte(tupleRefsByLength<value_type>) };
	template<typename value_type> JSONIFIER_ALWAYS_INLINE_VARIABLE auto tupleReferencesByFirstByte{ consolidateTupleRefs(sortedTupleReferencesByFirstByte<value_type>) };

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
		std::array<size_t, 2048 / setSimdWidth(2048)> bucketSizes{};
		JSONIFIER_ALIGN std::array<uint8_t, 2049> controlBytes{};
		JSONIFIER_ALIGN std::array<uint8_t, 256> uniqueIndices{};
		JSONIFIER_ALIGN std::array<size_t, 2049> indices{};
		size_t bucketSize{ setSimdWidth(2048) };
		size_t numGroups{ 2048 / bucketSize };
		ct_key_hasher hasher{};
		hash_map_type type{};
		size_t uniqueIndex{};
		char firstChar{};

		JSONIFIER_ALWAYS_INLINE constexpr hash_map_construction_data() noexcept = default;
	};

	template<typename value_type> struct empty_data {
		JSONIFIER_ALWAYS_INLINE constexpr empty_data(const hash_map_construction_data<value_type>& newData) noexcept : type{ newData.type } {};
		hash_map_type type{};
	};

	template<typename value_type> struct single_element_data {
		static constexpr size_t storageSize{ 1 };
		JSONIFIER_ALWAYS_INLINE constexpr single_element_data(const hash_map_construction_data<value_type>& newData) noexcept : type{ newData.type } {};
		hash_map_type type{};
	};

	template<typename value_type> struct double_element_data {
		static constexpr size_t storageSize{ 2 };
		JSONIFIER_ALWAYS_INLINE constexpr double_element_data(const hash_map_construction_data<value_type>& newData) noexcept
			: uniqueIndex{ newData.uniqueIndex }, type{ newData.type } {};
		size_t uniqueIndex{};
		hash_map_type type{};
	};

	template<typename value_type> struct triple_element_data {
		static constexpr size_t storageSize{ 3 };
		JSONIFIER_ALWAYS_INLINE constexpr triple_element_data(const hash_map_construction_data<value_type>& newData) noexcept
			: uniqueIndex{ newData.uniqueIndex }, type{ newData.type }, firstChar{ newData.firstChar }, seed{ newData.hasher.seed } {};
		size_t uniqueIndex{};
		hash_map_type type{};
		char firstChar{};
		size_t seed{};
	};

	template<typename value_type> struct single_byte_data {
		static constexpr size_t storageSize{ 256 };
		JSONIFIER_ALWAYS_INLINE constexpr single_byte_data(const hash_map_construction_data<value_type>& newData) noexcept
			: uniqueIndices{ newData.uniqueIndices }, uniqueIndex{ newData.uniqueIndex }, type{ newData.type } {};
		JSONIFIER_ALIGN std::array<uint8_t, 256> uniqueIndices{};
		size_t uniqueIndex{};
		hash_map_type type{};
	};

	template<typename value_type> struct first_byte_and_unique_index_data {
		static constexpr size_t storageSize{ 256 };
		JSONIFIER_ALWAYS_INLINE constexpr first_byte_and_unique_index_data(const hash_map_construction_data<value_type>& newData) noexcept
			: uniqueIndices{ newData.uniqueIndices }, type{ newData.type } {};
		JSONIFIER_ALIGN std::array<uint8_t, 256> uniqueIndices{};
		hash_map_type type{};
	};

	template<typename value_type> struct unique_byte_and_length_data {
		static constexpr size_t storageSize{ 2048 };
		JSONIFIER_ALWAYS_INLINE constexpr unique_byte_and_length_data(const hash_map_construction_data<value_type>& newData) noexcept
			: indices{ newData.indices }, uniqueIndex{ newData.uniqueIndex }, type{ newData.type } {};
		JSONIFIER_ALIGN std::array<size_t, 2049> indices{};
		size_t uniqueIndex{};
		hash_map_type type{};
	};

	template<typename value_type> struct unique_per_length_data {
		static constexpr size_t storageSize{ 256 };
		JSONIFIER_ALWAYS_INLINE constexpr unique_per_length_data(const hash_map_construction_data<value_type>& newData) noexcept
			: uniqueIndices{ newData.uniqueIndices }, type{ newData.type } {};
		JSONIFIER_ALIGN std::array<uint8_t, 256> uniqueIndices{};
		hash_map_type type{};
	};

	template<typename value_type> struct simd_full_length_data {
		static constexpr size_t storageSize{ 2048 };
		JSONIFIER_ALWAYS_INLINE constexpr simd_full_length_data(const hash_map_construction_data<value_type>& newData) noexcept
			: controlBytes{ newData.controlBytes }, indices{ newData.indices }, bucketSize{ newData.bucketSize }, numGroups{ newData.numGroups },
			  uniqueIndex{ newData.uniqueIndex }, type{ newData.type }, seed{ newData.hasher.seed } {};
		JSONIFIER_ALIGN std::array<uint8_t, 2049> controlBytes{};
		JSONIFIER_ALIGN std::array<size_t, 2049> indices{};
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

	template<typename value_type> constexpr auto keyStatsVal = keyStatsImpl(tupleReferencesByFirstByte<value_type>);

	template<typename value_type> JSONIFIER_ALWAYS_INLINE constexpr auto collectSimdFullLengthHashMapData(const tuple_references& pairsNew) noexcept {
		hash_map_construction_data<value_type> returnValues{};
		bool collided{};
		for (size_t w = 0; w < keyStatsVal<value_type>.maxLength; ++w) {
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

	template<typename value_type> JSONIFIER_ALWAYS_INLINE constexpr auto collectUniqueByteAndLengthHashMapData(const tuple_references& pairsNew) noexcept {
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
					returnValues.uniqueIndices[results[x].rootPtr[0].key[0]] = static_cast<uint8_t>(uniqueIndex);
				}
			}
		} else {
			return collectUniqueByteAndLengthHashMapData<value_type>(pairsNew);
		}
		returnValues.type = hash_map_type::first_byte_and_unique_index;
		return returnValues;
	}

	template<typename value_type> JSONIFIER_ALWAYS_INLINE constexpr auto collectSingleByteHashMapData(const tuple_references& pairsNew) noexcept {
		hash_map_construction_data<value_type> returnValues{};
		returnValues.uniqueIndex = keyStatsVal<value_type>.uniqueIndex;
		if (returnValues.uniqueIndex != std::numeric_limits<size_t>::max()) {
			returnValues.uniqueIndices.fill(returnValues.uniqueIndices.size() - 1);
			for (size_t x = 0; x < pairsNew.count; ++x) {
				const auto slot					 = pairsNew.rootPtr[x].key.data()[returnValues.uniqueIndex];
				returnValues.uniqueIndices[slot] = static_cast<uint8_t>(x);
			}
			returnValues.type = hash_map_type::single_byte;
			return returnValues;
		} else {
			return collectFirstByteAndUniqueIndexHashMapData<value_type>(pairsNew);
		}
	}

	template<typename value_type> JSONIFIER_ALWAYS_INLINE constexpr auto collectTripleElementHashMapData(const tuple_references& pairsNew) noexcept {
		hash_map_construction_data<value_type> returnValues{};
		returnValues.uniqueIndex = keyStatsVal<value_type>.uniqueIndex;
		bool collided{ true };
		while (returnValues.uniqueIndex != std::numeric_limits<size_t>::max()) {
			returnValues.firstChar = static_cast<uint8_t>(pairsNew.rootPtr[0].key[returnValues.uniqueIndex]);
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

	template<typename value_type> JSONIFIER_ALWAYS_INLINE constexpr auto collectDoubleElementHashMapData(const tuple_references& pairsNew) noexcept {
		hash_map_construction_data<value_type> returnValues{};
		returnValues.uniqueIndex = keyStatsVal<value_type>.uniqueIndex;
		bool collided{ true };
		while (returnValues.uniqueIndex != std::numeric_limits<size_t>::max()) {
			if ((pairsNew.rootPtr[0].key[returnValues.uniqueIndex] & 1) == 0 && (pairsNew.rootPtr[1].key[returnValues.uniqueIndex] & 1) == 1) {
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

	template<typename value_type> JSONIFIER_ALWAYS_INLINE constexpr auto collectMapConstructionDataImpl() noexcept {
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

	template<typename value_type> JSONIFIER_ALWAYS_INLINE constexpr auto collectMapConstructionData() noexcept {
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

	template<size_t keyMaxLength> constexpr auto generateMappingsForLengths(const tuple_references& keys, const std::array<uint8_t, 256>& uniqueIndices) {
		std::array<size_t, (keyMaxLength + 1) * 256> mappings{};

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
	constexpr auto generateMappingsForFirstBytes(const std::array<first_bytes, firstCharCount>& keys, const std::array<uint8_t, 256>& uniqueIndices) {
		constexpr size_t flattenedSize = 256 * 256;
		std::array<size_t, flattenedSize> flattenedMappings{};
		flattenedMappings.fill(flattenedMappings.size() - 1);

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
				}
			}
		}

		return flattenedMappings;
	}

#if !defined(NDEBUG)
	inline std::unordered_map<std::string, uint32_t> types{};
#endif

	template<typename value_type, typename iterator_newer> struct hash_map {
		static constexpr auto hashData = collectMapConstructionData<std::remove_cvref_t<value_type>>();
		static constexpr auto subAmount01{ []() constexpr {
			return ((keyStatsVal<value_type>.maxLength - keyStatsVal<value_type>.minLength) >= bytesPerStep) ? keyStatsVal<value_type>.minLength : 0;
		}() };

		static constexpr auto subAmount02{ []() constexpr {
			return ((keyStatsVal<value_type>.maxLength - keyStatsVal<value_type>.minLength) >= bytesPerStep)
				? (keyStatsVal<value_type>.maxLength - keyStatsVal<value_type>.minLength + 2)
				: (keyStatsVal<value_type>.maxLength + 2);
		}() };

		JSONIFIER_ALWAYS_INLINE static size_t findIndex(iterator_newer& iter, iterator_newer& end) noexcept {
			static constexpr auto checkForEnd = [](const auto* iter, const auto* end, const auto distance) {
				return (iter + distance) < end;
			};
#if !defined(NDEBUG)
			if (!types.contains(typeid(value_type).name())) {
				types[typeid(value_type).name()] = static_cast<uint32_t>(hashData.type);
			}
#endif
			if constexpr (hashData.type == hash_map_type::single_element) {
				return 0;
			} else if constexpr (hashData.type == hash_map_type::double_element) {
				if JSONIFIER_LIKELY ((checkForEnd(iter, end, hashData.uniqueIndex))) {
					return iter[hashData.uniqueIndex] & 1;
				}
				return hashData.storageSize;
			} else if constexpr (hashData.type == hash_map_type::triple_element) {
				if JSONIFIER_LIKELY ((checkForEnd(iter, end, hashData.uniqueIndex))) {
					return (static_cast<uint8_t>(iter[hashData.uniqueIndex] ^ hashData.firstChar) * hashData.seed) & 3;
				}
				return hashData.storageSize;
			} else if constexpr (hashData.type == hash_map_type::single_byte) {
				if JSONIFIER_LIKELY ((checkForEnd(iter, end, hashData.uniqueIndex))) {
					return hashData.uniqueIndices[static_cast<uint8_t>(iter[hashData.uniqueIndex])];
				}
				return hashData.storageSize;
			} else if constexpr (hashData.type == hash_map_type::first_byte_and_unique_index) {
				static constexpr auto uniqueFirstByteCount{ countFirstBytes(tupleReferencesByFirstByte<value_type>) };
				static constexpr auto mappings{ generateMappingsForFirstBytes(collectFirstBytes<uniqueFirstByteCount>(tupleReferencesByFirstByte<value_type>),
					hashData.uniqueIndices) };
				const uint8_t firstByte = static_cast<uint8_t>(iter[0]);
				const uint8_t uniqueIdx = hashData.uniqueIndices[firstByte];
				if JSONIFIER_LIKELY ((checkForEnd(iter, end, uniqueIdx))) {
					const uint8_t keyChar	  = static_cast<uint8_t>(iter[uniqueIdx]);
					const size_t flattenedIdx = (static_cast<size_t>(firstByte) << 8) | static_cast<size_t>(keyChar);
					return mappings[flattenedIdx];
				}
				return hashData.storageSize;
			} else if constexpr (hashData.type == hash_map_type::unique_byte_and_length) {
				static constexpr size_t storageMask = hashData.storageSize - 1;
				const auto newPtr					= char_comparison<'"'>::memchar(iter + subAmount01, subAmount02);
				if JSONIFIER_LIKELY ((newPtr)) {
					const size_t length = static_cast<size_t>(newPtr - iter);
					if JSONIFIER_LIKELY ((checkForEnd(iter, end, hashData.uniqueIndex))) {
						const size_t combinedKey = iter[hashData.uniqueIndex] ^ length;
						return hashData.indices[combinedKey & storageMask];
					}
				}
				return hashData.storageSize;
			} else if constexpr (hashData.type == hash_map_type::unique_per_length) {
				static constexpr auto mappings = generateMappingsForLengths<keyStatsVal<value_type>.maxLength>(tupleReferencesByLength<value_type>, hashData.uniqueIndices);
				const auto newPtr			   = char_comparison<'"'>::memchar(iter + subAmount01, subAmount02);
				if JSONIFIER_LIKELY ((newPtr)) {
					const size_t length			= static_cast<size_t>(newPtr - iter);
					const size_t localUniqueIdx = hashData.uniqueIndices[length];
					if JSONIFIER_LIKELY ((localUniqueIdx != 255 && checkForEnd(iter, end, localUniqueIdx))) {
						return mappings[(length << 8) | iter[localUniqueIdx]];
					}
				}
				return hashData.storageSize;
			} else if constexpr (hashData.type == hash_map_type::simd_full_length) {
				using simd_type = map_simd_t<hashData.storageSize>;
				static constexpr rt_key_hasher<hashData.seed> hasher{};
				static constexpr auto sizeMask{ hashData.numGroups - 1 };
				const auto newPtr = char_comparison<'"'>::memchar(iter + subAmount01, subAmount02);
				if JSONIFIER_LIKELY ((newPtr)) {
					size_t length = static_cast<size_t>(newPtr - iter);
					length		  = (hashData.uniqueIndex > length) ? length : hashData.uniqueIndex;
					if JSONIFIER_LIKELY ((checkForEnd(iter, end, length))) {
						const auto hash			 = hasher.hashKeyRt(iter, length);
						const size_t group		 = (hash >> 8) & (sizeMask);
						const size_t resultIndex = group * hashData.bucketSize;
						const auto simdValues	 = simd_internal::gatherValues<simd_type>(hashData.controlBytes.data() + resultIndex);
						const auto matches		 = simd_internal::opCmpEq(simd_internal::gatherValue<simd_type>(static_cast<uint8_t>(hash)), simdValues);
						const size_t tz			 = simd_internal::tzcnt(matches);
						return hashData.indices[resultIndex + tz];
					}
				}
				return hashData.storageSize;
			} else if constexpr (hashData.type == hash_map_type::empty) {
				return 0;
			}
		}
	};
};
