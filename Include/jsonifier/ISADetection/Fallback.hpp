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

	template<typename value_type>
	concept simd_int_128_t = std::same_as<jsonifier::concepts::unwrap<value_type>, simd_int_128>;

	using simd_int_t = simd_int_128;

	template<simd_int_128_t simd_type, typename char_type> jsonifier_inline simd_type gatherValues(char_type* str) {
		simd_int_t returnValue{};
		std::memcpy(returnValue.m128x_uint64, str, sizeof(simd_int_t));
		return returnValue;
	}

	template<simd_int_128_t simd_type, typename char_type> jsonifier_inline simd_type gatherValuesU(char_type* str) {
		simd_int_t returnValue{};
		std::memcpy(returnValue.m128x_uint64, str, sizeof(simd_int_t));
		return returnValue;
	}

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

	jsonifier_inline simd_int_t notSi128(const simd_int_t& valOne) {
	}

	template<uint64_t index = 0> jsonifier_inline simd_int_t cmpeqEpi8(const simd_int_t& a, const simd_int_t& b) {
		simd_int_t result{};
		if jsonifier_constexpr (index < 2) {
			cmpeqEpi8Helper<0, index>(result, a, b);
			result = orSi128(result, cmpeqEpi8<index + 1>(a, b));
		}
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

	template<uint64_t index = 0> jsonifier_inline simd_int_t shuffleEpi8(const simd_int_t& a, const simd_int_t& b) {
		simd_int_t result{};
		if jsonifier_constexpr (index < 16) {
			static jsonifier_constexpr uint64_t i		 = index * 8;
			static jsonifier_constexpr uint64_t bitIndex = i + 7;

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

	jsonifier_inline simd_int_t set1Epi8(int8_t valueNew) {
		simd_int_t returnValue{};
		std::memset(&returnValue, valueNew, sizeof(simd_int_t));
		return returnValue;
	}

	template<simd_int_128_t return_type> jsonifier_constexpr return_type simdFromTable(const uint8_t arrayNew01[sizeof(simd_int_128)]) {
		simd_int_128 returnValue{};
		for (uint64_t x = 0; x < sizeof(simd_int_128); ++x) {
			returnValue.m128x_uint8[x] = arrayNew01[x];
		}
		return returnValue;
	}

	template<simd_int_128_t return_type> jsonifier_constexpr return_type simdFromValue(uint8_t value) {
		simd_int_128 returnValue{};
		for (uint64_t x = 0; x < sizeof(simd_int_128); ++x) {
			returnValue.m128x_uint8[x] = value;
		}
		return returnValue;
	}

	template<> class simd_base_internal<128> {
	  public:
		static jsonifier_constexpr simd_int_t backslashes{ simdFromValue<simd_int_t>(0x5Cu) };
		static jsonifier_constexpr simd_int_t quotes{ simdFromValue<simd_int_t>(0x22u) };

		jsonifier_inline simd_base_internal() noexcept = default;

		jsonifier_inline static bool opBool(const simd_int_t& value) {
			return !testzSi128(value, value);
		}

		jsonifier_inline static simd_int_t opOr(const simd_int_t& value, const simd_int_t& other) {
			return orSi128(value, other);
		}

		jsonifier_inline static simd_int_t opSub(const simd_int_t& value, const simd_int_t& other) {
			return subEpi64(value, other);
		}

		jsonifier_inline static simd_int_t opAnd(const simd_int_t& value, const simd_int_t& other) {
			return andSi128(value, other);
		}

		jsonifier_inline static simd_int_t opXor(const simd_int_t& value, const simd_int_t& other) {
			return xorSi128(value, other);
		}

		jsonifier_inline static string_parsing_type cmpeq(const simd_int_t& value, uint8_t other) {
			return movemaskEpi8(cmpeqEpi8(value, set1Epi8(other)));
		}

		jsonifier_inline static string_parsing_type cmpeq(const simd_int_t& value, const simd_int_t& other) {
			return movemaskEpi8(cmpeqEpi8(value, other));
		}

		jsonifier_inline static simd_int_t opNot(const simd_int_t& value) {
			simd_int_t result{};
			result.m128x_uint64[0] = ~value.m128x_uint64[0];
			result.m128x_uint64[1] = ~value.m128x_uint64[1];
			return result;
		}

		jsonifier_inline static void collectWhitespaceAsSimdBase(simd_int_t& value, simd_int_t valuesNew[StridesPerStep]) {
			static jsonifier_constexpr uint8_t arrayNew01[]{ 0x20, 0x64, 0x64, 0x64, 0x11, 0x64, 0x71, 0x02, 0x64, 0x09, 0x0A, 0x70, 0x64, 0x0D, 0x64, 0x64 };
			static jsonifier_constexpr simd_int_t whitespaceTable{ simdFromTable<simd_int_t>(arrayNew01) };
			insertUint16<0>(value, cmpeq(shuffle(valuesNew[0], whitespaceTable), valuesNew[0]));
			insertUint16<1>(value, cmpeq(shuffle(valuesNew[1], whitespaceTable), valuesNew[1]));
			insertUint16<2>(value, cmpeq(shuffle(valuesNew[2], whitespaceTable), valuesNew[2]));
			insertUint16<3>(value, cmpeq(shuffle(valuesNew[3], whitespaceTable), valuesNew[3]));
			insertUint16<4>(value, cmpeq(shuffle(valuesNew[4], whitespaceTable), valuesNew[4]));
			insertUint16<5>(value, cmpeq(shuffle(valuesNew[5], whitespaceTable), valuesNew[5]));
			insertUint16<6>(value, cmpeq(shuffle(valuesNew[6], whitespaceTable), valuesNew[6]));
			insertUint16<7>(value, cmpeq(shuffle(valuesNew[7], whitespaceTable), valuesNew[7]));
		}

		jsonifier_inline static void collectBackslashesAsSimdBase(simd_int_t& value, simd_int_t valuesNew[StridesPerStep]) {
			static jsonifier_constexpr simd_int_t backslashes{ simdFromValue<simd_int_t>(0x5C) };
			insertUint16<0>(value, cmpeq(valuesNew[0], backslashes));
			insertUint16<1>(value, cmpeq(valuesNew[1], backslashes));
			insertUint16<2>(value, cmpeq(valuesNew[2], backslashes));
			insertUint16<3>(value, cmpeq(valuesNew[3], backslashes));
			insertUint16<4>(value, cmpeq(valuesNew[4], backslashes));
			insertUint16<5>(value, cmpeq(valuesNew[5], backslashes));
			insertUint16<6>(value, cmpeq(valuesNew[6], backslashes));
			insertUint16<7>(value, cmpeq(valuesNew[7], backslashes));
		}

		jsonifier_inline static void collectStructuralsAsSimdBase(simd_int_t& value, simd_int_t valuesNew[StridesPerStep]) {
			static jsonifier_constexpr uint8_t arrayNew01[]{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3Au, 0x7Bu, 0x2Cu, 0x7Du, 0x00, 0x00 };
			static jsonifier_constexpr simd_int_t opTableVal{ simdFromTable<simd_int_t>(arrayNew01) };
			static jsonifier_constexpr simd_int_t chars{ simdFromValue<simd_int_t>(0x20) };
			insertUint16<0>(value, cmpeq(shuffle(valuesNew[0], opTableVal), (opOr(valuesNew[0], chars))));
			insertUint16<1>(value, cmpeq(shuffle(valuesNew[1], opTableVal), (opOr(valuesNew[1], chars))));
			insertUint16<2>(value, cmpeq(shuffle(valuesNew[2], opTableVal), (opOr(valuesNew[2], chars))));
			insertUint16<3>(value, cmpeq(shuffle(valuesNew[3], opTableVal), (opOr(valuesNew[3], chars))));
			insertUint16<4>(value, cmpeq(shuffle(valuesNew[4], opTableVal), (opOr(valuesNew[4], chars))));
			insertUint16<5>(value, cmpeq(shuffle(valuesNew[5], opTableVal), (opOr(valuesNew[5], chars))));
			insertUint16<6>(value, cmpeq(shuffle(valuesNew[6], opTableVal), (opOr(valuesNew[6], chars))));
			insertUint16<7>(value, cmpeq(shuffle(valuesNew[7], opTableVal), (opOr(valuesNew[7], chars))));
		}

		jsonifier_inline static void collectQuotesAsSimdBase(simd_int_t& value, simd_int_t valuesNew[StridesPerStep]) {
			static jsonifier_constexpr simd_int_t quotes{ simdFromValue<simd_int_t>(0x22u) };
			insertUint16<0>(value, cmpeq(valuesNew[0], quotes));
			insertUint16<1>(value, cmpeq(valuesNew[1], quotes));
			insertUint16<2>(value, cmpeq(valuesNew[2], quotes));
			insertUint16<3>(value, cmpeq(valuesNew[3], quotes));
			insertUint16<4>(value, cmpeq(valuesNew[4], quotes));
			insertUint16<5>(value, cmpeq(valuesNew[5], quotes));
			insertUint16<6>(value, cmpeq(valuesNew[6], quotes));
			insertUint16<7>(value, cmpeq(valuesNew[7], quotes));
		}

		template<uint64_t index = 0> jsonifier_inline static uint64_t getUint64(const simd_int_t& value) {
			if (index >= 0 && index < 2) {
				return value.m128x_uint64[index];
			}
			return 0;
		}

		template<uint64_t index = 0> jsonifier_inline static void insertUint64(simd_int_t& value, uint64_t valueNew) {
			if (index >= 0 && index < 2) {
				value.m128x_uint64[index] = valueNew;
			}
			return;
		}

		template<uint64_t index = 0> jsonifier_inline static void insertUint16(simd_int_t& value, uint16_t valueNew) {
			if (index < 0 || index >= 8) {
				return;
			}
			value.m128x_uint16[index] = valueNew;
			return;
		}

		jsonifier_inline static simd_int_t bitAndNot(const simd_int_t& other, const simd_int_t& value) {
			return andNotSi128(value, other);
		}

		jsonifier_inline static simd_int_t shuffle(const simd_int_t& other, const simd_int_t& value) {
			return shuffleEpi8(value, other);
		}

		template<uint64_t amount> static jsonifier_inline simd_int_t shl(const simd_int_t& value) {
			simd_int_t currentValues{};
			insertUint64<0>(currentValues, getUint64<0>(value) << amount);
			static jsonifier_constexpr uint64_t shiftBetween = amount % 64;
			insertUint64<1>(currentValues, (getUint64<1>(value) << amount) | (getUint64<0>(value) >> (64 - shiftBetween)));
			return currentValues;
		}

		template<typename value_type> jsonifier_inline static void store(const simd_int_t& value, value_type* storageLocation) {
			std::memcpy(storageLocation, &value, sizeof(value));
		}

		template<typename value_type> jsonifier_inline static void storeu(const simd_int_t& value, value_type* storageLocation) {
			std::memcpy(storageLocation, &value, sizeof(value));
		}

		jsonifier_inline static simd_int_t setLSB(const simd_int_t& value, bool valueNew) {
			if (valueNew) {
				return orSi128(value, setEpi64x(0, 0x1));
			} else {
				return andNotSi128(setEpi64x(0, 0x1), value);
			}
		}

		jsonifier_inline static bool getMSB(const simd_int_t& value) {
			simd_int_t result = andSi128(value, setEpi64x(0x8000000000000000, 0));
			return !testzSi128(result, result);
		}

		jsonifier_inline static uint64_t prefixXor(uint64_t prevInString) {
			prevInString ^= prevInString << 1;
			prevInString ^= prevInString << 2;
			prevInString ^= prevInString << 4;
			prevInString ^= prevInString << 8;
			prevInString ^= prevInString << 16;
			prevInString ^= prevInString << 32;
			return prevInString;
		}

		jsonifier_inline static simd_int_t carrylessMultiplication(const simd_int_t& value, uint64_t& prevInString) {
			simd_int_t valuesNew{};
			insertUint64<0>(valuesNew, prefixXor(getUint64<0>(value)) ^ prevInString);
			prevInString = uint64_t(static_cast<int64_t>(getUint64<0>(valuesNew)) >> 63);
			insertUint64<1>(valuesNew, prefixXor(getUint64<1>(value)) ^ prevInString);
			prevInString = uint64_t(static_cast<int64_t>(getUint64<1>(valuesNew)) >> 63);
			return valuesNew;
		}

		jsonifier_inline static simd_int_t follows(const simd_int_t& value, bool& overflow) {
			simd_int_t result = shl<1>(value);
			result			  = setLSB(result, overflow);
			overflow		  = getMSB(value);
			return result;
		}

		jsonifier_inline static void printBits(uint64_t values, const std::string& valuesTitle) {
			std::cout << valuesTitle;
			std::cout << std::bitset<64>{ values };
			std::cout << std::endl;
		}

		jsonifier_inline static const simd_int_t& printBits(const simd_int_t& value, const std::string& valuesTitle) noexcept {
			uint8_t values[BytesPerStep]{};
			store(value, values);
			std::cout << valuesTitle;
			for (uint64_t x = 0; x < BytesPerStep; ++x) {
				for (uint64_t y = 0; y < 8; ++y) {
					std::cout << std::bitset<1>{ static_cast<uint64_t>(*(values + x)) >> y };
				}
			}
			std::cout << std::endl;
			return value;
		}
	};

#endif

}