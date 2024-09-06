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

	constexpr std::array<size_t, 7> hashMapMaxSizes{ { 16, 32, 64, 128, 256, 512, 1024 } };

	template<size_t length> struct map_simd {
		using type = std::conditional_t<length >= 64 && bytesPerStep >= 64, jsonifier_simd_int_512,
			std::conditional_t<length >= 32 && bytesPerStep >= 32, jsonifier_simd_int_256, jsonifier_simd_int_128>>;
	};

	template<size_t length> using map_simd_t = map_simd<length>::type;

	template<size_t length> struct map_integer {
		using type = std::conditional_t<length >= 256, uint16_t, uint8_t>;
	};

	template<size_t length> using map_integer_t = map_integer<length>::type;

	enum class hash_map_type {
		unset				   = 0,
		empty				   = 1,
		single_element		   = 2,
		double_element		   = 3,
		triple_element		   = 4,
		single_byte			   = 5,
		unique_byte_and_length = 6,
		unique_per_length	   = 7,
		simd_full_length	   = 8,
	};

	JSONIFIER_ALWAYS_INLINE static constexpr size_t setSimdWidth(size_t length) noexcept {
		return length >= 64ull && bytesPerStep >= 64ull ? 64ull : length >= 32ull && bytesPerStep >= 32ull ? 32ull : 16ull;
	}

	static constexpr size_t getMaxSizeIndex(size_t currentSize) {
		for (size_t x = 0; x < hashMapMaxSizes.size(); ++x) {
			if (currentSize <= hashMapMaxSizes[x]) {
				return x;
			}
		}
		return std::numeric_limits<size_t>::max();
	}

	struct key_stats_t {
		size_t minLength{ (std::numeric_limits<size_t>::max)() };
		size_t lengthRange{};
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

	template<typename value_type> struct single_byte_data {
		JSONIFIER_ALIGN std::array<uint8_t, 256> indices{};
		size_t uniqueIndex{};
	};

	template<typename value_type, size_t maxSizeIndexNew> struct unique_byte_and_length_data {
		using integer_type = map_integer_t<hashMapMaxSizes[maxSizeIndexNew]>;
		JSONIFIER_ALIGN std::array<integer_type, hashMapMaxSizes[maxSizeIndexNew] + 1> indices{};
		key_hasher<integer_type> hasher{};
		size_t uniqueIndex{};
	};

	template<typename value_type, size_t maxSizeIndexNew> struct unique_per_length_data {
		std::array<uint8_t, 256> uniqueIndices{};
	};

	template<typename value_type, size_t maxSizeIndexNew> struct simd_full_length_data {
		using simd_type	   = map_simd_t<hashMapMaxSizes[maxSizeIndexNew]>;
		using integer_type = map_integer_t<hashMapMaxSizes[maxSizeIndexNew]>;
		JSONIFIER_ALIGN std::array<uint8_t, hashMapMaxSizes[maxSizeIndexNew] + 1> controlBytes{};
		JSONIFIER_ALIGN std::array<integer_type, hashMapMaxSizes[maxSizeIndexNew] + 1> indices{};
		size_t bucketSize{ setSimdWidth(hashMapMaxSizes[maxSizeIndexNew]) };
		size_t numGroups{ hashMapMaxSizes[maxSizeIndexNew] / bucketSize };
		key_hasher<integer_type> hasher{};
		size_t uniqueIndex{};
	};

	template<typename value_type, size_t maxSizeIndexNew> struct hash_map_construction_data {
		using simd_type	   = map_simd_t<hashMapMaxSizes[maxSizeIndexNew]>;
		using integer_type = map_integer_t<hashMapMaxSizes[maxSizeIndexNew]>;
		std::array<size_t, hashMapMaxSizes[maxSizeIndexNew] / setSimdWidth(hashMapMaxSizes[maxSizeIndexNew])> bucketSizes{};
		JSONIFIER_ALIGN std::array<uint8_t, hashMapMaxSizes[maxSizeIndexNew] + 1> controlBytes{};
		JSONIFIER_ALIGN std::array<integer_type, hashMapMaxSizes[maxSizeIndexNew] + 1> indices{};
		integer_type bucketSize{ setSimdWidth(hashMapMaxSizes[maxSizeIndexNew]) };
		integer_type numGroups{ static_cast<integer_type>(hashMapMaxSizes[maxSizeIndexNew] / bucketSize) };
		integer_type storageSize{ static_cast<integer_type>(hashMapMaxSizes[maxSizeIndexNew]) };
		std::array<uint8_t, 256> uniqueIndices{};
		key_hasher<integer_type> hasher{};
		key_stats_t keyStatsVal{};
		hash_map_type type{};
		size_t uniqueIndex{};

		JSONIFIER_ALWAYS_INLINE constexpr hash_map_construction_data() noexcept = default;
	};

	struct string_lengths {
		tuple_references tupleRefs{};
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
				tuple_references tupleRefs{};
				tupleRefs.rootPtr		= &values.rootPtr[x];
				valuesNew[currentIndex] = string_lengths{ tupleRefs, values.rootPtr[x].key.size() };
				++valuesNew[currentIndex].tupleRefs.count;
				++currentIndex;
			} else {
				for (auto& value: valuesNew) {
					if (value.length == values.rootPtr[x].key.size()) {
						++value.tupleRefs.count;
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
		stats.lengthRange = stats.maxLength - stats.minLength;
		return stats;
	}

	template<size_t size> JSONIFIER_ALWAYS_INLINE constexpr auto keyStats(const std::array<string_lengths, size>& tupleRefs) noexcept {
		std::array<key_stats_t, size> returnValues{};
		for (uint64_t x = 0; x < size; ++x) {
			returnValues[x] = keyStatsImpl(tupleRefs[x].tupleRefs);
		}
		return returnValues;
	}

	template<typename value_type, size_t maxSizeIndexNew, template<typename, size_t> typename hash_map_construction_data_type> using hash_map_construction_data_variant =
		std::variant<hash_map_construction_data_type<value_type, 0>, hash_map_construction_data_type<value_type, 1>, hash_map_construction_data_type<value_type, 2>,
			hash_map_construction_data_type<value_type, 3>, hash_map_construction_data_type<value_type, 4>, hash_map_construction_data_type<value_type, 5>,
			hash_map_construction_data_type<value_type, 6>>;

	template<typename value_type, size_t maxSizeIndexNew>
	JSONIFIER_ALWAYS_INLINE constexpr auto collectSimdFullLengthHashMapData(const jsonifier_internal::tuple_references& pairsNew) noexcept {
		auto keyStatsVal			 = jsonifier_internal::keyStatsImpl(pairsNew);
		auto constructForGivenLength = [&](const auto maxSizeIndex, auto&& constructForGivenLength) mutable {
			jsonifier_internal::hash_map_construction_data<value_type, maxSizeIndex> returnValues{};
			using integer_type		 = jsonifier_internal::map_integer_t<jsonifier_internal::hashMapMaxSizes[maxSizeIndex]>;
			returnValues.keyStatsVal = keyStatsVal;
			bool collided{};
			for (size_t w = 0; w < returnValues.keyStatsVal.maxLength; ++w) {
				returnValues.uniqueIndex = w;
				for (size_t x = 0; x < 2; ++x) {
					std::fill(returnValues.controlBytes.begin(), returnValues.controlBytes.end(), std::numeric_limits<uint8_t>::max());
					std::fill(returnValues.indices.begin(), returnValues.indices.end(), static_cast<integer_type>(returnValues.indices.size() - 1));
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
							jsonifier_internal::contains(returnValues.controlBytes.data() + groupPos * returnValues.bucketSize, ctrlByte, returnValues.bucketSize)) {
							std::fill(returnValues.bucketSizes.data(), returnValues.bucketSizes.data() + returnValues.numGroups, 0);
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
				if constexpr (maxSizeIndex < std::size(jsonifier_internal::hashMapMaxSizes) - 1) {
					return jsonifier_internal::hash_map_construction_data_variant<value_type, maxSizeIndexNew, jsonifier_internal::hash_map_construction_data>{
						constructForGivenLength(std::integral_constant<size_t, maxSizeIndex + 1>{}, constructForGivenLength)
					};
				} else {
					returnValues.type		 = jsonifier_internal::hash_map_type::unset;
					returnValues.uniqueIndex = std::numeric_limits<size_t>::max();
					return jsonifier_internal::hash_map_construction_data_variant<value_type, maxSizeIndexNew, jsonifier_internal::hash_map_construction_data>{ returnValues };
				}
			} else {
				returnValues.type = jsonifier_internal::hash_map_type::simd_full_length;
				return jsonifier_internal::hash_map_construction_data_variant<value_type, maxSizeIndexNew, jsonifier_internal::hash_map_construction_data>{ returnValues };
			}
		};
		return constructForGivenLength(std::integral_constant<size_t, maxSizeIndexNew>{}, constructForGivenLength);
	}

	template<typename value_type, size_t maxSizeIndexNew> constexpr auto collectUniquePerLengthHashMapData(const tuple_references& pairsNew) {
		constexpr auto uniqueLengthCount = countUniqueLengths(tupleReferencesByLength<value_type>);
		constexpr auto results			 = collectLengths<uniqueLengthCount>(tupleReferencesByLength<value_type>);
		constexpr auto keyStatsVal		 = keyStatsImpl(tupleReferencesByLength<value_type>);
		constexpr auto keyStatsValNew	 = keyStats(results);
		hash_map_construction_data<value_type, maxSizeIndexNew> returnValues{};
		returnValues.keyStatsVal = keyStatsVal;
		returnValues.uniqueIndices.fill(255);
		if (keyStatsVal.maxLength < 256) {
			size_t uniqueIndex = findUniqueColumnIndex(results[0].tupleRefs, keyStatsValNew[0].minLength);
			if (uniqueIndex == std::numeric_limits<size_t>::max()) {
				return collectSimdFullLengthHashMapData<value_type, getMaxSizeIndex(1024)>(pairsNew);
			} else {
				returnValues.uniqueIndices[results[0].length] = static_cast<uint8_t>(uniqueIndex);
			}
			for (size_t x = 1; x < uniqueLengthCount; ++x) {
				uniqueIndex = findUniqueColumnIndex(results[x].tupleRefs, keyStatsValNew[x].minLength, uniqueIndex + 1);
				if (uniqueIndex == std::numeric_limits<size_t>::max()) {
					return collectSimdFullLengthHashMapData<value_type, getMaxSizeIndex(1024)>(pairsNew);
				} else {
					returnValues.uniqueIndices[results[x].length] = static_cast<uint8_t>(uniqueIndex);
				}
			}
		} else {
			return collectSimdFullLengthHashMapData<value_type, getMaxSizeIndex(1024)>(pairsNew);
		}
		returnValues.type = hash_map_type::unique_per_length;
		return hash_map_construction_data_variant<value_type, maxSizeIndexNew, hash_map_construction_data>{ returnValues };
	}

	template<typename value_type, size_t maxSizeIndexNew> JSONIFIER_ALWAYS_INLINE constexpr auto collectUniqueByteAndLengthHashMapData(const tuple_references& pairsNew) noexcept {
		constexpr auto keyStatsVal	 = keyStatsImpl(tupleReferencesByLength<value_type>);
		auto constructForGivenLength = [&](const auto maxSizeIndex, auto&& constructForGivenLength) mutable {
			hash_map_construction_data<value_type, maxSizeIndex> returnValues{};
			using integer_type		 = map_integer_t<hashMapMaxSizes[maxSizeIndex]>;
			returnValues.keyStatsVal = keyStatsVal;
			bool collided{ true };
			while (returnValues.uniqueIndex < keyStatsVal.minLength) {
				std::fill(returnValues.indices.begin(), returnValues.indices.end(), static_cast<integer_type>(returnValues.indices.size() - 1));
				returnValues.hasher.updateSeed();
				collided = false;

				for (size_t y = 0; y < pairsNew.count; ++y) {
					const auto hash = pairsNew.rootPtr[y].key[returnValues.uniqueIndex] ^ pairsNew.rootPtr[y].key.size();
					const auto slot = hash % returnValues.storageSize;
					if (returnValues.indices[slot] != returnValues.indices.size() - 1) {
						collided = true;
						break;
					}
					returnValues.indices[slot] = y;
				}
				if (!collided) {
					break;
				}
				++returnValues.uniqueIndex;
			}
			if (collided) {
				if constexpr (maxSizeIndex < std::size(hashMapMaxSizes) - 1) {
					return hash_map_construction_data_variant<value_type, maxSizeIndex, hash_map_construction_data>{ constructForGivenLength(
						std::integral_constant<size_t, maxSizeIndex + 1>{}, constructForGivenLength) };
				} else {
					return collectUniquePerLengthHashMapData<value_type, maxSizeIndexNew>(pairsNew);
				}
			} else {
				returnValues.type = hash_map_type::unique_byte_and_length;
				return hash_map_construction_data_variant<value_type, maxSizeIndexNew, hash_map_construction_data>{ returnValues };
			}
		};
		return constructForGivenLength(std::integral_constant<size_t, maxSizeIndexNew>{}, constructForGivenLength);
	}

	template<typename value_type, size_t maxSizeIndexNew> JSONIFIER_ALWAYS_INLINE constexpr auto collectSingleByteHashMapData(const tuple_references& pairsNew) noexcept {
		hash_map_construction_data<value_type, getMaxSizeIndex(256)> returnValues{};
		returnValues.keyStatsVal = keyStatsImpl(tupleReferencesByLength<value_type>);
		using integer_type		 = uint8_t;
		returnValues.uniqueIndex = findUniqueColumnIndex(pairsNew, returnValues.keyStatsVal.minLength);
		if (returnValues.uniqueIndex != std::numeric_limits<size_t>::max()) {
			std::fill(returnValues.indices.begin(), returnValues.indices.end(), static_cast<integer_type>(returnValues.indices.size() - 1));
			for (size_t y = 0; y < pairsNew.count; ++y) {
				const auto slot			   = pairsNew.rootPtr[y].key.data()[returnValues.uniqueIndex];
				returnValues.indices[slot] = y;
			}
			returnValues.type = hash_map_type::single_byte;
			return hash_map_construction_data_variant<value_type, getMaxSizeIndex(256), hash_map_construction_data>{ returnValues };
		} else {
			return collectUniqueByteAndLengthHashMapData<value_type, maxSizeIndexNew>(pairsNew);
		}
	}

	template<typename value_type, size_t maxSizeIndexNew> JSONIFIER_ALWAYS_INLINE constexpr auto collectTripleElementHashMapData(const tuple_references& pairsNew) noexcept {
		hash_map_construction_data<value_type, maxSizeIndexNew> returnValues{};
		returnValues.keyStatsVal = keyStatsImpl(tupleReferencesByLength<value_type>);
		returnValues.uniqueIndex = findUniqueColumnIndex(pairsNew, returnValues.keyStatsVal.minLength);
		bool collided{ true };
		while (returnValues.uniqueIndex != std::numeric_limits<size_t>::max()) {
			const auto first = static_cast<uint8_t>(pairsNew.rootPtr[0].key[returnValues.uniqueIndex]);
			const auto mix1	 = static_cast<uint8_t>(pairsNew.rootPtr[1].key[returnValues.uniqueIndex]) ^ first;
			const auto mix2	 = static_cast<uint8_t>(pairsNew.rootPtr[2].key[returnValues.uniqueIndex]) ^ first;
			for (size_t x = 0; x < 4; ++x) {
				uint8_t hash1 = (mix1 ^ returnValues.hasher.seed) & 3;
				uint8_t hash2 = (mix2 ^ returnValues.hasher.seed) & 3;

				if (hash1 == 1 && hash2 == 2) {
					collided = false;
					break;
				} else {
					returnValues.hasher.updateSeed();
				}
			}
			if (!collided) {
				break;
			}
			returnValues.uniqueIndex = findUniqueColumnIndex(pairsNew, returnValues.keyStatsVal.minLength, returnValues.uniqueIndex + 1);
		}
		if (collided) {
			return collectSingleByteHashMapData<value_type, maxSizeIndexNew>(pairsNew);
		}
		returnValues.type = hash_map_type::triple_element;
		return hash_map_construction_data_variant<value_type, 0, hash_map_construction_data>{ returnValues };
	}

	template<typename value_type, size_t maxSizeIndexNew> JSONIFIER_ALWAYS_INLINE constexpr auto collectDoubleElementHashMapData(const tuple_references& pairsNew) noexcept {
		hash_map_construction_data<value_type, maxSizeIndexNew> returnValues{};
		returnValues.keyStatsVal = keyStatsImpl(tupleReferencesByLength<value_type>);
		returnValues.uniqueIndex = findUniqueColumnIndex(pairsNew, returnValues.keyStatsVal.minLength);
		bool collided{ true };
		while (returnValues.uniqueIndex != std::numeric_limits<size_t>::max()) {
			if ((pairsNew.rootPtr[0].key[returnValues.uniqueIndex] & 1) == 0 && (pairsNew.rootPtr[1].key[returnValues.uniqueIndex] & 1) == 1) {
				collided = false;
				break;
			}
			returnValues.uniqueIndex = findUniqueColumnIndex(pairsNew, returnValues.keyStatsVal.minLength, returnValues.uniqueIndex + 1);
		}
		if (collided) {
			return collectSingleByteHashMapData<value_type, maxSizeIndexNew>(pairsNew);
		}
		returnValues.type = hash_map_type::double_element;
		return hash_map_construction_data_variant<value_type, 0, hash_map_construction_data>{ returnValues };
	}

	template<typename value_type> JSONIFIER_ALWAYS_INLINE constexpr auto collectMapConstructionDataImpl() noexcept {
		if constexpr (tupleReferencesByLength<value_type>.count == 0) {
			hash_map_construction_data<value_type, 0> returnValues{};
			returnValues.type = hash_map_type::empty;
			return hash_map_construction_data_variant<value_type, 0, hash_map_construction_data>{ returnValues };
		} else if constexpr (tupleReferencesByLength<value_type>.count == 1) {
			hash_map_construction_data<value_type, 0> returnValues{};
			returnValues.type = hash_map_type::single_element;
			return hash_map_construction_data_variant<value_type, 0, hash_map_construction_data>{ returnValues };
		} else if constexpr (tupleReferencesByLength<value_type>.count == 2) {
			return collectDoubleElementHashMapData<value_type, getMaxSizeIndex(tupleReferencesByLength<value_type>.count)>(tupleReferencesByLength<value_type>);
		} else if constexpr (tupleReferencesByLength<value_type>.count == 3) {
			return collectTripleElementHashMapData<value_type, getMaxSizeIndex(tupleReferencesByLength<value_type>.count)>(tupleReferencesByLength<value_type>);
		} else {
			return collectSingleByteHashMapData<value_type, getMaxSizeIndex(tupleReferencesByLength<value_type>.count)>(tupleReferencesByLength<value_type>);
		}
	}

	template<typename value_type> JSONIFIER_ALWAYS_INLINE constexpr auto collectMapConstructionData() noexcept {
		constexpr auto newTuple = collectMapConstructionDataImpl<value_type>();
		constexpr auto newIndex = newTuple.index();
		return std::get<newIndex>(newTuple);
	}

	template<size_t keyMaxLength> constexpr auto generateMappings(const tuple_references& keys, const std::array<uint8_t, 256>& uniqueIndices) {
		std::array<size_t, (keyMaxLength + 1) * 256> mappings{};

		mappings.fill(static_cast<size_t>(-1));

		for (size_t i = 0; i < keys.count; ++i) {
			const auto& key = keys.rootPtr[i].key;

			uint8_t uniqueIndex = uniqueIndices[key.size()];

			if (uniqueIndex != 255 && uniqueIndex < key.size()) {
				uint8_t keyChar		= static_cast<uint8_t>(key[uniqueIndex]);
				size_t flatIndex	= key.size() * 256 + keyChar;
				mappings[flatIndex] = static_cast<size_t>(i);
			}
		}

		return mappings;
	}

	template<typename value_type, typename iterator_newer> struct hash_map {
		static constexpr auto hashData					   = collectMapConstructionData<unwrap_t<value_type>>();
		JSONIFIER_ALIGN static constexpr auto controlBytes = hashData.controlBytes;
		JSONIFIER_ALIGN static constexpr auto indices	   = hashData.indices;
		static constexpr auto uniqueIndex				   = hashData.uniqueIndex;
		static constexpr auto subAmount01{ []() constexpr {
			return ((hashData.keyStatsVal.maxLength - hashData.keyStatsVal.minLength) >= bytesPerStep) ? hashData.keyStatsVal.minLength : 0;
		}() };

		static constexpr auto subAmount02{ []() constexpr {
			return ((hashData.keyStatsVal.maxLength - hashData.keyStatsVal.minLength) >= bytesPerStep) ? (hashData.keyStatsVal.maxLength - hashData.keyStatsVal.minLength + 2)
																									   : (hashData.keyStatsVal.maxLength + 2);
		}() };

		JSONIFIER_ALWAYS_INLINE static size_t findIndex(iterator_newer& iter, iterator_newer& end) noexcept {
			if constexpr (hashData.type == hash_map_type::single_element) {
				return 0;
			} else if constexpr (hashData.type == hash_map_type::double_element) {
				return iter[uniqueIndex] & 1;
			} else if constexpr (hashData.type == hash_map_type::triple_element) {
				static constexpr auto firstChar = std::get<0>(coreTupleV<value_type>).view()[uniqueIndex];
				return (uint8_t(iter[uniqueIndex] ^ firstChar) ^ hashData.hasher.seed) & 3;
			} else if constexpr (hashData.type == hash_map_type::single_byte) {
				return indices[iter[uniqueIndex]];
			} else if constexpr (hashData.type == hash_map_type::unique_byte_and_length) {
				auto newPtr = memchar<'"'>(iter + subAmount01, subAmount02);
				if (newPtr) [[likely]] {
					return indices[(iter[uniqueIndex] ^ static_cast<size_t>(newPtr - (iter))) & (hashData.storageSize - 1)];
				}
				return hashData.storageSize;
			} else if constexpr (hashData.type == hash_map_type::unique_per_length) {
				static constexpr auto mappings = generateMappings<hashData.keyStatsVal.maxLength>(tupleReferencesByLength<value_type>, hashData.uniqueIndices);
				auto newPtr					   = memchar<'"'>(iter + subAmount01, subAmount02);
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
				auto newPtr		= memchar<'"'>(iter + subAmount01, subAmount02);
				if (newPtr) [[likely]] {
					size_t length		   = static_cast<size_t>(newPtr - (iter));
					length				   = uniqueIndex > length ? length : uniqueIndex;
					const auto hash		   = hashData.hasher.hashKeyRt(iter, length);
					const auto resultIndex = ((hash >> 8) & (hashData.numGroups - 1)) * hashData.bucketSize;
					return indices[(simd_internal::tzcnt(simd_internal::opCmpEq(simd_internal::gatherValue<simd_type>(static_cast<uint8_t>(hash)),
										simd_internal::gatherValues<simd_type>(controlBytes.data() + resultIndex))) +
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