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
	License along with this library; if not, Write to the Free Software
	Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301
	USA
*/
/// https://github.com/RealTimeChris/Jsonifier
/// Feb 3, 2023
#pragma once

#include <immintrin.h>
#include <iostream>
#include <stdlib.h>

namespace Jsonifier {

#ifdef INSTRUCTION_SET_TYPE_AVX512
	inline size_t findSingleCharacterAvx(const void* string, size_t lengthNew, const char charToFind) noexcept {
		size_t result{ std::string::npos };
		size_t currentIndex{ lengthNew / 32 };
		const auto arrayChar{ _mm256_set1_epi8(charToFind) };
		auto newPtr = reinterpret_cast<const __m256i*>(string);
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
			::memcpy(charArray, string, lengthNew - remainder);
			mask = _mm256_movemask_epi8(_mm256_cmpeq_epi8(arrayChar, _mm256_loadu_epi8(charArray)));

			if (mask != 0) {
				result = currentIndex + _tzcnt_u32(mask);
				return result;
			}
			currentIndex += remainder;
		}

		return result;
	}

	inline bool compareStringsSmall16Avx512(const void* str1, const void* str2, size_t lengthNew) noexcept {
		const size_t remainder{ lengthNew % 16 };
		auto* destVector = static_cast<const __m128i*>(str1);
		const auto* sourceVector = static_cast<const __m128i*>(str2);
		if (_mm_movemask_epi8(_mm_and_si128(_mm_set1_epi64x(-1ll), _mm_cmpeq_epi8(_mm_loadu_si128(destVector++), _mm_loadu_si128(sourceVector++)))) !=
			(0x0000ffff)) {
			return false;
		}
		if (remainder > 0) {
			return (memcmp(destVector, sourceVector, remainder) == 0);
		}
		return true;
	}

	inline bool compareStringsSmall32Avx512(const void* str1, const void* str2, size_t lengthNew) noexcept {
		const size_t remainder{ lengthNew % 32 };
		auto* destVector = reinterpret_cast<const __m256i*>(str1);
		const auto* sourceVector = reinterpret_cast<const __m256i*>(str2);
		if (_mm256_movemask_epi8(_mm256_and_si256(_mm256_set1_epi64x(-1ll),
				_mm256_cmpeq_epi8(_mm256_loadu_si256(destVector++), _mm256_loadu_si256(sourceVector++)))) != (0xffffffff)) {
			return false;
		}
		if (remainder < 16 && remainder > 0) {
			return (memcmp(destVector, sourceVector, remainder) == 0);
		}
		if (remainder > 0) {
			return compareStringsSmall16Avx512(destVector, sourceVector, remainder);
		}
		return true;
	}

	inline bool compareStringsSmall64Avx512(const void* str1, const void* str2, size_t lengthNew) noexcept {
		const size_t remainder{ lengthNew % 64 };
		auto* destVector = reinterpret_cast<const __m512i*>(str1);
		const auto* sourceVector = reinterpret_cast<const __m512i*>(str2);
		__mmask64 result{ std::numeric_limits<uint64_t>::max() };
		auto cmp = _mm512_cmpeq_epi8_mask(_mm512_loadu_si512(destVector++), _mm512_loadu_si512(sourceVector++));
		result = result & cmp;
		if (result != -1) {
			return false;
		}
		if (remainder < 16 && remainder > 0) {
			return (memcmp(destVector, sourceVector, remainder) == 0);
		}
		if (remainder < 32 && remainder > 0) {
			return compareStringsSmall16Avx512(destVector, sourceVector, remainder);
		}
		if (remainder > 0) {
			return compareStringsSmall32Avx512(destVector, sourceVector, remainder);
		}
		return true;
	}

	inline bool compareStringsSmall128Avx512(const void* str1, const void* str2, size_t lengthNew) noexcept {
		const size_t remainder{ lengthNew % 128 };
		auto* destVector = reinterpret_cast<const __m512i*>(str1);
		const auto* sourceVector = reinterpret_cast<const __m512i*>(str2);
		__mmask64 result{ std::numeric_limits<uint64_t>::max() };
		auto cmp = _mm512_cmpeq_epi8_mask(_mm512_loadu_si512(destVector++), _mm512_loadu_si512(sourceVector++));
		result = result & cmp;
		cmp = _mm512_cmpeq_epi8_mask(_mm512_loadu_si512(destVector++), _mm512_loadu_si512(sourceVector++));
		result = result & cmp;
		if (result != -1) {
			return false;
		}
		if (remainder < 16 && remainder > 0) {
			return (memcmp(destVector, sourceVector, remainder) == 0);
		}
		if (remainder < 32 && remainder > 0) {
			return compareStringsSmall16Avx512(destVector, sourceVector, remainder);
		}
		if (remainder < 64 && remainder > 0) {
			return compareStringsSmall32Avx512(destVector, sourceVector, remainder);
		}
		if (remainder > 0) {
			return compareStringsSmall64Avx512(destVector, sourceVector, remainder);
		}
		return true;
	}

	inline bool compareStringsSmall256Avx512(const void* str1, const void* str2, size_t lengthNew) noexcept {
		const size_t remainder{ lengthNew % 256 };
		auto* destVector = reinterpret_cast<const __m512i*>(str1);
		const auto* sourceVector = reinterpret_cast<const __m512i*>(str2);
		__mmask64 result{ std::numeric_limits<uint64_t>::max() };
		__mmask64 cmp{};
		for (size_t x = lengthNew / (sizeof(__m512i) * 8); x > 0; --x) {
			cmp = _mm512_cmpeq_epi8_mask(_mm512_loadu_si512(destVector++), _mm512_loadu_si512(sourceVector++));
			result = result & cmp;
			cmp = _mm512_cmpeq_epi8_mask(_mm512_loadu_si512(destVector++), _mm512_loadu_si512(sourceVector++));
			result = result & cmp;
			cmp = _mm512_cmpeq_epi8_mask(_mm512_loadu_si512(destVector++), _mm512_loadu_si512(sourceVector++));
			result = result & cmp;
			cmp = _mm512_cmpeq_epi8_mask(_mm512_loadu_si512(destVector++), _mm512_loadu_si512(sourceVector++));
			result = result & cmp;
			if (result != -1) {
				return false;
			}
		}
		if (remainder < 16 && remainder > 0) {
			return (memcmp(destVector, sourceVector, remainder) == 0);
		}
		if (remainder < 32 && remainder > 0) {
			return compareStringsSmall16Avx512(destVector, sourceVector, remainder);
		}
		if (remainder < 64 && remainder > 0) {
			return compareStringsSmall32Avx512(destVector, sourceVector, remainder);
		}
		if (remainder < 128 && remainder > 0) {
			return compareStringsSmall64Avx512(destVector, sourceVector, remainder);
		}
		if (remainder > 0) {
			return compareStringsSmall128Avx512(destVector, sourceVector, remainder);
		}
		return true;
	}

	inline bool compareStringsSmallAvx512(const void* str1, const void* str2, size_t length) noexcept {
		if (length < 16) {
			return (memcmp(str1, str2, length) == 0);
		} else if (length < 32) {
			return compareStringsSmall16Avx512(str1, str2, length);
		} else if (length < 64) {
			return compareStringsSmall32Avx512(str1, str2, length);
		} else if (length < 128) {
			return compareStringsSmall64Avx512(str1, str2, length);
		} else if (length < 256) {
			return compareStringsSmall128Avx512(str1, str2, length);
		} else {
			return compareStringsSmall256Avx512(str1, str2, length);
		}
	}

	inline bool compareStrings16Avx512(const void* str1, const void* str2, size_t lengthNew) noexcept {
		const size_t remainder{ lengthNew % 16 };
		auto* destVector = static_cast<const __m128i*>(str1);
		const auto* sourceVector = static_cast<const __m128i*>(str2);
		if (_mm_movemask_epi8(_mm_and_si128(_mm_set1_epi64x(-1ll),
				_mm_cmpeq_epi8(_mm_loadu_si128(reinterpret_cast<const __m128i*>(destVector++)), _mm_loadu_si128(sourceVector++)))) != (0x0000ffff)) {
			return false;
		}
		_mm_sfence();
		if (remainder > 0) {
			return (memcmp(destVector, sourceVector, remainder) == 0);
		}
		return true;
	}

	inline bool compareStrings32Avx512(const void* str1, const void* str2, size_t lengthNew) noexcept {
		const size_t remainder{ lengthNew % 32 };
		auto* destVector = reinterpret_cast<const __m256i*>(str1);
		const auto* sourceVector = reinterpret_cast<const __m256i*>(str2);
		if (_mm256_movemask_epi8(_mm256_and_si256(_mm256_set1_epi64x(-1ll),
				_mm256_cmpeq_epi8(_mm256_load_si256(destVector++), _mm256_load_si256(sourceVector++)))) != (0xffffffff)) {
			return false;
		}
		_mm_sfence();
		if (remainder < 16 && remainder > 0) {
			return (memcmp(destVector, sourceVector, remainder) == 0);
		}
		if (remainder > 0) {
			return compareStringsSmall16Avx512(destVector, sourceVector, remainder);
		}
		return true;
	}

	inline bool compareStrings64Avx512(const void* str1, const void* str2, size_t lengthNew) noexcept {
		const size_t remainder{ lengthNew % 64 };
		auto* destVector = reinterpret_cast<const __m512i*>(str1);
		const auto* sourceVector = reinterpret_cast<const __m512i*>(str2);
		__mmask64 result{ std::numeric_limits<uint64_t>::max() };
		auto cmp = _mm512_cmpeq_epi8_mask(_mm512_load_si512(destVector++), _mm512_load_si512(sourceVector++));
		result = result & cmp;
		_mm_sfence();
		if (result != -1) {
			return false;
		}
		if (remainder < 16 && remainder > 0) {
			return (memcmp(destVector, sourceVector, remainder) == 0);
		}
		if (remainder < 32 && remainder > 0) {
			return compareStrings16Avx512(destVector, sourceVector, remainder);
		}
		if (remainder > 0) {
			return compareStrings32Avx512(destVector, sourceVector, remainder);
		}
		return true;
	}

	inline bool compareStrings128Avx512(const void* str1, const void* str2, size_t lengthNew) noexcept {
		const size_t remainder{ lengthNew % 128 };
		auto* destVector = reinterpret_cast<const __m512i*>(str1);
		const auto* sourceVector = reinterpret_cast<const __m512i*>(str2);
		__mmask64 result{ std::numeric_limits<uint64_t>::max() };
		auto cmp = _mm512_cmpeq_epi8_mask(_mm512_load_si512(destVector++), _mm512_load_si512(sourceVector++));
		result = result & cmp;
		cmp = _mm512_cmpeq_epi8_mask(_mm512_load_si512(destVector++), _mm512_load_si512(sourceVector++));
		result = result & cmp;
		if (result != -1) {
			return false;
		}
		_mm_sfence();
		if (remainder < 16 && remainder > 0) {
			return (memcmp(destVector, sourceVector, remainder) == 0);
		}
		if (remainder < 32 && remainder > 0) {
			return compareStrings16Avx512(destVector, sourceVector, remainder);
		}
		if (remainder < 64 && remainder > 0) {
			return compareStrings32Avx512(destVector, sourceVector, remainder);
		}
		if (remainder > 0) {
			return compareStrings64Avx512(destVector, sourceVector, remainder);
		}
		return true;
	}

	inline bool compareStrings256Avx512(const void* str1, const void* str2, size_t lengthNew) noexcept {
		const size_t remainder{ lengthNew % 256 };
		auto* destVector = reinterpret_cast<const __m512i*>(str1);
		const auto* sourceVector = reinterpret_cast<const __m512i*>(str2);
		__mmask64 result{ std::numeric_limits<uint64_t>::max() };
		__mmask64 cmp{};
		for (size_t x = lengthNew / (sizeof(__m512i) * 8); x > 0; --x) {
			cmp = _mm512_cmpeq_epi8_mask(_mm512_load_si512(destVector++), _mm512_load_si512(sourceVector++));
			result = result & cmp;
			cmp = _mm512_cmpeq_epi8_mask(_mm512_load_si512(destVector++), _mm512_load_si512(sourceVector++));
			result = result & cmp;
			cmp = _mm512_cmpeq_epi8_mask(_mm512_load_si512(destVector++), _mm512_load_si512(sourceVector++));
			result = result & cmp;
			cmp = _mm512_cmpeq_epi8_mask(_mm512_load_si512(destVector++), _mm512_load_si512(sourceVector++));
			result = result & cmp;
			if (result != -1) {
				return false;
			}
		}
		_mm_sfence();
		if (remainder < 16 && remainder > 0) {
			return (memcmp(destVector, sourceVector, remainder) == 0);
		}
		if (remainder < 32 && remainder > 0) {
			return compareStrings16Avx512(destVector, sourceVector, remainder);
		}
		if (remainder < 64 && remainder > 0) {
			return compareStrings32Avx512(destVector, sourceVector, remainder);
		}
		if (remainder < 128 && remainder > 0) {
			return compareStrings64Avx512(destVector, sourceVector, remainder);
		}
		if (remainder > 0) {
			return compareStrings128Avx512(destVector, sourceVector, remainder);
		}
		return true;
	}

	inline bool compareFast(const void* destVector, const void* sourceVector, size_t lengthNew) noexcept {
		if (lengthNew < 256ull * 1024ull) {
			return compareStringsSmallAvx512(destVector, sourceVector, lengthNew);
		} else {
			return compareStrings256Avx512(destVector, sourceVector, lengthNew);
		}
	}
