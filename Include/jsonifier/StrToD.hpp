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

	JSONIFIER_ALWAYS_INLINE_VARIABLE bool expTable[]{ false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,
		false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,
		false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,
		false, false, false, true, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,
		false, false, false, false, false, false, false, false, false, false, false, true, false, false, false, false, false, false, false, false, false, false, false, false,
		false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,
		false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,
		false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,
		false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,
		false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,
		false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false };

	JSONIFIER_ALWAYS_INLINE_VARIABLE bool expFracTable[]{ false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,
		false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,
		false, false, false, false, false, true, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,
		false, false, false, false, true, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,
		false, false, false, false, false, false, false, false, false, false, false, false, true, false, false, false, false, false, false, false, false, false, false, false,
		false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,
		false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,
		false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,
		false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,
		false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,
		false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false };

	JSONIFIER_ALWAYS_INLINE_VARIABLE char decimal{ '.' };
	JSONIFIER_ALWAYS_INLINE_VARIABLE char minus{ '-' };
	JSONIFIER_ALWAYS_INLINE_VARIABLE char plus{ '+' };
	JSONIFIER_ALWAYS_INLINE_VARIABLE char zero{ '0' };
	JSONIFIER_ALWAYS_INLINE_VARIABLE char nine{ '9' };

	template<typename value_type, typename char_t> JSONIFIER_INLINE bool parseFloat(value_type& value, char_t const*& iter, char_t const* end = nullptr) noexcept {
		using namespace jsonifier_fast_float;
		span<const char_t> fraction;

		int64_t expNumber{};
		int64_t exponent{};
		uint64_t mantissa{};

		if (iter >= end) {
			return false;
		}

		const bool negative{ *iter == minus };
		bool tooManyDigits{ false };

		if (negative) {
			++iter;

			if JSONIFIER_UNLIKELY (!JSONIFIER_IS_DIGIT(*iter)) {
				return false;
			}
		}

		span<const char_t> integer{ iter };

		if (uint64_t val; (end - iter) >= 2 && (val = read2_to_u64(iter) - 0x3030, is_made_of_two_digits_no_sub(val))) {
			mantissa = mantissa * 100 + parse_two_digits_unrolled_no_sub(val);
			iter += 2;
		}

		while (JSONIFIER_IS_DIGIT(*iter)) {
			mantissa = 10 * mantissa + static_cast<uint8_t>(*iter - zero);
			++iter;
		}

		int64_t digitCount = static_cast<int64_t>(iter - integer.ptr);
		integer.end		   = integer.ptr + static_cast<uint64_t>(digitCount);

		if JSONIFIER_UNLIKELY (digitCount == 0 || (integer.ptr[0] == zero && digitCount > 1)) {
			return false;
		}

		char_t const* before;

		if (*iter == decimal) {
			++iter;
			before = iter;

			loop_parse_if_eight_digits(iter, end, mantissa);

			while ((iter != end) && JSONIFIER_IS_DIGIT(*iter)) {
				uint8_t digit = uint8_t(*iter - char('0'));
				++iter;
				mantissa = mantissa * 10 + digit;
			}

			exponent	 = before - iter;
			fraction.ptr = before;
			fraction.end = fraction.ptr + static_cast<uint64_t>(iter - before);
			digitCount -= exponent;

			if JSONIFIER_UNLIKELY (exponent == 0) {
				return false;
			}
		}

		if (expTable[static_cast<uint8_t>(*iter)]) {
			before = iter;
			++iter;
			bool negExp = false;
			if (minus == *iter) {
				negExp = true;
				++iter;
			} else if (plus == *iter) {
				++iter;
			}
			if (!JSONIFIER_IS_DIGIT(*iter)) {
				iter = before;
			} else {
				while (JSONIFIER_IS_DIGIT(*iter)) {
					if (expNumber < 0x10000000) {
						expNumber = 10 * expNumber + static_cast<uint8_t>(*iter - zero);
					}
					++iter;
				}
				if (negExp) {
					expNumber = -expNumber;
				}
				exponent += expNumber;
			}
		}

		if (digitCount > 19) {
			before = integer.ptr;
			while ((*before == zero || *before == decimal)) {
				if (*before == zero) {
					--digitCount;
				}
				++before;
			}

			if (digitCount > 19) {
				tooManyDigits = true;
				mantissa	  = 0;
				before		  = integer.ptr;
				static constexpr uint64_t minNineteenDigitInteger{ 1000000000000000000 };
				while ((mantissa < minNineteenDigitInteger) && (before != integer.end)) {
					mantissa = mantissa * 10 + static_cast<uint8_t>(*before - zero);
					++before;
				}
				if (mantissa >= minNineteenDigitInteger) {
					exponent = integer.end - before + expNumber;
				} else {
					before = fraction.ptr;
					while ((mantissa < minNineteenDigitInteger) && (before != fraction.end)) {
						mantissa = mantissa * 10 + static_cast<uint8_t>(*before - zero);
						++before;
					}
					exponent = fraction.ptr - before + expNumber;
				}
			}
		}

		if (binary_format<value_type>::min_exponent_fast_path <= exponent && exponent <= binary_format<value_type>::max_exponent_fast_path && !tooManyDigits) {
			if (rounds_to_nearest::roundsToNearest) {
				if (mantissa <= binary_format<value_type>::max_mantissa_fast_path_value) {
					value = value_type(mantissa);
					if (exponent < 0) {
						value = value / binary_format<value_type>::exact_power_of_ten(-exponent);
					} else {
						value = value * binary_format<value_type>::exact_power_of_ten(exponent);
					}
					if (negative) {
						value = -value;
					}
					return true;
				}
			} else {
				if (exponent >= 0 && mantissa <= binary_format<value_type>::max_mantissa_fast_path(exponent)) {
#if defined(__clang__) || defined(JSONIFIER_FASTFLOAT_32BIT)
					if (mantissa == 0) {
						value = negative ? value_type(-0.) : value_type(0.);
						return true;
					}
#endif
					value = value_type(mantissa) * binary_format<value_type>::exact_power_of_ten(exponent);
					if (negative) {
						value = -value;
					}
					return true;
				}
			}
		}
		adjusted_mantissa am = compute_float<binary_format<value_type>>(exponent, mantissa);
		if (tooManyDigits && am.power2 >= 0) {
			if (am != compute_float<binary_format<value_type>>(exponent, mantissa + 1)) {
				am = compute_error<binary_format<value_type>>(exponent, mantissa);
			}
		}
		if JSONIFIER_UNLIKELY (am.power2 < 0) {
			am = digit_comp<value_type>(integer, fraction, mantissa, exponent, am);
		}
		to_float(negative, am, value);
		if JSONIFIER_UNLIKELY ((mantissa != 0 && am.mantissa == 0 && am.power2 == 0) || am.power2 == binary_format<value_type>::infinite_power) {
			return false;
		}
		return true;
	}
}