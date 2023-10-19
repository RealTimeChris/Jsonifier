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
	concept avx_int_128_t = std::same_as<jsonifier::concepts::unwrap_t<value_type>, avx_int_128>;

	using avx_int_t = avx_int_128;

	template<typename ReturnT> inline ReturnT gatherValues(jsonifier::concepts::float_t auto* str);

	inline uint64_t packUint64(const uint8_t* arr) {
		return (static_cast<uint64_t>(arr[7]) << 56) | (static_cast<uint64_t>(arr[6]) << 48) | (static_cast<uint64_t>(arr[5]) << 40) | (static_cast<uint64_t>(arr[4]) << 32) |
			(static_cast<uint64_t>(arr[3]) << 24) | (static_cast<uint64_t>(arr[2]) << 16) | (static_cast<uint64_t>(arr[1]) << 8) | static_cast<uint64_t>(arr[0]);
	}

	inline void packUint64Array(const uint8_t* input, avx_int_t& output) {
		output.m128x_uint64[0] = packUint64(const_cast<string_buffer_ptr>(input));
		output.m128x_uint64[1] = packUint64(const_cast<string_buffer_ptr>(input + 8));
	}

	template<avx_int_128_t avx_type> inline avx_type gatherValues(jsonifier::concepts::char_type auto* str) {
		alignas(JsonifierAlignment) avx_int_t newArray{};
		packUint64Array(str, newArray);
		return newArray;
	}

	template<uint64_t index = 0, uint64_t index02 = 0> inline void movemaskEpi8Helper02(int16_t& result, const avx_int_t& a) {
		if constexpr (index < 8) {
			uint8_t value = (a.m128x_uint64[index02] >> (index * 8)) & 0xff;
			int16_t mask  = (value >> 7) & 1;
			result |= mask << (index02 * 8 + index);
			movemaskEpi8Helper02<index + 1, index02>(result, a);
		}
	}

	template<uint64_t index = 0> inline int16_t movemaskEpi8(const avx_int_t& a) {
		int16_t result{};
		if constexpr (index < 2) {
			movemaskEpi8Helper02<0, index>(result, a);
			result |= movemaskEpi8<index + 1>(a);
		}
		return result;
	}

	inline avx_int_t orSi128(const avx_int_t& valOne, const avx_int_t& valTwo) {
		avx_int_t value{};
		value.m128x_uint64[0] = valOne.m128x_uint64[0] | valTwo.m128x_uint64[0];
		value.m128x_uint64[1] = valOne.m128x_uint64[1] | valTwo.m128x_uint64[1];
		return value;
	}

	inline avx_int_t andSi128(const avx_int_t& valOne, const avx_int_t& valTwo) {
		avx_int_t value{};
		value.m128x_uint64[0] = valOne.m128x_uint64[0] & valTwo.m128x_uint64[0];
		value.m128x_uint64[1] = valOne.m128x_uint64[1] & valTwo.m128x_uint64[1];
		return value;
	}

	inline avx_int_t andNotSi128(const avx_int_t& valOne, const avx_int_t& valTwo) {
		avx_int_t result{};
		result.m128x_uint64[0] = valTwo.m128x_uint64[0] & (~valOne.m128x_uint64[0]);
		result.m128x_uint64[1] = valTwo.m128x_uint64[1] & (~valOne.m128x_uint64[1]);
		return result;
	}

	inline avx_int_t xorSi128(const avx_int_t& valOne, const avx_int_t& valTwo) {
		avx_int_t value{};
		value.m128x_uint64[0] = valOne.m128x_uint64[0] ^ valTwo.m128x_uint64[0];
		value.m128x_uint64[1] = valOne.m128x_uint64[1] ^ valTwo.m128x_uint64[1];
		return value;
	}

	inline avx_int_t subEpi8(const avx_int_t& valOne, const avx_int_t& valTwo) {
		avx_int_t result{};
		result.m128x_uint64[0] = valOne.m128x_uint64[0] - valTwo.m128x_uint64[0];
		result.m128x_uint64[1] = valOne.m128x_uint64[1] - valTwo.m128x_uint64[1];
		return result;
	}

	inline avx_int_t notSi128(const avx_int_t& valOne) {
		avx_int_t result{};
		result.m128x_uint64[0] = ~valOne.m128x_uint64[0];
		result.m128x_uint64[1] = ~valOne.m128x_uint64[1];
		return result;
	}

	template<uint64_t index = 0> inline avx_int_t cmpeqEpi8(const avx_int_t& a, const avx_int_t& b) {
		avx_int_t result{};
		if constexpr (index < 2) {
			cmpeqEpi8Helper<0, index>(result, a, b);
			result = orSi128(result, cmpeqEpi8<index + 1>(a, b));
		}
		return result;
	}

	template<uint64_t index = 0, uint64_t index02 = 0> inline void cmpeqEpi8Helper(avx_int_t& result, const avx_int_t& a, const avx_int_t& b) {
		if constexpr (index < 8) {
			uint8_t a8 = (a.m128x_uint64[index02] >> (index * 8)) & 0xff;
			uint8_t b8 = (b.m128x_uint64[index02] >> (index * 8)) & 0xff;
			result.m128x_uint64[index02] |= (a8 == b8 ? 0xFFULL : 0) << (index * 8);
			cmpeqEpi8Helper<index + 1, index02>(result, a, b);
		}
	}

	inline bool testzSi128(const avx_int_t& valOne, const avx_int_t& valTwo) {
		avx_int_t result{};
		result.m128x_uint64[0] = valOne.m128x_uint64[0] & valTwo.m128x_uint64[0];
		result.m128x_uint64[1] = valOne.m128x_uint64[1] & valTwo.m128x_uint64[1];
		return result.m128x_uint64[0] == 0 && result.m128x_uint64[1] == 0;
	}

	inline avx_int_t setEpi64x(uint64_t argOne, uint64_t argTwo) {
		avx_int_t returnValue{};
		returnValue.m128x_uint64[0] = argTwo;
		returnValue.m128x_uint64[1] = argOne;
		return returnValue;
	}

	inline avx_int_t set1Epi64x(uint64_t argOne) {
		avx_int_t returnValue{};
		returnValue.m128x_uint64[0] = argOne;
		returnValue.m128x_uint64[1] = argOne;
		return returnValue;
	}

	inline avx_int_t& insertUint16(avx_int_t& value, uint16_t newValue, uint64_t position) {
		if (position < 0 || position >= 8) {
			return value;
		}
		value.m128x_uint16[position] = newValue;
		return value;
	}

	inline avx_int_t& insertUint64(avx_int_t& value, uint64_t newValue, uint64_t position) {
		if (position >= 0 && position < 2) {
			value.m128x_uint64[position] = newValue;
		}
		return value;
	}

	inline uint64_t extractUint64(avx_int_t value, uint64_t index) {
		if (index >= 0 && index < 2) {
			return static_cast<uint64_t>(value.m128x_uint64[index]);
		}
		return 0;
	}

	template<uint64_t index = 0> inline avx_int_t shuffleEpi8(const avx_int_t& a, const avx_int_t& b) {
		avx_int_t result{};
		if constexpr (index < 16) {
			static constexpr uint64_t i		   = index * 8;
			static constexpr uint64_t bitIndex = i + 7;

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

	template<int64_t index = 0, int64_t index02 = 0> inline void shuffleEpi8Helper02(uint64_t& result, const avx_int_t& b) {
		if constexpr (index >= index02) {
			result = (result << 1) | ((b.m128x_uint64[index >> 6] >> (index & 0x3F)) & 1);
			shuffleEpi8Helper02<index - 1, index02>(result, b);
		}
	}

	template<uint64_t index = 0, uint64_t index02 = 0> inline void shuffleEpi8Helper03(avx_int_t& result, uint64_t& result02, const avx_int_t& a) {
		if constexpr (index < index02) {
			result.m128x_uint64[index >> 6] |= ((a.m128x_uint64[result02 >> 6] >> (result02 & 0x3F)) & 1ULL) << (index & 0x3F);
			result02++;
			shuffleEpi8Helper03<index + 1, index02>(result, result02, a);
		}
	}

	inline avx_int_t set1Epi8(int8_t newValue) {
		avx_int_t returnValue{};
		std::memset(&returnValue, newValue, sizeof(avx_int_t));
		return returnValue;
	}

	template<avx_int_128_t return_type> constexpr return_type simdTable(const uint8_t arrayNew[sizeof(avx_int_t)]) {
		avx_int_t returnValue{};
		for (uint64_t x = 0; x < sizeof(avx_int_t); ++x) {
			returnValue.m128x_uint8[x] = arrayNew[x];
		}
		return returnValue;
	}

	template<avx_int_128_t return_type> constexpr return_type simdValues(uint8_t value) {
		avx_int_t returnValue{};
		for (uint64_t x = 0; x < sizeof(avx_int_t); ++x) {
			returnValue.m128x_uint8[x] = value;
		}
		return returnValue;
	}

	inline string_parsing_type cmpEq(const avx_int_t& lhs, const avx_int_t& rhs) {
		return static_cast<string_parsing_type>(movemaskEpi8(cmpeqEpi8(lhs, rhs)));
	}

	inline string_parsing_type cmpEq(const avx_int_t& lhs, uint8_t rhs) {
		return static_cast<string_parsing_type>(movemaskEpi8(cmpeqEpi8(lhs, set1Epi8(static_cast<char>(rhs)))));
	}

	inline avx_int_t opOr(const avx_int_t& lhs, const avx_int_t& rhs) {
		return orSi128(lhs, rhs);
	}

	inline avx_int_t opSub(const avx_int_t& lhs, const avx_int_t& rhs) {
		return subEpi8(lhs, rhs);
	}

	inline avx_int_t opAnd(const avx_int_t& lhs, const avx_int_t& rhs) {
		return andSi128(lhs, rhs);
	}

	inline avx_int_t opXor(const avx_int_t& lhs, const avx_int_t& rhs) {
		return xorSi128(lhs, rhs);
	}

	inline void setLSB(avx_int_t& values, bool valueNew) {
		if (valueNew) {
			values = orSi128(values, setEpi64x(0, 0x1));
		} else {
			values = andNotSi128(setEpi64x(0, 0x1), values);
		}
	}

	template<uint64_t index = 0> uint64_t getUint64(avx_int_t& value) {
		static_assert(index < SixtyFourBitsPerStep, "Sorry, but that index value is incorrect.");
		return static_cast<uint64_t>(extractUint64(value, index));
	}

	template<uint64_t index = 0> inline void insertUint64(avx_int_t& value, uint64_t valueNew) {
		static_assert(index < SixtyFourBitsPerStep, "Sorry, but that index value is incorrect.");
		value = insertUint64(value, static_cast<int64_t>(valueNew), index);
	}

	template<uint64_t amount> inline void shl(avx_int_t& values) {
		avx_int_t currentValues{};
		insertUint64<0>(currentValues, getUint64<0>(values) << amount);
		uint64_t shiftBetween = amount % 64;
		insertUint64<1>(currentValues, (getUint64<1>(values) << amount) | (getUint64(values) >> (64 - shiftBetween)));
		values = currentValues;
	}

	inline bool getMSB(avx_int_t& values) {
		avx_int_t result = andSi128(values, setEpi64x(0x8000000000000000, 0));
		return !testzSi128(result, result);
	}

	template<uint64_t index = 0> inline void insertUint16(avx_int_t& values, string_parsing_type valueNew) {
		static_assert(index < StridesPerStep, "Sorry, but that index value is incorrect.");
		values = insertUint16(values, static_cast<int16_t>(valueNew), index);
	}

	template<uint64_t index = 0> inline void addValues(avx_int_t& values, string_parsing_type valuesToAdd) {
		insertUint16<index>(values, valuesToAdd);
	}

	inline avx_int_t shuffle(const avx_int_t& other, const avx_int_t& values) {
		return shuffleEpi8(values, other);
	}

	template<uint64_t index = 0> inline void convertWhitespaceToSimdBase(avx_int_t& values, avx_int_t valuesNew[StridesPerStep]) {
		if constexpr (index < StridesPerStep) {
			static constexpr uint8_t arrayNew[]{ ' ', 100, 100, 100, 17, 100, 113, 2, 100, '\t', '\n', 112, 100, '\r', 100, 100 };
			static constexpr avx_int_t whitespaceTable{ simdTable<avx_int_t>(arrayNew) };
			addValues<index>(values, cmpEq(shuffle(valuesNew[index], whitespaceTable), valuesNew[index]));
			convertWhitespaceToSimdBase<index + 1>(values, valuesNew);
		}
	}

	template<uint64_t index = 0> inline void convertBackslashesToSimdBase(avx_int_t& values, avx_int_t valuesNew[StridesPerStep]) {
		if constexpr (index < StridesPerStep) {
			static constexpr avx_int_t backslashesVal{ simdValues<avx_int_t>('\\') };
			addValues<index>(values, cmpEq(valuesNew[index], backslashesVal));
			convertBackslashesToSimdBase<index + 1>(values, valuesNew);
		}
	}

	template<uint64_t index = 0> inline void convertStructuralsToSimdBase(avx_int_t& values, avx_int_t valuesNew[StridesPerStep]) {
		if constexpr (index < StridesPerStep) {
			static constexpr uint8_t arrayNew[]{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, ':', '{', ',', '}', 0, 0 };
			static constexpr avx_int_t opTableVal{ simdTable<avx_int_t>(arrayNew) };
			static constexpr avx_int_t chars{ simdValues<avx_int_t>(0x20) };
			addValues<index>(values, cmpEq(shuffle(valuesNew[index], opTableVal), opOr(valuesNew[index], chars)));
			convertStructuralsToSimdBase<index + 1>(values, valuesNew);
		}
	}

	template<uint64_t index = 0> inline void convertQuotesToSimdBase(avx_int_t& values, avx_int_t valuesNew[StridesPerStep]) {
		if constexpr (index < StridesPerStep) {
			static constexpr avx_int_t quotesVal{ simdValues<avx_int_t>('"') };
			addValues<index>(values, cmpEq(valuesNew[index], quotesVal));
			convertQuotesToSimdBase<index + 1>(values, valuesNew);
		}
	}

	inline uint64_t prefixXor(uint64_t prevInString) {
		prevInString ^= prevInString << 1;
		prevInString ^= prevInString << 2;
		prevInString ^= prevInString << 4;
		prevInString ^= prevInString << 8;
		prevInString ^= prevInString << 16;
		prevInString ^= prevInString << 32;
		return prevInString;
	}

	inline avx_int_t carrylessMultiplication(avx_int_t& value, uint64_t& prevInString) {
		avx_int_t values_uint64New{};
		avx_int_t valueLow{ value };
		insertUint64(values_uint64New, prefixXor(valueLow.m128x_uint64[0]) ^ prevInString, 0);
		prevInString = uint64_t(static_cast<int64_t>(getUint64<0>(values_uint64New)) >> 63);
		insertUint64(values_uint64New, prefixXor(valueLow.m128x_uint64[1]) ^ prevInString, 1);
		prevInString = uint64_t(static_cast<int64_t>(getUint64<1>(values_uint64New)) >> 63);
		return values_uint64New;
	}

	inline avx_int_t follows(avx_int_t& value, bool& overflow) {
		avx_int_t result = value;
		shl<1>(result);
		setLSB(result, overflow);
		overflow = getMSB(value);
		return result;
	}

	inline avx_int_t bitAndNot(const avx_int_t& values, const avx_int_t& other) {
		return andNotSi128(values, other);
	}

	inline bool opBool(const avx_int_t& value) {
		return !testzSi128(value, value);
	}

	inline avx_int_t opNot(const avx_int_t& value) {
		return notSi128(value);
	}

	template<typename value_type> inline void store(const avx_int_t& values, value_type* storageLocation) {
		std::memcpy(storageLocation, &values, sizeof(values));
	}

	inline void printBits(uint64_t values, const std::string& valuesTitle) {
		std::cout << valuesTitle;
		std::cout << std::bitset<64>{ values };
		std::cout << std::endl;
	}

	inline const avx_int_t& printBits(avx_int_t& value, const std::string& valuesTitle) noexcept {
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


#endif

}