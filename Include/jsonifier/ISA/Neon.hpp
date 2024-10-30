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

	template<simd_int_128_type simd_int_t01, simd_int_128_type simd_int_t02> JSONIFIER_ALWAYS_INLINE auto opShuffle(simd_int_t01&& value, simd_int_t02&& other) noexcept {
		static const uint8x16_t mask{ vdupq_n_u8(0x0f) };
		return vqtbl1q_u8(value, vandq_u8(other, mask));
	}

	template<simd_int_128_type simd_int_t01, simd_int_128_type simd_int_t02> JSONIFIER_ALWAYS_INLINE jsonifier_simd_int_128 opAndNot(simd_int_t01&& value, simd_int_t02&& other) {
		return vbicq_u8(value, other);
	}

	template<simd_int_128_type simd_int_t01, simd_int_128_type simd_int_t02> JSONIFIER_ALWAYS_INLINE jsonifier_simd_int_128 opAnd(simd_int_t01&& value, simd_int_t02&& other) {
		return vandq_u8(value, other);
	}

	template<simd_int_128_type simd_int_t01, simd_int_128_type simd_int_t02> JSONIFIER_ALWAYS_INLINE jsonifier_simd_int_128 opOr(simd_int_t01&& value, simd_int_t02&& other) {
		return vorrq_u8(value, other);
	}

	template<simd_int_128_type simd_int_t01, simd_int_128_type simd_int_t02> JSONIFIER_ALWAYS_INLINE jsonifier_simd_int_128 opXor(simd_int_t01&& value, simd_int_t02&& other) {
		return veorq_u8(value, other);
	}

	template<simd_int_128_type simd_int_t01> JSONIFIER_ALWAYS_INLINE bool opTest(simd_int_t01&& value) {
		return vmaxvq_u8(value) != 0;
	}

	template<simd_int_128_type simd_int_t01, simd_int_128_type simd_int_t02> JSONIFIER_ALWAYS_INLINE auto opSub64(simd_int_t01&& value, simd_int_t02&& other) noexcept {
		return vreinterpretq_u8_u64(vsubq_u64(value, other));
	}

	template<simd_int_128_type simd_int_t01, simd_int_128_type simd_int_t02> JSONIFIER_ALWAYS_INLINE auto opAdd8(simd_int_t01&& value, simd_int_t02&& other) noexcept {
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

	template<simd_int_128_type simd_int_type_new, typename char_type> JSONIFIER_ALWAYS_INLINE void store(const simd_int_type_new& value, char_type* storageLocation) noexcept {
		vst1q_u64(storageLocation, vreinterpretq_u64_u8(value));
	}

	template<simd_int_128_type simd_int_type_new, typename char_type>
		requires(sizeof(char_type) == 1)
	JSONIFIER_ALWAYS_INLINE void store(const simd_int_type_new& value, char_type* storageLocation) noexcept {
		vst1q_u8(storageLocation, value);
	}

	template<simd_int_128_type simd_int_t01, simd_int_128_type simd_int_t02> JSONIFIER_ALWAYS_INLINE uint16_t opCmpEq(simd_int_t01&& value, simd_int_t02&& other) noexcept {
		uint8x16_t matchMask = vceqq_u8(value, other);
		return static_cast<uint16_t>(((matchMask[0] & 0x80) >> 7) | ((matchMask[1] & 0x80) >> 6) | ((matchMask[2] & 0x80) >> 5) | ((matchMask[3] & 0x80) >> 4) |
			((matchMask[4] & 0x80) >> 3) | ((matchMask[5] & 0x80) >> 2) | ((matchMask[6] & 0x80) >> 1) | ((matchMask[7] & 0x80) << 0) | ((matchMask[8] & 0x80) << 1) |
			((matchMask[9] & 0x80) << 2) | ((matchMask[10] & 0x80) << 3) | ((matchMask[11] & 0x80) << 4) | ((matchMask[12] & 0x80) << 5) | ((matchMask[13] & 0x80) << 6) |
			((matchMask[14] & 0x80) << 7) | ((matchMask[15] & 0x80) << 8));
	}

	template<simd_int_128_type simd_int_t01, simd_int_128_type simd_int_t02> JSONIFIER_ALWAYS_INLINE uint16_t opCmpLt(simd_int_t01&& value, simd_int_t02&& other) noexcept {
		uint8x16_t matchMask{ vcgtq_u8(other, value) };
		return static_cast<uint16_t>(((matchMask[0] & 0x80) >> 7) | ((matchMask[1] & 0x80) >> 6) | ((matchMask[2] & 0x80) >> 5) | ((matchMask[3] & 0x80) >> 4) |
			((matchMask[4] & 0x80) >> 3) | ((matchMask[5] & 0x80) >> 2) | ((matchMask[6] & 0x80) >> 1) | ((matchMask[7] & 0x80) << 0) | ((matchMask[8] & 0x80) << 1) |
			((matchMask[9] & 0x80) << 2) | ((matchMask[10] & 0x80) << 3) | ((matchMask[11] & 0x80) << 4) | ((matchMask[12] & 0x80) << 5) | ((matchMask[13] & 0x80) << 6) |
			((matchMask[14] & 0x80) << 7) | ((matchMask[15] & 0x80) << 8));
	}

	template<simd_int_128_type simd_int_t01> JSONIFIER_ALWAYS_INLINE auto opNot(simd_int_t01&& value) noexcept {
		return vmvnq_u8(value);
	}

	template<simd_int_128_type simd_int_t01> JSONIFIER_ALWAYS_INLINE jsonifier_simd_int_128 opSetLSB(simd_int_t01&& value, bool valueNew) {
		static constexpr uint8x16_t mask{ 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
		return valueNew ? vorrq_u8(value, mask) : vbicq_u8(value, mask);
	}

	template<simd_int_128_type simd_int_t01> JSONIFIER_ALWAYS_INLINE bool opGetMSB(simd_int_t01&& value) {
		return (vgetq_lane_u8(value, 15) & 0x80) != 0;
	}

#endif

}
