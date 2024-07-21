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

	struct string_length_counts {
		uint64_t length{};
		uint64_t count{};
	};

	struct tuple_element_reference {
		jsonifier::string_view key{};
		uint64_t tupleIndex{};
	};

	template<size_t index, typename value_type> constexpr tuple_element_reference extract(const value_type& t) {
		return { std::get<0>(t), index };
	}

	template<typename tuple_type, size_t... I> constexpr auto collectTupleElementReferencesImpl(const tuple_type& t, std::index_sequence<I...>) {
		return std::array<tuple_element_reference, sizeof...(I)>{ extract<I>(std::get<I>(t))... };
	}

	template<typename tuple_type> constexpr auto collectTupleElementReferences(const tuple_type& t) {
		return collectTupleElementReferencesImpl(t, std::make_index_sequence<std::tuple_size_v<tuple_type>>{});
	}

	template<uint64_t size> constexpr auto sortTupleElementReferences(const std::array<tuple_element_reference, size>& values) {
		std::array<tuple_element_reference, size> newValues{ values };
		std::sort(newValues.begin(), newValues.end(), [](auto lhs, auto rhs) {
			return lhs.key.size() < rhs.key.size();
		});
		return newValues;
	}

	template<const auto& ArrayOfReferences> constexpr std::size_t getMaxLength() {
		std::size_t maxLen = 0;
		for (const auto& ref: ArrayOfReferences) {
			if (get_length(ref.key) > maxLen) {
				maxLen = get_length(ref.key);
			}
		}
		return maxLen;
	}

	template<const auto& arr> constexpr std::size_t countUniqueStringLengths() {
		uint64_t count{};
		uint64_t currentLength{};
		for (uint64_t x = 0; x < arr.size(); ++x) {
			if (currentLength != arr[x].key.size()) {
				currentLength = arr[x].key.size();
				++count;
			}
		}
		return count;
	}

	template<std::size_t counts, const auto& arr> constexpr auto countUniqueStringLengths() {
		std::array<string_length_counts, counts> returnValues{};
		uint64_t currentIndex	  = 0;
		std::size_t currentLength = static_cast<std::size_t>(-1);

		for (std::size_t x = 0; x < arr.size(); ++x) {
			auto keyLength = arr[x].key.size();
			if (currentLength != keyLength) {
				currentLength					  = keyLength;
				returnValues[currentIndex].length = keyLength;
				returnValues[currentIndex].count  = 1;
				++currentIndex;
			} else {
				++returnValues[currentIndex - 1].count;
			}
		}

		return returnValues;
	}

	template<uint64_t inputIndex, const auto& stringLengths> constexpr auto getCurrentSubTupleIndex() {
		for (uint64_t x = 1; x < stringLengths.size(); ++x) {
			if (inputIndex == stringLengths[x].length) {
				return x;
			}
		}
		return std::numeric_limits<uint64_t>::max();
	}

	template<uint64_t inputIndex, const auto& stringLengths> constexpr auto getCurrentStartingIndex() {
		uint64_t currentStartingIndex{};
		for (uint64_t x = 0; x < inputIndex; ++x) {
			currentStartingIndex += stringLengths[x].count;
		}
		return currentStartingIndex;
	}

	template<const auto& stringLengths, const auto tupleReferences, const auto& tuple, uint64_t startIndex, uint64_t... indices>
	constexpr auto constructNewSubTupleInternal(std::index_sequence<indices...>) {
		return std::tuple_cat(std::make_tuple(std::get<tupleReferences[startIndex + indices].tupleIndex>(tuple))...);
	}

	template<const auto& stringLengths, const auto tupleReferences, const auto& tuple, uint64_t startIndex, uint64_t maxIndex> constexpr auto constructNewSubTuple() {
		return std::make_tuple(constructNewSubTupleInternal<stringLengths, tupleReferences, tuple, startIndex>(std::make_index_sequence<maxIndex - startIndex>{}));
	}

	template<const auto& stringLengths, const auto& tupleReferences, const auto& tuple, std::size_t... indices>
	constexpr auto constructNewTupleInternal(std::index_sequence<indices...>) {
		return std::tuple_cat(constructNewSubTuple<stringLengths, tupleReferences, tuple, getCurrentStartingIndex<indices, stringLengths>(),
			getCurrentStartingIndex<indices + 1, stringLengths>()>()...);
	}

	template<const auto& stringLengths, const auto tupleReferences, const auto& tuple> constexpr auto constructNewTuple() {
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
		constexpr auto& first = std::get<0>(std::get<I>(tuple));
		using T0			  = unwrap_t<decltype(first)>;
		if constexpr (std::is_member_pointer_v<T0>) {
			return getName<first>();
		}
		return static_cast<jsonifier::string_view>(first);
	}

	template<const auto& tuple, uint64_t maxIndex, uint64_t index = 0> JSONIFIER_INLINE constexpr auto keyStatsInternal(key_stats_t stats) {
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
		} else {
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
		using type = std::conditional_t<length >= 64 && bytesPerStep >= 64, simd_int_512, std::conditional_t<length >= 32 && bytesPerStep >= 32, simd_int_256, simd_int_128>>;
	};

	template<size_t length> using tuple_simd_t = tuple_simd<length>::type;

	template<const auto& tuple, size_t index> JSONIFIER_INLINE bool compareStringFunctionNonConst(const char* string01) {
		static constexpr auto currentKey = getKey<tuple, index>();
		return compare<currentKey.size()>(currentKey.data(), string01);
	}

	template<const auto& tuple, size_t index> constexpr bool compareStringFunctionConst(const char* string01) {
		constexpr auto currentKey = getKey<tuple, index>();
		return currentKey == jsonifier::string_view{ string01, currentKey.size() };
	}

	template<const auto& tuple, size_t index> using compare_string_function_non_const_tuple_ptr = decltype(&compareStringFunctionNonConst<tuple, index>);

	template<const auto& tuple, size_t index> using compare_string_function_const_tuple_ptr = decltype(&compareStringFunctionConst<tuple, index>);

	template<const auto& tuple, size_t... indices> constexpr auto generateConstCompareStringFunctionPtrArrayInternal(std::index_sequence<indices...>) {
		return std::array<compare_string_function_const_tuple_ptr<tuple, 0>, sizeof...(indices)>{ { &compareStringFunctionConst<tuple, indices>... } };
	}

	template<const auto& tuple> constexpr auto generateConstCompareStringFunctionPtrArray() {
		constexpr auto tupleSize = std::tuple_size_v<unwrap_t<decltype(tuple)>>;
		return generateConstCompareStringFunctionPtrArrayInternal<tuple>(std::make_index_sequence<tupleSize>{});
	}

	template<const auto& tuple, size_t... indices> constexpr auto generateNonConstCompareStringFunctionPtrArrayInternal(std::index_sequence<indices...>) {
		return std::array<compare_string_function_non_const_tuple_ptr<tuple, 0>, sizeof...(indices)>{ { &compareStringFunctionNonConst<tuple, indices>... } };
	}

	template<const auto& tuple> constexpr auto generateNonConstCompareStringFunctionPtrArray() {
		constexpr auto tupleSize = std::tuple_size_v<unwrap_t<decltype(tuple)>>;
		return generateNonConstCompareStringFunctionPtrArrayInternal<tuple>(std::make_index_sequence<tupleSize>{});
	}

	struct hash_tuple_base {
		bool isItTrue{ false };
		constexpr operator bool() {
			return isItTrue;
		}

		template<typename... arg_types> constexpr auto find(arg_types&&... args) const {
			return nullptr;
		}
	};

	template<typename key_type_new, const auto& tupleNew, size_t actualCount, size_t storageSizeNew> struct simd_hash_tuple : public key_hasher, public hash_tuple_base {
		using simd_type	   = map_simd_t<storageSizeNew>;
		using key_type	   = key_type_new;
		using size_type	   = size_t;
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

		constexpr simd_hash_tuple() noexcept {
			isItTrue = true;
		};

		template<typename function_type> JSONIFIER_INLINE constexpr auto find(const char* iter, size_type keyLength, function_type& functionPtrs) const noexcept {
			if (!std::is_constant_evaluated()) {
				JSONIFIER_ALIGN const auto keySize	   = keyLength > stringLength ? stringLength : keyLength;
				JSONIFIER_ALIGN const auto hash		   = hashKeyRt(iter, keySize);
				JSONIFIER_ALIGN const auto resultIndex = ((hash >> 8) % numGroups) * bucketSize;
				JSONIFIER_ALIGN const auto finalIndex  = (simd_internal::tzcnt(simd_internal::opCmpEq(simd_internal::gatherValue<simd_type>(static_cast<control_type>(hash)),
															  simd_internal::gatherValues<simd_type>(controlBytes + resultIndex))) +
					 resultIndex);
				if (nonConstCompareStringFunctions[items[finalIndex]](iter)) {
					return functionPtrs.data() + items[finalIndex];
				} else {
					return functionPtrs.data() + functionPtrs.size();
				}
			} else {
				JSONIFIER_ALIGN const auto keySize	   = keyLength > stringLength ? stringLength : keyLength;
				JSONIFIER_ALIGN const auto hash		   = hashKeyCt(iter, keySize);
				JSONIFIER_ALIGN const auto resultIndex = ((hash >> 8) % numGroups) * bucketSize;
				JSONIFIER_ALIGN const auto finalIndex  = (constMatch(controlBytes + resultIndex, static_cast<control_type>(hash)) + resultIndex);
				if (constCompareStringFunctions[items[finalIndex]](iter)) {
					return functionPtrs.data() + items[finalIndex];
				} else {
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

	template<size_t startingValue, size_t actualCount, typename key_type, const auto& tuple, template<typename, const auto&, size_t, size_t> typename map_type>
	using tuple_variant = std::variant<map_type<key_type, tuple, actualCount, startingValue>, map_type<key_type, tuple, actualCount, startingValue * 2>,
		map_type<key_type, tuple, actualCount, startingValue * 4>, map_type<key_type, tuple, actualCount, startingValue * 8>,
		map_type<key_type, tuple, actualCount, startingValue * 16>>;

	template<typename key_type, const auto& tuple, size_t actualCount, size_t storageSize>
	constexpr auto constructSimdHashSubTupleFinal(const std::array<key_type, actualCount>& pairsNew, map_construction_values constructionValues)
		-> tuple_variant<16, actualCount, key_type, tuple, simd_hash_tuple> {
		constexpr size_t bucketSize = setSimdWidth<storageSize>();
		constexpr size_t numGroups	= storageSize > bucketSize ? storageSize / bucketSize : 1;
		simd_hash_tuple<key_type, tuple, actualCount, storageSize> simdHashSubTupleNew{};
		simdHashSubTupleNew.setSeedCt(constructionValues.seed);
		simdHashSubTupleNew.stringLength = constructionValues.stringLength;
		std::array<size_t, numGroups> bucketSizes{};
		for (size_t x = 0; x < actualCount; ++x) {
			const auto keySize					   = pairsNew[x].size() > simdHashSubTupleNew.stringLength ? simdHashSubTupleNew.stringLength : pairsNew[x].size();
			const auto hash						   = simdHashSubTupleNew.hashKeyCt(pairsNew[x].data(), keySize);
			const auto groupPos					   = (hash >> 8) % numGroups;
			const auto ctrlByte					   = static_cast<uint8_t>(hash);
			const auto bucketSizeNew			   = ++bucketSizes[groupPos];
			const auto slot						   = ((groupPos * bucketSize) + bucketSizeNew);
			simdHashSubTupleNew.items[slot]		   = x;
			simdHashSubTupleNew.controlBytes[slot] = ctrlByte;
		}
		return tuple_variant<16, actualCount, key_type, tuple, simd_hash_tuple>{ simd_hash_tuple<key_type, tuple, actualCount, storageSize>(simdHashSubTupleNew) };
	}

	template<typename key_type, const auto& tuple, size_t actualCount> using construct_simd_hash_sub_tuple_function_ptr =
		decltype(&constructSimdHashSubTupleFinal<key_type, tuple, actualCount, 16ull>);

	template<typename key_type, const auto& tuple, size_t actualCount>
	constexpr construct_simd_hash_sub_tuple_function_ptr<key_type, tuple, actualCount> constructSimdHashSubTupleFinalPtrs[5] = {
		&constructSimdHashSubTupleFinal<key_type, tuple, actualCount, 16ull>, &constructSimdHashSubTupleFinal<key_type, tuple, actualCount, 32ull>,
		&constructSimdHashSubTupleFinal<key_type, tuple, actualCount, 64ull>, &constructSimdHashSubTupleFinal<key_type, tuple, actualCount, 128ull>,
		&constructSimdHashSubTupleFinal<key_type, tuple, actualCount, 256ull>
	};

	template<size_t maxSizeIndex, typename key_type, const auto& tuple, size_t actualCount>
	constexpr auto constructSimdHashSubTupleInternal(const std::array<key_type, actualCount>& pairsNew, xoshiro256 prng, key_stats_t& keyStatsVal) {
		auto constructForGivenStringLength = [&](size_t stringLength, auto&& constructForGivenStringLength, const auto maxSizeIndexNew) mutable -> map_construction_values {
			constexpr size_t bucketSize	 = setSimdWidth<hashTupleMaxSizes[maxSizeIndexNew]>();
			constexpr size_t storageSize = hashTupleMaxSizes[maxSizeIndexNew];
			constexpr size_t numGroups	 = storageSize > bucketSize ? storageSize / bucketSize : 1;
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
					const auto keySize		 = pairsNew[y].size() > stringLength ? stringLength : pairsNew[y].size();
					const auto hash			 = hasherNew.hashKeyCt(pairsNew[y].data(), keySize);
					const auto groupPos		 = (hash >> 8) % numGroups;
					const auto ctrlByte		 = static_cast<uint8_t>(hash);
					const auto bucketSizeNew = ++bucketSizes[groupPos];
					const auto slot			 = ((groupPos * bucketSize) + bucketSizeNew);

					if (bucketSizeNew >= bucketSize || contains(slots.data() + groupPos * bucketSize, slot, bucketSize) ||
						contains(controlBytes.data() + groupPos * bucketSize, ctrlByte, bucketSize)) {
						collided = true;
						break;
					}

					controlBytes[slot] = ctrlByte;
					slots[y]		   = slot;
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
				} else if (stringLength < keyStatsVal.maxLength - 1) {
					return constructForGivenStringLength(stringLength + 1, constructForGivenStringLength, std::integral_constant<size_t, maxSizeIndexNew>{});
				}
				returnValues.success = false;
				return returnValues;
			}
			returnValues.maxSizeIndex = maxSizeIndexNew;
			returnValues.stringLength = stringLength;
			returnValues.seed		  = seed;
			returnValues.success	  = true;
			return returnValues;
		};

		map_construction_values bestValues{ constructForGivenStringLength(keyStatsVal.minLength, constructForGivenStringLength, std::integral_constant<size_t, 0>{}) };

		return bestValues;
	}

	template<typename key_type, const auto& tuple, size_t actualCount> constexpr auto constructSimdHashSubTuple(const std::array<key_type, actualCount>& pairsNew) {
		key_stats_t keyStatsVal = keyStats<tuple>();
		auto constructionValues =
			constructSimdHashSubTupleInternal<getMaxSizeIndex<actualCount>(hashTupleMaxSizes), key_type, tuple, actualCount>(pairsNew, xoshiro256{}, keyStatsVal);
		return constructSimdHashSubTupleFinalPtrs<key_type, tuple, actualCount>[constructionValues.maxSizeIndex](pairsNew, constructionValues);
	}

	template<typename key_type_new, const auto& tupleNew, size_t actualCount, size_t storageSizeNew> struct minimal_char_hash_sub_tuple : public key_hasher,
																																		  public hash_tuple_base {
		using key_type	= key_type_new;
		using size_type = size_t;
		static constexpr auto tuple{ tupleNew };
		static constexpr auto nonConstCompareStringFunctions{ generateNonConstCompareStringFunctionPtrArray<tuple>() };
		static constexpr auto constCompareStringFunctions{ generateConstCompareStringFunctionPtrArray<tuple>() };
		JSONIFIER_ALIGN std::array<size_t, storageSizeNew> items{};
		JSONIFIER_ALIGN size_t stringLength{};

		constexpr minimal_char_hash_sub_tuple() noexcept {
			isItTrue = true;
		};

		constexpr operator bool() {
			return true;
		}

		template<typename function_type> JSONIFIER_INLINE constexpr auto find(const char* iter, size_type keyLength, function_type& functionPtrs) const noexcept {
			JSONIFIER_ALIGN const auto stringLengthNew = keyLength > stringLength ? stringLength : keyLength;
			JSONIFIER_ALIGN const auto hash			   = seed * (operator uint64_t() ^ iter[0]) + iter[stringLengthNew - 1];
			JSONIFIER_ALIGN const auto finalIndex	   = hash % storageSizeNew;
			if (!std::is_constant_evaluated()) {
				if (nonConstCompareStringFunctions[items[finalIndex]](iter)) {
					return functionPtrs.data() + items[finalIndex];
				} else {
					return functionPtrs.data() + functionPtrs.size();
				}
			} else {
				if (constCompareStringFunctions[items[finalIndex]](iter)) {
					return functionPtrs.data() + items[finalIndex];
				} else {
					return functionPtrs.data() + functionPtrs.size();
				}
			}
		}
	};

	template<size_t startingValue, size_t actualCount, typename key_type, const auto& tuple, template<typename, const auto&, size_t, size_t> typename tuple_type>
	using tuple_variant = std::variant<tuple_type<key_type, tuple, actualCount, startingValue>, tuple_type<key_type, tuple, actualCount, startingValue * 2>,
		tuple_type<key_type, tuple, actualCount, startingValue * 4>, tuple_type<key_type, tuple, actualCount, startingValue * 8>,
		tuple_type<key_type, tuple, actualCount, startingValue * 16>>;

	template<typename key_type, const auto& tuple, size_t actualCount, size_t storageSize>
	constexpr auto constructMinimalCharHashSubTupleFinal(const std::array<key_type, actualCount>& pairsNew, tuple_construction_values constructionValues)
		-> tuple_variant<16ull, actualCount, key_type, tuple, minimal_char_hash_sub_tuple> {
		minimal_char_hash_sub_tuple<key_type, tuple, actualCount, storageSize> minimalCharHashSubTupleNew{};
		minimalCharHashSubTupleNew.stringLength = constructionValues.stringLength;
		minimalCharHashSubTupleNew.setSeedCt(constructionValues.seed);
		for (size_t x = 0; x < actualCount; ++x) {
			const auto keySize		   = pairsNew[x].size();
			const auto stringLengthNew = keySize > minimalCharHashSubTupleNew.stringLength ? minimalCharHashSubTupleNew.stringLength : keySize;
			const auto hash = constructionValues.seed * (minimalCharHashSubTupleNew.operator uint64_t() ^ pairsNew[x].data()[0]) + pairsNew[x].data()[stringLengthNew - 1];
			const auto slot = hash % storageSize;
			minimalCharHashSubTupleNew.items[slot] = x;
		}
		return tuple_variant<16ull, actualCount, key_type, tuple, minimal_char_hash_sub_tuple>{ minimal_char_hash_sub_tuple<key_type, tuple, actualCount, storageSize>{
			minimalCharHashSubTupleNew } };
	}

	template<typename key_type, const auto& tuple, size_t actualCount> using construct_minimal_char_hash_sub_tuple_function_ptr =
		decltype(&constructMinimalCharHashSubTupleFinal<key_type, tuple, actualCount, 16ull>);

	template<typename key_type, const auto& tuple, size_t actualCount>
	constexpr construct_minimal_char_hash_sub_tuple_function_ptr<key_type, tuple, actualCount> constructMinimalCharHashSubTupleFinalPtrs[5] = {
		&constructMinimalCharHashSubTupleFinal<key_type, tuple, actualCount, 16ull>, &constructMinimalCharHashSubTupleFinal<key_type, tuple, actualCount, 32ull>,
		&constructMinimalCharHashSubTupleFinal<key_type, tuple, actualCount, 64ull>, &constructMinimalCharHashSubTupleFinal<key_type, tuple, actualCount, 128ull>,
		&constructMinimalCharHashSubTupleFinal<key_type, tuple, actualCount, 256ull>
	};

	template<size_t maxSizeIndex, typename key_type, const auto& tuple, size_t actualCount>
	constexpr auto constructMinimalCharHashSubTupleInternal(const std::array<key_type, actualCount>& pairsNew, xoshiro256 prng, key_stats_t& keyStatsVal) {
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
					collided				   = false;
					const auto keySize		   = pairsNew[y].size();
					const auto stringLengthNew = keySize > stringLength ? stringLength : keySize;
					const auto hash			   = seed * (hasherNew.operator uint64_t() ^ pairsNew[y].data()[0]) + pairsNew[y].data()[stringLengthNew - 1];
					const auto slot			   = hash % storageSize;

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
				} else if (stringLength < keyStatsVal.maxLength - 1) {
					return constructForGivenStringLength(stringLength + 1, constructForGivenStringLength, std::integral_constant<size_t, maxSizeIndexNew>{});
				}
				returnValues.success = false;
				return returnValues;
			}
			returnValues.maxSizeIndex = maxSizeIndexNew;
			returnValues.stringLength = stringLength;
			returnValues.seed		  = seed;
			returnValues.success	  = true;
			return returnValues;
		};

		tuple_construction_values bestValues{ constructForGivenStringLength(keyStatsVal.minLength, constructForGivenStringLength, std::integral_constant<size_t, 0>{}) };

		return bestValues;
	}

	template<typename key_type, const auto& tuple, size_t actualCount> constexpr auto constructMinimalCharHashSubTuple(const std::array<key_type, actualCount>& pairsNew) {
		auto keyStatsVal = keyStats<tuple>();
		auto constructionValues =
			constructMinimalCharHashSubTupleInternal<getMaxSizeIndex<actualCount>(hashTupleMaxSizes), key_type, tuple, actualCount>(pairsNew, xoshiro256{}, keyStatsVal);
		return constructMinimalCharHashSubTupleFinalPtrs<key_type, tuple, actualCount>[constructionValues.maxSizeIndex](pairsNew, constructionValues);
	}

	template<const auto& tuple, size_t I> constexpr auto keyValue() noexcept {
		constexpr auto& first = std::get<0>(std::get<I>(tuple));
		using T0			  = unwrap_t<decltype(first)>;
		if constexpr (std::is_member_pointer_v<T0>) {
			return std::tuple{ getName<first>(), first };
		} else {
			return std::tuple{ jsonifier::string_view(first), std::get<1>(std::get<I>(tuple)) };
		}
	}

	template<const auto& tuple, size_t I> constexpr auto getValue() noexcept {
		constexpr auto& first = std::get<1>(std::get<I>(tuple));
		using T0			  = unwrap_t<decltype(first)>;
		return first;
	}

	template<const auto& tuple, size_t... I> constexpr auto makeSubTupleImpl(std::index_sequence<I...>) {
		constexpr auto n = std::tuple_size_v<unwrap_t<decltype(tuple)>>;

		if constexpr (n == 0) {
			return nullptr;
		} else if constexpr (n < 16) {
			constexpr auto tupleNew	   = constructMinimalCharHashSubTuple<jsonifier::string_view, tuple, n>({ getKey<tuple, I>()... });
			constexpr auto newIndex	   = tupleNew.index();
			constexpr auto newSubTuple = std::get<newIndex>(tupleNew);
			if constexpr (newSubTuple.stringLength == std::numeric_limits<size_t>::max()) {
				constexpr auto mapNewer	  = constructSimdHashSubTuple<jsonifier::string_view, tuple, n>({ getKey<tuple, I>()... });
				constexpr auto newIndexer = mapNewer.index();
				return std::get<newIndexer>(mapNewer);
			} else {
				return newSubTuple;
			}
		} else {
			constexpr auto mapNew	   = constructSimdHashSubTuple<jsonifier::string_view, tuple, n>({ getKey<tuple, I>()... });
			constexpr auto newIndex	   = mapNew.index();
			constexpr auto newSubTuple = std::get<newIndex>(mapNew);
			return newSubTuple;
		}
	}

	template<typename Tuple> struct tuple_types;

	template<typename... Types> struct tuple_types<std::tuple<Types...>> {
		using type = std::variant<hash_tuple_base, Types...>;
	};

	template<typename Tuple> using tuple_types_t = typename tuple_types<Tuple>::type;

	template<typename value_type, size_t stringLength> JSONIFIER_INLINE constexpr auto collectSubTuple();

	template<typename value_type, size_t index> using collect_sub_tuple_ptr = decltype(&collectSubTuple<value_type, index>);

	template<typename value_type, size_t... indices> constexpr auto generateCollectSubTuplePtrArrayInternal(std::index_sequence<indices...>) {
		return std::array<collect_sub_tuple_ptr<value_type, 0>, sizeof...(indices)>{ { &collectSubTuple<value_type, indices>... } };
	}

	template<typename value_type> constexpr auto generateCollectSubTuplePtrArray();

	template<const auto& tuple> constexpr auto makeSubTuple() {
		return makeSubTupleImpl<tuple>(std::make_index_sequence<std::tuple_size_v<unwrap_t<decltype(tuple)>>>{});
	}

	template<typename value_type, const auto& tuple, std::size_t currentIndex> constexpr auto makeHashTupleInternal() {
		constexpr auto& newTuple = std::get<currentIndex>(tuple);
		return std::make_tuple(makeSubTuple<newTuple>());
	}

	template<typename value_type, const auto& tuple, std::size_t... indices> constexpr auto makeHashTupleInternal(std::index_sequence<indices...>) {
		return std::tuple_cat(makeHashTupleInternal<value_type, tuple, indices>()...);
	}

	template<typename value_type> struct tuple_construction_data {
		static constexpr auto& tuple			= jsonifier::concepts::coreV<value_type>;
		static constexpr auto arrayOfRefs		= collectTupleElementReferences(tuple);
		static constexpr auto sortedRefs		= sortTupleElementReferences(arrayOfRefs);
		static constexpr auto stringLengthCount = countUniqueStringLengths<sortedRefs>();
		static constexpr auto stringLengths		= countUniqueStringLengths<stringLengthCount, sortedRefs>();
		static constexpr auto intermediateTuple = constructNewTuple<stringLengths, sortedRefs, tuple>();
		static constexpr auto finalTuple		= makeHashTupleInternal<value_type, intermediateTuple>(std::make_index_sequence<stringLengths.size()>{});
	};

	template<typename value_type, size_t stringLength> constexpr auto collectSubTuple() {
		using tuple_type   = typename std::decay_t<decltype(tuple_construction_data<value_type>::finalTuple)>;
		using variant_type = tuple_types_t<tuple_type>;

		constexpr size_t index = getCurrentSubTupleIndex<stringLength, tuple_construction_data<value_type>::stringLengths>();

		if constexpr (index < std::tuple_size_v<tuple_type>) {
			return variant_type{ std::in_place_index<index + 1>, std::get<index>(tuple_construction_data<value_type>::finalTuple) };
		} else {
			return variant_type{ std::in_place_index<0> };
		}
	};

	template<typename value_type> constexpr auto generateCollectSubTuplePtrArray() {
		return generateCollectSubTuplePtrArrayInternal<value_type>(std::make_index_sequence<512>{});
	}

	template<typename value_type> constexpr auto generateMappingIndices() {
		constexpr auto stringLengths = tuple_construction_data<value_type>::stringLengths;
		std::array<uint64_t, 512> returnValues{};
		returnValues.fill(std::numeric_limits<uint64_t>::max());
		for (uint64_t x = 0; x < stringLengths.size(); ++x) {
			returnValues[stringLengths[x].length] = x % stringLengths[x].length;
		}
		return returnValues;
	}

	template<const auto& variant, uint64_t currentIndex = 0> constexpr decltype(auto) visit() {
		if constexpr (currentIndex < std::variant_size_v<unwrap_t<decltype(variant)>>) {
			if constexpr (currentIndex == variant.index()) {
				return std::get<currentIndex>(variant);
			}
			return &visit<variant, currentIndex + 1>();
		} else {
			return static_cast<decltype(&std::get<currentIndex - 1>(variant))&>(nullptr);
		}
	}

	template<typename base_type, uint64_t currentIndex = 0, typename variant_type> constexpr auto* visit(variant_type& variant) {
		if constexpr (currentIndex < std::variant_size_v<std::remove_cvref_t<variant_type>>) {
			if (currentIndex == variant.index()) {
				return static_cast<const base_type*>(&std::get<currentIndex>(variant));
			} else {
				return static_cast<const base_type*>(visit<base_type, currentIndex>(variant));
			}
		} else {
			return static_cast<const base_type*>(nullptr);
		}
	}

	template<typename value_type> struct hash_tuple {
		using size_type	   = size_t;
		using control_type = uint8_t;
		static constexpr auto mappingIndices{ generateMappingIndices<value_type>() };
		static constexpr auto getSubTuplePtrArray{ generateCollectSubTuplePtrArray<value_type>() };

		constexpr hash_tuple() noexcept {};

		constexpr operator bool() {
			return true;
		}

		template<typename... arg_types> constexpr auto find(size_type stringLength) const {
			std::cout << "STRING LENGTH: " << stringLength << std::endl;
			for (auto& value: tuple_construction_data<value_type>::stringLengths) {
				std::cout << "LENGTH: " << value.length << std::endl;
			}
			const auto newVariant = getSubTuplePtrArray[stringLength]();
			std::cout << "INPUT: " << typeid(getSubTuplePtrArray[stringLength]()).name() << std::endl;
			return visit<hash_tuple_base>(newVariant);
		}
	};

	template<typename value_type> constexpr auto makeHashTuple() {
		constexpr auto& newTuple = tuple_construction_data<value_type>::finalTuple;
		constexpr auto newSize	 = std::tuple_size_v<unwrap_t<decltype(newTuple)>>;
		constexpr auto hashTuple = hash_tuple<value_type>{};
		return hashTuple;
	}

}