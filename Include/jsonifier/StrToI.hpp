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
		auto rawCompVal{ std::numeric_limits<value_type>::max() };
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
		auto rawCompVal{ size_t((std::numeric_limits<value_type>::max)()) + 1 };
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
		returnValues['e'] = true;
		returnValues['E'] = true;
		return returnValues;
	}() };

	JSONIFIER_ALWAYS_INLINE_VARIABLE std::array<bool, 256> expFracTable{ [] {
		std::array<bool, 256> returnValues{};
		returnValues['e'] = true;
		returnValues['E'] = true;
		returnValues['.'] = true;
		return returnValues;
	}() };

	JSONIFIER_ALWAYS_INLINE_VARIABLE std::array<bool, 256> plusOrMinusTable{ [] {
		std::array<bool, 256> returnValues{};
		returnValues['-'] = true;
		returnValues['+'] = true;
		return returnValues;
	}() };

#define isPlusOrMinus(c) plusOrMinusTable[static_cast<uint8_t>(c)]

#define isExponentOrFractional(c) expFracTable[static_cast<uint8_t>(c)]

#define isExponent(c) expTable[static_cast<uint8_t>(c)]

#define isDigit(c) digiTable[static_cast<uint8_t>(c)]

	JSONIFIER_ALWAYS_INLINE_VARIABLE char decimalPoint{ '.' };
	JSONIFIER_ALWAYS_INLINE_VARIABLE char zero{ '0' };

