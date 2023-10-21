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
	inline static uint64_t mulhi64(uint64_t a, uint64_t b) {
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
	inline static uint64_t mulhi64(uint64_t a, uint64_t b) {
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
	static constexpr auto pow10SigTable128MinExp	  = -343;
	static constexpr auto pow10SigTable128MaxExp	  = 324;
	static constexpr auto pow10SigTable128MinExactExp = 0;
	static constexpr auto pow10SigTable128MaxExactExp = 55;

	inline static void pow10TableGetSig128(int32_t exp10, uint64_t* hi, uint64_t* lo) {
		int32_t idx = exp10 - (pow10SigTable128MinExp);
		*hi			= pow10SigTable128[idx * 2ull];
		*lo			= pow10SigTable128[idx * 2ull + 1ull];
	}

	inline static uint64_t sig2FromExp10(int32_t exp10) {
		return pow10SigTable128[static_cast<uint64_t>(exp10) - pow10SigTable128MinExp];
	}

	inline static int32_t exp2FromExp10(int32_t exp10) {
		return (((exp10 * 217706 - 4128768) >> 16) + 126);
	}

	static constexpr uint8_t digiTypeZero	 = 1 << 0;
	static constexpr uint8_t digiTypeNonZero = 1 << 1;
	static constexpr uint8_t digiTypePos	 = 1 << 2;
	static constexpr uint8_t digiTypeNeg	 = 1 << 3;
	static constexpr uint8_t digiTypeDot	 = 1 << 4;
	static constexpr uint8_t digiTypeExp	 = 1 << 5;

	inline static bool digiIsType(uint8_t d, uint8_t type) {
		return (digiTable<uint8_t>[d] & type) != 0;
	}

	inline static bool digiIsFp(uint8_t d) {
		return digiIsType(d, uint8_t(digiTypeDot | digiTypeExp));
	}

	inline static bool digiIsDigitOrFp(uint8_t d) {
		return digiIsType(d, uint8_t(digiTypeZero | digiTypeNonZero | digiTypeDot | digiTypeExp));
	}

#define repeatIIn_1_18(x) \
	{ x(1) x(2) x(3) x(4) x(5) x(6) x(7) x(8) x(9) x(10) x(11) x(12) x(13) x(14) x(15) x(16) x(17) x(18) }

	static constexpr auto eBit		   = static_cast<uint8_t>(0x45u ^ 0x65u);
	static constexpr auto f64MaxDecExp = 308;
	static constexpr auto f64MinDecExp = (-324);

	inline static consteval uint32_t ceillog2(uint32_t x) {
		return x < 2 ? x : 1 + ceillog2(x >> 1);
	}

	struct big_int_t {
		jsonifier::vector<uint32_t> data = {};

		inline big_int_t(uint64_t num) {
			uint32_t lowerWord = uint32_t(num);
			uint32_t upperWord = uint32_t(num >> 32);
			if (upperWord > 0) {
				data = { lowerWord, upperWord };
			} else {
				data = { lowerWord };
			}
		}

		inline void mulU32(uint32_t num) {
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

		inline void mulPow10(uint32_t pow10) {
			for (; pow10 >= 9; pow10 -= 9) {
				mulU32(static_cast<uint32_t>(powersOfTenInt[9]));
			}
			if (pow10) {
				mulU32(static_cast<uint32_t>(powersOfTenInt[pow10]));
			}
		}

		inline void mulPow2(uint32_t exp) {
			uint32_t shft = exp % 32;
			uint32_t move = exp / 32;
			uint32_t idx  = static_cast<uint32_t>(data.size()) - 1;
			if (shft == 0) {
				data.resize(data.size() + move);
				for (; idx > 0; idx--) {
					data[static_cast<uint64_t>(idx) + static_cast<uint64_t>(move) - 1ull] = data[static_cast<uint64_t>(idx) - 1ull];
				}
				while (move)
					data[--move] = 0;
			} else {
				data.resize(data.size() + move + 1);
				++idx;
				for (; idx > 0; idx--) {
					uint32_t num = data[idx] << shft;
					num |= data[idx - 1ull] >> (32 - shft);
					data[static_cast<uint64_t>(idx) + static_cast<uint64_t>(move)] = num;
				}
				data[move] = data[0] << shft;
				if (data.back() == 0)
					data.resize(data.size() - 1);
				while (move)
					data[--move] = 0;
			}
		}

		inline auto operator<=>(const big_int_t& rhs) const {
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

	inline static void u128Mul(uint64_t a, uint64_t b, uint64_t* hi, uint64_t* lo) {
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

	inline static void u128MulAdd(uint64_t a, uint64_t b, uint64_t c, uint64_t* hi, uint64_t* lo) {
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

	inline static uint64_t roundToOdd(uint64_t hi, uint64_t lo, uint64_t codePoint) {
		uint64_t xHi{}, xLo{}, yHi{}, yLo{};
		u128Mul(codePoint, lo, &xHi, &xLo);
		u128MulAdd(codePoint, hi, xHi, &yHi, &yLo);
		return yHi | (yLo > 1);
	}

	template<jsonifier::concepts::unsigned_t value_type, typename value_type02> inline static bool parseNumber(value_type& val, value_type02* cur) {
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
		sign = (*hdr == 0x2Du);
		cur += sign;
		auto applySign = [&](auto&& val) -> value_type {
			return static_cast<value_type>(val);
		};
		sig = static_cast<uint64_t>(*cur) - static_cast<uint64_t>(0x30u);
		if (sig > 9) {
			return false;
		}
		static constexpr auto zero = static_cast<uint8_t>(0x30u);
#define expr_intg(x) \
	if ((numTmp = static_cast<uint64_t>(cur[x] - static_cast<uint64_t>(zero))) <= 9) [[likely]] \
		sig = static_cast<uint64_t>(numTmp) + sig * 10ull; \
	else { \
		goto digi_sepr_##x; \
	}
		repeatIIn_1_18(expr_intg);
#undef expr_intg
		cur += 19;
		if (!digiIsDigitOrFp(*cur)) {
			val = static_cast<value_type>(sig);
			return true;
		}
		goto digi_intg_more;
#define expr_sepr(x) \
	digi_sepr_##x : if ((!digiIsFp(cur[x]))) [[likely]] { \
		cur += x; \
		val = static_cast<value_type>(applySign(sig)); \
		return true; \
	} \
	dotPos = cur + x; \
	if ((cur[x] == 0x2Eu)) [[likely]] { \
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
				cur += 20ull + fracZeroes;
		if (uint8_t(*cur - zero) > 9)
			goto digi_frac_end;
		goto digi_frac_more;
#define expr_stop(x) \
	digi_stop_##x : cur += x##ull + 1ull + fracZeroes; \
	goto digi_frac_end;
		repeatIIn_1_18(expr_stop)
#undef expr_stop
			digi_intg_more : static constexpr uint64_t u64Max = std::numeric_limits<uint64_t>::max();
		if ((numTmp = static_cast<uint64_t>(*cur - static_cast<uint64_t>(zero))) < 10) {
			if (!digiIsDigitOrFp(cur[1])) {
				if ((sig < (u64Max / 10)) || (sig == (u64Max / 10) && numTmp <= (u64Max % 10))) {
					sig = numTmp + sig * 10;
					++cur;
					val = static_cast<value_type>(sig);
					return true;
				}
			}
		}
		if ((eBit | *cur) == 0x65u) {
			dotPos = cur;
			goto digi_exp_more;
		}
		if (*cur == 0x2Eu) {
			dotPos = cur++;
			if (uint8_t(*cur - static_cast<uint64_t>(zero)) > 9) {
				return false;
			}
		}
	digi_frac_more:
		sigCut = cur;
		sig += (*cur >= 0x35);
		while (uint8_t(*++cur - static_cast<uint64_t>(zero)) < 10) {
		}
		if (!dotPos) {
			dotPos = cur;
			if (*cur == 0x2Eu) {
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
		while (*tmp == 0x30u || *tmp == 0x2Eu)
			tmp--;
		if (tmp < sigCut) {
			sigCut = nullptr;
		}
		if ((eBit | *cur) == 0x65u)
			goto digi_exp_more;
		goto digi_exp_finish;
	digi_frac_end:
		expSig = -int32_t((cur - dotPos) - 1);
		if ((eBit | *cur) != 0x65u) [[likely]] {
			if ((expSig < f64MinDecExp - 19)) [[unlikely]] {
				val = static_cast<value_type>(applySign(0));
				return true;
			}
			exp = expSig;
			goto digi_finish;
		} else {
			goto digi_exp_more;
		}
	digi_exp_more:
		expSign = (*++cur == 0x2Du);
		cur += (*cur == 0x2Bu || *cur == 0x2Du);
		if (uint8_t(*cur - zero) > 9) [[unlikely]] {
			goto digi_finish;
		}
		while (*cur == 0x30u)
			++cur;
		tmp = cur;
		uint8_t c;
		while (uint8_t(c = static_cast<uint8_t>(*cur - zero)) < 10) {
			++cur;
			expLit = c + int32_t(expLit) * 10;
		}
		if ((cur - tmp >= 6)) [[unlikely]] {
			if (sig == 0 || expSign) {
				val = static_cast<value_type>(applySign(0));
				val = static_cast<value_type>(sig);
				return true;
			} else {
				val = static_cast<value_type>(applySign(std::numeric_limits<value_type>::infinity()));
				return true;
			}
		}
		expSig += expSign ? -expLit : expLit;
	digi_exp_finish:
		if (sig == 0) {
			val = static_cast<value_type>((!sign) ? -0 : 0);
			return true;
		}
		if (expSig < -20) {
			val = static_cast<value_type>(applySign(0));
			return true;
		} else if (expSig > 20) {
			val = static_cast<value_type>(applySign(std::numeric_limits<value_type>::infinity()));
			return true;
		}
		exp = expSig;
	digi_finish:

		val = static_cast<value_type>(sig);
		if (exp >= 0 && exp < 20) {
			val *= static_cast<value_type>(powersOfTenInt[exp]);
		} else if (exp > -20 && exp < 0) {
			val /= static_cast<value_type>(powersOfTenInt[-exp]);
		}
		return true;
	}

	template<jsonifier::concepts::signed_t value_type, typename value_type02> inline static bool parseNumber(value_type& val, value_type02* cur) {
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
		sign = (*hdr == 0x2Du);
		cur += sign;
		auto applySign = [&](auto&& val) -> value_type {
			return sign ? -static_cast<value_type>(val) : static_cast<value_type>(val);
		};
		sig = static_cast<uint64_t>(*cur) - static_cast<uint64_t>(0x30u);
		if (sig > 9) {
			if (*cur == 0x6Eu && cur[1] == 0x75 && cur[2] == 0x6Cu && cur[3] == 0x6Cu) {
				cur += 4;
				val = static_cast<value_type>(std::numeric_limits<value_type>::quiet_NaN());
				return true;
			} else if ((*cur | eBit) == 0x6Eu && (cur[1] | eBit) == 0x61u && (cur[2] | eBit) == 0x6Eu) {
				cur += 3;
				val = static_cast<value_type>(sign ? -std::numeric_limits<value_type>::quiet_NaN() : std::numeric_limits<value_type>::quiet_NaN());
				return true;
			} else {
				return false;
			}
		}
		static constexpr auto zero = static_cast<uint8_t>(0x30u);
#define expr_intg(x) \
	if ((numTmp = static_cast<uint64_t>(cur[x] - static_cast<uint64_t>(zero))) <= 9) [[likely]] \
		sig = static_cast<uint64_t>(numTmp) + sig * 10ull; \
	else { \
		goto digi_sepr_##x; \
	}
		repeatIIn_1_18(expr_intg);
#undef expr_intg
		cur += 19;
		if (!digiIsDigitOrFp(*cur)) {
			val = static_cast<value_type>(sig);
			val *= sign ? -1 : 1;
			return true;
		}
		goto digi_intg_more;
#define expr_sepr(x) \
	digi_sepr_##x : if ((!digiIsFp(cur[x]))) [[likely]] { \
		cur += x; \
		val = static_cast<value_type>(applySign(sig)); \
		return true; \
	} \
	dotPos = cur + x; \
	if ((cur[x] == 0x2Eu)) [[likely]] { \
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
				cur += 20ull + fracZeroes;
		if (uint8_t(*cur - static_cast<uint64_t>(zero)) > 9)
			goto digi_frac_end;
		goto digi_frac_more;
#define expr_stop(x) \
	digi_stop_##x : cur += x##ull + 1ull + fracZeroes; \
	goto digi_frac_end;
		repeatIIn_1_18(expr_stop)
#undef expr_stop
			digi_intg_more : static constexpr uint64_t u64Max = std::numeric_limits<uint64_t>::max();
		if ((numTmp = static_cast<uint64_t>(*cur - static_cast<uint64_t>(zero))) < 10) {
			if (!digiIsDigitOrFp(cur[1])) {
				if ((sig < (u64Max / 10)) || (sig == (u64Max / 10) && numTmp <= (u64Max % 10))) {
					sig = numTmp + sig * 10;
					++cur;
					val = static_cast<value_type>(sig);
					val *= sign ? -1 : 1;
					return true;
				}
			}
		}
		if ((eBit | *cur) == 0x65u) {
			dotPos = cur;
			goto digi_exp_more;
		}
		if (*cur == 0x2Eu) {
			dotPos = cur++;
			if (uint8_t(*cur - zero) > 9) {
				return false;
			}
		}
	digi_frac_more:
		sigCut = cur;
		sig += (*cur >= 0x35);
		while (uint8_t(*++cur - zero) < 10) {
		}
		if (!dotPos) {
			dotPos = cur;
			if (*cur == 0x2Eu) {
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
		while (*tmp == 0x30u || *tmp == 0x2Eu)
			tmp--;
		if (tmp < sigCut) {
			sigCut = nullptr;
		}
		if ((eBit | *cur) == 0x65u)
			goto digi_exp_more;
		goto digi_exp_finish;
	digi_frac_end:
		expSig = -int32_t((cur - dotPos) - 1);
		if ((eBit | *cur) != 0x65u) [[likely]] {
			if ((expSig < f64MinDecExp - 19)) [[unlikely]] {
				val = static_cast<value_type>(applySign(0));
				return true;
			}
			exp = expSig;
			goto digi_finish;
		} else {
			goto digi_exp_more;
		}
	digi_exp_more:
		expSign = (*++cur == 0x2Du);
		cur += (*cur == 0x2Bu || *cur == 0x2Du);
		if (uint8_t(*cur - zero) > 9) [[unlikely]] {
			goto digi_finish;
		}
		while (*cur == 0x30u)
			++cur;
		tmp = cur;
		uint8_t c;
		while (uint8_t(c = static_cast<uint8_t>(*cur - zero)) < 10) {
			++cur;
			expLit = c + int32_t(expLit) * 10;
		}
		if ((cur - tmp >= 6)) [[unlikely]] {
			if (sig == 0 || expSign) {
				val = static_cast<value_type>(applySign(0));
				val = static_cast<value_type>(sig);
				return true;
			} else {
				val = static_cast<value_type>(applySign(std::numeric_limits<value_type>::infinity()));
				return true;
			}
		}
		expSig += expSign ? -expLit : expLit;
	digi_exp_finish:
		if (sig == 0) {
			val = static_cast<value_type>((sign) ? -0 : 0);
			return true;
		}
		if (expSig < -20) {
			val = static_cast<value_type>(applySign(0));
			return true;
		} else if (expSig > 20) {
			val = static_cast<value_type>(applySign(std::numeric_limits<value_type>::infinity()));
			return true;
		}
		exp = expSig;
	digi_finish:

		val = static_cast<value_type>(sig);
		val *= sign ? -1 : 1;
		if (exp >= 0 && exp < 20) {
			val *= static_cast<value_type>(powersOfTenInt[exp]);
		} else if (exp > -20 && exp < 0) {
			val /= static_cast<value_type>(powersOfTenInt[-exp]);
		}
		return true;
	}

	template<jsonifier::concepts::float_t value_type, typename value_type02> inline static bool parseNumber(value_type& val, value_type02* cur) {
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
		sign = (*hdr == 0x2Du);
		cur += sign;
		auto applySign = [&](auto&& val) -> value_type {
			if constexpr (std::is_unsigned_v<value_type>) {
				return static_cast<value_type>(val);
			} else {
				return sign ? -static_cast<value_type>(val) : static_cast<value_type>(val);
			}
		};
		sig = static_cast<uint64_t>(*cur) - 0x30u;
		if (sig > 9) {
			if (*cur == 0x6Eu && cur[1] == 0x75 && cur[2] == 0x6Cu && cur[3] == 0x6Cu) {
				cur += 4;
				val = static_cast<value_type>(std::numeric_limits<value_type>::quiet_NaN());
				return true;
			} else if ((*cur | eBit) == 0x6Eu && (cur[1] | eBit) == 0x61u && (cur[2] | eBit) == 0x6Eu) {
				cur += 3;
				val = static_cast<value_type>(sign ? -std::numeric_limits<value_type>::quiet_NaN() : std::numeric_limits<value_type>::quiet_NaN());
				return true;
			} else {
				return false;
			}
		}
		static constexpr auto zero = static_cast<uint8_t>(0x30u);
#define expr_intg(x) \
	if ((numTmp = static_cast<uint64_t>(cur[x] - static_cast<uint64_t>(zero))) <= 9) [[likely]] \
		sig = static_cast<uint64_t>(numTmp) + sig * 10ull; \
	else { \
		goto digi_sepr_##x; \
	}
		repeatIIn_1_18(expr_intg);
#undef expr_intg
		cur += 19;
		if (!digiIsDigitOrFp(*cur)) {
			val = static_cast<value_type>(sig);
			if constexpr (!std::is_unsigned_v<value_type>) {
				val *= sign ? -1 : 1;
			}
			return true;
		}
		goto digi_intg_more;
#define expr_sepr(x) \
	digi_sepr_##x : if ((!digiIsFp(cur[x]))) [[likely]] { \
		cur += x; \
		val = static_cast<value_type>(applySign(sig)); \
		return true; \
	} \
	dotPos = cur + x; \
	if ((cur[x] == 0x2Eu)) [[likely]] { \
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
				cur += 20ull + fracZeroes;
		if (uint8_t(*cur - zero) > 9)
			goto digi_frac_end;
		goto digi_frac_more;
#define expr_stop(x) \
	digi_stop_##x : cur += x##ull + 1ull + fracZeroes; \
	goto digi_frac_end;
		repeatIIn_1_18(expr_stop)
#undef expr_stop
			digi_intg_more : static constexpr uint64_t u64Max = std::numeric_limits<uint64_t>::max();
		if ((numTmp = static_cast<uint64_t>(*cur - static_cast<uint64_t>(zero))) < 10) {
			if (!digiIsDigitOrFp(cur[1])) {
				if ((sig < (u64Max / 10)) || (sig == (u64Max / 10) && numTmp <= (u64Max % 10))) {
					sig = numTmp + sig * 10;
					++cur;
					val = static_cast<value_type>(sig);
					if constexpr (!std::is_unsigned_v<value_type>) {
						val *= sign ? -1 : 1;
					}
					return true;
				}
			}
		}
		if ((eBit | *cur) == 0x65u) {
			dotPos = cur;
			goto digi_exp_more;
		}
		if (*cur == 0x2Eu) {
			dotPos = cur++;
			if (uint8_t(*cur - zero) > 9) {
				return false;
			}
		}
	digi_frac_more:
		sigCut = cur;
		sig += (*cur >= 0x35);
		while (uint8_t(*++cur - zero) < 10) {
		}
		if (!dotPos) {
			dotPos = cur;
			if (*cur == 0x2Eu) {
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
		while (*tmp == 0x30u || *tmp == 0x2Eu)
			tmp--;
		if (tmp < sigCut) {
			sigCut = nullptr;
		}
		if ((eBit | *cur) == 0x65u)
			goto digi_exp_more;
		goto digi_exp_finish;
	digi_frac_end:
		expSig = -int32_t((cur - dotPos) - 1);
		if ((eBit | *cur) != 0x65u) [[likely]] {
			if ((expSig < f64MinDecExp - 19)) [[unlikely]] {
				val = static_cast<value_type>(applySign(0));
				return true;
			}
			exp = expSig;
			goto digi_finish;
		} else {
			goto digi_exp_more;
		}
	digi_exp_more:
		expSign = (*++cur == 0x2Du);
		cur += (*cur == 0x2Bu || *cur == 0x2Du);
		if (uint8_t(*cur - zero) > 9) [[unlikely]] {
			goto digi_finish;
		}
		while (*cur == 0x30u)
			++cur;
		tmp = cur;
		uint8_t c;
		while (uint8_t(c = static_cast<uint8_t>(*cur - zero)) < 10) {
			++cur;
			expLit = c + int32_t(expLit) * 10;
		}
		if ((cur - tmp >= 6)) [[unlikely]] {
			if (sig == 0 || expSign) {
				val = static_cast<value_type>(applySign(0));
				val = static_cast<value_type>(sig);
				return true;
			} else {
				val = static_cast<value_type>(applySign(std::numeric_limits<value_type>::infinity()));
				return true;
			}
		}
		expSig += expSign ? -expLit : expLit;
	digi_exp_finish:
		if (sig == 0) {
			val = static_cast<value_type>(sign ? -value_type{ 0 } : value_type{ 0 });
			return true;
		}
		if ((expSig < f64MinDecExp - 19)) [[unlikely]] {
			val = static_cast<value_type>(sign ? -value_type{ 0 } : value_type{ 0 });
			return true;
		} else if ((expSig > f64MaxDecExp)) [[unlikely]] {
			val = static_cast<value_type>(sign ? -std::numeric_limits<value_type>::infinity() : std::numeric_limits<value_type>::infinity());
			return true;
		}
		exp = expSig;
	digi_finish:

		if constexpr (std::floating_point<value_type>) {
			if (sig < (uint64_t(1) << 53) && std::abs(exp) <= 22) {
				val = static_cast<value_type>(sig);
				if constexpr (!std::is_unsigned_v<value_type>) {
					val *= sign ? -1 : 1;
				}
				if (exp >= 0 && exp < 23) {
					val *= static_cast<value_type>(powersOfTenFloat[exp]);
				} else if (exp > -23 && exp < 0) {
					val /= static_cast<value_type>(powersOfTenFloat[-exp]);
				}
				return true;
			}
		} else {
			if (sig < (uint64_t(1) << 24) && std::abs(exp) <= 8) {
				val = static_cast<value_type>(sig);
				if constexpr (!std::is_unsigned_v<value_type>) {
					val *= sign ? -1 : 1;
				}
				if (exp >= 0 && exp < 23) {
					val *= static_cast<value_type>(powersOfTenFloat[exp]);
				} else if (exp > -23 && exp < 0) {
					val /= static_cast<value_type>(powersOfTenFloat[-exp]);
				}
				return true;
			}
		}

		static_assert(std::numeric_limits<value_type>::is_iec559);
		static_assert(std::numeric_limits<value_type>::radix == 2);
		static_assert(std::same_as<float, std::unwrap_ref_decay_t<value_type>> || std::same_as<double, std::unwrap_ref_decay_t<value_type>>);
		static_assert(sizeof(float) == 4 && sizeof(double) == 8);

		using raw_t								= std::conditional_t<std::same_as<float, std::unwrap_ref_decay_t<value_type>>, uint32_t, uint64_t>;
		const auto sigLeadingZeroes				= std::countl_zero(sig);
		const auto sigNorm						= sig << sigLeadingZeroes;
		const auto sig2Norm						= sig2FromExp10(exp);
		const auto sigProduct					= mulhi64(sigNorm, sig2Norm) + 1;
		const auto sigProductStartsWith1		= sigProduct >> 63;
		auto mantisa							= sigProduct << (2 - sigProductStartsWith1);
		static constexpr uint64_t roundMask		= uint64_t(1) << 63 >> (std::numeric_limits<value_type>::digits - 1);
		static constexpr uint32_t exponentBits	= ceillog2(std::numeric_limits<value_type>::max_exponent - std::numeric_limits<value_type>::min_exponent + 1);
		static constexpr uint32_t mantissaShift = exponentBits + 1 + 64 - 8 * sizeof(raw_t);
		int32_t exp2							= static_cast<int32_t>(exp2FromExp10(exp) + static_cast<uint32_t>(-sigLeadingZeroes + sigProductStartsWith1));

		if (exp2 < std::numeric_limits<value_type>::min_exponent - 1) [[unlikely]] {
			val = static_cast<value_type>(sign ? -value_type(0) : value_type(0));
			return true;
		} else if (exp2 > std::numeric_limits<value_type>::max_exponent - 1) [[unlikely]] {
			val = static_cast<value_type>(sign ? -std::numeric_limits<value_type>::infinity() : std::numeric_limits<value_type>::infinity());
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
					128ull - std::numeric_limits<value_type>::digits - (2ull - sigProductStartsWith1))) {
				round = 1;
			}
		}

		auto num = raw_t(sign) << (sizeof(raw_t) * 8ull - 1ull) | raw_t(mantisa >> mantissaShift) |
			(raw_t(static_cast<uint64_t>(exp2) + std::numeric_limits<value_type>::max_exponent - 1ull) << (std::numeric_limits<value_type>::digits - 1ull));
		num += raw_t(round);
		std::memcpy(&val, &num, sizeof(value_type));
		return true;
	}

	template<jsonifier::concepts::unsigned_t value_type02, typename value_type> inline static value_type* toChars(value_type* buf, value_type02 val) {
		uint32_t aa, bb, cc, dd, ee, aabb, bbcc, ccdd, ddee, aabbcc;
		uint32_t lz;

		if (val < 0x64u) {
			lz = val < 10;
			std::memcpy(&buf[0], &charTable<value_type>[val * 2ull + lz], 2);
			buf -= lz;
			return buf + 2ull;
		} else if (val < 10000) {
			aa = (val * 5243) >> 19;
			bb = val - aa * 0x64u;
			lz = aa < 10;
			std::memcpy(&buf[0], &charTable<value_type>[aa * 2ull + lz], 2);
			buf -= lz;
			std::memcpy(&buf[2], &charTable<value_type>[2ull * bb], 2);

			return buf + 4ull;
		} else if (val < 1000000) {
			aa	 = uint32_t((uint64_t(val) * 429497) >> 32);
			bbcc = val - aa * 10000;
			bb	 = (bbcc * 5243) >> 19;
			cc	 = bbcc - bb * 0x64u;
			lz	 = aa < 10;
			std::memcpy(buf, charTable<value_type> + aa * 2ull + lz, 2);
			buf -= lz;
			std::memcpy(buf + 2ull, charTable<value_type> + bb * 2ull, 2);
			std::memcpy(buf + 4ull, charTable<value_type> + cc * 2ull, 2);
			return buf + 6ull;
		} else if (val < 100000000) {
			aabb = uint32_t((uint64_t(val) * 109951163) >> 40);
			ccdd = val - aabb * 10000;
			aa	 = (aabb * 5243) >> 19;
			cc	 = (ccdd * 5243) >> 19;
			bb	 = aabb - aa * 0x64u;
			dd	 = ccdd - cc * 0x64u;
			lz	 = aa < 10;
			std::memcpy(buf, charTable<value_type> + aa * 2ull + lz, 2);
			buf -= lz;
			std::memcpy(buf + 2ull, charTable<value_type> + bb * 2ull, 2);
			std::memcpy(buf + 4ull, charTable<value_type> + cc * 2ull, 2);
			std::memcpy(buf + 6ull, charTable<value_type> + dd * 2ull, 2);
			return buf + 8;
		} else {
			aabbcc = uint32_t((uint64_t(val) * 3518437209ul) >> 45);
			aa	   = uint32_t((uint64_t(aabbcc) * 429497) >> 32);
			ddee   = val - aabbcc * 10000;
			bbcc   = aabbcc - aa * 10000;
			bb	   = (bbcc * 5243) >> 19;
			dd	   = (ddee * 5243) >> 19;
			cc	   = bbcc - bb * 0x64u;
			ee	   = ddee - dd * 0x64u;
			lz	   = aa < 10;
			std::memcpy(buf, charTable<value_type> + aa * 2ull + lz, 2);
			buf -= lz;
			std::memcpy(buf + 2ull, charTable<value_type> + bb * 2ull, 2);
			std::memcpy(buf + 4ull, charTable<value_type> + cc * 2ull, 2);
			std::memcpy(buf + 6ull, charTable<value_type> + dd * 2ull, 2);
			std::memcpy(buf + 8, charTable<value_type> + ee * 2ull, 2);
			return buf + 10;
		}
	}

	template<jsonifier::concepts::signed_t value_type02, typename value_type> inline static value_type* toChars(value_type* buf, value_type02 val) {
		uint32_t neg  = uint32_t(-val);
		uint64_t sign = val < 0;
		*buf		  = 0x2Du;
		return toChars(buf + sign, sign ? uint32_t(neg) : uint32_t(val));
	}

	template<typename value_type> inline static value_type* toCharsU64Len8(value_type* buf, uint32_t val) {
		uint32_t aa, bb, cc, dd, aabb, ccdd;
		aabb = uint32_t((uint64_t(val) * 109951163) >> 40);
		ccdd = val - aabb * 10000;
		aa	 = (aabb * 5243) >> 19;
		cc	 = (ccdd * 5243) >> 19;
		bb	 = aabb - aa * 0x64u;
		dd	 = ccdd - cc * 0x64u;
		std::memcpy(buf, charTable<value_type> + aa * 2ull, 2);
		std::memcpy(buf + 2ull, charTable<value_type> + bb * 2ull, 2);
		std::memcpy(buf + 4ull, charTable<value_type> + cc * 2ull, 2);
		std::memcpy(buf + 6ull, charTable<value_type> + dd * 2ull, 2);
		return buf + 8;
	}

	template<typename value_type> inline static value_type* toCharsU64Len4(value_type* buf, uint32_t val) {
		uint32_t aa, bb;
		aa = (val * 5243) >> 19;
		bb = val - aa * 0x64u;
		std::memcpy(buf, charTable<value_type> + aa * 2ull, 2);
		std::memcpy(buf + 2ull, charTable<value_type> + bb * 2ull, 2);
		return buf + 4ull;
	}

	template<typename value_type> inline static value_type* toCharsU64Len18(value_type* buf, uint32_t val) {
		uint32_t aa, bb, cc, dd, aabb, bbcc, ccdd, lz;

		if (val < 0x64u) {
			lz = val < 10;
			std::memcpy(buf, charTable<value_type> + val * 2ull + lz, 2);
			buf -= lz;
			return buf + 2ull;
		} else if (val < 10000) {
			aa = (val * 5243) >> 19;
			bb = val - aa * 0x64u;
			lz = aa < 10;
			std::memcpy(buf, charTable<value_type> + aa * 2ull + lz, 2);
			buf -= lz;
			std::memcpy(buf + 2ull, charTable<value_type> + bb * 2ull, 2);
			return buf + 4ull;
		} else if (val < 1000000) {
			aa	 = uint32_t((uint64_t(val) * 429497) >> 32);
			bbcc = val - aa * 10000;
			bb	 = (bbcc * 5243) >> 19;
			cc	 = bbcc - bb * 0x64u;
			lz	 = aa < 10;
			std::memcpy(buf, charTable<value_type> + aa * 2ull + lz, 2);
			buf -= lz;
			std::memcpy(buf + 2ull, charTable<value_type> + bb * 2ull, 2);
			std::memcpy(buf + 4ull, charTable<value_type> + cc * 2ull, 2);
			return buf + 6ull;
		} else {
			aabb = uint32_t((uint64_t(val) * 109951163) >> 40);
			ccdd = val - aabb * 10000;
			aa	 = (aabb * 5243) >> 19;
			cc	 = (ccdd * 5243) >> 19;
			bb	 = aabb - aa * 0x64u;
			dd	 = ccdd - cc * 0x64u;
			lz	 = aa < 10;
			std::memcpy(buf, charTable<value_type> + aa * 2ull + lz, 2);
			buf -= lz;
			std::memcpy(buf + 2ull, charTable<value_type> + bb * 2ull, 2);
			std::memcpy(buf + 4ull, charTable<value_type> + cc * 2ull, 2);
			std::memcpy(buf + 6ull, charTable<value_type> + dd * 2ull, 2);
			return buf + 8;
		}
	}

	template<typename value_type> inline static value_type* toCharsU64Len58(value_type* buf, uint32_t val) {
		uint32_t aa, bb, cc, dd, aabb, bbcc, ccdd, lz;

		if (val < 1000000) {
			aa	 = uint32_t((uint64_t(val) * 429497) >> 32);
			bbcc = val - aa * 10000;
			bb	 = (bbcc * 5243) >> 19;
			cc	 = bbcc - bb * 0x64u;
			lz	 = aa < 10;
			std::memcpy(buf, charTable<value_type> + aa * 2ull + lz, 2);
			buf -= lz;
			std::memcpy(buf + 2ull, charTable<value_type> + bb * 2ull, 2);
			std::memcpy(buf + 4ull, charTable<value_type> + cc * 2ull, 2);
			return buf + 6ull;
		} else {
			aabb = uint32_t((uint64_t(val) * 109951163) >> 40);
			ccdd = val - aabb * 10000;
			aa	 = (aabb * 5243) >> 19;
			cc	 = (ccdd * 5243) >> 19;
			bb	 = aabb - aa * 0x64u;
			dd	 = ccdd - cc * 0x64u;
			lz	 = aa < 10;
			std::memcpy(buf, charTable<value_type> + aa * 2ull + lz, 2);
			buf -= lz;
			std::memcpy(buf + 2ull, charTable<value_type> + bb * 2ull, 2);
			std::memcpy(buf + 4ull, charTable<value_type> + cc * 2ull, 2);
			std::memcpy(buf + 6ull, charTable<value_type> + dd * 2ull, 2);
			return buf + 8;
		}
	}

	template<jsonifier::concepts::unsigned_t value_type, typename char_type>
		requires(sizeof(value_type) == 8)
	inline static char_type* toChars(char_type* buf, value_type val) {
		uint64_t tmp, hgh;
		uint32_t mid, low;

		if (val < 100000000) {
			buf = toCharsU64Len18(buf, uint32_t(val));
			return buf;
		} else if (val < 100000000ull * 100000000ull) {
			hgh = val / 100000000;
			low = uint32_t(val - hgh * 100000000);
			buf = toCharsU64Len18(buf, uint32_t(hgh));
			buf = toCharsU64Len8(buf, low);
			return buf;
		} else {
			tmp = val / 100000000;
			low = uint32_t(val - tmp * 100000000);
			hgh = uint32_t(tmp / 10000);
			mid = uint32_t(tmp - hgh * 10000);
			buf = toCharsU64Len58(buf, uint32_t(hgh));
			buf = toCharsU64Len4(buf, mid);
			buf = toCharsU64Len8(buf, low);
			return buf;
		}
	}

	template<jsonifier::concepts::signed_t value_type, typename char_type>
		requires(sizeof(value_type) == 8)
	inline static char_type* toChars(char_type* buf, value_type val) {
		uint64_t neg  = uint64_t(-val);
		uint64_t sign = val < 0;
		*buf		  = 0x2Du;
		return toChars(buf + sign, sign ? uint64_t(neg) : uint64_t(val));
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
	inline static void f64BinToDec(uint64_t sigRaw, int32_t expRaw, uint64_t sigBin, int32_t expBin, uint64_t* sigDec, int32_t* expDec) {
		bool isEven, lowerBoundCloser, uInside, wInside, round_up;
		uint64_t s, sp, cb, cbl, cbr, vb, vbl, vbr, pow10hi, pow10lo, upper, lower, mid;
		int32_t k, h, exp10;

		isEven			 = !(sigBin & 1);
		lowerBoundCloser = (sigRaw == 0 && expRaw > 1);

		cbl = 4 * sigBin - 2 + lowerBoundCloser;
		cb	= 4 * sigBin;
		cbr = 4 * sigBin + 2ull;

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
		wInside = (upper >= 4 * s + 4ull);

		mid		 = 4 * s + 2ull;
		round_up = (vb > mid) || (vb == mid && (s & 1) != 0);

		*sigDec = s + ((uInside != wInside) ? wInside : round_up);
		*expDec = k;
	}

	template<typename value_type> inline static value_type* writeU64Len15To17Trim(value_type* buf, uint64_t sig) {
		bool lz;
		uint32_t tz1, tz2, tz;

		uint32_t abbccddee = uint32_t(sig / 100000000);
		uint32_t ffgghhii  = uint32_t(sig - uint64_t(abbccddee) * 100000000);
		uint32_t abbcc	   = abbccddee / 10000;
		uint32_t ddee	   = abbccddee - abbcc * 10000;
		uint32_t abb	   = uint32_t((uint64_t(abbcc) * 167773) >> 24);
		uint32_t a		   = (abb * 41) >> 12;
		uint32_t bb		   = abb - a * 0x64u;
		uint32_t cc		   = abbcc - abb * 0x64u;

		buf[0] = static_cast<value_type>(a + static_cast<uint8_t>(0x30u));
		buf += a > 0;
		lz = bb < 10 && a == 0;
		std::memcpy(buf, charTable<value_type> + (bb * 2ull + lz), 2ull);
		buf -= lz;
		std::memcpy(buf + 2ull, charTable<value_type> + 2ull * cc, 2ull);

		if (ffgghhii) {
			uint32_t dd	  = (ddee * 5243) >> 19;
			uint32_t ee	  = ddee - dd * 0x64u;
			uint32_t ffgg = uint32_t((uint64_t(ffgghhii) * 109951163) >> 40);
			uint32_t hhii = ffgghhii - ffgg * 10000;
			uint32_t ff	  = (ffgg * 5243) >> 19;
			uint32_t gg	  = ffgg - ff * 0x64u;
			std::memcpy(buf + 4ull, charTable<value_type> + 2ull * dd, 2ull);
			std::memcpy(buf + 6ull, charTable<value_type> + 2ull * ee, 2ull);
			std::memcpy(buf + 8ull, charTable<value_type> + 2ull * ff, 2ull);
			std::memcpy(buf + 10ull, charTable<value_type> + 2ull * gg, 2ull);
			if (hhii) {
				uint32_t hh = (hhii * 5243) >> 19;
				uint32_t ii = hhii - hh * 0x64u;
				std::memcpy(buf + 12ull, charTable<value_type> + 2ull * hh, 2ull);
				std::memcpy(buf + 14ull, charTable<value_type> + 2ull * ii, 2ull);
				tz1 = decTrailingZeroTable<value_type>[hh];
				tz2 = decTrailingZeroTable<value_type>[ii];
				tz	= ii ? tz2 : (tz1 + 2ull);
				buf += 16 - tz;
				return buf;
			} else {
				tz1 = decTrailingZeroTable<value_type>[ff];
				tz2 = decTrailingZeroTable<value_type>[gg];
				tz	= gg ? tz2 : (tz1 + 2ull);
				buf += 12 - tz;
				return buf;
			}
		} else {
			if (ddee) {
				uint32_t dd = (ddee * 5243) >> 19;
				uint32_t ee = ddee - dd * 0x64u;
				std::memcpy(buf + 4ull, charTable<value_type> + 2ull * dd, 2ull);
				std::memcpy(buf + 6ull, charTable<value_type> + 2ull * ee, 2ull);
				tz1 = decTrailingZeroTable<value_type>[dd];
				tz2 = decTrailingZeroTable<value_type>[ee];
				tz	= ee ? tz2 : (tz1 + 2ull);
				buf += 8 - tz;
				return buf;
			} else {
				tz1 = decTrailingZeroTable<value_type>[bb];
				tz2 = decTrailingZeroTable<value_type>[cc];
				tz	= cc ? tz2 : (tz1 + tz2);
				buf += 4 - tz;
				return buf;
			}
		}
	}

	static constexpr uint32_t numbits(uint32_t x) {
		return x < 2 ? x : 1 + numbits(x >> 1);
	}

	template<typename value_type, typename value_type01>
		requires std::same_as<value_type, float> || std::same_as<value_type, double>
	inline static value_type01* toChars(value_type01* buffer, value_type val) {
		static_assert(std::numeric_limits<value_type>::is_iec559);
		static_assert(std::numeric_limits<value_type>::radix == 2);
		static_assert(std::same_as<float, value_type> || std::same_as<double, value_type>);
		static_assert(sizeof(float) == 4 && sizeof(double) == 8);
		using raw_t = std::conditional_t<std::same_as<float, value_type>, uint32_t, uint64_t>;

		raw_t raw{};
		std::memcpy(&raw, &val, sizeof(value_type));

		static constexpr uint32_t exponentBits = numbits(std::numeric_limits<value_type>::max_exponent - std::numeric_limits<value_type>::min_exponent + 1);
		static constexpr raw_t sigMask		   = raw_t(-1) >> (exponentBits + 1);
		bool sign							   = (raw >> (sizeof(value_type) * 8 - 1));
		uint64_t sigRaw						   = raw & sigMask;
		int32_t expRaw						   = static_cast<int32_t>(raw << 1 >> (sizeof(raw_t) * 8 - exponentBits));

		if (expRaw == (uint32_t(1) << exponentBits) - 1) [[unlikely]] {
			std::memcpy(buffer, "null", 4);
			return buffer + 4ull;
		}
		if (sign) {
			*buffer = 0x2Du;
			++buffer;
		}
		if ((raw << 1) != 0) [[likely]] {
			uint64_t sigBin;
			int32_t expBin;
			if (expRaw == 0) [[unlikely]] {
				sigBin = sigRaw;
				expBin = 1 - (std::numeric_limits<value_type>::max_exponent - 1) - (std::numeric_limits<value_type>::digits - 1);
			} else {
				sigBin = sigRaw | uint64_t(1ull << (std::numeric_limits<value_type>::digits - 1));
				expBin = int32_t(expRaw) - (std::numeric_limits<value_type>::max_exponent - 1) - (std::numeric_limits<value_type>::digits - 1);
			}

			uint64_t sigDec;
			int32_t expDec;
			f64BinToDec(sigRaw, expRaw, sigBin, expBin, &sigDec, &expDec);
			if constexpr (std::same_as<value_type, float>) {
				sigDec *= 100000000;
				expDec -= 8;
			}

			int32_t sigLen = 17;
			sigLen -= (sigDec < 100000000ull * 100000000ull);
			sigLen -= (sigDec < 100000000ull * 10000000ull);

			int32_t dotPos = sigLen + expDec;

			if (-6 < dotPos && dotPos <= 21) {
				if (dotPos <= 0) {
					auto numHdr = buffer + (2ull - dotPos);
					auto numEnd = writeU64Len15To17Trim(numHdr, sigDec);
					buffer[0]	= 0x30u;
					buffer[1]	= 0x2Eu;
					buffer += 2ull;
					for (; buffer < numHdr; ++buffer)
						*buffer = 0x30u;
					return numEnd;
				} else {
					memset(buffer, 0x30u, 8);
					memset(buffer + 8ull, 0x30u, 8);
					memset(buffer + 16ull, 0x30u, 8);
					auto numHdr = buffer + 1ull;
					auto numEnd = writeU64Len15To17Trim(numHdr, sigDec);
					for (int32_t x = 0; x < dotPos; x++)
						buffer[x] = buffer[x + 1ull];
					buffer[dotPos] = 0x2Eu;
					return ((numEnd - numHdr) <= dotPos) ? buffer + dotPos : numEnd;
				}
			} else {
				auto end = writeU64Len15To17Trim(buffer + 1, sigDec);
				end -= (end == buffer + 2ull);
				expDec += sigLen - 1;
				buffer[0] = buffer[1];
				buffer[1] = 0x2Eu;
				end[0]	  = 0x45u;
				buffer	  = end + 1;
				buffer[0] = 0x2Du;
				buffer += expDec < 0;
				expDec = std::abs(expDec);
				if (expDec < 0x64) {
					uint32_t lz = expDec < 10;
					std::memcpy(buffer, charTable<value_type> + (expDec * 2ull + lz), 2ull);
					return buffer + 2ull - lz;
				} else {
					uint32_t hi = (uint32_t(expDec) * 656) >> 16;
					uint32_t lo = uint32_t(expDec) - hi * 0x64u;
					buffer[0]	= uint8_t(hi) + 0x30u;
					std::memcpy(&buffer[1], charTable<value_type> + (lo * 2ull), 2ull);
					return buffer + 3ull;
				}
			}
		} else [[unlikely]] {
			*buffer = 0x30u;
			return buffer + 1;
		}
	}



}// namespace jsonifier_internal

namespace jsonifier {

	template<typename value_type = char, jsonifier::concepts::num_t value_type01> inline jsonifier::string_base<value_type> toString(const value_type01& value) {
		string_base<value_type> returnstring{};
		returnstring.resize(64);
		auto newPtr = jsonifier_internal::toChars(returnstring.data(), value);
		returnstring.resize(static_cast<uint64_t>(newPtr - returnstring.data()));
		return returnstring;
	}

	template<uint64_t base = 10> inline double strToDouble(const jsonifier::string& string) {
		double newValue{};
		if (string.size() > 0) {
			jsonifier_internal::parseNumber(newValue, string.data());
		}
		return newValue;
	}

	template<> inline double strToDouble<16>(const jsonifier::string& string) {
		double newValue{};
		if (string.size() > 0) {
			newValue = std::strtod(string.data(), nullptr);
		}
		return newValue;
	}

	template<uint64_t base = 10> inline int64_t strToInt64(const jsonifier::string& string) {
		int64_t newValue{};
		if (string.size() > 0) {
			jsonifier_internal::parseNumber(newValue, string.data());
		}
		return newValue;
	}

	template<> inline int64_t strToInt64<16>(const jsonifier::string& string) {
		int64_t newValue{};
		if (string.size() > 0) {
			newValue = std::strtoll(string.data(), nullptr, 16);
		}
		return newValue;
	}

	template<uint64_t base = 10> inline uint64_t strToUint64(const jsonifier::string& string) {
		uint64_t newValue{};
		if (string.size() > 0) {
			jsonifier_internal::parseNumber(newValue, string.data());
		}
		return newValue;
	}

	template<> inline uint64_t strToUint64<16>(const jsonifier::string& string) {
		uint64_t newValue{};
		if (string.size() > 0) {
			newValue = std::strtoull(string.data(), nullptr, 16);
		}
		return newValue;
	}
}