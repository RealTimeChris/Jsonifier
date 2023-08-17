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
/// https://github.com/RealTimeChris/Jsonifier
/// Feb 3, 2023
#pragma once

#include <immintrin.h>
#include <iostream>
#include <stdlib.h>
#include <cstring>
#include <limits>

#ifdef max
	#undef max
#endif

#ifdef min
	#undef min
#endif

namespace JsonifierInternal {

#ifdef T_AVX512

	template<typename ValueType> inline __m128i gatherValues128(const ValueType* str) {
		alignas(16) float newArray[sizeof(__m128i) / sizeof(float)]{};
		std::memcpy(newArray, str, sizeof(__m128i));
		return _mm_castps_si128(_mm_load_ps(newArray));
	}

	template<typename ValueType> inline __m256i gatherValues256(const ValueType* str) {
		alignas(32) float newArray[sizeof(__m256i) / sizeof(float)]{};
		std::memcpy(newArray, str, sizeof(__m256i));
		return _mm256_castps_si256(_mm256_load_ps(newArray));
	}

	template<typename ValueType> inline __m512i gatherValues512(const ValueType* str) {
		return _mm512_loadu_epi64(str);
	}

	template<typename ValueType> inline size_t findSingleCharacter(const ValueType* str, size_t length, ValueType target) {
		__m512i targetVec = _mm512_set1_epi8(target);
		__m512i currentVec = gatherValues512<ValueType>(str);
		const uint32_t vecSize = sizeof(__m512);
		size_t remainingBytes = length;
		__m512i compareResult{};
		size_t index = 0;
		while (remainingBytes >= vecSize) {
			currentVec = gatherValues512<ValueType>(str);
			auto mask = _mm512_cmpeq_epi8_mask(targetVec, currentVec);


			if (mask != 0) {
				uint64_t matchingIndex = _tzcnt_u64(mask);
				return index + matchingIndex;
			}

			str += vecSize;
			index += vecSize;
			remainingBytes -= vecSize;
		}

		for (size_t i = 0; i < remainingBytes; i++) {
			if (str[i] == target) {
				return index + i;
			}
		}

		return std::numeric_limits<size_t>::max();
	}

	template<typename ValueType01, typename ValueType02>
	inline bool compareValues(const ValueType01* string1, const ValueType02* string2, size_t length);

	template<typename ValueType, typename IndexType> inline uint64_t getIntervalCount(uint64_t originalLength) {
		return originalLength * sizeof(ValueType) / sizeof(IndexType);
	}

	template<typename ValueType01, typename ValueType02>
	inline bool compareValues16(const ValueType01* string1, const ValueType02* string2, size_t length) {
		constexpr size_t vectorSize = sizeof(__m128i) / sizeof(ValueType01);
		const size_t intervalCount = getIntervalCount<ValueType01, __m128i>(length);
		const size_t remainder = length % vectorSize;
		for (size_t x = 0; x < intervalCount; ++x) {
			auto destVector = gatherValues128<ValueType01>(string1 + x * vectorSize);
			auto sourceVector = gatherValues128<ValueType02>(string2 + x * vectorSize);
			if (_mm_movemask_epi8(_mm_cmpeq_epi8(destVector, sourceVector)) != (0xffff)) {
				return false;
			}
		}
		if (remainder > 0) {
			return compareValues(string1 + (vectorSize * intervalCount), string2 + (vectorSize * intervalCount), remainder);
		}
		return true;
	}

	template<typename ValueType01, typename ValueType02>
	inline bool compareValues32(const ValueType01* string1, const ValueType02* string2, size_t length) {
		constexpr size_t vectorSize = sizeof(__m256i) / sizeof(ValueType01);
		const size_t intervalCount = getIntervalCount<ValueType01, __m256i>(length);
		const size_t remainder = length % vectorSize;
		for (size_t x = 0; x < intervalCount; ++x) {
			auto destVector = gatherValues256<ValueType01>(string1 + x * vectorSize);
			auto sourceVector = gatherValues256<ValueType02>(string2 + x * vectorSize);
			if (_mm256_movemask_epi8(_mm256_cmpeq_epi8(destVector, sourceVector)) != (0xffffffff)) {
				return false;
			}
		}
		if (remainder > 0) {
			return compareValues(string1 + (vectorSize * intervalCount), string2 + (vectorSize * intervalCount), remainder);
		}
		return true;
	}

