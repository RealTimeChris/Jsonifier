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

#if AVX_TYPE == 124

	inline bool compareStrings16(const void* string1, const void* string2, size_t length) noexcept {
		const size_t remainder{ length % 16 };
		auto destVector = static_cast<const __m128i*>(string1);
		auto sourceVector = static_cast<const __m128i*>(string2);
		if (_mm_movemask_epi8(_mm_cmpeq_epi8(_mm_loadu_si128(destVector), _mm_loadu_si128(sourceVector))) != (0x0000ffff)) {
			return false;
		}
		if (remainder > 0) {
			return (std::memcmp(++destVector, ++sourceVector, remainder) == 0);
		}
		return true;
	}

	inline bool compareStrings32(const void* string1, const void* string2, size_t length) noexcept {
		const size_t remainder{ length % 32 };
		auto destVector = static_cast<const __m256i*>(string1);
		auto sourceVector = static_cast<const __m256i*>(string2);
		if (_mm256_movemask_epi8(_mm256_cmpeq_epi8(_mm256_loadu_si256(destVector), _mm256_loadu_si256(sourceVector))) != (0xffffffff)) {
			return false;
		}
		if (remainder >= 16) {
			return compareStrings16(++destVector, ++sourceVector, remainder);
		}
		if (remainder > 0) {
			return (std::memcmp(++destVector, ++sourceVector, remainder) == 0);
		}
		return true;
	}

	inline bool compareStrings64(const void* string1, const void* string2, size_t length) noexcept {
		const size_t remainder{ length % 64 };
		auto destVector = static_cast<const __m512i*>(string1);
		auto sourceVector = static_cast<const __m512i*>(string2);
		if (_mm512_cmp_epi8_mask(_mm512_loadu_si512(destVector), _mm512_loadu_si512(sourceVector), _MM_CMPINT_EQ) != (0xffffffffffffffff)) {
			return false;
		}
		if (remainder >= 32) {
			return compareStrings32(++destVector, ++sourceVector, remainder);
		}
		if (remainder >= 16) {
			return compareStrings16(++destVector, ++sourceVector, remainder);
		}
		if (remainder > 0) {
			return (std::memcmp(++destVector, ++sourceVector, remainder) == 0);
		}
		return true;
	}

	inline bool compareStrings128(const void* string1, const void* string2, size_t length) noexcept {
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

	inline bool compareStrings256(const void* string1, const void* string2, size_t length) noexcept {
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

	inline bool compareStrings512(const void* string1, const void* string2, size_t length) noexcept {
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

	inline bool compareStrings(const char* string1, const char* string2, size_t length) noexcept {
		if (length >= 512) {
			return compareStrings512(string1, string2, length);
		} else if (length >= 256) {
			return compareStrings256(string1, string2, length);
		} else if (length >= 128) {
			return compareStrings128(string1, string2, length);
		} else if (length >= 64) {
			return compareStrings64(string1, string2, length);
		} else if (length >= 32) {
			return compareStrings32(string1, string2, length);
		} else if (length >= 16) {
			return compareStrings16(string1, string2, length);
		} else {
			return (std::memcmp(string1, string2, length) == 0);
		}
	}

	inline bool compareFast(const char* destVector, const char* sourceVector, size_t length) noexcept {
		return compareStrings(destVector, sourceVector, length);
	}

#elif AVX_TYPE == 125

	inline bool compareStrings16(const void* string1, const void* string2, size_t length) noexcept {
		const size_t remainder{ length % 16 };
		auto destVector = static_cast<const __m128i*>(string1);
		auto sourceVector = static_cast<const __m128i*>(string2);
		if (_mm_movemask_epi8(_mm_cmpeq_epi8(_mm_loadu_si128(destVector), _mm_loadu_si128(sourceVector))) != (0x0000ffff)) {
			return false;
		}
		if (remainder > 0) {
			return (std::memcmp(++destVector, ++sourceVector, remainder) == 0);
		}
		return true;
	}

	inline bool compareStrings32(const void* string1, const void* string2, size_t length) noexcept {
		const size_t remainder{ length % 32 };
		auto destVector = static_cast<const __m256i*>(string1);
		auto sourceVector = static_cast<const __m256i*>(string2);
		if (_mm256_movemask_epi8(_mm256_cmpeq_epi8(_mm256_loadu_si256(destVector), _mm256_loadu_si256(sourceVector))) != (0xffffffff)) {
			return false;
		}
		if (remainder >= 16) {
			return compareStrings16(++destVector, ++sourceVector, remainder);
		}
		if (remainder > 0) {
			return (std::memcmp(++destVector, ++sourceVector, remainder) == 0);
		}
		return true;
	}

	inline bool compareStrings64(const void* string1, const void* string2, size_t length) noexcept {
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

	inline bool compareStrings128(const void* string1, const void* string2, size_t length) noexcept {
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

	inline bool compareStrings256(const void* string1, const void* string2, size_t length) noexcept {
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

	inline bool compareStrings(const char* string1, const char* string2, size_t length) noexcept {
		if (length >= 256) {
			return compareStrings256(string1, string2, length);
		} else if (length >= 128) {
			return compareStrings128(string1, string2, length);
		} else if (length >= 64) {
			return compareStrings64(string1, string2, length);
		} else if (length >= 32) {
			return compareStrings32(string1, string2, length);
		} else if (length >= 16) {
			return compareStrings16(string1, string2, length);
		} else {
			return (std::memcmp(string1, string2, length) == 0);
		}
	}

	inline bool compareFast(const char* destVector, const char* sourceVector, size_t length) noexcept {
		return compareStrings(destVector, sourceVector, length);
	}

#elif AVX_TYPE == 126

	inline bool compareStrings16(const void* string1, const void* string2, size_t length) noexcept {
		const size_t remainder{ length % 16 };
		auto destVector = static_cast<const __m128i*>(string1);
		auto sourceVector = static_cast<const __m128i*>(string2);
		if (_mm_movemask_epi8(_mm_and_si128(_mm_set1_epi64x(-1ll), _mm_cmpeq_epi8(_mm_loadu_si128(destVector), _mm_loadu_si128(sourceVector)))) !=
			(0x0000ffff)) {
			return false;
		}
		if (remainder > 0) {
			return (std::memcmp(++destVector, ++sourceVector, remainder) == 0);
		}
		return true;
	}

	inline bool compareStrings32(const void* string1, const void* string2, size_t length) noexcept {
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
		if (remainder >= 16) {
			return compareStrings16(destVector, sourceVector, remainder);
		}
		if (remainder > 0) {
			return (std::memcmp(destVector, sourceVector, remainder) == 0);
		}
		return true;
	}

	inline bool compareStrings64(const void* string1, const void* string2, size_t length) noexcept {
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

	inline bool compareStrings128(const void* string1, const void* string2, size_t length) noexcept {
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

	inline bool compareStrings(const char* string1, const char* string2, size_t length) noexcept {
		if (length >= 128) {
			return compareStrings128(string1, string2, length);
		} else if (length >= 64) {
			return compareStrings64(string1, string2, length);
		} else if (length >= 32) {
			return compareStrings32(string1, string2, length);
		} else if (length >= 16) {
			return compareStrings16(string1, string2, length);
		} else {
			return (std::memcmp(string1, string2, length) == 0);
		}
	}

	inline bool compareFast(const char* destVector, const char* sourceVector, size_t length) noexcept {
		return compareStrings(destVector, sourceVector, length);
	}
#else
	inline bool compareFast(const void* destVector, const void* sourceVector, size_t length) noexcept {
		return std::string_view{ static_cast<const char*>(destVector), length } == std::string_view{ static_cast<const char*>(sourceVector), length };
	}
#endif

	class JsonifierCoreInternal {
	  public:
		inline static bool compare(const char* destVector, const char* sourceVector, size_t length) noexcept {
			return compareFast(destVector, sourceVector, length);
		}
	};

}