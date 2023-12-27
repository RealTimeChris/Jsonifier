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

#include <jsonifier/ISA/ISADetectionBase.hpp>

namespace jsonifier_internal {

#if JSONIFIER_CHECK_FOR_INSTRUCTION(JSONIFIER_AVX2)

	template<simd_int_type simd_int_t01, simd_int_type simd_int_t02> JSONIFIER_INLINE simd_int_t simd_base::opAndNot(simd_int_t01&& value, simd_int_t02&& other) {
		return _mm256_andnot_si256(std::forward<simd_int_t02>(other), std::forward<simd_int_t01>(value));
	}

	template<simd_int_type simd_int_t01, simd_int_type simd_int_t02> JSONIFIER_INLINE simd_int_t simd_base::opAnd(simd_int_t01&& value, simd_int_t02&& other) {
		return _mm256_and_si256(std::forward<simd_int_t01>(value), std::forward<simd_int_t02>(other));
	}

	template<simd_int_type simd_int_t01, simd_int_type simd_int_t02> JSONIFIER_INLINE simd_int_t simd_base::opXor(simd_int_t01&& value, simd_int_t02&& other) {
		return _mm256_xor_si256(std::forward<simd_int_t01>(value), std::forward<simd_int_t02>(other));
	}

	template<simd_int_type simd_int_t01, simd_int_type simd_int_t02> JSONIFIER_INLINE simd_int_t simd_base::opOr(simd_int_t01&& value, simd_int_t02&& other) {
		return _mm256_or_si256(std::forward<simd_int_t01>(value), std::forward<simd_int_t02>(other));
	}

	template<simd_int_type simd_int_t01> JSONIFIER_INLINE simd_int_t simd_base::opSetLSB(simd_int_t01&& value, bool valueNew) {
		jsonifier::concepts::unwrap_t<simd_int_t> mask = _mm256_set_epi64x(0x00ll, 0x00ll, 0x00ll, 0x01ll);
		return valueNew ? _mm256_or_si256(value, mask) : _mm256_andnot_si256(mask, value);
	}

	template<simd_int_type simd_int_t01> JSONIFIER_INLINE simd_int_t simd_base::opNot(simd_int_t01&& value) {
		return _mm256_xor_si256(std::forward<simd_int_t01>(value), _mm256_set1_epi64x(0xFFFFFFFFFFFFFFFFll));
	}

	template<simd_int_type simd_int_t01> JSONIFIER_INLINE bool simd_base::opGetMSB(simd_int_t01&& value) {
		simd_int_t result = _mm256_and_si256(std::forward<simd_int_t01>(value), _mm256_set_epi64x(0x8000000000000000ll, 0x00ll, 0x00ll, 0x00ll));
		return !_mm256_testz_si256(result, result);
	}

	template<simd_int_type simd_int_t01> JSONIFIER_INLINE bool simd_base::opBool(simd_int_t01&& value) {
		return !_mm256_testz_si256(value, value);
	}

	JSONIFIER_INLINE simd_int_t simd_base::reset() {
		return _mm256_setzero_si256();
	}

#endif

}