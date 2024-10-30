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

namespace simd_internal {

#if !JSONIFIER_CHECK_FOR_INSTRUCTION(JSONIFIER_AVX) && !JSONIFIER_CHECK_FOR_INSTRUCTION(JSONIFIER_AVX2) && !JSONIFIER_CHECK_FOR_INSTRUCTION(JSONIFIER_AVX512) && \
	!JSONIFIER_CHECK_FOR_INSTRUCTION(JSONIFIER_NEON)

	template<simd_int_128_type simd_int_t01, simd_int_128_type simd_int_t02> JSONIFIER_ALWAYS_INLINE auto opCmpLt(simd_int_t01&& value, simd_int_t02&& other) noexcept {
		jsonifier_simd_int_128 offset = mm128Set1Epi8(static_cast<char>(0x80));
		return mm128MovemaskEpi8(mm128CmpGtEpi8(mm128AddEpi8(other, offset), mm128AddEpi8(value, offset)), std::make_index_sequence<16>{});
	}

	template<simd_int_128_type simd_int_t01, simd_int_128_type simd_int_t02> JSONIFIER_ALWAYS_INLINE auto opXor(simd_int_t01&& value, simd_int_t02&& other) noexcept {
		return mm128XorSi128(value, other);
	}

	template<simd_int_128_type simd_int_t01, simd_int_128_type simd_int_t02> JSONIFIER_ALWAYS_INLINE auto opAnd(simd_int_t01&& value, simd_int_t02&& other) noexcept {
		return mm128AndSi128(value, other);
	}

	template<simd_int_128_type simd_int_t01, simd_int_128_type simd_int_t02> JSONIFIER_ALWAYS_INLINE auto opOr(simd_int_t01&& value, simd_int_t02&& other) noexcept {
		return mm128OrSi128(value, other);
	}

	template<simd_int_128_type simd_int_t01, simd_int_128_type simd_int_t02> JSONIFIER_ALWAYS_INLINE auto opAndNot(simd_int_t01&& value, simd_int_t02&& other) noexcept {
		return mm128AndNotSi128(other, value);
	}

	template<simd_int_128_type simd_int_t01> JSONIFIER_ALWAYS_INLINE auto opNot(simd_int_t01&& value) noexcept {
		return mm128XorSi128(value, mm128Set1Epi64x(0xFFFFFFFFFFFFFFFFll));
	}

	template<simd_int_128_type simd_int_t01> JSONIFIER_ALWAYS_INLINE auto opTest(simd_int_t01&& value) noexcept {
		return !mm128TestzSi128(value, value);
	}

	template<simd_int_128_type simd_type> JSONIFIER_ALWAYS_INLINE jsonifier_simd_int_128 opSetLSB(simd_type&& value, bool valueNew) noexcept {
		jsonifier_simd_int_128 mask{ 0x01u, '\0' };
		return valueNew ? mm128OrSi128(value, mask) : mm128AndNotSi128(value, mask);
	}

	template<simd_int_128_type simd_type> JSONIFIER_ALWAYS_INLINE bool opGetMSB(simd_type&& value) noexcept {
		auto result = mm128AndSi128(value, mm128SetrEpi64x(0x8000000000000000ll, 0x00ll));
		return !mm128TestzSi128(result, result);
	}

	template<simd_int_128_type simd_int_t01, simd_int_128_type simd_int_t02> uint16_t opCmpEq(simd_int_t01&& value, simd_int_t02&& other) noexcept {
		return static_cast<uint16_t>(mm128MovemaskEpi8(mm128CmpEqEpi8(std::forward<simd_int_t01>(value), std::forward<simd_int_t02>(other), std::make_index_sequence<16>{}),
			std::make_index_sequence<16>{}));
	}

	template<simd_int_128_type simd_int_type_new> JSONIFIER_ALWAYS_INLINE simd_int_type_new gatherValues(const void* str) noexcept {
		jsonifier_simd_int_t returnValue{};
		std::memcpy(&returnValue, str, sizeof(jsonifier_simd_int_t));
		return returnValue;
	}

	template<simd_int_128_type simd_int_type_new> JSONIFIER_ALWAYS_INLINE simd_int_type_new gatherValuesU(const void* str) noexcept {
		jsonifier_simd_int_t returnValue{};
		std::memcpy(&returnValue, str, sizeof(jsonifier_simd_int_t));
		return returnValue;
	}

	template<simd_int_128_type simd_int_type_new, typename char_type> JSONIFIER_ALWAYS_INLINE simd_int_type_new gatherValue(char_type str) noexcept {
		jsonifier_simd_int_t returnValue{};
		std::memset(&returnValue, str, sizeof(jsonifier_simd_int_t));
		return returnValue;
	}

	JSONIFIER_ALWAYS_INLINE void store(const jsonifier_simd_int_t& value, void* storageLocation) noexcept {
		std::memcpy(storageLocation, &value, sizeof(jsonifier_simd_int_t));
	}

	template<simd_int_128_type simd_int_t01, simd_int_128_type simd_int_t02> JSONIFIER_ALWAYS_INLINE auto opShuffle(simd_int_t01&& value, simd_int_t02&& other) noexcept {
		return mm128ShuffleEpi8(std::forward<simd_int_t01>(value), std::forward<simd_int_t02>(other), std::make_index_sequence<16>{});
	}

#endif

}