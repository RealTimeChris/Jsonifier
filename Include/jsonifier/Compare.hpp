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
#include <cstdint>
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

	template<typename ValueType> inline __m128i gatherValues128(const ValueType str[sizeof(__m128) / sizeof(ValueType)]) {
		alignas(16) float newArray[sizeof(__m128i) / sizeof(float)]{};
		std::memcpy(newArray, str, sizeof(__m128i));
		return _mm_castps_si128(_mm_load_ps(newArray));
	}

	template<typename ValueType> inline __m256i gatherValues256(const ValueType str[sizeof(__m256) / sizeof(ValueType)]) {
		alignas(32) float newArray[sizeof(__m256i) / sizeof(float)]{};
		std::memcpy(newArray, str, sizeof(__m256i));
		return _mm256_castps_si256(_mm256_load_ps(newArray));
	}

	template<typename ValueType> inline __m512i gatherValues512(const ValueType str[sizeof(__m512) / sizeof(ValueType)]) {
		return _mm512_load_si512(str);
	}

	template<typename ValueType> inline size_t findSingleCharacter(const ValueType* str, size_t length, ValueType target) {
		__m512i targetVec = _mm512_set1_epi8(target);
		__m512i strVec = _mm512_load_si512(str);
		const size_t vecSize = sizeof(__m512i);
		size_t remainingBytes = length;

		size_t index = 0;
		while (remainingBytes >= vecSize) {
			size_t mask = _mm512_cmpeq_epi64_mask(targetVec, strVec);

			if (mask != 0) {
				size_t matchingIndex = _tzcnt_u64(mask);
				return index + matchingIndex;
			}
			index += vecSize;
			strVec = _mm512_load_si512(str + index);
			remainingBytes -= vecSize;
		}

		const ValueType* remainingStr = str + index;
		for (size_t i = 0; i < remainingBytes; i++) {
			if (remainingStr[i] == target) {
				return index + i;
			}
		}

		return std::numeric_limits<size_t>::max();
	}

	template<typename ValueType01, typename ValueType02>
	inline bool compareValues(const ValueType01* string1, const ValueType02* string2, size_t length) noexcept;

	template<typename ValueType01, typename ValueType02>
	inline bool compareValues16(const ValueType01* string1, const ValueType02* string2, size_t length) noexcept {
		const size_t remainder{ length % 16 };
		auto destVector = gatherValues128<ValueType01>(string1);
		auto sourceVector = gatherValues128<ValueType02>(string2);
		if (static_cast<uint16_t>(_mm_movemask_epi8(_mm_cmpeq_epi64(destVector, sourceVector))) != (std::numeric_limits<uint16_t>::max())) {
			return false;
		}
		if (remainder > 0) {
			return compareValues(string1 + 16, string2 + 16, remainder);
		}
		return true;
	}

	template<typename ValueType01, typename ValueType02>
	inline bool compareValues32(const ValueType01* string1, const ValueType02* string2, size_t length) noexcept {
		const size_t remainder{ length % 32 };
		auto destVector = gatherValues256<ValueType01>(string1);
		auto sourceVector = gatherValues256<ValueType02>(string2);
		if (static_cast<uint32_t>(_mm256_movemask_epi8(_mm256_cmpeq_epi64(destVector, sourceVector))) != (std::numeric_limits<uint32_t>::max())) {
			return false;
		}
		if (remainder > 0) {
			return compareValues(string1 + 32, string2 + 32, remainder);
		}
		return true;
	}

	template<typename ValueType01, typename ValueType02>
	inline bool compareValues64(const ValueType01* string1, const ValueType02* string2, size_t length) noexcept {
		const size_t remainder{ length % 64 };
		auto destVector = gatherValues512<ValueType01>(string1);
		auto sourceVector = gatherValues512<ValueType02>(string2);
		if (_mm512_cmpeq_epi64_mask(destVector, sourceVector) != (std::numeric_limits<size_t>::max())) {
			return false;
		}
		if (remainder > 0) {
			return compareValues(string1 + 64, string2 + 64, remainder);
		}
		return true;
	}

	template<typename ValueType01, typename ValueType02>
	inline bool compareValues128(const ValueType01* string1, const ValueType02* string2, size_t length) noexcept {
		const size_t remainder{ length % 128 };
		auto destVector = gatherValues512<ValueType01>(string1);
		auto sourceVector = gatherValues512<ValueType02>(string2);
		size_t result{ std::numeric_limits<size_t>::max() };
		size_t cmp{ _mm512_cmpeq_epi64_mask(destVector, sourceVector) };
		result = result & cmp;
		destVector = gatherValues512<ValueType01>(string1 + 64);
		sourceVector = gatherValues512<ValueType02>(string2 + 64);
		cmp = _mm512_cmpeq_epi64_mask(destVector, sourceVector);
		result = result & cmp;
		if (result != (std::numeric_limits<size_t>::max())) {
			return false;
		}
		if (remainder > 0) {
			return compareValues(string1 + 128, string2 + 128, remainder);
		}
		return true;
	}

	template<typename ValueType01, typename ValueType02>
	inline bool compareValues256(const ValueType01* string1, const ValueType02* string2, size_t length) noexcept {
		const size_t remainder{ length % 256 };
		auto destVector = gatherValues512<ValueType01>(string1);
		auto sourceVector = gatherValues512<ValueType02>(string2);
		size_t result{ std::numeric_limits<size_t>::max() };
		size_t cmp{ _mm512_cmpeq_epi64_mask(destVector, sourceVector) };
		result = result & cmp;
		destVector = gatherValues512<ValueType01>(string1 + 64);
		sourceVector = gatherValues512<ValueType02>(string2 + 64);
		cmp = _mm512_cmpeq_epi64_mask(destVector, sourceVector);
		result = result & cmp;
		destVector = gatherValues512<ValueType01>(string1 + 128);
		sourceVector = gatherValues512<ValueType02>(string2 + 128);
		cmp = _mm512_cmpeq_epi64_mask(destVector, sourceVector);
		result = result & cmp;
		destVector = gatherValues512<ValueType01>(string1 + 192);
		sourceVector = gatherValues512<ValueType02>(string2 + 192);
		cmp = _mm512_cmpeq_epi64_mask(destVector, sourceVector);
		result = result & cmp;
		if (result != (std::numeric_limits<size_t>::max())) {
			return false;
		}
		if (remainder > 0) {
			return compareValues(string1 + 256, string2 + 256, remainder);
		}
		return true;
	}

	template<typename ValueType01, typename ValueType02>
	inline bool compareValues512(const ValueType01* string1, const ValueType02* string2, size_t length) noexcept {
		const size_t remainder{ length % 512 };
		size_t intervalCount{ length / 512 };
		auto destVector = gatherValues512<ValueType01>(string1);
		auto sourceVector = gatherValues512<ValueType02>(string2);
		size_t result{ std::numeric_limits<size_t>::max() };
		size_t cmp{};
		for (size_t x = intervalCount; x > 0; --x) {
			cmp = _mm512_cmpeq_epi64_mask(destVector, sourceVector);
			result = result & cmp;
			destVector = gatherValues512<ValueType01>(string1 + x * (1ull * 64));
			sourceVector = gatherValues512<ValueType02>(string2 + x * (1ull * 64));
			cmp = _mm512_cmpeq_epi64_mask(destVector, sourceVector);
			result = result & cmp;
			destVector = gatherValues512<ValueType01>(string1 + x * (2ull * 64));
			sourceVector = gatherValues512<ValueType02>(string2 + x * (2ull * 64));
			cmp = _mm512_cmpeq_epi64_mask(destVector, sourceVector);
			result = result & cmp;
			destVector = gatherValues512<ValueType01>(string1 + x * (3ull * 64));
			sourceVector = gatherValues512<ValueType02>(string2 + x * (3ull * 64));
			cmp = _mm512_cmpeq_epi64_mask(destVector, sourceVector);
			result = result & cmp;
			destVector = gatherValues512<ValueType01>(string1 + x * (4ull * 64));
			sourceVector = gatherValues512<ValueType02>(string2 + x * (4ull * 64));
			cmp = _mm512_cmpeq_epi64_mask(destVector, sourceVector);
			result = result & cmp;
			destVector = gatherValues512<ValueType01>(string1 + x * (5ull * 64));
			sourceVector = gatherValues512<ValueType02>(string2 + x * (5ull * 64));
			cmp = _mm512_cmpeq_epi64_mask(destVector, sourceVector);
			result = result & cmp;
			destVector = gatherValues512<ValueType01>(string1 + x * (6ull * 64));
			sourceVector = gatherValues512<ValueType02>(string2 + x * (6ull * 64));
			cmp = _mm512_cmpeq_epi64_mask(destVector, sourceVector);
			result = result & cmp;
			destVector = gatherValues512<ValueType01>(string1 + x * (7ull * 64));
			sourceVector = gatherValues512<ValueType02>(string2 + x * (7ull * 64));
			cmp = _mm512_cmpeq_epi64_mask(destVector, sourceVector);
			result = result & cmp;
			destVector = gatherValues512<ValueType01>(string1 + x * (8ull * 64));
			sourceVector = gatherValues512<ValueType02>(string2 + x * (8ull * 64));
			if (result != (std::numeric_limits<size_t>::max())) {
				return false;
			}
		}
		if (remainder > 0) {
			return compareValues(string1 + intervalCount * 64, string2 + intervalCount * 64, remainder);
		}
		return true;
	}

	template<typename ValueType01, typename ValueType02>
	inline bool compareValues(const ValueType01* string1, const ValueType02* string2, size_t length) noexcept {
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

#elif defined T_AVX2

	template<typename ValueType> inline __m128i gatherValues128(const ValueType str[sizeof(__m128) / sizeof(ValueType)]) {
		alignas(16) float newArray[sizeof(__m128i) / sizeof(float)]{};
		std::memcpy(newArray, str, sizeof(__m128i));
		return _mm_castps_si128(_mm_load_ps(newArray));
	}

	template<typename ValueType> inline __m256i gatherValues256(const ValueType str[sizeof(__m256) / sizeof(ValueType)]) {
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
	inline bool compareValues(const ValueType01* string1, const ValueType02* string2, size_t length) noexcept;

	template<typename ValueType01, typename ValueType02>
	inline bool compareValues16(const ValueType01* string1, const ValueType02* string2, size_t length) noexcept {
		const size_t remainder{ length % 16 };
		auto destVector = gatherValues128<ValueType01>(string1);
		auto sourceVector = gatherValues128<ValueType02>(string2);
		if (static_cast<uint16_t>(_mm_movemask_epi8(_mm_cmpeq_epi64(destVector, sourceVector))) != (std::numeric_limits<uint16_t>::max())) {
			return false;
		}
		if (remainder > 0) {
			return compareValues(string1 + 16, string2 + 16, remainder);
		}
		return true;
	}

	template<typename ValueType01, typename ValueType02>
	inline bool compareValues32(const ValueType01* string1, const ValueType02* string2, size_t length) noexcept {
		const size_t remainder{ length % 32 };
		auto destVector = gatherValues256<ValueType01>(string1);
		auto sourceVector = gatherValues256<ValueType02>(string2);
		if (static_cast<uint32_t>(_mm256_movemask_epi8(_mm256_cmpeq_epi64(destVector, sourceVector))) != (std::numeric_limits<uint32_t>::max())) {
			return false;
		}
		if (remainder > 0) {
			return compareValues(string1 + 32, string2 + 32, remainder);
		}
		return true;
	}

	template<typename ValueType01, typename ValueType02>
	inline bool compareValues64(const ValueType01* string1, const ValueType02* string2, size_t length) noexcept {
		const size_t remainder{ length % 64 };
		auto destVector = gatherValues256<ValueType01>(string1);
		auto sourceVector = gatherValues256<ValueType02>(string2);
		__m256i result{ _mm256_set1_epi64x(std::numeric_limits<size_t>::max()) };
		__m256i cmp = _mm256_cmpeq_epi64(destVector, sourceVector);
		result = _mm256_and_si256(result, cmp);
		destVector = gatherValues256<ValueType01>(string1 + 32);
		sourceVector = gatherValues256<ValueType02>(string2 + 32);
		cmp = _mm256_cmpeq_epi64(destVector, sourceVector);
		result = _mm256_and_si256(result, cmp);
		if (static_cast<uint32_t>(_mm256_movemask_epi8(result)) != (std::numeric_limits<uint32_t>::max())) {
			return false;
		}
		if (remainder > 0) {
			return compareValues(string1 + 64, string2 + 64, remainder);
		}
		return true;
	}

	template<typename ValueType01, typename ValueType02>
	inline bool compareValues128(const ValueType01* string1, const ValueType02* string2, size_t length) noexcept {
		const size_t remainder{ length % 128 };
		auto destVector = gatherValues256<ValueType01>(string1);
		auto sourceVector = gatherValues256<ValueType02>(string2);
		__m256i result{ _mm256_set1_epi64x(std::numeric_limits<size_t>::max()) };
		__m256i cmp{ _mm256_cmpeq_epi64(destVector, sourceVector) };
		result = _mm256_and_si256(result, cmp);
		destVector = gatherValues256<ValueType01>(string1 + 32);
		sourceVector = gatherValues256<ValueType02>(string2 + 32);
		cmp = _mm256_cmpeq_epi64(destVector, sourceVector);
		result = _mm256_and_si256(result, cmp);
		destVector = gatherValues256<ValueType01>(string1 + 64);
		sourceVector = gatherValues256<ValueType02>(string2 + 64);
		cmp = _mm256_cmpeq_epi64(destVector, sourceVector);
		result = _mm256_and_si256(result, cmp);
		destVector = gatherValues256<ValueType01>(string1 + 96);
		sourceVector = gatherValues256<ValueType02>(string2 + 96);
		cmp = _mm256_cmpeq_epi64(destVector, sourceVector);
		result = _mm256_and_si256(result, cmp);
		if (static_cast<uint32_t>(_mm256_movemask_epi8(result)) != (std::numeric_limits<uint32_t>::max())) {
			return false;
		}
		if (remainder > 0) {
			return compareValues(string1 + 128, string2 + 128, remainder);
		}
		return true;
	}

	template<typename ValueType01, typename ValueType02>
	inline bool compareValues256(const ValueType01* string1, const ValueType02* string2, size_t length) noexcept {
		const size_t intervalCount{ length / 256 };
		const size_t remainder{ length % 256 };
		auto destVector = gatherValues256<ValueType01>(string1);
		auto sourceVector = gatherValues256<ValueType02>(string2);
		__m256i result{ _mm256_set1_epi64x(std::numeric_limits<size_t>::max()) };
		__m256i cmp{};
		for (size_t x = intervalCount; x > 0; --x) {
			cmp = _mm256_cmpeq_epi64(destVector, sourceVector);
			result = _mm256_and_si256(result, cmp);
			destVector = gatherValues256<ValueType01>(string1 + x * (1ull * 32));
			sourceVector = gatherValues256<ValueType02>(string2 + x * (1ull * 32));
			cmp = _mm256_cmpeq_epi64(destVector, sourceVector);
			result = _mm256_and_si256(result, cmp);
			destVector = gatherValues256<ValueType01>(string1 + x * (2ull * 32));
			sourceVector = gatherValues256<ValueType02>(string2 + x * (2ull * 32));
			cmp = _mm256_cmpeq_epi64(destVector, sourceVector);
			result = _mm256_and_si256(result, cmp);
			destVector = gatherValues256<ValueType01>(string1 + x * (3ull * 32));
			sourceVector = gatherValues256<ValueType02>(string2 + x * (3ull * 32));
			cmp = _mm256_cmpeq_epi64(destVector, sourceVector);
			result = _mm256_and_si256(result, cmp);
			destVector = gatherValues256<ValueType01>(string1 + x * (4ull * 32));
			sourceVector = gatherValues256<ValueType02>(string2 + x * (4ull * 32));
			cmp = _mm256_cmpeq_epi64(destVector, sourceVector);
			result = _mm256_and_si256(result, cmp);
			destVector = gatherValues256<ValueType01>(string1 + x * (5ull * 32));
			sourceVector = gatherValues256<ValueType02>(string2 + x * (5ull * 32));
			cmp = _mm256_cmpeq_epi64(destVector, sourceVector);
			result = _mm256_and_si256(result, cmp);
			destVector = gatherValues256<ValueType01>(string1 + x * (6ull * 32));
			sourceVector = gatherValues256<ValueType02>(string2 + x * (6ull * 32));
			cmp = _mm256_cmpeq_epi64(destVector, sourceVector);
			result = _mm256_and_si256(result, cmp);
			destVector = gatherValues256<ValueType01>(string1 + x * (7ull * 32));
			sourceVector = gatherValues256<ValueType02>(string2 + x * (7ull * 32));
			cmp = _mm256_cmpeq_epi64(destVector, sourceVector);
			result = _mm256_and_si256(result, cmp);
			destVector = gatherValues256<ValueType01>(string1 + x * (8ull * 32));
			sourceVector = gatherValues256<ValueType02>(string2 + x * (8ull * 32));
			if (static_cast<uint32_t>(_mm256_movemask_epi8(result)) != (std::numeric_limits<uint32_t>::max())) {
				return false;
			}
		}
		if (remainder > 0) {
			return compareValues(string1 + intervalCount * 32, string2 + intervalCount * 32, remainder);
		}
		return true;
	}

	template<typename ValueType01, typename ValueType02>
	inline bool compareValues(const ValueType01* string1, const ValueType02* string2, size_t length) noexcept {
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

#elif defined T_AVX

	template<typename ValueType> inline __m128i gatherValues128(const ValueType str[sizeof(__m128) / sizeof(ValueType)]) {
		alignas(16) float newArray[sizeof(__m128i) / sizeof(float)]{};
		std::memcpy(newArray, str, sizeof(__m128i));
		auto returnValue = _mm_castps_si128(_mm_load_ps(newArray));
		return returnValue;
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
				uint16_t matchingIndex = _tzcnt_u16(mask);
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
	inline bool compareValues(const ValueType01* string1, const ValueType02* string2, size_t length) noexcept;

	template<typename ValueType01, typename ValueType02>
	inline bool compareValues16(const ValueType01* string1, const ValueType02* string2, size_t length) noexcept {
		const size_t remainder{ length % 16 };
		auto destVector = gatherValues128<ValueType01>(string1);
		auto sourceVector = gatherValues128<ValueType02>(string2);
		if (static_cast<uint16_t>(_mm_movemask_epi8(_mm_cmpeq_epi64(destVector, sourceVector))) != (std::numeric_limits<uint16_t>::max())) {
			return false;
		}
		if (remainder > 0) {
			return compareValues(string1 + 16, string2 + 16, remainder);
		}
		return true;
	}

	template<typename ValueType01, typename ValueType02>
	inline bool compareValues32(const ValueType01* string1, const ValueType02* string2, uint32_t length) noexcept {
		const uint32_t remainder{ length % 32 };
		auto destVector = gatherValues128<ValueType01>(string1);
		auto sourceVector = gatherValues128<ValueType02>(string2);
		__m128i result{ _mm_set1_epi32(std::numeric_limits<uint32_t>::max()) };
		__m128i cmp = _mm_cmpeq_epi32(destVector, sourceVector);
		result = _mm_and_si128(result, cmp);
		destVector = gatherValues128<ValueType01>(string1 + 16);
		sourceVector = gatherValues128<ValueType02>(string2 + 16);
		cmp = _mm_cmpeq_epi32(destVector, sourceVector);
		result = _mm_and_si128(result, cmp);
		if (static_cast<uint16_t>(_mm_movemask_epi8(result)) != (std::numeric_limits<uint16_t>::max())) {
			return false;
		}
		if (remainder > 0) {
			return compareValues(string1 + 32, string2 + 32, remainder);
		}
		return true;
	}

	template<typename ValueType01, typename ValueType02>
	inline bool compareValues64(const ValueType01* string1, const ValueType02* string2, size_t length) noexcept {
		const size_t remainder{ length % 64 };
		auto destVector = gatherValues128<ValueType01>(string1);
		auto sourceVector = gatherValues128<ValueType02>(string2);
		__m128i result{ _mm_set1_epi64x(std::numeric_limits<size_t>::max()) };
		__m128i cmp = _mm_cmpeq_epi64(destVector, sourceVector);
		result = _mm_and_si128(result, cmp);
		destVector = gatherValues128<ValueType01>(string1 + 16);
		sourceVector = gatherValues128<ValueType02>(string2 + 16);
		cmp = _mm_cmpeq_epi64(destVector, sourceVector);
		result = _mm_and_si128(result, cmp);
		destVector = gatherValues128<ValueType01>(string1 + 32);
		sourceVector = gatherValues128<ValueType02>(string2 + 32);
		cmp = _mm_cmpeq_epi64(destVector, sourceVector);
		result = _mm_and_si128(result, cmp);
		destVector = gatherValues128<ValueType01>(string1 + 48);
		sourceVector = gatherValues128<ValueType02>(string2 + 48);
		cmp = _mm_cmpeq_epi64(destVector, sourceVector);
		result = _mm_and_si128(result, cmp);
		destVector = gatherValues128<ValueType01>(string1 + 64);
		sourceVector = gatherValues128<ValueType02>(string2 + 64);
		cmp = _mm_cmpeq_epi64(destVector, sourceVector);
		result = _mm_and_si128(result, cmp);
		if (static_cast<uint32_t>(_mm_movemask_epi8(result)) != (std::numeric_limits<uint32_t>::max())) {
			return false;
		}
		if (remainder > 0) {
			return compareValues(string1 + 64, string2 + 64, remainder);
		}
		return true;
	}

	template<typename ValueType01, typename ValueType02>
	inline bool compareValues128(const ValueType01* string1, const ValueType02* string2, size_t length) noexcept {
		const size_t intervalCount{ length / 128 };
		const size_t remainder{ length % 128 };
		auto destVector = gatherValues128<ValueType01>(string1);
		auto sourceVector = gatherValues128<ValueType02>(string2);
		__m128i result{ _mm_set1_epi64x(std::numeric_limits<size_t>::max()) };
		__m128i cmp{};
		for (size_t x = intervalCount; x > 0; --x) {
			cmp = _mm_cmpeq_epi64(destVector, sourceVector);
			result = _mm_and_si128(result, cmp);
			destVector = gatherValues128<ValueType01>(string1 + x * (1ull * 16));
			sourceVector = gatherValues128<ValueType02>(string2 + x * (1ull * 16));
			cmp = _mm_cmpeq_epi64(destVector, sourceVector);
			result = _mm_and_si128(result, cmp);
			destVector = gatherValues128<ValueType01>(string1 + x * (2ull * 16));
			sourceVector = gatherValues128<ValueType02>(string2 + x * (2ull * 16));
			cmp = _mm_cmpeq_epi64(destVector, sourceVector);
			result = _mm_and_si128(result, cmp);
			destVector = gatherValues128<ValueType01>(string1 + x * (3ull * 16));
			sourceVector = gatherValues128<ValueType02>(string2 + x * (3ull * 16));
			cmp = _mm_cmpeq_epi64(destVector, sourceVector);
			result = _mm_and_si128(result, cmp);
			destVector = gatherValues128<ValueType01>(string1 + x * (4ull * 16));
			sourceVector = gatherValues128<ValueType02>(string2 + x * (4ull * 16));
			cmp = _mm_cmpeq_epi64(destVector, sourceVector);
			result = _mm_and_si128(result, cmp);
			destVector = gatherValues128<ValueType01>(string1 + x * (5ull * 16));
			sourceVector = gatherValues128<ValueType02>(string2 + x * (5ull * 16));
			cmp = _mm_cmpeq_epi64(destVector, sourceVector);
			result = _mm_and_si128(result, cmp);
			destVector = gatherValues128<ValueType01>(string1 + x * (6ull * 16));
			sourceVector = gatherValues128<ValueType02>(string2 + x * (6ull * 16));
			cmp = _mm_cmpeq_epi64(destVector, sourceVector);
			result = _mm_and_si128(result, cmp);
			destVector = gatherValues128<ValueType01>(string1 + x * (7ull * 16));
			sourceVector = gatherValues128<ValueType02>(string2 + x * (7ull * 16));
			cmp = _mm_cmpeq_epi64(destVector, sourceVector);
			result = _mm_and_si128(result, cmp);
			destVector = gatherValues128<ValueType01>(string1 + x * (8ull * 16));
			sourceVector = gatherValues128<ValueType02>(string2 + x * (8ull * 16));
			if (static_cast<uint16_t>(_mm_movemask_epi8(result)) != (std::numeric_limits<uint16_t>::max())) {
				return false;
			}
		}
		if (remainder > 0) {
			return compareValues(string1 + intervalCount * 16, string2 + intervalCount * 16, remainder);
		}
		return true;
	}

	template<typename ValueType01, typename ValueType02>
	inline bool compareValues(const ValueType01* string1, const ValueType02* string2, size_t length) noexcept {
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

	template<typename ValueType01, typename ValueType02>
	inline size_t findSingleCharacter(const ValueType01* str, size_t length, ValueType02 target) {
		return std::basic_string_view{ static_cast<const ValueType01*>(str), length }.find(target);
	}

	template<typename ValueType01, typename ValueType02> inline bool compareSectionOfString(ValueType01* string01, ValueType02* string02) {
		std::remove_const_t<ValueType01> currentArray01[8]{};
		std::memcpy(currentArray01, string02, 8);
		return memcmp(string01, currentArray01, 8) == 0;
	}

	template<typename ValueType01, typename ValueType02>
	inline bool compareValues(const ValueType01* destVector, const ValueType02* sourceVector, size_t length) noexcept {
		auto originalLength{ length };
		auto remainder{ length % 8 };
		while (length >= 8) {
			if (!compareSectionOfString(destVector + (originalLength - length), sourceVector + (originalLength - length))) {
				return false;
			}
			length -= 8;
		}
		return memcmp(destVector + (originalLength - length), sourceVector + (originalLength - length), remainder);
	}

#endif

	class JsonifierCoreInternal {
	  public:
		template<typename ValueType01, typename ValueType02>
		inline static bool compare(const ValueType01* destVector, const ValueType02* sourceVector, size_t length) noexcept {
			return compareValues(destVector, sourceVector, length * sizeof(ValueType01));
		}
	};

}
