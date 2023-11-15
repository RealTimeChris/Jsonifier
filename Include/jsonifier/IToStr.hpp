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

	template<typename value_type> inline value_type* toCharsU64Len8(value_type* buf, uint64_t val) {
		uint64_t aa, bb, cc, dd, aabb, ccdd;
		aabb = static_cast<uint64_t>(static_cast<uint64_t>(val * 109951163ULL) >> 40);
		ccdd = val - aabb * 10000ULL;
		aa	 = (aabb * 5243ULL) >> 19;
		cc	 = (ccdd * 5243ULL) >> 19;
		bb	 = aabb - aa * 100ULL;
		dd	 = ccdd - cc * 100ULL;
		std::memcpy(buf + 0, charTable + aa * 2, 2);
		std::memcpy(buf + 2, charTable + bb * 2, 2);
		std::memcpy(buf + 4, charTable + cc * 2, 2);
		std::memcpy(buf + 6, charTable + dd * 2, 2);
		return buf + 8;
	}

	template<typename value_type> inline value_type* toCharsU64Len4(value_type* buf, uint64_t val) {
		uint64_t aa, bb;
		aa = (val * 5243ULL) >> 19;
		bb = val - aa * 100ULL;
		std::memcpy(buf + 0, charTable + aa * 2, 2);
		std::memcpy(buf + 2, charTable + bb * 2, 2);
		return buf + 4;
	}

	template<typename value_type> inline value_type* toCharsU64Len18(value_type* buf, uint64_t val) {
		uint64_t aa, bb, cc, dd, aabb, bbcc, ccdd, lz;

		if (val < 100ULL) {
			lz = val < 10ULL;
			std::memcpy(buf + 0, charTable + val * 2 + lz, 2);
			buf -= lz;
			return buf + 2;

		} else if (val < 10000ULL) {
			aa = (val * 5243ULL) >> 19;
			bb = val - aa * 100ULL;
			lz = aa < 10ULL;
			std::memcpy(buf + 0, charTable + aa * 2 + lz, 2);
			buf -= lz;
			std::memcpy(buf + 2, charTable + bb * 2, 2);
			return buf + 4;

		} else if (val < 1000000ULL) {
			aa	 = static_cast<uint64_t>(static_cast<uint64_t>(val * 429497ULL) >> 32);
			bbcc = val - aa * 10000ULL;
			bb	 = (bbcc * 5243ULL) >> 19;
			cc	 = bbcc - bb * 100ULL;
			lz	 = aa < 10ULL;
			std::memcpy(buf + 0, charTable + aa * 2 + lz, 2);
			buf -= lz;
			std::memcpy(buf + 2, charTable + bb * 2, 2);
			std::memcpy(buf + 4, charTable + cc * 2, 2);
			return buf + 6;

		} else {
			aabb = static_cast<uint64_t>(static_cast<uint64_t>(val * 109951163ULL) >> 40);
			ccdd = val - aabb * 10000ULL;
			aa	 = (aabb * 5243ULL) >> 19;
			cc	 = (ccdd * 5243ULL) >> 19;
			bb	 = aabb - aa * 100ULL;
			dd	 = ccdd - cc * 100ULL;
			lz	 = aa < 10ULL;
			std::memcpy(buf + 0, charTable + aa * 2 + lz, 2);
			buf -= lz;
			std::memcpy(buf + 2, charTable + bb * 2, 2);
			std::memcpy(buf + 4, charTable + cc * 2, 2);
			std::memcpy(buf + 6, charTable + dd * 2, 2);
			return buf + 8;
		}
	}

	template<typename value_type> inline value_type* toCharsU64Len58(value_type* buf, uint64_t val) {
		uint64_t aa, bb, cc, dd, aabb, bbcc, ccdd, lz;

		if (val < 1000000ULL) {
			aa	 = static_cast<uint64_t>(static_cast<uint64_t>(val * 429497ULL) >> 32);
			bbcc = val - aa * 10000ULL;
			bb	 = (bbcc * 5243ULL) >> 19;
			cc	 = bbcc - bb * 100ULL;
			lz	 = aa < 10ULL;
			std::memcpy(buf + 0, charTable + aa * 2 + lz, 2);
			buf -= lz;
			std::memcpy(buf + 2, charTable + bb * 2, 2);
			std::memcpy(buf + 4, charTable + cc * 2, 2);
			return buf + 6;

		} else {
			aabb = static_cast<uint64_t>(static_cast<uint64_t>(val * 109951163ULL) >> 40);
			ccdd = val - aabb * 10000ULL;
			aa	 = (aabb * 5243ULL) >> 19;
			cc	 = (ccdd * 5243ULL) >> 19;
			bb	 = aabb - aa * 100ULL;
			dd	 = ccdd - cc * 100ULL;
			lz	 = aa < 10ULL;
			std::memcpy(buf + 0, charTable + aa * 2 + lz, 2);
			buf -= lz;
			std::memcpy(buf + 2, charTable + bb * 2, 2);
			std::memcpy(buf + 4, charTable + cc * 2, 2);
			std::memcpy(buf + 6, charTable + dd * 2, 2);
			return buf + 8;
		}
	}

	template<jsonifier::concepts::unsigned_t value_type, typename char_type> inline char_type* toChars(char_type* buf, value_type value) {
		uint64_t tmp, hgh;
		uint64_t mid, low;

		if (value < 100000000ULL) {
			buf = toCharsU64Len18(buf, static_cast<uint64_t>(value));
			return buf;
		} else if (value < 100000000ULL * 100000000ULL) {
			hgh = value / 100000000ULL;
			low = static_cast<uint64_t>(value - hgh * 100000000ULL);
			buf = toCharsU64Len18(buf, static_cast<uint64_t>(hgh));
			buf = toCharsU64Len8(buf, low);
			return buf;
		} else {
			tmp = value / 100000000ULL;
			low = static_cast<uint64_t>(value - tmp * 100000000ULL);
			hgh = static_cast<uint64_t>(tmp / 10000ULL);
			mid = static_cast<uint64_t>(tmp - hgh * 10000ULL);
			buf = toCharsU64Len58(buf, static_cast<uint64_t>(hgh));
			buf = toCharsU64Len4(buf, mid);
			buf = toCharsU64Len8(buf, low);
			return buf;
		}
	}

	template<jsonifier::concepts::signed_t value_type, typename char_type> inline char_type* toChars(char_type* buf, value_type value) {
		uint64_t neg  = static_cast<uint64_t>(-value);
		uint64_t sign = value < 0;
		*buf		  = '-';
		return toChars(buf + sign, sign ? static_cast<uint64_t>(neg) : static_cast<uint64_t>(value));
	}

}// namespace jsonifier_internal
