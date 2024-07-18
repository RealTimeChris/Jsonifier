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

	template<typename value_type01, typename value_type02> constexpr bool contains(const value_type01* hashData, value_type02 byteToCheckFor, size_t size) {
		for (size_t x = 0; x < size; ++x) {
			if (hashData[x] == byteToCheckFor) {
				return true;
			}
		}
		return false;
	}

	template<size_t size, size_t length> constexpr size_t getMaxSizeIndex(const size_t (&maxSizes)[length]) {
		for (size_t x = 0; x < std::size(maxSizes); ++x) {
			if (size <= maxSizes[x]) {
				return x;
			}
		}
		return std::size(maxSizes) - 1;
	}

	constexpr size_t simdHashMapMaxSizes[]{ 16, 32, 64, 128, 256 };
	constexpr size_t minimalCharHashMapMaxSizes[]{ 16, 32, 64, 128, 256 };

	struct map_construction_values {
		size_t stringLength{ std::numeric_limits<size_t>::max() };
		size_t maxSizeIndex{ std::numeric_limits<size_t>::max() };
		bool success{};
		size_t seed{};
	};

	template<size_t length> struct map_simd {
		using type = std::conditional_t<length >= 64 && bytesPerStep >= 64, simd_int_512, std::conditional_t<length >= 32 && bytesPerStep >= 32, simd_int_256, simd_int_128>>;
	};

	template<size_t length> constexpr size_t setSimdWidth() {
		return length >= 64 && bytesPerStep >= 64 ? 64 : length >= 32 && bytesPerStep >= 32 ? 32 : 16;
	}

	template<size_t length> using map_simd_t = map_simd<length>::type;

	template<typename string_type01, typename string_type02> JSONIFIER_INLINE constexpr bool compareSvConst(const string_type01& lhs, const string_type02& rhs) noexcept {
		return lhs == rhs;
	}

	template<typename string_type01, typename string_type02> JSONIFIER_INLINE constexpr bool compareSvNonConst(const string_type01& lhs, const string_type02& rhs) noexcept {
		return lhs.size() == rhs.size() && compare(lhs.data(), rhs.data(), lhs.size());
	}

	template<typename value_type, size_t index> JSONIFIER_INLINE bool compareStringFunctionNonConst(const char* string01) {
		static constexpr auto currentKey = getKey<value_type, index>();
		return compare<currentKey.size()>(currentKey.data(), string01);
	}

	template<typename value_type, size_t index> constexpr bool compareStringFunctionConst(const char* string01) {
		constexpr auto currentKey = getKey<value_type, index>();
		return currentKey == jsonifier::string_view{ string01, currentKey.size() };
	}

	template<typename value_type, size_t index> using compare_string_function_non_const_ptr = decltype(&compareStringFunctionNonConst<value_type, index>);

	template<typename value_type, size_t index> using compare_string_function_const_ptr = decltype(&compareStringFunctionConst<value_type, index>);

	template<typename value_type, size_t... indices> constexpr auto generateConstCompareStringFunctionPtrArrayInternal(std::index_sequence<indices...>) {
		return std::array<compare_string_function_const_ptr<value_type, 0>, sizeof...(indices)>{ { &compareStringFunctionConst<value_type, indices>... } };
	}

	template<typename value_type> constexpr auto generateConstCompareStringFunctionPtrArray() {
		constexpr auto tupleSize = std::tuple_size_v<jsonifier::concepts::core_t<value_type>>;
		return generateConstCompareStringFunctionPtrArrayInternal<value_type>(std::make_index_sequence<tupleSize>{});
	}

	template<typename value_type, size_t... indices> constexpr auto generateNonConstCompareStringFunctionPtrArrayInternal(std::index_sequence<indices...>) {
		return std::array<compare_string_function_non_const_ptr<value_type, 0>, sizeof...(indices)>{ { &compareStringFunctionNonConst<value_type, indices>... } };
	}

	template<typename value_type> constexpr auto generateNonConstCompareStringFunctionPtrArray() {
		constexpr auto tupleSize = std::tuple_size_v<jsonifier::concepts::core_t<value_type>>;
		return generateNonConstCompareStringFunctionPtrArrayInternal<value_type>(std::make_index_sequence<tupleSize>{});
	}

	template<typename key_type_new, typename value_type_new, size_t actualCount, size_t storageSizeNew, const auto& tuple> struct simd_hash_map : public key_hasher {
		using simd_type		= map_simd_t<storageSizeNew>;
		using key_type		= key_type_new;
		using value_type	= value_type_new;
		using const_pointer = const unwrap_t<value_type>*;
		using size_type		= size_t;
		using control_type	= uint8_t;
		static constexpr size_type storageSize{ storageSizeNew };
		static constexpr size_type bucketSize = setSimdWidth<storageSize>();
		static constexpr auto nonConstCompareStringFunctions{ generateNonConstCompareStringFunctionPtrArray<value_type>() };
		static constexpr auto constCompareStringFunctions{ generateConstCompareStringFunctionPtrArray<value_type>() };
		static constexpr size_type numGroups = storageSize > bucketSize ? storageSize / bucketSize : 1;
		JSONIFIER_ALIGN std::array<size_t, storageSize + 1> items{};
		JSONIFIER_ALIGN control_type controlBytes[storageSize]{};
		JSONIFIER_ALIGN size_type stringLength{};

		constexpr simd_hash_map() noexcept {};

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

	template<size_t startingValue, size_t actualCount, typename key_type, typename value_type, const auto& tuple,
		template<typename, typename, size_t, size_t, const auto&> typename map_type>
	using map_variant = std::variant<map_type<key_type, value_type, actualCount, startingValue, tuple>, map_type<key_type, value_type, actualCount, startingValue * 2, tuple>,
		map_type<key_type, value_type, actualCount, startingValue * 4, tuple>, map_type<key_type, value_type, actualCount, startingValue * 8, tuple>,
		map_type<key_type, value_type, actualCount, startingValue * 16, tuple>>;

	template<typename key_type, typename value_type, size_t actualCount, size_t storageSize, const auto& tuple> constexpr auto constructSimdHashMapFinal(
		const std::array<key_type, actualCount>& pairsNew, map_construction_values constructionValues) -> map_variant<16, actualCount, key_type, value_type, tuple, simd_hash_map> {
		constexpr size_t bucketSize = setSimdWidth<storageSize>();
		constexpr size_t numGroups	= storageSize > bucketSize ? storageSize / bucketSize : 1;
		simd_hash_map<key_type, value_type, actualCount, storageSize, tuple> simdHashMapNew{};
		simdHashMapNew.setSeedCt(constructionValues.seed);
		simdHashMapNew.stringLength = constructionValues.stringLength;
		std::array<size_t, numGroups> bucketSizes{};
		for (size_t x = 0; x < actualCount; ++x) {
			const auto keySize				  = pairsNew[x].size() > simdHashMapNew.stringLength ? simdHashMapNew.stringLength : pairsNew[x].size();
			const auto hash					  = simdHashMapNew.hashKeyCt(pairsNew[x].data(), keySize);
			const auto groupPos				  = (hash >> 8) % numGroups;
			const auto ctrlByte				  = static_cast<uint8_t>(hash);
			const auto bucketSizeNew		  = ++bucketSizes[groupPos];
			const auto slot					  = ((groupPos * bucketSize) + bucketSizeNew);
			simdHashMapNew.items[slot]		  = x;
			simdHashMapNew.controlBytes[slot] = ctrlByte;
		}
		return map_variant<16, actualCount, key_type, value_type, tuple, simd_hash_map>{ simd_hash_map<key_type, value_type, actualCount, storageSize, tuple>(simdHashMapNew) };
	}

	template<typename key_type, typename value_type, size_t actualCount, const auto& tuple> using construct_simd_hash_map_function_ptr =
		decltype(&constructSimdHashMapFinal<key_type, value_type, actualCount, 16ull, tuple>);

	template<typename key_type, typename value_type, size_t actualCount, const auto& tuple>
	constexpr construct_simd_hash_map_function_ptr<key_type, value_type, actualCount, tuple> constructSimdHashMapFinalPtrs[5] = {
		&constructSimdHashMapFinal<key_type, value_type, actualCount, 16ull, tuple>, &constructSimdHashMapFinal<key_type, value_type, actualCount, 32ull, tuple>,
		&constructSimdHashMapFinal<key_type, value_type, actualCount, 64ull, tuple>, &constructSimdHashMapFinal<key_type, value_type, actualCount, 128ull, tuple>,
		&constructSimdHashMapFinal<key_type, value_type, actualCount, 256ull, tuple>
	};

	template<size_t maxSizeIndex, typename key_type, typename value_type, size_t actualCount>
	constexpr auto constructSimdHashMapInternal(const std::array<key_type, actualCount>& pairsNew, xoshiro256 prng, key_stats_t& keyStatsVal) {
		constexpr size_t bucketSize	 = setSimdWidth<simdHashMapMaxSizes[maxSizeIndex]>();
		constexpr size_t storageSize = simdHashMapMaxSizes[maxSizeIndex];
		constexpr size_t numGroups	 = storageSize > bucketSize ? storageSize / bucketSize : 1;

		auto constructForGivenStringLength = [&](size_t stringLength) constexpr -> map_construction_values {
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
				returnValues.success = false;
				return returnValues;
			}
			returnValues.maxSizeIndex = maxSizeIndex;
			returnValues.stringLength = stringLength;
			returnValues.seed		  = seed;
			returnValues.success	  = true;
			return returnValues;
		};

		map_construction_values bestValues{};

		size_t failedCount{};
		for (int64_t x = static_cast<int64_t>(keyStatsVal.maxLength); x >= static_cast<int64_t>(keyStatsVal.minLength) && x > 0; --x) {
			auto newValues = constructForGivenStringLength(static_cast<size_t>(x));
			if (newValues.success && newValues.stringLength <= bestValues.stringLength) {
				bestValues			  = newValues;
				keyStatsVal.maxLength = static_cast<size_t>(x) - 1;
			} else if (failedCount < 3) {
				++failedCount;
			} else {
				break;
			}
		}

		if (bestValues.success) {
			return bestValues;
		}

		if constexpr (maxSizeIndex < std::size(simdHashMapMaxSizes) - 1) {
			auto newValues = constructSimdHashMapInternal<maxSizeIndex + 1, key_type, value_type, actualCount>(pairsNew, prng, keyStatsVal);
			if (newValues.success && newValues.stringLength <= bestValues.stringLength) {
				bestValues = newValues;
			}
		}

		return bestValues;
	}

	template<typename key_type, typename value_type, size_t actualCount, const auto& tuple> constexpr auto constructSimdHashMap(const std::array<key_type, actualCount>& pairsNew) {
		key_stats_t keyStatsVal = keyStats<value_type>();
		auto constructionValues =
			constructSimdHashMapInternal<getMaxSizeIndex<actualCount>(simdHashMapMaxSizes), key_type, value_type, actualCount>(pairsNew, xoshiro256{}, keyStatsVal);
		return constructSimdHashMapFinalPtrs<key_type, value_type, actualCount, tuple>[constructionValues.maxSizeIndex](pairsNew, constructionValues);
	}

	template<typename key_type_new, typename value_type_new, size_t actualCount, size_t storageSizeNew, const auto& tuple> struct minimal_char_hash_map : public key_hasher {
		using key_type		= key_type_new;
		using value_type	= unwrap_t<value_type_new>;
		using const_pointer = const value_type*;
		using size_type		= size_t;
		static constexpr auto nonConstCompareStringFunctions{ generateNonConstCompareStringFunctionPtrArray<value_type>() };
		static constexpr auto constCompareStringFunctions{ generateConstCompareStringFunctionPtrArray<value_type>() };
		JSONIFIER_ALIGN std::array<size_t, storageSizeNew> items{};
		JSONIFIER_ALIGN size_t stringLength{};

		constexpr minimal_char_hash_map() noexcept = default;

		template<typename function_type> JSONIFIER_INLINE constexpr auto find(const char* iter, size_type keyLength, function_type& functionPtrs) const noexcept {
			JSONIFIER_ALIGN const auto stringLengthNew = keyLength > stringLength ? stringLength : keyLength;
			JSONIFIER_ALIGN const auto hash			   = seed * (operator size_t() ^ iter[0]) + iter[stringLengthNew - 1];
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

	template<typename key_type, typename value_type, size_t actualCount, size_t storageSize, const auto& tuple>
	constexpr auto constructMinimalCharHashMapFinal(const std::array<key_type, actualCount>& pairsNew,
		map_construction_values constructionValues) -> map_variant<16ull, actualCount, key_type, value_type, tuple, minimal_char_hash_map> {
		minimal_char_hash_map<key_type, value_type, actualCount, storageSize, tuple> minimalCharHashMapNew{};
		minimalCharHashMapNew.stringLength = constructionValues.stringLength;
		minimalCharHashMapNew.setSeedCt(constructionValues.seed);
		for (size_t x = 0; x < actualCount; ++x) {
			const auto keySize		   = pairsNew[x].size();
			const auto stringLengthNew = keySize > minimalCharHashMapNew.stringLength ? minimalCharHashMapNew.stringLength : keySize;
			const auto hash			   = constructionValues.seed * (minimalCharHashMapNew.operator size_t() ^ pairsNew[x].data()[0]) + pairsNew[x].data()[stringLengthNew - 1];
			const auto slot			   = hash % storageSize;
			minimalCharHashMapNew.items[slot] = x;
		}
		return map_variant<16ull, actualCount, key_type, value_type, tuple, minimal_char_hash_map>{ minimal_char_hash_map<key_type, value_type, actualCount, storageSize, tuple>{
			minimalCharHashMapNew } };
	}

	template<typename key_type, typename value_type, size_t actualCount, const auto& tuple> using construct_minimal_char_hash_map_function_ptr =
		decltype(&constructMinimalCharHashMapFinal<key_type, value_type, actualCount, 16ull, tuple>);

	template<typename key_type, typename value_type, size_t actualCount, const auto& tuple>
	constexpr construct_minimal_char_hash_map_function_ptr<key_type, value_type, actualCount, tuple> constructMinimalCharHashMapFinalPtrs[5] = {
		&constructMinimalCharHashMapFinal<key_type, value_type, actualCount, 16ull, tuple>, &constructMinimalCharHashMapFinal<key_type, value_type, actualCount, 32ull, tuple>,
		&constructMinimalCharHashMapFinal<key_type, value_type, actualCount, 64ull, tuple>, &constructMinimalCharHashMapFinal<key_type, value_type, actualCount, 128ull, tuple>,
		&constructMinimalCharHashMapFinal<key_type, value_type, actualCount, 256ull, tuple>
	};

	template<size_t maxSizeIndex, typename key_type, typename value_type, size_t actualCount>
	constexpr auto constructMinimalCharHashMapInternal(const std::array<key_type, actualCount>& pairsNew, xoshiro256 prng, key_stats_t& keyStatsVal) {
		constexpr size_t storageSize	   = minimalCharHashMapMaxSizes[maxSizeIndex];
		auto constructForGivenStringLength = [&](size_t stringLength) mutable -> map_construction_values {
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
					const auto hash			   = seed * (hasherNew.operator size_t() ^ pairsNew[y].data()[0]) + pairsNew[y].data()[stringLengthNew - 1];
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
			map_construction_values returnValues{};
			if (collided) {
				returnValues.success = false;
				return returnValues;
			}
			returnValues.maxSizeIndex = maxSizeIndex;
			returnValues.stringLength = stringLength;
			returnValues.seed		  = seed;
			returnValues.success	  = true;
			return returnValues;
		};

		map_construction_values bestValues{};

		size_t failedCount{};
		for (int64_t x = static_cast<int64_t>(keyStatsVal.maxLength); x >= static_cast<int64_t>(keyStatsVal.minLength) && x > 0; --x) {
			auto newValues = constructForGivenStringLength(static_cast<size_t>(x));
			if (newValues.success && newValues.stringLength <= bestValues.stringLength) {
				bestValues			  = newValues;
				keyStatsVal.maxLength = static_cast<size_t>(x) - 1;
			} else if (failedCount < 3) {
				++failedCount;
			} else {
				break;
			}
		}

		if (bestValues.success) {
			return bestValues;
		}

		if constexpr (maxSizeIndex < std::size(minimalCharHashMapMaxSizes) - 1) {
			auto newValues = constructMinimalCharHashMapInternal<maxSizeIndex + 1, key_type, value_type, actualCount>(pairsNew, prng, keyStatsVal);
			if (newValues.success && newValues.stringLength <= bestValues.stringLength) {
				bestValues = newValues;
			}
		}

		return bestValues;
	}

	template<typename key_type, typename value_type, size_t actualCount, const auto& newTuple>
	constexpr auto constructMinimalCharHashMap(const std::array<key_type, actualCount>& pairsNew) {
		auto keyStatsVal = keyStats<value_type>();
		auto constructionValues =
			constructMinimalCharHashMapInternal<getMaxSizeIndex<actualCount>(minimalCharHashMapMaxSizes), key_type, value_type, actualCount>(pairsNew, xoshiro256{}, keyStatsVal);
		return constructMinimalCharHashMapFinalPtrs<key_type, value_type, actualCount, newTuple>[constructionValues.maxSizeIndex](pairsNew, constructionValues);
	}

	template<const jsonifier::string_view& S, bool CheckSize = true> JSONIFIER_INLINE constexpr bool cxStringCmp(const jsonifier::string_view key) noexcept {
		if (!std::is_constant_evaluated()) {
			constexpr auto size = S.size();
			if constexpr (CheckSize) {
				return (size == key.size()) && compare<size>(S.data(), key.data());
			} else {
				return compare<size>(S.data(), key.data());
			}
		} else {
			return key == S;
		}
	}

	template<const jsonifier::string_view& lhs> JSONIFIER_INLINE constexpr bool compareSv(const jsonifier::string_view rhs) noexcept {
		if (!std::is_constant_evaluated()) {
			constexpr auto size = lhs.size();
			return (size == rhs.size()) && compare<size>(lhs.data(), rhs.data());
		} else {
			return lhs == rhs;
		}
	}

	template<typename value_type, size_t I> struct core_sv {
		static constexpr jsonifier::string_view value = getKey<value_type, I>();
	};

	template<typename value_type, size_t I> constexpr auto keyValue() noexcept {
		using value_t		  = jsonifier::concepts::core_t<value_type>;
		constexpr auto& first = std::get<0>(std::get<I>(jsonifier::concepts::coreV<value_type>));
		using T0			  = unwrap_t<decltype(first)>;
		if constexpr (std::is_member_pointer_v<T0>) {
			return std::tuple{ getName<first>(), first };
		} else {
			return std::tuple{ jsonifier::string_view(first), std::get<1>(std::get<I>(jsonifier::concepts::coreV<value_type>)) };
		}
	}

	template<typename value_type, size_t I> constexpr auto getValue() noexcept {
		constexpr auto& first = std::get<1>(std::get<I>(jsonifier::concepts::coreV<value_type>));
		using T0			  = unwrap_t<decltype(first)>;
		return first;
	}

	template<typename value_type> constexpr auto collectTuple() {
		return jsonifier::concepts::coreV<value_type>;
	}

	template<typename value_type, const auto& newTuple, size_t... I> constexpr auto makeMapImpl(std::index_sequence<I...>) {
		constexpr auto n = std::tuple_size_v<jsonifier::concepts::core_t<value_type>>;

		if constexpr (n == 0) {
			return nullptr;
		} else if constexpr (n < 16) {
			constexpr auto mapNew	= constructMinimalCharHashMap<jsonifier::string_view, value_type, n, newTuple>({ getKey<value_type, I>()... });
			constexpr auto newIndex = mapNew.index();
			constexpr auto newMap	= std::get<newIndex>(mapNew);
			if constexpr (newMap.stringLength == std::numeric_limits<size_t>::max()) {
				constexpr auto mapNewer	  = constructSimdHashMap<jsonifier::string_view, value_type, n, newTuple>({ getKey<value_type, I>()... });
				constexpr auto newIndexer = mapNewer.index();
				return std::get<newIndexer>(mapNewer);
			} else {
				return newMap;
			}
		} else {
			constexpr auto mapNew	= constructSimdHashMap<jsonifier::string_view, value_type, n, newTuple>({ getKey<value_type, I>()... });
			constexpr auto newIndex = mapNew.index();
			constexpr auto newMap	= std::get<newIndex>(mapNew);
			return newMap;
		}
	}

	template<typename value_type, const auto& newTuple> constexpr auto makeMap() {
		constexpr auto indices = std::make_index_sequence<std::tuple_size_v<jsonifier::concepts::core_t<value_type>>>{};
		return makeMapImpl<value_type, newTuple>(indices);
	}

}