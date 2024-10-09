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
/// Note: Most of the code in this header was sampled from Glaze library: https://github.com/stephenberry/glaze
/// https://github.com/RealTimeChris/jsonifier
/// Nov 13, 2023
#pragma once

#include <jsonifier/Allocator.hpp>
#include <jsonifier/FastFloat.hpp>
#include <jsonifier/StrToD.hpp>

#include <concepts>
#include <cstdint>
#include <cstring>
#include <array>

namespace jsonifier_internal {

	JSONIFIER_ALWAYS_INLINE_VARIABLE std::array<size_t, 20> powerOfTenInt{ 1ull, 10ull, 100ull, 1000ull, 10000ull, 100000ull, 1000000ull, 10000000ull, 100000000ull, 1000000000ull,
		10000000000ull, 100000000000ull, 1000000000000ull, 10000000000000ull, 100000000000000ull, 1000000000000000ull, 10000000000000000ull, 100000000000000000ull,
		1000000000000000000ull, 10000000000000000000ull };

	template<typename value_type> JSONIFIER_ALWAYS_INLINE_VARIABLE std::array<value_type, 10> rawCompValsPos{ [] {
		constexpr auto rawCompVal{ std::numeric_limits<value_type>::max() };
		std::array<value_type, 10> returnValues{};
		returnValues[0] = (rawCompVal - 0) / 10;
		returnValues[1] = (rawCompVal - 1) / 10;
		returnValues[2] = (rawCompVal - 2) / 10;
		returnValues[3] = (rawCompVal - 3) / 10;
		returnValues[4] = (rawCompVal - 4) / 10;
		returnValues[5] = (rawCompVal - 5) / 10;
		returnValues[6] = (rawCompVal - 6) / 10;
		returnValues[7] = (rawCompVal - 7) / 10;
		returnValues[8] = (rawCompVal - 8) / 10;
		returnValues[9] = (rawCompVal - 9) / 10;
		return returnValues;
	}() };

	template<typename value_type> JSONIFIER_ALWAYS_INLINE_VARIABLE std::array<value_type, 10> rawCompValsNeg{ [] {
		constexpr auto rawCompVal{ size_t(std::numeric_limits<value_type>::max()) + 1 };
		std::array<value_type, 10> returnValues{};
		returnValues[0] = (rawCompVal - 0) / 10;
		returnValues[1] = (rawCompVal - 1) / 10;
		returnValues[2] = (rawCompVal - 2) / 10;
		returnValues[3] = (rawCompVal - 3) / 10;
		returnValues[4] = (rawCompVal - 4) / 10;
		returnValues[5] = (rawCompVal - 5) / 10;
		returnValues[6] = (rawCompVal - 6) / 10;
		returnValues[7] = (rawCompVal - 7) / 10;
		returnValues[8] = (rawCompVal - 8) / 10;
		returnValues[9] = (rawCompVal - 9) / 10;
		return returnValues;
	}() };

	JSONIFIER_ALWAYS_INLINE_VARIABLE std::array<bool, 256> digiTable{ [] {
		std::array<bool, 256> returnValues{};
		returnValues['0'] = true;
		returnValues['1'] = true;
		returnValues['2'] = true;
		returnValues['3'] = true;
		returnValues['4'] = true;
		returnValues['5'] = true;
		returnValues['6'] = true;
		returnValues['7'] = true;
		returnValues['8'] = true;
		returnValues['9'] = true;
		return returnValues;
	}() };

	JSONIFIER_ALWAYS_INLINE_VARIABLE std::array<bool, 256> expTable{ [] {
		std::array<bool, 256> returnValues{};
		returnValues['E'] = true;
		returnValues['e'] = true;
		return returnValues;
	}() };

	JSONIFIER_ALWAYS_INLINE_VARIABLE std::array<bool, 256> expFracTable{ [] {
		std::array<bool, 256> returnValues{};
		returnValues['.'] = true;
		returnValues['E'] = true;
		returnValues['e'] = true;
		return returnValues;
	}() };

