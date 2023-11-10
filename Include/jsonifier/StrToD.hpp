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

// Based on yyjson: https://github.com/ibireme/yyjson/blob/master/src/yyjson.c with some changes to rounding and
// dirrect for floats
// TODO: Subnormals are not handled right now.
// Wontfix: Numbers with more than 19 sigfigs may be off by 1ulp. No algorithm should be outputing more than 17
// digits so I dont think roundtripping matters if you supply extra digits
// https://stackoverflow.com/questions/28868367/getting-the-high-part-of-64-bit-integer-multiplication
#ifdef __SIZEOF_INT128__
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
		uint64_t aLo	   = ( uint64_t )a;
		uint64_t aHi	   = a >> 32;
		uint64_t bLo	   = ( uint64_t )b;
		uint64_t bHi	   = b >> 32;
		uint64_t axbHi  = aHi * bHi;
		uint64_t axbMid = aHi * bLo;
		uint64_t bxaMid = bHi * aLo;
		uint64_t axbLo  = aLo * bLo;
		uint64_t carryBit = (( uint64_t )( uint64_t )axbMid + ( uint64_t )( uint64_t )bxaMid + (axbLo >> 32)) >> 32;
		uint64_t multhi	   = axbHi + (axbMid >> 32) + (bxaMid >> 32) + carryBit;
		return multhi;
	}
#endif

	constexpr auto pow10SigTableMinExp	 = -343;
	constexpr auto pow10SigTableMaxExp	 = 324;
	constexpr auto pow10SigTableMinExact = 0;
	constexpr auto pow10SigTableMaxExact = 27;

	JSONIFIER_INLINE uint64_t sig2FromExp10(int64_t exp10) noexcept {
		return pow10SigTable[exp10 - pow10SigTableMinExp];
	}

	JSONIFIER_INLINE int64_t exp2FromExp10(int64_t exp10) noexcept {
		return (((exp10 * 217706 - 4128768) >> 16) + 126);
	}

	template<typename value_type> constexpr value_type digiTypeZero	   = 1 << 0;
	template<typename value_type> constexpr value_type digiTypeNonZero = 1 << 1;
	template<typename value_type> constexpr value_type digiTypePos	   = 1 << 2;
	template<typename value_type> constexpr value_type digiTypeNeg	   = 1 << 3;
	template<typename value_type> constexpr value_type digiTypeDot	   = 1 << 4;
	template<typename value_type> constexpr value_type digiTypeExp	   = 1 << 5;

	template<typename value_type> JSONIFIER_INLINE bool digiIsType(value_type d, value_type type) noexcept {
		return (digiTable[static_cast<uint64_t>(d)] & type) != 0;
	}

	template<typename value_type> JSONIFIER_INLINE bool digiIsFp(value_type d) noexcept {
		return digiIsType(d, static_cast<value_type>(digiTypeDot<value_type> | digiTypeExp<value_type>));
	}

	template<typename value_type> JSONIFIER_INLINE bool digiIsDigitOrFp(value_type d) noexcept {
		return digiIsType(d, static_cast<value_type>(digiTypeZero<value_type> | digiTypeNonZero<value_type> | digiTypeDot<value_type> | digiTypeExp<value_type>));
	}

