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

	constexpr std::array<uint64_t, 20> powerOfTenInt{ 1ull, 10ull, 100ull, 1000ull, 10000ull, 100000ull, 1000000ull, 10000000ull, 100000000ull, 1000000000ull, 10000000000ull,
		100000000000ull, 1000000000000ull, 10000000000000ull, 100000000000000ull, 1000000000000000ull, 10000000000000000ull, 100000000000000000ull, 1000000000000000000ull,
		10000000000000000000ull };


	JSONIFIER_ALWAYS_INLINE int64_t fastFloor(double x) {
		int64_t i = static_cast<int64_t>(x);
		return (x < i) ? i - 1 : i;
	}

	enum class parsing_state {
		starting			  = 0,
		collecting_integer	  = 2,
		collecting_fractional = 3,
		collecting_exponent	  = 5,
		finishing			  = 8,
	};

	static constexpr uint8_t digiTypeDigit = 1 << 1;
	static constexpr uint8_t digiTypePos   = 1 << 2;
	static constexpr uint8_t digiTypeNeg   = 1 << 3;
	static constexpr uint8_t digiTypeDot   = 1 << 4;
	static constexpr uint8_t digiTypeExp   = 1 << 5;

	constexpr std::array<bool, 256> digiTableBool{ [] {
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

	constexpr std::array<uint8_t, 256> digiTable{ [] {
		std::array<uint8_t, 256> returnValues{};
		returnValues['0'] = digiTypeDigit;
		returnValues['1'] = digiTypeDigit;
		returnValues['2'] = digiTypeDigit;
		returnValues['3'] = digiTypeDigit;
		returnValues['4'] = digiTypeDigit;
		returnValues['5'] = digiTypeDigit;
		returnValues['6'] = digiTypeDigit;
		returnValues['7'] = digiTypeDigit;
		returnValues['8'] = digiTypeDigit;
		returnValues['9'] = digiTypeDigit;
		returnValues['+'] = digiTypePos;
		returnValues['-'] = digiTypeNeg;
		returnValues['.'] = digiTypeDot;
		returnValues['e'] = digiTypeExp;
		returnValues['E'] = digiTypeExp;
		return returnValues;
	}() };

#define isPlusOrMinus(d) ((digiTable[d] & uint8_t(digiTypePos | digiTypeNeg)) != 0)

#define isExponent(d) ((digiTable[d] & uint8_t(digiTypeExp)) != 0)

	static constexpr char decimalPoint{ '.' };
	static constexpr char smallE{ 'e' };
	static constexpr char minus{ '-' };
	static constexpr char bigE{ 'E' };
	static constexpr char plus{ '+' };
	static constexpr char zero{ '0' };
	static constexpr char nine{ '9' };

#define isDigit(d) digiTableBool[d]

#define toDigit(c) (static_cast<char>(c) - zero)

	template<typename value_type, typename char_type> struct integer_parser;

	template<jsonifier::concepts::unsigned_type value_type, typename char_type> struct integer_parser<value_type, char_type> {
		int64_t fracDigits{};
		value_type fracValue{};
		int64_t exp{};
		int8_t expSign{};
		template<parsing_state state> JSONIFIER_MAYBE_ALWAYS_INLINE bool parseValue(const char_type*& cur, value_type& value) {
			if constexpr (state == parsing_state::starting) {
				if (isDigit(*cur)) {
					value = toDigit(*cur);
					++cur;
					return integer_parser<value_type, char_type>::parseValue<parsing_state::collecting_integer>(cur, value);
				} else {
					return false;
				}
			} else if constexpr (state == parsing_state::collecting_integer) {
				int8_t numTmp;
				while (isDigit(*cur)) {
					numTmp = toDigit(*cur);
					static constexpr auto rawCompVal{ std::numeric_limits<value_type>::max() };
					const auto comparisonValue{ (rawCompVal - numTmp) / 10 };
					if (value > comparisonValue) [[unlikely]] {
						return false;
					}
					value = numTmp + value * 10;
					++cur;
				}
				if (*cur == '.') {
					++cur;
					return integer_parser<value_type, char_type>::parseValue<parsing_state::collecting_fractional>(cur, value);
				} else if (isExponent(*cur)) {
					++cur;
					return integer_parser<value_type, char_type>::parseValue<parsing_state::collecting_exponent>(cur, value);
				} else {
					return true;
				}
			} else if constexpr (state == parsing_state::collecting_fractional) {
				if (isDigit(*cur)) [[likely]] {
					while (isDigit(*cur)) {
						fracValue = toDigit(*cur) + fracValue * 10;
						++fracDigits;
						++cur;
					}
					if (isExponent(*cur)) {
						++cur;
						return integer_parser<value_type, char_type>::parseValue<parsing_state::collecting_exponent>(cur, value);
					} else {
						return true;
					}
				} else {
					return false;
				}
			} else if constexpr (state == parsing_state::collecting_exponent) {
				if (isPlusOrMinus(*cur)) {
					if (*cur == '-') {
						expSign = -1;
					}
					++cur;
				}
				if (isDigit(*cur)) [[likely]] {
					while (isDigit(*cur)) {
						exp = toDigit(*cur) + exp * 10;
						++cur;
					}
					return integer_parser<value_type, char_type>::parseValue<parsing_state::finishing>(cur, value);
				} else {
					return false;
				}
			} else if constexpr (state == parsing_state::finishing) {
				if (exp > 19) [[unlikely]] {
					return false;
				}

				exp *= expSign;

				static constexpr auto doubleMax{ std::numeric_limits<value_type>::max() };
				static constexpr auto doubleMin{ std::numeric_limits<value_type>::min() };

				if (fracDigits + exp > 0) {
					auto fractionalCorrection = exp > fracDigits ? fracValue * powerOfTenInt[exp - fracDigits] : fracValue * powerOfTenInt[fracDigits - exp];
					if (exp > 0) {
						if (value > doubleMax / powerOfTenInt[exp]) [[unlikely]] {
							return false;
						}
						value *= powerOfTenInt[exp];
						value += fractionalCorrection;
					} else {
						if (value < doubleMin * powerOfTenInt[-exp]) [[unlikely]] {
							return false;
						}
						value /= powerOfTenInt[-exp];
						value += fractionalCorrection;
					}
					value = static_cast<value_type>(value);
				} else {
					if (exp > 0) {
						if (value > doubleMax / powerOfTenInt[exp]) [[unlikely]] {
							return false;
						}
						value *= powerOfTenInt[exp];
					} else {
						if (value < doubleMin * powerOfTenInt[-exp]) [[unlikely]] {
							return false;
						}
						value /= powerOfTenInt[-exp];
					}
				}


				return true;
			} else {
				return false;
			}
		}

		JSONIFIER_ALWAYS_INLINE bool parseInt(value_type& value, char_type*& cur) noexcept {
			fracDigits = 0;
			fracValue  = 0;
			exp		   = 0;
			expSign	   = 1;
			return integer_parser<value_type, char_type>::parseValue<parsing_state::starting>(cur, value);
		}
	};

	template<jsonifier::concepts::signed_type value_type, typename char_type> struct integer_parser<value_type, char_type> {
		value_type fracDigits{};
		value_type fracValue{};
		int64_t exp{};
		int8_t expSign{};

		template<parsing_state state, bool positive> JSONIFIER_MAYBE_ALWAYS_INLINE bool parseValue(const char_type*& cur, value_type& value) {
			if constexpr (state == parsing_state::starting) {
				if (isDigit(*cur)) {
					value = toDigit(*cur);
					++cur;
					return integer_parser<value_type, char_type>::parseValue<parsing_state::collecting_integer, positive>(cur, value);
				} else {
					return false;
				}
			} else if constexpr (state == parsing_state::collecting_integer) {
				int8_t numTmp;
				while (isDigit(*cur)) {
					numTmp = toDigit(*cur);
					if constexpr (positive) {
						static constexpr auto compVal{ std::numeric_limits<value_type>::max() };
						const auto comparisonValue{ (compVal - numTmp) / 10 };
						if (value > comparisonValue) [[unlikely]] {
							return false;
						}
					} else {
						static constexpr auto compVal{ static_cast<size_t>(std::numeric_limits<value_type>::max()) + 1 };
						const auto comparisonValue{ (compVal - numTmp) / 10 };
						if (value > comparisonValue) [[unlikely]] {
							return false;
						}
					}
					value = numTmp + value * 10;
					++cur;
				}
				if (*cur == '.') {
					++cur;
					return integer_parser<value_type, char_type>::parseValue<parsing_state::collecting_fractional, positive>(cur, value);
				} else if (isExponent(*cur)) {
					++cur;
					return integer_parser<value_type, char_type>::parseValue<parsing_state::collecting_exponent, positive>(cur, value);
				} else {
					return true;
				}
			} else if constexpr (state == parsing_state::collecting_fractional) {
				if (isDigit(*cur)) [[likely]] {
					while (isDigit(*cur)) {
						fracValue = toDigit(*cur) + fracValue * 10;
						++fracDigits;
						++cur;
					}
					if (isExponent(*cur)) {
						++cur;
						return integer_parser<value_type, char_type>::parseValue<parsing_state::collecting_exponent, positive>(cur, value);
					} else {
						return true;
					}
				} else {
					return false;
				}
			} else if constexpr (state == parsing_state::collecting_exponent) {
				if (isPlusOrMinus(*cur)) {
					if (*cur == '-') {
						expSign = -1;
					}
					++cur;
				}
				if (isDigit(*cur)) [[likely]] {
					while (isDigit(*cur)) {
						exp = toDigit(*cur) + exp * 10;
						++cur;
					}
					return integer_parser<value_type, char_type>::parseValue<parsing_state::finishing, positive>(cur, value);
				} else {
					return false;
				}
			} else if constexpr (state == parsing_state::finishing) {
				if (exp > 19) [[unlikely]] {
					return false;
				}

				exp *= expSign;

				static constexpr auto doubleMax{ std::numeric_limits<value_type>::max() };
				static constexpr auto doubleMin{ std::numeric_limits<value_type>::min() };

				if (fracDigits + exp > 0) {
					auto fractionalCorrection = exp > fracDigits ? fracValue * powerOfTenInt[exp - fracDigits] : fracValue * powerOfTenInt[fracDigits - exp];
					if (exp > 0) {
						if (value > doubleMax / powerOfTenInt[exp]) [[unlikely]] {
							return false;
						}
						value *= powerOfTenInt[exp];
						value += fractionalCorrection;
					} else {
						if (value < doubleMin * powerOfTenInt[-exp]) [[unlikely]] {
							return false;
						}
						value /= powerOfTenInt[-exp];
						value += fractionalCorrection;
					}
					value = static_cast<value_type>(value);
				} else {
					if (exp > 0) {
						if (value > doubleMax / powerOfTenInt[exp]) [[unlikely]] {
							return false;
						}
						value *= powerOfTenInt[exp];
					} else {
						if (value < doubleMin * powerOfTenInt[-exp]) [[unlikely]] {
							return false;
						}
						value /= powerOfTenInt[-exp];
					}
				}

				return true;
			} else {
				return false;
			}
		}

		JSONIFIER_ALWAYS_INLINE bool parseInt(value_type& value, char_type*& cur) noexcept {
			fracDigits = 0;
			fracValue  = 0;
			expSign	   = 1;
			exp		   = 0;
			if (*cur == '-') {
				++cur;
				auto result = integer_parser<value_type, char_type>::parseValue<parsing_state::starting, false>(cur, value);
				value *= -1;
				return result;
			} else {
				return integer_parser<value_type, char_type>::parseValue<parsing_state::starting, true>(cur, value);
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