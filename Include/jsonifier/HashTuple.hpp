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

#include <jsonifier/TypeEntities.hpp>
#include <jsonifier/StringView.hpp>
#include <jsonifier/Reflection.hpp>
#include <jsonifier/Derailleur.hpp>
#include <jsonifier/Tuple.hpp>
#include <algorithm>
#include <numeric>

namespace jsonifier_internal {

	struct tuple_reference {
		size_t stringLength{};
		size_t oldIndex{};
		size_t newIndex{};
	};

	struct string_length_counts {
		size_t length{};
		size_t count{};
	};

	template<typename tuple_type, size_t currentIndex = 0, size_t maxIndex = std::tuple_size_v<jsonifier_internal::unwrap_t<tuple_type>>>
	constexpr auto collectTupleReferences(tuple_type&& tuple, std::array<tuple_reference, maxIndex>& arrayOfRefs) {
		if constexpr (currentIndex < maxIndex) {
			tuple_reference values{};
			values.oldIndex			  = currentIndex;
			values.stringLength		  = std::get<currentIndex>(tuple).view().size();
			arrayOfRefs[currentIndex] = values;
			return collectTupleReferences<tuple_type, currentIndex + 1>(tuple, arrayOfRefs);
		} else {
			return arrayOfRefs;
		}
	}

	template<size_t size> constexpr void bubbleSort(std::array<tuple_reference, size>& arrayOfRefs) {
		for (size_t i = 0; i < size - 1; ++i) {
			for (size_t j = 0; j < size - i - 1; ++j) {
				if (arrayOfRefs[j].stringLength > arrayOfRefs[j + 1].stringLength) {
					std::swap(arrayOfRefs[j], arrayOfRefs[j + 1]);
				}
			}
		}
		for (size_t i = 0; i < size; ++i) {
			arrayOfRefs[i].newIndex = i;
		}
	}

	template<typename value_type> constexpr auto sortTupleReferences() {
		constexpr auto tuple = jsonifier::core<unwrap_t<value_type>>::parseValue.parseValue;
		std::array<tuple_reference, std::tuple_size_v<jsonifier_internal::unwrap_t<decltype(tuple)>>> arrayOfRefs{};
		auto collectedRefs = collectTupleReferences(tuple, arrayOfRefs);
		bubbleSort(collectedRefs);
		return collectedRefs;
	}

	template<size_t start, size_t end, size_t... indices> constexpr auto generateCustomIndexSequence(std::index_sequence<indices...>) {
		return std::index_sequence<(start + indices)...>{};
	}

	template<size_t start, size_t end> struct custom_index_sequence_generator;

	template<size_t start, size_t end> struct custom_index_sequence_generator {
		using type = decltype(generateCustomIndexSequence<start, end>(std::make_index_sequence<end - start + 1>{}));
	};

	template<size_t start, size_t end> using custom_index_sequence = typename custom_index_sequence_generator<start, end>::type;

	template<size_t size> constexpr auto countUniqueStringLengths(const std::array<tuple_reference, size>& arrayOfRefs) {
		size_t currentLength{};
		size_t currentCount{};
		for (size_t x = 0; x < size; ++x) {
			if (arrayOfRefs[x].stringLength != currentLength) {
				currentLength = arrayOfRefs[x].stringLength;
				++currentCount;
			}
		}
		return currentCount;
	}

	template<size_t counts, size_t size> constexpr auto collectUniqueStringLengths(const std::array<tuple_reference, size>& arrayOfRefs) {
		std::array<string_length_counts, counts> returnValues{};
		size_t currentIndex	 = 0;
		size_t currentLength = static_cast<size_t>(-1);

		for (size_t x = 0; x < arrayOfRefs.size(); ++x) {
			auto keyLength = arrayOfRefs[x].stringLength;
			if (currentLength != keyLength && currentIndex < counts) {
				currentLength					  = keyLength;
				returnValues[currentIndex].length = keyLength;
				returnValues[currentIndex].count  = 1;
				++currentIndex;
			} else if (currentIndex - 1 < returnValues.size()) {
				++returnValues[currentIndex - 1].count;
			}
		}

		return returnValues;
	}

	template<size_t inputIndex, const auto& stringLengths> constexpr auto getCurrentStartingIndex() {
		size_t currentStartingIndex{};
		for (size_t x = 0; x < inputIndex; ++x) {
			currentStartingIndex += stringLengths[x].count;
		}
		return currentStartingIndex;
	}

