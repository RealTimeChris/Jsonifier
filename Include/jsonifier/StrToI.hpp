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

	constexpr std::array<bool, 256> digiTable{ [] {
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

	constexpr std::array<bool, 256> expTable{ [] {
		std::array<bool, 256> returnValues{};
		returnValues['e'] = true;
		returnValues['E'] = true;
		return returnValues;
	}() };

	constexpr std::array<bool, 256> expFracTable{ [] {
		std::array<bool, 256> returnValues{};
		returnValues['e'] = true;
		returnValues['E'] = true;
		returnValues['.'] = true;
		return returnValues;
	}() };

	constexpr std::array<bool, 256> plusOrMinusTable{ [] {
		std::array<bool, 256> returnValues{};
		returnValues['-'] = true;
		returnValues['+'] = true;
		return returnValues;
	}() };

#define isPlusOrMinus(d) plusOrMinusTable[d]

#define isExponentOrFractional(d) expFracTable[d]

#define isExponent(d) expTable[d]

#define isDigit(d) digiTable[d]

	static constexpr char decimalPoint{ '.' };
	static constexpr char zero{ '0' };

#define toDigit(c) (static_cast<char>(c) - zero)

	template<typename value_type, typename char_type> struct integer_parser;

	template<jsonifier::concepts::unsigned_type value_type, typename char_type> struct integer_parser<value_type, char_type> {
		value_type fracValue;
		int64_t fracDigits;
		int8_t expSign;
		int8_t numTmp;
		int64_t exp;

		template<bool first = true> JSONIFIER_ALWAYS_INLINE bool parseFraction(const char_type*& iter, value_type& value) {
			if constexpr (first) {
				fracDigits = 0;
				if (isDigit(*iter)) [[likely]] {
					fracValue = toDigit(*iter);
					++fracDigits;
					++iter;
					return parseFraction<false>(iter, value);
				} else {
					return false;
				}
			} else {
				while (isDigit(*iter)) {
					fracValue = toDigit(*iter) + fracValue * 10;
					++fracDigits;
					++iter;
				}
				if (isExponent(*iter)) {
					++iter;
					return parseExponent(iter, value);
				} else {
					return true;
				}
			}
		}

		template<bool first = true> JSONIFIER_ALWAYS_INLINE bool parseExponent(const char_type*& iter, value_type& value) {
			if constexpr (first) {
				expSign = 1;
				if (isPlusOrMinus(*iter)) {
					if (*iter == '-') {
						expSign = -1;
					}
					++iter;
				}
				if (isDigit(*iter)) [[likely]] {
					exp = toDigit(*iter);
					++iter;
					return parseExponent<false>(iter, value);
				} else {
					return false;
				}
			} else {
				while (isDigit(*iter)) [[likely]] {
					exp = toDigit(*iter) + exp * 10;
					++iter;
				}
				return parseFinish(iter, value);
			}
		}

		JSONIFIER_ALWAYS_INLINE bool parseFinish(const char_type*& iter, value_type& value) {
			if (exp > 19) [[unlikely]] {
				return false;
			}

			exp *= expSign;

			static constexpr value_type doubleMax = std::numeric_limits<value_type>::max();
			static constexpr value_type doubleMin = std::numeric_limits<value_type>::min();
			const auto powerExp					  = powerOfTenInt[exp > 0 ? exp : -exp];

			if (fracDigits + exp > 0) {
				auto fractionalCorrection = exp > fracDigits ? fracValue * powerOfTenInt[exp - fracDigits] : fracValue * powerOfTenInt[fracDigits - exp];
				if (exp > 0) {
					if (value > doubleMax / powerExp) [[unlikely]] {
						return false;
					}
					value *= powerExp;
					value += fractionalCorrection;
				} else {
					if (value < doubleMin * powerExp) [[unlikely]] {
						return false;
					}
					value /= powerExp;
					value += fractionalCorrection;
				}
			} else {
				if (exp > 0) {
					if (value > doubleMax / powerExp) [[unlikely]] {
						return false;
					}
					value *= powerExp;
				} else {
					if (value < doubleMin * powerExp) [[unlikely]] {
						return false;
					}
					value /= powerExp;
				}
			}

			return true;
		}

		template<size_t index = 0> JSONIFIER_ALWAYS_INLINE bool parseInteger(const char_type*& iter, value_type& value) {
			if constexpr (index < 3) {
				if constexpr (index == 0) {
					if (isDigit(*iter)) {
						value = toDigit(*iter);
						++iter;
						return parseInteger<index + 1>(iter, value);
					} else {
						return false;
					}
				} else if constexpr (index == 2) {
					if (*(iter - 2) == zero) {
						return false;
					}
				}
				if (isDigit(*iter)) [[likely]] {
					numTmp = toDigit(*iter);
					value  = numTmp + value * 10;
					++iter;
					return parseInteger<index + 1>(iter, value);
				} else {
					if (!isExponentOrFractional(*iter)) {
						return true;
					}
					if (*iter == decimalPoint) {
						++iter;
						return parseFraction(iter, value);
					} else {
						fracDigits = 0;
						fracValue  = 0;
						++iter;
						return parseExponent(iter, value);
					}
				}
			}
			while (isDigit(*iter)) {
				numTmp = toDigit(*iter);
				static constexpr auto rawCompVal{ std::numeric_limits<value_type>::max() };
				const auto comparisonValue{ (rawCompVal - numTmp) / 10 };
				if (value > comparisonValue) [[unlikely]] {
					return false;
				}
				value = numTmp + value * 10;
				++iter;
			}
			if (!isExponentOrFractional(*iter)) [[likely]] {
				return true;
			} else if (*iter == decimalPoint) {
				++iter;
				return parseFraction(iter, value);
			} else {
				fracDigits = 0;
				fracValue  = 0;
				++iter;
				return parseExponent(iter, value);
			}
		}

		JSONIFIER_ALWAYS_INLINE bool parseInt(value_type& value, char_type*& iter) noexcept {
			return integer_parser<value_type, char_type>::parseInteger<0>(iter, value);
		}
	};

	template<jsonifier::concepts::signed_type value_type, typename char_type> struct integer_parser<value_type, char_type> {
		value_type fracValue;
		int64_t fracDigits;
		int8_t expSign;
		int8_t numTmp;
		int64_t exp;

		template<bool first = true> JSONIFIER_ALWAYS_INLINE bool parseFraction(const char_type*& iter, value_type& value) {
			if constexpr (first) {
				fracDigits = 0;
				if (isDigit(*iter)) [[likely]] {
					fracValue = toDigit(*iter);
					++fracDigits;
					++iter;
					return parseFraction<false>(iter, value);
				} else {
					return false;
				}
			} else {
				while (isDigit(*iter)) {
					fracValue = toDigit(*iter) + fracValue * 10;
					++fracDigits;
					++iter;
				}
				if (isExponent(*iter)) {
					++iter;
					return parseExponent(iter, value);
				} else {
					return true;
				}
			}
		}

		template<bool first = true> JSONIFIER_ALWAYS_INLINE bool parseExponent(const char_type*& iter, value_type& value) {
			if constexpr (first) {
				expSign = 1;
				if (isPlusOrMinus(*iter)) {
					if (*iter == '-') {
						expSign = -1;
					}
					++iter;
				}
				if (isDigit(*iter)) [[likely]] {
					exp = toDigit(*iter);
					++iter;
					return parseExponent<false>(iter, value);
				} else {
					return false;
				}
			} else {
				while (isDigit(*iter)) [[likely]] {
					exp = toDigit(*iter) + exp * 10;
					++iter;
				}
				return parseFinish(iter, value);
			}
		}

		JSONIFIER_ALWAYS_INLINE bool parseFinish(const char_type*& iter, value_type& value) {
			if (exp > 19) [[unlikely]] {
				return false;
			}

			exp *= expSign;

			static constexpr value_type doubleMax = std::numeric_limits<value_type>::max();
			static constexpr value_type doubleMin = std::numeric_limits<value_type>::min();
			const auto powerExp					  = powerOfTenInt[exp > 0 ? exp : -exp];

			if (fracDigits + exp > 0) {
				auto fractionalCorrection = exp > fracDigits ? fracValue * powerOfTenInt[exp - fracDigits] : fracValue * powerOfTenInt[fracDigits - exp];
				if (exp > 0) {
					if (value > doubleMax / powerExp) [[unlikely]] {
						return false;
					}
					value *= powerExp;
					value += fractionalCorrection;
				} else {
					if (value < doubleMin * powerExp) [[unlikely]] {
						return false;
					}
					value /= powerExp;
					value += fractionalCorrection;
				}
			} else {
				if (exp > 0) {
					if (value > doubleMax / powerExp) [[unlikely]] {
						return false;
					}
					value *= powerExp;
				} else {
					if (value < doubleMin * powerExp) [[unlikely]] {
						return false;
					}
					value /= powerExp;
				}
			}

			return true;
		}

		template<size_t index = 0> JSONIFIER_ALWAYS_INLINE bool parseInteger(const char_type*& iter, value_type& value) {
			if constexpr (index < 3) {
				if constexpr (index == 0) {
					if (isDigit(*iter)) {
						value = toDigit(*iter);
						++iter;
						return parseInteger<index + 1>(iter, value);
					} else {
						return false;
					}
				} else if constexpr (index == 2) {
					if (*(iter - 2) == zero) {
						return false;
					}
				}
				if (isDigit(*iter)) [[likely]] {
					numTmp = toDigit(*iter);
					value  = numTmp + value * 10;
					++iter;
					return parseInteger<index + 1>(iter, value);
				} else {
					if (!isExponentOrFractional(*iter)) {
						return true;
					}
					if (*iter == decimalPoint) {
						++iter;
						return parseFraction(iter, value);
					} else {
						fracDigits = 0;
						fracValue  = 0;
						++iter;
						return parseExponent(iter, value);
					}
				}
			}
			while (isDigit(*iter)) {
				numTmp = toDigit(*iter);
				value  = numTmp + value * 10;
				++iter;
			}
			if (!isExponentOrFractional(*iter)) [[likely]] {
				return true;
			} else if (*iter == decimalPoint) {
				++iter;
				return parseFraction(iter, value);
			} else {
				fracDigits = 0;
				fracValue  = 0;
				++iter;
				return parseExponent(iter, value);
			}
		}

		JSONIFIER_ALWAYS_INLINE bool parseInt(value_type& value, char_type*& iter) noexcept {
			bool negative = (*iter == '-');
			if (negative) {
				++iter;
			}
			auto result							 = integer_parser<value_type, char_type>::parseInteger(iter, value);
			static constexpr auto maxValue = uint64_t((std::numeric_limits<value_type>::max)());
			static constexpr auto minValue		 = uint64_t((std::numeric_limits<value_type>::max)()) + 1;

			if (negative) {
				if (value <= static_cast<uint64_t>(minValue)) {
					value *= -1;
					return result;
				} else {
					return false;
				}
			} else {
				return (value <= maxValue) ? result : false;
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

		if (isExponent(*c)) {
			++c;

			bool negative = false;
			if (isPlusOrMinus(*c)) {
				negative = (*c == '-');
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