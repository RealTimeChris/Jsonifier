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

	JSONIFIER_ALWAYS_INLINE int64_t fastAbs(int64_t value) {
		return (value ^ (value >> 63)) - (value >> 63);
	}

	constexpr uint8_t digiTypeZero = 1 << 0;

	constexpr uint8_t digiTypeNonZero = 1 << 1;

	constexpr uint8_t digiTypePos = 1 << 2;

	constexpr uint8_t digi_type_neg = 1 << 3;

	constexpr uint8_t digiTypeDot = 1 << 4;

	constexpr uint8_t digiTypeExp = 1 << 5;

	constexpr std::array<uint8_t, 256> digiTable = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00, 0x08, 0x10, 0x00, 0x01,
		0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

	JSONIFIER_ALWAYS_INLINE constexpr bool isDigitOrFp(uint8_t d) noexcept {
		return (digiTable[d] & uint8_t(digiTypeZero | digiTypeNonZero | digiTypeDot | digiTypeExp)) != 0;
	}

	JSONIFIER_ALWAYS_INLINE_VARIABLE char decimalPoint{ '.' };
	JSONIFIER_ALWAYS_INLINE_VARIABLE char smallE{ 'e' };
	JSONIFIER_ALWAYS_INLINE_VARIABLE char minus{ '-' };
	JSONIFIER_ALWAYS_INLINE_VARIABLE char bigE{ 'E' };
	JSONIFIER_ALWAYS_INLINE_VARIABLE char plus{ '+' };
	JSONIFIER_ALWAYS_INLINE_VARIABLE char zero{ '0' };
	JSONIFIER_ALWAYS_INLINE_VARIABLE char nine{ '9' };

	JSONIFIER_ALWAYS_INLINE bool isDigit(char c) {
		return c >= zero && c <= nine;
	}

	JSONIFIER_ALWAYS_INLINE uint8_t toDigit(char c) {
		return static_cast<uint8_t>(c - zero);
	}

	template<typename value_type, typename char_type> struct integer_parser;

	template<size_t index, jsonifier::concepts::unsigned_type value_type, typename char_type> struct integer_parser_helper {
		static constexpr auto maxIndex{ 21ull };
		JSONIFIER_ALWAYS_INLINE static int8_t exprIntg(uint8_t numTmp, value_type sig, const char_type* cur, value_type& value, value_type fracDigits, value_type fracValue,
			value_type intDigits) {
			if constexpr (index < maxIndex) {
				if constexpr (index == 2) {
					if (*cur == zero) {
						return 0;
					}
				}
				if (isDigitOrFp(cur[index])) [[likely]] {
					if (isDigit(cur[index])) {
						numTmp = toDigit(cur[index]);
						if (sig > (std::numeric_limits<value_type>::max() - numTmp) / 10) [[unlikely]] {
							return 0;
						}
						++intDigits;
						sig = numTmp + sig * 10;
						return integer_parser_helper<index + 1, value_type, char_type>::exprIntg(numTmp, sig, cur, value, fracDigits, fracValue, intDigits);
					} else if (cur[index] == decimalPoint) {
						return integer_parser<value_type, char_type>::parseFracIter(cur, sig, value, fracDigits, fracValue, intDigits, index + 1);
					} else if (cur[index] == smallE || cur[index] == bigE) {
						numTmp = 0;
						return integer_parser<value_type, char_type>::parseExpIter(cur, sig, fracDigits, numTmp, value, fracValue, intDigits, 0, 1, index + 1);
					}
				} else [[unlikely]] {
					value = sig;
					return index;
				}
			} else [[unlikely]] {
				return 0;
			}
			return 0;
		}
	};

	template<jsonifier::concepts::unsigned_type value_type, typename char_type> struct integer_parser<value_type, char_type> {
		static constexpr auto maxIndex{ 21ull };

		JSONIFIER_ALWAYS_INLINE static int8_t parseFinish(const char_type* cur, value_type sig, value_type fracDigits, uint8_t numTmp, value_type& value, value_type fracValue,
			value_type intDigits, int8_t expSign, int64_t exp, size_t globalIndex) {
			exp *= expSign;

			if (fastAbs(exp) > 19) [[unlikely]] {
				return 0;
			}

			double fractionalCorrection = static_cast<double>(fracValue) / powerOfTenInt[fracDigits];
			double combinedValue		= (static_cast<double>(sig) + fractionalCorrection);
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

			value = static_cast<value_type>(combinedValue);
			return globalIndex;
		}

		// New iterative version of parseExp
		JSONIFIER_ALWAYS_INLINE static int8_t parseExpIter(const char_type* cur, value_type sig, value_type fracDigits, uint8_t numTmp, value_type& value, value_type fracValue,
			value_type intDigits, int64_t exp, int8_t expSign, size_t globalIndex) {
			if ((cur[globalIndex] == plus || cur[globalIndex] == minus)) {
				expSign = (cur[globalIndex] == minus) ? -1 : 1;
				globalIndex++;
			}
			if (!isDigit(cur[globalIndex])) {
				return 0;
			}
			while (isDigit(cur[globalIndex])) {
				numTmp = toDigit(cur[globalIndex]);
				exp	   = numTmp + exp * 10;
				globalIndex++;
			}
			return parseFinish(cur + globalIndex, sig, fracDigits, numTmp, value, fracValue, intDigits, expSign, exp, globalIndex);
		}

		// New iterative version of parseFrac
		JSONIFIER_ALWAYS_INLINE static int8_t parseFracIter(const char_type* cur, value_type sig, value_type& value, value_type fracDigits, value_type fracValue,
			value_type intDigits, size_t globalIndex) {
			while (isDigit(cur[globalIndex])) {
				fracValue = toDigit(cur[globalIndex]) + fracValue * 10;
				fracDigits++;
				globalIndex++;
			}
			if (cur[globalIndex] == bigE || cur[globalIndex] == smallE) {
				return parseExpIter(cur, sig, fracDigits, 0, value, fracValue, intDigits, 0, 1, globalIndex + 1);
			}
			if (!isDigit(cur[globalIndex])) {
				value = sig;
				return globalIndex;
			}
			return 0;
		}

		JSONIFIER_ALWAYS_INLINE static int8_t parseInt(value_type& value, char_type*& cur) noexcept {
			uint8_t numTmp		  = 0;
			value_type sig		  = 0;
			value_type fracDigits = 0;
			value_type intDigits  = 0;
			value_type fracValue  = 0;

			auto result = integer_parser_helper<0, value_type, char_type>::exprIntg(numTmp, sig, cur, value, fracDigits, fracValue, intDigits);
			cur += result;
			return static_cast<bool>(result);
		}
	};

	template<jsonifier::concepts::signed_type value_type, typename char_type> struct integer_parser<value_type, char_type> {
		static constexpr auto maxIndex{ 20ull };

		JSONIFIER_ALWAYS_INLINE static bool parseExp(const char_type*& cur, value_type sig, value_type fracDigits, int8_t numTmp, value_type& value, size_t index,
			value_type fracValue, int8_t sign, value_type intDigits) {
			int8_t expSign = 1;
			++index;
			if (cur[index] == plus) {
				++index;
			} else if (cur[index] == minus) {
				expSign = -1;
				++index;
			}
			if (!isDigit(cur[index])) [[unlikely]] {
				return false;
			}
			value_type exp = 0;
			while (isDigit(cur[index])) {
				numTmp = toDigit(cur[index]);
				++index;
				exp = numTmp + exp * 10;
			}
			exp *= expSign;

			if (expSign == -1 && fastAbs(exp) >= intDigits) [[unlikely]] {
				if (sign == -1) {
					value = -1;
					cur += index;
					return true;
				} else {
					value = 0;
					cur += index;
					return true;
				}
			}

			if (fastAbs(exp) > 19) [[unlikely]] {
				return false;
			}

			double fractionalCorrection = static_cast<double>(fracValue) / powerOfTenInt[fracDigits];
			double combinedValue		= (static_cast<double>(sig) + fractionalCorrection);
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

			if (sign == -1 && combinedValue > 0.0) {
				value = fastFloor(combinedValue * sign);
			} else {
				value = static_cast<value_type>(combinedValue) * sign;
			}

			cur += index;
			return true;
		}

		JSONIFIER_ALWAYS_INLINE static bool parseFrac(int8_t numTmp, value_type sig, const char_type*& cur, size_t index, value_type& value, value_type fracDigits,
			value_type fracValue, int8_t sign, value_type intDigits) {
			++index;
			if (!isDigit(cur[index])) [[unlikely]] {
				return false;
			}
			while (isDigit(cur[index])) {
				numTmp = toDigit(cur[index]);
				++fracDigits;
				++index;
				fracValue = numTmp + fracValue * 10;
			}
			if (cur[index] == bigE || cur[index] == smallE) {
				return parseExp(cur, sig, fracDigits, numTmp, value, index, fracValue, sign, intDigits);
			}
			value = sig * sign;
			cur += index;
			return true;
		};

		template<size_t index, bool positive> JSONIFIER_ALWAYS_INLINE static bool exprIntg(int8_t numTmp, value_type sig, const char_type*& cur, value_type& value,
			value_type fracDigits, value_type fracValue, int8_t sign, value_type intDigits) {
			if constexpr (index < maxIndex) {
				if constexpr (index == 2) {
					if (*cur == zero) {
						return false;
					}
				}
				if (isDigit(cur[index])) [[likely]] {
					numTmp = toDigit(cur[index]);
					++intDigits;
					sig = numTmp + sig * 10;
					return exprIntg<index + 1, positive>(numTmp, sig, cur, value, fracDigits, fracValue, sign, intDigits);
				} else if (cur[index] == decimalPoint) {
					return parseFrac(numTmp, sig, cur, index, value, fracDigits, fracValue, sign, intDigits);
				} else if (cur[index] == smallE || cur[index] == bigE) {
					return parseExp(cur, sig, fracDigits, numTmp, value, index, fracValue, sign, intDigits);
				} else {
					value = sig * sign;
					cur += index;
					return true;
				}
			} else [[unlikely]] {
				return false;
			}
		}

		JSONIFIER_INLINE static bool parseInt(value_type& value, char_type*& cur) noexcept {
			int8_t numTmp		  = 0;
			value_type sig		  = 0;
			value_type fracDigits = 0;
			value_type intDigits  = 0;
			value_type fracValue  = 0;

			int8_t sign = 1;
			if (*cur == minus) {
				sign = -1;
				++cur;
				if (!isDigit(*cur)) [[unlikely]] {
					return false;
				}
			}

			return sign == -1 ? exprIntg<0, false>(numTmp, sig, cur, value, fracDigits, fracValue, sign, intDigits)
							  : exprIntg<0, true>(numTmp, sig, cur, value, fracDigits, fracValue, sign, intDigits);
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