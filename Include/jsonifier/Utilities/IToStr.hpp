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
		JSONIFIER_ALIGN(64ULL)
		inline static constexpr char charTable01[]{ 0x30, 0x30, 0x30, 0x31, 0x30, 0x32, 0x30, 0x33, 0x30, 0x34, 0x30, 0x35, 0x30, 0x36, 0x30, 0x37, 0x30, 0x38, 0x30, 0x39, 0x31,
			0x30, 0x31, 0x31, 0x31, 0x32, 0x31, 0x33, 0x31, 0x34, 0x31, 0x35, 0x31, 0x36, 0x31, 0x37, 0x31, 0x38, 0x31, 0x39, 0x32, 0x30, 0x32, 0x31, 0x32, 0x32, 0x32, 0x33, 0x32,
			0x34, 0x32, 0x35, 0x32, 0x36, 0x32, 0x37, 0x32, 0x38, 0x32, 0x39, 0x33, 0x30, 0x33, 0x31, 0x33, 0x32, 0x33, 0x33, 0x33, 0x34, 0x33, 0x35, 0x33, 0x36, 0x33, 0x37, 0x33,
			0x38, 0x33, 0x39, 0x34, 0x30, 0x34, 0x31, 0x34, 0x32, 0x34, 0x33, 0x34, 0x34, 0x34, 0x35, 0x34, 0x36, 0x34, 0x37, 0x34, 0x38, 0x34, 0x39, 0x35, 0x30, 0x35, 0x31, 0x35,
			0x32, 0x35, 0x33, 0x35, 0x34, 0x35, 0x35, 0x35, 0x36, 0x35, 0x37, 0x35, 0x38, 0x35, 0x39, 0x36, 0x30, 0x36, 0x31, 0x36, 0x32, 0x36, 0x33, 0x36, 0x34, 0x36, 0x35, 0x36,
			0x36, 0x36, 0x37, 0x36, 0x38, 0x36, 0x39, 0x37, 0x30, 0x37, 0x31, 0x37, 0x32, 0x37, 0x33, 0x37, 0x34, 0x37, 0x35, 0x37, 0x36, 0x37, 0x37, 0x37, 0x38, 0x37, 0x39, 0x38,
			0x30, 0x38, 0x31, 0x38, 0x32, 0x38, 0x33, 0x38, 0x34, 0x38, 0x35, 0x38, 0x36, 0x38, 0x37, 0x38, 0x38, 0x38, 0x39, 0x39, 0x30, 0x39, 0x31, 0x39, 0x32, 0x39, 0x33, 0x39,
			0x34, 0x39, 0x35, 0x39, 0x36, 0x39, 0x37, 0x39, 0x38, 0x39, 0x39 };
		JSONIFIER_ALIGN(64ULL)
		inline static constexpr uint16_t charTable02[]{ 0x3030, 0x3130, 0x3230, 0x3330, 0x3430, 0x3530, 0x3630, 0x3730, 0x3830, 0x3930, 0x3031, 0x3131, 0x3231, 0x3331, 0x3431,
			0x3531, 0x3631, 0x3731, 0x3831, 0x3931, 0x3032, 0x3132, 0x3232, 0x3332, 0x3432, 0x3532, 0x3632, 0x3732, 0x3832, 0x3932, 0x3033, 0x3133, 0x3233, 0x3333, 0x3433, 0x3533,
			0x3633, 0x3733, 0x3833, 0x3933, 0x3034, 0x3134, 0x3234, 0x3334, 0x3434, 0x3534, 0x3634, 0x3734, 0x3834, 0x3934, 0x3035, 0x3135, 0x3235, 0x3335, 0x3435, 0x3535, 0x3635,
			0x3735, 0x3835, 0x3935, 0x3036, 0x3136, 0x3236, 0x3336, 0x3436, 0x3536, 0x3636, 0x3736, 0x3836, 0x3936, 0x3037, 0x3137, 0x3237, 0x3337, 0x3437, 0x3537, 0x3637, 0x3737,
			0x3837, 0x3937, 0x3038, 0x3138, 0x3238, 0x3338, 0x3438, 0x3538, 0x3638, 0x3738, 0x3838, 0x3938, 0x3039, 0x3139, 0x3239, 0x3339, 0x3439, 0x3539, 0x3639, 0x3739, 0x3839,
			0x3939 };
		JSONIFIER_ALIGN(64ULL)
		inline static constexpr auto charTable04{ [] {
			std::array<uint32_t, 10000> return_values{};
			for (uint32_t i = 0; i < 10000; ++i) {
				return_values[i] = (0x30 + (i / 1000)) | ((0x30 + ((i / 100) % 10)) << 8) | ((0x30 + ((i / 10) % 10)) << 16) | ((0x30 + (i % 10)) << 24);
			}
			return return_values;
		}() };
	};

	template<uint64_t shift, std::integral value_type> JSONIFIER_INLINE constexpr value_type operator<<(const value_type arg, integral_constant<uint64_t, shift>) noexcept {
		constexpr uint64_t shift_amount{ shift };
		return arg << shift_amount;
	}

	template<uint64_t shift, std::integral value_type> JSONIFIER_INLINE constexpr value_type& operator<<=(value_type& arg, integral_constant<uint64_t, shift>) noexcept {
		return arg = arg << integral_constant<uint64_t, shift>{};
	}

	template<uint64_t shift, std::integral value_type> JSONIFIER_INLINE constexpr value_type operator>>(const value_type arg, integral_constant<uint64_t, shift>) noexcept {
		constexpr uint64_t shift_amount{ shift };
		return arg >> shift_amount;
	}

	template<uint64_t shift, std::integral value_type> JSONIFIER_INLINE constexpr value_type& operator>>=(value_type& arg, integral_constant<uint64_t, shift>) noexcept {
		return arg = arg >> integral_constant<uint64_t, shift>{};
	}

	template<uint64_t multiplier, uint64_t shift> struct multiply_and_shift {
		template<typename value_type> JSONIFIER_INLINE static uint64_t impl(value_type value) noexcept {
#if JSONIFIER_COMPILER_CLANG || JSONIFIER_COMPILER_GCC
			const __uint128_t product = static_cast<__uint128_t>(value) * multiplier;
			return static_cast<uint64_t>(product >> integral_constant<uint64_t, shift>{});
#elif JSONIFIER_COMPILER_MSVC
			uint64_t high_part;
			uint64_t low_part = _umul128(multiplier, value, &high_part);
			if constexpr (shift < 64ULL) {
				return static_cast<uint64_t>((low_part >> integral_constant<uint64_t, shift>{}) | (high_part << integral_constant<uint64_t, 64ULL - shift>{}));
			} else {
				return static_cast<uint64_t>(high_part >> integral_constant<uint64_t, shift - 64ULL>{});
			}
#else
			uint64_t high_part;
			const uint64_t low_part = mul128Generic(value, multiplier, &high_part);
			if constexpr (shift < 64ULL) {
				return static_cast<uint64_t>((low_part >> integral_constant<uint64_t, shift>{}) | (high_part << integral_constant<uint64_t, 64ULL - shift>{}));
			} else {
				return static_cast<uint64_t>(high_part >> integral_constant<uint64_t, shift - 64ULL>{});
			}
#endif
		}
	};

	template<uint64_t size> struct char_holder {
		char values[size]{};
	};

	template<uint64_t digit_length> struct to_chars_impl;

	template<> struct to_chars_impl<2ULL> {
		JSONIFIER_INLINE static char* impl(char* buf, const uint64_t value) noexcept {
			JSONIFIER_ALIGN(64ULL) static constexpr const char* char_table_ptr{ fiwb<void>::charTable01 };
			const uint64_t lz						= value < 10ULL;
			*reinterpret_cast<char_holder<2>*>(buf) = *reinterpret_cast<const char_holder<2>*>(char_table_ptr + (value * 2ULL + lz));
			buf -= lz;
			return buf + 2ULL;
		}
	};

	template<> struct to_chars_impl<4ULL> {
		JSONIFIER_INLINE static char* impl(char* buf, const uint64_t value) noexcept {
			JSONIFIER_ALIGN(64ULL) static constexpr const char* char_table_ptr{ fiwb<void>::charTable01 };
			JSONIFIER_ALIGN(64ULL) static constexpr const uint16_t* int16_table{ fiwb<void>::charTable02 };
			const uint64_t aa						= (value * 5243ULL) >> integral_constant<uint64_t, 19ULL>{};
			const uint64_t lz						= value < 1000ULL;
			*reinterpret_cast<char_holder<2>*>(buf) = *reinterpret_cast<const char_holder<2>*>(char_table_ptr + (aa * 2ULL + lz));
			buf -= lz;
			*reinterpret_cast<char_holder<2>*>(buf + 2ULL) = *reinterpret_cast<const char_holder<2>*>(int16_table + (value - aa * 100ULL));
			return buf + 4ULL;
		}
	};

	template<> struct to_chars_impl<6ULL> {
		JSONIFIER_INLINE static char* impl(char* buf, const uint64_t value) noexcept {
			JSONIFIER_ALIGN(64ULL) static constexpr const char* char_table_ptr{ fiwb<void>::charTable01 };
			JSONIFIER_ALIGN(64ULL) static constexpr const uint32_t* int32_table{ fiwb<void>::charTable04.data() };
			uint64_t aa								= (value * 429497ULL) >> integral_constant<uint64_t, 32ULL>{};
			const uint64_t lz						= value < 100000ULL;
			*reinterpret_cast<char_holder<2>*>(buf) = *reinterpret_cast<const char_holder<2>*>(char_table_ptr + (aa * 2ULL + lz));
			buf -= lz;
			const uint64_t remainder					   = value - aa * 10000ULL;
			*reinterpret_cast<char_holder<4>*>(buf + 2ULL) = *reinterpret_cast<const char_holder<4>*>(int32_table + remainder);
			return buf + 6ULL;
		}
	};

	template<> struct to_chars_impl<8ULL> {
		JSONIFIER_INLINE static char* impl(char* buf, const uint64_t value) noexcept {
			JSONIFIER_ALIGN(64ULL) static constexpr const char* char_table_ptr{ fiwb<void>::charTable01 };
			JSONIFIER_ALIGN(64ULL) static constexpr const uint16_t* int16_table{ fiwb<void>::charTable02 };
			JSONIFIER_ALIGN(64ULL) static constexpr const uint32_t* int32_table{ fiwb<void>::charTable04.data() };
			uint64_t aabb							= (value * 109951163ULL) >> integral_constant<uint64_t, 40ULL>{};
			uint64_t aa								= (aabb * 5243ULL) >> integral_constant<uint64_t, 19ULL>{};
			const uint64_t lz						= value < 10000000ULL;
			*reinterpret_cast<char_holder<2>*>(buf) = *reinterpret_cast<const char_holder<2>*>(char_table_ptr + (aa * 2ULL + lz));
			buf -= lz;
			*reinterpret_cast<char_holder<2>*>(buf + 2ULL) = *reinterpret_cast<const char_holder<2>*>(int16_table + (aabb - aa * 100ULL));
			const uint64_t ccdd							   = value - aabb * 10000ULL;
			*reinterpret_cast<char_holder<4>*>(buf + 4ULL) = *reinterpret_cast<const char_holder<4>*>(int32_table + ccdd);
			return buf + 8ULL;
		}
	};

	template<> struct to_chars_impl<10ULL> {
		JSONIFIER_INLINE static char* impl(char* buf, const uint64_t value) noexcept {
			JSONIFIER_ALIGN(64ULL) static constexpr const char* char_table_ptr{ fiwb<void>::charTable01 };
			JSONIFIER_ALIGN(64ULL) static constexpr const uint32_t* int32_table{ fiwb<void>::charTable04.data() };
			const uint64_t high						= multiply_and_shift<6189700196426901375ULL, 89ULL>::impl(value);
			const uint64_t low						= value - high * 100000000ULL;
			const uint64_t lz						= high < 10ULL;
			*reinterpret_cast<char_holder<2>*>(buf) = *reinterpret_cast<const char_holder<2>*>(char_table_ptr + (high * 2ULL + lz));
			buf -= lz;
			const uint64_t aabb							   = (low * 109951163ULL) >> integral_constant<uint64_t, 40ULL>{};
			const uint64_t ccdd							   = low - aabb * 10000ULL;
			*reinterpret_cast<char_holder<4>*>(buf + 2ULL) = *reinterpret_cast<const char_holder<4>*>(int32_table + aabb);
			*reinterpret_cast<char_holder<4>*>(buf + 6ULL) = *reinterpret_cast<const char_holder<4>*>(int32_table + ccdd);
			return buf + 10ULL;
		}
	};

	template<> struct to_chars_impl<12ULL> {
		JSONIFIER_INLINE static char* impl(char* buf, const uint64_t value) noexcept {
			JSONIFIER_ALIGN(64ULL) static constexpr const char* char_table_ptr{ fiwb<void>::charTable01 };
			JSONIFIER_ALIGN(64ULL) static constexpr const uint16_t* int16_table{ fiwb<void>::charTable02 };
			JSONIFIER_ALIGN(64ULL) static constexpr const uint32_t* int32_table{ fiwb<void>::charTable04.data() };
			const uint64_t high						= multiply_and_shift<6189700196426901375ULL, 89ULL>::impl(value);
			const uint64_t low						= value - high * 100000000ULL;
			uint64_t aa								= (high * 5243ULL) >> integral_constant<uint64_t, 19ULL>{};
			const uint64_t lz						= aa < 10ULL;
			*reinterpret_cast<char_holder<2>*>(buf) = *reinterpret_cast<const char_holder<2>*>(char_table_ptr + (aa * 2ULL + lz));
			buf -= lz;
			*reinterpret_cast<char_holder<2>*>(buf + 2ULL) = *reinterpret_cast<const char_holder<2>*>(int16_table + (high - aa * 100ULL));
			const uint64_t aabb							   = (low * 109951163ULL) >> integral_constant<uint64_t, 40ULL>{};
			const uint64_t ccdd							   = low - aabb * 10000ULL;
			*reinterpret_cast<char_holder<4>*>(buf + 4ULL) = *reinterpret_cast<const char_holder<4>*>(int32_table + aabb);
			*reinterpret_cast<char_holder<4>*>(buf + 8ULL) = *reinterpret_cast<const char_holder<4>*>(int32_table + ccdd);
			return buf + 12ULL;
		}
	};

	template<> struct to_chars_impl<14ULL> {
		JSONIFIER_INLINE static char* impl(char* buf, const uint64_t value) noexcept {
			JSONIFIER_ALIGN(64ULL) static constexpr const char* char_table_ptr{ fiwb<void>::charTable01 };
			JSONIFIER_ALIGN(64ULL) static constexpr const uint32_t* int32_table{ fiwb<void>::charTable04.data() };
			const uint64_t high						= multiply_and_shift<6189700196426901375ULL, 89ULL>::impl(value);
			const uint64_t low						= value - high * 100000000ULL;
			uint64_t aa								= (high * 429497ULL) >> integral_constant<uint64_t, 32ULL>{};
			const uint64_t lz						= aa < 10ULL;
			const uint64_t bbcc						= high - aa * 10000ULL;
			*reinterpret_cast<char_holder<2>*>(buf) = *reinterpret_cast<const char_holder<2>*>(char_table_ptr + (aa * 2ULL + lz));
			buf -= lz;
			*reinterpret_cast<char_holder<4>*>(buf + 2ULL)	= *reinterpret_cast<const char_holder<4>*>(int32_table + bbcc);
			const uint64_t aabb								= (low * 109951163ULL) >> integral_constant<uint64_t, 40ULL>{};
			const uint64_t ccdd								= low - aabb * 10000ULL;
			*reinterpret_cast<char_holder<4>*>(buf + 6ULL)	= *reinterpret_cast<const char_holder<4>*>(int32_table + aabb);
			*reinterpret_cast<char_holder<4>*>(buf + 10ULL) = *reinterpret_cast<const char_holder<4>*>(int32_table + ccdd);
			return buf + 14ULL;
		}
	};

	template<> struct to_chars_impl<16ULL> {
		JSONIFIER_INLINE static char* impl(char* buf, const uint64_t value) noexcept {
			JSONIFIER_ALIGN(64ULL) static constexpr const char* char_table_ptr{ fiwb<void>::charTable01 };
			JSONIFIER_ALIGN(64ULL) static constexpr const uint16_t* int16_table{ fiwb<void>::charTable02 };
			JSONIFIER_ALIGN(64ULL) static constexpr const uint32_t* int32_table{ fiwb<void>::charTable04.data() };
			const uint64_t high						= multiply_and_shift<6189700196426901375ULL, 89ULL>::impl(value);
			const uint64_t low						= value - high * 100000000ULL;
			uint64_t aabb							= (high * 109951163ULL) >> integral_constant<uint64_t, 40ULL>{};
			uint64_t ccdd							= high - aabb * 10000ULL;
			uint64_t aa								= (aabb * 5243ULL) >> integral_constant<uint64_t, 19ULL>{};
			const uint64_t lz						= aa < 10ULL;
			const uint64_t bb						= aabb - aa * 100ULL;
			*reinterpret_cast<char_holder<2>*>(buf) = *reinterpret_cast<const char_holder<2>*>(char_table_ptr + (aa * 2ULL + lz));
			buf -= lz;
			*reinterpret_cast<char_holder<2>*>(buf + 2ULL)	= *reinterpret_cast<const char_holder<2>*>(int16_table + bb);
			*reinterpret_cast<char_holder<4>*>(buf + 4ULL)	= *reinterpret_cast<const char_holder<4>*>(int32_table + ccdd);
			aabb											= (low * 109951163ULL) >> integral_constant<uint64_t, 40ULL>{};
			ccdd											= low - aabb * 10000ULL;
			*reinterpret_cast<char_holder<4>*>(buf + 8ULL)	= *reinterpret_cast<const char_holder<4>*>(int32_table + aabb);
			*reinterpret_cast<char_holder<4>*>(buf + 12ULL) = *reinterpret_cast<const char_holder<4>*>(int32_table + ccdd);
			return buf + 16ULL;
		}
	};

	template<> struct to_chars_impl<18ULL> {
		JSONIFIER_INLINE static char* impl(char* buf, const uint64_t value) noexcept {
			JSONIFIER_ALIGN(64ULL) static constexpr const char* char_table_ptr{ fiwb<void>::charTable01 };
			JSONIFIER_ALIGN(64ULL) static constexpr const uint32_t* int32_table{ fiwb<void>::charTable04.data() };
			const uint64_t high						= multiply_and_shift<6189700196426901375ULL, 89ULL>::impl(value);
			const uint64_t low						= value - high * 100000000ULL;
			const uint64_t high10					= multiply_and_shift<6189700196426901375ULL, 89ULL>::impl(high);
			const uint64_t low10					= high - high10 * 100000000ULL;
			const uint64_t lz						= high10 < 10ULL;
			*reinterpret_cast<char_holder<2>*>(buf) = *reinterpret_cast<const char_holder<2>*>(char_table_ptr + (high10 * 2ULL + lz));
			buf -= lz;
			const uint64_t aabb								= (low10 * 109951163ULL) >> integral_constant<uint64_t, 40ULL>{};
			const uint64_t ccdd								= low10 - aabb * 10000ULL;
			*reinterpret_cast<char_holder<4>*>(buf + 2ULL)	= *reinterpret_cast<const char_holder<4>*>(int32_table + aabb);
			*reinterpret_cast<char_holder<4>*>(buf + 6ULL)	= *reinterpret_cast<const char_holder<4>*>(int32_table + ccdd);
			const uint64_t eeff								= (low * 109951163ULL) >> integral_constant<uint64_t, 40ULL>{};
			const uint64_t gghh								= low - eeff * 10000ULL;
			*reinterpret_cast<char_holder<4>*>(buf + 10ULL) = *reinterpret_cast<const char_holder<4>*>(int32_table + eeff);
			*reinterpret_cast<char_holder<4>*>(buf + 14ULL) = *reinterpret_cast<const char_holder<4>*>(int32_table + gghh);
			return buf + 18ULL;
		}
	};

	template<> struct to_chars_impl<20ULL> {
		JSONIFIER_INLINE static char* impl(char* buf, const uint64_t value) noexcept {
			JSONIFIER_ALIGN(64ULL) static constexpr const char* char_table_ptr{ fiwb<void>::charTable01 };
			JSONIFIER_ALIGN(64ULL) static constexpr const uint16_t* int16_table{ fiwb<void>::charTable02 };
			JSONIFIER_ALIGN(64ULL) static constexpr const uint32_t* int32_table{ fiwb<void>::charTable04.data() };
			const uint64_t high						= multiply_and_shift<6189700196426901375ULL, 89ULL>::impl(value);
			const uint64_t low						= value - high * 100000000ULL;
			const uint64_t high12					= multiply_and_shift<6189700196426901375ULL, 89ULL>::impl(high);
			const uint64_t low12					= high - high12 * 100000000ULL;
			uint64_t aa								= (high12 * 5243ULL) >> integral_constant<uint64_t, 19ULL>{};
			const uint64_t lz						= aa < 10ULL;
			*reinterpret_cast<char_holder<2>*>(buf) = *reinterpret_cast<const char_holder<2>*>(char_table_ptr + (aa * 2ULL + lz));
			buf -= lz;
			*reinterpret_cast<char_holder<2>*>(buf + 2ULL)	= *reinterpret_cast<const char_holder<2>*>(int16_table + (high12 - aa * 100ULL));
			const uint64_t aabb								= (low12 * 109951163ULL) >> integral_constant<uint64_t, 40ULL>{};
			const uint64_t ccdd								= low12 - aabb * 10000ULL;
			*reinterpret_cast<char_holder<4>*>(buf + 4ULL)	= *reinterpret_cast<const char_holder<4>*>(int32_table + aabb);
			*reinterpret_cast<char_holder<4>*>(buf + 8ULL)	= *reinterpret_cast<const char_holder<4>*>(int32_table + ccdd);
			const uint64_t eeff								= (low * 109951163ULL) >> integral_constant<uint64_t, 40ULL>{};
			const uint64_t gghh								= low - eeff * 10000ULL;
			*reinterpret_cast<char_holder<4>*>(buf + 12ULL) = *reinterpret_cast<const char_holder<4>*>(int32_table + eeff);
			*reinterpret_cast<char_holder<4>*>(buf + 16ULL) = *reinterpret_cast<const char_holder<4>*>(int32_table + gghh);
			return buf + 20ULL;
		}
	};

	template<typename value_type> struct to_chars;

	template<std::integral value_type> struct to_chars<value_type> {
		template<concepts::uns64_t value_type_new> JSONIFIER_INLINE static char* impl(char* buf, const value_type_new value) noexcept {
			if (value < 10000ULL) {
				if (value < 100ULL) {
					return to_chars_impl<2ULL>::impl(buf, value);
				} else {
					return to_chars_impl<4ULL>::impl(buf, value);
				}
			} else if (value < 100000000ULL) {
				if (value < 1000000ULL) {
					return to_chars_impl<6ULL>::impl(buf, value);
				} else {
					return to_chars_impl<8ULL>::impl(buf, value);
				}
			} else if (value < 1000000000000ULL) {
				if (value < 10000000000ULL) {
					return to_chars_impl<10ULL>::impl(buf, value);
				} else {
					return to_chars_impl<12ULL>::impl(buf, value);
				}
			} else if (value < 10000000000000000ULL) {
				if (value < 100000000000000ULL) {
					return to_chars_impl<14ULL>::impl(buf, value);
				} else {
					return to_chars_impl<16ULL>::impl(buf, value);
				}
			} else if (value < 1000000000000000000ULL) {
				return to_chars_impl<18ULL>::impl(buf, value);
			} else {
				return to_chars_impl<20ULL>::impl(buf, value);
			}
		}

		template<concepts::sig64_t value_type_new> JSONIFIER_INLINE static char* impl(char* buf, const value_type_new value) noexcept {
			constexpr auto shift_amount = sizeof(value_type_new) * 8ULL - 1ULL;
			using unsigned_type			= std::make_unsigned_t<value_type_new>;
			*buf						= '-';
			return to_chars::impl(buf + (value < 0), static_cast<uint64_t>((static_cast<unsigned_type>(value) ^ (value >> shift_amount)) - (value >> shift_amount)));
		}
	};

};// namespace internal
