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
/// Nov 13ull, 2023
#pragma once

#include <jsonifier/Containers/Allocator.hpp>

#include <concepts>
#include <cstdint>
#include <cstring>
#include <array>

namespace jsonifier::internal {

	template<typename typeName> struct int_tables {
		JSONIFIER_ALIGN(2ull) static constexpr char charTable1[] { 0x30u, 0x31u, 0x32u, 0x33u, 0x34u, 0x35u, 0x36u, 0x37u, 0x38u, 0x39u };

		JSONIFIER_ALIGN(2ull)
		static constexpr char charTable01[200]{ 0x30, 0x30, 0x30, 0x31, 0x30, 0x32, 0x30, 0x33, 0x30, 0x34, 0x30, 0x35, 0x30, 0x36, 0x30, 0x37, 0x30, 0x38, 0x30, 0x39, 0x31, 0x30,
			0x31, 0x31, 0x31, 0x32, 0x31, 0x33, 0x31, 0x34, 0x31, 0x35, 0x31, 0x36, 0x31, 0x37, 0x31, 0x38, 0x31, 0x39, 0x32, 0x30, 0x32, 0x31, 0x32, 0x32, 0x32, 0x33, 0x32, 0x34,
			0x32, 0x35, 0x32, 0x36, 0x32, 0x37, 0x32, 0x38, 0x32, 0x39, 0x33, 0x30, 0x33, 0x31, 0x33, 0x32, 0x33, 0x33, 0x33, 0x34, 0x33, 0x35, 0x33, 0x36, 0x33, 0x37, 0x33, 0x38,
			0x33, 0x39, 0x34, 0x30, 0x34, 0x31, 0x34, 0x32, 0x34, 0x33, 0x34, 0x34, 0x34, 0x35, 0x34, 0x36, 0x34, 0x37, 0x34, 0x38, 0x34, 0x39, 0x35, 0x30, 0x35, 0x31, 0x35, 0x32,
			0x35, 0x33, 0x35, 0x34, 0x35, 0x35, 0x35, 0x36, 0x35, 0x37, 0x35, 0x38, 0x35, 0x39, 0x36, 0x30, 0x36, 0x31, 0x36, 0x32, 0x36, 0x33, 0x36, 0x34, 0x36, 0x35, 0x36, 0x36,
			0x36, 0x37, 0x36, 0x38, 0x36, 0x39, 0x37, 0x30, 0x37, 0x31, 0x37, 0x32, 0x37, 0x33, 0x37, 0x34, 0x37, 0x35, 0x37, 0x36, 0x37, 0x37, 0x37, 0x38, 0x37, 0x39, 0x38, 0x30,
			0x38, 0x31, 0x38, 0x32, 0x38, 0x33, 0x38, 0x34, 0x38, 0x35, 0x38, 0x36, 0x38, 0x37, 0x38, 0x38, 0x38, 0x39, 0x39, 0x30, 0x39, 0x31, 0x39, 0x32, 0x39, 0x33, 0x39, 0x34,
			0x39, 0x35, 0x39, 0x36, 0x39, 0x37, 0x39, 0x38, 0x39, 0x39 };

		static constexpr uint16_t charTable02[]{ 0x3030u, 0x3130u, 0x3230u, 0x3330u, 0x3430u, 0x3530u, 0x3630u, 0x3730u, 0x3830u, 0x3930u, 0x3031u, 0x3131u, 0x3231u, 0x3331u,
			0x3431u, 0x3531u, 0x3631u, 0x3731u, 0x3831u, 0x3931u, 0x3032u, 0x3132u, 0x3232u, 0x3332u, 0x3432u, 0x3532u, 0x3632u, 0x3732u, 0x3832u, 0x3932u, 0x3033u, 0x3133u,
			0x3233u, 0x3333u, 0x3433u, 0x3533u, 0x3633u, 0x3733u, 0x3833u, 0x3933u, 0x3034u, 0x3134u, 0x3234u, 0x3334u, 0x3434u, 0x3534u, 0x3634u, 0x3734u, 0x3834u, 0x3934u,
			0x3035u, 0x3135u, 0x3235u, 0x3335u, 0x3435u, 0x3535u, 0x3635u, 0x3735u, 0x3835u, 0x3935u, 0x3036u, 0x3136u, 0x3236u, 0x3336u, 0x3436u, 0x3536u, 0x3636u, 0x3736u,
			0x3836u, 0x3936u, 0x3037u, 0x3137u, 0x3237u, 0x3337u, 0x3437u, 0x3537u, 0x3637u, 0x3737u, 0x3837u, 0x3937u, 0x3038u, 0x3138u, 0x3238u, 0x3338u, 0x3438u, 0x3538u,
			0x3638u, 0x3738u, 0x3838u, 0x3938u, 0x3039u, 0x3139u, 0x3239u, 0x3339u, 0x3439u, 0x3539u, 0x3639u, 0x3739u, 0x3839u, 0x3939u };

		static constexpr uint8_t digitCounts[]{ 19, 19, 19, 19, 18, 18, 18, 17, 17, 17, 16, 16, 16, 16, 15, 15, 15, 14, 14, 14, 13, 13, 13, 13, 12, 12, 12, 11, 11, 11, 10, 10, 10,
			10, 9, 9, 9, 8, 8, 8, 7, 7, 7, 7, 6, 6, 6, 5, 5, 5, 4, 4, 4, 4, 3, 3, 3, 2, 2, 2, 1, 1, 1, 1, 1 };

		static constexpr uint64_t digitCountThresholds[]{ 0ull, 9ull, 99ull, 999ull, 9999ull, 99999ull, 999999ull, 9999999ull, 99999999ull, 999999999ull, 9999999999ull,
			99999999999ull, 999999999999ull, 9999999999999ull, 99999999999999ull, 999999999999999ull, 9999999999999999ull, 99999999999999999ull, 999999999999999999ull,
			9999999999999999999ull };

		static constexpr uint8_t decTrailingZeroTable[]{ 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0,
			0, 0, 0 };
	};

