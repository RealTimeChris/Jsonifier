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

	JSONIFIER_INLINE static char* toChars15Tz(char* buf, uint64_t sig) noexcept {
		const uint32_t abbccddee = static_cast<uint32_t>(sig / 100000000ull);
		const uint32_t ffgghhii	 = static_cast<uint32_t>(sig - static_cast<uint64_t>(abbccddee) * 100000000ull);
		const uint32_t abbcc	 = abbccddee / 10000u;
		const uint32_t ddee		 = abbccddee - abbcc * 10000u;
		const uint32_t abb		 = static_cast<uint32_t>((static_cast<uint64_t>(abbcc) * 167773ull) >> 24);
		const uint32_t a		 = (abb * 41u) >> 12;
		const uint32_t bb		 = abb - a * 100u;
		const uint32_t cc		 = abbcc - abb * 100u;

		buf[0] = static_cast<char>(bb) + '0';
		std::memcpy(buf + 1, int_tables<void>::charTable02 + cc, 2);

		if (ffgghhii) {
			const uint32_t dd	= (ddee * 5243u) >> 19;
			const uint32_t ee	= ddee - dd * 100u;
			const uint32_t ffgg = static_cast<uint32_t>((static_cast<uint64_t>(ffgghhii) * 109951163ull) >> 40);
			const uint32_t hhii = ffgghhii - ffgg * 10000u;
			const uint32_t ff	= (ffgg * 5243u) >> 19;
			const uint32_t gg	= ffgg - ff * 100u;
			std::memcpy(buf + 3, int_tables<void>::charTable02 + dd, 2);
			std::memcpy(buf + 5, int_tables<void>::charTable02 + ee, 2);
			std::memcpy(buf + 7, int_tables<void>::charTable02 + ff, 2);
			std::memcpy(buf + 9, int_tables<void>::charTable02 + gg, 2);
			if (hhii) {
				const uint32_t hh = (hhii * 5243u) >> 19;
				const uint32_t ii = hhii - hh * 100u;
				std::memcpy(buf + 11, int_tables<void>::charTable02 + hh, 2);
				std::memcpy(buf + 13, int_tables<void>::charTable02 + ii, 2);
				const uint32_t tz1 = int_tables<void>::decTrailingZeroTable[hh];
				const uint32_t tz2 = int_tables<void>::decTrailingZeroTable[ii];
				const uint32_t tz  = ii ? tz2 : (tz1 + 2);
				buf += 15 - tz;
				return buf;
			} else {
				const uint32_t tz1 = int_tables<void>::decTrailingZeroTable[ff];
				const uint32_t tz2 = int_tables<void>::decTrailingZeroTable[gg];
				const uint32_t tz  = gg ? tz2 : (tz1 + 2);
				buf += 11 - tz;
				return buf;
			}
		} else {
			if (ddee) {
				const uint32_t dd = (ddee * 5243u) >> 19;
				const uint32_t ee = ddee - dd * 100u;
				std::memcpy(buf + 3, int_tables<void>::charTable02 + dd, 2);
				std::memcpy(buf + 5, int_tables<void>::charTable02 + ee, 2);
				const uint32_t tz1 = int_tables<void>::decTrailingZeroTable[dd];
				const uint32_t tz2 = int_tables<void>::decTrailingZeroTable[ee];
				const uint32_t tz  = ee ? tz2 : (tz1 + 2);
				buf += 7 - tz;
				return buf;
			} else {
				const uint32_t tz1 = int_tables<void>::decTrailingZeroTable[bb];
				const uint32_t tz2 = int_tables<void>::decTrailingZeroTable[cc];
				const uint32_t tz  = cc ? tz2 : (tz1 + tz2);
				buf += 3 - tz;
				return buf;
			}
		}
	}

	JSONIFIER_INLINE static char* toChars16Tz(char* buf, uint64_t sig) noexcept {
		const uint32_t abbccddee = static_cast<uint32_t>(sig / 100000000ull);
		const uint32_t ffgghhii	 = static_cast<uint32_t>(sig - static_cast<uint64_t>(abbccddee) * 100000000ull);
		const uint32_t abbcc	 = abbccddee / 10000u;
		const uint32_t ddee		 = abbccddee - abbcc * 10000u;
		const uint32_t abb		 = static_cast<uint32_t>((static_cast<uint64_t>(abbcc) * 167773ull) >> 24);
		const uint32_t a		 = (abb * 41u) >> 12;
		const uint32_t bb		 = abb - a * 100u;
		const uint32_t cc		 = abbcc - abb * 100u;

		std::memcpy(buf, int_tables<void>::charTable02 + bb, 2);
		std::memcpy(buf + 2, int_tables<void>::charTable02 + cc, 2);

		if (ffgghhii) {
			const uint32_t dd	= (ddee * 5243u) >> 19;
			const uint32_t ee	= ddee - dd * 100u;
			const uint32_t ffgg = static_cast<uint32_t>((static_cast<uint64_t>(ffgghhii) * 109951163ull) >> 40);
			const uint32_t hhii = ffgghhii - ffgg * 10000u;
			const uint32_t ff	= (ffgg * 5243u) >> 19;
			const uint32_t gg	= ffgg - ff * 100u;
			std::memcpy(buf + 4, int_tables<void>::charTable02 + dd, 2);
			std::memcpy(buf + 6, int_tables<void>::charTable02 + ee, 2);
			std::memcpy(buf + 8, int_tables<void>::charTable02 + ff, 2);
			std::memcpy(buf + 10, int_tables<void>::charTable02 + gg, 2);
			if (hhii) {
				const uint32_t hh = (hhii * 5243u) >> 19;
				const uint32_t ii = hhii - hh * 100u;
				std::memcpy(buf + 12, int_tables<void>::charTable02 + hh, 2);
				std::memcpy(buf + 14, int_tables<void>::charTable02 + ii, 2);
				const uint32_t tz1 = int_tables<void>::decTrailingZeroTable[hh];
				const uint32_t tz2 = int_tables<void>::decTrailingZeroTable[ii];
				const uint32_t tz  = ii ? tz2 : (tz1 + 2);
				buf += 16 - tz;
				return buf;
			} else {
				const uint32_t tz1 = int_tables<void>::decTrailingZeroTable[ff];
				const uint32_t tz2 = int_tables<void>::decTrailingZeroTable[gg];
				const uint32_t tz  = gg ? tz2 : (tz1 + 2);
				buf += 12 - tz;
				return buf;
			}
		} else {
			if (ddee) {
				const uint32_t dd = (ddee * 5243u) >> 19;
				const uint32_t ee = ddee - dd * 100u;
				std::memcpy(buf + 4, int_tables<void>::charTable02 + dd, 2);
				std::memcpy(buf + 6, int_tables<void>::charTable02 + ee, 2);
				const uint32_t tz1 = int_tables<void>::decTrailingZeroTable[dd];
				const uint32_t tz2 = int_tables<void>::decTrailingZeroTable[ee];
				const uint32_t tz  = ee ? tz2 : (tz1 + 2);
				buf += 8 - tz;
				return buf;
			} else {
				const uint32_t tz1 = int_tables<void>::decTrailingZeroTable[bb];
				const uint32_t tz2 = int_tables<void>::decTrailingZeroTable[cc];
				const uint32_t tz  = cc ? tz2 : (tz1 + tz2);
				buf += 4 - tz;
				return buf;
			}
		}
	}

	JSONIFIER_INLINE static char* toChars17Tz(char* buf, uint64_t sig) noexcept {
		const uint32_t abbccddee = static_cast<uint32_t>(sig / 100000000ull);
		const uint32_t ffgghhii	 = static_cast<uint32_t>(sig - static_cast<uint64_t>(abbccddee) * 100000000ull);
		const uint32_t abbcc	 = abbccddee / 10000u;
		const uint32_t ddee		 = abbccddee - abbcc * 10000u;
		const uint32_t abb		 = static_cast<uint32_t>((static_cast<uint64_t>(abbcc) * 167773ull) >> 24);
		const uint32_t a		 = (abb * 41u) >> 12;
		const uint32_t bb		 = abb - a * 100u;
		const uint32_t cc		 = abbcc - abb * 100u;

		buf[0] = static_cast<char>(a) + '0';
		std::memcpy(buf + 1, int_tables<void>::charTable02 + bb, 2);
		std::memcpy(buf + 3, int_tables<void>::charTable02 + cc, 2);

		if (ffgghhii) {
			const uint32_t dd	= (ddee * 5243u) >> 19;
			const uint32_t ee	= ddee - dd * 100u;
			const uint32_t ffgg = static_cast<uint32_t>((static_cast<uint64_t>(ffgghhii) * 109951163ull) >> 40);
			const uint32_t hhii = ffgghhii - ffgg * 10000u;
			const uint32_t ff	= (ffgg * 5243u) >> 19;
			const uint32_t gg	= ffgg - ff * 100u;
			std::memcpy(buf + 5, int_tables<void>::charTable02 + dd, 2);
			std::memcpy(buf + 7, int_tables<void>::charTable02 + ee, 2);
			std::memcpy(buf + 9, int_tables<void>::charTable02 + ff, 2);
			std::memcpy(buf + 11, int_tables<void>::charTable02 + gg, 2);
			if (hhii) {
				const uint32_t hh = (hhii * 5243u) >> 19;
				const uint32_t ii = hhii - hh * 100u;
				std::memcpy(buf + 13, int_tables<void>::charTable02 + hh, 2);
				std::memcpy(buf + 15, int_tables<void>::charTable02 + ii, 2);
				const uint32_t tz1 = int_tables<void>::decTrailingZeroTable[hh];
				const uint32_t tz2 = int_tables<void>::decTrailingZeroTable[ii];
				const uint32_t tz  = ii ? tz2 : (tz1 + 2);
				buf += 17 - tz;
				return buf;
			} else {
				const uint32_t tz1 = int_tables<void>::decTrailingZeroTable[ff];
				const uint32_t tz2 = int_tables<void>::decTrailingZeroTable[gg];
				const uint32_t tz  = gg ? tz2 : (tz1 + 2);
				buf += 13 - tz;
				return buf;
			}
		} else {
			if (ddee) {
				const uint32_t dd = (ddee * 5243u) >> 19;
				const uint32_t ee = ddee - dd * 100u;
				std::memcpy(buf + 5, int_tables<void>::charTable02 + dd, 2);
				std::memcpy(buf + 7, int_tables<void>::charTable02 + ee, 2);
				const uint32_t tz1 = int_tables<void>::decTrailingZeroTable[dd];
				const uint32_t tz2 = int_tables<void>::decTrailingZeroTable[ee];
				const uint32_t tz  = ee ? tz2 : (tz1 + 2);
				buf += 9 - tz;
				return buf;
			} else {
				const uint32_t tz1 = int_tables<void>::decTrailingZeroTable[bb];
				const uint32_t tz2 = int_tables<void>::decTrailingZeroTable[cc];
				const uint32_t tz  = cc ? tz2 : (tz1 + tz2);
				buf += 5 - tz;
				return buf;
			}
		}
	}

	JSONIFIER_INLINE static char* toCharsTz(char* buf, uint64_t sig, uint64_t digitCount) noexcept {
		switch (digitCount) {
			case 15:
				return toChars15Tz(buf, sig);
			case 16:
				return toChars16Tz(buf, sig);
			case 17:
				return toChars17Tz(buf, sig);
			default:
				return buf;
		}
	}

	static consteval uint32_t numbits(uint32_t x) noexcept {
		return x < 2 ? x : 1 + numbits(x >> 1);
	}

	JSONIFIER_INLINE static int64_t abs(int64_t value) noexcept {
		const uint64_t temp = static_cast<uint64_t>(value >> 63);
		value ^= temp;
		value += temp & 1;
		return value;
	}

	template<std::floating_point value_type> JSONIFIER_INLINE static char* toChars(char* buf, value_type val) noexcept {
		static_assert(std::numeric_limits<value_type>::is_iec559);
		static_assert(std::numeric_limits<value_type>::radix == 2);
		static_assert(std::is_same_v<float, value_type> || std::is_same_v<double, value_type>);
		static_assert(sizeof(float) == 4 && sizeof(double) == 8);
		using raw = std::conditional_t<std::is_same_v<float, value_type>, uint32_t, uint64_t>;

		if (val != 0.0) {
			using conversion					   = jsonifier_jkj::dragonbox::default_float_bit_carrier_conversion_traits<value_type>;
			using format_traits					   = jsonifier_jkj::dragonbox::ieee754_binary_traits<typename conversion::format, typename conversion::carrier_uint>;
			static constexpr uint32_t expBitsCount = numbits(std::numeric_limits<value_type>::max_exponent - std::numeric_limits<value_type>::min_exponent + 1);
			const auto floatBits				   = jsonifier_jkj::dragonbox::make_float_bits<value_type, conversion, format_traits>(val);
			const auto expBits					   = floatBits.extract_exponent_bits();
			const auto s						   = floatBits.remove_exponent_bits();
			static constexpr auto bitMask{ (static_cast<uint32_t>(1) << expBitsCount) - 1 };

			if (expBits == bitMask) [[unlikely]] {
				std::memcpy(buf, "null", 4);
				return buf + 4;
			}

			*buf						  = '-';
			static constexpr auto zeroNew = value_type(0.0);
			buf += (val < zeroNew);

			const auto v =
				jsonifier_jkj::dragonbox::to_decimal_ex(s, expBits, jsonifier_jkj::dragonbox::policy::sign::ignore, jsonifier_jkj::dragonbox::policy::trailing_zero::ignore);
			const uint64_t sigDec	= static_cast<uint64_t>(v.significand);
			int64_t expDec			= v.exponent;
			const int64_t numDigits = static_cast<int64_t>(fastDigitCount(sigDec));
			const int64_t dotPos	= numDigits + expDec;

			if (-6 < dotPos && dotPos <= 21) {
				if (dotPos <= 0) {
					auto numHdr = buf + (2 - dotPos);
					auto numEnd = toCharsTz(numHdr, sigDec, numDigits);
					buf[0]		= '0';
					buf[1]		= '.';
					buf += 2;
					std::memset(buf, '0', size_t(numHdr - buf));
					return numEnd;
				} else {
					std::memset(buf, '0', 24);
					auto numHdr = buf + 1;
					auto numEnd = toCharsTz(numHdr, sigDec, numDigits);
					std::memmove(buf, buf + 1, size_t(dotPos));
					buf[dotPos] = '.';
					return ((numEnd - numHdr) <= dotPos) ? buf + dotPos : numEnd;
				}
			} else {
				auto end = toCharsTz(buf + 1, sigDec, numDigits);
				end -= (end == buf + 2);
				expDec += numDigits - 1;
				buf[0] = buf[1];
				buf[1] = '.';
				end[0] = 'E';
				buf	   = end + 1;
				buf[0] = '-';
				buf += expDec < 0;

				expDec = abs(expDec);
				if (expDec < 100) {
					uint32_t lz = expDec < 10;
					std::memcpy(buf, int_tables<void>::charTable01 + (expDec * 2 + lz), 2);
					return buf + 2 - lz;
				} else {
					const uint32_t hi = (static_cast<uint32_t>(expDec) * 656) >> 16;
					const uint32_t lo = static_cast<uint32_t>(expDec) - hi * 100u;
					buf[0]			  = static_cast<char>(hi) + '0';
					std::memcpy(&buf[1], int_tables<void>::charTable02 + (lo), 2);
					return buf + 3;
				}
			}
		} else {
			*buf = '-';
			static constexpr auto shiftMask{ (sizeof(value_type) * 8 - 1) };
			buf += (std::bit_cast<raw>(val) >> shiftMask);
			*buf = '0';
			return buf + 1;
		}
	}
}