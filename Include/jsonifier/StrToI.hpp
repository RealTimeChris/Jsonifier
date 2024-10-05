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
#include <jsonifier/Array.hpp>

#include <concepts>
#include <cstdint>
#include <cstring>
#include <array>

namespace jsonifier_internal {

	static constexpr array<double, 20> powerOfTenInt{ 1e0, 1e1, 1e2, 1e3, 1e4, 1e5, 1e6, 1e7, 1e8, 1e9, 1e10, 1e11, 1e12, 1e13, 1e14, 1e15, 1e16, 1e17, 1e18, 1e19 };

	JSONIFIER_ALWAYS_INLINE int64_t fastFloor(double x) {
		int64_t i = static_cast<int64_t>(x);
		return (x < i) ? i - 1 : i;
	}

	enum class parsing_state {
		starting				  = 0,
		collecting_integer_start  = 1,
		collecting_integer		  = 2,
		collecting_fractional	  = 3,
		finishing_fractional	  = 4,
		collecting_exponent		  = 5,
		checking_digit			  = 6,
		collecting_exponent_value = 7,
		finishing				  = 8,
	};

	static constexpr uint8_t digiTypeDigit = 1 << 1;
	static constexpr uint8_t digiTypePos   = 1 << 2;
	static constexpr uint8_t digiTypeNeg   = 1 << 3;
	static constexpr uint8_t digiTypeDot   = 1 << 4;
	static constexpr uint8_t digiTypeExp   = 1 << 5;

#define fastAbs(value) (value ^ (value >> 63)) - (value >> 63)

	constexpr array<uint8_t, 256> digiTable = { [] {
		array<uint8_t, 256> returnValues{};
		returnValues['0']					= digiTypeDigit;
		returnValues['1']					= digiTypeDigit;
		returnValues['2']					= digiTypeDigit;
		returnValues['3']					= digiTypeDigit;
		returnValues['4']					= digiTypeDigit;
		returnValues['5']					= digiTypeDigit;
		returnValues['6']					= digiTypeDigit;
		returnValues['7']					= digiTypeDigit;
		returnValues['8']					= digiTypeDigit;
		returnValues['9']					= digiTypeDigit;
		returnValues['+']					= digiTypePos;
		returnValues['-']					= digiTypeNeg;
		returnValues['.']					= digiTypeDot;
		returnValues['e']					= digiTypeExp;
		returnValues['E']					= digiTypeExp;
		return returnValues;
	}() };

#define isDigitOrFp(d) ((digiTable[d] & uint8_t(digiTypeDigit | digiTypeDot | digiTypeExp)) != 0)

#define isNumberValue(d) ((digiTable[d] & uint8_t(digiTypeNeg | digiTypeDigit)) != 0)

#define isPlusOrMinus(d) ((digiTable[d] & uint8_t(digiTypePos | digiTypeNeg)) != 0)

#define isExponent(d) ((digiTable[d] & uint8_t(digiTypeExp)) != 0)

	static constexpr char decimalPoint{ '.' };
	static constexpr char smallE{ 'e' };
	static constexpr char minus{ '-' };
	static constexpr char bigE{ 'E' };
	static constexpr char plus{ '+' };
	static constexpr char zero{ '0' };
	static constexpr char nine{ '9' };

#define isDigit(d) ((digiTable[d] & uint8_t(digiTypeDigit)) != 0)

#define toDigit(c) (static_cast<char>(c) - zero)

	template<size_t index, typename value_type, typename char_type> struct integer_parser_helper;

	template<typename value_type, typename char_type> struct integer_parser;

