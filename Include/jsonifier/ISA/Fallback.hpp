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

#if !JSONIFIER_CHECK_FOR_INSTRUCTION(JSONIFIER_AVX) && !JSONIFIER_CHECK_FOR_INSTRUCTION(JSONIFIER_AVX2) && !JSONIFIER_CHECK_FOR_INSTRUCTION(JSONIFIER_AVX512) && \
	!JSONIFIER_CHECK_FOR_INSTRUCTION(JSONIFIER_NEON)

	template<jsonifier::concepts::simd_int_type simd_int_t01, jsonifier::concepts::simd_int_type simd_int_t02>
	JSONIFIER_INLINE jsonifier_internal::__m128x opAndNot(simd_int_t01&& value, simd_int_t02&& other) {
		return jsonifier_internal::mm128AndNotSi128N(std::forward<simd_int_t02>(other), std::forward<simd_int_t01>(value));
	}

	template<jsonifier::concepts::simd_int_type simd_int_t01, jsonifier::concepts::simd_int_type simd_int_t02>
	JSONIFIER_INLINE jsonifier_internal::__m128x opAnd(simd_int_t01&& value, simd_int_t02&& other) {
		return jsonifier_internal::mm128AndSi128(std::forward<simd_int_t01>(value), std::forward<simd_int_t02>(other));
	}

	template<jsonifier::concepts::simd_int_type simd_int_t01, jsonifier::concepts::simd_int_type simd_int_t02>
	JSONIFIER_INLINE jsonifier_internal::__m128x opXor(simd_int_t01&& value, simd_int_t02&& other) {
		return jsonifier_internal::mm128XorSi128(std::forward<simd_int_t01>(value), std::forward<simd_int_t02>(other));
	}

	template<jsonifier::concepts::simd_int_128_type simd_int_t01, jsonifier::concepts::simd_int_128_type simd_int_t02>
	JSONIFIER_INLINE jsonifier_simd_int_128 opAdd(simd_int_t01&& value, simd_int_t02&& other) {
		return jsonifier_internal::mm128AddEpi64(std::forward<simd_int_t01>(value), std::forward<simd_int_t02>(other));
	}

	template<int32_t other, jsonifier::concepts::simd_int_128_type simd_int_t01> JSONIFIER_INLINE jsonifier_simd_int_128 opSlli(simd_int_t01&& value) {
		return jsonifier_internal::mm128SlliEpi64(std::forward<simd_int_t01>(value), other);
	}

	template<int32_t other, jsonifier::concepts::simd_int_128_type simd_int_t01> JSONIFIER_INLINE jsonifier_simd_int_128 opSrli(simd_int_t01&& value) {
		return jsonifier_internal::mm128SrliEpi64(std::forward<simd_int_t01>(value), other);
	}

	template<int32_t other, jsonifier::concepts::simd_int_128_type simd_int_t01> JSONIFIER_INLINE jsonifier_simd_int_128 opShuffle(simd_int_t01&& value) {
		return jsonifier_internal::mm128ShuffleEpi32(std::forward<simd_int_t01>(value), other);
	}

	template<jsonifier::concepts::simd_int_128_type simd_int_t01, jsonifier::concepts::simd_int_128_type simd_int_t02>
	JSONIFIER_INLINE jsonifier_simd_int_128 opMul(simd_int_t01&& value, simd_int_t02&& other) {
		return jsonifier_internal::mm128MulEpi32(std::forward<simd_int_t01>(value), std::forward<simd_int_t02>(other));
	}

	template<jsonifier::concepts::simd_int_type simd_int_t01, jsonifier::concepts::simd_int_type simd_int_t02>
	JSONIFIER_INLINE jsonifier_internal::__m128x opOr(simd_int_t01&& value, simd_int_t02&& other) {
		return jsonifier_internal::mm128OrSi128(std::forward<simd_int_t01>(value), std::forward<simd_int_t02>(other));
	}

	template<jsonifier::concepts::simd_int_type simd_int_t01> JSONIFIER_INLINE jsonifier_internal::__m128x opSetLSB(simd_int_t01&& value, bool valueNew) {
		jsonifier_internal::unwrap_t<jsonifier_internal::__m128x> mask = jsonifier_internal::mm128SetrEpi64x(0x00ll, 0x01ll);
		return valueNew ? jsonifier_internal::mm128OrSi128(value, mask) : jsonifier_internal::mm128AndNotSi128N(mask, value);
	}

	template<jsonifier::concepts::simd_int_type simd_int_t01> JSONIFIER_INLINE jsonifier_internal::__m128x opNot(simd_int_t01&& value) {
		return jsonifier_internal::mm128XorSi128(std::forward<simd_int_t01>(value), jsonifier_internal::mm128Set1Epi64x(0xFFFFFFFFFFFFFFFFll));
	}

	template<jsonifier::concepts::simd_int_type simd_int_t01> JSONIFIER_INLINE bool opGetMSB(simd_int_t01&& value) {
		jsonifier_internal::__m128x result =
			jsonifier_internal::mm128AndSi128(std::forward<simd_int_t01>(value), jsonifier_internal::mm128SetrEpi64x(0x8000000000000000ll, 0x00ll));
		return !mm128TestzSi128(result, result);
	}

	template<jsonifier::concepts::simd_int_type simd_int_t01> JSONIFIER_INLINE bool opBool(simd_int_t01&& value) {
		return !jsonifier_internal::mm128TestzSi128(value, value);
	}

	template<typename simd_int_t> JSONIFIER_INLINE jsonifier_internal::__m128x reset() {
		return jsonifier_internal::__m128x{};
	}

#endif

}