#define repeat_in_1_18(x) \
	{ x(1) x(2) x(3) x(4) x(5) x(6) x(7) x(8) x(9) x(10) x(11) x(12) x(13) x(14) x(15) x(16) x(17) x(18) }

	template<typename value_type> constexpr auto eBit = static_cast<value_type>(0x45u ^ 0x65u);
	constexpr auto f64MinDecExp						  = (-324);
	constexpr auto f64MaxDecExp						  = 308;

	consteval uint64_t ceillog2(uint64_t x) {
		return x < 2 ? x : 1 + ceillog2(x >> 1);
	}

	struct big_int_t final {
		std::vector<uint64_t> data = {};

		big_int_t(uint64_t num) noexcept {
			uint64_t lowerWord = uint64_t(num);
			uint64_t upperWord = uint64_t(num >> 32);
			if (upperWord > 0) {
				data = { lowerWord, upperWord };
			} else {
				data = { lowerWord };
			}
		}

		void mulU32(uint64_t num) noexcept {
			uint64_t carry = 0;
			for (uint64_t i = 0; i < data.size(); i++) {
				uint64_t res		= uint64_t(data[i]) * uint64_t(num) + uint64_t(carry);
				uint64_t lowerWord = uint64_t(res);
				uint64_t upperWord = uint64_t(res >> 32);
				data[i]				= lowerWord;
				carry				= upperWord;
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
				while (move) {
					data[--move] = 0;
				}
			} else {
				data.resize(data.size() + move + 1);
				++idx;
				for (; idx > 0; idx--) {
					uint64_t num = data[idx] << shft;
					num |= data[idx - 1] >> (32 - shft);
					data[idx + move] = num;
				}
				data[move] = data[0] << shft;
				if (data.back() == 0) {
					data.pop_back();
				}
				while (move) {
					data[--move] = 0;
				}
			}
		}

		auto operator<=>(const big_int_t& rhs) const noexcept {
			if (data.size() < rhs.data.size()) {
				return -1;
			}
			if (data.size() > rhs.data.size()) {
				return 1;
			}
			for (auto i = data.size() - 1; i > 0; --i) {
				if (data[i] < rhs.data[i]) {
					return -1;
				}
				if (data[i] > rhs.data[i]) {
					return 1;
				}
			}
			return 0;
		}
	};

	template<jsonifier::concepts::float_t value_type, typename char_type> JSONIFIER_INLINE bool parseNumber(value_type& val, const char_type* cur) noexcept {
		[[maybe_unused]] const char_type* sigEnd{};
		const char_type *tmp{}, *sigCut{}, *dotPos{}, *hdr{ cur };
		uint64_t fracZeros{}, numTmp{}, sig{};
		int64_t exp{}, expSig{}, expLit{};
		bool sign{ (*hdr == 0x2Du) }, expSign{};
		if (*cur == 0x30u && numberTable[*(cur + 1)] || *(cur + 1) == 0x78u) {
			return false;
		}
		cur += sign;
		auto applySign = [&](auto&& val) -> value_type {
			return sign ? -static_cast<value_type>(val) : static_cast<value_type>(val);
		};
		sig = uint64_t(*cur - 0x30u);
		if (sig > 9) {
			if constexpr (std::integral<value_type>) {
				return false;
			} else if (*cur == 0x6Eu && cur[1] == 0x75u && cur[2] == 0x6Cu && cur[3] == 0x6Cu) {
				cur += 4;
				val = std::numeric_limits<value_type>::quiet_NaN();
				return true;
			} else if (( *cur | eBit<char_type> ) == 0x6Eu && ( cur[1] | eBit<char_type> ) == 0x61u && ( cur[2] | eBit<char_type> ) == 0x6Eu) {
				cur += 3;
				val = sign ? -std::numeric_limits<value_type>::quiet_NaN() : std::numeric_limits<value_type>::quiet_NaN();
				return true;
			} else {
				return false;
			}
		}
		constexpr auto zero = static_cast<char_type>(0x30u);
#define expr_intg(i) \
	if (numTmp = static_cast<uint64_t>(asciiToValueTable[static_cast<uint64_t>(cur[i])]); numTmp <= 9) [[likely]] \
		sig = numTmp + sig * 10; \
	else { \
		if constexpr (i > 1) { \
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
		if (!digiIsDigitOrFp(*cur)) {
			val = static_cast<value_type>(sig);
			if constexpr (!std::is_unsigned_v<value_type>) {
				val *= sign ? -1 : 1;
			}
			return true;
		}
		goto digi_intg_more;
#define expr_sepr(i) \
	digi_sepr_##i : if ((!digiIsFp(cur[i]))) [[likely]] { \
		cur += i; \
		val = applySign(sig); \
		return true; \
	} \
	dotPos = cur + i; \
	if ((cur[i] == 0x2Eu)) [[likely]] { \
		if (sig == 0) \
			while (asciiToValueTable[static_cast<uint64_t>(cur[static_cast<uint64_t>(fracZeros + i + 1)])] == zero) \
				++fracZeros; \
		goto digi_frac_##i; \
	} \
	cur += i; \
	sigEnd = cur; \
	goto digi_exp_more;
		repeat_in_1_18(expr_sepr)
#undef expr_sepr
#define expr_frac(i) \
	digi_frac_##i : if (((numTmp = static_cast<uint64_t>(asciiToValueTable[static_cast<uint64_t>(cur[i + 1 + fracZeros])])) <= 9)) [[likely]] sig = numTmp + sig * 10; \
	else { \
		goto digi_stop_##i; \
	}
			repeat_in_1_18(expr_frac)
#undef expr_frac
				cur += 20 + fracZeros;
		if (auto newValue = asciiToValueTable[static_cast<uint64_t>(*cur)]; newValue > 9) [[unlikely]]
			goto digi_frac_end;
		goto digi_frac_more;
#define expr_stop(i) \
	digi_stop_##i : cur += i + 1 + fracZeros; \
	goto digi_frac_end;
		repeat_in_1_18(expr_stop)
#undef expr_stop
			digi_intg_more : static constexpr uint64_t U64_MAX = (std::numeric_limits<uint64_t>::max)();
		if (numTmp = static_cast<uint64_t>(asciiToValueTable[static_cast<uint64_t>(*cur)]); numTmp < 10) {
			if (!digiIsDigitOrFp(cur[1])) {
				if ((sig < (U64_MAX / 10)) || (sig == (U64_MAX / 10) && numTmp <= (U64_MAX % 10))) {
					sig = numTmp + sig * 10;
					cur++;
					val = static_cast<value_type>(sig);
					if constexpr (!std::is_unsigned_v<value_type>) {
						val *= sign ? -1 : 1;
					}
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
		sig += (*cur >= 0x35u);
		while (asciiToValueTable[static_cast<uint64_t>(*++cur)] < 10) {
		}
		if (!dotPos) {
			dotPos = cur;
			if (*cur == 0x2Eu) {
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
		while (*tmp == 0x30u || *tmp == 0x2Eu) {
			--tmp;
		}
		if (tmp < sigCut) {
			sigCut = nullptr;
		} else {
			sigEnd = cur;
		}
		if ((eBit<char_type> | *cur) == 0x65u)
			goto digi_exp_more;
		goto digi_exp_finish;
	digi_frac_end:
		sigEnd = cur;
		expSig = -int64_t((cur - dotPos) - 1);
		if (expSig == 0) {
			return false;
		}
		if ((eBit<char_type> | *cur) != 0x65u) [[likely]] {
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
		expSign = (*++cur == 0x2Du);
		cur += (*cur == 0x2Bu || *cur == 0x2Du);
		if (auto newValue = asciiToValueTable[static_cast<uint64_t>(*cur)]; newValue > 9) [[unlikely]] {
			return false;
		}
		while (*cur == 0x30u) {
			++cur;
		}
		tmp = cur;
		char_type c;
		while ((c = *cur - zero) < 10) {
			++cur;
			expLit = static_cast<int64_t>(c + uint64_t(expLit) * 10);
		}
		if ((cur - tmp >= 6)) [[unlikely]] {
			if (sig == 0 || expSign) {
				val = applySign(0);
				val = static_cast<value_type>(sig);
				return true;
			} else {
				val = applySign(std::numeric_limits<value_type>::infinity());
				return true;
			}
		}
		expSig += expSign ? -expLit : expLit;
	digi_exp_finish:
		if (sig == 0) {
			val = (sign ? -value_type{ 0 } : value_type{ 0 });
			return true;
		}
		if ((expSig < f64MinDecExp - 19)) [[unlikely]] {
			val = (sign ? -value_type{ 0 } : value_type{ 0 });
			return true;
		} else if ((expSig > f64MaxDecExp)) [[unlikely]] {
			val = sign ? -std::numeric_limits<value_type>::infinity() : std::numeric_limits<value_type>::infinity();
			return true;
		}
		exp = expSig;
	digi_finish:

		if constexpr (std::is_same_v<double, value_type>) {
			if (sig < (uint64_t(1) << 53) && std::abs(exp) <= 22) {
				val = static_cast<value_type>(sig);
				if constexpr (!std::is_unsigned_v<value_type>) {
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
				val = static_cast<value_type>(sig);
				if constexpr (!std::is_unsigned_v<value_type>) {
					val *= sign ? -1 : 1;
				}
				if (exp >= 0) {
					val *= static_cast<value_type>(powersOfTenFloat[exp]);
				} else {
					val /= static_cast<value_type>(powersOfTenFloat[-exp]);
				}
				return true;
			}
		}

		if (sig == 0) [[unlikely]] {
			val = value_type(0);
			return true;
		}

		static_assert(std::numeric_limits<value_type>::is_iec559);
		static_assert(std::numeric_limits<value_type>::radix == 2);
		static_assert(std::is_same_v<float, std::decay_t<value_type>> || std::is_same_v<double, std::decay_t<value_type>>);
		static_assert(sizeof(float) == 4 && sizeof(double) == 8);

		using raw_t						  = std::conditional_t<std::is_same_v<float, std::decay_t<value_type>>, uint64_t, uint64_t>;
		const auto sigLeadingZeros		  = std::countl_zero(sig);
		const auto sigNorm				  = sig << sigLeadingZeros;
		const auto sig2Norm				  = sig2FromExp10(exp);
		const auto sigProduct			  = mulhi64(sigNorm, sig2Norm) + 1;
		const auto sigProductStartsWith1  = sigProduct >> 63;
		auto mantisa					  = sigProduct << (2 - sigProductStartsWith1);
		constexpr uint64_t roundMask	  = uint64_t(1) << 63 >> (std::numeric_limits<value_type>::digits - 1);
		constexpr uint64_t exponentBits	  = ceillog2(std::numeric_limits<value_type>::max_exponent - std::numeric_limits<value_type>::min_exponent + 1);
		constexpr uint64_t mantissaShifit = exponentBits + 1 + 64 - 8 * sizeof(raw_t);
		int64_t exp2					  = static_cast<int64_t>(exp2FromExp10(exp) + static_cast<uint64_t>(-sigLeadingZeros + sigProductStartsWith1));

		if (exp2 < std::numeric_limits<value_type>::min_exponent - 1) [[unlikely]] {
			val = sign ? -value_type(0) : value_type(0);
			return true;
		} else if (exp2 > std::numeric_limits<value_type>::max_exponent - 1) [[unlikely]] {
			val = sign ? -std::numeric_limits<value_type>::infinity() : std::numeric_limits<value_type>::infinity();
			return true;
		}

		uint64_t round = 0;
		if (roundMask & mantisa) {
			if (mantisa << (std::numeric_limits<value_type>::digits) == 0) {
				auto sigUpper	  = (mantisa >> (mantissaShifit - 1)) | (uint64_t(1) << 63 >> (mantissaShifit - 2)) | 1;
				int64_t exp2Upper = exp2 - std::numeric_limits<value_type>::digits;

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
			} else if ((exp < pow10SigTableMinExact || exp > pow10SigTableMaxExact) || (mantisa & (roundMask << 1)) ||
				(static_cast<uint64_t>(std::countr_zero(sigNorm) + std::countr_zero(sig2Norm)) < 128 - std::numeric_limits<value_type>::digits - (2 - sigProductStartsWith1))) {
				round = 1;
			}
		}

		auto num = raw_t(sign) << (sizeof(raw_t) * 8 - 1) | raw_t(mantisa >> mantissaShifit) |
			(raw_t(exp2 + std::numeric_limits<value_type>::max_exponent - 1) << (std::numeric_limits<value_type>::digits - 1));
		num += raw_t(round);
		std::memcpy(&val, &num, sizeof(value_type));
		return true;
	}

}