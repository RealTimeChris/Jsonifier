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

#include <jsonifier/ISADetection/ISADetectionBase.hpp>

namespace jsonifier_internal {

#if !JSONIFIER_CHECK_FOR_INSTRUCTION(JSONIFIER_AVX) && !JSONIFIER_CHECK_FOR_INSTRUCTION(JSONIFIER_AVX2) && !JSONIFIER_CHECK_FOR_INSTRUCTION(JSONIFIER_AVX512)

	template<uint64_t index = 0, uint64_t index02 = 0> jsonifier_inline void movemaskEpi8Helper02(int16_t& result, const simd_int_t& a) {
		if jsonifier_constexpr (index < 8) {
			uint8_t value = (a.m128x_uint64[index02] >> (index * 8)) & 0xff;
			int16_t mask  = (value >> 7) & 1;
			result |= mask << (index02 * 8 + index);
			movemaskEpi8Helper02<index + 1, index02>(result, a);
		}
	}

	template<uint64_t index = 0> jsonifier_inline int16_t movemaskEpi8(const simd_int_t& a) {
		int16_t result{};
		if jsonifier_constexpr (index < 2) {
			movemaskEpi8Helper02<0, index>(result, a);
			result |= movemaskEpi8<index + 1>(a);
		}
		return result;
	}

	jsonifier_inline simd_int_t orSi128(const simd_int_t& valOne, const simd_int_t& valTwo) {
		simd_int_t value{};
		value.m128x_uint64[0] = valOne.m128x_uint64[0] | valTwo.m128x_uint64[0];
		value.m128x_uint64[1] = valOne.m128x_uint64[1] | valTwo.m128x_uint64[1];
		return value;
	}

	jsonifier_inline simd_int_t andSi128(const simd_int_t& valOne, const simd_int_t& valTwo) {
		simd_int_t value{};
		value.m128x_uint64[0] = valOne.m128x_uint64[0] & valTwo.m128x_uint64[0];
		value.m128x_uint64[1] = valOne.m128x_uint64[1] & valTwo.m128x_uint64[1];
		return value;
	}

	jsonifier_inline simd_int_t andNotSi128(const simd_int_t& valOne, const simd_int_t& valTwo) {
		simd_int_t result{};
		result.m128x_uint64[0] = valTwo.m128x_uint64[0] & (~valOne.m128x_uint64[0]);
		result.m128x_uint64[1] = valTwo.m128x_uint64[1] & (~valOne.m128x_uint64[1]);
		return result;
	}

	jsonifier_inline simd_int_t xorSi128(const simd_int_t& valOne, const simd_int_t& valTwo) {
		simd_int_t value{};
		value.m128x_uint64[0] = valOne.m128x_uint64[0] ^ valTwo.m128x_uint64[0];
		value.m128x_uint64[1] = valOne.m128x_uint64[1] ^ valTwo.m128x_uint64[1];
		return value;
	}

	jsonifier_inline simd_int_t subEpi64(const simd_int_t& valOne, const simd_int_t& valTwo) {
		simd_int_t result{};
		result.m128x_uint64[0] = valOne.m128x_uint64[0] - valTwo.m128x_uint64[0];
		result.m128x_uint64[1] = valOne.m128x_uint64[1] - valTwo.m128x_uint64[1];
		return result;
	}

	template<uint64_t index = 0, uint64_t index02 = 0> jsonifier_inline void cmpeqEpi8Helper(simd_int_t& result, const simd_int_t& a, const simd_int_t& b) {
		if jsonifier_constexpr (index < 8) {
			uint8_t a8 = (a.m128x_uint64[index02] >> (index * 8)) & 0xff;
			uint8_t b8 = (b.m128x_uint64[index02] >> (index * 8)) & 0xff;
			result.m128x_uint64[index02] |= (a8 == b8 ? 0xFFULL : 0) << (index * 8);
			cmpeqEpi8Helper<index + 1, index02>(result, a, b);
		}
	}

	template<uint64_t index = 0> jsonifier_inline simd_int_t cmpeqEpi8(const simd_int_t& a, const simd_int_t& b) {
		simd_int_t result{};
		if jsonifier_constexpr (index < 2) {
			cmpeqEpi8Helper<0, index>(result, a, b);
			result = orSi128(result, cmpeqEpi8<index + 1>(a, b));
		}
		return result;
	}

	jsonifier_inline bool testzSi128(const simd_int_t& valOne, const simd_int_t& valTwo) {
		simd_int_t result{};
		result.m128x_uint64[0] = valOne.m128x_uint64[0] & valTwo.m128x_uint64[0];
		result.m128x_uint64[1] = valOne.m128x_uint64[1] & valTwo.m128x_uint64[1];
		return result.m128x_uint64[0] == 0 && result.m128x_uint64[1] == 0;
	}

