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
		uint64_t aLo	  = a;
		uint64_t aHi	  = a >> 32;
		uint64_t bLo	  = b;
		uint64_t bHi	  = b >> 32;
		uint64_t axbHi	  = aHi * bHi;
		uint64_t axbMid	  = aHi * bLo;
		uint64_t bxaMid	  = bHi * aLo;
		uint64_t axbLo	  = aLo * bLo;
		uint64_t carryBit = (axbMid + bxaMid + (axbLo >> 32)) >> 32;
		uint64_t multhi	  = axbHi + (axbMid >> 32) + (bxaMid >> 32) + carryBit;
		return multhi;
	}
#endif

	constexpr auto pow10SigTableMinExp{ -343 };
	constexpr auto pow10SigTableMaxExp{ 324 };
	constexpr auto pow10SigTableMinExact{ 0 };
	constexpr auto pow10SigTableMaxExact{ 27 };

	JSONIFIER_INLINE uint64_t sig2FromExp10(int64_t exp10) noexcept {
		return pow10SigTable[exp10 - pow10SigTableMinExp];
	}

	JSONIFIER_INLINE int64_t exp2FromExp10(int64_t exp10) noexcept {
		return (((exp10 * 217706 - 4128768) >> 16) + 126);
	}

	constexpr uint8_t digiTypeZero	  = 1 << 0;
	constexpr uint8_t digiTypeNonZero = 1 << 1;
	constexpr uint8_t digiTypeDot	  = 1 << 4;
	constexpr uint8_t digiTypeExp	  = 1 << 5;

	template<typename value_type> JSONIFIER_INLINE bool digiIsType(value_type d, value_type type) noexcept {
		return (static_cast<value_type>(digiTable[d]) & type) != 0;
	}

	template<typename value_type> JSONIFIER_INLINE bool digiIsFp(value_type d) noexcept {
		return digiIsType(d, value_type(digiTypeDot | digiTypeExp));
	}

	template<typename value_type> JSONIFIER_INLINE bool digiIsDigitOrFp(value_type d) noexcept {
		return digiIsType(d, value_type(digiTypeZero | digiTypeNonZero | digiTypeDot | digiTypeExp));
	}

