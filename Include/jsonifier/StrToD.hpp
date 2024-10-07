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

	template<typename T, typename UC> constexpr bool parseFloat(UC const*& first, UC const* end, T& value) noexcept {
		using namespace fast_float;
		static_assert(is_supported_float_type<T>(), "only some floating-point types are supported");
		static_assert(is_supported_char_type<UC>(), "only char, wchar_t, char16_t and char32_t are supported");

		static constexpr UC decimalPoint = '.';
		static constexpr UC smallE		 = 'e';
		static constexpr UC bigE		 = 'E';
		static constexpr UC minus		 = '-';
		static constexpr UC plus		 = '+';
		static constexpr UC zero		 = '0';

		parsed_number_string_t<UC> answer;
		answer.valid		   = false;
		answer.too_many_digits = false;
		answer.negative		   = (*first == minus);
		if (*first == minus) {
			++first;

			if JSONIFIER_UNLIKELY ((!is_integer(*first))) {
				return false;
			}
		}
		UC const* const start_digits = first;

		uint64_t i = 0;

		while (is_integer(*first)) {
			i = 10 * i + uint64_t(*first - zero);
			++first;
		}

		UC const* const end_of_integer_part = first;
		int64_t digit_count					= int64_t(end_of_integer_part - start_digits);
		answer.integer						= fast_float::span<const UC>(start_digits, size_t(digit_count));

		if (digit_count == 0 || (start_digits[0] == zero && digit_count > 1)) {
			return false;
		}

		int64_t exponent			 = 0;
		const bool has_decimal_point = [&] {
			return (*first == decimalPoint);
		}();
		if (has_decimal_point) {
			++first;
			UC const* before = first;
			loop_parse_if_eight_digits(first, end, i);

			while (is_integer(*first)) {
				uint8_t digit = uint8_t(*first - zero);
				++first;
				i = i * 10 + digit;
			}
			exponent		= before - first;
			answer.fraction = fast_float::span<const UC>(before, size_t(first - before));
			digit_count -= exponent;
		}

		if (has_decimal_point && exponent == 0) {
			return false;
		}

		int64_t exp_number = 0;

		if ((smallE == *first) || (bigE == *first)) {
			UC const* location_of_e = first;
			++first;
			bool neg_exp = false;
			if (minus == *first) {
				neg_exp = true;
				++first;
			} else if (plus == *first) {
				++first;
			}
			if (!is_integer(*first)) {
				first = location_of_e;
			} else {
				while (is_integer(*first)) {
					uint8_t digit = uint8_t(*first - zero);
					if (exp_number < 0x10000000) {
						exp_number = 10 * exp_number + digit;
					}
					++first;
				}
				if (neg_exp) {
					exp_number = -exp_number;
				}
				exponent += exp_number;
			}
		}

		answer.lastmatch = first;
		answer.valid	 = true;

		if (digit_count > 19) {
			UC const* start = start_digits;
			while ((*start == zero || *start == decimalPoint)) {
				if (*start == zero) {
					--digit_count;
				}
				++start;
			}

			if (digit_count > 19) {
				answer.too_many_digits = true;
				i					   = 0;
				first				   = answer.integer.ptr;
				UC const* int_end	   = first + answer.integer.len();
				static constexpr uint64_t minimal_nineteen_digit_integer{ 1000000000000000000 };
				while ((i < minimal_nineteen_digit_integer) && (first != int_end)) {
					i = i * 10 + uint64_t(*first - zero);
					++first;
				}
				if (i >= minimal_nineteen_digit_integer) {
					exponent = end_of_integer_part - first + exp_number;
				} else {
					first			   = answer.fraction.ptr;
					UC const* frac_end = first + answer.fraction.len();
					while ((i < minimal_nineteen_digit_integer) && (first != frac_end)) {
						i = i * 10 + uint64_t(*first - zero);
						++first;
					}
					exponent = answer.fraction.ptr - first + exp_number;
				}
			}
		}
		answer.exponent = exponent;
		answer.mantissa = i;
		if JSONIFIER_LIKELY ((answer.valid)) {
			first = answer.lastmatch;
			return from_chars_advanced(answer, value).ptr != nullptr;
		} else {
			return false;
		}
	}
}