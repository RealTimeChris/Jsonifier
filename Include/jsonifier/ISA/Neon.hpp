/*
	MIT License

	Copyright (c) 2024 RealTimeChris

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

#if defined(JSONIFIER_NEON)

	static uint8x16_t mask{ vdupq_n_u8(0x0f) };

	template<jsonifier::concepts::simd_int_128_type simd_int_t01, jsonifier::concepts::simd_int_128_type simd_int_t02>
	JSONIFIER_ALWAYS_INLINE auto opShuffle(simd_int_t01&& value, simd_int_t02&& other) noexcept {
		return vqtbl1q_u8(value, vandq_u8(other, mask));
	}

	template<jsonifier::concepts::simd_int_128_type simd_int_t01, jsonifier::concepts::simd_int_128_type simd_int_t02>
	JSONIFIER_ALWAYS_INLINE jsonifier_simd_int_128 opAndNot(simd_int_t01&& value, simd_int_t02&& other) noexcept {
		return vbicq_u8(value, other);
	}

	template<jsonifier::concepts::simd_int_128_type simd_int_t01, jsonifier::concepts::simd_int_128_type simd_int_t02>
	JSONIFIER_ALWAYS_INLINE jsonifier_simd_int_128 opAnd(simd_int_t01&& value, simd_int_t02&& other) noexcept {
		return vandq_u8(value, other);
	}

	template<jsonifier::concepts::simd_int_128_type simd_int_t01, jsonifier::concepts::simd_int_128_type simd_int_t02>
	JSONIFIER_ALWAYS_INLINE jsonifier_simd_int_128 opOr(simd_int_t01&& value, simd_int_t02&& other) noexcept {
		return vorrq_u8(value, other);
	}

	template<jsonifier::concepts::simd_int_128_type simd_int_t01, jsonifier::concepts::simd_int_128_type simd_int_t02>
	JSONIFIER_ALWAYS_INLINE jsonifier_simd_int_128 opXor(simd_int_t01&& value, simd_int_t02&& other) noexcept {
		return veorq_u8(value, other);
	}

	template<jsonifier::concepts::simd_int_128_type simd_int_t01, jsonifier::concepts::simd_int_128_type simd_int_t02>
	JSONIFIER_ALWAYS_INLINE bool opTest(simd_int_t01&& value, simd_int_t02&&) noexcept {
		return vmaxvq_u8(value) != 0;
	}

	template<jsonifier::concepts::simd_int_128_type simd_int_t01, jsonifier::concepts::simd_int_128_type simd_int_t02>
	JSONIFIER_ALWAYS_INLINE auto opSub64(simd_int_t01&& value, simd_int_t02&& other) noexcept {
		return vreinterpretq_u8_u64(vsubq_u64(value, other));
	}

	template<jsonifier::concepts::simd_int_128_type simd_int_t01, jsonifier::concepts::simd_int_128_type simd_int_t02>
	JSONIFIER_ALWAYS_INLINE auto opAdd8(simd_int_t01&& value, simd_int_t02&& other) noexcept {
		return vaddq_u8(value, other);
	}

	template<jsonifier::concepts::simd_int_128_type simd_int_type_new, jsonifier::concepts::uint16_type char_type>
	JSONIFIER_ALWAYS_INLINE simd_int_type_new gatherValues(char_type* str) noexcept {
		return vreinterpretq_u8_u16(vld1q_u16(str));
	}

	template<jsonifier::concepts::simd_int_128_type simd_int_type_new, jsonifier::concepts::uint64_type char_type>
	JSONIFIER_ALWAYS_INLINE simd_int_type_new gatherValues(char_type* str) noexcept {
		return vreinterpretq_u8_u64(vld1q_u64(str));
	}

	template<jsonifier::concepts::simd_int_128_type simd_int_type_new> JSONIFIER_ALWAYS_INLINE simd_int_type_new gatherValues(const void* str) noexcept {
		return vld1q_u8(static_cast<const uint8_t*>(str));
	}

	template<jsonifier::concepts::simd_int_128_type simd_int_type_new> JSONIFIER_ALWAYS_INLINE simd_int_type_new gatherValuesU(const void* str) noexcept {
		return vld1q_u8(static_cast<const uint8_t*>(str));
	}

	template<jsonifier::concepts::simd_int_128_type simd_int_type_new, jsonifier::concepts::uint64_type char_type>
	JSONIFIER_ALWAYS_INLINE simd_int_type_new gatherValue(char_type str) noexcept {
		return vdupq_n_u64(str);
	}

	template<jsonifier::concepts::simd_int_128_type simd_int_type_new, typename char_type>
		requires(sizeof(char_type) == 1)
	JSONIFIER_ALWAYS_INLINE simd_int_type_new gatherValue(char_type str) noexcept {
		return vdupq_n_u8(str);
	}

	template<jsonifier::concepts::simd_int_128_type simd_int_type_new, typename char_type>
	JSONIFIER_ALWAYS_INLINE void store(const simd_int_type_new& value, char_type* storageLocation) noexcept {
		vst1q_u64(storageLocation, vreinterpretq_u64_u8(value));
	}

	template<jsonifier::concepts::simd_int_128_type simd_int_type_new, jsonifier::concepts::uint8_type char_type>
	JSONIFIER_ALWAYS_INLINE void store(const simd_int_type_new& value, char_type* storageLocation) noexcept {
		vst1q_u8(storageLocation, value);
	}

	static constexpr uint8x16_t bitMask{ 0x01, 0x02, 0x4, 0x8, 0x10, 0x20, 0x40, 0x80, 0x01, 0x02, 0x4, 0x8, 0x10, 0x20, 0x40, 0x80 };

	template<jsonifier::concepts::simd_int_128_type simd_int_t01> JSONIFIER_ALWAYS_INLINE uint64_t opBitMask(simd_int_t01&& value) noexcept {
		uint8x16_t input	 = value;
		uint16x8_t high_bits = vreinterpretq_u16_u8(vshrq_n_u8(input, 7));
		uint32x4_t paired16	 = vreinterpretq_u32_u16(vsraq_n_u16(high_bits, high_bits, 7));
		uint64x2_t paired32	 = vreinterpretq_u64_u32(vsraq_n_u32(paired16, paired16, 14));
		uint8x16_t paired64	 = vreinterpretq_u8_u64(vsraq_n_u64(paired32, paired32, 28));
		return vgetq_lane_u8(paired64, 0) | (( int32_t )vgetq_lane_u8(paired64, 8) << 8);
	}

	template<jsonifier::concepts::simd_int_128_type simd_int_t01, jsonifier::concepts::simd_int_128_type simd_int_t02>
	JSONIFIER_ALWAYS_INLINE uint16_t opCmpEq(simd_int_t01&& value, simd_int_t02&& other) noexcept {
		return opBitMask(vceqq_u8(value, other));
	}

	template<jsonifier::concepts::simd_int_128_type simd_int_t01, jsonifier::concepts::simd_int_128_type simd_int_t02>
	JSONIFIER_ALWAYS_INLINE uint16_t opCmpLt(simd_int_t01&& value, simd_int_t02&& other) noexcept {
		return opBitMask(vcgtq_u8(other, value));
	}

	#define opNot(x) vmvnq_u8(x)

	template<jsonifier::concepts::simd_int_128_type simd_int_t01> JSONIFIER_ALWAYS_INLINE jsonifier_simd_int_128 opSetLSB(simd_int_t01&& value, bool valueNew) noexcept {
		constexpr uint8x16_t mask{ 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
		return valueNew ? vorrq_u8(value, mask) : vbicq_u8(value, mask);
	}

	template<jsonifier::concepts::simd_int_128_type simd_int_t01> JSONIFIER_ALWAYS_INLINE bool opGetMSB(simd_int_t01&& value) noexcept {
		return (vgetq_lane_u8(value, 15) & 0x80) != 0;
	}

#endif

}