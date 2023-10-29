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
/// https://github.com/RealTimeChris/jsonifier
/// Feb 3, 2023
/// Most of the code in this header was sampled from Glaze library - https://github.com/stephenberry/glaze
#pragma once

#include <jsonifier/Allocator.hpp>
#include <jsonifier/Tables.hpp>

#include <concepts>
#include <cstdint>
#include <cstring>
#include <array>

#if defined(_M_X64) || defined(_M_ARM64)
	#if !defined(__linux__)
		#include <intrin.h>
	#else
		#include <x86intrin.h>
	#endif
#endif

namespace jsonifier_internal {

	// Based on yyjson: https://github.com/ibireme/yyjson/blob/master/src/yyjson.c with some changes to rounding and
	// dirrect for floats

	// https://stackoverflow.com/questions/28868367/getting-the-high-part-of-64-bit-integer-multiplication
#if defined(__SIZEOF_INT128__)
	jsonifier_inline static uint64_t mulhi64(uint64_t a, uint64_t b) {
	#if defined(__GNUC__) || defined(__GNUG__)
		#pragma GCC diagnostic push
		#pragma GCC diagnostic ignored "-Wpedantic"
		unsigned __int128 prod = a * static_cast<unsigned __int128>(b);
		#pragma GCC diagnostic pop
	#endif
		return prod >> 64;
	}
#elif defined(_M_X64) || defined(_M_ARM64)
	#define mulhi64 __umulh
#else
	jsonifier_inline static uint64_t mulhi64(uint64_t a, uint64_t b) {
		uint64_t aLo	  = ( uint32_t )a;
		uint64_t aHi	  = a >> 32;
		uint64_t bLo	  = ( uint32_t )b;
		uint64_t bHi	  = b >> 32;
		uint64_t axbHi	  = aHi * bHi;
		uint64_t axbMid	  = aHi * bLo;
		uint64_t bxaMid	  = bHi * aLo;
		uint64_t axbLo	  = aLo * bLo;
		uint64_t carryBit = (( uint64_t )( uint32_t )axbMid + ( uint64_t )( uint32_t )bxaMid + (axbLo >> 32)) >> 32;
		uint64_t multhi	  = axbHi + (axbMid >> 32) + (bxaMid >> 32) + carryBit;
		return multhi;
	}
#endif

	// Source: https://github.com/ibireme/yyjson/blob/master/src/yyjson.c
	static jsonifier_constexpr auto pow10SigTable128MinExp		= -343;
	static jsonifier_constexpr auto pow10SigTable128MaxExp		= 324;
	static jsonifier_constexpr auto pow10SigTable128MinExactExp = 0;
	static jsonifier_constexpr auto pow10SigTable128MaxExactExp = 55;

	jsonifier_inline static void pow10TableGetSig128(int32_t exp10, uint64_t* hi, uint64_t* lo) {
		int32_t idx = exp10 - (pow10SigTable128MinExp);
		*hi			= pow10SigTable128[idx * 2ULL];
		*lo			= pow10SigTable128[idx * 2ULL + 1ULL];
	}

	jsonifier_inline static uint64_t sig2FromExp10(int32_t exp10) {
		return pow10SigTable128[static_cast<uint64_t>(exp10) - pow10SigTable128MinExp];
	}

	jsonifier_inline static int32_t exp2FromExp10(int32_t exp10) {
		return (((exp10 * 217706 - 4128768) >> 16) + 126);
	}

	template<typename value_type> static jsonifier_constexpr value_type digiTypeZero	= 1 << 0;
	template<typename value_type> static jsonifier_constexpr value_type digiTypeNonZero = 1 << 1;
	template<typename value_type> static jsonifier_constexpr value_type digiTypePos		= 1 << 2;
	template<typename value_type> static jsonifier_constexpr value_type digiTypeNeg		= 1 << 3;
	template<typename value_type> static jsonifier_constexpr value_type digiTypeDot		= 1 << 4;
	template<typename value_type> static jsonifier_constexpr value_type digiTypeExp		= 1 << 5;

	template<typename value_type> jsonifier_inline static bool digiIsType(value_type d, value_type type) {
		return (digiTable<value_type>[d] & type) != 0;
	}

	template<typename value_type> jsonifier_inline static bool digiIsFp(value_type d) {
		return digiIsType<value_type>(d, uint8_t(digiTypeDot<value_type> | digiTypeExp<value_type>));
	}
	template<typename value_type> jsonifier_inline static bool digiIsDigitOrFp(value_type d) {
		return digiIsType<value_type>(d, uint8_t(digiTypeZero<value_type> | digiTypeNonZero<value_type> | digiTypeDot<value_type> | digiTypeExp<value_type>));
	}

#define repeatIIn_1_18(x) \
	{ x(1) x(2) x(3) x(4) x(5) x(6) x(7) x(8) x(9) x(10) x(11) x(12) x(13) x(14) x(15) x(16) x(17) x(18) }

	static jsonifier_constexpr auto eBit		 = static_cast<uint8_t>('E' ^ 'e');
	static jsonifier_constexpr auto f64MaxDecExp = 308;
	static jsonifier_constexpr auto f64MinDecExp = (-324);

	jsonifier_inline static consteval uint32_t ceillog2(uint32_t x) {
		return x < 2 ? x : 1 + ceillog2(x >> 1);
	}

	struct big_int_t {
		jsonifier::vector<uint32_t> data = {};

		jsonifier_inline big_int_t(uint64_t num) {
			uint32_t lowerWord = uint32_t(num);
			uint32_t upperWord = uint32_t(num >> 32);
			if (upperWord > 0) {
				data = { lowerWord, upperWord };
			} else {
				data = { lowerWord };
			}
		}

		jsonifier_inline void mulU32(uint32_t num) {
			uint32_t carry = 0;
			for (uint64_t x = 0; x < data.size(); x++) {
				uint64_t res	   = uint64_t(data[x]) * uint64_t(num) + uint64_t(carry);
				uint32_t lowerWord = uint32_t(res);
				uint32_t upperWord = uint32_t(res >> 32);
				data[x]			   = lowerWord;
				carry			   = upperWord;
			}
			if (carry != 0) {
				data.emplace_back(carry);
			}
		}

		jsonifier_inline void mulPow10(uint32_t pow10) {
			for (; pow10 >= 9; pow10 -= 9) {
				mulU32(static_cast<uint32_t>(powersOfTenInt[9]));
			}
			if (pow10) {
				mulU32(static_cast<uint32_t>(powersOfTenInt[pow10]));
			}
		}

		jsonifier_inline void mulPow2(uint32_t exp) {
			uint32_t shft = exp % 32;
			uint32_t move = exp / 32;
			uint32_t idx  = static_cast<uint32_t>(data.size()) - 1;
			if (shft == 0) {
				data.resize(data.size() + move);
				for (; idx > 0; idx--) {
					data[static_cast<uint64_t>(idx) + static_cast<uint64_t>(move) - 1ULL] = data[static_cast<uint64_t>(idx) - 1ULL];
				}
				while (move)
					data[--move] = 0;
			} else {
				data.resize(data.size() + move + 1);
				++idx;
				for (; idx > 0; idx--) {
					uint32_t num = data[idx] << shft;
					num |= data[idx - 1ULL] >> (32 - shft);
					data[static_cast<uint64_t>(idx) + static_cast<uint64_t>(move)] = num;
				}
				data[move] = data[0] << shft;
				if (data.back() == 0)
					data.resize(data.size() - 1);
				while (move)
					data[--move] = 0;
			}
		}