	template<typename value_type> constexpr auto sortedTupleReferences{ sortTupleReferences<value_type>() };
	template<typename value_type> constexpr auto uniqueStringLengthCount{ countUniqueStringLengths(sortedTupleReferences<value_type>) };
	template<typename value_type> constexpr auto uniqueStringLengths{ collectUniqueStringLengths<uniqueStringLengthCount<value_type>>(sortedTupleReferences<value_type>) };

	template<typename value_type, size_t... indices> constexpr auto createGroupedSubTuple(std::index_sequence<indices...>) {
		constexpr auto& oldTuple  = jsonifier::concepts::coreV<value_type>;
		constexpr auto& tupleRefs = sortedTupleReferences<value_type>;
		return std::make_tuple(std::get<tupleRefs[indices].oldIndex>(oldTuple)...);
	}

	template<typename value_type, size_t startIndex, size_t count> constexpr auto createGroupedTuple() {
		constexpr auto& tupleRefs = sortedTupleReferences<value_type>;
		return createGroupedSubTuple<value_type>(custom_index_sequence<startIndex, (startIndex + count) - 1>{});
	}

	template<typename value_type, size_t... indices> constexpr auto createNewTupleInternal(std::index_sequence<indices...>) {
		constexpr auto& stringLengths = uniqueStringLengths<value_type>;
		return std::make_tuple(createGroupedTuple<value_type, getCurrentStartingIndex<indices, stringLengths>(), stringLengths[indices].count>()...);
	}

	template<typename value_type> constexpr auto createNewTuple() {
		constexpr auto& tupleRefs = sortedTupleReferences<value_type>;
		return createNewTupleInternal<value_type>(std::make_index_sequence<countUniqueStringLengths(tupleRefs)>{});
	}

	template<typename value_type> constexpr auto finalTuple = createNewTuple<value_type>();

	template<typename value_type> using final_tuple_t = decltype(finalTuple<unwrap_t<value_type>>);

	template<typename value_type> struct hash_tuple;

	template<typename value_type01, typename value_type02> constexpr bool contains(const value_type01* hashData, value_type02 byteToCheckFor, size_t size) {
		for (size_t x = 0; x < size; ++x) {
			if (hashData[x] == byteToCheckFor) {
				return true;
			}
		}
		return false;
	}

	template<size_t size, size_t length> constexpr size_t getMaxSizeIndex(const std::array<size_t, length>& maxSizes) {
		for (size_t x = 0; x < std::size(maxSizes); ++x) {
			if (size <= maxSizes[x]) {
				return x;
			}
		}
		return std::size(maxSizes) - 1;
	}

	template<size_t length> constexpr size_t setSimdWidth() {
		return length >= 64 && bytesPerStep >= 64 ? 64 : length >= 32 && bytesPerStep >= 32 ? 32 : 16;
	}

	constexpr std::array<size_t, 4> hashTupleMaxSizes{ 16, 32, 64, 128 };

	template<size_t length> struct tuple_simd {
		using type = std::conditional_t<length >= 64 && bytesPerStep >= 64, jsonifier_simd_int_512,
			std::conditional_t<length >= 32 && bytesPerStep >= 32, jsonifier_simd_int_256, jsonifier_simd_int_128>>;
	};

	template<size_t length> using tuple_simd_t = tuple_simd<length>::type;

	template<const auto& tuple, size_t index> JSONIFIER_INLINE bool compareStringFunctionNonConst(const char* string01) {
		if constexpr (index < std::tuple_size_v<jsonifier_internal::unwrap_t<decltype(tuple)>>) {
			static constexpr auto currentKey = jsonifier_internal::getKey<tuple, index>();
			return jsonifier_internal::compare<currentKey.size()>(currentKey.data(), string01);
		} else {
			return false;
		}
	}

	template<const auto& tuple, size_t index> JSONIFIER_INLINE constexpr bool compareStringFunctionConst(const char* string01) {
		if constexpr (index < std::tuple_size_v<jsonifier_internal::unwrap_t<decltype(tuple)>>) {
			constexpr auto currentKey = jsonifier_internal::getKey<tuple, index>();
			return currentKey == jsonifier::string_view{ string01, currentKey.size() };
		} else {
			return false;
		}
	}

