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
/// https://github.com/RealTimeChris/jsonifier
/// Feb 3, 2023
#pragma once

#include <jsonifier/Base.hpp>
#include <jsonifier/TypeEntities.hpp>
#include <jsonifier/ISA/ShuffleValues.hpp>
#include <jsonifier/ISA/CompareValues.hpp>
#include <jsonifier/ISA/GatherValues.hpp>
#include <jsonifier/ISA/Fallback.hpp>
#include <jsonifier/ISA/AVX.hpp>
#include <jsonifier/ISA/AVX2.hpp>
#include <jsonifier/ISA/AVX512.hpp>
#include <iostream>
#include <concepts>
#include <sstream>
#include <cstring>
#include <cstdint>
#include <bitset>
#include <array>

namespace simd_internal {

	using avx_list = jsonifier::concepts::type_list<jsonifier::concepts::type_holder<16, simd_int_128, uint16_t, std::numeric_limits<uint16_t>::max()>,
		jsonifier::concepts::type_holder<32, simd_int_256, uint32_t, std::numeric_limits<uint32_t>::max()>,
		jsonifier::concepts::type_holder<64, simd_int_512, uint64_t, std::numeric_limits<uint64_t>::max()>>;

	using avx_integer_list =
		jsonifier::concepts::type_list<jsonifier::concepts::type_holder<8, uint64_t, uint64_t, 8>, jsonifier::concepts::type_holder<16, simd_int_128, uint16_t, 16>,
			jsonifier::concepts::type_holder<32, simd_int_256, uint32_t, 32>, jsonifier::concepts::type_holder<64, simd_int_512, uint64_t, 64>>;

	template<jsonifier::concepts::unsigned_type value_type> void printBits(value_type values, const std::string& valuesTitle);

	template<simd_int_type simd_type> const simd_type& printBits(const simd_type& value, const std::string& valuesTitle) noexcept;

	JSONIFIER_INLINE static uint64_t prefixXor(uint64_t prevInString) {
		prevInString ^= prevInString << 1;
		prevInString ^= prevInString << 2;
		prevInString ^= prevInString << 4;
		prevInString ^= prevInString << 8;
		prevInString ^= prevInString << 16;
		prevInString ^= prevInString << 32;
		return prevInString;
	}

	template<typename simd_int_t01> JSONIFIER_INLINE static simd_int_t opClMul(simd_int_t01&& value, int64_t& prevInString) {
		JSONIFIER_ALIGN uint64_t values[SixtyFourBitsPerStep];
		store(value, values);
		values[0] = prefixXor(values[0]) ^ prevInString;
		prevInString = static_cast<int64_t>(values[0]) >> 63;
		values[1]	 = prefixXor(values[1]) ^ prevInString;
		prevInString = static_cast<int64_t>(values[1]) >> 63;
		if constexpr (SixtyFourBitsPerStep > 2) {
			values[2]	 = prefixXor(values[2]) ^ prevInString;
			prevInString = static_cast<int64_t>(values[2]) >> 63;
			values[3]	 = prefixXor(values[3]) ^ prevInString;
			prevInString = static_cast<int64_t>(values[3]) >> 63;
		}
		if constexpr (SixtyFourBitsPerStep > 4) {
			values[4] = prefixXor(values[4]) ^ prevInString;
			prevInString  = static_cast<int64_t>(values[4]) >> 63;
			values[5] = prefixXor(values[5]) ^ prevInString;
			prevInString  = static_cast<int64_t>(values[5]) >> 63;
			values[6] = prefixXor(values[6]) ^ prevInString;
			prevInString  = static_cast<int64_t>(values[6]) >> 63;
			values[7] = prefixXor(values[7]) ^ prevInString;
			prevInString  = static_cast<int64_t>(values[7]) >> 63; 
		}
		return gatherValues<simd_int_t>(values);
	}

