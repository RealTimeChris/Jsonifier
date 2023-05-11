/*
Jsonifier - For parsing and serializing Json - very rapidly.
Copyright (C) 2023 Chris M. (RealTimeChris)

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301
USA
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

#if defined(INSTRUCTION_SET_TYPE_AVX512)
	inline size_t findSingleCharacterFast(const void* str, size_t lengthNew, const char charToFind) noexcept {
		size_t result{ std::numeric_limits<uint64_t>::max() };
		size_t currentIndex{ lengthNew / 32 };
		auto arrayChar{ _mm512_set1_epi8(charToFind) };
		auto newPtr = static_cast<const __m512i*>(str);
		size_t remainder{ lengthNew % 32 };
		int64_t mask{};
		for (; currentIndex > 0; --currentIndex) {
			mask = _mm512_cmpeq_epi8_mask(arrayChar, _mm512_loadu_si512(newPtr++));

			if (mask != 0) {
				result = currentIndex + _tzcnt_u32(mask);
				return result;
			}

			currentIndex += 32;
		}

		if (remainder > 0) {
			char charArray[32];
			std::memcpy(charArray, str, lengthNew - remainder);
			mask = _mm512_cmpeq_epi8_mask(arrayChar, _mm512_loadu_si512(reinterpret_cast<__m512i*>(charArray)));

			if (mask != 0) {
				result = currentIndex + _tzcnt_u32(mask);
				return result;
			}
			currentIndex += remainder;
		}

		return result;
	}

	inline bool compareStrings16(const void* str1, const void* str2, size_t lengthNew) noexcept {
		const size_t remainder{ lengthNew % 16 };
		auto destVector = static_cast<const __m128i*>(str1);
		auto sourceVector = static_cast<const __m128i*>(str2);
		if (_mm_movemask_epi8(_mm_cmpeq_epi8(_mm_loadu_si128(destVector++), _mm_loadu_si128(sourceVector++))) != (0x0000ffff)) {
			return false;
		}
		if (remainder > 0) {
			return (std::memcmp(destVector, sourceVector, remainder) == 0);
		}
		return true;
	}

	inline bool compareStrings32(const void* str1, const void* str2, size_t lengthNew) noexcept {
		const size_t remainder{ lengthNew % 32 };
		auto destVector = static_cast<const __m256i*>(str1);
		auto sourceVector = static_cast<const __m256i*>(str2);
		if (_mm256_movemask_epi8(_mm256_cmpeq_epi8(_mm256_loadu_si256(destVector++), _mm256_loadu_si256(sourceVector++))) != (0xffffffff)) {
			return false;
		}
		if (remainder >= 16) {
			return compareStrings16(destVector, sourceVector, remainder);
		}
		if (remainder > 0) {
			return (std::memcmp(destVector, sourceVector, remainder) == 0);
		}
		return true;
	}

	inline bool compareStrings64(const void* str1, const void* str2, size_t lengthNew) noexcept {
		const size_t remainder{ lengthNew % 64 };
		auto destVector = static_cast<const __m512i*>(str1);
		auto sourceVector = static_cast<const __m512i*>(str2);
		if (_mm512_cmp_epi8_mask(_mm512_loadu_si512(destVector++), _mm512_loadu_si512(sourceVector++), _MM_CMPINT_EQ) !=
			(std::numeric_limits<size_t>::max())) {
			return false;
		}
		if (remainder >= 32) {
			return compareStrings32(destVector, sourceVector, remainder);
		}
		if (remainder >= 16) {
			return compareStrings16(destVector, sourceVector, remainder);
		}
		if (remainder > 0) {
			return (std::memcmp(destVector, sourceVector, remainder) == 0);
		}
		return true;
	}

	inline bool compareStrings128(const void* str1, const void* str2, size_t lengthNew) noexcept {
		const size_t remainder{ lengthNew % 128 };
		auto destVector = static_cast<const __m512i*>(str1);
		auto sourceVector = static_cast<const __m512i*>(str2);
		uint64_t result{ std::numeric_limits<uint64_t>::max() };
		uint64_t cmp = _mm512_cmp_epi8_mask(_mm512_loadu_si512(destVector++), _mm512_loadu_si512(sourceVector++), _MM_CMPINT_EQ);
		result = result & cmp;
		cmp = _mm512_cmp_epi8_mask(_mm512_loadu_si512(destVector++), _mm512_loadu_si512(sourceVector++), _MM_CMPINT_EQ);
		result = result & cmp;
		if (result != std::numeric_limits<size_t>::max()) {
			return false;
		}
		if (remainder >= 64) {
			return compareStrings64(destVector, sourceVector, remainder);
		}
		if (remainder >= 32) {
			return compareStrings32(destVector, sourceVector, remainder);
		}
		if (remainder >= 16) {
			return compareStrings16(destVector, sourceVector, remainder);
		}
		if (remainder > 0) {
			return (std::memcmp(destVector, sourceVector, remainder) == 0);
		}
		return true;
	}

	inline bool compareStrings256(const void* str1, const void* str2, size_t lengthNew) noexcept {
		const size_t remainder{ lengthNew % 256 };
		auto destVector = static_cast<const __m512i*>(str1);
		auto sourceVector = static_cast<const __m512i*>(str2);
		uint64_t result{ std::numeric_limits<uint64_t>::max() };
		uint64_t cmp = _mm512_cmp_epi8_mask(_mm512_loadu_si512(destVector++), _mm512_loadu_si512(sourceVector++), _MM_CMPINT_EQ);
		result = result & cmp;
		cmp = _mm512_cmp_epi8_mask(_mm512_loadu_si512(destVector++), _mm512_loadu_si512(sourceVector++), _MM_CMPINT_EQ);
		result = result & cmp;
		cmp = _mm512_cmp_epi8_mask(_mm512_loadu_si512(destVector++), _mm512_loadu_si512(sourceVector++), _MM_CMPINT_EQ);
		result = result & cmp;
		cmp = _mm512_cmp_epi8_mask(_mm512_loadu_si512(destVector++), _mm512_loadu_si512(sourceVector++), _MM_CMPINT_EQ);
		result = result & cmp;
		if (result != std::numeric_limits<size_t>::max()) {
			return false;
		}
		if (remainder >= 128) {
			return compareStrings128(destVector, sourceVector, remainder);
		}
		if (remainder >= 64) {
			return compareStrings64(destVector, sourceVector, remainder);
		}
		if (remainder >= 32) {
			return compareStrings32(destVector, sourceVector, remainder);
		}
		if (remainder >= 16) {
			return compareStrings16(destVector, sourceVector, remainder);
		}
		if (remainder > 0) {
			return (std::memcmp(destVector, sourceVector, remainder) == 0);
		}
		return true;
	}

	inline bool compareStrings512(const void* str1, const void* str2, size_t lengthNew) noexcept {
		const size_t remainder{ lengthNew % 512 };
		auto destVector = static_cast<const __m512i*>(str1);
		auto sourceVector = static_cast<const __m512i*>(str2);
		uint64_t result{ std::numeric_limits<uint64_t>::max() };
		uint64_t cmp{};
		for (size_t x = lengthNew / (sizeof(__m512i) * 8); x > 0; --x) {
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
			if (result != (0xffffffff)) {
				return false;
			}
		}
		if (remainder >= 256) {
			return compareStrings256(destVector, sourceVector, remainder);
		}
		if (remainder >= 128) {
			return compareStrings128(destVector, sourceVector, remainder);
		}
		if (remainder >= 64) {
			return compareStrings64(destVector, sourceVector, remainder);
		}
		if (remainder >= 32) {
			return compareStrings32(destVector, sourceVector, remainder);
		}
		if (remainder >= 16) {
			return compareStrings16(destVector, sourceVector, remainder);
		}
		if (remainder > 0) {
			return (std::memcmp(destVector, sourceVector, remainder) == 0);
		}
		return true;
	}

	inline bool compareStrings(const char* str1, const char* str2, size_t length) noexcept {
		if (length < 16) {
			return (std::memcmp(str1, str2, length) == 0);
		} else if (length < 32) {
			return compareStrings16(str1, str2, length);
		} else if (length < 64) {
			return compareStrings32(str1, str2, length);
		} else if (length < 128) {
			return compareStrings64(str1, str2, length);
		} else if (length < 256) {
			return compareStrings128(str1, str2, length);
		} else if (length < 512) {
			return compareStrings256(str1, str2, length);
		} else {
			return compareStrings512(str1, str2, length);
		}
	}

	inline bool compareFast(const char* destVector, const char* sourceVector, size_t lengthNew) noexcept {
		return compareStrings(destVector, sourceVector, lengthNew);
	}
#elif defined(INSTRUCTION_SET_TYPE_AVX2)
	inline size_t findSingleCharacterFast(const void* str, size_t lengthNew, const char charToFind) noexcept {
		size_t result{ std::numeric_limits<uint64_t>::max() };
		size_t currentIndex{ lengthNew / 32 };
		auto arrayChar{ _mm256_set1_epi8(charToFind) };
		auto newPtr = static_cast<const __m256i*>(str);
		size_t remainder{ lengthNew % 32 };
		int32_t mask{};
		for (; currentIndex > 0; --currentIndex) {
			mask = _mm256_movemask_epi8(_mm256_cmpeq_epi8(arrayChar, _mm256_loadu_si256(newPtr++)));

			if (mask != 0) {
				result = currentIndex + _tzcnt_u32(mask);
				return result;
			}

			currentIndex += 32;
		}

		if (remainder > 0) {
			char charArray[32];
			std::memcpy(charArray, str, lengthNew - remainder);
			mask = _mm256_movemask_epi8(_mm256_cmpeq_epi8(arrayChar, _mm256_loadu_si256(reinterpret_cast<__m256i*>(charArray))));

			if (mask != 0) {
				result = currentIndex + _tzcnt_u32(mask);
				return result;
			}
			currentIndex += remainder;
		}

		return result;
	}

	inline bool compareStrings16(const void* str1, const void* str2, size_t lengthNew) noexcept {
		const size_t remainder{ lengthNew % 16 };
		auto destVector = static_cast<const __m128i*>(str1);
		auto sourceVector = static_cast<const __m128i*>(str2);
		if (_mm_movemask_epi8(_mm_cmpeq_epi8(_mm_loadu_si128(destVector++), _mm_loadu_si128(sourceVector++))) != (0x0000ffff)) {
			return false;
		}
		if (remainder > 0) {
			return (std::memcmp(destVector, sourceVector, remainder) == 0);
		}
		return true;
	}

	inline bool compareStrings32(const void* str1, const void* str2, size_t lengthNew) noexcept {
		const size_t remainder{ lengthNew % 32 };
		auto destVector = static_cast<const __m256i*>(str1);
		auto sourceVector = static_cast<const __m256i*>(str2);
		if (_mm256_movemask_epi8(_mm256_cmpeq_epi8(_mm256_loadu_si256(destVector++), _mm256_loadu_si256(sourceVector++))) != (0xffffffff)) {
			return false;
		}
		if (remainder >= 16) {
			return compareStrings16(destVector, sourceVector, remainder);
		}
		if (remainder > 0) {
			return (std::memcmp(destVector, sourceVector, remainder) == 0);
		}
		return true;
	}

	inline bool compareStrings64(const void* str1, const void* str2, size_t lengthNew) noexcept {
		const size_t remainder{ lengthNew % 64 };
		auto destVector = static_cast<const __m256i*>(str1);
		auto sourceVector = static_cast<const __m256i*>(str2);
		__m256i result{ _mm256_set1_epi64x(-1ll) };
		__m256i cmp = _mm256_cmpeq_epi8(_mm256_loadu_si256(destVector++), _mm256_loadu_si256(sourceVector++));
		result = _mm256_and_si256(result, cmp);
		cmp = _mm256_cmpeq_epi8(_mm256_loadu_si256(destVector++), _mm256_loadu_si256(sourceVector++));
		result = _mm256_and_si256(result, cmp);
		if (_mm256_movemask_epi8(result) != (0xffffffff)) {
			return false;
		}
		if (remainder >= 32) {
			return compareStrings32(destVector, sourceVector, remainder);
		}
		if (remainder >= 16) {
			return compareStrings16(destVector, sourceVector, remainder);
		}
		if (remainder > 0) {
			return (std::memcmp(destVector, sourceVector, remainder) == 0);
		}
		return true;
	}

	inline bool compareStrings128(const void* str1, const void* str2, size_t lengthNew) noexcept {
		const size_t remainder{ lengthNew % 128 };
		auto destVector = static_cast<const __m256i*>(str1);
		auto sourceVector = static_cast<const __m256i*>(str2);
		__m256i result{ _mm256_set1_epi64x(-1ll) };
		__m256i cmp = _mm256_cmpeq_epi8(_mm256_loadu_si256(destVector++), _mm256_loadu_si256(sourceVector++));
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
		if (remainder >= 64) {
			return compareStrings64(destVector, sourceVector, remainder);
		}
		if (remainder >= 32) {
			return compareStrings32(destVector, sourceVector, remainder);
		}
		if (remainder >= 16) {
			return compareStrings16(destVector, sourceVector, remainder);
		}
		if (remainder > 0) {
			return (std::memcmp(destVector, sourceVector, remainder) == 0);
		}
		return true;
	}

	inline bool compareStrings256(const void* str1, const void* str2, size_t lengthNew) noexcept {
		const size_t remainder{ lengthNew % 256 };
		auto destVector = static_cast<const __m256i*>(str1);
		auto sourceVector = static_cast<const __m256i*>(str2);
		__m256i result{ _mm256_set1_epi64x(-1ll) };
		__m256i cmp{};
		uint32_t mask{};
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
		mask = _mm256_movemask_epi8(result);
		if (mask != (0xffffffff)) {
			return false;
		}
		if (remainder >= 128) {
			return compareStrings128(destVector, sourceVector, remainder);
		}
		if (remainder >= 64) {
			return compareStrings64(destVector, sourceVector, remainder);
		}
		if (remainder >= 32) {
			return compareStrings32(destVector, sourceVector, remainder);
		}
		if (remainder >= 16) {
			return compareStrings16(destVector, sourceVector, remainder);
		}
		if (remainder > 0) {
			return (std::memcmp(destVector, sourceVector, remainder) == 0);
		}
		return true;
	}

	inline bool compareStrings512(const void* str1, const void* str2, size_t lengthNew) noexcept {
		const size_t remainder{ lengthNew % 512 };
		auto destVector = static_cast<const __m256i*>(str1);
		auto sourceVector = static_cast<const __m256i*>(str2);
		__m256i result{ _mm256_set1_epi64x(-1ll) };
		__m256i cmp{};
		uint32_t mask{};
		for (size_t x = lengthNew / (sizeof(__m256i) * 16); x > 0; --x) {
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
			mask = _mm256_movemask_epi8(result);
			if (mask != (0xffffffff)) {
				return false;
			}
		}
		if (remainder >= 256) {
			return compareStrings256(destVector, sourceVector, remainder);
		}
		if (remainder >= 128) {
			return compareStrings128(destVector, sourceVector, remainder);
		}
		if (remainder >= 64) {
			return compareStrings64(destVector, sourceVector, remainder);
		}
		if (remainder >= 32) {
			return compareStrings32(destVector, sourceVector, remainder);
		}
		if (remainder >= 16) {
			return compareStrings16(destVector, sourceVector, remainder);
		}
		if (remainder > 0) {
			return (std::memcmp(destVector, sourceVector, remainder) == 0);
		}
		return true;
	}

	inline bool compareStrings(const char* str1, const char* str2, size_t length) noexcept {
		if (length < 16) {
			return (std::memcmp(str1, str2, length) == 0);
		} else if (length < 32) {
			return compareStrings16(str1, str2, length);
		} else if (length < 64) {
			return compareStrings32(str1, str2, length);
		} else if (length < 128) {
			return compareStrings64(str1, str2, length);
		} else if (length < 256) {
			return compareStrings128(str1, str2, length);
		} else if (length < 512) {
			return compareStrings256(str1, str2, length);
		} else {
			return compareStrings512(str1, str2, length);
		}
	}

	inline bool compareFast(const char* destVector, const char* sourceVector, size_t lengthNew) noexcept {
		return compareStrings(destVector, sourceVector, lengthNew);
	}
#elif INSTRUCTION_SET_TYPE_AVX
	inline size_t findSingleCharacterFast(const void* str, size_t lengthNew, const char charToFind) noexcept {
		size_t result{ std::string::npos };
		size_t currentIndex{ lengthNew / 32 };
		auto arrayChar{ _mm_set1_epi8(charToFind) };
		auto newPtr = static_cast<const __m128i*>(str);
		size_t remainder{ lengthNew % 32 };
		int32_t mask{};
		for (; currentIndex > 0; --currentIndex) {
			mask = _mm_movemask_epi8(_mm_cmpeq_epi8(arrayChar, _mm_loadu_si128(newPtr++)));

			if (mask != 0) {
				result = currentIndex + _tzcnt_u32(mask);
				return result;
			}

			currentIndex += 32;
		}

		if (remainder > 0) {
			char charArray[32];
			std::memcpy(charArray, str, lengthNew - remainder);
			mask = _mm_movemask_epi8(_mm_cmpeq_epi8(arrayChar, _mm_loadu_si128(reinterpret_cast<__m128i*>(charArray))));

			if (mask != 0) {
				result = currentIndex + _tzcnt_u32(mask);
				return result;
			}
			currentIndex += remainder;
		}

		return result;
	}

	inline bool compareStrings16Avx(const void* str1, const void* str2, size_t lengthNew) noexcept {
		const size_t remainder{ lengthNew % 16 };
		auto destVector = static_cast<const __m128i*>(str1);
		auto sourceVector = static_cast<const __m128i*>(str2);
		if (_mm_movemask_epi8(_mm_and_si128(_mm_set1_epi64x(-1ll), _mm_cmpeq_epi8(_mm_loadu_si128(destVector++), _mm_loadu_si128(sourceVector++)))) !=
			(0x0000ffff)) {
			return false;
		}
		if (remainder > 0) {
			return (std::memcmp(destVector, sourceVector, remainder) == 0);
		}
		return true;
	}

	inline bool compareStrings32Avx(const void* str1, const void* str2, size_t lengthNew) noexcept {
		const size_t remainder{ lengthNew % 32 };
		auto destVector = static_cast<const __m128i*>(str1);
		auto sourceVector = static_cast<const __m128i*>(str2);
		if (_mm_movemask_epi8(_mm_and_si128(_mm_set1_epi64x(-1ll),
				_mm_cmpeq_epi8(_mm_loadu_si128(destVector++), _mm_loadu_si128(sourceVector++)))) != (0xffffffff)) {
			return false;
		}
		if (remainder < 16 && remainder > 0) {
			return (std::memcmp(destVector, sourceVector, remainder) == 0);
		}
		if (remainder > 0) {
			return compareStrings16Avx(destVector, sourceVector, remainder);
		}
		return true;
	}

	inline bool compareStrings64Avx(const void* str1, const void* str2, size_t lengthNew) noexcept {
		const size_t remainder{ lengthNew % 64 };
		auto destVector = static_cast<const __m128i*>(str1);
		auto sourceVector = static_cast<const __m128i*>(str2);
		__m128i result{ _mm_set1_epi64x(-1ll) };
		__m128i cmp = _mm_cmpeq_epi8(_mm_loadu_si128(destVector++), _mm_loadu_si128(sourceVector++));
		result = _mm_and_si128(result, cmp);
		cmp = _mm_cmpeq_epi8(_mm_loadu_si128(destVector++), _mm_loadu_si128(sourceVector++));
		result = _mm_and_si128(result, cmp);
		if (_mm_movemask_epi8(result) != (0xffffffff)) {
			return false;
		}
		if (remainder < 16 && remainder > 0) {
			return (std::memcmp(destVector, sourceVector, remainder) == 0);
		}
		if (remainder < 32 && remainder > 0) {
			return compareStrings16Avx(destVector, sourceVector, remainder);
		}
		if (remainder > 0) {
			return compareStrings32Avx(destVector, sourceVector, remainder);
		}
		return true;
	}

	inline bool compareStrings128Avx(const void* str1, const void* str2, size_t lengthNew) noexcept {
		const size_t remainder{ lengthNew % 128 };
		auto destVector = static_cast<const __m128i*>(str1);
		auto sourceVector = static_cast<const __m128i*>(str2);
		__m128i result{ _mm_set1_epi64x(-1ll) };
		__m128i cmp = _mm_cmpeq_epi8(_mm_loadu_si128(destVector++), _mm_loadu_si128(sourceVector++));
		result = _mm_and_si128(result, cmp);
		cmp = _mm_cmpeq_epi8(_mm_loadu_si128(destVector++), _mm_loadu_si128(sourceVector++));
		result = _mm_and_si128(result, cmp);
		cmp = _mm_cmpeq_epi8(_mm_loadu_si128(destVector++), _mm_loadu_si128(sourceVector++));
		result = _mm_and_si128(result, cmp);
		cmp = _mm_cmpeq_epi8(_mm_loadu_si128(destVector++), _mm_loadu_si128(sourceVector++));
		result = _mm_and_si128(result, cmp);
		if (_mm_movemask_epi8(result) != (0xffffffff)) {
			return false;
		}
		if (remainder < 16 && remainder > 0) {
			return (std::memcmp(destVector, sourceVector, remainder) == 0);
		}
		if (remainder < 32 && remainder > 0) {
			return compareStrings16Avx(destVector, sourceVector, remainder);
		}
		if (remainder < 64 && remainder > 0) {
			return compareStrings32Avx(destVector, sourceVector, remainder);
		}
		if (remainder > 0) {
			return compareStrings64Avx(destVector, sourceVector, remainder);
		}
		return true;
	}

	inline bool compareStrings256Avx(const void* str1, const void* str2, size_t lengthNew) noexcept {
		const size_t remainder{ lengthNew % 128 };
		auto destVector = static_cast<const __m128i*>(str1);
		auto sourceVector = static_cast<const __m128i*>(str2);
		__m128i result{ _mm_set1_epi64x(-1ll) };
		__m128i cmp{};
		uint32_t mask{};
		for (size_t x = lengthNew / (sizeof(__m128i) * 8); x > 0; --x) {
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
			mask = _mm_movemask_epi8(result);
			if (mask != (0xffffffff)) {
				return false;
			}
		}
		if (remainder < 16 && remainder > 0) {
			return (std::memcmp(destVector, sourceVector, remainder) == 0);
		}
		if (remainder < 32 && remainder > 0) {
			return compareStrings16Avx(destVector, sourceVector, remainder);
		}
		if (remainder < 64 && remainder > 0) {
			return compareStrings32Avx(destVector, sourceVector, remainder);
		}
		if (remainder < 128 && remainder > 0) {
			return compareStrings64Avx(destVector, sourceVector, remainder);
		}
		if (remainder > 0) {
			return compareStrings128Avx(destVector, sourceVector, remainder);
		}
		return true;
	}

	inline bool compareStringsAvx(const void* str1, const void* str2, size_t length) noexcept {
		if (length < 16) {
			return (std::memcmp(str1, str2, length) == 0);
		} else if (length < 32) {
			return compareStrings16Avx(str1, str2, length);
		} else if (length < 64) {
			return compareStrings32Avx(str1, str2, length);
		} else if (length < 128) {
			return compareStrings64Avx(str1, str2, length);
		} else if (length < 256) {
			return compareStrings128Avx(str1, str2, length);
		} else {
			return compareStrings256Avx(str1, str2, length);
		}
	}

	inline bool compareFast(const void* destVector, const void* sourceVector, size_t lengthNew) noexcept {
		return compareStringsAvx(destVector, sourceVector, lengthNew);
	}
#else
	inline bool compareFast(const void* destVector, const void* sourceVector, size_t lengthNew) noexcept {
		return std::string_view{ static_cast<const char*>(destVector) } == std::string_view{ static_cast<const char*>(sourceVector) };
	}
#endif

	class JsonifierCoreInternal {
	  public:
		inline static bool compare(const char* destVector, const char* sourceVector, size_t lengthNew) noexcept {
			return compareFast(destVector, sourceVector, lengthNew);
		}
	};

}