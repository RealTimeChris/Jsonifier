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

#include <jsonifier/Simd/CTimeSimdTypes.hpp>
#include <jsonifier/Simd/SimdTypes.hpp>
#include <jsonifier/Simd/Bmi.hpp>

namespace jsonifier::simd {

#if !JSONIFIER_CHECK_FOR_INSTRUCTION(JSONIFIER_AVX) && !JSONIFIER_CHECK_FOR_INSTRUCTION(JSONIFIER_AVX2) && !JSONIFIER_CHECK_FOR_INSTRUCTION(JSONIFIER_AVX512) && \
	!JSONIFIER_CHECK_FOR_INSTRUCTION(JSONIFIER_NEON)

	template<typename value_type> JSONIFIER_INLINE static value_type postCmpTzcnt(value_type value) noexcept {
		return tzcnt(value);
	}

	template<simd_int_128_type simd_int_type_new> JSONIFIER_INLINE static simd_int_type_new gatherValues(const void* str) noexcept {
		return gather_values<simd_int_type_new>(static_cast<const simd_x<simd_classes::x_128>*>(str));
	}

	template<simd_int_128_type simd_int_type_new> JSONIFIER_INLINE static simd_int_type_new gatherValuesU(const void* str, void*) noexcept {
		return gather_values_u<simd_int_type_new>(static_cast<const simd_x<simd_classes::x_128>*>(str));
	}

	template<simd_int_128_type simd_int_type_new, typename char_t>
		requires(sizeof(char_t) == 8)
	JSONIFIER_INLINE static simd_int_type_new gatherValue(char_t str) noexcept {
		return op_set1_epi64<simd_int_type_new>(static_cast<int64_t>(str));
	}

	template<simd_int_128_type simd_int_type_new, typename char_t>
		requires(sizeof(char_t) == 1)
	JSONIFIER_INLINE static simd_int_type_new gatherValue(char_t str, std::source_location location = std::source_location::current()) noexcept {
		return op_set1_epi8<simd_int_type_new>(static_cast<char>(str));
	}

	template<simd_int_128_type simd_int_type_new> JSONIFIER_INLINE static void store(const simd_int_type_new& value, void* storageLocation) noexcept {
		store_values(value, storageLocation);
	}

	template<simd_int_128_type simd_int_type_new> JSONIFIER_INLINE static void storeU(const simd_int_type_new& value, void* storageLocation, void*) noexcept {
		store_values_u(value, storageLocation);
	}

	template<simd_int_128_type simd_int_t01, simd_int_128_type simd_int_t02> JSONIFIER_INLINE static auto opCmpEq(const simd_int_t01& value, const simd_int_t02& other) noexcept {
		return static_cast<uint16_t>(op_cmp_eq_bitmask<simd_int_t01>(value, other));
	}

	template<simd_int_128_type simd_int_t01, simd_int_128_type simd_int_t02> JSONIFIER_INLINE static auto opCmpLt(const simd_int_t01& value, const simd_int_t02& other) noexcept {
		static constexpr jsonifier_simd_int_128 offset = op_set1_epi8<simd_int_t01>(static_cast<char>(0x80));
		return static_cast<uint32_t>(op_bitmask<simd_int_t01>(op_cmp_gt<simd_int_t01>(op_add<simd_int_t01>(other, offset), op_add<simd_int_t01>(value, offset))));
	}

	template<simd_int_128_type simd_int_t01, simd_int_128_type simd_int_t02>
	JSONIFIER_INLINE static auto opCmpEqRaw(const simd_int_t01& value, const simd_int_t02& other) noexcept {
		return op_cmp_eq_epi8<simd_int_t01>(value, other);
	}

	template<simd_int_128_type simd_int_t01, simd_int_128_type simd_int_t02>
	JSONIFIER_INLINE static auto opCmpLtRaw(const simd_int_t01& value, const simd_int_t02& other) noexcept {
		static constexpr jsonifier_simd_int_128 offset = op_set1_epi8<simd_int_t01>(static_cast<char>(0x80));
		return op_cmp_gt<simd_int_t01>(op_add<simd_int_t01>(other, offset), op_add<simd_int_t01>(value, offset));
	}

	template<simd_int_128_type simd_int_t01, simd_int_128_type simd_int_t02>
	JSONIFIER_INLINE static auto opCmpEqBitMask(const simd_int_t01& value, const simd_int_t02& other) noexcept {
		return static_cast<uint16_t>(op_cmp_eq_bitmask<simd_int_t01>(value, other));
	}

	template<simd_int_128_type simd_int_t01> JSONIFIER_INLINE static auto opBitMaskRaw(const simd_int_t01& value) noexcept {
		return op_bitmask<simd_int_t01>(value);
	}

	template<simd_int_128_type simd_int_t01, simd_int_128_type simd_int_t02> JSONIFIER_INLINE static auto opShuffle(const simd_int_t01& value, const simd_int_t02& other) noexcept {
		return op_shuffle<simd_int_t01>(value, other);
	}

	template<simd_int_128_type simd_int_t01, simd_int_128_type simd_int_t02> JSONIFIER_INLINE static auto opXor(const simd_int_t01& value, const simd_int_t02& other) noexcept {
		return op_xor<simd_int_t01>(value, other);
	}

	template<simd_int_128_type simd_int_t01, simd_int_128_type simd_int_t02> JSONIFIER_INLINE static auto opAnd(const simd_int_t01& value, const simd_int_t02& other) noexcept {
		return op_and<simd_int_t01>(value, other);
	}

	template<simd_int_128_type simd_int_t01, simd_int_128_type simd_int_t02> JSONIFIER_INLINE static auto opOr(const simd_int_t01& value, const simd_int_t02& other) noexcept {
		return op_or<simd_int_t01>(value, other);
	}

	template<simd_int_128_type simd_int_t01, simd_int_128_type simd_int_t02> JSONIFIER_INLINE static auto opAndNot(const simd_int_t01& value, const simd_int_t02& other) noexcept {
		return op_andnot<simd_int_t01>(other, value);
	}

	template<simd_int_128_type simd_int_t01> JSONIFIER_INLINE static auto opTest(const simd_int_t01& value) noexcept {
		return !op_test<simd_int_t01>(value);
	}

	template<simd_int_128_type simd_int_t01> JSONIFIER_INLINE static auto opNot(const simd_int_t01& value) noexcept {
		return op_xor<simd_int_t01>(value, op_set1_epi64<simd_int_t01>(0xFFFFFFFFFFFFFFFFll));
	}

	template<simd_int_128_type simd_type> JSONIFIER_INLINE static jsonifier_simd_int_128 opSetLSB(const simd_type& value, bool valueNew) noexcept {
		static constexpr jsonifier_simd_int_128 mask{ op_set_epi64<simd_type>(0x01uLL, 0ULL) };
		return valueNew ? op_or<jsonifier_simd_int_128>(value, mask) : op_andnot<jsonifier_simd_int_128>(mask, value);
	}

	template<simd_int_128_type simd_type> JSONIFIER_INLINE static bool opGetMSB(const simd_type& value) noexcept {
		const simd_type result = op_and<simd_type>(value, op_set_epi64<simd_type>(0x00ll, 0x8000000000000000ll));
		return !op_test<simd_type>(result);
	}

#endif

}