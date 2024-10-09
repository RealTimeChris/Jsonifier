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

	JSONIFIER_ALWAYS_INLINE_VARIABLE char decimal{ '.' };
	JSONIFIER_ALWAYS_INLINE_VARIABLE char minus{ '-' };
	JSONIFIER_ALWAYS_INLINE_VARIABLE char zero{ '0' };

#define toDigit(c) (static_cast<value_type>(static_cast<char>(c) - zero))

	template<typename value_type, typename char_type> struct integer_parser {
		JSONIFIER_ALWAYS_INLINE bool parseFraction(value_type& value, char_type*& iter, std::remove_const_t<char_type>& numTmpNew) {
			numTmpNew								  = *iter;
			std::remove_const_t<char_type> fracDigits = 0;
			if JSONIFIER_LIKELY ((digiTable[static_cast<uint8_t>(numTmpNew)])) {
				int64_t fracValue = toDigit(numTmpNew);
				++fracDigits;
				++iter;
				numTmpNew = *iter;
				while
					JSONIFIER_LIKELY((digiTable[static_cast<uint8_t>(numTmpNew)])) {
						fracValue = toDigit(numTmpNew) + fracValue * 10;
						++fracDigits;
						++iter;
						numTmpNew = *iter;
					}
				return (expTable[static_cast<uint8_t>(numTmpNew)]) ? (++iter, parseExponent(value, iter, numTmpNew, fracDigits, fracValue)) : true;
			}
			JSONIFIER_UNLIKELY(else) {
				return false;
			}
		}

		JSONIFIER_ALWAYS_INLINE bool parseExponent(value_type& value, char_type*& iter, std::remove_const_t<char_type>& numTmpNew, int8_t fracDigits = 0, int64_t fracValue = 0) {
			numTmpNew	   = *iter;
			int8_t expSign = 1;
			if (plusOrMinusTable[static_cast<uint8_t>(numTmpNew)]) {
				if (numTmpNew == minus) {
					expSign = -1;
				}
				++iter;
			}
			numTmpNew = *iter;
			if JSONIFIER_LIKELY ((digiTable[static_cast<uint8_t>(numTmpNew)])) {
				int64_t expValue = toDigit(numTmpNew);
				++iter;
				numTmpNew = *iter;
				while
					JSONIFIER_LIKELY((digiTable[static_cast<uint8_t>(numTmpNew)])) {
						expValue = toDigit(numTmpNew) + expValue * 10;
						++iter;
						numTmpNew = *iter;
					}
				return parseFinish(value, iter, expSign, expValue, fracDigits, fracValue);
			}
			JSONIFIER_UNLIKELY(else) {
				return false;
			}
		}

		JSONIFIER_ALWAYS_INLINE bool parseFinish(value_type& value, char_type*& iter, int8_t& expSign, int64_t& expValue, int8_t& fracDigits, int64_t& fracValue) {
			if JSONIFIER_LIKELY ((expValue <= 19)) {
				const auto powerExp = powerOfTenInt[expValue];

				expValue *= expSign;

				static constexpr value_type doubleMax = std::numeric_limits<value_type>::max();
				static constexpr value_type doubleMin = std::numeric_limits<value_type>::min();

				if (fracDigits + expValue >= 0) {
					const auto fractionalCorrection = expValue > fracDigits ? fracValue * powerOfTenInt[expValue - fracDigits] : fracValue / powerOfTenInt[fracDigits - expValue];
					return (expSign > 0) ? ((value <= (doubleMax / powerExp)) ? (value *= powerExp, value += fractionalCorrection, true) : false)
										 : ((value >= (doubleMin * powerExp)) ? (value /= powerExp, value += fractionalCorrection, true) : false);
				} else {
					return (expSign > 0) ? ((value <= (doubleMax / powerExp)) ? (value *= powerExp, true) : false)
										 : ((value >= (doubleMin * powerExp)) ? (value /= powerExp, true) : false);
				}
				return true;
			}
			JSONIFIER_UNLIKELY(else) {
				return false;
			}
		}

		template<bool positive = true> JSONIFIER_ALWAYS_INLINE bool parseInteger(value_type& value, char_type*& iter) {
			static constexpr auto maxLoopIndex{ jsonifier::concepts::signed_type<value_type> ? 16ull : 17ull };
			std::remove_const_t<char_type> numTmpNew = *iter;
			if JSONIFIER_LIKELY ((digiTable[static_cast<uint8_t>(numTmpNew)])) {
				value = toDigit(numTmpNew);
				++iter;
				numTmpNew = *iter;

				if JSONIFIER_LIKELY ((digiTable[static_cast<uint8_t>(numTmpNew)])) {
					value = toDigit(numTmpNew) + value * 10;
					++iter;
				}
				JSONIFIER_UNLIKELY(else) {
					return (!expFracTable[static_cast<uint8_t>(numTmpNew)]) ? true
						: (numTmpNew == decimal)							? (++iter, parseFraction(value, iter, numTmpNew))
																			: (++iter, parseExponent(value, iter, numTmpNew));
				}
				numTmpNew = *iter;

				if JSONIFIER_LIKELY ((*(iter - 2) != zero)) {
					int8_t x{};
					for
						JSONIFIER_LIKELY((; x < maxLoopIndex; ++x)) {
							if (digiTable[static_cast<uint8_t>(numTmpNew)]) {
								value = toDigit(numTmpNew) + value * 10;
								++iter;
								numTmpNew = *iter;
							} else {
								break;
							}
						}
					if JSONIFIER_UNLIKELY ((x > maxLoopIndex - 1 && digiTable[static_cast<uint8_t>(numTmpNew)])) {
						numTmpNew = toDigit(numTmpNew);
						if constexpr (positive) {
							if JSONIFIER_UNLIKELY ((value > rawCompValsPos<value_type>[numTmpNew])) {
								return false;
							}
						} else {
							if JSONIFIER_UNLIKELY ((value > rawCompValsNeg<value_type>[numTmpNew])) {
								return false;
							}
						}
						value = numTmpNew + value * 10;
						++iter;
						return true;
					} else {
						return (!expFracTable[static_cast<uint8_t>(numTmpNew)]) ? true
							: (numTmpNew == decimal)							? (++iter, parseFraction(value, iter, numTmpNew))
																				: (++iter, parseExponent(value, iter, numTmpNew));
					}
				}
				JSONIFIER_UNLIKELY(else) {
					return false;
				}
			}
			JSONIFIER_UNLIKELY(else) {
				return false;
			}
		}

		JSONIFIER_ALWAYS_INLINE bool parseInt(value_type& value, char_type*& iter) noexcept {
			if constexpr (jsonifier::concepts::signed_type<value_type>) {
				static constexpr auto maxValue = size_t((std::numeric_limits<value_type>::max)());
				static constexpr auto minValue = size_t((std::numeric_limits<value_type>::max)()) + 1;
				bool result;
				return ((*iter == minus ? (++iter, result = integer_parser<value_type, char_type>::parseInteger<false>(value, iter), value *= -1, result)
										: integer_parser<value_type, char_type>::parseInteger(value, iter)));
			} else {
				return integer_parser<value_type, char_type>::parseInteger(value, iter);
			}
		}
	};

}