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

#if AVX_TYPE == 127

	inline uint64_t findSingleCharacter(const void* str, size_t length, char target) {
		__m512i targetVec = _mm512_set1_epi8(target);
		const __m512i* strVec = static_cast<const __m512i*>(str);
		const uint64_t vecSize = sizeof(__m512i);
		size_t remainingBytes = length;

		size_t index = 0;
		while (remainingBytes >= vecSize) {
			__m512i currentVec = _mm512_loadu_si512(strVec);
			uint64_t mask = _mm512_cmp_epi8_mask(targetVec, currentVec, _MM_CMPINT_EQ);

			if (mask != 0) {
				uint64_t matchingIndex = _tzcnt_u64(static_cast<uint64_t>(mask));
				return index + matchingIndex;
			}

			++strVec;
			index += vecSize;
			remainingBytes -= vecSize;
		}

		const char* remainingStr = reinterpret_cast<const char*>(strVec);
		for (size_t i = 0; i < remainingBytes; i++) {
			if (remainingStr[i] == target) {
				return index + i;
			}
		}

		return -1;
	}

	inline bool compareValues(const void* string1, const void* string2, size_t length) noexcept;

	inline bool compareValues16(const void* string1, const void* string2, size_t length) noexcept {
		const size_t remainder{ length % 16 };
		auto destVector = static_cast<const __m128i*>(string1);
		auto sourceVector = static_cast<const __m128i*>(string2);
		if (_mm_movemask_epi8(_mm_cmpeq_epi8(_mm_loadu_si128(destVector++), _mm_loadu_si128(sourceVector++))) != (0x0000ffff)) {
			return false;
		}
		if (remainder > 0) {
			return compareValues(destVector, sourceVector, remainder);
		}
		return true;
	}

	inline bool compareValues32(const void* string1, const void* string2, size_t length) noexcept {
		const size_t remainder{ length % 32 };
		auto destVector = static_cast<const __m256i*>(string1);
		auto sourceVector = static_cast<const __m256i*>(string2);
		if (_mm256_movemask_epi8(_mm256_cmpeq_epi8(_mm256_loadu_si256(destVector++), _mm256_loadu_si256(sourceVector++))) != (0xffffffff)) {
			return false;
		}
		if (remainder > 0) {
			return compareValues(destVector, sourceVector, remainder);
		}
		return true;
	}

	inline bool compareValues64(const void* string1, const void* string2, size_t length) noexcept {
		const size_t remainder{ length % 64 };
		auto destVector = static_cast<const __m512i*>(string1);
		auto sourceVector = static_cast<const __m512i*>(string2);
		if (_mm512_cmp_epi8_mask(_mm512_loadu_si512(destVector++), _mm512_loadu_si512(sourceVector++), _MM_CMPINT_EQ) != (0xffffffffffffffff)) {
			return false;
		}
		if (remainder > 0) {
			return compareValues(destVector, sourceVector, remainder);
		}
		return true;
	}

	inline bool compareValues128(const void* string1, const void* string2, size_t length) noexcept {
		const size_t remainder{ length % 128 };
		auto destVector = static_cast<const __m512i*>(string1);
		auto sourceVector = static_cast<const __m512i*>(string2);
		size_t result{ 0xffffffffffffffff };
		size_t cmp{ _mm512_cmp_epi8_mask(_mm512_loadu_si512(destVector++), _mm512_loadu_si512(sourceVector++), _MM_CMPINT_EQ) };
		result = result & cmp;
		cmp = _mm512_cmp_epi8_mask(_mm512_loadu_si512(destVector++), _mm512_loadu_si512(sourceVector++), _MM_CMPINT_EQ);
		result = result & cmp;
		if (result != (0xffffffffffffffff)) {
			return false;
		}
		if (remainder > 0) {
			return compareValues(destVector, sourceVector, remainder);
		}
		return true;
	}

	inline bool compareValues256(const void* string1, const void* string2, size_t length) noexcept {
		const size_t remainder{ length % 256 };
		auto destVector = static_cast<const __m512i*>(string1);
		auto sourceVector = static_cast<const __m512i*>(string2);
		size_t result{ std::numeric_limits<size_t>::max() };
		size_t cmp{ _mm512_cmp_epi8_mask(_mm512_loadu_si512(destVector++), _mm512_loadu_si512(sourceVector++), _MM_CMPINT_EQ) };
		result = result & cmp;
		cmp = _mm512_cmp_epi8_mask(_mm512_loadu_si512(destVector++), _mm512_loadu_si512(sourceVector++), _MM_CMPINT_EQ);
		result = result & cmp;
		cmp = _mm512_cmp_epi8_mask(_mm512_loadu_si512(destVector++), _mm512_loadu_si512(sourceVector++), _MM_CMPINT_EQ);
		result = result & cmp;
		cmp = _mm512_cmp_epi8_mask(_mm512_loadu_si512(destVector++), _mm512_loadu_si512(sourceVector++), _MM_CMPINT_EQ);
		result = result & cmp;
		if (result != (0xffffffffffffffff)) {
			return false;
		}
		if (remainder > 0) {
			return compareValues(destVector, sourceVector, remainder);
		}
		return true;
	}

	inline bool compareValues512(const void* string1, const void* string2, size_t length) noexcept {
		const size_t remainder{ length % 512 };
		size_t intervalCount{ length / 512 };
		auto destVector = static_cast<const __m512i*>(string1);
		auto sourceVector = static_cast<const __m512i*>(string2);
		size_t result{ std::numeric_limits<size_t>::max() };
		size_t cmp{};
		for (size_t x = intervalCount; x > 0; --x) {
			cmp = _mm512_cmp_epi8_mask(_mm512_loadu_si512(destVector++), _mm512_loadu_si512(sourceVector++), _MM_CMPINT_EQ);
			result = result & cmp;
			cmp = _mm512_cmp_epi8_mask(_mm512_loadu_si512(destVector++), _mm512_loadu_si512(sourceVector++), _MM_CMPINT_EQ);
			result = result & cmp;
			cmp = _mm512_cmp_epi8_mask(_mm512_loadu_si512(destVector++), _mm512_loadu_si512(sourceVector++), _MM_CMPINT_EQ);
			result = result & cmp;
			cmp = _mm512_cmp_epi8_mask(_mm512_loadu_si512(destVector++), _mm512_loadu_si512(sourceVector++), _MM_CMPINT_EQ);
			result = result & cmp;
			cmp = _mm512_cmp_epi8_mask(_mm512_loadu_si512(destVector++), _mm512_loadu_si512(sourceVector++), _MM_CMPINT_EQ);
			result = result & cmp;
			cmp = _mm512_cmp_epi8_mask(_mm512_loadu_si512(destVector++), _mm512_loadu_si512(sourceVector++), _MM_CMPINT_EQ);
			result = result & cmp;
			cmp = _mm512_cmp_epi8_mask(_mm512_loadu_si512(destVector++), _mm512_loadu_si512(sourceVector++), _MM_CMPINT_EQ);
			result = result & cmp;
			cmp = _mm512_cmp_epi8_mask(_mm512_loadu_si512(destVector++), _mm512_loadu_si512(sourceVector++), _MM_CMPINT_EQ);
			result = result & cmp;
			if (result != (0xffffffffffffffff)) {
				return false;
			}
		}
		if (remainder > 0) {
			return compareValues(destVector, sourceVector, remainder);
		}
		return true;
	}

	inline bool compareValues(const void* string1, const void* string2, size_t length) noexcept {
		if (length >= 512) {
			return compareValues512(string1, string2, length);
		} else if (length >= 256) {
			return compareValues256(string1, string2, length);
		} else if (length >= 128) {
			return compareValues128(string1, string2, length);
		} else if (length >= 64) {
			return compareValues64(string1, string2, length);
		} else if (length >= 32) {
			return compareValues32(string1, string2, length);
		} else if (length >= 16) {
			return compareValues16(string1, string2, length);
		} else {
			return (std::memcmp(string1, string2, length) == 0);
		}
	}

