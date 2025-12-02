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

		inline static constexpr uint8_t decTrailingZeroTable[]{ 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0,
			0, 0, 0, 0, 0 };

		inline static constexpr uint8_t digitCounts[]{ 19, 19, 19, 19, 18, 18, 18, 17, 17, 17, 16, 16, 16, 16, 15, 15, 15, 14, 14, 14, 13, 13, 13, 13, 12, 12, 12, 11, 11, 11, 10,
			10, 10, 10, 9, 9, 9, 8, 8, 8, 7, 7, 7, 7, 6, 6, 6, 5, 5, 5, 4, 4, 4, 4, 3, 3, 3, 2, 2, 2, 1, 1, 1, 1, 1 };

		inline static constexpr uint64_t digitCountThresholds[]{ 0ULL, 9ULL, 99ULL, 999ULL, 9999ULL, 99999ULL, 999999ULL, 9999999ULL, 99999999ULL, 999999999ULL, 9999999999ULL,
			99999999999ULL, 999999999999ULL, 9999999999999ULL, 99999999999999ULL, 999999999999999ULL, 9999999999999999ULL, 99999999999999999ULL, 999999999999999999ULL,
			9999999999999999999ULL };
	};

	JSONIFIER_INLINE static uint64_t fastDigitCount(const uint64_t inputValue) {
		const uint64_t originalDigitCount{ int_tables<void>::digitCounts[std::countl_zero(inputValue)] };
		return originalDigitCount + static_cast<uint64_t>(inputValue > int_tables<void>::digitCountThresholds[originalDigitCount]);
	}

	template<uint64_t divisor, uint64_t multiplier, uint32_t shift> struct multiply_and_shift {
		template<typename value_type> JSONIFIER_INLINE static uint64_t impl(value_type value) noexcept {
#if defined(JSONIFIER_CLANG) && defined(_M_ARM64) && !defined(__MINGW32__)
			return value / divisor;
#else
	#if defined(__SIZEOF_INT128__)
			const __int128_t product = static_cast<__int128_t>(value) * multiplier;
			return static_cast<uint64_t>(product >> shift);
	#elif defined(_M_ARM64) && !defined(__MINGW32__)
			value_type values;
			values = __umulh(value, expValue);
			value  = value * expValue;
			return values == 0;
	#elif (defined(_WIN64) && !defined(__clang__))
			uint64_t high_part;
			const uint64_t low_part = _umul128(value, multiplier, &high_part);
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
#endif
		}
	};

	template<uint64_t, typename value_type> struct to_chars_impl {};

	template<typename value_type> struct to_chars_impl<1, value_type> {
		JSONIFIER_INLINE static char* impl(char* buf, const value_type value) noexcept {
			buf[0] = int_tables<void>::charTable00[value];
			return buf + 1ULL;
		}
	};

	template<typename value_type> struct to_chars_impl<2, value_type> {};
	template<typename value_type> struct to_chars_impl<3, value_type> {};
	template<typename value_type> struct to_chars_impl<4, value_type> {};
	template<typename value_type> struct to_chars_impl<5, value_type> {};
	template<typename value_type> struct to_chars_impl<6, value_type> {};
	template<typename value_type> struct to_chars_impl<7, value_type> {};
	template<typename value_type> struct to_chars_impl<8, value_type> {};
	template<typename value_type> struct to_chars_impl<9, value_type> {};
	template<typename value_type> struct to_chars_impl<10, value_type> {};
	template<typename value_type> struct to_chars_impl<11, value_type> {};
	template<typename value_type> struct to_chars_impl<12, value_type> {};
	template<typename value_type> struct to_chars_impl<13, value_type> {};
	template<typename value_type> struct to_chars_impl<14, value_type> {};
	template<typename value_type> struct to_chars_impl<15, value_type> {};
	template<typename value_type> struct to_chars_impl<16, value_type> {};
	template<typename value_type> struct to_chars_impl<17, value_type> {};
	template<typename value_type> struct to_chars_impl<18, value_type> {};
	template<typename value_type> struct to_chars_impl<19, value_type> {};
	template<typename value_type> struct to_chars_impl<20, value_type> {};


	template<typename value_type> JSONIFIER_INLINE static char* toChars1(char* buf, const value_type value) noexcept {
		buf[0] = int_tables<void>::charTable00[value];
		return buf + 1ULL;
	}

	template<typename value_type> JSONIFIER_INLINE static char* toChars2(char* buf, const value_type value) noexcept {
		std::memcpy(buf, int_tables<void>::charTable02 + value, 2ULL);
		return buf + 2ULL;
	}

	template<typename value_type> JSONIFIER_INLINE static char* toChars3(char* buf, const value_type value) noexcept {
		const uint32_t aa = (value * 5243u) >> 19ULL;
		buf[0]			  = int_tables<void>::charTable00[aa];
		std::memcpy(buf + 1ULL, int_tables<void>::charTable02 + (value - aa * 100u), 2ULL);
		return buf + 3ULL;
	}

	template<typename value_type> JSONIFIER_INLINE static char* toChars4(char* buf, const value_type value) noexcept {
		const uint32_t aa = (value * 5243u) >> 19ULL;
		std::memcpy(buf, int_tables<void>::charTable02 + aa, 2ULL);
		std::memcpy(buf + 2ULL, int_tables<void>::charTable02 + (value - aa * 100u), 2ULL);
		return buf + 4ULL;
	}

	template<typename value_type> JSONIFIER_INLINE static char* toChars5(char* buf, const value_type value) noexcept {
		uint32_t aa		  = static_cast<uint32_t>((value * 429497ULL) >> 32ULL);
		buf[0]			  = int_tables<void>::charTable00[aa];
		aa				  = value - aa * 10000u;
		const uint32_t bb = (aa * 5243u) >> 19ULL;
		std::memcpy(buf + 1ULL, int_tables<void>::charTable02 + bb, 2ULL);
		std::memcpy(buf + 3ULL, int_tables<void>::charTable02 + (aa - bb * 100u), 2ULL);
		return buf + 5ULL;
	}

	template<typename value_type> JSONIFIER_INLINE static char* toChars6(char* buf, const value_type value) noexcept {
		uint32_t aa = static_cast<uint32_t>((value * 429497ULL) >> 32ULL);
		std::memcpy(buf, int_tables<void>::charTable02 + aa, 2ULL);
		aa				  = value - aa * 10000u;
		const uint32_t bb = (aa * 5243u) >> 19ULL;
		std::memcpy(buf + 2ULL, int_tables<void>::charTable02 + bb, 2ULL);
		std::memcpy(buf + 4ULL, int_tables<void>::charTable02 + (aa - bb * 100u), 2ULL);
		return buf + 6ULL;
	}

	template<typename value_type> JSONIFIER_INLINE static char* toChars7(char* buf, const value_type value) noexcept {
		uint32_t aabb = static_cast<uint32_t>((value * 109951163ULL) >> 40ULL);
		uint32_t aa	  = (aabb * 5243u) >> 19ULL;
		buf[0]		  = int_tables<void>::charTable00[aa];
		std::memcpy(buf + 1ULL, int_tables<void>::charTable02 + (aabb - aa * 100u), 2ULL);
		aabb = value - aabb * 10000u;
		aa	 = (aabb * 5243u) >> 19ULL;
		std::memcpy(buf + 3ULL, int_tables<void>::charTable02 + aa, 2ULL);
		std::memcpy(buf + 5ULL, int_tables<void>::charTable02 + (aabb - aa * 100u), 2ULL);
		return buf + 7ULL;
	}

	template<typename value_type> JSONIFIER_INLINE static char* toChars8(char* buf, const value_type value) noexcept {
		uint32_t aabb = static_cast<uint32_t>((value * 109951163ULL) >> 40ULL);
		uint32_t aa	  = (aabb * 5243u) >> 19ULL;
		std::memcpy(buf, int_tables<void>::charTable02 + aa, 2ULL);
		std::memcpy(buf + 2ULL, int_tables<void>::charTable02 + (aabb - aa * 100u), 2ULL);
		aabb = value - aabb * 10000u;
		aa	 = (aabb * 5243u) >> 19ULL;
		std::memcpy(buf + 4ULL, int_tables<void>::charTable02 + aa, 2ULL);
		std::memcpy(buf + 6ULL, int_tables<void>::charTable02 + (aabb - aa * 100u), 2ULL);
		return buf + 8ULL;
	}

	template<typename value_type> JSONIFIER_INLINE static char* toChars9(char* buf, const value_type value) noexcept {
		uint32_t high = static_cast<uint32_t>((value * 720575941ULL) >> 56ULL);
		buf[0]		  = int_tables<void>::charTable00[high];
		high		  = value - high * 100000000u;
		uint32_t aabb = static_cast<uint32_t>((high * 109951163ULL) >> 40ULL);
		uint32_t aa	  = (aabb * 5243u) >> 19ULL;
		std::memcpy(buf + 1ULL, int_tables<void>::charTable02 + aa, 2ULL);
		std::memcpy(buf + 3ULL, int_tables<void>::charTable02 + (aabb - aa * 100u), 2ULL);
		aabb = high - aabb * 10000u;
		aa	 = (aabb * 5243u) >> 19ULL;
		std::memcpy(buf + 5ULL, int_tables<void>::charTable02 + aa, 2ULL);
		std::memcpy(buf + 7ULL, int_tables<void>::charTable02 + (aabb - aa * 100u), 2ULL);
		return buf + 9ULL;
	}

	template<typename value_type> JSONIFIER_INLINE static char* toChars10(char* buf, const value_type value) noexcept {
		const uint32_t high = static_cast<uint32_t>((value * 1801439851ULL) >> 54ULL);
		const uint32_t low	= static_cast<uint32_t>(value - high * 10000000ULL);
		uint32_t aa			= (high * 5243u) >> 19ULL;
		buf[0]				= int_tables<void>::charTable00[aa];
		std::memcpy(buf + 1ULL, int_tables<void>::charTable02 + (high - aa * 100u), 2ULL);
		uint32_t aabb = static_cast<uint32_t>((low * 109951163ULL) >> 40ULL);
		aa			  = (aabb * 5243u) >> 19ULL;
		buf[3]		  = int_tables<void>::charTable00[aa];
		std::memcpy(buf + 4ULL, int_tables<void>::charTable02 + (aabb - aa * 100u), 2ULL);
		aabb = low - aabb * 10000u;
		aa	 = (aabb * 5243u) >> 19ULL;
		std::memcpy(buf + 6ULL, int_tables<void>::charTable02 + aa, 2ULL);
		std::memcpy(buf + 8ULL, int_tables<void>::charTable02 + (aabb - aa * 100u), 2ULL);
		return buf + 10;
	}

	template<typename value_type> JSONIFIER_INLINE static char* toChars11(char* buf, const value_type value) noexcept {
		const uint64_t high = multiply_and_shift<10000000ULL, 15474250491067253437ULL, 87>::impl(value);
		const uint32_t low	= static_cast<uint32_t>(value - high * 10000000ULL);
		uint32_t aa			= (high * 5243u) >> 19ULL;
		std::memcpy(buf, int_tables<void>::charTable02 + aa, 2ULL);
		std::memcpy(buf + 2ULL, int_tables<void>::charTable02 + (high - aa * 100u), 2ULL);
		uint32_t aabb = static_cast<uint32_t>((low * 109951163ULL) >> 40ULL);
		aa			  = (aabb * 5243u) >> 19ULL;
		buf[4]		  = int_tables<void>::charTable00[aa];
		std::memcpy(buf + 5ULL, int_tables<void>::charTable02 + (aabb - aa * 100u), 2ULL);
		aabb = low - aabb * 10000u;
		aa	 = (aabb * 5243u) >> 19ULL;
		std::memcpy(buf + 7ULL, int_tables<void>::charTable02 + aa, 2ULL);
		std::memcpy(buf + 9ULL, int_tables<void>::charTable02 + (aabb - aa * 100u), 2ULL);
		return buf + 11;
	}

	template<typename value_type> JSONIFIER_INLINE static char* toChars12(char* buf, const value_type value) noexcept {
		const uint64_t high = multiply_and_shift<100000000ULL, 6189700196426901375ULL, 89>::impl(value);
		const uint64_t low	= static_cast<uint64_t>(value - high * 100000000ULL);
		uint64_t aa			= (high * 5243u) >> 19ULL;
		std::memcpy(buf, int_tables<void>::charTable02 + aa, 2ULL);
		std::memcpy(buf + 2ULL, int_tables<void>::charTable02 + (high - aa * 100u), 2ULL);
		uint64_t aabb = static_cast<uint64_t>((low * 109951163ULL) >> 40ULL);
		aa			  = (aabb * 5243u) >> 19ULL;
		std::memcpy(buf + 4ULL, int_tables<void>::charTable02 + aa, 2ULL);
		std::memcpy(buf + 6ULL, int_tables<void>::charTable02 + (aabb - aa * 100u), 2ULL);
		aabb = low - aabb * 10000u;
		aa	 = (aabb * 5243u) >> 19ULL;
		std::memcpy(buf + 8ULL, int_tables<void>::charTable02 + aa, 2ULL);
		std::memcpy(buf + 10ULL, int_tables<void>::charTable02 + (aabb - aa * 100u), 2ULL);
		return buf + 12;
	}

	template<typename value_type> JSONIFIER_INLINE static char* toChars13(char* buf, const value_type value) noexcept {
		const uint64_t high = multiply_and_shift<100000000ULL, 6189700196426901375ULL, 89>::impl(value);
		const uint32_t low	= static_cast<uint32_t>(value - high * 100000000ULL);
		uint32_t aa			= static_cast<uint32_t>((high * 429497ULL) >> 32ULL);
		const uint32_t bbcc = high - aa * 10000u;
		const uint32_t bb	= (bbcc * 5243u) >> 19ULL;
		uint32_t cc			= bbcc - bb * 100u;
		buf[0]				= int_tables<void>::charTable00[aa];
		std::memcpy(buf + 1ULL, int_tables<void>::charTable02 + bb, 2ULL);
		std::memcpy(buf + 3ULL, int_tables<void>::charTable02 + cc, 2ULL);
		const uint32_t aabb = static_cast<uint32_t>((low * 109951163ULL) >> 40ULL);
		const uint32_t ccdd = low - aabb * 10000u;
		aa					= (aabb * 5243u) >> 19ULL;
		cc					= (ccdd * 5243u) >> 19ULL;
		std::memcpy(buf + 5ULL, int_tables<void>::charTable02 + aa, 2ULL);
		std::memcpy(buf + 7ULL, int_tables<void>::charTable02 + (aabb - aa * 100u), 2ULL);
		std::memcpy(buf + 9ULL, int_tables<void>::charTable02 + cc, 2ULL);
		std::memcpy(buf + 11ULL, int_tables<void>::charTable02 + (ccdd - cc * 100u), 2ULL);
		return buf + 13ULL;
	}

	template<typename value_type> JSONIFIER_INLINE static char* toChars14(char* buf, const value_type value) noexcept {
		const uint64_t high = multiply_and_shift<100000000ULL, 6189700196426901375ULL, 89>::impl(value);
		const uint32_t low	= static_cast<uint32_t>(value - high * 100000000ULL);
		uint32_t aa			= static_cast<uint32_t>((high * 429497ULL) >> 32ULL);
		const uint32_t bbcc = high - aa * 10000u;
		const uint32_t bb	= (bbcc * 5243u) >> 19ULL;
		uint32_t cc			= bbcc - bb * 100u;
		std::memcpy(buf, int_tables<void>::charTable02 + aa, 2ULL);
		std::memcpy(buf + 2ULL, int_tables<void>::charTable02 + bb, 2ULL);
		std::memcpy(buf + 4ULL, int_tables<void>::charTable02 + cc, 2ULL);
		const uint32_t aabb = static_cast<uint32_t>((low * 109951163ULL) >> 40ULL);
		const uint32_t ccdd = low - aabb * 10000u;
		aa					= (aabb * 5243u) >> 19ULL;
		cc					= (ccdd * 5243u) >> 19ULL;
		std::memcpy(buf + 6ULL, int_tables<void>::charTable02 + aa, 2ULL);
		std::memcpy(buf + 8ULL, int_tables<void>::charTable02 + (aabb - aa * 100u), 2ULL);
		std::memcpy(buf + 10ULL, int_tables<void>::charTable02 + cc, 2ULL);
		std::memcpy(buf + 12ULL, int_tables<void>::charTable02 + (ccdd - cc * 100u), 2ULL);
		return buf + 14ULL;
	}

	template<typename value_type> JSONIFIER_INLINE static char* toChars15(char* buf, const value_type value) noexcept {
		const uint64_t high = multiply_and_shift<100000000ULL, 6189700196426901375ULL, 89>::impl(value);
		const uint32_t low	= static_cast<uint32_t>(value - high * 100000000ULL);
		uint32_t aabb		= static_cast<uint32_t>((high * 109951163ULL) >> 40ULL);
		uint32_t ccdd		= static_cast<uint32_t>(high - aabb * 10000u);
		uint32_t aa			= (aabb * 5243u) >> 19ULL;
		uint32_t cc			= (ccdd * 5243u) >> 19ULL;
		const uint32_t bb	= aabb - aa * 100u;
		const uint32_t dd	= ccdd - cc * 100u;
		buf[0]				= int_tables<void>::charTable00[aa];
		std::memcpy(buf + 1ULL, int_tables<void>::charTable02 + bb, 2ULL);
		std::memcpy(buf + 3ULL, int_tables<void>::charTable02 + cc, 2ULL);
		std::memcpy(buf + 5ULL, int_tables<void>::charTable02 + dd, 2ULL);
		aabb = static_cast<uint32_t>((low * 109951163ULL) >> 40ULL);
		ccdd = low - aabb * 10000u;
		aa	 = (aabb * 5243u) >> 19ULL;
		cc	 = (ccdd * 5243u) >> 19ULL;
		std::memcpy(buf + 7ULL, int_tables<void>::charTable02 + aa, 2ULL);
		std::memcpy(buf + 9ULL, int_tables<void>::charTable02 + (aabb - aa * 100u), 2ULL);
		std::memcpy(buf + 11ULL, int_tables<void>::charTable02 + cc, 2ULL);
		std::memcpy(buf + 13ULL, int_tables<void>::charTable02 + (ccdd - cc * 100u), 2ULL);
		return buf + 15ULL;
	}

	template<typename value_type> JSONIFIER_INLINE static char* toChars16(char* buf, const value_type value) noexcept {
		const uint64_t high = multiply_and_shift<100000000ULL, 6189700196426901375ULL, 89>::impl(value);
		const uint32_t low	= static_cast<uint32_t>(value - high * 100000000ULL);
		uint32_t aabb		= static_cast<uint32_t>((high * 109951163ULL) >> 40ULL);
		uint32_t ccdd		= static_cast<uint32_t>(high - aabb * 10000u);
		uint32_t aa			= (aabb * 5243u) >> 19ULL;
		uint32_t cc			= (ccdd * 5243u) >> 19ULL;
		const uint32_t bb	= aabb - aa * 100u;
		const uint32_t dd	= ccdd - cc * 100u;
		std::memcpy(buf, int_tables<void>::charTable02 + aa, 2ULL);
		std::memcpy(buf + 2ULL, int_tables<void>::charTable02 + bb, 2ULL);
		std::memcpy(buf + 4ULL, int_tables<void>::charTable02 + cc, 2ULL);
		std::memcpy(buf + 6ULL, int_tables<void>::charTable02 + dd, 2ULL);
		aabb = static_cast<uint32_t>((low * 109951163ULL) >> 40ULL);
		ccdd = low - aabb * 10000u;
		aa	 = (aabb * 5243u) >> 19ULL;
		cc	 = (ccdd * 5243u) >> 19ULL;
		std::memcpy(buf + 8ULL, int_tables<void>::charTable02 + aa, 2ULL);
		std::memcpy(buf + 10ULL, int_tables<void>::charTable02 + (aabb - aa * 100u), 2ULL);
		std::memcpy(buf + 12ULL, int_tables<void>::charTable02 + cc, 2ULL);
		std::memcpy(buf + 14ULL, int_tables<void>::charTable02 + (ccdd - cc * 100u), 2ULL);
		return buf + 16ULL;
	}

	template<typename value_type> JSONIFIER_INLINE static char* toChars17(char* buf, const value_type value) noexcept {
		const uint64_t high	 = multiply_and_shift<100000000ULL, 6189700196426901375ULL, 89>::impl(value);
		const uint64_t low	 = value - high * 100000000ULL;
		const uint64_t high9 = (high * 720575941ULL) >> 56ULL;
		const uint64_t low9	 = high - high9 * 100000000ULL;
		buf[0]				 = int_tables<void>::charTable00[high9];
		uint64_t aabb		 = (low9 * 109951163ULL) >> 40ULL;
		uint64_t aa			 = (aabb * 5243u) >> 19ULL;
		std::memcpy(buf + 1ULL, int_tables<void>::charTable02 + aa, 2ULL);
		std::memcpy(buf + 3ULL, int_tables<void>::charTable02 + (aabb - aa * 100u), 2ULL);
		aabb = low9 - aabb * 10000u;
		aa	 = (aabb * 5243u) >> 19ULL;
		std::memcpy(buf + 5ULL, int_tables<void>::charTable02 + aa, 2ULL);
		std::memcpy(buf + 7ULL, int_tables<void>::charTable02 + (aabb - aa * 100u), 2ULL);
		aabb = (low * 109951163ULL) >> 40ULL;
		aa	 = (aabb * 5243u) >> 19ULL;
		std::memcpy(buf + 9ULL, int_tables<void>::charTable02 + aa, 2ULL);
		std::memcpy(buf + 11ULL, int_tables<void>::charTable02 + (aabb - aa * 100u), 2ULL);
		aabb = low - aabb * 10000u;
		aa	 = (aabb * 5243u) >> 19ULL;
		std::memcpy(buf + 13ULL, int_tables<void>::charTable02 + aa, 2ULL);
		std::memcpy(buf + 15ULL, int_tables<void>::charTable02 + (aabb - aa * 100u), 2ULL);
		return buf + 17;
	}

	template<typename value_type> JSONIFIER_INLINE static char* toChars18(char* buf, const value_type value) noexcept {
		const uint64_t high	  = multiply_and_shift<100000000ULL, 6189700196426901375ULL, 89>::impl(value);
		const uint64_t low	  = value - high * 100000000ULL;
		const uint64_t high10 = (high * 1801439851ULL) >> 54;
		const uint64_t low10  = high - high10 * 10000000ULL;
		uint64_t aa			  = (high10 * 5243u) >> 19ULL;
		buf[0]				  = int_tables<void>::charTable00[aa];
		std::memcpy(buf + 1ULL, int_tables<void>::charTable02 + (high10 - aa * 100u), 2ULL);
		uint64_t aabb = (low10 * 109951163ULL) >> 40ULL;
		aa			  = (aabb * 5243u) >> 19ULL;
		buf[3]		  = int_tables<void>::charTable00[aa];
		std::memcpy(buf + 4ULL, int_tables<void>::charTable02 + (aabb - aa * 100u), 2ULL);
		aabb = low10 - aabb * 10000u;
		aa	 = (aabb * 5243u) >> 19ULL;
		std::memcpy(buf + 6ULL, int_tables<void>::charTable02 + aa, 2ULL);
		std::memcpy(buf + 8ULL, int_tables<void>::charTable02 + (aabb - aa * 100u), 2ULL);
		aabb = (low * 109951163ULL) >> 40ULL;
		aa	 = (aabb * 5243u) >> 19ULL;
		std::memcpy(buf + 10ULL, int_tables<void>::charTable02 + aa, 2ULL);
		std::memcpy(buf + 12ULL, int_tables<void>::charTable02 + (aabb - aa * 100u), 2ULL);
		aabb = low - aabb * 10000u;
		aa	 = (aabb * 5243u) >> 19ULL;
		std::memcpy(buf + 14ULL, int_tables<void>::charTable02 + aa, 2ULL);
		std::memcpy(buf + 16ULL, int_tables<void>::charTable02 + (aabb - aa * 100u), 2ULL);
		return buf + 18;
	}

	template<typename value_type> JSONIFIER_INLINE static char* toChars19(char* buf, const value_type value) noexcept {
		const uint64_t high	  = multiply_and_shift<100000000ULL, 6189700196426901375ULL, 89>::impl(value);
		const uint64_t low	  = value - high * 100000000ULL;
		const uint64_t high11 = multiply_and_shift<10000000ULL, 15474250491067253437ULL, 87>::impl(high);
		const uint64_t low11  = high - high11 * 10000000ULL;
		uint64_t aa			  = (high11 * 5243u) >> 19ULL;
		std::memcpy(buf, int_tables<void>::charTable02 + aa, 2ULL);
		std::memcpy(buf + 2ULL, int_tables<void>::charTable02 + (high11 - aa * 100u), 2ULL);
		uint64_t aabb = (low11 * 109951163ULL) >> 40ULL;
		aa			  = (aabb * 5243u) >> 19ULL;
		buf[4]		  = int_tables<void>::charTable00[aa];
		std::memcpy(buf + 5ULL, int_tables<void>::charTable02 + (aabb - aa * 100u), 2ULL);
		aabb = low11 - aabb * 10000u;
		aa	 = (aabb * 5243u) >> 19ULL;
		std::memcpy(buf + 7ULL, int_tables<void>::charTable02 + aa, 2ULL);
		std::memcpy(buf + 9ULL, int_tables<void>::charTable02 + (aabb - aa * 100u), 2ULL);
		aabb = (low * 109951163ULL) >> 40ULL;
		aa	 = (aabb * 5243u) >> 19ULL;
		std::memcpy(buf + 11ULL, int_tables<void>::charTable02 + aa, 2ULL);
		std::memcpy(buf + 13ULL, int_tables<void>::charTable02 + (aabb - aa * 100u), 2ULL);
		aabb = low - aabb * 10000u;
		aa	 = (aabb * 5243u) >> 19ULL;
		std::memcpy(buf + 15ULL, int_tables<void>::charTable02 + aa, 2ULL);
		std::memcpy(buf + 17ULL, int_tables<void>::charTable02 + (aabb - aa * 100u), 2ULL);
		return buf + 19;
	}

	template<typename value_type> JSONIFIER_INLINE static char* toChars20(char* buf, const value_type value) noexcept {
		const uint64_t high	  = multiply_and_shift<100000000ULL, 6189700196426901375ULL, 89>::impl(value);
		const uint64_t low	  = value - high * 100000000ULL;
		const uint64_t high12 = multiply_and_shift<100000000ULL, 6189700196426901375ULL, 89>::impl(high);
		const uint64_t low12  = high - high12 * 100000000ULL;
		uint64_t aa			  = (high12 * 5243u) >> 19ULL;
		std::memcpy(buf, int_tables<void>::charTable02 + aa, 2ULL);
		std::memcpy(buf + 2ULL, int_tables<void>::charTable02 + (high12 - aa * 100u), 2ULL);
		uint64_t aabb = (low12 * 109951163ULL) >> 40ULL;
		aa			  = (aabb * 5243u) >> 19ULL;
		std::memcpy(buf + 4ULL, int_tables<void>::charTable02 + aa, 2ULL);
		std::memcpy(buf + 6ULL, int_tables<void>::charTable02 + (aabb - aa * 100u), 2ULL);
		aabb = low12 - aabb * 10000u;
		aa	 = (aabb * 5243u) >> 19ULL;
		std::memcpy(buf + 8ULL, int_tables<void>::charTable02 + aa, 2ULL);
		std::memcpy(buf + 10ULL, int_tables<void>::charTable02 + (aabb - aa * 100u), 2ULL);
		aabb = (low * 109951163ULL) >> 40ULL;
		aa	 = (aabb * 5243u) >> 19ULL;
		std::memcpy(buf + 12ULL, int_tables<void>::charTable02 + aa, 2ULL);
		std::memcpy(buf + 14ULL, int_tables<void>::charTable02 + (aabb - aa * 100u), 2ULL);
		aabb = low - aabb * 10000u;
		aa	 = (aabb * 5243u) >> 19ULL;
		std::memcpy(buf + 16ULL, int_tables<void>::charTable02 + aa, 2ULL);
		std::memcpy(buf + 18ULL, int_tables<void>::charTable02 + (aabb - aa * 100u), 2ULL);
		return buf + 20;
	}

	template<concepts::uns64_t value_type> JSONIFIER_INLINE char* toChars(char* buf, const value_type value) noexcept {
		switch (fastDigitCount(value)) {
			case 20: {
				return toChars20(buf, value);
			}
			case 19: {
				return toChars19(buf, value);
			}
			case 18: {
				return toChars18(buf, value);
			}
			case 17: {
				return toChars17(buf, value);
			}
			case 16: {
				return toChars16(buf, value);
			}
			case 15: {
				return toChars15(buf, value);
			}
			case 14: {
				return toChars14(buf, value);
			}
			case 13: {
				return toChars13(buf, value);
			}
			case 12: {
				return toChars12(buf, value);
			}
			case 11: {
				return toChars11(buf, value);
			}
			case 10: {
				return toChars10(buf, value);
			}
			case 9: {
				return toChars9(buf, static_cast<uint32_t>(value));
			}
			case 8: {
				return toChars8(buf, static_cast<uint32_t>(value));
			}
			case 7: {
				return toChars7(buf, static_cast<uint32_t>(value));
			}
			case 6: {
				return toChars6(buf, static_cast<uint32_t>(value));
			}
			case 5: {
				return toChars5(buf, static_cast<uint32_t>(value));
			}
			case 4: {
				return toChars4(buf, static_cast<uint32_t>(value));
			}
			case 3: {
				return toChars3(buf, static_cast<uint32_t>(value));
			}
			case 2: {
				return toChars2(buf, static_cast<uint32_t>(value));
			}
			default: {
				return toChars1(buf, static_cast<uint32_t>(value));
			}
		}
	}

	template<concepts::sig64_t value_type> JSONIFIER_INLINE static char* toChars(char* buf, const value_type value) noexcept {
		*buf = '-';
		return toChars(buf + (value < 0), static_cast<uint64_t>(value ^ (value >> 63ULL)) - (value >> 63ULL));
	}

};// namespace internal
