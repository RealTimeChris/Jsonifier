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
#include <jsonifier/StringView.hpp>

namespace Jsonifier {

	inline size_t findSingleCharacter(const char* const string, size_t lengthNew, const char charToFind) noexcept {
		size_t result{ std::string::npos };
		size_t currentIndex{ lengthNew / 32 };
		const auto arrayChar{ _mm256_set1_epi8(charToFind) };
		auto newPtr = reinterpret_cast<const __m256i*>(string);
		size_t remainder{ lengthNew % 32 };
		int32_t mask{};
		for (; currentIndex > 0; --currentIndex) {
			mask = _mm256_movemask_epi8(_mm256_cmpeq_epi8(arrayChar, _mm256_load_si256(newPtr++)));

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

	inline bool compareStringsSmall16(const void* str1, const void* str2, size_t lengthNew) noexcept {
		const size_t remainder{ lengthNew % 16 };
		auto* destVector = static_cast<const __m128i*>(str1);
		const auto* sourceVector = static_cast<const __m128i*>(str2);
		if (_mm_movemask_epi8(_mm_and_si128(_mm_set1_epi64x(-1ull), _mm_cmpeq_epi8(_mm_load_si128(destVector++), _mm_load_si128(sourceVector++)))) !=
			(0x0000ffff)) {
			return false;
		}
		if (remainder > 0) {
			return (memcmp(destVector, sourceVector, remainder) == 0);
		}
		return true;
	}

	inline bool compareStringsSmall32(const void* str1, const void* str2, size_t lengthNew) noexcept {
		const size_t remainder{ lengthNew % 32 };
		auto* destVector = reinterpret_cast<const __m256i*>(str1);
		const auto* sourceVector = reinterpret_cast<const __m256i*>(str2);
		if (_mm256_movemask_epi8(_mm256_and_si256(_mm256_set1_epi64x(-1ull),
				_mm256_cmpeq_epi8(_mm256_load_si256(destVector++), _mm256_load_si256(sourceVector++)))) != (0xffffffff)) {
			return false;
		}
		if (remainder < 16 && remainder > 0) {
			return (memcmp(destVector, sourceVector, remainder) == 0);
		}
		if (remainder > 0) {
			return compareStringsSmall16(destVector, sourceVector, remainder);
		}
		return true;
	}

	inline bool compareStringsSmall64(const void* str1, const void* str2, size_t lengthNew) noexcept {
		const size_t remainder{ lengthNew % 64 };
		auto* destVector = reinterpret_cast<const __m256i*>(str1);
		const auto* sourceVector = reinterpret_cast<const __m256i*>(str2);
		__m256i result{ _mm256_set1_epi64x(-1ull) };
		__m256i cmp = _mm256_cmpeq_epi8(_mm256_load_si256(destVector++), _mm256_load_si256(sourceVector++));
		result = _mm256_and_si256(result, cmp);
		cmp = _mm256_cmpeq_epi8(_mm256_load_si256(destVector++), _mm256_load_si256(sourceVector++));
		result = _mm256_and_si256(result, cmp);
		if (_mm256_movemask_epi8(result) != (0xffffffff)) {
			return false;
		}
		if (remainder < 16 && remainder > 0) {
			return (memcmp(destVector, sourceVector, remainder) == 0);
		}
		if (remainder < 32 && remainder > 0) {
			return compareStringsSmall16(destVector, sourceVector, remainder);
		}
		if (remainder > 0) {
			return compareStringsSmall32(destVector, sourceVector, remainder);
		}
		return true;
	}

	inline bool compareStringsSmall128(const void* str1, const void* str2, size_t lengthNew) noexcept {
		const size_t remainder{ lengthNew % 128 };
		auto* destVector = reinterpret_cast<const __m256i*>(str1);
		const auto* sourceVector = reinterpret_cast<const __m256i*>(str2);
		__m256i result{ _mm256_set1_epi64x(-1ull) };
		__m256i cmp = _mm256_cmpeq_epi8(_mm256_load_si256(destVector++), _mm256_load_si256(sourceVector++));
		result = _mm256_and_si256(result, cmp);
		cmp = _mm256_cmpeq_epi8(_mm256_load_si256(destVector++), _mm256_load_si256(sourceVector++));
		result = _mm256_and_si256(result, cmp);
		cmp = _mm256_cmpeq_epi8(_mm256_load_si256(destVector++), _mm256_load_si256(sourceVector++));
		result = _mm256_and_si256(result, cmp);
		cmp = _mm256_cmpeq_epi8(_mm256_load_si256(destVector++), _mm256_load_si256(sourceVector++));
		result = _mm256_and_si256(result, cmp);
		if (_mm256_movemask_epi8(result) != (0xffffffff)) {
			return false;
		}
		if (remainder < 16 && remainder > 0) {
			return (memcmp(destVector, sourceVector, remainder) == 0);
		}
		if (remainder < 32 && remainder > 0) {
			return compareStringsSmall16(destVector, sourceVector, remainder);
		}
		if (remainder < 64 && remainder > 0) {
			return compareStringsSmall32(destVector, sourceVector, remainder);
		}
		if (remainder > 0) {
			return compareStringsSmall64(destVector, sourceVector, remainder);
		}
		return true;
	}

	inline bool compareStringsSmall256(const void* str1, const void* str2, size_t lengthNew) noexcept {
		const size_t remainder{ lengthNew % 256 };
		auto* destVector = reinterpret_cast<const __m256i*>(str1);
		const auto* sourceVector = reinterpret_cast<const __m256i*>(str2);
		__m256i result{ _mm256_set1_epi64x(-1ull) };
		__m256i cmp{};
		uint32_t mask{};
		for (size_t x = lengthNew / (sizeof(__m256i) * 8); x > 0; --x) {
			cmp = _mm256_cmpeq_epi8(_mm256_load_si256(destVector++), _mm256_load_si256(sourceVector++));
			result = _mm256_and_si256(result, cmp);
			cmp = _mm256_cmpeq_epi8(_mm256_load_si256(destVector++), _mm256_load_si256(sourceVector++));
			result = _mm256_and_si256(result, cmp);
			cmp = _mm256_cmpeq_epi8(_mm256_load_si256(destVector++), _mm256_load_si256(sourceVector++));
			result = _mm256_and_si256(result, cmp);
			cmp = _mm256_cmpeq_epi8(_mm256_load_si256(destVector++), _mm256_load_si256(sourceVector++));
			result = _mm256_and_si256(result, cmp);
			cmp = _mm256_cmpeq_epi8(_mm256_load_si256(destVector++), _mm256_load_si256(sourceVector++));
			result = _mm256_and_si256(result, cmp);
			cmp = _mm256_cmpeq_epi8(_mm256_load_si256(destVector++), _mm256_load_si256(sourceVector++));
			result = _mm256_and_si256(result, cmp);
			cmp = _mm256_cmpeq_epi8(_mm256_load_si256(destVector++), _mm256_load_si256(sourceVector++));
			result = _mm256_and_si256(result, cmp);
			cmp = _mm256_cmpeq_epi8(_mm256_load_si256(destVector++), _mm256_load_si256(sourceVector++));
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
			return compareStringsSmall16(destVector, sourceVector, remainder);
		}
		if (remainder < 64 && remainder > 0) {
			return compareStringsSmall32(destVector, sourceVector, remainder);
		}
		if (remainder < 128 && remainder > 0) {
			return compareStringsSmall64(destVector, sourceVector, remainder);
		}
		if (remainder > 0) {
			return compareStringsSmall128(destVector, sourceVector, remainder);
		}
		return true;
	}

	inline bool compareStringsSmall(const char* str1, const char* str2, size_t length) noexcept {
		if (length < 16) {
			return (memcmp(str1, str2, length) == 0);
		} else if (length < 32) {
			return compareStringsSmall16(str1, str2, length);
		} else if (length < 64) {
			return compareStringsSmall32(str1, str2, length);
		} else if (length < 128) {
			return compareStringsSmall64(str1, str2, length);
		} else if (length < 256) {
			return compareStringsSmall128(str1, str2, length);
		} else {
			return compareStringsSmall256(str1, str2, length);
		}
	}

	inline bool compareStrings16(const void* str1, const void* str2, size_t lengthNew) noexcept {
		const size_t remainder{ lengthNew % 16 };
		auto* destVector = static_cast<const __m128i*>(str1);
		const auto* sourceVector = static_cast<const __m128i*>(str2);
		if (_mm_movemask_epi8(_mm_and_si128(_mm_set1_epi64x(-1ull),
				_mm_cmpeq_epi8(_mm_stream_load_si128(destVector++), _mm_stream_load_si128(sourceVector++)))) != (0x0000ffff)) {
			return false;
		}
		_mm_sfence();
		if (remainder > 0) {
			return (memcmp(destVector, sourceVector, remainder) == 0);
		}
		return true;
	}

	inline bool compareStrings32(const void* str1, const void* str2, size_t lengthNew) noexcept {
		const size_t remainder{ lengthNew % 32 };
		auto* destVector = reinterpret_cast<const __m256i*>(str1);
		const auto* sourceVector = reinterpret_cast<const __m256i*>(str2);
		if (_mm256_movemask_epi8(_mm256_and_si256(_mm256_set1_epi64x(-1ull),
				_mm256_cmpeq_epi8(_mm256_stream_load_si256(destVector++), _mm256_stream_load_si256(sourceVector++)))) != (0xffffffff)) {
			return false;
		}
		_mm_sfence();
		if (remainder < 16 && remainder > 0) {
			return (memcmp(destVector, sourceVector, remainder) == 0);
		}
		if (remainder > 0) {
			return compareStringsSmall16(destVector, sourceVector, remainder);
		}
		return true;
	}

	inline bool compareStrings64(const void* str1, const void* str2, size_t lengthNew) noexcept {
		const size_t remainder{ lengthNew % 64 };
		auto* destVector = reinterpret_cast<const __m256i*>(str1);
		const auto* sourceVector = reinterpret_cast<const __m256i*>(str2);
		__m256i result{ _mm256_set1_epi64x(-1ull) };
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
			return compareStrings16(destVector, sourceVector, remainder);
		}
		if (remainder > 0) {
			return compareStrings32(destVector, sourceVector, remainder);
		}
		return true;
	}

	inline bool compareStrings128(const void* str1, const void* str2, size_t lengthNew) noexcept {
		const size_t remainder{ lengthNew % 128 };
		auto* destVector = reinterpret_cast<const __m256i*>(str1);
		const auto* sourceVector = reinterpret_cast<const __m256i*>(str2);
		__m256i result{ _mm256_set1_epi64x(-1ull) };
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
			return compareStrings16(destVector, sourceVector, remainder);
		}
		if (remainder < 64 && remainder > 0) {
			return compareStrings32(destVector, sourceVector, remainder);
		}
		if (remainder > 0) {
			return compareStrings64(destVector, sourceVector, remainder);
		}
		return true;
	}

