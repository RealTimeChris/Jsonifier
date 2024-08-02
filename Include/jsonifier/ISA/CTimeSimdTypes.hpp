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

#include <type_traits>
#include <algorithm>
#include <cstdint>
#include <utility>

namespace jsonifier_internal {

	union __m128x {
		template<typename value_type> constexpr __m128x(value_type arg01, value_type arg02, value_type arg03, value_type arg04, value_type arg05, value_type arg06,
			value_type arg07, value_type arg08, value_type arg09, value_type arg10, value_type arg11, value_type arg12, value_type arg13, value_type arg14, value_type arg15,
			value_type arg16) noexcept {
			m128x_uint64[0] = static_cast<size_t>(arg01);
			m128x_uint64[0] |= static_cast<size_t>(arg02) << 8;
			m128x_uint64[0] |= static_cast<size_t>(arg03) << 16;
			m128x_uint64[0] |= static_cast<size_t>(arg04) << 24;
			m128x_uint64[0] |= static_cast<size_t>(arg05) << 32;
			m128x_uint64[0] |= static_cast<size_t>(arg06) << 40;
			m128x_uint64[0] |= static_cast<size_t>(arg07) << 48;
			m128x_uint64[0] |= static_cast<size_t>(arg08) << 56;
			m128x_uint64[1] = static_cast<size_t>(arg09);
			m128x_uint64[1] |= static_cast<size_t>(arg10) << 8;
			m128x_uint64[1] |= static_cast<size_t>(arg11) << 16;
			m128x_uint64[1] |= static_cast<size_t>(arg12) << 24;
			m128x_uint64[1] |= static_cast<size_t>(arg13) << 32;
			m128x_uint64[1] |= static_cast<size_t>(arg14) << 40;
			m128x_uint64[1] |= static_cast<size_t>(arg15) << 48;
			m128x_uint64[1] |= static_cast<size_t>(arg16) << 56;
		}

		constexpr __m128x(size_t argOne, size_t argTwo) noexcept {
			m128x_uint64[0] = argOne;
			m128x_uint64[1] = argTwo;
		}

		constexpr __m128x() noexcept {
			m128x_uint64[0] = 0;
			m128x_uint64[1] = 0;
		}

#if JSONIFIER_WIN
		int8_t m128x_int8[16]{};
		int16_t m128x_int16[8];
		int32_t m128x_int32[4];
		int64_t m128x_int64[2];
		uint8_t m128x_uint8[16];
		int16_t m128x_uint16[8];
		int32_t m128x_uint32[4];
		size_t m128x_uint64[2];
#else
		int64_t m128x_int64[2];
		int32_t m128x_int32[4];
		int16_t m128x_int16[8];
		int8_t m128x_int8[16]{};
		size_t m128x_uint64[2];
		int32_t m128x_uint32[4];
		int16_t m128x_uint16[8];
		uint8_t m128x_uint8[16];
#endif
	};

	union __m256x {
		template<typename value_type> constexpr __m256x(value_type arg01, value_type arg02, value_type arg03, value_type arg04, value_type arg05, value_type arg06,
			value_type arg07, value_type arg08, value_type arg09, value_type arg10, value_type arg11, value_type arg12, value_type arg13, value_type arg14, value_type arg15,
			value_type arg16, value_type arg17, value_type arg18, value_type arg19, value_type arg20, value_type arg21, value_type arg22, value_type arg23, value_type arg24,
			value_type arg25, value_type arg26, value_type arg27, value_type arg28, value_type arg29, value_type arg30, value_type arg31, value_type arg32) noexcept {
			m256x_uint64[0] = static_cast<size_t>(arg01);
			m256x_uint64[0] |= static_cast<size_t>(arg02) << 8;
			m256x_uint64[0] |= static_cast<size_t>(arg03) << 16;
			m256x_uint64[0] |= static_cast<size_t>(arg04) << 24;
			m256x_uint64[0] |= static_cast<size_t>(arg05) << 32;
			m256x_uint64[0] |= static_cast<size_t>(arg06) << 40;
			m256x_uint64[0] |= static_cast<size_t>(arg07) << 48;
			m256x_uint64[0] |= static_cast<size_t>(arg08) << 56;
			m256x_uint64[1] = static_cast<size_t>(arg09);
			m256x_uint64[1] |= static_cast<size_t>(arg10) << 8;
			m256x_uint64[1] |= static_cast<size_t>(arg11) << 16;
			m256x_uint64[1] |= static_cast<size_t>(arg12) << 24;
			m256x_uint64[1] |= static_cast<size_t>(arg13) << 32;
			m256x_uint64[1] |= static_cast<size_t>(arg14) << 40;
			m256x_uint64[1] |= static_cast<size_t>(arg15) << 48;
			m256x_uint64[1] |= static_cast<size_t>(arg16) << 56;
			m256x_uint64[2] = static_cast<size_t>(arg17);
			m256x_uint64[2] |= static_cast<size_t>(arg18) << 8;
			m256x_uint64[2] |= static_cast<size_t>(arg19) << 16;
			m256x_uint64[2] |= static_cast<size_t>(arg20) << 24;
			m256x_uint64[2] |= static_cast<size_t>(arg21) << 32;
			m256x_uint64[2] |= static_cast<size_t>(arg22) << 40;
			m256x_uint64[2] |= static_cast<size_t>(arg23) << 48;
			m256x_uint64[2] |= static_cast<size_t>(arg24) << 56;
			m256x_uint64[3] = static_cast<size_t>(arg25);
			m256x_uint64[3] |= static_cast<size_t>(arg26) << 8;
			m256x_uint64[3] |= static_cast<size_t>(arg27) << 16;
			m256x_uint64[3] |= static_cast<size_t>(arg28) << 24;
			m256x_uint64[3] |= static_cast<size_t>(arg29) << 32;
			m256x_uint64[3] |= static_cast<size_t>(arg30) << 40;
			m256x_uint64[3] |= static_cast<size_t>(arg31) << 48;
			m256x_uint64[3] |= static_cast<size_t>(arg32) << 56;
		}

		constexpr __m256x(size_t argOne, size_t argTwo, size_t argThree, size_t argFour) noexcept {
			m256x_uint64[0] = argOne;
			m256x_uint64[1] = argTwo;
			m256x_uint64[2] = argThree;
			m256x_uint64[3] = argFour;
		}

