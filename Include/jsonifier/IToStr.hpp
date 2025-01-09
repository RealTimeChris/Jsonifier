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

#include <jsonifier/Allocator.hpp>

#include <concepts>
#include <cstdint>
#include <cstring>
#include <array>

namespace jsonifier::internal {

	alignas(2ull) JSONIFIER_INLINE_VARIABLE char charTable1[]{ 0x30u, 0x31u, 0x32u, 0x33u, 0x34u, 0x35u, 0x36u, 0x37u, 0x38u, 0x39u };

	alignas(2ull) JSONIFIER_INLINE_VARIABLE char charTable2[]{ 0x30u, 0x30u, 0x30u, 0x31u, 0x30u, 0x32u, 0x30u, 0x33u, 0x30u, 0x34u, 0x30u, 0x35u, 0x30u, 0x36u, 0x30u, 0x37u,
		0x30u, 0x38u, 0x30u, 0x39u, 0x31u, 0x30u, 0x31u, 0x31u, 0x31u, 0x32u, 0x31u, 0x33u, 0x31u, 0x34u, 0x31u, 0x35u, 0x31u, 0x36u, 0x31u, 0x37u, 0x31u, 0x38u, 0x31u, 0x39u,
		0x32u, 0x30u, 0x32u, 0x31u, 0x32u, 0x32u, 0x32u, 0x33u, 0x32u, 0x34u, 0x32u, 0x35u, 0x32u, 0x36u, 0x32u, 0x37u, 0x32u, 0x38u, 0x32u, 0x39u, 0x33u, 0x30u, 0x33u, 0x31u,
		0x33u, 0x32u, 0x33u, 0x33u, 0x33u, 0x34u, 0x33u, 0x35u, 0x33u, 0x36u, 0x33u, 0x37u, 0x33u, 0x38u, 0x33u, 0x39u, 0x34u, 0x30u, 0x34u, 0x31u, 0x34u, 0x32u, 0x34u, 0x33u,
		0x34u, 0x34u, 0x34u, 0x35u, 0x34u, 0x36u, 0x34u, 0x37u, 0x34u, 0x38u, 0x34u, 0x39u, 0x35u, 0x30u, 0x35u, 0x31u, 0x35u, 0x32u, 0x35u, 0x33u, 0x35u, 0x34u, 0x35u, 0x35u,
		0x35u, 0x36u, 0x35u, 0x37u, 0x35u, 0x38u, 0x35u, 0x39u, 0x36u, 0x30u, 0x36u, 0x31u, 0x36u, 0x32u, 0x36u, 0x33u, 0x36u, 0x34u, 0x36u, 0x35u, 0x36u, 0x36u, 0x36u, 0x37u,
		0x36u, 0x38u, 0x36u, 0x39u, 0x37u, 0x30u, 0x37u, 0x31u, 0x37u, 0x32u, 0x37u, 0x33u, 0x37u, 0x34u, 0x37u, 0x35u, 0x37u, 0x36u, 0x37u, 0x37u, 0x37u, 0x38u, 0x37u, 0x39u,
		0x38u, 0x30u, 0x38u, 0x31u, 0x38u, 0x32u, 0x38u, 0x33u, 0x38u, 0x34u, 0x38u, 0x35u, 0x38u, 0x36u, 0x38u, 0x37u, 0x38u, 0x38u, 0x38u, 0x39u, 0x39u, 0x30u, 0x39u, 0x31u,
		0x39u, 0x32u, 0x39u, 0x33u, 0x39u, 0x34u, 0x39u, 0x35u, 0x39u, 0x36u, 0x39u, 0x37u, 0x39u, 0x38u, 0x39u, 0x39u };