#elif AVX_TYPE == 126

	inline uint64_t findSingleCharacter(const void* str, size_t length, char target) {
		__m256i targetVec = _mm256_set1_epi8(target);
		const __m256i* strVec = static_cast<const __m256i*>(str);
		const uint32_t vecSize = sizeof(__m256i);
		size_t remainingBytes = length;

		size_t index = 0;
		while (remainingBytes >= vecSize) {
			__m256i currentVec = _mm256_loadu_si256(strVec);
			__m256i compareResult = _mm256_cmpeq_epi8(targetVec, currentVec);
			uint32_t mask = _mm256_movemask_epi8(compareResult);


			if (mask != 0) {
				uint32_t matchingIndex = _tzcnt_u32(static_cast<uint32_t>(mask));
				return index + matchingIndex;
			}

			++strVec;
			index += vecSize;
			remainingBytes -= vecSize;
		}

		const char* remainingStr = reinterpret_cast<const char*>(strVec);
		for (size_t i = 0; i < remainingBytes; i++) {
			if (remainingStr[i] == target) {
				return index + i;
			}
		}

		return -1;
	}

	inline bool compareValues(const void* string1, const void* string2, size_t length) noexcept;

	inline bool compareValues16(const void* string1, const void* string2, size_t length) noexcept {
		const size_t remainder{ length % 16 };
		auto destVector = static_cast<const __m128i*>(string1);
		auto sourceVector = static_cast<const __m128i*>(string2);
		if (_mm_movemask_epi8(_mm_cmpeq_epi8(_mm_loadu_si128(destVector++), _mm_loadu_si128(sourceVector++))) != (0x0000ffff)) {
			return false;
		}
		if (remainder > 0) {
			return compareValues(destVector, sourceVector, remainder);
		}
		return true;
	}

	inline bool compareValues32(const void* string1, const void* string2, size_t length) noexcept {
		const size_t remainder{ length % 32 };
		auto destVector = static_cast<const __m256i*>(string1);
		auto sourceVector = static_cast<const __m256i*>(string2);
		if (_mm256_movemask_epi8(_mm256_cmpeq_epi8(_mm256_loadu_si256(destVector++), _mm256_loadu_si256(sourceVector++))) != (0xffffffff)) {
			return false;
		}
		if (remainder > 0) {
			return compareValues(destVector, sourceVector, remainder);
		}
		return true;
	}

	inline bool compareValues64(const void* string1, const void* string2, size_t length) noexcept {
		const size_t remainder{ length % 64 };
		auto destVector = static_cast<const __m256i*>(string1);
		auto sourceVector = static_cast<const __m256i*>(string2);
		__m256i result{ _mm256_set1_epi64x(-1ll) };
		__m256i cmp = _mm256_cmpeq_epi8(_mm256_loadu_si256(destVector++), _mm256_loadu_si256(sourceVector++));
		result = _mm256_and_si256(result, cmp);
		cmp = _mm256_cmpeq_epi8(_mm256_loadu_si256(destVector++), _mm256_loadu_si256(sourceVector++));
		result = _mm256_and_si256(result, cmp);
		if (_mm256_movemask_epi8(result) != (0xffffffff)) {
			return false;
		}
		if (remainder > 0) {
			return compareValues(destVector, sourceVector, remainder);
		}
		return true;
	}

	inline bool compareValues128(const void* string1, const void* string2, size_t length) noexcept {
		const size_t remainder{ length % 128 };
		auto destVector = static_cast<const __m256i*>(string1);
		auto sourceVector = static_cast<const __m256i*>(string2);
		__m256i result{ _mm256_set1_epi64x(-1ll) };
		__m256i cmp{ _mm256_cmpeq_epi8(_mm256_loadu_si256(destVector++), _mm256_loadu_si256(sourceVector++)) };
		result = _mm256_and_si256(result, cmp);
		cmp = _mm256_cmpeq_epi8(_mm256_loadu_si256(destVector++), _mm256_loadu_si256(sourceVector++));
		result = _mm256_and_si256(result, cmp);
		cmp = _mm256_cmpeq_epi8(_mm256_loadu_si256(destVector++), _mm256_loadu_si256(sourceVector++));
		result = _mm256_and_si256(result, cmp);
		cmp = _mm256_cmpeq_epi8(_mm256_loadu_si256(destVector++), _mm256_loadu_si256(sourceVector++));
		result = _mm256_and_si256(result, cmp);
		if (_mm256_movemask_epi8(result) != (0xffffffff)) {
			return false;
		}
		if (remainder > 0) {
			return compareValues(destVector, sourceVector, remainder);
		}
		return true;
	}

	inline bool compareValues256(const void* string1, const void* string2, size_t length) noexcept {
		const size_t intervalCount{ length / 256 };
		const size_t remainder{ length % 256 };
		auto destVector = static_cast<const __m256i*>(string1);
		auto sourceVector = static_cast<const __m256i*>(string2);
		__m256i result{ _mm256_set1_epi64x(-1ll) };
		__m256i cmp{};
		for (size_t x = intervalCount; x > 0; --x) {
			cmp = _mm256_cmpeq_epi8(_mm256_loadu_si256(destVector++), _mm256_loadu_si256(sourceVector++));
			result = _mm256_and_si256(result, cmp);
			cmp = _mm256_cmpeq_epi8(_mm256_loadu_si256(destVector++), _mm256_loadu_si256(sourceVector++));
			result = _mm256_and_si256(result, cmp);
			cmp = _mm256_cmpeq_epi8(_mm256_loadu_si256(destVector++), _mm256_loadu_si256(sourceVector++));
			result = _mm256_and_si256(result, cmp);
			cmp = _mm256_cmpeq_epi8(_mm256_loadu_si256(destVector++), _mm256_loadu_si256(sourceVector++));
			result = _mm256_and_si256(result, cmp);
			cmp = _mm256_cmpeq_epi8(_mm256_loadu_si256(destVector++), _mm256_loadu_si256(sourceVector++));
			result = _mm256_and_si256(result, cmp);
			cmp = _mm256_cmpeq_epi8(_mm256_loadu_si256(destVector++), _mm256_loadu_si256(sourceVector++));
			result = _mm256_and_si256(result, cmp);
			cmp = _mm256_cmpeq_epi8(_mm256_loadu_si256(destVector++), _mm256_loadu_si256(sourceVector++));
			result = _mm256_and_si256(result, cmp);
			cmp = _mm256_cmpeq_epi8(_mm256_loadu_si256(destVector++), _mm256_loadu_si256(sourceVector++));
			result = _mm256_and_si256(result, cmp);
			if (_mm256_movemask_epi8(result) != (0xffffffff)) {
				return false;
			}
		}
		if (remainder > 0) {
			return compareValues(destVector, sourceVector, remainder);
		}
		return true;
	}

	inline bool compareValues(const void* string1, const void* string2, size_t length) noexcept {
		if (length >= 256) {
			return compareValues256(string1, string2, length);
		} else if (length >= 128) {
			return compareValues128(string1, string2, length);
		} else if (length >= 64) {
			return compareValues64(string1, string2, length);
		} else if (length >= 32) {
			return compareValues32(string1, string2, length);
		} else if (length >= 16) {
			return compareValues16(string1, string2, length);
		} else {
			return (std::memcmp(string1, string2, length) == 0);
		}
	}

