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

	alignas(2) constexpr char charTable1[]{ 0x30u, 0x31u, 0x32u, 0x33u, 0x34u, 0x35u, 0x36u, 0x37u, 0x38u, 0x39u };

	alignas(2) constexpr char charTable2[]{ 0x30u, 0x30u, 0x30u, 0x31u, 0x30u, 0x32u, 0x30u, 0x33u, 0x30u, 0x34u, 0x30u, 0x35u, 0x30u, 0x36u, 0x30u, 0x37u, 0x30u, 0x38u, 0x30u,
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
	JSONIFIER_INLINE uint32_t fastDigitCount(const uint32_t x) noexcept {
		return (x + digitCountTable[31 - simd_internal::lzcnt(x | 1)]) >> 32;
	}

	struct integer_serializer {
		template<size_t index> JSONIFIER_INLINE static char* impl(char* buf, uint64_t value) noexcept {
			uint32_t aa, bb, cc, dd, aabb, bbcc, ccdd, ee, ddee;
			uint64_t aabbcc;
			if constexpr (index == 1) {
				std::memcpy(buf, charTable1 + value, 1);
				return buf + 1;
			} else if constexpr (index == 2) {
				std::memcpy(buf, charTable2 + value * 2, 2);
				return buf + 2;
			} else if constexpr (index == 3) {
				aa = (value * 5243) >> 19;
				bb = value - aa * 100;
				std::memcpy(buf, charTable1 + aa, 1);
				std::memcpy(buf + 1, charTable2 + bb * 2, 2);
				return buf + 3;
			} else if constexpr (index == 4) {
				aa = (value * 5243) >> 19;
				bb = value - aa * 100;
				std::memcpy(buf, charTable2 + aa * 2, 2);
				std::memcpy(buf + 2, charTable2 + bb * 2, 2);
				return buf + 4;
			} else if constexpr (index == 5) {
				aa	 = (value * 429497) >> 32;
				bbcc = value - aa * 10000;
				bb	 = (bbcc * 5243) >> 19;
				cc	 = bbcc - bb * 100;
				std::memcpy(buf, charTable1 + aa, 1);
				std::memcpy(buf + 1, charTable2 + bb * 2, 2);
				std::memcpy(buf + 3, charTable2 + cc * 2, 2);
				return buf + 5;
			} else if constexpr (index == 6) {
				aa	 = (value * 429497) >> 32;
				bbcc = value - aa * 10000;
				bb	 = (bbcc * 5243) >> 19;
				cc	 = bbcc - bb * 100;
				std::memcpy(buf, charTable2 + aa * 2, 2);
				std::memcpy(buf + 2, charTable2 + bb * 2, 2);
				std::memcpy(buf + 4, charTable2 + cc * 2, 2);
				return buf + 6;
			} else if constexpr (index == 7) {
				aabb = (value * 109951163) >> 40;
				ccdd = value - aabb * 10000;
				aa	 = (aabb * 5243) >> 19;
				cc	 = (ccdd * 5243) >> 19;
				bb	 = aabb - aa * 100;
				dd	 = ccdd - cc * 100;
				std::memcpy(buf + 0, charTable1 + aa, 1);
				std::memcpy(buf + 1, charTable2 + bb * 2, 2);
				std::memcpy(buf + 3, charTable2 + cc * 2, 2);
				std::memcpy(buf + 5, charTable2 + dd * 2, 2);
				return buf + 7;
			} else if constexpr (index == 8) {
				aabb = (value * 109951163) >> 40;
				ccdd = value - aabb * 10000;
				aa	 = (aabb * 5243) >> 19;
				cc	 = (ccdd * 5243) >> 19;
				bb	 = aabb - aa * 100;
				dd	 = ccdd - cc * 100;
				std::memcpy(buf, charTable2 + aa * 2, 2);
				std::memcpy(buf + 2, charTable2 + bb * 2, 2);
				std::memcpy(buf + 4, charTable2 + cc * 2, 2);
				std::memcpy(buf + 6, charTable2 + dd * 2, 2);
				return buf + 8;
			} else if constexpr (index == 9) {
				aabbcc = (value * 3518437209ul) >> 45;
				aa	   = (aabbcc * 429497) >> 32;
				ddee   = value - aabbcc * 10000;
				bbcc   = aabbcc - aa * 10000;
				bb	   = (bbcc * 5243) >> 19;
				dd	   = (ddee * 5243) >> 19;
				cc	   = bbcc - bb * 100;
				ee	   = ddee - dd * 100;
				std::memcpy(buf + 0, charTable1 + aa, 1);
				std::memcpy(buf + 1, charTable2 + bb * 2, 2);
				std::memcpy(buf + 3, charTable2 + cc * 2, 2);
				std::memcpy(buf + 5, charTable2 + dd * 2, 2);
				std::memcpy(buf + 7, charTable2 + ee * 2, 2);
				return buf + 9;
			} else if constexpr (index == 10) {
				aabbcc = (value * 3518437209ul) >> 45;
				aa	   = (aabbcc * 429497) >> 32;
				ddee   = value - aabbcc * 10000;
				bbcc   = aabbcc - aa * 10000;
				bb	   = (bbcc * 5243) >> 19;
				dd	   = (ddee * 5243) >> 19;
				cc	   = bbcc - bb * 100;
				ee	   = ddee - dd * 100;
				std::memcpy(buf + 0, charTable2 + aa * 2, 2);
				std::memcpy(buf + 2, charTable2 + bb * 2, 2);
				std::memcpy(buf + 4, charTable2 + cc * 2, 2);
				std::memcpy(buf + 6, charTable2 + dd * 2, 2);
				std::memcpy(buf + 8, charTable2 + ee * 2, 2);
				return buf + 10;
			} else {
				return buf;
			}
		}
	};

	template<size_t... indices> static constexpr auto generateFunctionPtrs(std::index_sequence<indices...>) noexcept {
		using function_type = decltype(&integer_serializer::impl<0>);
		return array<function_type, sizeof...(indices)>{ { integer_serializer::impl<indices> }... };
	}

	static constexpr auto functionPtrsSerialize{ generateFunctionPtrs(std::make_index_sequence<11>{}) };

	template<typename value_type> JSONIFIER_INLINE static char* toCharsByDigitCountUint32Max(char* buf, value_type value) noexcept {
		uint64_t numDigits{ fastDigitCount(static_cast<uint32_t>(value)) };
		uint32_t aa, bb, cc, dd, aabb, bbcc, ccdd, ee, ddee;
		uint64_t aabbcc;
		return functionPtrsSerialize[numDigits](buf, value);
	}

	template<typename value_type> JSONIFIER_INLINE static char* toCharsByDigitCount1_8(char* buf, value_type value) noexcept {
		uint64_t numDigits{ fastDigitCount(static_cast<uint32_t>(value)) };
		uint64_t aa, bb, cc, dd, aabb, bbcc, ccdd;
		return functionPtrsSerialize[numDigits](buf, value);
	}

	template<typename value_type> JSONIFIER_INLINE static char* toCharsByDigitCount5_8(char* buf, value_type value) noexcept {
		uint32_t numDigits{ fastDigitCount(static_cast<uint32_t>(value)) };
		uint32_t aa, bb, cc, dd, aabb, bbcc, ccdd;
		return functionPtrsSerialize[numDigits](buf, value);
	}

	template<jsonifier::concepts::uns64_t value_type> JSONIFIER_INLINE static char* toChars(char* buf, value_type value) noexcept {
		if (value < std::numeric_limits<uint32_t>::max()) {
			return toCharsByDigitCountUint32Max(buf, value);
		} else {
			static constexpr value_type tenQuadrillion{ 10000000000000000 };
			uint64_t aa, aaNew, aabb, ccdd, cc, bb, dd, tmp;
			if (value >= tenQuadrillion) {
				tmp	  = value / 100000000;
				aa	  = tmp / 10000;
				aaNew = tmp - aa * 10000;
				buf	  = toCharsByDigitCount5_8(buf, aa);
				aa	  = (aaNew * 5243) >> 19;
				bb	  = aaNew - aa * 100;
				std::memcpy(buf, charTable2 + aa * 2, 2);
				std::memcpy(buf + 2, charTable2 + bb * 2, 2);
				aaNew = value - tmp * 100000000;
				aabb  = (aaNew * 109951163) >> 40;
				ccdd  = aaNew - aabb * 10000;
				aa	  = (aabb * 5243) >> 19;
				cc	  = (ccdd * 5243) >> 19;
				bb	  = aabb - aa * 100;
				dd	  = ccdd - cc * 100;
				std::memcpy(buf + 4, charTable2 + aa * 2, 2);
				std::memcpy(buf + 6, charTable2 + bb * 2, 2);
				std::memcpy(buf + 8, charTable2 + cc * 2, 2);
				std::memcpy(buf + 10, charTable2 + dd * 2, 2);
				return buf + 12;
			} else {
				aa	  = value / 100000000;
				buf	  = toCharsByDigitCount1_8(buf, aa);
				aaNew = value - aa * 100000000;
				aabb  = (aaNew * 109951163) >> 40;
				ccdd  = aaNew - aabb * 10000;
				aa	  = (aabb * 5243) >> 19;
				cc	  = (ccdd * 5243) >> 19;
				bb	  = aabb - aa * 100;
				dd	  = ccdd - cc * 100;
				std::memcpy(buf, charTable2 + aa * 2, 2);
				std::memcpy(buf + 2, charTable2 + bb * 2, 2);
				std::memcpy(buf + 4, charTable2 + cc * 2, 2);
				std::memcpy(buf + 6, charTable2 + dd * 2, 2);
				return buf + 8;
				return buf + 8;
			}
		}
	}

	template<jsonifier::concepts::sig64_t value_type> JSONIFIER_INLINE static char* toChars(char* buf, value_type value) noexcept {
		*buf = '-';
		return toChars<uint64_t>(buf + (value < 0), static_cast<uint64_t>(value ^ (value >> 63)) - (value >> 63));
	}

}// namespace jsonifier_internal