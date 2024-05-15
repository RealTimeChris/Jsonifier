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

	template<typename value_type>
		requires std::is_same_v<value_type, uint32_t>
	JSONIFIER_INLINE auto* toChars(auto* buf, value_type val) noexcept {
		uint32_t aa, bb, cc, dd, ee, aabb, bbcc, ccdd, ddee, aabbcc;
		uint32_t lz;

		if (val < 100) {
			lz = val < 10;
			std::memcpy(buf, charTable + (val * 2 + lz), 2);
			buf -= lz;
			return buf + 2;
		} else if (val < 10000) {
			aa = (val * 5243) >> 19;
			bb = val - aa * 100;
			lz = aa < 10;
			std::memcpy(buf, charTable + (aa * 2 + lz), 2);
			buf -= lz;
			std::memcpy(buf + 2, charTable + (2 * bb), 2);

			return buf + 4;
		} else if (val < 1000000) {
			aa	 = uint32_t((uint64_t(val) * 429497) >> 32);
			bbcc = val - aa * 10000;
			bb	 = (bbcc * 5243) >> 19;
			cc	 = bbcc - bb * 100;
			lz	 = aa < 10;
			std::memcpy(buf, charTable + aa * 2 + lz, 2);
			buf -= lz;
			std::memcpy(buf + 2, charTable + bb * 2, 2);
			std::memcpy(buf + 4, charTable + cc * 2, 2);
			return buf + 6;
		} else if (val < 100000000) {
			aabb = uint32_t((uint64_t(val) * 109951163) >> 40);
			ccdd = val - aabb * 10000;
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
		} else {
			aabbcc = uint32_t((uint64_t(val) * 3518437209ul) >> 45);
			aa	   = uint32_t((uint64_t(aabbcc) * 429497) >> 32);
			ddee   = val - aabbcc * 10000;
			bbcc   = aabbcc - aa * 10000;
			bb	   = (bbcc * 5243) >> 19;
			dd	   = (ddee * 5243) >> 19;
			cc	   = bbcc - bb * 100;
			ee	   = ddee - dd * 100;
			lz	   = aa < 10;
			std::memcpy(buf, charTable + aa * 2 + lz, 2);
			buf -= lz;
			std::memcpy(buf + 2, charTable + bb * 2, 2);
			std::memcpy(buf + 4, charTable + cc * 2, 2);
			std::memcpy(buf + 6, charTable + dd * 2, 2);
			std::memcpy(buf + 8, charTable + ee * 2, 2);
			return buf + 10;
		}
	}

	template<typename value_type>
		requires std::is_same_v<value_type, int32_t>
	JSONIFIER_INLINE auto* toChars(auto* buf, value_type x) noexcept {
		*buf = '-';
		return toChars(buf + (x < 0), uint32_t(x ^ (x >> 31)) - (x >> 31));
	}

	template<typename char_type> JSONIFIER_INLINE char_type* to_chars_u64_len_8(char_type* buf, uint32_t val) noexcept {
		const uint32_t aabb = uint32_t((uint64_t(val) * 109951163) >> 40);
		const uint32_t ccdd = val - aabb * 10000;
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

	template<typename char_type> JSONIFIER_INLINE char_type* to_chars_u64_len_4(char_type* buf, uint32_t val) noexcept {
		const uint32_t aa = (val * 5243) >> 19;
		const uint32_t bb = val - aa * 100;
		std::memcpy(buf, charTable + aa * 2, 2);
		std::memcpy(buf + 2, charTable + bb * 2, 2);
		return buf + 4;
	}

	template<typename char_type> JSONIFIER_INLINE char_type* to_chars_u64_len_1_8(char_type* buf, uint32_t val) noexcept {
		uint32_t aa, bb, cc, dd, aabb, bbcc, ccdd, lz;

		if (val < 100) {
			lz = val < 10;
			std::memcpy(buf, charTable + val * 2 + lz, 2);
			buf -= lz;
			return buf + 2;
		} else if (val < 10000) {
			aa = (val * 5243) >> 19;
			bb = val - aa * 100;
			lz = aa < 10;
			std::memcpy(buf, charTable + aa * 2 + lz, 2);
			buf -= lz;
			std::memcpy(buf + 2, charTable + bb * 2, 2);
			return buf + 4;
		} else if (val < 1000000) {
			aa	 = uint32_t((uint64_t(val) * 429497) >> 32);
			bbcc = val - aa * 10000;
			bb	 = (bbcc * 5243) >> 19;
			cc	 = bbcc - bb * 100;
			lz	 = aa < 10;
			std::memcpy(buf, charTable + aa * 2 + lz, 2);
			buf -= lz;
			std::memcpy(buf + 2, charTable + bb * 2, 2);
			std::memcpy(buf + 4, charTable + cc * 2, 2);
			return buf + 6;
		} else {
			aabb = uint32_t((uint64_t(val) * 109951163) >> 40);
			ccdd = val - aabb * 10000;
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

	template<typename char_type> JSONIFIER_INLINE char_type* to_chars_u64_len_5_8(char_type* buf, uint32_t val) noexcept {
		if (val < 1000000) {
			const uint32_t aa	= uint32_t((uint64_t(val) * 429497) >> 32);
			const uint32_t bbcc = val - aa * 10000;
			const uint32_t bb	= (bbcc * 5243) >> 19;
			const uint32_t cc	= bbcc - bb * 100;
			const uint32_t lz	= aa < 10;
			std::memcpy(buf, charTable + aa * 2 + lz, 2);
			buf -= lz;
			std::memcpy(buf + 2, charTable + bb * 2, 2);
			std::memcpy(buf + 4, charTable + cc * 2, 2);
			return buf + 6;
		} else {
			const uint32_t aabb = uint32_t((uint64_t(val) * 109951163) >> 40);
			const uint32_t ccdd = val - aabb * 10000;
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

	template<typename value_type, typename char_type>
		requires std::is_same_v<value_type, uint64_t>
	JSONIFIER_INLINE char_type* toChars(char_type* buf, value_type val) noexcept {
		if (val < 100000000) {
			buf = to_chars_u64_len_1_8(buf, uint32_t(val));
			return buf;
		} else if (val < 100000000ull * 100000000ull) {
			const uint64_t hgh = val / 100000000;
			const auto low	   = uint32_t(val - hgh * 100000000);
			buf				   = to_chars_u64_len_1_8(buf, uint32_t(hgh));
			buf				   = to_chars_u64_len_8(buf, low);
			return buf;
		} else {
			const uint64_t tmp = val / 100000000;
			const auto low	   = uint32_t(val - tmp * 100000000);
			const auto hgh	   = uint32_t(tmp / 10000);
			const auto mid	   = uint32_t(tmp - hgh * 10000);
			buf				   = to_chars_u64_len_5_8(buf, hgh);
			buf				   = to_chars_u64_len_4(buf, mid);
			buf				   = to_chars_u64_len_8(buf, low);
			return buf;
		}
	}

	template<typename value_type, typename char_type>
		requires std::is_same_v<value_type, int64_t>
	JSONIFIER_INLINE char_type* toChars(char_type* buf, value_type x) noexcept {
		*buf = '-';
		return toChars(buf + (x < 0), uint64_t(x ^ (x >> 63)) - (x >> 63));
	}

}// namespace jsonifier_internal