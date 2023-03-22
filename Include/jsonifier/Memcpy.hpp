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
	inline void fastMemcpySmall16Avx512(void* dest, const void* source, size_t lengthNew) noexcept {
		auto remainder{ lengthNew % 16 };
		__m128i* destVector = static_cast<__m128i*>(dest);
		const __m128i* sourceVector = static_cast<const __m128i*>(source);
		_mm_stream_si128(reinterpret_cast<__m128i*>(destVector++), _mm_loadu_si128(sourceVector++));

		_mm_sfence();
		if (remainder > 0) {
			::memcpy(destVector, sourceVector, remainder);
			return;
		}
	}

	inline void fastMemcpySmall32Avx512(void* dest, const void* source, size_t lengthNew) noexcept {
		auto remainder{ lengthNew % 32 };
		__m256i* destVector = static_cast<__m256i*>(dest);
		const __m256i* sourceVector = static_cast<const __m256i*>(source);
		_mm256_store_si256(destVector++, _mm256_loadu_si256(sourceVector++));

		_mm_sfence();
		if (remainder < 16 && remainder > 0) {
			::memcpy(destVector, sourceVector, remainder);
			return;
		}
		if (remainder > 0) {
			return fastMemcpySmall16Avx512(destVector, sourceVector, remainder);
		}
	}

	inline void fastMemcpySmall64Avx512(void* dest, const void* source, size_t lengthNew) {
		auto remainder{ lengthNew % 64 };
		__m512i* destVector = static_cast<__m512i*>(dest);
		const __m512i* sourceVector = static_cast<const __m512i*>(source);
		_mm_prefetch(reinterpret_cast<const char*>(sourceVector + 1), _MM_HINT_T0);
		_mm512_store_si512(destVector++, _mm512_loadu_si512(sourceVector++));

		_mm_sfence();
		if (remainder < 16 && remainder > 0) {
			::memcpy(destVector, sourceVector, remainder);
			return;
		}
		if (remainder < 32 && remainder > 0) {
			return fastMemcpySmall16Avx512(destVector, sourceVector, remainder);
		}
		if (remainder > 0) {
			return fastMemcpySmall32Avx512(destVector, sourceVector, remainder);
		}
	}

	inline void fastMemcpySmall128Avx512(void* dest, const void* source, size_t lengthNew) {
		auto remainder{ lengthNew % 128 };
		__m512i* destVector = static_cast<__m512i*>(dest);
		const __m512i* sourceVector = static_cast<const __m512i*>(source);
		_mm_prefetch(reinterpret_cast<const char*>(sourceVector + 1), _MM_HINT_T0);
		_mm512_stream_si512(destVector++, _mm512_loadu_si512(sourceVector++));
		_mm512_stream_si512(destVector++, _mm512_loadu_si512(sourceVector++));

		_mm_sfence();
		if (remainder < 16 && remainder > 0) {
			::memcpy(destVector, sourceVector, remainder);
			return;
		}
		if (remainder < 32 && remainder > 0) {
			return fastMemcpySmall16Avx512(destVector, sourceVector, remainder);
		}
		if (remainder < 64 && remainder > 0) {
			return fastMemcpySmall32Avx512(destVector, sourceVector, remainder);
		}
		if (remainder > 0) {
			return fastMemcpySmall64Avx512(destVector, sourceVector, remainder);
		}
	}

	inline void fastMemcpySmall256Avx512(void* dest, const void* source, size_t lengthNew) noexcept {
		auto remainder{ lengthNew % 256 };
		__m512i* destVector = static_cast<__m512i*>(dest);
		const __m512i* sourceVector = static_cast<const __m512i*>(source);
		for (size_t x = lengthNew / (sizeof(__m512i) * 8); x > 0; --x) {
			_mm_prefetch(reinterpret_cast<const char*>(sourceVector + 1), _MM_HINT_T0);
			_mm512_stream_si512(destVector++, _mm512_loadu_si512(sourceVector++));
			_mm512_stream_si512(destVector++, _mm512_loadu_si512(sourceVector++));
			_mm_prefetch(reinterpret_cast<const char*>(sourceVector + 1), _MM_HINT_T0);
			_mm512_stream_si512(destVector++, _mm512_loadu_si512(sourceVector++));
			_mm512_stream_si512(destVector++, _mm512_loadu_si512(sourceVector++));
		}

		_mm_sfence();
		if (remainder < 16 && remainder > 0) {
			::memcpy(destVector, sourceVector, remainder);
			return;
		}
		if (remainder < 32 && remainder > 0) {
			return fastMemcpySmall16Avx512(destVector, sourceVector, remainder);
		}
		if (remainder < 64 && remainder > 0) {
			return fastMemcpySmall32Avx512(destVector, sourceVector, remainder);
		}
		if (remainder < 128 && remainder > 0) {
			return fastMemcpySmall64Avx512(destVector, sourceVector, remainder);
		}
		if (remainder > 0) {
			return fastMemcpySmall128Avx512(destVector, sourceVector, remainder);
		}
	}

	inline void fastMemcpySmallAvx512(void* destVector, const void* sourceVector, size_t lengthNew) noexcept {
		if (lengthNew < 16) {
			::memcpy(destVector, sourceVector, lengthNew);
		} else if (lengthNew < 32) {
			fastMemcpySmall16Avx512(destVector, sourceVector, lengthNew);
		} else if (lengthNew < 64) {
			fastMemcpySmall32Avx512(destVector, sourceVector, lengthNew);
		} else if (lengthNew < 128) {
			fastMemcpySmall64Avx512(destVector, sourceVector, lengthNew);
		} else if (lengthNew < 256) {
			fastMemcpySmall128Avx512(destVector, sourceVector, lengthNew);
		} else {
			fastMemcpySmall256Avx512(destVector, sourceVector, lengthNew);
		}
	};

	inline void fastMemcpy1616Avx512(void* dest, const void* source, size_t lengthNew) noexcept {
		auto remainder{ lengthNew % 16 };
		__m128i* destVector = static_cast<__m128i*>(dest);
		const __m128i* sourceVector = static_cast<const __m128i*>(source);
		_mm_stream_si128(reinterpret_cast<__m128i*>(destVector++), _mm_loadu_si128(sourceVector++));
		_mm_sfence();
		if (remainder > 0) {
			::memcpy(destVector, sourceVector, remainder);
		}
	}

	inline void fastMemcpy32Avx512(void* dest, const void* source, size_t lengthNew) noexcept {
		auto remainder{ lengthNew % 32 };
		__m256i* destVector = static_cast<__m256i*>(dest);
		const __m256i* sourceVector = static_cast<const __m256i*>(source);
		_mm256_stream_si256(destVector++, _mm256_load_si256(sourceVector++));
		_mm_sfence();
		if (remainder < 16 && remainder > 0) {
			::memcpy(destVector, sourceVector, remainder);
			return;
		}
		if (remainder > 0) {
			return fastMemcpy1616Avx512(destVector, sourceVector, remainder);
		}
	}

	inline void fastMemcpy64Avx512(void* dest, const void* source, size_t lengthNew) noexcept {
		auto remainder{ lengthNew % 64 };
		__m512i* destVector = static_cast<__m512i*>(dest);
		const __m512i* sourceVector = static_cast<const __m512i*>(source);
		_mm512_stream_si512(destVector++, _mm512_load_si512(sourceVector++));
		_mm_sfence();
		if (remainder < 16 && remainder > 0) {
			::memcpy(destVector, sourceVector, remainder);
			return;
		}
		if (remainder < 32 && remainder > 0) {
			return fastMemcpy1616Avx512(destVector, sourceVector, remainder);
		}
		if (remainder > 0) {
			return fastMemcpy32Avx512(destVector, sourceVector, remainder);
		}
	}

	inline void fastMemcpy128Avx512(void* dest, const void* source, size_t lengthNew) noexcept {
		auto remainder{ lengthNew % 128 };
		__m512i* destVector = static_cast<__m512i*>(dest);
		const __m512i* sourceVector = static_cast<const __m512i*>(source);
		_mm512_stream_si512(destVector++, _mm512_load_si512(sourceVector++));
		_mm512_stream_si512(destVector++, _mm512_load_si512(sourceVector++));
		_mm_sfence();
		if (remainder < 16 && remainder > 0) {
			::memcpy(destVector, sourceVector, remainder);
			return;
		}
		if (remainder < 32 && remainder > 0) {
			return fastMemcpy1616Avx512(destVector, sourceVector, remainder);
		}
		if (remainder < 64 && remainder > 0) {
			return fastMemcpy32Avx512(destVector, sourceVector, remainder);
		}
		if (remainder > 0) {
			return fastMemcpy64Avx512(destVector, sourceVector, remainder);
		}
	}

	inline void fastMemcpy256Avx512(void* dest, const void* source, size_t lengthNew) noexcept {
		auto remainder{ lengthNew % 256 };
		__m512i* destVector = static_cast<__m512i*>(dest);
		const __m512i* sourceVector = static_cast<const __m512i*>(source);
		for (size_t x = lengthNew / (sizeof(__m512i) * 8); x > 0; --x) {
			_mm512_stream_si512(destVector++, _mm512_load_si512(sourceVector++));
			_mm512_stream_si512(destVector++, _mm512_load_si512(sourceVector++));
			_mm512_stream_si512(destVector++, _mm512_load_si512(sourceVector++));
			_mm512_stream_si512(destVector++, _mm512_load_si512(sourceVector++));
		}
		_mm_sfence();
		if (remainder < 16 && remainder > 0) {
			::memcpy(destVector, sourceVector, remainder);
			return;
		}
		if (remainder < 32 && remainder > 0) {
			return fastMemcpy1616Avx512(destVector, sourceVector, remainder);
		}
		if (remainder < 64 && remainder > 0) {
			return fastMemcpy32Avx512(destVector, sourceVector, remainder);
		}
		if (remainder < 128 && remainder > 0) {
			return fastMemcpy64Avx512(destVector, sourceVector, remainder);
		}
		if (remainder > 0) {
			return fastMemcpy128Avx512(destVector, sourceVector, remainder);
		}
	}

	inline void memcpyFast(void* destVector, const void* sourceVector, size_t lengthNew) noexcept {
		if (lengthNew < 256ull * 1024ull) {
			fastMemcpySmallAvx512(destVector, sourceVector, lengthNew);
			return;
		} else {
			fastMemcpy256Avx512(destVector, sourceVector, lengthNew);
		}
	}