	JSONIFIER_ALWAYS_INLINE_VARIABLE std::array<bool, 256> plusOrMinusTable{ [] {
		std::array<bool, 256> returnValues{};
		returnValues['+'] = true;
		returnValues['-'] = true;
		return returnValues;
	}() };

	JSONIFIER_ALWAYS_INLINE int64_t fastAbs(int64_t x) {
		int64_t mask = x >> 63;
		return (x ^ mask) - mask;
	}

	JSONIFIER_ALWAYS_INLINE_VARIABLE char decimal{ '.' };
	JSONIFIER_ALWAYS_INLINE_VARIABLE char minus{ '-' };
	JSONIFIER_ALWAYS_INLINE_VARIABLE char zero{ '0' };

#define toDigit(iter) (static_cast<char>(iter) - jsonifier_internal::zero)

	template<typename value_type, typename char_type> struct integer_parser {
		uint8_t chunk8;
		uint8_t lowerDigits8;
		uint8_t upperDigits8;
		uint16_t chunk16;
		uint16_t lowerDigits16;
		uint16_t upperDigits16;
		uint32_t chunk32;
		uint32_t lowerDigits32;
		uint32_t upperDigits32;
		uint64_t chunk64;
		uint64_t lowerDigits64;
		uint64_t upperDigits64;
		int8_t currentIndex{};
		int64_t fracDigits{};
		int64_t intDigits{};
		size_t fracValue{};
		int64_t expValue{};
		int8_t expSign{};
		int8_t numTmp{};

#define parse_2_chars(powerIndex, first, results, string) \
	{ \
		std::memcpy(&chunk16, string, 2); \
		constexpr auto multiplier = powerOfTenInt[powerIndex]; \
		lowerDigits16			  = (chunk16 & 0x0f000f000f000f00) >> 8; \
		upperDigits16			  = (chunk16 & 0x000f000f000f000f) * 10; \
		if (first) { \
			results = (lowerDigits16 + upperDigits16) * multiplier; \
		} else { \
			results += (lowerDigits16 + upperDigits16) * multiplier; \
		} \
	}

#define parse_4_chars(powerIndex, first, results, string) \
	{ \
		std::memcpy(&chunk32, string, 4); \
		constexpr auto multiplier = powerOfTenInt[powerIndex]; \
		lowerDigits32			  = (chunk32 & 0x0f000f000f000f00) >> 8; \
		upperDigits32			  = (chunk32 & 0x000f000f000f000f) * 10; \
		chunk32					  = lowerDigits32 + upperDigits32; \
		lowerDigits32			  = (chunk32 & 0x00ff000000ff0000) >> 16; \
		upperDigits32			  = (chunk32 & 0x000000ff000000ff) * 100; \
		if (first) { \
			results = (lowerDigits32 + upperDigits32) * multiplier; \
		} else { \
			results += (lowerDigits32 + upperDigits32) * multiplier; \
		} \
	}

#define parse_8_chars(powerIndex, first, results, string) \
	{ \
		std::memcpy(&chunk64, string, sizeof(chunk64)); \
		constexpr auto multiplier = powerOfTenInt[powerIndex]; \
		lowerDigits64			  = (chunk64 & 0x0f000f000f000f00) >> 8; \
		upperDigits64			  = (chunk64 & 0x000f000f000f000f) * 10; \
		chunk64					  = lowerDigits64 + upperDigits64; \
		lowerDigits64			  = (chunk64 & 0x00ff000000ff0000) >> 16; \
		upperDigits64			  = (chunk64 & 0x000000ff000000ff) * 100; \
		chunk64					  = lowerDigits64 + upperDigits64; \
		lowerDigits64			  = (chunk64 & 0x0000ffff00000000) >> 32; \
		upperDigits64			  = (chunk64 & 0x000000000000ffff) * 10000; \
		if (first) { \
			results = (lowerDigits64 + upperDigits64) * multiplier; \
		} else { \
			results += (lowerDigits64 + upperDigits64) * multiplier; \
		} \
	}

#define callParseChars(positive, index, results, iter) parseChars<positive, index>(results, iter)

		template<bool positive, size_t length> JSONIFIER_ALWAYS_INLINE bool parseChars(value_type& results, char_type*& s) noexcept {
			if constexpr (length == 20) {
				parse_8_chars(12, true, results, s);
				parse_8_chars(4, false, results, s + 8);
				value_type tempResults = 0;
				parse_4_chars(0, true, tempResults, s + 16);
				if constexpr (positive) {
					if (results > (std::numeric_limits<value_type>::max() - tempResults)) {
						return false;
					}
				} else {
					if (results > (std::numeric_limits<value_type>::max() + 1ULL - tempResults)) {
						return false;
					}
				}
				results += tempResults;
				s += length;
			} else if constexpr (length == 19) {
				parse_8_chars(11, true, results, s);
				parse_8_chars(3, false, results, s + 8);
				parse_2_chars(1, false, results, s + 16);
				if constexpr (positive) {
					if (results > (std::numeric_limits<value_type>::max() - toDigit(*(s + 18)))) {
						return false;
					}
				} else {
					if (results > (std::numeric_limits<value_type>::max() + 1ULL - toDigit(*(s + 18)))) {
						return false;
					}
				}
				results += toDigit(*(s + 18));
				s += length;
			} else if constexpr (length == 18) {
				parse_8_chars(10, true, results, s);
				parse_8_chars(2, false, results, s + 8);
				parse_2_chars(0, false, results, s + 16);
				s += length;
			} else if constexpr (length == 17) {
				parse_8_chars(9, true, results, s);
				parse_8_chars(1, false, results, s + 8);
				results += toDigit(*(s + 16));
				s += length;
			} else if constexpr (length == 16) {
				parse_8_chars(8, true, results, s);
				parse_8_chars(0, false, results, s + 8);
				s += length;
			} else if constexpr (length == 15) {
				parse_8_chars(7, true, results, s);
				parse_4_chars(3, false, results, s + 8);
				parse_2_chars(1, false, results, s + 12);
				results += toDigit(*(s + 14));
				s += length;
			} else if constexpr (length == 14) {
				parse_8_chars(6, true, results, s);
				parse_4_chars(2, false, results, s + 8);
				parse_2_chars(0, false, results, s + 12);
				s += length;
			} else if constexpr (length == 13) {
				parse_8_chars(5, true, results, s);
				parse_4_chars(1, false, results, s + 8);
				results += toDigit(*(s + 12));
				s += length;
			} else if constexpr (length == 12) {
				parse_8_chars(4, true, results, s);
				parse_4_chars(0, false, results, s + 8);
				s += length;
			} else if constexpr (length == 11) {
				parse_8_chars(3, true, results, s);
				parse_2_chars(1, false, results, s + 8);
				results += toDigit(*(s + 10));
				s += length;
			} else if constexpr (length == 10) {
				parse_8_chars(2, true, results, s);
				parse_2_chars(0, false, results, s + 8);
				s += length;
			} else if constexpr (length == 9) {
				parse_8_chars(1, true, results, s);
				results += toDigit(*(s + 8));
				s += length;
			} else if constexpr (length == 8) {
				parse_8_chars(0, true, results, s);
				s += length;
			} else if constexpr (length == 7) {
				parse_4_chars(2, true, results, s);
				parse_2_chars(0, false, results, s + 4);
				results += toDigit(*(s + 6));
				s += length;
			} else if constexpr (length == 6) {
				parse_4_chars(2, true, results, s);
				parse_2_chars(0, false, results, s + 4);
				s += length;
			} else if constexpr (length == 5) {
				parse_4_chars(1, true, results, s);
				results += toDigit(*(s + 4));
				s += length;
			} else if constexpr (length == 4) {
				parse_4_chars(0, true, results, s);
				s += length;
			} else if constexpr (length == 3) {
				parse_2_chars(1, true, results, s);
				results += toDigit(*(s + 2));
				s += length;
			} else if constexpr (length == 2) {
				parse_2_chars(0, true, results, s);
				s += length;
			} else if constexpr (length == 1) {
				results += toDigit(*(s));
				s += length;
			}
			return true;
		}

		template<bool positive, size_t... indices> static constexpr auto generateFunctionPtrsImpl(std::index_sequence<indices...>) {
			using function_type = decltype(&integer_parser<value_type, char_type>::parseChars<positive, 0>);
			return std::array<function_type, sizeof...(indices)>{ &integer_parser<value_type, char_type>::parseChars<positive, indices>... };
		}

		template<bool positive> static constexpr auto generateFunctionPtrs() {
			return generateFunctionPtrsImpl<positive>(std::make_index_sequence<21>{});
		}

		JSONIFIER_ALWAYS_INLINE bool parseFraction(value_type& value, char_type*& iter) {
			fracDigits = 0;
			if JSONIFIER_LIKELY ((digiTable[static_cast<uint8_t>(iter[currentIndex])])) {
				fracValue = toDigit(iter[currentIndex]);
				++fracDigits;
				++currentIndex;
				while
					JSONIFIER_LIKELY((digiTable[static_cast<uint8_t>(iter[currentIndex])])) {
						fracValue = toDigit(iter[currentIndex]) + fracValue * 10;
						++fracDigits;
						++currentIndex;
					}
				return (expTable[static_cast<uint8_t>(iter[currentIndex])]) ? (++currentIndex, parseExponent(value, iter)) : true;
			}
			JSONIFIER_UNLIKELY(else) {
				return false;
			}
		}

		JSONIFIER_ALWAYS_INLINE bool parseExponent(value_type& value, char_type*& iter) {
			expSign = 1;
			if (plusOrMinusTable[static_cast<uint8_t>(iter[currentIndex])]) {
				if (iter[currentIndex] == minus) {
					expSign = -1;
				}
				++currentIndex;
			}
			if JSONIFIER_LIKELY ((digiTable[static_cast<uint8_t>(iter[currentIndex])])) {
				expValue = toDigit(iter[currentIndex]);
				++currentIndex;
				while
					JSONIFIER_LIKELY((digiTable[static_cast<uint8_t>(iter[currentIndex])])) {
						expValue = toDigit(iter[currentIndex]) + expValue * 10;
						++currentIndex;
					}
				return parseFinish(value, iter);
			}
			JSONIFIER_UNLIKELY(else) {
				return false;
			}
		}

		template<bool positive = true> JSONIFIER_ALWAYS_INLINE bool parseFinish(value_type& value, char_type*& iter) {
			if (intDigits > 0) {
				static constexpr auto functionPtrs{ generateFunctionPtrs<positive>() };
				(this->*functionPtrs[intDigits])(value, iter);
				return true;
			} else {
				return false;
			}
			if (intDigits > 0) {
				//functionPtrs[intDigits](value, iter);
				if (fracDigits > 0) {
					if JSONIFIER_LIKELY ((expValue <= 19)) {
						const auto powerExp = powerOfTenInt[expValue];

						expValue *= expSign;

						static constexpr value_type doubleMax = std::numeric_limits<value_type>::max();
						static constexpr value_type doubleMin = std::numeric_limits<value_type>::min();

						if (fracDigits + expValue >= 0) {
							const auto fractionalCorrection =
								expValue > fracDigits ? fracValue * powerOfTenInt[expValue - fracDigits] : fracValue / powerOfTenInt[fracDigits - expValue];
							return (expSign > 0) ? ((value <= (doubleMax / powerExp)) ? (value *= powerExp, value += fractionalCorrection, true) : false)
												 : ((value >= (doubleMin * powerExp)) ? (value /= powerExp, value += fractionalCorrection, true) : false);
						} else {
							return (expSign > 0) ? ((value <= (doubleMax / powerExp)) ? (value *= powerExp, true) : false)
												 : ((value >= (doubleMin * powerExp)) ? (value /= powerExp, true) : false);
						}
						return true;
					}
				}
				JSONIFIER_UNLIKELY(else) {
					return true;
				}
			} else {
				return false;
			}
		}

		template<bool positive = true> JSONIFIER_ALWAYS_INLINE bool parseInteger(value_type& value, char_type*& iter) {
			intDigits = 0;
			if JSONIFIER_LIKELY ((digiTable[static_cast<uint8_t>(iter[intDigits])])) {
				if (digiTable[iter[intDigits]]) {
					++intDigits;
				} else {
					return parseChars<positive, 0>(value, iter);
				}

				if (digiTable[iter[intDigits]]) {
					++intDigits;
				} else {
					return parseChars<positive, 1>(value, iter);
				}

				if (digiTable[iter[intDigits]]) {
					++intDigits;
				} else {
					return parseChars<positive, 2>(value, iter);
				}

				if (*iter == zero) {
					return false;
				}

				while (digiTable[iter[intDigits]]) {
					++intDigits;
				}

				switch (intDigits) {
					case 0: {
						return parseChars<positive, 0>(value, iter);
					}
					case 1: {
						return parseChars<positive, 1>(value, iter);
					}
					case 2: {
						return parseChars<positive, 2>(value, iter);
					}
					case 3: {
						return parseChars<positive, 3>(value, iter);
					}
					case 4: {
						return parseChars<positive, 4>(value, iter);
					}
					case 5: {
						return parseChars<positive, 5>(value, iter);
					}
					case 6: {
						return parseChars<positive, 6>(value, iter);
					}
					case 7: {
						return parseChars<positive, 7>(value, iter);
					}
					case 8: {
						return parseChars<positive, 8>(value, iter);
					}
					case 9: {
						return parseChars<positive, 9>(value, iter);
					}
					case 10: {
						return parseChars<positive, 10>(value, iter);
					}
					case 11: {
						return parseChars<positive, 11>(value, iter);
					}
					case 12: {
						return parseChars<positive, 12>(value, iter);
					}
					case 13: {
						return parseChars<positive, 13>(value, iter);
					}
					case 14: {
						return parseChars<positive, 14>(value, iter);
					}
					case 15: {
						return parseChars<positive, 15>(value, iter);
					}
					case 16: {
						return parseChars<positive, 16>(value, iter);
					}
					case 17: {
						return parseChars<positive, 17>(value, iter);
					}
					case 18: {
						return parseChars<positive, 18>(value, iter);
					}
					case 19: {
						return parseChars<positive, 19>(value, iter);
					}
					case 20: {
						return parseChars<positive, 20>(value, iter);
					}
				}
			}
			JSONIFIER_UNLIKELY(else) {
				return false;
			}
		}

		JSONIFIER_ALWAYS_INLINE bool parseInt(value_type& value, char_type*& iter) noexcept {
			jsonifierPrefetchImpl(iter);
			if constexpr (jsonifier::concepts::signed_type<value_type>) {
				static constexpr auto maxValue = size_t((std::numeric_limits<value_type>::max)());
				static constexpr auto minValue = size_t((std::numeric_limits<value_type>::max)()) + 1;
				bool result;
				return ((iter[currentIndex = 0] == minus ? (++iter, result = integer_parser<value_type, char_type>::parseInteger<false>(value, iter), value *= -1, result)
														 : integer_parser<value_type, char_type>::parseInteger(value, iter)));
			} else {
				currentIndex = 0;
				return integer_parser<value_type, char_type>::parseInteger(value, iter);
			}
		}
	};

}