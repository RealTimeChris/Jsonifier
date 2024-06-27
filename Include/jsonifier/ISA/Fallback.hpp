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

#include <jsonifier/Config.hpp>

namespace simd_internal {

#if !JSONIFIER_CHECK_FOR_INSTRUCTION(JSONIFIER_AVX) && !JSONIFIER_CHECK_FOR_INSTRUCTION(JSONIFIER_AVX2) && !JSONIFIER_CHECK_FOR_INSTRUCTION(JSONIFIER_AVX512) && \
	!JSONIFIER_CHECK_FOR_INSTRUCTION(JSONIFIER_NEON)

	template<typename simd_int_t01, size_t... indices> JSONIFIER_INLINE string_parsing_type _mm128_movemask_epi8(simd_int_t01&& a, std::index_sequence<indices...>&&) {
		string_parsing_type mask{ 0 };
		((mask |= (a.m128x_int8[indices] & 0x80) ? (1 << indices) : 0), ...);
		return mask;
	}

	template<typename simd_int_t01, typename simd_int_t02> JSONIFIER_INLINE simd_int_t _mm128_or_si128(simd_int_t01&& valOne, simd_int_t02&& valTwo) {
		simd_int_t value{};
		memcpy(value.m128x_uint64, valOne.m128x_uint64, sizeof(value));
		value.m128x_uint64[0] |= valTwo.m128x_uint64[0];
		value.m128x_uint64[1] |= valTwo.m128x_uint64[1];
		return value;
	}

	template<typename simd_int_t01, typename simd_int_t02> JSONIFIER_INLINE simd_int_t _mm128_and_si128(simd_int_t01&& valOne, simd_int_t02&& valTwo) {
		simd_int_t value{};
		memcpy(value.m128x_uint64, valOne.m128x_uint64, sizeof(value));
		value.m128x_uint64[0] &= valTwo.m128x_uint64[0];
		value.m128x_uint64[1] &= valTwo.m128x_uint64[1];
		return value;
	}

	template<typename simd_int_t01, typename simd_int_t02> JSONIFIER_INLINE simd_int_t _mm128_andnot_si128(simd_int_t01&& valOne, simd_int_t02&& valTwo) {
		simd_int_t value{};
		memcpy(value.m128x_uint64, valTwo.m128x_uint64, sizeof(value));
		value.m128x_uint64[0] &= ~valOne.m128x_uint64[0];
		value.m128x_uint64[1] &= ~valOne.m128x_uint64[1];
		return value;
	}

	template<typename simd_int_t01, typename simd_int_t02> JSONIFIER_INLINE simd_int_t _mm128_xor_si128(simd_int_t01&& valOne, simd_int_t02&& valTwo) {
		simd_int_t value{};
		memcpy(value.m128x_uint64, valOne.m128x_uint64, sizeof(value));
		value.m128x_uint64[0] ^= valTwo.m128x_uint64[0];
		value.m128x_uint64[1] ^= valTwo.m128x_uint64[1];
		return value;
	}

	template<typename simd_int_t01, typename simd_int_t02, size_t... indices>
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

	template<typename simd_int_t01, typename simd_int_t02, size_t... indices>
	JSONIFIER_INLINE simd_int_t _mm128_shuffle_epi8(simd_int_t01&& a, simd_int_t02&& b, std::index_sequence<indices...>) {
		simd_int_t result{};
		size_t index{};
		(((index = b.m128x_uint8[indices] & 0x0F), (result.m128x_uint8[indices] = a.m128x_uint8[index])), ...);
		return result;
	}

	template<simd_int_type simd_int_t01, simd_int_type simd_int_t02> JSONIFIER_INLINE simd_int_t opAndNot(simd_int_t01&& value, simd_int_t02&& other) {
		return _mm128_andnot_si128(std::forward<simd_int_t02>(other), std::forward<simd_int_t01>(value));
	}

	template<simd_int_type simd_int_t01, simd_int_type simd_int_t02> JSONIFIER_INLINE simd_int_t opAnd(simd_int_t01&& value, simd_int_t02&& other) {
		return _mm128_and_si128(std::forward<simd_int_t01>(value), std::forward<simd_int_t02>(other));
	}

	template<simd_int_type simd_int_t01, simd_int_type simd_int_t02> JSONIFIER_INLINE simd_int_t opXor(simd_int_t01&& value, simd_int_t02&& other) {
		return _mm128_xor_si128(std::forward<simd_int_t01>(value), std::forward<simd_int_t02>(other));
	}

	template<simd_int_type simd_int_t01, simd_int_type simd_int_t02> JSONIFIER_INLINE simd_int_t opOr(simd_int_t01&& value, simd_int_t02&& other) {
		return _mm128_or_si128(std::forward<simd_int_t01>(value), std::forward<simd_int_t02>(other));
	}

	template<simd_int_type simd_int_t01> JSONIFIER_INLINE simd_int_t opSetLSB(simd_int_t01&& value, bool valueNew) {
		jsonifier::concepts::unwrap_t<simd_int_t> mask = _mm128_set_epi64x(0x00ll, 0x01ll);
		return valueNew ? _mm128_or_si128(value, mask) : _mm128_andnot_si128(mask, value);
	}

	template<simd_int_type simd_int_t01> JSONIFIER_INLINE simd_int_t opNot(simd_int_t01&& value) {
		return _mm128_xor_si128(std::forward<simd_int_t01>(value), _mm128_set1_epi64x(0xFFFFFFFFFFFFFFFFll));
	}

	template<simd_int_type simd_int_t01> JSONIFIER_INLINE bool opGetMSB(simd_int_t01&& value) {
		simd_int_t result = _mm128_and_si128(std::forward<simd_int_t01>(value), _mm128_set_epi64x(0x8000000000000000ll, 0x00ll));
		return !_mm128_testz_si128(result, result);
	}

	template<simd_int_type simd_int_t01> JSONIFIER_INLINE bool opBool(simd_int_t01&& value) {
		return !_mm128_testz_si128(value, value);
	}

	JSONIFIER_INLINE simd_int_t reset() {
		return simd_int_t{};
	}

#endif

}