		constexpr __m256x() noexcept {
			m256x_uint64[0] = 0;
			m256x_uint64[1] = 0;
			m256x_uint64[2] = 0;
			m256x_uint64[3] = 0;
		}

#if JSONIFIER_WIN
		int8_t m256x_int8[32]{};
		int16_t m256x_int16[16];
		int32_t m256x_int32[8];
		int64_t m256x_int64[4];
		uint8_t m256x_uint8[32];
		int16_t m256x_uint16[16];
		int32_t m256x_uint32[8];
		size_t m256x_uint64[4];
#else
		int64_t m256x_int64[4];
		int32_t m256x_int32[8];
		int16_t m256x_int16[16];
		int8_t m256x_int8[32]{};
		size_t m256x_uint64[4];
		int32_t m256x_uint32[8];
		int16_t m256x_uint16[16];
		uint8_t m256x_uint8[32];
#endif
	};

	union __m512x {
		template<typename value_type> constexpr __m512x(value_type arg01, value_type arg02, value_type arg03, value_type arg04, value_type arg05, value_type arg06,
			value_type arg07, value_type arg08, value_type arg09, value_type arg10, value_type arg11, value_type arg12, value_type arg13, value_type arg14, value_type arg15,
			value_type arg16, value_type arg17, value_type arg18, value_type arg19, value_type arg20, value_type arg21, value_type arg22, value_type arg23, value_type arg24,
			value_type arg25, value_type arg26, value_type arg27, value_type arg28, value_type arg29, value_type arg30, value_type arg31, value_type arg32, value_type arg33,
			value_type arg34, value_type arg35, value_type arg36, value_type arg37, value_type arg38, value_type arg39, value_type arg40, value_type arg41, value_type arg42,
			value_type arg43, value_type arg44, value_type arg45, value_type arg46, value_type arg47, value_type arg48, value_type arg49, value_type arg50, value_type arg51,
			value_type arg52, value_type arg53, value_type arg54, value_type arg55, value_type arg56, value_type arg57, value_type arg58, value_type arg59, value_type arg60,
			value_type arg61, value_type arg62, value_type arg63, value_type arg64) noexcept {
			m512x_uint64[0] = static_cast<size_t>(arg01) | (static_cast<size_t>(arg02) << 8) | (static_cast<size_t>(arg03) << 16) | (static_cast<size_t>(arg04) << 24) |
				(static_cast<size_t>(arg05) << 32) | (static_cast<size_t>(arg06) << 40) | (static_cast<size_t>(arg07) << 48) | (static_cast<size_t>(arg08) << 56);
			m512x_uint64[1] = static_cast<size_t>(arg09) | (static_cast<size_t>(arg10) << 8) | (static_cast<size_t>(arg11) << 16) | (static_cast<size_t>(arg12) << 24) |
				(static_cast<size_t>(arg13) << 32) | (static_cast<size_t>(arg14) << 40) | (static_cast<size_t>(arg15) << 48) | (static_cast<size_t>(arg16) << 56);
			m512x_uint64[2] = static_cast<size_t>(arg17) | (static_cast<size_t>(arg18) << 8) | (static_cast<size_t>(arg19) << 16) | (static_cast<size_t>(arg20) << 24) |
				(static_cast<size_t>(arg21) << 32) | (static_cast<size_t>(arg22) << 40) | (static_cast<size_t>(arg23) << 48) | (static_cast<size_t>(arg24) << 56);
			m512x_uint64[3] = static_cast<size_t>(arg25) | (static_cast<size_t>(arg26) << 8) | (static_cast<size_t>(arg27) << 16) | (static_cast<size_t>(arg28) << 24) |
				(static_cast<size_t>(arg29) << 32) | (static_cast<size_t>(arg30) << 40) | (static_cast<size_t>(arg31) << 48) | (static_cast<size_t>(arg32) << 56);
			m512x_uint64[4] = static_cast<size_t>(arg33) | (static_cast<size_t>(arg34) << 8) | (static_cast<size_t>(arg35) << 16) | (static_cast<size_t>(arg36) << 24) |
				(static_cast<size_t>(arg37) << 32) | (static_cast<size_t>(arg38) << 40) | (static_cast<size_t>(arg39) << 48) | (static_cast<size_t>(arg40) << 56);
			m512x_uint64[5] = static_cast<size_t>(arg41) | (static_cast<size_t>(arg42) << 8) | (static_cast<size_t>(arg43) << 16) | (static_cast<size_t>(arg44) << 24) |
				(static_cast<size_t>(arg45) << 32) | (static_cast<size_t>(arg46) << 40) | (static_cast<size_t>(arg47) << 48) | (static_cast<size_t>(arg48) << 56);
			m512x_uint64[6] = static_cast<size_t>(arg49) | (static_cast<size_t>(arg50) << 8) | (static_cast<size_t>(arg51) << 16) | (static_cast<size_t>(arg52) << 24) |
				(static_cast<size_t>(arg53) << 32) | (static_cast<size_t>(arg54) << 40) | (static_cast<size_t>(arg55) << 48) | (static_cast<size_t>(arg56) << 56);
			m512x_uint64[7] = static_cast<size_t>(arg57) | (static_cast<size_t>(arg58) << 8) | (static_cast<size_t>(arg59) << 16) | (static_cast<size_t>(arg60) << 24) |
				(static_cast<size_t>(arg61) << 32) | (static_cast<size_t>(arg62) << 40) | (static_cast<size_t>(arg63) << 48) | (static_cast<size_t>(arg64) << 56);
		}

		constexpr __m512x(size_t argOne, size_t argTwo, size_t argThree, size_t argFour, size_t argFive, size_t argSix, size_t argSeven, size_t argEight) noexcept {
			m512x_uint64[0] = argOne;
			m512x_uint64[1] = argTwo;
			m512x_uint64[2] = argThree;
			m512x_uint64[3] = argFour;
			m512x_uint64[4] = argFive;
			m512x_uint64[5] = argSix;
			m512x_uint64[6] = argSeven;
			m512x_uint64[7] = argEight;
		}

