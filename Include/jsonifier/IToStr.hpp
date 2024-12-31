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

	alignas(2) constexpr char charTable[]{ 0x30u, 0x30u, 0x30u, 0x31u, 0x30u, 0x32u, 0x30u, 0x33u, 0x30u, 0x34u, 0x30u, 0x35u, 0x30u, 0x36u, 0x30u, 0x37u, 0x30u, 0x38u, 0x30u,
		0x39u, 0x31u, 0x30u, 0x31u, 0x31u, 0x31u, 0x32u, 0x31u, 0x33u, 0x31u, 0x34u, 0x31u, 0x35u, 0x31u, 0x36u, 0x31u, 0x37u, 0x31u, 0x38u, 0x31u, 0x39u, 0x32u, 0x30u, 0x32u,
		0x31u, 0x32u, 0x32u, 0x32u, 0x33u, 0x32u, 0x34u, 0x32u, 0x35u, 0x32u, 0x36u, 0x32u, 0x37u, 0x32u, 0x38u, 0x32u, 0x39u, 0x33u, 0x30u, 0x33u, 0x31u, 0x33u, 0x32u, 0x33u,
		0x33u, 0x33u, 0x34u, 0x33u, 0x35u, 0x33u, 0x36u, 0x33u, 0x37u, 0x33u, 0x38u, 0x33u, 0x39u, 0x34u, 0x30u, 0x34u, 0x31u, 0x34u, 0x32u, 0x34u, 0x33u, 0x34u, 0x34u, 0x34u,
		0x35u, 0x34u, 0x36u, 0x34u, 0x37u, 0x34u, 0x38u, 0x34u, 0x39u, 0x35u, 0x30u, 0x35u, 0x31u, 0x35u, 0x32u, 0x35u, 0x33u, 0x35u, 0x34u, 0x35u, 0x35u, 0x35u, 0x36u, 0x35u,
		0x37u, 0x35u, 0x38u, 0x35u, 0x39u, 0x36u, 0x30u, 0x36u, 0x31u, 0x36u, 0x32u, 0x36u, 0x33u, 0x36u, 0x34u, 0x36u, 0x35u, 0x36u, 0x36u, 0x36u, 0x37u, 0x36u, 0x38u, 0x36u,
		0x39u, 0x37u, 0x30u, 0x37u, 0x31u, 0x37u, 0x32u, 0x37u, 0x33u, 0x37u, 0x34u, 0x37u, 0x35u, 0x37u, 0x36u, 0x37u, 0x37u, 0x37u, 0x38u, 0x37u, 0x39u, 0x38u, 0x30u, 0x38u,
		0x31u, 0x38u, 0x32u, 0x38u, 0x33u, 0x38u, 0x34u, 0x38u, 0x35u, 0x38u, 0x36u, 0x38u, 0x37u, 0x38u, 0x38u, 0x38u, 0x39u, 0x39u, 0x30u, 0x39u, 0x31u, 0x39u, 0x32u, 0x39u,
		0x33u, 0x39u, 0x34u, 0x39u, 0x35u, 0x39u, 0x36u, 0x39u, 0x37u, 0x39u, 0x38u, 0x39u, 0x39u };

	JSONIFIER_INLINE char* length4(char* buf, uint64_t value) noexcept {
		const uint64_t aa = (value * 5243) >> 19;
		const uint64_t bb = value - aa * 100;
		std::memcpy(buf, charTable + aa * 2, 2);
		std::memcpy(buf + 2, charTable + bb * 2, 2);
		return buf + 4;
	}

	JSONIFIER_INLINE char* length8(char* buf, uint64_t value) noexcept {
		const uint64_t aabb = (value * 109951163) >> 40;
		const uint64_t ccdd = value - aabb * 10000;
		const uint64_t aa	= (aabb * 5243) >> 19;
		const uint64_t cc	= (ccdd * 5243) >> 19;
		const uint64_t bb	= aabb - aa * 100;
		const uint64_t dd	= ccdd - cc * 100;
		std::memcpy(buf, charTable + aa * 2, 2);
		std::memcpy(buf + 2, charTable + bb * 2, 2);
		std::memcpy(buf + 4, charTable + cc * 2, 2);
		std::memcpy(buf + 6, charTable + dd * 2, 2);
		return buf + 8;
	}

	JSONIFIER_INLINE static char* lt100(char* buf, uint64_t value) noexcept {
		const bool lz = value < 10;
		std::memcpy(buf, charTable + value * 2 + lz, 2);
		buf -= lz;
		return buf + 2;
	}

	JSONIFIER_INLINE static char* lt10000(char* buf, uint64_t value) noexcept {
		const uint64_t aa = (value * 5243) >> 19;
		const uint64_t bb = value - aa * 100;
		const bool lz	  = aa < 10;
		std::memcpy(buf, charTable + aa * 2 + lz, 2);
		buf -= lz;
		std::memcpy(buf + 2, charTable + bb * 2, 2);
		return buf + 4;
	}

	JSONIFIER_INLINE static char* lt1000000(char* buf, uint64_t value) noexcept {
		const uint64_t aa	= (value * 429497) >> 32;
		const uint64_t bbcc = value - aa * 10000;
		const uint64_t bb	= (bbcc * 5243) >> 19;
		const uint64_t cc	= bbcc - bb * 100;
		const bool lz		= aa < 10;
		std::memcpy(buf, charTable + aa * 2 + lz, 2);
		buf -= lz;
		std::memcpy(buf + 2, charTable + bb * 2, 2);
		std::memcpy(buf + 4, charTable + cc * 2, 2);
		return buf + 6;
	}

	JSONIFIER_INLINE static char* lt100000000(char* buf, uint64_t value) noexcept {
		const uint64_t aabb = (value * 109951163) >> 40;
		const uint64_t ccdd = value - aabb * 10000;
		const uint64_t aa	= (aabb * 5243) >> 19;
		const uint64_t cc	= (ccdd * 5243) >> 19;
		const uint64_t bb	= aabb - aa * 100;
		const uint64_t dd	= ccdd - cc * 100;
		const bool lz		= aa < 10;
		std::memcpy(buf, charTable + aa * 2 + lz, 2);
		buf -= lz;
		std::memcpy(buf + 2, charTable + bb * 2, 2);
		std::memcpy(buf + 4, charTable + cc * 2, 2);
		std::memcpy(buf + 6, charTable + dd * 2, 2);
		return buf + 8;
	}

	JSONIFIER_INLINE static char* lt10000000000000000(char* buf, uint64_t value) noexcept {
		const uint64_t hgh = value / 100000000;
		const uint64_t low = value - hgh * 100000000;
		static constexpr uint64_t hundred{ 100 };
		static constexpr uint64_t tenThousand{ 10000 };
		static constexpr uint64_t million{ 1000000 };
		if (hgh >= million) {
			buf = lt100000000(buf, hgh);
		} else if (hgh >= tenThousand) {
			buf = lt1000000(buf, hgh);
		} else if (hgh >= hundred) {
			buf = lt10000(buf, hgh);
		} else {
			buf = lt100(buf, hgh);
		}
		buf = length8(buf, low);
		return buf;
	}

	JSONIFIER_INLINE static char* gte10000000000000000(char* buf, uint64_t value) noexcept {
		const uint64_t tmp = value / 100000000;
		const uint64_t low = value - tmp * 100000000;
		const uint64_t hgh = tmp / 10000;
		const uint64_t mid = tmp - hgh * 10000;
		static constexpr uint64_t million{ 1000000 };
		if (hgh >= million) {
			buf = lt100000000(buf, hgh);
		} else {
			buf = lt1000000(buf, hgh);
		}
		buf = length4(buf, mid);
		buf = length8(buf, low);
		return buf;
	}

	template<jsonifier::concepts::uns64_t value_type> JSONIFIER_INLINE static char* toChars(char* buf, value_type value) noexcept {
		static constexpr value_type hundred{ 100 };
		static constexpr value_type tenThousand{ 10000 };
		static constexpr value_type million{ 1000000 };
		static constexpr value_type tenMillion{ 100000000 };
		static constexpr value_type tenQuadrillion{ 10000000000000000 };
		if (value >= tenQuadrillion) {
			return gte10000000000000000(buf, value);
		} else if (value >= tenMillion) {
			return lt10000000000000000(buf, value);
		} else if (value >= million) {
			return lt100000000(buf, value);
		} else if (value >= tenThousand) {
			return lt1000000(buf, value);
		} else if (value >= hundred) {
			return lt10000(buf, value);
		} else {
			return lt100(buf, value);
		}
		return buf;
	}

	template<jsonifier::concepts::sig64_t value_type> JSONIFIER_INLINE static char* toChars(char* buf, value_type value) noexcept {
		*buf = '-';
		return toChars<uint64_t>(buf + (value < 0), static_cast<uint64_t>(value ^ (value >> 63)) - (value >> 63));
	}

}// namespace jsonifier_internal