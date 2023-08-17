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
/// https://github.com/RealTimeChris/Jsonifier
/// Feb 3, 2023
/// Most of the code in this header was sampled from Glaze library - https://github.com/stephenberry/glaze
#pragma once

#include <jsonifier/StringUtils.hpp>

#include <array>
#include <concepts>
#include <cstdint>
#include <cstring>

#if defined _M_X64 || _M_ARM64
	#if !defined __linux__
		#include <intrin.h>
	#else
		#include <x86intrin.h>
	#endif
#endif

namespace JsonifierInternal {

	// Based on yyjson: https://github.com/ibireme/yyjson/blob/master/src/yyjson.c with some changes to rounding and
	// dirrect for floats

	// https://stackoverflow.com/questions/28868367/getting-the-high-part-of-64-bit-integer-multiplication
#if defined __SIZEOF_INT128__
	static inline uint64_t mulhi64(uint64_t a, uint64_t b) {
	#if defined __GNUC__ || __GNUG__
		#pragma GCC diagnostic push
		#pragma GCC diagnostic ignored "-Wpedantic"
		unsigned __int128 prod = a * static_cast<unsigned __int128>(b);
		#pragma GCC diagnostic pop
	#endif
		return prod >> 64;
	}
#elif defined _M_X64 || _M_ARM64
	#define mulhi64 __umulh
#else
	static inline uint64_t mulhi64(uint64_t a, uint64_t b) {
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

	static inline void pow10TableGetSig128(int32_t exp10, uint64_t* hi, uint64_t* lo) {
		int32_t idx = exp10 - (pow10SigTable128MinExp);
		*hi			= pow10SigTable128[idx * 2ull];
		*lo			= pow10SigTable128[idx * 2ull + 1ull];
	}

	static inline uint64_t sig2FromExp10(int32_t exp10) {
		return pow10SigTable128[static_cast<uint64_t>(exp10) - pow10SigTable128MinExp];
	}

	static inline int32_t exp2FromExp10(int32_t exp10) {
		return (((exp10 * 217706 - 4128768) >> 16) + 126);
	}

	static constexpr uint8_t digiTypeZero	 = 1 << 0;
	static constexpr uint8_t digiTypeNonZero = 1 << 1;
	static constexpr uint8_t digiTypePos	 = 1 << 2;
	static constexpr uint8_t digiTypeNeg	 = 1 << 3;
	static constexpr uint8_t digiTypeDot	 = 1 << 4;
	static constexpr uint8_t digiTypeExp	 = 1 << 5;

	static inline bool digiIsType(uint8_t d, uint8_t type) {
		return (digiTable[d] & type) != 0;
	}

	static inline bool digiIsFp(uint8_t d) {
		return digiIsType(d, uint8_t(digiTypeDot | digiTypeExp));
	}

	static inline bool digiIsDigitOrFp(uint8_t d) {
		return digiIsType(d, uint8_t(digiTypeZero | digiTypeNonZero | digiTypeDot | digiTypeExp));
	}

#define repeatIIn_1_18(x) \
	{ x(1) x(2) x(3) x(4) x(5) x(6) x(7) x(8) x(9) x(10) x(11) x(12) x(13) x(14) x(15) x(16) x(17) x(18) }

	static constexpr auto eBit		   = static_cast<uint8_t>('E' ^ 'e');
	static constexpr auto f64MaxDecExp = 308;
	static constexpr auto f64MinDecExp = (-324);

	static inline consteval uint32_t ceillog2(uint32_t x) {
		return x < 2 ? x : 1 + ceillog2(x >> 1);
	}

	struct BigIntT {
		Jsonifier::Vector<uint32_t> data = {};

		inline BigIntT(uint64_t num) {
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

		inline auto operator<=>(const BigIntT& rhs) const {
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

	static inline void u128Mul(uint64_t a, uint64_t b, uint64_t* hi, uint64_t* lo) {
#if defined __SIZEOF_INT128__
	#if defined __GNUC__ || __GNUG__
		#pragma GCC diagnostic push
		#pragma GCC diagnostic ignored "-Wpedantic"
	#endif
		unsigned __int128 m = static_cast<unsigned __int128>(a) * b;
	#if defined __GNUC__ || __GNUG__
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

	static inline void u128MulAdd(uint64_t a, uint64_t b, uint64_t c, uint64_t* hi, uint64_t* lo) {
#if defined __SIZEOF_INT128__
	#if defined __GNUC__ || __GNUG__
		#pragma GCC diagnostic push
		#pragma GCC diagnostic ignored "-Wpedantic"
	#endif
		unsigned __int128 m = static_cast<unsigned __int128>(a) * b + c;
	#if defined __GNUC__ || __GNUG__
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

	static inline uint64_t roundToOdd(uint64_t hi, uint64_t lo, uint64_t codePoint) {
		uint64_t xHi, xLo, yHi, yLo;
		u128Mul(codePoint, lo, &xHi, &xLo);
		u128MulAdd(codePoint, hi, xHi, &yHi, &yLo);
		return yHi | (yLo > 1);
	}

	template<typename ValueType> static inline bool parseNumber(ValueType& val, auto* cur) {
		StringViewPtr sigCut				  = nullptr;
		[[maybe_unused]] StringViewPtr sigEnd = nullptr;
		StringViewPtr dotPos				  = nullptr;
		uint32_t fracZeros					  = 0;
		uint64_t sig						  = 0;
		int32_t exp							  = 0;
		bool expSign;
		int32_t expSig = 0;
		int32_t expLit = 0;
		uint64_t numTmp;
		StringViewPtr tmp;
		StringViewPtr hdr = cur;
		bool sign;
		sign = (*hdr == '-');
		cur += sign;
		auto applySign = [&](auto&& val) -> ValueType {
			if constexpr (std::is_unsigned_v<ValueType>) {
				return static_cast<ValueType>(val);
			} else {
				return sign ? -static_cast<ValueType>(val) : static_cast<ValueType>(val);
			}
		};
		sig = uint64_t(*cur - '0');
		if (sig > 9) {
			if constexpr (std::integral<ValueType>) {
				return false;
			} else if (*cur == 'n' && cur[1] == 'u' && cur[2] == 'l' && cur[3] == 'l') {
				cur += 4;
				val = std::numeric_limits<ValueType>::quiet_NaN();
				return true;
			} else if ((*cur | eBit) == 'n' && (cur[1] | eBit) == 'a' && (cur[2] | eBit) == 'n') {
				cur += 3;
				val = sign ? -std::numeric_limits<ValueType>::quiet_NaN() : std::numeric_limits<ValueType>::quiet_NaN();
				return true;
			} else {
				return false;
			}
		}
		static constexpr auto zero = static_cast<uint8_t>('0');
#define expr_intg(x) \
	if ((numTmp = cur[x] - zero) <= 9) [[likely]] \
		sig = numTmp + sig * 10; \
	else { \
		goto digi_sepr_##x; \
	}
		repeatIIn_1_18(expr_intg);
#undef expr_intg
		cur += 19;
		if (!digiIsDigitOrFp(*cur)) {
			val = static_cast<ValueType>(sig);
			if constexpr (!std::is_unsigned_v<ValueType>) {
				val *= sign ? -1 : 1;
			}
			return true;
		}
		goto digi_intg_more;
#define expr_sepr(x) \
	digi_sepr_##x : if ((!digiIsFp(cur[x]))) [[likely]] { \
		cur += x; \
		val = applySign(sig); \
		return true; \
	} \
	dotPos = cur + x; \
	if ((cur[x] == '.')) [[likely]] { \
		if (sig == 0) \
			while (cur[fracZeros + x + 1] == zero) \
				++fracZeros; \
		goto digi_frac_##x; \
	} \
	cur += x; \
	sigEnd = cur; \
	goto digi_exp_more;
		repeatIIn_1_18(expr_sepr)
#undef expr_sepr
#define expr_frac(x) \
	digi_frac_##x : if (((numTmp = static_cast<uint64_t>(cur[x + 1 + fracZeros] - zero)) <= 9)) [[likely]] sig = numTmp + sig * 10; \
	else { \
		goto digi_stop_##x; \
	}
			repeatIIn_1_18(expr_frac)
#undef expr_frac
				cur += 20ull + fracZeros;
		if (uint8_t(*cur - zero) > 9)
			goto digi_frac_end;
		goto digi_frac_more;
#define expr_stop(x) \
	digi_stop_##x : cur += x##ull + 1ull + fracZeros; \
	goto digi_frac_end;
		repeatIIn_1_18(expr_stop)
#undef expr_stop
			digi_intg_more : static constexpr uint64_t u64Max = (std::numeric_limits<uint64_t>::max)();
		if ((numTmp = *cur - zero) < 10) {
			if (!digiIsDigitOrFp(cur[1])) {
				if ((sig < (u64Max / 10)) || (sig == (u64Max / 10) && numTmp <= (u64Max % 10))) {
					sig = numTmp + sig * 10;
					++cur;
					val = static_cast<ValueType>(sig);
					if constexpr (!std::is_unsigned_v<ValueType>) {
						val *= sign ? -1 : 1;
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
		} else {
			sigEnd = cur;
		}
		if ((eBit | *cur) == 'e')
			goto digi_exp_more;
		goto digi_exp_finish;
	digi_frac_end:
		sigEnd = cur;
		expSig = -int32_t((cur - dotPos) - 1);
		if ((eBit | *cur) != 'e') [[likely]] {
			if ((expSig < f64MinDecExp - 19)) [[unlikely]] {
				val = applySign(0);
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
		while (uint8_t(c = *cur - zero) < 10) {
			++cur;
			expLit = c + uint32_t(expLit) * 10;
		}
		if ((cur - tmp >= 6)) [[unlikely]] {
			if (sig == 0 || expSign) {
				val = applySign(0);
				val = static_cast<ValueType>(sig);
				return true;
			} else {
				val = applySign(std::numeric_limits<ValueType>::infinity());
				return true;
			}
		}
		expSig += expSign ? -expLit : expLit;
	digi_exp_finish:
		if constexpr (std::integral<ValueType>) {
			if (sig == 0) {
				val = ((sign && !std::is_unsigned_v<ValueType>) ? -0 : 0);
				return true;
			}
			if (expSig < -20) {
				val = applySign(0);
				return true;
			} else if (expSig > 20) {
				val = applySign(std::numeric_limits<ValueType>::infinity());
				return true;
			}
			exp = expSig;
		} else {
			if (sig == 0) {
				val = (sign ? -ValueType{ 0 } : ValueType{ 0 });
				return true;
			}
			if ((expSig < f64MinDecExp - 19)) [[unlikely]] {
				val = (sign ? -ValueType{ 0 } : ValueType{ 0 });
				return true;
			} else if ((expSig > f64MaxDecExp)) [[unlikely]] {
				val = sign ? -std::numeric_limits<ValueType>::infinity() : std::numeric_limits<ValueType>::infinity();
				return true;
			}
			exp = expSig;
		}
	digi_finish:

		if constexpr (std::integral<ValueType>) {
			val = static_cast<ValueType>(sig);
			if constexpr (!std::is_unsigned_v<ValueType>) {
				val *= sign ? -1 : 1;
			}
			if (exp >= 0) {
				val *= ValueType(powersOfTenInt[exp]);
			} else {
				val /= ValueType(powersOfTenInt[-exp]);
			}
			return true;
		} else {
			if constexpr (std::same_as<double, ValueType>) {
				if (sig < (uint64_t(1) << 53) && std::abs(exp) <= 22) {
					val = static_cast<ValueType>(sig);
					if constexpr (!std::is_unsigned_v<ValueType>) {
						val *= sign ? -1 : 1;
					}
					if (exp >= 0) {
						val *= powersOfTenFloat[exp];
					} else {
						val /= powersOfTenFloat[-exp];
					}
					return true;
				}
			} else {
				if (sig < (uint64_t(1) << 24) && std::abs(exp) <= 8) {
					val = static_cast<ValueType>(sig);
					if constexpr (!std::is_unsigned_v<ValueType>) {
						val *= sign ? -1 : 1;
					}
					if (exp >= 0) {
						val *= static_cast<ValueType>(powersOfTenFloat[exp]);
					} else {
						val /= static_cast<ValueType>(powersOfTenFloat[-exp]);
					}
					return true;
				}
			}

			static_assert(std::numeric_limits<ValueType>::is_iec559);
			static_assert(std::numeric_limits<ValueType>::radix == 2);
			static_assert(std::same_as<float, RefUnwrap<ValueType>> || std::same_as<double, RefUnwrap<ValueType>>);
			static_assert(sizeof(float) == 4 && sizeof(double) == 8);

			using raw_t								= std::conditional_t<std::same_as<float, RefUnwrap<ValueType>>, uint32_t, uint64_t>;
			const auto sigLeadingZeros				= std::countl_zero(sig);
			const auto sigNorm						= sig << sigLeadingZeros;
			const auto sig2Norm						= sig2FromExp10(exp);
			const auto sigProduct					= mulhi64(sigNorm, sig2Norm) + 1;
			const auto sigProductStartsWith1		= sigProduct >> 63;
			auto mantisa							= sigProduct << (2 - sigProductStartsWith1);
			static constexpr uint64_t roundMask		= uint64_t(1) << 63 >> (std::numeric_limits<ValueType>::digits - 1);
			static constexpr uint32_t exponentBits	= ceillog2(std::numeric_limits<ValueType>::max_exponent - std::numeric_limits<ValueType>::min_exponent + 1);
			static constexpr uint32_t mantissaShift = exponentBits + 1 + 64 - 8 * sizeof(raw_t);
			int32_t exp2							= exp2FromExp10(exp) + static_cast<uint32_t>(-sigLeadingZeros + sigProductStartsWith1);

			if (exp2 < std::numeric_limits<ValueType>::min_exponent - 1) [[unlikely]] {
				val = sign ? -ValueType(0) : ValueType(0);
				return true;
			} else if (exp2 > std::numeric_limits<ValueType>::max_exponent - 1) [[unlikely]] {
				val = sign ? -std::numeric_limits<ValueType>::infinity() : std::numeric_limits<ValueType>::infinity();
				return true;
			}

			uint64_t round = 0;
			if (roundMask & mantisa) {
				if (mantisa << (std::numeric_limits<ValueType>::digits) == 0) {
					auto sigUpper	  = (mantisa >> (mantissaShift - 1)) | (uint64_t(1) << 63 >> (mantissaShift - 2)) | 1;
					int32_t exp2Upper = exp2 - std::numeric_limits<ValueType>::digits;

					BigIntT bigComp{ sigUpper };
					BigIntT bigFull{ sig };
					if (exp >= 0) {
						bigFull.mulPow10(exp);
					} else {
						bigComp.mulPow10(-exp);
					}
					if (exp2Upper >= 0) {
						bigComp.mulPow2(exp2Upper);
					} else {
						bigFull.mulPow2(-exp2Upper);
					}
					auto cmp = bigFull <=> bigComp;
					if (cmp != 0) [[likely]] {
						round = (cmp > 0);
					} else {
						round = (mantisa & (roundMask << 1)) != 0;
					}
				} else if ((exp < pow10SigTable128MinExactExp || exp > pow10SigTable128MaxExactExp) || (mantisa & (roundMask << 1)) ||
					(static_cast<uint64_t>(std::countr_zero(sigNorm)) + static_cast<uint64_t>(std::countr_zero(sig2Norm)) <
						128ull - std::numeric_limits<ValueType>::digits - (2ull - sigProductStartsWith1))) {
					round = 1;
				}
			}

			auto num = raw_t(sign) << (sizeof(raw_t) * 8ull - 1ull) | raw_t(mantisa >> mantissaShift) |
				(raw_t(static_cast<uint64_t>(exp2) + std::numeric_limits<ValueType>::max_exponent - 1ull) << (std::numeric_limits<ValueType>::digits - 1ull));
			num += raw_t(round);
			std::memcpy(&val, &num, sizeof(ValueType));
			return true;
		}
	}

	template<UnsignedT ValueType> static inline auto* toChars(auto* buf, ValueType val) {
		uint32_t aa, bb, cc, dd, ee, aabb, bbcc, ccdd, ddee, aabbcc;
		uint32_t lz;

		if (val < 100) {
			lz = val < 10;
			std::memcpy(&buf[0], &charTable[val * 2ull + lz], 2);
			buf -= lz;
			return buf + 2ull;
		} else if (val < 10000) {
			aa = (val * 5243) >> 19;
			bb = val - aa * 100;
			lz = aa < 10;
			std::memcpy(&buf[0], &charTable[aa * 2ull + lz], 2);
			buf -= lz;
			std::memcpy(&buf[2], &charTable[2 * bb], 2);

			return buf + 4ull;
		} else if (val < 1000000) {
			aa	 = uint32_t((uint64_t(val) * 429497) >> 32);
			bbcc = val - aa * 10000;
			bb	 = (bbcc * 5243) >> 19;
			cc	 = bbcc - bb * 100;
			lz	 = aa < 10;
			std::memcpy(buf, charTable + aa * 2ull + lz, 2);
			buf -= lz;
			std::memcpy(buf + 2ull, charTable + bb * 2ull, 2);
			std::memcpy(buf + 4ull, charTable + cc * 2ull, 2);
			return buf + 6ull;
		} else if (val < 100000000) {
			aabb = uint32_t((uint64_t(val) * 109951163) >> 40);
			ccdd = val - aabb * 10000;
			aa	 = (aabb * 5243) >> 19;
			cc	 = (ccdd * 5243) >> 19;
			bb	 = aabb - aa * 100;
			dd	 = ccdd - cc * 100;
			lz	 = aa < 10;
			std::memcpy(buf, charTable + aa * 2ull + lz, 2);
			buf -= lz;
			std::memcpy(buf + 2ull, charTable + bb * 2ull, 2);
			std::memcpy(buf + 4ull, charTable + cc * 2ull, 2);
			std::memcpy(buf + 6ull, charTable + dd * 2ull, 2);
			return buf + 8;
		} else {
			aabbcc = uint32_t((uint64_t(val) * 3518437209ul) >> 45);
			aa	   = uint32_t((uint64_t(aabbcc) * 429497) >> 32);
			ddee   = val - aabbcc * 10000;
			bbcc   = aabbcc - aa * 10000;
			bb	   = (bbcc * 5243) >> 19;
			dd	   = (ddee * 5243) >> 19;
			cc	   = bbcc - bb * 100;
			ee	   = ddee - dd * 100;
			lz	   = aa < 10;
			std::memcpy(buf, charTable + aa * 2ull + lz, 2);
			buf -= lz;
			std::memcpy(buf + 2ull, charTable + bb * 2ull, 2);
			std::memcpy(buf + 4ull, charTable + cc * 2ull, 2);
			std::memcpy(buf + 6ull, charTable + dd * 2ull, 2);
			std::memcpy(buf + 8, charTable + ee * 2ull, 2);
			return buf + 10;
		}
	}

	template<SignedT ValueType> static inline auto* toChars(auto* buf, ValueType val) {
		uint32_t neg  = uint32_t(-val);
		uint64_t sign = val < 0;
		*buf		  = '-';
		return toChars(buf + sign, sign ? uint32_t(neg) : uint32_t(val));
	}

	static inline auto* toCharsU64Len8(auto* buf, uint32_t val) {
		uint32_t aa, bb, cc, dd, aabb, ccdd;
		aabb = uint32_t((uint64_t(val) * 109951163) >> 40);
		ccdd = val - aabb * 10000;
		aa	 = (aabb * 5243) >> 19;
		cc	 = (ccdd * 5243) >> 19;
		bb	 = aabb - aa * 100;
		dd	 = ccdd - cc * 100;
		std::memcpy(buf, charTable + aa * 2ull, 2);
		std::memcpy(buf + 2ull, charTable + bb * 2ull, 2);
		std::memcpy(buf + 4ull, charTable + cc * 2ull, 2);
		std::memcpy(buf + 6ull, charTable + dd * 2ull, 2);
		return buf + 8;
	}

	static inline auto* toCharsU64Len4(auto* buf, uint32_t val) {
		uint32_t aa, bb;
		aa = (val * 5243) >> 19;
		bb = val - aa * 100;
		std::memcpy(buf, charTable + aa * 2ull, 2);
		std::memcpy(buf + 2ull, charTable + bb * 2ull, 2);
		return buf + 4ull;
	}

	static inline auto* toCharsU64Len18(auto* buf, uint32_t val) {
		uint32_t aa, bb, cc, dd, aabb, bbcc, ccdd, lz;

		if (val < 100) {
			lz = val < 10;
			std::memcpy(buf, charTable + val * 2ull + lz, 2);
			buf -= lz;
			return buf + 2ull;
		} else if (val < 10000) {
			aa = (val * 5243) >> 19;
			bb = val - aa * 100;
			lz = aa < 10;
			std::memcpy(buf, charTable + aa * 2ull + lz, 2);
			buf -= lz;
			std::memcpy(buf + 2ull, charTable + bb * 2ull, 2);
			return buf + 4ull;
		} else if (val < 1000000) {
			aa	 = uint32_t((uint64_t(val) * 429497) >> 32);
			bbcc = val - aa * 10000;
			bb	 = (bbcc * 5243) >> 19;
			cc	 = bbcc - bb * 100;
			lz	 = aa < 10;
			std::memcpy(buf, charTable + aa * 2ull + lz, 2);
			buf -= lz;
			std::memcpy(buf + 2ull, charTable + bb * 2ull, 2);
			std::memcpy(buf + 4ull, charTable + cc * 2ull, 2);
			return buf + 6ull;
		} else {
			aabb = uint32_t((uint64_t(val) * 109951163) >> 40);
			ccdd = val - aabb * 10000;
			aa	 = (aabb * 5243) >> 19;
			cc	 = (ccdd * 5243) >> 19;
			bb	 = aabb - aa * 100;
			dd	 = ccdd - cc * 100;
			lz	 = aa < 10;
			std::memcpy(buf, charTable + aa * 2ull + lz, 2);
			buf -= lz;
			std::memcpy(buf + 2ull, charTable + bb * 2ull, 2);
			std::memcpy(buf + 4ull, charTable + cc * 2ull, 2);
			std::memcpy(buf + 6ull, charTable + dd * 2ull, 2);
			return buf + 8;
		}
	}

	static inline auto* toCharsU64Len58(auto* buf, uint32_t val) {
		uint32_t aa, bb, cc, dd, aabb, bbcc, ccdd, lz;

		if (val < 1000000) {
			aa	 = uint32_t((uint64_t(val) * 429497) >> 32);
			bbcc = val - aa * 10000;
			bb	 = (bbcc * 5243) >> 19;
			cc	 = bbcc - bb * 100;
			lz	 = aa < 10;
			std::memcpy(buf, charTable + aa * 2ull + lz, 2);
			buf -= lz;
			std::memcpy(buf + 2ull, charTable + bb * 2ull, 2);
			std::memcpy(buf + 4ull, charTable + cc * 2ull, 2);
			return buf + 6ull;
		} else {
			aabb = uint32_t((uint64_t(val) * 109951163) >> 40);
			ccdd = val - aabb * 10000;
			aa	 = (aabb * 5243) >> 19;
			cc	 = (ccdd * 5243) >> 19;
			bb	 = aabb - aa * 100;
			dd	 = ccdd - cc * 100;
			lz	 = aa < 10;
			std::memcpy(buf, charTable + aa * 2ull + lz, 2);
			buf -= lz;
			std::memcpy(buf + 2ull, charTable + bb * 2ull, 2);
			std::memcpy(buf + 4ull, charTable + cc * 2ull, 2);
			std::memcpy(buf + 6ull, charTable + dd * 2ull, 2);
			return buf + 8;
		}
	}

	template<UnsignedT ValueType>
		requires(sizeof(ValueType) == 8)
	static inline auto* toChars(auto* buf, ValueType val) {
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

	template<SignedT ValueType>
		requires(sizeof(ValueType) == 8)
	static inline auto* toChars(auto* buf, ValueType val) {
		uint64_t neg  = uint64_t(-val);
		uint64_t sign = val < 0;
		*buf		  = '-';
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
	static inline void f64BinToDec(uint64_t sigRaw, int32_t expRaw, uint64_t sigBin, int32_t expBin, uint64_t* sigDec, int32_t* expDec) {
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

	static inline char* writeU64Len15To17Trim(char* buf, uint64_t sig) {
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

		buf[0] = uint8_t(a + '0');
		buf += a > 0;
		lz = bb < 10 && a == 0;
		std::memcpy(buf, charTable + (bb * 2ull + lz), 2ull);
		buf -= lz;
		std::memcpy(buf + 2ull, charTable + 2ull * cc, 2ull);

		if (ffgghhii) {
			uint32_t dd	  = (ddee * 5243) >> 19;
			uint32_t ee	  = ddee - dd * 100;
			uint32_t ffgg = uint32_t((uint64_t(ffgghhii) * 109951163) >> 40);
			uint32_t hhii = ffgghhii - ffgg * 10000;
			uint32_t ff	  = (ffgg * 5243) >> 19;
			uint32_t gg	  = ffgg - ff * 100;
			std::memcpy(buf + 4ull, charTable + 2ull * dd, 2ull);
			std::memcpy(buf + 6ull, charTable + 2ull * ee, 2ull);
			std::memcpy(buf + 8ull, charTable + 2ull * ff, 2ull);
			std::memcpy(buf + 10ull, charTable + 2ull * gg, 2ull);
			if (hhii) {
				uint32_t hh = (hhii * 5243) >> 19;
				uint32_t ii = hhii - hh * 100;
				std::memcpy(buf + 12ull, charTable + 2ull * hh, 2ull);
				std::memcpy(buf + 14ull, charTable + 2ull * ii, 2ull);
				tz1 = decTrailingZeroTable[hh];
				tz2 = decTrailingZeroTable[ii];
				tz	= ii ? tz2 : (tz1 + 2ull);
				buf += 16 - tz;
				return buf;
			} else {
				tz1 = decTrailingZeroTable[ff];
				tz2 = decTrailingZeroTable[gg];
				tz	= gg ? tz2 : (tz1 + 2ull);
				buf += 12 - tz;
				return buf;
			}
		} else {
			if (ddee) {
				uint32_t dd = (ddee * 5243) >> 19;
				uint32_t ee = ddee - dd * 100;
				std::memcpy(buf + 4ull, charTable + 2ull * dd, 2ull);
				std::memcpy(buf + 6ull, charTable + 2ull * ee, 2ull);
				tz1 = decTrailingZeroTable[dd];
				tz2 = decTrailingZeroTable[ee];
				tz	= ee ? tz2 : (tz1 + 2ull);
				buf += 8 - tz;
				return buf;
			} else {
				tz1 = decTrailingZeroTable[bb];
				tz2 = decTrailingZeroTable[cc];
				tz	= cc ? tz2 : (tz1 + tz2);
				buf += 4 - tz;
				return buf;
			}
		}
	}

	static constexpr uint32_t numbits(uint32_t x) {
		return x < 2 ? x : 1 + numbits(x >> 1);
	}

	template<typename ValueType>
		requires std::same_as<ValueType, float> || std::same_as<ValueType, double>
	static inline char* toChars(char* buffer, ValueType val) {
		static_assert(std::numeric_limits<ValueType>::is_iec559);
		static_assert(std::numeric_limits<ValueType>::radix == 2);
		static_assert(std::same_as<float, ValueType> || std::same_as<double, ValueType>);
		static_assert(sizeof(float) == 4 && sizeof(double) == 8);
		using raw_t = std::conditional_t<std::same_as<float, ValueType>, uint32_t, uint64_t>;

		raw_t raw{};
		std::memcpy(&raw, &val, sizeof(ValueType));

		static constexpr uint32_t exponentBits = numbits(std::numeric_limits<ValueType>::max_exponent - std::numeric_limits<ValueType>::min_exponent + 1);
		static constexpr raw_t sigMask		   = raw_t(-1) >> (exponentBits + 1);
		bool sign							   = (raw >> (sizeof(ValueType) * 8 - 1));
		uint64_t sigRaw						   = raw & sigMask;
		int32_t expRaw						   = raw << 1 >> (sizeof(raw_t) * 8 - exponentBits);

		if (expRaw == (uint32_t(1) << exponentBits) - 1) [[unlikely]] {
			std::memcpy(buffer, "null", 4);
			return buffer + 4ull;
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
				expBin = 1 - (std::numeric_limits<ValueType>::max_exponent - 1) - (std::numeric_limits<ValueType>::digits - 1);
			} else {
				sigBin = sigRaw | uint64_t(1ull << (std::numeric_limits<ValueType>::digits - 1));
				expBin = int32_t(expRaw) - (std::numeric_limits<ValueType>::max_exponent - 1) - (std::numeric_limits<ValueType>::digits - 1);
			}

			uint64_t sigDec;
			int32_t expDec;
			f64BinToDec(sigRaw, expRaw, sigBin, expBin, &sigDec, &expDec);
			if constexpr (std::same_as<ValueType, float>) {
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
					buffer[0]	= '0';
					buffer[1]	= '.';
					buffer += 2ull;
					for (; buffer < numHdr; ++buffer)
						*buffer = '0';
					return numEnd;
				} else {
					memset(buffer, '0', 8);
					memset(buffer + 8ull, '0', 8);
					memset(buffer + 16ull, '0', 8);
					auto numHdr = buffer + 1ull;
					auto numEnd = writeU64Len15To17Trim(numHdr, sigDec);
					for (int32_t x = 0; x < dotPos; x++)
						buffer[x] = buffer[x + 1ull];
					buffer[dotPos] = '.';
					return ((numEnd - numHdr) <= dotPos) ? buffer + dotPos : numEnd;
				}
			} else {
				auto end = writeU64Len15To17Trim(buffer + 1, sigDec);
				end -= (end == buffer + 2ull);
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
					std::memcpy(buffer, charTable + (expDec * 2ull + lz), 2ull);
					return buffer + 2ull - lz;
				} else {
					uint32_t hi = (uint32_t(expDec) * 656) >> 16;
					uint32_t lo = uint32_t(expDec) - hi * 100;
					buffer[0]	= uint8_t(hi) + '0';
					std::memcpy(&buffer[1], charTable + (lo * 2ull), 2ull);
					return buffer + 3ull;
				}
			}
		} else [[unlikely]] {
			*buffer = '0';
			return buffer + 1;
		}
	}



}// namespace JsonifierInternal

namespace Jsonifier {

	template<JsonifierInternal::NumT ValueType01> Jsonifier::StringBase<char> toString(const ValueType01& value) {
		StringBase<char> returnString{};
		returnString.resize(64);
		auto newPtr = JsonifierInternal::toChars<ValueType01>(returnString.data(), value);
		returnString.resize(newPtr - returnString.data());
		return returnString;
	}

}
