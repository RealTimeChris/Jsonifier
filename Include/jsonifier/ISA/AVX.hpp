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

#include <jsonifier/TypeEntities.hpp>

namespace simd_internal {

#if JSONIFIER_CHECK_FOR_AVX(JSONIFIER_AVX)

	template<simd_int_128_type simd_int_t01, simd_int_128_type simd_int_t02> JSONIFIER_INLINE simd_int_128 opAndNot(simd_int_t01&& value, simd_int_t02&& other) {
		return _mm_andnot_si128(std::forward<simd_int_t02>(other), std::forward<simd_int_t01>(value));
	}

	template<simd_int_128_type simd_int_t01, simd_int_128_type simd_int_t02> JSONIFIER_INLINE simd_int_128 opAnd(simd_int_t01&& value, simd_int_t02&& other) {
		return _mm_and_si128(std::forward<simd_int_t01>(value), std::forward<simd_int_t02>(other));
	}

	template<simd_int_128_type simd_int_t01, simd_int_128_type simd_int_t02> JSONIFIER_INLINE simd_int_128 opXor(simd_int_t01&& value, simd_int_t02&& other) {
		return _mm_xor_si128(std::forward<simd_int_t01>(value), std::forward<simd_int_t02>(other));
	}

	template<simd_int_128_type simd_int_t01, simd_int_128_type simd_int_t02> JSONIFIER_INLINE simd_int_128 opOr(simd_int_t01&& value, simd_int_t02&& other) {
		return _mm_or_si128(std::forward<simd_int_t01>(value), std::forward<simd_int_t02>(other));
	}

	template<simd_int_128_type simd_int_t01> JSONIFIER_INLINE simd_int_128 opSetLSB(simd_int_t01&& value, bool valueNew) {
	#if defined(JSONIFIER_WIN) || JSONIFIER_LINUX
		static constexpr simd_int_128 mask{ 0x01u, '\0' };
	#else
		static constexpr simd_int_128 mask{ 0x01u, '\0', '\0', '\0', '\0', '\0', '\0', '\0' };
	#endif
		return valueNew ? _mm_or_si128(value, mask) : _mm_andnot_si128(mask, value);
	}

	template<simd_int_128_type simd_int_t01> JSONIFIER_INLINE simd_int_128 opNot(simd_int_t01&& value) {
		return _mm_xor_si128(std::forward<simd_int_t01>(value), _mm_set1_epi64x(0xFFFFFFFFFFFFFFFFll));
	}

	template<simd_int_128_type simd_int_t01> JSONIFIER_INLINE bool opGetMSB(simd_int_t01&& value) {
		simd_int_128 result = _mm_and_si128(std::forward<simd_int_t01>(value), _mm_set_epi64x(0x8000000000000000ll, 0x00ll));
		return !_mm_testz_si128(result, result);
	}

	template<simd_int_128_type simd_int_t01> JSONIFIER_INLINE bool opBool(simd_int_t01&& value) {
		return !_mm_testz_si128(value, value);
	}

	template<simd_int_128_type simd_int_t01> JSONIFIER_INLINE simd_int_t01 reset() {
		return _mm_setzero_si128();
	}

	#if JSONIFIER_CHECK_FOR_AVX(JSONIFIER_AVX2)

	template<simd_int_256_type simd_int_t01, simd_int_256_type simd_int_t02> JSONIFIER_INLINE simd_int_256 opAndNot(simd_int_t01&& value, simd_int_t02&& other) {
		return _mm256_andnot_si256(std::forward<simd_int_t02>(other), std::forward<simd_int_t01>(value));
	}

	template<simd_int_256_type simd_int_t01, simd_int_256_type simd_int_t02> JSONIFIER_INLINE simd_int_256 opAnd(simd_int_t01&& value, simd_int_t02&& other) {
		return _mm256_and_si256(std::forward<simd_int_t01>(value), std::forward<simd_int_t02>(other));
	}

	template<simd_int_256_type simd_int_t01, simd_int_256_type simd_int_t02> JSONIFIER_INLINE simd_int_256 opXor(simd_int_t01&& value, simd_int_t02&& other) {
		return _mm256_xor_si256(std::forward<simd_int_t01>(value), std::forward<simd_int_t02>(other));
	}

	template<simd_int_256_type simd_int_t01, simd_int_256_type simd_int_t02> JSONIFIER_INLINE simd_int_256 opOr(simd_int_t01&& value, simd_int_t02&& other) {
		return _mm256_or_si256(std::forward<simd_int_t01>(value), std::forward<simd_int_t02>(other));
	}