#define repeat_in_1_18(x) \
	{ x(1) x(2) x(3) x(4) x(5) x(6) x(7) x(8) x(9) x(10) x(11) x(12) x(13) x(14) x(15) x(16) x(17) x(18) }

	constexpr auto eBit			   = static_cast<uint8_t>(0x45u ^ 0x65u);
	constexpr int64_t f64MaxDecExp = 308;
	constexpr int64_t f64MinDecExp = (-324);

	consteval uint64_t ceillog2(uint64_t x) {
		return x < 2 ? x : 1 + ceillog2(x >> 1);
	}

	struct big_int_t final {
		std::vector<uint64_t> data = {};

		JSONIFIER_INLINE big_int_t(uint64_t num) noexcept {
			uint64_t lower_word = uint64_t(num);
			uint64_t upper_word = uint64_t(num >> 32);
			if (upper_word > 0) {
				data = { lower_word, upper_word };
			} else {
				data = { lower_word };
			}
		}

		JSONIFIER_INLINE void mulU32(uint64_t num) noexcept {
			uint64_t carry = 0;
			for (std::size_t i = 0; i < data.size(); i++) {
				uint64_t res		= uint64_t(data[i]) * uint64_t(num) + uint64_t(carry);
				uint64_t lower_word = uint64_t(res);
				uint64_t upper_word = uint64_t(res >> 32);
				data[i]				= lower_word;
				carry				= upper_word;
			}
			if (carry != 0) {
				data.emplace_back(carry);
			}
		}

		JSONIFIER_INLINE void mulPow10(uint64_t pow10) noexcept {
			for (; pow10 >= 9; pow10 -= 9) {
				mulU32(static_cast<uint64_t>(powersOfTenInt[9]));
			}
			if (pow10) {
				mulU32(static_cast<uint64_t>(powersOfTenInt[pow10]));
			}
		}

		JSONIFIER_INLINE void mulPow2(uint64_t exp) noexcept {
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

		JSONIFIER_INLINE auto operator<=>(const big_int_t& rhs) const noexcept {
			if (data.size() < rhs.data.size())
				return -1;
			if (data.size() > rhs.data.size())
				return 1;
			for (auto i = data.size() - 1; i > 0; --i) {
				;
				if (data[i] < rhs.data[i])
					return -1;
				if (data[i] > rhs.data[i])
					return 1;
			}
			return 0;
		}
	};

	template<jsonifier::concepts::float_t value_type, typename char_type>
		requires(sizeof(value_type) <= 8)
	JSONIFIER_INLINE bool parseNumber(value_type& val, const char_type* cur) noexcept {
		const char_type* sig_cut				  = nullptr;
		[[maybe_unused]] const char_type* sig_end = nullptr;
		const char_type* dot_pos				  = nullptr;
		uint64_t frac_zeros						  = 0;
		uint64_t sig							  = 0;
		int64_t exp								  = 0;
		bool exp_sign;
		int64_t exp_sig = 0;
		int64_t exp_lit = 0;
		uint64_t num_tmp;
		const char_type* tmp;
		const char_type* hdr = cur;
		bool sign			 = (*hdr == 0x2Du);
		cur += sign;
		auto apply_sign = [&](auto&& val) -> value_type {
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
			} else if ((*cur | eBit) == 0x6Eu && (cur[1] | eBit) == 0x61u && (cur[2] | eBit) == 0x6Eu) {
				cur += 3;
				val = sign ? -std::numeric_limits<value_type>::quiet_NaN() : std::numeric_limits<value_type>::quiet_NaN();
				return true;
			} else {
				return false;
			}
		}
		constexpr auto zero = static_cast<char_type>(0x30u);
#define expr_intg(i) \
	if (num_tmp = static_cast<uint64_t>(asciiToValueTable[static_cast<uint64_t>(cur[i])]); num_tmp <= 9) [[likely]] \
		sig = num_tmp + sig * 10; \
	else { \
		if (*cur == zero) \
			return false; \
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
		val = apply_sign(sig); \
		return true; \
	} \
	dot_pos = cur + i; \
	if ((cur[i] == 0x2Eu)) [[likely]] { \
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
	digi_frac_##i : if (((num_tmp = static_cast<uint64_t>(asciiToValueTable[cur[i + 1 + frac_zeros]]))); num_tmp <= 9) [[likely]] sig = num_tmp + sig * 10; \
	else { \
		goto digi_stop_##i; \
	}
			repeat_in_1_18(expr_frac)
#undef expr_frac
				cur += 20 + frac_zeros;
		if (char_type(asciiToValueTable[static_cast<uint64_t>(*cur)]) > 9)
			goto digi_frac_end;
		goto digi_frac_more;
#define expr_stop(i) \
	digi_stop_##i : cur += i + 1 + frac_zeros; \
	goto digi_frac_end;
		repeat_in_1_18(expr_stop)
#undef expr_stop
			digi_intg_more : static constexpr uint64_t U64_MAX = (std::numeric_limits<uint64_t>::max)();// todo
		if ((num_tmp = static_cast<uint64_t>(asciiToValueTable[static_cast<uint64_t>(*cur)])) < 10) {
			if (!digiIsDigitOrFp(cur[1])) {
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
		if ((eBit | *cur) == 0x65u) {
			dot_pos = cur;
			goto digi_exp_more;
		}
		if (*cur == 0x2Eu) {
			dot_pos = cur++;
			if (char_type(asciiToValueTable[static_cast<uint64_t>(*cur)]) > 9) {
				return false;
			}
		}
	digi_frac_more:
		sig_cut = cur;
		sig += (*cur >= 0x35u);
		while (asciiToValueTable[static_cast<uint64_t>(*++cur)] < 10) {
		}
		if (!dot_pos) {
			dot_pos = cur;
			if (*cur == 0x2Eu) {
				if (asciiToValueTable[static_cast<uint64_t>(*++cur)] > 9) {
					return false;
				}
				while (asciiToValueTable[static_cast<uint64_t>(*++cur)] < 10) {
				}
			}
		}
		exp_sig = static_cast<int64_t>(dot_pos - sig_cut);
		exp_sig += (dot_pos < sig_cut);
		tmp = cur - 1;
		while (*tmp == 0x30u || *tmp == 0x2Eu) {
			--tmp;
		}
		if (tmp < sig_cut) {
			sig_cut = nullptr;
		} else {
			sig_end = cur;
		}
		if ((eBit | *cur) == 0x65u)
			goto digi_exp_more;
		goto digi_exp_finish;
	digi_frac_end:
		sig_end = cur;
		exp_sig = -int64_t((cur - dot_pos) - 1);
		if (exp_sig == 0)
			return false;
		if ((eBit | *cur) != 0x65u) [[likely]] {
			if ((exp_sig < f64MinDecExp - 19)) [[unlikely]] {
				val = apply_sign(0);
				return true;
			}
			exp = exp_sig;
			goto digi_finish;
		} else {
			goto digi_exp_more;
		}
	digi_exp_more:
		exp_sign = (*++cur == 0x2Du);
		cur += (*cur == 0x2Bu || *cur == 0x2Du);
		if (char_type(asciiToValueTable[static_cast<uint64_t>(*cur)]) > 9) [[unlikely]] {
			return false;
		}
		while (*cur == 0x30u) {
			++cur;
		}
		tmp = cur;
		char_type c;
		while (char_type(c = asciiToValueTable[static_cast<uint64_t>(*cur)]) < 10) {
			++cur;
			exp_lit = static_cast<int64_t>(c + uint64_t(exp_lit) * 10);
		}
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
	digi_finish:

		if constexpr (std::is_same_v<double, value_type>) {
			if (sig < (uint64_t(1) << 53) && std::abs(exp) <= 22) {
				val = static_cast<value_type>(sig);
				if constexpr (!std::is_unsigned_v<value_type>) {
					val *= sign ? -1 : 1;
				}
				if (exp >= 0 && exp < 23) {
					val *= powersOfTenFloat[exp];
				} else if (auto newValue = -exp; exp < 0 && newValue < 23) {
					val /= powersOfTenFloat[newValue];
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
				} else if (exp < 0 && -exp < 23) {
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

		using raw_t							 = std::conditional_t<std::is_same_v<float, std::decay_t<value_type>>, uint64_t, uint64_t>;
		const auto sig_leading_zeros		 = std::countl_zero(sig);
		const auto sig_norm					 = sig << sig_leading_zeros;
		const auto sig2_norm				 = sig2FromExp10(exp);
		const auto sig_product				 = mulhi64(sig_norm, sig2_norm) + 1;
		const auto sig_product_starts_with_1 = sig_product >> 63;
		auto mantisa						 = sig_product << (2 - sig_product_starts_with_1);
		constexpr uint64_t round_mask		 = uint64_t(1) << 63 >> (std::numeric_limits<value_type>::digits - 1);
		constexpr uint64_t exponent_bits	 = ceillog2(std::numeric_limits<value_type>::max_exponent - std::numeric_limits<value_type>::min_exponent + 1);
		constexpr uint64_t mantisa_shift	 = exponent_bits + 1 + 64 - 8 * sizeof(raw_t);
		int64_t exp2						 = static_cast<int64_t>(exp2FromExp10(exp) + static_cast<uint64_t>(-sig_leading_zeros + sig_product_starts_with_1));

		if (exp2 < std::numeric_limits<value_type>::min_exponent - 1) [[unlikely]] {
			val = sign ? -value_type(0) : value_type(0);
			return true;
		} else if (exp2 > std::numeric_limits<value_type>::max_exponent - 1) [[unlikely]] {
			val = sign ? -std::numeric_limits<value_type>::infinity() : std::numeric_limits<value_type>::infinity();
			return true;
		}

		uint64_t round = 0;
		if (round_mask & mantisa) {
			if (mantisa << (std::numeric_limits<value_type>::digits) == 0) {
				auto sig_upper	   = (mantisa >> (mantisa_shift - 1)) | (uint64_t(1) << 63 >> (mantisa_shift - 2)) | 1;
				int64_t exp2_upper = exp2 - std::numeric_limits<value_type>::digits;

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
					round = (mantisa & (round_mask << 1)) != 0;
				}
			} else if ((exp < pow10SigTableMinExact || exp > pow10SigTableMaxExact) || (mantisa & (round_mask << 1)) ||
				(static_cast<size_t>(tzcnt(sig_norm) + tzcnt(sig2_norm)) < 128 - std::numeric_limits<value_type>::digits - (2 - sig_product_starts_with_1))) {
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