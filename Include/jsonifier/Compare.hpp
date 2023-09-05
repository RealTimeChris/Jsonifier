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
#include <cstring>

#if defined max
	#undef max
#endif

#if defined min
	#undef min
#endif

namespace JsonifierInternal {

	template<typename ValueType>
	concept FloatingT = std::floating_point<ValueType>;

#if defined T_AVX512

	template<typename ValueType> inline __m128i gatherValues128(const ValueType* str) {
		alignas(ALIGNMENT) float newArray[sizeof(__m128i) / sizeof(float)]{};
		std::memcpy(newArray, str, sizeof(__m128i));
		return _mm_castps_si128(_mm_load_ps(newArray));
	}

	template<typename ValueType> inline __m256i gatherValues256(const ValueType* str) {
		alignas(ALIGNMENT) float newArray[sizeof(__m256i) / sizeof(float)]{};
		std::memcpy(newArray, str, sizeof(__m256i));
		return _mm256_castps_si256(_mm256_load_ps(newArray));
	}

	template<typename ValueType> inline __m512i gatherValues512(const ValueType* str) {
		return _mm512_loadu_epi64(str);
	}

	template<FloatingT ValueType> inline __m128 gatherValues128(const ValueType* str) {
		return _mm_load_ps(str);
	}

	template<FloatingT ValueType> inline __m256 gatherValues256(const ValueType* str) {
		return _mm256_load_ps(str);
	}

	template<FloatingT ValueType> inline __m512 gatherValues512(const ValueType* str) {
		return _mm512_loadu_ps(str);
	}

	template<typename ValueType01, typename ValueType02> inline uint64_t findFirstCharacterNotEqual(const ValueType01* str, uint64_t length, ValueType02 target) {
		static constexpr uint64_t vecSize = sizeof(__m512);
		const __m512i targetVec			  = _mm512_set1_epi8(target);
		uint64_t remainingBytes			  = length;
		__m512i currentVec{};
		uint64_t index{};
		uint64_t mask{};

		while (remainingBytes >= vecSize) {
			currentVec = gatherValues512<ValueType01>(str);
			mask	   = _mm512_cmpeq_epi8_mask(targetVec, currentVec);

			if (mask != 0xffffffff) {
				uint64_t firstNonMatchMask = ~mask;
				return index + _tzcnt_u64(firstNonMatchMask);
			}

			str += vecSize;
			index += vecSize;
			remainingBytes -= vecSize;
		}

		for (uint64_t i = 0; i < remainingBytes; i++) {
			if (str[i] != target) {
				return index + i;
			}
		}

		return std::numeric_limits<uint64_t>::max();
	}

	template<typename ValueType01, typename ValueType02> inline uint64_t findSingleCharacter(const ValueType01* str, uint64_t length, ValueType02 target) {
		static constexpr uint64_t vecSize = sizeof(__m512);
		const __m512i targetVec			  = _mm512_set1_epi8(target);
		uint64_t remainingBytes{ length };
		__m512i currentVec{};
		uint64_t index{};
		uint64_t mask{};
		while (remainingBytes >= vecSize) {
			currentVec = gatherValues512<ValueType01>(str);
			mask	   = _mm512_cmpeq_epi8_mask(targetVec, currentVec);

			if (mask != 0) {
				return index + _tzcnt_u64(mask);
			}

			str += vecSize;
			index += vecSize;
			remainingBytes -= vecSize;
		}

		for (uint64_t i = 0; i < remainingBytes; i++) {
			if (str[i] == target) {
				return index + i;
			}
		}

		return std::numeric_limits<uint64_t>::max();
	}

	template<typename ValueType01, typename ValueType02> inline uint64_t findLastSingleCharacter(const ValueType01* str, uint64_t length, ValueType02 target) {
		static constexpr uint64_t vecSize = sizeof(__m512);
		const __m512i targetVec			  = _mm512_set1_epi8(target);
		uint64_t remainingBytes{};
		__m512i currentVec{};
		uint64_t index{ length };
		uint64_t mask{};
		str += length - vecSize;
		while (remainingBytes >= 0) {
			currentVec = gatherValues512<ValueType01>(str);
			mask	   = _mm512_cmpeq_epi8_mask(targetVec, currentVec);

			if (mask != 0) {
				return index - _lzcnt_u64(mask);
			}

			str -= vecSize;
			index -= vecSize;
			remainingBytes -= vecSize;
		}

		for (uint64_t i = remainingBytes; i > 0; --i) {
			if (str[i] == target) {
				return index + i;
			}
		}
		return std::numeric_limits<uint64_t>::max();
	}

