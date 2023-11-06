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

	JSONIFIER_INLINE void u128Mul(uint64_t a, uint64_t b, uint64_t* hi, uint64_t* lo) noexcept {
#if defined(__SIZEOF_INT128__)
	#if defined(__GNUC__) || defined(__GNUG__)
		#pragma GCC diagnostic push
		#pragma GCC diagnostic ignored "-Wpedantic"
	#endif
		unsigned __int128 m = static_cast<unsigned __int128>(a) * b;
	#if defined(__GNUC__) || defined(__GNUG__)
		#pragma GCC diagnostic pop
	#endif
		*hi = m >> 64;
		*lo = m;
#elif defined(_M_X64)
		*lo = _umul128(a, b, hi);
#elif defined(_M_ARM64)
		*hi = __umulh(a, b);
		*lo = a * b;
#else
		uint64_t a0 = a, a1 = (a >> 32);
		uint64_t b0 = (b), b1 = (b >> 32);
		uint64_t p00 = a0 * b0, p01 = a0 * b1;
		uint64_t p10 = a1 * b0, p11 = a1 * b1;
		uint64_t m0	 = p01 + (p00 >> 32);
		uint64_t m00 = (m0), m01 = (m0 >> 32);
		uint64_t m1	 = p10 + m00;
		uint64_t m10 = (m1), m11 = (m1 >> 32);
		*hi = p11 + m01 + m11;
		*lo = (m10 << 32) | p00;
#endif
	}

	JSONIFIER_INLINE void u128MulAdd(uint64_t a, uint64_t b, uint64_t c, uint64_t* hi, uint64_t* lo) noexcept {
#if defined(__SIZEOF_INT128__)
	#if defined(__GNUC__) || defined(__GNUG__)
		#pragma GCC diagnostic push
		#pragma GCC diagnostic ignored "-Wpedantic"
	#endif
		unsigned __int128 m = static_cast<unsigned __int128>(a) * b + c;
	#if defined(__GNUC__) || defined(__GNUG__)
		#pragma GCC diagnostic pop
	#endif
		*hi = m >> 64;
		*lo = m;
#else
		uint64_t h{}, l{}, t{};
		u128Mul(a, b, &h, &l);
		t = l + c;
		h += ((t < l) | (t < c));
		*hi = h;
		*lo = t;
#endif
	}

	JSONIFIER_INLINE uint64_t roundToOdd(uint64_t hi, uint64_t lo, uint64_t cp) noexcept {
		uint64_t xHi{}, xLo{}, yHi{}, yLo{};
		u128Mul(cp, lo, &xHi, &xLo);
		u128MulAdd(cp, hi, xHi, &yHi, &yLo);
		return yHi | (yLo > 1);
	}

	constexpr auto pow10SigTable128MinExp	   = -343;
	constexpr auto pow10SigTable128MaxExp	   = 324;
	constexpr auto pow10SigTable128MinExactExp = 0;
	constexpr auto pow10SigTable128MaxExactExp = 55;

	JSONIFIER_INLINE void pow10TableGetSig128(const int64_t exp10, uint64_t hilo[2]) noexcept {
		const int64_t idx = exp10 - (pow10SigTable128MinExp);
		std::memcpy(hilo, pow10SigTable128 + idx * 2, 16);
	}

	JSONIFIER_INLINE void f64BiniToDec(uint64_t sigRaw, int64_t expRaw, uint64_t sigBin, int64_t expBin, uint64_t* sigDec, int64_t* expDec) noexcept {
		uint64_t sp{}, mid{};

		const bool isEven			= !(sigBin & 1);
		const bool lowerBoundCloser = (sigRaw == 0 && expRaw > 1);

		const uint64_t cb  = 4 * sigBin;
		const uint64_t cbl = cb - 2 + lowerBoundCloser;
		const uint64_t cbr = cb + 2;

		const int64_t k = (expBin * 315653 - (lowerBoundCloser ? 131237 : 0)) >> 20;

		const int64_t exp10 = -k;
		const int64_t h		= expBin + ((exp10 * 217707) >> 16) + 1;

		uint64_t pow10hilo[2];
		pow10TableGetSig128(exp10, pow10hilo);
		const uint64_t& pow10hi = pow10hilo[0];
		uint64_t& pow10lo		= pow10hilo[1];
		pow10lo += (exp10 < pow10SigTable128MinExactExp || exp10 > pow10SigTable128MaxExactExp);
		const uint64_t vbl = roundToOdd(pow10hi, pow10lo, cbl << h);
		const uint64_t vb  = roundToOdd(pow10hi, pow10lo, cb << h);
		const uint64_t vbr = roundToOdd(pow10hi, pow10lo, cbr << h);

		const uint64_t lower = vbl + !isEven;
		const uint64_t upper = vbr - !isEven;

		bool uInside{}, wInside{};

		const uint64_t s = vb / 4;
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

		mid				   = 4 * s + 2;
		const bool roundUp = (vb > mid) || (vb == mid && (s & 1) != 0);

		*sigDec = s + ((uInside != wInside) ? wInside : roundUp);
		*expDec = k;
	}

	template<typename value_type> JSONIFIER_INLINE value_type* writeUint64TLen15To17Trim(value_type* buf, uint64_t sig) noexcept {
		uint64_t abbccddee = sig / 100000000ull;
		uint64_t ffgghhii  = sig - abbccddee * 100000000ull;
		uint64_t abbcc	   = abbccddee / 10000ull;
		uint64_t ddee	   = abbccddee - abbcc * 10000ull;
		uint64_t abb	   = (abbcc * 167773ull) >> 24;
		uint64_t a		   = (abb * 41) >> 12;
		uint64_t bb		   = abb - a * 100;
		uint64_t cc		   = abbcc - abb * 100;

		buf[0] = static_cast<value_type>(a + 0x30u);
		buf += a > 0;
		bool lz = bb < 10 && a == 0;
		std::memcpy(buf, charTable + (bb * 2 + lz), 2);
		buf -= lz;
		std::memcpy(buf + 2, charTable + 2 * cc, 2);

		if (ffgghhii) {
			uint64_t dd	  = (ddee * 5243) >> 19;
			uint64_t ee	  = ddee - dd * 100;
			uint64_t ffgg = (ffgghhii * 109951163ull) >> 40;
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
				uint64_t tz1 = decTrailingZeroTable[hh];
				uint64_t tz2 = decTrailingZeroTable[ii];
				uint64_t tz	 = ii ? tz2 : (tz1 + 2);
				buf += 16 - tz;
				return buf;
			} else {
				uint64_t tz1 = decTrailingZeroTable[ff];
				uint64_t tz2 = decTrailingZeroTable[gg];
				uint64_t tz	 = gg ? tz2 : (tz1 + 2);
				buf += 12 - tz;
				return buf;
			}
		} else {
			if (ddee) {
				uint64_t dd = (ddee * 5243) >> 19;
				uint64_t ee = ddee - dd * 100;
				std::memcpy(buf + 4, charTable + 2 * dd, 2);
				std::memcpy(buf + 6, charTable + 2 * ee, 2);
				uint64_t tz1 = decTrailingZeroTable[dd];
				uint64_t tz2 = decTrailingZeroTable[ee];
				uint64_t tz	 = ee ? tz2 : (tz1 + 2);
				buf += 8 - tz;
				return buf;
			} else {
				uint64_t tz1 = decTrailingZeroTable[bb];
				uint64_t tz2 = decTrailingZeroTable[cc];
				uint64_t tz	 = cc ? tz2 : (tz1 + tz2);
				buf += 4 - tz;
				return buf;
			}
		}
	}

	consteval uint64_t numbits(uint64_t x) noexcept {
		return x < 2 ? x : 1 + numbits(x >> 1);
	}

	template<std::floating_point value_type01, typename char_type> JSONIFIER_INLINE char_type* toChars(char_type* buffer, value_type01 val) noexcept {
		static_assert(std::numeric_limits<value_type01>::is_iec559);
		static_assert(std::numeric_limits<value_type01>::radix == 2);
		static_assert(std::same_as<float, value_type01> || std::same_as<double, value_type01>);
		static_assert(sizeof(float) == 4 && sizeof(double) == 8);
		using uint64_t = std::conditional_t<std::same_as<float, value_type01>, uint64_t, uint64_t>;

		uint64_t raw{};
		std::memcpy(&raw, &val, sizeof(value_type01));

		constexpr uint64_t exponentBits = numbits(std::numeric_limits<value_type01>::max_exponent - std::numeric_limits<value_type01>::min_exponent + 1);
		constexpr uint64_t sigMask		= uint64_t(-1) >> (exponentBits + 1);
		bool sign						= (raw >> (sizeof(value_type01) * 8 - 1));
		uint64_t sigRaw					= raw & sigMask;
		int64_t expRaw					= static_cast<int64_t>(raw << 1 >> (sizeof(uint64_t) * 8 - exponentBits));

		if (expRaw == (uint64_t(1) << exponentBits) - 1) [[unlikely]] {
			std::memcpy(buffer, "null", 4);
			return buffer + 4;
		}
		if (sign) {
			*buffer = 0x2Du;
			++buffer;
		}
		if ((raw << 1) != 0) [[likely]] {
			uint64_t sigBin;
			int64_t expBin;
			if (expRaw == 0) [[unlikely]] {
				sigBin = sigRaw;
				expBin = 1 - (std::numeric_limits<value_type01>::max_exponent - 1) - (std::numeric_limits<value_type01>::digits - 1);
			} else {
				sigBin = sigRaw | uint64_t(1ull << (std::numeric_limits<value_type01>::digits - 1));
				expBin = static_cast<int64_t>(expRaw) - (std::numeric_limits<value_type01>::max_exponent - 1) - (std::numeric_limits<value_type01>::digits - 1);
			}

			uint64_t sigDec;
			int64_t expDec;
			f64BiniToDec(sigRaw, expRaw, sigBin, expBin, &sigDec, &expDec);
			if constexpr (std::same_as<value_type01, float>) {
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
					buffer[0]	= 0x30u;
					buffer[1]	= 0x2Eu;
					buffer += 2;
					for (; buffer < numHdr; ++buffer)
						*buffer = 0x30u;
					return numEnd;
				} else {
					std::memset(buffer, 0x30u, 8);
					std::memset(buffer + 8, 0x30u, 8);
					std::memset(buffer + 16, 0x30u, 8);
					auto numHdr = buffer + 1;
					auto numEnd = writeUint64TLen15To17Trim(numHdr, sigDec);
					for (int32_t x = 0; x < dotPos; x++)
						buffer[x] = buffer[x + 1];
					buffer[dotPos] = 0x2Eu;
					return ((numEnd - numHdr) <= dotPos) ? buffer + dotPos : numEnd;
				}
			} else {
				auto end = writeUint64TLen15To17Trim(buffer + 1, sigDec);
				end -= (end == buffer + 2);
				expDec += sigLen - 1;
				buffer[0] = buffer[1];
				buffer[1] = 0x2Eu;
				end[0]	  = 0x45u;
				buffer	  = end + 1;
				buffer[0] = 0x2Du;
				buffer += expDec < 0;
				expDec = std::abs(expDec);
				if (expDec < 100) {
					uint64_t lz = expDec < 10;
					std::memcpy(buffer, charTable + (expDec * 2 + lz), 2);
					return buffer + 2 - lz;
				} else {
					const uint64_t hi = (static_cast<uint64_t>(expDec) * 656) >> 16;
					const uint64_t lo = static_cast<uint64_t>(expDec) - hi * 100;
					buffer[0]		  = static_cast<char_type>(hi) + static_cast<char_type>(0x30u);
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