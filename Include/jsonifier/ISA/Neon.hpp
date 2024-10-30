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

namespace simd_internal {

	#if JSONIFIER_CHECK_FOR_INSTRUCTION(JSONIFIER_NEON)

	static uint8x16_t mask{ vdupq_n_u8(0x0f) };

	template<simd_int_128_type simd_int_t01, simd_int_128_type simd_int_t02>
	JSONIFIER_ALWAYS_INLINE auto opShuffle(simd_int_t01&& value, simd_int_t02&& other) noexcept {
		return vqtbl1q_u8(value, vandq_u8(other, mask));
	}

	template<simd_int_128_type simd_int_t01, simd_int_128_type simd_int_t02>
	JSONIFIER_ALWAYS_INLINE jsonifier_simd_int_128 opAndNot(simd_int_t01&& value, simd_int_t02&& other) {
		return vbicq_u8(value, other);
	}

	template<simd_int_128_type simd_int_t01, simd_int_128_type simd_int_t02>
	JSONIFIER_ALWAYS_INLINE jsonifier_simd_int_128 opAnd(simd_int_t01&& value, simd_int_t02&& other) {
		return vandq_u8(value, other);
	}

	template<simd_int_128_type simd_int_t01, simd_int_128_type simd_int_t02>
	JSONIFIER_ALWAYS_INLINE jsonifier_simd_int_128 opOr(simd_int_t01&& value, simd_int_t02&& other) {
		return vorrq_u8(value, other);
	}

	template<simd_int_128_type simd_int_t01, simd_int_128_type simd_int_t02>
	JSONIFIER_ALWAYS_INLINE jsonifier_simd_int_128 opXor(simd_int_t01&& value, simd_int_t02&& other) {
		return veorq_u8(value, other);
	}

	template<simd_int_128_type simd_int_t01> JSONIFIER_ALWAYS_INLINE bool opTest(simd_int_t01&& value) {
		return vmaxvq_u8(value) != 0;
	}

	template<simd_int_128_type simd_int_t01, simd_int_128_type simd_int_t02>
	JSONIFIER_ALWAYS_INLINE auto opSub64(simd_int_t01&& value, simd_int_t02&& other) noexcept {
		return vreinterpretq_u8_u64(vsubq_u64(value, other));
	}

	template<simd_int_128_type simd_int_t01, simd_int_128_type simd_int_t02>
	JSONIFIER_ALWAYS_INLINE auto opAdd8(simd_int_t01&& value, simd_int_t02&& other) noexcept {
		return vaddq_u8(value, other);
	}

	template<simd_int_128_type simd_int_type_new> JSONIFIER_ALWAYS_INLINE simd_int_type_new gatherValues(const void* str) noexcept {
		return vld1q_u8(static_cast<const uint8_t*>(str));
	}

	template<simd_int_128_type simd_int_type_new> JSONIFIER_ALWAYS_INLINE simd_int_type_new gatherValuesU(const void* str) noexcept {
		return vld1q_u8(static_cast<const uint8_t*>(str));
	}

	template<simd_int_128_type simd_int_type_new, typename char_type>
		requires(sizeof(char_type) == 8)
	JSONIFIER_ALWAYS_INLINE simd_int_type_new gatherValue(char_type str) noexcept {
		return vdupq_n_u64(str);
	}

	template<simd_int_128_type simd_int_type_new, typename char_type>
		requires(sizeof(char_type) == 1)
	JSONIFIER_ALWAYS_INLINE simd_int_type_new gatherValue(char_type str) noexcept {
		return vdupq_n_u8(str);
	}

	template<simd_int_128_type simd_int_type_new, typename char_type>
	JSONIFIER_ALWAYS_INLINE void store(const simd_int_type_new& value, char_type* storageLocation) noexcept {
		vst1q_u64(storageLocation, vreinterpretq_u64_u8(value));
	}

	template<simd_int_128_type simd_int_type_new, typename char_type>
		requires(sizeof(char_type) == 1)
	JSONIFIER_ALWAYS_INLINE void store(const simd_int_type_new& value, char_type* storageLocation) noexcept {
		vst1q_u8(storageLocation, value);
	}

	static constexpr uint8x16_t bitMask{ 0x01, 0x02, 0x4, 0x8, 0x10, 0x20, 0x40, 0x80, 0x01, 0x02, 0x4, 0x8, 0x10, 0x20, 0x40, 0x80 };

	JSONIFIER_ALWAYS_INLINE uint16_t packBitsFromNibbles(uint64_t nibbles) {
		return static_cast<uint16_t>((nibbles & 0x01) | ((nibbles & 0x10) >> 3) | ((nibbles & 0x100) >> 6) | ((nibbles & 0x1000) >> 9) | ((nibbles & 0x10000) >> 12) |
			((nibbles & 0x100000) >> 15) | ((nibbles & 0x1000000) >> 18) | ((nibbles & 0x10000000) >> 21) | ((nibbles & 0x100000000) >> 24) | ((nibbles & 0x1000000000) >> 27) |
			((nibbles & 0x10000000000) >> 30) | ((nibbles & 0x100000000000) >> 33) | ((nibbles & 0x1000000000000) >> 36) | ((nibbles & 0x10000000000000) >> 39) |
			((nibbles & 0x100000000000000) >> 42) | ((nibbles & 0x1000000000000000) >> 45));
	}

	template<simd_int_128_type simd_int_t01> JSONIFIER_ALWAYS_INLINE uint16_t opBitMask(simd_int_t01&& value) noexcept {
		static const uint8x16_t idx	  = { 0, 8, 1, 9, 2, 10, 3, 11, 4, 12, 5, 13, 6, 14, 7, 15 };
		static const uint16x8_t mbits = { 0x0101, 0x0202, 0x0404, 0x0808, 0x1010, 0x2020, 0x4040, 0x8080 };
		const uint8x16_t perm		  = vqtbl1q_u8(value, idx);
		return vaddvq_u16(vandq_u16(vreinterpretq_u16_u8(perm), mbits));
	}

	template<simd_int_128_type simd_int_t01, simd_int_128_type simd_int_t02>
	JSONIFIER_ALWAYS_INLINE uint16_t opCmpEq(simd_int_t01&& value, simd_int_t02&& other) noexcept {
		return opBitMask(vceqq_u8(value, other));
	}

	template<simd_int_128_type simd_int_t01, simd_int_128_type simd_int_t02>
	JSONIFIER_ALWAYS_INLINE uint16_t opCmpLt(simd_int_t01&& value, simd_int_t02&& other) noexcept {
		return opBitMask(vcgtq_u8(other, value));
	}

	template<simd_int_128_type simd_int_t01> JSONIFIER_ALWAYS_INLINE auto opNot(simd_int_t01&& value) noexcept {
		return vmvnq_u8(value);
	}

	template<simd_int_128_type simd_int_t01> JSONIFIER_ALWAYS_INLINE jsonifier_simd_int_128 opSetLSB(simd_int_t01&& value, bool valueNew) {
		constexpr uint8x16_t mask{ 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
		return valueNew ? vorrq_u8(value, mask) : vbicq_u8(value, mask);
	}

	template<simd_int_128_type simd_int_t01> JSONIFIER_ALWAYS_INLINE bool opGetMSB(simd_int_t01&& value) {
		return (vgetq_lane_u8(value, 15) & 0x80) != 0;
	}

	#endif

}

