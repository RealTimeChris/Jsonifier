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

#include <type_traits>
#include <algorithm>
#include <cstdint>
#include <utility>

namespace simd_internal {

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

	template<typename simd_int_t01, size_t... indices> constexpr uint16_t mm128MovemaskEpi8Impl(const simd_int_t01& a, std::index_sequence<indices...>&&) noexcept {
		uint16_t mask{};
		((mask |= (a.m128x_int8[indices] & 0x80) ? (1 << indices) : 0), ...);
		return mask;
	}

	template<typename simd_int_t01> constexpr uint16_t mm128MovemaskEpi8(const simd_int_t01& a) noexcept {
		return mm128MovemaskEpi8Impl(a, std::make_index_sequence<16>{});
	}

	constexpr __m128x mm128AddEpi8(const __m128x& a, const __m128x& b) noexcept {
		__m128x result;
		for (int i = 0; i < 16; ++i) {
			result.m128x_int8[i] = static_cast<int8_t>(a.m128x_int8[i] + b.m128x_int8[i]);
		}
		return result;
	}

	constexpr __m128x mm128CmpGtEpi8(const __m128x& a, const __m128x& b) noexcept {
		__m128x result;

		for (size_t i = 0; i < 16; ++i) {
			result.m128x_uint8[i] = static_cast<uint8_t>(static_cast<int8_t>(a.m128x_uint8[i]) > static_cast<int8_t>(b.m128x_uint8[i]) ? 0xFF : 0x00);
		}

		return result;
	}

	constexpr __m128x mm128Set1Epi8(uint8_t value) noexcept {
		__m128x result;

		size_t repeated_value = 0;
		for (int32_t i = 0; i < 8; ++i) {
			repeated_value |= (static_cast<size_t>(value) << (i * 8));
		}

		for (int32_t i = 0; i < 2; ++i) {
			result.m128x_uint64[i] = repeated_value;
		}

		return result;
	}

	template<typename simd_int_t01, typename simd_int_t02> constexpr __m128x mm128OrSi128(const simd_int_t01& valOne, const simd_int_t02& valTwo) noexcept {
		__m128x value{};
		std::copy(valOne.m128x_uint64, valOne.m128x_uint64 + 2, value.m128x_uint64);
		value.m128x_uint64[0] |= valTwo.m128x_uint64[0];
		value.m128x_uint64[1] |= valTwo.m128x_uint64[1];
		return value;
	}

	template<typename simd_int_t01, typename simd_int_t02> constexpr __m128x mm128AndSi128(const simd_int_t01& valOne, const simd_int_t02& valTwo) noexcept {
		__m128x value{};
		std::copy(valOne.m128x_uint64, valOne.m128x_uint64 + 2, value.m128x_uint64);
		value.m128x_uint64[0] &= valTwo.m128x_uint64[0];
		value.m128x_uint64[1] &= valTwo.m128x_uint64[1];
		return value;
	}

	template<typename simd_int_t01, typename simd_int_t02> constexpr __m128x mm128AndNotSi128(const simd_int_t01& valOne, const simd_int_t02& valTwo) noexcept {
		__m128x value{};
		std::copy(valOne.m128x_uint64, valOne.m128x_uint64 + 2, value.m128x_uint64);
		value.m128x_uint64[0] &= ~valOne.m128x_uint64[0];
		value.m128x_uint64[1] &= ~valOne.m128x_uint64[1];
		return value;
	}

	template<typename simd_int_t01, typename simd_int_t02> constexpr __m128x mm128XorSi128(const simd_int_t01& valOne, const simd_int_t02& valTwo) noexcept {
		__m128x value{};
		std::copy(valOne.m128x_uint64, valOne.m128x_uint64 + 2, value.m128x_uint64);
		value.m128x_uint64[0] ^= valTwo.m128x_uint64[0];
		value.m128x_uint64[1] ^= valTwo.m128x_uint64[1];
		return value;
	}

	template<typename simd_int_t01, typename simd_int_t02, size_t... indices>
	constexpr __m128x mm128CmpEqEpi8Impl(const simd_int_t01& a, const simd_int_t02& b, std::index_sequence<indices...>&&) noexcept {
		__m128x result{};
		((result.m128x_int8[indices] = (a.m128x_int8[indices] == b.m128x_int8[indices]) ? 0xFF : 0), ...);
		return result;
	}

	template<typename simd_int_t01, typename simd_int_t02> constexpr __m128x mm128CmpEqEpi8(const simd_int_t01& a, const simd_int_t02& b) noexcept {
		return mm128CmpEqEpi8Impl(a, b, std::make_index_sequence<16>{});
	}

	template<typename simd_int_t01, typename simd_int_t02> constexpr bool mm128TestzSi128(simd_int_t01& valOneNew, simd_int_t02& valTwo) noexcept {
		std::remove_const_t<simd_int_t01> valOne{ valOneNew };
		valOne.m128x_uint64[0] &= valTwo.m128x_uint64[0];
		valOne.m128x_uint64[1] &= valTwo.m128x_uint64[1];
		return valOne.m128x_uint64[0] == 0 && valOne.m128x_uint64[1] == 0;
	}

	constexpr __m128x mm128SetEpi64x(size_t argOne, size_t argTwo) noexcept {
		__m128x returnValue{};
		std::copy(&argOne, &argOne + 1, returnValue.m128x_uint64);
		std::copy(&argTwo, &argTwo + 1, returnValue.m128x_uint64 + 1);
		return returnValue;
	}

	constexpr __m128x mm128Set1Epi64x(size_t argOne) noexcept {
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

		uint32_t aVal[4];
		uint32_t bVal[4];

		for (int32_t i = 0; i < 4; ++i) {
			aVal[i] = get32(a.m128x_uint64, i);
			bVal[i] = get32(b.m128x_uint64, i);
		}

		set64(result.m128x_uint64, 0, static_cast<size_t>(aVal[0]) * static_cast<size_t>(bVal[0]) | static_cast<size_t>(aVal[1]) * static_cast<size_t>(bVal[1]));
		set64(result.m128x_uint64, 1, static_cast<size_t>(aVal[2]) * static_cast<size_t>(bVal[2]) | static_cast<size_t>(aVal[3]) * static_cast<size_t>(bVal[3]));

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
		size_t extendedValue = (static_cast<size_t>(value) << 32) | value;
		return __m128x{ extendedValue, extendedValue };
	}

	template<typename value_type> constexpr __m128x mm128LoadUSi128(const __m128x* ptr) noexcept {
		return *ptr;
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
	constexpr __m128x mm128ShuffleEpi8(const simd_int_t01& a, const simd_int_t02& b, std::index_sequence<indices...>) noexcept {
		__m128x result{};
		size_t index{};
		(((index = b.m128x_uint8[indices] & 0x0F), (result.m128x_uint8[indices] = a.m128x_uint8[index])), ...);
		return result;
	}

}