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

#if !JSONIFIER_CHECK_FOR_INSTRUCTION(JSONIFIER_AVX) && !JSONIFIER_CHECK_FOR_INSTRUCTION(JSONIFIER_AVX2) && !JSONIFIER_CHECK_FOR_INSTRUCTION(JSONIFIER_AVX512)

	template<size_t index01, size_t index02> inline void _mm128_movemask_epi8_helper01(const simd_int_t& a, uint8_t& value, int16_t& result, int16_t& mask) {
		static constexpr uint64_t newIndex01{ index01 * 8 };
		static constexpr uint64_t newIndex02{ index02 * 8 };
		value = (a.m128x_uint64[index01] >> (newIndex02)) & 0xFF, (mask = (value >> 7) & 1), result |= mask << (newIndex01 + index02);
	}

	template<size_t index, size_t... indices> inline void _mm128_movemask_epi8_helper(const simd_int_t& a, int16_t& result, std::index_sequence<indices...>) {
		int16_t mask{};
		uint8_t value{};
		((_mm128_movemask_epi8_helper01<index, indices>(a, value, result, mask)), ...);
	}

	template<size_t... indices> inline int16_t _mm128_movemask_epi8(const simd_int_t& a, std::index_sequence<indices...>) {
		int16_t result{};
		((_mm128_movemask_epi8_helper<indices>(a, result, std::make_index_sequence<8>{})), ...);
		return result;
	}

	inline simd_int_t _mm128_or_si128(const simd_int_t& valOne, const simd_int_t& valTwo) {
		simd_int_t value{};
		value.m128x_uint64[0] = valOne.m128x_uint64[0] | valTwo.m128x_uint64[0];
		value.m128x_uint64[1] = valOne.m128x_uint64[1] | valTwo.m128x_uint64[1];
		return value;
	}

	inline simd_int_t _mm128_and_si128(const simd_int_t& valOne, const simd_int_t& valTwo) {
		simd_int_t value{};
		value.m128x_uint64[0] = valOne.m128x_uint64[0] & valTwo.m128x_uint64[0];
		value.m128x_uint64[1] = valOne.m128x_uint64[1] & valTwo.m128x_uint64[1];
		return value;
	}

	inline simd_int_t _mm128_andnot_si128(const simd_int_t& valOne, const simd_int_t& valTwo) {
		simd_int_t result{};
		result.m128x_uint64[0] = valTwo.m128x_uint64[0] & (~valOne.m128x_uint64[0]);
		result.m128x_uint64[1] = valTwo.m128x_uint64[1] & (~valOne.m128x_uint64[1]);
		return result;
	}

	inline simd_int_t _mm128_xor_si128(const simd_int_t& valOne, const simd_int_t& valTwo) {
		simd_int_t value{};
		value.m128x_uint64[0] = valOne.m128x_uint64[0] ^ valTwo.m128x_uint64[0];
		value.m128x_uint64[1] = valOne.m128x_uint64[1] ^ valTwo.m128x_uint64[1];
		return value;
	}

	template<uint64_t index02 = 0, size_t... indices>
	inline void _mm128_cmpeq_epi8_helper(simd_int_t& result, const simd_int_t& a, const simd_int_t& b, std::index_sequence<indices...>) {
		uint8_t a8{};
		uint8_t b8{};
		(((a8 = (a.m128x_uint64[index02] >> (indices * 8)) & 0xFF), (b8 = (b.m128x_uint64[index02] >> (indices * 8)) & 0xFF),
			 (result.m128x_uint64[index02] |= (a8 == b8 ? 0xFFull : 0) << (indices * 8))),
			...);
	}

	template<size_t... indices> inline simd_int_t _mm128_cmpeq_epi8(const simd_int_t& a, const simd_int_t& b, std::index_sequence<indices...>) {
		simd_int_t result{};
		(((_mm128_cmpeq_epi8_helper<indices>(result, a, b, std::make_index_sequence<8>{})),
			 (result = _mm128_or_si128(result, _mm128_cmpeq_epi8(a, b, std::make_index_sequence<sizeof...(indices) - 1>{})))),
			...);
		return result;
	}

	inline bool _mm128_testz_si128(const simd_int_t& valOne, const simd_int_t& valTwo) {
		simd_int_t result{};
		result.m128x_uint64[0] = valOne.m128x_uint64[0] & valTwo.m128x_uint64[0];
		result.m128x_uint64[1] = valOne.m128x_uint64[1] & valTwo.m128x_uint64[1];
		return result.m128x_uint64[0] == 0 && result.m128x_uint64[1] == 0;
	}

	inline simd_int_t _mm128_set_epi64x(uint64_t argOne, uint64_t argTwo) {
		simd_int_t returnValue{};
		returnValue.m128x_uint64[0] = argTwo;
		returnValue.m128x_uint64[1] = argOne;
		return returnValue;
	}

	inline simd_int_t _mm128_set1_epi64x(uint64_t argOne) {
		simd_int_t returnValue{};
		returnValue.m128x_uint64[0] = argOne;
		returnValue.m128x_uint64[1] = argOne;
		return returnValue;
	}

	template<size_t... indices> inline simd_int_t _mm128_shuffle_epi8(const simd_int_t& a, const simd_int_t& b, std::index_sequence<indices...>) {
		simd_int_t result{};
		int32_t index{};
		(((index = b.m128x_uint8[indices] & 0x0F), (result.m128x_uint8[indices] = a.m128x_uint8[index])), ...);
		return result;
	}

	inline simd_int_t _mm128_setzero_si128() {
		return simd_int_t{};
	}

	inline string_parsing_type simd_base::cmpeq(const simd_int_t& value, const simd_int_t& other) {
		return static_cast<string_parsing_type>(_mm128_movemask_epi8(_mm128_cmpeq_epi8(value, other, std::make_index_sequence<2>{}), std::make_index_sequence<2>{}));
	}

	inline simd_int_t simd_base::opShuffle(const simd_int_t& value, const simd_int_t& other) {
		return _mm128_shuffle_epi8(value, other, std::make_index_sequence<16>{});
	}

	inline simd_int_t simd_base::opAndNot(const simd_int_t& value, const simd_int_t& other) {
		return _mm128_andnot_si128(other, value);
	}

	inline simd_int_t simd_base::opAnd(const simd_int_t& value, const simd_int_t& other) {
		return _mm128_and_si128(value, other);
	}

	inline simd_int_t simd_base::opXor(const simd_int_t& value, const simd_int_t& other) {
		return _mm128_xor_si128(value, other);
	}

	inline simd_int_t simd_base::opOr(const simd_int_t& value, const simd_int_t& other) {
		return _mm128_or_si128(value, other);
	}

	inline simd_int_t simd_base::setLSB(const simd_int_t& value, bool valueNew) {
		if (valueNew) {
			return _mm128_or_si128(value, _mm128_set_epi64x(0x00ll, 0x01ll));
		} else {
			return _mm128_andnot_si128(_mm128_set_epi64x(0x00ll, 0x01ll), value);
		}
	}

	inline simd_int_t simd_base::opNot(const simd_int_t& value) {
		return _mm128_xor_si128(value, _mm128_set1_epi64x(0xFFFFFFFFFFFFFFFFll));
	}

	inline bool simd_base::getMSB(const simd_int_t& value) {
		simd_int_t result = _mm128_and_si128(value, _mm128_set_epi64x(0x8000000000000000ll, 0x00ll));
		return !_mm128_testz_si128(result, result);
	}

	inline bool simd_base::opBool(const simd_int_t& value) {
		return !_mm128_testz_si128(value, value);
	}

	inline simd_int_t simd_base::reset() {
		return _mm128_setzero_si128();
	}

#endif

}