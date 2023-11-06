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

	template<typename value_type> inline value_type* toCharsU64Len8(value_type* buf, uint64_t value) {
		uint64_t aa, bb, cc, dd, aabb, ccdd;
		aabb = uint64_t((uint64_t(value) * 109951163) >> 40);
		ccdd = value - aabb * 10000;
		aa	 = (aabb * 5243) >> 19;
		cc	 = (ccdd * 5243) >> 19;
		bb	 = aabb - aa * 100;
		dd	 = ccdd - cc * 100;
		std::memcpy(buf, charTable + aa * 2ULL, 2);
		std::memcpy(buf + 2ULL, charTable + bb * 2ULL, 2);
		std::memcpy(buf + 4ULL, charTable + cc * 2ULL, 2);
		std::memcpy(buf + 6ULL, charTable + dd * 2ULL, 2);
		return buf + 8;
	}

	template<typename value_type> inline value_type* toCharsU64Len4(value_type* buf, uint64_t value) {
		uint64_t aa, bb;
		aa = (value * 5243) >> 19;
		bb = value - aa * 100;
		std::memcpy(buf, charTable + aa * 2ULL, 2);
		std::memcpy(buf + 2ULL, charTable + bb * 2ULL, 2);
		return buf + 4ULL;
	}

	template<typename value_type> inline value_type* toCharsU64Len18(value_type* buf, uint64_t value) {
		uint64_t aa, bb, cc, dd, aabb, bbcc, ccdd, lz;

		if (value < 100) {
			lz = value < 10;
			std::memcpy(buf, charTable + value * 2ULL + lz, 2);
			buf -= lz;
			return buf + 2ULL;
		} else if (value < 10000) {
			aa = (value * 5243) >> 19;
			bb = value - aa * 100;
			lz = aa < 10;
			std::memcpy(buf, charTable + aa * 2ULL + lz, 2);
			buf -= lz;
			std::memcpy(buf + 2ULL, charTable + bb * 2ULL, 2);
			return buf + 4ULL;
		} else if (value < 1000000) {
			aa	 = uint64_t((uint64_t(value) * 429497) >> 32);
			bbcc = value - aa * 10000;
			bb	 = (bbcc * 5243) >> 19;
			cc	 = bbcc - bb * 100;
			lz	 = aa < 10;
			std::memcpy(buf, charTable + aa * 2ULL + lz, 2);
			buf -= lz;
			std::memcpy(buf + 2ULL, charTable + bb * 2ULL, 2);
			std::memcpy(buf + 4ULL, charTable + cc * 2ULL, 2);
			return buf + 6ULL;
		} else {
			aabb = uint64_t((uint64_t(value) * 109951163) >> 40);
			ccdd = value - aabb * 10000;
			aa	 = (aabb * 5243) >> 19;
			cc	 = (ccdd * 5243) >> 19;
			bb	 = aabb - aa * 100;
			dd	 = ccdd - cc * 100;
			lz	 = aa < 10;
			std::memcpy(buf, charTable + aa * 2ULL + lz, 2);
			buf -= lz;
			std::memcpy(buf + 2ULL, charTable + bb * 2ULL, 2);
			std::memcpy(buf + 4ULL, charTable + cc * 2ULL, 2);
			std::memcpy(buf + 6ULL, charTable + dd * 2ULL, 2);
			return buf + 8;
		}
	}

	template<typename value_type> inline value_type* toCharsU64Len58(value_type* buf, uint64_t value) {
		uint64_t aa, bb, cc, dd, aabb, bbcc, ccdd, lz;

		if (value < 1000000) {
			aa	 = uint64_t((uint64_t(value) * 429497) >> 32);
			bbcc = value - aa * 10000;
			bb	 = (bbcc * 5243) >> 19;
			cc	 = bbcc - bb * 100;
			lz	 = aa < 10;
			std::memcpy(buf, charTable + aa * 2ULL + lz, 2);
			buf -= lz;
			std::memcpy(buf + 2ULL, charTable + bb * 2ULL, 2);
			std::memcpy(buf + 4ULL, charTable + cc * 2ULL, 2);
			return buf + 6ULL;
		} else {
			aabb = uint64_t((uint64_t(value) * 109951163) >> 40);
			ccdd = value - aabb * 10000;
			aa	 = (aabb * 5243) >> 19;
			cc	 = (ccdd * 5243) >> 19;
			bb	 = aabb - aa * 100;
			dd	 = ccdd - cc * 100;
			lz	 = aa < 10;
			std::memcpy(buf, charTable + aa * 2ULL + lz, 2);
			buf -= lz;
			std::memcpy(buf + 2ULL, charTable + bb * 2ULL, 2);
			std::memcpy(buf + 4ULL, charTable + cc * 2ULL, 2);
			std::memcpy(buf + 6ULL, charTable + dd * 2ULL, 2);
			return buf + 8;
		}
	}

	template<jsonifier::concepts::unsigned_t value_type, typename char_type> inline char_type* toChars(char_type* buf, value_type value) {
		uint64_t tmp, hgh;
		uint64_t mid, low;

		if (value < 100000000) {
			buf = toCharsU64Len18(buf, uint64_t(value));
			return buf;
		} else if (value < 100000000ULL * 100000000ULL) {
			hgh = value / 100000000ULL;
			low = uint64_t(value - hgh * 100000000ULL);
			buf = toCharsU64Len18(buf, uint64_t(hgh));
			buf = toCharsU64Len8(buf, low);
			return buf;
		} else {
			tmp = value / 100000000ULL;
			low = uint64_t(value - tmp * 100000000ULL);
			hgh = uint64_t(tmp / 10000ULL);
			mid = uint64_t(tmp - hgh * 10000ULL);
			buf = toCharsU64Len58(buf, uint64_t(hgh));
			buf = toCharsU64Len4(buf, mid);
			buf = toCharsU64Len8(buf, low);
			return buf;
		}
	}

	template<jsonifier::concepts::signed_t value_type, typename char_type> inline char_type* toChars(char_type* buf, value_type value) {
		uint64_t neg  = uint64_t(-value);
		uint64_t sign = value < 0;
		*buf		  = '-';
		return toChars(buf + sign, sign ? uint64_t(neg) : uint64_t(value));
	}

}// namespace jsonifier_internal