	template<simd_int_256_type simd_int_t01> JSONIFIER_INLINE simd_int_256 opSetLSB(simd_int_t01&& value, bool valueNew) {
		#if defined(JSONIFIER_WIN) || JSONIFIER_LINUX
		static constexpr simd_int_256 mask{ 0x01u, '\0', '\0', '\0' };
		#else
		static constexpr simd_int_256 mask{ 0x01u, '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0' };
		#endif
		return valueNew ? _mm256_or_si256(value, mask) : _mm256_andnot_si256(mask, value);
	}

	template<simd_int_256_type simd_int_t01> JSONIFIER_INLINE simd_int_256 opNot(simd_int_t01&& value) {
		return _mm256_xor_si256(std::forward<simd_int_t01>(value), _mm256_set1_epi64x(0xFFFFFFFFFFFFFFFFll));
	}

	template<simd_int_256_type simd_int_t01> JSONIFIER_INLINE bool opGetMSB(simd_int_t01&& value) {
		simd_int_256 result = _mm256_and_si256(std::forward<simd_int_t01>(value), _mm256_set_epi64x(0x8000000000000000ll, 0x00ll, 0x00ll, 0x00ll));
		return !_mm256_testz_si256(result, result);
	}

	template<simd_int_256_type simd_int_t01> JSONIFIER_INLINE bool opBool(simd_int_t01&& value) {
		return !_mm256_testz_si256(value, value);
	}

	template<simd_int_256_type simd_int_t01> JSONIFIER_INLINE simd_int_t01 reset() {
		return _mm256_setzero_si256();
	}

		#if JSONIFIER_CHECK_FOR_AVX(JSONIFIER_AVX512)

	template<simd_int_512_type simd_int_t01, simd_int_512_type simd_int_t02> JSONIFIER_INLINE simd_int_512 opAndNot(simd_int_t01&& value, simd_int_t02&& other) {
		return _mm512_andnot_si512(std::forward<simd_int_t02>(other), std::forward<simd_int_t01>(value));
	}

	template<simd_int_512_type simd_int_t01, simd_int_512_type simd_int_t02> JSONIFIER_INLINE simd_int_512 opAnd(simd_int_t01&& value, simd_int_t02&& other) {
		return _mm512_and_si512(std::forward<simd_int_t01>(value), std::forward<simd_int_t02>(other));
	}

	template<simd_int_512_type simd_int_t01, simd_int_512_type simd_int_t02> JSONIFIER_INLINE simd_int_512 opXor(simd_int_t01&& value, simd_int_t02&& other) {
		return _mm512_xor_si512(std::forward<simd_int_t01>(value), std::forward<simd_int_t02>(other));
	}

	template<simd_int_512_type simd_int_t01, simd_int_512_type simd_int_t02> JSONIFIER_INLINE simd_int_512 opOr(simd_int_t01&& value, simd_int_t02&& other) {
		return _mm512_or_si512(std::forward<simd_int_t01>(value), std::forward<simd_int_t02>(other));
	}

	template<simd_int_512_type simd_int_t01> JSONIFIER_INLINE simd_int_512 opSetLSB(simd_int_t01&& value, bool valueNew) {
			#if defined(JSONIFIER_WIN) || JSONIFIER_LINUX
		static constexpr simd_int_512 mask{ 0x01u, '\0', '\0', '\0', '\0', '\0', '\0', '\0' };
			#else
		static constexpr simd_int_512 mask{ 0x01u, '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
			'\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0' };
			#endif
		return valueNew ? _mm512_or_si512(value, mask) : _mm512_andnot_si512(mask, value);
	}

	template<simd_int_512_type simd_int_t01> JSONIFIER_INLINE simd_int_512 opNot(simd_int_t01&& value) {
		return _mm512_xor_si512(std::forward<simd_int_t01>(value), _mm512_set1_epi64(0xFFFFFFFFFFFFFFFFll));
	}

	template<simd_int_512_type simd_int_t01> JSONIFIER_INLINE bool opGetMSB(simd_int_t01&& value) {
		simd_int_512 result = _mm512_and_si512(value, _mm512_set_epi64(0x8000000000000000ll, 0x00ll, 0x00ll, 0x00ll, 0x00ll, 0x00ll, 0x00ll, 0x00ll));
		return _mm512_test_epi64_mask(result, result);
	}

	template<simd_int_512_type simd_int_t01> JSONIFIER_INLINE bool opBool(simd_int_t01&& value) {
		return _mm512_test_epi64_mask(value, value);
	}

	template<simd_int_512_type simd_int_t01> JSONIFIER_INLINE simd_int_t01 reset() {
		return _mm512_setzero_si512();
	}

		#endif

	#endif

#endif

}