	template<const auto& tuple, size_t index> using compare_string_function_non_const_tuple_ptr = decltype(&compareStringFunctionNonConst<tuple, index>);

	template<const auto& tuple, size_t index> using compare_string_function_const_tuple_ptr = decltype(&compareStringFunctionConst<tuple, index>);

	template<const auto& tuple, size_t currentIndex> constexpr auto getCurrentConstFunction() {
		return &compareStringFunctionConst<tuple, currentIndex>;
	}

	template<const auto& tuple, size_t... indices> JSONIFIER_INLINE constexpr auto generateConstCompareStringFunctionPtrArrayInternal(std::index_sequence<indices...>) {
		return std::array<compare_string_function_non_const_tuple_ptr<tuple, 0>, sizeof...(indices)>{ { getCurrentConstFunction<tuple, indices>()... } };
	}

	template<uint64_t size, const auto& tuple> JSONIFIER_INLINE constexpr auto generateConstCompareStringFunctionPtrArray() {
		return generateConstCompareStringFunctionPtrArrayInternal<tuple>(std::make_index_sequence<size>{});
	}

	template<const auto& tuple, size_t currentIndex> constexpr auto getCurrentNonConstFunction() {
		return &compareStringFunctionNonConst<tuple, currentIndex>;
	}

	template<const auto& tuple, size_t... indices> JSONIFIER_INLINE constexpr auto generateNonConstCompareStringFunctionPtrArrayInternal(std::index_sequence<indices...>) {
		return std::array<compare_string_function_non_const_tuple_ptr<tuple, 0>, sizeof...(indices)>{ { getCurrentNonConstFunction<tuple, indices>()... } };
	}

	template<uint64_t size, const auto& tuple> JSONIFIER_INLINE constexpr auto generateNonConstCompareStringFunctionPtrArray() {
		return generateNonConstCompareStringFunctionPtrArrayInternal<tuple>(std::make_index_sequence<size>{});
	}

	template<typename key_type_new, typename value_type> struct simd_hash_tuple_construction_data_pre : public jsonifier_internal::key_hasher {
		JSONIFIER_ALIGN size_t stringLength{ std::numeric_limits<size_t>::max() };
		JSONIFIER_ALIGN size_t maxSizeIndex{};

		constexpr simd_hash_tuple_construction_data_pre() noexcept = default;
	};

	template<typename key_type_new, typename value_type, size_t subTupleIndexNew, size_t actualCountNew, size_t storageSizeNew, typename tuple_type>
	struct simd_hash_tuple_construction_data : public jsonifier_internal::key_hasher {
		using simd_type = tuple_simd_t<storageSizeNew>;
		JSONIFIER_ALIGN size_t actualCount{ actualCountNew };
		JSONIFIER_ALIGN size_t storageSize{ storageSizeNew };
		JSONIFIER_ALIGN size_t subTupleIndex{ subTupleIndexNew };
		JSONIFIER_ALIGN size_t bucketSize{ setSimdWidth<storageSizeNew>() };
		JSONIFIER_ALIGN size_t numGroups{ storageSize > bucketSize ? storageSize / bucketSize : 1 };
		JSONIFIER_ALIGN std::array<uint8_t, storageSizeNew> controlBytes{};
		JSONIFIER_ALIGN size_t stringLength{};
		static constexpr auto tuple{ std::get<subTupleIndexNew>(finalTuple<value_type>) };

		constexpr simd_hash_tuple_construction_data() noexcept = default;
	};

	template<typename key_type, typename value_type, typename tuple_type, size_t subTupleIndex, size_t actualCount, size_t storageSize,
		template<typename, typename, size_t, size_t, size_t, typename> typename simd_hash_tuple_construction_data_type>
	using simd_hash_tuple_construction_data_variant =
		std::variant<simd_hash_tuple_construction_data_type<key_type, value_type, subTupleIndex, actualCount, storageSize, tuple_type>,
			simd_hash_tuple_construction_data_type<key_type, value_type, subTupleIndex, actualCount, storageSize * 2, tuple_type>,
			simd_hash_tuple_construction_data_type<key_type, value_type, subTupleIndex, actualCount, storageSize * 4, tuple_type>,
			simd_hash_tuple_construction_data_type<key_type, value_type, subTupleIndex, actualCount, storageSize * 8, tuple_type>>;