	template<typename ValueType01, typename ValueType02>
	inline bool compareValues64(const ValueType01* string1, const ValueType02* string2, size_t length) {
		constexpr size_t vectorSize = sizeof(__m512i) / sizeof(ValueType01);
		const size_t intervalCount = getIntervalCount<ValueType01, __m512i>(length);
		const size_t remainder = length % vectorSize;
		for (size_t x = 0; x < intervalCount; ++x) {
			auto destVector = gatherValues512<ValueType01>(string1 + x * vectorSize);
			auto sourceVector = gatherValues512<ValueType02>(string2 + x * vectorSize);
			if (_mm512_cmpeq_epi8_mask(destVector, sourceVector) != (0xffffffffffffffff)) {
				return false;
			}
		}
		if (remainder > 0) {
			return compareValues(string1 + (vectorSize * intervalCount), string2 + (vectorSize * intervalCount), remainder);
		}
		return true;
	}

	template<typename ValueType01, typename ValueType02>
	inline bool compareValues(const ValueType01* string1, const ValueType02* string2, size_t length) {
		if (length >= 64) {
			return compareValues64(string1, string2, length);
		} else if (length >= 32) {
			return compareValues32(string1, string2, length);
		} else if (length >= 16) {
			return compareValues16(string1, string2, length);
		} else {
			return (std::memcmp(string1, string2, length * sizeof(ValueType01)) == 0);
		}
	}

#elif defined T_AVX2

	template<typename ValueType> inline __m128i gatherValues128(const ValueType* str) {
		alignas(16) float newArray[sizeof(__m128i) / sizeof(float)]{};
		std::memcpy(newArray, str, sizeof(__m128i));
		return _mm_castps_si128(_mm_load_ps(newArray));
	}

	template<typename ValueType> inline __m256i gatherValues256(const ValueType* str) {
		alignas(32) float newArray[sizeof(__m256i) / sizeof(float)]{};
		std::memcpy(newArray, str, sizeof(__m256i));
		return _mm256_castps_si256(_mm256_load_ps(newArray));
	}

	template<typename ValueType> inline size_t findSingleCharacter(const ValueType* str, size_t length, ValueType target) {
		__m256i targetVec = _mm256_set1_epi8(target);
		__m256i currentVec = gatherValues256<ValueType>(str);
		const uint32_t vecSize = sizeof(__m256);
		size_t remainingBytes = length;
		__m256i compareResult{};
		size_t index = 0;
		while (remainingBytes >= vecSize) {
			currentVec = gatherValues256<ValueType>(str);
			compareResult = _mm256_cmpeq_epi64(targetVec, currentVec);
			uint32_t mask = _mm256_movemask_epi8(compareResult);


			if (mask != 0) {
				uint32_t matchingIndex = _tzcnt_u32(mask);
				return index + matchingIndex;
			}

			str += vecSize;
			index += vecSize;
			remainingBytes -= vecSize;
		}

		for (size_t i = 0; i < remainingBytes; i++) {
			if (str[i] == target) {
				return index + i;
			}
		}

		return std::numeric_limits<size_t>::max();
	}

	template<typename ValueType01, typename ValueType02>
	inline bool compareValues(const ValueType01* string1, const ValueType02* string2, size_t length);

	template<typename ValueType, typename IndexType> inline uint64_t getIntervalCount(uint64_t originalLength) {
		return originalLength * sizeof(ValueType) / sizeof(IndexType);
	}

	template<typename ValueType01, typename ValueType02>
	inline bool compareValues16(const ValueType01* string1, const ValueType02* string2, size_t length) {
		constexpr size_t vectorSize = sizeof(__m128i) / sizeof(ValueType01);
		const size_t intervalCount = getIntervalCount<ValueType01, __m128i>(length);
		const size_t remainder = length % vectorSize;
		for (size_t x = 0; x < intervalCount; ++x) {
			auto destVector = gatherValues128<ValueType01>(string1 + x * vectorSize);
			auto sourceVector = gatherValues128<ValueType02>(string2 + x * vectorSize);
			if (_mm_movemask_epi8(_mm_cmpeq_epi8(destVector, sourceVector)) != (0xffff)) {
				return false;
			}
		}
		if (remainder > 0) {
			return compareValues(string1 + (vectorSize * intervalCount), string2 + (vectorSize * intervalCount), remainder);
		}
		return true;
	}

	template<typename ValueType01, typename ValueType02>
	inline bool compareValues32(const ValueType01* string1, const ValueType02* string2, size_t length) {
		constexpr size_t vectorSize = sizeof(__m256i) / sizeof(ValueType01);
		const size_t intervalCount = getIntervalCount<ValueType01, __m256i>(length);
		const size_t remainder = length % vectorSize;
		for (size_t x = 0; x < intervalCount; ++x) {
			auto destVector = gatherValues256<ValueType01>(string1 + x * vectorSize);
			auto sourceVector = gatherValues256<ValueType02>(string2 + x * vectorSize);
			if (_mm256_movemask_epi8(_mm256_cmpeq_epi8(destVector, sourceVector)) != (0xffffffff)) {
				return false;
			}
		}
		if (remainder > 0) {
			return compareValues(string1 + (vectorSize * intervalCount), string2 + (vectorSize * intervalCount), remainder);
		}
		return true;
	}

