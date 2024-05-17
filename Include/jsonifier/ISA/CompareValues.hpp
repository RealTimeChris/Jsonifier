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
#include <jsonifier/Base.hpp>

namespace simd_internal {

#if JSONIFIER_CHECK_FOR_INSTRUCTION(JSONIFIER_NEON)

	template<simd_int_128_type simd_int_t01> JSONIFIER_INLINE static uint16_t toBitMask(simd_int_t01&& value) {
		static constexpr uint8x16_t bit_mask{ 0x01, 0x02, 0x4, 0x8, 0x10, 0x20, 0x40, 0x80, 0x01, 0x02, 0x4, 0x8, 0x10, 0x20, 0x40, 0x80 };
		auto minput	   = value & bit_mask;
		uint8x16_t tmp = vpaddq_u8(minput, minput);
		tmp			   = vpaddq_u8(tmp, tmp);
		tmp			   = vpaddq_u8(tmp, tmp);
		return static_cast<uint16_t>(vgetq_lane_u16(vreinterpretq_u16_u8(tmp), 0));
	}

	template<simd_int_128_type simd_int_t01, simd_int_128_type simd_int_t02> JSONIFIER_INLINE static uint16_t opCmpEq(simd_int_t01&& value, simd_int_t02&& other) {
		return toBitMask(vceqq_u8(value, other));
	}

#elif JSONIFIER_CHECK_FOR_AVX(JSONIFIER_AVX)

	template<simd_int_128_type simd_int_t01, simd_int_128_type simd_int_t02> JSONIFIER_INLINE static auto opCmpEq(simd_int_t01&& value, simd_int_t02&& other) {
		return static_cast<uint16_t>(_mm_movemask_epi8(_mm_cmpeq_epi8(std::forward<simd_int_t01>(value), std::forward<simd_int_t02>(other))));
	}

	#if JSONIFIER_CHECK_FOR_AVX(JSONIFIER_AVX2)

	template<simd_int_256_type simd_int_t01, simd_int_256_type simd_int_t02> JSONIFIER_INLINE static auto opCmpEq(simd_int_t01&& value, simd_int_t02&& other) {
		return static_cast<uint32_t>(_mm256_movemask_epi8(_mm256_cmpeq_epi8(std::forward<simd_int_t01>(value), std::forward<simd_int_t02>(other))));
	}

		#if JSONIFIER_CHECK_FOR_AVX(JSONIFIER_AVX512)

	template<simd_int_512_type simd_int_t01, simd_int_512_type simd_int_t02> JSONIFIER_INLINE static auto opCmpEq(simd_int_t01&& value, simd_int_t02&& other) {
		return static_cast<uint64_t>(_mm512_cmpeq_epi8_mask(std::forward<simd_int_t01>(value), std::forward<simd_int_t02>(other)));
	}

		#endif

	#endif

#else

	template<simd_int_128_type simd_int_t01, simd_int_128_type simd_int_t02> JSONIFIER_INLINE static uint16_t opCmpEq(simd_int_t01&& value, simd_int_t02&& other) {
		return static_cast<uint16_t>(_mm128_movemask_epi8(_mm128_cmpeq_epi8(std::forward<simd_int_t01>(value), std::forward<simd_int_t02>(other), std::make_index_sequence<16>{}),
			std::make_index_sequence<16>{}));
	}

#endif

}