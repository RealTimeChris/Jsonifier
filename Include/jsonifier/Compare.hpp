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
/// Feb 3, 2023
#pragma once

#include <jsonifier/ISADetection.hpp>

namespace jsonifier_internal {

#if JSONIFIER_CHECK_FOR_INSTRUCTION(JSONIFIER_AVX512)

	template<typename value_type01, typename value_type02> inline uint64_t findFirstNotOf(const value_type01* str, uint64_t length, value_type02 target) {
		static constexpr uint64_t vecSize = sizeof(__m512);
		const __m512i targetVec			  = _mm512_set1_epi8(target);
		uint64_t remainingBytes{ length };
		__m512i currentVec{};
		uint64_t index{};

		while (remainingBytes >= vecSize) {
			currentVec	   = gatherValues512(str);
			__mmask64 mask = _mm512_cmpeq_epi8_mask(targetVec, currentVec);

			if (mask != 0xffffffffffffffff) {
				uint64_t firstNonMatchMask = ~mask;
				return index + tzCount(firstNonMatchMask);
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

	template<typename value_type01, typename value_type02> inline uint64_t findFirstNotOf(const value_type01* str, uint64_t length, const value_type02* target) {
		static constexpr uint64_t vecSize = sizeof(__m512);
		uint64_t remainingBytes{ length };
		uint64_t index{};
		__mmask64 mask{};

		if (target[0] == '\0') {
			return std::numeric_limits<uint64_t>::max();
		}

		while (remainingBytes >= vecSize) {
			__m512i currentVec = gatherValues512(str);
			mask			   = _mm512_cmp_epu8_mask(currentVec, _mm512_set1_epi8(target[0]), _MM_CMPINT_EQ);

			if (mask != 0xffffffffffffffff) {
				uint64_t firstMatchMask = mask;
				return index + tzCount(firstMatchMask);
			}

			str += vecSize;
			index += vecSize;
			remainingBytes -= vecSize;
		}

		for (uint64_t i = 0; i < remainingBytes; i++) {
			bool found = false;
			for (uint64_t j = 0; target[j] != '\0'; ++j) {
				if (str[i] == target[j]) {
					found = true;
					break;
				}
			}

			if (!found) {
				return index + i;
			}
		}

		return std::numeric_limits<uint64_t>::max();
	}

	template<typename value_type01, typename value_type02> inline uint64_t findFirstOf(const value_type01* str, uint64_t length, value_type02 target) {
		static constexpr uint64_t vecSize = sizeof(__m512);
		const __m512i targetVec			  = _mm512_set1_epi8(target);
		uint64_t remainingBytes{ length };
		__m512i currentVec{};
		uint64_t index{};
		uint64_t mask{};

		while (remainingBytes >= vecSize) {
			currentVec = gatherValues512(str);
			mask	   = _mm512_cmpeq_epi8_mask(targetVec, currentVec);

			if (mask != 0) {
				return index + tzCount(mask);
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

	template<typename value_type01, typename value_type02> inline uint64_t findFirstOf(const value_type01* str, uint64_t length, const value_type02* targets) {
		static constexpr uint64_t vecSize = sizeof(__m512);
		uint64_t remainingBytes{ length };
		__m512i currentVec{};
		uint64_t index{};
		uint64_t mask{};

		if (targets[0] == '\0') {
			return std::numeric_limits<uint64_t>::max();
		}

		while (remainingBytes >= vecSize) {
			currentVec = gatherValues512(str);
			mask	   = _mm512_cmp_epu8_mask(currentVec, _mm512_set1_epi8(targets[0]), _MM_CMPINT_EQ);

			if (mask != 0) {
				return index + tzCount(mask);
			}

			str += vecSize;
			index += vecSize;
			remainingBytes -= vecSize;
		}

		for (uint64_t i = 0; i < remainingBytes; i++) {
			for (uint64_t j = 0; targets[j] != '\0'; ++j) {
				if (str[i] == targets[j]) {
					return index + i;
				}
			}
		}

		return std::numeric_limits<uint64_t>::max();
	}

	template<typename value_type01, typename value_type02> inline uint64_t findLastOf(const value_type01* str, uint64_t length, value_type02 target) {
		static constexpr uint64_t vecSize = sizeof(__m512);
		const __m512i targetVec			  = _mm512_set1_epi8(target);
		uint64_t index{ length };
		uint64_t mask{};

		while (index >= vecSize) {
			index -= vecSize;
			str -= vecSize;
			__m512i currentVec = gatherValues512(str);
			mask			   = _mm512_cmpeq_epi8_mask(targetVec, currentVec);

			if (mask != 0) {
				return index + lzCount(mask);
			}
		}

		for (uint64_t i = index; i > 0; --i) {
			if (str[i - 1] == target) {
				return i - 1;
			}
		}

		return std::numeric_limits<uint64_t>::max();
	}

	template<typename value_type01, typename value_type02> inline uint64_t findLastOf(const value_type01* str, uint64_t length, const value_type02* targets) {
		static constexpr uint64_t vecSize = sizeof(__m512);
		uint64_t index{ length };
		uint64_t mask{};

		if (targets[0] == '\0') {
			return std::numeric_limits<uint64_t>::max();
		}

		while (index >= vecSize) {
			index -= vecSize;
			str -= vecSize;
			__m512i currentVec = gatherValues512(str);
			mask			   = _mm512_cmp_epu8_mask(currentVec, _mm512_set1_epi8(targets[0]), _MM_CMPINT_EQ);

			if (mask != 0) {
				return index + lzCount(mask);
			}
		}

		for (uint64_t i = index; i > 0; --i) {
			for (uint64_t j = 0; targets[j] != '\0'; ++j) {
				if (str[i - 1] == targets[j]) {
					return i - 1;
				}
			}
		}

		return std::numeric_limits<uint64_t>::max();
	}

	template<typename value_type01, typename value_type02> inline uint64_t find(const value_type01* str, uint64_t length, const value_type02* sub, uint64_t subLength = 1) {
		static constexpr uint64_t vecSize = sizeof(__m512);
		uint64_t remainingBytes{ length };
		uint64_t index{};

		if (subLength == 0) {
			return 0;
		}

		while (remainingBytes >= vecSize) {
			__m512i currentVec = gatherValues512(str);

			for (uint64_t i = 0; i < subLength; ++i) {
				const __m512i subVec = _mm512_set1_epi8(sub[i]);
				uint64_t mask		 = _mm512_cmpeq_epi8_mask(subVec, currentVec);
				if (mask != 0) {
					uint64_t pos = index + tzCount(mask);
					if (strncmp(str + pos, sub, subLength) == 0) {
						return pos;
					}
				}
			}

			str += vecSize;
			index += vecSize;
			remainingBytes -= vecSize;
		}


		for (uint64_t i = 0; i <= remainingBytes - subLength; i++) {
			bool found = true;

			for (uint64_t j = 0; j < subLength; ++j) {
				if (str[i + j] != sub[j]) {
					found = false;
					break;
				}
			}

			if (found) {
				return index + i;
			}
		}

		return std::numeric_limits<uint64_t>::max();
	}

	template<typename value_type01, typename value_type02> inline bool compareValues(const value_type01* string1, const value_type02* string2, uint64_t length);

	template<typename value_type, typename IndexType> inline uint64_t getIntervalCount(uint64_t originalLength) {
		return originalLength * sizeof(value_type) / sizeof(IndexType);
	}

	template<float_t value_type01, float_t value_type02> inline bool compareValues16(const value_type01* string1, const value_type02* string2, uint64_t length) {
		static constexpr uint64_t vectorSize = sizeof(__m128i) / sizeof(value_type01);
		const uint64_t intervalCount		 = getIntervalCount<value_type01, __m128i>(length);
		const uint64_t remainder			 = length % vectorSize;
		static constexpr uint16_t maskValue{ 0xffff };

		auto destvector	  = gatherValues128(string1);
		auto sourcevector = gatherValues128(string2);
		if (_mm_movemask_ps(_mm_cmpeq_ps(destvector, sourcevector)) != maskValue) {
			return false;
		}

		for (uint64_t x = 1; x < intervalCount; ++x) {
			destvector	 = gatherValues128(string1 + x * vectorSize);
			sourcevector = gatherValues128(string2 + x * vectorSize);
			if (_mm_movemask_ps(_mm_cmpeq_ps(destvector, sourcevector)) != maskValue) {
				return false;
			}
		}

		if (remainder > 0) {
			return compareValues(string1 + (vectorSize * intervalCount), string2 + (vectorSize * intervalCount), remainder);
		}

		return true;
	}

	template<typename value_type01, typename value_type02> inline bool compareValues16(const value_type01* string1, const value_type02* string2, uint64_t length) {
		static constexpr uint64_t vectorSize = sizeof(__m128i) / sizeof(value_type01);
		const uint64_t intervalCount		 = getIntervalCount<value_type01, __m128i>(length);
		const uint64_t remainder			 = length % vectorSize;
		static constexpr uint16_t maskValue{ 0xffff };
		auto destvector	  = gatherValues128(string1);
		auto sourcevector = gatherValues128(string2);
		if (_mm_movemask_epi8(_mm_cmpeq_epi8(destvector, sourcevector)) != maskValue) {
			return false;
		}
		for (uint64_t x = 1; x < intervalCount; ++x) {
			destvector	 = gatherValues128(string1 + x * vectorSize);
			sourcevector = gatherValues128(string2 + x * vectorSize);
			if (_mm_movemask_epi8(_mm_cmpeq_epi8(destvector, sourcevector)) != maskValue) {
				return false;
			}
		}
		if (remainder > 0) {
			return compareValues(string1 + (vectorSize * intervalCount), string2 + (vectorSize * intervalCount), remainder);
		}
		return true;
	}

	template<float_t value_type01, float_t value_type02> inline bool compareValues32(const value_type01* string1, const value_type02* string2, uint64_t length) {
		static constexpr uint64_t vectorSize = sizeof(__m256i) / sizeof(value_type01);
		const uint64_t intervalCount		 = getIntervalCount<value_type01, __m256i>(length);
		const uint64_t remainder			 = length % vectorSize;
		static constexpr uint32_t maskValue{ 0xffffffff };
		auto destvector	  = gatherValues256(string1);
		auto sourcevector = gatherValues256(string2);
		if (_mm256_movemask_ps(_mm256_cmp_ps(destvector, sourcevector, _CMP_EQ_OQ)) != maskValue) {
			return false;
		}
		for (uint64_t x = 1; x < intervalCount; ++x) {
			destvector	 = gatherValues256(string1 + x * vectorSize);
			sourcevector = gatherValues256(string2 + x * vectorSize);
			if (_mm256_movemask_ps(_mm256_cmp_ps(destvector, sourcevector, _CMP_EQ_OQ)) != maskValue) {
				return false;
			}
		}
		if (remainder > 0) {
			return compareValues(string1 + (vectorSize * intervalCount), string2 + (vectorSize * intervalCount), remainder);
		}
		return true;
	}

	template<typename value_type01, typename value_type02> inline bool compareValues32(const value_type01* string1, const value_type02* string2, uint64_t length) {
		static constexpr uint64_t vectorSize = sizeof(__m256i) / sizeof(value_type01);
		const uint64_t intervalCount		 = getIntervalCount<value_type01, __m256i>(length);
		const uint64_t remainder			 = length % vectorSize;
		static constexpr uint32_t maskValue{ 0xffffffff };
		auto destvector	  = gatherValues256(string1);
		auto sourcevector = gatherValues256(string2);
		if (_mm256_movemask_epi8(_mm256_cmpeq_epi8(destvector, sourcevector)) != maskValue) {
			return false;
		}
		for (uint64_t x = 1; x < intervalCount; ++x) {
			destvector	 = gatherValues256(string1 + x * vectorSize);
			sourcevector = gatherValues256(string2 + x * vectorSize);
			if (_mm256_movemask_epi8(_mm256_cmpeq_epi8(destvector, sourcevector)) != maskValue) {
				return false;
			}
		}
		if (remainder > 0) {
			return compareValues(string1 + (vectorSize * intervalCount), string2 + (vectorSize * intervalCount), remainder);
		}
		return true;
	}

	template<float_t value_type01, float_t value_type02> inline bool compareValues64(const value_type01* string1, const value_type02* string2, uint64_t length) {
		static constexpr uint64_t vectorSize = sizeof(__m512i) / sizeof(value_type01);
		const uint64_t intervalCount		 = getIntervalCount<value_type01, __m512i>(length);
		const uint64_t remainder			 = length % vectorSize;
		static constexpr uint64_t maskValue{ 0xffffffffffffffff };
		auto destvector	  = gatherValues512(string1);
		auto sourcevector = gatherValues512(string2);
		if (_mm512_cmpeq_epi8_mask(destvector, sourcevector) != maskValue) {
			return false;
		}
		for (uint64_t x = 1; x < intervalCount; ++x) {
			destvector	 = gatherValues512(string1 + x * vectorSize);
			sourcevector = gatherValues512(string2 + x * vectorSize);
			if (_mm512_cmpeq_epi8_mask(destvector, sourcevector) != maskValue) {
				return false;
			}
		}
		if (remainder > 0) {
			return compareValues(string1 + (vectorSize * intervalCount), string2 + (vectorSize * intervalCount), remainder);
		}
		return true;
	}

	template<typename value_type01, typename value_type02> inline bool compareValues64(const value_type01* string1, const value_type02* string2, uint64_t length) {
		static constexpr uint64_t vectorSize = sizeof(__m512i) / sizeof(value_type01);
		const uint64_t intervalCount		 = getIntervalCount<value_type01, __m512i>(length);
		const uint64_t remainder			 = length % vectorSize;
		static constexpr uint64_t maskValue{ 0xffffffffffffffff };
		auto destvector	  = gatherValues512(string1);
		auto sourcevector = gatherValues512(string2);
		if (_mm512_cmpeq_epi8_mask(destvector, sourcevector) != maskValue) {
			return false;
		}
		for (uint64_t x = 1; x < intervalCount; ++x) {
			destvector	 = gatherValues512(string1 + x * vectorSize);
			sourcevector = gatherValues512(string2 + x * vectorSize);
			if (_mm512_cmpeq_epi8_mask(destvector, sourcevector) != maskValue) {
				return false;
			}
		}
		if (remainder > 0) {
			return compareValues(string1 + (vectorSize * intervalCount), string2 + (vectorSize * intervalCount), remainder);
		}
		return true;
	}

	template<typename value_type01, typename value_type02> inline bool compareValues(const value_type01* string1, const value_type02* string2, uint64_t length) {
		if (length >= 64) {
			return compareValues64(string1, string2, length);
		} else if (length >= 32) {
			return compareValues32(string1, string2, length);
		} else if (length >= 16) {
			return compareValues16(string1, string2, length);
		} else {
			return (std::memcmp(string1, string2, length * sizeof(value_type01)) == 0);
		}
	}

#elif JSONIFIER_CHECK_FOR_INSTRUCTION(JSONIFIER_AVX2)

	template<typename value_type01, typename value_type02> inline uint64_t findFirstNotOf(const value_type01* str, uint64_t length, value_type02 target) {
		static constexpr uint64_t vecSize = sizeof(__m256);
		const __m256i targetVec			  = _mm256_set1_epi8(target);
		uint64_t remainingBytes{ length };
		__m256i currentVec{};
		uint64_t index{};

		while (remainingBytes >= vecSize) {
			currentVec	   = gatherValues256(str);
			__mmask32 mask = _mm256_movemask_epi8(_mm256_cmpeq_epi8(targetVec, currentVec));

			if (mask != 0xffffffff) {
				uint64_t firstNonMatchMask = ~mask;
				return index + tzCount(firstNonMatchMask);
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

	template<typename value_type01, typename value_type02> inline uint64_t findFirstNotOf(const value_type01* str, uint64_t length, const value_type02* target) {
		static constexpr uint64_t vecSize = sizeof(__m256);
		uint64_t remainingBytes{ length };
		uint64_t index{};
		__mmask32 mask{};

		if (target[0] == '\0') {
			return std::numeric_limits<uint64_t>::max();
		}

		while (remainingBytes >= vecSize) {
			__m256i currentVec = gatherValues256(str);
			mask			   = _mm256_movemask_epi8(_mm256_cmpeq_epi8(currentVec, _mm256_set1_epi8(target[0])));

			if (mask != 0xffffffff) {
				uint64_t firstMatchMask = mask;
				return index + tzCount(firstMatchMask);
			}

			str += vecSize;
			index += vecSize;
			remainingBytes -= vecSize;
		}

		for (uint64_t i = 0; i < remainingBytes; i++) {
			bool found = false;
			for (uint64_t j = 0; target[j] != '\0'; ++j) {
				if (str[i] == target[j]) {
					found = true;
					break;
				}
			}

			if (!found) {
				return index + i;
			}
		}

		return std::numeric_limits<uint64_t>::max();
	}

	template<typename value_type01, typename value_type02> inline uint64_t findFirstOf(const value_type01* str, uint64_t length, value_type02 target) {
		static constexpr uint64_t vecSize = sizeof(__m256);
		const __m256i targetVec			  = _mm256_set1_epi8(target);
		uint64_t remainingBytes{ length };
		__m256i currentVec{};
		uint64_t index{};
		uint32_t mask{};

		while (remainingBytes >= vecSize) {
			currentVec = gatherValues256(str);
			mask	   = _mm256_movemask_epi8(_mm256_cmpeq_epi8(targetVec, currentVec));

			if (mask != 0) {
				return index + tzCount(mask);
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

	template<typename value_type01, typename value_type02> inline uint64_t findFirstOf(const value_type01* str, uint64_t length, const value_type02* targets) {
		static constexpr uint64_t vecSize = sizeof(__m256);
		uint64_t remainingBytes{ length };
		__m256i currentVec{};
		uint64_t index{};
		uint32_t mask{};

		if (targets[0] == '\0') {
			return std::numeric_limits<uint64_t>::max();
		}

		while (remainingBytes >= vecSize) {
			currentVec = gatherValues256(str);
			mask	   = _mm256_movemask_epi8(_mm256_cmpeq_epi8(currentVec, _mm256_set1_epi8(targets[0])));

			if (mask != 0) {
				return index + tzCount(mask);
			}

			str += vecSize;
			index += vecSize;
			remainingBytes -= vecSize;
		}

		for (uint64_t i = 0; i < remainingBytes; i++) {
			for (uint64_t j = 0; targets[j] != '\0'; ++j) {
				if (str[i] == targets[j]) {
					return index + i;
				}
			}
		}

		return std::numeric_limits<uint64_t>::max();
	}

	template<typename value_type01, typename value_type02> inline uint64_t findLastOf(const value_type01* str, uint64_t length, value_type02 target) {
		static constexpr uint64_t vecSize = sizeof(__m256);
		const __m256i targetVec			  = _mm256_set1_epi8(target);
		uint64_t index{ length };
		uint32_t mask{};

		while (index >= vecSize) {
			index -= vecSize;
			str -= vecSize;
			__m256i currentVec = gatherValues256(str);
			mask			   = _mm256_movemask_epi8(_mm256_cmpeq_epi8(targetVec, currentVec));

			if (mask != 0) {
				return index + lzCount(mask);
			}
		}

		for (uint64_t i = index; i > 0; --i) {
			if (str[i - 1] == target) {
				return i - 1;
			}
		}

		return std::numeric_limits<uint64_t>::max();
	}

	template<typename value_type01, typename value_type02> inline uint64_t findLastOf(const value_type01* str, uint64_t length, const value_type02* targets) {
		static constexpr uint64_t vecSize = sizeof(__m256);
		uint64_t index{ length };
		uint32_t mask{};

		if (targets[0] == '\0') {
			return std::numeric_limits<uint64_t>::max();
		}

		while (index >= vecSize) {
			index -= vecSize;
			str -= vecSize;
			__m256i currentVec = gatherValues256(str);
			mask			   = _mm256_movemask_epi8(_mm256_cmpeq_epi8(currentVec, _mm256_set1_epi8(targets[0])));

			if (mask != 0) {
				return index + lzCount(mask);
			}
		}

		for (uint64_t i = index; i > 0; --i) {
			for (uint64_t j = 0; targets[j] != '\0'; ++j) {
				if (str[i - 1] == targets[j]) {
					return i - 1;
				}
			}
		}

		return std::numeric_limits<uint64_t>::max();
	}

	template<typename value_type01, typename value_type02> inline uint64_t find(const value_type01* str, uint64_t length, const value_type02* sub, uint64_t subLength = 1) {
		static constexpr uint64_t vecSize = sizeof(__m256);
		uint64_t remainingBytes{ length };
		uint64_t index{};

		if (subLength == 0) {
			return 0;
		}

		while (remainingBytes >= vecSize) {
			__m256i currentVec = gatherValues256(str);

			for (uint64_t i = 0; i < subLength; ++i) {
				const __m256i subVec = _mm256_set1_epi8(sub[i]);
				uint32_t mask		 = _mm256_movemask_epi8(_mm256_cmpeq_epi8(subVec, currentVec));
				if (mask != 0) {
					uint64_t pos = index + tzCount(mask);
					if (strncmp(str + pos, sub, subLength) == 0) {
						return pos;
					}
				}
			}

			str += vecSize;
			index += vecSize;
			remainingBytes -= vecSize;
		}


		for (uint64_t i = 0; i <= remainingBytes - subLength; i++) {
			bool found = true;

			for (uint64_t j = 0; j < subLength; ++j) {
				if (str[i + j] != sub[j]) {
					found = false;
					break;
				}
			}

			if (found) {
				return index + i;
			}
		}

		return std::numeric_limits<uint64_t>::max();
	}

	template<typename value_type01, typename value_type02> inline bool compareValues(const value_type01* string1, const value_type02* string2, uint64_t length);

	template<typename value_type, typename IndexType> inline uint64_t getIntervalCount(uint64_t originalLength) {
		return originalLength * sizeof(value_type) / sizeof(IndexType);
	}

	template<float_t value_type01, float_t value_type02> inline bool compareValues16(const value_type01* string1, const value_type02* string2, uint64_t length) {
		static constexpr uint64_t vectorSize = sizeof(__m128i) / sizeof(value_type01);
		const uint64_t intervalCount		 = getIntervalCount<value_type01, __m128i>(length);
		const uint64_t remainder			 = length % vectorSize;
		static constexpr uint16_t maskValue{ 0xffff };

		auto destvector	  = gatherValues128(string1);
		auto sourcevector = gatherValues128(string2);
		if (_mm_movemask_ps(_mm_cmpeq_ps(destvector, sourcevector)) != maskValue) {
			return false;
		}

		for (uint64_t x = 1; x < intervalCount; ++x) {
			destvector	 = gatherValues128(string1 + x * vectorSize);
			sourcevector = gatherValues128(string2 + x * vectorSize);
			if (_mm_movemask_ps(_mm_cmpeq_ps(destvector, sourcevector)) != maskValue) {
				return false;
			}
		}

		if (remainder > 0) {
			return compareValues(string1 + (vectorSize * intervalCount), string2 + (vectorSize * intervalCount), remainder);
		}

		return true;
	}

	template<typename value_type01, typename value_type02> inline bool compareValues16(const value_type01* string1, const value_type02* string2, uint64_t length) {
		static constexpr uint64_t vectorSize = sizeof(__m128i) / sizeof(value_type01);
		const uint64_t intervalCount		 = getIntervalCount<value_type01, __m128i>(length);
		const uint64_t remainder			 = length % vectorSize;
		static constexpr uint16_t maskValue{ 0xffff };
		auto destvector	  = gatherValues128(string1);
		auto sourcevector = gatherValues128(string2);
		if (_mm_movemask_epi8(_mm_cmpeq_epi8(destvector, sourcevector)) != maskValue) {
			return false;
		}
		for (uint64_t x = 1; x < intervalCount; ++x) {
			destvector	 = gatherValues128(string1 + x * vectorSize);
			sourcevector = gatherValues128(string2 + x * vectorSize);
			if (_mm_movemask_epi8(_mm_cmpeq_epi8(destvector, sourcevector)) != maskValue) {
				return false;
			}
		}
		if (remainder > 0) {
			return compareValues(string1 + (vectorSize * intervalCount), string2 + (vectorSize * intervalCount), remainder);
		}
		return true;
	}

	template<float_t value_type01, float_t value_type02> inline bool compareValues32(const value_type01* string1, const value_type02* string2, uint64_t length) {
		static constexpr uint64_t vectorSize = sizeof(__m256i) / sizeof(value_type01);
		const uint64_t intervalCount		 = getIntervalCount<value_type01, __m256i>(length);
		const uint64_t remainder			 = length % vectorSize;
		static constexpr uint32_t maskValue{ 0xffffffff };
		auto destvector	  = gatherValues256(string1);
		auto sourcevector = gatherValues256(string2);
		if (_mm256_movemask_ps(_mm256_cmp_ps(destvector, sourcevector, _CMP_EQ_OQ)) != maskValue) {
			return false;
		}
		for (uint64_t x = 1; x < intervalCount; ++x) {
			destvector	 = gatherValues256(string1 + x * vectorSize);
			sourcevector = gatherValues256(string2 + x * vectorSize);
			if (_mm256_movemask_ps(_mm256_cmp_ps(destvector, sourcevector, _CMP_EQ_OQ)) != maskValue) {
				return false;
			}
		}
		if (remainder > 0) {
			return compareValues(string1 + (vectorSize * intervalCount), string2 + (vectorSize * intervalCount), remainder);
		}
		return true;
	}

	template<typename value_type01, typename value_type02> inline bool compareValues32(const value_type01* string1, const value_type02* string2, uint64_t length) {
		static constexpr uint64_t vectorSize = sizeof(__m256i) / sizeof(value_type01);
		const uint64_t intervalCount		 = getIntervalCount<value_type01, __m256i>(length);
		const uint64_t remainder			 = length % vectorSize;
		static constexpr uint32_t maskValue{ 0xffffffff };
		auto destvector	  = gatherValues256(string1);
		auto sourcevector = gatherValues256(string2);
		if (_mm256_movemask_epi8(_mm256_cmpeq_epi8(destvector, sourcevector)) != static_cast<int32_t>(maskValue)) {
			return false;
		}
		for (uint64_t x = 1; x < intervalCount; ++x) {
			destvector	 = gatherValues256(string1 + x * vectorSize);
			sourcevector = gatherValues256(string2 + x * vectorSize);
			if (_mm256_movemask_epi8(_mm256_cmpeq_epi8(destvector, sourcevector)) != static_cast<int32_t>(maskValue)) {
				return false;
			}
		}
		if (remainder > 0) {
			return compareValues(string1 + (vectorSize * intervalCount), string2 + (vectorSize * intervalCount), remainder);
		}
		return true;
	}

	template<typename value_type01, typename value_type02> inline bool compareValues(const value_type01* string1, const value_type02* string2, uint64_t length) {
		if (length >= 32) {
			return compareValues32(string1, string2, length);
		} else if (length >= 16) {
			return compareValues16(string1, string2, length);
		} else {
			return (std::memcmp(string1, string2, length * sizeof(value_type01)) == 0);
		}
	}

#elif JSONIFIER_CHECK_FOR_INSTRUCTION(JSONIFIER_AVX)

	template<typename value_type01, typename value_type02> inline uint64_t findFirstNotOf(const value_type01* str, uint64_t length, value_type02 target) {
		static constexpr uint64_t vecSize = sizeof(__m128);
		const __m128i targetVec			  = _mm_set1_epi8(target);
		uint64_t remainingBytes{ length };
		__m128i currentVec{};
		uint64_t index{};

		while (remainingBytes >= vecSize) {
			currentVec	   = gatherValues128(str);
			__mmask32 mask = _mm_movemask_epi8(_mm_cmpeq_epi8(targetVec, currentVec));

			if (mask != 0xffff) {
				uint64_t firstNonMatchMask = ~mask;
				return index + tzCount(firstNonMatchMask);
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

	template<typename value_type01, typename value_type02> inline uint64_t findFirstNotOf(const value_type01* str, uint64_t length, const value_type02* target) {
		static constexpr uint64_t vecSize = sizeof(__m128);
		uint64_t remainingBytes{ length };
		uint64_t index{};
		__mmask32 mask{};

		if (target[0] == '\0') {
			return std::numeric_limits<uint64_t>::max();
		}

		while (remainingBytes >= vecSize) {
			__m128i currentVec = gatherValues128(str);
			mask			   = _mm_movemask_epi8(_mm_cmpeq_epi8(currentVec, _mm_set1_epi8(target[0])));

			if (mask != 0xffff) {
				uint64_t firstMatchMask = mask;
				return index + tzCount(firstMatchMask);
			}

			str += vecSize;
			index += vecSize;
			remainingBytes -= vecSize;
		}

		for (uint64_t i = 0; i < remainingBytes; i++) {
			bool found = false;
			for (uint64_t j = 0; target[j] != '\0'; ++j) {
				if (str[i] == target[j]) {
					found = true;
					break;
				}
			}

			if (!found) {
				return index + i;
			}
		}

		return std::numeric_limits<uint64_t>::max();
	}

	template<typename value_type01, typename value_type02> inline uint64_t findFirstOf(const value_type01* str, uint64_t length, value_type02 target) {
		static constexpr uint64_t vecSize = sizeof(__m128);
		const __m128i targetVec			  = _mm_set1_epi8(target);
		uint64_t remainingBytes{ length };
		__m128i currentVec{};
		uint64_t index{};
		uint32_t mask{};

		while (remainingBytes >= vecSize) {
			currentVec = gatherValues128(str);
			mask	   = _mm_movemask_epi8(_mm_cmpeq_epi8(targetVec, currentVec));

			if (mask != 0) {
				return index + tzCount(mask);
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

	template<typename value_type01, typename value_type02> inline uint64_t findFirstOf(const value_type01* str, uint64_t length, const value_type02* targets) {
		static constexpr uint64_t vecSize = sizeof(__m128);
		uint64_t remainingBytes{ length };
		__m128i currentVec{};
		uint64_t index{};
		uint32_t mask{};

		if (targets[0] == '\0') {
			return std::numeric_limits<uint64_t>::max();
		}

		while (remainingBytes >= vecSize) {
			currentVec = gatherValues128(str);
			mask	   = _mm_movemask_epi8(_mm_cmpeq_epi8(currentVec, _mm_set1_epi8(targets[0])));

			if (mask != 0) {
				return index + tzCount(mask);
			}

			str += vecSize;
			index += vecSize;
			remainingBytes -= vecSize;
		}

		for (uint64_t i = 0; i < remainingBytes; i++) {
			for (uint64_t j = 0; targets[j] != '\0'; ++j) {
				if (str[i] == targets[j]) {
					return index + i;
				}
			}
		}

		return std::numeric_limits<uint64_t>::max();
	}

	template<typename value_type01, typename value_type02> inline uint64_t findLastOf(const value_type01* str, uint64_t length, value_type02 target) {
		static constexpr uint64_t vecSize = sizeof(__m128);
		const __m128i targetVec			  = _mm_set1_epi8(target);
		uint64_t index{ length };
		uint32_t mask{};

		while (index >= vecSize) {
			index -= vecSize;
			str -= vecSize;
			__m128i currentVec = gatherValues128(str);
			mask			   = _mm_movemask_epi8(_mm_cmpeq_epi8(targetVec, currentVec));

			if (mask != 0) {
				return index + lzCount(mask);
			}
		}

		for (uint64_t i = index; i > 0; --i) {
			if (str[i - 1] == target) {
				return i - 1;
			}
		}

		return std::numeric_limits<uint64_t>::max();
	}

	template<typename value_type01, typename value_type02> inline uint64_t findLastOf(const value_type01* str, uint64_t length, const value_type02* targets) {
		static constexpr uint64_t vecSize = sizeof(__m128);
		uint64_t index{ length };
		uint32_t mask{};

		if (targets[0] == '\0') {
			return std::numeric_limits<uint64_t>::max();
		}

		while (index >= vecSize) {
			index -= vecSize;
			str -= vecSize;
			__m128i currentVec = gatherValues128(str);
			mask			   = _mm_movemask_epi8(_mm_cmpeq_epi8(currentVec, _mm_set1_epi8(targets[0])));

			if (mask != 0) {
				return index + lzCount(mask);
			}
		}

		for (uint64_t i = index; i > 0; --i) {
			for (uint64_t j = 0; targets[j] != '\0'; ++j) {
				if (str[i - 1] == targets[j]) {
					return i - 1;
				}
			}
		}

		return std::numeric_limits<uint64_t>::max();
	}

	template<typename value_type01, typename value_type02> inline uint64_t find(const value_type01* str, uint64_t length, const value_type02* sub, uint64_t subLength = 1) {
		static constexpr uint64_t vecSize = sizeof(__m128);
		uint64_t remainingBytes{ length };
		uint64_t index{};

		if (subLength == 0) {
			return 0;
		}

		while (remainingBytes >= vecSize) {
			__m128i currentVec = gatherValues128(str);

			for (uint64_t i = 0; i < subLength; ++i) {
				const __m128i subVec = _mm_set1_epi8(sub[i]);
				uint16_t mask		 = _mm_movemask_epi8(_mm_cmpeq_epi8(subVec, currentVec));
				if (mask != 0) {
					uint64_t pos = index + tzCount(mask);
					if (strncmp(str + pos, sub, subLength) == 0) {
						return pos;
					}
				}
			}

			str += vecSize;
			index += vecSize;
			remainingBytes -= vecSize;
		}


		for (uint64_t i = 0; i <= remainingBytes - subLength; i++) {
			bool found = true;

			for (uint64_t j = 0; j < subLength; ++j) {
				if (str[i + j] != sub[j]) {
					found = false;
					break;
				}
			}

			if (found) {
				return index + i;
			}
		}

		return std::numeric_limits<uint64_t>::max();
	}

	template<typename value_type01, typename value_type02> inline bool compareValues(const value_type01* string1, const value_type02* string2, uint64_t length) {
		return (std::memcmp(string1, string2, length * sizeof(value_type01)) == 0);
	}

#else

	template<typename value_type01, typename value_type02> inline uint64_t findFirstCharacterNotEqual(const value_type01* str, uint64_t length, value_type02 target) {
		return std::basic_string_view<value_type01>{ static_cast<const value_type01*>(str), length }.findFirstNotOf(static_cast<value_type01>(target));
	}

	template<typename value_type01, typename value_type02> inline uint64_t findSingleCharacter(const value_type01* str, uint64_t length, value_type02 target) {
		return std::basic_string_view<value_type01>{ static_cast<const value_type01*>(str), length }.find(static_cast<value_type01>(target));
	}

	template<typename value_type01, typename value_type02> inline uint64_t findLastSingleCharacter(const value_type01* str, uint64_t length, value_type02 target) {
		return std::basic_string_view<value_type01>{ static_cast<const value_type01*>(str), length }.findLastOf(static_cast<value_type01>(target));
	}

	template<typename value_type01, typename value_type02> inline bool compareValues(const void* destvector, const void* sourcevector, uint64_t length) {
		return std::basic_string_view<std::remove_pointer_t<value_type01>>{ static_cast<const value_type01*>(destvector), length } ==
			std::basic_string_view<std::remove_pointer_t<value_type01>>{ static_cast<const value_type01*>(sourcevector), length };
	}

#endif

	class jsonifier_core_internal {
	  public:
		template<typename value_type01, typename value_type02> inline static bool compare(const value_type01* destvector, const value_type02* sourcevector, uint64_t length) {
			return compareValues<value_type01, value_type02>(destvector, sourcevector, length);
		}
	};

}// namespace jsonifier_internal
