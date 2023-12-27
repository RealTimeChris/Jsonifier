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
#include <jsonifier/Tables.hpp>

#include <concepts>
#include <cstdint>
#include <cstring>
#include <array>

namespace jsonifier_internal {

	constexpr bool isDigit(const char c) noexcept {
		return c >= '0' && c <= '9';
	}

	constexpr bool isSafeAddition(uint64_t a, uint64_t b) noexcept {
		return a <= (std::numeric_limits<uint64_t>::max)() - b;
	}

	constexpr bool isSafeMultiplication10(uint64_t a) noexcept {
		constexpr uint64_t b = (std::numeric_limits<uint64_t>::max)() / 10;
		return a <= b;
	}

	template<jsonifier::concepts::unsigned_t value_type, typename char_type> JSONIFIER_INLINE bool parseNumberInternal(value_type& value, char_type* cur) {
		static constexpr auto is_volatile = std::is_volatile_v<std::remove_reference_t<decltype(value)>>;
		const char_type* sig_cut{};
		[[maybe_unused]] const char_type* sig_end{};
		const char_type* dot_pos{};
		uint32_t frac_zeros = 0;
		uint64_t sig		= uint64_t(*cur - '0');
		int32_t exp			= 0;
		bool exp_sign;
		int32_t exp_sig = 0;
		int32_t exp_lit = 0;
		uint64_t num_tmp;
		const char_type* tmp;

		if (sig > 9) [[unlikely]] {
			return false;
		}
		static constexpr auto zero = uint8_t('0');
#define expr_intg(i) \
	if ((num_tmp = cur[i] - zero) <= 9) [[likely]] \
		sig = num_tmp + sig * 10; \
	else { \
		goto digi_sepr_##i; \
	}
		repeat_in_1_18(expr_intg);
#undef expr_intg
		if (*cur == zero)
			return false;
		cur += 19;
		if (!digiIsDigitOrFp(*cur)) {
			value = static_cast<value_type>(sig);
			return true;
		}
		goto digi_intg_more;
#define expr_sepr(i) \
	digi_sepr_##i : if ((!digiIsFp(uint8_t(cur[i])))) [[likely]] { \
		cur += i; \
		value = sig; \
		return true; \
	} \
	dot_pos = cur + i; \
	if ((cur[i] == '.')) [[likely]] { \
		if (sig == 0) \
			while (cur[frac_zeros + i + 1] == zero) \
				++frac_zeros; \
		goto digi_frac_##i; \
	} \
	cur += i; \
	sig_end = cur; \
	goto digi_exp_more;
		repeat_in_1_18(expr_sepr)
#undef expr_sepr
#define expr_frac(i) \
	digi_frac_##i : if (((num_tmp = uint64_t(cur[i + 1 + frac_zeros] - zero)) <= 9)) [[likely]] sig = num_tmp + sig * 10; \
	else { \
		goto digi_stop_##i; \
	}
			repeat_in_1_18(expr_frac)
#undef expr_frac
				cur += 20 + frac_zeros;
		if (uint8_t(*cur - zero) > 9)
			goto digi_frac_end;
		goto digi_frac_more;
#define expr_stop(i) \
	digi_stop_##i : cur += i + 1 + frac_zeros; \
	goto digi_frac_end;
		repeat_in_1_18(expr_stop)
#undef expr_stop
			digi_intg_more : static constexpr uint64_t U64_MAX = (std::numeric_limits<uint64_t>::max)();
		if ((num_tmp = *cur - zero) < 10) {
			if (!digiIsDigitOrFp(cur[1])) {
				if ((sig < (U64_MAX / 10)) || (sig == (U64_MAX / 10) && num_tmp <= (U64_MAX % 10))) {
					sig = num_tmp + sig * 10;
					cur++;
					value = static_cast<value_type>(sig);
					return true;
				}
			}
		}
		if ((eBit | *cur) == 'e') {
			dot_pos = cur;
			goto digi_exp_more;
		}
		if (*cur == '.') {
			dot_pos = cur++;
			if (uint8_t(*cur - zero) > 9) [[unlikely]] {
				return false;
			}
		}
	digi_frac_more:
		sig_cut = cur;
		sig += (*cur >= '5');
		while (uint8_t(*++cur - zero) < 10) {
		}
		if (!dot_pos) {
			dot_pos = cur;
			if (*cur == '.') {
				if (uint8_t(*++cur - zero) > 9) [[unlikely]] {
					return false;
				}
				while (uint8_t(*++cur - zero) < 10) {
				}
			}
		}
		exp_sig = int32_t(dot_pos - sig_cut);
		exp_sig += (dot_pos < sig_cut);
		tmp = cur - 1;
		while (*tmp == '0' || *tmp == '.')
			tmp--;
		if (tmp < sig_cut) {
			sig_cut = nullptr;
		} else {
			sig_end = cur;
		}
		if ((eBit | *cur) == 'e')
			goto digi_exp_more;
		goto digi_exp_finish;
	digi_frac_end:
		sig_end = cur;
		exp_sig = -int32_t((cur - dot_pos) - 1);
		if (exp_sig == 0) [[unlikely]]
			return false;
		if ((eBit | *cur) != 'e') [[likely]] {
			if ((exp_sig < f64MinDecExp - 19)) [[unlikely]] {
				value = 0;
				return true;
			}
			exp = exp_sig;
			goto digi_finish;
		} else {
			goto digi_exp_more;
		}
	digi_exp_more:
		exp_sign = (*++cur == '-');
		cur += (*cur == '+' || *cur == '-');
		if (uint8_t(*cur - zero) > 9) {
			return false;
		}
		while (*cur == '0')
			++cur;
		tmp = cur;
		uint8_t c;
		while (uint8_t(c = *cur - zero) < 10) {
			++cur;
			exp_lit = c + uint32_t(exp_lit) * 10;
		}
		if ((cur - tmp >= 6)) [[unlikely]] {
			if (sig == 0 || exp_sign) {
				value = 0;
				return true;
			} else {
				return false;
			}
		}
		exp_sig += exp_sign ? -exp_lit : exp_lit;
	digi_exp_finish:
		if (sig == 0) {
			value = 0;
			return true;
		}
		if (exp_sig == 19) {
			if constexpr (is_volatile) {
				value = value * value_type(powersOfTenInt[exp_sig - 1]);
				if (isSafeMultiplication10(value)) [[likely]] {
					value = value * 10;
					return value;
				} else [[unlikely]] {
					return false;
				}
			} else {
				value *= value_type(powersOfTenInt[exp_sig - 1]);
				if (isSafeMultiplication10(value)) [[likely]] {
					return value *= 10;
				} else [[unlikely]] {
					return false;
				}
			}
		} else if (exp_sig >= 20) [[unlikely]] {
			return false;
		}
		exp = exp_sig;
	digi_finish:

