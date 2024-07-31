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

	template<const auto& tuple, size_t currentIndex = 0, size_t maxIndex = std::tuple_size_v<unwrap_t<decltype(tuple)>>>
	JSONIFIER_INLINE constexpr auto collectTupleReferences(std::array<tuple_reference, maxIndex> arrayOfRefs = std::array<tuple_reference, maxIndex>{}) {
		if constexpr (currentIndex < maxIndex) {
			tuple_reference values{};
			values.oldIndex			  = currentIndex;
			values.stringLength		  = std::get<currentIndex>(tuple).view().size();
			arrayOfRefs[currentIndex] = values;
			return collectTupleReferences<tuple, currentIndex + 1>(arrayOfRefs);
		} else {
			return arrayOfRefs;
		}
	}

	template<const auto& arrayOfRefs> JSONIFIER_INLINE constexpr auto bubbleSort() {
		std::array<tuple_reference, arrayOfRefs.size()> returnValues{ arrayOfRefs };
		for (size_t i = 0; i < arrayOfRefs.size() - 1; ++i) {
			for (size_t j = 0; j < arrayOfRefs.size() - i - 1; ++j) {
				if (returnValues[j].stringLength > returnValues[j + 1].stringLength) {
					std::swap(returnValues[j], returnValues[j + 1]);
				}
			}
		}
		for (size_t i = 0; i < arrayOfRefs.size(); ++i) {
			returnValues[i].newIndex = i;
		}
		return returnValues;
	}

	template<typename value_type> JSONIFIER_INLINE constexpr auto sortTupleReferences() {
		constexpr auto& tuple		 = jsonifier::core<unwrap_t<value_type>>::parseValue.parseValue;
		constexpr auto collectedRefs = collectTupleReferences<tuple>();
		constexpr auto newRefs		 = bubbleSort<make_static<collectedRefs>::value>();
		return newRefs;
	}

	template<size_t start, size_t end, size_t... indices> JSONIFIER_INLINE constexpr auto generateCustomIndexSequence(std::index_sequence<indices...>) {
		return std::index_sequence<(start + indices)...>{};
	}

	template<size_t start, size_t end> struct custom_index_sequence_generator;

	template<size_t start, size_t end> struct custom_index_sequence_generator {
		using type = decltype(generateCustomIndexSequence<start, end>(std::make_index_sequence<end - start + 1>{}));
	};

	template<size_t start, size_t end> using custom_index_sequence = typename custom_index_sequence_generator<start, end>::type;

	template<size_t size> JSONIFIER_INLINE constexpr auto countUniqueStringLengths(const std::array<tuple_reference, size>& arrayOfRefs) {
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

	template<size_t counts, size_t size> JSONIFIER_INLINE constexpr auto collectUniqueStringLengths(const std::array<tuple_reference, size>& arrayOfRefs) {
		std::array<string_length_counts, counts> returnValues{};
		size_t currentIndex	 = 0;
		size_t currentLength = 0;

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

	template<uint64_t inputIndex, const auto& stringLengths> JSONIFIER_INLINE constexpr auto getCurrentStringLength() {
		for (size_t x = 0; x < stringLengths.size(); ++x) {
			if (inputIndex == x) {
				return stringLengths[x].length;
			}
		}
		return std::numeric_limits<size_t>::max();
	}

	template<size_t subTupleIndex, const auto& stringLengths> JSONIFIER_INLINE constexpr auto getCurrentStartingIndex() {
		size_t currentStartingIndex{};
		for (size_t x = 0; x < subTupleIndex; ++x) {
			currentStartingIndex += stringLengths[x].count;
		}
		return currentStartingIndex;
	}

	template<typename value_type> struct final_tuple_construction_static_data {
		static constexpr auto sortedTupleReferences{ sortTupleReferences<unwrap_t<value_type>>() };
		static constexpr auto uniqueStringLengthCount{ countUniqueStringLengths(sortedTupleReferences) };
		static constexpr auto uniqueStringLengths{ collectUniqueStringLengths<uniqueStringLengthCount>(sortedTupleReferences) };
	};

	template<typename value_type, size_t... indices> JSONIFIER_INLINE constexpr auto createGroupedSubTuple(std::index_sequence<indices...>) {
		constexpr auto& oldTuple  = jsonifier::concepts::coreV<value_type>;
		constexpr auto& tupleRefs = final_tuple_construction_static_data<value_type>::sortedTupleReferences;
		return std::make_tuple(std::get<tupleRefs[indices].oldIndex>(oldTuple)...);
	}

	template<typename value_type, size_t startIndex, size_t count> JSONIFIER_INLINE constexpr auto createGroupedTuple() {
		return createGroupedSubTuple<value_type>(custom_index_sequence<startIndex, (startIndex + count) - 1>{});
	}

	template<typename value_type, size_t... indices> JSONIFIER_INLINE constexpr auto createNewTupleInternal(std::index_sequence<indices...>) {
		constexpr auto& stringLengths = final_tuple_construction_static_data<value_type>::uniqueStringLengths;
		return std::make_tuple(createGroupedTuple<value_type, getCurrentStartingIndex<indices, stringLengths>(), stringLengths[indices].count>()...);
	}

	template<typename value_type> JSONIFIER_INLINE constexpr auto createNewTuple() {
		constexpr auto& tupleRefs = final_tuple_construction_static_data<value_type>::sortedTupleReferences;
		return createNewTupleInternal<value_type>(std::make_index_sequence<countUniqueStringLengths(tupleRefs)>{});
	}

	template<typename value_type> struct final_tuple_static_data {
		static constexpr auto staticData = createNewTuple<unwrap_t<value_type>>();
	};

	template<typename value_type01, typename value_type02> JSONIFIER_INLINE constexpr bool contains(const value_type01* hashData, value_type02 byteToCheckFor, size_t size) {
		for (size_t x = 0; x < size; ++x) {
			if (hashData[x] == byteToCheckFor) {
				return true;
			}
		}
		return false;
	}

	template<size_t size, size_t length> JSONIFIER_INLINE constexpr size_t getMaxSizeIndex(const std::array<size_t, length>& maxSizes) {
		for (size_t x = 0; x < std::size(maxSizes); ++x) {
			if (size <= maxSizes[x]) {
				return x;
			}
		}
		return std::size(maxSizes) - 1;
	}

	template<size_t length> JSONIFIER_INLINE constexpr size_t setSimdWidth() {
		return length >= 64 && bytesPerStep >= 64 ? 64 : length >= 32 && bytesPerStep >= 32 ? 32 : 16;
	}

	constexpr std::array<size_t, 5> hashTupleMaxSizes{ 16, 32, 64, 128, 256 };

	template<size_t length> struct tuple_simd {
		using type = std::conditional_t<length >= 64 && bytesPerStep >= 64, jsonifier_simd_int_512,
			std::conditional_t<length >= 32 && bytesPerStep >= 32, jsonifier_simd_int_256, jsonifier_simd_int_128>>;
	};

	template<size_t length> using tuple_simd_t = tuple_simd<length>::type;

	template<uint64_t index, const auto& tuple> struct string_compare_wrapper_const {
		JSONIFIER_INLINE static constexpr bool op(const char* string01) {
			if constexpr (index < std::tuple_size_v<unwrap_t<decltype(tuple)>>) {
				constexpr auto currentKey = getKey<tuple, index>();
				return currentKey == jsonifier::string_view{ string01, currentKey.size() };
			} else {
				return false;
			}
		}
	};

	template<uint64_t index, const auto& tuple> struct string_compare_wrapper_non_const {
		JSONIFIER_INLINE static bool op(const char* string01) {
			if constexpr (index < std::tuple_size_v<unwrap_t<decltype(tuple)>>) {
				constexpr auto currentKey = getKey<tuple, index>();
				return compare<currentKey.size()>(currentKey.data(), string01);
			} else {
				return false;
			}
		}
	};

	template<typename value_type, size_t subTupleIndex> JSONIFIER_INLINE constexpr size_t getActualSize() {
		constexpr auto& subTuple = std::get<subTupleIndex>(final_tuple_static_data<value_type>::staticData);
		using sub_tuple_t		 = unwrap_t<decltype(subTuple)>;
		return std::tuple_size_v<sub_tuple_t>;
	}

	template<template<uint64_t, const auto&> typename function_wrapper, const auto& tuple, size_t... indices>
	JSONIFIER_INLINE constexpr auto generateArrayOfFunctionPtrs(std::index_sequence<indices...>) {
		return std::array<decltype(&function_wrapper<0, tuple>::op), sizeof...(indices)>{ &function_wrapper<indices, tuple>::op... };
	}

	enum class sub_tuple_type {
		unset		 = 0,
		simd		 = 1,
		minimal_char = 2,
	};

	template<typename key_type_new, typename value_type, size_t subTupleIndexNew, size_t maxSizeIndexNew> struct sub_tuple_construction_data {
		using simd_type = tuple_simd_t<hashTupleMaxSizes[maxSizeIndexNew]>;
		JSONIFIER_ALIGN std::array<uint8_t, hashTupleMaxSizes[maxSizeIndexNew]> controlBytes{};
		JSONIFIER_ALIGN std::array<size_t, hashTupleMaxSizes[maxSizeIndexNew] + 1> indices{};
		size_t maxSizeIndex{ maxSizeIndexNew };
		size_t actualCount{ getActualSize<value_type, subTupleIndexNew>() };
		size_t storageSize{ hashTupleMaxSizes[maxSizeIndexNew] };
		size_t subTupleIndex{ subTupleIndexNew };
		size_t bucketSize{ setSimdWidth<hashTupleMaxSizes[maxSizeIndexNew]>() };
		size_t numGroups{ storageSize > bucketSize ? storageSize / bucketSize : 1 };
		sub_tuple_type type{};
		size_t stringLength{};
		key_hasher hasher{};

		JSONIFIER_INLINE constexpr sub_tuple_construction_data() noexcept = default;
	};

	template<typename key_type, typename value_type, size_t subTupleIndex, template<typename, typename, size_t, size_t> typename sub_tuple_construction_data_type>
	using sub_tuple_construction_data_variant = std::variant<sub_tuple_construction_data_type<key_type, value_type, subTupleIndex, 0>,
		sub_tuple_construction_data_type<key_type, value_type, subTupleIndex, 1>, sub_tuple_construction_data_type<key_type, value_type, subTupleIndex, 2>,
		sub_tuple_construction_data_type<key_type, value_type, subTupleIndex, 3>, sub_tuple_construction_data_type<key_type, value_type, subTupleIndex, 4>>;

	template<size_t size> JSONIFIER_INLINE constexpr size_t findUniqueColumnIndex(const std::array<jsonifier::string_view, size>& strings, const key_stats_t& keyStats) {
		constexpr size_t alphabetSize = 256;

		for (size_t index = 0; index < keyStats.maxLength; ++index) {
			bool allDifferent = true;
			std::array<bool, alphabetSize> seen{};

			for (const auto& str: strings) {
				char c = str[index % str.size()];
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

		return keyStats.minLength;
	}

	template<typename key_type, typename value_type, size_t subTupleIndexNew>
	JSONIFIER_INLINE constexpr auto collectSimdSubTupleData(const std::array<key_type, getActualSize<value_type, subTupleIndexNew>()>& pairsNew) {
		constexpr auto& tuple = std::get<subTupleIndexNew>(final_tuple_static_data<value_type>::staticData);
		auto keyStatsVal	  = keyStats<tuple>();
		xoshiro256 prng{};
		const auto stringLength = findUniqueColumnIndex(pairsNew, keyStatsVal);
		auto constructForGivenStringLength =
			[&](const auto maxSizeIndex, auto&& constructForGivenStringLength,
				size_t stringLength) mutable -> sub_tuple_construction_data_variant<key_type, value_type, subTupleIndexNew, sub_tuple_construction_data> {
			constexpr size_t bucketSize	 = setSimdWidth<hashTupleMaxSizes[maxSizeIndex]>();
			constexpr size_t storageSize = hashTupleMaxSizes[maxSizeIndex];
			constexpr size_t numGroups	 = storageSize > bucketSize ? storageSize / bucketSize : 1;
			sub_tuple_construction_data<key_type, value_type, subTupleIndexNew, maxSizeIndex> returnValues{};
			returnValues.stringLength = stringLength;
			size_t bucketSizes[numGroups]{};
			bool collided{};
			for (size_t x = 0; x < 2; ++x) {
				std::fill(returnValues.controlBytes.begin(), returnValues.controlBytes.end(), std::numeric_limits<uint8_t>::max());
				std::fill(returnValues.indices.begin(), returnValues.indices.end(), returnValues.indices.size() - 1);
				returnValues.hasher.setSeedCt(prng());
				collided = false;
				for (size_t y = 0; y < getActualSize<value_type, subTupleIndexNew>(); ++y) {
					const auto hash			 = (returnValues.hasher.operator size_t() ^ pairsNew[y].data()[stringLength]);
					const auto groupPos		 = (hash) % numGroups;
					const auto ctrlByte		 = static_cast<uint8_t>(hash);
					const auto bucketSizeNew = bucketSizes[groupPos]++;
					const auto slot			 = ((groupPos * bucketSize) + bucketSizeNew);

					if (bucketSizeNew >= bucketSize || contains(returnValues.controlBytes.data() + groupPos * bucketSize, ctrlByte, bucketSize)) {
						std::fill(bucketSizes, bucketSizes + numGroups, 0);
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
			if (collided) {
				if constexpr (maxSizeIndex < std::size(hashTupleMaxSizes) - 1) {
					return sub_tuple_construction_data_variant<key_type, value_type, subTupleIndexNew, sub_tuple_construction_data>{ constructForGivenStringLength(
						std::integral_constant<size_t, maxSizeIndex + 1>{}, constructForGivenStringLength, stringLength) };
				}
				returnValues.stringLength = std::numeric_limits<size_t>::max();
				returnValues.maxSizeIndex = std::numeric_limits<size_t>::max();
				return returnValues;
			}
			returnValues.type = sub_tuple_type::simd;
			return returnValues;
		};

		return constructForGivenStringLength(std::integral_constant<size_t, 0>{}, constructForGivenStringLength, stringLength);
	}

	template<typename key_type, typename value_type, size_t subTupleIndexNew>
	JSONIFIER_INLINE constexpr auto collectMinimalCharSubTupleData(const std::array<key_type, getActualSize<value_type, subTupleIndexNew>()>& pairsNew) {
		constexpr auto& tuple = std::get<subTupleIndexNew>(final_tuple_static_data<value_type>::staticData);
		auto keyStatsVal	  = keyStats<tuple>();
		xoshiro256 prng{};
		const auto stringLength = findUniqueColumnIndex(pairsNew, keyStatsVal);
		auto constructForGivenStringLength =
			[&](const auto maxSizeIndex, auto&& constructForGivenStringLength,
				size_t stringLength) mutable -> sub_tuple_construction_data_variant<key_type, value_type, subTupleIndexNew, sub_tuple_construction_data> {
			constexpr size_t storageSize = hashTupleMaxSizes[maxSizeIndex];
			sub_tuple_construction_data<key_type, value_type, subTupleIndexNew, maxSizeIndex> returnValues{};
			returnValues.stringLength = stringLength;
			bool collided{};
			for (size_t x = 0; x < 2; ++x) {
				std::fill(returnValues.indices.begin(), returnValues.indices.end(), returnValues.indices.size() - 1);
				returnValues.hasher.setSeedCt(prng());
				collided = false;
				for (size_t y = 0; y < getActualSize<value_type, subTupleIndexNew>(); ++y) {
					const auto hash = (returnValues.hasher.operator size_t() ^ pairsNew[y].data()[stringLength]);
					const auto slot = hash % storageSize;
					if (returnValues.indices[slot] != returnValues.indices.size() - 1) {
						collided = true;
						break;
					}
					returnValues.indices[slot] = y;
				}
				if (!collided) {
					break;
				}
			}
			if (collided) {
				if constexpr (maxSizeIndex < std::size(hashTupleMaxSizes) - 1) {
					return sub_tuple_construction_data_variant<key_type, value_type, subTupleIndexNew, sub_tuple_construction_data>{ constructForGivenStringLength(
						std::integral_constant<size_t, maxSizeIndex + 1>{}, constructForGivenStringLength, stringLength) };
				}
				returnValues.stringLength = std::numeric_limits<size_t>::max();
				returnValues.maxSizeIndex = std::numeric_limits<size_t>::max();
				return returnValues;
			}
			returnValues.type = sub_tuple_type::minimal_char;
			return returnValues;
		};

		return constructForGivenStringLength(std::integral_constant<size_t, 0>{}, constructForGivenStringLength, stringLength);
	}

	template<typename value_type, const auto& tuple, size_t subTupleIndex, size_t... I>
	JSONIFIER_INLINE constexpr auto generateSubTupleConstructionDataImpl(std::index_sequence<I...>) {
		using tuple_type		 = unwrap_t<decltype(tuple)>;
		constexpr auto tupleSize = std::tuple_size_v<tuple_type>;

		if constexpr (tupleSize == 0) {
			return nullptr;
		} else {
			return collectSimdSubTupleData<jsonifier::string_view, value_type, subTupleIndex>({ getKey<tuple, I>()... });
		}
	}

	template<typename value_type, size_t subTupleIndex> JSONIFIER_INLINE constexpr auto generateSubTupleConstructionData() {
		constexpr auto tupleSize = std::tuple_size_v<unwrap_t<decltype(std::get<subTupleIndex>(final_tuple_static_data<value_type>::staticData))>>;
		constexpr auto mapNew	 = generateSubTupleConstructionDataImpl<value_type, std::get<subTupleIndex>(final_tuple_static_data<value_type>::staticData), subTupleIndex>(
			   std::make_index_sequence<tupleSize>{});
		constexpr auto newIndex = mapNew.index();
		return std::get<newIndex>(mapNew);
	}

	template<typename value_type, size_t index> struct sub_tuple_construction_static_data {
		static constexpr auto staticData{ generateSubTupleConstructionData<unwrap_t<value_type>, index>() };
	};

	template<typename value_type, size_t subTupleIndexNew> struct simd_sub_tuple {
		static constexpr auto& constructionData{ sub_tuple_construction_static_data<value_type, subTupleIndexNew>::staticData };
		using simd_type	   = typename decltype(sub_tuple_construction_static_data<value_type, subTupleIndexNew>::staticData)::simd_type;
		using size_type	   = size_t;
		using control_type = uint8_t;
		static constexpr auto& tuple{ std::get<subTupleIndexNew>(final_tuple_static_data<value_type>::staticData) };
		static constexpr auto subTupleIndex{ subTupleIndexNew };

		static constexpr key_hasher hasher{ constructionData.hasher.operator size_t() };

		static constexpr auto nonConstCompareStringFunctions{ generateArrayOfFunctionPtrs<string_compare_wrapper_non_const, tuple>(
			std::make_index_sequence<constructionData.storageSize + 1>{}) };

		static constexpr auto constCompareStringFunctions{ generateArrayOfFunctionPtrs<string_compare_wrapper_const, tuple>(
			std::make_index_sequence<constructionData.storageSize + 1>{}) };

		JSONIFIER_INLINE constexpr simd_sub_tuple() noexcept = default;

		template<const auto& functionPtrs, typename... arg_types> JSONIFIER_INLINE static constexpr auto find(const char* iter, arg_types&&... args) noexcept {
			if (!std::is_constant_evaluated()) {
				JSONIFIER_ALIGN const auto hash		   = (hasher.operator size_t() ^ iter[constructionData.stringLength]);
				JSONIFIER_ALIGN const auto resultIndex = ((hash) % constructionData.numGroups) * constructionData.bucketSize;
				JSONIFIER_ALIGN const auto finalIndex  = (simd_internal::tzcnt(simd_internal::opCmpEq(simd_internal::gatherValue<simd_type>(static_cast<control_type>(hash)),
															  simd_internal::gatherValues<simd_type>(constructionData.controlBytes.data() + resultIndex))) +
					 resultIndex);
				if (nonConstCompareStringFunctions[constructionData.indices[finalIndex]](iter)) {
					functionPtrs[constructionData.indices[finalIndex]](std::forward<arg_types>(args)...);
					return true;
				} else {
					return false;
				}
			} else {
				JSONIFIER_ALIGN const auto hash		   = (hasher.operator size_t() ^ iter[constructionData.stringLength]);
				JSONIFIER_ALIGN const auto resultIndex = ((hash) % constructionData.numGroups) * constructionData.bucketSize;
				JSONIFIER_ALIGN const auto finalIndex  = (constMatch(constructionData.controlBytes.data() + resultIndex, static_cast<control_type>(hash)) + resultIndex);
				if (constCompareStringFunctions[constructionData.indices[finalIndex]](iter)) {
					functionPtrs[constructionData.indices[finalIndex]](std::forward<arg_types>(args)...);
					return true;
				} else {
					return false;
				}
			}
		}

	  protected:
		JSONIFIER_INLINE static constexpr size_type tzcnt(size_type value) {
			size_type count{};
			while ((value & 1) == 0 && value != 0) {
				value >>= 1;
				++count;
			}
			return count;
		}

		JSONIFIER_INLINE static constexpr size_type constMatch(const control_type* hashData, control_type hash) {
			size_type mask = 0;
			for (size_type x = 0; x < constructionData.bucketSize; ++x) {
				if (hashData[x] == hash) {
					mask |= (1ull << x);
				}
			}
			return tzcnt(mask);
		}
	};

	template<typename value_type> struct hash_tuple;

	template<typename value_type, size_t subTupleIndexNew> struct minimal_char_sub_tuple {
		static constexpr auto& constructionData{ sub_tuple_construction_static_data<value_type, subTupleIndexNew>::staticData };
		using simd_type	   = typename decltype(sub_tuple_construction_static_data<value_type, subTupleIndexNew>::staticData)::simd_type;
		using size_type	   = size_t;
		using control_type = uint8_t;
		static constexpr auto& tuple{ std::get<subTupleIndexNew>(final_tuple_static_data<value_type>::staticData) };
		static constexpr auto subTupleIndex{ subTupleIndexNew };

		static constexpr key_hasher hasher{ constructionData.hasher.operator size_t() };

		static constexpr auto nonConstCompareStringFunctions{ generateArrayOfFunctionPtrs<string_compare_wrapper_non_const, tuple>(
			std::make_index_sequence<constructionData.storageSize + 1>{}) };

		static constexpr auto constCompareStringFunctions{ generateArrayOfFunctionPtrs<string_compare_wrapper_const, tuple>(
			std::make_index_sequence<constructionData.storageSize + 1>{}) };

		JSONIFIER_INLINE constexpr minimal_char_sub_tuple() noexcept = default;

		template<const auto& functionPtrs, typename... arg_types> JSONIFIER_INLINE static constexpr auto find(const char* iter, arg_types&&... args) noexcept {
			if (!std::is_constant_evaluated()) {
				JSONIFIER_ALIGN const auto hash		  = (hasher.operator size_t() ^ iter[constructionData.stringLength]);
				JSONIFIER_ALIGN const auto finalIndex = hash % constructionData.storageSize;
				if (nonConstCompareStringFunctions[constructionData.indices[finalIndex]](iter)) {
					functionPtrs[constructionData.indices[finalIndex]](std::forward<arg_types>(args)...);
					return true;
				} else {
					return false;
				}
			} else {
				JSONIFIER_ALIGN const auto hash		  = (hasher.operator size_t() ^ iter[constructionData.stringLength]);
				JSONIFIER_ALIGN const auto finalIndex = hash % constructionData.storageSize;
				if (constCompareStringFunctions[constructionData.indices[finalIndex]](iter)) {
					functionPtrs[constructionData.indices[finalIndex]](std::forward<arg_types>(args)...);
					return true;
				} else {
					return false;
				}
			}
		}
	};

	template<size_t inputIndex, const auto& stringLengths> JSONIFIER_INLINE constexpr auto getCurrentSubTupleIndex() {
		for (size_t x = 0; x < stringLengths.size(); ++x) {
			if (inputIndex == stringLengths[x].length) {
				return x;
			}
		}
		return std::numeric_limits<size_t>::max();
	}

	template<const auto& function, typename value_type, size_t stringLength, typename... arg_types> JSONIFIER_INLINE auto collectSubTuple(arg_types&&... args) {
		using tuple_type	   = unwrap_t<decltype(final_tuple_static_data<value_type>::staticData)>;
		constexpr size_t index = getCurrentSubTupleIndex<stringLength, final_tuple_construction_static_data<value_type>::uniqueStringLengths>();
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

	template<typename value_type, size_t subTupleIndexNew> JSONIFIER_INLINE constexpr auto createSubTuple() {
		if constexpr (sub_tuple_construction_static_data<value_type, subTupleIndexNew>::staticData.type == sub_tuple_type::simd) {
			return simd_sub_tuple<value_type, subTupleIndexNew>{};
		} else if constexpr (sub_tuple_construction_static_data<value_type, subTupleIndexNew>::staticData.type == sub_tuple_type::minimal_char) {
			return minimal_char_sub_tuple<value_type, subTupleIndexNew>{};
		}
	}

	template<typename value_type, size_t... indices> JSONIFIER_INLINE constexpr auto collectSubTupleImpl(std::index_sequence<indices...>) {
		return std::make_tuple(createSubTuple<value_type, indices>()...);
	}

	template<typename value_type> JSONIFIER_INLINE constexpr auto makeHashTupleInternal() {
		constexpr auto tupleSize{ std::tuple_size_v<decltype(final_tuple_static_data<value_type>::staticData)> };
		return collectSubTupleImpl<value_type>(std::make_index_sequence<tupleSize>{});
	}

	template<typename value_type, size_t subTupleIndex> using sub_tuple_t =
		std::conditional_t<sub_tuple_construction_static_data<value_type, subTupleIndex>::staticData.type == sub_tuple_type::simd, simd_sub_tuple<value_type, subTupleIndex>,
			minimal_char_sub_tuple<value_type, subTupleIndex>>;

	template<typename value_type> struct hash_tuple {
		using size_type = size_t;
		static constexpr auto tuple{ makeHashTupleInternal<unwrap_t<value_type>>() };

		JSONIFIER_INLINE constexpr hash_tuple() noexcept = default;

		template<size_t subTupleIndex, const auto& subTupleFunctionPtrArray, typename... arg_types>
		JSONIFIER_INLINE static constexpr auto find(const char* ptr01, arg_types&&... args) {
			return sub_tuple_t<value_type, subTupleIndex>::template find<subTupleFunctionPtrArray>(ptr01, std::forward<arg_types>(args)...);
		}

		template<const auto& function, typename... arg_types> JSONIFIER_INLINE static constexpr auto find(size_type stringLength, arg_types&&... args) {
			constexpr auto getSubTuplePtrArray = generateCollectSubTuplePtrArray<function, unwrap_t<value_type>, arg_types...>();
			return getSubTuplePtrArray[stringLength](std::forward<arg_types>(args)...);
		}
	};

}