	template<jsonifier::concepts::unsigned_type value_type, typename char_type> struct integer_parser<value_type, char_type> {
		template<parsing_state state> struct parse_function_caller {
			JSONIFIER_MAYBE_ALWAYS_INLINE static bool parseValue(const char_type*& cur, value_type fracDigits, value_type& value, value_type fracValue, value_type intDigits,
				int64_t& exp, int8_t& expSign) {
				if constexpr (state == parsing_state::starting) {
					if (isDigit(*cur)) {
						value = toDigit(*cur);
						++cur;
						return parse_function_caller<parsing_state::collecting_integer>::parseValue(cur, fracDigits, value, fracValue, intDigits, exp, expSign);
					} else {
						return false;
					}
				} else if constexpr (state == parsing_state::collecting_integer) {
					if (isDigit(*cur)) {
						auto numTmp = toDigit(*cur);
						auto comparisonValue{ (std::numeric_limits<value_type>::max() - numTmp) / 10 };
						if (value > comparisonValue) [[unlikely]] {
							return false;
						}
						value = numTmp + value * 10;
						++cur;
						return parse_function_caller<parsing_state::collecting_integer>::parseValue(cur, fracDigits, value, fracValue, intDigits, exp, expSign);
					}
					if (isExponent(*cur)) {
						++cur;
						return parse_function_caller<parsing_state::collecting_exponent>::parseValue(cur, fracDigits, value, fracValue, intDigits, exp, expSign);
					} else if (*cur == '.') {
						++cur;
						return parse_function_caller<parsing_state::collecting_fractional>::parseValue(cur, fracDigits, value, fracValue, intDigits, exp, expSign);
					} else {
						return true;
					}
				} else if constexpr (state == parsing_state::collecting_fractional) {
					if (isDigit(*cur)) {
						fracValue = toDigit(*cur) + fracValue * 10;
						++fracDigits;
						++cur;
						return parse_function_caller<parsing_state::collecting_fractional>::parseValue(cur, fracDigits, value, fracValue, intDigits, exp, expSign);
					}
					return parse_function_caller<parsing_state::finishing_fractional>::parseValue(cur, fracDigits, value, fracValue, intDigits, exp, expSign);
				} else if constexpr (state == parsing_state::finishing_fractional) {
					if (isExponent(*cur)) {
						++cur;
						return parse_function_caller<parsing_state::collecting_exponent>::parseValue(cur, fracDigits, value, fracValue, intDigits, exp, expSign);
					} else {
						return true;
					}
				} else if constexpr (state == parsing_state::collecting_exponent) {
					if (isPlusOrMinus(*cur)) {
						if (*cur == '-') {
							expSign = -1;
						}
						++cur;
					}
					return parse_function_caller<parsing_state::checking_digit>::parseValue(cur, fracDigits, value, fracValue, intDigits, exp, expSign);

				} else if constexpr (state == parsing_state::checking_digit) {
					if (isDigit(*cur)) {
						return parse_function_caller<parsing_state::collecting_exponent_value>::parseValue(cur, fracDigits, value, fracValue, intDigits, exp, expSign);
					} else {
						return false;
					}

				} else if constexpr (state == parsing_state::collecting_exponent_value) {
					if (isDigit(*cur)) {
						exp = toDigit(*cur) + exp * 10;
						++cur;
						return parse_function_caller<parsing_state::collecting_exponent_value>::parseValue(cur, fracDigits, value, fracValue, intDigits, exp, expSign);
					}
					return parse_function_caller<parsing_state::finishing>::parseValue(cur, fracDigits, value, fracValue, intDigits, exp, expSign);
				} else if constexpr (state == parsing_state::finishing) {
					exp *= expSign;

					if (fastAbs(exp) > 19) [[unlikely]] {
						return false;
					}

					double fractionalCorrection = static_cast<double>(fracValue) / powerOfTenInt[fracDigits];
					double combinedValue		= static_cast<double>(value) + fractionalCorrection;
					static constexpr auto doubleMax{ static_cast<double>(std::numeric_limits<value_type>::max()) };
					static constexpr auto doubleMin{ static_cast<double>(std::numeric_limits<value_type>::min()) };
					if (exp > 0) {
						if (combinedValue > doubleMax / powerOfTenInt[exp]) [[unlikely]] {
							return false;
						}
						combinedValue *= powerOfTenInt[exp];
					} else {
						if (combinedValue < doubleMin * powerOfTenInt[-exp]) [[unlikely]] {
							return false;
						}
						combinedValue /= powerOfTenInt[-exp];
					}

					value = static_cast<value_type>(combinedValue);
					return true;
				} else {
					return false;
				}
			}
		};

		JSONIFIER_ALWAYS_INLINE static bool parseInt(value_type& value, char_type*& cur) noexcept {
			value_type fracDigits = 0;
			value_type intDigits  = 0;
			value_type fracValue  = 0;
			int64_t exp			  = 0;
			int8_t expSign		  = 1;
			return parse_function_caller<parsing_state::starting>::parseValue(cur, fracDigits, value, fracValue, intDigits, exp, expSign);
		}
	};