		constexpr __m512x() noexcept {
			for (int32_t i = 0; i < 8; ++i) {
				m512x_uint64[i] = 0;
			}
		}

#if JSONIFIER_WIN
		int8_t m512x_int8[64]{};
		int16_t m512x_int16[32];
		int32_t m512x_int32[16];
		int64_t m512x_int64[8];
		uint8_t m512x_uint8[64];
		uint16_t m512x_uint16[32];
		uint32_t m512x_uint32[16];
		size_t m512x_uint64[8];
#else
		int64_t m512x_int64[8];
		int32_t m512x_int32[16];
		int16_t m512x_int16[32];
		int8_t m512x_int8[64]{};
		size_t m512x_uint64[8];
		uint32_t m512x_uint32[16];
		uint16_t m512x_uint16[32];
		uint8_t m512x_uint8[64];
#endif
	};

	template<typename simd_int_t01, size_t... indices> JSONIFIER_ALWAYS_INLINE constexpr uint16_t mm128MovemaskEpi8(simd_int_t01&& a, std::index_sequence<indices...>&&) noexcept {
		uint16_t mask{ 0 };
		((mask |= (a.m128x_int8[indices] & 0x80) ? (1 << indices) : 0), ...);
		return mask;
	}

	template<typename simd_int_t01, typename simd_int_t02> JSONIFIER_ALWAYS_INLINE constexpr __m128x mm128OrSi128(simd_int_t01&& valOne, simd_int_t02&& valTwo) noexcept {
		__m128x value{};
		std::copy(valOne.m128x_uint64, valOne.m128x_uint64 + 2, value.m128x_uint64);
		value.m128x_uint64[0] |= valTwo.m128x_uint64[0];
		value.m128x_uint64[1] |= valTwo.m128x_uint64[1];
		return value;
	}

	template<typename simd_int_t01, typename simd_int_t02> JSONIFIER_ALWAYS_INLINE constexpr __m128x mm128AndSi128(simd_int_t01&& valOne, simd_int_t02&& valTwo) noexcept {
		__m128x value{};
		std::copy(valOne.m128x_uint64, valOne.m128x_uint64 + 2, value.m128x_uint64);
		value.m128x_uint64[0] &= valTwo.m128x_uint64[0];
		value.m128x_uint64[1] &= valTwo.m128x_uint64[1];
		return value;
	}

	template<typename simd_int_t01, typename simd_int_t02> JSONIFIER_ALWAYS_INLINE constexpr __m128x mm128AndNotSi128(simd_int_t01&& valOne, simd_int_t02&& valTwo) noexcept {
		__m128x value{};
		std::copy(valOne.m128x_uint64, valOne.m128x_uint64 + 2, value.m128x_uint64);
		value.m128x_uint64[0] &= ~valOne.m128x_uint64[0];
		value.m128x_uint64[1] &= ~valOne.m128x_uint64[1];
		return value;
	}

	template<typename simd_int_t01, typename simd_int_t02> JSONIFIER_ALWAYS_INLINE constexpr __m128x mm128XorSi128(simd_int_t01&& valOne, simd_int_t02&& valTwo) noexcept {
		__m128x value{};
		std::copy(valOne.m128x_uint64, valOne.m128x_uint64 + 2, value.m128x_uint64);
		value.m128x_uint64[0] ^= valTwo.m128x_uint64[0];
		value.m128x_uint64[1] ^= valTwo.m128x_uint64[1];
		return value;
	}

	template<typename simd_int_t01, typename simd_int_t02, size_t... indices>
	JSONIFIER_ALWAYS_INLINE constexpr __m128x mm128CmpEqEpi8(simd_int_t01&& a, simd_int_t02&& b, std::index_sequence<indices...>&&) noexcept {
		__m128x result{};
		((result.m128x_int8[indices] = (a.m128x_int8[indices] == b.m128x_int8[indices]) ? 0xFF : 0), ...);
		return result;
	}

	template<typename simd_int_t01, typename simd_int_t02> JSONIFIER_ALWAYS_INLINE constexpr bool mm128TestzSi128(simd_int_t01& valOne, simd_int_t02& valTwo) noexcept {
		valOne.m128x_uint64[0] &= valTwo.m128x_uint64[0];
		valOne.m128x_uint64[1] &= valTwo.m128x_uint64[1];
		return valOne.m128x_uint64[0] == 0 && valOne.m128x_uint64[1] == 0;
	}

	JSONIFIER_ALWAYS_INLINE constexpr __m128x mm128SetrEpi64x(size_t argOne, size_t argTwo) noexcept {
		__m128x returnValue{};
		std::copy(&argOne, &argOne + 1, returnValue.m128x_uint64);
		std::copy(&argTwo, &argTwo + 1, returnValue.m128x_uint64 + 1);
		return returnValue;
	}

	JSONIFIER_ALWAYS_INLINE constexpr __m128x mm128Set1Epi64x(size_t argOne) noexcept {
		__m128x returnValue{};
		std::copy(&argOne, &argOne + 1, returnValue.m128x_uint64);
		std::copy(&argOne, &argOne + 1, returnValue.m128x_uint64 + 1);
		return returnValue;
	}

	constexpr __m128x mm128BlendVEpi8(const __m128x& a, const __m128x& b, const __m128x& mask) noexcept {
		__m128x result;
		for (int32_t i = 0; i < 2; ++i) {
			result.m128x_uint64[i] = 0;
			for (int32_t j = 0; j < 8; ++j) {
				uint8_t maskByte	= (mask.m128x_uint64[1 - i] >> (j * 8)) & 0xFF;
				uint8_t aByte		= (a.m128x_uint64[1 - i] >> (j * 8)) & 0xFF;
				uint8_t bByte		= (b.m128x_uint64[1 - i] >> (j * 8)) & 0xFF;
				uint8_t blendedByte = (maskByte ? bByte : aByte);
				result.m128x_uint64[i] |= (static_cast<size_t>(blendedByte) << (j * 8));
			}
		}
		return result;
	}

	constexpr __m128x mm128AddEpi64(const __m128x& value01, const __m128x& value02) noexcept {
		__m128x returnValue{};
		__m128x value01New{ value01 }, value02New{ value02 };
		returnValue.m128x_uint64[0] = value01New.m128x_uint64[0] + value02New.m128x_uint64[0];
		returnValue.m128x_uint64[1] = value01New.m128x_uint64[1] + value02New.m128x_uint64[1];
		return returnValue;
	}