#elif INSTRUCTION_SET_TYPE_AVX2
	inline size_t findSingleCharacterAvx(const void* string, size_t lengthNew, const char charToFind) noexcept {
		size_t result{ std::string::npos };
		size_t currentIndex{ lengthNew / 32 };
		const auto arrayChar{ _mm256_set1_epi8(charToFind) };
		auto newPtr = reinterpret_cast<const __m256i*>(string);
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
			::memcpy(charArray, string, lengthNew - remainder);
			mask = _mm256_movemask_epi8(_mm256_cmpeq_epi8(arrayChar, _mm256_loadu_epi8(charArray)));

			if (mask != 0) {
				result = currentIndex + _tzcnt_u32(mask);
				return result;
			}
			currentIndex += remainder;
		}

		return result;
	}

	inline bool compareStringsSmall16Avx2(const void* str1, const void* str2, size_t lengthNew) noexcept {
		const size_t remainder{ lengthNew % 16 };
		auto* destVector = static_cast<const __m128i*>(str1);
		const auto* sourceVector = static_cast<const __m128i*>(str2);
		if (_mm_movemask_epi8(_mm_and_si128(_mm_set1_epi64x(-1ll), _mm_cmpeq_epi8(_mm_loadu_si128(destVector++), _mm_loadu_si128(sourceVector++)))) !=
			(0x0000ffff)) {
			return false;
		}
		if (remainder > 0) {
			return (memcmp(destVector, sourceVector, remainder) == 0);
		}
		return true;
	}

	inline bool compareStringsSmall32Avx2(const void* str1, const void* str2, size_t lengthNew) noexcept {
		const size_t remainder{ lengthNew % 32 };
		auto* destVector = reinterpret_cast<const __m256i*>(str1);
		const auto* sourceVector = reinterpret_cast<const __m256i*>(str2);
		if (_mm256_movemask_epi8(_mm256_and_si256(_mm256_set1_epi64x(-1ll),
				_mm256_cmpeq_epi8(_mm256_loadu_si256(destVector++), _mm256_loadu_si256(sourceVector++)))) != (0xffffffff)) {
			return false;
		}
		if (remainder < 16 && remainder > 0) {
			return (memcmp(destVector, sourceVector, remainder) == 0);
		}
		if (remainder > 0) {
			return compareStringsSmall16Avx2(destVector, sourceVector, remainder);
		}
		return true;
	}

	inline bool compareStringsSmall64Avx2(const void* str1, const void* str2, size_t lengthNew) noexcept {
		const size_t remainder{ lengthNew % 64 };
		auto* destVector = reinterpret_cast<const __m256i*>(str1);
		const auto* sourceVector = reinterpret_cast<const __m256i*>(str2);
		__m256i result{ _mm256_set1_epi64x(-1ll) };
		__m256i cmp = _mm256_cmpeq_epi8(_mm256_loadu_si256(destVector++), _mm256_loadu_si256(sourceVector++));
		result = _mm256_and_si256(result, cmp);
		cmp = _mm256_cmpeq_epi8(_mm256_loadu_si256(destVector++), _mm256_loadu_si256(sourceVector++));
		result = _mm256_and_si256(result, cmp);
		if (_mm256_movemask_epi8(result) != (0xffffffff)) {
			return false;
		}
		if (remainder < 16 && remainder > 0) {
			return (memcmp(destVector, sourceVector, remainder) == 0);
		}
		if (remainder < 32 && remainder > 0) {
			return compareStringsSmall16Avx2(destVector, sourceVector, remainder);
		}
		if (remainder > 0) {
			return compareStringsSmall32Avx2(destVector, sourceVector, remainder);
		}
		return true;
	}

	inline bool compareStringsSmall128Avx2(const void* str1, const void* str2, size_t lengthNew) noexcept {
		const size_t remainder{ lengthNew % 128 };
		auto* destVector = reinterpret_cast<const __m256i*>(str1);
		const auto* sourceVector = reinterpret_cast<const __m256i*>(str2);
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
		if (remainder < 16 && remainder > 0) {
			return (memcmp(destVector, sourceVector, remainder) == 0);
		}
		if (remainder < 32 && remainder > 0) {
			return compareStringsSmall16Avx2(destVector, sourceVector, remainder);
		}
		if (remainder < 64 && remainder > 0) {
			return compareStringsSmall32Avx2(destVector, sourceVector, remainder);
		}
		if (remainder > 0) {
			return compareStringsSmall64Avx2(destVector, sourceVector, remainder);
		}
		return true;
	}

	inline bool compareStringsSmall256Avx2(const void* str1, const void* str2, size_t lengthNew) noexcept {
		const size_t remainder{ lengthNew % 256 };
		auto* destVector = reinterpret_cast<const __m256i*>(str1);
		const auto* sourceVector = reinterpret_cast<const __m256i*>(str2);
		__m256i result{ _mm256_set1_epi64x(-1ll) };
		__m256i cmp{};
		uint32_t mask{};
		for (size_t x = lengthNew / (sizeof(__m256i) * 8); x > 0; --x) {
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
		if (remainder < 16 && remainder > 0) {
			return (memcmp(destVector, sourceVector, remainder) == 0);
		}
		if (remainder < 32 && remainder > 0) {
			return compareStringsSmall16Avx2(destVector, sourceVector, remainder);
		}
		if (remainder < 64 && remainder > 0) {
			return compareStringsSmall32Avx2(destVector, sourceVector, remainder);
		}
		if (remainder < 128 && remainder > 0) {
			return compareStringsSmall64Avx2(destVector, sourceVector, remainder);
		}
		if (remainder > 0) {
			return compareStringsSmall128Avx2(destVector, sourceVector, remainder);
		}
		return true;
	}

	inline bool compareStringsSmallAvx2(const void* str1, const void* str2, size_t length) noexcept {
		if (length < 16) {
			return (memcmp(str1, str2, length) == 0);
		} else if (length < 32) {
			return compareStringsSmall16Avx2(str1, str2, length);
		} else if (length < 64) {
			return compareStringsSmall32Avx2(str1, str2, length);
		} else if (length < 128) {
			return compareStringsSmall64Avx2(str1, str2, length);
		} else if (length < 256) {
			return compareStringsSmall128Avx2(str1, str2, length);
		} else {
			return compareStringsSmall256Avx2(str1, str2, length);
		}
	}

	inline bool compareStrings16Avx2(const void* str1, const void* str2, size_t lengthNew) noexcept {
		const size_t remainder{ lengthNew % 16 };
		auto* destVector = static_cast<const __m128i*>(str1);
		const auto* sourceVector = static_cast<const __m128i*>(str2);
		if (_mm_movemask_epi8(_mm_and_si128(_mm_set1_epi64x(-1ll),
				_mm_cmpeq_epi8(_mm_loadu_si128(reinterpret_cast<const __m128i*>(destVector++)), _mm_loadu_si128(sourceVector++)))) != (0x0000ffff)) {
			return false;
		}
		_mm_sfence();
		if (remainder > 0) {
			return (memcmp(destVector, sourceVector, remainder) == 0);
		}
		return true;
	}

	inline bool compareStrings32Avx2(const void* str1, const void* str2, size_t lengthNew) noexcept {
		const size_t remainder{ lengthNew % 32 };
		auto* destVector = reinterpret_cast<const __m256i*>(str1);
		const auto* sourceVector = reinterpret_cast<const __m256i*>(str2);
		if (_mm256_movemask_epi8(_mm256_and_si256(_mm256_set1_epi64x(-1ll),
				_mm256_cmpeq_epi8(_mm256_stream_load_si256(destVector++), _mm256_stream_load_si256(sourceVector++)))) != (0xffffffff)) {
			return false;
		}
		_mm_sfence();
		if (remainder < 16 && remainder > 0) {
			return (memcmp(destVector, sourceVector, remainder) == 0);
		}
		if (remainder > 0) {
			return compareStringsSmall16Avx2(destVector, sourceVector, remainder);
		}
		return true;
	}

	inline bool compareStrings64Avx2(const void* str1, const void* str2, size_t lengthNew) noexcept {
		const size_t remainder{ lengthNew % 64 };
		auto* destVector = reinterpret_cast<const __m256i*>(str1);
		const auto* sourceVector = reinterpret_cast<const __m256i*>(str2);
		__m256i result{ _mm256_set1_epi64x(-1ll) };
		__m256i cmp = _mm256_cmpeq_epi8(_mm256_stream_load_si256(destVector++), _mm256_stream_load_si256(sourceVector++));
		result = _mm256_and_si256(result, cmp);
		cmp = _mm256_cmpeq_epi8(_mm256_stream_load_si256(destVector++), _mm256_stream_load_si256(sourceVector++));
		result = _mm256_and_si256(result, cmp);
		_mm_sfence();
		if (_mm256_movemask_epi8(result) != (0xffffffff)) {
			return false;
		}
		if (remainder < 16 && remainder > 0) {
			return (memcmp(destVector, sourceVector, remainder) == 0);
		}
		if (remainder < 32 && remainder > 0) {
			return compareStrings16Avx2(destVector, sourceVector, remainder);
		}
		if (remainder > 0) {
			return compareStrings32Avx2(destVector, sourceVector, remainder);
		}
		return true;
	}

	inline bool compareStrings128Avx2(const void* str1, const void* str2, size_t lengthNew) noexcept {
		const size_t remainder{ lengthNew % 128 };
		auto* destVector = reinterpret_cast<const __m256i*>(str1);
		const auto* sourceVector = reinterpret_cast<const __m256i*>(str2);
		__m256i result{ _mm256_set1_epi64x(-1ll) };
		__m256i cmp = _mm256_cmpeq_epi8(_mm256_stream_load_si256(destVector++), _mm256_stream_load_si256(sourceVector++));
		result = _mm256_and_si256(result, cmp);
		cmp = _mm256_cmpeq_epi8(_mm256_stream_load_si256(destVector++), _mm256_stream_load_si256(sourceVector++));
		result = _mm256_and_si256(result, cmp);
		cmp = _mm256_cmpeq_epi8(_mm256_stream_load_si256(destVector++), _mm256_stream_load_si256(sourceVector++));
		result = _mm256_and_si256(result, cmp);
		cmp = _mm256_cmpeq_epi8(_mm256_stream_load_si256(destVector++), _mm256_stream_load_si256(sourceVector++));
		result = _mm256_and_si256(result, cmp);
		if (_mm256_movemask_epi8(result) != (0xffffffff)) {
			return false;
		}
		_mm_sfence();
		if (remainder < 16 && remainder > 0) {
			return (memcmp(destVector, sourceVector, remainder) == 0);
		}
		if (remainder < 32 && remainder > 0) {
			return compareStrings16Avx2(destVector, sourceVector, remainder);
		}
		if (remainder < 64 && remainder > 0) {
			return compareStrings32Avx2(destVector, sourceVector, remainder);
		}
		if (remainder > 0) {
			return compareStrings64Avx2(destVector, sourceVector, remainder);
		}
		return true;
	}

	inline bool compareStrings256Avx2(const void* str1, const void* str2, size_t lengthNew) noexcept {
		const size_t remainder{ lengthNew % 256 };
		auto* destVector = reinterpret_cast<const __m256i*>(str1);
		const auto* sourceVector = reinterpret_cast<const __m256i*>(str2);
		__m256i result{ _mm256_set1_epi64x(-1ll) };
		__m256i cmp{};
		uint32_t mask{};
		for (size_t x = lengthNew / (sizeof(__m256i) * 8); x > 0; --x) {
			cmp = _mm256_cmpeq_epi8(_mm256_stream_load_si256(destVector++), _mm256_stream_load_si256(sourceVector++));
			result = _mm256_and_si256(result, cmp);
			cmp = _mm256_cmpeq_epi8(_mm256_stream_load_si256(destVector++), _mm256_stream_load_si256(sourceVector++));
			result = _mm256_and_si256(result, cmp);
			cmp = _mm256_cmpeq_epi8(_mm256_stream_load_si256(destVector++), _mm256_stream_load_si256(sourceVector++));
			result = _mm256_and_si256(result, cmp);
			cmp = _mm256_cmpeq_epi8(_mm256_stream_load_si256(destVector++), _mm256_stream_load_si256(sourceVector++));
			result = _mm256_and_si256(result, cmp);
			cmp = _mm256_cmpeq_epi8(_mm256_stream_load_si256(destVector++), _mm256_stream_load_si256(sourceVector++));
			result = _mm256_and_si256(result, cmp);
			cmp = _mm256_cmpeq_epi8(_mm256_stream_load_si256(destVector++), _mm256_stream_load_si256(sourceVector++));
			result = _mm256_and_si256(result, cmp);
			cmp = _mm256_cmpeq_epi8(_mm256_stream_load_si256(destVector++), _mm256_stream_load_si256(sourceVector++));
			result = _mm256_and_si256(result, cmp);
			cmp = _mm256_cmpeq_epi8(_mm256_stream_load_si256(destVector++), _mm256_stream_load_si256(sourceVector++));
			result = _mm256_and_si256(result, cmp);
			mask = _mm256_movemask_epi8(result);
			if (mask != (0xffffffff)) {
				return false;
			}
		}
		_mm_sfence();
		if (remainder < 16 && remainder > 0) {
			return (memcmp(destVector, sourceVector, remainder) == 0);
		}
		if (remainder < 32 && remainder > 0) {
			return compareStrings16Avx2(destVector, sourceVector, remainder);
		}
		if (remainder < 64 && remainder > 0) {
			return compareStrings32Avx2(destVector, sourceVector, remainder);
		}
		if (remainder < 128 && remainder > 0) {
			return compareStrings64Avx2(destVector, sourceVector, remainder);
		}
		if (remainder > 0) {
			return compareStrings128Avx2(destVector, sourceVector, remainder);
		}
		return true;
	}

	inline bool compareFast(const void* destVector, const void* sourceVector, size_t lengthNew) noexcept {
		if (lengthNew < 256ull * 1024ull) {
			return compareStringsSmallAvx2(destVector, sourceVector, lengthNew);
		} else {
			return compareStrings256Avx2(destVector, sourceVector, lengthNew);
		}
	}
#elif INSTRUCTION_SET_TYPE_AVX
	inline size_t findSingleCharacterAvx(const void* string, size_t lengthNew, const char charToFind) noexcept {
		size_t result{ std::string::npos };
		size_t currentIndex{ lengthNew / 32 };
		const auto arrayChar{ _mm256_set1_epi8(charToFind) };
		auto newPtr = reinterpret_cast<const __m256i*>(string);
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
			::memcpy(charArray, string, lengthNew - remainder);
			mask = _mm256_movemask_epi8(_mm256_cmpeq_epi8(arrayChar, _mm256_loadu_epi8(charArray)));

			if (mask != 0) {
				result = currentIndex + _tzcnt_u32(mask);
				return result;
			}
			currentIndex += remainder;
		}

		return result;
	}

	inline bool compareStringsSmall16Avx(const void* str1, const void* str2, size_t lengthNew) noexcept {
		const size_t remainder{ lengthNew % 16 };
		auto* destVector = static_cast<const __m128i*>(str1);
		const auto* sourceVector = static_cast<const __m128i*>(str2);
		if (_mm_movemask_epi8(_mm_and_si128(_mm_set1_epi64x(-1ll), _mm_cmpeq_epi8(_mm_loadu_si128(destVector++), _mm_loadu_si128(sourceVector++)))) !=
			(0x0000ffff)) {
			return false;
		}
		if (remainder > 0) {
			return (memcmp(destVector, sourceVector, remainder) == 0);
		}
		return true;
	}

	inline bool compareStringsSmall32Avx(const void* str1, const void* str2, size_t lengthNew) noexcept {
		const size_t remainder{ lengthNew % 32 };
		auto* destVector = reinterpret_cast<const __m256i*>(str1);
		const auto* sourceVector = reinterpret_cast<const __m256i*>(str2);
		if (_mm256_movemask_epi8(_mm256_and_si256(_mm256_set1_epi64x(-1ll),
				_mm256_cmpeq_epi8(_mm256_loadu_si256(destVector++), _mm256_loadu_si256(sourceVector++)))) != (0xffffffff)) {
			return false;
		}
		if (remainder < 16 && remainder > 0) {
			return (memcmp(destVector, sourceVector, remainder) == 0);
		}
		if (remainder > 0) {
			return compareStringsSmall16Avx(destVector, sourceVector, remainder);
		}
		return true;
	}

	inline bool compareStringsSmall64Avx(const void* str1, const void* str2, size_t lengthNew) noexcept {
		const size_t remainder{ lengthNew % 64 };
		auto* destVector = reinterpret_cast<const __m256i*>(str1);
		const auto* sourceVector = reinterpret_cast<const __m256i*>(str2);
		__m256i result{ _mm256_set1_epi64x(-1ll) };
		__m256i cmp = _mm256_cmpeq_epi8(_mm256_loadu_si256(destVector++), _mm256_loadu_si256(sourceVector++));
		result = _mm256_and_si256(result, cmp);
		cmp = _mm256_cmpeq_epi8(_mm256_loadu_si256(destVector++), _mm256_loadu_si256(sourceVector++));
		result = _mm256_and_si256(result, cmp);
		if (_mm256_movemask_epi8(result) != (0xffffffff)) {
			return false;
		}
		if (remainder < 16 && remainder > 0) {
			return (memcmp(destVector, sourceVector, remainder) == 0);
		}
		if (remainder < 32 && remainder > 0) {
			return compareStringsSmall16Avx(destVector, sourceVector, remainder);
		}
		if (remainder > 0) {
			return compareStringsSmall32Avx(destVector, sourceVector, remainder);
		}
		return true;
	}

	inline bool compareStringsSmall128Avx(const void* str1, const void* str2, size_t lengthNew) noexcept {
		const size_t remainder{ lengthNew % 128 };
		auto* destVector = reinterpret_cast<const __m256i*>(str1);
		const auto* sourceVector = reinterpret_cast<const __m256i*>(str2);
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
		if (remainder < 16 && remainder > 0) {
			return (memcmp(destVector, sourceVector, remainder) == 0);
		}
		if (remainder < 32 && remainder > 0) {
			return compareStringsSmall16Avx(destVector, sourceVector, remainder);
		}
		if (remainder < 64 && remainder > 0) {
			return compareStringsSmall32Avx(destVector, sourceVector, remainder);
		}
		if (remainder > 0) {
			return compareStringsSmall64Avx(destVector, sourceVector, remainder);
		}
		return true;
	}

	inline bool compareStringsSmall256Avx(const void* str1, const void* str2, size_t lengthNew) noexcept {
		const size_t remainder{ lengthNew % 256 };
		auto* destVector = reinterpret_cast<const __m256i*>(str1);
		const auto* sourceVector = reinterpret_cast<const __m256i*>(str2);
		__m256i result{ _mm256_set1_epi64x(-1ll) };
		__m256i cmp{};
		uint32_t mask{};
		for (size_t x = lengthNew / (sizeof(__m256i) * 8); x > 0; --x) {
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
		if (remainder < 16 && remainder > 0) {
			return (memcmp(destVector, sourceVector, remainder) == 0);
		}
		if (remainder < 32 && remainder > 0) {
			return compareStringsSmall16Avx(destVector, sourceVector, remainder);
		}
		if (remainder < 64 && remainder > 0) {
			return compareStringsSmall32Avx(destVector, sourceVector, remainder);
		}
		if (remainder < 128 && remainder > 0) {
			return compareStringsSmall64Avx(destVector, sourceVector, remainder);
		}
		if (remainder > 0) {
			return compareStringsSmall128Avx(destVector, sourceVector, remainder);
		}
		return true;
	}

	inline bool compareStringsSmallAvx(const void* str1, const void* str2, size_t length) noexcept {
		if (length < 16) {
			return (memcmp(str1, str2, length) == 0);
		} else if (length < 32) {
			return compareStringsSmall16Avx(str1, str2, length);
		} else if (length < 64) {
			return compareStringsSmall32Avx(str1, str2, length);
		} else if (length < 128) {
			return compareStringsSmall64Avx(str1, str2, length);
		} else if (length < 256) {
			return compareStringsSmall128Avx(str1, str2, length);
		} else {
			return compareStringsSmall256Avx(str1, str2, length);
		}
	}

	inline bool compareStrings16Avx(const void* str1, const void* str2, size_t lengthNew) noexcept {
		const size_t remainder{ lengthNew % 16 };
		auto* destVector = static_cast<const __m128i*>(str1);
		const auto* sourceVector = static_cast<const __m128i*>(str2);
		if (_mm_movemask_epi8(_mm_and_si128(_mm_set1_epi64x(-1ll),
				_mm_cmpeq_epi8(_mm_loadu_si128(reinterpret_cast<const __m128i*>(destVector++)), _mm_loadu_si128(sourceVector++)))) != (0x0000ffff)) {
			return false;
		}
		_mm_sfence();
		if (remainder > 0) {
			return (memcmp(destVector, sourceVector, remainder) == 0);
		}
		return true;
	}

	inline bool compareStrings32Avx(const void* str1, const void* str2, size_t lengthNew) noexcept {
		const size_t remainder{ lengthNew % 32 };
		auto* destVector = reinterpret_cast<const __m256i*>(str1);
		const auto* sourceVector = reinterpret_cast<const __m256i*>(str2);
		if (_mm256_movemask_epi8(_mm256_and_si256(_mm256_set1_epi64x(-1ll),
				_mm256_cmpeq_epi8(_mm256_stream_load_si256(destVector++), _mm256_stream_load_si256(sourceVector++)))) != (0xffffffff)) {
			return false;
		}
		_mm_sfence();
		if (remainder < 16 && remainder > 0) {
			return (memcmp(destVector, sourceVector, remainder) == 0);
		}
		if (remainder > 0) {
			return compareStringsSmall16Avx(destVector, sourceVector, remainder);
		}
		return true;
	}

	inline bool compareStrings64Avx(const void* str1, const void* str2, size_t lengthNew) noexcept {
		const size_t remainder{ lengthNew % 64 };
		auto* destVector = reinterpret_cast<const __m256i*>(str1);
		const auto* sourceVector = reinterpret_cast<const __m256i*>(str2);
		__m256i result{ _mm256_set1_epi64x(-1ll) };
		__m256i cmp = _mm256_cmpeq_epi8(_mm256_stream_load_si256(destVector++), _mm256_stream_load_si256(sourceVector++));
		result = _mm256_and_si256(result, cmp);
		cmp = _mm256_cmpeq_epi8(_mm256_stream_load_si256(destVector++), _mm256_stream_load_si256(sourceVector++));
		result = _mm256_and_si256(result, cmp);
		_mm_sfence();
		if (_mm256_movemask_epi8(result) != (0xffffffff)) {
			return false;
		}
		if (remainder < 16 && remainder > 0) {
			return (memcmp(destVector, sourceVector, remainder) == 0);
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
		auto* destVector = reinterpret_cast<const __m256i*>(str1);
		const auto* sourceVector = reinterpret_cast<const __m256i*>(str2);
		__m256i result{ _mm256_set1_epi64x(-1ll) };
		__m256i cmp = _mm256_cmpeq_epi8(_mm256_stream_load_si256(destVector++), _mm256_stream_load_si256(sourceVector++));
		result = _mm256_and_si256(result, cmp);
		cmp = _mm256_cmpeq_epi8(_mm256_stream_load_si256(destVector++), _mm256_stream_load_si256(sourceVector++));
		result = _mm256_and_si256(result, cmp);
		cmp = _mm256_cmpeq_epi8(_mm256_stream_load_si256(destVector++), _mm256_stream_load_si256(sourceVector++));
		result = _mm256_and_si256(result, cmp);
		cmp = _mm256_cmpeq_epi8(_mm256_stream_load_si256(destVector++), _mm256_stream_load_si256(sourceVector++));
		result = _mm256_and_si256(result, cmp);
		if (_mm256_movemask_epi8(result) != (0xffffffff)) {
			return false;
		}
		_mm_sfence();
		if (remainder < 16 && remainder > 0) {
			return (memcmp(destVector, sourceVector, remainder) == 0);
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
		const size_t remainder{ lengthNew % 256 };
		auto* destVector = reinterpret_cast<const __m256i*>(str1);
		const auto* sourceVector = reinterpret_cast<const __m256i*>(str2);
		__m256i result{ _mm256_set1_epi64x(-1ll) };
		__m256i cmp{};
		uint32_t mask{};
		for (size_t x = lengthNew / (sizeof(__m256i) * 8); x > 0; --x) {
			cmp = _mm256_cmpeq_epi8(_mm256_stream_load_si256(destVector++), _mm256_stream_load_si256(sourceVector++));
			result = _mm256_and_si256(result, cmp);
			cmp = _mm256_cmpeq_epi8(_mm256_stream_load_si256(destVector++), _mm256_stream_load_si256(sourceVector++));
			result = _mm256_and_si256(result, cmp);
			cmp = _mm256_cmpeq_epi8(_mm256_stream_load_si256(destVector++), _mm256_stream_load_si256(sourceVector++));
			result = _mm256_and_si256(result, cmp);
			cmp = _mm256_cmpeq_epi8(_mm256_stream_load_si256(destVector++), _mm256_stream_load_si256(sourceVector++));
			result = _mm256_and_si256(result, cmp);
			cmp = _mm256_cmpeq_epi8(_mm256_stream_load_si256(destVector++), _mm256_stream_load_si256(sourceVector++));
			result = _mm256_and_si256(result, cmp);
			cmp = _mm256_cmpeq_epi8(_mm256_stream_load_si256(destVector++), _mm256_stream_load_si256(sourceVector++));
			result = _mm256_and_si256(result, cmp);
			cmp = _mm256_cmpeq_epi8(_mm256_stream_load_si256(destVector++), _mm256_stream_load_si256(sourceVector++));
			result = _mm256_and_si256(result, cmp);
			cmp = _mm256_cmpeq_epi8(_mm256_stream_load_si256(destVector++), _mm256_stream_load_si256(sourceVector++));
			result = _mm256_and_si256(result, cmp);
			mask = _mm256_movemask_epi8(result);
			if (mask != (0xffffffff)) {
				return false;
			}
		}
		_mm_sfence();
		if (remainder < 16 && remainder > 0) {
			return (memcmp(destVector, sourceVector, remainder) == 0);
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

	inline bool compareFast(const void* destVector, const void* sourceVector, size_t lengthNew) noexcept {
		if (lengthNew < 256ull * 1024ull) {
			return compareStringsSmallAvx(destVector, sourceVector, lengthNew);
		} else {
			return compareStrings256Avx(destVector, sourceVector, lengthNew);
		}
	}
#else
	inline bool compareFast(const void* destVector, const void* sourceVector, size_t lengthNew) noexcept {
		return std::string_view{ reinterpret_cast<const char*>(destVector) } == std::string_view{ reinterpret_cast<const char*>(sourceVector) };
	}
#endif
}