	template<typename ValueType01, typename ValueType02>
	inline bool compareValues(const ValueType01* string1, const ValueType02* string2, size_t length) {
		if (length >= 32) {
			return compareValues32(string1, string2, length);
		} else if (length >= 16) {
			return compareValues16(string1, string2, length);
		} else {
			return (std::memcmp(string1, string2, length * sizeof(ValueType01)) == 0);
		}
	}

#elif defined T_AVX

	template<typename ValueType> inline __m128i gatherValues128(const ValueType* str) {
		alignas(16) float newArray[sizeof(__m128i) / sizeof(float)]{};
		std::memcpy(newArray, str, sizeof(__m128i));
		return _mm_castps_si128(_mm_load_ps(newArray));
	}

	template<typename ValueType> inline size_t findSingleCharacter(const ValueType* str, size_t length, ValueType target) {
		__m128i targetVec = _mm_set1_epi8(target);
		__m128i currentVec = gatherValues128<ValueType>(str);
		const uint32_t vecSize = sizeof(__m128);
		size_t remainingBytes = length;
		__m128i compareResult{};
		size_t index = 0;
		while (remainingBytes >= vecSize) {
			currentVec = gatherValues128<ValueType>(str);
			compareResult = _mm_cmpeq_epi64(targetVec, currentVec);
			uint32_t mask = _mm_movemask_epi8(compareResult);


			if (mask != 0) {
				uint32_t matchingIndex = _tzcnt_u32(mask);
				return index + matchingIndex;
			}

			str += vecSize;
			index += vecSize;
			remainingBytes -= vecSize;
		}

		for (size_t i = 0; i < remainingBytes; i++) {
			if (str[i] == target) {
				return index + i;
			}
		}

		return std::numeric_limits<size_t>::max();
	}

	template<typename ValueType01, typename ValueType02>
	inline bool compareValues(const ValueType01* string1, const ValueType02* string2, size_t length);

	template<typename ValueType, typename IndexType> inline uint64_t getIntervalCount(uint64_t originalLength) {
		return originalLength * sizeof(ValueType) / sizeof(IndexType);
	}

	template<typename ValueType01, typename ValueType02>
	inline bool compareValues16(const ValueType01* string1, const ValueType02* string2, size_t length) {
		constexpr size_t vectorSize = sizeof(__m128i) / sizeof(ValueType01);
		const size_t intervalCount = getIntervalCount<ValueType01, __m128i>(length);
		const size_t remainder = length % vectorSize;
		for (size_t x = 0; x < intervalCount; ++x) {
			auto destVector = gatherValues128<ValueType01>(string1 + x * vectorSize);
			auto sourceVector = gatherValues128<ValueType02>(string2 + x * vectorSize);
			if (_mm_movemask_epi8(_mm_cmpeq_epi8(destVector, sourceVector)) != (0x0000ffff)) {
				return false;
			}
		}
		if (remainder > 0) {
			return compareValues(string1 + (vectorSize * intervalCount), string2 + (vectorSize * intervalCount), remainder);
		}
		return true;
	}

	template<typename ValueType01, typename ValueType02>
	inline bool compareValues(const ValueType01* string1, const ValueType02* string2, size_t length) {
		if (length >= 16) {
			return compareValues16(string1, string2, length);
		} else {
			return (std::memcmp(string1, string2, length * sizeof(ValueType01)) == 0);
		}
	}

#else

	inline uint64_t findSingleCharacter(const void* str, size_t length, char target) {
		return std::string_view{ static_cast<const char*>(str), length }.find(target);
	}

	template<typename ValueType01, typename ValueType02>
	inline bool compareValues(const void* destVector, const void* sourceVector, size_t length) {
		return std::string_view{ static_cast<const char*>(destVector), length } ==
			std::string_view{ static_cast<const char*>(sourceVector), length * sizeof(ValueType01) };
	}

#endif

	class JsonifierCoreInternal {
	  public:
		template<typename ValueType01, typename ValueType02>
		inline static bool compare(const ValueType01* destVector, const ValueType02* sourceVector, size_t length) {
			return compareValues<ValueType01, ValueType02>(destVector, sourceVector, length);
		}
	};

}