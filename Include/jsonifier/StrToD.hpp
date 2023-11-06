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

#if defined(_M_X64) || defined(_M_ARM64)
	#include <intrin.h>
#endif

namespace jsonifier_internal {

	constexpr uint8_t digiTypeZero	  = 1 << 0;
	constexpr uint8_t digiTypeNonZero = 1 << 1;
	constexpr uint8_t digiTypePos	  = 1 << 2;
	constexpr uint8_t digiTypeNeg	  = 1 << 3;
	constexpr uint8_t digiTypeDot	  = 1 << 4;
	constexpr uint8_t digiTypeExp	  = 1 << 5;

#if defined(__SIZEOF_INT128__) && (__SIZEOF_INT128__ == 16) && (defined(__GNUC__) || defined(__clang__) || defined(__INTEL_COMPILER))
	inline uint64_t mulhi64(uint64_t a, uint64_t b) {
	#if defined(__GNUC__) || defined(__GNUG__)
		#pragma GCC diagnostic push
		#pragma GCC diagnostic ignored "-Wpedantic"
		unsigned __int128 prod = a * static_cast<unsigned __int128>(b);
		#pragma GCC diagnostic pop
	#else
		unsigned __int128 prod = a * static_cast<unsigned __int128>(b);
	#endif
		return prod >> 64;
	}
#elif defined(_M_X64) || defined(_M_ARM64)
	#define mulhi64 __umulh
#else
	inline uint64_t mulhi64(uint64_t a, uint64_t b) noexcept {
		uint64_t a_lo	   = ( uint64_t )a;
		uint64_t a_hi	   = a >> 32;
		uint64_t b_lo	   = ( uint64_t )b;
		uint64_t b_hi	   = b >> 32;
		uint64_t a_x_b_hi  = a_hi * b_hi;
		uint64_t a_x_b_mid = a_hi * b_lo;
		uint64_t b_x_a_mid = b_hi * a_lo;
		uint64_t a_x_b_lo  = a_lo * b_lo;
		uint64_t carry_bit = (( uint64_t )( uint64_t )a_x_b_mid + ( uint64_t )( uint64_t )b_x_a_mid + (a_x_b_lo >> 32)) >> 32;
		uint64_t multhi	   = a_x_b_hi + (a_x_b_mid >> 32) + (b_x_a_mid >> 32) + carry_bit;
		return multhi;
	}
#endif

	consteval uint64_t ceillog2(uint64_t x) {
		return x < 2 ? x : 1 + ceillog2(x >> 1);
	}

	inline uint64_t sig2FromExp10(int64_t exp10) noexcept {
		return pow10SigTable[exp10 - pow10SigTableMinExp];
	}

	inline int64_t exp2FromExp10(int64_t exp10) noexcept {
		return (((exp10 * 217706 - 4128768) >> 16) + 126);
	}

	struct big_int_t {
		jsonifier::vector<uint64_t> data{};

		inline big_int_t(uint64_t num) {
			uint64_t lowerWord = uint64_t(num);
			uint64_t upperWord = uint64_t(num >> 32);
			if (upperWord > 0) {
				data = { lowerWord, upperWord };
			} else {
				data = { lowerWord };
			}
		}

		inline void mulU32(uint64_t num) {
			uint64_t carry = 0;
			for (uint64_t x = 0; x < data.size(); x++) {
				uint64_t res	   = uint64_t(data[x]) * uint64_t(num) + uint64_t(carry);
				uint64_t lowerWord = uint64_t(res);
				uint64_t upperWord = uint64_t(res >> 32);
				data[x]			   = lowerWord;
				carry			   = upperWord;
			}
			if (carry != 0) {
				data.emplace_back(carry);
			}
		}

		inline void mulPow10(uint64_t pow10) {
			for (; pow10 >= 9; pow10 -= 9) {
				mulU32(static_cast<uint64_t>(powersOfTenInt[9]));
			}
			if (pow10) {
				mulU32(static_cast<uint64_t>(powersOfTenInt[pow10]));
			}
		}

