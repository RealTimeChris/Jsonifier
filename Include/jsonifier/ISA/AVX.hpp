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

#if JSONIFIER_CHECK_FOR_INSTRUCTION(JSONIFIER_AVX512) || JSONIFIER_CHECK_FOR_INSTRUCTION(JSONIFIER_AVX2) || JSONIFIER_CHECK_FOR_INSTRUCTION(JSONIFIER_AVX)

	template<typename value_type> JSONIFIER_ALWAYS_INLINE value_type postCmpTzcnt(value_type value) noexcept {
		return tzcnt(value);
	}

	template<simd_int_128_type simd_int_type_new> JSONIFIER_ALWAYS_INLINE simd_int_type_new gatherValues(const void* str) noexcept {
		return _mm_load_si128(static_cast<const __m128i*>(str));
	}

	template<simd_int_128_type simd_int_type_new> JSONIFIER_ALWAYS_INLINE simd_int_type_new gatherValuesU(const void* str) noexcept {
		return _mm_loadu_si128(static_cast<const __m128i*>(str));
	}

	template<simd_int_128_type simd_int_type_new, typename char_t>
		requires(sizeof(std::remove_cvref_t<char_t>) == 8)
	JSONIFIER_ALWAYS_INLINE simd_int_type_new gatherValue(char_t str) noexcept {
		return _mm_set1_epi64x(static_cast<int64_t>(str));
	}

	template<simd_int_128_type simd_int_type_new, typename char_t>
		requires(sizeof(std::remove_cvref_t<char_t>) == 1)
	JSONIFIER_ALWAYS_INLINE simd_int_type_new gatherValue(char_t str) noexcept {
		return _mm_set1_epi8(static_cast<char>(str));
	}

	template<simd_int_128_type simd_int_type_new> JSONIFIER_ALWAYS_INLINE void store(const simd_int_type_new& value, void* storageLocation) noexcept {
		_mm_store_si128(static_cast<__m128i*>(storageLocation), value);
	}

	template<simd_int_128_type simd_int_t01, simd_int_128_type simd_int_t02> JSONIFIER_ALWAYS_INLINE auto opCmpEq(const simd_int_t01& value, const simd_int_t02& other) noexcept {
		return static_cast<uint16_t>(_mm_movemask_epi8(_mm_cmpeq_epi8(value, other)));
	}

	template<simd_int_128_type simd_int_t01, simd_int_128_type simd_int_t02> JSONIFIER_ALWAYS_INLINE auto opCmpLt(const simd_int_t01& value, const simd_int_t02& other) noexcept {
		jsonifier_simd_int_128 offset = _mm_set1_epi8(static_cast<char>(0x80));
		return static_cast<uint16_t>(_mm_movemask_epi8(_mm_cmpgt_epi8(_mm_add_epi8(other, offset), _mm_add_epi8(value, offset))));
	}

	template<simd_int_128_type simd_int_t01, simd_int_128_type simd_int_t02>
	JSONIFIER_ALWAYS_INLINE auto opCmpEqRaw(const simd_int_t01& value, const simd_int_t02& other) noexcept {
		return _mm_cmpeq_epi8(value, other);
	}

	template<simd_int_128_type simd_int_t01, simd_int_128_type simd_int_t02>
	JSONIFIER_ALWAYS_INLINE auto opCmpLtRaw(const simd_int_t01& value, const simd_int_t02& other) noexcept {
		jsonifier_simd_int_128 offset = _mm_set1_epi8(static_cast<char>(0x80));
		return _mm_cmpgt_epi8(_mm_add_epi8(other, offset), _mm_add_epi8(value, offset));
	}

	template<simd_int_128_type simd_int_t01, simd_int_128_type simd_int_t02>
	JSONIFIER_ALWAYS_INLINE auto opCmpEqBitMask(const simd_int_t01& value, const simd_int_t02& other) noexcept {
		return static_cast<uint16_t>(_mm_movemask_epi8(_mm_cmpeq_epi8(value, other)));
	}

	template<simd_int_128_type simd_int_t01> JSONIFIER_ALWAYS_INLINE auto opBitMaskRaw(const simd_int_t01& value) noexcept {
		return _mm_movemask_epi8(value);
	}

	template<simd_int_128_type simd_int_t01, simd_int_128_type simd_int_t02> JSONIFIER_ALWAYS_INLINE auto opShuffle(const simd_int_t01& value, const simd_int_t02& other) noexcept {
		return _mm_shuffle_epi8(value, other);
	}

	template<simd_int_128_type simd_int_t01, simd_int_128_type simd_int_t02> JSONIFIER_ALWAYS_INLINE auto opXor(const simd_int_t01& value, const simd_int_t02& other) noexcept {
		return _mm_xor_si128(value, other);
	}

	template<simd_int_128_type simd_int_t01, simd_int_128_type simd_int_t02> JSONIFIER_ALWAYS_INLINE auto opAnd(const simd_int_t01& value, const simd_int_t02& other) noexcept {
		return _mm_and_si128(value, other);
	}

	template<simd_int_128_type simd_int_t01, simd_int_128_type simd_int_t02> JSONIFIER_ALWAYS_INLINE auto opOr(const simd_int_t01& value, const simd_int_t02& other) noexcept {
		return _mm_or_si128(value, other);
	}

	template<simd_int_128_type simd_int_t01, simd_int_128_type simd_int_t02> JSONIFIER_ALWAYS_INLINE auto opAndNot(const simd_int_t01& value, const simd_int_t02& other) noexcept {
		return _mm_andnot_si128(other, value);
	}

	template<simd_int_128_type simd_int_t01> JSONIFIER_ALWAYS_INLINE auto opTest(const simd_int_t01& value) noexcept {
		return !_mm_testz_si128(value, value);
	}

	template<simd_int_128_type simd_int_t01> JSONIFIER_ALWAYS_INLINE auto opNot(const simd_int_t01& value) noexcept {
		return _mm_xor_si128(value, _mm_set1_epi64x(0xFFFFFFFFFFFFFFFFll));
	}

	template<simd_int_128_type simd_type> JSONIFIER_ALWAYS_INLINE jsonifier_simd_int_128 opSetLSB(const simd_type& value, bool valueNew) noexcept {
		jsonifier_simd_int_128 mask{ _mm_set_epi64x(0, 0x01u) };
		return valueNew ? _mm_or_si128(value, mask) : _mm_andnot_si128(mask, value);
	}

	template<simd_int_128_type simd_type> JSONIFIER_ALWAYS_INLINE bool opGetMSB(const simd_type& value) noexcept {
		jsonifier_simd_int_128 result = _mm_and_si128(value, _mm_set_epi64x(0x8000000000000000ll, 0x00ll));
		return !_mm_testz_si128(result, result);
	}

	#if JSONIFIER_CHECK_FOR_INSTRUCTION(JSONIFIER_AVX512) || JSONIFIER_CHECK_FOR_INSTRUCTION(JSONIFIER_AVX2)

	template<simd_int_256_type simd_int_type_new> JSONIFIER_ALWAYS_INLINE simd_int_type_new gatherValues(const void* str) noexcept {
		return _mm256_load_si256(static_cast<const __m256i*>(str));
	}

	template<simd_int_256_type simd_int_type_new> JSONIFIER_ALWAYS_INLINE simd_int_type_new gatherValuesU(const void* str) noexcept {
		return _mm256_loadu_si256(static_cast<const __m256i*>(str));
	}

	template<simd_int_256_type simd_int_type_new, typename char_t>
		requires(sizeof(std::remove_cvref_t<char_t>) == 8)
	JSONIFIER_ALWAYS_INLINE simd_int_type_new gatherValue(char_t value) noexcept {
		return _mm256_set1_epi64x(static_cast<int64_t>(value));
	}

	template<simd_int_256_type simd_int_type_new, typename char_t>
		requires(sizeof(std::remove_cvref_t<char_t>) == 1)
	JSONIFIER_ALWAYS_INLINE simd_int_type_new gatherValue(char_t value) noexcept {
		return _mm256_set1_epi8(static_cast<char>(value));
	}

	template<simd_int_256_type simd_int_type_new> JSONIFIER_ALWAYS_INLINE void store(const simd_int_type_new& value, void* storageLocation) noexcept {
		_mm256_store_si256(static_cast<__m256i*>(storageLocation), value);
	}

	template<simd_int_256_type simd_int_t01, simd_int_256_type simd_int_t02> JSONIFIER_ALWAYS_INLINE auto opCmpEq(const simd_int_t01& value, const simd_int_t02& other) noexcept {
		return static_cast<uint32_t>(_mm256_movemask_epi8(_mm256_cmpeq_epi8(value, other)));
	}

	template<simd_int_256_type simd_int_t01, simd_int_256_type simd_int_t02> JSONIFIER_ALWAYS_INLINE auto opCmpLt(const simd_int_t01& value, const simd_int_t02& other) noexcept {
		jsonifier_simd_int_256 offset = _mm256_set1_epi8(static_cast<char>(0x80));
		return static_cast<uint32_t>(_mm256_movemask_epi8(_mm256_cmpgt_epi8(_mm256_add_epi8(other, offset), _mm256_add_epi8(value, offset))));
	}

	template<simd_int_256_type simd_int_t01, simd_int_256_type simd_int_t02>
	JSONIFIER_ALWAYS_INLINE auto opCmpEqRaw(const simd_int_t01& value, const simd_int_t02& other) noexcept {
		return _mm256_cmpeq_epi8(value, other);
	}

	template<simd_int_256_type simd_int_t01, simd_int_256_type simd_int_t02>
	JSONIFIER_ALWAYS_INLINE auto opCmpLtRaw(const simd_int_t01& value, const simd_int_t02& other) noexcept {
		jsonifier_simd_int_256 offset = _mm256_set1_epi8(static_cast<char>(0x80));
		return _mm256_cmpgt_epi8(_mm256_add_epi8(other, offset), _mm256_add_epi8(value, offset));
	}

	template<simd_int_256_type simd_int_t01, simd_int_256_type simd_int_t02>
	JSONIFIER_ALWAYS_INLINE auto opCmpEqBitMask(const simd_int_t01& value, const simd_int_t02& other) noexcept {
		return static_cast<uint32_t>(_mm256_movemask_epi8(_mm256_cmpeq_epi8(value, other)));
	}

	template<simd_int_256_type simd_int_t01> JSONIFIER_ALWAYS_INLINE auto opBitMaskRaw(const simd_int_t01& value) noexcept {
		return _mm256_movemask_epi8(value);
	}

	template<simd_int_256_type simd_int_t01, simd_int_256_type simd_int_t02> JSONIFIER_ALWAYS_INLINE auto opShuffle(const simd_int_t01& value, const simd_int_t02& other) noexcept {
		return _mm256_shuffle_epi8(value, other);
	}

	template<simd_int_256_type simd_int_t01, simd_int_256_type simd_int_t02> JSONIFIER_ALWAYS_INLINE auto opXor(const simd_int_t01& value, const simd_int_t02& other) noexcept {
		return _mm256_xor_si256(value, other);
	}

	template<simd_int_256_type simd_int_t01, simd_int_256_type simd_int_t02> JSONIFIER_ALWAYS_INLINE auto opAnd(const simd_int_t01& value, const simd_int_t02& other) noexcept {
		return _mm256_and_si256(value, other);
	}

	template<simd_int_256_type simd_int_t01, simd_int_256_type simd_int_t02> JSONIFIER_ALWAYS_INLINE auto opOr(const simd_int_t01& value, const simd_int_t02& other) noexcept {
		return _mm256_or_si256(value, other);
	}

	template<simd_int_256_type simd_int_t01, simd_int_256_type simd_int_t02> JSONIFIER_ALWAYS_INLINE auto opAndNot(const simd_int_t01& value, const simd_int_t02& other) noexcept {
		return _mm256_andnot_si256(other, value);
	}

	template<simd_int_256_type simd_int_t01> JSONIFIER_ALWAYS_INLINE auto opTest(const simd_int_t01& value) noexcept {
		return !_mm256_testz_si256(value, value);
	}

	template<simd_int_256_type simd_int_t01> JSONIFIER_ALWAYS_INLINE auto opNot(const simd_int_t01& value) noexcept {
		return _mm256_xor_si256(value, _mm256_set1_epi64x(0xFFFFFFFFFFFFFFFFll));
	}

	template<simd_int_256_type simd_type> JSONIFIER_ALWAYS_INLINE jsonifier_simd_int_256 opSetLSB(const simd_type& value, bool valueNew) noexcept {
		jsonifier_simd_int_256 mask{ _mm256_set_epi64x(0, 0, 0, 0x01u) };
		return valueNew ? _mm256_or_si256(value, mask) : _mm256_andnot_si256(mask, value);
	}

	template<simd_int_256_type simd_type> JSONIFIER_ALWAYS_INLINE bool opGetMSB(const simd_type& value) noexcept {
		jsonifier_simd_int_256 result = _mm256_and_si256(value, _mm256_set_epi64x(0x8000000000000000ll, 0x00ll, 0x00ll, 0x00ll));
		return !_mm256_testz_si256(result, result);
	}

		#if JSONIFIER_CHECK_FOR_INSTRUCTION(JSONIFIER_AVX512)

	template<simd_int_512_type simd_int_type_new> JSONIFIER_ALWAYS_INLINE simd_int_type_new gatherValues(const void* str) noexcept {
		return _mm512_load_si512(static_cast<const __m512i*>(str));
	}

	template<simd_int_512_type simd_int_type_new> JSONIFIER_ALWAYS_INLINE simd_int_type_new gatherValuesU(const void* str) noexcept {
		return _mm512_loadu_si512(static_cast<const __m512i*>(str));
	}

	template<simd_int_512_type simd_int_type_new, typename char_t>
		requires(sizeof(std::remove_cvref_t<char_t>) == 8)
	JSONIFIER_ALWAYS_INLINE simd_int_type_new gatherValue(char_t value) noexcept {
		return _mm512_set1_epi64(static_cast<int64_t>(value));
	}

	template<simd_int_512_type simd_int_type_new, typename char_t>
		requires(sizeof(std::remove_cvref_t<char_t>) == 1)
	JSONIFIER_ALWAYS_INLINE simd_int_type_new gatherValue(char_t value) noexcept {
		return _mm512_set1_epi8(static_cast<char>(value));
	}

	template<simd_int_512_type simd_int_type_new> JSONIFIER_ALWAYS_INLINE void store(const simd_int_type_new& value, void* storageLocation) noexcept {
		_mm512_store_si512(static_cast<__m512i*>(storageLocation), value);
	}

	template<simd_int_512_type simd_int_t01, simd_int_512_type simd_int_t02> JSONIFIER_ALWAYS_INLINE auto opCmpEq(const simd_int_t01& value, const simd_int_t02& other) noexcept {
		return static_cast<uint64_t>(_mm512_cmpeq_epi8_mask(value, other));
	}

	template<simd_int_512_type simd_int_t01, simd_int_512_type simd_int_t02> JSONIFIER_ALWAYS_INLINE auto opCmpLt(const simd_int_t01& value, const simd_int_t02& other) noexcept {
		return static_cast<uint64_t>(_mm512_cmpgt_epi8_mask(other, value));
	}

	template<simd_int_512_type simd_int_t01, simd_int_512_type simd_int_t02>
	JSONIFIER_ALWAYS_INLINE auto opCmpEqRaw(const simd_int_t01& value, const simd_int_t02& other) noexcept {
		return _mm512_maskz_set1_epi8(_mm512_cmpeq_epi8_mask(value, other), 0xFF);
	}

	template<simd_int_512_type simd_int_t01, simd_int_512_type simd_int_t02>
	JSONIFIER_ALWAYS_INLINE auto opCmpLtRaw(const simd_int_t01& value, const simd_int_t02& other) noexcept {
		return _mm512_maskz_set1_epi8(_mm512_cmpeq_epi8_mask(value, other), 0xFF);
	}

	template<simd_int_512_type simd_int_t01, simd_int_512_type simd_int_t02>
	JSONIFIER_ALWAYS_INLINE auto opCmpEqBitMask(const simd_int_t01& value, const simd_int_t02& other) noexcept {
		return static_cast<uint64_t>(_mm512_cmpeq_epi8_mask(value, other));
	}

	template<simd_int_512_type simd_int_t01> JSONIFIER_ALWAYS_INLINE auto opBitMaskRaw(const simd_int_t01& value) noexcept {
		return _mm512_movepi8_mask(value);
	}

	template<simd_int_512_type simd_int_t01, simd_int_512_type simd_int_t02> JSONIFIER_ALWAYS_INLINE auto opShuffle(const simd_int_t01& value, const simd_int_t02& other) noexcept {
		return _mm512_shuffle_epi8(value, other);
	}

	template<simd_int_512_type simd_int_t01, simd_int_512_type simd_int_t02> JSONIFIER_ALWAYS_INLINE auto opXor(const simd_int_t01& value, const simd_int_t02& other) noexcept {
		return _mm512_xor_si512(value, other);
	}

	template<simd_int_512_type simd_int_t01, simd_int_512_type simd_int_t02> JSONIFIER_ALWAYS_INLINE auto opAnd(const simd_int_t01& value, const simd_int_t02& other) noexcept {
		return _mm512_and_si512(value, other);
	}

	template<simd_int_512_type simd_int_t01, simd_int_512_type simd_int_t02> JSONIFIER_ALWAYS_INLINE auto opOr(const simd_int_t01& value, const simd_int_t02& other) noexcept {
		return _mm512_or_si512(value, other);
	}

	template<simd_int_512_type simd_int_t01, simd_int_512_type simd_int_t02> JSONIFIER_ALWAYS_INLINE auto opAndNot(const simd_int_t01& value, const simd_int_t02& other) noexcept {
		return _mm512_andnot_si512(other, value);
	}

	template<simd_int_512_type simd_int_t01> JSONIFIER_ALWAYS_INLINE auto opTest(const simd_int_t01& value) noexcept {
		return !_mm512_test_epi64_mask(value, value);
	}

	template<simd_int_512_type simd_type> JSONIFIER_ALWAYS_INLINE jsonifier_simd_int_512 opSetLSB(const simd_type& value, bool valueNew) noexcept {
		jsonifier_simd_int_512 mask{ _mm512_set_epi64(0, 0, 0, 0, 0, 0, 0, 0x01u) };
		return valueNew ? _mm512_or_si512(value, mask) : _mm512_andnot_si512(mask, value);
	}

	template<simd_int_512_type simd_type> JSONIFIER_ALWAYS_INLINE bool opGetMSB(const simd_type& value) noexcept {
		jsonifier_simd_int_512 result = _mm512_and_si512(value, _mm512_set_epi64(0x8000000000000000ll, 0x00ll, 0x00ll, 0x00ll, 0x00ll, 0x00ll, 0x00ll, 0x00ll));
		return !_mm512_test_epi64_mask(result, result);
	}

	template<simd_int_512_type simd_int_t01> JSONIFIER_ALWAYS_INLINE auto opNot(const simd_int_t01& value) noexcept {
		return _mm512_xor_si512(value, _mm512_set1_epi64(0xFFFFFFFFFFFFFFFFll));
	}

		#endif

	#endif

#endif

}