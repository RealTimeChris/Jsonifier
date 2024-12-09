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
/// Note: Most of the code in this header was sampled from Glaze library: https://github.com/stephenberry/glaze
/// https://github.com/RealTimeChris/jsonifier
/// Nov 13, 2023
#pragma once

#include <jsonifier/Allocator.hpp>

#include <concepts>
#include <cstdint>
#include <cstring>
#include <array>

namespace jsonifier_internal {

	constexpr uint8_t charTable[]{ 0x30u, 0x30u, 0x30u, 0x31u, 0x30u, 0x32u, 0x30u, 0x33u, 0x30u, 0x34u, 0x30u, 0x35u, 0x30u, 0x36u, 0x30u, 0x37u, 0x30u, 0x38u, 0x30u, 0x39u,
		0x31u, 0x30u, 0x31u, 0x31u, 0x31u, 0x32u, 0x31u, 0x33u, 0x31u, 0x34u, 0x31u, 0x35u, 0x31u, 0x36u, 0x31u, 0x37u, 0x31u, 0x38u, 0x31u, 0x39u, 0x32u, 0x30u, 0x32u, 0x31u,
		0x32u, 0x32u, 0x32u, 0x33u, 0x32u, 0x34u, 0x32u, 0x35u, 0x32u, 0x36u, 0x32u, 0x37u, 0x32u, 0x38u, 0x32u, 0x39u, 0x33u, 0x30u, 0x33u, 0x31u, 0x33u, 0x32u, 0x33u, 0x33u,
		0x33u, 0x34u, 0x33u, 0x35u, 0x33u, 0x36u, 0x33u, 0x37u, 0x33u, 0x38u, 0x33u, 0x39u, 0x34u, 0x30u, 0x34u, 0x31u, 0x34u, 0x32u, 0x34u, 0x33u, 0x34u, 0x34u, 0x34u, 0x35u,
		0x34u, 0x36u, 0x34u, 0x37u, 0x34u, 0x38u, 0x34u, 0x39u, 0x35u, 0x30u, 0x35u, 0x31u, 0x35u, 0x32u, 0x35u, 0x33u, 0x35u, 0x34u, 0x35u, 0x35u, 0x35u, 0x36u, 0x35u, 0x37u,
		0x35u, 0x38u, 0x35u, 0x39u, 0x36u, 0x30u, 0x36u, 0x31u, 0x36u, 0x32u, 0x36u, 0x33u, 0x36u, 0x34u, 0x36u, 0x35u, 0x36u, 0x36u, 0x36u, 0x37u, 0x36u, 0x38u, 0x36u, 0x39u,
		0x37u, 0x30u, 0x37u, 0x31u, 0x37u, 0x32u, 0x37u, 0x33u, 0x37u, 0x34u, 0x37u, 0x35u, 0x37u, 0x36u, 0x37u, 0x37u, 0x37u, 0x38u, 0x37u, 0x39u, 0x38u, 0x30u, 0x38u, 0x31u,
		0x38u, 0x32u, 0x38u, 0x33u, 0x38u, 0x34u, 0x38u, 0x35u, 0x38u, 0x36u, 0x38u, 0x37u, 0x38u, 0x38u, 0x38u, 0x39u, 0x39u, 0x30u, 0x39u, 0x31u, 0x39u, 0x32u, 0x39u, 0x33u,
		0x39u, 0x34u, 0x39u, 0x35u, 0x39u, 0x36u, 0x39u, 0x37u, 0x39u, 0x38u, 0x39u, 0x39u };

	JSONIFIER_ALWAYS_INLINE char* toCharsU64Len8(char* buf, uint32_t value) noexcept {
		const uint32_t aabb = static_cast<uint32_t>((static_cast<uint64_t>(value) * 109951163) >> 40);
		const uint32_t ccdd = value - aabb * 10000;
		const uint32_t aa	= (aabb * 5243) >> 19;
		const uint32_t cc	= (ccdd * 5243) >> 19;
		const uint32_t bb	= aabb - aa * 100;
		const uint32_t dd	= ccdd - cc * 100;
		std::memcpy(buf, charTable + aa * 2, 2);
		std::memcpy(buf + 2, charTable + bb * 2, 2);
		std::memcpy(buf + 4, charTable + cc * 2, 2);
		std::memcpy(buf + 6, charTable + dd * 2, 2);
		return buf + 8;
	}

	JSONIFIER_ALWAYS_INLINE char* toCharsU64Len4(char* buf, uint32_t value) noexcept {
		const uint32_t aa = (value * 5243) >> 19;
		const uint32_t bb = value - aa * 100;
		std::memcpy(buf, charTable + aa * 2, 2);
		std::memcpy(buf + 2, charTable + bb * 2, 2);
		return buf + 4;
	}

