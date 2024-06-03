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

#include <jsonifier/ISA/Fallback.hpp>
#include <jsonifier/ISA/Neon.hpp>

namespace simd_internal {

#if JSONIFIER_CHECK_FOR_INSTRUCTION(JSONIFIER_NEON)

	template<simd_int_128_type simd_int_t01, simd_int_128_type simd_int_t02> JSONIFIER_INLINE static simd_int_128 opShuffle(simd_int_t01&& value, simd_int_t02&& other) {
		static constexpr uint8x16_t mask{ 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F };
		return vqtbl1q_u8(std::forward<simd_int_t01>(value), vandq_u8(std::forward<simd_int_t02>(other), mask));
	}

#elif JSONIFIER_CHECK_FOR_AVX(JSONIFIER_AVX)

	template<simd_int_128_type simd_int_t01, simd_int_128_type simd_int_t02> JSONIFIER_INLINE static simd_int_128 opShuffle(simd_int_t01&& value, simd_int_t02&& other) {
		return _mm_shuffle_epi8(std::forward<simd_int_t01>(value), std::forward<simd_int_t02>(other));
	}

	#if JSONIFIER_CHECK_FOR_AVX(JSONIFIER_AVX2)

	template<simd_int_256_type simd_int_t01, simd_int_256_type simd_int_t02> JSONIFIER_INLINE static simd_int_256 opShuffle(simd_int_t01&& value, simd_int_t02&& other) {
		return _mm256_shuffle_epi8(std::forward<simd_int_t01>(value), std::forward<simd_int_t02>(other));
	}

		#if JSONIFIER_CHECK_FOR_AVX(JSONIFIER_AVX512)

	template<simd_int_512_type simd_int_t01, simd_int_512_type simd_int_t02> JSONIFIER_INLINE static simd_int_512 opShuffle(simd_int_t01&& value, simd_int_t02&& other) {
		return _mm512_shuffle_epi8(std::forward<simd_int_t01>(value), std::forward<simd_int_t02>(other));
	}

		#endif

	#endif

#else

	template<simd_int_128_type simd_int_t01, simd_int_128_type simd_int_t02> JSONIFIER_INLINE static simd_int_128 opShuffle(simd_int_t01&& value, simd_int_t02&& other) {
		return _mm128_shuffle_epi8(std::forward<simd_int_t01>(value), std::forward<simd_int_t02>(other), std::make_index_sequence<16>{});
	}

#endif

}