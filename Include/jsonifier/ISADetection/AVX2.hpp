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

#if JSONIFIER_CHECK_FOR_INSTRUCTION(JSONIFIER_AVX2)

	inline string_parsing_type simd_base::cmpeq(const simd_int_t& other, const simd_int_t& value) {
		return static_cast<string_parsing_type>(_mm256_movemask_epi8(_mm256_cmpeq_epi8(value, other)));
	}

	inline simd_int_t simd_base::bitAndNot(const simd_int_t& value, const simd_int_t& other) {
		return _mm256_andnot_si256(other, value);
	}

	inline simd_int_t simd_base::shuffle(const simd_int_t& value, const simd_int_t& other) {
		return _mm256_shuffle_epi8(value, other);
	}

	inline simd_int_t simd_base::opOr(const simd_int_t& other, const simd_int_t& value) {
		return _mm256_or_si256(value, other);
	}

	inline simd_int_t simd_base::opAnd(const simd_int_t& other, const simd_int_t& value) {
		return _mm256_and_si256(value, other);
	}

	inline simd_int_t simd_base::opXor(const simd_int_t& other, const simd_int_t& value) {
		return _mm256_xor_si256(value, other);
	}

	inline simd_int_t simd_base::setLSB(const simd_int_t& value, bool valueNew) {
		if (valueNew) {
			return _mm256_or_si256(value, _mm256_set_epi64x(0x00, 0x00, 0x00, 0x01));
		} else {
			return _mm256_andnot_si256(_mm256_set_epi64x(0x00, 0x00, 0x00, 0x01), value);
		}
	}

	inline simd_int_t simd_base::opNot(const simd_int_t& value) {
		return _mm256_xor_si256(value, _mm256_set1_epi64x(static_cast<int64_t>(std::numeric_limits<uint64_t>::max())));
	}

	inline bool simd_base::getMSB(const simd_int_t& value) {
		simd_int_t result = _mm256_and_si256(value, _mm256_set_epi64x(0x8000000000000000, 0x00, 0x00, 0x00));
		return !_mm256_testz_si256(result, result);
	}

	inline bool simd_base::opBool(const simd_int_t& value) {
		return !_mm256_testz_si256(value, value);
	}

	inline simd_int_t simd_base::reset() {
		return _mm256_setzero_si256();
	}

#endif

}