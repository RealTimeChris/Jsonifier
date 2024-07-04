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

	template<typename simd_int_t01, size_t... indices> JSONIFIER_INLINE string_parsing_type mm128MovemaskEpi8(simd_int_t01&& a, std::index_sequence<indices...>&&) {
		string_parsing_type mask{ 0 };
		((mask |= (a.m128x_int8[indices] & 0x80) ? (1 << indices) : 0), ...);
		return mask;
	}

	template<typename simd_int_t01, typename simd_int_t02> JSONIFIER_INLINE constexpr __m128x mm128OrSi128(simd_int_t01&& valOne, simd_int_t02&& valTwo) {
		__m128x value{};
		memcpy(value.m128x_uint64, valOne.m128x_uint64, sizeof(value));
		value.m128x_uint64[0] |= valTwo.m128x_uint64[0];
		value.m128x_uint64[1] |= valTwo.m128x_uint64[1];
		return value;
	}

	template<typename simd_int_t01, typename simd_int_t02> JSONIFIER_INLINE constexpr __m128x mm128AndSi128(simd_int_t01&& valOne, simd_int_t02&& valTwo) {
		__m128x value{};
		memcpy(value.m128x_uint64, valOne.m128x_uint64, sizeof(value));
		value.m128x_uint64[0] &= valTwo.m128x_uint64[0];
		value.m128x_uint64[1] &= valTwo.m128x_uint64[1];
		return value;
	}

	template<typename simd_int_t01, typename simd_int_t02> JSONIFIER_INLINE constexpr __m128x mm128AndNotSi128N(simd_int_t01&& valOne, simd_int_t02&& valTwo) {
		__m128x value{};
		memcpy(value.m128x_uint64, valTwo.m128x_uint64, sizeof(value));
		value.m128x_uint64[0] &= ~valOne.m128x_uint64[0];
		value.m128x_uint64[1] &= ~valOne.m128x_uint64[1];
		return value;
	}

	template<typename simd_int_t01, typename simd_int_t02> JSONIFIER_INLINE constexpr __m128x mm128XorSi128(simd_int_t01&& valOne, simd_int_t02&& valTwo) {
		__m128x value{};
		std::copy(valOne.m128x_uint64, valOne.m128x_uint64 + 2, value.m128x_uint64);
		value.m128x_uint64[0] ^= valTwo.m128x_uint64[0];
		value.m128x_uint64[1] ^= valTwo.m128x_uint64[1];
		return value;
	}

	template<typename simd_int_t01, typename simd_int_t02, size_t... indices>
	JSONIFIER_INLINE __m128x mm128CmpEqEpi8(simd_int_t01&& a, simd_int_t02&& b, std::index_sequence<indices...>&&) {
		__m128x result{};
		((result.m128x_int8[indices] = (a.m128x_int8[indices] == b.m128x_int8[indices]) ? 0xFF : 0), ...);
		return result;
	}

	template<typename simd_int_t01, typename simd_int_t02> JSONIFIER_INLINE bool mm128TestzSi128(simd_int_t01& valOne, simd_int_t02& valTwo) {
		valOne.m128x_uint64[0] &= valTwo.m128x_uint64[0];
		valOne.m128x_uint64[1] &= valTwo.m128x_uint64[1];
		return valOne.m128x_uint64[0] == 0 && valOne.m128x_uint64[1] == 0;
	}

	JSONIFIER_INLINE __m128x mm128SetrEpi64xRt(uint64_t argOne, uint64_t argTwo) {
		__m128x returnValue{};
		std::memcpy(&returnValue.m128x_uint64[0], &argTwo, sizeof(uint64_t));
		std::memcpy(&returnValue.m128x_uint64[1], &argOne, sizeof(uint64_t));
		return returnValue;
	}

	constexpr __m128x mm128SetrEpi64xCt(uint64_t argOne, uint64_t argTwo) {
		__m128x returnValue{};
		returnValue.m128x_uint64[0] = argTwo;
		returnValue.m128x_uint64[1] = argOne;
		return returnValue;
	}

	JSONIFIER_INLINE __m128x mm128Set1Epi64xRt(uint64_t argOne) {
		__m128x returnValue{};
		std::memcpy(&returnValue.m128x_uint64[0], &argOne, sizeof(uint64_t));
		std::memcpy(&returnValue.m128x_uint64[1], &argOne, sizeof(uint64_t));
		return returnValue;
	}

	constexpr __m128x mm128Set1Epi64xCt(uint64_t argOne) {
		__m128x returnValue{};
		returnValue.m128x_uint64[0] = argOne;
		returnValue.m128x_uint64[1] = argOne;
		return returnValue;
	}

	constexpr __m128x mm128BlendVEpi8(const __m128x& a, const __m128x& b, const __m128x& mask) {
		__m128x result;
		for (int32_t i = 0; i < 2; ++i) {
			result.m128x_uint64[i] = 0;
			for (int32_t j = 0; j < 8; ++j) {
				uint8_t maskByte	= (mask.m128x_uint64[1 - i] >> (j * 8)) & 0xFF;
				uint8_t aByte		= (a.m128x_uint64[1 - i] >> (j * 8)) & 0xFF;
				uint8_t bByte		= (b.m128x_uint64[1 - i] >> (j * 8)) & 0xFF;
				uint8_t blendedByte = (maskByte ? bByte : aByte);
				result.m128x_uint64[i] |= (static_cast<uint64_t>(blendedByte) << (j * 8));
			}
		}
		return result;
	}

	constexpr __m128x mm128AddEpi64(const __m128x& value01, const __m128x& value02) {
		__m128x returnValue{};
		__m128x value01New{ value01  }, value02New{ value02 };
		returnValue.m128x_uint64[0] = value01New.m128x_uint64[0] + value02New.m128x_uint64[0];
		returnValue.m128x_uint64[1] = value01New.m128x_uint64[1] + value02New.m128x_uint64[1];
		return returnValue;
	}

	constexpr void mm128StoreUSi128(uint8_t* ptr, const __m128x& data) {
		for (int32_t i = 0; i < 8; ++i) {
			ptr[i] = static_cast<uint8_t>(data.m128x_uint64[0] >> (i * 8));
		}

		for (int32_t i = 0; i < 8; ++i) {
			ptr[i + 8] = static_cast<uint8_t>(data.m128x_uint64[1] >> (i * 8));
		}
	}

	constexpr void mm128StoreUSi128(uint64_t* ptr, const __m128x& data) {
		for (int32_t i = 0; i < 2; ++i) {
			ptr[i] = static_cast<uint64_t>(data.m128x_uint64[i]);
		}
	}

	constexpr uint32_t get32(const uint64_t* data, int index) {
		return (data[index / 2] >> (32 * (index % 2))) & 0xFFFFFFFF;
	}

	constexpr void set32(uint64_t* data, int index, uint32_t value) {
		data[index / 2] &= ~(0xFFFFFFFFull << (32 * (index % 2)));
		data[index / 2] |= (static_cast<uint64_t>(value) << (32 * (index % 2)));
	}

	constexpr void set64(uint64_t* data, int index, uint64_t value) {
		data[index] = value;
	}

	constexpr __m128x mm128ShuffleEpi32(const __m128x& a, int imm8) {
		__m128x result{};
		set32(result.m128x_uint64, 0, get32(a.m128x_uint64, (imm8 >> 0) & 0x3));
		set32(result.m128x_uint64, 1, get32(a.m128x_uint64, (imm8 >> 2) & 0x3));
		set32(result.m128x_uint64, 2, get32(a.m128x_uint64, (imm8 >> 4) & 0x3));
		set32(result.m128x_uint64, 3, get32(a.m128x_uint64, (imm8 >> 6) & 0x3));
		return result;
	}

	constexpr __m128x mm128MulEpi32(const __m128x& a, const __m128x& b) {
		__m128x result{};

		// Perform element-wise multiplication for each 32-bit integer
		uint32_t a_val[4];
		uint32_t b_val[4];

		// Extract 32-bit values from the input __m128x structs
		for (int i = 0; i < 4; ++i) {
			a_val[i] = get32(a.m128x_uint64, i);
			b_val[i] = get32(b.m128x_uint64, i);
		}

		// Perform multiplications
		set64(result.m128x_uint64, 0, static_cast<uint64_t>(a_val[0]) * static_cast<uint64_t>(b_val[0]) | static_cast<uint64_t>(a_val[1]) * static_cast<uint64_t>(b_val[1]));
		set64(result.m128x_uint64, 1, static_cast<uint64_t>(a_val[2]) * static_cast<uint64_t>(b_val[2]) | static_cast<uint64_t>(a_val[3]) * static_cast<uint64_t>(b_val[3]));

		return result;
	}

	constexpr __m128x mm128SubEpi64(const __m128x& value01, const __m128x& value02) {
		__m128x returnValue{};
		returnValue.m128x_uint64[0] = value01.m128x_uint64[0] - value02.m128x_uint64[0];
		returnValue.m128x_uint64[1] = value01.m128x_uint64[1] - value02.m128x_uint64[1];
		return returnValue;
	}

	constexpr __m128x mm128SrliEpi64(const __m128x& a, uint32_t imm8) {
		__m128x result{};
		result.m128x_uint64[0] = a.m128x_uint64[0] >> imm8;
		result.m128x_uint64[1] = a.m128x_uint64[1] >> imm8;
		return result;
	}

	constexpr __m128x mm128SlliEpi64(const __m128x& a, uint32_t imm8) {
		__m128x result{};
		result.m128x_uint64[0] = a.m128x_uint64[0] << imm8;
		result.m128x_uint64[1] = a.m128x_uint64[1] << imm8;
		return result;
	}

	constexpr __m128x mm128Set1Epi32(uint32_t value) {
		uint64_t extended_value = (static_cast<uint64_t>(value) << 32) | value;
		return __m128x{ extended_value, extended_value };
	}

	template<typename value_type> constexpr __m128x mm128LoadUSi128(const value_type* ptr) {
		uint64_t low  = 0;
		uint64_t high = 0;

		for (int32_t i = 0; i < 8; ++i) {
			low |= static_cast<const uint64_t>(ptr[i]) << (i * 8);
		}

		for (int32_t i = 0; i < 8; ++i) {
			high |= static_cast<const uint64_t>(ptr[i + 8]) << (i * 8);
		}
		__m128x returnValues{};
		returnValues.m128x_uint64[0] = low;
		returnValues.m128x_uint64[1] = high;
		return returnValues;
	}

	constexpr __m128x mm128LoadUSi128(const uint64_t* ptr) {
		__m128x returnValues{};
		returnValues.m128x_uint64[0] = ptr[0];
		returnValues.m128x_uint64[1] = ptr[1];
		return returnValues;
	}

	constexpr __m128x mm128LoadUSi128(const __m128x* ptr) {
		__m128x returnValues{ *ptr };
		return returnValues;
	}

	constexpr __m128x mm128SetZero() {
		return __m128x{};
	}

	template<typename simd_int_t01, typename simd_int_t02, size_t... indices>
	JSONIFIER_INLINE __m128x mm128ShuffleEpi8(simd_int_t01&& a, simd_int_t02&& b, std::index_sequence<indices...>) {
		__m128x result{};
		size_t index{};
		(((index = b.m128x_uint8[indices] & 0x0F), (result.m128x_uint8[indices] = a.m128x_uint8[index])), ...);
		return result;
	}

