/*
	MIT License

	Copyright (c) 2024 RealTimeChris

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
/// Nov 13, 2023
#pragma once

#include <jsonifier/Containers/Allocator.hpp>
#include <jsonifier/Utilities/DragonBox.hpp>

#include <concepts>
#include <cstdint>
#include <cstring>
#include <array>

namespace jsonifier::internal {

	inline constexpr uint8_t decTrailingZeroTable[] = { 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0,
		0, 0 };

	JSONIFIER_INLINE auto* writeu64Len15To17Trim(auto* buf, uint64_t sig) noexcept {
		uint32_t tz1, tz2, tz;
		const uint64_t abbccddee = multiply_and_shift<6189700196426901375ULL, 89ULL>::impl(sig);
		const uint64_t ffgghhii	 = sig - abbccddee * 100000000;
		uint32_t abbcc			 = abbccddee / 10000;
		uint32_t ddee			 = abbccddee - abbcc * 10000;
		uint32_t abb			 = uint32_t((uint64_t(abbcc) * 167773) >> 24);
		uint32_t a				 = (abb * 41) >> 12;
		uint32_t bb				 = abb - a * 100;
		uint32_t cc				 = abbcc - abb * 100;
		buf[0]					 = uint8_t(a + '0');
		buf += a > 0;
		bool lz = bb < 10 && a == 0;
		std::memcpy(buf, fiwb<void>::charTable01 + (bb * 2 + lz), 2);
		buf -= lz;
		std::memcpy(buf + 2, fiwb<void>::charTable01 + 2 * cc, 2);

		if (ffgghhii) {
			uint32_t dd	  = (ddee * 5243) >> 19;
			uint32_t ee	  = ddee - dd * 100;
			uint32_t ffgg = uint32_t((uint64_t(ffgghhii) * 109951163) >> 40);
			uint32_t hhii = ffgghhii - ffgg * 10000;
			uint32_t ff	  = (ffgg * 5243) >> 19;
			uint32_t gg	  = ffgg - ff * 100;
			std::memcpy(buf + 4, fiwb<void>::charTable01 + 2 * dd, 2);
			std::memcpy(buf + 6, fiwb<void>::charTable01 + 2 * ee, 2);
			std::memcpy(buf + 8, fiwb<void>::charTable01 + 2 * ff, 2);
			std::memcpy(buf + 10, fiwb<void>::charTable01 + 2 * gg, 2);
			if (hhii) {
				uint32_t hh = (hhii * 5243) >> 19;
				uint32_t ii = hhii - hh * 100;
				std::memcpy(buf + 12, fiwb<void>::charTable01 + 2 * hh, 2);
				std::memcpy(buf + 14, fiwb<void>::charTable01 + 2 * ii, 2);
				tz1 = decTrailingZeroTable[hh];
				tz2 = decTrailingZeroTable[ii];
				tz	= ii ? tz2 : (tz1 + 2);
				buf += 16 - tz;
				return buf;
			} else {
				tz1 = decTrailingZeroTable[ff];
				tz2 = decTrailingZeroTable[gg];
				tz	= gg ? tz2 : (tz1 + 2);
				buf += 12 - tz;
				return buf;
			}
		} else {
			if (ddee) {
				uint32_t dd = (ddee * 5243) >> 19;
				uint32_t ee = ddee - dd * 100;
				std::memcpy(buf + 4, fiwb<void>::charTable01 + 2 * dd, 2);
				std::memcpy(buf + 6, fiwb<void>::charTable01 + 2 * ee, 2);
				tz1 = decTrailingZeroTable[dd];
				tz2 = decTrailingZeroTable[ee];
				tz	= ee ? tz2 : (tz1 + 2);
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

	consteval uint32_t numbits(uint32_t x) noexcept {
		return x < 2 ? x : 1 + numbits(x >> 1);
	}

	JSONIFIER_INLINE static int64_t abs(int64_t value) noexcept {
		const uint64_t temp = static_cast<uint64_t>(value >> 63);
		value ^= temp;
		value += temp & 1;
		return value;
	}

	template<concepts::float_t value_type> struct to_chars<value_type> {
		JSONIFIER_INLINE static char* impl(char* buf, value_type val) noexcept {
			static_assert(std::numeric_limits<value_type>::is_iec559);
			static_assert(std::numeric_limits<value_type>::radix == 2);
			static_assert(std::is_same_v<float, value_type> || std::is_same_v<double, value_type>);
			static_assert(sizeof(float) == 4 && sizeof(double) == 8);
			constexpr bool is_float = std::is_same_v<float, value_type>;
			using Raw				= std::conditional_t<std::is_same_v<float, value_type>, uint32_t, uint64_t>;

			if (val == 0.0) {
				*buf = '-';
				buf += (std::bit_cast<Raw>(val) >> (sizeof(value_type) * 8 - 1));
				*buf = '0';
				return buf + 1;
			}

			using Conversion						 = jsonifier_jkj::dragonbox::default_float_bit_carrier_conversion_traits<value_type>;
			using FormatTraits						 = jsonifier_jkj::dragonbox::ieee754_binary_traits<typename Conversion::format, typename Conversion::carrier_uint>;
			static constexpr uint32_t exp_bits_count = numbits(std::numeric_limits<value_type>::max_exponent - std::numeric_limits<value_type>::min_exponent + 1);
			const auto float_bits					 = jsonifier_jkj::dragonbox::make_float_bits<value_type, Conversion, FormatTraits>(val);
			const auto exp_bits						 = float_bits.extract_exponent_bits();
			const auto s							 = float_bits.remove_exponent_bits();

			if (exp_bits == (uint32_t(1) << exp_bits_count) - 1) [[unlikely]] {
				std::memcpy(buf, "null", 4);
				return buf + 4;
			}

			*buf				= '-';
			constexpr auto zero = value_type(0.0);
			buf += (val < zero);

			const auto v =
				jsonifier_jkj::dragonbox::to_decimal_ex(s, exp_bits, jsonifier_jkj::dragonbox::policy::sign::ignore, jsonifier_jkj::dragonbox::policy::trailing_zero::ignore);

			uint64_t sig_dec = v.significand;
			int32_t exp_dec	 = v.exponent;

			int32_t sig_len = 17;
			sig_len -= (sig_dec < 100000000ull * 100000000ull);
			sig_len -= (sig_dec < 100000000ull * 10000000ull);
			int32_t dot_pos = sig_len + exp_dec;

			if (-6 < dot_pos && dot_pos <= 21) {
				if (dot_pos <= 0) {
					auto num_hdr = buf + (2 - dot_pos);
					auto num_end = writeu64Len15To17Trim(num_hdr, sig_dec);
					buf[0]		 = '0';
					buf[1]		 = '.';
					buf += 2;
					std::memset(buf, '0', size_t(num_hdr - buf));
					return num_end;
				} else {
					std::memset(buf, '0', 24);
					auto num_hdr = buf + 1;
					auto num_end = writeu64Len15To17Trim(num_hdr, sig_dec);
					std::memmove(buf, buf + 1, size_t(dot_pos));
					buf[dot_pos] = '.';
					return ((num_end - num_hdr) <= dot_pos) ? buf + dot_pos : num_end;
				}
			} else {
				auto end = writeu64Len15To17Trim(buf + 1, sig_dec);
				end -= (end == buf + 2);
				exp_dec += sig_len - 1;
				buf[0] = buf[1];
				buf[1] = '.';
				end[0] = 'E';
				buf	   = end + 1;
				buf[0] = '-';
				buf += exp_dec < 0;
				exp_dec = abs(exp_dec);
				if (exp_dec < 100) {
					uint32_t lz = exp_dec < 10;
					std::memcpy(buf, fiwb<void>::charTable01 + (exp_dec * 2 + lz), 2);
					return buf + 2 - lz;
				} else {
					const uint32_t hi = (uint32_t(exp_dec) * 656) >> 16;
					const uint32_t lo = uint32_t(exp_dec) - hi * 100;
					buf[0]			  = uint8_t(hi) + '0';
					std::memcpy(&buf[1], fiwb<void>::charTable01 + (lo * 2), 2);
					return buf + 3;
				}
			}
		}
	};
}