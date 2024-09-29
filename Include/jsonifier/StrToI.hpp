/*
	MIT License

	Copyright (c) 2024 RealTimeChris

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
#include <jsonifier/StrToD.hpp>

#include <concepts>
#include <cstdint>
#include <cstring>
#include <array>

namespace jsonifier_internal {

	constexpr std::array<bool, 256> digitTableBool{ []() {
		std::array<bool, 256> returnValues{};
		returnValues[0x30u] = true;
		returnValues[0x31u] = true;
		returnValues[0x32u] = true;
		returnValues[0x33u] = true;
		returnValues[0x34u] = true;
		returnValues[0x35u] = true;
		returnValues[0x36u] = true;
		returnValues[0x37u] = true;
		returnValues[0x38u] = true;
		returnValues[0x39u] = true;
		return returnValues;
	}() };

	template<jsonifier::concepts::signed_type value_type, typename UC>
	JSONIFIER_ALWAYS_INLINE constexpr bool parse_number_string_int(UC const*& p, UC const* pend, value_type& value) noexcept {
		using namespace jsonifier_fast_float;
		static constexpr UC decimal_point = '.';
		static constexpr UC zero		  = '0';
		static constexpr UC minus		  = '-';

		if (*p == minus) {
			++p;

			if (!digitTableBool[*p]) [[unlikely]] {
				return false;
			}
		}

		UC const* const start_digits = p;
		int64_t numTmp;
		while (is_integer(*p)) {
			numTmp = uint64_t(*p - zero);
			if (value > uint64_t(-(INT64_MIN + numTmp) / 10)) {
				return false;
			}
			value = 10 * value + numTmp;
			++p;
		}

		int64_t digit_count = int64_t(p - start_digits);

		if (digit_count == 0 || (start_digits[0] == zero && digit_count > 1)) {
			return false;
		}
		int64_t exponent{};
		const bool has_decimal_point{ *p == decimal_point };
		if (has_decimal_point) {
			++p;
			UC const* before = p;
			loop_parse_if_eight_digits(p, pend, value);

			while (is_integer(*p)) {
				uint8_t digit = uint8_t(*p - zero);
				++p;
				value = value * 10 + digit;
			}
			exponent = before - p;
			digit_count -= exponent;
		}

		if (has_decimal_point && exponent == 0) {
			return false;
		}

		int64_t exp_number = 0;

		if ((UC('e') == *p) || (UC('E') == *p)) {
			UC const* location_of_e = p;
			++p;
			bool neg_exp = false;
			if (UC('-') == *p) {
				neg_exp = true;
				++p;
			} else if (UC('+') == *p) {
				++p;
			}
			if (!is_integer(*p)) {
				p = location_of_e;
			} else {
				while (is_integer(*p)) {
					uint8_t digit = uint8_t(*p - zero);
					if (exp_number < 0x10000000) {
						exp_number = 10 * exp_number + digit;
					}
					++p;
				}
				if (neg_exp) {
					exp_number = -exp_number;
				}
				exponent += exp_number;
			}
		}

		if (digit_count > 19) {
			UC const* start = start_digits;
			while ((*start == zero || *start == decimal_point)) {
				if (*start == zero) {
					--digit_count;
				}
				++start;
			}

			if (digit_count > 20) {
				return false;
			}
		}

		for (int64_t i = 0; i < exponent; ++i) {
			if (value > INT64_MAX / 10) {
				return false;
			}
			value *= 10;
		}

		return true;
	}

	template<jsonifier::concepts::signed_type value_type, typename UC> constexpr bool parseInteger(UC const*& first, UC const* last, value_type& value) noexcept {
		value = 0;
		return parse_number_string_int<value_type, UC>(first, last, value);
	}

	template<jsonifier::concepts::unsigned_type value_type, typename UC>
	JSONIFIER_ALWAYS_INLINE constexpr bool parse_number_string_int(UC const*& p, UC const* pend, value_type& value) noexcept {
		using namespace jsonifier_fast_float;
		static constexpr UC decimal_point = '.';
		static constexpr UC zero		  = '0';
		static constexpr UC minus		  = '-';

		if (*p == minus) {
			return false;
		}
		UC const* const start_digits = p;
		size_t numTmp;
		while (is_integer(*p)) {
			numTmp = uint64_t(*p - zero);
			if (value > ((UINT64_MAX - numTmp) / 10)) {
				return false;
			}
			value = 10 * value + numTmp;
			++p;
		}

		int64_t digit_count = int64_t(p - start_digits);

		if (digit_count == 0 || (start_digits[0] == zero && digit_count > 1)) {
			return false;
		}
		int64_t exponent{};
		const bool has_decimal_point{ *p == decimal_point };
		if (has_decimal_point) {
			++p;
			UC const* before = p;
			loop_parse_if_eight_digits(p, pend, value);

			while (is_integer(*p)) {
				uint8_t digit = uint8_t(*p - zero);
				++p;
				value = value * 10 + digit;
			}
			exponent = before - p;
			digit_count -= exponent;
		}

		if (has_decimal_point && exponent == 0) {
			return false;
		}

		int64_t exp_number = 0;

		if ((UC('e') == *p) || (UC('E') == *p)) {
			UC const* location_of_e = p;
			++p;
			bool neg_exp = false;
			if (UC('-') == *p) {
				neg_exp = true;
				++p;
			} else if (UC('+') == *p) {
				++p;
			}
			if (!is_integer(*p)) {
				p = location_of_e;
			} else {
				while (is_integer(*p)) {
					uint8_t digit = uint8_t(*p - zero);
					if (exp_number < 0x10000000) {
						exp_number = 10 * exp_number + digit;
					}
					++p;
				}
				if (neg_exp) {
					exp_number = -exp_number;
				}
				exponent += exp_number;
			}
		}

		if (digit_count > 19) {
			UC const* start = start_digits;
			while ((*start == zero || *start == decimal_point)) {
				if (*start == zero) {
					--digit_count;
				}
				++start;
			}

			if (digit_count > 20) {
				return false;
			}
		}

		for (int64_t i = 0; i < exponent; ++i) {
			if (value > INT64_MAX / 10) {
				return false;
			}
			value *= 10;
		}

		return true;
	}

	template<jsonifier::concepts::unsigned_type value_type, typename UC> constexpr bool parseInteger(UC const*& first, UC const* last, value_type& value) noexcept {
		value = 0;
		return parse_number_string_int<value_type, UC>(first, last, value);
	}

	JSONIFIER_ALWAYS_INLINE constexpr bool isSafeAddition(uint64_t a, uint64_t b) noexcept {
		return a <= (std::numeric_limits<uint64_t>::max)() - b;
	}

	JSONIFIER_ALWAYS_INLINE constexpr bool isSafeMultiplication10(uint64_t a) noexcept {
		constexpr uint64_t b = (std::numeric_limits<uint64_t>::max)() / 10;
		return a <= b;
	}

	template<typename value_type, typename char_type> JSONIFIER_ALWAYS_INLINE constexpr bool stoui64(value_type& res, const char_type* c) noexcept {
		if (!digitTableBool[static_cast<uint64_t>(*c)]) [[unlikely]] {
			return false;
		}

		constexpr std::array<uint32_t, 4> maxDigitsFromSize = { 4, 6, 11, 20 };
		constexpr auto N									= maxDigitsFromSize[static_cast<uint64_t>(std::bit_width(sizeof(value_type)) - 1)];

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

		while (digitTableBool[static_cast<uint64_t>(*c)]) {
			consumeDigit();
		}
		auto n = std::distance(digits.begin(), nextDigit);

		if (*c == '.') {
			++c;
			while (digitTableBool[static_cast<uint64_t>(*c)]) {
				consumeDigit();
			}
		}

		if (*c == 'e' || *c == 'E') {
			++c;

			bool negative = false;
			if (*c == '+' || *c == '-') {
				negative = (*c == '-');
				++c;
			}
			uint8_t exp = 0;
			while (digitTableBool[static_cast<uint64_t>(*c)] && exp < 128) {
				exp = static_cast<uint8_t>(10 * exp + (*c - 0x30u));
				++c;
			}
			n += negative ? -exp : exp;
		}

		res = 0;
		if (n < 0) [[unlikely]] {
			return true;
		}

		if constexpr (std::is_same_v<value_type, uint64_t>) {
			if (n > 20) [[unlikely]] {
				return false;
			}

			if (n == 20) [[unlikely]] {
				for (auto k = 0; k < 19; ++k) {
					res = static_cast<value_type>(10) * res + static_cast<value_type>(digits[static_cast<uint64_t>(k)]);
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
					res = static_cast<value_type>(10) * res + static_cast<value_type>(digits[static_cast<uint64_t>(k)]);
				}
			}
		} else {
			if (n >= N) [[unlikely]] {
				return false;
			} else [[likely]] {
				for (auto k = 0; k < n; ++k) {
					res = static_cast<value_type>(10) * res + static_cast<value_type>(digits[static_cast<uint64_t>(k)]);
				}
			}
		}

		return true;
	}
}