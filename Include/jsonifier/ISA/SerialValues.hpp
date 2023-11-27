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

	template<jsonifier::concepts::integer_t simd_int_t01, size_t... indices>
	JSONIFIER_INLINE string_parsing_type _mmx_movemask_epi8(simd_int_t01&& a, std::index_sequence<indices...>&&) {
		string_parsing_type mask{ 0 };
		((mask |= (a.m128x_int8[indices] & 0x80) ? (1 << indices) : 0), ...);
		return mask;
	}

	template<jsonifier::concepts::integer_t simd_int_t01, jsonifier::concepts::integer_t simd_int_t02>
	JSONIFIER_INLINE simd_int_t01 _mmx_or_six(simd_int_t01&& valOne, simd_int_t02&& valTwo) {
		return valOne | valTwo;
	}

	template<jsonifier::concepts::integer_t simd_int_t01, jsonifier::concepts::integer_t simd_int_t02>
	JSONIFIER_INLINE simd_int_t01 _mmx_and_six(simd_int_t01&& valOne, simd_int_t02&& valTwo) {
		return valOne & valTwo;
	}

	template<jsonifier::concepts::integer_t simd_int_t01, jsonifier::concepts::integer_t simd_int_t02>
	JSONIFIER_INLINE simd_int_t01 _mmx_andnot_six(simd_int_t01&& valOne, simd_int_t02&& valTwo) {
		return valOne & ~valTwo;
	}

	template<jsonifier::concepts::integer_t simd_int_t01, jsonifier::concepts::integer_t simd_int_t02>
	JSONIFIER_INLINE simd_int_t01 _mmx_xor_six(simd_int_t01&& valOne, simd_int_t02&& valTwo) {
		return valOne ^ valTwo;
	}

	template<jsonifier::concepts::integer_t simd_int_t01, jsonifier::concepts::integer_t simd_int_t02, size_t... indices>
	JSONIFIER_INLINE simd_int_t01 _mmx_cmpeq_epi8(simd_int_t01&& a, simd_int_t02&& b, std::index_sequence<indices...>&&) {
		simd_int_t01 result{};
		((result.m128x_int8[indices] = (a.m128x_int8[indices] == b.m128x_int8[indices]) ? 0xFF : 0), ...);
		return result;
	}

	template<jsonifier::concepts::integer_t simd_int_t01, jsonifier::concepts::integer_t simd_int_t02>
	JSONIFIER_INLINE bool _mmx_testz_six(simd_int_t01&& valOne, simd_int_t02&& valTwo) {
		valOne.m128x_uint64[0] &= valTwo.m128x_uint64[0];
		valOne.m128x_uint64[1] &= valTwo.m128x_uint64[1];
		return valOne.m128x_uint64[0] == 0 && valOne.m128x_uint64[1] == 0;
	}

	template<jsonifier::concepts::integer_t simd_int_t01> JSONIFIER_INLINE simd_int_t01 _mmx_set_epi64(simd_int_t01 argOne) {
		simd_int_t01 returnValue{};
		std::memcpy(&returnValue.m128x_uint64[1], &argOne, sizeof(uint64_t));
		return returnValue;
	}

	template<jsonifier::concepts::integer_t simd_int_t01> JSONIFIER_INLINE simd_int_t01 _mmx_set1_epi64(simd_int_t01 argOne) {
		simd_int_t01 returnValue{};
		std::memcpy(&returnValue.m128x_uint64[0], &argOne, sizeof(uint64_t));
		std::memcpy(&returnValue.m128x_uint64[1], &argOne, sizeof(uint64_t));
		return returnValue;
	}

	template<jsonifier::concepts::integer_t simd_int_t01, jsonifier::concepts::integer_t simd_int_t02, size_t... indices>
	JSONIFIER_INLINE simd_int_t01 _mmx_shuffle_epi8(simd_int_t01&& a, simd_int_t02&& b, std::index_sequence<indices...>) {
		simd_int_t01 result{};
		uint64_t index{};
		(((index = b.m128x_uint8[indices] & 0x0F), (result.m128x_uint8[indices] = a.m128x_uint8[index])), ...);
		return result;
	}

	template<jsonifier::concepts::integer_t simd_int_t01, jsonifier::concepts::integer_t simd_int_t02>
	JSONIFIER_INLINE string_parsing_type simd_base::cmpeq(simd_int_t01&& value, simd_int_t02&& other) {
		return static_cast<string_parsing_type>(_mmx_cmpeq_epi8_mask(std::forward<simd_int_t01>(value), std::forward<simd_int_t02>(other)));
	}

	template<jsonifier::concepts::integer_t simd_int_t01, jsonifier::concepts::integer_t simd_int_t02>
	JSONIFIER_INLINE simd_int_t01 simd_base::opShuffle(simd_int_t01&& value, simd_int_t02&& other) {
		return _mmx_shuffle_epi8(std::forward<simd_int_t01>(value), std::forward<simd_int_t02>(other));
	}

	template<jsonifier::concepts::integer_t simd_int_t01, jsonifier::concepts::integer_t simd_int_t02>
	JSONIFIER_INLINE simd_int_t01 simd_base::opAndNot(simd_int_t01&& value, simd_int_t02&& other) {
		return _mmx_andnot_six(std::forward<simd_int_t02>(other), std::forward<simd_int_t01>(value));
	}

	template<jsonifier::concepts::integer_t simd_int_t01, jsonifier::concepts::integer_t simd_int_t02>
	JSONIFIER_INLINE simd_int_t01 simd_base::opAnd(simd_int_t01&& value, simd_int_t02&& other) {
		return _mmx_and_six(std::forward<simd_int_t01>(value), std::forward<simd_int_t02>(other));
	}

	template<jsonifier::concepts::integer_t simd_int_t01, jsonifier::concepts::integer_t simd_int_t02>
	JSONIFIER_INLINE simd_int_t01 simd_base::opXor(simd_int_t01&& value, simd_int_t02&& other) {
		return _mmx_xor_six(std::forward<simd_int_t01>(value), std::forward<simd_int_t02>(other));
	}

	template<jsonifier::concepts::integer_t simd_int_t01, jsonifier::concepts::integer_t simd_int_t02>
	JSONIFIER_INLINE simd_int_t01 simd_base::opOr(simd_int_t01&& value, simd_int_t02&& other) {
		return _mmx_or_six(std::forward<simd_int_t01>(value), std::forward<simd_int_t02>(other));
	}

	template<jsonifier::concepts::integer_t integer_t_new> JSONIFIER_INLINE integer_t_new simd_base::setLSB(integer_t_new&& value, bool valueNew) {
		jsonifier::concepts::unwrap<integer_t_new> mask = _mmx_set_epi64(0x01ll);
		return valueNew ? _mmx_or_six(value, mask) : _mmx_andnot_six(mask, value);
	}

	template<jsonifier::concepts::integer_t integer_t_new> JSONIFIER_INLINE integer_t_new simd_base::opNot(integer_t_new&& value) {
		return _mmx_xor_six(std::forward<integer_t_new>(value), _mmx_set1_epi64(0xFFFFFFFFFFFFFFFFll));
	}

	template<jsonifier::concepts::integer_t integer_t_new> JSONIFIER_INLINE bool simd_base::getMSB(integer_t_new&& value) {
		integer_t_new result = _mmx_and_six(value, _mmx_set_epi64(0x8000000000000000l));
		return _mmx_test_epi64_mask(result, result);
	}

	template<jsonifier::concepts::integer_t integer_t_new> JSONIFIER_INLINE bool simd_base::opBool(integer_t_new&& value) {
		return _mmx_test_epi64_mask(value, value);
	}

}