		inline void mulPow2(uint64_t exp) {
			uint64_t shft = exp % 32;
			uint64_t move = exp / 32;
			uint64_t idx  = static_cast<uint64_t>(data.size()) - 1;
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
					uint64_t num = data[idx] << shft;
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

#define repeat_in_1_18(x) \
	{ x(1) x(2) x(3) x(4) x(5) x(6) x(7) x(8) x(9) x(10) x(11) x(12) x(13) x(14) x(15) x(16) x(17) x(18) }

	constexpr auto eBit			= static_cast<uint8_t>('E' ^ 'e');
	constexpr auto f64MaxDecExp = 308;
	constexpr auto f64MinDecExp = (-324);

	inline bool digiIsType(uint8_t d, uint8_t type) noexcept {
		return (digiTable[d] & type) != 0;
	}

	inline bool digiIsFp(uint8_t d) noexcept {
		return digiIsType(d, uint8_t(digiTypeDot | digiTypeExp));
	}

	inline bool digiIsDigitOrFp(uint8_t d) noexcept {
		return digiIsType(d, uint8_t(digiTypeZero | digiTypeNonZero | digiTypeDot | digiTypeExp));
	}

	template<std::floating_point value_type01, typename value_type> inline bool parseNumber(value_type01& val, value_type* curNew) noexcept {
		const value_type* cur{ curNew };
		const value_type* sigCut{};
		[[maybe_unused]] const value_type* sigEnd{};
		const value_type* dotPos{};
		uint64_t fracZeros{};
		uint64_t sig{};
		int64_t exp{};
		bool expSign{};
		int64_t expSig{};
		int64_t expLit{};
		uint64_t numTmp{};
		const value_type* tmp{};
		const value_type* hdr{ cur };
		bool sign{ (*hdr == '-') };
		cur += sign;
		auto applySign = [&](auto&& val) -> value_type01 {
			return sign ? -static_cast<value_type01>(val) : static_cast<value_type01>(val);
		};

		sig = static_cast<uint64_t>(*cur) - static_cast<uint64_t>('0');
		if (sig > 9) {
			if constexpr (std::integral<value_type01>) {
				return false;
			} else if (*cur == 'n' && cur[1] == 'u' && cur[2] == 'l' && cur[3] == 'l') {
				cur += 4;
				val = std::numeric_limits<value_type01>::quiet_NaN();
				return true;
			} else if ((*cur | eBit) == 'n' && (cur[1] | eBit) == 'a' && (cur[2] | eBit) == 'n') {
				cur += 3;
				val = sign ? -std::numeric_limits<value_type01>::quiet_NaN() : std::numeric_limits<value_type01>::quiet_NaN();
				return true;
			} else {
				return false;
			}
		}
		constexpr auto zero = static_cast<uint8_t>('0');
#define expr_intg(i) \
	if ((numTmp = static_cast<uint64_t>(cur[i]) - static_cast<uint64_t>(zero)) <= 9ULL) [[likely]] \
		sig = numTmp + sig * 10ULL; \
	else { \
		if constexpr (i > 1ULL) { \
			if (*cur == zero) \
				return false; \
		} \
		goto digi_sepr_##i; \
	}
		repeat_in_1_18(expr_intg);
#undef expr_intg
		if (*cur == zero) [[unlikely]] {
			return false;
		}
		cur += 19;
		if (!digiIsDigitOrFp(static_cast<uint8_t>(*cur))) {
			val = static_cast<value_type01>(sig);
			if constexpr (!std::is_unsigned_v<value_type01>) {
				val *= sign ? -1 : 1;
			}
			return true;
		}
		goto digi_intg_more;

#define expr_sepr(i) \
	digi_sepr_##i : if ((!digiIsFp(static_cast<uint8_t>(cur[i])))) [[likely]] { \
		cur += i; \
		val = applySign(sig); \
		return true; \
	} \
	dotPos = cur + i; \
	if ((cur[i] == '.')) [[likely]] { \
		if (sig == 0) \
			while (cur[fracZeros + i + 1] == zero) \
				++fracZeros; \
		goto digi_frac_##i; \
	} \
	cur += i; \
	sigEnd = cur; \
	goto digi_exp_more;
		repeat_in_1_18(expr_sepr)
#undef expr_sepr

#define expr_frac(i) \
	digi_frac_##i : if (((numTmp = static_cast<uint64_t>(cur[i + 1 + fracZeros]) - static_cast<uint64_t>(zero)) <= 9)) [[likely]] sig = numTmp + sig * 10ULL; \
	else { \
		goto digi_stop_##i; \
	}
			repeat_in_1_18(expr_frac)
#undef expr_frac
				cur += 20ULL + fracZeros;
		if (uint8_t(*cur - zero) > 9)
			goto digi_frac_end;
		goto digi_frac_more;

#define expr_stop(i) \
	digi_stop_##i : cur += i + 1 + fracZeros; \
	goto digi_frac_end;
		repeat_in_1_18(expr_stop)
#undef expr_stop

			digi_intg_more : static constexpr uint64_t U64_MAX = (std::numeric_limits<uint64_t>::max)();
		if ((numTmp = static_cast<uint64_t>(*cur) - static_cast<uint64_t>(zero)) < 10) {
			if (!digiIsDigitOrFp(static_cast<uint8_t>(cur[1]))) {
				if ((sig < (U64_MAX / 10)) || (sig == (U64_MAX / 10) && numTmp <= (U64_MAX % 10))) {
					sig = numTmp + sig * 10;
					cur++;
					val = static_cast<value_type01>(sig);
					if constexpr (!std::is_unsigned_v<value_type01>) {
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
		expSig = static_cast<int64_t>(dotPos - sigCut);
		expSig += (dotPos < sigCut);
		tmp = cur - 1;
		while (*tmp == '0' || *tmp == '.') {
			--tmp;
		}
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
		expSig = -int64_t((cur - dotPos) - 1);
		if (expSig == 0)
			return false;
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
			return false;
		}
		while (*cur == '0') {
			++cur;
		}
		tmp = cur;
		uint8_t c;
		while (uint8_t(c = static_cast<uint8_t>(*cur - zero)) < 10) {
			++cur;
			expLit = static_cast<int64_t>(c + uint64_t(expLit) * 10);
		}
		if ((cur - tmp >= 6)) [[unlikely]] {
			if (sig == 0 || expSign) {
				val = applySign(0);
				val = static_cast<value_type01>(sig);
				return true;
			} else {
				val = applySign(std::numeric_limits<value_type01>::infinity());
				return true;
			}
		}
		expSig += expSign ? -expLit : expLit;
	digi_exp_finish:
		if (sig == 0) {
			val = (sign ? -value_type01{ 0 } : value_type01{ 0 });
			return true;
		}
		if ((expSig < f64MinDecExp - 19)) [[unlikely]] {
			val = (sign ? -value_type01{ 0 } : value_type01{ 0 });
			return true;
		} else if ((expSig > f64MaxDecExp)) [[unlikely]] {
			val = sign ? -std::numeric_limits<value_type01>::infinity() : std::numeric_limits<value_type01>::infinity();
			return true;
		}
		exp = expSig;
	digi_finish:

		if constexpr (std::is_same_v<double, value_type01>) {
			if (sig < (uint64_t(1) << 53) && std::abs(exp) <= 22) {
				val = static_cast<value_type01>(sig);
				if constexpr (!std::is_unsigned_v<value_type01>) {
					val *= sign ? -1 : 1;
				}
				if (exp >= 0 && exp < 23) {
					val *= powersOfTenFloat[exp];
				} else if (-exp < 23) {
					val /= powersOfTenFloat[-exp];
				}
				return true;
			}
		} else {
			if (sig < (uint64_t(1) << 24) && std::abs(exp) <= 8) {
				val = static_cast<value_type01>(sig);
				if constexpr (!std::is_unsigned_v<value_type01>) {
					val *= sign ? -1 : 1;
				}
				if (exp >= 0 && exp < 23) {
					val *= powersOfTenFloat[exp];
				} else if (-exp < 23) {
					val /= powersOfTenFloat[-exp];
				}
				return true;
			}
		}

		if (sig == 0) [[unlikely]] {
			val = value_type01(0);
			return true;
		}

		static_assert(std::numeric_limits<value_type01>::is_iec559);
		static_assert(std::numeric_limits<value_type01>::radix == 2);
		static_assert(std::is_same_v<float, std::decay_t<value_type01>> || std::is_same_v<double, std::decay_t<value_type01>>);
		static_assert(sizeof(float) == 4 && sizeof(double) == 8);

		using raw_t						 = std::conditional_t<std::is_same_v<float, std::decay_t<value_type01>>, uint64_t, uint64_t>;
		const auto sigLeadingZeros		 = std::countl_zero(sig);
		const auto sigNorm				 = sig << sigLeadingZeros;
		const auto sig2Norm				 = sig2FromExp10(exp);
		const auto sigProduct			 = mulhi64(sigNorm, sig2Norm) + 1;
		const auto sigProductStartsWith1 = sigProduct >> 63;
		auto mantisa					 = sigProduct << (2 - sigProductStartsWith1);
		constexpr uint64_t roundMask	 = uint64_t(1) << 63 >> (std::numeric_limits<value_type01>::digits - 1);
		constexpr uint64_t exponentBits	 = ceillog2(std::numeric_limits<value_type01>::max_exponent - std::numeric_limits<value_type01>::min_exponent + 1);
		constexpr uint64_t mantissaShift = exponentBits + 1 + 64 - 8 * sizeof(raw_t);
		int64_t exp2					 = exp2FromExp10(exp) + static_cast<int64_t>(-sigLeadingZeros + sigProductStartsWith1);

		if (exp2 < std::numeric_limits<value_type01>::min_exponent - 1) [[unlikely]] {
			val = sign ? -value_type01(0) : value_type01(0);
			return true;
		} else if (exp2 > std::numeric_limits<value_type01>::max_exponent - 1) [[unlikely]] {
			val = sign ? -std::numeric_limits<value_type01>::infinity() : std::numeric_limits<value_type01>::infinity();
			return true;
		}

		uint64_t round = 0;
		if (roundMask & mantisa) {
			if (mantisa << (std::numeric_limits<value_type01>::digits) == 0) {
				auto sigUpper	  = (mantisa >> (mantissaShift - 1)) | (uint64_t(1) << 63 >> (mantissaShift - 2)) | 1;
				int64_t exp2Upper = exp2 - std::numeric_limits<value_type01>::digits;

				big_int_t bigComp{ sigUpper };
				big_int_t bigFull{ sig };
				if (exp >= 0) {
					bigFull.mulPow10(static_cast<uint64_t>(exp));
				} else {
					bigComp.mulPow10(static_cast<uint64_t>(-exp));
				}
				if (exp2Upper >= 0) {
					bigComp.mulPow2(static_cast<uint64_t>(exp2Upper));
				} else {
					bigFull.mulPow2(static_cast<uint64_t>(-exp2Upper));
				}
				auto cmp = bigFull <=> bigComp;
				if (cmp != 0) [[likely]] {
					round = (cmp > 0);
				} else {
					round = (mantisa & (roundMask << 1)) != 0;
				}
			} else if ((exp < pow10SigTableMinExactExp || exp > pow10SigTableMaxExactExp) || (mantisa & (roundMask << 1)) ||
				(static_cast<uint64_t>(std::countr_zero(sigNorm)) + static_cast<uint64_t>(std::countr_zero(sig2Norm)) <
					128 - std::numeric_limits<value_type01>::digits - (2 - sigProductStartsWith1))) {
				round = 1;
			}
		}

		auto num = raw_t(sign) << (sizeof(raw_t) * 8 - 1) | raw_t(mantisa >> mantissaShift) |
			(raw_t(exp2 + std::numeric_limits<value_type01>::max_exponent - 1) << (std::numeric_limits<value_type01>::digits - 1));
		num += raw_t(round);
		std::memcpy(&val, &num, sizeof(value_type01));
		return true;
	}
}