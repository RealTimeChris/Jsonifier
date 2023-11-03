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

#if JSONIFIER_CHECK_FOR_INSTRUCTION(JSONIFIER_AVX) && !JSONIFIER_CHECK_FOR_INSTRUCTION(JSONIFIER_AVX2) && !JSONIFIER_CHECK_FOR_INSTRUCTION(JSONIFIER_AVX512)

	jsonifier_inline simd_int_t simd_base_internal<BitsPerStep>::opOr(const simd_int_t& other, const simd_int_t& value) {
		return _mm_or_si128(value, other);
	}

	jsonifier_inline simd_int_t simd_base_internal<BitsPerStep>::opAnd(const simd_int_t& other, const simd_int_t& value) {
		return _mm_and_si128(value, other);
	}

	jsonifier_inline simd_int_t simd_base_internal<BitsPerStep>::opXor(const simd_int_t& other, const simd_int_t& value) {
		return _mm_xor_si128(value, other);
	}

	jsonifier_inline string_parsing_type simd_base_internal<BitsPerStep>::cmpeq(const simd_int_t& other, const simd_int_t& value) {
		return static_cast<string_parsing_type>(_mm_movemask_epi8(_mm_cmpeq_epi8(value, other)));
	}

	jsonifier_inline simd_int_t simd_base_internal<BitsPerStep>::opNot(const simd_int_t& value) {
		return _mm_xor_si128(value, _mm_set1_epi64x(static_cast<int64_t>(std::numeric_limits<uint64_t>::max())));
	}

	jsonifier_inline bool simd_base_internal<BitsPerStep>::opBool(const simd_int_t& value) {
		return !_mm_testz_si128(value, value);
	}

	jsonifier_inline simd_int_t simd_base_internal<BitsPerStep>::bitAndNot(const simd_int_t& value, const simd_int_t& other) {
		return _mm_andnot_si128(other, value);
	}

	jsonifier_inline simd_int_t simd_base_internal<BitsPerStep>::shuffle(const simd_int_t& value, const simd_int_t& other) {
		return _mm_shuffle_epi8(value, other);
	}

	template<typename value_type> jsonifier_inline void simd_base_internal<BitsPerStep>::storeu(const simd_int_t& value, value_type* storageLocation) {
		_mm_storeu_si128(reinterpret_cast<__m128i*>(storageLocation), value);
	}

	template<typename value_type> jsonifier_inline void simd_base_internal<BitsPerStep>::store(const simd_int_t& value, value_type* storageLocation) {
		_mm_store_si128(reinterpret_cast<__m128i*>(storageLocation), value);
	}

	jsonifier_inline simd_int_t simd_base_internal<BitsPerStep>::setLSB(const simd_int_t& value, bool valueNew) {
		if (valueNew) {
			return _mm_or_si128(value, _mm_set_epi64x(0x00, 0x01));
		} else {
			return _mm_andnot_si128(_mm_set_epi64x(0x00, 0x01), value);
		}
	}

	jsonifier_inline bool simd_base_internal<BitsPerStep>::getMSB(const simd_int_t& value) {
		simd_int_t result = _mm_and_si128(value, _mm_set_epi64x(0x8000000000000000, 0x00));
		return !_mm_testz_si128(result, result);
	}

	jsonifier_inline void simd_base_internal<BitsPerStep>::processValue(const simd_int_128& allOnes, uint64_t& value, uint64_t& prevInString) {
		value		 = static_cast<uint64_t>(_mm_cvtsi128_si64(_mm_clmulepi64_si128(_mm_set_epi64x(0ULL, static_cast<int64_t>(value)), allOnes, 0))) ^ prevInString;
		prevInString = uint64_t(static_cast<int64_t>(value) >> 63);
	}

	jsonifier_inline simd_int_t simd_base_internal<BitsPerStep>::carrylessMultiplication(const simd_int_t& value, uint64_t& prevInString) {
		jsonifier_constexpr simd_int_128 allOnes{ simdFromValue<simd_int_128>(0xFFu) };
		alignas(BytesPerStep) uint64_t valuesNewer01[SixtyFourBitsPerStep]{};
		store(value, valuesNewer01);
		processValue(allOnes, valuesNewer01[0], prevInString);
		processValue(allOnes, valuesNewer01[1], prevInString);
		return gatherValues<simd_int_t>(valuesNewer01);
	}

#endif

}