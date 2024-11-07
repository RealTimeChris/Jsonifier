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
/// https://github.com/RealTimeChris/jsonifier
/// Feb 3, 2023
#pragma once

#include <jsonifier/FastFloat.hpp>

namespace jsonifier_internal {

	template<typename T, typename UC> constexpr bool parseFloat(UC const*& iter, UC const* end, T& value) noexcept {
		using namespace fast_float;
		static_assert(is_supported_float_t<T>(), "only some floating-point types are supported");
		static_assert(is_supported_char_t<UC>(), "only char, wchar_t, char16_t and char32_t are supported");

		static constexpr UC decimalNew = '.';
		static constexpr UC smallE	   = 'e';
		static constexpr UC bigE	   = 'E';
		static constexpr UC minusNew   = '-';
		static constexpr UC plusNew	   = '+';
		static constexpr UC zeroNew	   = '0';

		parsed_number_string_t<UC> answer;
		answer.valid		   = false;
		answer.too_many_digits = false;
		answer.negative		   = (*iter == minusNew);
		if (answer.negative) {
			++iter;

			if JSONIFIER_UNLIKELY (!is_integer(*iter)) {
				return false;
			}
		}
		UC const* const start_digits = iter;

		uint64_t i = 0;

		while (is_integer(*iter)) {
			i = 10 * i + static_cast<uint64_t>(*iter - zeroNew);
			++iter;
		}

		UC const* const end_of_integer_part = iter;
		int64_t digit_count					= static_cast<int64_t>(end_of_integer_part - start_digits);
		answer.integer.length				= static_cast<size_t>(digit_count);
		answer.integer.ptr					= start_digits;

		if (digit_count == 0 || (start_digits[0] == zeroNew && digit_count > 1)) {
			return false;
		}

		int64_t exponent			 = 0;
		const bool has_decimal_point = [&] {
			return (*iter == decimalNew);
		}();
		if (has_decimal_point) {
			++iter;
			UC const* before = iter;
			loop_parse_if_eight_digits(iter, end, i);

			while (is_integer(*iter)) {
				uint8_t digit = static_cast<uint8_t>(*iter - zeroNew);
				++iter;
				i = i * 10 + digit;
			}
			exponent			   = before - iter;
			answer.fraction.length = static_cast<size_t>(iter - before);
			answer.fraction.ptr	   = before;
			digit_count -= exponent;
		}

		if (has_decimal_point && exponent == 0) {
			return false;
		}

		int64_t exp_number = 0;

		if ((smallE == *iter) || (bigE == *iter)) {
			UC const* location_of_e = iter;
			++iter;
			bool neg_exp = false;
			if (minusNew == *iter) {
				neg_exp = true;
				++iter;
			} else if (plusNew == *iter) {
				++iter;
			}
			if (!is_integer(*iter)) {
				iter = location_of_e;
			} else {
				while (is_integer(*iter)) {
					uint8_t digit = static_cast<uint8_t>(*iter - zeroNew);
					if (exp_number < 0x10000000) {
						exp_number = 10 * exp_number + digit;
					}
					++iter;
				}
				if (neg_exp) {
					exp_number = -exp_number;
				}
				exponent += exp_number;
			}
		}

		answer.lastmatch = iter;
		answer.valid	 = true;

		if (digit_count > 19) {
			UC const* start = start_digits;
			while ((*start == zeroNew || *start == decimalNew)) {
				if (*start == zeroNew) {
					--digit_count;
				}
				++start;
			}

			if (digit_count > 19) {
				answer.too_many_digits = true;
				i					   = 0;
				iter				   = answer.integer.ptr;
				UC const* int_end	   = iter + answer.integer.len();
				static constexpr uint64_t minimal_nineteen_digit_integer{ 1000000000000000000 };
				while ((i < minimal_nineteen_digit_integer) && (iter != int_end)) {
					i = i * 10 + static_cast<uint64_t>(*iter - zeroNew);
					++iter;
				}
				if (i >= minimal_nineteen_digit_integer) {
					exponent = end_of_integer_part - iter + exp_number;
				} else {
					iter			   = answer.fraction.ptr;
					UC const* frac_end = iter + answer.fraction.len();
					while ((i < minimal_nineteen_digit_integer) && (iter != frac_end)) {
						i = i * 10 + static_cast<uint64_t>(*iter - zeroNew);
						++iter;
					}
					exponent = answer.fraction.ptr - iter + exp_number;
				}
			}
		}
		answer.exponent = exponent;
		answer.mantissa = i;
		if JSONIFIER_LIKELY (answer.valid) {
			iter = answer.lastmatch;
			return from_chars_advanced(answer, value).ptr != nullptr;
		} else {
			return false;
		}
	}
}