	JSONIFIER_INLINE_VARIABLE uint16_t charTable02[]{ 0x3030u, 0x3130u, 0x3230u, 0x3330u, 0x3430u, 0x3530u, 0x3630u, 0x3730u, 0x3830u, 0x3930u, 0x3031u, 0x3131u, 0x3231u, 0x3331u,
		0x3431u, 0x3531u, 0x3631u, 0x3731u, 0x3831u, 0x3931u, 0x3032u, 0x3132u, 0x3232u, 0x3332u, 0x3432u, 0x3532u, 0x3632u, 0x3732u, 0x3832u, 0x3932u, 0x3033u, 0x3133u, 0x3233u,
		0x3333u, 0x3433u, 0x3533u, 0x3633u, 0x3733u, 0x3833u, 0x3933u, 0x3034u, 0x3134u, 0x3234u, 0x3334u, 0x3434u, 0x3534u, 0x3634u, 0x3734u, 0x3834u, 0x3934u, 0x3035u, 0x3135u,
		0x3235u, 0x3335u, 0x3435u, 0x3535u, 0x3635u, 0x3735u, 0x3835u, 0x3935u, 0x3036u, 0x3136u, 0x3236u, 0x3336u, 0x3436u, 0x3536u, 0x3636u, 0x3736u, 0x3836u, 0x3936u, 0x3037u,
		0x3137u, 0x3237u, 0x3337u, 0x3437u, 0x3537u, 0x3637u, 0x3737u, 0x3837u, 0x3937u, 0x3038u, 0x3138u, 0x3238u, 0x3338u, 0x3438u, 0x3538u, 0x3638u, 0x3738u, 0x3838u, 0x3938u,
		0x3039u, 0x3139u, 0x3239u, 0x3339u, 0x3439u, 0x3539u, 0x3639u, 0x3739u, 0x3839u, 0x3939u };

	JSONIFIER_INLINE_VARIABLE uint8_t digitCounts[]{ 19, 19, 19, 19, 18, 18, 18, 17, 17, 17, 16, 16, 16, 16, 15, 15, 15, 14, 14, 14, 13, 13, 13, 13, 12, 12, 12, 11, 11, 11, 10, 10,
		10, 10, 9, 9, 9, 8, 8, 8, 7, 7, 7, 7, 6, 6, 6, 5, 5, 5, 4, 4, 4, 4, 3, 3, 3, 2, 2, 2, 1, 1, 1, 1, 1 };

	JSONIFIER_INLINE_VARIABLE uint64_t digitCountThresholds[]{ 0ull, 9ull, 99ull, 999ull, 9999ull, 99999ull, 999999ull, 9999999ull, 99999999ull, 999999999ull, 9999999999ull,
		99999999999ull, 999999999999ull, 9999999999999ull, 99999999999999ull, 999999999999999ull, 9999999999999999ull, 99999999999999999ull, 999999999999999999ull,
		9999999999999999999ull };

	enum class int_lengths {
		one		  = 1,
		two		  = 2,
		three	  = 3,
		four	  = 4,
		five	  = 5,
		six		  = 6,
		seven	  = 7,
		eight	  = 8,
		nine	  = 9,
		ten		  = 10,
		eleven	  = 11,
		twelve	  = 12,
		thirteen  = 13,
		fourteen  = 14,
		fifteen	  = 15,
		sixteen	  = 16,
		seventeen = 17,
		eighteen  = 18,
		nineteen  = 19,
		twenty	  = 20
	};

	JSONIFIER_INLINE int_lengths fastDigitCount(const uint64_t inputValue) {
		const uint64_t originalDigitCount{ digitCounts[jsonifier::simd::lzcnt(inputValue)] };
		return static_cast<int_lengths>(originalDigitCount + static_cast<uint64_t>(inputValue > digitCountThresholds[originalDigitCount]));
	}

	JSONIFIER_INLINE string_buffer_ptr length1(string_buffer_ptr buf, uint64_t value) {
		buf[0] = charTable1[value];
		return buf + 1ull;
	}

	JSONIFIER_INLINE string_buffer_ptr length2(string_buffer_ptr buf, uint64_t value) {
		std::memcpy(buf, charTable02 + value, 2ull);
		return buf + 2ull;
	}

	JSONIFIER_INLINE string_buffer_ptr length3(string_buffer_ptr buf, uint64_t value) {
		uint64_t aa = (value * 5243ull) >> 19ull;
		buf[0]		= charTable1[aa];
		std::memcpy(buf + 1ull, charTable02 + value - aa * 100ull, 2ull);
		return buf + 3ull;
	}

	JSONIFIER_INLINE string_buffer_ptr length4(string_buffer_ptr buf, uint64_t value) {
		uint64_t aa = (value * 5243ull) >> 19ull;
		std::memcpy(buf, charTable02 + aa, 2ull);
		std::memcpy(buf + 2ull, charTable02 + value - aa * 100ull, 2ull);
		return buf + 4ull;
	}