	template<typename key_type, typename value_type, typename tuple_type, size_t subTupleIndex, size_t actualCount, size_t storageSize> constexpr auto
	collectHashSubTupleConstructionDataFinal(const std::array<key_type, actualCount>& pairsNew, simd_hash_tuple_construction_data_pre<key_type, value_type> constructionValues)
		-> simd_hash_tuple_construction_data_variant<key_type, value_type, tuple_type, subTupleIndex, actualCount, 16, simd_hash_tuple_construction_data> {
		constexpr size_t bucketSize = setSimdWidth<storageSize>();
		constexpr size_t numGroups	= storageSize > bucketSize ? storageSize / bucketSize : 1;
		simd_hash_tuple_construction_data<key_type, value_type, subTupleIndex, actualCount, storageSize, tuple_type> simdHashMapNew{};
		simdHashMapNew.setSeedCt(constructionValues.operator uint64_t());
		simdHashMapNew.stringLength = constructionValues.stringLength;
		std::fill(simdHashMapNew.controlBytes.begin(), simdHashMapNew.controlBytes.end(), std::numeric_limits<uint8_t>::max());
		std::array<size_t, numGroups> bucketSizes{};
		for (size_t x = 0; x < actualCount; ++x) {
			const auto keySize				  = simdHashMapNew.stringLength;
			const auto hash					  = simdHashMapNew.hashKeyCt(pairsNew[x].data(), keySize);
			const auto groupPos				  = (hash >> 8) % numGroups;
			const auto ctrlByte				  = static_cast<uint8_t>(hash);
			const auto bucketSizeNew		  = bucketSizes[groupPos]++;
			const auto slot					  = ((groupPos * bucketSize) + bucketSizeNew);
			simdHashMapNew.controlBytes[slot] = ctrlByte;
		}

		return simd_hash_tuple_construction_data_variant<key_type, value_type, tuple_type, subTupleIndex, actualCount, 16, simd_hash_tuple_construction_data>{
			simd_hash_tuple_construction_data<key_type, value_type, subTupleIndex, actualCount, storageSize, tuple_type>(simdHashMapNew)
		};
	}

	template<typename key_type, typename value_type, typename tuple_type, size_t subTupleIndex, size_t actualCount> using construct_simd_hash_map_function_ptr =
		decltype(&collectHashSubTupleConstructionDataFinal<key_type, value_type, tuple_type, subTupleIndex, actualCount, 16ull>);

	template<typename key_type, typename value_type, typename tuple_type, size_t subTupleIndex, size_t actualCount>
	constexpr construct_simd_hash_map_function_ptr<key_type, value_type, tuple_type, subTupleIndex, actualCount> collectHashSubTupleConstructionDataFinalPtrs[4] = {
		&collectHashSubTupleConstructionDataFinal<key_type, value_type, tuple_type, subTupleIndex, actualCount, 16ull>,
		&collectHashSubTupleConstructionDataFinal<key_type, value_type, tuple_type, subTupleIndex, actualCount, 32ull>,
		&collectHashSubTupleConstructionDataFinal<key_type, value_type, tuple_type, subTupleIndex, actualCount, 64ull>,
		&collectHashSubTupleConstructionDataFinal<key_type, value_type, tuple_type, subTupleIndex, actualCount, 128ull>
	};