	template<typename simd_int_t01> JSONIFIER_INLINE static simd_int_t opSub(simd_int_t01&& value, simd_int_t01&& other) {
		JSONIFIER_ALIGN uint64_t values[SixtyFourBitsPerStep * 2];
		store(value, values);
		store(other, values + SixtyFourBitsPerStep);
		bool carryInNew{};
		values[SixtyFourBitsPerStep]	 = values[0] - values[SixtyFourBitsPerStep] - static_cast<uint64_t>(carryInNew);
		carryInNew						 = values[SixtyFourBitsPerStep] > values[0];
		values[1 + SixtyFourBitsPerStep] = values[1] - values[1 + SixtyFourBitsPerStep] - static_cast<uint64_t>(carryInNew);
		carryInNew						 = values[1 + SixtyFourBitsPerStep] > values[1];
		if constexpr (SixtyFourBitsPerStep > 2) {
			values[2 + SixtyFourBitsPerStep] = values[2] - values[2 + SixtyFourBitsPerStep] - static_cast<uint64_t>(carryInNew);
			carryInNew						 = values[2 + SixtyFourBitsPerStep] > values[2];
			values[3 + SixtyFourBitsPerStep] = values[3] - values[3 + SixtyFourBitsPerStep] - static_cast<uint64_t>(carryInNew);
			carryInNew						 = values[3 + SixtyFourBitsPerStep] > values[3];
		}
		if constexpr (SixtyFourBitsPerStep > 4) {
			values[4 + SixtyFourBitsPerStep] = values[4] - values[4 + SixtyFourBitsPerStep] - static_cast<uint64_t>(carryInNew);
			carryInNew						 = values[4 + SixtyFourBitsPerStep] > values[4];
			values[5 + SixtyFourBitsPerStep] = values[5] - values[5 + SixtyFourBitsPerStep] - static_cast<uint64_t>(carryInNew);
			carryInNew						 = values[5 + SixtyFourBitsPerStep] > values[5];
			values[6 + SixtyFourBitsPerStep] = values[6] - values[6 + SixtyFourBitsPerStep] - static_cast<uint64_t>(carryInNew);
			carryInNew						 = values[6 + SixtyFourBitsPerStep] > values[6];
			values[7 + SixtyFourBitsPerStep] = values[7] - values[7 + SixtyFourBitsPerStep] - static_cast<uint64_t>(carryInNew);
			carryInNew						 = values[7 + SixtyFourBitsPerStep] > values[7];
		}
		return gatherValues<simd_int_t>(values + SixtyFourBitsPerStep);
	}

	template<uint64_t amount, typename simd_int_t01> JSONIFIER_INLINE static simd_int_t opShl(simd_int_t01&& value) {
		JSONIFIER_ALIGN uint64_t values[SixtyFourBitsPerStep * 2];
		store(value, values);
		static constexpr uint64_t shiftAmount{ 64 - amount };
		values[SixtyFourBitsPerStep]	 = values[0] << amount;
		values[1 + SixtyFourBitsPerStep] = values[1] << amount | values[1 - 1] >> (shiftAmount);
		if constexpr (SixtyFourBitsPerStep > 2) {
			values[2 + SixtyFourBitsPerStep] = values[2] << amount | values[2 - 1] >> (shiftAmount);
			values[3 + SixtyFourBitsPerStep] = values[3] << amount | values[3 - 1] >> (shiftAmount);
		}
		if constexpr (SixtyFourBitsPerStep > 4) {
			values[4 + SixtyFourBitsPerStep] = values[4] << amount | values[4 - 1] >> (shiftAmount);
			values[5 + SixtyFourBitsPerStep] = values[5] << amount | values[5 - 1] >> (shiftAmount);
			values[6 + SixtyFourBitsPerStep] = values[6] << amount | values[6 - 1] >> (shiftAmount);
			values[7 + SixtyFourBitsPerStep] = values[7] << amount | values[7 - 1] >> (shiftAmount);
		}
		return gatherValues<simd_int_t>(values + SixtyFourBitsPerStep);
	}

	template<typename simd_int_t01> JSONIFIER_INLINE static simd_int_t opFollows(simd_int_t01&& value, bool& overflow) {
		bool oldOverflow = overflow;
		overflow		 = opGetMSB(value);
		return opSetLSB(opShl<1>(value), oldOverflow);
	}

}