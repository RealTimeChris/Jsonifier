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

	inline void u128Mul(uint64_t a, uint64_t b, uint64_t* hi, uint64_t* lo) noexcept {
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
		uint64_t a0 = ( uint64_t )(a), a1 = ( uint64_t )(a >> 32);
		uint64_t b0 = ( uint64_t )(b), b1 = ( uint64_t )(b >> 32);
		uint64_t p00 = ( uint64_t )a0 * b0, p01 = ( uint64_t )a0 * b1;
		uint64_t p10 = ( uint64_t )a1 * b0, p11 = ( uint64_t )a1 * b1;
		uint64_t m0	 = p01 + (p00 >> 32);
		uint64_t m00 = ( uint64_t )(m0), m01 = ( uint64_t )(m0 >> 32);
		uint64_t m1	 = p10 + m00;
		uint64_t m10 = ( uint64_t )(m1), m11 = ( uint64_t )(m1 >> 32);
		*hi = p11 + m01 + m11;
		*lo = (( uint64_t )m10 << 32) | ( uint64_t )p00;
#endif
	}

	inline void u128MulAdd(uint64_t a, uint64_t b, uint64_t c, uint64_t* hi, uint64_t* lo) noexcept {
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

	constexpr auto pow10SigTableMinExp{ -343 };
	constexpr auto pow10SigTableMaxExp{ 324 };
	constexpr auto pow10SigTableMinExactExp{ 0 };
	constexpr auto pow10SigTableMaxExactExp{ 55 };

	consteval uint64_t numbits(uint64_t x) noexcept {
		return x < 2 ? x : 1 + numbits(x >> 1);
	}

	inline void pow10TableGetSig(int64_t exp10, uint64_t* hi, uint64_t* lo) {
		int64_t idx = exp10 - (pow10SigTableMinExp);
		*hi			= pow10SigTable[idx * 2];
		*lo			= pow10SigTable[idx * 2 + 1];
	}

	inline void pow10TableGetExp(int64_t exp10, int64_t* exp2) {
		*exp2 = (exp10 * 217706 - 4128768) >> 16;
	}

	inline uint64_t roundToOdd(uint64_t hi, uint64_t lo, uint64_t cp) {
		uint64_t x_hi, x_lo, y_hi, y_lo;
		u128Mul(cp, lo, &x_hi, &x_lo);
		u128MulAdd(cp, hi, x_hi, &y_hi, &y_lo);
		return y_hi | (y_lo > 1);
	}

	inline void f64BinToDec(uint64_t sigRaw, uint64_t expRaw, uint64_t sigBin, int64_t expBin, uint64_t* sigDec, int64_t* expDec) {
		bool is_even, regularSpacing, uInside, wInside, roundUp;
		uint64_t s, sp, cb, cbl, cbr, vb, vbl, vbr, pow10hi, pow10lo, upper, lower, mid;
		int64_t k, h, exp10;

		is_even		   = !(sigBin & 1);
		regularSpacing = (sigRaw == 0 && expRaw > 1);

		cbl	  = 4 * sigBin - 2 + regularSpacing;
		cb	  = 4 * sigBin;
		cbr	  = 4 * sigBin + 2;
		k	  = ( int64_t )(expBin * 315653 - (regularSpacing ? 131237 : 0)) >> 20;
		exp10 = -k;
		h	  = expBin + ((exp10 * 217707) >> 16) + 1;

		pow10TableGetSig(exp10, &pow10hi, &pow10lo);
		pow10lo += (exp10 < pow10SigTableMinExactExp || exp10 > pow10SigTableMaxExactExp);
		vbl = roundToOdd(pow10hi, pow10lo, cbl << h);
		vb	= roundToOdd(pow10hi, pow10lo, cb << h);
		vbr = roundToOdd(pow10hi, pow10lo, cbr << h);

		lower = vbl + !is_even;
		upper = vbr - !is_even;

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
		wInside = (upper >= 4 * s + 4);

		mid		= 4 * s + 2;
		roundUp = (vb > mid) || (vb == mid && (s & 1) != 0);

		*sigDec = s + ((uInside != wInside) ? wInside : roundUp);
		*expDec = k;
	}

	template<typename value_type> inline value_type* writeUint64TLen15To17Trim(value_type* buf, uint64_t sig) noexcept {
		uint64_t tz1, tz2, tz;

		uint64_t abbccddee = uint64_t(sig / 100000000);
		uint64_t ffgghhii  = uint64_t(sig - uint64_t(abbccddee) * 100000000);
		uint64_t abbcc	   = abbccddee / 10000;
		uint64_t ddee	   = abbccddee - abbcc * 10000;
		uint64_t abb	   = uint64_t((uint64_t(abbcc) * 167773) >> 24);
		uint64_t a		   = (abb * 41) >> 12;
		uint64_t bb		   = abb - a * 100;
		uint64_t cc		   = abbcc - abb * 100;

		buf[0] = static_cast<value_type>(uint8_t(a + '0'));
		buf += a > 0;
		bool lz = bb < 10 && a == 0;
		std::memcpy(buf, charTable + (bb * 2 + lz), 2);
		buf -= lz;
		std::memcpy(buf + 2, charTable + 2 * cc, 2);

		if (ffgghhii) {
			uint64_t dd	  = (ddee * 5243) >> 19;
			uint64_t ee	  = ddee - dd * 100;
			uint64_t ffgg = uint64_t((uint64_t(ffgghhii) * 109951163) >> 40);
			uint64_t hhii = ffgghhii - ffgg * 10000;
			uint64_t ff	  = (ffgg * 5243) >> 19;
			uint64_t gg	  = ffgg - ff * 100;
			std::memcpy(buf + 4, charTable + 2 * dd, 2);
			std::memcpy(buf + 6, charTable + 2 * ee, 2);
			std::memcpy(buf + 8, charTable + 2 * ff, 2);
			std::memcpy(buf + 10, charTable + 2 * gg, 2);
			if (hhii) {
				uint64_t hh = (hhii * 5243) >> 19;
				uint64_t ii = hhii - hh * 100;
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
				uint64_t dd = (ddee * 5243) >> 19;
				uint64_t ee = ddee - dd * 100;
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

	template<std::floating_point T, typename value_type> inline value_type* toChars(value_type* buffer, T val) noexcept {
		static_assert(std::numeric_limits<T>::is_iec559);
		static_assert(std::numeric_limits<T>::radix == 2);
		static_assert(std::is_same_v<float, T> || std::is_same_v<double, T>);
		static_assert(sizeof(float) == 4 && sizeof(double) == 8);
		using raw_t = std::conditional_t<std::is_same_v<float, T>, uint64_t, uint64_t>;

		raw_t raw;
		std::memcpy(&raw, &val, sizeof(T));

		constexpr uint64_t exponentBits = numbits(std::numeric_limits<T>::max_exponent - std::numeric_limits<T>::min_exponent + 1);
		constexpr raw_t sigMask			= raw_t(-1) >> (exponentBits + 1);
		bool sign						= (raw >> (sizeof(T) * 8 - 1));
		uint64_t sigRaw					= raw & sigMask;
		int64_t expRaw					= static_cast<int64_t>(raw << 1 >> (sizeof(raw_t) * 8 - exponentBits));

		if (expRaw == (uint64_t(1) << exponentBits) - 1) [[unlikely]] {
			std::memcpy(buffer, "null", 4);
			return buffer + 4;
		}
		if (sign) {
			*buffer = '-';
			++buffer;
		}

		if ((raw << 1) != 0) [[likely]] {
			uint64_t sigBin;
			int64_t expBin;
			if (expRaw == 0) [[unlikely]] {
				sigBin = sigRaw;
				expBin = 1 - (std::numeric_limits<T>::max_exponent - 1) - (std::numeric_limits<T>::digits - 1);
			} else {
				sigBin = sigRaw | uint64_t(1ull << (std::numeric_limits<T>::digits - 1));
				expBin = int64_t(expRaw) - (std::numeric_limits<T>::max_exponent - 1) - (std::numeric_limits<T>::digits - 1);
			}

			uint64_t sigDec;
			int64_t expDec;
			f64BinToDec(sigRaw, static_cast<uint64_t>(expRaw), sigBin, expBin, &sigDec, &expDec);
			if constexpr (std::same_as<T, float>) {
				sigDec *= 100000000;
				expDec -= 8;
			}

			int64_t sigLen = 17;
			sigLen -= (sigDec < 100000000ull * 100000000ull);
			sigLen -= (sigDec < 100000000ull * 10000000ull);

			int64_t dotPos = sigLen + expDec;

			if (-6 < dotPos && dotPos <= 21) {
				if (dotPos <= 0) {
					auto numHdr = buffer + (2 - dotPos);
					auto numEnd = writeUint64TLen15To17Trim(numHdr, sigDec);
					buffer[0]	= '0';
					buffer[1]	= '.';
					buffer += 2;
					for (; buffer < numHdr; ++buffer)
						*buffer = '0';
					return numEnd;
				} else {
					std::memset(buffer, '0', 8);
					std::memset(buffer + 8, '0', 8);
					std::memset(buffer + 16, '0', 8);
					auto numHdr = buffer + 1;
					auto numEnd = writeUint64TLen15To17Trim(numHdr, sigDec);
					for (int32_t i = 0; i < dotPos; i++)
						buffer[i] = buffer[i + 1];
					buffer[dotPos] = '.';
					return ((numEnd - numHdr) <= dotPos) ? buffer + dotPos : numEnd;
				}
			} else {
				auto end = writeUint64TLen15To17Trim(buffer + 1, sigDec);
				end -= (end == buffer + 2);
				expDec += sigLen - 1;
				buffer[0] = buffer[1];
				buffer[1] = '.';
				end[0]	  = 'E';
				buffer	  = end + 1;
				buffer[0] = '-';
				buffer += expDec < 0;
				expDec = std::abs(expDec);
				if (expDec < 100) {
					uint64_t lz = expDec < 10;
					std::memcpy(buffer, charTable + (expDec * 2 + lz), 2);
					return buffer + 2 - lz;
				} else {
					const uint64_t hi = (uint64_t(expDec) * 656) >> 16;
					const uint64_t lo = uint64_t(expDec) - hi * 100;
					buffer[0]		  = uint8_t(hi) + '0';
					std::memcpy(&buffer[1], charTable + (lo * 2), 2);
					return buffer + 3;
				}
			}
		} else [[unlikely]] {
			*buffer = '0';
			return buffer + 1;
		}
	}

}