	template<typename ValueType01, typename ValueType02> inline bool compareValues(const ValueType01* string1, const ValueType02* string2, uint64_t length);

	template<typename ValueType, typename IndexType> inline uint64_t getIntervalCount(uint64_t originalLength) {
		return originalLength * sizeof(ValueType) / sizeof(IndexType);
	}

	template<FloatingT ValueType01, FloatingT ValueType02> inline bool compareValues16(const ValueType01* string1, const ValueType02* string2, uint64_t length) {
		static constexpr uint64_t vectorSize = sizeof(__m128i) / sizeof(ValueType01);
		const uint64_t intervalCount		 = getIntervalCount<ValueType01, __m128i>(length);
		const uint64_t remainder			 = length % vectorSize;
		static constexpr uint16_t maskValue{ 0xffff };

		auto destVector	  = gatherValues128<ValueType01>(string1);
		auto sourceVector = gatherValues128<ValueType02>(string2);
		if (_mm_movemask_ps(_mm_cmpeq_ps(destVector, sourceVector)) != maskValue) {
			return false;
		}

		for (uint64_t x = 1; x < intervalCount; ++x) {
			destVector	 = gatherValues128<ValueType01>(string1 + x * vectorSize);
			sourceVector = gatherValues128<ValueType02>(string2 + x * vectorSize);
			if (_mm_movemask_ps(_mm_cmpeq_ps(destVector, sourceVector)) != maskValue) {
				return false;
			}
		}

		if (remainder > 0) {
			return compareValues(string1 + (vectorSize * intervalCount), string2 + (vectorSize * intervalCount), remainder);
		}

		return true;
	}

	template<typename ValueType01, typename ValueType02> inline bool compareValues16(const ValueType01* string1, const ValueType02* string2, uint64_t length) {
		static constexpr uint64_t vectorSize = sizeof(__m128i) / sizeof(ValueType01);
		const uint64_t intervalCount		 = getIntervalCount<ValueType01, __m128i>(length);
		const uint64_t remainder			 = length % vectorSize;
		static constexpr uint16_t maskValue{ 0xffff };
		auto destVector	  = gatherValues128<ValueType01>(string1);
		auto sourceVector = gatherValues128<ValueType02>(string2);
		if (_mm_movemask_epi8(_mm_cmpeq_epi8(destVector, sourceVector)) != maskValue) {
			return false;
		}
		for (uint64_t x = 1; x < intervalCount; ++x) {
			destVector	 = gatherValues128<ValueType01>(string1 + x * vectorSize);
			sourceVector = gatherValues128<ValueType02>(string2 + x * vectorSize);
			if (_mm_movemask_epi8(_mm_cmpeq_epi8(destVector, sourceVector)) != maskValue) {
				return false;
			}
		}
		if (remainder > 0) {
			return compareValues(string1 + (vectorSize * intervalCount), string2 + (vectorSize * intervalCount), remainder);
		}
		return true;
	}

	template<FloatingT ValueType01, FloatingT ValueType02> inline bool compareValues32(const ValueType01* string1, const ValueType02* string2, uint64_t length) {
		static constexpr uint64_t vectorSize = sizeof(__m256i) / sizeof(ValueType01);
		const uint64_t intervalCount		 = getIntervalCount<ValueType01, __m256i>(length);
		const uint64_t remainder			 = length % vectorSize;
		static constexpr uint32_t maskValue{ 0xffffffff };
		auto destVector	  = gatherValues256<ValueType01>(string1);
		auto sourceVector = gatherValues256<ValueType02>(string2);
		if (_mm256_movemask_ps(_mm256_cmp_ps(destVector, sourceVector, _CMP_EQ_OQ)) != maskValue) {
			return false;
		}
		for (uint64_t x = 1; x < intervalCount; ++x) {
			destVector	 = gatherValues256<ValueType01>(string1 + x * vectorSize);
			sourceVector = gatherValues256<ValueType02>(string2 + x * vectorSize);
			if (_mm256_movemask_ps(_mm256_cmp_ps(destVector, sourceVector, _CMP_EQ_OQ)) != maskValue) {
				return false;
			}
		}
		if (remainder > 0) {
			return compareValues(string1 + (vectorSize * intervalCount), string2 + (vectorSize * intervalCount), remainder);
		}
		return true;
	}