	constexpr void mm128StoreUSi128(uint8_t* ptr, const __m128x& data) noexcept {
		for (int32_t i = 0; i < 8; ++i) {
			ptr[i] = static_cast<uint8_t>(data.m128x_uint64[0] >> (i * 8));
		}

		for (int32_t i = 0; i < 8; ++i) {
			ptr[i + 8] = static_cast<uint8_t>(data.m128x_uint64[1] >> (i * 8));
		}
	}

	template<typename value_type> constexpr void mm128StoreUSi128(value_type* ptr, const __m128x& data) noexcept {
		for (int32_t i = 0; i < 2; ++i) {
			ptr[i] = static_cast<size_t>(data.m128x_uint64[i]);
		}
	}

	template<typename value_type> constexpr uint32_t get32(const value_type* data, int32_t index) noexcept {
		return (data[index / 2] >> (32 * (index % 2))) & 0xFFFFFFFF;
	}

	template<typename value_type> constexpr void set32(value_type* data, int32_t index, uint32_t value) noexcept {
		data[index / 2] &= ~(0xFFFFFFFFull << (32 * (index % 2)));
		data[index / 2] |= (static_cast<size_t>(value) << (32 * (index % 2)));
	}

	template<typename value_type> constexpr void set64(value_type* data, int32_t index, size_t value) noexcept {
		data[index] = value;
	}

	constexpr __m128x mm128ShuffleEpi32(const __m128x& a, int32_t imm8) noexcept {
		__m128x result{};
		set32(result.m128x_uint64, 0, get32(a.m128x_uint64, (imm8 >> 0) & 0x3));
		set32(result.m128x_uint64, 1, get32(a.m128x_uint64, (imm8 >> 2) & 0x3));
		set32(result.m128x_uint64, 2, get32(a.m128x_uint64, (imm8 >> 4) & 0x3));
		set32(result.m128x_uint64, 3, get32(a.m128x_uint64, (imm8 >> 6) & 0x3));
		return result;
	}

	constexpr __m128x mm128MulEpi32(const __m128x& a, const __m128x& b) noexcept {
		__m128x result{};

		uint32_t a_val[4];
		uint32_t b_val[4];

		for (int32_t i = 0; i < 4; ++i) {
			a_val[i] = get32(a.m128x_uint64, i);
			b_val[i] = get32(b.m128x_uint64, i);
		}

		set64(result.m128x_uint64, 0, static_cast<size_t>(a_val[0]) * static_cast<size_t>(b_val[0]) | static_cast<size_t>(a_val[1]) * static_cast<size_t>(b_val[1]));
		set64(result.m128x_uint64, 1, static_cast<size_t>(a_val[2]) * static_cast<size_t>(b_val[2]) | static_cast<size_t>(a_val[3]) * static_cast<size_t>(b_val[3]));

		return result;
	}

	constexpr __m128x mm128SubEpi64(const __m128x& value01, const __m128x& value02) noexcept {
		__m128x returnValue{};
		returnValue.m128x_uint64[0] = value01.m128x_uint64[0] - value02.m128x_uint64[0];
		returnValue.m128x_uint64[1] = value01.m128x_uint64[1] - value02.m128x_uint64[1];
		return returnValue;
	}

	constexpr __m128x mm128SrliEpi64(const __m128x& a, uint32_t imm8) noexcept {
		__m128x result{};
		result.m128x_uint64[0] = a.m128x_uint64[0] >> imm8;
		result.m128x_uint64[1] = a.m128x_uint64[1] >> imm8;
		return result;
	}

	constexpr __m128x mm128SlliEpi64(const __m128x& a, uint32_t imm8) noexcept {
		__m128x result{};
		result.m128x_uint64[0] = a.m128x_uint64[0] << imm8;
		result.m128x_uint64[1] = a.m128x_uint64[1] << imm8;
		return result;
	}

	constexpr __m128x mm128Set1Epi32(uint32_t value) noexcept {
		size_t extended_value = (static_cast<size_t>(value) << 32) | value;
		return __m128x{ extended_value, extended_value };
	}

	template<typename value_type> constexpr __m128x mm128LoadUSi128(const value_type* ptr) noexcept {
		size_t low	= 0;
		size_t high = 0;

		for (int32_t i = 0; i < 8; ++i) {
			low |= static_cast<const size_t>(ptr[i]) << (i * 8);
		}

		for (int32_t i = 0; i < 8; ++i) {
			high |= static_cast<const size_t>(ptr[i + 8]) << (i * 8);
		}
		__m128x returnValues{};
		returnValues.m128x_uint64[0] = low;
		returnValues.m128x_uint64[1] = high;
		return returnValues;
	}

	template<typename value_type>
		requires(sizeof(value_type) == 8)
	constexpr __m128x mm128LoadUSi128(const value_type* ptr) noexcept {
		__m128x returnValues{};
		returnValues.m128x_uint64[0] = ptr[0];
		returnValues.m128x_uint64[1] = ptr[1];
		return returnValues;
	}

	constexpr __m128x mm128LoadUSi128(const __m128x* ptr) noexcept {
		__m128x returnValues{ *ptr };
		return returnValues;
	}

	constexpr __m128x mm128SetZero() noexcept {
		return __m128x{};
	}

	template<typename simd_int_t01, typename simd_int_t02, size_t... indices>
	JSONIFIER_ALWAYS_INLINE constexpr __m128x mm128ShuffleEpi8(simd_int_t01&& a, simd_int_t02&& b, std::index_sequence<indices...>) noexcept {
		__m128x result{};
		size_t index{};
		(((index = b.m128x_uint8[indices] & 0x0F), (result.m128x_uint8[indices] = a.m128x_uint8[index])), ...);
		return result;
	}

	template<typename simd_int_t01, typename simd_int_t02> JSONIFIER_ALWAYS_INLINE constexpr __m256x mm256AndSi256(simd_int_t01&& valOne, simd_int_t02&& valTwo) noexcept {
		__m256x value{};
		memcpy(value.m256x_uint64, valOne.m256x_uint64, sizeof(value));
		value.m256x_uint64[0] &= valTwo.m256x_uint64[0];
		value.m256x_uint64[1] &= valTwo.m256x_uint64[1];
		value.m256x_uint64[2] &= valTwo.m256x_uint64[2];
		value.m256x_uint64[3] &= valTwo.m256x_uint64[3];
		return value;
	}