	template<jsonifier::concepts::signed_type value_type, typename char_type> struct integer_parser<value_type, char_type> {
		template<parsing_state state, bool positive> struct parse_function_caller {
			JSONIFIER_MAYBE_ALWAYS_INLINE static bool parseValue(const char_type*& cur, value_type fracDigits, value_type& value, value_type fracValue, value_type intDigits,
				int64_t& exp, int8_t& expSign) {
				if constexpr (state == parsing_state::starting) {
					if (isNumberValue(*cur)) {
						if (*cur == minus) {
							++cur;
							value = toDigit(*cur);
							++cur;
							return parse_function_caller<parsing_state::collecting_integer, false>::parseValue(cur, fracDigits, value, fracValue, intDigits, exp, expSign);
						}
						value = toDigit(*cur);
						++cur;
						return parse_function_caller<parsing_state::collecting_integer, true>::parseValue(cur, fracDigits, value, fracValue, intDigits, exp, expSign);
					} else {
						return false;
					}
				} else if constexpr (state == parsing_state::collecting_integer) {
					if (isDigit(*cur)) {
						auto numTmp = toDigit(*cur);
						auto posComparisonValue{ size_t(std::numeric_limits<value_type>::max() - numTmp) / 10 };
						auto negComparisonValue{ (static_cast<size_t>(std::numeric_limits<value_type>::min()) - numTmp) / 10 };
						if constexpr (positive) {
							if (value > posComparisonValue) [[unlikely]] {
								return false;
							}
						} else {
							if (value > negComparisonValue) [[unlikely]] {
								return false;
							}
						}
						value = numTmp + value * 10;
						++cur;
						return parse_function_caller<parsing_state::collecting_integer, positive>::parseValue(cur, fracDigits, value, fracValue, intDigits, exp, expSign);
					}
					if (isExponent(*cur)) {
						++cur;
						return parse_function_caller<parsing_state::collecting_exponent, positive>::parseValue(cur, fracDigits, value, fracValue, intDigits, exp, expSign);
					} else if (*cur == '.') {
						++cur;
						return parse_function_caller<parsing_state::collecting_fractional, positive>::parseValue(cur, fracDigits, value, fracValue, intDigits, exp, expSign);
					} else {
						if constexpr (positive) {
							return true;
						} else {
							value *= -1;
							return true;
						}
					}
				} else if constexpr (state == parsing_state::collecting_fractional) {
					if (isDigit(*cur)) {
						fracValue = toDigit(*cur) + fracValue * 10;
						++fracDigits;
						++cur;
						return parse_function_caller<parsing_state::collecting_fractional, positive>::parseValue(cur, fracDigits, value, fracValue, intDigits, exp, expSign);
					}
					return parse_function_caller<parsing_state::finishing_fractional, positive>::parseValue(cur, fracDigits, value, fracValue, intDigits, exp, expSign);
				} else if constexpr (state == parsing_state::finishing_fractional) {
					if (isExponent(*cur)) {
						++cur;
						return parse_function_caller<parsing_state::collecting_exponent, positive>::parseValue(cur, fracDigits, value, fracValue, intDigits, exp, expSign);
					} else {
						if constexpr (positive) {
							return true;
						} else {
							value *= -1;
							return true;
						}
					}
				} else if constexpr (state == parsing_state::collecting_exponent) {
					if (isPlusOrMinus(*cur)) {
						if (*cur == '-') {
							expSign = -1;
						}
						++cur;
					}
					return parse_function_caller<parsing_state::checking_digit, positive>::parseValue(cur, fracDigits, value, fracValue, intDigits, exp, expSign);

				} else if constexpr (state == parsing_state::checking_digit) {
					if (isDigit(*cur)) {
						return parse_function_caller<parsing_state::collecting_exponent_value, positive>::parseValue(cur, fracDigits, value, fracValue, intDigits, exp, expSign);
					} else {
						return false;
					}

				} else if constexpr (state == parsing_state::collecting_exponent_value) {
					if (isDigit(*cur)) {
						exp = toDigit(*cur) + exp * 10;
						++cur;
						return parse_function_caller<parsing_state::collecting_exponent_value, positive>::parseValue(cur, fracDigits, value, fracValue, intDigits, exp, expSign);
					}
					return parse_function_caller<parsing_state::finishing, positive>::parseValue(cur, fracDigits, value, fracValue, intDigits, exp, expSign);
				} else if constexpr (state == parsing_state::finishing) {
					exp *= expSign;

					if (fastAbs(exp) > 19) [[unlikely]] {
						return false;
					}

					auto fractionalCorrection = fracValue / powerOfTenInt[fracDigits];
					auto combinedValue		  = (value + fractionalCorrection);

					static constexpr auto doubleMax{ (std::numeric_limits<value_type>::max()) };
					static constexpr auto doubleMin{ (std::numeric_limits<value_type>::min()) };
					if (exp > 0) {
						if (combinedValue > doubleMax / powerOfTenInt[exp]) [[unlikely]] {
							return false;
						}
						combinedValue *= powerOfTenInt[exp];
					} else {
						if (combinedValue < doubleMin * powerOfTenInt[-exp]) [[unlikely]] {
							return false;
						}
						combinedValue /= powerOfTenInt[-exp];
					}

					if constexpr (positive) {
						value = static_cast<value_type>(combinedValue);

					} else {
						if (combinedValue > 0.0) {
							value = fastFloor(combinedValue * -1);
						} else {
							value = static_cast<value_type>(combinedValue);
						}
					}

					return true;
				} else {
					return false;
				}
			}
		};

		JSONIFIER_ALWAYS_INLINE static bool parseInt(value_type& value, char_type*& cur) noexcept {
			value_type fracDigits = 0;
			value_type intDigits  = 0;
			value_type fracValue  = 0;
			int8_t expSign		  = 1;
			int64_t exp			  = 0;
			return parse_function_caller<parsing_state::starting, true>::parseValue(cur, fracDigits, value, fracValue, intDigits, exp, expSign);
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
		if (!isDigit(*c)) [[unlikely]] {
			return false;
		}

		constexpr array<uint32_t, 4> maxDigitsFromSize = { 4, 6, 11, 20 };
		constexpr auto N							   = maxDigitsFromSize[static_cast<size_t>(std::bit_width(sizeof(value_type)) - 1)];

		array<uint8_t, N> digits{ 0 };
		auto nextDigit	  = digits.begin();
		auto consumeDigit = [&c, &nextDigit, &digits]() {
			if (nextDigit < digits.cend()) [[likely]] {
				*nextDigit = static_cast<uint8_t>(*c - 0x30u);
				++nextDigit;
			}
			++c;
		};

		if (*c == 0x30u) {
			++c;
			++nextDigit;

			if (*c == 0x30u) [[unlikely]] {
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

		if (*c == smallE || *c == bigE) {
			++c;

			bool negative = false;
			if (*c == plus || *c == minus) {
				negative = (*c == minus);
				++c;
			}
			uint8_t exp = 0;
			while (isDigit(*c) && exp < 128) {
				exp = static_cast<uint8_t>(10 * exp + (*c - 0x30u));
				++c;
			}
			n += negative ? -exp : exp;
		}

		res = 0;
		if (n < 0) [[unlikely]] {
			return true;
		}

		if constexpr (std::is_same_v<value_type, size_t>) {
			if (n > 20) [[unlikely]] {
				return false;
			}

			if (n == 20) [[unlikely]] {
				for (auto k = 0; k < 19; ++k) {
					res = static_cast<value_type>(10) * res + static_cast<value_type>(digits[static_cast<size_t>(k)]);
				}

				if (isSafeMultiplication10(res)) [[likely]] {
					res *= 10;
				} else [[unlikely]] {
					return false;
				}
				if (isSafeAddition(res, digits.back())) [[likely]] {
					res += digits.back();
				} else [[unlikely]] {
					return false;
				}
			} else [[likely]] {
				for (auto k = 0; k < n; ++k) {
					res = static_cast<value_type>(10) * res + static_cast<value_type>(digits[static_cast<size_t>(k)]);
				}
			}
		} else {
			if (n >= N) [[unlikely]] {
				return false;
			} else [[likely]] {
				for (auto k = 0; k < n; ++k) {
					res = static_cast<value_type>(10) * res + static_cast<value_type>(digits[static_cast<size_t>(k)]);
				}
			}
		}

		return true;
	}
}