/*
	MIT License

	Copyright (c) 2023 RealTimeChris

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
#include <jsonifier/Tables.hpp>

#include <concepts>
#include <cstdint>
#include <cstring>
#include <array>

namespace jsonifier_internal {

	template<typename char_type> JSONIFIER_INLINE char_type* toCharsU64Len4(char_type* buf, uint64_t val) noexcept {
		const uint64_t aa = (val * 5243ull) >> 19;
		const uint64_t bb = val - aa * 100ull;
		std::memcpy(buf, charTable + aa * 2, 2);
		std::memcpy(buf + 2, charTable + bb * 2, 2);
		return buf + 4;
	}

	template<typename char_type> JSONIFIER_INLINE char_type* toCharsU64Len8(char_type* buf, uint64_t val) noexcept {
		uint64_t aa{}, bb{}, cc{}, dd{}, aabb{}, ccdd{};
		aabb = (val * 109951163ull) >> 40;
		ccdd = val - aabb * 10000ull;
		aa	 = (aabb * 5243ull) >> 19;
		cc	 = (ccdd * 5243ull) >> 19;
		bb	 = aabb - aa * 100ull;
		dd	 = ccdd - cc * 100ull;
		std::memcpy(buf, charTable + aa * 2, 2);
		std::memcpy(buf + 2, charTable + bb * 2, 2);
		std::memcpy(buf + 4, charTable + cc * 2, 2);
		std::memcpy(buf + 6, charTable + dd * 2, 2);
		return buf + 8;
	}

	template<typename char_type> JSONIFIER_INLINE char_type* toCharsU64Len18(char_type* buf, uint64_t val) noexcept {
		uint64_t aa{}, bb{}, cc{}, dd{}, aabb{}, bbcc{}, ccdd{}, lz{};

		if (val < 100ull) {
			lz = val < 10ull;
			std::memcpy(buf, charTable + val * 2 + lz, 2);
			buf -= lz;
			return buf + 2;
		} else if (val < 10000ull) {
			aa = (val * 5243ull) >> 19;
			bb = val - aa * 100ull;
			lz = aa < 10ull;
			std::memcpy(buf, charTable + aa * 2 + lz, 2);
			buf -= lz;
			std::memcpy(buf + 2, charTable + bb * 2, 2);
			return buf + 4;
		} else if (val < 1000000ull) {
			aa	 = (val * 429497ull) >> 32;
			bbcc = val - aa * 10000ull;
			bb	 = (bbcc * 5243ull) >> 19;
			cc	 = bbcc - bb * 100ull;
			lz	 = aa < 10ull;
			std::memcpy(buf, charTable + aa * 2 + lz, 2);
			buf -= lz;
			std::memcpy(buf + 2, charTable + bb * 2, 2);
			std::memcpy(buf + 4, charTable + cc * 2, 2);
			return buf + 6;
		} else {
			aabb = (val * 109951163ull) >> 40;
			ccdd = val - aabb * 10000ull;
			aa	 = (aabb * 5243ull) >> 19;
			cc	 = (ccdd * 5243ull) >> 19;
			bb	 = aabb - aa * 100ull;
			dd	 = ccdd - cc * 100ull;
			lz	 = aa < 10ull;
			std::memcpy(buf, charTable + aa * 2 + lz, 2);
			buf -= lz;
			std::memcpy(buf + 2, charTable + bb * 2, 2);
			std::memcpy(buf + 4, charTable + cc * 2, 2);
			std::memcpy(buf + 6, charTable + dd * 2, 2);
			return buf + 8;
		}
	}

	template<typename char_type> JSONIFIER_INLINE char_type* toCharsU64Len58(char_type* buf, uint64_t val) noexcept {
		if (val < 1000000) {
			const uint64_t aa	= (val * 429497ull) >> 32;
			const uint64_t bbcc = val - aa * 10000ull;
			const uint64_t bb	= (bbcc * 5243ull) >> 19;
			const uint64_t cc	= bbcc - bb * 100ull;
			const uint64_t lz	= aa < 10ull;
			std::memcpy(buf, charTable + aa * 2 + lz, 2);
			buf -= lz;
			std::memcpy(buf + 2, charTable + bb * 2, 2);
			std::memcpy(buf + 4, charTable + cc * 2, 2);
			return buf + 6;
		} else {
			const uint64_t aabb = (val * 109951163ull) >> 40;
			const uint64_t ccdd = val - aabb * 10000ull;
			const uint64_t aa	= (aabb * 5243ull) >> 19;
			const uint64_t cc	= (ccdd * 5243ull) >> 19;
			const uint64_t bb	= aabb - aa * 100ull;
			const uint64_t dd	= ccdd - cc * 100ull;
			const uint64_t lz	= aa < 10ull;
			std::memcpy(buf, charTable + aa * 2 + lz, 2);
			buf -= lz;
			std::memcpy(buf + 2, charTable + bb * 2, 2);
			std::memcpy(buf + 4, charTable + cc * 2, 2);
			std::memcpy(buf + 6, charTable + dd * 2, 2);
			return buf + 8;
		}
	}

	template<jsonifier::concepts::uint64_type value_type01, typename char_type> JSONIFIER_INLINE char_type* toChars(char_type* buf, value_type01 val) noexcept {
		if (val < 100000000) {
			buf = toCharsU64Len18(buf, val);
			return buf;
		} else if (val < 100000000ull * 100000000ull) {
			const uint64_t hgh = static_cast<uint64_t>(val) / 100000000ull;
			const uint64_t low = static_cast<uint64_t>(val) - hgh * 100000000ull;
			buf				   = toCharsU64Len18(buf, hgh);
			buf				   = toCharsU64Len8(buf, low);
			return buf;
		} else {
			const uint64_t tmp = static_cast<uint64_t>(val) / 100000000ull;
			const uint64_t low = static_cast<uint64_t>(val) - tmp * 100000000ull;
			const uint64_t hgh = tmp / 10000ull;
			const uint64_t mid = tmp - hgh * 10000ull;
			buf				   = toCharsU64Len58(buf, hgh);
			buf				   = toCharsU64Len4(buf, mid);
			buf				   = toCharsU64Len8(buf, low);
			return buf;
		}
	}

	template<jsonifier::concepts::int64_type value_type01, typename char_type> JSONIFIER_INLINE char_type* toChars(char_type* buf, value_type01 val) noexcept {
		*buf = 0x2Du;
		return toChars(buf + (val < 0), uint64_t(val ^ (val >> 63)) - (val >> 63));
	}

}// namespace jsonifier_internal
