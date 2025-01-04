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

	alignas(2) JSONIFIER_INLINE_VARIABLE char charTable1[]{ 0x30u, 0x31u, 0x32u, 0x33u, 0x34u, 0x35u, 0x36u, 0x37u, 0x38u, 0x39u };

	alignas(2) JSONIFIER_INLINE_VARIABLE char charTable2[]{ 0x30u, 0x30u, 0x30u, 0x31u, 0x30u, 0x32u, 0x30u, 0x33u, 0x30u, 0x34u, 0x30u, 0x35u, 0x30u, 0x36u, 0x30u, 0x37u, 0x30u,
		0x38u, 0x30u, 0x39u, 0x31u, 0x30u, 0x31u, 0x31u, 0x31u, 0x32u, 0x31u, 0x33u, 0x31u, 0x34u, 0x31u, 0x35u, 0x31u, 0x36u, 0x31u, 0x37u, 0x31u, 0x38u, 0x31u, 0x39u, 0x32u,
		0x30u, 0x32u, 0x31u, 0x32u, 0x32u, 0x32u, 0x33u, 0x32u, 0x34u, 0x32u, 0x35u, 0x32u, 0x36u, 0x32u, 0x37u, 0x32u, 0x38u, 0x32u, 0x39u, 0x33u, 0x30u, 0x33u, 0x31u, 0x33u,
		0x32u, 0x33u, 0x33u, 0x33u, 0x34u, 0x33u, 0x35u, 0x33u, 0x36u, 0x33u, 0x37u, 0x33u, 0x38u, 0x33u, 0x39u, 0x34u, 0x30u, 0x34u, 0x31u, 0x34u, 0x32u, 0x34u, 0x33u, 0x34u,
		0x34u, 0x34u, 0x35u, 0x34u, 0x36u, 0x34u, 0x37u, 0x34u, 0x38u, 0x34u, 0x39u, 0x35u, 0x30u, 0x35u, 0x31u, 0x35u, 0x32u, 0x35u, 0x33u, 0x35u, 0x34u, 0x35u, 0x35u, 0x35u,
		0x36u, 0x35u, 0x37u, 0x35u, 0x38u, 0x35u, 0x39u, 0x36u, 0x30u, 0x36u, 0x31u, 0x36u, 0x32u, 0x36u, 0x33u, 0x36u, 0x34u, 0x36u, 0x35u, 0x36u, 0x36u, 0x36u, 0x37u, 0x36u,
		0x38u, 0x36u, 0x39u, 0x37u, 0x30u, 0x37u, 0x31u, 0x37u, 0x32u, 0x37u, 0x33u, 0x37u, 0x34u, 0x37u, 0x35u, 0x37u, 0x36u, 0x37u, 0x37u, 0x37u, 0x38u, 0x37u, 0x39u, 0x38u,
		0x30u, 0x38u, 0x31u, 0x38u, 0x32u, 0x38u, 0x33u, 0x38u, 0x34u, 0x38u, 0x35u, 0x38u, 0x36u, 0x38u, 0x37u, 0x38u, 0x38u, 0x38u, 0x39u, 0x39u, 0x30u, 0x39u, 0x31u, 0x39u,
		0x32u, 0x39u, 0x33u, 0x39u, 0x34u, 0x39u, 0x35u, 0x39u, 0x36u, 0x39u, 0x37u, 0x39u, 0x38u, 0x39u, 0x39u };

	JSONIFIER_INLINE_VARIABLE uint8_t digitCounts[]{ 19, 19, 19, 19, 18, 18, 18, 17, 17, 17, 16, 16, 16, 16, 15, 15, 15, 14, 14, 14, 13, 13, 13, 13, 12, 12, 12, 11, 11, 11, 10, 10,
		10, 10, 9, 9, 9, 8, 8, 8, 7, 7, 7, 7, 6, 6, 6, 5, 5, 5, 4, 4, 4, 4, 3, 3, 3, 2, 2, 2, 1, 1, 1, 1, 1 };

	JSONIFIER_INLINE_VARIABLE uint64_t digitCountThresholds[]{ 0ull, 9ull, 99ull, 999ull, 9999ull, 99999ull, 999999ull, 9999999ull, 99999999ull, 999999999ull, 9999999999ull,
		99999999999ull, 999999999999ull, 9999999999999ull, 99999999999999ull, 999999999999999ull, 9999999999999999ull, 99999999999999999ull, 999999999999999999ull,
		9999999999999999999ull };

	JSONIFIER_INLINE uint64_t fastDigitCount(const uint64_t inputValue) {
		const uint64_t originalDigitCount{ static_cast<uint64_t>(digitCounts[simd_internal::lzcnt(inputValue)]) };
		return originalDigitCount + static_cast<uint64_t>(inputValue > digitCountThresholds[originalDigitCount]);
	}

	JSONIFIER_INLINE static string_buffer_ptr length1(string_buffer_ptr buf, const uint64_t value) noexcept {
		std::memcpy(buf, charTable1 + value, 1);
		return buf + 1;
	}

	JSONIFIER_INLINE static string_buffer_ptr length2(string_buffer_ptr buf, const uint64_t value) noexcept {
		std::memcpy(buf, charTable2 + value * 2, 2);
		return buf + 2;
	}

	JSONIFIER_INLINE static string_buffer_ptr length3(string_buffer_ptr buf, const uint64_t value) noexcept {
		const uint64_t aa = (value * 5243) >> 19;
		std::memcpy(buf, charTable1 + aa, 2);
		std::memcpy(buf + 1, charTable2 + (value - aa * 100) * 2, 2);
		return buf + 3;
	}

	JSONIFIER_INLINE static string_buffer_ptr length4(string_buffer_ptr buf, const uint64_t value) noexcept {
		const uint64_t aa = (value * 5243) >> 19;
		std::memcpy(buf, charTable2 + aa * 2, 2);
		std::memcpy(buf + 2, charTable2 + (value - aa * 100) * 2, 2);
		return buf + 4;
	}

	JSONIFIER_INLINE static string_buffer_ptr length5(string_buffer_ptr buf, const uint64_t value) noexcept {
		const uint64_t aa	= (value * 429497) >> 32;
		const uint64_t bbcc = value - aa * 10000;
		const uint64_t bb	= (bbcc * 5243) >> 19;
		std::memcpy(buf, charTable1 + aa, 2);
		std::memcpy(buf + 1, charTable2 + bb * 2, 2);
		std::memcpy(buf + 3, charTable2 + (bbcc - bb * 100) * 2, 2);
		return buf + 5;
	}

	JSONIFIER_INLINE static string_buffer_ptr length6(string_buffer_ptr buf, const uint64_t value) noexcept {
		const uint64_t aa	= (value * 429497) >> 32;
		const uint64_t bbcc = value - aa * 10000;
		const uint64_t bb	= (bbcc * 5243) >> 19;
		std::memcpy(buf, charTable2 + aa * 2, 2);
		std::memcpy(buf + 2, charTable2 + bb * 2, 2);
		std::memcpy(buf + 4, charTable2 + (bbcc - bb * 100) * 2, 2);
		return buf + 6;
	}

	JSONIFIER_INLINE static string_buffer_ptr length7(string_buffer_ptr buf, const uint64_t value) noexcept {
		const uint64_t aabb = (value * 109951163) >> 40;
		const uint64_t ccdd = value - aabb * 10000;
		const uint64_t aa	= (aabb * 5243) >> 19;
		const uint64_t cc	= (ccdd * 5243) >> 19;
		std::memcpy(buf, charTable1 + aa, 2);
		std::memcpy(buf + 1, charTable2 + (aabb - aa * 100) * 2, 2);
		std::memcpy(buf + 3, charTable2 + cc * 2, 2);
		std::memcpy(buf + 5, charTable2 + (ccdd - cc * 100) * 2, 2);
		return buf + 7;
	}

	JSONIFIER_INLINE static string_buffer_ptr length8(string_buffer_ptr buf, const uint64_t value) noexcept {
		const uint64_t aabb = (value * 109951163) >> 40;
		const uint64_t ccdd = value - aabb * 10000;
		const uint64_t aa	= (aabb * 5243) >> 19;
		const uint64_t cc	= (ccdd * 5243) >> 19;
		std::memcpy(buf, charTable2 + aa * 2, 2);
		std::memcpy(buf + 2, charTable2 + (aabb - aa * 100) * 2, 2);
		std::memcpy(buf + 4, charTable2 + cc * 2, 2);
		std::memcpy(buf + 6, charTable2 + (ccdd - cc * 100) * 2, 2);
		return buf + 8;
	}

	JSONIFIER_INLINE static string_buffer_ptr length9(string_buffer_ptr buf, const uint64_t value) noexcept {
		const uint64_t high = value / 100000000ull;
		const uint64_t low	= value - high * 100000000ull;
		const uint64_t aabb = (low * 109951163ull) >> 40ull;
		const uint64_t ccdd = low - aabb * 10000ull;
		const uint64_t aa	= (aabb * 5243ull) >> 19ull;
		const uint64_t bb	= aabb - aa * 100ull;
		const uint64_t cc	= (ccdd * 5243ull) >> 19ull;
		std::memcpy(buf, charTable1 + high, 1ull);
		std::memcpy(buf + 1ull, charTable2 + aa * 2ull, 2ull);
		std::memcpy(buf + 3ull, charTable2 + bb * 2ull, 2ull);
		std::memcpy(buf + 5ull, charTable2 + cc * 2ull, 2ull);
		std::memcpy(buf + 7ull, charTable2 + (ccdd - cc * 100ull) * 2ull, 2ull);
		return buf + 9ull;
	}

	JSONIFIER_INLINE static string_buffer_ptr length10(string_buffer_ptr buf, const uint64_t value) noexcept {
		const uint64_t high = value / 100000000ull;
		const uint64_t low	= value - high * 100000000ull;
		const uint64_t aabb = (low * 109951163ull) >> 40ull;
		const uint64_t ccdd = low - aabb * 10000ull;
		const uint64_t aa	= (aabb * 5243ull) >> 19ull;
		const uint64_t bb	= aabb - aa * 100ull;
		const uint64_t cc	= (ccdd * 5243ull) >> 19ull;
		std::memcpy(buf, charTable2 + high * 2ull, 2ull);
		std::memcpy(buf + 2ull, charTable2 + aa * 2ull, 2ull);
		std::memcpy(buf + 4ull, charTable2 + bb * 2ull, 2ull);
		std::memcpy(buf + 6ull, charTable2 + cc * 2ull, 2ull);
		std::memcpy(buf + 8ull, charTable2 + (ccdd - cc * 100ull) * 2ull, 2ull);
		return buf + 10ull;
	}

	JSONIFIER_INLINE static string_buffer_ptr length11(string_buffer_ptr buf, const uint64_t value) noexcept {
		const uint64_t high = value / 100000000ull;
		const uint64_t low	= value - high * 100000000ull;
		uint64_t aa			= (high * 5243ull) >> 19ull;
		uint64_t bb			= high - aa * 100ull;
		const uint64_t aabb = (low * 109951163ull) >> 40ull;
		const uint64_t ccdd = low - aabb * 10000ull;
		std::memcpy(buf, charTable1 + aa, 1ull);
		std::memcpy(buf + 1ull, charTable2 + bb * 2ull, 2ull);
		aa				  = (aabb * 5243ull) >> 19ull;
		bb				  = aabb - aa * 100ull;
		const uint64_t cc = (ccdd * 5243ull) >> 19ull;
		std::memcpy(buf + 3ull, charTable2 + aa * 2ull, 2ull);
		std::memcpy(buf + 5ull, charTable2 + bb * 2ull, 2ull);
		std::memcpy(buf + 7ull, charTable2 + cc * 2ull, 2ull);
		std::memcpy(buf + 9ull, charTable2 + (ccdd - cc * 100ull) * 2ull, 2ull);
		return buf + 11ull;
	}

	JSONIFIER_INLINE static string_buffer_ptr length12(string_buffer_ptr buf, const uint64_t value) noexcept {
		const uint64_t high = value / 100000000ull;
		const uint64_t low	= value - high * 100000000ull;
		uint64_t aa			= (high * 5243ull) >> 19ull;
		uint64_t bb			= high - aa * 100ull;
		std::memcpy(buf, charTable2 + aa * 2ull, 2ull);
		std::memcpy(buf + 2ull, charTable2 + bb * 2ull, 2ull);
		const uint64_t aabb = (low * 109951163ull) >> 40ull;
		const uint64_t ccdd = low - aabb * 10000ull;
		aa					= (aabb * 5243ull) >> 19ull;
		bb					= aabb - aa * 100ull;
		const uint64_t cc	= (ccdd * 5243ull) >> 19ull;
		std::memcpy(buf + 4ull, charTable2 + aa * 2ull, 2ull);
		std::memcpy(buf + 6ull, charTable2 + bb * 2ull, 2ull);
		std::memcpy(buf + 8ull, charTable2 + cc * 2ull, 2ull);
		std::memcpy(buf + 10ull, charTable2 + (ccdd - cc * 100ull) * 2ull, 2ull);
		return buf + 12ull;
	}

	JSONIFIER_INLINE static string_buffer_ptr length13(string_buffer_ptr buf, const uint64_t value) noexcept {
		const uint64_t high = value / 100000000ull;
		const uint64_t low	= value - high * 100000000ull;
		uint64_t aa			= (high * 429497ull) >> 32ull;
		const uint64_t bbcc = high - aa * 10000ull;
		uint64_t bb			= (bbcc * 5243ull) >> 19ull;
		uint64_t cc			= bbcc - bb * 100ull;
		std::memcpy(buf, charTable1 + aa, 1ull);
		std::memcpy(buf + 1ull, charTable2 + bb * 2ull, 2ull);
		std::memcpy(buf + 3ull, charTable2 + cc * 2ull, 2ull);
		const uint64_t aabb = (low * 109951163ull) >> 40ull;
		const uint64_t ccdd = low - aabb * 10000ull;
		aa					= (aabb * 5243ull) >> 19ull;
		bb					= aabb - aa * 100ull;
		cc					= (ccdd * 5243ull) >> 19ull;
		std::memcpy(buf + 5ull, charTable2 + aa * 2ull, 2ull);
		std::memcpy(buf + 7ull, charTable2 + bb * 2ull, 2ull);
		std::memcpy(buf + 9ull, charTable2 + cc * 2ull, 2ull);
		std::memcpy(buf + 11ull, charTable2 + (ccdd - cc * 100ull) * 2ull, 2ull);
		return buf + 13ull;
	}

	JSONIFIER_INLINE static string_buffer_ptr length14(string_buffer_ptr buf, const uint64_t value) noexcept {
		const uint64_t high = value / 100000000ull;
		const uint64_t low	= value - high * 100000000ull;
		const uint64_t aabb = (low * 109951163ull) >> 40ull;
		const uint64_t ccdd = low - aabb * 10000ull;
		uint64_t aa			= (high * 429497ull) >> 32ull;
		const uint64_t bbcc = high - aa * 10000ull;
		uint64_t bb			= (bbcc * 5243ull) >> 19ull;
		uint64_t cc			= bbcc - bb * 100ull;
		std::memcpy(buf, charTable2 + aa * 2ull, 2ull);
		std::memcpy(buf + 2ull, charTable2 + bb * 2ull, 2ull);
		std::memcpy(buf + 4ull, charTable2 + cc * 2ull, 2ull);
		aa = (aabb * 5243ull) >> 19ull;
		bb = aabb - aa * 100ull;
		cc = (ccdd * 5243ull) >> 19ull;
		std::memcpy(buf + 6ull, charTable2 + aa * 2ull, 2ull);
		std::memcpy(buf + 8ull, charTable2 + bb * 2ull, 2ull);
		std::memcpy(buf + 10ull, charTable2 + cc * 2ull, 2ull);
		std::memcpy(buf + 12ull, charTable2 + (ccdd - cc * 100ull) * 2ull, 2ull);
		return buf + 14ull;
	}

	JSONIFIER_INLINE static string_buffer_ptr length15(string_buffer_ptr buf, const uint64_t value) noexcept {
		const uint64_t high = value / 100000000ull;
		const uint64_t low	= value - high * 100000000ull;
		uint64_t aabb		= (high * 109951163ull) >> 40ull;
		uint64_t ccdd		= high - aabb * 10000ull;
		uint64_t aa			= (aabb * 5243ull) >> 19ull;
		uint64_t bb			= aabb - aa * 100ull;
		uint64_t cc			= (ccdd * 5243ull) >> 19ull;
		uint64_t dd			= ccdd - cc * 100ull;
		std::memcpy(buf, charTable1 + aa, 1ull);
		std::memcpy(buf + 1ull, charTable2 + bb * 2ull, 2ull);
		std::memcpy(buf + 3ull, charTable2 + cc * 2ull, 2ull);
		std::memcpy(buf + 5ull, charTable2 + dd * 2ull, 2ull);
		aabb = (low * 109951163ull) >> 40ull;
		ccdd = low - aabb * 10000ull;
		aa	 = (aabb * 5243ull) >> 19ull;
		bb	 = aabb - aa * 100ull;
		cc	 = (ccdd * 5243ull) >> 19ull;
		dd	 = ccdd - cc * 100ull;
		std::memcpy(buf + 7ull, charTable2 + aa * 2ull, 2ull);
		std::memcpy(buf + 9ull, charTable2 + bb * 2ull, 2ull);
		std::memcpy(buf + 11ull, charTable2 + cc * 2ull, 2ull);
		std::memcpy(buf + 13ull, charTable2 + dd * 2ull, 2ull);
		return buf + 15ull;
	}

	JSONIFIER_INLINE static string_buffer_ptr length16(string_buffer_ptr buf, const uint64_t value) noexcept {
		const uint64_t high = value / 100000000ull;
		const uint64_t low	= value - high * 100000000ull;
		uint64_t aabb		= (high * 109951163ull) >> 40ull;
		uint64_t ccdd		= high - aabb * 10000ull;
		uint64_t aa			= (aabb * 5243ull) >> 19ull;
		uint64_t bb			= aabb - aa * 100ull;
		uint64_t cc			= (ccdd * 5243ull) >> 19ull;
		uint64_t dd			= ccdd - cc * 100ull;
		std::memcpy(buf, charTable2 + aa * 2ull, 2ull);
		std::memcpy(buf + 2ull, charTable2 + bb * 2ull, 2ull);
		std::memcpy(buf + 4ull, charTable2 + cc * 2ull, 2ull);
		std::memcpy(buf + 6ull, charTable2 + dd * 2ull, 2ull);
		aabb = (low * 109951163ull) >> 40ull;
		ccdd = low - aabb * 10000ull;
		aa	 = (aabb * 5243ull) >> 19ull;
		bb	 = aabb - aa * 100ull;
		cc	 = (ccdd * 5243ull) >> 19ull;
		dd	 = ccdd - cc * 100ull;
		std::memcpy(buf + 8ull, charTable2 + aa * 2ull, 2ull);
		std::memcpy(buf + 10ull, charTable2 + bb * 2ull, 2ull);
		std::memcpy(buf + 12ull, charTable2 + cc * 2ull, 2ull);
		std::memcpy(buf + 14ull, charTable2 + dd * 2ull, 2ull);
		return buf + 16ull;
	}

	JSONIFIER_INLINE static string_buffer_ptr length17(string_buffer_ptr buf, const uint64_t value) noexcept {
		const uint64_t tmp	= value / 100000000ull;
		const uint64_t low	= value - tmp * 100000000ull;
		const uint64_t high = tmp / 10000ull;
		const uint64_t mid	= tmp - high * 10000ull;
		const uint64_t aabb = (low * 109951163ull) >> 40ull;
		const uint64_t ccdd = low - aabb * 10000ull;
		uint64_t aa			= (high * 429497ull) >> 32ull;
		const uint64_t bbcc = high - aa * 10000ull;
		uint64_t bb			= (bbcc * 5243ull) >> 19ull;
		uint64_t cc			= bbcc - bb * 100ull;
		std::memcpy(buf, charTable1 + aa, 1ull);
		std::memcpy(buf + 1ull, charTable2 + bb * 2ull, 2ull);
		std::memcpy(buf + 3ull, charTable2 + cc * 2ull, 2ull);
		aa = (mid * 5243ull) >> 19ull;
		bb = mid - aa * 100ull;
		std::memcpy(buf + 5ull, charTable2 + aa * 2ull, 2ull);
		std::memcpy(buf + 7ull, charTable2 + bb * 2ull, 2ull);
		aa				  = (aabb * 5243ull) >> 19ull;
		bb				  = aabb - aa * 100ull;
		cc				  = (ccdd * 5243ull) >> 19ull;
		const uint64_t dd = ccdd - cc * 100ull;
		std::memcpy(buf + 9ull, charTable2 + aa * 2ull, 2ull);
		std::memcpy(buf + 11ull, charTable2 + bb * 2ull, 2ull);
		std::memcpy(buf + 13ull, charTable2 + cc * 2ull, 2ull);
		std::memcpy(buf + 15ull, charTable2 + dd * 2ull, 2ull);
		return buf + 17ull;
	}

	JSONIFIER_INLINE static string_buffer_ptr length18(string_buffer_ptr buf, const uint64_t value) noexcept {
		const uint64_t tmp	= value / 100000000ull;
		const uint64_t low	= value - tmp * 100000000ull;
		const uint64_t high = tmp / 10000ull;
		const uint64_t mid	= tmp - high * 10000ull;
		const uint64_t aabb = (low * 109951163ull) >> 40ull;
		const uint64_t ccdd = low - aabb * 10000ull;
		uint64_t aa			= (high * 429497ull) >> 32ull;
		const uint64_t bbcc = high - aa * 10000ull;
		uint64_t bb			= (bbcc * 5243ull) >> 19ull;
		uint64_t cc			= bbcc - bb * 100ull;
		std::memcpy(buf, charTable2 + aa * 2ull, 2ull);
		std::memcpy(buf + 2ull, charTable2 + bb * 2ull, 2ull);
		std::memcpy(buf + 4ull, charTable2 + cc * 2ull, 2ull);
		aa = (mid * 5243ull) >> 19ull;
		bb = mid - aa * 100ull;
		std::memcpy(buf + 6ull, charTable2 + aa * 2ull, 2ull);
		std::memcpy(buf + 8ull, charTable2 + bb * 2ull, 2ull);
		aa				  = (aabb * 5243ull) >> 19ull;
		bb				  = aabb - aa * 100ull;
		cc				  = (ccdd * 5243ull) >> 19ull;
		const uint64_t dd = ccdd - cc * 100ull;
		std::memcpy(buf + 10ull, charTable2 + aa * 2ull, 2ull);
		std::memcpy(buf + 12ull, charTable2 + bb * 2ull, 2ull);
		std::memcpy(buf + 14ull, charTable2 + cc * 2ull, 2ull);
		std::memcpy(buf + 16ull, charTable2 + dd * 2ull, 2ull);
		return buf + 18ull;
	}

	JSONIFIER_INLINE static string_buffer_ptr length19(string_buffer_ptr buf, const uint64_t value) noexcept {
		const uint64_t tmp	= value / 100000000ull;
		const uint64_t low	= value - tmp * 100000000ull;
		const uint64_t high = tmp / 10000ull;
		const uint64_t mid	= tmp - high * 10000ull;
		uint64_t aabb		= (high * 109951163ull) >> 40ull;
		uint64_t ccdd		= high - aabb * 10000ull;
		uint64_t aa			= (aabb * 5243ull) >> 19ull;
		uint64_t bb			= aabb - aa * 100ull;
		uint64_t cc			= (ccdd * 5243ull) >> 19ull;
		uint64_t dd			= ccdd - cc * 100ull;
		std::memcpy(buf, charTable1 + aa, 1ull);
		std::memcpy(buf + 1ull, charTable2 + bb * 2ull, 2ull);
		std::memcpy(buf + 3ull, charTable2 + cc * 2ull, 2ull);
		std::memcpy(buf + 5ull, charTable2 + dd * 2ull, 2ull);
		aa = (mid * 5243ull) >> 19ull;
		bb = mid - aa * 100ull;
		std::memcpy(buf + 7ull, charTable2 + aa * 2ull, 2ull);
		std::memcpy(buf + 9ull, charTable2 + bb * 2ull, 2ull);
		aabb = (low * 109951163ull) >> 40ull;
		ccdd = low - aabb * 10000ull;
		aa	 = (aabb * 5243ull) >> 19ull;
		bb	 = aabb - aa * 100ull;
		cc	 = (ccdd * 5243ull) >> 19ull;
		dd	 = ccdd - cc * 100ull;
		std::memcpy(buf + 11ull, charTable2 + aa * 2ull, 2ull);
		std::memcpy(buf + 13ull, charTable2 + bb * 2ull, 2ull);
		std::memcpy(buf + 15ull, charTable2 + cc * 2ull, 2ull);
		std::memcpy(buf + 17ull, charTable2 + dd * 2ull, 2ull);
		return buf + 19ull;
	}

	JSONIFIER_INLINE static string_buffer_ptr length20(string_buffer_ptr buf, const uint64_t value) noexcept {
		const uint64_t tmp	= value / 100000000ull;
		const uint64_t low	= value - tmp * 100000000ull;
		const uint64_t high = tmp / 10000ull;
		const uint64_t mid	= tmp - high * 10000ull;
		uint64_t aabb		= (high * 109951163ull) >> 40ull;
		uint64_t ccdd		= high - aabb * 10000ull;
		uint64_t aa			= (aabb * 5243ull) >> 19ull;
		uint64_t bb			= aabb - aa * 100ull;
		uint64_t cc			= (ccdd * 5243ull) >> 19ull;
		uint64_t dd			= ccdd - cc * 100ull;
		std::memcpy(buf, charTable2 + aa * 2ull, 2ull);
		std::memcpy(buf + 2ull, charTable2 + bb * 2ull, 2ull);
		std::memcpy(buf + 4ull, charTable2 + cc * 2ull, 2ull);
		std::memcpy(buf + 6ull, charTable2 + dd * 2ull, 2ull);
		aa = (mid * 5243ull) >> 19ull;
		bb = mid - aa * 100ull;
		std::memcpy(buf + 8ull, charTable2 + aa * 2ull, 2ull);
		std::memcpy(buf + 10ull, charTable2 + bb * 2ull, 2ull);
		aabb = (low * 109951163ull) >> 40ull;
		ccdd = low - aabb * 10000ull;
		aa	 = (aabb * 5243ull) >> 19ull;
		bb	 = aabb - aa * 100ull;
		cc	 = (ccdd * 5243ull) >> 19ull;
		dd	 = ccdd - cc * 100ull;
		std::memcpy(buf + 12ull, charTable2 + aa * 2ull, 2ull);
		std::memcpy(buf + 14ull, charTable2 + bb * 2ull, 2ull);
		std::memcpy(buf + 16ull, charTable2 + cc * 2ull, 2ull);
		std::memcpy(buf + 18ull, charTable2 + dd * 2ull, 2ull);
		return buf + 20ull;
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

	template<jsonifier::concepts::uns64_t value_type_new> JSONIFIER_INLINE static string_buffer_ptr toChars(string_buffer_ptr buf, const value_type_new value) noexcept {
		return impl64(buf, value);
	}

	template<jsonifier::concepts::sig64_t value_type_new> JSONIFIER_INLINE static string_buffer_ptr toChars(string_buffer_ptr buf, const value_type_new value) noexcept {
		*buf = '-';
		return toChars(buf + (value < 0), static_cast<const uint64_t>(value ^ (value >> 63)) - (value >> 63));
	}

}// namespace jsonifier_internal