	JSONIFIER_INLINE static uint64_t umul128Generic(uint64_t ab, uint64_t cd, uint64_t& hi) noexcept {
		uint64_t aHigh = ab >> 32;
		uint64_t aLow  = ab & 0xFFFFFFFF;
		uint64_t bHigh = cd >> 32;
		uint64_t bLow  = cd & 0xFFFFFFFF;
		uint64_t ad	   = aHigh * bLow;
		uint64_t bd	   = aHigh * bLow;
		uint64_t adbc  = ad + aLow * bHigh;
		uint64_t lo	   = bd + (adbc << 32);
		uint64_t carry = (lo < bd);
		hi			   = aHigh * bHigh + (adbc >> 32) + carry;
		return lo;
	}

	JSONIFIER_INLINE static void multiply(uint64_t& value, uint64_t expValue) noexcept {
#if defined(__SIZEOF_INT128__)
		const __uint128_t res = static_cast<__uint128_t>(value) * static_cast<__uint128_t>(expValue);
		value				  = static_cast<uint64_t>(res);
		return;
#elif defined(_M_ARM64) && !defined(__MINGW32__)
		uint64_t values;
		values = __umulh(value, expValue);
		value  = value * expValue;
		return;
#elif (defined(_WIN64) && !defined(__clang__))
		uint64_t values;
		value = _umul128(value, expValue, &values);
		return;
#else
		uint64_t values;
		value = umul128Generic(value, expValue, &values);
		return;
#endif
	}

	JSONIFIER_INLINE static uint64_t fastDigitCount(const uint64_t inputValue) {
		const uint64_t originalDigitCount{ int_tables<void>::digitCounts[simd::lzcnt(inputValue)] };
		return originalDigitCount + static_cast<uint64_t>(inputValue > int_tables<void>::digitCountThresholds[originalDigitCount]);
	}