#elif INSTRUCTION_SET_TYPE_AVX2
	inline void fastMemcpySmall16Avx(void* dest, const void* source, size_t lengthNew) noexcept {
		auto remainder{ lengthNew % 16 };
		__m128i* destVector = static_cast<__m128i*>(dest);
		const __m128i* sourceVector = static_cast<const __m128i*>(source);
		_mm_stream_si128(reinterpret_cast<__m128i*>(destVector++), _mm_loadu_si128(sourceVector++));

		_mm_sfence();
		if (remainder > 0) {
			::memcpy(destVector, sourceVector, remainder);
			return;
		}
	}

	inline void fastMemcpySmall32Avx2(void* dest, const void* source, size_t lengthNew) noexcept {
		auto remainder{ lengthNew % 32 };
		__m256i* destVector = static_cast<__m256i*>(dest);
		const __m256i* sourceVector = static_cast<const __m256i*>(source);
		_mm256_store_si256(destVector++, _mm256_loadu_si256(sourceVector++));

		_mm_sfence();
		if (remainder < 16 && remainder > 0) {
			::memcpy(destVector, sourceVector, remainder);
			return;
		}
		if (remainder > 0) {
			return fastMemcpySmall16Avx(destVector, sourceVector, remainder);
		}
	}

	inline void fastMemcpySmall64Avx2(void* dest, const void* source, size_t lengthNew) {
		auto remainder{ lengthNew % 64 };
		__m256i* destVector = static_cast<__m256i*>(dest);
		const __m256i* sourceVector = static_cast<const __m256i*>(source);
		_mm_prefetch(reinterpret_cast<const char*>(sourceVector + 1), _MM_HINT_T0);
		_mm256_store_si256(destVector++, _mm256_loadu_si256(sourceVector++));
		_mm256_store_si256(destVector++, _mm256_loadu_si256(sourceVector++));

		_mm_sfence();
		if (remainder < 16 && remainder > 0) {
			::memcpy(destVector, sourceVector, remainder);
			return;
		}
		if (remainder < 32 && remainder > 0) {
			return fastMemcpySmall16Avx(destVector, sourceVector, remainder);
		}
		if (remainder > 0) {
			return fastMemcpySmall32Avx2(destVector, sourceVector, remainder);
		}
	}

	inline void fastMemcpySmall128Avx2(void* dest, const void* source, size_t lengthNew) {
		auto remainder{ lengthNew % 128 };
		__m256i* destVector = static_cast<__m256i*>(dest);
		const __m256i* sourceVector = static_cast<const __m256i*>(source);
		_mm_prefetch(reinterpret_cast<const char*>(sourceVector + 1), _MM_HINT_T0);
		_mm256_stream_si256(destVector++, _mm256_loadu_si256(sourceVector++));
		_mm256_stream_si256(destVector++, _mm256_loadu_si256(sourceVector++));
		_mm256_stream_si256(destVector++, _mm256_loadu_si256(sourceVector++));
		_mm256_stream_si256(destVector++, _mm256_loadu_si256(sourceVector++));

		_mm_sfence();
		if (remainder < 16 && remainder > 0) {
			::memcpy(destVector, sourceVector, remainder);
			return;
		}
		if (remainder < 32 && remainder > 0) {
			return fastMemcpySmall16Avx(destVector, sourceVector, remainder);
		}
		if (remainder < 64 && remainder > 0) {
			return fastMemcpySmall32Avx2(destVector, sourceVector, remainder);
		}
		if (remainder > 0) {
			return fastMemcpySmall64Avx2(destVector, sourceVector, remainder);
		}
	}

	inline void fastMemcpySmall256Avx2(void* dest, const void* source, size_t lengthNew) noexcept {
		auto remainder{ lengthNew % 256 };
		__m256i* destVector = static_cast<__m256i*>(dest);
		const __m256i* sourceVector = static_cast<const __m256i*>(source);
		for (size_t x = lengthNew / (sizeof(__m256i) * 8); x > 0; --x) {
			_mm_prefetch(reinterpret_cast<const char*>(sourceVector + 1), _MM_HINT_T0);
			_mm256_stream_si256(destVector++, _mm256_loadu_si256(sourceVector++));
			_mm256_stream_si256(destVector++, _mm256_loadu_si256(sourceVector++));
			_mm_prefetch(reinterpret_cast<const char*>(sourceVector + 1), _MM_HINT_T0);
			_mm256_stream_si256(destVector++, _mm256_loadu_si256(sourceVector++));
			_mm256_stream_si256(destVector++, _mm256_loadu_si256(sourceVector++));
			_mm_prefetch(reinterpret_cast<const char*>(sourceVector + 1), _MM_HINT_T0);
			_mm256_stream_si256(destVector++, _mm256_loadu_si256(sourceVector++));
			_mm256_stream_si256(destVector++, _mm256_loadu_si256(sourceVector++));
			_mm_prefetch(reinterpret_cast<const char*>(sourceVector + 1), _MM_HINT_T0);
			_mm256_stream_si256(destVector++, _mm256_loadu_si256(sourceVector++));
			_mm256_stream_si256(destVector++, _mm256_loadu_si256(sourceVector++));
		}

		_mm_sfence();
		if (remainder < 16 && remainder > 0) {
			::memcpy(destVector, sourceVector, remainder);
			return;
		}
		if (remainder < 32 && remainder > 0) {
			return fastMemcpySmall16Avx(destVector, sourceVector, remainder);
		}
		if (remainder < 64 && remainder > 0) {
			return fastMemcpySmall32Avx2(destVector, sourceVector, remainder);
		}
		if (remainder < 128 && remainder > 0) {
			return fastMemcpySmall64Avx2(destVector, sourceVector, remainder);
		}
		if (remainder > 0) {
			return fastMemcpySmall128Avx2(destVector, sourceVector, remainder);
		}
	}

	inline void fastMemcpySmallAvx2(void* destVector,const void* sourceVector, size_t lengthNew) noexcept {
		if (lengthNew < 16) {
			::memcpy(destVector, sourceVector, lengthNew);
		} else if (lengthNew < 32) {
			fastMemcpySmall16Avx(destVector, sourceVector, lengthNew);
		} else if (lengthNew < 64) {
			fastMemcpySmall32Avx2(destVector, sourceVector, lengthNew);
		} else if (lengthNew < 128) {
			fastMemcpySmall64Avx2(destVector, sourceVector, lengthNew);
		} else if (lengthNew < 256) {
			fastMemcpySmall128Avx2(destVector, sourceVector, lengthNew);
		} else {
			fastMemcpySmall256Avx2(destVector, sourceVector, lengthNew);
		}
	};

	inline void fastMemcpy16Avx2(void* dest, const void* source, size_t lengthNew) noexcept {
		auto remainder{ lengthNew % 16 };
		__m128i* destVector = static_cast<__m128i*>(dest);
		 const __m128i* sourceVector = static_cast<const __m128i*>(source);
		_mm_stream_si128(reinterpret_cast<__m128i*>(destVector++), _mm_loadu_si128(sourceVector++));
		_mm_sfence();
		if (remainder > 0) {
			::memcpy(destVector, sourceVector, remainder);
		}
	}

	inline void fastMemcpy32Avx2(void* dest, const void* source, size_t lengthNew) noexcept {
		auto remainder{ lengthNew % 32 };
		__m256i* destVector = static_cast<__m256i*>(dest);
		const __m256i* sourceVector = static_cast<const __m256i*>(source);
		_mm256_stream_si256(destVector++, _mm256_stream_load_si256(sourceVector++));
		_mm_sfence();
		if (remainder < 16 && remainder > 0) {
			::memcpy(destVector, sourceVector, remainder);
			return;
		}
		if (remainder > 0) {
			return fastMemcpy16Avx2(destVector, sourceVector, remainder);
		}
	}

	inline void fastMemcpy64Avx2(void* dest, const void* source, size_t lengthNew) noexcept {
		auto remainder{ lengthNew % 64 };
		__m256i* destVector = static_cast<__m256i*>(dest);
		const __m256i* sourceVector = static_cast<const __m256i*>(source);
		_mm256_stream_si256(destVector++, _mm256_stream_load_si256(sourceVector++));
		_mm256_stream_si256(destVector++, _mm256_stream_load_si256(sourceVector++));
		_mm_sfence();
		if (remainder < 16 && remainder > 0) {
			::memcpy(destVector, sourceVector, remainder);
			return;
		}
		if (remainder < 32 && remainder > 0) {
			return fastMemcpy16Avx2(destVector, sourceVector, remainder);
		}
		if (remainder > 0) {
			return fastMemcpy32Avx2(destVector, sourceVector, remainder);
		}
	}

	inline void fastMemcpy128Avx2(void* dest, const void* source, size_t lengthNew) noexcept {
		auto remainder{ lengthNew % 128 };
		__m256i* destVector = static_cast<__m256i*>(dest);
		const __m256i* sourceVector = static_cast<const __m256i*>(source);
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
			return fastMemcpy16Avx2(destVector, sourceVector, remainder);
		}
		if (remainder < 64 && remainder > 0) {
			return fastMemcpy32Avx2(destVector, sourceVector, remainder);
		}
		if (remainder > 0) {
			return fastMemcpy64Avx2(destVector, sourceVector, remainder);
		}
	}

	inline void fastMemcpy256Avx2(void* dest, const void* source, size_t lengthNew) noexcept {
		auto remainder{ lengthNew % 256 };
		__m256i* destVector = static_cast<__m256i*>(dest);
		const __m256i* sourceVector = static_cast<const __m256i*>(source);
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
			return fastMemcpy16Avx2(destVector, sourceVector, remainder);
		}
		if (remainder < 64 && remainder > 0) {
			return fastMemcpy32Avx2(destVector, sourceVector, remainder);
		}
		if (remainder < 128 && remainder > 0) {
			return fastMemcpy64Avx2(destVector, sourceVector, remainder);
		}
		if (remainder > 0) {
			return fastMemcpy128Avx2(destVector, sourceVector, remainder);
		}
	}

	inline void memcpyFast(void* destVector, const void* sourceVector, size_t lengthNew) noexcept {
		if (lengthNew < 256ull * 1024ull) {
			fastMemcpySmallAvx2(destVector, sourceVector, lengthNew);
			return;
		} else {
			fastMemcpy256Avx2(destVector, sourceVector, lengthNew);
		}
	}
