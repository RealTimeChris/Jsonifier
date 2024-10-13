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

#include <jsonifier/ISA/SimdTypes.hpp>
#include <jsonifier/ISA/Bmi.hpp>

namespace simd_internal {

#if JSONIFIER_CHECK_FOR_INSTRUCTION(JSONIFIER_NEON)

	template<typename value_type> JSONIFIER_ALWAYS_INLINE value_type postCmpTzcnt(value_type value) noexcept {
		return tzcnt(value) >> 2;
	}

	template<simd_int_128_type simd_int_type_new> JSONIFIER_ALWAYS_INLINE simd_int_type_new gatherValues(const void* str) noexcept {
		return vld1q_u8(static_cast<const uint8_t*>(str));
	}

	template<simd_int_128_type simd_int_type_new> JSONIFIER_ALWAYS_INLINE simd_int_type_new gatherValuesU(const void* str) noexcept {
		return vld1q_u8(static_cast<const uint8_t*>(str));
	}

	template<simd_int_128_type simd_int_type_new, typename char_t>
		requires(sizeof(char_t) == 8)
	JSONIFIER_ALWAYS_INLINE simd_int_type_new gatherValue(char_t str) noexcept {
		return vdupq_n_u64(str);
	}

	template<simd_int_128_type simd_int_type_new, typename char_t>
		requires(sizeof(char_t) == 1)
	JSONIFIER_ALWAYS_INLINE simd_int_type_new gatherValue(char_t str) noexcept {
		return vdupq_n_u8(str);
	}

	template<simd_int_128_type simd_int_type_new, typename char_t> JSONIFIER_ALWAYS_INLINE void store(const simd_int_type_new& value, char_t* storageLocation) noexcept {
		vst1q_u64(storageLocation, vreinterpretq_u64_u8(value));
	}

	template<simd_int_128_type simd_int_type_new, typename char_t>
		requires(sizeof(char_t) == 1)
	JSONIFIER_ALWAYS_INLINE void store(const simd_int_type_new& value, char_t* storageLocation) noexcept {
		vst1q_u8(storageLocation, value);
	}

	template<simd_int_128_type simd_int_t01, simd_int_128_type simd_int_t02> JSONIFIER_ALWAYS_INLINE auto opCmpEq(const simd_int_t01& value, const simd_int_t02& other) noexcept {
		return vget_lane_u64(vreinterpret_u64_u8(vshrn_n_u16(vceqq_u8(value, other), 4)), 0);
	}

	template<simd_int_128_type simd_int_t01, simd_int_128_type simd_int_t02> JSONIFIER_ALWAYS_INLINE auto opCmpLt(const simd_int_t01& value, const simd_int_t02& other) noexcept {
		return vget_lane_u64(vreinterpret_u64_u8(vshrn_n_u16(vcgtq_u8(other, value), 4)), 0);
	}

	template<simd_int_128_type simd_int_t01, simd_int_128_type simd_int_t02>
	JSONIFIER_ALWAYS_INLINE auto opCmpEqRaw(const simd_int_t01& value, const simd_int_t02& other) noexcept {
		return vceqq_u8(value, other);
	}

	template<simd_int_128_type simd_int_t01, simd_int_128_type simd_int_t02>
	JSONIFIER_ALWAYS_INLINE auto opCmpLtRaw(const simd_int_t01& value, const simd_int_t02& other) noexcept {
		return vcgtq_u8(other, value);
	}

	template<simd_int_128_type simd_int_t01> JSONIFIER_ALWAYS_INLINE uint64_t opBitMaskRaw(const simd_int_t01& value) noexcept {
		return vget_lane_u64(vreinterpret_u64_u8(vshrn_n_u16(value, 4)), 0);
	}

	template<simd_int_128_type simd_int_t01> JSONIFIER_ALWAYS_INLINE uint64_t opBitMask(const simd_int_t01& value) noexcept {
		uint16x8_t highBits = vreinterpretq_u16_u8(vshrq_n_u8(value, 7));
		uint32x4_t paired16 = vreinterpretq_u32_u16(vsraq_n_u16(highBits, highBits, 7));
		uint64x2_t paired32 = vreinterpretq_u64_u32(vsraq_n_u32(paired16, paired16, 14));
		uint8x16_t paired64 = vreinterpretq_u8_u64(vsraq_n_u64(paired32, paired32, 28));
		return vgetq_lane_u8(paired64, 0) | (( int32_t )vgetq_lane_u8(paired64, 8) << 8);
	}

	template<simd_int_128_type simd_int_t01, simd_int_128_type simd_int_t02>
	JSONIFIER_ALWAYS_INLINE auto opCmpEqBitMask(const simd_int_t01& value, const simd_int_t02& other) noexcept {
		return opBitMask(vceqq_u8(value, other));
	}

	template<simd_int_128_type simd_int_t01, simd_int_128_type simd_int_t02> JSONIFIER_ALWAYS_INLINE auto opShuffle(const simd_int_t01& value, const simd_int_t02& other) noexcept {
		auto bitMask{ vdupq_n_u8(0x0F) };
		return vqtbl1q_u8(value, vandq_u8(other, bitMask));
	}

	template<simd_int_128_type simd_int_t01, simd_int_128_type simd_int_t02>
	JSONIFIER_ALWAYS_INLINE jsonifier_simd_int_128 opXor(const simd_int_t01& value, const simd_int_t02& other) noexcept {
		return veorq_u8(value, other);
	}

	template<simd_int_128_type simd_int_t01, simd_int_128_type simd_int_t02>
	JSONIFIER_ALWAYS_INLINE jsonifier_simd_int_128 opAnd(const simd_int_t01& value, const simd_int_t02& other) noexcept {
		return vandq_u8(value, other);
	}

	template<simd_int_128_type simd_int_t01, simd_int_128_type simd_int_t02>
	JSONIFIER_ALWAYS_INLINE jsonifier_simd_int_128 opOr(const simd_int_t01& value, const simd_int_t02& other) noexcept {
		return vorrq_u8(value, other);
	}

	template<simd_int_128_type simd_int_t01, simd_int_128_type simd_int_t02>
	JSONIFIER_ALWAYS_INLINE jsonifier_simd_int_128 opAndNot(const simd_int_t01& value, const simd_int_t02& other) noexcept {
		return vbicq_u8(value, other);
	}

	template<simd_int_128_type simd_int_t01> JSONIFIER_ALWAYS_INLINE bool opTest(const simd_int_t01& value) noexcept {
		return vmaxvq_u8(value) != 0;
	}

	template<simd_int_128_type simd_int_t01> JSONIFIER_ALWAYS_INLINE auto opNot(const simd_int_t01& value) noexcept {
		return vmvnq_u8(value);
	}

	template<simd_int_128_type simd_int_t01> JSONIFIER_ALWAYS_INLINE jsonifier_simd_int_128 opSetLSB(const simd_int_t01& value, bool valueNew) noexcept {
		uint8x16_t mask{ 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
		return valueNew ? vorrq_u8(value, mask) : vbicq_u8(value, mask);
	}

	template<simd_int_128_type simd_int_t01> JSONIFIER_ALWAYS_INLINE bool opGetMSB(const simd_int_t01& value) noexcept {
		return (vgetq_lane_u8(value, 15) & 0x80) != 0;
	}

#endif

}
