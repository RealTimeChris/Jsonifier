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

	struct alignas(2ull) char_pair {
		char values[2]{};
	};

	alignas(2ull) JSONIFIER_INLINE_VARIABLE char charTable1[]{ 0x30u, 0x31u, 0x32u, 0x33u, 0x34u, 0x35u, 0x36u, 0x37u, 0x38u, 0x39u };

	alignas(2ull) JSONIFIER_INLINE_VARIABLE char_pair charTable2Real[100]{ 0x30u, 0x30u, 0x30u, 0x31u, 0x30u, 0x32u, 0x30u, 0x33u, 0x30u, 0x34u, 0x30u, 0x35u, 0x30u, 0x36u, 0x30u,
		0x37u, 0x30u, 0x38u, 0x30u, 0x39u, 0x31u, 0x30u, 0x31u, 0x31u, 0x31u, 0x32u, 0x31u, 0x33u, 0x31u, 0x34u, 0x31u, 0x35u, 0x31u, 0x36u, 0x31u, 0x37u, 0x31u, 0x38u, 0x31u,
		0x39u, 0x32u, 0x30u, 0x32u, 0x31u, 0x32u, 0x32u, 0x32u, 0x33u, 0x32u, 0x34u, 0x32u, 0x35u, 0x32u, 0x36u, 0x32u, 0x37u, 0x32u, 0x38u, 0x32u, 0x39u, 0x33u, 0x30u, 0x33u,
		0x31u, 0x33u, 0x32u, 0x33u, 0x33u, 0x33u, 0x34u, 0x33u, 0x35u, 0x33u, 0x36u, 0x33u, 0x37u, 0x33u, 0x38u, 0x33u, 0x39u, 0x34u, 0x30u, 0x34u, 0x31u, 0x34u, 0x32u, 0x34u,
		0x33u, 0x34u, 0x34u, 0x34u, 0x35u, 0x34u, 0x36u, 0x34u, 0x37u, 0x34u, 0x38u, 0x34u, 0x39u, 0x35u, 0x30u, 0x35u, 0x31u, 0x35u, 0x32u, 0x35u, 0x33u, 0x35u, 0x34u, 0x35u,
		0x35u, 0x35u, 0x36u, 0x35u, 0x37u, 0x35u, 0x38u, 0x35u, 0x39u, 0x36u, 0x30u, 0x36u, 0x31u, 0x36u, 0x32u, 0x36u, 0x33u, 0x36u, 0x34u, 0x36u, 0x35u, 0x36u, 0x36u, 0x36u,
		0x37u, 0x36u, 0x38u, 0x36u, 0x39u, 0x37u, 0x30u, 0x37u, 0x31u, 0x37u, 0x32u, 0x37u, 0x33u, 0x37u, 0x34u, 0x37u, 0x35u, 0x37u, 0x36u, 0x37u, 0x37u, 0x37u, 0x38u, 0x37u,
		0x39u, 0x38u, 0x30u, 0x38u, 0x31u, 0x38u, 0x32u, 0x38u, 0x33u, 0x38u, 0x34u, 0x38u, 0x35u, 0x38u, 0x36u, 0x38u, 0x37u, 0x38u, 0x38u, 0x38u, 0x39u, 0x39u, 0x30u, 0x39u,
		0x31u, 0x39u, 0x32u, 0x39u, 0x33u, 0x39u, 0x34u, 0x39u, 0x35u, 0x39u, 0x36u, 0x39u, 0x37u, 0x39u, 0x38u, 0x39u, 0x39u };

	alignas(2ull) JSONIFIER_INLINE_VARIABLE char charTable2[]{ 0x30u, 0x30u, 0x30u, 0x31u, 0x30u, 0x32u, 0x30u, 0x33u, 0x30u, 0x34u, 0x30u, 0x35u, 0x30u, 0x36u, 0x30u, 0x37u,
		0x30u, 0x38u, 0x30u, 0x39u, 0x31u, 0x30u, 0x31u, 0x31u, 0x31u, 0x32u, 0x31u, 0x33u, 0x31u, 0x34u, 0x31u, 0x35u, 0x31u, 0x36u, 0x31u, 0x37u, 0x31u, 0x38u, 0x31u, 0x39u,
		0x32u, 0x30u, 0x32u, 0x31u, 0x32u, 0x32u, 0x32u, 0x33u, 0x32u, 0x34u, 0x32u, 0x35u, 0x32u, 0x36u, 0x32u, 0x37u, 0x32u, 0x38u, 0x32u, 0x39u, 0x33u, 0x30u, 0x33u, 0x31u,
		0x33u, 0x32u, 0x33u, 0x33u, 0x33u, 0x34u, 0x33u, 0x35u, 0x33u, 0x36u, 0x33u, 0x37u, 0x33u, 0x38u, 0x33u, 0x39u, 0x34u, 0x30u, 0x34u, 0x31u, 0x34u, 0x32u, 0x34u, 0x33u,
		0x34u, 0x34u, 0x34u, 0x35u, 0x34u, 0x36u, 0x34u, 0x37u, 0x34u, 0x38u, 0x34u, 0x39u, 0x35u, 0x30u, 0x35u, 0x31u, 0x35u, 0x32u, 0x35u, 0x33u, 0x35u, 0x34u, 0x35u, 0x35u,
		0x35u, 0x36u, 0x35u, 0x37u, 0x35u, 0x38u, 0x35u, 0x39u, 0x36u, 0x30u, 0x36u, 0x31u, 0x36u, 0x32u, 0x36u, 0x33u, 0x36u, 0x34u, 0x36u, 0x35u, 0x36u, 0x36u, 0x36u, 0x37u,
		0x36u, 0x38u, 0x36u, 0x39u, 0x37u, 0x30u, 0x37u, 0x31u, 0x37u, 0x32u, 0x37u, 0x33u, 0x37u, 0x34u, 0x37u, 0x35u, 0x37u, 0x36u, 0x37u, 0x37u, 0x37u, 0x38u, 0x37u, 0x39u,
		0x38u, 0x30u, 0x38u, 0x31u, 0x38u, 0x32u, 0x38u, 0x33u, 0x38u, 0x34u, 0x38u, 0x35u, 0x38u, 0x36u, 0x38u, 0x37u, 0x38u, 0x38u, 0x38u, 0x39u, 0x39u, 0x30u, 0x39u, 0x31u,
		0x39u, 0x32u, 0x39u, 0x33u, 0x39u, 0x34u, 0x39u, 0x35u, 0x39u, 0x36u, 0x39u, 0x37u, 0x39u, 0x38u, 0x39u, 0x39u };

	JSONIFIER_INLINE_VARIABLE array<uint8_t, 65> digitCounts{ { 19, 19, 19, 19, 18, 18, 18, 17, 17, 17, 16, 16, 16, 16, 15, 15, 15, 14, 14, 14, 13, 13, 13, 13, 12, 12, 12, 11, 11,
		11, 10, 10, 10, 10, 9, 9, 9, 8, 8, 8, 7, 7, 7, 7, 6, 6, 6, 5, 5, 5, 4, 4, 4, 4, 3, 3, 3, 2, 2, 2, 1, 1, 1, 1, 1 } };

	JSONIFIER_INLINE_VARIABLE array<uint64_t, 20> digitCountThresholds{ { 0ull, 9ull, 99ull, 999ull, 9999ull, 99999ull, 999999ull, 9999999ull, 99999999ull, 999999999ull,
		9999999999ull, 99999999999ull, 999999999999ull, 9999999999999ull, 99999999999999ull, 999999999999999ull, 9999999999999999ull, 99999999999999999ull, 999999999999999999ull,
		9999999999999999999ull } };

	JSONIFIER_INLINE uint64_t fastDigitCount(const uint64_t inputValue) {
		const uint64_t originalDigitCount{ static_cast<uint64_t>(digitCounts[simd_internal::lzcnt(inputValue)]) };
		return originalDigitCount + static_cast<uint64_t>(inputValue > digitCountThresholds[originalDigitCount]);
	}

	JSONIFIER_INLINE static string_buffer_ptr length1(string_buffer_ptr buf, const uint64_t value) noexcept {
		std::memcpy(buf, charTable1 + value, 1ull);
		return buf + 1ull;
	}

	JSONIFIER_INLINE static string_buffer_ptr length2(string_buffer_ptr buf, const uint64_t value) noexcept {
		std::memcpy(buf, charTable2Real + value, 2ull);
		return buf + 2ull;
	}

	JSONIFIER_INLINE static string_buffer_ptr length3(string_buffer_ptr buf, const uint64_t value) noexcept {
		static constexpr uint64_t multiplier{ uint64_t(10 * 0x1p24 / 1e3 + 1) };
		static constexpr uint64_t bitMask{ uint64_t(0x1p24) - 1 };
		const uint64_t f0 = (multiplier * value);
		const uint64_t f2 = ((f0 & bitMask) * 100);
		std::memcpy(buf, charTable1 + (f0 >> 24), 2ull);
		std::memcpy(buf + 1, charTable2Real + (f2 >> 24), 2ull);
		return buf + 3ull;
	}

	JSONIFIER_INLINE static string_buffer_ptr length4(string_buffer_ptr buf, const uint64_t value) noexcept {
		static constexpr uint64_t multiplier{ uint64_t(10 * 0x1p24 / 1e3 + 1) };
		static constexpr uint64_t bitMask{ uint64_t(0x1p24) - 1 };
		const uint64_t f0 = (multiplier * value);
		const uint64_t f2 = ((f0 & bitMask) * 100);
		std::memcpy(buf, charTable2Real + (f0 >> 24), 2ull);
		std::memcpy(buf + 2, charTable2Real + (f2 >> 24), 2ull);
		return buf + 4ull;
	}

	JSONIFIER_INLINE static string_buffer_ptr length5(string_buffer_ptr buf, const uint64_t value) noexcept {
		static constexpr uint64_t multiplier{ uint64_t(10 * 0x1p32 / 1e5 + 1) };
		static constexpr uint64_t bitMask{ uint64_t(0x1p32) - 1 };
		const uint64_t f0 = multiplier * value;
		const uint64_t f2 = (f0 & bitMask) * 100;
		const uint64_t f4 = (f2 & bitMask) * 100;
		std::memcpy(buf, charTable1 + (f0 >> 32), 1ull);
		std::memcpy(buf + 1, charTable2Real + (f2 >> 32), 2ull);
		std::memcpy(buf + 3, charTable2Real + (f4 >> 32), 2ull);
		return buf + 5ull;
	}

	JSONIFIER_INLINE static string_buffer_ptr length6(string_buffer_ptr buf, const uint64_t value) noexcept {
		static constexpr uint64_t multiplier{ uint64_t(10 * 0x1p32 / 1e5 + 1) };
		static constexpr uint64_t bitMask{ uint64_t(0x1p32) - 1 };
		const uint64_t f0 = multiplier * value;
		const uint64_t f2 = (f0 & bitMask) * 100;
		const uint64_t f4 = (f2 & bitMask) * 100;
		std::memcpy(buf, charTable2Real + (f0 >> 32), 2ull);
		std::memcpy(buf + 2, charTable2Real + (f2 >> 32), 2ull);
		std::memcpy(buf + 4, charTable2Real + (f4 >> 32), 2ull);
		return buf + 6ull;
	}

	JSONIFIER_INLINE static string_buffer_ptr length7(string_buffer_ptr buf, const uint64_t value) noexcept {
		static constexpr uint64_t multiplier{ uint64_t(10 * 0x1p48 / 1e7 + 1) };
		static constexpr uint64_t bitMask{ uint64_t(0x1p32) - 1 };
		const uint64_t f0 = multiplier * value >> 16;
		const uint64_t f2 = (f0 & bitMask) * 100;
		const uint64_t f4 = (f2 & bitMask) * 100;
		std::memcpy(buf, charTable1 + (f0 >> 32), 1ull);
		std::memcpy(buf + 1, charTable2Real + (f2 >> 32), 2ull);
		std::memcpy(buf + 3, charTable2Real + (f4 >> 32), 2ull);
		std::memcpy(buf + 5, charTable2Real + (((f4 & bitMask) * 100) >> 32), 2ull);
		return buf + 7ull;
	}

	JSONIFIER_INLINE static string_buffer_ptr length8(string_buffer_ptr buf, const uint64_t value) noexcept {
		static constexpr uint64_t multiplier{ uint64_t(10 * 0x1p48 / 1e7 + 1) };
		static constexpr uint64_t bitMask{ uint64_t(0x1p32) - 1 };
		const uint64_t f0 = multiplier * value >> 16;
		const uint64_t f2 = (f0 & bitMask) * 100;
		const uint64_t f4 = (f2 & bitMask) * 100;
		std::memcpy(buf, charTable2Real + (f0 >> 32), 2ull);
		std::memcpy(buf + 2, charTable2Real + (f2 >> 32), 2ull);
		std::memcpy(buf + 4, charTable2Real + (f4 >> 32), 2ull);
		std::memcpy(buf + 6, charTable2Real + (((f4 & bitMask) * 100) >> 32), 2ull);
		return buf + 8ull;
	}

	JSONIFIER_INLINE static string_buffer_ptr length9(string_buffer_ptr buf, const uint64_t value) noexcept {
		static constexpr uint64_t multiplier{ uint64_t(10 * uint64_t(0x1p57) / uint64_t(1e9) + 1) };
		static constexpr uint64_t bitMask{ uint64_t(0x1p57) - 1 };
		const uint64_t f0 = multiplier * value;
		const uint64_t f2 = (f0 & bitMask) * 100;
		const uint64_t f4 = (f2 & bitMask) * 100;
		const uint64_t f6 = (f4 & bitMask) * 100;
		std::memcpy(buf, charTable1 + (f0 >> 57), 1ull);
		std::memcpy(buf + 1, charTable2Real + (f2 >> 57), 2ull);
		std::memcpy(buf + 3, charTable2Real + (f4 >> 57), 2ull);
		std::memcpy(buf + 5, charTable2Real + (f6 >> 57), 2ull);
		std::memcpy(buf + 7, charTable2Real + (((f6 & bitMask) * 100) >> 57), 2ull);
		return buf + 9ull;
	}

	JSONIFIER_INLINE static string_buffer_ptr length10(string_buffer_ptr buf, const uint64_t value) noexcept {
		static constexpr uint64_t multiplier{ uint64_t(10 * 0x1p48 / 1e7 + 1) };
		static constexpr uint64_t bitMask{ uint64_t(0x1p32) - 1 };
		const uint64_t ab = value / 100000000ull;
		std::memcpy(buf, charTable2Real + ab, 2ull);
		const uint64_t cdefghijk = value - ab * 100000000ull;
		const uint64_t f0		 = multiplier * cdefghijk >> 16;
		const uint64_t f2		 = (f0 & bitMask) * 100;
		const uint64_t f4		 = (f2 & bitMask) * 100;
		std::memcpy(buf + 2, charTable2Real + (f0 >> 32), 2ull);
		std::memcpy(buf + 4, charTable2Real + (f2 >> 32), 2ull);
		std::memcpy(buf + 6, charTable2Real + (f4 >> 32), 2ull);
		std::memcpy(buf + 8, charTable2Real + (((f4 & bitMask) * 100) >> 32), 2ull);
		return buf + 10;
	}

	JSONIFIER_INLINE static string_buffer_ptr length11(string_buffer_ptr buf, const uint64_t value) noexcept {
		uint64_t abc = value / 100000000ull;
		static constexpr uint64_t multiplier3{ uint64_t(10 * 0x1p24 / 1e3 + 1) };
		static constexpr uint64_t bitMask3{ uint64_t(0x1p24) - 1 };
		uint64_t f0 = (multiplier3 * abc);
		uint64_t f2 = ((f0 & bitMask3) * 100);
		std::memcpy(buf, charTable1 + (f0 >> 24), 2ull);
		std::memcpy(buf + 1, charTable2Real + (f2 >> 24), 2ull);
		static constexpr uint64_t multiplier{ uint64_t(10 * 0x1p48 / 1e7 + 1) };
		static constexpr uint64_t bitMask{ uint64_t(0x1p32) - 1 };
		const uint64_t defghijkl = value - abc * 100000000ull;
		f0						 = multiplier * defghijkl >> 16;
		f2						 = (f0 & bitMask) * 100;
		const uint64_t f4		 = (f2 & bitMask) * 100;
		std::memcpy(buf + 3, charTable2Real + (f0 >> 32), 2ull);
		std::memcpy(buf + 5, charTable2Real + (f2 >> 32), 2ull);
		std::memcpy(buf + 7, charTable2Real + (f4 >> 32), 2ull);
		std::memcpy(buf + 9, charTable2Real + (((f4 & bitMask) * 100) >> 32), 2ull);
		return buf + 11;
	}

	JSONIFIER_INLINE static string_buffer_ptr length12(string_buffer_ptr buf, const uint64_t value) noexcept {
		uint64_t abcd = value / 100000000ull;
		static constexpr uint64_t multiplier4{ uint64_t(10 * 0x1p24 / 1e3 + 1) };
		static constexpr uint64_t bitMask4{ uint64_t(0x1p24) - 1 };
		uint64_t f0 = (multiplier4 * abcd);
		uint64_t f2 = ((f0 & bitMask4) * 100);
		std::memcpy(buf, charTable2Real + (f0 >> 24), 2ull);
		std::memcpy(buf + 2, charTable2Real + (f2 >> 24), 2ull);
		static constexpr uint64_t multiplier{ uint64_t(10 * 0x1p48 / 1e7 + 1) };
		static constexpr uint64_t bitMask{ uint64_t(0x1p32) - 1 };
		f0				  = multiplier * (value - abcd * 100000000ull) >> 16;
		f2				  = (f0 & bitMask) * 100;
		const uint64_t f4 = (f2 & bitMask) * 100;
		std::memcpy(buf + 4, charTable2Real + (f0 >> 32), 2ull);
		std::memcpy(buf + 6, charTable2Real + (f2 >> 32), 2ull);
		std::memcpy(buf + 8, charTable2Real + (f4 >> 32), 2ull);
		std::memcpy(buf + 10, charTable2Real + (((f4 & bitMask) * 100) >> 32), 2ull);
		return buf + 12;
	}

	JSONIFIER_INLINE static string_buffer_ptr length13(string_buffer_ptr buf, const uint64_t value) noexcept {
		uint64_t abcde = value / 100000000ull;
		static constexpr uint64_t multiplier5{ uint64_t(10 * 0x1p32 / 1e5 + 1) };
		static constexpr uint64_t bitMask5{ uint64_t(0x1p32) - 1 };
		uint64_t f0 = multiplier5 * abcde;
		uint64_t f2 = (f0 & bitMask5) * 100;
		uint64_t f4 = (f2 & bitMask5) * 100;
		std::memcpy(buf, charTable1 + (f0 >> 32), 1ull);
		std::memcpy(buf + 1, charTable2Real + (f2 >> 32), 2ull);
		std::memcpy(buf + 3, charTable2Real + (f4 >> 32), 2ull);
		static constexpr uint64_t multiplier{ uint64_t(10 * 0x1p48 / 1e7 + 1) };
		static constexpr uint64_t bitMask{ uint64_t(0x1p32) - 1 };
		f0 = multiplier * (value - abcde * 100000000ull) >> 16;
		f2 = (f0 & bitMask) * 100;
		f4 = (f2 & bitMask) * 100;
		std::memcpy(buf + 5, charTable2Real + (f0 >> 32), 2ull);
		std::memcpy(buf + 7, charTable2Real + (f2 >> 32), 2ull);
		std::memcpy(buf + 9, charTable2Real + (f4 >> 32), 2ull);
		std::memcpy(buf + 11, charTable2Real + (((f4 & bitMask) * 100) >> 32), 2ull);
		return buf + 13;
	}

	JSONIFIER_INLINE static string_buffer_ptr length14(string_buffer_ptr buf, const uint64_t value) noexcept {
		uint64_t abcdef = value / 100000000ull;
		static constexpr uint64_t multiplier6{ uint64_t(10 * 0x1p32 / 1e5 + 1) };
		static constexpr uint64_t bitMask6{ uint64_t(0x1p32) - 1 };
		uint64_t f0 = multiplier6 * abcdef;
		uint64_t f2 = (f0 & bitMask6) * 100;
		uint64_t f4 = (f2 & bitMask6) * 100;
		std::memcpy(buf, charTable2Real + (f0 >> 32), 2ull);
		std::memcpy(buf + 2, charTable2Real + (f2 >> 32), 2ull);
		std::memcpy(buf + 4, charTable2Real + (f4 >> 32), 2ull);
		static constexpr uint64_t multiplier{ uint64_t(10 * 0x1p48 / 1e7 + 1) };
		static constexpr uint64_t bitMask{ uint64_t(0x1p32) - 1 };
		f0 = multiplier * (value - abcdef * 100000000ull) >> 16;
		f2 = (f0 & bitMask) * 100;
		f4 = (f2 & bitMask) * 100;
		std::memcpy(buf + 6, charTable2Real + (f0 >> 32), 2ull);
		std::memcpy(buf + 8, charTable2Real + (f2 >> 32), 2ull);
		std::memcpy(buf + 10, charTable2Real + (f4 >> 32), 2ull);
		std::memcpy(buf + 12, charTable2Real + (((f4 & bitMask) * 100) >> 32), 2ull);
		return buf + 14;
	}

	JSONIFIER_INLINE static string_buffer_ptr length15(string_buffer_ptr buf, const uint64_t value) noexcept {
		uint64_t abcdefg = value / 100000000ull;
		static constexpr uint64_t multiplier7{ uint64_t(10 * 0x1p48 / 1e7 + 1) };
		static constexpr uint64_t bitMask7{ uint64_t(0x1p32) - 1 };
		uint64_t f0 = multiplier7 * abcdefg >> 16;
		uint64_t f2 = (f0 & bitMask7) * 100;
		uint64_t f4 = (f2 & bitMask7) * 100;
		uint64_t f6 = (f4 & bitMask7) * 100;
		std::memcpy(buf, charTable1 + (f0 >> 32), 1ull);
		std::memcpy(buf + 1, charTable2Real + (f2 >> 32), 2ull);
		std::memcpy(buf + 3, charTable2Real + (f4 >> 32), 2ull);
		std::memcpy(buf + 5, charTable2Real + (((f4 & bitMask7) * 100) >> 32), 2ull);
		static constexpr uint64_t multiplier{ uint64_t(10 * 0x1p48 / 1e7 + 1) };
		static constexpr uint64_t bitMask{ uint64_t(0x1p32) - 1 };
		f0 = multiplier * (value - abcdefg * 100000000ull) >> 16;
		f2 = (f0 & bitMask) * 100;
		f4 = (f2 & bitMask) * 100;
		std::memcpy(buf + 7, charTable2Real + (f0 >> 32), 2ull);
		std::memcpy(buf + 9, charTable2Real + (f2 >> 32), 2ull);
		std::memcpy(buf + 11, charTable2Real + (f4 >> 32), 2ull);
		std::memcpy(buf + 13, charTable2Real + (((f4 & bitMask) * 100) >> 32), 2ull);
		return buf + 15;
	}

	JSONIFIER_INLINE static string_buffer_ptr length16(string_buffer_ptr buf, const uint64_t value) noexcept {
		uint64_t abcdefgh = value / 100000000ull;
		static constexpr uint64_t multiplier{ uint64_t(10 * 0x1p48 / 1e7 + 1) };
		static constexpr uint64_t bitMask{ uint64_t(0x1p32) - 1 };
		uint64_t f0 = multiplier * abcdefgh >> 16;
		uint64_t f2 = (f0 & bitMask) * 100;
		uint64_t f4 = (f2 & bitMask) * 100;
		uint64_t f6 = (f4 & bitMask) * 100;
		std::memcpy(buf, charTable2Real + (f0 >> 32), 2ull);
		std::memcpy(buf + 2, charTable2Real + (f2 >> 32), 2ull);
		std::memcpy(buf + 4, charTable2Real + (f4 >> 32), 2ull);
		std::memcpy(buf + 6, charTable2Real + (f6 >> 32), 2ull);
		f0 = multiplier * (value - abcdefgh * 100000000ull) >> 16;
		f2 = (f0 & bitMask) * 100;
		f4 = (f2 & bitMask) * 100;
		std::memcpy(buf + 8, charTable2Real + (f0 >> 32), 2ull);
		std::memcpy(buf + 10, charTable2Real + (f2 >> 32), 2ull);
		std::memcpy(buf + 12, charTable2Real + (f4 >> 32), 2ull);
		std::memcpy(buf + 14, charTable2Real + (((f4 & bitMask) * 100) >> 32), 2ull);
		return buf + 16;
	}

	JSONIFIER_INLINE static string_buffer_ptr length17(string_buffer_ptr buf, const uint64_t value) noexcept {
		uint64_t abcdefghi = value / 100000000ull;
		static constexpr uint64_t multiplier9{ uint64_t(10 * uint64_t(0x1p57) / uint64_t(1e9) + 1) };
		static constexpr uint64_t bitMask9{ uint64_t(0x1p57) - 1 };
		uint64_t f0		  = multiplier9 * abcdefghi;
		uint64_t f2		  = (f0 & bitMask9) * 100;
		uint64_t f4		  = (f2 & bitMask9) * 100;
		uint64_t f6		  = (f4 & bitMask9) * 100;
		const uint64_t f8 = (f6 & bitMask9) * 100;
		std::memcpy(buf, charTable1 + (f0 >> 57), 1ull);
		std::memcpy(buf + 1, charTable2Real + (f2 >> 57), 2ull);
		std::memcpy(buf + 3, charTable2Real + (f4 >> 57), 2ull);
		std::memcpy(buf + 5, charTable2Real + (f6 >> 57), 2ull);
		std::memcpy(buf + 7, charTable2Real + (f8 >> 57), 2ull);
		static constexpr uint64_t multiplier{ uint64_t(10 * 0x1p48 / 1e7 + 1) };
		static constexpr uint64_t bitMask{ uint64_t(0x1p32) - 1 };
		f0 = multiplier * (value - abcdefghi * 100000000ull) >> 16;
		f2 = (f0 & bitMask) * 100;
		f4 = (f2 & bitMask) * 100;
		std::memcpy(buf + 9, charTable2Real + (f0 >> 32), 2ull);
		std::memcpy(buf + 11, charTable2Real + (f2 >> 32), 2ull);
		std::memcpy(buf + 13, charTable2Real + (f4 >> 32), 2ull);
		std::memcpy(buf + 15, charTable2Real + (((f4 & bitMask) * 100) >> 32), 2ull);
		return buf + 17;
	}

	JSONIFIER_INLINE static string_buffer_ptr length18(string_buffer_ptr buf, const uint64_t value) noexcept {
		uint64_t abcdefghij = value / 100000000ull;
		static constexpr uint64_t multiplier10{ uint64_t(10 * 0x1p48 / 1e7 + 1) };
		static constexpr uint64_t bitMask10{ uint64_t(0x1p32) - 1 };
		const uint64_t ab = abcdefghij / 100000000ull;
		std::memcpy(buf, charTable2Real + ab, 2ull);
		const uint64_t cdefghijk = abcdefghij - ab * 100000000ull;
		uint64_t f0				 = multiplier10 * cdefghijk >> 16;
		uint64_t f2				 = (f0 & bitMask10) * 100;
		uint64_t f4				 = (f2 & bitMask10) * 100;
		uint64_t f6				 = (f4 & bitMask10) * 100;
		std::memcpy(buf + 2, charTable2Real + (f0 >> 32), 2ull);
		std::memcpy(buf + 4, charTable2Real + (f2 >> 32), 2ull);
		std::memcpy(buf + 6, charTable2Real + (f4 >> 32), 2ull);
		std::memcpy(buf + 8, charTable2Real + (f6 >> 32), 2ull);
		static constexpr uint64_t multiplier{ uint64_t(10 * 0x1p48 / 1e7 + 1) };
		static constexpr uint64_t bitMask{ uint64_t(0x1p32) - 1 };
		f0 = multiplier * (value - abcdefghij * 100000000ull) >> 16;
		f2 = (f0 & bitMask) * 100;
		f4 = (f2 & bitMask) * 100;
		std::memcpy(buf + 10, charTable2Real + (f0 >> 32), 2ull);
		std::memcpy(buf + 12, charTable2Real + (f2 >> 32), 2ull);
		std::memcpy(buf + 14, charTable2Real + (f4 >> 32), 2ull);
		std::memcpy(buf + 16, charTable2Real + (((f4 & bitMask) * 100) >> 32), 2ull);
		return buf + 18;
	}

	JSONIFIER_INLINE static string_buffer_ptr length19(string_buffer_ptr buf, const uint64_t value) noexcept {
		const uint64_t tmp = value / 100000;
		const uint64_t hgh = tmp / 100000;
		const uint64_t mid = tmp - hgh * 100000;
		const uint64_t low = value - tmp * 100000;
		buf				   = length9(buf, hgh);
		buf				   = length5(buf, mid);
		return length5(buf, low);
	}

	JSONIFIER_INLINE static string_buffer_ptr length20(string_buffer_ptr buf, const uint64_t value) noexcept {
		const uint64_t tmp = value / 100000;
		const uint64_t hgh = tmp / 100000;
		const uint64_t mid = tmp - hgh * 100000;
		const uint64_t low = value - tmp * 100000;
		buf				   = length10(buf, hgh);
		buf				   = length5(buf, mid);
		return length5(buf, low);
	}

	JSONIFIER_INLINE static string_buffer_ptr impl64(string_buffer_ptr buf, const uint64_t value) noexcept {
		const uint64_t index{ fastDigitCount(value) };
		switch (index) {
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
			case 11: {
				return length11(buf, value);
			}
			case 12: {
				return length12(buf, value);
			}
			case 13: {
				return length13(buf, value);
			}
			case 14: {
				return length14(buf, value);
			}
			case 15: {
				return length15(buf, value);
			}
			case 16: {
				return length16(buf, value);
			}
			case 17: {
				return length17(buf, value);
			}
			case 18: {
				return length18(buf, value);
			}
			case 19: {
				return length19(buf, value);
			}
			case 20: {
				return length20(buf, value);
			}
			default: {
				std::unreachable();
			}
		}
	}

	template<typename value_type_new> JSONIFIER_INLINE static string_buffer_ptr toChars(string_buffer_ptr buf, const value_type_new value) noexcept {
		if constexpr (jsonifier::concepts::sig64_t<value_type_new>) {
			*buf = '-';
			return impl64(buf + (value < 0), uint64_t(value ^ (value >> 63)) - (value >> 63));
		} else {
			return impl64(buf, value);
		}
	}

}// namespace jsonifier_internal