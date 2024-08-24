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

namespace jsonifier_internal {

	template<typename value_type, size_t size> std::ostream& operator<<(std::ostream& os, const std::array<value_type, size>& values) {
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

	struct tuple_reference {
		jsonifier::string_view key{};
		size_t oldIndex{};
	};

	struct tuple_references {
		const tuple_reference* rootPtr{};
		size_t count{};
	};

	template<size_t maxIndex, size_t currentIndex = 0, typename tuple_type>
	constexpr auto collectTupleRefsImpl(const tuple_type& tuple, std::array<tuple_reference, maxIndex>& tupleRefs) {
		if constexpr (currentIndex < maxIndex) {
			tupleRefs[currentIndex].key		 = std::get<currentIndex>(tuple).view();
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

	template<size_t size> constexpr auto consolidateTupleRefs(const std::array<tuple_reference, size>& tupleRefs) {
		tuple_references returnValues{};
		returnValues.rootPtr = &tupleRefs[0];
		returnValues.count	 = size;
		return returnValues;
	}

	template<typename value_type> inline static constexpr auto tupleRefs{ collectTupleRefs(jsonifier::concepts::coreV<value_type>) };
	template<typename value_type> inline static constexpr auto sortedTupleReferences{ sortTupleRefsByFirstByte(tupleRefs<value_type>) };
	template<typename value_type> inline static constexpr auto tupleReferences{ consolidateTupleRefs(sortedTupleReferences<value_type>) };

	template<typename value_type, size_t... indices> JSONIFIER_ALWAYS_INLINE constexpr auto createNewTupleImpl(std::index_sequence<indices...>) noexcept {
		return std::make_tuple(std::get<sortedTupleReferences<value_type>[indices].oldIndex>(jsonifier::concepts::coreV<value_type>)...);
	}

	template<typename value_type> JSONIFIER_ALWAYS_INLINE constexpr auto createNewTuple() noexcept {
		constexpr auto& tupleRefs = sortedTupleReferences<value_type>;
		return createNewTupleImpl<value_type>(std::make_index_sequence<tupleRefs.size()>{});
	}

	template<typename value_type> inline static constexpr auto coreTupleV{ createNewTuple<unwrap_t<value_type>>() };

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

	constexpr std::array<size_t, 7> hashMapMaxSizes{ 16, 32, 64, 128, 256, 512, 1024 };

	template<size_t length> struct map_simd {
		using type = std::conditional_t<length >= 64 && bytesPerStep >= 64, jsonifier_simd_int_512,
			std::conditional_t<length >= 32 && bytesPerStep >= 32, jsonifier_simd_int_256, jsonifier_simd_int_128>>;
	};

	template<size_t length> using map_simd_t = map_simd<length>::type;

	enum class hash_map_type {
		unset				   = 0,
		empty				   = 1,
		single_element		   = 2,
		double_element		   = 3,
		triple_element		   = 4,
		single_byte			   = 5,
		unique_byte_and_length = 6,
		simd_full_length	   = 7,
	};

	JSONIFIER_ALWAYS_INLINE constexpr size_t setSimdWidth(size_t length) noexcept {
		return length >= 64 && bytesPerStep >= 64 ? 64 : length >= 32 && bytesPerStep >= 32 ? 32 : 16;
	}

	constexpr size_t getMaxSizeIndex(size_t currentSize) {
		for (uint64_t x = 0; x < hashMapMaxSizes.size(); ++x) {
			if (currentSize <= hashMapMaxSizes[x]) {
				return x;
			}
		}
		return std::numeric_limits<size_t>::max();
	}

	struct key_stats_t {
		size_t minLength{ (std::numeric_limits<size_t>::max)() };
		size_t maxSizeIndex{};
		hash_map_type type{};
		size_t uniqueIndex{};
		size_t lengthRange{};
		key_hasher hasher{};
		size_t maxLength{};
		size_t keyCount{};
	};

	JSONIFIER_ALWAYS_INLINE constexpr size_t findUniqueColumnIndex(const tuple_references& tupleRefs, const key_stats_t& keyStats, size_t startingIndex = 0) noexcept {
		constexpr size_t alphabetSize = 256;

		for (size_t index = startingIndex; index < keyStats.minLength; ++index) {
			bool allDifferent = true;
			std::array<bool, alphabetSize> seen{};

			for (uint64_t x = 0; x < tupleRefs.count; ++x) {
				char c = tupleRefs.rootPtr[x].key[index % tupleRefs.rootPtr[x].key.size()];
				if (seen[static_cast<unsigned char>(c)]) {
					allDifferent = false;
					break;
				}
				seen[static_cast<unsigned char>(c)] = true;
			}

			if (allDifferent) {
				return index;
			}
		}

		return std::numeric_limits<size_t>::max();
	}

	template<typename value_type, size_t maxSizeIndexNew> struct hash_map_construction_data {
		using simd_type = map_simd_t<hashMapMaxSizes[maxSizeIndexNew]>;
		std::array<size_t, hashMapMaxSizes[maxSizeIndexNew] / setSimdWidth(hashMapMaxSizes[maxSizeIndexNew])> bucketSizes{};
		JSONIFIER_ALIGN std::array<uint8_t, hashMapMaxSizes[maxSizeIndexNew] + 1> controlBytes{};
		JSONIFIER_ALIGN std::array<size_t, hashMapMaxSizes[maxSizeIndexNew] + 1> indices{};
		size_t bucketSize{ setSimdWidth(hashMapMaxSizes[maxSizeIndexNew]) };
		size_t numGroups{ hashMapMaxSizes[maxSizeIndexNew] / bucketSize };
		size_t storageSize{ hashMapMaxSizes[maxSizeIndexNew] };
		size_t maxSizeIndex{ maxSizeIndexNew };
		key_stats_t keyStatsVal{};
		size_t uniqueIndex02{};
		hash_map_type type{};
		size_t uniqueIndex{};
		key_hasher hasher{};
		size_t xorValue{};

		JSONIFIER_ALWAYS_INLINE constexpr hash_map_construction_data() noexcept = default;
	};

	template<typename value_type, size_t I> JSONIFIER_ALWAYS_INLINE constexpr const jsonifier::string_view& getKey() noexcept {
		return std::get<I>(coreTupleV<value_type>).view();
	}

	template<typename value_type, size_t maxIndex, size_t index> JSONIFIER_ALWAYS_INLINE constexpr auto keyStatsImpl(key_stats_t stats) noexcept {
		if constexpr (index < maxIndex) {
			constexpr const jsonifier::string_view& key{ getKey<value_type, index>() };
			constexpr auto n{ key.size() };
			if (n < stats.minLength) {
				stats.minLength = n;
			}
			if (n > stats.maxLength) {
				stats.maxLength = n;
			}
			return keyStatsImpl<value_type, maxIndex, index + 1>(stats);
		} else {
			if constexpr (maxIndex > 0) {
				stats.lengthRange = stats.maxLength - stats.minLength;
			}
			return stats;
		}
	}

	template<typename value_type> JSONIFIER_ALWAYS_INLINE constexpr auto keyStats() noexcept {
		constexpr auto N{ std::tuple_size_v<core_tuple_t<value_type>> };

		return keyStatsImpl<value_type, N, 0>(key_stats_t{});
	}

	template<typename value_type, size_t maxSizeIndexNew, template<typename, size_t> typename hash_map_construction_data_type> using hash_map_construction_data_variant =
		std::variant<hash_map_construction_data_type<value_type, 0>, hash_map_construction_data_type<value_type, 1>, hash_map_construction_data_type<value_type, 2>,
			hash_map_construction_data_type<value_type, 3>, hash_map_construction_data_type<value_type, 4>, hash_map_construction_data_type<value_type, 5>,
			hash_map_construction_data_type<value_type, 6>>;

	template<typename value_type, size_t maxSizeIndexNew> JSONIFIER_ALWAYS_INLINE constexpr auto collectSimdFullLengthHashMapData(const tuple_references& pairsNew) noexcept {
		constexpr auto keyStatsVal		   = keyStats<value_type>();
		auto constructForGivenStringLength = [&](const auto maxSizeIndex, auto&& constructForGivenStringLength) mutable {
			hash_map_construction_data<value_type, maxSizeIndex> returnValues{};
			returnValues.keyStatsVal = keyStatsVal;
			bool collided{};
			for (size_t w = 0; w < returnValues.keyStatsVal.maxLength; ++w) {
				returnValues.uniqueIndex = w;
				for (size_t x = 0; x < 2; ++x) {
					std::fill(returnValues.controlBytes.begin(), returnValues.controlBytes.end(), std::numeric_limits<uint8_t>::max());
					std::fill(returnValues.indices.begin(), returnValues.indices.end(), returnValues.indices.size() - 1);
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
				if constexpr (maxSizeIndex < std::size(hashMapMaxSizes) - 1) {
					return hash_map_construction_data_variant<value_type, maxSizeIndexNew, jsonifier_internal::hash_map_construction_data>{ constructForGivenStringLength(
						std::integral_constant<size_t, maxSizeIndex + 1>{}, constructForGivenStringLength) };
				} else {
					returnValues.type		 = hash_map_type::unset;
					returnValues.uniqueIndex = std::numeric_limits<size_t>::max();
					return hash_map_construction_data_variant<value_type, maxSizeIndexNew, jsonifier_internal::hash_map_construction_data>{ returnValues };
				}
			} else {
				returnValues.type = hash_map_type::simd_full_length;
				return hash_map_construction_data_variant<value_type, maxSizeIndexNew, jsonifier_internal::hash_map_construction_data>{ returnValues };
			}
		};
		return constructForGivenStringLength(std::integral_constant<size_t, maxSizeIndexNew>{}, constructForGivenStringLength);
	}

	template<typename value_type, size_t maxSizeIndexNew> JSONIFIER_ALWAYS_INLINE constexpr auto collectUniqueSingleByteHashMapData(const tuple_references& pairsNew) noexcept {
		constexpr auto keyStatsVal		   = keyStats<value_type>();
		auto constructForGivenStringLength = [&](const auto maxSizeIndex, auto&& constructForGivenStringLength) mutable {
			hash_map_construction_data<value_type, maxSizeIndex> returnValues{};
			returnValues.keyStatsVal = keyStatsVal;
			bool collided{ true };
			while (returnValues.uniqueIndex < keyStatsVal.minLength) {
				std::fill(returnValues.indices.begin(), returnValues.indices.end(), returnValues.indices.size() - 1);
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
					return hash_map_construction_data_variant<value_type, maxSizeIndex, jsonifier_internal::hash_map_construction_data>{ constructForGivenStringLength(
						std::integral_constant<size_t, maxSizeIndex + 1>{}, constructForGivenStringLength) };
				} else {
					return hash_map_construction_data_variant<value_type, maxSizeIndexNew, jsonifier_internal::hash_map_construction_data>{
						collectSimdFullLengthHashMapData<value_type, maxSizeIndexNew>(pairsNew)
					};
				}
			} else {
				++returnValues.uniqueIndex;
				returnValues.type = hash_map_type::unique_byte_and_length;
				return hash_map_construction_data_variant<value_type, maxSizeIndexNew, jsonifier_internal::hash_map_construction_data>{ returnValues };
			}
		};
		return constructForGivenStringLength(std::integral_constant<size_t, maxSizeIndexNew>{}, constructForGivenStringLength);
	}

	template<typename value_type, size_t maxSizeIndexNew> JSONIFIER_ALWAYS_INLINE constexpr auto collectSingleByteHashMapData(const tuple_references& pairsNew) noexcept {
		constexpr auto keyStatsVal = keyStats<value_type>();
		hash_map_construction_data<value_type, getMaxSizeIndex(256)> returnValues{};
		returnValues.keyStatsVal = keyStatsVal;
		returnValues.uniqueIndex = findUniqueColumnIndex(pairsNew, returnValues.keyStatsVal);
		if (returnValues.uniqueIndex != std::numeric_limits<size_t>::max()) {
			std::fill(returnValues.indices.begin(), returnValues.indices.end(), returnValues.indices.size() - 1);
			for (size_t y = 0; y < pairsNew.count; ++y) {
				const auto slot			   = pairsNew.rootPtr[y].key.data()[returnValues.uniqueIndex];
				returnValues.indices[slot] = y;
			}
			++returnValues.uniqueIndex;
			returnValues.type = hash_map_type::single_byte;
			return hash_map_construction_data_variant<value_type, getMaxSizeIndex(256), jsonifier_internal::hash_map_construction_data>{ returnValues };
		} else {
			return hash_map_construction_data_variant<value_type, maxSizeIndexNew, jsonifier_internal::hash_map_construction_data>{
				collectUniqueSingleByteHashMapData<value_type, maxSizeIndexNew>(pairsNew)
			};
		}
	}

	template<typename value_type, size_t maxSizeIndexNew> JSONIFIER_ALWAYS_INLINE constexpr auto collectTripleElementHashMapData(const tuple_references& pairsNew) noexcept {
		constexpr auto keyStatsVal		   = keyStats<value_type>();
		auto constructForGivenStringLength = [&](const auto maxSizeIndex) mutable {
			hash_map_construction_data<value_type, maxSizeIndex> returnValues{};
			returnValues.keyStatsVal = keyStatsVal;
			bool collided{ true };
			returnValues.uniqueIndex = findUniqueColumnIndex(pairsNew, returnValues.keyStatsVal);
			const auto first		 = static_cast<uint8_t>(pairsNew.rootPtr[0].key[returnValues.uniqueIndex]);
			const auto mix1			 = static_cast<uint8_t>(pairsNew.rootPtr[1].key[returnValues.uniqueIndex]) ^ first;
			const auto mix2			 = static_cast<uint8_t>(pairsNew.rootPtr[2].key[returnValues.uniqueIndex]) ^ first;
			while (returnValues.uniqueIndex != std::numeric_limits<size_t>::max()) {
				for (size_t x = 0; x < 10; ++x) {
					uint8_t hash1 = (mix1 * returnValues.hasher.seed) % 4;
					uint8_t hash2 = (mix2 * returnValues.hasher.seed) % 4;
					returnValues.hasher.updateSeed();

					if (hash1 == 1 && hash2 == 2) {
						collided = false;
						break;
					}
				}
				if (!collided) {
					break;
				}
				returnValues.uniqueIndex = findUniqueColumnIndex(pairsNew, returnValues.keyStatsVal, returnValues.uniqueIndex + 1);
			}
			if (collided) {
				return hash_map_construction_data_variant<value_type, 0, jsonifier_internal::hash_map_construction_data>{ collectSingleByteHashMapData<value_type, maxSizeIndexNew>(
					pairsNew) };
			}
			++returnValues.uniqueIndex;
			returnValues.type = hash_map_type::triple_element;
			return hash_map_construction_data_variant<value_type, 0, jsonifier_internal::hash_map_construction_data>{ returnValues };
		};
		return constructForGivenStringLength(std::integral_constant<size_t, maxSizeIndexNew>{});
	}

	template<typename value_type, size_t maxSizeIndexNew> JSONIFIER_ALWAYS_INLINE constexpr auto collectDoubleElementHashMapData(const tuple_references& pairsNew) noexcept {
		constexpr auto keyStatsVal		   = keyStats<value_type>();
		auto constructForGivenStringLength = [&](const auto maxSizeIndex) mutable {
			hash_map_construction_data<value_type, maxSizeIndex> returnValues{};
			returnValues.keyStatsVal = keyStatsVal;
			bool collided{ true };
			returnValues.uniqueIndex = findUniqueColumnIndex(pairsNew, returnValues.keyStatsVal);
			while (returnValues.uniqueIndex != std::numeric_limits<size_t>::max()) {
				if (pairsNew.rootPtr[0].key[returnValues.uniqueIndex] % 2 == 0 && pairsNew.rootPtr[1].key[returnValues.uniqueIndex] % 2 == 1) {
					collided = false;
					break;
				}
				returnValues.uniqueIndex = findUniqueColumnIndex(pairsNew, returnValues.keyStatsVal, returnValues.uniqueIndex + 1);
			}
			if (collided) {
				return hash_map_construction_data_variant<value_type, 0, jsonifier_internal::hash_map_construction_data>{ collectSingleByteHashMapData<value_type, maxSizeIndexNew>(
					pairsNew) };
			}
			++returnValues.uniqueIndex;
			returnValues.type = hash_map_type::double_element;
			return hash_map_construction_data_variant<value_type, 0, jsonifier_internal::hash_map_construction_data>{ returnValues };
		};
		return constructForGivenStringLength(std::integral_constant<size_t, maxSizeIndexNew>{});
	}

	template<typename value_type> JSONIFIER_ALWAYS_INLINE constexpr auto collectMapConstructionDataImpl() noexcept {
		if constexpr (tupleReferences<value_type>.count >= 16) {
			return collectSingleByteHashMapData<value_type, getMaxSizeIndex(tupleReferences<value_type>.count)>(tupleReferences<value_type>);
		} else if constexpr (tupleReferences<value_type>.count == 0) {
			hash_map_construction_data<value_type, 0> returnValues{};
			returnValues.type = hash_map_type::empty;
			return hash_map_construction_data_variant<value_type, 0, jsonifier_internal::hash_map_construction_data>{ returnValues };
		} else if constexpr (tupleReferences<value_type>.count == 1) {
			hash_map_construction_data<value_type, 0> returnValues{};
			returnValues.type = hash_map_type::single_element;
			return hash_map_construction_data_variant<value_type, 0, jsonifier_internal::hash_map_construction_data>{ returnValues };
		} else if constexpr (tupleReferences<value_type>.count == 2) {
			return collectDoubleElementHashMapData<value_type, getMaxSizeIndex(tupleReferences<value_type>.count)>(tupleReferences<value_type>);
		} else if constexpr (tupleReferences<value_type>.count == 3) {
			return collectTripleElementHashMapData<value_type, getMaxSizeIndex(tupleReferences<value_type>.count)>(tupleReferences<value_type>);
		} else {
			return collectSingleByteHashMapData<value_type, getMaxSizeIndex(tupleReferences<value_type>.count)>(tupleReferences<value_type>);
		}
	}

	template<typename value_type> JSONIFIER_ALWAYS_INLINE constexpr auto collectMapConstructionData() noexcept {
		constexpr auto newTuple = collectMapConstructionDataImpl<value_type>();
		constexpr auto newIndex = newTuple.index();
		return std::get<newIndex>(newTuple);
	}

	template<typename value_type> static constexpr auto hashDataNew{ collectMapConstructionData<value_type>() };

	template<typename value_type, typename iterator_newer> JSONIFIER_ALWAYS_INLINE static size_t findIndex(iterator_newer& iter, iterator_newer& end) noexcept {
		static constexpr auto hashData{ collectMapConstructionData<value_type>() };
		JSONIFIER_ALIGN static constexpr auto controlBytes = hashData.controlBytes;
		JSONIFIER_ALIGN static constexpr auto indices	   = hashData.indices;
		static constexpr auto uniqueIndex				   = hashData.uniqueIndex;
		if constexpr (hashData.type == hash_map_type::single_element) {
			return 0;
		} else if constexpr (hashData.type == hash_map_type::double_element) {
			return iter[uniqueIndex] & 1;
		} else if constexpr (hashData.type == hash_map_type::triple_element) {
			static constexpr auto firstChar = std::get<0>(coreTupleV<value_type>).view()[uniqueIndex];
			return (uint8_t(iter[uniqueIndex] ^ firstChar) * hashData.hasher.seed) & 3;
		} else if constexpr (hashData.type == hash_map_type::single_byte) {
			return indices[iter[uniqueIndex]];
		} else if constexpr (hashData.type == hash_map_type::unique_byte_and_length) {
			const char* newPtr{};
			if constexpr (hashData.keyStatsVal.minLength >= 4) {
				newPtr = iter + 1 + hashData.keyStatsVal.minLength;
			} else {
				newPtr = iter + 1;
			}
			newPtr = memchar<'"'>(newPtr, static_cast<size_t>(end - (iter)));
			if (newPtr != nullptr) [[likely]] {
				size_t length	= static_cast<size_t>(newPtr - (iter + 1));
				const auto hash = iter[uniqueIndex] ^ length;
				const auto slot = hash % hashData.storageSize;
				return indices[slot];
			}
			return hashData.storageSize;
		} else if constexpr (hashData.type == hash_map_type::simd_full_length) {
			using simd_type = typename unwrap_t<decltype(hashData)>::simd_type;
			const char* newPtr{};
			if constexpr (hashData.keyStatsVal.minLength >= 4) {
				newPtr = iter + 1 + hashData.keyStatsVal.minLength;
			} else {
				newPtr = iter + 1;
			}
			newPtr				   = memchar<'"'>(newPtr, static_cast<size_t>(end - (iter)));
			size_t length		   = static_cast<size_t>(newPtr - (iter + 1));
			length				   = uniqueIndex > length ? length : uniqueIndex;
			const auto hash		   = hashData.hasher.hashKeyRt(iter + 1, length);
			const auto resultIndex = ((hash >> 8) & (hashData.numGroups - 1)) * hashData.bucketSize;
			const auto finalIndex  = (tzcnt(simd_internal::opCmpEq(simd_internal::gatherValue<simd_type>(static_cast<uint8_t>(hash)),
										  simd_internal::gatherValues<simd_type>(controlBytes.data() + resultIndex))) +
				 resultIndex);
			return indices[finalIndex];
		} else if constexpr (hashData.type == hash_map_type::empty) {
			return 0;
		} else {
			static_assert(hashData.type != hash_map_type::unset, "Sorry, but we failed to construct this hash-map!");
			return false;
		}
	}


}