	template<typename key_type, typename value_type, size_t actualCount> constexpr auto collectHashSubTupleConstructionDataHelper(const std::array<key_type, actualCount>& pairsNew,
		jsonifier_internal::xoshiro256 prng, jsonifier_internal::key_stats_t keyStatsVal) {
		auto constructForGivenStringLength = [&](const auto maxSizeIndex, auto&& constructForGivenStringLength,
												 size_t stringLength) mutable -> simd_hash_tuple_construction_data_pre<key_type, value_type> {
			constexpr size_t bucketSize	 = setSimdWidth<hashTupleMaxSizes[maxSizeIndex]>();
			constexpr size_t storageSize = hashTupleMaxSizes[maxSizeIndex];
			constexpr size_t numGroups	 = storageSize > bucketSize ? storageSize / bucketSize : 1;
			simd_hash_tuple_construction_data_pre<key_type, value_type> returnValues{};
			size_t bucketSizes[numGroups]{};
			bool collided{};
			std::array<uint8_t, storageSize> controlBytes{};
			for (size_t x = 0; x < 2; ++x) {
				std::fill(controlBytes.begin(), controlBytes.end(), std::numeric_limits<uint8_t>::max());
				auto seed = prng();
				returnValues.setSeedCt(seed);
				collided = false;
				for (size_t y = 0; y < actualCount; ++y) {
					const auto keySize		 = pairsNew[y].size() > stringLength ? stringLength : pairsNew[y].size();
					const auto hash			 = returnValues.hashKeyCt(pairsNew[y].data(), keySize);
					const auto groupPos		 = (hash >> 8) % numGroups;
					const auto ctrlByte		 = static_cast<uint8_t>(hash);
					const auto bucketSizeNew = bucketSizes[groupPos]++;
					const auto slot			 = ((groupPos * bucketSize) + bucketSizeNew);

					if (bucketSizeNew >= bucketSize || contains(controlBytes.data() + groupPos * bucketSize, ctrlByte, bucketSize)) {
						std::fill(bucketSizes, bucketSizes + numGroups, 0);
						collided = true;
						break;
					}
					controlBytes[slot] = ctrlByte;
				}
				if (!collided) {
					break;
				}
			}
			if (collided) {
				if constexpr (maxSizeIndex < std::size(hashTupleMaxSizes) - 1) {
					return constructForGivenStringLength(std::integral_constant<size_t, maxSizeIndex + 1>{}, constructForGivenStringLength, stringLength);
				} else if (stringLength < keyStatsVal.maxLength - 1) {
					return constructForGivenStringLength(std::integral_constant<size_t, 0>{}, constructForGivenStringLength, stringLength + 1);
				}
				returnValues.stringLength = std::numeric_limits<size_t>::max();
				returnValues.maxSizeIndex = std::numeric_limits<size_t>::max();
				return returnValues;
			}
			returnValues.stringLength = stringLength;
			returnValues.maxSizeIndex = maxSizeIndex;
			return returnValues;
		};

		return constructForGivenStringLength(std::integral_constant<size_t, 0>{}, constructForGivenStringLength, 1);
	}

	template<typename tuple_type, typename key_type, typename value_type, size_t actualCount, size_t subTupleIndex>
	constexpr auto collectHashSubTupleConstructionData(const std::array<key_type, actualCount>& pairsNew) {
		constexpr auto& tuple	= std::get<subTupleIndex>(finalTuple<value_type>);
		auto keyStatsVal		= jsonifier_internal::keyStats<tuple>();
		auto constructionValues = collectHashSubTupleConstructionDataHelper<key_type, value_type, actualCount>(pairsNew, jsonifier_internal::xoshiro256{}, keyStatsVal);
		return collectHashSubTupleConstructionDataFinalPtrs<key_type, value_type, tuple_type, subTupleIndex, actualCount>[constructionValues.maxSizeIndex](pairsNew,
			constructionValues);
	}

	template<typename value_type, const auto& tuple, size_t subTupleIndex, size_t... I>
	JSONIFIER_INLINE constexpr auto generateSubTupleConstructionDataImpl(std::index_sequence<I...>) {
		using tuple_type		 = jsonifier_internal::unwrap_t<decltype(tuple)>;
		constexpr auto tupleSize = std::tuple_size_v<tuple_type>;

		if constexpr (tupleSize == 0) {
			return nullptr;
		} else {
			jsonifier_internal::xoshiro256 prng{};
			constexpr auto mapNew =
				collectHashSubTupleConstructionData<tuple_type, jsonifier::string_view, value_type, tupleSize, subTupleIndex>({ jsonifier_internal::getKey<tuple, I>()... });
			constexpr auto newIndex	   = mapNew.index();
			constexpr auto newSubTuple = std::get<newIndex>(mapNew);
			return newSubTuple;
		}
	}

	template<typename value_type, size_t subTupleIndex> JSONIFIER_INLINE constexpr auto generateSubTupleConstructionData() {
		constexpr auto tupleSize = std::tuple_size_v<jsonifier_internal::unwrap_t<decltype(std::get<subTupleIndex>(finalTuple<value_type>))>>;
		return generateSubTupleConstructionDataImpl<value_type, std::get<subTupleIndex>(finalTuple<value_type>), subTupleIndex>(std::make_index_sequence<tupleSize>{});
	}

