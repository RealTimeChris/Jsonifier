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

	template<size_t length> struct map_simd {
		using type = std::conditional_t<length >= 64 && bytesPerStep >= 64, jsonifier_simd_int_512, std::conditional_t<length >= 32 && bytesPerStep >= 32, jsonifier_simd_int_256, jsonifier_simd_int_128>>;
	};

	template<size_t length> constexpr size_t setSimdWidth() {
		return length >= 64 && bytesPerStep >= 64 ? 64 : length >= 32 && bytesPerStep >= 32 ? 32 : 16;
	}

	template<size_t length> using map_simd_t = map_simd<length>::type;

	struct map_construction_values {
		size_t stringLength{ std::numeric_limits<size_t>::max() };
		size_t maxSizeIndex{ std::numeric_limits<size_t>::max() };
		bool success{};
		size_t seed{};
	};

	template<typename value_type01, typename value_type02> constexpr bool contains(const value_type01* hashData, value_type02 byteToCheckFor, size_t size) {
		for (size_t x = 0; x < size; ++x) {
			if (hashData[x] == byteToCheckFor) {
				return true;
			}
		}
		return false;
	}

	template<size_t size, size_t length> constexpr size_t getMaxSizeIndex(const std::array<size_t, length> maxSizes) {
		for (size_t x = 0; x < std::size(maxSizes); ++x) {
			if (size <= maxSizes[x]) {
				return x;
			}
		}
		return std::size(maxSizes) - 1;
	}

	struct string_length_counts {
		size_t length{};
		size_t count{};
	};

	struct tuple_element_reference {
		jsonifier::string_view key{};
		size_t tupleIndex{};
	};

	template<size_t index, typename value_type> constexpr tuple_element_reference extract(const value_type& t) {
		return { t.view(), index };
	}

	template<typename tuple_type, size_t... I> constexpr auto collectTupleElementReferencesImpl(const tuple_type& t, std::index_sequence<I...>) {
		return std::array<tuple_element_reference, sizeof...(I)>{ extract<I>(std::get<I>(t))... };
	}

	template<typename tuple_type> constexpr auto collectTupleElementReferences(const tuple_type& t) {
		return collectTupleElementReferencesImpl(t, std::make_index_sequence<std::tuple_size_v<tuple_type>>{});
	}

	template<size_t size> constexpr auto sortTupleElementReferences(const std::array<tuple_element_reference, size>& values) {
		std::array<tuple_element_reference, size> newValues{ values };
		std::sort(newValues.begin(), newValues.end(), [](auto lhs, auto rhs) {
			return lhs.key.size() < rhs.key.size();
			});
		return newValues;
	}

	template<const auto& arr> constexpr std::size_t countUniqueStringLengths() {
		size_t count{};
		size_t currentLength{};
		for (size_t x = 0; x < arr.size(); ++x) {
			if (currentLength != arr[x].key.size()) {
				currentLength = arr[x].key.size();
				++count;
			}
		}
		return count;
	}

	template<std::size_t counts, const auto& arr> constexpr auto countUniqueStringLengths() {
		std::array<string_length_counts, counts> returnValues{};
		size_t currentIndex = 0;
		std::size_t currentLength = static_cast<std::size_t>(-1);

		for (std::size_t x = 0; x < arr.size(); ++x) {
			auto keyLength = arr[x].key.size();
			if (currentLength != keyLength && currentIndex < counts) {
				currentLength = keyLength;
				returnValues[currentIndex].length = keyLength;
				returnValues[currentIndex].count = 1;
				++currentIndex;
			}
			else {
				++returnValues[currentIndex - 1].count;
			}
		}

		return returnValues;
	}

	template<size_t inputIndex, const auto& stringLengths> constexpr auto getCurrentSubTupleIndex() {
		for (size_t x = 0; x < stringLengths.size(); ++x) {
			if (inputIndex == stringLengths[x].length) {
				return x;
			}
		}
		return std::numeric_limits<size_t>::max();
	}

	template<size_t inputIndex, const auto& stringLengths> constexpr auto getCurrentStartingIndex() {
		size_t currentStartingIndex{};
		for (size_t x = 0; x < inputIndex; ++x) {
			currentStartingIndex += stringLengths[x].count;
		}
		return currentStartingIndex;
	}

	template<const auto& stringLengths, const auto& tupleReferences, const auto& tuple, size_t startIndex, size_t... indices>
	constexpr auto constructNewSubTupleInternal(std::index_sequence<indices...>) {
		return std::make_tuple(std::get<tupleReferences[startIndex + indices].tupleIndex>(tuple)...);
	}

	template<const auto& stringLengths, const auto& tupleReferences, const auto& tuple, size_t startIndex, size_t maxIndex> constexpr auto constructNewSubTuple() {
		return constructNewSubTupleInternal<stringLengths, tupleReferences, tuple, startIndex>(std::make_index_sequence<maxIndex - startIndex>{});
	}

	template<const auto& stringLengths, const auto& tupleReferences, const auto& tuple, std::size_t... indices>
	constexpr auto constructNewTupleInternal(std::index_sequence<indices...>) {
		return std::make_tuple(constructNewSubTuple<stringLengths, tupleReferences, tuple, getCurrentStartingIndex<indices, stringLengths>(),
			getCurrentStartingIndex<indices + 1, stringLengths>()>()...);
	}

	template<const auto& stringLengths, const auto& tupleReferences, const auto& tuple> constexpr auto constructNewTuple() {
		return constructNewTupleInternal<stringLengths, tupleReferences, tuple>(std::make_index_sequence<stringLengths.size()>{});
	}

	constexpr std::array<size_t, 5> hashTupleMaxSizes{ 16, 32, 64, 128, 256 };

	struct tuple_construction_values {
		size_t stringLength{ std::numeric_limits<size_t>::max() };
		size_t maxSizeIndex{ 0 };
		bool success{};
		size_t seed{};
	};

	template<const auto& tuple, size_t I> constexpr jsonifier::string_view getKey() noexcept {
		constexpr auto first = std::get<I>(tuple).view();
		using T0 = unwrap_t<decltype(first)>;
		if constexpr (std::is_member_pointer_v<T0>) {
			return getName<first>();
		}
		return static_cast<jsonifier::string_view>(first);
	}

	template<const auto& tuple, size_t maxIndex, size_t index = 0> JSONIFIER_INLINE constexpr auto keyStatsInternal(key_stats_t stats) {
		if constexpr (index < maxIndex) {
			constexpr jsonifier::string_view key{ getKey<tuple, index>() };
			constexpr auto n{ key.size() };
			if (n < stats.minLength) {
				stats.minLength = n;
			}
			if (n > stats.maxLength) {
				stats.maxLength = n;
			}
			return keyStatsInternal<tuple, maxIndex, index + 1>(stats);
		}
		else {
			if constexpr (maxIndex > 0) {
				stats.lengthRange = stats.maxLength - stats.minLength;
			}
			return stats;
		}
	}

	template<const auto& tuple> JSONIFIER_INLINE constexpr auto keyStats() {
		constexpr auto N{ std::tuple_size_v<unwrap_t<decltype(tuple)>> };

		return keyStatsInternal<tuple, N, 0>(key_stats_t{});
	}

	template<size_t length> struct tuple_simd {
		using type = std::conditional_t<length >= 64 && bytesPerStep >= 64, jsonifier_simd_int_512, std::conditional_t<length >= 32 && bytesPerStep >= 32, jsonifier_simd_int_256, jsonifier_simd_int_128>>;
	};

	template<size_t length> using tuple_simd_t = tuple_simd<length>::type;

	template<const auto& tuple, size_t index> JSONIFIER_INLINE bool compareStringFunctionNonConst(const char* string01) {
		static constexpr auto currentKey = getKey<tuple, index>();
		return compare<currentKey.size()>(currentKey.data(), string01);
	}

	template<const auto& tuple, size_t index> JSONIFIER_INLINE constexpr bool compareStringFunctionConst(const char* string01) {
		constexpr auto currentKey = getKey<tuple, index>();
		return currentKey == jsonifier::string_view{ string01, currentKey.size() };
	}

	template<const auto& tuple, size_t index> using compare_string_function_non_const_tuple_ptr = decltype(&compareStringFunctionNonConst<tuple, index>);

	template<const auto& tuple, size_t index> using compare_string_function_const_tuple_ptr = decltype(&compareStringFunctionConst<tuple, index>);

	template<const auto& tuple, size_t... indices> JSONIFIER_INLINE constexpr auto generateConstCompareStringFunctionPtrArrayInternal(std::index_sequence<indices...>) {
		return std::array<compare_string_function_const_tuple_ptr<tuple, 0>, sizeof...(indices)>{ { &compareStringFunctionConst<tuple, indices>... } };
	}

	template<const auto& tuple> JSONIFIER_INLINE constexpr auto generateConstCompareStringFunctionPtrArray() {
		constexpr auto tupleSize = std::tuple_size_v<unwrap_t<decltype(tuple)>>;
		return generateConstCompareStringFunctionPtrArrayInternal<tuple>(std::make_index_sequence<tupleSize>{});
	}

	template<const auto& tuple, size_t... indices> JSONIFIER_INLINE constexpr auto generateNonConstCompareStringFunctionPtrArrayInternal(std::index_sequence<indices...>) {
		return std::array<compare_string_function_non_const_tuple_ptr<tuple, 0>, sizeof...(indices)>{ { &compareStringFunctionNonConst<tuple, indices>... } };
	}

	template<const auto& tuple> JSONIFIER_INLINE constexpr auto generateNonConstCompareStringFunctionPtrArray() {
		constexpr auto tupleSize = std::tuple_size_v<unwrap_t<decltype(tuple)>>;
		return generateNonConstCompareStringFunctionPtrArrayInternal<tuple>(std::make_index_sequence<tupleSize>{});
	}

	template<typename value_type, typename key_type_new, const auto& tupleNew, size_t actualCount, size_t storageSizeNew> struct simd_hash_tuple : public key_hasher {
		using simd_type = map_simd_t<storageSizeNew>;
		using key_type = key_type_new;
		using size_type = size_t;
		using control_type = uint8_t;
		static constexpr size_type storageSize{ storageSizeNew };
		static constexpr size_type bucketSize = setSimdWidth<storageSize>();
		static constexpr auto tuple{ tupleNew };
		static constexpr auto nonConstCompareStringFunctions{ generateNonConstCompareStringFunctionPtrArray<tuple>() };
		static constexpr auto constCompareStringFunctions{ generateConstCompareStringFunctionPtrArray<tuple>() };
		static constexpr size_type numGroups = storageSize > bucketSize ? storageSize / bucketSize : 1;
		JSONIFIER_ALIGN std::array<size_t, storageSize + 1> items{};
		JSONIFIER_ALIGN control_type controlBytes[storageSize]{};
		JSONIFIER_ALIGN size_type stringLength{};

		constexpr simd_hash_tuple() noexcept = default;

		template<const auto& functionPtrs> JSONIFIER_INLINE constexpr auto find(const char* iter, size_type keyLength) const noexcept {
			if (!std::is_constant_evaluated()) {
				JSONIFIER_ALIGN const auto keySize = keyLength > stringLength ? stringLength : keyLength;
				JSONIFIER_ALIGN const auto hash = hashKeyRt(iter, keySize);
				JSONIFIER_ALIGN const auto resultIndex = ((hash >> 8) % numGroups) * bucketSize;
				JSONIFIER_ALIGN const auto finalIndex = (simd_internal::tzcnt(simd_internal::opCmpEq(simd_internal::gatherValue<simd_type>(static_cast<control_type>(hash)),
					simd_internal::gatherValues<simd_type>(controlBytes + resultIndex))) +
					resultIndex);
				if (nonConstCompareStringFunctions[items[finalIndex]](iter)) {
					return functionPtrs.data() + items[finalIndex];
				}
				else {
					return functionPtrs.data() + functionPtrs.size();
				}
			}
			else {
				JSONIFIER_ALIGN const auto keySize = keyLength > stringLength ? stringLength : keyLength;
				JSONIFIER_ALIGN const auto hash = hashKeyCt(iter, keySize);
				JSONIFIER_ALIGN const auto resultIndex = ((hash >> 8) % numGroups) * bucketSize;
				JSONIFIER_ALIGN const auto finalIndex = (constMatch(controlBytes + resultIndex, static_cast<control_type>(hash)) + resultIndex);
				if (constCompareStringFunctions[items[finalIndex]](iter)) {
					return functionPtrs.data() + items[finalIndex];
				}
				else {
					return functionPtrs.data() + functionPtrs.size();
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
			for (size_type x = 0; x < bucketSize; ++x) {
				if (hashData[x] == hash) {
					mask |= (1ull << x);
				}
			}
			return tzcnt(mask);
		}
	};

	template<size_t startingValue, size_t actualCount, typename value_type, typename key_type, const auto& tuple, template<typename, typename, const auto&, size_t, size_t> typename map_type>
	using tuple_variant = std::variant<map_type<value_type, key_type, tuple, actualCount, startingValue>, map_type<value_type, key_type, tuple, actualCount, startingValue * 2>,
		map_type<value_type, key_type, tuple, actualCount, startingValue * 4>, map_type<value_type, key_type, tuple, actualCount, startingValue * 8>,
		map_type<value_type, key_type, tuple, actualCount, startingValue * 16>>;

	template<typename value_type, typename key_type, const auto& tuple, size_t actualCount, size_t storageSize>
	constexpr auto constructSimdHashSubTupleFinal(const std::array<key_type, actualCount>& pairsNew, map_construction_values constructionValues)
		-> tuple_variant<16, actualCount, value_type, key_type, tuple, simd_hash_tuple> {
		constexpr size_t bucketSize = setSimdWidth<storageSize>();
		constexpr size_t numGroups = storageSize > bucketSize ? storageSize / bucketSize : 1;
		simd_hash_tuple<value_type, key_type, tuple, actualCount, storageSize> simdHashSubTupleNew{};
		simdHashSubTupleNew.setSeedCt(constructionValues.seed);
		simdHashSubTupleNew.stringLength = constructionValues.stringLength;
		std::array<size_t, numGroups> bucketSizes{};
		for (size_t x = 0; x < actualCount; ++x) {
			const auto keySize = pairsNew[x].size() > simdHashSubTupleNew.stringLength ? simdHashSubTupleNew.stringLength : pairsNew[x].size();
			const auto hash = simdHashSubTupleNew.hashKeyCt(pairsNew[x].data(), keySize);
			const auto groupPos = (hash >> 8) % numGroups;
			const auto ctrlByte = static_cast<uint8_t>(hash);
			const auto bucketSizeNew = ++bucketSizes[groupPos];
			const auto slot = ((groupPos * bucketSize) + bucketSizeNew);
			simdHashSubTupleNew.items[slot] = x;
			simdHashSubTupleNew.controlBytes[slot] = ctrlByte;
		}
		return tuple_variant<16, actualCount, value_type, key_type, tuple, simd_hash_tuple>{ simd_hash_tuple<value_type, key_type, tuple, actualCount, storageSize>(simdHashSubTupleNew) };
	}

	template<typename value_type, typename key_type, const auto& tuple, size_t actualCount> using construct_simd_hash_sub_tuple_function_ptr =
		decltype(&constructSimdHashSubTupleFinal<value_type, key_type, tuple, actualCount, 16ull>);

	template<typename value_type, typename key_type, const auto& tuple, size_t actualCount>
	constexpr construct_simd_hash_sub_tuple_function_ptr<value_type, key_type, tuple, actualCount> constructSimdHashSubTupleFinalPtrs[5] = {
		&constructSimdHashSubTupleFinal<value_type, key_type, tuple, actualCount, 16ull>, &constructSimdHashSubTupleFinal<value_type, key_type, tuple, actualCount, 32ull>,
		&constructSimdHashSubTupleFinal<value_type, key_type, tuple, actualCount, 64ull>, &constructSimdHashSubTupleFinal<value_type, key_type, tuple, actualCount, 128ull>,
		&constructSimdHashSubTupleFinal<value_type, key_type, tuple, actualCount, 256ull>
	};

	template<size_t maxSizeIndex, typename value_type, typename key_type, const auto& tuple, size_t actualCount>
	JSONIFIER_INLINE constexpr auto constructSimdHashSubTupleInternal(const std::array<key_type, actualCount>& pairsNew, xoshiro256 prng, key_stats_t& keyStatsVal) {
		auto constructForGivenStringLength = [&](size_t stringLength, auto&& constructForGivenStringLength, const auto maxSizeIndexNew) mutable -> map_construction_values {
			constexpr size_t bucketSize = setSimdWidth<hashTupleMaxSizes[maxSizeIndexNew]>();
			constexpr size_t storageSize = hashTupleMaxSizes[maxSizeIndexNew];
			constexpr size_t numGroups = storageSize > bucketSize ? storageSize / bucketSize : 1;
			std::array<uint8_t, storageSize> controlBytes{};
			std::array<size_t, numGroups> bucketSizes{};
			std::array<size_t, storageSize> slots{};
			key_hasher hasherNew{};
			bool collided{};
			size_t seed{};

			for (size_t x = 0; x < 2; ++x) {
				seed = prng();
				hasherNew.setSeedCt(seed);
				collided = false;
				std::fill(slots.begin(), slots.end(), std::numeric_limits<size_t>::max());

				for (size_t y = 0; y < actualCount; ++y) {
					const auto keySize = pairsNew[y].size() > stringLength ? stringLength : pairsNew[y].size();
					const auto hash = hasherNew.hashKeyCt(pairsNew[y].data(), keySize);
					const auto groupPos = (hash >> 8) % numGroups;
					const auto ctrlByte = static_cast<uint8_t>(hash);
					const auto bucketSizeNew = ++bucketSizes[groupPos];
					const auto slot = ((groupPos * bucketSize) + bucketSizeNew);

					if (bucketSizeNew >= bucketSize || contains(slots.data() + groupPos * bucketSize, slot, bucketSize) ||
						contains(controlBytes.data() + groupPos * bucketSize, ctrlByte, bucketSize)) {
						collided = true;
						break;
					}

					controlBytes[slot] = ctrlByte;
					slots[y] = slot;
				}
				if (!collided) {
					break;
				}
				std::fill(controlBytes.begin(), controlBytes.end(), 0);
				std::fill(bucketSizes.begin(), bucketSizes.end(), 0);
			}
			map_construction_values returnValues{};
			if (collided) {
				if constexpr (maxSizeIndexNew < std::size(hashTupleMaxSizes) - 1) {
					return constructForGivenStringLength(keyStatsVal.minLength, constructForGivenStringLength, std::integral_constant<size_t, maxSizeIndexNew + 1>{});
				}
				else if (stringLength < keyStatsVal.maxLength - 1) {
					return constructForGivenStringLength(stringLength + 1, constructForGivenStringLength, std::integral_constant<size_t, maxSizeIndexNew>{});
				}
				returnValues.success = false;
				return returnValues;
			}
			returnValues.maxSizeIndex = maxSizeIndexNew;
			returnValues.stringLength = stringLength;
			returnValues.seed = seed;
			returnValues.success = true;
			return returnValues;
			};

		map_construction_values bestValues{ constructForGivenStringLength(keyStatsVal.minLength, constructForGivenStringLength, std::integral_constant<size_t, 0>{}) };

		return bestValues;
	}

	template<typename value_type, typename key_type, const auto& tuple, size_t actualCount> JSONIFIER_INLINE constexpr auto constructSimdHashSubTuple(const std::array<key_type, actualCount>& pairsNew) {
		key_stats_t keyStatsVal = keyStats<tuple>();
		auto constructionValues =
			constructSimdHashSubTupleInternal<getMaxSizeIndex<actualCount>(hashTupleMaxSizes), value_type, key_type, tuple, actualCount>(pairsNew, xoshiro256{}, keyStatsVal);
		return constructSimdHashSubTupleFinalPtrs<value_type, key_type, tuple, actualCount>[constructionValues.maxSizeIndex](pairsNew, constructionValues);
	}

	template<typename value_type, typename key_type_new, const auto& tupleNew, size_t actualCount, size_t storageSizeNew> struct minimal_char_hash_sub_tuple : public key_hasher {
		using key_type = key_type_new;
		using size_type = size_t;
		static constexpr auto tuple{ tupleNew };
		static constexpr auto nonConstCompareStringFunctions{ generateNonConstCompareStringFunctionPtrArray<tuple>() };
		static constexpr auto constCompareStringFunctions{ generateConstCompareStringFunctionPtrArray<tuple>() };
		JSONIFIER_ALIGN std::array<size_t, storageSizeNew> items{};
		JSONIFIER_ALIGN size_t stringLength{};

		constexpr minimal_char_hash_sub_tuple() noexcept = default;

		template<const auto& functionPtrs> JSONIFIER_INLINE constexpr auto find(const char* iter, size_type keyLength) const noexcept {
			JSONIFIER_ALIGN const auto stringLengthNew = keyLength > stringLength ? stringLength : keyLength;
			JSONIFIER_ALIGN const auto hash = seed * (operator uint64_t() ^ iter[0]) + iter[stringLengthNew - 1];
			JSONIFIER_ALIGN const auto finalIndex = hash % storageSizeNew;
			if (!std::is_constant_evaluated()) {
				if (nonConstCompareStringFunctions[items[finalIndex]](iter)) {
					return functionPtrs.data() + items[finalIndex];
				}
				else {
					return functionPtrs.data() + functionPtrs.size();
				}
			}
			else {
				if (constCompareStringFunctions[items[finalIndex]](iter)) {
					return functionPtrs.data() + items[finalIndex];
				}
				else {
					return functionPtrs.data() + functionPtrs.size();
				}
			}
		}
	};

	template<size_t startingValue, size_t actualCount, typename value_type, typename key_type, const auto& tuple, template<typename, typename, const auto&, size_t, size_t> typename tuple_type>
	using tuple_variant = std::variant<tuple_type<value_type, key_type, tuple, actualCount, startingValue>, tuple_type<value_type, key_type, tuple, actualCount, startingValue * 2>,
		tuple_type<value_type, key_type, tuple, actualCount, startingValue * 4>, tuple_type<value_type, key_type, tuple, actualCount, startingValue * 8>,
		tuple_type<value_type, key_type, tuple, actualCount, startingValue * 16>>;

	template<typename value_type, typename key_type, const auto& tuple, size_t actualCount, size_t storageSize>
	JSONIFIER_INLINE constexpr auto constructMinimalCharHashSubTupleFinal(const std::array<key_type, actualCount>& pairsNew, tuple_construction_values constructionValues)
		-> tuple_variant<16ull, actualCount, value_type, key_type, tuple, minimal_char_hash_sub_tuple> {
		minimal_char_hash_sub_tuple<value_type, key_type, tuple, actualCount, storageSize> minimalCharHashSubTupleNew{};
		minimalCharHashSubTupleNew.stringLength = constructionValues.stringLength;
		minimalCharHashSubTupleNew.setSeedCt(constructionValues.seed);
		for (size_t x = 0; x < actualCount; ++x) {
			const auto keySize = pairsNew[x].size();
			const auto stringLengthNew = keySize > minimalCharHashSubTupleNew.stringLength ? minimalCharHashSubTupleNew.stringLength : keySize;
			const auto hash = constructionValues.seed * (minimalCharHashSubTupleNew.operator uint64_t() ^ pairsNew[x].data()[0]) + pairsNew[x].data()[stringLengthNew - 1];
			const auto slot = hash % storageSize;
			minimalCharHashSubTupleNew.items[slot] = x;
		}
		return tuple_variant<16ull, actualCount, value_type, key_type, tuple, minimal_char_hash_sub_tuple>{ minimal_char_hash_sub_tuple<value_type, key_type, tuple, actualCount, storageSize>{
			minimalCharHashSubTupleNew } };
	}

	template<template<typename, typename, const auto&, size_t> typename hash_wrapper, typename value_type, typename key_type, const auto& tuple, std::size_t... indices> JSONIFIER_INLINE constexpr auto generateArrayOfFunctionPtrs(std::index_sequence<indices...>) {
		return std::array<decltype(&hash_wrapper<value_type, key_type, tuple, 0>::op), sizeof...(indices)>{ &hash_wrapper<value_type, key_type, tuple, indices>::op... };
	}

	template<typename value_type, typename key_type, const auto& tuple, size_t actualCount> using construct_minimal_char_hash_sub_tuple_function_ptr =
		decltype(&constructMinimalCharHashSubTupleFinal<value_type, key_type, tuple, actualCount, 16ull>);

	template<typename value_type, typename key_type, const auto& tuple, size_t actualCount>
	constexpr construct_minimal_char_hash_sub_tuple_function_ptr<value_type, key_type, tuple, actualCount> constructMinimalCharHashSubTupleFinalPtrs[5] = {
		&constructMinimalCharHashSubTupleFinal<value_type, key_type, tuple, actualCount, 16ull>, &constructMinimalCharHashSubTupleFinal<value_type, key_type, tuple, actualCount, 32ull>,
		&constructMinimalCharHashSubTupleFinal<value_type, key_type, tuple, actualCount, 64ull>, &constructMinimalCharHashSubTupleFinal<value_type, key_type, tuple, actualCount, 128ull>,
		&constructMinimalCharHashSubTupleFinal<value_type, key_type, tuple, actualCount, 256ull>
	};

	template<size_t maxSizeIndex, typename value_type, typename key_type, const auto& tuple, size_t actualCount>
	JSONIFIER_INLINE constexpr auto constructMinimalCharHashSubTupleInternal(const std::array<key_type, actualCount>& pairsNew, xoshiro256 prng, key_stats_t& keyStatsVal) {
		auto constructForGivenStringLength = [&](size_t stringLength, auto&& constructForGivenStringLength, const auto maxSizeIndexNew) mutable -> tuple_construction_values {
			constexpr size_t storageSize = hashTupleMaxSizes[maxSizeIndexNew];
			size_t slots[storageSize]{};
			key_hasher hasherNew{};
			bool collided{};
			size_t seed{};
			std::fill(slots, slots + storageSize, std::numeric_limits<size_t>::max());
			for (size_t x = 0; x < 2; ++x) {
				seed = prng();
				hasherNew.setSeedCt(seed);
				for (size_t y = 0; y < actualCount; ++y) {
					collided = false;
					const auto keySize = pairsNew[y].size();
					const auto stringLengthNew = keySize > stringLength ? stringLength : keySize;
					const auto hash = seed * (hasherNew.operator uint64_t() ^ pairsNew[y].data()[0]) + pairsNew[y].data()[stringLengthNew - 1];
					const auto slot = hash % storageSize;

					if (contains(slots, slot, storageSize)) {
						std::fill(slots, slots + storageSize, std::numeric_limits<size_t>::max());
						collided = true;
						break;
					}
					slots[y] = slot;
				}
				if (!collided) {
					break;
				}
			}
			tuple_construction_values returnValues{};
			if (collided) {
				if constexpr (maxSizeIndexNew < std::size(hashTupleMaxSizes) - 1) {
					return constructForGivenStringLength(keyStatsVal.minLength, constructForGivenStringLength, std::integral_constant<size_t, maxSizeIndexNew + 1>{});
				}
				else if (stringLength < keyStatsVal.maxLength - 1) {
					return constructForGivenStringLength(stringLength + 1, constructForGivenStringLength, std::integral_constant<size_t, maxSizeIndexNew>{});
				}
				returnValues.success = false;
				return returnValues;
			}
			returnValues.maxSizeIndex = maxSizeIndexNew;
			returnValues.stringLength = stringLength;
			returnValues.seed = seed;
			returnValues.success = true;
			return returnValues;
			};

		tuple_construction_values bestValues{ constructForGivenStringLength(keyStatsVal.minLength, constructForGivenStringLength, std::integral_constant<size_t, 0>{}) };

		return bestValues;
	}

	template<typename value_type, typename key_type, const auto& tuple, size_t actualCount> JSONIFIER_INLINE constexpr auto constructMinimalCharHashSubTuple(const std::array<key_type, actualCount>& pairsNew) {
		auto keyStatsVal = keyStats<tuple>();
		auto constructionValues =
			constructMinimalCharHashSubTupleInternal<getMaxSizeIndex<actualCount>(hashTupleMaxSizes), value_type, key_type, tuple, actualCount>(pairsNew, xoshiro256{}, keyStatsVal);
		return constructMinimalCharHashSubTupleFinalPtrs<value_type, key_type, tuple, actualCount>[constructionValues.maxSizeIndex](pairsNew, constructionValues);
	}

	template<typename value_type, const auto& tuple, size_t... I> JSONIFIER_INLINE constexpr auto makeSubTupleImpl(std::index_sequence<I...>) {
		constexpr auto n = std::tuple_size_v<unwrap_t<decltype(tuple)>>;

		if constexpr (n == 0) {
			return nullptr;
		}
		else if constexpr (n < 16) {
			constexpr auto tupleNew = constructMinimalCharHashSubTuple<value_type, jsonifier::string_view, tuple, n>({ getKey<tuple, I>()... });
			constexpr auto newIndex = tupleNew.index();
			constexpr auto newSubTuple = std::get<newIndex>(tupleNew);
			if constexpr (newSubTuple.stringLength == std::numeric_limits<size_t>::max()) {
				constexpr auto mapNewer = constructSimdHashSubTuple<value_type, jsonifier::string_view, tuple, n>({ getKey<tuple, I>()... });
				constexpr auto newIndexer = mapNewer.index();
				return std::get<newIndexer>(mapNewer);
			}
			else {
				return newSubTuple;
			}
		}
		else {
			constexpr auto mapNew = constructSimdHashSubTuple<value_type, jsonifier::string_view, tuple, n>({ getKey<tuple, I>()... });
			constexpr auto newIndex = mapNew.index();
			constexpr auto newSubTuple = std::get<newIndex>(mapNew);
			return newSubTuple;
		}
	}

	template<const auto& function, typename value_type, size_t stringLength, typename... arg_types> JSONIFIER_INLINE constexpr auto collectSubTuple(arg_types&&...args);

	template<typename value_type, const auto& tuple> JSONIFIER_INLINE constexpr auto makeSubTuple() {
		return makeSubTupleImpl<value_type, tuple>(std::make_index_sequence<std::tuple_size_v<unwrap_t<decltype(tuple)>>>{});
	}

	template<typename value_type, const auto& tuple, std::size_t currentIndex> JSONIFIER_INLINE constexpr auto makeHashTupleInternal() {
		constexpr auto& newTuple = std::get<currentIndex>(tuple);
		return std::make_tuple(makeSubTuple<value_type, newTuple>());
	}

	template<typename value_type, const auto& tuple, std::size_t... indices> JSONIFIER_INLINE constexpr auto makeHashTupleInternal(std::index_sequence<indices...>) {
		return std::tuple_cat(makeHashTupleInternal<value_type, tuple, indices>()...);
	}

	template<typename value_type> struct tuple_construction_data {
		static constexpr auto& tuple = jsonifier::concepts::coreV<value_type>;
		static constexpr auto arrayOfRefs = collectTupleElementReferences(tuple);
		static constexpr auto sortedRefs = sortTupleElementReferences(arrayOfRefs);
		static constexpr auto stringLengthCount = countUniqueStringLengths<sortedRefs>();
		static constexpr auto stringLengths = countUniqueStringLengths<stringLengthCount, sortedRefs>();
		static constexpr auto intermediateTuple = constructNewTuple<stringLengths, sortedRefs, tuple>();
		static constexpr auto finalTuple = makeHashTupleInternal<value_type, intermediateTuple>(std::make_index_sequence<stringLengths.size()>{});
	};

	template<const auto& function, typename value_type, size_t stringLength, typename... arg_types> JSONIFIER_INLINE constexpr auto collectSubTuple(arg_types&&...args) {
		using tuple_type = typename std::decay_t<decltype(tuple_construction_data<value_type>::finalTuple)>;

		constexpr size_t index = getCurrentSubTupleIndex<stringLength, tuple_construction_data<value_type>::stringLengths>();

		if constexpr (index < std::tuple_size_v<tuple_type>) {
			return function(std::integral_constant<size_t, index>{}, std::forward<arg_types>(args)...);
		}
		else {
			return false;
		}
	};

	template<const auto& function, typename value_type, size_t index, typename... arg_types> using collect_sub_tuple_ptr = decltype(&collectSubTuple<function, value_type, index, arg_types...>);

	template<const auto& function, typename value_type, typename... arg_types, size_t... indices> JSONIFIER_INLINE constexpr auto generateCollectSubTuplePtrArrayInternal(std::index_sequence<indices...>) {
		return std::array<collect_sub_tuple_ptr<function, value_type, 0, arg_types...>, sizeof...(indices)>{ { &collectSubTuple<function, value_type, indices, arg_types...>... } };
	}

	template<const auto& function, typename value_type, typename... arg_types> JSONIFIER_INLINE constexpr auto generateCollectSubTuplePtrArray() {
		return generateCollectSubTuplePtrArrayInternal<function, value_type, arg_types...>(std::make_index_sequence<512>{});
	}

	template<typename value_type> struct hash_tuple {
		using size_type = size_t;

		constexpr hash_tuple() noexcept {};

		template<const auto& function, typename... arg_types>   JSONIFIER_INLINE constexpr auto find(size_type stringLength, arg_types&&... args) const {
			constexpr auto getSubTuplePtrArray = generateCollectSubTuplePtrArray<function, value_type, arg_types...>();
			return getSubTuplePtrArray[stringLength](std::forward<arg_types>(args)...);
		}
	};

	template<typename value_type> JSONIFIER_INLINE constexpr auto makeHashTuple() {
		return hash_tuple<value_type>{};
	}

}