		jsonifier_inline auto operator<=>(const big_int_t& rhs) const {
			if (data.size() < rhs.data.size())
				return -1;
			if (data.size() > rhs.data.size())
				return 1;
			for (auto x = data.size() - 1; x > 0; --x) {
				;
				if (data[x] < rhs.data[x])
					return -1;
				if (data[x] > rhs.data[x])
					return 1;
			}
			return 0;
		}
	};

	// Source: https://github.com/ibireme/yyjson/blob/master/src/yyjson.c

	jsonifier_inline static void u128Mul(uint64_t a, uint64_t b, uint64_t* hi, uint64_t* lo) {
#if defined(__SIZEOF_INT128__)
	#if defined(__GNUC__) || defined(__GNUG__)
		#pragma GCC diagnostic push
		#pragma GCC diagnostic ignored "-Wpedantic"
	#endif
		unsigned __int128 m = static_cast<unsigned __int128>(a) * b;
	#if defined(__GNUC__) || defined(__GNUG__)
		#pragma GCC diagnostic pop
	#endif
		*hi = uint64_t(m >> 64);
		*lo = uint64_t(m);
#elif defined(_M_X64)
		*lo = _umul128(a, b, hi);
#elif defined(_M_ARM64)
		*hi = __umulh(a, b);
		*lo = a * b;
#else
		uint32_t a0 = ( uint32_t )(a), array01 = ( uint32_t )(a >> 32);
		uint32_t b0 = ( uint32_t )(b), b1 = ( uint32_t )(b >> 32);
		uint64_t p00 = ( uint64_t )a0 * b0, p01 = ( uint64_t )a0 * b1;
		uint64_t p10 = ( uint64_t )array01 * b0, p11 = ( uint64_t )array01 * b1;
		uint64_t m0	 = p01 + (p00 >> 32);
		uint32_t m00 = ( uint32_t )(m0), m01 = ( uint32_t )(m0 >> 32);
		uint64_t m1	 = p10 + m00;
		uint32_t m10 = ( uint32_t )(m1), m11 = ( uint32_t )(m1 >> 32);
		*hi = p11 + m01 + m11;
		*lo = (( uint64_t )m10 << 32) | ( uint32_t )p00;
#endif
	}

	jsonifier_inline static void u128MulAdd(uint64_t a, uint64_t b, uint64_t c, uint64_t* hi, uint64_t* lo) {
#if defined(__SIZEOF_INT128__)
	#if defined(__GNUC__) || defined(__GNUG__)
		#pragma GCC diagnostic push
		#pragma GCC diagnostic ignored "-Wpedantic"
	#endif
		unsigned __int128 m = static_cast<unsigned __int128>(a) * b + c;
	#if defined(__GNUC__) || defined(__GNUG__)
		#pragma GCC diagnostic pop
	#endif
		*hi = uint64_t(m >> 64);
		*lo = uint64_t(m);
#else
		uint64_t h, l, t;
		u128Mul(a, b, &h, &l);
		t = l + c;
		h += ((t < l) | (t < c));
		*hi = h;
		*lo = t;
#endif
	}

	jsonifier_inline static uint64_t roundToOdd(uint64_t hi, uint64_t lo, uint64_t codePoint) {
		uint64_t xHi{}, xLo{}, yHi{}, yLo{};
		u128Mul(codePoint, lo, &xHi, &xLo);
		u128MulAdd(codePoint, hi, xHi, &yHi, &yLo);
		return yHi | (yLo > 1);
	}