	template<typename simd_int_t01, typename simd_int_t02> JSONIFIER_ALWAYS_INLINE constexpr __m256x mm256XorSi256(simd_int_t01&& valOne, simd_int_t02&& valTwo) noexcept {
		__m256x value{};
		std::copy(valOne.m256x_uint64, valOne.m256x_uint64 + 4, value.m256x_uint64);
		value.m256x_uint64[0] ^= valTwo.m256x_uint64[0];
		value.m256x_uint64[1] ^= valTwo.m256x_uint64[1];
		value.m256x_uint64[2] ^= valTwo.m256x_uint64[2];
		value.m256x_uint64[3] ^= valTwo.m256x_uint64[3];
		return value;
	}

	constexpr __m256x mm256SetrEpi64x(size_t argOne, size_t argTwo, size_t argThree, size_t argFour) noexcept {
		__m256x returnValue{};
		returnValue.m256x_uint64[0] = argFour;
		returnValue.m256x_uint64[1] = argThree;
		returnValue.m256x_uint64[2] = argTwo;
		returnValue.m256x_uint64[3] = argOne;
		return returnValue;
	}

	constexpr __m256x mm256AddEpi64(const __m256x& value01, const __m256x& value02) noexcept {
		__m256x returnValue{};
		__m256x value01New{ value01 }, value02New{ value02 };
		returnValue.m256x_uint64[0] = value01New.m256x_uint64[0] + value02New.m256x_uint64[0];
		returnValue.m256x_uint64[1] = value01New.m256x_uint64[1] + value02New.m256x_uint64[1];
		returnValue.m256x_uint64[2] = value01New.m256x_uint64[2] + value02New.m256x_uint64[2];
		returnValue.m256x_uint64[3] = value01New.m256x_uint64[3] + value02New.m256x_uint64[3];
		return returnValue;
	}

	constexpr void mm256StoreUSi256(uint8_t* ptr, const __m256x& data) noexcept {
		for (int32_t i = 0; i < 8; ++i) {
			ptr[i] = static_cast<uint8_t>(data.m256x_uint64[0] >> (i * 8));
		}

		for (int32_t i = 0; i < 8; ++i) {
			ptr[i + 8] = static_cast<uint8_t>(data.m256x_uint64[1] >> (i * 8));
		}

		for (int32_t i = 0; i < 8; ++i) {
			ptr[i + 16] = static_cast<uint8_t>(data.m256x_uint64[2] >> (i * 8));
		}

		for (int32_t i = 0; i < 8; ++i) {
			ptr[i + 24] = static_cast<uint8_t>(data.m256x_uint64[3] >> (i * 8));
		}
	}

	template<typename value_type> constexpr void mm256StoreUSi256(value_type* ptr, const __m256x& data) noexcept {
		for (int32_t i = 0; i < 4; ++i) {
			ptr[i] = static_cast<size_t>(data.m256x_uint64[i]);
		}
	}

	constexpr __m256x mm256ShuffleEpi32(const __m256x& a, int32_t imm8) noexcept {
		__m256x result{};
		set32(result.m256x_uint64, 0, get32(a.m256x_uint64, (imm8 >> 0) & 0x3));
		set32(result.m256x_uint64, 1, get32(a.m256x_uint64, (imm8 >> 2) & 0x3));
		set32(result.m256x_uint64, 2, get32(a.m256x_uint64, (imm8 >> 4) & 0x3));
		set32(result.m256x_uint64, 3, get32(a.m256x_uint64, (imm8 >> 6) & 0x3));
		set32(result.m256x_uint64, 4, get32(a.m256x_uint64, (imm8 >> 8) & 0x3));
		set32(result.m256x_uint64, 5, get32(a.m256x_uint64, (imm8 >> 10) & 0x3));
		set32(result.m256x_uint64, 6, get32(a.m256x_uint64, (imm8 >> 12) & 0x3));
		set32(result.m256x_uint64, 7, get32(a.m256x_uint64, (imm8 >> 14) & 0x3));
		return result;
	}

	constexpr __m256x mm256MulEpi32(const __m256x& a, const __m256x& b) noexcept {
		__m256x result{};

		uint32_t a_val[8];
		uint32_t b_val[8];
		for (int32_t i = 0; i < 8; ++i) {
			a_val[i] = get32(a.m256x_uint64, i);
			b_val[i] = get32(b.m256x_uint64, i);
		}
		set64(result.m256x_uint64, 0, static_cast<size_t>(a_val[0]) * static_cast<size_t>(b_val[0]) | static_cast<size_t>(a_val[1]) * static_cast<size_t>(b_val[1]));
		set64(result.m256x_uint64, 1, static_cast<size_t>(a_val[2]) * static_cast<size_t>(b_val[2]) | static_cast<size_t>(a_val[3]) * static_cast<size_t>(b_val[3]));
		set64(result.m256x_uint64, 2, static_cast<size_t>(a_val[4]) * static_cast<size_t>(b_val[4]) | static_cast<size_t>(a_val[5]) * static_cast<size_t>(b_val[5]));
		set64(result.m256x_uint64, 3, static_cast<size_t>(a_val[6]) * static_cast<size_t>(b_val[6]) | static_cast<size_t>(a_val[7]) * static_cast<size_t>(b_val[7]));

		return result;
	}

	constexpr __m256x mm256SubEpi64(const __m256x& value01, const __m256x& value02) noexcept {
		__m256x returnValue{};
		returnValue.m256x_uint64[0] = value01.m256x_uint64[0] - value02.m256x_uint64[0];
		returnValue.m256x_uint64[1] = value01.m256x_uint64[1] - value02.m256x_uint64[1];
		returnValue.m256x_uint64[2] = value01.m256x_uint64[2] - value02.m256x_uint64[2];
		returnValue.m256x_uint64[3] = value01.m256x_uint64[3] - value02.m256x_uint64[3];
		return returnValue;
	}

	constexpr __m256x mm256SrliEpi64(const __m256x& a, uint32_t imm8) noexcept {
		__m256x result{};
		result.m256x_uint64[0] = a.m256x_uint64[0] >> imm8;
		result.m256x_uint64[1] = a.m256x_uint64[1] >> imm8;
		result.m256x_uint64[2] = a.m256x_uint64[2] >> imm8;
		result.m256x_uint64[3] = a.m256x_uint64[3] >> imm8;
		return result;
	}