#elif AVX_TYPE == 125

	inline uint64_t findSingleCharacter(const void* str, size_t length, char target) {
		__m128i targetVec = _mm_set1_epi8(target);
		const __m128i* strVec = static_cast<const __m128i*>(str);
		const uint32_t vecSize = sizeof(__m128i);
		size_t remainingBytes = length;

		size_t index = 0;
		while (remainingBytes >= vecSize) {
			__m128i currentVec = _mm_loadu_si128(strVec);
			__m128i compareResult = _mm_cmpeq_epi8(targetVec, currentVec);
			uint16_t mask = _mm_movemask_epi8(compareResult);

			if (mask != 0) {
				uint32_t matchingIndex = _tzcnt_u16(static_cast<uint16_t>(mask));
				return index + matchingIndex;
			}

			++strVec;
			index += vecSize;
			remainingBytes -= vecSize;
		}

		const char* remainingStr = reinterpret_cast<const char*>(strVec);
		for (size_t i = 0; i < remainingBytes; i++) {
			if (remainingStr[i] == target) {
				return index + i;
			}
		}

		return -1;
	}

	inline bool compareValues(const void* string1, const void* string2, size_t length) noexcept;

	inline bool compareValues16(const void* string1, const void* string2, size_t length) noexcept {
		const size_t remainder{ length % 16 };
		auto destVector = static_cast<const __m128i*>(string1);
		auto sourceVector = static_cast<const __m128i*>(string2);
		if (_mm_movemask_epi8(_mm_cmpeq_epi8(_mm_loadu_si128(destVector++), _mm_loadu_si128(sourceVector++))) != (0x0000ffff)) {
			return false;
		}
		if (remainder > 0) {
			return compareValues(destVector, sourceVector, remainder);
		}
		return true;
	}

	inline bool compareValues32(const void* string1, const void* string2, size_t length) noexcept {
		const size_t remainder{ length % 32 };
		auto destVector = static_cast<const __m128i*>(string1);
		auto sourceVector = static_cast<const __m128i*>(string2);
		__m128i result{ _mm_set1_epi64x(-1ll) };
		__m128i cmp = _mm_cmpeq_epi8(_mm_loadu_si128(destVector++), _mm_loadu_si128(sourceVector++));
		result = _mm_and_si128(result, cmp);
		cmp = _mm_cmpeq_epi8(_mm_loadu_si128(destVector++), _mm_loadu_si128(sourceVector++));
		result = _mm_and_si128(result, cmp);
		if (_mm_movemask_epi8(result) != (0x0000ffff)) {
			return false;
		}
		if (remainder > 0) {
			return compareValues(destVector, sourceVector, remainder);
		}
		return true;
	}

	inline bool compareValues64(const void* string1, const void* string2, size_t length) noexcept {
		const size_t remainder{ length % 64 };
		auto destVector = static_cast<const __m128i*>(string1);
		auto sourceVector = static_cast<const __m128i*>(string2);
		__m128i result{ _mm_set1_epi64x(-1ll) };
		__m128i cmp = _mm_cmpeq_epi8(_mm_loadu_si128(destVector++), _mm_loadu_si128(sourceVector++));
		result = _mm_and_si128(result, cmp);
		cmp = _mm_cmpeq_epi8(_mm_loadu_si128(destVector++), _mm_loadu_si128(sourceVector++));
		result = _mm_and_si128(result, cmp);
		cmp = _mm_cmpeq_epi8(_mm_loadu_si128(destVector++), _mm_loadu_si128(sourceVector++));
		result = _mm_and_si128(result, cmp);
		cmp = _mm_cmpeq_epi8(_mm_loadu_si128(destVector++), _mm_loadu_si128(sourceVector++));
		result = _mm_and_si128(result, cmp);
		if (_mm_movemask_epi8(result) != (0x0000ffff)) {
			return false;
		}
		if (remainder > 0) {
			return compareValues(destVector, sourceVector, remainder);
		}
		return true;
	}

	inline bool compareValues128(const void* string1, const void* string2, size_t length) noexcept {
		const size_t intervalCount{ length / 128 };
		const size_t remainder{ length % 128 };
		auto destVector = static_cast<const __m128i*>(string1);
		auto sourceVector = static_cast<const __m128i*>(string2);
		__m128i result{ _mm_set1_epi64x(-1ll) };
		__m128i cmp{};
		for (size_t x{ intervalCount }; x > 0; --x) {
			cmp = _mm_cmpeq_epi8(_mm_loadu_si128(destVector++), _mm_loadu_si128(sourceVector++));
			result = _mm_and_si128(result, cmp);
			cmp = _mm_cmpeq_epi8(_mm_loadu_si128(destVector++), _mm_loadu_si128(sourceVector++));
			result = _mm_and_si128(result, cmp);
			cmp = _mm_cmpeq_epi8(_mm_loadu_si128(destVector++), _mm_loadu_si128(sourceVector++));
			result = _mm_and_si128(result, cmp);
			cmp = _mm_cmpeq_epi8(_mm_loadu_si128(destVector++), _mm_loadu_si128(sourceVector++));
			result = _mm_and_si128(result, cmp);
			cmp = _mm_cmpeq_epi8(_mm_loadu_si128(destVector++), _mm_loadu_si128(sourceVector++));
			result = _mm_and_si128(result, cmp);
			cmp = _mm_cmpeq_epi8(_mm_loadu_si128(destVector++), _mm_loadu_si128(sourceVector++));
			result = _mm_and_si128(result, cmp);
			cmp = _mm_cmpeq_epi8(_mm_loadu_si128(destVector++), _mm_loadu_si128(sourceVector++));
			result = _mm_and_si128(result, cmp);
			cmp = _mm_cmpeq_epi8(_mm_loadu_si128(destVector++), _mm_loadu_si128(sourceVector++));
			result = _mm_and_si128(result, cmp);
			if (_mm_movemask_epi8(result) != (0x0000ffff)) {
				return false;
			}
		}
		if (remainder > 0) {
			return compareValues(destVector, sourceVector, remainder);
		}
		return true;
	}

	inline bool compareValues(const void* string1, const void* string2, size_t length) noexcept {
		if (length >= 128) {
			return compareValues128(string1, string2, length);
		} else if (length >= 64) {
			return compareValues64(string1, string2, length);
		} else if (length >= 32) {
			return compareValues32(string1, string2, length);
		} else if (length >= 16) {
			return compareValues16(string1, string2, length);
		} else {
			return (std::memcmp(string1, string2, length) == 0);
		}
	}

#else

	inline uint64_t findSingleCharacter(const void* str, size_t length, char target) {
		return std::string_view{ static_cast<const char*>(str), length }.find(target);
	}

	inline bool compareValues(const void* destVector, const void* sourceVector, size_t length) noexcept {
		return std::string_view{ static_cast<const char*>(destVector), length } == std::string_view{ static_cast<const char*>(sourceVector), length };
	}

#endif

	class JsonifierCoreInternal {
	  public:
		template<typename ValueType> inline static bool compare(const ValueType* destVector, const ValueType* sourceVector, size_t length) noexcept {
			return compareValues(static_cast<const void*>(destVector), static_cast<const void*>(sourceVector), length * sizeof(ValueType));
		}
	};

}
