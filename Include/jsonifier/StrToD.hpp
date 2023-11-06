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

// https://stackoverflow.com/questions/28868367/getting-the-high-part-of-64-bit-integer-multiplication
#if defined(__SIZEOF_INT128__)
	JSONIFIER_INLINE uint64_t mulhi64(uint64_t a, uint64_t b) {
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
	JSONIFIER_INLINE uint64_t mulhi64(uint64_t a, uint64_t b) noexcept {
		uint64_t aLo	  = a;
		uint64_t aHi	  = a >> 32;
		uint64_t bLo	  = b;
		uint64_t bHi	  = b >> 32;
		uint64_t axbHi	  = aHi * bHi;
		uint64_t axbMid	  = aHi * bLo;
		uint64_t bxaMid	  = bHi * aLo;
		uint64_t axbLo	  = aLo * bLo;
		uint64_t carryBit = axbMid + bxaMid + (axbLo >> 32) >> 32;
		uint64_t multhi	  = axbHi + (axbMid >> 32) + (bxaMid >> 32) + carryBit;
		return multhi;
	}
#endif

	constexpr int64_t pow10SigTableMinExp	= -343;
	constexpr int64_t pow10SigTableMaxExp	= 324;
	constexpr int64_t pow10SigTableMinExact = 0;
	constexpr int64_t pow10SigTableMaxExact = 27;

	JSONIFIER_INLINE uint64_t sig2FromExp10(uint64_t exp10) noexcept {
		return pow10SigTable[exp10 - pow10SigTableMinExp];
	}

	JSONIFIER_INLINE int64_t exp2FromExp10(int64_t exp10) noexcept {
		return (((exp10 * 217706 - 4128768) >> 16) + 126);
	}

	constexpr uint8_t digiTypeZero	  = 1 << 0;
	constexpr uint8_t digiTypeNonZero = 1 << 1;
	constexpr uint8_t digiTypeNeg	  = 1 << 3;
	constexpr uint8_t digiTypeDot	  = 1 << 4;
	constexpr uint8_t digiTypeExp	  = 1 << 5;

	JSONIFIER_INLINE bool digiIsType(uint8_t d, uint8_t type) noexcept {
		return (digiTable[d] & type) != 0;
	}

	JSONIFIER_INLINE bool digiIsFp(uint8_t d) noexcept {
		return digiIsType(d, uint8_t(digiTypeDot | digiTypeExp));
	}

	JSONIFIER_INLINE bool digiIsDigitOrFp(uint8_t d) noexcept {
		return digiIsType(d, uint8_t(digiTypeZero | digiTypeNonZero | digiTypeDot | digiTypeExp));
	}

#define repeat_in_1_18(x) \
	{ x(1) x(2) x(3) x(4) x(5) x(6) x(7) x(8) x(9) x(10) x(11) x(12) x(13) x(14) x(15) x(16) x(17) x(18) }

	constexpr uint8_t eBit		   = static_cast<uint8_t>(0x45u ^ 0x65u);
	constexpr int64_t f64MaxDecExp = 308;
	constexpr int64_t f64MinDecExp = (-324);

	consteval uint64_t ceillog2(uint64_t x) {
		return x < 2 ? x : 1 + ceillog2(x >> 1);
	}

	struct big_int_t final {
		std::vector<uint64_t> data = {};

		big_int_t(uint64_t num) noexcept {
			uint64_t lowerWord = num;
			uint64_t upperWord = num >> 32;
			if (upperWord > 0) {
				data = { lowerWord, upperWord };
			} else {
				data = { lowerWord };
			}
		}

		void mulU32(uint64_t num) noexcept {
			uint64_t carry = 0;
			for (std::size_t x = 0; x < data.size(); x++) {
				uint64_t res	   = data[x] * num + carry;
				uint64_t lowerWord = res;
				uint64_t upperWord = res >> 32;
				data[x]			   = lowerWord;
				carry			   = upperWord;
			}
			if (carry != 0) {
				data.emplace_back(carry);
			}
		}

		void mulPow10(uint64_t pow10) noexcept {
			for (; pow10 >= 9; pow10 -= 9) {
				mulU32(static_cast<uint64_t>(powersOfTenInt[9]));
			}
			if (pow10) {
				mulU32(static_cast<uint64_t>(powersOfTenInt[pow10]));
			}
		}

		void mulPow2(uint64_t exp) noexcept {
			uint64_t shft = exp % 32;
			uint64_t move = exp / 32;
			uint64_t idx  = static_cast<uint64_t>(data.size()) - 1;
			if (shft == 0) {
				data.resize(data.size() + move);
				for (; idx > 0; idx--) {
					data[idx + move - 1] = data[idx - 1];
				}
				while (move)
					data[--move] = 0;
			} else {
				data.resize(data.size() + move + 1);
				++idx;
				for (; idx > 0; idx--) {
					uint64_t num = data[idx] << shft;
					num |= data[idx - 1] >> (32 - shft);
					data[idx + move] = num;
				}
				data[move] = data[0] << shft;
				if (data.back() == 0)
					data.pop_back();
				while (move)
					data[--move] = 0;
			}
		}

		auto operator<=>(const big_int_t& rhs) const noexcept {
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

	template<jsonifier::concepts::float_t value_type01, typename char_type> JSONIFIER_INLINE bool parseNumber(value_type01& value, char_type* cur) noexcept {
		[[maybe_unused]] const char_type* sigEnd{};
		const char_type *tmp{}, *sigCut{}, *dotPos{}, *hdr{ cur };
		uint64_t fracZeros{}, numTmp{}, sig{};
		int64_t exp{}, expSig{}, expLit{};
		bool sign{ (*hdr == 0x2Du) }, expSign{};
		cur += sign;
		auto applySign = [&](auto&& value) -> value_type01 {
			return sign ? -static_cast<value_type01>(value) : static_cast<value_type01>(value);
		};

		sig = static_cast<uint64_t>(asciiToValueTable[static_cast<uint64_t>(*cur)]);
		if (sig > 9) {
			if (*cur == 0x6Eu && cur[1] == 0x75u && cur[2] == 0x6Cu && cur[3] == 0x6Cu) {
				cur += 4;
				value = 0;
				return true;
			} else if ((*cur | eBit) == 0x6Eu && (cur[1] | eBit) == 0x61u && (cur[2] | eBit) == 0x6Eu) {
				cur += 3;
				value = sign ? -std::numeric_limits<value_type01>::quiet_NaN() : std::numeric_limits<value_type01>::quiet_NaN();
				return true;
			} else {
				return false;
			}
		}
		constexpr auto zero = static_cast<uint8_t>(0x30u);
#define expr_intg(x) \
	if ((numTmp = static_cast<uint64_t>(asciiToValueTable[static_cast<uint64_t>(cur[x])])) <= 9) [[likely]] \
		sig = numTmp + sig * 10; \
	else { \
		if (*cur == zero) \
			return false; \
		goto digi_sepr_##x; \
	}
		repeat_in_1_18(expr_intg);
#undef expr_intg
		if (*cur == zero) [[unlikely]] {
			return false;
		}
		cur += 19;
		if (!digiIsDigitOrFp(static_cast<uint8_t>(*cur))) {
			value = static_cast<value_type01>(sig);
			if constexpr (!std::unsigned_integral<value_type01>) {
				value *= sign ? -1 : 1;
			}
			return true;
		}
		goto digi_intg_more;
#define expr_sepr(x) \
	digi_sepr_##x : if ((!digiIsFp(static_cast<uint8_t>(cur[x])))) [[likely]] { \
		cur += x; \
		value = applySign(sig); \
		return true; \
	} \
	dotPos = cur + x; \
	if ((cur[x] == 0x2Eu)) [[likely]] { \
		if (sig == 0) \
			while (cur[fracZeros + x + 1] == zero) \
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
	digi_stop_##x : cur += x + 1 + fracZeros; \
	goto digi_frac_end;
		repeat_in_1_18(expr_stop)
#undef expr_stop
			digi_intg_more : static constexpr uint64_t u64Max = (std::numeric_limits<uint64_t>::max)();
		if (numTmp = static_cast<uint64_t>(asciiToValueTable[static_cast<uint64_t>(*cur)]); numTmp < 10) {
			if (!digiIsDigitOrFp(static_cast<uint8_t>(cur[1]))) {
				if ((sig < (u64Max / 10)) || (sig == (u64Max / 10) && numTmp <= (u64Max % 10))) {
					sig = numTmp + sig * 10;
					cur++;
					value = static_cast<value_type01>(sig);
					if constexpr (!std::unsigned_integral<value_type01>) {
						value *= sign ? -1 : 1;
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
			if (asciiToValueTable[static_cast<uint64_t>(*cur)] > 9) {
				return false;
			}
		}
	digi_frac_more:
		sigCut = cur;
		sig += (*cur >= 0x35u);
		while (asciiToValueTable[*++cur] < 10) {
		}
		if (!dotPos) {
			dotPos = cur;
			if (*cur == 0x2Eu) {
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
		while (*tmp == 0x30u || *tmp == 0x2Eu) {
			--tmp;
		}
		if (tmp < sigCut) {
			sigCut = nullptr;
		} else {
			sigEnd = cur;
		}
		if ((eBit | *cur) == 0x65u)
			goto digi_exp_more;
		goto digi_exp_finish;
	digi_frac_end:
		sigEnd = cur;
		expSig = -int64_t((cur - dotPos) - 1);
		if (expSig == 0) {
			return false;
		}
		if ((eBit | *cur) != 0x65u) [[likely]] {
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
		expSign = (*++cur == 0x2Du);
		cur += (*cur == 0x2Bu || *cur == 0x2Du);
		if (asciiToValueTable[static_cast<uint64_t>(*cur)] > 9) [[unlikely]] {
			return false;
		}
		while (*cur == 0x30u) {
			++cur;
		}
		tmp = cur;
		uint8_t c{};
		while (c < 10) {
			c = static_cast<uint8_t>(asciiToValueTable[static_cast<uint64_t>(*cur)]);
			++cur;
			expLit = c + int64_t(expLit) * 10;
		}
		if (cur - tmp >= 6) [[unlikely]] {
			if (sig == 0 || expSign) {
				value = applySign(0);
				value = static_cast<value_type01>(sig);
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
			value = (sign ? -value_type01{ 0 } : value_type01{ 0 });
			return true;
		}
		if (expSig < f64MinDecExp - 19) [[unlikely]] {
			value = (sign ? -value_type01{ 0 } : value_type01{ 0 });
			return true;
		} else if (expSig > f64MaxDecExp) [[unlikely]] {
			value = sign ? -std::numeric_limits<value_type01>::infinity() : std::numeric_limits<value_type01>::infinity();
			return true;
		}
		exp = expSig;
	digi_finish:

		if constexpr (std::same_as<double, value_type01>) {
			if (sig < (uint64_t(1) << 53) && std::abs(exp) <= 22) {
				value = static_cast<value_type01>(sig);
				if constexpr (!std::unsigned_integral<value_type01>) {
					value *= sign ? -1 : 1;
				}
				if (exp >= 0 && exp < 23) {
					value *= static_cast<value_type01>(powersOfTenFloat[exp]);
				} else if (-exp < 23) {
					value /= static_cast<value_type01>(powersOfTenFloat[-exp]);
				}
				return true;
			}
		} else {
			if (sig < (uint64_t(1) << 24) && std::abs(exp) <= 8) {
				value = static_cast<value_type01>(sig);
				if constexpr (!std::unsigned_integral<value_type01>) {
					value *= sign ? -1 : 1;
				}
				if (exp >= 0 && exp < 23) {
					value *= static_cast<value_type01>(powersOfTenFloat[exp]);
				} else if (-exp < 23) {
					value /= static_cast<value_type01>(powersOfTenFloat[-exp]);
				}
				return true;
			}
		}

		if (sig == 0) [[unlikely]] {
			value = value_type01(0);
			return true;
		}

		static_assert(std::numeric_limits<value_type01>::is_iec559);
		static_assert(std::numeric_limits<value_type01>::radix == 2);
		static_assert(std::same_as<float, std::decay_t<value_type01>> || std::same_as<double, std::decay_t<value_type01>>);
		static_assert(sizeof(float) == 4 && sizeof(double) == 8);

		const auto sigLeadingZeros		 = std::countl_zero(sig);
		const auto sigNorm				 = sig << sigLeadingZeros;
		const auto sig2Norm				 = sig2FromExp10(static_cast<uint64_t>(exp));
		const auto sigProduct			 = mulhi64(sigNorm, sig2Norm) + 1;
		const auto sigProductStartsWith1 = sigProduct >> 63;
		auto mantisa					 = sigProduct << (2 - sigProductStartsWith1);
		constexpr uint64_t roundMask	 = uint64_t(1) << 63 >> (std::numeric_limits<value_type01>::digits - 1);
		constexpr uint64_t exponentBits	 = ceillog2(std::numeric_limits<value_type01>::max_exponent - std::numeric_limits<value_type01>::min_exponent + 1);
		constexpr uint64_t mantissaShift = exponentBits + 1 + 64 - 8 * sizeof(uint64_t);
		int64_t exp2					 = static_cast<int64_t>(exp2FromExp10(exp) + static_cast<uint64_t>(-sigLeadingZeros + sigProductStartsWith1));

		if (exp2 < std::numeric_limits<value_type01>::min_exponent - 1) [[unlikely]] {
			value = sign ? -value_type01(0) : value_type01(0);
			return true;
		} else if (exp2 > std::numeric_limits<value_type01>::max_exponent - 1) [[unlikely]] {
			value = sign ? -std::numeric_limits<value_type01>::infinity() : std::numeric_limits<value_type01>::infinity();
			return true;
		}

		uint64_t round = 0;
		if (roundMask & mantisa) {
			if (mantisa << (std::numeric_limits<value_type01>::digits) == 0) {
				auto sig_upper	   = (mantisa >> (mantissaShift - 1)) | (uint64_t(1) << 63 >> (mantissaShift - 2)) | 1;
				int64_t exp2_upper = exp2 - std::numeric_limits<value_type01>::digits;

				big_int_t big_comp{ sig_upper };
				big_int_t big_full{ sig };
				if (exp >= 0) {
					big_full.mulPow10(static_cast<uint64_t>(exp));
				} else {
					big_comp.mulPow10(static_cast<uint64_t>(-exp));
				}
				if (exp2_upper >= 0) {
					big_comp.mulPow2(static_cast<uint64_t>(exp2_upper));
				} else {
					big_full.mulPow2(static_cast<uint64_t>(-exp2_upper));
				}
				auto cmp = big_full <=> big_comp;
				if (cmp != 0) [[likely]] {
					round = (cmp > 0);
				} else {
					round = (mantisa & (roundMask << 1)) != 0;
				}
			} else if ((exp < pow10SigTableMinExact || exp > pow10SigTableMaxExact) || (mantisa & (roundMask << 1)) ||
				(static_cast<size_t>(std::countr_zero(sigNorm)) + static_cast<size_t>(std::countr_zero(sig2Norm)) <
					128 - std::numeric_limits<value_type01>::digits - (2 - sigProductStartsWith1))) {
				round = 1;
			}
		}

		auto num = uint64_t(sign) << (sizeof(uint64_t) * 8 - 1) | uint64_t(mantisa >> mantissaShift) |
			(uint64_t(exp2 + std::numeric_limits<value_type01>::max_exponent - 1) << (std::numeric_limits<value_type01>::digits - 1));
		num += uint64_t(round);
		std::memcpy(&value, &num, sizeof(value_type01));
		return true;
	}
}