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

#include <jsonifier/Base.hpp>

namespace simd_internal {

#if JSONIFIER_CHECK_FOR_INSTRUCTION(JSONIFIER_AVX)

	template<simd_int_type simd_int_t01, simd_int_type simd_int_t02> JSONIFIER_INLINE simd_int_t opAndNot(simd_int_t01&& value, simd_int_t02&& other) {
		return _mm_andnot_si128(std::forward<simd_int_t02>(other), std::forward<simd_int_t01>(value));
	}

	template<simd_int_type simd_int_t01, simd_int_type simd_int_t02> JSONIFIER_INLINE simd_int_t opAnd(simd_int_t01&& value, simd_int_t02&& other) {
		return _mm_and_si128(std::forward<simd_int_t01>(value), std::forward<simd_int_t02>(other));
	}

	template<simd_int_type simd_int_t01, simd_int_type simd_int_t02> JSONIFIER_INLINE simd_int_t opXor(simd_int_t01&& value, simd_int_t02&& other) {
		return _mm_xor_si128(std::forward<simd_int_t01>(value), std::forward<simd_int_t02>(other));
	}

	template<simd_int_type simd_int_t01, simd_int_type simd_int_t02> JSONIFIER_INLINE simd_int_t opOr(simd_int_t01&& value, simd_int_t02&& other) {
		return _mm_or_si128(std::forward<simd_int_t01>(value), std::forward<simd_int_t02>(other));
	}

	template<simd_int_type simd_int_t01> JSONIFIER_INLINE simd_int_t opSetLSB(simd_int_t01&& value, bool valueNew) {
	#if defined(JSONIFIER_WIN) || defined(JSONIFIER_LINUX)
		static constexpr simd_int_t mask{ 0x01u, 0x00u };
	#else
		static constexpr simd_int_t mask{ 0x01u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u };
	#endif
		return valueNew ? _mm_or_si128(value, mask) : _mm_andnot_si128(mask, value);
	}

	template<simd_int_type simd_int_t01> JSONIFIER_INLINE simd_int_t opNot(simd_int_t01&& value) {
		return _mm_xor_si128(std::forward<simd_int_t01>(value), _mm_set1_epi64x(0xFFFFFFFFFFFFFFFFll));
	}

	template<simd_int_type simd_int_t01> JSONIFIER_INLINE static bool opGetMSB(simd_int_t01&& value) {
		simd_int_t result = _mm_and_si128(std::forward<simd_int_t01>(value), _mm_set_epi64x(0x8000000000000000ll, 0x00ll));
		return !_mm_testz_si128(result, result);
	}

	template<simd_int_type simd_int_t01> JSONIFIER_INLINE static bool opBool(simd_int_t01&& value) {
		return !_mm_testz_si128(value, value);
	}

	JSONIFIER_INLINE simd_int_t reset() {
		return _mm_setzero_si128();
	}

#endif

}