	jsonifier_inline simd_int_t setEpi64x(uint64_t argOne, uint64_t argTwo) {
		simd_int_t returnValue{};
		returnValue.m128x_uint64[0] = argTwo;
		returnValue.m128x_uint64[1] = argOne;
		return returnValue;
	}

	jsonifier_inline simd_int_t set1Epi64x(uint64_t argOne) {
		simd_int_t returnValue{};
		returnValue.m128x_uint64[0] = argOne;
		returnValue.m128x_uint64[1] = argOne;
		return returnValue;
	}

	template<int64_t index = 0, int64_t index02 = 0> jsonifier_inline void shuffleEpi8Helper02(uint64_t& result, const simd_int_t& b) {
		if jsonifier_constexpr (index >= index02) {
			result = (result << 1) | ((b.m128x_uint64[index >> 6] >> (index & 0x3F)) & 1);
			shuffleEpi8Helper02<index - 1, index02>(result, b);
		}
	}

	template<uint64_t index = 0, uint64_t index02 = 0> jsonifier_inline void shuffleEpi8Helper03(simd_int_t& result, uint64_t& result02, const simd_int_t& a) {
		if jsonifier_constexpr (index < index02) {
			result.m128x_uint64[index >> 6] |= ((a.m128x_uint64[result02 >> 6] >> (result02 & 0x3F)) & 1ULL) << (index & 0x3F);
			result02++;
			shuffleEpi8Helper03<index + 1, index02>(result, result02, a);
		}
	}

	template<uint64_t index = 0> jsonifier_inline simd_int_t shuffleEpi8(const simd_int_t& a, const simd_int_t& b) {
		simd_int_t result{};
		if jsonifier_constexpr (index < 16) {
			jsonifier_constexpr uint64_t i		  = index * 8;
			jsonifier_constexpr uint64_t bitIndex = i + 7;

			if ((b.m128x_uint64[bitIndex >> 6] & (1ULL << (bitIndex & 0x3F))) == (1ULL << (bitIndex & 0x3F))) {
				result.m128x_uint64[bitIndex >> 6] &= ~(1ULL << (bitIndex & 0x3F));
			} else {
				uint64_t indexNew = 0;
				shuffleEpi8Helper02<i + 3, i>(indexNew, b);

				uint64_t aIndex = indexNew * 8;
				shuffleEpi8Helper03<i, i + 7>(result, aIndex, a);
			}
			result = orSi128(result, shuffleEpi8<index + 1>(a, b));
		}
		return result;
	}

	jsonifier_inline simd_int_t setZeroSi128() {
		return simd_int_t{};
	}

	jsonifier_inline simd_int_t set1Epi8(int8_t valueNew) {
		simd_int_t returnValue{};
		std::memset(&returnValue, valueNew, sizeof(simd_int_t));
		return returnValue;
	}

	jsonifier_inline string_parsing_type simd_base::cmpeq(const simd_int_t& other, const simd_int_t& value) {
		return static_cast<string_parsing_type>(movemaskEpi8(cmpeqEpi8(value, other)));
	}

	jsonifier_inline simd_int_t simd_base::bitAndNot(const simd_int_t& value, const simd_int_t& other) {
		return andNotSi128(other, value);
	}

	jsonifier_inline simd_int_t simd_base::shuffle(const simd_int_t& value, const simd_int_t& other) {
		return shuffleEpi8(value, other);
	}

	jsonifier_inline simd_int_t simd_base::opOr(const simd_int_t& other, const simd_int_t& value) {
		return orSi128(value, other);
	}

	jsonifier_inline simd_int_t simd_base::opAnd(const simd_int_t& other, const simd_int_t& value) {
		return andSi128(value, other);
	}

	jsonifier_inline simd_int_t simd_base::opXor(const simd_int_t& other, const simd_int_t& value) {
		return xorSi128(value, other);
	}

	jsonifier_inline simd_int_t simd_base::setLSB(const simd_int_t& value, bool valueNew) {
		if (valueNew) {
			return orSi128(value, setEpi64x(0x00, 0x01));
		} else {
			return andNotSi128(setEpi64x(0x00, 0x01), value);
		}
	}

	jsonifier_inline simd_int_t simd_base::opNot(const simd_int_t& value) {
		return xorSi128(value, set1Epi64x(static_cast<int64_t>(std::numeric_limits<uint64_t>::max())));
	}

	jsonifier_inline bool simd_base::getMSB(const simd_int_t& value) {
		simd_int_t result = andSi128(value, setEpi64x(0x8000000000000000, 0x00));
		return !testzSi128(result, result);
	}

	jsonifier_inline bool simd_base::opBool(const simd_int_t& value) {
		return !testzSi128(value, value);
	}

	jsonifier_inline simd_int_t simd_base::reset() {
		return setZeroSi128();
	}

#endif

}