	template<typename ValueType01, typename ValueType02> inline bool compareValues32(const ValueType01* string1, const ValueType02* string2, uint64_t length) {
		static constexpr uint64_t vectorSize = sizeof(__m256i) / sizeof(ValueType01);
		const uint64_t intervalCount		 = getIntervalCount<ValueType01, __m256i>(length);
		const uint64_t remainder			 = length % vectorSize;
		static constexpr uint32_t maskValue{ 0xffffffff };
		auto destVector	  = gatherValues256<ValueType01>(string1);
		auto sourceVector = gatherValues256<ValueType02>(string2);
		if (_mm256_movemask_epi8(_mm256_cmpeq_epi8(destVector, sourceVector)) != maskValue) {
			return false;
		}
		for (uint64_t x = 1; x < intervalCount; ++x) {
			destVector	 = gatherValues256<ValueType01>(string1 + x * vectorSize);
			sourceVector = gatherValues256<ValueType02>(string2 + x * vectorSize);
			if (_mm256_movemask_epi8(_mm256_cmpeq_epi8(destVector, sourceVector)) != maskValue) {
				return false;
			}
		}
		if (remainder > 0) {
			return compareValues(string1 + (vectorSize * intervalCount), string2 + (vectorSize * intervalCount), remainder);
		}
		return true;
	}

	template<FloatingT ValueType01, FloatingT ValueType02> inline bool compareValues64(const ValueType01* string1, const ValueType02* string2, uint64_t length) {
		static constexpr uint64_t vectorSize = sizeof(__m512i) / sizeof(ValueType01);
		const uint64_t intervalCount		 = getIntervalCount<ValueType01, __m512i>(length);
		const uint64_t remainder			 = length % vectorSize;
		static constexpr uint64_t maskValue{ 0xffffffffffffffff };
		auto destVector	  = gatherValues512<ValueType01>(string1);
		auto sourceVector = gatherValues512<ValueType02>(string2);
		if (_mm512_cmpeq_epi8_mask(destVector, sourceVector) != maskValue) {
			return false;
		}
		for (uint64_t x = 1; x < intervalCount; ++x) {
			destVector	 = gatherValues512<ValueType01>(string1 + x * vectorSize);
			sourceVector = gatherValues512<ValueType02>(string2 + x * vectorSize);
			if (_mm512_cmpeq_epi8_mask(destVector, sourceVector) != maskValue) {
				return false;
			}
		}
		if (remainder > 0) {
			return compareValues(string1 + (vectorSize * intervalCount), string2 + (vectorSize * intervalCount), remainder);
		}
		return true;
	}

	template<typename ValueType01, typename ValueType02> inline bool compareValues64(const ValueType01* string1, const ValueType02* string2, uint64_t length) {
		static constexpr uint64_t vectorSize = sizeof(__m512i) / sizeof(ValueType01);
		const uint64_t intervalCount		 = getIntervalCount<ValueType01, __m512i>(length);
		const uint64_t remainder			 = length % vectorSize;
		static constexpr uint64_t maskValue{ 0xffffffffffffffff };
		auto destVector	  = gatherValues512<ValueType01>(string1);
		auto sourceVector = gatherValues512<ValueType02>(string2);
		if (_mm512_cmpeq_epi8_mask(destVector, sourceVector) != maskValue) {
			return false;
		}
		for (uint64_t x = 1; x < intervalCount; ++x) {
			destVector	 = gatherValues512<ValueType01>(string1 + x * vectorSize);
			sourceVector = gatherValues512<ValueType02>(string2 + x * vectorSize);
			if (_mm512_cmpeq_epi8_mask(destVector, sourceVector) != maskValue) {
				return false;
			}
		}
		if (remainder > 0) {
			return compareValues(string1 + (vectorSize * intervalCount), string2 + (vectorSize * intervalCount), remainder);
		}
		return true;
	}

