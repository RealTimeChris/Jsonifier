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

	template<jsonifier::concepts::signed_t value_type01, typename char_type> inline bool parseNumber(value_type01& value, char_type* cur) {
		[[maybe_unused]] const char_type* sigEnd{};
		const char_type *tmp{}, *sigCut{}, *dotPos{}, *hdr{ cur };
		uint64_t fracZeros{}, numTmp{}, sig{};
		int64_t exp{}, expSig{}, expLit{};
		bool sign{ (*hdr == '-') }, expSign{};
		auto applySign = [&](auto&& value) -> value_type01 {
			return sign ? -static_cast<value_type01>(value) : static_cast<value_type01>(value);
		};
		cur += sign;
		sig = static_cast<uint64_t>(asciiToValueTable[static_cast<uint64_t>(*cur)]);
		if (sig > 9) {
			if (*cur == 'n' && cur[1] == 0x75u && cur[2] == 'l' && cur[3] == 'l') {
				value = applySign(0);
				return true;
			} else if ((*cur | eBit) == 'n' && (cur[1] | eBit) == 'a' && (cur[2] | eBit) == 'n') {
				value = applySign(std::numeric_limits<value_type01>::quiet_NaN());
				return true;
			} else {
				return false;
			}
		}
		static constexpr auto zero = static_cast<uint8_t>('0');
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
	if (cur[x] == '.') [[likely]] { \
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
	digi_frac_##x : if (numTmp = static_cast<uint64_t>(asciiToValueTable[cur[x + 1 + fracZeros]]); numTmp <= 9) [[likely]] sig = numTmp + sig * 10; \
	else { \
		goto digi_stop_##x; \
	}
			repeat_in_1_18(expr_frac)
#undef expr_frac
				cur += 20ull + fracZeros;
		if (asciiToValueTable[static_cast<uint64_t>(*cur)] > 9)
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
		if ((eBit | *cur) == 'e') {
			dotPos = cur;
			goto digi_exp_more;
		}
		if (*cur == '.') {
			dotPos = cur++;
			if (asciiToValueTable[static_cast<uint64_t>(*cur)] > 9) {
				return false;
			}
		}
	digi_frac_more:
		sigCut = cur;
		sig += (*cur >= '5');
		while (asciiToValueTable[*++cur] < 10) {
		}
		if (!dotPos) {
			dotPos = cur;
			if (*cur == '.') {
				if (asciiToValueTable[*++cur] > 9) {
					return false;
				}
				while (asciiToValueTable[*++cur] < 10) {
				}
			}
		}
		expSig = static_cast<int64_t>(dotPos - sigCut);
		expSig += (dotPos < sigCut);
		tmp = cur - 1;
		while (*tmp == '0' || *tmp == '.')
			tmp--;
		if (tmp < sigCut) {
			sigCut = nullptr;
		}
		if ((eBit | *cur) == 'e')
			goto digi_exp_more;
		goto digi_exp_finish;
	digi_frac_end:
		expSig = -int64_t((cur - dotPos) - 1);
		if ((eBit | *cur) != 'e') [[likely]] {
			if (expSig < f64MinDecExp - 19) [[unlikely]] {
				value = applySign(0);
				return true;
			}
			exp = expSig;
			goto digi_finish;
		} else {
			goto digi_exp_more;
		}
	digi_exp_more : {
		expSign = (*++cur == '-');
		cur += (*cur == '+' || *cur == '-');
		if (asciiToValueTable[static_cast<uint64_t>(*cur)] > 9) [[unlikely]] {
			goto digi_finish;
		}
		while (*cur == '0')
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

	template<jsonifier::concepts::unsigned_t value_type01, typename char_type> inline bool parseNumber(value_type01& value, char_type* cur) {
		[[maybe_unused]] const char_type* sigEnd{};
		const char_type *tmp{}, *sigCut{}, *dotPos{};
		uint64_t fracZeros{}, numTmp{}, sig{};
		int64_t exp{}, expSig{}, expLit{};
		sig = static_cast<uint64_t>(asciiToValueTable[static_cast<uint64_t>(*cur)]);
		if (sig > 9) {
			if (*cur == 'n' && cur[1] == 0x75u && cur[2] == 'l' && cur[3] == 'l') {
				value = static_cast<value_type01>(0);
				return true;
			} else if ((*cur | eBit) == 'n' && (cur[1] | eBit) == 'a' && (cur[2] | eBit) == 'n') {
				value = static_cast<value_type01>(std::numeric_limits<value_type01>::quiet_NaN());
				return true;
			} else {
				return false;
			}
		}
		static constexpr auto zero = static_cast<uint8_t>('0');
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
	if (cur[x] == '.') [[likely]] { \
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
	digi_frac_##x : if (numTmp = static_cast<uint64_t>(asciiToValueTable[cur[x + 1 + fracZeros]]); numTmp <= 9) [[likely]] sig = numTmp + sig * 10; \
	else { \
		goto digi_stop_##x; \
	}
			repeat_in_1_18(expr_frac)
#undef expr_frac
				cur += 20ull + fracZeros;
		if (asciiToValueTable[static_cast<uint64_t>(*cur)] > 9)
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
		if ((eBit | *cur) == 'e') {
			dotPos = cur;
			goto digi_exp_more;
		}
		if (*cur == '.') {
			dotPos = cur++;
			if (asciiToValueTable[static_cast<uint64_t>(*cur)] > 9) {
				return false;
			}
		}
	digi_frac_more:
		sigCut = cur;
		sig += (*cur >= '5');
		while (asciiToValueTable[*++cur] < 10) {
		}
		if (!dotPos) {
			dotPos = cur;
			if (*cur == '.') {
				if (asciiToValueTable[*++cur] > 9) {
					return false;
				}
				while (asciiToValueTable[*++cur] < 10) {
				}
			}
		}
		expSig = static_cast<int64_t>(dotPos - sigCut);
		expSig += (dotPos < sigCut);
		tmp = cur - 1;
		while (*tmp == '0' || *tmp == '.')
			tmp--;
		if (tmp < sigCut) {
			sigCut = nullptr;
		}
		if ((eBit | *cur) == 'e')
			goto digi_exp_more;
		goto digi_exp_finish;
	digi_frac_end:
		expSig = -int64_t((cur - dotPos) - 1);
		if ((eBit | *cur) != 'e') [[likely]] {
			if (expSig < f64MinDecExp - 19) [[unlikely]] {
				value = static_cast<value_type01>(0);
				return true;
			}
			exp = expSig;
			goto digi_finish;
		} else {
			goto digi_exp_more;
		}
	digi_exp_more : {
		cur += (*cur == '+' || *cur == '-');
		if (asciiToValueTable[static_cast<uint64_t>(*cur)] > 9) [[unlikely]] {
			goto digi_finish;
		}
		while (*cur == '0')
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
}