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

#include <jsonifier/ISADetection/ISADetectionBase.hpp>

namespace jsonifier_internal {

#if JSONIFIER_CHECK_FOR_INSTRUCTION(JSONIFIER_AVX512)

	JSONIFIER_INLINE string_parsing_type simd_base::cmpeq(const simd_int_t& value, const simd_int_t& other) {
		return static_cast<string_parsing_type>(_mm512_cmpeq_epi8_mask(value, other));
	}

	JSONIFIER_INLINE simd_int_t simd_base::opShuffle(const simd_int_t& value, const simd_int_t& other) {
		return _mm512_shuffle_epi8(value, other);
	}

	JSONIFIER_INLINE simd_int_t simd_base::opAndNot(const simd_int_t& value, const simd_int_t& other) {
		return _mm512_andnot_si512(other, value);
	}

	JSONIFIER_INLINE simd_int_t simd_base::opAnd(const simd_int_t& value, const simd_int_t& other) {
		return _mm512_and_si512(value, other);
	}

	JSONIFIER_INLINE simd_int_t simd_base::opXor(const simd_int_t& value, const simd_int_t& other) {
		return _mm512_xor_si512(value, other);
	}

	JSONIFIER_INLINE simd_int_t simd_base::opOr(const simd_int_t& value, const simd_int_t& other) {
		return _mm512_or_si512(value, other);
	}

	JSONIFIER_INLINE simd_int_t simd_base::setLSB(const simd_int_t& value, bool valueNew) {
		if (valueNew) {
			return _mm512_or_si512(value, _mm512_set_epi64(0x00ll, 0x00ll, 0x00ll, 0x00ll, 0x00ll, 0x00ll, 0x00ll, 0x01ll));
		} else {
			return _mm512_andnot_si512(_mm512_set_epi64(0x00ll, 0x00ll, 0x00ll, 0x00ll, 0x00ll, 0x00ll, 0x00ll, 0x01ll), value);
		}
	}

	JSONIFIER_INLINE simd_int_t simd_base::opNot(const simd_int_t& value) {
		return _mm512_xor_si512(value, _mm512_set1_epi64(0xFFFFFFFFFFFFFFFFll));
	}

	JSONIFIER_INLINE bool simd_base::getMSB(const simd_int_t& value) {
		simd_int_t result = _mm512_and_si512(value, _mm512_set_epi64(0x8000000000000000ll, 0x00ll, 0x00ll, 0x00ll, 0x00ll, 0x00ll, 0x00ll, 0x00ll));
		return _mm512_test_epi64_mask(result, result);
	}

	JSONIFIER_INLINE bool simd_base::opBool(const simd_int_t& value) {
		return _mm512_test_epi64_mask(value, value);
	}

	JSONIFIER_INLINE simd_int_t simd_base::reset() {
		return _mm512_setzero_si512();
	}

#endif

}