		if (exp <= -20) [[unlikely]] {
			value = value_type(0);
			return true;
		}

		value = static_cast<value_type>(sig);
		if constexpr (is_volatile) {
			if (exp >= 0) {
				value = value * value_type(powersOfTenInt[exp]);
			} else {
				value = value / value_type(powersOfTenInt[-exp]);
			}
		} else {
			if (exp >= 0) {
				value *= value_type(powersOfTenInt[exp]);
			} else {
				value /= value_type(powersOfTenInt[-exp]);
			}
		}
		return true;
	}

	template<jsonifier::concepts::integer_t value_type, typename char_type> JSONIFIER_INLINE bool parseNumber(value_type& value, char_type* curNew) {
		static constexpr auto maximum = uint64_t((std::numeric_limits<value_type>::max)());
		if constexpr (std::is_unsigned_v<value_type>) {
			if constexpr (std::same_as<value_type, uint64_t>) {
				if (*curNew == '-') [[unlikely]] {
					return false;
				}
				static_assert(sizeof(*curNew) == sizeof(char));
				const char_type* cur = reinterpret_cast<const char_type*>(&*curNew);
				if constexpr (std::is_volatile_v<decltype(value)>) {
					uint64_t i{};
					auto s = parseNumberInternal<uint64_t, char_type>(i, cur);
					if (!s) [[unlikely]] {
						return false;
					}
					value = i;
				} else {
					auto s = parseNumberInternal<jsonifier::concepts::decay_keep_volatile_t<decltype(value)>, char_type>(value, cur);
					if (!s) [[unlikely]] {
						return false;
					}
				}
			} else {
				uint64_t i{};
				if (*curNew == '-') [[unlikely]] {
					return false;
				}

				static_assert(sizeof(*curNew) == sizeof(char));
				const char_type* cur = reinterpret_cast<const char_type*>(&*curNew);
				auto s				 = parseNumberInternal<std::decay_t<decltype(i)>, char_type>(i, cur);
				if (!s) [[unlikely]] {
					return false;
				}

				if (i > maximum) [[unlikely]] {
					return false;
				}
				value = value_type(i);
			}
		} else {
			uint64_t i{};
			int sign = 1;
			if (*curNew == '-') {
				sign = -1;
				++curNew;
			}

			static_assert(sizeof(*curNew) == sizeof(char));
			const char_type* cur = reinterpret_cast<const char_type*>(&*curNew);
			auto s				 = parseNumberInternal<jsonifier::concepts::decay_keep_volatile_t<decltype(i)>, char_type>(i, cur);
			if (!s) [[unlikely]] {
				return false;
			}

			if (sign == -1) {
				static constexpr auto min_abs = uint64_t((std::numeric_limits<value_type>::max)()) + 1;
				if (i > min_abs) [[unlikely]] {
					return false;
				}
				value = value_type(sign * i);
			} else {
				if (i > maximum) [[unlikely]] {
					return false;
				}
				value = value_type(i);
			}
		}
		return true;
	}

	template<typename value_type, typename char_type> constexpr bool stoui64(value_type& res, const char_type* c) noexcept {
		if (!isDigit(*c)) [[unlikely]] {
			return false;
		}

		constexpr std::array<uint32_t, 4> max_digits_from_size = { 4, 6, 11, 20 };
		constexpr auto N									   = max_digits_from_size[std::bit_width(sizeof(value_type)) - 1];

		std::array<uint8_t, N> digits{ 0 };
		auto next_digit	   = digits.begin();
		auto consume_digit = [&c, &next_digit, &digits]() {
			if (next_digit < digits.cend()) [[likely]] {
				*next_digit = (*c - '0');
				++next_digit;
			}
			++c;
		};

		if (*c == '0') {
			++c;
			++next_digit;

			if (*c == '0') [[unlikely]] {
				return false;
			}
		}

		while (isDigit(*c)) {
			consume_digit();
		}
		auto n = std::distance(digits.begin(), next_digit);

		if (*c == '.') {
			++c;
			while (isDigit(*c)) {
				consume_digit();
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
			while (isDigit(*c) && exp < 128) {
				exp = 10 * exp + (*c - '0');
				++c;
			}
			n += negative ? -exp : exp;
		}

		res = 0;
		if (n < 0) [[unlikely]] {
			return true;
		}

		if constexpr (std::same_as<value_type, uint64_t>) {
			if (n > 20) [[unlikely]] {
				return false;
			}

			if (n == 20) [[unlikely]] {
				for (auto k = 0; k < 19; ++k) {
					res = 10 * res + digits[k];
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
					res = 10 * res + digits[k];
				}
			}
		} else {
			if (n >= N) [[unlikely]] {
				return false;
			} else [[likely]] {
				for (auto k = 0; k < n; ++k) {
					res = 10 * res + digits[k];
				}
			}
		}

		return true;
	}

	template<typename value_type, typename char_type> constexpr bool stoui64(value_type& res, char_type* curNew) noexcept {
		static_assert(sizeof(*curNew) == sizeof(char));
		const char_type* cur = reinterpret_cast<const char_type*>(&*curNew);
		if (stoui64(res, cur)) {
			return true;
		}
		return false;
	}
}