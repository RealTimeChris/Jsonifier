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

	inline static constexpr uint8_t decTrailingZeroTable[]{ 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0,
		0, 0, 0 };

	inline static constexpr uint8_t digitCounts[]{ 19, 19, 19, 19, 18, 18, 18, 17, 17, 17, 16, 16, 16, 16, 15, 15, 15, 14, 14, 14, 13, 13, 13, 13, 12, 12, 12, 11, 11, 11, 10, 10,
		10, 10, 9, 9, 9, 8, 8, 8, 7, 7, 7, 7, 6, 6, 6, 5, 5, 5, 4, 4, 4, 4, 3, 3, 3, 2, 2, 2, 1, 1, 1, 1, 1 };

	inline static constexpr uint64_t digitCountThresholds[]{ 0ULL, 9ULL, 99ULL, 999ULL, 9999ULL, 99999ULL, 999999ULL, 9999999ULL, 99999999ULL, 999999999ULL, 9999999999ULL,
		99999999999ULL, 999999999999ULL, 9999999999999ULL, 99999999999999ULL, 999999999999999ULL, 9999999999999999ULL, 99999999999999999ULL, 999999999999999999ULL,
		9999999999999999999ULL };

	JSONIFIER_INLINE static uint64_t fastDigitCount(const uint64_t inputValue) {
		const uint64_t originalDigitCount{ digitCounts[std::countl_zero(inputValue)] };
		return originalDigitCount + static_cast<uint64_t>(inputValue > digitCountThresholds[originalDigitCount]);
	}

	template<typename typeName> struct fiwb {
		inline static constexpr char charTable00[]{ 0x30u, 0x31u, 0x32u, 0x33u, 0x34u, 0x35u, 0x36u, 0x37u, 0x38u, 0x39u };

		inline static constexpr char charTable01[]{ 0x30, 0x30, 0x30, 0x31, 0x30, 0x32, 0x30, 0x33, 0x30, 0x34, 0x30, 0x35, 0x30, 0x36, 0x30, 0x37, 0x30, 0x38, 0x30, 0x39, 0x31,
			0x30, 0x31, 0x31, 0x31, 0x32, 0x31, 0x33, 0x31, 0x34, 0x31, 0x35, 0x31, 0x36, 0x31, 0x37, 0x31, 0x38, 0x31, 0x39, 0x32, 0x30, 0x32, 0x31, 0x32, 0x32, 0x32, 0x33, 0x32,
			0x34, 0x32, 0x35, 0x32, 0x36, 0x32, 0x37, 0x32, 0x38, 0x32, 0x39, 0x33, 0x30, 0x33, 0x31, 0x33, 0x32, 0x33, 0x33, 0x33, 0x34, 0x33, 0x35, 0x33, 0x36, 0x33, 0x37, 0x33,
			0x38, 0x33, 0x39, 0x34, 0x30, 0x34, 0x31, 0x34, 0x32, 0x34, 0x33, 0x34, 0x34, 0x34, 0x35, 0x34, 0x36, 0x34, 0x37, 0x34, 0x38, 0x34, 0x39, 0x35, 0x30, 0x35, 0x31, 0x35,
			0x32, 0x35, 0x33, 0x35, 0x34, 0x35, 0x35, 0x35, 0x36, 0x35, 0x37, 0x35, 0x38, 0x35, 0x39, 0x36, 0x30, 0x36, 0x31, 0x36, 0x32, 0x36, 0x33, 0x36, 0x34, 0x36, 0x35, 0x36,
			0x36, 0x36, 0x37, 0x36, 0x38, 0x36, 0x39, 0x37, 0x30, 0x37, 0x31, 0x37, 0x32, 0x37, 0x33, 0x37, 0x34, 0x37, 0x35, 0x37, 0x36, 0x37, 0x37, 0x37, 0x38, 0x37, 0x39, 0x38,
			0x30, 0x38, 0x31, 0x38, 0x32, 0x38, 0x33, 0x38, 0x34, 0x38, 0x35, 0x38, 0x36, 0x38, 0x37, 0x38, 0x38, 0x38, 0x39, 0x39, 0x30, 0x39, 0x31, 0x39, 0x32, 0x39, 0x33, 0x39,
			0x34, 0x39, 0x35, 0x39, 0x36, 0x39, 0x37, 0x39, 0x38, 0x39, 0x39 };

		inline static constexpr uint16_t charTable02[]{ 0x3030, 0x3130, 0x3230, 0x3330, 0x3430, 0x3530, 0x3630, 0x3730, 0x3830, 0x3930, 0x3031, 0x3131, 0x3231, 0x3331, 0x3431,
			0x3531, 0x3631, 0x3731, 0x3831, 0x3931, 0x3032, 0x3132, 0x3232, 0x3332, 0x3432, 0x3532, 0x3632, 0x3732, 0x3832, 0x3932, 0x3033, 0x3133, 0x3233, 0x3333, 0x3433, 0x3533,
			0x3633, 0x3733, 0x3833, 0x3933, 0x3034, 0x3134, 0x3234, 0x3334, 0x3434, 0x3534, 0x3634, 0x3734, 0x3834, 0x3934, 0x3035, 0x3135, 0x3235, 0x3335, 0x3435, 0x3535, 0x3635,
			0x3735, 0x3835, 0x3935, 0x3036, 0x3136, 0x3236, 0x3336, 0x3436, 0x3536, 0x3636, 0x3736, 0x3836, 0x3936, 0x3037, 0x3137, 0x3237, 0x3337, 0x3437, 0x3537, 0x3637, 0x3737,
			0x3837, 0x3937, 0x3038, 0x3138, 0x3238, 0x3338, 0x3438, 0x3538, 0x3638, 0x3738, 0x3838, 0x3938, 0x3039, 0x3139, 0x3239, 0x3339, 0x3439, 0x3539, 0x3639, 0x3739, 0x3839,
			0x3939 };
	};

	template<uint64_t divisor, uint64_t multiplier, uint64_t shift> struct multiply_and_shift {
		template<typename value_type> JSONIFIER_INLINE static uint64_t impl(value_type value) noexcept {
#if JSONIFIER_COMPILER_CLANG || JSONIFIER_COMPILER_GCC
			const __uint128_t product = static_cast<__uint128_t>(value) * multiplier;
			return static_cast<uint64_t>(product >> shift);
#elif JSONIFIER_COMPILER_MSVC
			uint64_t high_part;
			uint64_t low_part = _umul128(multiplier, value, &high_part);
			if constexpr (shift < 64) {
				return static_cast<uint64_t>((low_part >> shift) | (high_part << (64ULL - shift)));
			} else {
				return static_cast<uint64_t>(high_part >> (shift - 64ULL));
			}
#else
			uint64_t high_part;
			const uint64_t low_part = mul128Generic(value, multiplier, &high_part);
			if constexpr (shift < 64) {
				return static_cast<uint64_t>((low_part >> shift) | (high_part << (64ULL - shift)));
			} else {
				return static_cast<uint64_t>(high_part >> (shift - 64ULL));
			}
#endif
		}
	};

	template<uint64_t digit_length> struct to_chars_impl;

	template<> struct to_chars_impl<2> {
		JSONIFIER_INLINE static char* impl(char* buf, const uint64_t value) noexcept {
			const uint64_t lz = value < 10;
			std::memcpy(buf, fiwb<void>::charTable01 + (value * 2 + lz), 2ULL);
			buf -= lz;
			return buf + 2ULL;
		}
	};

	template<> struct to_chars_impl<4> {
		JSONIFIER_INLINE static char* impl(char* buf, const uint64_t value) noexcept {
			const uint64_t aa = (value * 5243u) >> 19ULL;
			const uint64_t lz = value < 1000;
			std::memcpy(buf, fiwb<void>::charTable01 + (aa * 2 + lz), 2ULL);
			buf -= lz;
			std::memcpy(buf + 2ULL, fiwb<void>::charTable02 + (value - aa * 100u), 2ULL);
			return buf + 4ULL;
		}
	};

	template<> struct to_chars_impl<6> {
		JSONIFIER_INLINE static char* impl(char* buf, const uint64_t value) noexcept {
			uint64_t aa		  = (value * 429497ULL) >> 32ULL;
			const uint64_t lz = value < 100000;
			std::memcpy(buf, fiwb<void>::charTable01 + (aa * 2 + lz), 2ULL);
			buf -= lz;
			aa				  = value - aa * 10000u;
			const uint64_t bb = (aa * 5243u) >> 19ULL;
			std::memcpy(buf + 2ULL, fiwb<void>::charTable02 + bb, 2ULL);
			std::memcpy(buf + 4ULL, fiwb<void>::charTable02 + (aa - bb * 100u), 2ULL);
			return buf + 6ULL;
		}
	};

	template<> struct to_chars_impl<8> {
		JSONIFIER_INLINE static char* impl(char* buf, const uint64_t value) noexcept {
			uint64_t aabb	  = (value * 109951163ULL) >> 40ULL;
			uint64_t aa		  = (aabb * 5243u) >> 19ULL;
			const uint64_t lz = value < 10000000;
			std::memcpy(buf, fiwb<void>::charTable01 + (aa * 2 + lz), 2ULL);
			buf -= lz;
			std::memcpy(buf + 2ULL, fiwb<void>::charTable02 + (aabb - aa * 100u), 2ULL);
			aabb = value - aabb * 10000u;
			aa	 = (aabb * 5243u) >> 19ULL;
			std::memcpy(buf + 4ULL, fiwb<void>::charTable02 + aa, 2ULL);
			std::memcpy(buf + 6ULL, fiwb<void>::charTable02 + (aabb - aa * 100u), 2ULL);
			return buf + 8ULL;
		}
	};

	template<> struct to_chars_impl<10> {
		JSONIFIER_INLINE static char* impl(char* buf, const uint64_t value) noexcept {
			const uint64_t high = multiply_and_shift<100000000ULL, 6189700196426901375ULL, 89>::impl(value);
			const uint64_t low	= value - high * 100000000ULL;
			const uint64_t lz	= high < 10;
			std::memcpy(buf, fiwb<void>::charTable01 + (high * 2 + lz), 2ULL);
			buf -= lz;
			uint64_t aabb = (low * 109951163ULL) >> 40ULL;
			uint64_t aa	  = (aabb * 5243u) >> 19ULL;
			std::memcpy(buf + 2ULL, fiwb<void>::charTable02 + aa, 2ULL);
			std::memcpy(buf + 4ULL, fiwb<void>::charTable02 + (aabb - aa * 100u), 2ULL);
			aabb = low - aabb * 10000u;
			aa	 = (aabb * 5243u) >> 19ULL;
			std::memcpy(buf + 6ULL, fiwb<void>::charTable02 + aa, 2ULL);
			std::memcpy(buf + 8ULL, fiwb<void>::charTable02 + (aabb - aa * 100u), 2ULL);
			return buf + 10;
		}
	};

	template<> struct to_chars_impl<12> {
		JSONIFIER_INLINE static char* impl(char* buf, const uint64_t value) noexcept {
			const uint64_t high = multiply_and_shift<100000000ULL, 6189700196426901375ULL, 89>::impl(value);
			const uint64_t low	= value - high * 100000000ULL;
			uint64_t aa			= (high * 5243u) >> 19ULL;
			const uint64_t lz	= aa < 10;
			std::memcpy(buf, fiwb<void>::charTable01 + (aa * 2 + lz), 2ULL);
			buf -= lz;
			std::memcpy(buf + 2ULL, fiwb<void>::charTable02 + (high - aa * 100u), 2ULL);
			uint64_t aabb = (low * 109951163ULL) >> 40ULL;
			aa			  = (aabb * 5243u) >> 19ULL;
			std::memcpy(buf + 4ULL, fiwb<void>::charTable02 + aa, 2ULL);
			std::memcpy(buf + 6ULL, fiwb<void>::charTable02 + (aabb - aa * 100u), 2ULL);
			aabb = low - aabb * 10000u;
			aa	 = (aabb * 5243u) >> 19ULL;
			std::memcpy(buf + 8ULL, fiwb<void>::charTable02 + aa, 2ULL);
			std::memcpy(buf + 10ULL, fiwb<void>::charTable02 + (aabb - aa * 100u), 2ULL);
			return buf + 12ULL;
		}
	};

	template<> struct to_chars_impl<14> {
		JSONIFIER_INLINE static char* impl(char* buf, const uint64_t value) noexcept {
			const uint64_t high = multiply_and_shift<100000000ULL, 6189700196426901375ULL, 89>::impl(value);
			const uint64_t low	= value - high * 100000000ULL;
			uint64_t aa			= (high * 429497ULL) >> 32ULL;
			const uint64_t lz	= aa < 10;
			const uint64_t bbcc = high - aa * 10000u;
			const uint64_t bb	= (bbcc * 5243u) >> 19ULL;
			uint64_t cc			= bbcc - bb * 100u;
			std::memcpy(buf, fiwb<void>::charTable01 + (aa * 2 + lz), 2ULL);
			buf -= lz;
			std::memcpy(buf + 2ULL, fiwb<void>::charTable02 + bb, 2ULL);
			std::memcpy(buf + 4ULL, fiwb<void>::charTable02 + cc, 2ULL);
			const uint64_t aabb = (low * 109951163ULL) >> 40ULL;
			const uint64_t ccdd = low - aabb * 10000u;
			aa					= (aabb * 5243u) >> 19ULL;
			cc					= (ccdd * 5243u) >> 19ULL;
			std::memcpy(buf + 6ULL, fiwb<void>::charTable02 + aa, 2ULL);
			std::memcpy(buf + 8ULL, fiwb<void>::charTable02 + (aabb - aa * 100u), 2ULL);
			std::memcpy(buf + 10ULL, fiwb<void>::charTable02 + cc, 2ULL);
			std::memcpy(buf + 12ULL, fiwb<void>::charTable02 + (ccdd - cc * 100u), 2ULL);
			return buf + 14ULL;
		}
	};

	template<> struct to_chars_impl<16> {
		JSONIFIER_INLINE static char* impl(char* buf, const uint64_t value) noexcept {
			const uint64_t high = multiply_and_shift<100000000ULL, 6189700196426901375ULL, 89>::impl(value);
			const uint64_t low	= value - high * 100000000ULL;
			uint64_t aabb		= (high * 109951163ULL) >> 40ULL;
			uint64_t ccdd		= high - aabb * 10000u;
			uint64_t aa			= (aabb * 5243u) >> 19ULL;
			const uint64_t lz	= aa < 10;
			uint64_t cc			= (ccdd * 5243u) >> 19ULL;
			const uint64_t bb	= aabb - aa * 100u;
			const uint64_t dd	= ccdd - cc * 100u;
			std::memcpy(buf, fiwb<void>::charTable01 + (aa * 2 + lz), 2ULL);
			buf -= lz;
			std::memcpy(buf + 2ULL, fiwb<void>::charTable02 + bb, 2ULL);
			std::memcpy(buf + 4ULL, fiwb<void>::charTable02 + cc, 2ULL);
			std::memcpy(buf + 6ULL, fiwb<void>::charTable02 + dd, 2ULL);
			aabb = (low * 109951163ULL) >> 40ULL;
			ccdd = low - aabb * 10000u;
			aa	 = (aabb * 5243u) >> 19ULL;
			cc	 = (ccdd * 5243u) >> 19ULL;
			std::memcpy(buf + 8ULL, fiwb<void>::charTable02 + aa, 2ULL);
			std::memcpy(buf + 10ULL, fiwb<void>::charTable02 + (aabb - aa * 100u), 2ULL);
			std::memcpy(buf + 12ULL, fiwb<void>::charTable02 + cc, 2ULL);
			std::memcpy(buf + 14ULL, fiwb<void>::charTable02 + (ccdd - cc * 100u), 2ULL);
			return buf + 16ULL;
		}
	};

	template<> struct to_chars_impl<18> {
		JSONIFIER_INLINE static char* impl(char* buf, const uint64_t value) noexcept {
			const uint64_t high	  = multiply_and_shift<100000000ULL, 6189700196426901375ULL, 89>::impl(value);
			const uint64_t low	  = value - high * 100000000ULL;
			const uint64_t high10 = multiply_and_shift<100000000ULL, 6189700196426901375ULL, 89>::impl(high);
			const uint64_t low10  = high - high10 * 100000000ULL;
			const uint64_t lz	  = high10 < 10;
			std::memcpy(buf, fiwb<void>::charTable01 + (high10 * 2 + lz), 2ULL);
			buf -= lz;
			uint64_t aabb = (low10 * 109951163ULL) >> 40ULL;
			uint64_t aa	  = (aabb * 5243u) >> 19ULL;
			std::memcpy(buf + 2ULL, fiwb<void>::charTable02 + aa, 2ULL);
			std::memcpy(buf + 4ULL, fiwb<void>::charTable02 + (aabb - aa * 100u), 2ULL);
			aabb = low10 - aabb * 10000u;
			aa	 = (aabb * 5243u) >> 19ULL;
			std::memcpy(buf + 6ULL, fiwb<void>::charTable02 + aa, 2ULL);
			std::memcpy(buf + 8ULL, fiwb<void>::charTable02 + (aabb - aa * 100u), 2ULL);
			aabb = (low * 109951163ULL) >> 40ULL;
			aa	 = (aabb * 5243u) >> 19ULL;
			std::memcpy(buf + 10ULL, fiwb<void>::charTable02 + aa, 2ULL);
			std::memcpy(buf + 12ULL, fiwb<void>::charTable02 + (aabb - aa * 100u), 2ULL);
			aabb = low - aabb * 10000u;
			aa	 = (aabb * 5243u) >> 19ULL;
			std::memcpy(buf + 14ULL, fiwb<void>::charTable02 + aa, 2ULL);
			std::memcpy(buf + 16ULL, fiwb<void>::charTable02 + (aabb - aa * 100u), 2ULL);
			return buf + 18ULL;
		}
	};

	template<> struct to_chars_impl<20> {
		JSONIFIER_INLINE static char* impl(char* buf, const uint64_t value) noexcept {
			const uint64_t high	  = multiply_and_shift<100000000ULL, 6189700196426901375ULL, 89>::impl(value);
			const uint64_t low	  = value - high * 100000000ULL;
			const uint64_t high12 = multiply_and_shift<100000000ULL, 6189700196426901375ULL, 89>::impl(high);
			const uint64_t low12  = high - high12 * 100000000ULL;
			uint64_t aa			  = (high12 * 5243u) >> 19ULL;
			const uint64_t lz	  = aa < 10;
			std::memcpy(buf, fiwb<void>::charTable01 + (aa * 2 + lz), 2ULL);
			buf -= lz;
			std::memcpy(buf + 2ULL, fiwb<void>::charTable02 + (high12 - aa * 100u), 2ULL);
			uint64_t aabb = (low12 * 109951163ULL) >> 40ULL;
			aa			  = (aabb * 5243u) >> 19ULL;
			std::memcpy(buf + 4ULL, fiwb<void>::charTable02 + aa, 2ULL);
			std::memcpy(buf + 6ULL, fiwb<void>::charTable02 + (aabb - aa * 100u), 2ULL);
			aabb = low12 - aabb * 10000u;
			aa	 = (aabb * 5243u) >> 19ULL;
			std::memcpy(buf + 8ULL, fiwb<void>::charTable02 + aa, 2ULL);
			std::memcpy(buf + 10ULL, fiwb<void>::charTable02 + (aabb - aa * 100u), 2ULL);
			aabb = (low * 109951163ULL) >> 40ULL;
			aa	 = (aabb * 5243u) >> 19ULL;
			std::memcpy(buf + 12ULL, fiwb<void>::charTable02 + aa, 2ULL);
			std::memcpy(buf + 14ULL, fiwb<void>::charTable02 + (aabb - aa * 100u), 2ULL);
			aabb = low - aabb * 10000u;
			aa	 = (aabb * 5243u) >> 19ULL;
			std::memcpy(buf + 16ULL, fiwb<void>::charTable02 + aa, 2ULL);
			std::memcpy(buf + 18ULL, fiwb<void>::charTable02 + (aabb - aa * 100u), 2ULL);
			return buf + 20ULL;
		}
	};

	template<typename value_type> struct to_chars;

	template<std::integral value_type> struct to_chars<value_type> {
		template<concepts::uns64_t value_type_new> JSONIFIER_INLINE static char* impl(char* buf, const value_type_new value) noexcept {
			if (value < 10000ULL) {
				if (value < 100ULL) {
					return to_chars_impl<2>::impl(buf, value);
				} else {
					return to_chars_impl<4>::impl(buf, value);
				}
			} else if (value < 100000000ULL) {
				if (value < 1000000ULL) {
					return to_chars_impl<6>::impl(buf, value);
				} else {
					return to_chars_impl<8>::impl(buf, value);
				}
			} else if (value < 1000000000000ULL) {
				if (value < 10000000000ULL) {
					return to_chars_impl<10>::impl(buf, value);
				} else {
					return to_chars_impl<12>::impl(buf, value);
				}
			} else if (value < 10000000000000000ULL) {
				if (value < 100000000000000ULL) {
					return to_chars_impl<14>::impl(buf, value);
				} else {
					return to_chars_impl<16>::impl(buf, value);
				}
			} else if (value < 1000000000000000000ULL) {
				return to_chars_impl<18>::impl(buf, value);
			} else {
				return to_chars_impl<20>::impl(buf, value);
			}
		}

		template<concepts::sig64_t value_type_new> JSONIFIER_INLINE static char* impl(char* buf, const value_type_new value) noexcept {
			constexpr auto shift_amount = sizeof(value_type_new) * 8 - 1;
			using unsigned_type			= std::make_unsigned_t<value_type_new>;
			*buf						= '-';
			return to_chars::impl(buf + (value < 0), static_cast<uint64_t>((static_cast<unsigned_type>(value) ^ (value >> shift_amount)) - (value >> shift_amount)));
		}
	};

};// namespace internal
