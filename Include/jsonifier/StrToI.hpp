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

	JSONIFIER_ALWAYS_INLINE_VARIABLE std::array<double, 20> powerOfTenInt{ 1e0, 1e1, 1e2, 1e3, 1e4, 1e5, 1e6, 1e7, 1e8, 1e9, 1e10, 1e11, 1e12, 1e13, 1e14, 1e15, 1e16, 1e17, 1e18,
		1e19 };

	JSONIFIER_ALWAYS_INLINE int64_t fastFloor(double x) {
		int64_t i = static_cast<int64_t>(x);
		return (x < i) ? i - 1 : i;
	}

	enum class parsing_state {
		starting				  = 0,
		collecting_integer		  = 1,
		finishing_integer		  = 2,
		collecting_fractional	  = 3,
		finishing_fractional	  = 4,
		collecting_exponent		  = 5,
		collecting_exponent_value = 6,
		finishing_exponent		  = 7
	};

	JSONIFIER_ALWAYS_INLINE_VARIABLE uint8_t digiTypeDigit = 1 << 1;
	JSONIFIER_ALWAYS_INLINE_VARIABLE uint8_t digiTypePos   = 1 << 2;
	JSONIFIER_ALWAYS_INLINE_VARIABLE uint8_t digiTypeNeg   = 1 << 3;
	JSONIFIER_ALWAYS_INLINE_VARIABLE uint8_t digiTypeDot   = 1 << 4;
	JSONIFIER_ALWAYS_INLINE_VARIABLE uint8_t digiTypeExp   = 1 << 5;

#define fastAbs(value) (value ^ (value >> 63)) - (value >> 63)

	constexpr std::array<uint8_t, 256> digiTable = { [] {
		std::array<uint8_t, 256> returnValues{};
		returnValues['0']						 = 1 << 1;
		returnValues['1']						 = 1 << 1;
		returnValues['2']						 = 1 << 1;
		returnValues['3']						 = 1 << 1;
		returnValues['4']						 = 1 << 1;
		returnValues['5']						 = 1 << 1;
		returnValues['6']						 = 1 << 1;
		returnValues['7']						 = 1 << 1;
		returnValues['8']						 = 1 << 1;
		returnValues['9']						 = 1 << 1;
		returnValues['+']						 = 1 << 2;
		returnValues['-']						 = 1 << 3;
		returnValues['.']						 = 1 << 4;
		returnValues['e']						 = 1 << 5;
		returnValues['E']						 = 1 << 5;
		return returnValues;
	}() };

#define isDigitOrFp(d) ((digiTable[d] & uint8_t(digiTypeDigit | digiTypeDot | digiTypeExp)) != 0)

#define isNumberValue(d) ((digiTable[d] & uint8_t(digiTypeExp | digiTypeDot | digiTypeNeg | digiTypePos | digiTypeDigit)) != 0)

#define isPlusOrMinus(d) (digiTable[d] & uint8_t(digiTypePos | digiTypeNeg))

#define isExponent(d) (digiTable[d] & uint8_t(digiTypeExp))

#define isFracOrExponent(d) (digiTable[d] & uint8_t(digiTypeExp | digiTypeDot))

	JSONIFIER_ALWAYS_INLINE_VARIABLE char decimalPoint{ '.' };
	JSONIFIER_ALWAYS_INLINE_VARIABLE char smallE{ 'e' };
	JSONIFIER_ALWAYS_INLINE_VARIABLE char minus{ '-' };
	JSONIFIER_ALWAYS_INLINE_VARIABLE char bigE{ 'E' };
	JSONIFIER_ALWAYS_INLINE_VARIABLE char plus{ '+' };
	JSONIFIER_ALWAYS_INLINE_VARIABLE char zero{ '0' };
	JSONIFIER_ALWAYS_INLINE_VARIABLE char nine{ '9' };

#define isDigit(d) (digiTable[d] & uint8_t(digiTypeDigit))

