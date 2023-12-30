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

	JSONIFIER_INLINE constexpr bool isDigit(const char c) noexcept {
		return c >= '0' && c <= '9';
	}

	JSONIFIER_INLINE constexpr bool isSafeAddition(uint64_t a, uint64_t b) noexcept {
		return a <= (std::numeric_limits<uint64_t>::max)() - b;
	}

	JSONIFIER_INLINE constexpr bool isSafeMultiplication10(uint64_t a) noexcept {
		constexpr uint64_t b = (std::numeric_limits<uint64_t>::max)() / 10;
		return a <= b;
	}

	template<jsonifier::concepts::signed_t value_type01, typename char_type> JSONIFIER_INLINE bool parseNumber(value_type01& value, char_type*& cur) {
		[[maybe_unused]] const char_type* sigEnd{};
		const char_type *tmp{}, *sigCut{}, *dotPos{}, *hdr{ cur };
		uint64_t fracZeros{}, numTmp{}, sig{};
		int64_t exp{}, expSig{}, expLit{};
		bool sign{ (*hdr == 0x2Du) }, expSign{};
		auto applySign = [&](auto&& value) -> value_type01 {
			return sign ? -static_cast<value_type01>(value) : static_cast<value_type01>(value);
		};
		if (*cur == 0x30u && numberTable[*(cur + 1)] || *(cur + 1) == 0x78u) {
			return false;
		}
		cur += sign;
		sig = static_cast<uint64_t>(asciiToValueTable[static_cast<uint64_t>(*cur)]);
		if (sig > 9) {
			if (*cur == 0x6Eu && cur[1] == 0x75u && cur[2] == 0x6Cu && cur[3] == 0x6Cu) {
				value = applySign(0);
				return true;
			} else if (( *cur | eBit<char_type> ) == 0x6Eu && ( cur[1] | eBit<char_type> ) == 0x61u && ( cur[2] | eBit<char_type> ) == 0x6Eu) {
				value = applySign(std::numeric_limits<value_type01>::quiet_NaN());
				return true;
			} else {
				return false;
			}
		}
		static constexpr auto zero = static_cast<uint8_t>(0x30u);
#define expr_intg(x) \
	if (numTmp = static_cast<uint64_t>(asciiToValueTable[static_cast<uint64_t>(cur[x])]); numTmp <= 9) [[likely]] \
		sig = static_cast<uint64_t>(numTmp) + sig * 10ull; \
	else { \
		goto digi_sepr_##x; \
	}
		repeat_in_1_18(expr_intg);
#undef expr_intg
		cur += 19;
		if (!digiIsDigitOrFp(static_cast<uint8_t>(*cur))) {
			value = applySign(static_cast<value_type01>(sig));
			return true;
		}
		goto digi_intg_more;
#define expr_sepr(x) \
	digi_sepr_##x : if (!digiIsFp(static_cast<uint8_t>(cur[x]))) [[likely]] { \
		cur += x; \
		value = applySign(static_cast<value_type01>(sig)); \
		return true; \
	} \
	dotPos = cur + x; \
	if (cur[x] == 0x2Eu) [[likely]] { \
		if (sig == 0) \
			while (cur[fracZeros + x + 1] == static_cast<uint64_t>(zero)) \
				++fracZeros; \
		goto digi_frac_##x; \
	} \
	cur += x; \
	sigEnd = cur; \
	goto digi_exp_more;
		repeat_in_1_18(expr_sepr)
#undef expr_sepr
#define expr_frac(x) \
	digi_frac_##x : if (numTmp = static_cast<uint64_t>(asciiToValueTable[static_cast<uint64_t>(cur[static_cast<uint64_t>(x + 1 + fracZeros)])]); numTmp <= 9) [[likely]] sig = \
						numTmp + sig * 10; \
	else { \
		goto digi_stop_##x; \
	}
			repeat_in_1_18(expr_frac)
#undef expr_frac
				cur += 20ull + fracZeros;
		if (auto newValue = asciiToValueTable[static_cast<uint64_t>(*cur)]; newValue > 9) [[unlikely]]
			goto digi_frac_end;
		goto digi_frac_more;
