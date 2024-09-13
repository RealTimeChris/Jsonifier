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

	template<jsonifier::concepts::simd_int_128_type simd_int_type_new, typename char_type> JSONIFIER_ALWAYS_INLINE simd_int_type_new gatherValues(char_type* str) noexcept {
		return vld1q_u8(reinterpret_cast<const uint8_t*>(str));
	}

	template<jsonifier::concepts::simd_int_128_type simd_int_type_new, typename char_type> JSONIFIER_ALWAYS_INLINE simd_int_type_new gatherValuesU(char_type* str) noexcept {
		return vld1q_u8(reinterpret_cast<const uint8_t*>(str));// NEON supports unaligned loads
	}

	template<jsonifier::concepts::simd_int_128_type simd_int_type_new, jsonifier::concepts::uint64_type char_type>
	JSONIFIER_ALWAYS_INLINE simd_int_type_new gatherValue(char_type str) noexcept {
		return vdupq_n_u64(str);
	}

	template<jsonifier::concepts::simd_int_128_type simd_int_type_new, typename char_type>
		requires(sizeof(char_type) == 1)
	JSONIFIER_ALWAYS_INLINE simd_int_type_new gatherValue(char_type str) noexcept {
		return vdupq_n_u8(static_cast<uint8_t>(str));
	}

	template<jsonifier::concepts::simd_int_128_type simd_int_type_new, typename char_type>
	JSONIFIER_ALWAYS_INLINE void store(simd_int_type_new&& value, char_type* storageLocation) noexcept {
		vst1q_u8(reinterpret_cast<uint8_t*>(storageLocation), std::forward<simd_int_type_new>(value));
	}

	template<jsonifier::concepts::simd_int_128_type simd_int_t01, jsonifier::concepts::simd_int_128_type simd_int_t02>
	JSONIFIER_ALWAYS_INLINE auto opCmpEq(simd_int_t01&& value, simd_int_t02&& other) noexcept {
		return static_cast<uint16_t>(vmaxvq_u8(vceqq_u8(std::forward<simd_int_t01>(value), std::forward<simd_int_t02>(other))));
	}

	template<jsonifier::concepts::simd_int_128_type simd_int_t01> JSONIFIER_ALWAYS_INLINE auto opBitMask(simd_int_t01&& value) noexcept {
		return static_cast<uint16_t>(vmaxvq_u8(std::forward<simd_int_t01>(value)));
	}

	template<jsonifier::concepts::simd_int_128_type simd_int_t01, jsonifier::concepts::simd_int_128_type simd_int_t02>
	JSONIFIER_ALWAYS_INLINE auto opCmpEqRaw(simd_int_t01&& value, simd_int_t02&& other) noexcept {
		return vceqq_u8(std::forward<simd_int_t01>(value), std::forward<simd_int_t02>(other));
	}

	template<jsonifier::concepts::simd_int_128_type simd_int_t01, jsonifier::concepts::simd_int_128_type simd_int_t02>
	JSONIFIER_ALWAYS_INLINE auto opShuffle(simd_int_t01&& value, simd_int_t02&& other) noexcept {
		return vqtbl1q_u8(std::forward<simd_int_t01>(value), std::forward<simd_int_t02>(other));
	}

	template<jsonifier::concepts::simd_int_128_type simd_int_t01, jsonifier::concepts::simd_int_128_type simd_int_t02>
	JSONIFIER_ALWAYS_INLINE auto opXor(simd_int_t01&& value, simd_int_t02&& other) noexcept {
		return veorq_u8(std::forward<simd_int_t01>(value), std::forward<simd_int_t02>(other));
	}

	template<jsonifier::concepts::simd_int_128_type simd_int_t01, jsonifier::concepts::simd_int_128_type simd_int_t02>
	JSONIFIER_ALWAYS_INLINE auto opSub64(simd_int_t01&& value, simd_int_t02&& other) noexcept {
		return vsubq_u64(std::forward<simd_int_t01>(value), std::forward<simd_int_t02>(other));
	}

	template<jsonifier::concepts::simd_int_128_type simd_int_t01, jsonifier::concepts::simd_int_128_type simd_int_t02>
	JSONIFIER_ALWAYS_INLINE auto opAnd(simd_int_t01&& value, simd_int_t02&& other) noexcept {
		return vandq_u8(std::forward<simd_int_t01>(value), std::forward<simd_int_t02>(other));
	}

	template<jsonifier::concepts::simd_int_128_type simd_int_t01, jsonifier::concepts::simd_int_128_type simd_int_t02>
	JSONIFIER_ALWAYS_INLINE auto opAndNot(simd_int_t01&& value, simd_int_t02&& other) noexcept {
		return vbicq_u8(std::forward<simd_int_t01>(other), std::forward<simd_int_t02>(value));
	}

	template<jsonifier::concepts::simd_int_128_type simd_int_t01, jsonifier::concepts::simd_int_128_type simd_int_t02>
	JSONIFIER_ALWAYS_INLINE auto opTest(simd_int_t01&& value, simd_int_t02&& other) noexcept {
		return vmaxvq_u8(vandq_u8(std::forward<simd_int_t01>(value), std::forward<simd_int_t02>(other))) != 0;
	}

	template<jsonifier::concepts::simd_int_128_type simd_type> JSONIFIER_ALWAYS_INLINE simd_type opSetLSB(simd_type&& value, bool valueNew) noexcept {
		uint8x16_t mask = vdupq_n_u8(0x01);
		return valueNew ? vorrq_u8(value, mask) : vbicq_u8(value, mask);
	}

	template<jsonifier::concepts::simd_int_128_type simd_type> JSONIFIER_ALWAYS_INLINE bool opGetMSB(simd_type&& value) noexcept {
		auto result = vandq_u64(vreinterpretq_u64_u8(value), vdupq_n_u64(0x8000000000000000ULL));
		return vmaxvq_u64(result) != 0;
	}

	#if defined(opOr)
		#undef opOr
	#endif
	#define opOr(x, y) vorrq_u8(x, y)

	#if defined(opNot)
		#undef opNot
	#endif
	#define opNot(x) veorq_u8(x, vdupq_n_u8(0xFF))

	#if defined(opBool)
		#undef opBool
	#endif
	#define opBool(value) vmaxvq_u8(value) != 0


#endif

}