	JSONIFIER_INLINE string_buffer_ptr length5(string_buffer_ptr buf, uint64_t value) {
		uint64_t aa	  = (value * 429497ull) >> 32ull;
		uint64_t bbcc = value - aa * 10000ull;
		uint64_t bb	  = (bbcc * 5243ull) >> 19ull;
		buf[0]		  = charTable1[aa];
		std::memcpy(buf + 1ull, charTable02 + bb, 2ull);
		std::memcpy(buf + 3ull, charTable02 + bbcc - bb * 100ull, 2ull);
		return buf + 5ull;
	}

	JSONIFIER_INLINE string_buffer_ptr length6(string_buffer_ptr buf, uint64_t value) {
		uint64_t aa	  = (value * 429497ull) >> 32ull;
		uint64_t bbcc = value - aa * 10000ull;
		uint64_t bb	  = (bbcc * 5243ull) >> 19ull;
		std::memcpy(buf, charTable02 + aa, 2ull);
		std::memcpy(buf + 2ull, charTable02 + bb, 2ull);
		std::memcpy(buf + 4ull, charTable02 + bbcc - bb * 100ull, 2ull);
		return buf + 6ull;
	}

	JSONIFIER_INLINE string_buffer_ptr length7(string_buffer_ptr buf, uint64_t value) {
		uint64_t aabb = (value * 109951163ull) >> 40ull;
		uint64_t ccdd = value - aabb * 10000ull;
		uint64_t aa	  = (aabb * 5243ull) >> 19ull;
		uint64_t cc	  = (ccdd * 5243ull) >> 19ull;
		buf[0]		  = charTable1[aa];
		std::memcpy(buf + 1ull, charTable02 + aabb - aa * 100ull, 2ull);
		std::memcpy(buf + 3ull, charTable02 + cc, 2ull);
		std::memcpy(buf + 5ull, charTable02 + ccdd - cc * 100ull, 2ull);
		return buf + 7ull;
	}

	JSONIFIER_INLINE string_buffer_ptr length8(string_buffer_ptr buf, uint64_t value) {
		uint64_t aabb = (value * 109951163ull) >> 40ull;
		uint64_t ccdd = value - aabb * 10000ull;
		uint64_t aa	  = (aabb * 5243ull) >> 19ull;
		uint64_t cc	  = (ccdd * 5243ull) >> 19ull;
		std::memcpy(buf, charTable02 + aa, 2ull);
		std::memcpy(buf + 2ull, charTable02 + aabb - aa * 100ull, 2ull);
		std::memcpy(buf + 4ull, charTable02 + cc, 2ull);
		std::memcpy(buf + 6ull, charTable02 + ccdd - cc * 100ull, 2ull);
		return buf + 8ull;
	}

	JSONIFIER_INLINE string_buffer_ptr length9(string_buffer_ptr buf, uint64_t value) {
		uint64_t high = (value * 720575941ull) >> 56ull;
		uint64_t low  = value - high * 100000000ull;
		buf[0]		  = charTable1[high];
		uint64_t aabb = (low * 109951163ull) >> 40ull;
		uint64_t ccdd = low - aabb * 10000ull;
		uint64_t aa	  = (aabb * 5243ull) >> 19ull;
		uint64_t cc	  = (ccdd * 5243ull) >> 19ull;
		std::memcpy(buf + 1ull, charTable02 + aa, 2ull);
		std::memcpy(buf + 3ull, charTable02 + aabb - aa * 100ull, 2ull);
		std::memcpy(buf + 5ull, charTable02 + cc, 2ull);
		std::memcpy(buf + 7ull, charTable02 + ccdd - cc * 100ull, 2ull);
		return buf + 9ull;
	}