	inline bool compareStrings256(const void* str1, const void* str2, size_t lengthNew) noexcept {
		const size_t remainder{ lengthNew % 256 };
		auto* destVector = reinterpret_cast<const __m256i*>(str1);
		const auto* sourceVector = reinterpret_cast<const __m256i*>(str2);
		__m256i result{ _mm256_set1_epi64x(-1ull) };
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
			return compareStrings16(destVector, sourceVector, remainder);
		}
		if (remainder < 64 && remainder > 0) {
			return compareStrings32(destVector, sourceVector, remainder);
		}
		if (remainder < 128 && remainder > 0) {
			return compareStrings64(destVector, sourceVector, remainder);
		}
		if (remainder > 0) {
			return compareStrings128(destVector, sourceVector, remainder);
		}
		return true;
	}

	inline bool compare(const char* destVector, const char* sourceVector, size_t lengthNew) noexcept {
		if (lengthNew < 256ull * 1024ull) {
			return compareStringsSmall(destVector, sourceVector, lengthNew);
		} else {
			return compareStrings256(destVector, sourceVector, lengthNew);
		}
	}

	inline void fastMemcpySmall16(void* dest, const void* source, size_t lengthNew) noexcept {
		auto remainder{ lengthNew % 16 };
		auto* destVector = static_cast<__m128i*>(dest);
		const auto* sourceVector = static_cast<const __m128i*>(source);
		_mm_store_si128(destVector++, _mm_load_si128(sourceVector++));

		_mm_sfence();
		if (remainder > 0) {
			::memcpy(destVector, sourceVector, remainder);
			return;
		}
	}

	inline void fastMemcpySmall32(void* dest, const void* source, size_t lengthNew) noexcept {
		auto remainder{ lengthNew % 32 };
		auto* destVector = static_cast<__m256i*>(dest);
		const auto* sourceVector = static_cast<const __m256i*>(source);
		_mm256_store_si256(destVector++, _mm256_load_si256(sourceVector++));

		_mm_sfence();
		if (remainder < 16 && remainder > 0) {
			::memcpy(destVector, sourceVector, remainder);
			return;
		}
		if (remainder > 0) {
			return fastMemcpySmall16(destVector, sourceVector, remainder);
		}
	}

	inline void fastMemcpySmall64(void* dest, const void* source, size_t lengthNew) {
		auto remainder{ lengthNew % 64 };
		auto* destVector = static_cast<__m256i*>(dest);
		const auto* sourceVector = static_cast<const __m256i*>(source);
		_mm_prefetch(reinterpret_cast<const char*>(sourceVector + 1), _MM_HINT_T0);
		_mm256_store_si256(destVector++, _mm256_load_si256(sourceVector++));
		_mm256_store_si256(destVector++, _mm256_load_si256(sourceVector++));

		_mm_sfence();
		if (remainder < 16 && remainder > 0) {
			::memcpy(destVector, sourceVector, remainder);
			return;
		}
		if (remainder < 32 && remainder > 0) {
			return fastMemcpySmall16(destVector, sourceVector, remainder);
		}
		if (remainder > 0) {
			return fastMemcpySmall32(destVector, sourceVector, remainder);
		}
	}

	inline void fastMemcpySmall128(void* dest, const void* source, size_t lengthNew) {
		auto remainder{ lengthNew % 128 };
		auto* destVector = static_cast<__m256i*>(dest);
		const auto* sourceVector = static_cast<const __m256i*>(source);
		_mm_prefetch(reinterpret_cast<const char*>(sourceVector + 2), _MM_HINT_T0);
		_mm256_stream_si256(destVector++, _mm256_load_si256(sourceVector++));
		_mm256_stream_si256(destVector++, _mm256_load_si256(sourceVector++));
		_mm256_stream_si256(destVector++, _mm256_load_si256(sourceVector++));
		_mm256_stream_si256(destVector++, _mm256_load_si256(sourceVector++));

		_mm_sfence();
		if (remainder < 16 && remainder > 0) {
			::memcpy(destVector, sourceVector, remainder);
			return;
		}
		if (remainder < 32 && remainder > 0) {
			return fastMemcpySmall16(destVector, sourceVector, remainder);
		}
		if (remainder < 64 && remainder > 0) {
			return fastMemcpySmall32(destVector, sourceVector, remainder);
		}
		if (remainder > 0) {
			return fastMemcpySmall64(destVector, sourceVector, remainder);
		}
	}

	inline void fastMemcpySmall256(void* dest, const void* source, size_t lengthNew) noexcept {
		auto remainder{ lengthNew % 256 };
		auto* destVector = static_cast<__m256i*>(dest);
		const auto* sourceVector = static_cast<const __m256i*>(source);
		for (size_t x = lengthNew / (sizeof(__m256i) * 8); x > 0; --x) {
			_mm_prefetch(reinterpret_cast<const char*>(sourceVector + 2), _MM_HINT_T0);
			_mm256_stream_si256(destVector++, _mm256_load_si256(sourceVector++));
			_mm256_stream_si256(destVector++, _mm256_load_si256(sourceVector++));
			_mm_prefetch(reinterpret_cast<const char*>(sourceVector + 2), _MM_HINT_T0);
			_mm256_stream_si256(destVector++, _mm256_load_si256(sourceVector++));
			_mm256_stream_si256(destVector++, _mm256_load_si256(sourceVector++));
			_mm_prefetch(reinterpret_cast<const char*>(sourceVector + 2), _MM_HINT_T0);
			_mm256_stream_si256(destVector++, _mm256_load_si256(sourceVector++));
			_mm256_stream_si256(destVector++, _mm256_load_si256(sourceVector++));
			_mm_prefetch(reinterpret_cast<const char*>(sourceVector + 2), _MM_HINT_T0);
			_mm256_stream_si256(destVector++, _mm256_load_si256(sourceVector++));
			_mm256_stream_si256(destVector++, _mm256_load_si256(sourceVector++));
		}

		_mm_sfence();
		if (remainder < 16 && remainder > 0) {
			::memcpy(destVector, sourceVector, remainder);
			return;
		}
		if (remainder < 32 && remainder > 0) {
			return fastMemcpySmall16(destVector, sourceVector, remainder);
		}
		if (remainder < 64 && remainder > 0) {
			return fastMemcpySmall32(destVector, sourceVector, remainder);
		}
		if (remainder < 128 && remainder > 0) {
			return fastMemcpySmall64(destVector, sourceVector, remainder);
		}
		if (remainder > 0) {
			return fastMemcpySmall128(destVector, sourceVector, remainder);
		}
	}

	inline void fastMemcpySmall(void* destVector, const void* sourceVector, size_t lengthNew) noexcept {
		if (lengthNew < 16) {
			::memcpy(destVector, sourceVector, lengthNew);
		} else if (lengthNew < 32) {
			fastMemcpySmall16(destVector, sourceVector, lengthNew);
		} else if (lengthNew < 64) {
			fastMemcpySmall32(destVector, sourceVector, lengthNew);
		} else if (lengthNew < 128) {
			fastMemcpySmall64(destVector, sourceVector, lengthNew);
		} else if (lengthNew < 256) {
			fastMemcpySmall128(destVector, sourceVector, lengthNew);
		} else {
			fastMemcpySmall256(destVector, sourceVector, lengthNew);
		}
	};

	inline void fastMemcpy16(void* dest, const void* source, size_t lengthNew) noexcept {
		auto remainder{ lengthNew % 16 };
		auto* destVector = static_cast<__m128i*>(dest);
		const auto* sourceVector = static_cast<const __m128i*>(source);
		_mm_stream_si128(destVector++, _mm_stream_load_si128(sourceVector++));
		_mm_sfence();
		if (remainder > 0) {
			::memcpy(destVector, sourceVector, remainder);
		}
	}

	inline void fastMemcpy32(void* dest, const void* source, size_t lengthNew) noexcept {
		auto remainder{ lengthNew % 32 };
		auto* destVector = static_cast<__m256i*>(dest);
		const auto* sourceVector = static_cast<const __m256i*>(source);
		_mm256_stream_si256(destVector++, _mm256_stream_load_si256(sourceVector++));
		_mm_sfence();
		if (remainder < 16 && remainder > 0) {
			::memcpy(destVector, sourceVector, remainder);
			return;
		}
		if (remainder > 0) {
			return fastMemcpy16(destVector, sourceVector, remainder);
		}
	}

	inline void fastMemcpy64(void* dest, const void* source, size_t lengthNew) noexcept {
		auto remainder{ lengthNew % 64 };
		auto* destVector = static_cast<__m256i*>(dest);
		const auto* sourceVector = static_cast<const __m256i*>(source);
		_mm256_stream_si256(destVector++, _mm256_stream_load_si256(sourceVector++));
		_mm256_stream_si256(destVector++, _mm256_stream_load_si256(sourceVector++));
		_mm_sfence();
		if (remainder < 16 && remainder > 0) {
			::memcpy(destVector, sourceVector, remainder);
			return;
		}
		if (remainder < 32 && remainder > 0) {
			return fastMemcpy16(destVector, sourceVector, remainder);
		}
		if (remainder > 0) {
			return fastMemcpy32(destVector, sourceVector, remainder);
		}
	}

	inline void fastMemcpy128(void* dest, const void* source, size_t lengthNew) noexcept {
		auto remainder{ lengthNew % 128 };
		auto* destVector = static_cast<__m256i*>(dest);
		const auto* sourceVector = static_cast<const __m256i*>(source);
		_mm256_stream_si256(destVector++, _mm256_stream_load_si256(sourceVector++));
		_mm256_stream_si256(destVector++, _mm256_stream_load_si256(sourceVector++));
		_mm256_stream_si256(destVector++, _mm256_stream_load_si256(sourceVector++));
		_mm256_stream_si256(destVector++, _mm256_stream_load_si256(sourceVector++));
		_mm_sfence();
		if (remainder < 16 && remainder > 0) {
			::memcpy(destVector, sourceVector, remainder);
			return;
		}
		if (remainder < 32 && remainder > 0) {
			return fastMemcpy16(destVector, sourceVector, remainder);
		}
		if (remainder < 64 && remainder > 0) {
			return fastMemcpy32(destVector, sourceVector, remainder);
		}
		if (remainder > 0) {
			return fastMemcpy64(destVector, sourceVector, remainder);
		}
	}

	inline void fastMemcpy256(void* dest, const void* source, size_t lengthNew) noexcept {
		auto remainder{ lengthNew % 256 };
		auto* destVector = static_cast<__m256i*>(dest);
		const auto* sourceVector = static_cast<const __m256i*>(source);
		for (size_t x = lengthNew / (sizeof(__m256i) * 8); x > 0; --x) {
			_mm256_stream_si256(destVector++, _mm256_stream_load_si256(sourceVector++));
			_mm256_stream_si256(destVector++, _mm256_stream_load_si256(sourceVector++));
			_mm256_stream_si256(destVector++, _mm256_stream_load_si256(sourceVector++));
			_mm256_stream_si256(destVector++, _mm256_stream_load_si256(sourceVector++));
			_mm256_stream_si256(destVector++, _mm256_stream_load_si256(sourceVector++));
			_mm256_stream_si256(destVector++, _mm256_stream_load_si256(sourceVector++));
			_mm256_stream_si256(destVector++, _mm256_stream_load_si256(sourceVector++));
			_mm256_stream_si256(destVector++, _mm256_stream_load_si256(sourceVector++));
		}
		_mm_sfence();
		if (remainder < 16 && remainder > 0) {
			::memcpy(destVector, sourceVector, remainder);
			return;
		}
		if (remainder < 32 && remainder > 0) {
			return fastMemcpy16(destVector, sourceVector, remainder);
		}
		if (remainder < 64 && remainder > 0) {
			return fastMemcpy32(destVector, sourceVector, remainder);
		}
		if (remainder < 128 && remainder > 0) {
			return fastMemcpy64(destVector, sourceVector, remainder);
		}
		if (remainder > 0) {
			return fastMemcpy128(destVector, sourceVector, remainder);
		}
	}

	inline void memcpy(void* destVector, const void* sourceVector, size_t lengthNew) noexcept {
		if (lengthNew < 256ull * 1024ull) {
			fastMemcpySmall(destVector, sourceVector, lengthNew);
			return;
		} else {
			fastMemcpy256(destVector, sourceVector, lengthNew);
		}
	}

	template<typename OTy> class MemoryPoolAllocator {
	  public:
		using value_type = OTy;
		using pointer = OTy*;
		using const_pointer = const OTy*;
		using size_type = std::size_t;
		using difference_type = std::ptrdiff_t;

		MemoryPoolAllocator() = default;
		MemoryPoolAllocator(const MemoryPoolAllocator&) = default;
		MemoryPoolAllocator& operator=(const MemoryPoolAllocator&) = default;

		template<typename U> MemoryPoolAllocator(const MemoryPoolAllocator<U>&) noexcept {
		}

		pointer allocate(size_type n) {
			if (n == 0) {
				return nullptr;
			}
			if (n <= MaxSize) {
				if (auto p = static_cast<pointer>(std::malloc(n * sizeof(value_type)))) {
					return p;
				}
			}
			throw std::bad_alloc();
		}

		void deallocate(pointer p, size_type n) {
			std::free(p);
		}

	  private:
		static constexpr size_type MaxSize = 1024;
	};

	template<typename OTy> struct MemoryCore {
	  public:
		using AllocatorType = MemoryPoolAllocator<OTy>;
		using AllocatorTraits = std::allocator_traits<AllocatorType>;

		inline MemoryCore() = default;

		inline MemoryCore& operator=(MemoryCore&& other) noexcept {
			if (this != &other) {
				currentSize = other.currentSize;
				objects = other.objects;
				other.objects = nullptr;
				other.currentSize = 0;
			}
			return *this;
		}

		inline MemoryCore(MemoryCore&& other) noexcept {
			*this = std::move(other);
		}

		inline MemoryCore& operator=(const MemoryCore& other) noexcept {
			if (this != &other) {
				this->resize(other.currentSize);
				if (std::copyable<OTy>) {
					Jsonifier::memcpy(objects, other.objects, other.currentSize);
				} else {
					for (size_t x = 0; x < currentSize; ++x) {
						objects[x] = std::move(other.objects[x]);
					}
				}
			}
			return *this;
		}

		inline constexpr MemoryCore(OTy* data) noexcept {
			objects = data;
		}

		inline MemoryCore(const MemoryCore& other) noexcept {
			*this = other;
		}

		inline OTy& operator[](size_t index) noexcept {
			return objects[index];
		}

		inline operator OTy*() noexcept {
			return objects;
		}

		inline size_t size() const {
			return currentSize;
		}

		inline OTy* data() const {
			return objects;
		}

		inline constexpr void resize(size_t newSize) noexcept {
			if (newSize != 0 && newSize != currentSize) {
				if (this->objects) {
					objects = static_cast<OTy*>(_aligned_realloc(objects, newSize, 32));
				} else {
					objects = static_cast<OTy*>(_aligned_malloc(newSize, 32));
				}
				currentSize = newSize;
			} else if (newSize == 0) {
				deallocate();
			}
		}

		inline ~MemoryCore() noexcept {
			deallocate();
		}

	  protected:
		size_t currentSize{};
		OTy* objects{};

		inline void deallocate() noexcept {
			if (currentSize > 0 && objects) {
				AllocatorType allocator{};
				for (size_t x = 0; x < currentSize; ++x) {
					AllocatorTraits::destroy(allocator, &objects[x]);
				}
				_aligned_free(objects);
				objects = nullptr;
				currentSize = 0;
			}
		}
	};

	struct  SimdBase128 {
	  public:
		inline SimdBase128(){};

		inline SimdBase128& operator=(SimdBase128&& other) noexcept {
			this->value = other.value;
			return *this;
		}

		inline SimdBase128(SimdBase128&& other) noexcept {
			*this = std::move(other);
		}

		inline SimdBase128& operator=(const SimdBase128& other) noexcept {
			this->value = other.value;
			return *this;
		}

		inline SimdBase128(const SimdBase128& other) noexcept {
			*this = other;
		}

		inline SimdBase128& operator=(__m128i&& other) noexcept {
			value = std::forward<__m128i>(other);
			return *this;
		}

		inline SimdBase128(__m128i&& other) noexcept {
			*this = std::move(other);
		}

		inline explicit SimdBase128(char other) noexcept {
			value = _mm_set1_epi8(other);
		}

		inline SimdBase128& operator=(const uint8_t* values) noexcept {
			value = _mm_loadu_epi8(values);
			return *this;
		}

		inline SimdBase128(const uint8_t* values) noexcept {
			*this = values;
		}

		inline SimdBase128& operator=(uint64_t* values) noexcept {
			this->value = _mm_loadu_epi64(values);
			return *this;
		}

		inline SimdBase128(uint64_t* values) noexcept {
			*this = values;
		}

		inline operator const __m128i&&() const {
			return std::forward<const __m128i>(value);
		}

		inline SimdBase128 operator|(SimdBase128&& other) noexcept {
			return _mm_or_si128(value, std::forward<SimdBase128>(other));
		}

		inline SimdBase128 operator|(SimdBase128& other) const {
			return _mm_or_si128(value, other);
		}

		inline SimdBase128 operator&(SimdBase128& other) noexcept {
			return _mm_and_si128(value, other);
		}

		inline SimdBase128 operator^(SimdBase128& other) noexcept {
			return _mm_xor_si128(value, other);
		}

		inline SimdBase128 operator==(const SimdBase128& other) const {
			return _mm_cmpeq_epi8(value, other);
		}

		inline SimdBase128 operator==(uint8_t other) noexcept {
			return _mm_cmpeq_epi8(value, _mm_set1_epi8(other));
		}

		inline SimdBase128 operator~() noexcept {
			return _mm_xor_si128(*this, _mm_set1_epi64x(-1ull));
		}

		inline SimdBase128 bitAndNot(SimdBase128&& other) noexcept {
			return _mm_andnot_si128(std::forward<SimdBase128>(other), value);
		}

		inline SimdBase128 bitAndNot(SimdBase128& other) noexcept {
			return _mm_andnot_si128(other, value);
		}

		inline SimdBase128 shuffle(SimdBase128& other) const {
			return _mm_shuffle_epi8(other, value);
		}

		inline void addValues(const SimdBase128&& values, size_t index) noexcept {
			*(reinterpret_cast<int32_t*>(&value) + index) = _mm_movemask_epi8(values);
		}

		inline int32_t toBitMask() noexcept {
			return _mm_movemask_epi8(*this);
		}

		inline void store(uint8_t destVector[16]) noexcept {
			_mm_store_si128(reinterpret_cast<__m128i*>(destVector), value);
		}

		inline void printBits(uint64_t values, const String& valuesTitle) noexcept {
			//std::cout<< valuesTitle;
			//std::cout<< std::bitset<64>{ values };
			//std::cout<< std::endl;
		}

		inline SimdBase128 printBits(const String& valuesTitle) noexcept {
			//std::cout<< valuesTitle;
			for (size_t x = 0; x < 32; ++x) {
				for (size_t y = 0; y < 8; ++y) {
					//std::cout<< std::bitset<1>{ static_cast<size_t>(*(static_cast<int8_t*>(&value) + x)) >> y };
				}
			}
			//std::cout<< std::endl;
			return *this;
		}


	  protected:
		__m128i value{};
	};

	struct  SimdBase256 {
	  public:
		inline SimdBase256(){};

		inline SimdBase256& operator=(SimdBase256&& other) noexcept {
			this->value = other.value;
			return *this;
		}

		inline SimdBase256(SimdBase256&& other) noexcept {
			*this = std::move(other);
		}

		inline SimdBase256& operator=(const SimdBase256& other) noexcept {
			this->value = other.value;
			return *this;
		}

		inline SimdBase256(const SimdBase256& other) noexcept {
			*this = other;
		}

		inline SimdBase256& operator=(__m256i&& other) noexcept {
			value = std::forward<__m256i>(other);
			return *this;
		}

		inline SimdBase256(__m256i&& other) noexcept {
			*this = std::move(other);
		}

		inline SimdBase256& operator=(char other) noexcept {
			value = _mm256_set1_epi8(other);
			return *this;
		}

		inline SimdBase256(char other) noexcept {
			*this = other;
		}

		inline SimdBase256& operator=(const uint8_t* values) noexcept {
			value = _mm256_loadu_epi8(values);
			return *this;
		}

		inline SimdBase256(const uint8_t* values) noexcept {
			*this = values;
		}

		inline SimdBase256& operator=(uint64_t* values) noexcept {
			this->value = _mm256_loadu_epi64(values);
			return *this;
		}

		inline SimdBase256(uint64_t* values) noexcept {
			*this = values;
		}

		inline operator const __m256i&&() const {
			return std::forward<const __m256i>(value);
		}

		inline SimdBase256 operator|(SimdBase256&& other) noexcept {
			return _mm256_or_si256(value, std::forward<SimdBase256>(other));
		}

		inline SimdBase256 operator|(SimdBase256& other) const {
			return _mm256_or_si256(value, other);
		}

		inline SimdBase256 operator&(SimdBase256& other) noexcept {
			return _mm256_and_si256(value, other);
		}

		inline SimdBase256 operator^(SimdBase256& other) noexcept {
			return _mm256_xor_si256(value, other);
		}

		inline SimdBase256 operator==(const SimdBase256& other) const {
			return _mm256_cmpeq_epi8(value, other);
		}

		inline SimdBase256 operator==(uint8_t other) noexcept {
			return _mm256_cmpeq_epi8(value, _mm256_set1_epi8(other));
		}

		inline SimdBase256 operator~() noexcept {
			return _mm256_xor_si256(*this, _mm256_set1_epi64x(-1ull));
		}

		inline void convertWhitespaceToSimdBase256(const SimdBase256* valuesNew) noexcept {
			SimdBase256 whitespaceTable{ { ' ', 100, 100, 100, 17, 100, 113, 2, 100, '\t', '\n', 112, 100, '\r', 100, 100, ' ', 100, 100, 100, 17,
				100, 113, 2, 100, '\t', '\n', 112, 100, '\r', 100, 100 } };
			for (size_t x = 0; x < 8; ++x) {
				addValues(valuesNew[x].shuffle(whitespaceTable) == valuesNew[x], x);
			}
		};

		inline void convertBackslashesToSimdBase256(const SimdBase256* valuesNew) noexcept {
			SimdBase256 backslashes{ _mm256_set1_epi8('\\') };
			for (size_t x = 0; x < 8; ++x) {
				addValues(valuesNew[x] == backslashes, x);
			}
		};

		inline void convertStructuralsToSimdBase256(const SimdBase256* valuesNew) noexcept {
			SimdBase256 opTable{ { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, ':', '{', ',', '}', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, ':', '{', ',', '}', 0, 0 } };
			SimdBase256 chars{ char{ 0x20 } };
			for (size_t x = 0; x < 8; ++x) {
				addValues({ valuesNew[x].shuffle(opTable) == (valuesNew[x] | chars) }, x);
			}
		};

		inline void convertQuotesToSimdBase256(const SimdBase256* valuesNew) noexcept {
			SimdBase256 quotes = _mm256_set1_epi8('"');
			for (size_t x = 0; x < 8; ++x) {
				addValues(valuesNew[x] == quotes, x);
			}
		};

		inline SimdBase256 bitAndNot(SimdBase256&& other) noexcept {
			return _mm256_andnot_si256(std::forward<SimdBase256>(other), value);
		}

		inline SimdBase256 bitAndNot(SimdBase256& other) noexcept {
			return _mm256_andnot_si256(other, value);
		}

		inline SimdBase256 shuffle(SimdBase256& other) const {
			return _mm256_shuffle_epi8(other, value);
		}

		inline void addValues(const SimdBase256&& values, size_t index) noexcept {
			*(reinterpret_cast<int32_t*>(&value) + index) = _mm256_movemask_epi8(values);
		}

		template<size_t amount> inline SimdBase256 shl() noexcept {
			return SimdBase256{ _mm256_slli_epi64(*this, (amount % 64)) } |
				_mm256_srli_epi64(_mm256_permute4x64_epi64(*this, 0b10010011), 64 - (amount % 64));
		}

		inline int32_t toBitMask() noexcept {
			return _mm256_movemask_epi8(*this);
		}

		inline void store(uint8_t destVector[32]) noexcept {
			_mm256_store_si256(reinterpret_cast<__m256i*>(destVector), value);
		}

		inline int64_t getInt64(size_t index) noexcept {
			switch (index) {
				case 0: {
					return _mm256_extract_epi64(value, 0);
				}
				case 1: {
					return _mm256_extract_epi64(value, 1);
				}
				case 2: {
					return _mm256_extract_epi64(value, 2);
				}
				case 3: {
					return _mm256_extract_epi64(value, 3);
				}
				default: {
					return _mm256_extract_epi64(value, 0);
				}
			}
		}

		inline void insertInt64(int64_t valueNew, size_t index) noexcept {
			switch (index) {
				case 0: {
					value = _mm256_insert_epi64(value, valueNew, 0);
					break;
				}
				case 1: {
					value = _mm256_insert_epi64(value, valueNew, 1);
					break;
				}
				case 2: {
					value = _mm256_insert_epi64(value, valueNew, 2);
					break;
				}
				case 3: {
					value = _mm256_insert_epi64(value, valueNew, 3);
					break;
				}
				default: {
					value = _mm256_insert_epi64(value, valueNew, 0);
					break;
				}
			}
		}

		inline void setFirstBit(bool onOrOff) noexcept {
			if (onOrOff) {
				insertInt64(getInt64(0) | 1ll, 0);
			} else {
				insertInt64(getInt64(0) & ~1ll, 0);
			}
		}

		inline bool checkLastBit() noexcept {
			return (getInt64(3) >> 63 & 1) << 63;
		}

		inline SimdBase256 carrylessMultiplication(uint64_t& prevInString) noexcept {
			SimdBase128 allOnes{ '\xFF' };
			SimdBase256 valuesNew{};
			uint64_t* values = reinterpret_cast<uint64_t*>(&valuesNew);
			values[0] = _mm_cvtsi128_si64(_mm_clmulepi64_si128(_mm256_extracti128_si256(value, 0), allOnes, 0)) ^ prevInString;
			prevInString = uint64_t(static_cast<int64_t>(values[0]) >> 63);
			values[1] = _mm_cvtsi128_si64(_mm_clmulepi64_si128(_mm256_extracti128_si256(value, 0), allOnes, 1)) ^ prevInString;
			prevInString = uint64_t(static_cast<int64_t>(values[1]) >> 63);
			values[2] = _mm_cvtsi128_si64(_mm_clmulepi64_si128(_mm256_extracti128_si256(value, 1), allOnes, 0)) ^ prevInString;
			prevInString = uint64_t(static_cast<int64_t>(values[2]) >> 63);
			values[3] = _mm_cvtsi128_si64(_mm_clmulepi64_si128(_mm256_extracti128_si256(value, 1), allOnes, 1)) ^ prevInString;
			prevInString = uint64_t(static_cast<int64_t>(values[3]) >> 63);
			return valuesNew;
		}

		inline bool collectCarries(SimdBase256& other1, SimdBase256& result) noexcept {
			bool returnValue{};
			uint64_t returnValue64{};
			for (size_t x = 0; x < 4; ++x) {
				if (_addcarry_u64(0, getInt64(x), other1.getInt64(x), &returnValue64)) {
					returnValue = true;
				} else {
					returnValue = false;
				}
				result.insertInt64(returnValue64, x);
			}
			return returnValue;
		}

		inline void printBits(uint64_t values, const String& valuesTitle) noexcept {
			//std::cout<< valuesTitle;
			//std::cout<< std::bitset<64>{ values };
			//std::cout<< std::endl;
		}

		inline SimdBase256 printBits(const String& valuesTitle) noexcept {
			//std::cout<< valuesTitle;
			for (size_t x = 0; x < 32; ++x) {
				for (size_t y = 0; y < 8; ++y) {
					//std::cout<< std::bitset<1>{ static_cast<size_t>(*(static_cast<int8_t*>(&value) + x)) >> y };
				}
			}
			//std::cout<< std::endl;
			return *this;
		}

	  protected:
		__m256i value{};
	};

	template<size_t StepSize> class StringBlockReader {
	  public:
		__forceinline StringBlockReader(StringViewPtr stringViewNew, size_t _len) noexcept {
			lengthMinusStep = _len < StepSize ? 0 : _len - StepSize;
			inString = stringViewNew;
			length = _len;
			index = 0;
		}

		__forceinline size_t getRemainder(StringBufferPtr dst) noexcept {
			if (length == index) {
				return 0;
			}
			memset(dst, 0x20, StepSize);
			memmove(dst, inString + index, (length - index));
			return length - index;
		}

		__forceinline StringViewPtr fullBlock() noexcept {
			StringViewPtr newPtr = inString + index;
			index += StepSize;
			return newPtr;
		}

		__forceinline bool hasFullBlock() noexcept {
			return index < lengthMinusStep;
		}

	  protected:
		StringViewPtr inString{};
		size_t lengthMinusStep{};
		size_t length{};
		size_t index{};
	};

	class  SimdStringReader {
	  public:
		__forceinline SimdStringReader() noexcept {};

		__forceinline void reset(size_t stringLengthRawNew, StringViewPtr stringViewNew) noexcept {
			structuralIndices.resize(round(stringLengthRawNew + 3, 256));
			stringLengthRaw = stringLengthRawNew;
			stringView = stringViewNew;
			stringIndex = 0;
			tapeIndex = 0;
			x = 0;
		}

		__forceinline void generateStructurals(StringViewPtr valueNew) noexcept {
			SimdBase256 newPtr[8];
			for (size_t y = 0; y < 8; ++y) {
				newPtr[y] = _mm256_loadu_epi8(valueNew + (32 * y));
			}
			whitespace.convertWhitespaceToSimdBase256(newPtr);
			backslash.convertBackslashesToSimdBase256(newPtr);
			op.convertStructuralsToSimdBase256(newPtr);
			quotes.convertQuotesToSimdBase256(newPtr);
			collectFinalStructurals();
			addTapeValues();
			stringIndex += 256;
		}

		__forceinline StringViewPtr getStringView() {
			return this->stringView;
		}

		__forceinline size_t getStringLength() {
			return this->stringLengthRaw;
		}

		__forceinline StructuralIndex getStructurals() {
			return this->structuralIndices.data();
		}

		__forceinline size_t getTapeLength() noexcept {
			return tapeIndex;
		}

	  protected:
		std::vector<uint32_t> structuralIndices{};
		StringViewPtr stringView{};
		SimdBase256 structurals{};
		size_t stringLengthRaw{};
		SimdBase256 whitespace{};
		uint64_t prevInString{};
		SimdBase256 backslash{};
		size_t stringIndex{};
		SimdBase256 quotes{};
		bool prevInScalar{};
		bool prevEscaped{};
		size_t tapeIndex{};
		SimdBase256 op{};
		int32_t x{};

		uint64_t round(int64_t a, int64_t size) noexcept {
			return (((a) + (( size )-1)) & ~(( size )-1));
		}

		__forceinline int64_t rollValuesIntoTape(size_t currentIndex, size_t y, int64_t newBits) noexcept {
			structuralIndices[(currentIndex * 8) + tapeIndex] = static_cast<uint32_t>(_tzcnt_u64(newBits) + (y * 64ull) + stringIndex);
			newBits = _blsr_u64(newBits);
			structuralIndices[1 + (currentIndex * 8) + tapeIndex] = static_cast<uint32_t>(_tzcnt_u64(newBits) + (y * 64ull) + stringIndex);
			newBits = _blsr_u64(newBits);
			structuralIndices[2 + (currentIndex * 8) + tapeIndex] = static_cast<uint32_t>(_tzcnt_u64(newBits) + (y * 64ull) + stringIndex);
			newBits = _blsr_u64(newBits);
			structuralIndices[3 + (currentIndex * 8) + tapeIndex] = static_cast<uint32_t>(_tzcnt_u64(newBits) + (y * 64ull) + stringIndex);
			newBits = _blsr_u64(newBits);
			structuralIndices[4 + (currentIndex * 8) + tapeIndex] = static_cast<uint32_t>(_tzcnt_u64(newBits) + (y * 64ull) + stringIndex);
			newBits = _blsr_u64(newBits);
			structuralIndices[5 + (currentIndex * 8) + tapeIndex] = static_cast<uint32_t>(_tzcnt_u64(newBits) + (y * 64ull) + stringIndex);
			newBits = _blsr_u64(newBits);
			structuralIndices[6 + (currentIndex * 8) + tapeIndex] = static_cast<uint32_t>(_tzcnt_u64(newBits) + (y * 64ull) + stringIndex);
			newBits = _blsr_u64(newBits);
			structuralIndices[7 + (currentIndex * 8) + tapeIndex] = static_cast<uint32_t>(_tzcnt_u64(newBits) + (y * 64ull) + stringIndex);
			newBits = _blsr_u64(newBits);
			return newBits;
		}

		__forceinline void addTapeValues() noexcept {
			int64_t newBits[4];
			_mm256_storeu_epi64(newBits, structurals);
			for (size_t y = 0; y < 4; ++y) {
				if (!newBits[y]) {
					continue;
				}
				int32_t cnt = static_cast<int32_t>(_mm_popcnt_u64(newBits[y]));
				if (cnt > 0) {
					newBits[y] = rollValuesIntoTape(0, y, newBits[y]);
					if (cnt > 8) {
						newBits[y] = rollValuesIntoTape(1, y, newBits[y]);
						if (cnt > 16) {
							newBits[y] = rollValuesIntoTape(2, y, newBits[y]);
							if (cnt > 24) {
								newBits[y] = rollValuesIntoTape(3, y, newBits[y]);
								if (cnt > 32) {
									newBits[y] = rollValuesIntoTape(4, y, newBits[y]);
									if (cnt > 40) {
										newBits[y] = rollValuesIntoTape(5, y, newBits[y]);
										if (cnt > 48) {
											newBits[y] = rollValuesIntoTape(6, y, newBits[y]);
											if (cnt > 56) {
												newBits[y] = rollValuesIntoTape(7, y, newBits[y]);
											}
										}
									}
								}
							}
						}
					}
				}
				tapeIndex += cnt;
			}
		}

		__forceinline void collectFinalStructurals() noexcept {
			backslash.setFirstBit(prevEscaped);
			SimdBase256 followsEscape = backslash.shl<1>() | prevEscaped;
			SimdBase256 evenBits{ _mm256_set1_epi8(0b01010101) };
			SimdBase256 oddSequenceStarts = backslash.bitAndNot(evenBits.bitAndNot(followsEscape));
			SimdBase256 sequencesStartingOnEvenBits{};
			prevEscaped = backslash.collectCarries(oddSequenceStarts, sequencesStartingOnEvenBits);
			SimdBase256 invert_mask = sequencesStartingOnEvenBits.shl<1>();
			SimdBase256 escaped = (evenBits ^ invert_mask) & followsEscape;
			quotes = quotes.bitAndNot(escaped);
			SimdBase256 inString = quotes.carrylessMultiplication(prevInString);
			SimdBase256 scalar = ~(op | whitespace);
			SimdBase256 nonQuoteScalar = ~(op | whitespace).bitAndNot(quotes);
			bool prevInScalarNew = prevInScalar;
			SimdBase256 shiftMask{ _mm256_set_epi64x(static_cast<int64_t>(static_cast<uint64_t>(0ull) - static_cast<uint64_t>(1ull << 62)), 0ull,
				0ull, 0ull) };
			prevInScalar = (nonQuoteScalar & shiftMask).checkLastBit();
			SimdBase256 followsNonQuoteScalar = nonQuoteScalar.shl<1>();
			followsNonQuoteScalar.setFirstBit(prevInScalarNew);
			structurals = (op | scalar.bitAndNot(followsNonQuoteScalar)).bitAndNot(inString ^ quotes);
		}
	};

}