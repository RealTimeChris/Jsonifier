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

	// Source: https://github.com/ibireme/yyjson/blob/master/src/yyjson.c

	/** Multiplies two 64-bit unsigned integers (a * b),
       returns the 128-bit result as 'hi' and 'lo'. */
	JSONIFIER_INLINE void u128Mul(uint64_t a, uint64_t b, uint64_t* hi, uint64_t* lo) noexcept {
#ifdef __SIZEOF_INT128__
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
		uint32_t a0 = ( uint32_t )(a), a1 = ( uint32_t )(a >> 32);
		uint32_t b0 = ( uint32_t )(b), b1 = ( uint32_t )(b >> 32);
		uint64_t p00 = ( uint64_t )a0 * b0, p01 = ( uint64_t )a0 * b1;
		uint64_t p10 = ( uint64_t )a1 * b0, p11 = ( uint64_t )a1 * b1;
		uint64_t m0	 = p01 + (p00 >> 32);
		uint32_t m00 = ( uint32_t )(m0), m01 = ( uint32_t )(m0 >> 32);
		uint64_t m1	 = p10 + m00;
		uint32_t m10 = ( uint32_t )(m1), m11 = ( uint32_t )(m1 >> 32);
		*hi = p11 + m01 + m11;
		*lo = (( uint64_t )m10 << 32) | ( uint32_t )p00;
#endif
	}

	JSONIFIER_INLINE void u128MulAdd(uint64_t a, uint64_t b, uint64_t c, uint64_t* hi, uint64_t* lo) noexcept {
#ifdef __SIZEOF_INT128__
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

	JSONIFIER_INLINE uint64_t roundToOdd(uint64_t hi, uint64_t lo, uint64_t cp) noexcept {
		uint64_t x_hi, x_lo, y_hi, y_lo;
		u128Mul(cp, lo, &x_hi, &x_lo);
		u128MulAdd(cp, hi, x_hi, &y_hi, &y_lo);
		return y_hi | (y_lo > 1);
	}

	constexpr auto pow10SigTable128MinExp	   = -343;
	constexpr auto pow10SigTable128MaxExp	   = 324;
	constexpr auto pow10SigTable128MinExactExp = 0;
	constexpr auto pow10SigTable128MaxExactExp = 55;

	JSONIFIER_INLINE void pow10TableGetSig128(const int32_t exp10, uint64_t hilo[2]) noexcept {
		const int32_t idx = exp10 - (pow10SigTable128MinExp);
		std::memcpy(hilo, pow10SigTable128 + idx * 2, 16);
	}

	JSONIFIER_INLINE void f64BinToDec(uint64_t sig_raw, int32_t exp_raw, uint64_t sig_bin, int32_t exp_bin, uint64_t* sig_dec, int32_t* exp_dec) noexcept {
		uint64_t sp, mid;

		const bool is_even			  = !(sig_bin & 1);
		const bool lower_bound_closer = (sig_raw == 0 && exp_raw > 1);

		const uint64_t cb  = 4 * sig_bin;
		const uint64_t cbl = cb - 2 + lower_bound_closer;
		const uint64_t cbr = cb + 2;

		const int32_t k = (exp_bin * 315653 - (lower_bound_closer ? 131237 : 0)) >> 20;

		const int32_t exp10 = -k;
		const int32_t h		= exp_bin + ((exp10 * 217707) >> 16) + 1;

		uint64_t pow10hilo[2];
		pow10TableGetSig128(exp10, pow10hilo);
		const uint64_t& pow10hi = pow10hilo[0];
		uint64_t& pow10lo		= pow10hilo[1];
		pow10lo += (exp10 < pow10SigTable128MinExactExp || exp10 > pow10SigTable128MaxExactExp);
		const uint64_t vbl = roundToOdd(pow10hi, pow10lo, cbl << h);
		const uint64_t vb  = roundToOdd(pow10hi, pow10lo, cb << h);
		const uint64_t vbr = roundToOdd(pow10hi, pow10lo, cbr << h);

		const uint64_t lower = vbl + !is_even;
		const uint64_t upper = vbr - !is_even;

		bool u_inside, w_inside;

		const uint64_t s = vb / 4;
		if (s >= 10) {
			sp		 = s / 10;
			u_inside = (lower <= 40 * sp);
			w_inside = (upper >= 40 * sp + 40);
			if (u_inside != w_inside) {
				*sig_dec = sp + w_inside;
				*exp_dec = k + 1;
				return;
			}
		}

		u_inside = (lower <= 4 * s);
		w_inside = (upper >= 4 * s + 4);

		mid					= 4 * s + 2;
		const bool round_up = (vb > mid) || (vb == mid && (s & 1) != 0);

		*sig_dec = s + ((u_inside != w_inside) ? w_inside : round_up);
		*exp_dec = k;
	}

	template<typename char_type> JSONIFIER_INLINE char_type* writeU64Len15To17Trim(char_type* buf, uint64_t sig) noexcept {
		uint32_t tz1, tz2, tz;

		uint32_t abbccddee = uint32_t(sig / 100000000);
		uint32_t ffgghhii  = uint32_t(sig - uint64_t(abbccddee) * 100000000);
		uint32_t abbcc	   = abbccddee / 10000;
		uint32_t ddee	   = abbccddee - abbcc * 10000;
		uint32_t abb	   = uint32_t((uint64_t(abbcc) * 167773) >> 24);
		uint32_t a		   = (abb * 41) >> 12;
		uint32_t bb		   = abb - a * 100;
		uint32_t cc		   = abbcc - abb * 100;

		buf[0] = char_type(a + 0x30u);
		buf += a > 0;
		bool lz = bb < 10 && a == 0;
		std::memcpy(buf, charTable + (bb * 2 + lz), 2);
		buf -= lz;
		std::memcpy(buf + 2, charTable + 2 * cc, 2);

		if (ffgghhii) {
			uint32_t dd	  = (ddee * 5243) >> 19;
			uint32_t ee	  = ddee - dd * 100;
			uint32_t ffgg = uint32_t((uint64_t(ffgghhii) * 109951163) >> 40);
			uint32_t hhii = ffgghhii - ffgg * 10000;
			uint32_t ff	  = (ffgg * 5243) >> 19;
			uint32_t gg	  = ffgg - ff * 100;
			std::memcpy(buf + 4, charTable + 2 * dd, 2);
			std::memcpy(buf + 6, charTable + 2 * ee, 2);
			std::memcpy(buf + 8, charTable + 2 * ff, 2);
			std::memcpy(buf + 10, charTable + 2 * gg, 2);
			if (hhii) {
				uint32_t hh = (hhii * 5243) >> 19;
				uint32_t ii = hhii - hh * 100;
				std::memcpy(buf + 12, charTable + 2 * hh, 2);
				std::memcpy(buf + 14, charTable + 2 * ii, 2);
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
				std::memcpy(buf + 4, charTable + 2 * dd, 2);
				std::memcpy(buf + 6, charTable + 2 * ee, 2);
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

	template<jsonifier::concepts::float_t value_type, typename char_type> JSONIFIER_INLINE char_type* toChars(char_type* buffer, value_type val) noexcept {
		static_assert(std::numeric_limits<value_type>::is_iec559);
		static_assert(std::numeric_limits<value_type>::radix == 2);
		static_assert(std::is_same_v<float, value_type> || std::is_same_v<double, value_type>);
		static_assert(sizeof(float) == 4 && sizeof(double) == 8);
		using raw_t = std::conditional_t<std::is_same_v<float, value_type>, uint32_t, uint64_t>;

		raw_t raw;
		std::memcpy(&raw, &val, sizeof(value_type));

		constexpr uint32_t exponentBits = numbits(std::numeric_limits<value_type>::max_exponent - std::numeric_limits<value_type>::min_exponent + 1);
		constexpr raw_t sig_mask		= raw_t(-1) >> (exponentBits + 1);
		bool sign						= (raw >> (sizeof(value_type) * 8 - 1));
		uint64_t sig_raw				= raw & sig_mask;
		int32_t exp_raw					= static_cast<int32_t>(raw << 1 >> (sizeof(raw_t) * 8 - exponentBits));

		if (exp_raw == (uint32_t(1) << exponentBits) - 1) [[unlikely]] {
			std::memcpy(buffer, "null", 4);
			return buffer + 4;
		}
		if (sign) {
			*buffer = 0x2Du;
			++buffer;
		}
		if ((raw << 1) != 0) [[likely]] {
			uint64_t sig_bin;
			int32_t exp_bin;
			if (exp_raw == 0) [[unlikely]] {
				sig_bin = sig_raw;
				exp_bin = 1 - (std::numeric_limits<value_type>::max_exponent - 1) - (std::numeric_limits<value_type>::digits - 1);
			} else {
				sig_bin = sig_raw | uint64_t(1ull << (std::numeric_limits<value_type>::digits - 1));
				exp_bin = int32_t(exp_raw) - (std::numeric_limits<value_type>::max_exponent - 1) - (std::numeric_limits<value_type>::digits - 1);
			}

			uint64_t sig_dec;
			int32_t exp_dec;
			f64BinToDec(sig_raw, exp_raw, sig_bin, exp_bin, &sig_dec, &exp_dec);
			if constexpr (std::same_as<value_type, float>) {
				sig_dec *= 100000000;
				exp_dec -= 8;
			}

			int32_t sig_len = 17;
			sig_len -= (sig_dec < 100000000ull * 100000000ull);
			sig_len -= (sig_dec < 100000000ull * 10000000ull);

			int32_t dotPos = sig_len + exp_dec;

			if (-6 < dotPos && dotPos <= 21) {
				if (dotPos <= 0) {
					auto num_hdr = buffer + (2 - dotPos);
					auto num_end = writeU64Len15To17Trim(num_hdr, sig_dec);
					buffer[0]	 = 0x30u;
					buffer[1]	 = 0x2Eu;
					buffer += 2;
					std::memset(buffer, 0x30u, static_cast<uint64_t>(num_hdr - buffer));
					return num_end;
				} else {
					std::memset(buffer, 0x30u, 8);
					std::memset(buffer + 8, 0x30u, 8);
					std::memset(buffer + 16, 0x30u, 8);
					auto num_hdr = buffer + 1;
					auto num_end = writeU64Len15To17Trim(num_hdr, sig_dec);
					std::memmove(buffer, buffer + 1, static_cast<uint64_t>(dotPos));
					buffer[dotPos] = 0x2Eu;
					return ((num_end - num_hdr) <= dotPos) ? buffer + dotPos : num_end;
				}
			} else {
				auto end = writeU64Len15To17Trim(buffer + 1, sig_dec);
				end -= (end == buffer + 2);
				exp_dec += sig_len - 1;
				buffer[0] = buffer[1];
				buffer[1] = 0x2Eu;
				end[0]	  = 0x45u;
				buffer	  = end + 1;
				buffer[0] = 0x2Du;
				buffer += exp_dec < 0;
				exp_dec = std::abs(exp_dec);
				if (exp_dec < 100) {
					uint32_t lz = exp_dec < 10;
					std::memcpy(buffer, charTable + (exp_dec * 2 + lz), 2);
					return buffer + 2 - lz;
				} else {
					const uint32_t hi = (uint32_t(exp_dec) * 656) >> 16;
					const uint32_t lo = uint32_t(exp_dec) - hi * 100;
					buffer[0]		  = uint8_t(hi) + 0x30;
					std::memcpy(&buffer[1], charTable + (lo * 2), 2);
					return buffer + 3;
				}
			}
		} else [[unlikely]] {
			*buffer = 0x30u;
			return buffer + 1;
		}
	}

}