#if !JSONIFIER_CHECK_FOR_INSTRUCTION(JSONIFIER_AVX) && !JSONIFIER_CHECK_FOR_INSTRUCTION(JSONIFIER_AVX2) && !JSONIFIER_CHECK_FOR_INSTRUCTION(JSONIFIER_AVX512) && \
	!JSONIFIER_CHECK_FOR_INSTRUCTION(JSONIFIER_NEON)

	template<jsonifier::concepts::simd_int_type simd_int_t01, jsonifier::concepts::simd_int_type simd_int_t02>
	JSONIFIER_INLINE __m128x opAndNot(simd_int_t01&& value, simd_int_t02&& other) {
		return mm128AndNotSi128N(std::forward<simd_int_t02>(other), std::forward<simd_int_t01>(value));
	}

	template<jsonifier::concepts::simd_int_type simd_int_t01, jsonifier::concepts::simd_int_type simd_int_t02>
	JSONIFIER_INLINE __m128x opAnd(simd_int_t01&& value, simd_int_t02&& other) {
		return mm128AndSi128(std::forward<simd_int_t01>(value), std::forward<simd_int_t02>(other));
	}

	template<jsonifier::concepts::simd_int_type simd_int_t01, jsonifier::concepts::simd_int_type simd_int_t02>
	JSONIFIER_INLINE __m128x opXor(simd_int_t01&& value, simd_int_t02&& other) {
		return mm128XorSi128(std::forward<simd_int_t01>(value), std::forward<simd_int_t02>(other));
	}

	template<jsonifier::concepts::simd_int_128_type simd_int_t01, jsonifier::concepts::simd_int_128_type simd_int_t02>
	JSONIFIER_INLINE simd_int_128 opAdd(simd_int_t01&& value, simd_int_t02&& other) {
		return mm128AddEpi64(std::forward<simd_int_t01>(value), std::forward<simd_int_t02>(other));
	}

	template<int32_t other, jsonifier::concepts::simd_int_128_type simd_int_t01> JSONIFIER_INLINE simd_int_128 opSlli(simd_int_t01&& value) {
		return mm128SlliEpi64(std::forward<simd_int_t01>(value), other);
	}

	template<int32_t other, jsonifier::concepts::simd_int_128_type simd_int_t01> JSONIFIER_INLINE simd_int_128 opSrli(simd_int_t01&& value) {
		return mm128SrliEpi64(std::forward<simd_int_t01>(value), other);
	}

	template<int32_t other, jsonifier::concepts::simd_int_128_type simd_int_t01> JSONIFIER_INLINE simd_int_128 opShuffle(simd_int_t01&& value) {
		return mm128ShuffleEpi32(std::forward<simd_int_t01>(value), other);
	}

	template<jsonifier::concepts::simd_int_128_type simd_int_t01, jsonifier::concepts::simd_int_128_type simd_int_t02>
	JSONIFIER_INLINE simd_int_128 opMul(simd_int_t01&& value, simd_int_t02&& other) {
		return mm128MulEpi32(std::forward<simd_int_t01>(value), std::forward<simd_int_t02>(other));
	}

	template<jsonifier::concepts::simd_int_type simd_int_t01, jsonifier::concepts::simd_int_type simd_int_t02>
	JSONIFIER_INLINE __m128x opOr(simd_int_t01&& value, simd_int_t02&& other) {
		return mm128OrSi128(std::forward<simd_int_t01>(value), std::forward<simd_int_t02>(other));
	}

	template<jsonifier::concepts::simd_int_type simd_int_t01> JSONIFIER_INLINE __m128x opSetLSB(simd_int_t01&& value, bool valueNew) {
		jsonifier_internal::unwrap_t<__m128x> mask = mm128SetrEpi64xRt(0x00ll, 0x01ll);
		return valueNew ? mm128OrSi128(value, mask) : mm128AndNotSi128N(mask, value);
	}

	template<jsonifier::concepts::simd_int_type simd_int_t01> JSONIFIER_INLINE __m128x opNot(simd_int_t01&& value) {
		return mm128XorSi128(std::forward<simd_int_t01>(value), mm128Set1Epi64xRt(0xFFFFFFFFFFFFFFFFll));
	}

	template<jsonifier::concepts::simd_int_type simd_int_t01> JSONIFIER_INLINE bool opGetMSB(simd_int_t01&& value) {
		__m128x result = mm128AndSi128(std::forward<simd_int_t01>(value), mm128SetrEpi64xRt(0x8000000000000000ll, 0x00ll));
		return !mm128TestzSi128(result, result);
	}

	template<jsonifier::concepts::simd_int_type simd_int_t01> JSONIFIER_INLINE bool opBool(simd_int_t01&& value) {
		return !mm128TestzSi128(value, value);
	}

	template<typename simd_int_t> JSONIFIER_INLINE __m128x reset() {
		return __m128x{};
	}

#endif

}