	constexpr __m256x mm256SlliEpi64(const __m256x& a, uint32_t imm8) noexcept {
		__m256x result{};
		result.m256x_uint64[0] = a.m256x_uint64[0] << imm8;
		result.m256x_uint64[1] = a.m256x_uint64[1] << imm8;
		result.m256x_uint64[2] = a.m256x_uint64[2] << imm8;
		result.m256x_uint64[3] = a.m256x_uint64[3] << imm8;
		return result;
	}

	constexpr __m256x mm256Set1Epi32(uint32_t value) noexcept {
		size_t extended_value = (static_cast<size_t>(value) << 32) | value;
		return __m256x{ extended_value, extended_value, extended_value, extended_value };
	}

	template<typename value_type> constexpr __m256x mm256LoadUSi256(const value_type* ptr) noexcept {
		size_t low01  = 0;
		size_t low02  = 0;
		size_t high01 = 0;
		size_t high02 = 0;

		for (int32_t i = 0; i < 8; ++i) {
			low01 |= static_cast<const size_t>(ptr[i]) << (i * 8);
		}

		for (int32_t i = 0; i < 8; ++i) {
			low02 |= static_cast<const size_t>(ptr[i + 8]) << (i * 8);
		}

		for (int32_t i = 0; i < 8; ++i) {
			high01 |= static_cast<const size_t>(ptr[i + 16]) << (i * 8);
		}

		for (int32_t i = 0; i < 8; ++i) {
			high02 |= static_cast<const size_t>(ptr[i + 24]) << (i * 8);
		}
		__m256x returnValues{};
		returnValues.m256x_uint64[0] = low01;
		returnValues.m256x_uint64[1] = low02;
		returnValues.m256x_uint64[2] = high01;
		returnValues.m256x_uint64[3] = high02;
		return returnValues;
	}

	template<typename value_type>
		requires(sizeof(value_type) == 8)
	constexpr __m256x mm256LoadUSi256(const value_type* ptr) noexcept {
		__m256x returnValues{};
		returnValues.m256x_uint64[0] = ptr[0];
		returnValues.m256x_uint64[1] = ptr[1];
		returnValues.m256x_uint64[2] = ptr[2];
		returnValues.m256x_uint64[3] = ptr[3];
		return returnValues;
	}

	constexpr __m256x mm256BlendVEpi8(const __m256x& a, const __m256x& b, const __m256x& mask) noexcept {
		__m256x result;
		for (int32_t i = 0; i < 2; ++i) {
			result.m256x_uint64[i] = 0;
			for (int32_t j = 0; j < 8; ++j) {
				uint8_t maskByte	= (mask.m256x_uint64[1 - i] >> (j * 8)) & 0xFF;
				uint8_t aByte		= (a.m256x_uint64[1 - i] >> (j * 8)) & 0xFF;
				uint8_t bByte		= (b.m256x_uint64[1 - i] >> (j * 8)) & 0xFF;
				uint8_t blendedByte = (maskByte ? bByte : aByte);
				result.m256x_uint64[i] |= (static_cast<size_t>(blendedByte) << (j * 8));
			}
		}
		return result;
	}

	constexpr __m256x mm256LoadUSi256(const __m256x* ptr) noexcept {
		__m256x returnValues{ *ptr };
		return returnValues;
	}

	template<typename value_type>
		requires(sizeof(value_type) == 1)
	constexpr __m512x mm512LoadUSi512(const value_type* ptr) noexcept {
		size_t low00  = 0;
		size_t low01  = 0;
		size_t low02  = 0;
		size_t low03  = 0;
		size_t high00 = 0;
		size_t high01 = 0;
		size_t high02 = 0;
		size_t high03 = 0;

		for (int32_t i = 0; i < 8; ++i) {
			low00 |= static_cast<const size_t>(ptr[i]) << (i * 8);
		}

		for (int32_t i = 0; i < 8; ++i) {
			low01 |= static_cast<const size_t>(ptr[i + 8]) << (i * 8);
		}

		for (int32_t i = 0; i < 8; ++i) {
			low02 |= static_cast<const size_t>(ptr[i + 16]) << (i * 8);
		}

		for (int32_t i = 0; i < 8; ++i) {
			low03 |= static_cast<const size_t>(ptr[i + 24]) << (i * 8);
		}

		for (int32_t i = 0; i < 8; ++i) {
			high00 |= static_cast<const size_t>(ptr[i + 32]) << (i * 8);
		}

		for (int32_t i = 0; i < 8; ++i) {
			high01 |= static_cast<const size_t>(ptr[i + 40]) << (i * 8);
		}

		for (int32_t i = 0; i < 8; ++i) {
			high02 |= static_cast<const size_t>(ptr[i + 48]) << (i * 8);
		}

		for (int32_t i = 0; i < 8; ++i) {
			high03 |= static_cast<const size_t>(ptr[i + 56]) << (i * 8);
		}
		__m512x returnValues{};
		returnValues.m512x_uint64[0] = low00;
		returnValues.m512x_uint64[1] = low01;
		returnValues.m512x_uint64[2] = low02;
		returnValues.m512x_uint64[3] = low03;
		returnValues.m512x_uint64[4] = high00;
		returnValues.m512x_uint64[5] = high01;
		returnValues.m512x_uint64[6] = high02;
		returnValues.m512x_uint64[7] = high03;
		return returnValues;
	}

	template<typename simd_int_t01, typename simd_int_t02> JSONIFIER_ALWAYS_INLINE constexpr __m512x mm512XorSi512(simd_int_t01&& valOne, simd_int_t02&& valTwo) noexcept {
		__m512x value{};
		std::copy(valOne.m512x_uint64, valOne.m512x_uint64 + 8, value.m512x_uint64);
		value.m512x_uint64[0] ^= valTwo.m512x_uint64[0];
		value.m512x_uint64[1] ^= valTwo.m512x_uint64[1];
		value.m512x_uint64[2] ^= valTwo.m512x_uint64[2];
		value.m512x_uint64[3] ^= valTwo.m512x_uint64[3];
		value.m512x_uint64[4] ^= valTwo.m512x_uint64[4];
		value.m512x_uint64[5] ^= valTwo.m512x_uint64[5];
		value.m512x_uint64[6] ^= valTwo.m512x_uint64[6];
		value.m512x_uint64[7] ^= valTwo.m512x_uint64[7];
		return value;
	}