#define toDigit(c) (static_cast<char>(c) - zero)

	template<typename value_type, typename char_type> struct integer_parser {
		int64_t fracDigits;
		size_t fracValue;
		int8_t expSign;
		int8_t numTmp;
		int64_t expValue;

		JSONIFIER_ALWAYS_INLINE bool parseFraction(const char_type*& iter, value_type& value) {
			fracDigits = 0;
			if JSONIFIER_LIKELY ((isDigit(*iter))) {
				fracValue = toDigit(*iter);
				++fracDigits;
				++iter;
				while (isDigit(*iter)) [[likely]] {
					fracValue = toDigit(*iter) + fracValue * 10;
					++fracDigits;
					++iter;
				}
				return (isExponent(*iter)) ? (++iter, parseExponent(iter, value)) : true;
			}
			JSONIFIER_UNLIKELY(else) {
				return false;
			}
		}

		JSONIFIER_ALWAYS_INLINE bool parseExponent(const char_type*& iter, value_type& value) {
			expSign = 1;
			if (isPlusOrMinus(*iter)) {
				if (*iter == '-') {
					expSign = -1;
				}
				++iter;
			}
			if JSONIFIER_LIKELY ((isDigit(*iter))) {
				expValue = toDigit(*iter);
				++iter;
				while (isDigit(*iter)) [[likely]] {
					expValue = toDigit(*iter) + expValue * 10;
					++iter;
				}
				return parseFinish(iter, value);
			}
			JSONIFIER_UNLIKELY(else) {
				return false;
			}
		}

		JSONIFIER_ALWAYS_INLINE bool parseFinish(const char_type*& iter, value_type& value) {
			if JSONIFIER_LIKELY ((expValue <= 19)) {
				const auto powerExp = powerOfTenInt[expValue];

				expValue *= expSign;

				static constexpr value_type doubleMax = std::numeric_limits<value_type>::max();
				static constexpr value_type doubleMin = std::numeric_limits<value_type>::min();

				if (fracDigits + expValue >= 0) {
					auto fractionalCorrection = expValue > fracDigits ? fracValue * powerOfTenInt[expValue - fracDigits] : fracValue / powerOfTenInt[fracDigits - expValue];
					return (expValue > 0) ? ((value <= (doubleMax / powerExp)) ? (value *= powerExp, value += fractionalCorrection, true) : false)
										  : ((value >= (doubleMin * powerExp)) ? (value /= powerExp, value += fractionalCorrection, true) : false);
				} else {
					return (expValue > 0) ? ((value <= (doubleMax / powerExp)) ? (value *= powerExp, true) : false)
										  : ((value >= (doubleMin * powerExp)) ? (value /= powerExp, true) : false);
				}
				return true;
			}
			JSONIFIER_UNLIKELY(else) {
				return false;
			}
		}

		template<bool positive = true> JSONIFIER_ALWAYS_INLINE bool parseInteger(const char_type*& iter, value_type& value) {
			static constexpr auto maxLoopIndex{ jsonifier::concepts::signed_type<value_type> ? 16ull : 17ull };
			if JSONIFIER_LIKELY ((isDigit(*iter))) {
				value = toDigit(*iter);
				++iter;

				if JSONIFIER_LIKELY ((isDigit(*iter))) {
					value = toDigit(*iter) + value * 10;
					++iter;
				}
				JSONIFIER_UNLIKELY(else) {
					return (!isExponentOrFractional(*iter)) ? true
						: (*iter == decimalPoint)			? (++iter, parseFraction(iter, value))
															: (fracDigits = 0, fracValue = 0, ++iter, parseExponent(iter, value));
				}

				if JSONIFIER_LIKELY ((*(iter - 2) != zero)) {
					size_t x{};
					for (; x < maxLoopIndex; ++x) [[likely]] {
						if (isDigit(*iter)) {
							value = toDigit(*iter) + value * 10;
							++iter;
						} else {
							return (!isExponentOrFractional(*iter)) ? true
								: (*iter == decimalPoint)			? (++iter, parseFraction(iter, value))
																	: (fracDigits = 0, fracValue = 0, ++iter, parseExponent(iter, value));
						}
					}
					if JSONIFIER_UNLIKELY ((x > maxLoopIndex - 1 && isDigit(*iter))) {
						numTmp = toDigit(*iter);
						if constexpr (jsonifier::concepts::unsigned_type<value_type>) {
							if JSONIFIER_UNLIKELY ((value > rawCompValsPos<value_type>[numTmp])) {
								return false;
							}
						} else {
							if constexpr (positive) {
								if JSONIFIER_UNLIKELY ((value > rawCompValsPos<value_type>[numTmp])) {
									return false;
								}
							} else {
								if JSONIFIER_UNLIKELY ((value > rawCompValsNeg<value_type>[numTmp])) {
									return false;
								}
							}
						}
						value = numTmp + value * 10;
						++iter;
					}
					return true;
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
				return ((*iter == '-' ? (++iter, result = integer_parser<value_type, char_type>::parseInteger<false>(iter, value), value *= -1, result)
									  : integer_parser<value_type, char_type>::parseInteger(iter, value)));
			} else {
				return integer_parser<value_type, char_type>::parseInteger(iter, value);
			}
		}
	};

	JSONIFIER_ALWAYS_INLINE constexpr bool isSafeAddition(size_t a, size_t b) noexcept {
		return a <= (std::numeric_limits<size_t>::max)() - b;
	}

	JSONIFIER_ALWAYS_INLINE constexpr bool isSafeMultiplication10(size_t a) noexcept {
		constexpr size_t b = (std::numeric_limits<size_t>::max)();
		return a <= b;
	}

	template<typename value_type, typename char_type> JSONIFIER_ALWAYS_INLINE constexpr bool stoui64(value_type res, const char_type* c) noexcept {
		if JSONIFIER_UNLIKELY ((!isDigit(*c))) {
			return false;
		}

		constexpr std::array<uint32_t, 4> maxDigitsFromSize = { 4, 6, 11, 20 };
		constexpr auto N									= maxDigitsFromSize[static_cast<size_t>(std::bit_width(sizeof(value_type)) - 1)];

		std::array<uint8_t, N> digits{ 0 };
		auto nextDigit	  = digits.begin();
		auto consumeDigit = [&c, &nextDigit, &digits]() {
			if JSONIFIER_LIKELY ((nextDigit < digits.cend())) {
				*nextDigit = static_cast<uint8_t>(*c - 0x30u);
				++nextDigit;
			}
			++c;
		};

		if (*c == 0x30u) {
			++c;
			++nextDigit;

			if JSONIFIER_UNLIKELY ((*c == 0x30u)) {
				return false;
			}
		}

		while (isDigit(*c)) {
			consumeDigit();
		}
		auto n = std::distance(digits.begin(), nextDigit);

		if (*c == decimalPoint) {
			++c;
			while (isDigit(*c)) {
				consumeDigit();
			}
		}

		if (isExponent(*c)) {
			++c;

			bool negative = false;
			if (isPlusOrMinus(*c)) {
				negative = (*c == '-');
				++c;
			}
			uint8_t expValue = 0;
			while (isDigit(*c) && expValue < 128) {
				expValue = static_cast<uint8_t>(10 * expValue + (*c - 0x30u));
				++c;
			}
			n += negative ? -expValue : expValue;
		}

		res = 0;
		if JSONIFIER_UNLIKELY ((n < 0)) {
			return true;
		}

		if constexpr (std::is_same_v<value_type, size_t>) {
			if JSONIFIER_UNLIKELY ((n > 20)) {
				return false;
			}

			if JSONIFIER_UNLIKELY ((n == 20)) {
				for (auto k = 0; k < 19; ++k) {
					res = static_cast<value_type>(10) * res + static_cast<value_type>(digits[static_cast<size_t>(k)]);
				}

				if JSONIFIER_LIKELY ((isSafeMultiplication10(res))) {
					res *= 10;
				}
				JSONIFIER_UNLIKELY(else) {
					return false;
				}
				if JSONIFIER_LIKELY ((isSafeAddition(res, digits.back()))) {
					res += digits.back();
				}
				JSONIFIER_UNLIKELY(else) {
					return false;
				}
			} else [[likely]] {
				for (auto k = 0; k < n; ++k) {
					res = static_cast<value_type>(10) * res + static_cast<value_type>(digits[static_cast<size_t>(k)]);
				}
			}
		} else {
			if JSONIFIER_UNLIKELY ((n >= N)) {
				return false;
			}
			JSONIFIER_LIKELY(else) {
				for (auto k = 0; k < n; ++k) {
					res = static_cast<value_type>(10) * res + static_cast<value_type>(digits[static_cast<size_t>(k)]);
				}
			}
		}

		return true;
	}
}