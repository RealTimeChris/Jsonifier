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

#include <jsonifier/Allocator.hpp>

#include <concepts>
#include <cstdint>
#include <cstring>
#include <array>

namespace jsonifier_internal {

	alignas(2) constexpr char charTable[]{ 0x30u, 0x30u, 0x30u, 0x31u, 0x30u, 0x32u, 0x30u, 0x33u, 0x30u, 0x34u, 0x30u, 0x35u, 0x30u, 0x36u, 0x30u, 0x37u, 0x30u, 0x38u, 0x30u,
		0x39u, 0x31u, 0x30u, 0x31u, 0x31u, 0x31u, 0x32u, 0x31u, 0x33u, 0x31u, 0x34u, 0x31u, 0x35u, 0x31u, 0x36u, 0x31u, 0x37u, 0x31u, 0x38u, 0x31u, 0x39u, 0x32u, 0x30u, 0x32u,
		0x31u, 0x32u, 0x32u, 0x32u, 0x33u, 0x32u, 0x34u, 0x32u, 0x35u, 0x32u, 0x36u, 0x32u, 0x37u, 0x32u, 0x38u, 0x32u, 0x39u, 0x33u, 0x30u, 0x33u, 0x31u, 0x33u, 0x32u, 0x33u,
		0x33u, 0x33u, 0x34u, 0x33u, 0x35u, 0x33u, 0x36u, 0x33u, 0x37u, 0x33u, 0x38u, 0x33u, 0x39u, 0x34u, 0x30u, 0x34u, 0x31u, 0x34u, 0x32u, 0x34u, 0x33u, 0x34u, 0x34u, 0x34u,
		0x35u, 0x34u, 0x36u, 0x34u, 0x37u, 0x34u, 0x38u, 0x34u, 0x39u, 0x35u, 0x30u, 0x35u, 0x31u, 0x35u, 0x32u, 0x35u, 0x33u, 0x35u, 0x34u, 0x35u, 0x35u, 0x35u, 0x36u, 0x35u,
		0x37u, 0x35u, 0x38u, 0x35u, 0x39u, 0x36u, 0x30u, 0x36u, 0x31u, 0x36u, 0x32u, 0x36u, 0x33u, 0x36u, 0x34u, 0x36u, 0x35u, 0x36u, 0x36u, 0x36u, 0x37u, 0x36u, 0x38u, 0x36u,
		0x39u, 0x37u, 0x30u, 0x37u, 0x31u, 0x37u, 0x32u, 0x37u, 0x33u, 0x37u, 0x34u, 0x37u, 0x35u, 0x37u, 0x36u, 0x37u, 0x37u, 0x37u, 0x38u, 0x37u, 0x39u, 0x38u, 0x30u, 0x38u,
		0x31u, 0x38u, 0x32u, 0x38u, 0x33u, 0x38u, 0x34u, 0x38u, 0x35u, 0x38u, 0x36u, 0x38u, 0x37u, 0x38u, 0x38u, 0x38u, 0x39u, 0x39u, 0x30u, 0x39u, 0x31u, 0x39u, 0x32u, 0x39u,
		0x33u, 0x39u, 0x34u, 0x39u, 0x35u, 0x39u, 0x36u, 0x39u, 0x37u, 0x39u, 0x38u, 0x39u, 0x39u };

	JSONIFIER_INLINE_VARIABLE uint64_t digitCountTable[]{ 4294967296, 8589934582, 8589934582, 8589934582, 12884901788, 12884901788, 12884901788, 17179868184, 17179868184,
		17179868184, 21474826480, 21474826480, 21474826480, 21474826480, 25769703776, 25769703776, 25769703776, 30063771072, 30063771072, 30063771072, 34349738368, 34349738368,
		34349738368, 34349738368, 38554705664, 38554705664, 38554705664, 41949672960, 41949672960, 41949672960, 42949672960, 42949672960 };

	// https://lemire.me/blog/2021/06/03/computing-the-number-of-digits-of-an-integer-even-faster/
	JSONIFIER_INLINE uint64_t fastDigitCount(const uint32_t x) noexcept {
		return (x + digitCountTable[31 - simd_internal::lzcnt(x | 1)]) >> 32;
	}