	template<jsonifier::concepts::uns64_t value_type> JSONIFIER_INLINE char* toChars(char* buf, value_type value) noexcept {
		const int_lengths index{ fastDigitCount(value) };
		switch (index) {
			case int_lengths::twenty: {
				static constexpr uint64_t multiplier01{ 109951163ull };
				static constexpr uint64_t multiplier02{ 5243ull };
				uint64_t tmp  = value / 100000000ull;
				uint64_t low  = value - tmp * 100000000ull;
				uint64_t high = tmp / 10000ull;
				uint64_t mid  = tmp - high * 10000ull;
				uint64_t aabb = (high * multiplier01) >> 40ull;
				uint64_t ccdd = high - aabb * 10000ull;
				uint64_t aa	  = (aabb * multiplier02) >> 19ull;
				uint64_t cc	  = (ccdd * multiplier02) >> 19ull;
				uint64_t bb	  = aabb - aa * 100ull;
				uint64_t dd	  = ccdd - cc * 100ull;
				std::memcpy(buf, charTable02 + aa, 2ull);
				std::memcpy(buf + 2ull, charTable02 + bb, 2ull);
				std::memcpy(buf + 4ull, charTable02 + cc, 2ull);
				std::memcpy(buf + 6ull, charTable02 + dd, 2ull);
				aa = (mid * multiplier02) >> 19ull;
				bb = mid - aa * 100ull;
				std::memcpy(buf + 8ull, charTable02 + aa, 2ull);
				std::memcpy(buf + 10ull, charTable02 + bb, 2ull);
				aabb = (low * multiplier01) >> 40ull;
				ccdd = low - aabb * 10000ull;
				aa	 = (aabb * multiplier02) >> 19ull;
				cc	 = (ccdd * multiplier02) >> 19ull;
				std::memcpy(buf + 12ull, charTable02 + aa, 2ull);
				std::memcpy(buf + 14ull, charTable02 + aabb - aa * 100ull, 2ull);
				std::memcpy(buf + 16ull, charTable02 + cc, 2ull);
				std::memcpy(buf + 18ull, charTable02 + ccdd - cc * 100ull, 2ull);
				return buf + 20ull;
			}
			case int_lengths::nineteen: {
				static constexpr uint64_t multiplier01{ 109951163ull };
				static constexpr uint64_t multiplier02{ 5243ull };
				uint64_t tmp  = value / 100000000ull;
				uint64_t low  = value - tmp * 100000000ull;
				uint64_t high = tmp / 10000ull;
				uint64_t mid  = tmp - high * 10000ull;
				uint64_t aabb = (high * multiplier01) >> 40ull;
				uint64_t ccdd = high - aabb * 10000ull;
				uint64_t aa	  = (aabb * multiplier02) >> 19ull;
				uint64_t cc	  = (ccdd * multiplier02) >> 19ull;
				uint64_t bb	  = aabb - aa * 100ull;
				uint64_t dd	  = ccdd - cc * 100ull;
				buf[0]		  = charTable1[aa];
				std::memcpy(buf + 1ull, charTable02 + bb, 2ull);
				std::memcpy(buf + 3ull, charTable02 + cc, 2ull);
				std::memcpy(buf + 5ull, charTable02 + dd, 2ull);
				aa = (mid * multiplier02) >> 19ull;
				bb = mid - aa * 100ull;
				std::memcpy(buf + 7ull, charTable02 + aa, 2ull);
				std::memcpy(buf + 9ull, charTable02 + bb, 2ull);
				aabb = (low * multiplier01) >> 40ull;
				ccdd = low - aabb * 10000ull;
				aa	 = (aabb * multiplier02) >> 19ull;
				cc	 = (ccdd * multiplier02) >> 19ull;
				std::memcpy(buf + 11ull, charTable02 + aa, 2ull);
				std::memcpy(buf + 13ull, charTable02 + aabb - aa * 100ull, 2ull);
				std::memcpy(buf + 15ull, charTable02 + cc, 2ull);
				std::memcpy(buf + 17ull, charTable02 + ccdd - cc * 100ull, 2ull);
				return buf + 19ull;
			}
			case int_lengths::eighteen: {
				static constexpr uint64_t multiplier01{ 429497ull };
				static constexpr uint64_t multiplier02{ 5243ull };
				static constexpr uint64_t multiplier03{ 109951163ull };
				uint64_t tmp  = value / 100000000ull;
				uint64_t low  = value - tmp * 100000000ull;
				uint64_t high = tmp / 10000ull;
				uint64_t mid  = tmp - high * 10000ull;
				uint64_t aa	  = (high * multiplier01) >> 32ull;
				uint64_t bbcc = high - aa * 10000ull;
				uint64_t bb	  = (bbcc * multiplier02) >> 19ull;
				uint64_t cc	  = bbcc - bb * 100ull;
				std::memcpy(buf, charTable02 + aa, 2ull);
				std::memcpy(buf + 2ull, charTable02 + bb, 2ull);
				std::memcpy(buf + 4ull, charTable02 + cc, 2ull);
				aa = (mid * 5243ull) >> 19ull;
				bb = mid - aa * 100ull;
				std::memcpy(buf + 6ull, charTable02 + aa, 2ull);
				std::memcpy(buf + 8ull, charTable02 + bb, 2ull);
				uint64_t aabb = (low * multiplier03) >> 40ull;
				uint64_t ccdd = low - aabb * 10000ull;
				aa			  = (aabb * multiplier02) >> 19ull;
				cc			  = (ccdd * multiplier02) >> 19ull;
				std::memcpy(buf + 10ull, charTable02 + aa, 2ull);
				std::memcpy(buf + 12ull, charTable02 + aabb - aa * 100ull, 2ull);
				std::memcpy(buf + 14ull, charTable02 + cc, 2ull);
				std::memcpy(buf + 16ull, charTable02 + ccdd - cc * 100ull, 2ull);
				return buf + 18ull;
			}
			case int_lengths::seventeen: {
				static constexpr uint64_t multiplier01{ 429497ull };
				static constexpr uint64_t multiplier02{ 5243ull };
				static constexpr uint64_t multiplier03{ 109951163ull };
				uint64_t tmp  = value / 100000000ull;
				uint64_t low  = value - tmp * 100000000ull;
				uint64_t high = tmp / 10000ull;
				uint64_t mid  = tmp - high * 10000ull;
				uint64_t aa	  = (high * multiplier01) >> 32ull;
				uint64_t bbcc = high - aa * 10000ull;
				uint64_t bb	  = (bbcc * multiplier02) >> 19ull;
				uint64_t cc	  = bbcc - bb * 100ull;
				buf[0]		  = charTable1[aa];
				std::memcpy(buf + 1ull, charTable02 + bb, 2ull);
				std::memcpy(buf + 3ull, charTable02 + cc, 2ull);
				aa = (mid * 5243ull) >> 19ull;
				bb = mid - aa * 100ull;
				std::memcpy(buf + 5ull, charTable02 + aa, 2ull);
				std::memcpy(buf + 7ull, charTable02 + bb, 2ull);
				uint64_t aabb = (low * multiplier03) >> 40ull;
				uint64_t ccdd = low - aabb * 10000ull;
				aa			  = (aabb * multiplier02) >> 19ull;
				cc			  = (ccdd * multiplier02) >> 19ull;
				std::memcpy(buf + 9ull, charTable02 + aa, 2ull);
				std::memcpy(buf + 11ull, charTable02 + aabb - aa * 100ull, 2ull);
				std::memcpy(buf + 13ull, charTable02 + cc, 2ull);
				std::memcpy(buf + 15ull, charTable02 + ccdd - cc * 100ull, 2ull);
				return buf + 17ull;
			}
			case int_lengths::sixteen: {
				static constexpr uint64_t multiplier01{ 109951163ull }; 
				static constexpr uint64_t multiplier02{ 5243ull };
				uint64_t high = value / 100000000ull;
				uint64_t low  = value - high * 100000000ull;
				uint64_t aabb = (high * multiplier01) >> 40ull;
				uint64_t ccdd = high - aabb * 10000ull;
				uint64_t aa	  = (aabb * multiplier02) >> 19ull;
				uint64_t cc	  = (ccdd * multiplier02) >> 19ull;
				uint64_t bb	  = aabb - aa * 100ull;
				uint64_t dd	  = ccdd - cc * 100ull;
				std::memcpy(buf, charTable02 + aa, 2ull);
				std::memcpy(buf + 2ull, charTable02 + bb, 2ull);
				std::memcpy(buf + 4ull, charTable02 + cc, 2ull);
				std::memcpy(buf + 6ull, charTable02 + dd, 2ull);
				aabb = (low * multiplier01) >> 40ull;
				ccdd = low - aabb * 10000ull;
				aa	 = (aabb * multiplier02) >> 19ull;
				cc	 = (ccdd * multiplier02) >> 19ull;
				std::memcpy(buf + 8ull, charTable02 + aa, 2ull);
				std::memcpy(buf + 10ull, charTable02 + aabb - aa * 100ull, 2ull);
				std::memcpy(buf + 12ull, charTable02 + cc, 2ull);
				std::memcpy(buf + 14ull, charTable02 + ccdd - cc * 100ull, 2ull);
				return buf + 16ull;
			}
			case int_lengths::fifteen: {
				static constexpr uint64_t multiplier01{ 109951163ull };
				static constexpr uint64_t multiplier02{ 5243ull };
				uint64_t high = value / 100000000ull;
				uint64_t low  = value - high * 100000000ull;
				uint64_t aabb = (high * multiplier01) >> 40ull;
				uint64_t ccdd = high - aabb * 10000ull;
				uint64_t aa	  = (aabb * multiplier02) >> 19ull;
				uint64_t cc	  = (ccdd * multiplier02) >> 19ull;
				uint64_t bb	  = aabb - aa * 100ull;
				uint64_t dd	  = ccdd - cc * 100ull;
				buf[0]		  = charTable1[aa];
				std::memcpy(buf + 1ull, charTable02 + bb, 2ull);
				std::memcpy(buf + 3ull, charTable02 + cc, 2ull);
				std::memcpy(buf + 5ull, charTable02 + dd, 2ull);
				aabb = (low * multiplier01) >> 40ull;
				ccdd = low - aabb * 10000ull;
				aa	 = (aabb * multiplier02) >> 19ull;
				cc	 = (ccdd * multiplier02) >> 19ull;
				std::memcpy(buf + 7ull, charTable02 + aa, 2ull);
				std::memcpy(buf + 9ull, charTable02 + aabb - aa * 100ull, 2ull);
				std::memcpy(buf + 11ull, charTable02 + cc, 2ull);
				std::memcpy(buf + 13ull, charTable02 + ccdd - cc * 100ull, 2ull);
				return buf + 15ull;
			}
			case int_lengths::fourteen: {
				static constexpr uint64_t multiplier01{ 429497ull };
				static constexpr uint64_t multiplier02{ 109951163ull };
				static constexpr uint64_t multiplier03{ 5243ull };
				uint64_t high = value / 100000000ull;
				uint64_t low  = value - high * 100000000ull;
				uint64_t aa	  = (high * multiplier01) >> 32ull;
				uint64_t bbcc = high - aa * 10000ull;
				uint64_t bb	  = (bbcc * multiplier03) >> 19ull;
				uint64_t cc	  = bbcc - bb * 100ull;
				std::memcpy(buf, charTable02 + aa, 2ull);
				std::memcpy(buf + 2ull, charTable02 + bb, 2ull);
				std::memcpy(buf + 4ull, charTable02 + cc, 2ull);
				uint64_t aabb = (low * multiplier02) >> 40ull;
				uint64_t ccdd = low - aabb * 10000ull;
				aa			  = (aabb * multiplier03) >> 19ull;
				cc			  = (ccdd * multiplier03) >> 19ull;
				std::memcpy(buf + 6ull, charTable02 + aa, 2ull);
				std::memcpy(buf + 8ull, charTable02 + aabb - aa * 100ull, 2ull);
				std::memcpy(buf + 10ull, charTable02 + cc, 2ull);
				std::memcpy(buf + 12ull, charTable02 + ccdd - cc * 100ull, 2ull);
				return buf + 14ull;
			}
			case int_lengths::thirteen: {
				static constexpr uint64_t multiplier01{ 429497ull };
				static constexpr uint64_t multiplier02{ 109951163ull };
				static constexpr uint64_t multiplier03{ 5243ull };
				uint64_t high = value / 100000000ull;
				uint64_t low  = value - high * 100000000ull;
				uint64_t aa	  = (high * multiplier01) >> 32ull;
				uint64_t bbcc = high - aa * 10000ull;
				uint64_t bb	  = (bbcc * multiplier03) >> 19ull;
				uint64_t cc	  = bbcc - bb * 100ull;
				buf[0]		  = charTable1[aa];
				std::memcpy(buf + 1ull, charTable02 + bb, 2ull);
				std::memcpy(buf + 3ull, charTable02 + cc, 2ull);
				uint64_t aabb = (low * multiplier02) >> 40ull;
				uint64_t ccdd = low - aabb * 10000ull;
				aa			  = (aabb * multiplier03) >> 19ull;
				cc			  = (ccdd * multiplier03) >> 19ull;
				std::memcpy(buf + 5ull, charTable02 + aa, 2ull);
				std::memcpy(buf + 7ull, charTable02 + aabb - aa * 100ull, 2ull);
				std::memcpy(buf + 9ull, charTable02 + cc, 2ull);
				std::memcpy(buf + 11ull, charTable02 + ccdd - cc * 100ull, 2ull);
				return buf + 13ull;
			}
			case int_lengths::twelve: {
				static constexpr uint64_t multiplier01{ 109951163ull };
				static constexpr uint64_t multiplier02{ 5243ull };
				uint64_t high = value / 100000000ull;
				uint64_t low  = value - high * 100000000ull;
				uint64_t aa	  = (high * multiplier02) >> 19ull;
				std::memcpy(buf, charTable02 + aa, 2ull);
				std::memcpy(buf + 2ull, charTable02 + high - aa * 100ull, 2ull);
				uint64_t aabb = (low * multiplier01) >> 40ull;
				uint64_t ccdd = low - aabb * 10000ull;
				aa			  = (aabb * multiplier02) >> 19ull;
				uint64_t cc	  = (ccdd * multiplier02) >> 19ull;
				std::memcpy(buf + 4, charTable02 + aa, 2ull);
				std::memcpy(buf + 6ull, charTable02 + aabb - aa * 100ull, 2ull);
				std::memcpy(buf + 8ull, charTable02 + cc, 2ull);
				std::memcpy(buf + 10ull, charTable02 + ccdd - cc * 100ull, 2ull);
				return buf + 12;
			}
			case int_lengths::eleven: {
				static constexpr uint64_t multiplier01{ 109951163ull };
				static constexpr uint64_t multiplier02{ 5243ull };
				uint64_t high = value / 10000000ull;
				uint64_t low  = value - high * 10000000ull;
				uint64_t aa	  = (high * multiplier02) >> 19ull;
				std::memcpy(buf, charTable02 + aa, 2ull);
				std::memcpy(buf + 2ull, charTable02 + high - aa * 100ull, 2ull);
				uint64_t aabb = (low * multiplier01) >> 40ull;
				uint64_t ccdd = low - aabb * 10000ull;
				aa			  = (aabb * multiplier02) >> 19ull;
				uint64_t cc	  = (ccdd * multiplier02) >> 19ull;
				buf[4]		  = charTable1[aa];
				std::memcpy(buf + 5ull, charTable02 + aabb - aa * 100ull, 2ull);
				std::memcpy(buf + 7ull, charTable02 + cc, 2ull);
				std::memcpy(buf + 9ull, charTable02 + ccdd - cc * 100ull, 2ull);
				return buf + 11;
			}
			case int_lengths::ten: {
				static constexpr uint64_t multiplier01{ 1801439851ull };
				static constexpr uint64_t multiplier02{ 109951163ull };
				static constexpr uint64_t multiplier03{ 5243ull };
				uint64_t high = (value * multiplier01) >> 54;
				uint64_t low  = value - high * 10000000ull;
				uint64_t aa	  = (high * multiplier03) >> 19ull;
				buf[0]		  = charTable1[aa];
				std::memcpy(buf + 1ull, charTable02 + high - aa * 100ull, 2ull);
				uint64_t aabb = (low * multiplier02) >> 40ull;
				uint64_t ccdd = low - aabb * 10000ull;
				aa			  = (aabb * multiplier03) >> 19ull;
				uint64_t cc	  = (ccdd * multiplier03) >> 19ull;
				buf[3]		  = charTable1[aa];
				std::memcpy(buf + 4ull, charTable02 + aabb - aa * 100ull, 2ull);
				std::memcpy(buf + 6ull, charTable02 + cc, 2ull);
				std::memcpy(buf + 8ull, charTable02 + ccdd - cc * 100ull, 2ull);
				return buf + 10;
			}
			case int_lengths::nine: {
				static constexpr uint64_t multiplier01{ 720575941ull };
				static constexpr uint64_t multiplier02{ 109951163ull };
				static constexpr uint64_t multiplier03{ 5243ull };
				uint64_t high = (value * multiplier01) >> 56ull;
				uint64_t low  = value - high * 100000000ull;
				buf[0]		  = charTable1[high];
				uint64_t aabb = (low * multiplier02) >> 40ull;
				uint64_t ccdd = low - aabb * 10000ull;
				uint64_t aa	  = (aabb * multiplier03) >> 19ull;
				uint64_t cc	  = (ccdd * multiplier03) >> 19ull;
				std::memcpy(buf + 1ull, charTable02 + aa, 2ull);
				std::memcpy(buf + 3ull, charTable02 + aabb - aa * 100ull, 2ull);
				std::memcpy(buf + 5ull, charTable02 + cc, 2ull);
				std::memcpy(buf + 7ull, charTable02 + ccdd - cc * 100ull, 2ull);
				return buf + 9ull;
			}
			case int_lengths::eight: {
				static constexpr uint64_t multiplier01{ 109951163ull };
				static constexpr uint64_t multiplier02{ 5243ull };
				uint64_t aabb = (value * multiplier01) >> 40ull;
				uint64_t ccdd = value - aabb * 10000ull;
				uint64_t aa	  = (aabb * multiplier02) >> 19ull;
				uint64_t cc	  = (ccdd * multiplier02) >> 19ull;
				std::memcpy(buf, charTable02 + aa, 2ull);
				std::memcpy(buf + 2ull, charTable02 + aabb - aa * 100ull, 2ull);
				std::memcpy(buf + 4ull, charTable02 + cc, 2ull);
				std::memcpy(buf + 6ull, charTable02 + ccdd - cc * 100ull, 2ull);
				return buf + 8ull;
			}
			case int_lengths::seven: {
				static constexpr uint64_t multiplier01{ 109951163ull };
				static constexpr uint64_t multiplier02{ 5243ull };
				uint64_t aabb = (value * multiplier01) >> 40ull;
				uint64_t ccdd = value - aabb * 10000ull;
				uint64_t aa	  = (aabb * multiplier02) >> 19ull;
				uint64_t cc	  = (ccdd * multiplier02) >> 19ull;
				buf[0]		  = charTable1[aa];
				std::memcpy(buf + 1ull, charTable02 + aabb - aa * 100ull, 2ull);
				std::memcpy(buf + 3ull, charTable02 + cc, 2ull);
				std::memcpy(buf + 5ull, charTable02 + ccdd - cc * 100ull, 2ull);
				return buf + 7ull;
			}
			case int_lengths::six: {
				static constexpr uint64_t multiplier01{ 429497ull };
				static constexpr uint64_t multiplier02{ 5243ull };
				uint64_t aa	  = (value * multiplier01) >> 32ull;
				uint64_t bbcc = value - aa * 10000ull;
				uint64_t bb	  = (bbcc * multiplier02) >> 19ull;
				std::memcpy(buf, charTable02 + aa, 2ull);
				std::memcpy(buf + 2ull, charTable02 + bb, 2ull);
				std::memcpy(buf + 4ull, charTable02 + bbcc - bb * 100ull, 2ull);
				return buf + 6ull;
			}
			case int_lengths::five: {
				static constexpr uint64_t multiplier01{ 429497ull };
				static constexpr uint64_t multiplier02{ 5243ull };
				uint64_t aa	  = (value * multiplier01) >> 32ull;
				uint64_t bbcc = value - aa * 10000ull;
				uint64_t bb	  = (bbcc * multiplier02) >> 19ull;
				buf[0]		  = charTable1[aa];
				std::memcpy(buf + 1ull, charTable02 + bb, 2ull);
				std::memcpy(buf + 3ull, charTable02 + bbcc - bb * 100ull, 2ull);
				return buf + 5ull;
			}
			case int_lengths::four: {
				static constexpr uint64_t multiplier{ 5243ull };
				uint64_t aa = (value * multiplier) >> 19ull;
				std::memcpy(buf, charTable02 + aa, 2ull);
				std::memcpy(buf + 2ull, charTable02 + value - aa * 100ull, 2ull);
				return buf + 4ull;
			}
			case int_lengths::three: {
				static constexpr uint64_t multiplier{ 5243ull };
				uint64_t aa = (value * multiplier) >> 19ull;
				buf[0]		= charTable1[aa];
				std::memcpy(buf + 1ull, charTable02 + value - aa * 100ull, 2ull);
				return buf + 3ull;
			}
			case int_lengths::two: {
				std::memcpy(buf, charTable02 + value, 2ull);
				return buf + 2ull;
			}
			default: {
				buf[0] = charTable1[value];
				return buf + 1ull;
			}
		}
	}

	template<jsonifier::concepts::sig64_t value_type> JSONIFIER_INLINE char* toChars(char* buf, value_type x) noexcept {
		*buf = '-';
		return toChars(buf + (x < 0), uint64_t(x ^ (x >> 63ull)) - (x >> 63ull));
	}

}// namespace jsonifier::internal