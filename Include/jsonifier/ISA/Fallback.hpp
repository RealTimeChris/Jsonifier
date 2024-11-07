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

#include <jsonifier/ISA/CTimeSimdTypes.hpp>
#include <jsonifier/ISA/SimdTypes.hpp>
#include <jsonifier/ISA/Bmi.hpp>

namespace simd_internal {

#if !JSONIFIER_CHECK_FOR_INSTRUCTION(JSONIFIER_AVX) && !JSONIFIER_CHECK_FOR_INSTRUCTION(JSONIFIER_AVX2) && !JSONIFIER_CHECK_FOR_INSTRUCTION(JSONIFIER_AVX512) && \
	!JSONIFIER_CHECK_FOR_INSTRUCTION(JSONIFIER_NEON)

	template<typename value_type> JSONIFIER_ALWAYS_INLINE value_type postCmpTzcnt(value_type value) noexcept {
		return tzcnt(value);
	}

	template<simd_int_128_type simd_int_type_new> JSONIFIER_ALWAYS_INLINE simd_int_type_new gatherValues(const void* str) noexcept {
		return mm128LoadUSi128(static_cast<const __m128x*>(str));
	}

	template<simd_int_128_type simd_int_type_new> JSONIFIER_ALWAYS_INLINE simd_int_type_new gatherValuesU(const void* str) noexcept {
		return mm128LoadUSi128(static_cast<const __m128x*>(str));
	}

	template<simd_int_128_type simd_int_type_new, typename char_t>
		requires(sizeof(std::remove_cvref_t<char_t>) == 8)
	JSONIFIER_ALWAYS_INLINE simd_int_type_new gatherValue(char_t str) noexcept {
		return mm128Set1Epi64x(static_cast<int64_t>(str));
	}

	template<simd_int_128_type simd_int_type_new, typename char_t>
		requires(sizeof(std::remove_cvref_t<char_t>) == 1)
	JSONIFIER_ALWAYS_INLINE simd_int_type_new gatherValue(char_t str) noexcept {
		return mm128Set1Epi8(static_cast<char>(str));
	}

	template<simd_int_128_type simd_int_type_new> JSONIFIER_ALWAYS_INLINE void store(const simd_int_type_new& value, void* storageLocation) noexcept {
		mm128StoreUSi128(static_cast<uint64_t*>(storageLocation), value);
	}

	template<simd_int_128_type simd_int_t01, simd_int_128_type simd_int_t02> JSONIFIER_ALWAYS_INLINE auto opCmpEq(const simd_int_t01& value, const simd_int_t02& other) noexcept {
		return static_cast<uint16_t>(mm128MovemaskEpi8(mm128CmpEqEpi8(value, other)));
	}

	template<simd_int_128_type simd_int_t01, simd_int_128_type simd_int_t02> JSONIFIER_ALWAYS_INLINE auto opCmpLt(const simd_int_t01& value, const simd_int_t02& other) noexcept {
		jsonifier_simd_int_128 offset = mm128Set1Epi8(static_cast<char>(0x80));
		return static_cast<uint32_t>(mm128MovemaskEpi8(mm128CmpEqEpi8(mm128AddEpi8(other, offset), mm128AddEpi8(value, offset))));
	}

	template<simd_int_128_type simd_int_t01, simd_int_128_type simd_int_t02>
	JSONIFIER_ALWAYS_INLINE auto opCmpEqRaw(const simd_int_t01& value, const simd_int_t02& other) noexcept {
		return mm128CmpEqEpi8(value, other);
	}

	template<simd_int_128_type simd_int_t01, simd_int_128_type simd_int_t02>
	JSONIFIER_ALWAYS_INLINE auto opCmpLtRaw(const simd_int_t01& value, const simd_int_t02& other) noexcept {
		jsonifier_simd_int_128 offset = mm128Set1Epi8(static_cast<char>(0x80));
		return mm128CmpEqEpi8(mm128AddEpi8(other, offset), mm128AddEpi8(value, offset));
	}

	template<simd_int_128_type simd_int_t01, simd_int_128_type simd_int_t02>
	JSONIFIER_ALWAYS_INLINE auto opCmpEqBitMask(const simd_int_t01& value, const simd_int_t02& other) noexcept {
		return static_cast<uint16_t>(mm128MovemaskEpi8(mm128CmpEqEpi8(value, other)));
	}

	template<simd_int_128_type simd_int_t01> JSONIFIER_ALWAYS_INLINE auto opBitMaskRaw(const simd_int_t01& value) noexcept {
		return mm128MovemaskEpi8(value);
	}

	template<simd_int_128_type simd_int_t01, simd_int_128_type simd_int_t02> JSONIFIER_ALWAYS_INLINE auto opShuffle(const simd_int_t01& value, const simd_int_t02& other) noexcept {
		return mm128ShuffleEpi8(value, other, std::make_index_sequence<16>{});
	}

	template<simd_int_128_type simd_int_t01, simd_int_128_type simd_int_t02> JSONIFIER_ALWAYS_INLINE auto opXor(const simd_int_t01& value, const simd_int_t02& other) noexcept {
		return mm128XorSi128(value, other);
	}

	template<simd_int_128_type simd_int_t01, simd_int_128_type simd_int_t02> JSONIFIER_ALWAYS_INLINE auto opAnd(const simd_int_t01& value, const simd_int_t02& other) noexcept {
		return mm128AndSi128(value, other);
	}

	template<simd_int_128_type simd_int_t01, simd_int_128_type simd_int_t02> JSONIFIER_ALWAYS_INLINE auto opOr(const simd_int_t01& value, const simd_int_t02& other) noexcept {
		return mm128OrSi128(value, other);
	}

	template<simd_int_128_type simd_int_t01, simd_int_128_type simd_int_t02> JSONIFIER_ALWAYS_INLINE auto opAndNot(const simd_int_t01& value, const simd_int_t02& other) noexcept {
		return mm128AndNotSi128(other, value);
	}

	template<simd_int_128_type simd_int_t01> JSONIFIER_ALWAYS_INLINE auto opTest(const simd_int_t01& value) noexcept {
		return !mm128TestzSi128(value, value);
	}

	template<simd_int_128_type simd_int_t01> JSONIFIER_ALWAYS_INLINE auto opNot(const simd_int_t01& value) noexcept {
		return mm128XorSi128(value, mm128Set1Epi64x(0xFFFFFFFFFFFFFFFFll));
	}

	template<simd_int_128_type simd_type> JSONIFIER_ALWAYS_INLINE jsonifier_simd_int_128 opSetLSB(const simd_type& value, bool valueNew) noexcept {
		jsonifier_simd_int_128 mask{ mm128SetEpi64x(0, 0x01u) };
		return valueNew ? mm128OrSi128(value, mask) : mm128AndNotSi128(mask, value);
	}

	template<simd_int_128_type simd_type> JSONIFIER_ALWAYS_INLINE bool opGetMSB(const simd_type& value) noexcept {
		jsonifier_simd_int_128 result = mm128AndSi128(value, mm128SetEpi64x(0x8000000000000000ll, 0x00ll));
		return !mm128TestzSi128(result, result);
	}

#endif

}