	constexpr __m512x mm512SrliEpi64(const __m512x& a, uint32_t imm8) noexcept {
		__m512x result{};
		result.m512x_uint64[0] = a.m512x_uint64[0] >> imm8;
		result.m512x_uint64[1] = a.m512x_uint64[1] >> imm8;
		result.m512x_uint64[2] = a.m512x_uint64[2] >> imm8;
		result.m512x_uint64[3] = a.m512x_uint64[3] >> imm8;
		result.m512x_uint64[4] = a.m512x_uint64[4] >> imm8;
		result.m512x_uint64[5] = a.m512x_uint64[5] >> imm8;
		result.m512x_uint64[6] = a.m512x_uint64[6] >> imm8;
		result.m512x_uint64[7] = a.m512x_uint64[7] >> imm8;
		return result;
	}

	constexpr __m512x mm512MulEpi32(const __m512x& a, const __m512x& b) noexcept {
		__m512x result{};

		uint32_t a_val[16];
		uint32_t b_val[16];
		for (int32_t i = 0; i < 16; ++i) {
			a_val[i] = get32(a.m512x_uint64, i);
			b_val[i] = get32(b.m512x_uint64, i);
		}
		set64(result.m512x_uint64, 0, static_cast<size_t>(a_val[0]) * static_cast<size_t>(b_val[0]) | static_cast<size_t>(a_val[1]) * static_cast<size_t>(b_val[1]));
		set64(result.m512x_uint64, 1, static_cast<size_t>(a_val[2]) * static_cast<size_t>(b_val[2]) | static_cast<size_t>(a_val[3]) * static_cast<size_t>(b_val[3]));
		set64(result.m512x_uint64, 2, static_cast<size_t>(a_val[4]) * static_cast<size_t>(b_val[4]) | static_cast<size_t>(a_val[5]) * static_cast<size_t>(b_val[5]));
		set64(result.m512x_uint64, 3, static_cast<size_t>(a_val[6]) * static_cast<size_t>(b_val[6]) | static_cast<size_t>(a_val[7]) * static_cast<size_t>(b_val[7]));
		set64(result.m512x_uint64, 4, static_cast<size_t>(a_val[8]) * static_cast<size_t>(b_val[8]) | static_cast<size_t>(a_val[9]) * static_cast<size_t>(b_val[9]));
		set64(result.m512x_uint64, 5, static_cast<size_t>(a_val[10]) * static_cast<size_t>(b_val[10]) | static_cast<size_t>(a_val[11]) * static_cast<size_t>(b_val[11]));
		set64(result.m512x_uint64, 6, static_cast<size_t>(a_val[12]) * static_cast<size_t>(b_val[12]) | static_cast<size_t>(a_val[13]) * static_cast<size_t>(b_val[13]));
		set64(result.m512x_uint64, 7, static_cast<size_t>(a_val[14]) * static_cast<size_t>(b_val[14]) | static_cast<size_t>(a_val[15]) * static_cast<size_t>(b_val[15]));

		return result;
	}

	constexpr __m512x mm512ShuffleEpi32(const __m512x& a, int32_t imm8) noexcept {
		__m512x result{};
		set32(result.m512x_uint64, 0, get32(a.m512x_uint64, (imm8 >> 0) & 0x3));
		set32(result.m512x_uint64, 1, get32(a.m512x_uint64, (imm8 >> 2) & 0x3));
		set32(result.m512x_uint64, 2, get32(a.m512x_uint64, (imm8 >> 4) & 0x3));
		set32(result.m512x_uint64, 3, get32(a.m512x_uint64, (imm8 >> 6) & 0x3));
		set32(result.m512x_uint64, 4, get32(a.m512x_uint64, (imm8 >> 8) & 0x3));
		set32(result.m512x_uint64, 5, get32(a.m512x_uint64, (imm8 >> 10) & 0x3));
		set32(result.m512x_uint64, 6, get32(a.m512x_uint64, (imm8 >> 12) & 0x3));
		set32(result.m512x_uint64, 7, get32(a.m512x_uint64, (imm8 >> 14) & 0x3));
		set32(result.m512x_uint64, 8, get32(a.m512x_uint64, (imm8 >> 16) & 0x3));
		set32(result.m512x_uint64, 9, get32(a.m512x_uint64, (imm8 >> 18) & 0x3));
		set32(result.m512x_uint64, 10, get32(a.m512x_uint64, (imm8 >> 20) & 0x3));
		set32(result.m512x_uint64, 11, get32(a.m512x_uint64, (imm8 >> 22) & 0x3));
		set32(result.m512x_uint64, 12, get32(a.m512x_uint64, (imm8 >> 24) & 0x3));
		set32(result.m512x_uint64, 13, get32(a.m512x_uint64, (imm8 >> 26) & 0x3));
		set32(result.m512x_uint64, 14, get32(a.m512x_uint64, (imm8 >> 28) & 0x3));
		set32(result.m512x_uint64, 15, get32(a.m512x_uint64, (imm8 >> 30) & 0x3));
		return result;
	}

	constexpr __m512x mm512AddEpi64(const __m512x& value01, const __m512x& value02) noexcept {
		__m512x returnValue{};
		__m512x value01New{ value01 }, value02New{ value02 };
		returnValue.m512x_uint64[0] = value01New.m512x_uint64[0] + value02New.m512x_uint64[0];
		returnValue.m512x_uint64[1] = value01New.m512x_uint64[1] + value02New.m512x_uint64[1];
		returnValue.m512x_uint64[2] = value01New.m512x_uint64[2] + value02New.m512x_uint64[2];
		returnValue.m512x_uint64[3] = value01New.m512x_uint64[3] + value02New.m512x_uint64[3];
		returnValue.m512x_uint64[4] = value01New.m512x_uint64[4] + value02New.m512x_uint64[4];
		returnValue.m512x_uint64[5] = value01New.m512x_uint64[5] + value02New.m512x_uint64[5];
		returnValue.m512x_uint64[6] = value01New.m512x_uint64[6] + value02New.m512x_uint64[6];
		returnValue.m512x_uint64[7] = value01New.m512x_uint64[7] + value02New.m512x_uint64[7];
		return returnValue;
	}