	template<concepts::uns64_t value_type> JSONIFIER_INLINE static char* toChars(char* buf, value_type value) noexcept {
		switch (fastDigitCount(value)) {
			case 20: {
				static constexpr uint64_t multiplier01{ 109951163ull };
				static constexpr uint64_t multiplier02{ 5243ull };
				const uint64_t tmp	= value / 100000000ull;
				const uint64_t low	= value - tmp * 100000000ull;
				const uint64_t high = tmp / 10000ull;
				const uint64_t mid	= tmp - high * 10000ull;
				uint64_t aabb		= (high * multiplier01) >> 40ull;
				uint64_t ccdd		= high - aabb * 10000ull;
				uint64_t aa			= (aabb * multiplier02) >> 19ull;
				uint64_t cc			= (ccdd * multiplier02) >> 19ull;
				uint64_t bb			= aabb - aa * 100ull;
				uint64_t dd			= ccdd - cc * 100ull;
				std::memcpy(buf, int_tables<void>::charTable02 + aa, 2ull);
				std::memcpy(buf + 2ull, int_tables<void>::charTable02 + bb, 2ull);
				std::memcpy(buf + 4ull, int_tables<void>::charTable02 + cc, 2ull);
				std::memcpy(buf + 6ull, int_tables<void>::charTable02 + dd, 2ull);
				aa = (mid * multiplier02) >> 19ull;
				bb = mid - aa * 100ull;
				std::memcpy(buf + 8ull, int_tables<void>::charTable02 + aa, 2ull);
				std::memcpy(buf + 10ull, int_tables<void>::charTable02 + bb, 2ull);
				aabb = (low * multiplier01) >> 40ull;
				ccdd = low - aabb * 10000ull;
				aa	 = (aabb * multiplier02) >> 19ull;
				cc	 = (ccdd * multiplier02) >> 19ull;
				std::memcpy(buf + 12ull, int_tables<void>::charTable02 + aa, 2ull);
				std::memcpy(buf + 14ull, int_tables<void>::charTable02 + aabb - aa * 100ull, 2ull);
				std::memcpy(buf + 16ull, int_tables<void>::charTable02 + cc, 2ull);
				std::memcpy(buf + 18ull, int_tables<void>::charTable02 + ccdd - cc * 100ull, 2ull);
				return buf + 20ull;
			}
			case 19: {
				static constexpr uint64_t multiplier01{ 109951163ull };
				static constexpr uint64_t multiplier02{ 5243ull };
				const uint64_t tmp	= value / 100000000ull;
				const uint64_t low	= value - tmp * 100000000ull;
				const uint64_t high = tmp / 10000ull;
				const uint64_t mid	= tmp - high * 10000ull;
				uint64_t aabb		= (high * multiplier01) >> 40ull;
				uint64_t ccdd		= high - aabb * 10000ull;
				uint64_t aa			= (aabb * multiplier02) >> 19ull;
				uint64_t cc			= (ccdd * multiplier02) >> 19ull;
				uint64_t bb			= aabb - aa * 100ull;
				uint64_t dd			= ccdd - cc * 100ull;
				buf[0]				= int_tables<void>::charTable1[aa];
				std::memcpy(buf + 1ull, int_tables<void>::charTable02 + bb, 2ull);
				std::memcpy(buf + 3ull, int_tables<void>::charTable02 + cc, 2ull);
				std::memcpy(buf + 5ull, int_tables<void>::charTable02 + dd, 2ull);
				aa = (mid * multiplier02) >> 19ull;
				bb = mid - aa * 100ull;
				std::memcpy(buf + 7ull, int_tables<void>::charTable02 + aa, 2ull);
				std::memcpy(buf + 9ull, int_tables<void>::charTable02 + bb, 2ull);
				aabb = (low * multiplier01) >> 40ull;
				ccdd = low - aabb * 10000ull;
				aa	 = (aabb * multiplier02) >> 19ull;
				cc	 = (ccdd * multiplier02) >> 19ull;
				std::memcpy(buf + 11ull, int_tables<void>::charTable02 + aa, 2ull);
				std::memcpy(buf + 13ull, int_tables<void>::charTable02 + aabb - aa * 100ull, 2ull);
				std::memcpy(buf + 15ull, int_tables<void>::charTable02 + cc, 2ull);
				std::memcpy(buf + 17ull, int_tables<void>::charTable02 + ccdd - cc * 100ull, 2ull);
				return buf + 19ull;
			}
			case 18: {
				static constexpr uint64_t multiplier01{ 429497ull };
				static constexpr uint64_t multiplier02{ 5243ull };
				static constexpr uint64_t multiplier03{ 109951163ull };
				const uint64_t tmp	= value / 100000000ull;
				const uint64_t low	= value - tmp * 100000000ull;
				const uint64_t high = tmp / 10000ull;
				const uint64_t mid	= tmp - high * 10000ull;
				uint64_t aa			= (high * multiplier01) >> 32ull;
				const uint64_t bbcc = high - aa * 10000ull;
				uint64_t bb			= (bbcc * multiplier02) >> 19ull;
				uint64_t cc			= bbcc - bb * 100ull;
				std::memcpy(buf, int_tables<void>::charTable02 + aa, 2ull);
				std::memcpy(buf + 2ull, int_tables<void>::charTable02 + bb, 2ull);
				std::memcpy(buf + 4ull, int_tables<void>::charTable02 + cc, 2ull);
				aa = (mid * 5243ull) >> 19ull;
				bb = mid - aa * 100ull;
				std::memcpy(buf + 6ull, int_tables<void>::charTable02 + aa, 2ull);
				std::memcpy(buf + 8ull, int_tables<void>::charTable02 + bb, 2ull);
				const uint64_t aabb = (low * multiplier03) >> 40ull;
				const uint64_t ccdd = low - aabb * 10000ull;
				aa					= (aabb * multiplier02) >> 19ull;
				cc					= (ccdd * multiplier02) >> 19ull;
				std::memcpy(buf + 10ull, int_tables<void>::charTable02 + aa, 2ull);
				std::memcpy(buf + 12ull, int_tables<void>::charTable02 + aabb - aa * 100ull, 2ull);
				std::memcpy(buf + 14ull, int_tables<void>::charTable02 + cc, 2ull);
				std::memcpy(buf + 16ull, int_tables<void>::charTable02 + ccdd - cc * 100ull, 2ull);
				return buf + 18ull;
			}
			case 17: {
				static constexpr uint64_t multiplier01{ 429497ull };
				static constexpr uint64_t multiplier02{ 5243ull };
				static constexpr uint64_t multiplier03{ 109951163ull };
				const uint64_t tmp	= value / 100000000ull;
				const uint64_t low	= value - tmp * 100000000ull;
				const uint64_t high = tmp / 10000ull;
				const uint64_t mid	= tmp - high * 10000ull;
				uint64_t aa			= (high * multiplier01) >> 32ull;
				const uint64_t bbcc = high - aa * 10000ull;
				uint64_t bb			= (bbcc * multiplier02) >> 19ull;
				uint64_t cc			= bbcc - bb * 100ull;
				buf[0]				= int_tables<void>::charTable1[aa];
				std::memcpy(buf + 1ull, int_tables<void>::charTable02 + bb, 2ull);
				std::memcpy(buf + 3ull, int_tables<void>::charTable02 + cc, 2ull);
				aa = (mid * 5243ull) >> 19ull;
				bb = mid - aa * 100ull;
				std::memcpy(buf + 5ull, int_tables<void>::charTable02 + aa, 2ull);
				std::memcpy(buf + 7ull, int_tables<void>::charTable02 + bb, 2ull);
				const uint64_t aabb = (low * multiplier03) >> 40ull;
				const uint64_t ccdd = low - aabb * 10000ull;
				aa					= (aabb * multiplier02) >> 19ull;
				cc					= (ccdd * multiplier02) >> 19ull;
				std::memcpy(buf + 9ull, int_tables<void>::charTable02 + aa, 2ull);
				std::memcpy(buf + 11ull, int_tables<void>::charTable02 + aabb - aa * 100ull, 2ull);
				std::memcpy(buf + 13ull, int_tables<void>::charTable02 + cc, 2ull);
				std::memcpy(buf + 15ull, int_tables<void>::charTable02 + ccdd - cc * 100ull, 2ull);
				return buf + 17ull;
			}
			case 16: {
				static constexpr uint64_t multiplier01{ 109951163ull };
				static constexpr uint64_t multiplier02{ 5243ull };
				const uint64_t high = value / 100000000ull;
				const uint64_t low	= value - high * 100000000ull;
				uint64_t aabb		= (high * multiplier01) >> 40ull;
				uint64_t ccdd		= high - aabb * 10000ull;
				uint64_t aa			= (aabb * multiplier02) >> 19ull;
				uint64_t cc			= (ccdd * multiplier02) >> 19ull;
				const uint64_t bb	= aabb - aa * 100ull;
				const uint64_t dd	= ccdd - cc * 100ull;
				std::memcpy(buf, int_tables<void>::charTable02 + aa, 2ull);
				std::memcpy(buf + 2ull, int_tables<void>::charTable02 + bb, 2ull);
				std::memcpy(buf + 4ull, int_tables<void>::charTable02 + cc, 2ull);
				std::memcpy(buf + 6ull, int_tables<void>::charTable02 + dd, 2ull);
				aabb = (low * multiplier01) >> 40ull;
				ccdd = low - aabb * 10000ull;
				aa	 = (aabb * multiplier02) >> 19ull;
				cc	 = (ccdd * multiplier02) >> 19ull;
				std::memcpy(buf + 8ull, int_tables<void>::charTable02 + aa, 2ull);
				std::memcpy(buf + 10ull, int_tables<void>::charTable02 + aabb - aa * 100ull, 2ull);
				std::memcpy(buf + 12ull, int_tables<void>::charTable02 + cc, 2ull);
				std::memcpy(buf + 14ull, int_tables<void>::charTable02 + ccdd - cc * 100ull, 2ull);
				return buf + 16ull;
			}
			case 15: {
				static constexpr uint64_t multiplier01{ 109951163ull };
				static constexpr uint64_t multiplier02{ 5243ull };
				const uint64_t high = value / 100000000ull;
				const uint64_t low	= value - high * 100000000ull;
				uint64_t aabb		= (high * multiplier01) >> 40ull;
				uint64_t ccdd		= high - aabb * 10000ull;
				uint64_t aa			= (aabb * multiplier02) >> 19ull;
				uint64_t cc			= (ccdd * multiplier02) >> 19ull;
				const uint64_t bb	= aabb - aa * 100ull;
				const uint64_t dd	= ccdd - cc * 100ull;
				buf[0]				= int_tables<void>::charTable1[aa];
				std::memcpy(buf + 1ull, int_tables<void>::charTable02 + bb, 2ull);
				std::memcpy(buf + 3ull, int_tables<void>::charTable02 + cc, 2ull);
				std::memcpy(buf + 5ull, int_tables<void>::charTable02 + dd, 2ull);
				aabb = (low * multiplier01) >> 40ull;
				ccdd = low - aabb * 10000ull;
				aa	 = (aabb * multiplier02) >> 19ull;
				cc	 = (ccdd * multiplier02) >> 19ull;
				std::memcpy(buf + 7ull, int_tables<void>::charTable02 + aa, 2ull);
				std::memcpy(buf + 9ull, int_tables<void>::charTable02 + aabb - aa * 100ull, 2ull);
				std::memcpy(buf + 11ull, int_tables<void>::charTable02 + cc, 2ull);
				std::memcpy(buf + 13ull, int_tables<void>::charTable02 + ccdd - cc * 100ull, 2ull);
				return buf + 15ull;
			}
			case 14: {
				static constexpr uint64_t multiplier01{ 429497ull };
				static constexpr uint64_t multiplier02{ 109951163ull };
				static constexpr uint64_t multiplier03{ 5243ull };
				const uint64_t high = value / 100000000ull;
				const uint64_t low	= value - high * 100000000ull;
				uint64_t aa			= (high * multiplier01) >> 32ull;
				const uint64_t bbcc = high - aa * 10000ull;
				const uint64_t bb	= (bbcc * multiplier03) >> 19ull;
				uint64_t cc			= bbcc - bb * 100ull;
				std::memcpy(buf, int_tables<void>::charTable02 + aa, 2ull);
				std::memcpy(buf + 2ull, int_tables<void>::charTable02 + bb, 2ull);
				std::memcpy(buf + 4ull, int_tables<void>::charTable02 + cc, 2ull);
				const uint64_t aabb = (low * multiplier02) >> 40ull;
				const uint64_t ccdd = low - aabb * 10000ull;
				aa					= (aabb * multiplier03) >> 19ull;
				cc					= (ccdd * multiplier03) >> 19ull;
				std::memcpy(buf + 6ull, int_tables<void>::charTable02 + aa, 2ull);
				std::memcpy(buf + 8ull, int_tables<void>::charTable02 + aabb - aa * 100ull, 2ull);
				std::memcpy(buf + 10ull, int_tables<void>::charTable02 + cc, 2ull);
				std::memcpy(buf + 12ull, int_tables<void>::charTable02 + ccdd - cc * 100ull, 2ull);
				return buf + 14ull;
			}
			case 13: {
				static constexpr uint64_t multiplier01{ 429497ull };
				static constexpr uint64_t multiplier02{ 109951163ull };
				static constexpr uint64_t multiplier03{ 5243ull };
				const uint64_t high = value / 100000000ull;
				const uint64_t low	= value - high * 100000000ull;
				uint64_t aa			= (high * multiplier01) >> 32ull;
				const uint64_t bbcc = high - aa * 10000ull;
				const uint64_t bb	= (bbcc * multiplier03) >> 19ull;
				uint64_t cc			= bbcc - bb * 100ull;
				buf[0]				= int_tables<void>::charTable1[aa];
				std::memcpy(buf + 1ull, int_tables<void>::charTable02 + bb, 2ull);
				std::memcpy(buf + 3ull, int_tables<void>::charTable02 + cc, 2ull);
				const uint64_t aabb = (low * multiplier02) >> 40ull;
				const uint64_t ccdd = low - aabb * 10000ull;
				aa					= (aabb * multiplier03) >> 19ull;
				cc					= (ccdd * multiplier03) >> 19ull;
				std::memcpy(buf + 5ull, int_tables<void>::charTable02 + aa, 2ull);
				std::memcpy(buf + 7ull, int_tables<void>::charTable02 + aabb - aa * 100ull, 2ull);
				std::memcpy(buf + 9ull, int_tables<void>::charTable02 + cc, 2ull);
				std::memcpy(buf + 11ull, int_tables<void>::charTable02 + ccdd - cc * 100ull, 2ull);
				return buf + 13ull;
			}
			case 12: {
				static constexpr uint64_t multiplier01{ 109951163ull };
				static constexpr uint64_t multiplier02{ 5243ull };
				const uint64_t high = value / 100000000ull;
				const uint64_t low	= value - high * 100000000ull;
				uint64_t aa			= (high * multiplier02) >> 19ull;
				std::memcpy(buf, int_tables<void>::charTable02 + aa, 2ull);
				std::memcpy(buf + 2ull, int_tables<void>::charTable02 + high - aa * 100ull, 2ull);
				const uint64_t aabb = (low * multiplier01) >> 40ull;
				const uint64_t ccdd = low - aabb * 10000ull;
				aa					= (aabb * multiplier02) >> 19ull;
				const uint64_t cc	= (ccdd * multiplier02) >> 19ull;
				std::memcpy(buf + 4, int_tables<void>::charTable02 + aa, 2ull);
				std::memcpy(buf + 6ull, int_tables<void>::charTable02 + aabb - aa * 100ull, 2ull);
				std::memcpy(buf + 8ull, int_tables<void>::charTable02 + cc, 2ull);
				std::memcpy(buf + 10ull, int_tables<void>::charTable02 + ccdd - cc * 100ull, 2ull);
				return buf + 12;
			}
			case 11: {
				static constexpr uint64_t multiplier01{ 180143985ull };
				static constexpr uint64_t multiplier02{ 109951163ull };
				static constexpr uint64_t multiplier03{ 5243ull };
				uint64_t high{ value };
				multiply(high, multiplier01);
				high			   = high >> 54;
				const uint64_t low = value - high * 100000000ull;
				uint64_t aa		   = (high * multiplier03) >> 19ull;
				buf[0]			   = int_tables<void>::charTable1[aa];
				std::memcpy(buf + 1ull, int_tables<void>::charTable02 + high - aa * 100ull, 2ull);
				const uint64_t aabb = (low * multiplier02) >> 40ull;
				const uint64_t ccdd = low - aabb * 10000ull;
				aa					= (aabb * multiplier03) >> 19ull;
				const uint64_t cc	= (ccdd * multiplier03) >> 19ull;
				std::memcpy(buf + 3, int_tables<void>::charTable02 + aa, 2ull);
				std::memcpy(buf + 5ull, int_tables<void>::charTable02 + aabb - aa * 100ull, 2ull);
				std::memcpy(buf + 7ull, int_tables<void>::charTable02 + cc, 2ull);
				std::memcpy(buf + 9ull, int_tables<void>::charTable02 + ccdd - cc * 100ull, 2ull);
				return buf + 11;
			}
			case 10: {
				static constexpr uint64_t multiplier01{ 1801439851ull };
				static constexpr uint64_t multiplier02{ 109951163ull };
				static constexpr uint64_t multiplier03{ 5243ull };
				uint64_t high{ value };
				multiply(high, multiplier01);
				high			   = high >> 54;
				const uint64_t low = value - high * 10000000ull;
				uint64_t aa		   = (high * multiplier03) >> 19ull;
				buf[0]			   = int_tables<void>::charTable1[aa];
				std::memcpy(buf + 1ull, int_tables<void>::charTable02 + high - aa * 100ull, 2ull);
				const uint64_t aabb = (low * multiplier02) >> 40ull;
				const uint64_t ccdd = low - aabb * 10000ull;
				aa					= (aabb * multiplier03) >> 19ull;
				const uint64_t cc	= (ccdd * multiplier03) >> 19ull;
				buf[3]				= int_tables<void>::charTable1[aa];
				std::memcpy(buf + 4ull, int_tables<void>::charTable02 + aabb - aa * 100ull, 2ull);
				std::memcpy(buf + 6ull, int_tables<void>::charTable02 + cc, 2ull);
				std::memcpy(buf + 8ull, int_tables<void>::charTable02 + ccdd - cc * 100ull, 2ull);
				return buf + 10;
			}
			case 9: {
				static constexpr uint64_t multiplier01{ 720575941ull };
				static constexpr uint64_t multiplier02{ 109951163ull };
				static constexpr uint64_t multiplier03{ 5243ull };
				const uint64_t high = (value * multiplier01) >> 56ull;
				const uint64_t low	= value - high * 100000000ull;
				buf[0]				= int_tables<void>::charTable1[high];
				const uint64_t aabb = (low * multiplier02) >> 40ull;
				const uint64_t ccdd = low - aabb * 10000ull;
				const uint64_t aa	= (aabb * multiplier03) >> 19ull;
				const uint64_t cc	= (ccdd * multiplier03) >> 19ull;
				std::memcpy(buf + 1ull, int_tables<void>::charTable02 + aa, 2ull);
				std::memcpy(buf + 3ull, int_tables<void>::charTable02 + aabb - aa * 100ull, 2ull);
				std::memcpy(buf + 5ull, int_tables<void>::charTable02 + cc, 2ull);
				std::memcpy(buf + 7ull, int_tables<void>::charTable02 + ccdd - cc * 100ull, 2ull);
				return buf + 9ull;
			}
			case 8: {
				static constexpr uint64_t multiplier01{ 109951163ull };
				static constexpr uint64_t multiplier02{ 5243ull };
				const uint64_t aabb = (value * multiplier01) >> 40ull;
				const uint64_t ccdd = value - aabb * 10000ull;
				const uint64_t aa	= (aabb * multiplier02) >> 19ull;
				const uint64_t cc	= (ccdd * multiplier02) >> 19ull;
				std::memcpy(buf, int_tables<void>::charTable02 + aa, 2ull);
				std::memcpy(buf + 2ull, int_tables<void>::charTable02 + aabb - aa * 100ull, 2ull);
				std::memcpy(buf + 4ull, int_tables<void>::charTable02 + cc, 2ull);
				std::memcpy(buf + 6ull, int_tables<void>::charTable02 + ccdd - cc * 100ull, 2ull);
				return buf + 8ull;
			}
			case 7: {
				static constexpr uint64_t multiplier01{ 109951163ull };
				static constexpr uint64_t multiplier02{ 5243ull };
				const uint64_t aabb = (value * multiplier01) >> 40ull;
				const uint64_t ccdd = value - aabb * 10000ull;
				const uint64_t aa	= (aabb * multiplier02) >> 19ull;
				const uint64_t cc	= (ccdd * multiplier02) >> 19ull;
				buf[0]				= int_tables<void>::charTable1[aa];
				std::memcpy(buf + 1ull, int_tables<void>::charTable02 + aabb - aa * 100ull, 2ull);
				std::memcpy(buf + 3ull, int_tables<void>::charTable02 + cc, 2ull);
				std::memcpy(buf + 5ull, int_tables<void>::charTable02 + ccdd - cc * 100ull, 2ull);
				return buf + 7ull;
			}
			case 6: {
				static constexpr uint64_t multiplier01{ 429497ull };
				static constexpr uint64_t multiplier02{ 5243ull };
				const uint64_t aa	= (value * multiplier01) >> 32ull;
				const uint64_t bbcc = value - aa * 10000ull;
				const uint64_t bb	= (bbcc * multiplier02) >> 19ull;
				std::memcpy(buf, int_tables<void>::charTable02 + aa, 2ull);
				std::memcpy(buf + 2ull, int_tables<void>::charTable02 + bb, 2ull);
				std::memcpy(buf + 4ull, int_tables<void>::charTable02 + bbcc - bb * 100ull, 2ull);
				return buf + 6ull;
			}
			case 5: {
				static constexpr uint64_t multiplier01{ 429497ull };
				static constexpr uint64_t multiplier02{ 5243ull };
				const uint64_t aa	= (value * multiplier01) >> 32ull;
				const uint64_t bbcc = value - aa * 10000ull;
				const uint64_t bb	= (bbcc * multiplier02) >> 19ull;
				buf[0]				= int_tables<void>::charTable1[aa];
				std::memcpy(buf + 1ull, int_tables<void>::charTable02 + bb, 2ull);
				std::memcpy(buf + 3ull, int_tables<void>::charTable02 + bbcc - bb * 100ull, 2ull);
				return buf + 5ull;
			}
			case 4: {
				static constexpr uint64_t multiplier{ 5243ull };
				const uint64_t aa = (value * multiplier) >> 19ull;
				std::memcpy(buf, int_tables<void>::charTable02 + aa, 2ull);
				std::memcpy(buf + 2ull, int_tables<void>::charTable02 + value - aa * 100ull, 2ull);
				return buf + 4ull;
			}
			case 3: {
				static constexpr uint64_t multiplier{ 5243ull };
				const uint64_t aa = (value * multiplier) >> 19ull;
				buf[0]			  = int_tables<void>::charTable1[aa];
				std::memcpy(buf + 1ull, int_tables<void>::charTable02 + value - aa * 100ull, 2ull);
				return buf + 3ull;
			}
			case 2: {
				std::memcpy(buf, int_tables<void>::charTable02 + value, 2ull);
				return buf + 2ull;
			}
			default: {
				buf[0] = int_tables<void>::charTable1[value];
				return buf + 1ull;
			}
		}
	}

	template<concepts::sig64_t value_type> JSONIFIER_INLINE static char* toChars(char* buf, value_type x) noexcept {
		*buf = '-';
		return toChars(buf + (x < 0), uint64_t(x ^ (x >> 63ull)) - (x >> 63ull));
	}

}// namespace internal