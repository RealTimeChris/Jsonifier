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

#if JSONIFIER_CHECK_FOR_INSTRUCTION(JSONIFIER_NEON)

	template<jsonifier::concepts::simd_int_128_type simd_int_t01, jsonifier::concepts::simd_int_128_type simd_int_t02>
	JSONIFIER_INLINE simd_int_128 opAndNot(simd_int_t01&& value, simd_int_t02&& other) {
		return vbicq_u8(std::forward<simd_int_t01>(value), std::forward<simd_int_t02>(other));
	}

	template<jsonifier::concepts::simd_int_128_type simd_int_t01, jsonifier::concepts::simd_int_128_type simd_int_t02>
	JSONIFIER_INLINE simd_int_128 opAnd(simd_int_t01&& value, simd_int_t02&& other) {
		return vandq_u8(std::forward<simd_int_t01>(value), std::forward<simd_int_t02>(other));
	}

	template<jsonifier::concepts::simd_int_128_type simd_int_t01, jsonifier::concepts::simd_int_128_type simd_int_t02>
	JSONIFIER_INLINE simd_int_128 opXor(simd_int_t01&& value, simd_int_t02&& other) {
		return veorq_u8(std::forward<simd_int_t01>(value), std::forward<simd_int_t02>(other));
	}

	template<jsonifier::concepts::simd_int_128_type simd_int_t01, jsonifier::concepts::simd_int_128_type simd_int_t02>
	JSONIFIER_INLINE simd_int_128 opOr(simd_int_t01&& value, simd_int_t02&& other) {
		return vorrq_u8(std::forward<simd_int_t01>(value), std::forward<simd_int_t02>(other));
	}

	template<jsonifier::concepts::simd_int_128_type simd_int_t01, jsonifier::concepts::simd_int_128_type simd_int_t02>
	JSONIFIER_INLINE simd_int_128 opAdd(simd_int_t01&& value, simd_int_t02&& other) {
		uint64x2_t a64 = vreinterpretq_u64_u8(value);
		uint64x2_t b64 = vreinterpretq_u64_u8(other);
		uint64x2_t result64 = vaddq_u64(a64, b64);
		return vreinterpretq_u8_u64(result64);
	}

	template<jsonifier::concepts::simd_int_128_type simd_int_t01> JSONIFIER_INLINE simd_int_128 opSetLSB(simd_int_t01&& value, bool valueNew) {
		constexpr uint8x16_t mask{ 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
		return valueNew ? vorrq_u8(std::forward<simd_int_t01>(value), mask) : vbicq_u8(std::forward<simd_int_t01>(value), mask);
	}

	template<int32_t other, jsonifier::concepts::simd_int_128_type simd_int_t01> JSONIFIER_INLINE simd_int_128 opSlli(simd_int_t01&& value) {
		uint64x2_t a64		= vreinterpretq_u64_u8(value);
		uint64x2_t result64 = vshlq_n_u64(a64, other);
		return vreinterpretq_u8_u64(result64);
	}

	template<int32_t other, jsonifier::concepts::simd_int_128_type simd_int_t01> JSONIFIER_INLINE simd_int_128 opSrli(simd_int_t01&& value) {
		uint64x2_t a64 = vreinterpretq_u64_u8(value);
		uint64x2_t result64 = vshrq_n_u64(a64, other);
		return vreinterpretq_u8_u64(result64);
	}

	template<jsonifier::concepts::simd_int_128_type simd_int_t01, jsonifier::concepts::simd_int_128_type simd_int_t02>
	JSONIFIER_INLINE simd_int_128 opMul(simd_int_t01&& value, simd_int_t02&& other) {
		uint32x4_t a32_0 = vreinterpretq_u32_u8(value);
		uint32x4_t b32_0 = vreinterpretq_u32_u8(other);
		uint32x4_t result32 = vmulq_u32(a32_0, b32_0);
		return vreinterpretq_u8_u32(result32);
	}

	template<jsonifier::concepts::simd_int_128_type simd_int_t01> JSONIFIER_INLINE simd_int_128 opNot(simd_int_t01&& value) {
		return vmvnq_u8(std::forward<simd_int_t01>(value));
	}

	template<jsonifier::concepts::simd_int_128_type simd_int_t01> JSONIFIER_INLINE bool opGetMSB(simd_int_t01&& value) {
		return (vgetq_lane_u8(value, 15) & 0x80) != 0;
	}

	template<jsonifier::concepts::simd_int_128_type simd_int_t01> JSONIFIER_INLINE bool opBool(simd_int_t01&& value) {
		return vmaxvq_u8(value) != 0;
	}

	template<jsonifier::concepts::simd_int_128_type simd_int_t01> JSONIFIER_INLINE simd_int_t01 reset() {
		return {};
	}

#endif

}