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

	static uint8x16_t mask{ vdupq_n_u8(0x0f) };

	template<jsonifier::concepts::simd_int_128_type simd_int_t01, jsonifier::concepts::simd_int_128_type simd_int_t02>
	JSONIFIER_ALWAYS_INLINE auto opShuffle(simd_int_t01&& value, simd_int_t02&& other) noexcept {
		return vqtbl1q_u8(std::forward<simd_int_t01>(value), vandq_u8(std::forward<simd_int_t02>(other), mask));
	}

	template<jsonifier::concepts::simd_int_128_type simd_int_type_new, jsonifier::concepts::uint16_type char_type>
	JSONIFIER_ALWAYS_INLINE simd_int_type_new gatherValues(char_type* str) noexcept {
		return vreinterpretq_u8_u16(vld1q_u16(str));
	}

	template<jsonifier::concepts::simd_int_128_type simd_int_type_new, jsonifier::concepts::uint64_type char_type>
	JSONIFIER_ALWAYS_INLINE simd_int_type_new gatherValues(char_type* str) noexcept {
		return vreinterpretq_u8_u64(vld1q_u64(str));
	}

	template<jsonifier::concepts::simd_int_128_type simd_int_t01, jsonifier::concepts::simd_int_128_type simd_int_t02>
	JSONIFIER_ALWAYS_INLINE bool opTest(simd_int_t01&& value, simd_int_t02&& other) noexcept {
		uint8x16_t andResult = vandq_u8(std::forward<simd_int_t01>(value), std::forward<simd_int_t02>(other));
		uint8x16_t zeroVec = vdupq_n_u8(0);
		uint8x16_t cmpResult = vceqq_u8(andResult, zeroVec);
		return vmaxvq_u8(cmpResult) == 0;
	}

	template<jsonifier::concepts::simd_int_128_type simd_int_t01, jsonifier::concepts::simd_int_128_type simd_int_t02>
	JSONIFIER_ALWAYS_INLINE auto opCmpEqRaw(simd_int_t01&& value, simd_int_t02&& other) noexcept {
		return vceqq_u8(std::forward<simd_int_t01>(value), std::forward<simd_int_t02>(other));
	}

	template<jsonifier::concepts::simd_int_128_type simd_int_type_new> JSONIFIER_ALWAYS_INLINE simd_int_type_new gatherValues(const void* str) noexcept {
		return vld1q_u8(static_cast<const uint8_t*>(str));
	}

	template<jsonifier::concepts::simd_int_128_type simd_int_type_new> JSONIFIER_ALWAYS_INLINE simd_int_type_new gatherValuesU(const void* str) noexcept {
		return vld1q_u8(static_cast<const uint8_t*>(str));
	}

	template<jsonifier::concepts::simd_int_128_type simd_int_type_new, typename char_type> JSONIFIER_ALWAYS_INLINE simd_int_type_new gatherValue(char_type str) noexcept {
		return vdupq_n_u8(str);
	}

	template<jsonifier::concepts::simd_int_128_type simd_int_type_new, typename char_type>
	JSONIFIER_ALWAYS_INLINE void store(simd_int_type_new&& value, char_type* storageLocation) noexcept {
		vst1q_u64(storageLocation, vreinterpretq_u64_u8(std::forward<simd_int_type_new>(value)));
	}

	template<jsonifier::concepts::simd_int_128_type simd_int_type_new, jsonifier::concepts::uint8_type char_type>
	JSONIFIER_ALWAYS_INLINE void store(simd_int_type_new&& value, char_type* storageLocation) noexcept {
		vst1q_u8(storageLocation, std::forward<simd_int_type_new>(value));
	}

	static constexpr uint8x16_t bitMask{ 0x01, 0x02, 0x4, 0x8, 0x10, 0x20, 0x40, 0x80, 0x01, 0x02, 0x4, 0x8, 0x10, 0x20, 0x40, 0x80 };

	template<jsonifier::concepts::simd_int_128_type simd_int_t01> JSONIFIER_ALWAYS_INLINE uint16_t opBitMask(simd_int_t01&& value) noexcept {
		auto minput	   = value & bitMask;
		uint8x16_t tmp = vpaddq_u8(minput, minput);
		tmp			   = vpaddq_u8(tmp, tmp);
		tmp			   = vpaddq_u8(tmp, tmp);
		return static_cast<uint16_t>(vgetq_lane_u16(vreinterpretq_u16_u8(tmp), 0));
	}

	template<jsonifier::concepts::simd_int_128_type simd_int_t01, jsonifier::concepts::simd_int_128_type simd_int_t02>
	JSONIFIER_ALWAYS_INLINE uint16_t opCmpEq(simd_int_t01&& value, simd_int_t02&& other) noexcept {
		return opBitMask(vceqq_u8(std::forward<simd_int_t01>(value), std::forward<simd_int_t02>(other)));
	}

	#define opAndNot(x, y) vbicq_u8(x, y)

	#define opAnd(x, y) vandq_u8(x, y)

	#define opOr(x, y) vorrq_u8(x, y)

	#define opXor(x, y) veorq_u8(x, y)

	#define opNot(x) vmvnq_u8(x)

	template<jsonifier::concepts::simd_int_128_type simd_type> JSONIFIER_ALWAYS_INLINE jsonifier_simd_int_128 opSetLSB(simd_type&& value, bool valueNew) noexcept {
		JSONIFIER_ALIGN uint8_t values[bytesPerStep]{ 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
		uint8x16_t mask02{ vld1q_u8(values) };
		return valueNew ? vorrq_u8(value, mask02) : vbicq_u8(value, mask02);
	}

	template<jsonifier::concepts::simd_int_128_type simd_type> JSONIFIER_ALWAYS_INLINE bool opGetMSB(simd_type&& value) noexcept {
		return (vgetq_lane_u8(value, 15) & 0x80) != 0;
	}

	#define opBool(value) (vmaxvq_u8(value) != 0)

#endif

}