	template<typename ValueType01, typename ValueType02> inline bool compareValues(const ValueType01* string1, const ValueType02* string2, uint64_t length) {
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
		alignas(ALIGNMENT) float newArray[sizeof(__m128i) / sizeof(float)]{};
		std::memcpy(newArray, str, sizeof(__m128i));
		return _mm_castps_si128(_mm_load_ps(newArray));
	}

	template<typename ValueType> inline __m256i gatherValues256(const ValueType* str) {
		alignas(ALIGNMENT) float newArray[sizeof(__m256i) / sizeof(float)]{};
		std::memcpy(newArray, str, sizeof(__m256i));
		return _mm256_castps_si256(_mm256_load_ps(newArray));
	}

	template<FloatingT ValueType> inline __m128 gatherValues128(const ValueType* str) {
		return _mm_load_ps(str);
	}

	template<FloatingT ValueType> inline __m256 gatherValues256(const ValueType* str) {
		return _mm256_load_ps(str);
	}

	template<typename ValueType01, typename ValueType02> inline uint64_t findFirstCharacterNotEqual(const ValueType01* str, uint64_t length, ValueType02 target) {
		static constexpr uint32_t vecSize = sizeof(__m256);
		const __m256i targetVec			  = _mm256_set1_epi8(target);
		uint64_t remainingBytes			  = length;
		__m256i currentVec{};
		uint64_t index{};
		uint32_t mask{};

		while (remainingBytes >= vecSize) {
			currentVec = gatherValues256<ValueType01>(str);
			mask	   = _mm256_movemask_epi8(_mm256_cmpeq_epi8(targetVec, currentVec));

			if (mask != 0xffffffff) {
				uint32_t firstNonMatchMask = ~mask;
				return index + _tzcnt_u32(firstNonMatchMask);
			}

			str += vecSize;
			index += vecSize;
			remainingBytes -= vecSize;
		}

		for (uint64_t i = 0; i < remainingBytes; i++) {
			if (str[i] != target) {
				return index + i;
			}
		}

		return std::numeric_limits<uint64_t>::max();
	}


	template<typename ValueType01, typename ValueType02> inline uint64_t findSingleCharacter(const ValueType01* str, uint64_t length, ValueType02 target) {
		static constexpr uint32_t vecSize = sizeof(__m256);
		const __m256i targetVec			  = _mm256_set1_epi8(target);
		uint64_t remainingBytes{ length };
		__m256i currentVec{};
		uint64_t index{};
		uint32_t mask{};
		while (remainingBytes >= vecSize) {
			currentVec = gatherValues256<ValueType01>(str);
			mask	   = _mm256_movemask_epi8(_mm256_cmpeq_epi8(targetVec, currentVec));

			if (mask != 0) {
				return index + _tzcnt_u32(mask);
			}

			str += vecSize;
			index += vecSize;
			remainingBytes -= vecSize;
		}

		for (uint64_t i = 0; i < remainingBytes; i++) {
			if (str[i] == target) {
				return index + i;
			}
		}

		return std::numeric_limits<uint64_t>::max();
	}

	template<typename ValueType01, typename ValueType02> inline uint64_t findLastSingleCharacter(const ValueType01* str, uint64_t length, ValueType02 target) {
		static constexpr uint32_t vecSize = sizeof(__m256);
		const __m256i targetVec			  = _mm256_set1_epi8(target);
		uint64_t remainingBytes{};
		__m256i currentVec{};
		uint64_t index{ length };
		uint32_t mask{};
		str += length - vecSize;
		while (remainingBytes >= 0) {
			currentVec = gatherValues256<ValueType01>(str);
			mask	   = _mm256_movemask_epi8(_mm256_cmpeq_epi8(targetVec, currentVec));

			if (mask != 0) {
				return index - _lzcnt_u32(mask);
			}

			str -= vecSize;
			index -= vecSize;
			remainingBytes -= vecSize;
		}

		for (uint64_t i = remainingBytes; i > 0; --i) {
			if (str[i] == target) {
				return index + i;
			}
		}
		return std::numeric_limits<uint64_t>::max();
	}