	constexpr std::array<uint32_t, 10000> generateCharTable4() {
		std::array<uint32_t, 10000> table{};

		for (uint32_t i = 0; i < 10000; ++i) {
			uint32_t thousands = i / 1000 + '0';
			uint32_t hundreds  = (i / 100) % 10 + '0';
			uint32_t tens	   = (i / 10) % 10 + '0';
			uint32_t ones	   = i % 10 + '0';
			table[i]		   = (ones << 24) | (tens << 16) | (hundreds << 8) | thousands;
		}

		return table;
	}

	constexpr std::array<uint32_t, 1000> generateCharTable3() {
		std::array<uint32_t, 1000> table{};
		for (uint32_t i = 0; i < 1000; ++i) {
			uint32_t hundreds = (i / 100) % 10 + '0';
			uint32_t tens	  = (i / 10) % 10 + '0';
			uint32_t ones	  = i % 10 + '0';
			table[i]		  = (ones << 16) | (tens << 8) | (hundreds);
		}

		return table;
	}

	constexpr std::array<uint16_t, 100> generateCharTable2() {
		std::array<uint16_t, 100> table{};
		for (uint32_t i = 0; i < 100; ++i) {
			uint32_t tens = (i / 10) % 10 + '0';
			uint32_t ones = i % 10 + '0';

			table[i] = static_cast<uint16_t>((ones << 8) | (tens));
		}

		return table;
	}

	constexpr std::array<char, 10> generateCharTable1() {
		std::array<char, 10> table{};
		for (uint32_t i = 0; i < 10; ++i) {
			uint32_t ones = i + '0';

			table[i] = static_cast<char>(ones);
		}

		return table;
	}

	constexpr auto charTable1 = generateCharTable1();
	constexpr auto charTable2 = generateCharTable2();
	constexpr auto charTable3 = generateCharTable3();
	constexpr auto charTable4 = generateCharTable4();

	JSONIFIER_INLINE char* length1(char* buf, uint64_t value) noexcept {
		const uint32_t high = static_cast<uint32_t>(value * 5243) >> 19;
		const uint32_t low	= static_cast<uint32_t>(value) - high * 100;
		std::memcpy(buf, charTable1.data() + low, 1);
		return buf + 1;
	}

	JSONIFIER_INLINE char* length2(char* buf, uint64_t value) noexcept {
		const uint32_t high = static_cast<uint32_t>(value * 5243) >> 19;
		const uint32_t low	= static_cast<uint32_t>(value) - high * 100;
		std::memcpy(buf, charTable2.data() + low, 2);
		return buf + 2;
	}

	JSONIFIER_INLINE char* length3(char* buf, uint64_t value) noexcept {
		const uint32_t high = static_cast<uint32_t>((value * 109951163) >> 40);
		const uint32_t low	= static_cast<uint32_t>(value) - high * 10000;
		std::memcpy(buf, charTable3.data() + low, 3);
		return buf + 3;
	}

	JSONIFIER_INLINE char* length4(char* buf, uint64_t value) noexcept {
		const uint32_t high = static_cast<uint32_t>((value * 109951163) >> 40);
		const uint32_t low	= static_cast<uint32_t>(value) - high * 10000;
		std::memcpy(buf, charTable4.data() + low, 4);
		return buf + 4;
	}

	JSONIFIER_INLINE char* length5(char* buf, uint64_t value) noexcept {
		const uint32_t high = static_cast<uint32_t>((value * 109951163) >> 40);
		const uint32_t low	= static_cast<uint32_t>(value) - high * 10000;
		std::memcpy(buf, charTable1.data() + high, 1);
		std::memcpy(buf + 1, charTable4.data() + low, 4);
		return buf + 5;
	}

	JSONIFIER_INLINE char* length6(char* buf, uint64_t value) noexcept {
		const uint32_t high = static_cast<uint32_t>((value * 109951163) >> 40);
		const uint32_t low	= static_cast<uint32_t>(value) - high * 10000;
		std::memcpy(buf, charTable2.data() + high, 2);
		std::memcpy(buf + 2, charTable4.data() + low, 4);
		return buf + 6;
	}