	template<typename value_type, size_t... indices> constexpr auto generateHashTupleConstructionDataImpl(std::index_sequence<indices...>) {
		return std::make_tuple(generateSubTupleConstructionData<value_type, indices>()...);
	}

	template<typename value_type> constexpr auto generateHashTupleConstructionData() {
		constexpr auto tupleSize = std::tuple_size_v<jsonifier_internal::unwrap_t<decltype(finalTuple<value_type>)>>;
		return generateHashTupleConstructionDataImpl<value_type>(std::make_index_sequence<tupleSize>{});
	}

	template<typename value_type> struct hash_tuple_construction_data {
		static constexpr auto constructionData = generateHashTupleConstructionData<value_type>();
	};

	template<typename value_type, size_t subTupleIndexNew> struct simd_hash_tuple : public jsonifier_internal::key_hasher {
		static constexpr auto& constructionData{ std::get<subTupleIndexNew>(hash_tuple_construction_data<value_type>::constructionData) };
		using simd_type	   = typename jsonifier_internal::unwrap_t<decltype(constructionData)>::simd_type;
		using size_type	   = size_t;
		using control_type = uint8_t;
		static constexpr auto& tuple{ constructionData.tuple };
		static constexpr auto subTupleIndex{ subTupleIndexNew };

		static constexpr auto nonConstCompareStringFunctions{ generateNonConstCompareStringFunctionPtrArray<constructionData.storageSize + 1, tuple>() };

		static constexpr auto constCompareStringFunctions{ generateConstCompareStringFunctionPtrArray<constructionData.storageSize + 1, tuple>() };

		constexpr simd_hash_tuple() noexcept = default;


		template<const auto& functionPtrs> JSONIFIER_INLINE constexpr auto find(const char* iter, size_type keyLength) const noexcept {
			if (!std::is_constant_evaluated()) {
				JSONIFIER_ALIGN const auto keySize	   = keyLength > constructionData.stringLength ? constructionData.stringLength : keyLength;
				JSONIFIER_ALIGN const auto hash		   = hashKeyRt(iter, keySize);
				JSONIFIER_ALIGN const auto resultIndex = ((hash >> 8) % constructionData.numGroups) * constructionData.bucketSize;
				JSONIFIER_ALIGN const auto finalIndex  = (simd_internal::tzcnt(simd_internal::opCmpEq(simd_internal::gatherValue<simd_type>(static_cast<control_type>(hash)),
															  simd_internal::gatherValues<simd_type>(constructionData.controlBytes.data() + resultIndex))) +
					 resultIndex);
				if (nonConstCompareStringFunctions[finalIndex](iter)) {
					return functionPtrs.data() + finalIndex;
				} else {
					return functionPtrs.data() + functionPtrs.size();
				}
			} else {
				JSONIFIER_ALIGN const auto keySize	   = keyLength > constructionData.stringLength ? constructionData.stringLength : keyLength;
				JSONIFIER_ALIGN const auto hash		   = hashKeyCt(iter, keySize);
				JSONIFIER_ALIGN const auto resultIndex = ((hash >> 8) % constructionData.numGroups) * constructionData.bucketSize;
				JSONIFIER_ALIGN const auto finalIndex  = (constMatch(constructionData.controlBytes.data() + resultIndex, static_cast<control_type>(hash)) + resultIndex);
				if (constCompareStringFunctions[finalIndex](iter)) {
					return functionPtrs.data() + finalIndex;
				} else {
					return functionPtrs.data() + functionPtrs.size();
				}
			}
		}

		JSONIFIER_INLINE constexpr auto find(const char* iter, size_type keyLength) const noexcept {
			if (!std::is_constant_evaluated()) {
				JSONIFIER_ALIGN const auto keySize	   = constructionData.stringLength;
				JSONIFIER_ALIGN const auto hash		   = hashKeyCt(iter, keySize);
				JSONIFIER_ALIGN const auto resultIndex = ((hash >> 8) % constructionData.numGroups) * constructionData.bucketSize;
				JSONIFIER_ALIGN const auto finalIndex  = (simd_internal::tzcnt(simd_internal::opCmpEq(simd_internal::gatherValue<simd_type>(static_cast<control_type>(hash)),
															  simd_internal::gatherValues<simd_type>(constructionData.controlBytes.data() + resultIndex))) +
					 resultIndex);
				if (constCompareStringFunctions[finalIndex](iter)) {
					return true;
				} else {
					throw false;
				}
			} else {
				JSONIFIER_ALIGN const auto keySize	   = keyLength > constructionData.stringLength ? constructionData.stringLength : keyLength;
				JSONIFIER_ALIGN const auto hash		   = hashKeyCt(iter, keySize);
				JSONIFIER_ALIGN const auto resultIndex = ((hash >> 8) % constructionData.numGroups) * constructionData.bucketSize;
				JSONIFIER_ALIGN const auto finalIndex  = (constMatch(constructionData.controlBytes.data() + resultIndex, static_cast<control_type>(hash)) + resultIndex);
				if (constCompareStringFunctions[finalIndex](iter)) {
					return true;
				} else {
					return false;
				}
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
			for (size_type x = 0; x < constructionData.bucketSize; ++x) {
				if (hashData[x] == hash) {
					mask |= (1ull << x);
				}
			}
			return tzcnt(mask);
		}
	};

	template<size_t inputIndex, const auto& stringLengths> constexpr auto getCurrentSubTupleIndex() {
		for (size_t x = 0; x < stringLengths.size(); ++x) {
			if (inputIndex == stringLengths[x].length) {
				return x;
			}
		}
		return std::numeric_limits<size_t>::max();
	}

	template<const auto& function, typename value_type, size_t stringLength, typename... arg_types> JSONIFIER_INLINE auto collectSubTuple(arg_types&&... args) {
		using tuple_type	   = typename std::decay_t<decltype(finalTuple<value_type>)>;
		constexpr size_t index = getCurrentSubTupleIndex<stringLength, uniqueStringLengths<value_type>>();
		if constexpr (index < std::tuple_size_v<tuple_type>) {
			return function(std::integral_constant<size_t, index>{}, std::forward<arg_types>(args)...);
		} else {
			return false;
		}
	};

	template<const auto& function, typename value_type, size_t index, typename... arg_types> using collect_sub_tuple_ptr =
		decltype(&collectSubTuple<function, value_type, index, arg_types...>);

	template<const auto& function, typename value_type, typename... arg_types, size_t... indices>
	JSONIFIER_INLINE constexpr auto generateCollectSubTuplePtrArrayInternal(std::index_sequence<indices...>) {
		return std::array<collect_sub_tuple_ptr<function, value_type, 0, arg_types...>, sizeof...(indices)>{ { &collectSubTuple<function, value_type, indices, arg_types...>... } };
	}

	template<const auto& function, typename value_type, typename... arg_types> JSONIFIER_INLINE constexpr auto generateCollectSubTuplePtrArray() {
		return generateCollectSubTuplePtrArrayInternal<function, value_type, arg_types...>(std::make_index_sequence<512>{});
	}

	template<typename value_type, size_t subTupleIndex> constexpr auto collectHashSubTuple() {
		simd_hash_tuple<value_type, subTupleIndex> subTuple{};
		subTuple.setSeedCt(std::get<subTupleIndex>(hash_tuple_construction_data<value_type>::constructionData).operator uint64_t());
		return subTuple;
	}

	template<typename value_type, std::size_t... indices> JSONIFIER_INLINE constexpr auto makeHashTupleInternal(std::index_sequence<indices...>) {
		return std::make_tuple(collectHashSubTuple<value_type, indices>()...);
	}

	template<typename value_type> struct hash_tuple {
		using size_type = size_t;
		static constexpr auto tupleSize{ std::tuple_size_v<decltype(finalTuple<value_type>)> };
		static constexpr auto tuple{ makeHashTupleInternal<value_type>(std::make_index_sequence<tupleSize>{}) };

		constexpr hash_tuple() noexcept {};

		template<const auto& function, typename... arg_types> JSONIFIER_INLINE constexpr auto find(size_type stringLength, arg_types&&... args) const {
			constexpr auto getSubTuplePtrArray = generateCollectSubTuplePtrArray<function, value_type, arg_types...>();
			return getSubTuplePtrArray[stringLength](std::forward<arg_types>(args)...);
		}
	};

	template<typename value_type> JSONIFIER_INLINE constexpr auto makeHashTuple() {
		return hash_tuple<value_type>{};
	}

}