	template<typename value_type>
		requires(sizeof(value_type) == 8)
	constexpr __m512x mm512LoadUSi512(const value_type* ptr) noexcept {
		__m512x returnValues{};
		returnValues.m512x_uint64[0] = ptr[0];
		returnValues.m512x_uint64[1] = ptr[1];
		returnValues.m512x_uint64[2] = ptr[2];
		returnValues.m512x_uint64[3] = ptr[3];
		returnValues.m512x_uint64[4] = ptr[4];
		returnValues.m512x_uint64[5] = ptr[5];
		returnValues.m512x_uint64[6] = ptr[6];
		returnValues.m512x_uint64[7] = ptr[7];
		return returnValues;
	}

	constexpr void mm512StoreUSi512(uint8_t* ptr, const __m512x& data) noexcept {
		for (int32_t i = 0; i < 8; ++i) {
			ptr[i] = static_cast<uint8_t>(data.m512x_uint64[0] >> (i * 8));
		}

		for (int32_t i = 0; i < 8; ++i) {
			ptr[i + 8] = static_cast<uint8_t>(data.m512x_uint64[1] >> (i * 8));
		}

		for (int32_t i = 0; i < 8; ++i) {
			ptr[i + 16] = static_cast<uint8_t>(data.m512x_uint64[2] >> (i * 8));
		}

		for (int32_t i = 0; i < 8; ++i) {
			ptr[i + 24] = static_cast<uint8_t>(data.m512x_uint64[3] >> (i * 8));
		}

		for (int32_t i = 0; i < 8; ++i) {
			ptr[i + 32] = static_cast<uint8_t>(data.m512x_uint64[4] >> (i * 8));
		}

		for (int32_t i = 0; i < 8; ++i) {
			ptr[i + 40] = static_cast<uint8_t>(data.m512x_uint64[5] >> (i * 8));
		}

		for (int32_t i = 0; i < 8; ++i) {
			ptr[i + 48] = static_cast<uint8_t>(data.m512x_uint64[6] >> (i * 8));
		}

		for (int32_t i = 0; i < 8; ++i) {
			ptr[i + 56] = static_cast<uint8_t>(data.m512x_uint64[7] >> (i * 8));
		}
	}

	template<typename value_type> constexpr void mm512StoreUSi512(value_type* ptr, const __m512x& data) noexcept {
		for (int32_t i = 0; i < 8; ++i) {
			ptr[i] = static_cast<size_t>(data.m512x_uint64[i]);
		}
	}

	constexpr __m512x mm512Set1Epi32(uint32_t value) noexcept {
		size_t extended_value = (static_cast<size_t>(value) << 32) | value;
		return __m512x{ extended_value, extended_value, extended_value, extended_value, extended_value, extended_value, extended_value, extended_value };
	}

	constexpr __m512x mm512SlliEpi64(const __m512x& a, uint32_t imm8) noexcept {
		__m512x result{};
		result.m512x_uint64[0] = a.m512x_uint64[0] << imm8;
		result.m512x_uint64[1] = a.m512x_uint64[1] << imm8;
		result.m512x_uint64[2] = a.m512x_uint64[2] << imm8;
		result.m512x_uint64[3] = a.m512x_uint64[3] << imm8;
		result.m512x_uint64[4] = a.m512x_uint64[4] << imm8;
		result.m512x_uint64[5] = a.m512x_uint64[5] << imm8;
		result.m512x_uint64[6] = a.m512x_uint64[6] << imm8;
		result.m512x_uint64[7] = a.m512x_uint64[7] << imm8;
		return result;
	}

	constexpr __m512x mm512TernarylogicEpi32(const __m512x& a, const __m512x& b, const __m512x& c, const uint8_t k) noexcept {
		__m512x result;

		for (int32_t i = 0; i < 8; ++i) {
			size_t val_a = a.m512x_uint64[i];
			size_t val_b = b.m512x_uint64[i];
			size_t val_c = c.m512x_uint64[i];

			size_t result_val = 0;

			for (int32_t bit = 0; bit < 64; ++bit) {
				uint8_t bit_a = (val_a >> bit) & 1;
				uint8_t bit_b = (val_b >> bit) & 1;
				uint8_t bit_c = (val_c >> bit) & 1;

				uint8_t index	   = static_cast<uint8_t>((bit_a << 2) | (bit_b << 1) | bit_c);
				uint8_t result_bit = static_cast<uint8_t>((k >> index) & 1);

				result_val |= (static_cast<size_t>(result_bit) << bit);
			}

			result.m512x_uint64[i] = result_val;
		}

		return result;
	}

	constexpr __m512x mm512MaskSubEpi64(const __m512x& a, uint8_t mask, const __m512x& b, const __m512x& c) noexcept {
		__m512x result;

		for (int32_t i = 0; i < 8; ++i) {
			if (mask & (1 << i)) {
				result.m512x_uint64[i] = b.m512x_uint64[i] - c.m512x_uint64[i];
			} else {
				result.m512x_uint64[i] = a.m512x_uint64[i];
			}
		}

		return result;
	}

	constexpr __m512x mm512Set1Epi64(size_t argOne) noexcept {
		__m512x returnValue{};
		returnValue.m512x_uint64[0] = argOne;
		returnValue.m512x_uint64[1] = argOne;
		returnValue.m512x_uint64[2] = argOne;
		returnValue.m512x_uint64[3] = argOne;
		returnValue.m512x_uint64[4] = argOne;
		returnValue.m512x_uint64[5] = argOne;
		returnValue.m512x_uint64[6] = argOne;
		returnValue.m512x_uint64[7] = argOne;
		return returnValue;
	}

	constexpr __m512x mm512Set1Epi8(uint8_t value) noexcept {
		__m512x result;

		size_t repeated_value = 0;
		for (int32_t i = 0; i < 8; ++i) {
			repeated_value |= (static_cast<size_t>(value) << (i * 8));
		}

		for (int32_t i = 0; i < 8; ++i) {
			result.m512x_uint64[i] = repeated_value;
		}

		return result;
	}

	constexpr __m512x mm512LoadUSi512(const __m512x* ptr) noexcept {
		__m512x returnValues{ *ptr };
		return returnValues;
	}

}