#elif INSTRUCTION_SET_TYPE_AVX
	inline void fastMemcpySmall16Avx(void* dest, const void* source, size_t lengthNew) noexcept {
		auto remainder{ lengthNew % 16 };
		__m128i* destVector = static_cast<__m128i*>(dest);
		 const __m128i* sourceVector = static_cast<const __m128i*>(source);
		_mm_stream_si128(reinterpret_cast<__m128i*>(destVector++), _mm_loadu_si128(sourceVector++));

		_mm_sfence();
		if (remainder > 0) {
			::memcpy(destVector, sourceVector, remainder);
			return;
		}
	}

	inline void fastMemcpySmall32Avx(void* dest, const void* source, size_t lengthNew) noexcept {
		auto remainder{ lengthNew % 32 };
		__m256i* destVector = static_cast<__m256i*>(dest);
		const __m256i* sourceVector = static_cast<const __m256i*>(source);
		_mm256_store_si256(destVector++, _mm256_loadu_si256(sourceVector++));

		_mm_sfence();
		if (remainder < 16 && remainder > 0) {
			::memcpy(destVector, sourceVector, remainder);
			return;
		}
		if (remainder > 0) {
			return fastMemcpySmall16Avx(destVector, sourceVector, remainder);
		}
	}

	inline void fastMemcpySmall64Avx(void* dest, const void* source, size_t lengthNew) {
		auto remainder{ lengthNew % 64 };
		__m256i* destVector = static_cast<__m256i*>(dest);
		const __m256i* sourceVector = static_cast<const __m256i*>(source);
		_mm_prefetch(reinterpret_cast<const char*>(sourceVector + 1), _MM_HINT_T0);
		_mm256_store_si256(destVector++, _mm256_loadu_si256(sourceVector++));
		_mm256_store_si256(destVector++, _mm256_loadu_si256(sourceVector++));

		_mm_sfence();
		if (remainder < 16 && remainder > 0) {
			::memcpy(destVector, sourceVector, remainder);
			return;
		}
		if (remainder < 32 && remainder > 0) {
			return fastMemcpySmall16Avx(destVector, sourceVector, remainder);
		}
		if (remainder > 0) {
			return fastMemcpySmall32Avx(destVector, sourceVector, remainder);
		}
	}

	inline void fastMemcpySmall128Avx(void* dest, const void* source, size_t lengthNew) {
		auto remainder{ lengthNew % 128 };
		__m256i* destVector = static_cast<__m256i*>(dest);
		const __m256i* sourceVector = static_cast<const __m256i*>(source);
		_mm_prefetch(reinterpret_cast<const char*>(sourceVector + 1), _MM_HINT_T0);
		_mm256_stream_si256(destVector++, _mm256_loadu_si256(sourceVector++));
		_mm256_stream_si256(destVector++, _mm256_loadu_si256(sourceVector++));
		_mm256_stream_si256(destVector++, _mm256_loadu_si256(sourceVector++));
		_mm256_stream_si256(destVector++, _mm256_loadu_si256(sourceVector++));

		_mm_sfence();
		if (remainder < 16 && remainder > 0) {
			::memcpy(destVector, sourceVector, remainder);
			return;
		}
		if (remainder < 32 && remainder > 0) {
			return fastMemcpySmall16Avx(destVector, sourceVector, remainder);
		}
		if (remainder < 64 && remainder > 0) {
			return fastMemcpySmall32Avx(destVector, sourceVector, remainder);
		}
		if (remainder > 0) {
			return fastMemcpySmall64Avx(destVector, sourceVector, remainder);
		}
	}

	inline void fastMemcpySmall256Avx(void* dest, const void* source, size_t lengthNew) noexcept {
		auto remainder{ lengthNew % 256 };
		__m256i* destVector = static_cast<__m256i*>(dest);
		const __m256i* sourceVector = static_cast<const __m256i*>(source);
		for (size_t x = lengthNew / (sizeof(__m256i) * 8); x > 0; --x) {
			_mm_prefetch(reinterpret_cast<const char*>(sourceVector + 1), _MM_HINT_T0);
			_mm256_stream_si256(destVector++, _mm256_loadu_si256(sourceVector++));
			_mm256_stream_si256(destVector++, _mm256_loadu_si256(sourceVector++));
			_mm_prefetch(reinterpret_cast<const char*>(sourceVector + 1), _MM_HINT_T0);
			_mm256_stream_si256(destVector++, _mm256_loadu_si256(sourceVector++));
			_mm256_stream_si256(destVector++, _mm256_loadu_si256(sourceVector++));
			_mm_prefetch(reinterpret_cast<const char*>(sourceVector + 1), _MM_HINT_T0);
			_mm256_stream_si256(destVector++, _mm256_loadu_si256(sourceVector++));
			_mm256_stream_si256(destVector++, _mm256_loadu_si256(sourceVector++));
			_mm_prefetch(reinterpret_cast<const char*>(sourceVector + 1), _MM_HINT_T0);
			_mm256_stream_si256(destVector++, _mm256_loadu_si256(sourceVector++));
			_mm256_stream_si256(destVector++, _mm256_loadu_si256(sourceVector++));
		}

		_mm_sfence();
		if (remainder < 16 && remainder > 0) {
			::memcpy(destVector, sourceVector, remainder);
			return;
		}
		if (remainder < 32 && remainder > 0) {
			return fastMemcpySmall16Avx(destVector, sourceVector, remainder);
		}
		if (remainder < 64 && remainder > 0) {
			return fastMemcpySmall32Avx(destVector, sourceVector, remainder);
		}
		if (remainder < 128 && remainder > 0) {
			return fastMemcpySmall64Avx(destVector, sourceVector, remainder);
		}
		if (remainder > 0) {
			return fastMemcpySmall128Avx(destVector, sourceVector, remainder);
		}
	}

	inline void fastMemcpySmallAvx(void* destVector, const void* sourceVector, size_t lengthNew) noexcept {
		if (lengthNew < 16) {
			::memcpy(destVector, sourceVector, lengthNew);
		} else if (lengthNew < 32) {
			fastMemcpySmall16Avx(destVector, sourceVector, lengthNew);
		} else if (lengthNew < 64) {
			fastMemcpySmall32Avx(destVector, sourceVector, lengthNew);
		} else if (lengthNew < 128) {
			fastMemcpySmall64Avx(destVector, sourceVector, lengthNew);
		} else if (lengthNew < 256) {
			fastMemcpySmall128Avx(destVector, sourceVector, lengthNew);
		} else {
			fastMemcpySmall256Avx(destVector, sourceVector, lengthNew);
		}
	};

	inline void fastMemcpy16Avx(void* dest, const void* source, size_t lengthNew) noexcept {
		auto remainder{ lengthNew % 16 };
		__m128i* destVector = static_cast<__m128i*>(dest);
		 const __m128i* sourceVector = static_cast<const __m128i*>(source);
		_mm_stream_si128(reinterpret_cast<__m128i*>(destVector++), _mm_loadu_si128(sourceVector++));
		_mm_sfence();
		if (remainder > 0) {
			::memcpy(destVector, sourceVector, remainder);
		}
	}

	inline void fastMemcpy32Avx(void* dest, const void* source, size_t lengthNew) noexcept {
		auto remainder{ lengthNew % 32 };
		__m256i* destVector = static_cast<__m256i*>(dest);
		const __m256i* sourceVector = static_cast<const __m256i*>(source);
		_mm256_stream_si256(destVector++, _mm256_stream_load_si256(sourceVector++));
		_mm_sfence();
		if (remainder < 16 && remainder > 0) {
			::memcpy(destVector, sourceVector, remainder);
			return;
		}
		if (remainder > 0) {
			return fastMemcpy16Avx(destVector, sourceVector, remainder);
		}
	}

	inline void fastMemcpy64Avx(void* dest, const void* source, size_t lengthNew) noexcept {
		auto remainder{ lengthNew % 64 };
		__m256i* destVector = static_cast<__m256i*>(dest);
		const __m256i* sourceVector = static_cast<const __m256i*>(source);
		_mm256_stream_si256(destVector++, _mm256_stream_load_si256(sourceVector++));
		_mm256_stream_si256(destVector++, _mm256_stream_load_si256(sourceVector++));
		_mm_sfence();
		if (remainder < 16 && remainder > 0) {
			::memcpy(destVector, sourceVector, remainder);
			return;
		}
		if (remainder < 32 && remainder > 0) {
			return fastMemcpy16Avx(destVector, sourceVector, remainder);
		}
		if (remainder > 0) {
			return fastMemcpy32Avx(destVector, sourceVector, remainder);
		}
	}

	inline void fastMemcpy128Avx(void* dest, const void* source, size_t lengthNew) noexcept {
		auto remainder{ lengthNew % 128 };
		__m256i* destVector = static_cast<__m256i*>(dest);
		const __m256i* sourceVector = static_cast<const __m256i*>(source);
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
			return fastMemcpy16Avx(destVector, sourceVector, remainder);
		}
		if (remainder < 64 && remainder > 0) {
			return fastMemcpy32Avx(destVector, sourceVector, remainder);
		}
		if (remainder > 0) {
			return fastMemcpy64Avx(destVector, sourceVector, remainder);
		}
	}

	inline void fastMemcpy256Avx(void* dest, const void* source, size_t lengthNew) noexcept {
		auto remainder{ lengthNew % 256 };
		__m256i* destVector = static_cast<__m256i*>(dest);
		const __m256i* sourceVector = static_cast<const __m256i*>(source);
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
			return fastMemcpy16Avx(destVector, sourceVector, remainder);
		}
		if (remainder < 64 && remainder > 0) {
			return fastMemcpy32Avx(destVector, sourceVector, remainder);
		}
		if (remainder < 128 && remainder > 0) {
			return fastMemcpy64Avx(destVector, sourceVector, remainder);
		}
		if (remainder > 0) {
			return fastMemcpy128Avx(destVector, sourceVector, remainder);
		}
	}

	inline void memcpyFast(void* destVector, const void* sourceVector, size_t lengthNew) noexcept {
		if (lengthNew < 256ull * 1024ull) {
			fastMemcpySmallAvx(destVector, sourceVector, lengthNew);
			return;
		} else {
			fastMemcpy256Avx(destVector, sourceVector, lengthNew);
		}
	}
#else
	inline void memcpyFast(void* destVector, const void* sourceVector, size_t lengthNew) noexcept {
		memcpy(destVector, sourceVector, lengthNew);
	}
#endif




}