	JSONIFIER_INLINE char* length7(char* buf, uint64_t value) noexcept {
		const uint32_t high = static_cast<uint32_t>((value * 109951163) >> 40);
		const uint32_t low	= static_cast<uint32_t>(value) - high * 10000;
		std::memcpy(buf, charTable3.data() + high, 3);
		std::memcpy(buf + 3, charTable4.data() + low, 4);
		return buf + 7;
	}

	JSONIFIER_INLINE char* length8(char* buf, uint64_t value) noexcept {
		const uint32_t high = static_cast<uint32_t>((value * 109951163) >> 40);
		const uint32_t low	= static_cast<uint32_t>(value) - high * 10000;
		std::memcpy(buf, charTable4.data() + high, 4);
		std::memcpy(buf + 4, charTable4.data() + low, 4);
		return buf + 8;
	}

	JSONIFIER_INLINE char* length9(char* buf, uint64_t value) noexcept {
		const uint64_t high = value / 100000000;
		const uint64_t low	= value - high * 100000000;
		buf					= length1(buf, high);
		buf					= length8(buf, low);
		return buf;
	}

	JSONIFIER_INLINE char* length10(char* buf, uint64_t value) noexcept {
		const uint64_t high = value / 100000000;
		const uint64_t low	= value - high * 100000000;
		buf					= length2(buf, high);
		buf					= length8(buf, low);
		return buf;
	}

	template<typename value_type> JSONIFIER_INLINE static char* toCharsByDigitCount(char* buf, value_type value) noexcept {
		uint64_t numDigits{ fastDigitCount(static_cast<uint32_t>(value)) };
		switch (numDigits) {
			case 1: {
				return length1(buf, value);
			}
			case 2: {
				return length2(buf, value);
			}
			case 3: {
				return length3(buf, value);
			}
			case 4: {
				return length4(buf, value);
			}
			case 5: {
				return length5(buf, value);
			}
			case 6: {
				return length6(buf, value);
			}
			case 7: {
				return length7(buf, value);
			}
			case 8: {
				return length8(buf, value);
			}
			case 9: {
				return length9(buf, value);
			}
			case 10: {
				return length10(buf, value);
			}
			default: {
				return buf;
			}
		}
	}

	JSONIFIER_INLINE static char* lt10000000000000000(char* buf, uint64_t value) noexcept {
		const uint64_t high = value / 100000000;
		const uint64_t low	= value - high * 100000000;
		buf					= toCharsByDigitCount(buf, high);
		buf					= length8(buf, low);
		return buf;
	}

	JSONIFIER_INLINE static char* gte10000000000000000(char* buf, uint64_t value) noexcept {
		const uint64_t tmp	= value / 100000000;
		const uint64_t high = tmp / 10000;
		const uint64_t mid	= tmp - high * 10000;
		const uint64_t low	= value - tmp * 100000000;
		buf					= toCharsByDigitCount(buf, high);
		buf					= length4(buf, mid);
		buf					= length8(buf, low);
		return buf;
	}

	template<jsonifier::concepts::uns64_t value_type> JSONIFIER_INLINE static char* toChars(char* buf, value_type value) noexcept {
		if (value < std::numeric_limits<uint32_t>::max()) {
			return toCharsByDigitCount(buf, value);
		} else {
			static constexpr value_type tenQuadrillion{ 10000000000000000 };
			static constexpr value_type tenMillion{ 100000000 };
			if (value >= tenQuadrillion) {
				return gte10000000000000000(buf, value);
			} else if (value >= tenMillion) {
				return lt10000000000000000(buf, value);
			}
			return buf;
		}
	}

	template<jsonifier::concepts::sig64_t value_type> JSONIFIER_INLINE static char* toChars(char* buf, value_type value) noexcept {
		*buf = '-';
		return toChars<uint64_t>(buf + (value < 0), static_cast<uint64_t>(value ^ (value >> 63)) - (value >> 63));
	}

}// namespace jsonifier_internal