	JSONIFIER_ALWAYS_INLINE char* toCharsU64Len18(char* buf, uint32_t value) noexcept {
		uint32_t aa, bb, cc, dd, aabb, bbcc, ccdd, lz;

		if (value < 100) {
			lz = value < 10;
			std::memcpy(buf, charTable + value * 2 + lz, 2);
			buf -= lz;
			return buf + 2;
		} else {
			if (value < 10000) {
				aa = (value * 5243) >> 19;
				bb = value - aa * 100;
				lz = aa < 10;
				std::memcpy(buf, charTable + aa * 2 + lz, 2);
				buf -= lz;
				std::memcpy(buf + 2, charTable + bb * 2, 2);
				return buf + 4;
			} else {
				if (value < 1000000) {
					aa	 = static_cast<uint32_t>((static_cast<uint64_t>(value) * 429497) >> 32);
					bbcc = value - aa * 10000;
					bb	 = (bbcc * 5243) >> 19;
					cc	 = bbcc - bb * 100;
					lz	 = aa < 10;
					std::memcpy(buf, charTable + aa * 2 + lz, 2);
					buf -= lz;
					std::memcpy(buf + 2, charTable + bb * 2, 2);
					std::memcpy(buf + 4, charTable + cc * 2, 2);
					return buf + 6;
				} else {
					aabb = static_cast<uint32_t>((static_cast<uint64_t>(value) * 109951163) >> 40);
					ccdd = value - aabb * 10000;
					aa	 = (aabb * 5243) >> 19;
					cc	 = (ccdd * 5243) >> 19;
					bb	 = aabb - aa * 100;
					dd	 = ccdd - cc * 100;
					lz	 = aa < 10;
					std::memcpy(buf, charTable + aa * 2 + lz, 2);
					buf -= lz;
					std::memcpy(buf + 2, charTable + bb * 2, 2);
					std::memcpy(buf + 4, charTable + cc * 2, 2);
					std::memcpy(buf + 6, charTable + dd * 2, 2);
					return buf + 8;
				}
			}
		}
	}

	JSONIFIER_ALWAYS_INLINE char* toCharsU64Len58(char* buf, uint32_t value) noexcept {
		if (value < 1000000) {
			const uint32_t aa	= static_cast<uint32_t>((static_cast<uint64_t>(value) * 429497) >> 32);
			const uint32_t bbcc = value - aa * 10000;
			const uint32_t bb	= (bbcc * 5243) >> 19;
			const uint32_t cc	= bbcc - bb * 100;
			const uint32_t lz	= aa < 10;
			std::memcpy(buf, charTable + aa * 2 + lz, 2);
			buf -= lz;
			std::memcpy(buf + 2, charTable + bb * 2, 2);
			std::memcpy(buf + 4, charTable + cc * 2, 2);
			return buf + 6;
		} else {
			const uint32_t aabb = static_cast<uint32_t>((static_cast<uint64_t>(value) * 109951163) >> 40);
			const uint32_t ccdd = value - aabb * 10000;
			const uint32_t aa	= (aabb * 5243) >> 19;
			const uint32_t cc	= (ccdd * 5243) >> 19;
			const uint32_t bb	= aabb - aa * 100;
			const uint32_t dd	= ccdd - cc * 100;
			const uint32_t lz	= aa < 10;
			std::memcpy(buf, charTable + aa * 2 + lz, 2);
			buf -= lz;
			std::memcpy(buf + 2, charTable + bb * 2, 2);
			std::memcpy(buf + 4, charTable + cc * 2, 2);
			std::memcpy(buf + 6, charTable + dd * 2, 2);
			return buf + 8;
		}
	}

	template<jsonifier::concepts::uns64_t value_type> JSONIFIER_ALWAYS_INLINE static char* toChars(char* buf, value_type value) noexcept {
		if (value == 0) {
			*buf = '0';
			return buf + 1;
		} else if (value < 100000000) {
			buf = toCharsU64Len18(buf, static_cast<uint32_t>(value));
			return buf;
		} else {
			if (value < 100000000ull * 100000000ull) {
				const uint64_t hgh = value / 100000000;
				auto low		   = static_cast<uint32_t>(value - hgh * 100000000);
				buf				   = toCharsU64Len18(buf, static_cast<uint32_t>(hgh));
				buf				   = toCharsU64Len8(buf, low);
				return buf;
			} else {
				const uint64_t tmp = value / 100000000;
				auto low		   = static_cast<uint32_t>(value - tmp * 100000000);
				auto hgh		   = static_cast<uint32_t>(tmp / 10000);
				auto mid		   = static_cast<uint32_t>(tmp - hgh * 10000);
				buf				   = toCharsU64Len58(buf, hgh);
				buf				   = toCharsU64Len4(buf, mid);
				buf				   = toCharsU64Len8(buf, low);
				return buf;
			}
		}
	}

	template<jsonifier::concepts::sig64_t value_type> JSONIFIER_ALWAYS_INLINE static char* toChars(char* buf, value_type value) noexcept {
		*buf = '-';
		return toChars<uint64_t>(buf + (value < 0), static_cast<uint64_t>(value ^ (value >> 63)) - (value >> 63));
	}

}// namespace jsonifier_internal