	template<typename ValueType01, typename ValueType02> inline bool compareValues(const ValueType01* string1, const ValueType02* string2, uint64_t length);

	template<typename ValueType, typename IndexType> inline uint64_t getIntervalCount(uint64_t originalLength) {
		return originalLength * sizeof(ValueType) / sizeof(IndexType);
	}

	template<FloatingT ValueType01, FloatingT ValueType02> inline bool compareValues16(const ValueType01* string1, const ValueType02* string2, uint64_t length) {
		static constexpr uint64_t vectorSize = sizeof(__m128i) / sizeof(ValueType01);
		const uint64_t intervalCount		 = getIntervalCount<ValueType01, __m128i>(length);
		const uint64_t remainder			 = length % vectorSize;
		static constexpr uint16_t maskValue{ 0xffff };

		auto destVector	  = gatherValues128<ValueType01>(string1);
		auto sourceVector = gatherValues128<ValueType02>(string2);
		if (_mm_movemask_ps(_mm_cmpeq_ps(destVector, sourceVector)) != maskValue) {
			return false;
		}

		for (uint64_t x = 1; x < intervalCount; ++x) {
			destVector	 = gatherValues128<ValueType01>(string1 + x * vectorSize);
			sourceVector = gatherValues128<ValueType02>(string2 + x * vectorSize);
			if (_mm_movemask_ps(_mm_cmpeq_ps(destVector, sourceVector)) != maskValue) {
				return false;
			}
		}

		if (remainder > 0) {
			return compareValues(string1 + (vectorSize * intervalCount), string2 + (vectorSize * intervalCount), remainder);
		}

		return true;
	}

	template<typename ValueType01, typename ValueType02> inline bool compareValues16(const ValueType01* string1, const ValueType02* string2, uint64_t length) {
		static constexpr uint64_t vectorSize = sizeof(__m128i) / sizeof(ValueType01);
		const uint64_t intervalCount		 = getIntervalCount<ValueType01, __m128i>(length);
		const uint64_t remainder			 = length % vectorSize;
		static constexpr uint16_t maskValue{ 0xffff };
		auto destVector	  = gatherValues128<ValueType01>(string1);
		auto sourceVector = gatherValues128<ValueType02>(string2);
		if (_mm_movemask_epi8(_mm_cmpeq_epi8(destVector, sourceVector)) != maskValue) {
			return false;
		}
		for (uint64_t x = 1; x < intervalCount; ++x) {
			destVector	 = gatherValues128<ValueType01>(string1 + x * vectorSize);
			sourceVector = gatherValues128<ValueType02>(string2 + x * vectorSize);
			if (_mm_movemask_epi8(_mm_cmpeq_epi8(destVector, sourceVector)) != maskValue) {
				return false;
			}
		}
		if (remainder > 0) {
			return compareValues(string1 + (vectorSize * intervalCount), string2 + (vectorSize * intervalCount), remainder);
		}
		return true;
	}

	template<FloatingT ValueType01, FloatingT ValueType02> inline bool compareValues32(const ValueType01* string1, const ValueType02* string2, uint64_t length) {
		static constexpr uint64_t vectorSize = sizeof(__m256i) / sizeof(ValueType01);
		const uint64_t intervalCount		 = getIntervalCount<ValueType01, __m256i>(length);
		const uint64_t remainder			 = length % vectorSize;
		static constexpr uint32_t maskValue{ 0xffffffff };
		auto destVector	  = gatherValues256<ValueType01>(string1);
		auto sourceVector = gatherValues256<ValueType02>(string2);
		if (_mm256_movemask_ps(_mm256_cmp_ps(destVector, sourceVector, _CMP_EQ_OQ)) != maskValue) {
			return false;
		}
		for (uint64_t x = 1; x < intervalCount; ++x) {
			destVector	 = gatherValues256<ValueType01>(string1 + x * vectorSize);
			sourceVector = gatherValues256<ValueType02>(string2 + x * vectorSize);
			if (_mm256_movemask_ps(_mm256_cmp_ps(destVector, sourceVector, _CMP_EQ_OQ)) != maskValue) {
				return false;
			}
		}
		if (remainder > 0) {
			return compareValues(string1 + (vectorSize * intervalCount), string2 + (vectorSize * intervalCount), remainder);
		}
		return true;
	}

