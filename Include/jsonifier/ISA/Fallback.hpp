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

#if !JSONIFIER_CHECK_FOR_INSTRUCTION(JSONIFIER_AVX) && !JSONIFIER_CHECK_FOR_INSTRUCTION(JSONIFIER_AVX2) && !JSONIFIER_CHECK_FOR_INSTRUCTION(JSONIFIER_AVX512)

	using avx_list = jsonifier::concepts::type_list<jsonifier::concepts::type_holder<16, simd_int_128, uint16_t, std::numeric_limits<uint16_t>::max()>>;

	using avx_integer_list = jsonifier::concepts::type_list<jsonifier::concepts::type_holder<16, simd_int_128, uint16_t, 16>,
		jsonifier::concepts::type_holder<8, uint64_t, uint64_t, 8>, jsonifier::concepts::type_holder<1, uint8_t, uint8_t, 2>>;

	template<simd_int_type simd_int_t01, size_t... indices> JSONIFIER_INLINE string_parsing_type _mm128_movemask_epi8(simd_int_t01&& a, std::index_sequence<indices...>&&) {
		string_parsing_type mask{ 0 };
		((mask |= (a.m128x_int8[indices] & 0x80) ? (1 << indices) : 0), ...);
		return mask;
	}

	template<simd_int_type simd_int_t01, simd_int_type simd_int_t02> JSONIFIER_INLINE simd_int_t _mm128_or_si128(simd_int_t01&& valOne, simd_int_t02&& valTwo) {
		simd_int_t value{};
		memcpy(value.m128x_uint64, valOne.m128x_uint64, sizeof(value));
		value.m128x_uint64[0] |= valTwo.m128x_uint64[0];
		value.m128x_uint64[1] |= valTwo.m128x_uint64[1];
		return value;
	}

	template<simd_int_type simd_int_t01, simd_int_type simd_int_t02> JSONIFIER_INLINE simd_int_t _mm128_and_si128(simd_int_t01&& valOne, simd_int_t02&& valTwo) {
		simd_int_t value{};
		memcpy(value.m128x_uint64, valOne.m128x_uint64, sizeof(value));
		value.m128x_uint64[0] &= valTwo.m128x_uint64[0];
		value.m128x_uint64[1] &= valTwo.m128x_uint64[1];
		return value;
	}

	template<simd_int_type simd_int_t01, simd_int_type simd_int_t02> JSONIFIER_INLINE simd_int_t _mm128_andnot_si128(simd_int_t01&& valOne, simd_int_t02&& valTwo) {
		simd_int_t value{};
		memcpy(value.m128x_uint64, valTwo.m128x_uint64, sizeof(value));
		value.m128x_uint64[0] &= ~valOne.m128x_uint64[0];
		value.m128x_uint64[1] &= ~valOne.m128x_uint64[1];
		return value;
	}

	template<simd_int_type simd_int_t01, simd_int_type simd_int_t02> JSONIFIER_INLINE simd_int_t _mm128_xor_si128(simd_int_t01&& valOne, simd_int_t02&& valTwo) {
		simd_int_t value{};
		memcpy(value.m128x_uint64, valOne.m128x_uint64, sizeof(value));
		value.m128x_uint64[0] ^= valTwo.m128x_uint64[0];
		value.m128x_uint64[1] ^= valTwo.m128x_uint64[1];
		return value;
	}

	template<simd_int_type simd_int_t01, simd_int_type simd_int_t02, size_t... indices>
	JSONIFIER_INLINE simd_int_t _mm128_cmpeq_epi8(simd_int_t01&& a, simd_int_t02&& b, std::index_sequence<indices...>&&) {
		simd_int_t result{};
		((result.m128x_int8[indices] = (a.m128x_int8[indices] == b.m128x_int8[indices]) ? 0xFF : 0), ...);
		return result;
	}

	JSONIFIER_INLINE bool _mm128_testz_si128(simd_int_t& valOne, simd_int_t& valTwo) {
		valOne.m128x_uint64[0] &= valTwo.m128x_uint64[0];
		valOne.m128x_uint64[1] &= valTwo.m128x_uint64[1];
		return valOne.m128x_uint64[0] == 0 && valOne.m128x_uint64[1] == 0;
	}

	JSONIFIER_INLINE simd_int_t _mm128_set_epi64x(uint64_t argOne, uint64_t argTwo) {
		simd_int_t returnValue{};
		std::memcpy(&returnValue.m128x_uint64[0], &argTwo, sizeof(uint64_t));
		std::memcpy(&returnValue.m128x_uint64[1], &argOne, sizeof(uint64_t));
		return returnValue;
	}

	JSONIFIER_INLINE simd_int_t _mm128_set1_epi64x(uint64_t argOne) {
		simd_int_t returnValue{};
		std::memcpy(&returnValue.m128x_uint64[0], &argOne, sizeof(uint64_t));
		std::memcpy(&returnValue.m128x_uint64[1], &argOne, sizeof(uint64_t));
		return returnValue;
	}

	template<simd_int_type simd_int_t01, simd_int_type simd_int_t02, size_t... indices>
	JSONIFIER_INLINE simd_int_t _mm128_shuffle_epi8(simd_int_t01&& a, simd_int_t02&& b, std::index_sequence<indices...>) {
		simd_int_t result{};
		uint64_t index{};
		(((index = b.m128x_uint8[indices] & 0x0F), (result.m128x_uint8[indices] = a.m128x_uint8[index])), ...);
		return result;
	}

	template<simd_int_type simd_int_t01, simd_int_type simd_int_t02> JSONIFIER_INLINE string_parsing_type simd_base::cmpeq(simd_int_t01&& value, simd_int_t02&& other) {
		return static_cast<string_parsing_type>(_mm128_movemask_epi8(
			_mm128_cmpeq_epi8(std::forward<simd_int_t01>(value), std::forward<simd_int_t02>(other), std::make_index_sequence<16>{}), std::make_index_sequence<16>{}));
	}

	template<simd_int_type simd_int_t01, simd_int_type simd_int_t02> JSONIFIER_INLINE simd_int_t simd_base::opShuffle(simd_int_t01&& value, simd_int_t02&& other) {
		return _mm128_shuffle_epi8(std::forward<simd_int_t01>(value), std::forward<simd_int_t02>(other), std::make_index_sequence<16>{});
	}

	template<simd_int_type simd_int_t01, simd_int_type simd_int_t02> JSONIFIER_INLINE simd_int_t simd_base::opAndNot(simd_int_t01&& value, simd_int_t02&& other) {
		return _mm128_andnot_si128(std::forward<simd_int_t02>(other), std::forward<simd_int_t01>(value));
	}

	template<simd_int_type simd_int_t01, simd_int_type simd_int_t02> JSONIFIER_INLINE simd_int_t simd_base::opAnd(simd_int_t01&& value, simd_int_t02&& other) {
		return _mm128_and_si128(std::forward<simd_int_t01>(value), std::forward<simd_int_t02>(other));
	}

	template<simd_int_type simd_int_t01, simd_int_type simd_int_t02> JSONIFIER_INLINE simd_int_t simd_base::opXor(simd_int_t01&& value, simd_int_t02&& other) {
		return _mm128_xor_si128(std::forward<simd_int_t01>(value), std::forward<simd_int_t02>(other));
	}

	template<simd_int_type simd_int_t01, simd_int_type simd_int_t02> JSONIFIER_INLINE simd_int_t simd_base::opOr(simd_int_t01&& value, simd_int_t02&& other) {
		return _mm128_or_si128(std::forward<simd_int_t01>(value), std::forward<simd_int_t02>(other));
	}

	template<simd_int_type simd_int_t01> JSONIFIER_INLINE simd_int_t simd_base::setLSB(simd_int_t01&& value, bool valueNew) {
		jsonifier::concepts::unwrap<simd_int_t> mask = _mm128_set_epi64x(0x00ll, 0x01ll);
		return valueNew ? _mm128_or_si128(value, mask) : _mm128_andnot_si128(mask, value);
	}

	template<simd_int_type simd_int_t01> JSONIFIER_INLINE simd_int_t simd_base::opNot(simd_int_t01&& value) {
		return _mm128_xor_si128(std::forward<simd_int_t01>(value), _mm128_set1_epi64x(0xFFFFFFFFFFFFFFFFll));
	}

	template<simd_int_type simd_int_t01> JSONIFIER_INLINE bool simd_base::getMSB(simd_int_t01&& value) {
		simd_int_t result = _mm128_and_si128(std::forward<simd_int_t01>(value), _mm128_set_epi64x(0x8000000000000000ll, 0x00ll));
		return !_mm128_testz_si128(result, result);
	}

	template<simd_int_type simd_int_t01> JSONIFIER_INLINE bool simd_base::opBool(simd_int_t01&& value) {
		return !_mm128_testz_si128(value, value);
	}

	JSONIFIER_INLINE simd_int_t simd_base::reset() {
		return simd_int_t{};
	}

#endif

}