#define expr_stop(x) \
	digi_stop_##x : cur += x##ull + 1ull + fracZeros; \
	goto digi_frac_end;
		repeat_in_1_18(expr_stop)
#undef expr_stop
			digi_intg_more : static constexpr uint64_t uint64_tMax = std::numeric_limits<uint64_t>::max();
		if (numTmp = static_cast<uint64_t>(asciiToValueTable[static_cast<uint64_t>(*cur)]); numTmp < 10) {
			if (!digiIsDigitOrFp(static_cast<uint8_t>(cur[1]))) {
				if ((sig < (uint64_tMax / 10)) || (sig == (uint64_tMax / 10) && numTmp <= (uint64_tMax % 10))) {
					sig = numTmp + sig * 10;
					++cur;
					value = applySign(static_cast<value_type01>(sig));
					return true;
				}
			}
		}
		if ((eBit<char_type> | *cur) == 0x65u) {
			dotPos = cur;
			goto digi_exp_more;
		}
		if (*cur == 0x2Eu) {
			dotPos = cur++;
			if (auto newValue = asciiToValueTable[static_cast<uint64_t>(*cur)]; newValue > 9) [[unlikely]] {
				return false;
			}
		}
	digi_frac_more:
		sigCut = cur;
		sig += (*cur >= 0x35);
		while (asciiToValueTable[static_cast<uint64_t>(*++cur)] < 10) {
		}
		if (!dotPos) {
			dotPos = cur;
			if (*cur == 0x2E) {
				if (auto newValue = asciiToValueTable[static_cast<uint64_t>(*++cur)]; newValue > 9) [[unlikely]] {
					return false;
				}
				while (asciiToValueTable[static_cast<uint64_t>(*++cur)] < 10) {
				}
			}
		}
		expSig = static_cast<int64_t>(dotPos - sigCut);
		expSig += (dotPos < sigCut);
		tmp = cur - 1;
		while (*tmp == 0x30u || *tmp == 0x2Eu)
			tmp--;
		if (tmp < sigCut) {
			sigCut = nullptr;
		}
		if ((eBit<char_type> | *cur) == 0x65u)
			goto digi_exp_more;
		goto digi_exp_finish;
	digi_frac_end:
		expSig = -int64_t((cur - dotPos) - 1);
		if ((eBit<char_type> | *cur) != 0x65u) [[likely]] {
			if (expSig < f64MinDecExp - 19) [[unlikely]] {
				value = applySign(0);
				return true;
			}
			exp = expSig;
			goto digi_finish;
		} else {
			goto digi_exp_more;
		}
	digi_exp_more: {
		expSign = (*++cur == 0x2Du);
		cur += (*cur == 0x2Bu || *cur == 0x2Du);
		if (auto newValue = asciiToValueTable[static_cast<uint64_t>(*cur)]; newValue > 9) [[unlikely]] {
			goto digi_finish;
		}
		while (*cur == 0x30u)
			++cur;
		tmp = cur;
		uint8_t c{};
		while (c < 10) {
			c = static_cast<uint8_t>(asciiToValueTable[static_cast<uint64_t>(*cur)]);
			++cur;
			expLit = c + int64_t(expLit) * 10;
		}
		if (cur - tmp >= 6) [[unlikely]] {
			if (sig == 0 || expSign) {
				value = applySign(sig);
				return true;
			} else {
				value = applySign(std::numeric_limits<value_type01>::infinity());
				return true;
			}
		}
		expSig += expSign ? -expLit : expLit;
	}

	digi_exp_finish:
		if (sig == 0) {
			value = applySign(!sign ? -0 : 0);
			return true;
		}
		if (expSig < -20) {
			value = applySign(0);
			return true;
		} else if (expSig > 20) {
			value = applySign(std::numeric_limits<value_type01>::infinity());
			return true;
		}
		exp = expSig;
	digi_finish:

		value = applySign(sig);
		if (exp >= 0 && exp < 20) {
			value *= applySign(powersOfTenInt[exp]);
		} else if (exp > -20 && exp < 0) {
			value /= applySign(powersOfTenInt[-exp]);
		}
		return true;
	}

	template<jsonifier::concepts::unsigned_t value_type01, typename char_type> JSONIFIER_INLINE bool parseNumber(value_type01& value, char_type*& cur) {
		[[maybe_unused]] const char_type* sigEnd{};
		const char_type *tmp{}, *sigCut{}, *dotPos{};
		uint64_t fracZeros{}, numTmp{}, sig{};
		int64_t exp{}, expSig{}, expLit{};
		sig = static_cast<uint64_t>(asciiToValueTable[static_cast<uint64_t>(*cur)]);
		if (*cur == 0x30u && numberTable[*(cur + 1)] || *(cur + 1) == 0x78u) {
			return false;
		}
		if (sig > 9) {
			if (*cur == 0x6Eu && cur[1] == 0x75u && cur[2] == 0x6Cu && cur[3] == 0x6Cu) {
				value = static_cast<value_type01>(0);
				return true;
			} else if (( *cur | eBit<char_type> ) == 0x6Eu && ( cur[1] | eBit<char_type> ) == 0x61u && ( cur[2] | eBit<char_type> ) == 0x6Eu) {
				value = static_cast<value_type01>(std::numeric_limits<value_type01>::quiet_NaN());
				return true;
			} else {
				return false;
			}
		}
		static constexpr auto zero = static_cast<uint8_t>(0x30u);
#define expr_intg(x) \
	if (numTmp = static_cast<uint64_t>(asciiToValueTable[static_cast<uint64_t>(cur[x])]); numTmp <= 9) [[likely]] \
		sig = static_cast<uint64_t>(numTmp) + sig * 10ull; \
	else { \
		goto digi_sepr_##x; \
	}
		repeat_in_1_18(expr_intg);
#undef expr_intg
		cur += 19;
		if (!digiIsDigitOrFp(static_cast<uint8_t>(*cur))) {
			value = static_cast<value_type01>(static_cast<value_type01>(sig));
			return true;
		}
		goto digi_intg_more;
#define expr_sepr(x) \
	digi_sepr_##x : if (!digiIsFp(static_cast<uint8_t>(cur[x]))) [[likely]] { \
		cur += x; \
		value = static_cast<value_type01>(static_cast<value_type01>(sig)); \
		return true; \
	} \
	dotPos = cur + x; \
	if (cur[x] == 0x2Eu) [[likely]] { \
		if (sig == 0) \
			while (cur[fracZeros + x + 1] == static_cast<uint64_t>(zero)) \
				++fracZeros; \
		goto digi_frac_##x; \
	} \
	cur += x; \
	sigEnd = cur; \
	goto digi_exp_more;
		repeat_in_1_18(expr_sepr)