	template<typename ValueType01, typename ValueType02> inline bool compareValues32(const ValueType01* string1, const ValueType02* string2, uint64_t length) {
		static constexpr uint64_t vectorSize = sizeof(__m256i) / sizeof(ValueType01);
		const uint64_t intervalCount		 = getIntervalCount<ValueType01, __m256i>(length);
		const uint64_t remainder			 = length % vectorSize;
		static constexpr uint32_t maskValue{ 0xffffffff };
		auto destVector	  = gatherValues256<ValueType01>(string1);
		auto sourceVector = gatherValues256<ValueType02>(string2);
		if (_mm256_movemask_epi8(_mm256_cmpeq_epi8(destVector, sourceVector)) != static_cast<int32_t>(maskValue)) {
			return false;
		}
		for (uint64_t x = 1; x < intervalCount; ++x) {
			destVector	 = gatherValues256<ValueType01>(string1 + x * vectorSize);
			sourceVector = gatherValues256<ValueType02>(string2 + x * vectorSize);
			if (_mm256_movemask_epi8(_mm256_cmpeq_epi8(destVector, sourceVector)) != static_cast<int32_t>(maskValue)) {
				return false;
			}
		}
		if (remainder > 0) {
			return compareValues(string1 + (vectorSize * intervalCount), string2 + (vectorSize * intervalCount), remainder);
		}
		return true;
	}

	template<typename ValueType01, typename ValueType02> inline bool compareValues(const ValueType01* string1, const ValueType02* string2, uint64_t length) {
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
		alignas(ALIGNMENT) float newArray[sizeof(__m128i) / sizeof(float)]{};
		std::memcpy(newArray, str, sizeof(__m128i));
		return _mm_castps_si128(_mm_load_ps(newArray));
	}

	template<typename ValueType01, typename ValueType02> inline uint64_t findFirstCharacterNotEqual(const ValueType01* str, uint64_t length, ValueType02 target) {
		static constexpr uint32_t vecSize = sizeof(__m128);
		const __m128i targetVec			  = _mm_set1_epi8(target);
		uint64_t remainingBytes			  = length;
		__m128i currentVec{};
		uint64_t index{};
		uint32_t mask{};

		while (remainingBytes >= vecSize) {
			currentVec = gatherValues128<ValueType01>(str);
			mask	   = _mm_movemask_epi8(_mm_cmpeq_epi8(targetVec, currentVec));

			if (mask != 0xffff) {
				uint32_t firstNonMatchMask = ~mask;
				return index + _tzcnt_u16(firstNonMatchMask);
			}

			str += vecSize;
			index += vecSize;
			remainingBytes -= vecSize;
		}

		for (uint64_t i = 0; i < remainingBytes; i++) {
			if (str[i] != target) {
				return index + i;
			}
		}

		return std::numeric_limits<uint64_t>::max();
	}


	template<typename ValueType01, typename ValueType02> inline uint64_t findSingleCharacter(const ValueType01* str, uint64_t length, ValueType02 target) {
		static constexpr uint32_t vecSize = sizeof(__m128);
		const __m128i targetVec			  = _mm_set1_epi8(target);
		uint64_t remainingBytes{ length };
		__m128i currentVec{};
		uint64_t index{};
		uint32_t mask{};
		while (remainingBytes >= vecSize) {
			currentVec = gatherValues128<ValueType01>(str);
			mask	   = _mm_movemask_epi8(_mm_cmpeq_epi8(targetVec, currentVec));

			if (mask != 0) {
				return index + _tzcnt_u16(mask);
			}

			str += vecSize;
			index += vecSize;
			remainingBytes -= vecSize;
		}

		for (uint64_t i = 0; i < remainingBytes; i++) {
			if (str[i] == target) {
				return index + i;
			}
		}

		return std::numeric_limits<uint64_t>::max();
	}