	template<jsonifier::concepts::unsigned_t value_type, typename value_type02> jsonifier_inline static bool parseNumber(value_type& value, value_type02* cur) {
		[[maybe_unused]] value_type02* sigEnd{};
		value_type02* sigCut{};
		value_type02* dotPos{};
		uint32_t fracZeroes{};
		uint64_t sig{};
		int32_t exp{};
		bool expSign{};
		int32_t expSig{};
		int32_t expLit{};
		uint64_t numTmp{};
		value_type02* tmp{};
		value_type02* hdr = cur;
		bool sign{};
		sign = (*hdr == '-');
		cur += sign;
		auto applySign = [&](auto&& value) -> value_type {
			return static_cast<value_type>(value);
		};
		sig = static_cast<uint64_t>(*cur) - static_cast<uint64_t>('0');
		if (sig > 9) {
			return false;
		}
		static jsonifier_constexpr auto zero = static_cast<uint8_t>('0');
#define expr_intg(x) \
	if ((numTmp = static_cast<uint64_t>(cur[x] - static_cast<uint64_t>(zero))) <= 9) [[likely]] \
		sig = static_cast<uint64_t>(numTmp) + sig * 10ULL; \
	else { \
		goto digi_sepr_##x; \
	}
		repeatIIn_1_18(expr_intg);
#undef expr_intg
		cur += 19;
		if (!digiIsDigitOrFp(*cur)) {
			value = static_cast<value_type>(sig);
			return true;
		}
		goto digi_intg_more;
#define expr_sepr(x) \
	digi_sepr_##x : if ((!digiIsFp(cur[x]))) [[likely]] { \
		cur += x; \
		value = static_cast<value_type>(applySign(sig)); \
		return true; \
	} \
	dotPos = cur + x; \
	if ((cur[x] == '.')) [[likely]] { \
		if (sig == 0) \
			while (cur[fracZeroes + x + 1] == static_cast<uint64_t>(zero)) \
				++fracZeroes; \
		goto digi_frac_##x; \
	} \
	cur += x; \
	sigEnd = cur; \
	goto digi_exp_more;
		repeatIIn_1_18(expr_sepr)
#undef expr_sepr
#define expr_frac(x) \
	digi_frac_##x : if (((numTmp = static_cast<uint64_t>(cur[x + 1 + fracZeroes] - static_cast<uint64_t>(zero))) <= 9)) [[likely]] sig = numTmp + sig * 10; \
	else { \
		goto digi_stop_##x; \
	}
			repeatIIn_1_18(expr_frac)
#undef expr_frac
				cur += 20ULL + fracZeroes;
		if (uint8_t(*cur - zero) > 9)
			goto digi_frac_end;
		goto digi_frac_more;
#define expr_stop(x) \
	digi_stop_##x : cur += x##ULL + 1ULL + fracZeroes; \
	goto digi_frac_end;
		repeatIIn_1_18(expr_stop)
#undef expr_stop
			digi_intg_more : static jsonifier_constexpr uint64_t u64Max = std::numeric_limits<uint64_t>::max();
		if ((numTmp = static_cast<uint64_t>(*cur - static_cast<uint64_t>(zero))) < 10) {
			if (!digiIsDigitOrFp(cur[1])) {
				if ((sig < (u64Max / 10)) || (sig == (u64Max / 10) && numTmp <= (u64Max % 10))) {
					sig = numTmp + sig * 10;
					++cur;
					value = static_cast<value_type>(sig);
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
			if (uint8_t(*cur - static_cast<uint64_t>(zero)) > 9) {
				return false;
			}
		}
	digi_frac_more:
		sigCut = cur;
		sig += (*cur >= '5');
		while (uint8_t(*++cur - static_cast<uint64_t>(zero)) < 10) {
		}
		if (!dotPos) {
			dotPos = cur;
			if (*cur == '.') {
				if (uint8_t(*++cur - static_cast<uint64_t>(zero)) > 9) {
					return false;
				}
				while (uint8_t(*++cur - static_cast<uint64_t>(zero)) < 10) {
				}
			}
		}
		expSig = static_cast<int32_t>(dotPos - sigCut);
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
		expSig = -int32_t((cur - dotPos) - 1);
		if ((eBit | *cur) != 'e') [[likely]] {
			if ((expSig < f64MinDecExp - 19)) [[unlikely]] {
				value = static_cast<value_type>(applySign(0));
				return true;
			}
			exp = expSig;
			goto digi_finish;
		} else {
			goto digi_exp_more;
		}
	digi_exp_more:
		expSign = (*++cur == '-');
		cur += (*cur == '+' || *cur == '-');
		if (uint8_t(*cur - zero) > 9) [[unlikely]] {
			goto digi_finish;
		}
		while (*cur == '0')
			++cur;
		tmp = cur;
		uint8_t c;
		while (uint8_t(c = static_cast<uint8_t>(*cur - zero)) < 10) {
			++cur;
			expLit = c + int32_t(expLit) * 10;
		}
		if ((cur - tmp >= 6)) [[unlikely]] {
			if (sig == 0 || expSign) {
				value = static_cast<value_type>(applySign(0));
				value = static_cast<value_type>(sig);
				return true;
			} else {
				value = static_cast<value_type>(applySign(std::numeric_limits<value_type>::infinity()));
				return true;
			}
		}
		expSig += expSign ? -expLit : expLit;
	digi_exp_finish:
		if (sig == 0) {
			value = static_cast<value_type>((!sign) ? -0 : 0);
			return true;
		}
		if (expSig < -20) {
			value = static_cast<value_type>(applySign(0));
			return true;
		} else if (expSig > 20) {
			value = static_cast<value_type>(applySign(std::numeric_limits<value_type>::infinity()));
			return true;
		}
		exp = expSig;
	digi_finish:

		value = static_cast<value_type>(sig);
		if (exp >= 0 && exp < 20) {
			value *= static_cast<value_type>(powersOfTenInt[exp]);
		} else if (exp > -20 && exp < 0) {
			value /= static_cast<value_type>(powersOfTenInt[-exp]);
		}
		return true;
	}

	template<jsonifier::concepts::signed_t value_type, typename value_type02> jsonifier_inline static bool parseNumber(value_type& value, value_type02* cur) {
		[[maybe_unused]] value_type02* sigEnd{};
		value_type02* sigCut{};
		value_type02* dotPos{};
		uint32_t fracZeroes{};
		uint64_t sig{};
		int32_t exp{};
		bool expSign{};
		int32_t expSig{};
		int32_t expLit{};
		uint64_t numTmp{};
		value_type02* tmp{};
		value_type02* hdr = cur;
		bool sign{};
		sign = (*hdr == '-');
		cur += sign;
		auto applySign = [&](auto&& value) -> value_type {
			return sign ? -static_cast<value_type>(value) : static_cast<value_type>(value);
		};
		sig = static_cast<uint64_t>(*cur) - static_cast<uint64_t>('0');
		if (sig > 9) {
			if (*cur == 'n' && cur[1] == 'u' && cur[2] == 'l' && cur[3] == 'l') {
				cur += 4;
				value = static_cast<value_type>(std::numeric_limits<value_type>::quiet_NaN());
				return true;
			} else if ((*cur | eBit) == 'n' && (cur[1] | eBit) == 'a' && (cur[2] | eBit) == 'n') {
				cur += 3;
				value = static_cast<value_type>(sign ? -std::numeric_limits<value_type>::quiet_NaN() : std::numeric_limits<value_type>::quiet_NaN());
				return true;
			} else {
				return false;
			}
		}
		static jsonifier_constexpr auto zero = static_cast<uint8_t>('0');
#define expr_intg(x) \
	if ((numTmp = static_cast<uint64_t>(cur[x] - static_cast<uint64_t>(zero))) <= 9) [[likely]] \
		sig = static_cast<uint64_t>(numTmp) + sig * 10ULL; \
	else { \
		goto digi_sepr_##x; \
	}
		repeatIIn_1_18(expr_intg);
#undef expr_intg
		cur += 19;
		if (!digiIsDigitOrFp(*cur)) {
			value = static_cast<value_type>(sig);
			value *= sign ? -1 : 1;
			return true;
		}
		goto digi_intg_more;
#define expr_sepr(x) \
	digi_sepr_##x : if ((!digiIsFp(cur[x]))) [[likely]] { \
		cur += x; \
		value = static_cast<value_type>(applySign(sig)); \
		return true; \
	} \
	dotPos = cur + x; \
	if ((cur[x] == '.')) [[likely]] { \
		if (sig == 0) \
			while (cur[fracZeroes + x + 1] == zero) \
				++fracZeroes; \
		goto digi_frac_##x; \
	} \
	cur += x; \
	sigEnd = cur; \
	goto digi_exp_more;
		repeatIIn_1_18(expr_sepr)
#undef expr_sepr
#define expr_frac(x) \
	digi_frac_##x : if (((numTmp = static_cast<uint64_t>(cur[x + 1 + fracZeroes] - static_cast<uint64_t>(zero))) <= 9)) [[likely]] sig = numTmp + sig * 10; \
	else { \
		goto digi_stop_##x; \
	}
			repeatIIn_1_18(expr_frac)
#undef expr_frac
				cur += 20ULL + fracZeroes;
		if (uint8_t(*cur - static_cast<uint64_t>(zero)) > 9)
			goto digi_frac_end;
		goto digi_frac_more;
#define expr_stop(x) \
	digi_stop_##x : cur += x##ULL + 1ULL + fracZeroes; \
	goto digi_frac_end;
		repeatIIn_1_18(expr_stop)
#undef expr_stop
			digi_intg_more : static jsonifier_constexpr uint64_t u64Max = std::numeric_limits<uint64_t>::max();
		if ((numTmp = static_cast<uint64_t>(*cur - static_cast<uint64_t>(zero))) < 10) {
			if (!digiIsDigitOrFp(cur[1])) {
				if ((sig < (u64Max / 10)) || (sig == (u64Max / 10) && numTmp <= (u64Max % 10))) {
					sig = numTmp + sig * 10;
					++cur;
					value = static_cast<value_type>(sig);
					value *= sign ? -1 : 1;
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
			if (uint8_t(*cur - zero) > 9) {
				return false;
			}
		}
	digi_frac_more:
		sigCut = cur;
		sig += (*cur >= '5');
		while (uint8_t(*++cur - zero) < 10) {
		}
		if (!dotPos) {
			dotPos = cur;
			if (*cur == '.') {
				if (uint8_t(*++cur - zero) > 9) {
					return false;
				}
				while (uint8_t(*++cur - zero) < 10) {
				}
			}
		}
		expSig = static_cast<int32_t>(dotPos - sigCut);
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
		expSig = -int32_t((cur - dotPos) - 1);
		if ((eBit | *cur) != 'e') [[likely]] {
			if ((expSig < f64MinDecExp - 19)) [[unlikely]] {
				value = static_cast<value_type>(applySign(0));
				return true;
			}
			exp = expSig;
			goto digi_finish;
		} else {
			goto digi_exp_more;
		}
	digi_exp_more:
		expSign = (*++cur == '-');
		cur += (*cur == '+' || *cur == '-');
		if (uint8_t(*cur - zero) > 9) [[unlikely]] {
			goto digi_finish;
		}
		while (*cur == '0')
			++cur;
		tmp = cur;
		uint8_t c;
		while (uint8_t(c = static_cast<uint8_t>(*cur - zero)) < 10) {
			++cur;
			expLit = c + int32_t(expLit) * 10;
		}
		if ((cur - tmp >= 6)) [[unlikely]] {
			if (sig == 0 || expSign) {
				value = static_cast<value_type>(applySign(0));
				value = static_cast<value_type>(sig);
				return true;
			} else {
				value = static_cast<value_type>(applySign(std::numeric_limits<value_type>::infinity()));
				return true;
			}
		}
		expSig += expSign ? -expLit : expLit;
	digi_exp_finish:
		if (sig == 0) {
			value = static_cast<value_type>((sign) ? -0 : 0);
			return true;
		}
		if (expSig < -20) {
			value = static_cast<value_type>(applySign(0));
			return true;
		} else if (expSig > 20) {
			value = static_cast<value_type>(applySign(std::numeric_limits<value_type>::infinity()));
			return true;
		}
		exp = expSig;
	digi_finish:

		value = static_cast<value_type>(sig);
		value *= sign ? -1 : 1;
		if (exp >= 0 && exp < 20) {
			value *= static_cast<value_type>(powersOfTenInt[exp]);
		} else if (exp > -20 && exp < 0) {
			value /= static_cast<value_type>(powersOfTenInt[-exp]);
		}
		return true;
	}

	template<jsonifier::concepts::float_t value_type, typename value_type02> jsonifier_inline static bool parseNumber(value_type& value, value_type02* cur) {
		[[maybe_unused]] value_type02* sigEnd{};
		value_type02* sigCut{};
		value_type02* dotPos{};
		uint32_t fracZeroes{};
		uint64_t sig{};
		int32_t exp{};
		bool expSign{};
		int32_t expSig{};
		int32_t expLit{};
		uint64_t numTmp{};
		value_type02* tmp{};
		value_type02* hdr = cur;
		bool sign{};
		sign = (*hdr == '-');
		cur += sign;
		auto applySign = [&](auto&& value) -> value_type {
			if jsonifier_constexpr (std::is_unsigned_v<value_type>) {
				return static_cast<value_type>(value);
			} else {
				return sign ? -static_cast<value_type>(value) : static_cast<value_type>(value);
			}
		};
		sig = static_cast<uint64_t>(*cur) - '0';
		if (sig > 9) {
			if (*cur == 'n' && cur[1] == 'u' && cur[2] == 'l' && cur[3] == 'l') {
				cur += 4;
				value = static_cast<value_type>(std::numeric_limits<value_type>::quiet_NaN());
				return true;
			} else if ((*cur | eBit) == 'n' && (cur[1] | eBit) == 'a' && (cur[2] | eBit) == 'n') {
				cur += 3;
				value = static_cast<value_type>(sign ? -std::numeric_limits<value_type>::quiet_NaN() : std::numeric_limits<value_type>::quiet_NaN());
				return true;
			} else {
				return false;
			}
		}
		static jsonifier_constexpr auto zero = static_cast<uint8_t>('0');
#define expr_intg(x) \
	if ((numTmp = static_cast<uint64_t>(cur[x] - static_cast<uint64_t>(zero))) <= 9) [[likely]] \
		sig = static_cast<uint64_t>(numTmp) + sig * 10ULL; \
	else { \
		goto digi_sepr_##x; \
	}
		repeatIIn_1_18(expr_intg);
#undef expr_intg
		cur += 19;
		if (!digiIsDigitOrFp(*cur)) {
			value = static_cast<value_type>(sig);
			if jsonifier_constexpr (!std::is_unsigned_v<value_type>) {
				value *= sign ? -1 : 1;
			}
			return true;
		}
		goto digi_intg_more;
#define expr_sepr(x) \
	digi_sepr_##x : if ((!digiIsFp(cur[x]))) [[likely]] { \
		cur += x; \
		value = static_cast<value_type>(applySign(sig)); \
		return true; \
	} \
	dotPos = cur + x; \
	if ((cur[x] == '.')) [[likely]] { \
		if (sig == 0) \
			while (cur[fracZeroes + x + 1] == zero) \
				++fracZeroes; \
		goto digi_frac_##x; \
	} \
	cur += x; \
	sigEnd = cur; \
	goto digi_exp_more;
		repeatIIn_1_18(expr_sepr)
#undef expr_sepr
#define expr_frac(x) \
	digi_frac_##x : if (((numTmp = static_cast<uint64_t>(cur[x + 1 + fracZeroes] - static_cast<uint64_t>(zero))) <= 9)) [[likely]] sig = numTmp + sig * 10; \
	else { \
		goto digi_stop_##x; \
	}
			repeatIIn_1_18(expr_frac)
#undef expr_frac
				cur += 20ULL + fracZeroes;
		if (uint8_t(*cur - zero) > 9)
			goto digi_frac_end;
		goto digi_frac_more;
#define expr_stop(x) \
	digi_stop_##x : cur += x##ULL + 1ULL + fracZeroes; \
	goto digi_frac_end;
		repeatIIn_1_18(expr_stop)
#undef expr_stop
			digi_intg_more : static jsonifier_constexpr uint64_t u64Max = std::numeric_limits<uint64_t>::max();
		if ((numTmp = static_cast<uint64_t>(*cur - static_cast<uint64_t>(zero))) < 10) {
			if (!digiIsDigitOrFp(cur[1])) {
				if ((sig < (u64Max / 10)) || (sig == (u64Max / 10) && numTmp <= (u64Max % 10))) {
					sig = numTmp + sig * 10;
					++cur;
					value = static_cast<value_type>(sig);
					if jsonifier_constexpr (!std::is_unsigned_v<value_type>) {
						value *= sign ? -1 : 1;
					}
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
			if (uint8_t(*cur - zero) > 9) {
				return false;
			}
		}
	digi_frac_more:
		sigCut = cur;
		sig += (*cur >= '5');
		while (uint8_t(*++cur - zero) < 10) {
		}
		if (!dotPos) {
			dotPos = cur;
			if (*cur == '.') {
				if (uint8_t(*++cur - zero) > 9) {
					return false;
				}
				while (uint8_t(*++cur - zero) < 10) {
				}
			}
		}
		expSig = static_cast<int32_t>(dotPos - sigCut);
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
		expSig = -int32_t((cur - dotPos) - 1);
		if ((eBit | *cur) != 'e') [[likely]] {
			if ((expSig < f64MinDecExp - 19)) [[unlikely]] {
				value = static_cast<value_type>(applySign(0));
				return true;
			}
			exp = expSig;
			goto digi_finish;
		} else {
			goto digi_exp_more;
		}
	digi_exp_more:
		expSign = (*++cur == '-');
		cur += (*cur == '+' || *cur == '-');
		if (uint8_t(*cur - zero) > 9) [[unlikely]] {
			goto digi_finish;
		}
		while (*cur == '0')
			++cur;
		tmp = cur;
		uint8_t c;
		while (uint8_t(c = static_cast<uint8_t>(*cur - zero)) < 10) {
			++cur;
			expLit = c + int32_t(expLit) * 10;
		}
		if ((cur - tmp >= 6)) [[unlikely]] {
			if (sig == 0 || expSign) {
				value = static_cast<value_type>(applySign(0));
				value = static_cast<value_type>(sig);
				return true;
			} else {
				value = static_cast<value_type>(applySign(std::numeric_limits<value_type>::infinity()));
				return true;
			}
		}
		expSig += expSign ? -expLit : expLit;
	digi_exp_finish:
		if (sig == 0) {
			value = static_cast<value_type>(sign ? -value_type{ 0 } : value_type{ 0 });
			return true;
		}
		if ((expSig < f64MinDecExp - 19)) [[unlikely]] {
			value = static_cast<value_type>(sign ? -value_type{ 0 } : value_type{ 0 });
			return true;
		} else if ((expSig > f64MaxDecExp)) [[unlikely]] {
			value = static_cast<value_type>(sign ? -std::numeric_limits<value_type>::infinity() : std::numeric_limits<value_type>::infinity());
			return true;
		}
		exp = expSig;
	digi_finish:

		if jsonifier_constexpr (std::floating_point<value_type>) {
			if (sig < (uint64_t(1) << 53) && std::abs(exp) <= 22) {
				value = static_cast<value_type>(sig);
				if jsonifier_constexpr (!std::is_unsigned_v<value_type>) {
					value *= sign ? -1 : 1;
				}
				if (exp >= 0 && exp < 23) {
					value *= static_cast<value_type>(powersOfTenFloat[exp]);
				} else if (exp > -23 && exp < 0) {
					value /= static_cast<value_type>(powersOfTenFloat[-exp]);
				}
				return true;
			}
		} else {
			if (sig < (uint64_t(1) << 24) && std::abs(exp) <= 8) {
				value = static_cast<value_type>(sig);
				if jsonifier_constexpr (!std::is_unsigned_v<value_type>) {
					value *= sign ? -1 : 1;
				}
				if (exp >= 0 && exp < 23) {
					value *= static_cast<value_type>(powersOfTenFloat[exp]);
				} else if (exp > -23 && exp < 0) {
					value /= static_cast<value_type>(powersOfTenFloat[-exp]);
				}
				return true;
			}
		}

		static_assert(std::numeric_limits<value_type>::is_iec559);
		static_assert(std::numeric_limits<value_type>::radix == 2);
		static_assert(std::same_as<float, jsonifier::concepts::unwrap<value_type>> || std::same_as<double, jsonifier::concepts::unwrap<value_type>>);
		static_assert(sizeof(float) == 4 && sizeof(double) == 8);

		using raw_t										  = std::conditional_t<std::same_as<float, jsonifier::concepts::unwrap<value_type>>, uint32_t, uint64_t>;
		const auto sigLeadingZeroes						  = std::countl_zero(sig);
		const auto sigNorm								  = sig << sigLeadingZeroes;
		const auto sig2Norm								  = sig2FromExp10(exp);
		const auto sigProduct							  = mulhi64(sigNorm, sig2Norm) + 1;
		const auto sigProductStartsWith1				  = sigProduct >> 63;
		auto mantisa									  = sigProduct << (2 - sigProductStartsWith1);
		static jsonifier_constexpr uint64_t roundMask	  = uint64_t(1) << 63 >> (std::numeric_limits<value_type>::digits - 1);
		static jsonifier_constexpr uint32_t exponentBits  = ceillog2(std::numeric_limits<value_type>::max_exponent - std::numeric_limits<value_type>::min_exponent + 1);
		static jsonifier_constexpr uint32_t mantissaShift = exponentBits + 1 + 64 - 8 * sizeof(raw_t);
		int32_t exp2									  = static_cast<int32_t>(exp2FromExp10(exp) + static_cast<uint32_t>(-sigLeadingZeroes + sigProductStartsWith1));

		if (exp2 < std::numeric_limits<value_type>::min_exponent - 1) [[unlikely]] {
			value = static_cast<value_type>(sign ? -value_type(0) : value_type(0));
			return true;
		} else if (exp2 > std::numeric_limits<value_type>::max_exponent - 1) [[unlikely]] {
			value = static_cast<value_type>(sign ? -std::numeric_limits<value_type>::infinity() : std::numeric_limits<value_type>::infinity());
			return true;
		}

		uint64_t round = 0;
		if (roundMask & mantisa) {
			if (mantisa << (std::numeric_limits<value_type>::digits) == 0) {
				auto sigUpper	  = (mantisa >> (mantissaShift - 1)) | (uint64_t(1) << 63 >> (mantissaShift - 2)) | 1;
				int32_t exp2Upper = exp2 - std::numeric_limits<value_type>::digits;

				big_int_t bigComp{ sigUpper };
				big_int_t bigFull{ sig };
				if (exp >= 0) {
					bigFull.mulPow10(static_cast<uint32_t>(exp));
				} else {
					bigComp.mulPow10(static_cast<uint32_t>(-exp));
				}
				if (exp2Upper >= 0) {
					bigComp.mulPow2(static_cast<uint32_t>(exp2Upper));
				} else {
					bigFull.mulPow2(static_cast<uint32_t>(-exp2Upper));
				}
				auto cmp = bigFull <=> bigComp;
				if (cmp != 0) [[likely]] {
					round = (cmp > 0);
				} else {
					round = (mantisa & (roundMask << 1)) != 0;
				}
			} else if ((exp < pow10SigTable128MinExactExp || exp > pow10SigTable128MaxExactExp) || (mantisa & (roundMask << 1)) ||
				(static_cast<uint64_t>(std::countr_zero(sigNorm)) + static_cast<uint64_t>(std::countr_zero(sig2Norm)) <
					128ULL - std::numeric_limits<value_type>::digits - (2ULL - sigProductStartsWith1))) {
				round = 1;
			}
		}

		auto num = raw_t(sign) << (sizeof(raw_t) * 8ULL - 1ULL) | raw_t(mantisa >> mantissaShift) |
			(raw_t(static_cast<uint64_t>(exp2) + std::numeric_limits<value_type>::max_exponent - 1ULL) << (std::numeric_limits<value_type>::digits - 1ULL));
		num += raw_t(round);
		std::memcpy(&value, &num, sizeof(value_type));
		return true;
	}

	template<jsonifier::concepts::unsigned_t value_type02, typename value_type> jsonifier_inline static value_type* toChars(value_type* buf, value_type02 value) {
		uint32_t aa, bb, cc, dd, ee, aabb, bbcc, ccdd, ddee, aabbcc;
		uint32_t lz;

		if (value < 100) {
			lz = value < 10;
			std::memcpy(&buf[0], &charTable<value_type>[value * 2ULL + lz], 2);
			buf -= lz;
			return buf + 2ULL;
		} else if (value < 10000) {
			aa = (value * 5243u) >> 19;
			bb = value - aa * 100;
			lz = aa < 10;
			std::memcpy(&buf[0], &charTable<value_type>[aa * 2ULL + lz], 2);
			buf -= lz;
			std::memcpy(&buf[2], &charTable<value_type>[2ULL * bb], 2);

			return buf + 4ULL;
		} else if (value < 1000000) {
			aa	 = uint32_t((uint64_t(value) * 429497) >> 32);
			bbcc = value - aa * 10000;
			bb	 = (bbcc * 5243) >> 19;
			cc	 = bbcc - bb * 100;
			lz	 = aa < 10;
			std::memcpy(buf, charTable<value_type> + aa * 2ULL + lz, 2);
			buf -= lz;
			std::memcpy(buf + 2ULL, charTable<value_type> + bb * 2ULL, 2);
			std::memcpy(buf + 4ULL, charTable<value_type> + cc * 2ULL, 2);
			return buf + 6ULL;
		} else if (value < 100000000) {
			aabb = uint32_t((uint64_t(value) * 109951163) >> 40);
			ccdd = value - aabb * 10000;
			aa	 = (aabb * 5243) >> 19;
			cc	 = (ccdd * 5243) >> 19;
			bb	 = aabb - aa * 100;
			dd	 = ccdd - cc * 100;
			lz	 = aa < 10;
			std::memcpy(buf, charTable<value_type> + aa * 2ULL + lz, 2);
			buf -= lz;
			std::memcpy(buf + 2ULL, charTable<value_type> + bb * 2ULL, 2);
			std::memcpy(buf + 4ULL, charTable<value_type> + cc * 2ULL, 2);
			std::memcpy(buf + 6ULL, charTable<value_type> + dd * 2ULL, 2);
			return buf + 8;
		} else {
			aabbcc = uint32_t((uint64_t(value) * 3518437209ul) >> 45);
			aa	   = uint32_t((uint64_t(aabbcc) * 429497) >> 32);
			ddee   = value - aabbcc * 10000;
			bbcc   = aabbcc - aa * 10000;
			bb	   = (bbcc * 5243) >> 19;
			dd	   = (ddee * 5243) >> 19;
			cc	   = bbcc - bb * 100;
			ee	   = ddee - dd * 100;
			lz	   = aa < 10;
			std::memcpy(buf, charTable<value_type> + aa * 2ULL + lz, 2);
			buf -= lz;
			std::memcpy(buf + 2ULL, charTable<value_type> + bb * 2ULL, 2);
			std::memcpy(buf + 4ULL, charTable<value_type> + cc * 2ULL, 2);
			std::memcpy(buf + 6ULL, charTable<value_type> + dd * 2ULL, 2);
			std::memcpy(buf + 8, charTable<value_type> + ee * 2ULL, 2);
			return buf + 10;
		}
	}

	template<jsonifier::concepts::signed_t value_type02, typename value_type> jsonifier_inline static value_type* toChars(value_type* buf, value_type02 value) {
		uint32_t neg  = uint32_t(-value);
		uint64_t sign = value < 0;
		*buf		  = '-';
		return toChars(buf + sign, sign ? uint32_t(neg) : uint32_t(value));
	}

	template<typename value_type> jsonifier_inline static value_type* toCharsU64Len8(value_type* buf, uint32_t value) {
		uint32_t aa, bb, cc, dd, aabb, ccdd;
		aabb = uint32_t((uint64_t(value) * 109951163) >> 40);
		ccdd = value - aabb * 10000;
		aa	 = (aabb * 5243) >> 19;
		cc	 = (ccdd * 5243) >> 19;
		bb	 = aabb - aa * 100;
		dd	 = ccdd - cc * 100;
		std::memcpy(buf, charTable<value_type> + aa * 2ULL, 2);
		std::memcpy(buf + 2ULL, charTable<value_type> + bb * 2ULL, 2);
		std::memcpy(buf + 4ULL, charTable<value_type> + cc * 2ULL, 2);
		std::memcpy(buf + 6ULL, charTable<value_type> + dd * 2ULL, 2);
		return buf + 8;
	}

	template<typename value_type> jsonifier_inline static value_type* toCharsU64Len4(value_type* buf, uint32_t value) {
		uint32_t aa, bb;
		aa = (value * 5243) >> 19;
		bb = value - aa * 100;
		std::memcpy(buf, charTable<value_type> + aa * 2ULL, 2);
		std::memcpy(buf + 2ULL, charTable<value_type> + bb * 2ULL, 2);
		return buf + 4ULL;
	}

	template<typename value_type> jsonifier_inline static value_type* toCharsU64Len18(value_type* buf, uint32_t value) {
		uint32_t aa, bb, cc, dd, aabb, bbcc, ccdd, lz;

		if (value < 100) {
			lz = value < 10;
			std::memcpy(buf, charTable<value_type> + value * 2ULL + lz, 2);
			buf -= lz;
			return buf + 2ULL;
		} else if (value < 10000) {
			aa = (value * 5243) >> 19;
			bb = value - aa * 100;
			lz = aa < 10;
			std::memcpy(buf, charTable<value_type> + aa * 2ULL + lz, 2);
			buf -= lz;
			std::memcpy(buf + 2ULL, charTable<value_type> + bb * 2ULL, 2);
			return buf + 4ULL;
		} else if (value < 1000000) {
			aa	 = uint32_t((uint64_t(value) * 429497) >> 32);
			bbcc = value - aa * 10000;
			bb	 = (bbcc * 5243) >> 19;
			cc	 = bbcc - bb * 100;
			lz	 = aa < 10;
			std::memcpy(buf, charTable<value_type> + aa * 2ULL + lz, 2);
			buf -= lz;
			std::memcpy(buf + 2ULL, charTable<value_type> + bb * 2ULL, 2);
			std::memcpy(buf + 4ULL, charTable<value_type> + cc * 2ULL, 2);
			return buf + 6ULL;
		} else {
			aabb = uint32_t((uint64_t(value) * 109951163) >> 40);
			ccdd = value - aabb * 10000;
			aa	 = (aabb * 5243) >> 19;
			cc	 = (ccdd * 5243) >> 19;
			bb	 = aabb - aa * 100;
			dd	 = ccdd - cc * 100;
			lz	 = aa < 10;
			std::memcpy(buf, charTable<value_type> + aa * 2ULL + lz, 2);
			buf -= lz;
			std::memcpy(buf + 2ULL, charTable<value_type> + bb * 2ULL, 2);
			std::memcpy(buf + 4ULL, charTable<value_type> + cc * 2ULL, 2);
			std::memcpy(buf + 6ULL, charTable<value_type> + dd * 2ULL, 2);
			return buf + 8;
		}
	}

	template<typename value_type> jsonifier_inline static value_type* toCharsU64Len58(value_type* buf, uint32_t value) {
		uint32_t aa, bb, cc, dd, aabb, bbcc, ccdd, lz;

		if (value < 1000000) {
			aa	 = uint32_t((uint64_t(value) * 429497) >> 32);
			bbcc = value - aa * 10000;
			bb	 = (bbcc * 5243) >> 19;
			cc	 = bbcc - bb * 100;
			lz	 = aa < 10;
			std::memcpy(buf, charTable<value_type> + aa * 2ULL + lz, 2);
			buf -= lz;
			std::memcpy(buf + 2ULL, charTable<value_type> + bb * 2ULL, 2);
			std::memcpy(buf + 4ULL, charTable<value_type> + cc * 2ULL, 2);
			return buf + 6ULL;
		} else {
			aabb = uint32_t((uint64_t(value) * 109951163) >> 40);
			ccdd = value - aabb * 10000;
			aa	 = (aabb * 5243) >> 19;
			cc	 = (ccdd * 5243) >> 19;
			bb	 = aabb - aa * 100;
			dd	 = ccdd - cc * 100;
			lz	 = aa < 10;
			std::memcpy(buf, charTable<value_type> + aa * 2ULL + lz, 2);
			buf -= lz;
			std::memcpy(buf + 2ULL, charTable<value_type> + bb * 2ULL, 2);
			std::memcpy(buf + 4ULL, charTable<value_type> + cc * 2ULL, 2);
			std::memcpy(buf + 6ULL, charTable<value_type> + dd * 2ULL, 2);
			return buf + 8;
		}
	}

	template<jsonifier::concepts::unsigned_int64_t value_type, typename char_type> jsonifier_inline static char_type* toChars(char_type* buf, value_type value) {
		uint64_t tmp, hgh;
		uint32_t mid, low;

		if (value < 100000000) {
			buf = toCharsU64Len18(buf, uint32_t(value));
			return buf;
		} else if (value < 100000000ULL * 100000000ULL) {
			hgh = value / 100000000;
			low = uint32_t(value - hgh * 100000000);
			buf = toCharsU64Len18(buf, uint32_t(hgh));
			buf = toCharsU64Len8(buf, low);
			return buf;
		} else {
			tmp = value / 100000000;
			low = uint32_t(value - tmp * 100000000);
			hgh = uint32_t(tmp / 10000);
			mid = uint32_t(tmp - hgh * 10000);
			buf = toCharsU64Len58(buf, uint32_t(hgh));
			buf = toCharsU64Len4(buf, mid);
			buf = toCharsU64Len8(buf, low);
			return buf;
		}
	}

	template<jsonifier::concepts::signed_int64_t value_type, typename char_type> jsonifier_inline static char_type* toChars(char_type* buf, value_type value) {
		uint64_t neg  = uint64_t(-value);
		uint64_t sign = value < 0;
		*buf		  = '-';
		return toChars(buf + sign, sign ? uint64_t(neg) : uint64_t(value));
	}

	/**
	This function use the Schubfach algorithm:
	Raffaello Giulietti, The Schubfach way to render doubles, 2020.
	https://drive.google.com/open?id=1luHhyQF9zKlM8yJ1nebU0OgVYhfC6CBN
	https://github.com/abolz/Drachennest

	See also:
	Dragonbox: A New Floating-Point Binary-to-Decimal Conversion Algorithm, 2020.
	https://github.com/jk-jeon/dragonbox/blob/master/other_files/Dragonbox.pdf
	https://github.com/jk-jeon/dragonbox
	*/
	jsonifier_inline static void f64BinToDec(uint64_t sigRaw, int32_t expRaw, uint64_t sigBin, int32_t expBin, uint64_t* sigDec, int32_t* expDec) {
		bool isEven, lowerBoundCloser, uInside, wInside, round_up;
		uint64_t s, sp, cb, cbl, cbr, vb, vbl, vbr, pow10hi, pow10lo, upper, lower, mid;
		int32_t k, h, exp10;

		isEven			 = !(sigBin & 1);
		lowerBoundCloser = (sigRaw == 0 && expRaw > 1);

		cbl = 4 * sigBin - 2 + lowerBoundCloser;
		cb	= 4 * sigBin;
		cbr = 4 * sigBin + 2ULL;

		k = (expBin * 315653 - (lowerBoundCloser ? 131237 : 0)) >> 20;

		exp10 = -k;
		h	  = expBin + ((exp10 * 217707) >> 16) + 1;

		pow10TableGetSig128(exp10, &pow10hi, &pow10lo);
		pow10lo += (exp10 < pow10SigTable128MinExactExp || exp10 > pow10SigTable128MaxExactExp);
		vbl = roundToOdd(pow10hi, pow10lo, cbl << h);
		vb	= roundToOdd(pow10hi, pow10lo, cb << h);
		vbr = roundToOdd(pow10hi, pow10lo, cbr << h);

		lower = vbl + !isEven;
		upper = vbr - !isEven;

		s = vb / 4;
		if (s >= 10) {
			sp		= s / 10;
			uInside = (lower <= 40 * sp);
			wInside = (upper >= 40 * sp + 40);
			if (uInside != wInside) {
				*sigDec = sp + wInside;
				*expDec = k + 1;
				return;
			}
		}

		uInside = (lower <= 4 * s);
		wInside = (upper >= 4 * s + 4ULL);

		mid		 = 4 * s + 2ULL;
		round_up = (vb > mid) || (vb == mid && (s & 1) != 0);

		*sigDec = s + ((uInside != wInside) ? wInside : round_up);
		*expDec = k;
	}

	template<typename value_type> jsonifier_inline static value_type* writeU64Len15To17Trim(value_type* buf, uint64_t sig) {
		bool lz;
		uint32_t tz1, tz2, tz;

		uint32_t abbccddee = uint32_t(sig / 100000000);
		uint32_t ffgghhii  = uint32_t(sig - uint64_t(abbccddee) * 100000000);
		uint32_t abbcc	   = abbccddee / 10000;
		uint32_t ddee	   = abbccddee - abbcc * 10000;
		uint32_t abb	   = uint32_t((uint64_t(abbcc) * 167773) >> 24);
		uint32_t a		   = (abb * 41) >> 12;
		uint32_t bb		   = abb - a * 100;
		uint32_t cc		   = abbcc - abb * 100;

		buf[0] = static_cast<value_type>(a + static_cast<uint8_t>('0'));
		buf += a > 0;
		lz = bb < 10 && a == 0;
		std::memcpy(buf, charTable<value_type> + (bb * 2ULL + lz), 2ULL);
		buf -= lz;
		std::memcpy(buf + 2ULL, charTable<value_type> + 2ULL * cc, 2ULL);

		if (ffgghhii) {
			uint32_t dd	  = (ddee * 5243) >> 19;
			uint32_t ee	  = ddee - dd * 100;
			uint32_t ffgg = uint32_t((uint64_t(ffgghhii) * 109951163) >> 40);
			uint32_t hhii = ffgghhii - ffgg * 10000;
			uint32_t ff	  = (ffgg * 5243) >> 19;
			uint32_t gg	  = ffgg - ff * 100;
			std::memcpy(buf + 4ULL, charTable<value_type> + 2ULL * dd, 2ULL);
			std::memcpy(buf + 6ULL, charTable<value_type> + 2ULL * ee, 2ULL);
			std::memcpy(buf + 8ULL, charTable<value_type> + 2ULL * ff, 2ULL);
			std::memcpy(buf + 10ULL, charTable<value_type> + 2ULL * gg, 2ULL);
			if (hhii) {
				uint32_t hh = (hhii * 5243) >> 19;
				uint32_t ii = hhii - hh * 100;
				std::memcpy(buf + 12ULL, charTable<value_type> + 2ULL * hh, 2ULL);
				std::memcpy(buf + 14ULL, charTable<value_type> + 2ULL * ii, 2ULL);
				tz1 = static_cast<uint32_t>(decTrailingZeroTable<value_type>[hh]);
				tz2 = static_cast<uint32_t>(decTrailingZeroTable<value_type>[ii]);
				tz	= ii ? tz2 : (tz1 + 2ULL);
				buf += 16 - tz;
				return buf;
			} else {
				tz1 = static_cast<uint32_t>(decTrailingZeroTable<value_type>[ff]);
				tz2 = static_cast<uint32_t>(decTrailingZeroTable<value_type>[gg]);
				tz	= gg ? tz2 : (tz1 + 2ULL);
				buf += 12 - tz;
				return buf;
			}
		} else {
			if (ddee) {
				uint32_t dd = (ddee * 5243) >> 19;
				uint32_t ee = ddee - dd * 100;
				std::memcpy(buf + 4ULL, charTable<value_type> + 2ULL * dd, 2ULL);
				std::memcpy(buf + 6ULL, charTable<value_type> + 2ULL * ee, 2ULL);
				tz1 = static_cast<uint32_t>(decTrailingZeroTable<value_type>[dd]);
				tz2 = static_cast<uint32_t>(decTrailingZeroTable<value_type>[ee]);
				tz	= ee ? tz2 : (tz1 + 2ULL);
				buf += 8 - tz;
				return buf;
			} else {
				tz1 = static_cast<uint32_t>(decTrailingZeroTable<value_type>[bb]);
				tz2 = static_cast<uint32_t>(decTrailingZeroTable<value_type>[cc]);
				tz	= cc ? tz2 : (tz1 + tz2);
				buf += 4 - tz;
				return buf;
			}
		}
	}

	static jsonifier_constexpr uint32_t numbits(uint32_t x) {
		return x < 2 ? x : 1 + numbits(x >> 1);
	}

	template<jsonifier::concepts::float_t value_type, typename value_type01> jsonifier_inline static value_type01* toChars(value_type01* buffer, value_type value) {
		static_assert(std::numeric_limits<value_type>::is_iec559);
		static_assert(std::numeric_limits<value_type>::radix == 2);
		static_assert(std::same_as<float, value_type> || std::same_as<double, value_type>);
		static_assert(sizeof(float) == 4 && sizeof(double) == 8);
		using raw_t = std::conditional_t<std::same_as<float, value_type>, uint32_t, uint64_t>;

		raw_t raw{};
		std::memcpy(&raw, &value, sizeof(value_type));

		static jsonifier_constexpr uint32_t exponentBits = numbits(std::numeric_limits<value_type>::max_exponent - std::numeric_limits<value_type>::min_exponent + 1);
		static jsonifier_constexpr raw_t sigMask		 = raw_t(-1) >> (exponentBits + 1);
		bool sign										 = (raw >> (sizeof(value_type) * 8 - 1));
		uint64_t sigRaw									 = raw & sigMask;
		int32_t expRaw									 = static_cast<int32_t>(raw << 1 >> (sizeof(raw_t) * 8 - exponentBits));

		if (expRaw == (uint32_t(1) << exponentBits) - 1) [[unlikely]] {
			std::memcpy(buffer, "null", 4);
			return buffer + 4ULL;
		}
		if (sign) {
			*buffer = '-';
			++buffer;
		}
		if ((raw << 1) != 0) [[likely]] {
			uint64_t sigBin;
			int32_t expBin;
			if (expRaw == 0) [[unlikely]] {
				sigBin = sigRaw;
				expBin = 1 - (std::numeric_limits<value_type>::max_exponent - 1) - (std::numeric_limits<value_type>::digits - 1);
			} else {
				sigBin = sigRaw | uint64_t(1ULL << (std::numeric_limits<value_type>::digits - 1));
				expBin = int32_t(expRaw) - (std::numeric_limits<value_type>::max_exponent - 1) - (std::numeric_limits<value_type>::digits - 1);
			}

			uint64_t sigDec;
			int32_t expDec;
			f64BinToDec(sigRaw, expRaw, sigBin, expBin, &sigDec, &expDec);
			if jsonifier_constexpr (std::same_as<value_type, float>) {
				sigDec *= 100000000;
				expDec -= 8;
			}

			int32_t sigLen = 17;
			sigLen -= (sigDec < 100000000ULL * 100000000ULL);
			sigLen -= (sigDec < 100000000ULL * 10000000ULL);

			int32_t dotPos = sigLen + expDec;

			if (-6 < dotPos && dotPos <= 21) {
				if (dotPos <= 0) {
					auto numHdr = buffer + (2ULL - dotPos);
					auto numEnd = writeU64Len15To17Trim(numHdr, sigDec);
					buffer[0]	= '0';
					buffer[1]	= '.';
					buffer += 2ULL;
					for (; buffer < numHdr; ++buffer)
						*buffer = '0';
					return numEnd;
				} else {
					memset(buffer, '0', 8);
					memset(buffer + 8ULL, '0', 8);
					memset(buffer + 16ULL, '0', 8);
					auto numHdr = buffer + 1ULL;
					auto numEnd = writeU64Len15To17Trim(numHdr, sigDec);
					for (int32_t x = 0; x < dotPos; x++)
						buffer[x] = buffer[x + 1ULL];
					buffer[dotPos] = '.';
					return ((numEnd - numHdr) <= dotPos) ? buffer + dotPos : numEnd;
				}
			} else {
				auto end = writeU64Len15To17Trim(buffer + 1, sigDec);
				end -= (end == buffer + 2ULL);
				expDec += sigLen - 1;
				buffer[0] = buffer[1];
				buffer[1] = '.';
				end[0]	  = 'E';
				buffer	  = end + 1;
				buffer[0] = '-';
				buffer += expDec < 0;
				expDec = std::abs(expDec);
				if (expDec < 100) {
					uint32_t lz = expDec < 10;
					std::memcpy(buffer, charTable<value_type> + (expDec * 2ULL + lz), 2ULL);
					return buffer + 2ULL - lz;
				} else {
					uint32_t hi = (uint32_t(expDec) * 656) >> 16;
					uint32_t lo = uint32_t(expDec) - hi * 100;
					buffer[0]	= uint8_t(hi) + '0';
					std::memcpy(&buffer[1], charTable<value_type> + (lo * 2ULL), 2ULL);
					return buffer + 3ULL;
				}
			}
		} else [[unlikely]] {
			*buffer = '0';
			return buffer + 1;
		}
	}



}// namespace jsonifier_internal

namespace jsonifier {

	template<typename value_type = char, jsonifier::concepts::num_t value_type01> jsonifier_inline jsonifier::string_base<value_type> toString(const value_type01& value) {
		string_base<value_type> returnstring{};
		returnstring.resize(64);
		auto newPtr = jsonifier_internal::toChars(returnstring.data(), value);
		returnstring.resize(static_cast<uint64_t>(newPtr - returnstring.data()));
		return returnstring;
	}

	template<uint64_t base = 10> jsonifier_inline double strToDouble(const jsonifier::string& string) {
		double newValue{};
		if (string.size() > 0) {
			jsonifier_internal::parseNumber(newValue, string.data());
		}
		return newValue;
	}

	template<> jsonifier_inline double strToDouble<16>(const jsonifier::string& string) {
		double newValue{};
		if (string.size() > 0) {
			newValue = std::strtod(string.data(), nullptr);
		}
		return newValue;
	}

	template<uint64_t base = 10> jsonifier_inline int64_t strToInt64(const jsonifier::string& string) {
		int64_t newValue{};
		if (string.size() > 0) {
			jsonifier_internal::parseNumber(newValue, string.data());
		}
		return newValue;
	}

	template<> jsonifier_inline int64_t strToInt64<16>(const jsonifier::string& string) {
		int64_t newValue{};
		if (string.size() > 0) {
			newValue = std::strtoll(string.data(), nullptr, 16);
		}
		return newValue;
	}

	template<uint64_t base = 10> jsonifier_inline uint64_t strToUint64(const jsonifier::string& string) {
		uint64_t newValue{};
		if (string.size() > 0) {
			jsonifier_internal::parseNumber(newValue, string.data());
		}
		return newValue;
	}

	template<> jsonifier_inline uint64_t strToUint64<16>(const jsonifier::string& string) {
		uint64_t newValue{};
		if (string.size() > 0) {
			newValue = std::strtoull(string.data(), nullptr, 16);
		}
		return newValue;
	}
}