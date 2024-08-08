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

	#define opShuffle(value, other) \
		([](auto v, auto o) { \
			static uint8x16_t mask{ 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F }; \
			return vqtbl1q_u8(v, vandq_u8(o, mask)); \
		})(value, other)

#elif JSONIFIER_CHECK_FOR_INSTRUCTION(JSONIFIER_ANY_AVX)

	#define opShuffle(value, other) \
		([](auto v, auto o) { \
			if constexpr (sizeof(decltype(v)) == 16) { \
				return _mm_shuffle_epi8(v, o); \
			} else if constexpr (sizeof(decltype(v)) == 32) { \
				return _mm256_shuffle_epi8(v, o); \
			} else if constexpr (sizeof(decltype(v)) == 64) { \
				return _mm512_shuffle_epi8(v, o); \
			} else { \
				static_assert(sizeof(decltype(v)) == 16 || sizeof(decltype(v)) == 32 || sizeof(decltype(v)) == 64, "Unsupported type size"); \
				return uint64_t(0); \
			} \
		})(value, other)

#else

	template<jsonifier::concepts::simd_int_128_type simd_int_t01, jsonifier::concepts::simd_int_128_type simd_int_t02>
	JSONIFIER_ALWAYS_INLINE static jsonifier_simd_int_128 opShuffle(simd_int_t01&& value, simd_int_t02&& other) noexcept {
		return mm128ShuffleEpi8(std::forward<simd_int_t01>(value), std::forward<simd_int_t02>(other), std::make_index_sequence<16>{});
	}

#endif

}