#define toDigit(c) (static_cast<char>(c) - zero)

	template<size_t index, typename value_type, typename char_type> struct integer_parser_helper;

	template<typename value_type, typename char_type> struct integer_parser;

	template<jsonifier::concepts::unsigned_type value_type, typename char_type> struct integer_parser<value_type, char_type> {
		static constexpr auto maxIndex{ 21ull };

		struct parser_function_caller {
			template<parsing_state state> JSONIFIER_MAYBE_ALWAYS_INLINE static bool parseValue(const char_type*& cur, value_type fracDigits, value_type& value,
				value_type fracValue, value_type intDigits, int64_t& exp, int8_t& expSign, uint8_t numTmp = 0) {
				if constexpr (state == parsing_state::starting) {
					if (isNumberValue(*cur)) {
						return callFunction<parsing_state::collecting_integer>(cur, fracDigits, value, fracValue, intDigits, exp, expSign);
					}
				} else if constexpr (state == parsing_state::collecting_integer) {
					if (isDigit(*cur)) {
						auto numTmpNew = toDigit(*cur);
						value		   = numTmpNew + value * 10;
						++cur;
						return callFunction<parsing_state::collecting_integer>(cur, fracDigits, value, fracValue, intDigits, exp, expSign, numTmpNew);
					}
					if (((value - numTmp) / 10) > (std::numeric_limits<value_type>::max() - numTmp) / 10) [[unlikely]] {
						return false;
					}
					return callFunction<parsing_state::finishing_integer>(cur, fracDigits, value, fracValue, intDigits, exp, expSign);
				} else if constexpr (state == parsing_state::finishing_integer) {
					if (isExponent(*cur)) {
						++cur;
						return callFunction<parsing_state::collecting_exponent>(cur, fracDigits, value, fracValue, intDigits, exp, expSign);
					} else if (*cur == '.') {
						++cur;
						return callFunction<parsing_state::collecting_fractional>(cur, fracDigits, value, fracValue, intDigits, exp, expSign);
					} else {
						return true;
					}
				} else if constexpr (state == parsing_state::collecting_fractional) {
					if (isDigit(*cur)) {
						auto numTmp = toDigit(*cur);
						fracValue	= numTmp + fracValue * 10;
						++fracDigits;
						++cur;
						return callFunction<parsing_state::collecting_fractional>(cur, fracDigits, value, fracValue, intDigits, exp, expSign);
					}
					return callFunction<parsing_state::finishing_fractional>(cur, fracDigits, value, fracValue, intDigits, exp, expSign);
				} else if constexpr (state == parsing_state::finishing_fractional) {
					if (isExponent(*cur)) {
						++cur;
						return callFunction<parsing_state::collecting_exponent>(cur, fracDigits, value, fracValue, intDigits, exp, expSign);
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
					if (!isDigit(*cur)) {
						return false;
					}
					return callFunction<parsing_state::collecting_exponent_value>(cur, fracDigits, value, fracValue, intDigits, exp, expSign);
				} else if constexpr (state == parsing_state::collecting_exponent_value) {
					if (isDigit(*cur)) {
						auto numTmpNew = toDigit(*cur);
						exp			   = numTmpNew + exp * 10;
						++cur;
						return callFunction<parsing_state::collecting_exponent_value>(cur, fracDigits, value, fracValue, intDigits, exp, expSign);
					}
					exp *= expSign;
					if (fastAbs(exp) > 19) [[unlikely]] {
						return false;
					}
					return callFunction<parsing_state::finishing_exponent>(cur, fracDigits, value, fracValue, intDigits, exp, expSign);
				} else if constexpr (state == parsing_state::finishing_exponent) {
					double fractionalCorrection = static_cast<double>(fracValue) / powerOfTenInt[fracDigits];
					double combinedValue		= (static_cast<double>(value) + fractionalCorrection);
					if (exp > 0) {
						if (combinedValue > static_cast<double>(std::numeric_limits<value_type>::max()) / powerOfTenInt[exp]) [[unlikely]] {
							return false;
						}
						combinedValue *= powerOfTenInt[exp];
					} else {
						if (combinedValue < static_cast<double>(std::numeric_limits<value_type>::min()) * powerOfTenInt[-exp]) [[unlikely]] {
							return false;
						}
						combinedValue /= powerOfTenInt[-exp];
					}

					value = static_cast<value_type>(combinedValue);
					return true;
				} else {
					return false;
				}
				return false;
			};
		};

		static constexpr auto generateFunctionPtrsImpl() {
			using function_type = decltype(&parser_function_caller::template parseValue<parsing_state::starting>);
			std::array<function_type, 8> returnValues{};
			returnValues[0] = &parser_function_caller::template parseValue<parsing_state::starting>;
			returnValues[1] = &parser_function_caller::template parseValue<parsing_state::collecting_integer>;
			returnValues[2] = &parser_function_caller::template parseValue<parsing_state::finishing_integer>;
			returnValues[3] = &parser_function_caller::template parseValue<parsing_state::collecting_fractional>;
			returnValues[4] = &parser_function_caller::template parseValue<parsing_state::finishing_fractional>;
			returnValues[5] = &parser_function_caller::template parseValue<parsing_state::collecting_exponent>;
			returnValues[6] = &parser_function_caller::template parseValue<parsing_state::collecting_exponent_value>;
			returnValues[7] = &parser_function_caller::template parseValue<parsing_state::finishing_exponent>;
			return returnValues;
		}

		static constexpr auto generateFunctionPtrs() {
			return generateFunctionPtrsImpl();
		}

		static constexpr auto functionPtrs{ generateFunctionPtrs() };

		template<parsing_state state> JSONIFIER_ALWAYS_INLINE static bool callFunction(const char_type*& cur, value_type fracDigits, value_type& value, value_type fracValue,
			value_type intDigits, int64_t& exp, int8_t& expSign, uint8_t numTmp = 0) {
			return functionPtrs[static_cast<int64_t>(state)](cur, fracDigits, value, fracValue, intDigits, exp, expSign, numTmp);
		}

		JSONIFIER_ALWAYS_INLINE static int8_t parseInt(value_type& value, char_type*& cur) noexcept {
			value_type fracDigits = 0;
			value_type intDigits  = 0;
			value_type fracValue  = 0;
			int64_t exp			  = 0;
			int8_t expSign		  = 1;
			return callFunction<parsing_state::starting>(cur, fracDigits, value, fracValue, intDigits, exp, expSign);
		}
	};

	template<jsonifier::concepts::signed_type value_type, typename char_type> struct integer_parser<value_type, char_type> {
		static constexpr auto maxIndex{ 20ull };

		struct parser_function_caller {
			template<parsing_state state, bool positive> JSONIFIER_MAYBE_ALWAYS_INLINE static bool parseValue(const char_type*& cur, int8_t sign, value_type fracDigits,
				value_type& value, value_type fracValue, value_type intDigits, int64_t& exp, int8_t& expSign, uint8_t numTmp = 0) {
				if constexpr (state == parsing_state::starting) {
					if (isNumberValue(*cur)) {
						return callFunction<parsing_state::collecting_integer>(cur, sign, fracDigits, value, fracValue, intDigits, exp, expSign);
					}
				} else if constexpr (state == parsing_state::collecting_integer) {
					if (isDigit(*cur)) {
						auto numTmpNew = toDigit(*cur);
						value		   = numTmpNew + value * 10;
						++cur;
						return callFunction<parsing_state::collecting_integer>(cur, sign, fracDigits, value, fracValue, intDigits, exp, expSign, numTmpNew);
					}
					if constexpr (positive) {
						if (((value - numTmp) / 10) > size_t(std::numeric_limits<value_type>::max() - numTmp) / 10) [[unlikely]] {
							return 0;
						}
					} else {
						if (((value - numTmp) / 10) > size_t(-(std::numeric_limits<value_type>::min()) - numTmp) / 10) [[unlikely]] {
							return 0;
						}
					}
					return callFunction<parsing_state::finishing_integer>(cur, sign, fracDigits, value, fracValue, intDigits, exp, expSign);
				} else if constexpr (state == parsing_state::finishing_integer) {
					if (isExponent(*cur)) {
						++cur;
						return callFunction<parsing_state::collecting_exponent>(cur, sign, fracDigits, value, fracValue, intDigits, exp, expSign);
					} else if (*cur == '.') {
						++cur;
						return callFunction<parsing_state::collecting_fractional>(cur, sign, fracDigits, value, fracValue, intDigits, exp, expSign);
					} else {
						value *= sign;
						return true;
					}
				} else if constexpr (state == parsing_state::collecting_fractional) {
					if (isDigit(*cur)) {
						auto numTmp = toDigit(*cur);
						fracValue	= numTmp + fracValue * 10;
						++fracDigits;
						++cur;
						return callFunction<parsing_state::collecting_fractional>(cur, sign, fracDigits, value, fracValue, intDigits, exp, expSign);
					}
					return callFunction<parsing_state::finishing_fractional>(cur, sign, fracDigits, value, fracValue, intDigits, exp, expSign);
				} else if constexpr (state == parsing_state::finishing_fractional) {
					if (isExponent(*cur)) {
						++cur;
						return callFunction<parsing_state::collecting_exponent>(cur, sign, fracDigits, value, fracValue, intDigits, exp, expSign);
					} else {
						value *= sign;
						return true;
					}
				} else if constexpr (state == parsing_state::collecting_exponent) {
					if (isPlusOrMinus(*cur)) {
						if (*cur == '-') {
							expSign = -1;
						}
						++cur;
					}
					if (!isDigit(*cur)) {
						return false;
					}
					return callFunction<parsing_state::collecting_exponent_value>(cur, sign, fracDigits, value, fracValue, intDigits, exp, expSign);
				} else if constexpr (state == parsing_state::collecting_exponent_value) {
					if (isDigit(*cur)) {
						auto numTmpNew = toDigit(*cur);
						exp			   = numTmpNew + exp * 10;
						++cur;
						return callFunction<parsing_state::collecting_exponent_value>(cur, sign, fracDigits, value, fracValue, intDigits, exp, expSign);
					}
					exp *= expSign;
					if (fastAbs(exp) > 19) [[unlikely]] {
						return false;
					}
					return callFunction<parsing_state::finishing_exponent>(cur, sign, fracDigits, value, fracValue, intDigits, exp, expSign);
				} else if constexpr (state == parsing_state::finishing_exponent) {
					double fractionalCorrection = static_cast<double>(fracValue) / powerOfTenInt[fracDigits];
					double combinedValue		= (static_cast<double>(value) + fractionalCorrection);
					if (exp > 0) {
						if (combinedValue > static_cast<double>(std::numeric_limits<value_type>::max()) / powerOfTenInt[exp]) [[unlikely]] {
							return 0;
						}
						combinedValue *= powerOfTenInt[exp];
					} else {
						if (combinedValue < static_cast<double>(std::numeric_limits<value_type>::min()) * powerOfTenInt[-exp]) [[unlikely]] {
							return 0;
						}
						combinedValue /= powerOfTenInt[-exp];
					}

					if (sign == -1 && combinedValue > 0.0) {
						value = fastFloor(combinedValue * sign);
					} else {
						value = static_cast<value_type>(combinedValue) * sign;
					}

					return true;
				} else {
					return false;
				}
				return false;
			};
		};

		template<bool positive> static constexpr auto generateFunctionPtrsImpl() {
			using function_type = decltype(&parser_function_caller::template parseValue<parsing_state::starting, positive>);
			std::array<function_type, 8> returnValues{};
			returnValues[0] = &parser_function_caller::template parseValue<parsing_state::starting, positive>;
			returnValues[1] = &parser_function_caller::template parseValue<parsing_state::collecting_integer, positive>;
			returnValues[2] = &parser_function_caller::template parseValue<parsing_state::finishing_integer, positive>;
			returnValues[3] = &parser_function_caller::template parseValue<parsing_state::collecting_fractional, positive>;
			returnValues[4] = &parser_function_caller::template parseValue<parsing_state::finishing_fractional, positive>;
			returnValues[5] = &parser_function_caller::template parseValue<parsing_state::collecting_exponent, positive>;
			returnValues[6] = &parser_function_caller::template parseValue<parsing_state::collecting_exponent_value, positive>;
			returnValues[7] = &parser_function_caller::template parseValue<parsing_state::finishing_exponent, positive>;
			return returnValues;
		}

		template<bool positive> static constexpr auto generateFunctionPtrs() {
			return generateFunctionPtrsImpl<positive>();
		}

		template<parsing_state state> JSONIFIER_ALWAYS_INLINE static bool callFunction(const char_type*& cur, int8_t sign, value_type fracDigits, value_type& value,
			value_type fracValue, value_type intDigits, int64_t& exp, int8_t& expSign, uint8_t numTmp = 0) {
			if (sign == 1) {
				static constexpr auto functionPtrs{ generateFunctionPtrs<true>() };
				return functionPtrs[static_cast<int64_t>(state)](cur, sign, fracDigits, value, fracValue, intDigits, exp, expSign, numTmp);
			} else {
				static constexpr auto functionPtrs{ generateFunctionPtrs<false>() };
				return functionPtrs[static_cast<int64_t>(state)](cur, sign, fracDigits, value, fracValue, intDigits, exp, expSign, numTmp);
			}
		}

		JSONIFIER_ALWAYS_INLINE static bool parseInt(value_type& value, char_type*& cur) noexcept {
			int8_t numTmp		  = 0;
			value_type sig		  = 0;
			value_type fracDigits = 0;
			value_type intDigits  = 0;
			value_type fracValue  = 0;
			int8_t expSign		  = 1;
			int64_t exp			  = 0;

			int8_t sign = 1;
			if (*cur == minus) {
				sign = -1;
				++cur;
				if (!isDigit(*cur)) [[unlikely]] {
					return false;
				}
			}

			return callFunction<parsing_state::starting>(cur, sign, fracDigits, value, fracValue, intDigits, exp, expSign);
		}
	};

	JSONIFIER_ALWAYS_INLINE constexpr bool isSafeAddition(size_t a, size_t b) noexcept {
		return a <= (std::numeric_limits<size_t>::max)() - b;
	}

	JSONIFIER_ALWAYS_INLINE constexpr bool isSafeMultiplication10(size_t a) noexcept {
		constexpr size_t b = (std::numeric_limits<size_t>::max)() / 10;
		return a <= b;
	}

	template<typename value_type, typename char_type> JSONIFIER_ALWAYS_INLINE constexpr bool stoui64(value_type res, const char_type* c) noexcept {
		if (!isDigit(*c)) [[unlikely]] {
			return false;
		}

		constexpr std::array<uint32_t, 4> maxDigitsFromSize = { 4, 6, 11, 20 };
		constexpr auto N									= maxDigitsFromSize[static_cast<size_t>(std::bit_width(sizeof(value_type)) - 1)];

		std::array<uint8_t, N> digits{ 0 };
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