#undef expr_sepr
#define expr_frac(x) \
	digi_frac_##x : if (numTmp = static_cast<uint64_t>(asciiToValueTable[static_cast<uint64_t>(cur[static_cast<uint64_t>(x + 1 + fracZeros)])]); numTmp <= 9) [[likely]] sig = \
						numTmp + sig * 10; \
	else { \
		goto digi_stop_##x; \
	}
			repeat_in_1_18(expr_frac)
#undef expr_frac
				cur += 20ull + fracZeros;
		if (auto newValue = asciiToValueTable[static_cast<uint64_t>(*cur)]; newValue > 9) [[unlikely]]
			goto digi_frac_end;
		goto digi_frac_more;
#define expr_stop(x) \
	digi_stop_##x : cur += x##ull + 1ull + fracZeros; \
	goto digi_frac_end;
		repeat_in_1_18(expr_stop)
#undef expr_stop
			digi_intg_more : static constexpr uint64_t uint64_tMax = std::numeric_limits<uint64_t>::max();
		if (numTmp = static_cast<uint64_t>(asciiToValueTable[static_cast<uint64_t>(*cur)]); numTmp < 10) {
			if (!digiIsDigitOrFp(static_cast<uint8_t>(cur[1]))) {
				if ((sig < (uint64_tMax / 10)) || (sig == (uint64_tMax / 10) && numTmp <= (uint64_tMax % 10))) {
					sig = numTmp + sig * 10;
					++cur;
					value = static_cast<value_type01>(static_cast<value_type01>(sig));
					return true;
				}
			}
		}
		if ((eBit<char_type> | *cur) == 0x65u) {
			dotPos = cur;
			goto digi_exp_more;
		}
		if (*cur == 0x2Eu) {
			dotPos = cur++;
			if (auto newValue = asciiToValueTable[static_cast<uint64_t>(*cur)]; newValue > 9) [[unlikely]] {
				return false;
			}
		}
	digi_frac_more:
		sigCut = cur;
		sig += (*cur >= 0x35);
		while (asciiToValueTable[static_cast<uint64_t>(*++cur)] < 10) {
		}
		if (!dotPos) {
			dotPos = cur;
			if (*cur == 0x2E) {
				if (auto newValue = asciiToValueTable[static_cast<uint64_t>(*++cur)]; newValue > 9) [[unlikely]] {
					return false;
				}
				while (asciiToValueTable[static_cast<uint64_t>(*++cur)] < 10) {
				}
			}
		}
		expSig = static_cast<int64_t>(dotPos - sigCut);
		expSig += (dotPos < sigCut);
		tmp = cur - 1;
		while (*tmp == 0x30u || *tmp == 0x2Eu)
			tmp--;
		if (tmp < sigCut) {
			sigCut = nullptr;
		}
		if ((eBit<char_type> | *cur) == 0x65u)
			goto digi_exp_more;
		goto digi_exp_finish;
	digi_frac_end:
		expSig = -int64_t((cur - dotPos) - 1);
		if ((eBit<char_type> | *cur) != 0x65u) [[likely]] {
			if (expSig < f64MinDecExp - 19) [[unlikely]] {
				value = static_cast<value_type01>(0);
				return true;
			}
			exp = expSig;
			goto digi_finish;
		} else {
			goto digi_exp_more;
		}
	digi_exp_more: {
		cur += (*cur == 0x2Bu || *cur == 0x2Du);
		if (auto newValue = asciiToValueTable[static_cast<uint64_t>(*cur)]; newValue > 9) [[unlikely]] {
			goto digi_finish;
		}
		while (*cur == 0x30u)
			++cur;
		tmp = cur;
		uint8_t c{};
		while (c < 10) {
			c = static_cast<uint8_t>(asciiToValueTable[static_cast<uint64_t>(*cur)]);
			++cur;
			expLit = c + int64_t(expLit) * 10;
		}
		if (cur - tmp >= 6) [[unlikely]] {
			if (sig == 0) {
				value = static_cast<value_type01>(sig);
				return true;
			} else {
				value = static_cast<value_type01>(std::numeric_limits<value_type01>::infinity());
				return true;
			}
		}
		expSig += expLit;
	}

	digi_exp_finish:
		if (sig == 0) {
			value = 0;
			return true;
		}
		if (expSig < -20) {
			value = static_cast<value_type01>(0);
			return true;
		} else if (expSig > 20) {
			value = static_cast<value_type01>(std::numeric_limits<value_type01>::infinity());
			return true;
		}
		exp = expSig;
	digi_finish:

		value = static_cast<value_type01>(sig);
		if (exp >= 0 && exp < 20) {
			value *= static_cast<value_type01>(powersOfTenInt[exp]);
		} else if (exp > -20 && exp < 0) {
			value /= static_cast<value_type01>(powersOfTenInt[-exp]);
		}
		return true;
	}

	template<class value_type> JSONIFIER_INLINE constexpr bool stoui64(value_type& res, const char*& c) noexcept {
		if (!isDigit(*c)) [[unlikely]] {
			return false;
		}

		static constexpr std::array<uint32_t, 4> max_digits_from_size = { 4, 6, 11, 20 };
		static constexpr auto N										  = max_digits_from_size[std::bit_width(sizeof(value_type)) - 1];

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

	template<class value_type> JSONIFIER_INLINE constexpr bool stoui64(value_type& res, auto& it) noexcept {
		static_assert(sizeof(*it) == sizeof(char));
		const char* cur = reinterpret_cast<const char*>(&*it);
		const char* beg = cur;
		if (stoui64(res, cur)) {
			it += (cur - beg);
			return true;
		}
		return false;
	}
}