	template<typename ValueType01, typename ValueType02> inline uint64_t findLastSingleCharacter(const ValueType01* str, uint64_t length, ValueType02 target) {
		static constexpr uint32_t vecSize = sizeof(__m128);
		const __m128i targetVec			  = _mm_set1_epi8(target);
		uint64_t remainingBytes{};
		__m128i currentVec{};
		uint64_t index{ length };
		uint32_t mask{};
		str += length - vecSize;
		while (remainingBytes >= 0) {
			currentVec = gatherValues128<ValueType01>(str);
			mask	   = _mm_movemask_epi8(_mm_cmpeq_epi8(targetVec, currentVec));

			if (mask != 0) {
				return index - _lzcnt_u32(mask);
			}

			str -= vecSize;
			index -= vecSize;
			remainingBytes -= vecSize;
		}

		for (uint64_t i = remainingBytes; i > 0; --i) {
			if (str[i] == target) {
				return index + i;
			}
		}
		return std::numeric_limits<uint64_t>::max();
	}

	template<typename ValueType01, typename ValueType02> inline bool compareValues(const ValueType01* string1, const ValueType02* string2, uint64_t length) {
		return (std::memcmp(string1, string2, length * sizeof(ValueType01)) == 0);
	}

#else

	struct __m128I {
		inline __m128I() noexcept = default;
		inline __m128I(__m128I&& valueNew) {
			*this = std::move(valueNew);
		}

		inline __m128I& operator=(__m128I&& valueNew) {
			values[0] = valueNew.values[0];
			values[1] = valueNew.values[1];
			return *this;
		}

		inline __m128I(const __m128I& valueNew) {
			*this = std::move(valueNew);
		}

		inline __m128I& operator=(const __m128I& valueNew) {
			values[0] = valueNew.values[0];
			values[1] = valueNew.values[1];
			return *this;
		}

		uint64_t values[2]{};
	};

	template<typename ValueType> inline __m128I gatherValues128(const ValueType* str) {
		alignas(ALIGNMENT) __m128I newArray{};
		std::memcpy(&newArray, str, sizeof(__m128I));
		return newArray;
	}

	template<typename ValueType01, typename ValueType02> inline uint64_t findFirstCharacterNotEqual(const ValueType01* str, uint64_t length, ValueType02 target) {
		return std::basic_string_view<ValueType01>{ static_cast<const ValueType01*>(str), length }.find_first_not_of(static_cast<ValueType01>(target));
	}

	template<typename ValueType01, typename ValueType02> inline uint64_t findSingleCharacter(const ValueType01* str, uint64_t length, ValueType02 target) {
		return std::basic_string_view<ValueType01>{ static_cast<const ValueType01*>(str), length }.find(static_cast<ValueType01>(target));
	}

	template<typename ValueType01, typename ValueType02> inline uint64_t findLastSingleCharacter(const ValueType01* str, uint64_t length, ValueType02 target) {
		return std::basic_string_view<ValueType01>{ static_cast<const ValueType01*>(str), length }.find_last_of(static_cast<ValueType01>(target));
	}

	template<typename ValueType01, typename ValueType02> inline bool compareValues(const void* destVector, const void* sourceVector, uint64_t length) {
		return std::basic_string_view<std::remove_pointer_t<ValueType01>>{ static_cast<const ValueType01*>(destVector), length } ==
			std::basic_string_view<std::remove_pointer_t<ValueType01>>{ static_cast<const ValueType01*>(sourceVector), length };
	}

#endif

	class JsonifierCoreInternal {
	  public:
		template<typename ValueType01, typename ValueType02> static inline bool compare(const ValueType01* destVector, const ValueType02* sourceVector, uint64_t length) {
			return compareValues<ValueType01, ValueType02>(destVector, sourceVector, length);
		}
	};

}// namespace JsonifierInternal