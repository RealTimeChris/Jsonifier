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
		uint64_t aLo	  = ( uint64_t )a;
		uint64_t aHi	  = a >> 32;
		uint64_t bLo	  = ( uint64_t )b;
		uint64_t bHi	  = b >> 32;
		uint64_t axbHi	  = aHi * bHi;
		uint64_t axbMid	  = aHi * bLo;
		uint64_t bxaMid	  = bHi * aLo;
		uint64_t axbLo	  = aLo * bLo;
		uint64_t carryBit = (( uint64_t )( uint64_t )axbMid + ( uint64_t )( uint64_t )bxaMid + (axbLo >> 32)) >> 32;
		uint64_t multhi	  = axbHi + (axbMid >> 32) + (bxaMid >> 32) + carryBit;
		return multhi;
	}
#endif
	inline constexpr auto pow10SigTableMinExp	= -343;
	inline constexpr auto pow10SigTableMaxExp	= 324;
	inline constexpr auto pow10SigTableMinExact = 0;
	inline constexpr auto pow10SigTableMaxExact = 27;

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
				uint64_t res	   = uint64_t(data[i]) * uint64_t(num) + uint64_t(carry);
				uint64_t lowerWord = uint64_t(res);
				uint64_t upperWord = uint64_t(res >> 32);
				data[i]			   = lowerWord;
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

	consteval uint32_t ceillog2(uint32_t x) {
		return x < 2 ? x : 1 + ceillog2(x >> 1);
	}

	template<jsonifier::concepts::float_t value_type, typename char_type>
		requires(sizeof(value_type) <= 8)
	inline bool parseNumber(value_type& val, char_type*& cur) noexcept {
		const char_type* sig_cut				  = nullptr;
		[[maybe_unused]] const char_type* sig_end = nullptr;
		const char_type* dot_pos				  = nullptr;
		uint32_t frac_zeros						  = 0;
		uint64_t sig							  = 0;
		int32_t exp								  = 0;
		bool exp_sign;
		int32_t exp_sig = 0;
		int32_t exp_lit = 0;
		uint64_t num_tmp;
		char_type* tmp;
		char_type* hdr = cur;
		bool sign	   = (*hdr == '-');
		cur += sign;
		auto apply_sign = [&](auto&& val) -> value_type {
			return sign ? -static_cast<value_type>(val) : static_cast<value_type>(val);
		};
		/* begin with non-zero digit */
		sig = uint64_t(*cur - '0');
		if (sig > 9) {
			if constexpr (std::integral<value_type>) {
				return false;
			} else if (*cur == 'n' && cur[1] == 'u' && cur[2] == 'l' && cur[3] == 'l') {
				cur += 4;
				val = std::numeric_limits<value_type>::quiet_NaN();
				return true;
			} else if (( *cur | eBit<char_type> ) == 'n' && ( cur[1] | eBit<char_type> ) == 'a' && ( cur[2] | eBit<char_type> ) == 'n') {
				cur += 3;
				val = sign ? -std::numeric_limits<value_type>::quiet_NaN() : std::numeric_limits<value_type>::quiet_NaN();
				return true;
			} else {
				return false;
			}
		}
		constexpr auto zero = static_cast<char_type>('0');
#define expr_intg(i) \
	if ((num_tmp = static_cast<uint64_t>(cur[i] - zero)) <= 9) [[likely]] \
		sig = num_tmp + sig * 10; \
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
		cur += 19; /* skip continuous 19 digits */
		if (!digiIsDigitOrFp(*cur)) {
			val = static_cast<value_type>(sig);
			if constexpr (!std::is_unsigned_v<value_type>) {
				val *= sign ? -1 : 1;
			}
			return true;
		}
		goto digi_intg_more; /* read more digits in integral part */
		/* process first non-digit character */
#define expr_sepr(i) \
	digi_sepr_##i : if ((!digiIsFp(cur[i]))) [[likely]] { \
		cur += i; \
		val = apply_sign(sig); \
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
		/* read fraction part */
#define expr_frac(i) \
	digi_frac_##i : if (((num_tmp = static_cast<uint64_t>(cur[i + 1 + frac_zeros] - zero)) <= 9)) [[likely]] sig = num_tmp + sig * 10; \
	else { \
		goto digi_stop_##i; \
	}
			repeat_in_1_18(expr_frac)
#undef expr_frac
				cur += 20 + frac_zeros; /* skip 19 digits and 1 decimal point */
		if (char_type(*cur - zero) > 9)
			goto digi_frac_end; /* fraction part end */
		goto digi_frac_more; /* read more digits in fraction part */
		/* significant part end */
#define expr_stop(i) \
	digi_stop_##i : cur += i + 1 + frac_zeros; \
	goto digi_frac_end;
		repeat_in_1_18(expr_stop)
#undef expr_stop
			/* read more digits in integral part */
			digi_intg_more : static constexpr uint64_t U64_MAX = (std::numeric_limits<uint64_t>::max)();// todo
		if ((num_tmp = static_cast<uint64_t>(*cur - zero)) < 10) {
			if (!digiIsDigitOrFp(cur[1])) {
				/* this number is an integer consisting of 20 digits */
				if ((sig < (U64_MAX / 10)) || (sig == (U64_MAX / 10) && num_tmp <= (U64_MAX % 10))) {
					sig = num_tmp + sig * 10;
					cur++;
					val = static_cast<value_type>(sig);
					if constexpr (!std::is_unsigned_v<value_type>) {
						val *= sign ? -1 : 1;
					}
					return true;
				}
			}
		}
		if ((eBit<char_type> | *cur) == 'e') {
			dot_pos = cur;
			goto digi_exp_more;
		}
		if (*cur == '.') {
			dot_pos = cur++;
			if (char_type(*cur - zero) > 9) {
				return false;
			}
		}
		/* read more digits in fraction part */
	digi_frac_more:
		sig_cut = cur; /* too large to fit in u64, excess digits need to be cut */
		sig += (*cur >= '5'); /* round */
		while (char_type(*++cur - zero) < 10) {
		}
		if (!dot_pos) {
			dot_pos = cur;
			if (*cur == '.') {
				if (char_type(*++cur - zero) > 9) {
					return false;
				}
				while (char_type(*++cur - zero) < 10) {
				}
			}
		}
		exp_sig = static_cast<int32_t>(dot_pos - sig_cut);
		exp_sig += (dot_pos < sig_cut);
		// ignore trailing zeros
		tmp = cur - 1;
		while (*tmp == '0' || *tmp == '.') {
			--tmp;
		}
		if (tmp < sig_cut) {
			sig_cut = nullptr;
		} else {
			sig_end = cur;
		}
		if ((eBit<char_type> | *cur) == 'e')
			goto digi_exp_more;
		goto digi_exp_finish;
		// fraction part end
	digi_frac_end:
		sig_end = cur;
		exp_sig = -int32_t((cur - dot_pos) - 1);
		if (exp_sig == 0)
			return false;
		if ((eBit<char_type> | *cur) != 'e') [[likely]] {
			if ((exp_sig < f64MinDecExp - 19)) [[unlikely]] {
				val = apply_sign(0);
				return true;
			}
			exp = exp_sig;
			goto digi_finish;
		} else {
			goto digi_exp_more;
		}
		// read exponent part
	digi_exp_more:
		exp_sign = (*++cur == '-');
		cur += (*cur == '+' || *cur == '-');
		if (char_type(*cur - zero) > 9) [[unlikely]] {
			return false;
		}
		while (*cur == '0') {
			++cur;
		}
		// read exponent literal
		tmp = cur;
		std::remove_const_t<char_type> c;
		while (char_type(c = *cur - zero) < 10) {
			++cur;
			exp_lit = static_cast<int32_t>(c) + int32_t(exp_lit) * 10;
		}
		// large exponent case
		if ((cur - tmp >= 6)) [[unlikely]] {
			if (sig == 0 || exp_sign) {
				val = apply_sign(0);
				val = static_cast<value_type>(sig);
				return true;
			} else {
				val = apply_sign(std::numeric_limits<value_type>::infinity());
				return true;
			}
		}
		exp_sig += exp_sign ? -exp_lit : exp_lit;
		// validate exponent value
	digi_exp_finish:
		if (sig == 0) {
			val = (sign ? -value_type{ 0 } : value_type{ 0 });
			return true;
		}
		if ((exp_sig < f64MinDecExp - 19)) [[unlikely]] {
			val = (sign ? -value_type{ 0 } : value_type{ 0 });
			return true;
		} else if ((exp_sig > f64MaxDecExp)) [[unlikely]] {
			val = sign ? -std::numeric_limits<value_type>::infinity() : std::numeric_limits<value_type>::infinity();
			return true;
		}
		exp = exp_sig;
		// all digit read finished
	digi_finish:

		if constexpr (std::is_same_v<double, value_type>) {
			// numbers must be exactly representable in this fast path
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

		if (sig == 0) [[unlikely]]// fast path is more likely for zeros (0.00000000000000000000000 is uncommon)
		{
			val = value_type(0);
			return true;
		}

		static_assert(std::numeric_limits<value_type>::is_iec559);
		static_assert(std::numeric_limits<value_type>::radix == 2);
		static_assert(std::is_same_v<float, std::decay_t<value_type>> || std::is_same_v<double, std::decay_t<value_type>>);
		static_assert(sizeof(float) == 4 && sizeof(double) == 8);

		using raw_t							 = std::conditional_t<std::is_same_v<float, std::decay_t<value_type>>, uint32_t, uint64_t>;
		const auto sig_leading_zeros		 = std::countl_zero(sig);
		const auto sig_norm					 = sig << sig_leading_zeros;
		const auto sig2_norm				 = sig2FromExp10(exp);
		const auto sig_product				 = mulhi64(sig_norm, sig2_norm) + 1;
		const auto sig_product_starts_with_1 = sig_product >> 63;
		auto mantisa						 = sig_product << (2 - sig_product_starts_with_1);
		constexpr uint64_t round_mask		 = uint64_t(1) << 63 >> (std::numeric_limits<value_type>::digits - 1);
		constexpr uint32_t exponent_bits	 = ceillog2(std::numeric_limits<value_type>::max_exponent - std::numeric_limits<value_type>::min_exponent + 1);
		constexpr uint32_t mantisa_shift	 = exponent_bits + 1 + 64 - 8 * sizeof(raw_t);
		uint32_t exp2						 = static_cast<uint32_t>(exp2FromExp10(exp)) + static_cast<uint32_t>(-sig_leading_zeros + sig_product_starts_with_1);

		if (static_cast<value_type>(exp2) < std::numeric_limits<value_type>::min_exponent - 1) [[unlikely]] {
			// TODO handle subnormal numbers
			val = sign ? -value_type(0) : value_type(0);
			return true;
		} else if (exp2 > std::numeric_limits<value_type>::max_exponent - 1) [[unlikely]] {
			val = sign ? -std::numeric_limits<value_type>::infinity() : std::numeric_limits<value_type>::infinity();
			return true;
		}

		uint64_t round = 0;
		if (round_mask & mantisa) {
			if (mantisa << (std::numeric_limits<value_type>::digits) == 0) {
				// We added one to the product so this is the case were the trailing bits were 1.
				// This is a problem since the product could underestimate by a bit and uness there is a zero bit to fall
				// into we cant be sure if we need to round or not
				auto sig_upper	   = (mantisa >> (mantisa_shift - 1)) | (uint64_t(1) << 63 >> (mantisa_shift - 2)) | 1;
				int32_t exp2_upper = static_cast<int32_t>(exp2 - std::numeric_limits<value_type>::digits);

				big_int_t big_comp{ sig_upper };
				big_int_t big_full{ sig };// Not dealing will ulp from sig_cut since we only care about roundtriping
					// machine doubles and only a human would use so many sigfigs
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
					// round down or round up
					round = (cmp > 0);
				} else {
					// falls midway, round to even
					round = (mantisa & (round_mask << 1)) != 0;
				}
			} else if ((exp < pow10SigTableMinExact || exp > pow10SigTableMaxExact)// If there are ones after 64 bits in sig2 then there will be
				// ones after the rounding bit in the product
				|| (mantisa & (round_mask << 1))// Odd nums need to round up regardless of if the rest is nonzero or not
				|| (static_cast<size_t>(std::countr_zero(sig_norm) + std::countr_zero(sig2_norm)) <
					   128 - std::numeric_limits<value_type>::digits - (2 - sig_product_starts_with_1))// Check where the least significant one is
			) {
				round = 1;
			}
		}

		auto num = raw_t(sign) << (sizeof(raw_t) * 8 - 1) | raw_t(mantisa >> mantisa_shift) |
			(raw_t(exp2 + std::numeric_limits<value_type>::max_exponent - 1) << (std::numeric_limits<value_type>::digits - 1));
		num += raw_t(round);
		std::memcpy(&val, &num, sizeof(value_type));
		return true;
	}

}