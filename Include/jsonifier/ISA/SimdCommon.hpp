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

#include <jsonifier/TypeEntities.hpp>
#include <jsonifier/ISA/Fallback.hpp>
#include <jsonifier/ISA/Popcount.hpp>
#include <jsonifier/ISA/AVX.hpp>
#include <jsonifier/ISA/Neon.hpp>
#include <iostream>
#include <concepts>
#include <sstream>
#include <cstring>
#include <cstdint>
#include <bitset>
#include <array>

namespace simd_internal {

	using avx_list = jsonifier_internal::type_list<jsonifier_internal::type_holder<16, jsonifier_simd_int_128, uint16_t, std::numeric_limits<uint16_t>::max()>,
		jsonifier_internal::type_holder<32, jsonifier_simd_int_256, uint32_t, std::numeric_limits<uint32_t>::max()>,
		jsonifier_internal::type_holder<64, jsonifier_simd_int_512, uint64_t, std::numeric_limits<uint64_t>::max()>>;

	using avx_integer_list =
		jsonifier_internal::type_list<jsonifier_internal::type_holder<8, uint64_t, uint64_t, 8>, jsonifier_internal::type_holder<16, jsonifier_simd_int_128, uint16_t, 16>,
			jsonifier_internal::type_holder<32, jsonifier_simd_int_256, uint32_t, 32>, jsonifier_internal::type_holder<64, jsonifier_simd_int_512, uint64_t, 64>>;

	template<jsonifier::concepts::unsigned_type value_type> void printBits(value_type values, const std::string& valuesTitle);

	template<jsonifier::concepts::simd_int_type simd_type> const simd_type& printBits(const simd_type& value, const std::string& valuesTitle) noexcept;

	JSONIFIER_ALWAYS_INLINE static uint64_t prefixXor(uint64_t prevInString) noexcept {
		prevInString ^= prevInString << 1;
		prevInString ^= prevInString << 2;
		prevInString ^= prevInString << 4;
		prevInString ^= prevInString << 8;
		prevInString ^= prevInString << 16;
		prevInString ^= prevInString << 32;
		return prevInString;
	}

	template<typename simd_int_t01> JSONIFIER_ALWAYS_INLINE jsonifier_simd_int_t opClMul(simd_int_t01&& value, int64_t& prevInString) noexcept {
		JSONIFIER_ALIGN uint64_t values[sixtyFourBitsPerStep];
		store(value, values);
		values[0]	 = prefixXor(values[0]) ^ prevInString;
		prevInString = static_cast<int64_t>(values[0]) >> 63;
		values[1]	 = prefixXor(values[1]) ^ prevInString;
		prevInString = static_cast<int64_t>(values[1]) >> 63;
		if constexpr (sixtyFourBitsPerStep > 2) {
			values[2]	 = prefixXor(values[2]) ^ prevInString;
			prevInString = static_cast<int64_t>(values[2]) >> 63;
			values[3]	 = prefixXor(values[3]) ^ prevInString;
			prevInString = static_cast<int64_t>(values[3]) >> 63;
		}
		if constexpr (sixtyFourBitsPerStep > 4) {
			values[4]	 = prefixXor(values[4]) ^ prevInString;
			prevInString = static_cast<int64_t>(values[4]) >> 63;
			values[5]	 = prefixXor(values[5]) ^ prevInString;
			prevInString = static_cast<int64_t>(values[5]) >> 63;
			values[6]	 = prefixXor(values[6]) ^ prevInString;
			prevInString = static_cast<int64_t>(values[6]) >> 63;
			values[7]	 = prefixXor(values[7]) ^ prevInString;
			prevInString = static_cast<int64_t>(values[7]) >> 63;
		}
		return gatherValues<jsonifier_simd_int_t>(values);
	}

	template<typename simd_int_t01> JSONIFIER_ALWAYS_INLINE jsonifier_simd_int_t opSub(simd_int_t01&& value, simd_int_t01&& other) noexcept {
		JSONIFIER_ALIGN uint64_t values[sixtyFourBitsPerStep * 2];
		store(value, values);
		store(other, values + sixtyFourBitsPerStep);
		bool carryInNew{};
		values[sixtyFourBitsPerStep]	 = values[0] - values[sixtyFourBitsPerStep] - static_cast<uint64_t>(carryInNew);
		carryInNew						 = values[sixtyFourBitsPerStep] > values[0];
		values[1 + sixtyFourBitsPerStep] = values[1] - values[1 + sixtyFourBitsPerStep] - static_cast<uint64_t>(carryInNew);
		carryInNew						 = values[1 + sixtyFourBitsPerStep] > values[1];
		if constexpr (sixtyFourBitsPerStep > 2) {
			values[2 + sixtyFourBitsPerStep] = values[2] - values[2 + sixtyFourBitsPerStep] - static_cast<uint64_t>(carryInNew);
			carryInNew						 = values[2 + sixtyFourBitsPerStep] > values[2];
			values[3 + sixtyFourBitsPerStep] = values[3] - values[3 + sixtyFourBitsPerStep] - static_cast<uint64_t>(carryInNew);
			carryInNew						 = values[3 + sixtyFourBitsPerStep] > values[3];
		}
		if constexpr (sixtyFourBitsPerStep > 4) {
			values[4 + sixtyFourBitsPerStep] = values[4] - values[4 + sixtyFourBitsPerStep] - static_cast<uint64_t>(carryInNew);
			carryInNew						 = values[4 + sixtyFourBitsPerStep] > values[4];
			values[5 + sixtyFourBitsPerStep] = values[5] - values[5 + sixtyFourBitsPerStep] - static_cast<uint64_t>(carryInNew);
			carryInNew						 = values[5 + sixtyFourBitsPerStep] > values[5];
			values[6 + sixtyFourBitsPerStep] = values[6] - values[6 + sixtyFourBitsPerStep] - static_cast<uint64_t>(carryInNew);
			carryInNew						 = values[6 + sixtyFourBitsPerStep] > values[6];
			values[7 + sixtyFourBitsPerStep] = values[7] - values[7 + sixtyFourBitsPerStep] - static_cast<uint64_t>(carryInNew);
			carryInNew						 = values[7 + sixtyFourBitsPerStep] > values[7];
		}
		return gatherValues<jsonifier_simd_int_t>(values + sixtyFourBitsPerStep);
	}

#define opShl(amount, value, result) \
	{ \
		JSONIFIER_ALIGN uint64_t values[sixtyFourBitsPerStep * 2]; \
		simd_internal::store(value, values); \
		static constexpr uint64_t shiftAmount{ 64 - amount }; \
		values[sixtyFourBitsPerStep]	 = values[0] << amount; \
		values[1 + sixtyFourBitsPerStep] = values[1] << amount | values[1 - 1] >> (shiftAmount); \
		if constexpr (sixtyFourBitsPerStep > 2) { \
			values[2 + sixtyFourBitsPerStep] = values[2] << amount | values[2 - 1] >> (shiftAmount); \
			values[3 + sixtyFourBitsPerStep] = values[3] << amount | values[3 - 1] >> (shiftAmount); \
		} \
		if constexpr (sixtyFourBitsPerStep > 4) { \
			values[4 + sixtyFourBitsPerStep] = values[4] << amount | values[4 - 1] >> (shiftAmount); \
			values[5 + sixtyFourBitsPerStep] = values[5] << amount | values[5 - 1] >> (shiftAmount); \
			values[6 + sixtyFourBitsPerStep] = values[6] << amount | values[6 - 1] >> (shiftAmount); \
			values[7 + sixtyFourBitsPerStep] = values[7] << amount | values[7 - 1] >> (shiftAmount); \
		} \
		result = simd_internal::gatherValues<jsonifier_simd_int_t>(values + sixtyFourBitsPerStep); \
	}

	template<typename simd_int_t01> JSONIFIER_ALWAYS_INLINE jsonifier_simd_int_t opFollows(simd_int_t01&& value, bool& overflow) noexcept {
		bool oldOverflow = overflow;
		overflow		 = opGetMSB(value);
		jsonifier_simd_int_t result;
		opShl(1, value, result);
		return simd_internal::opSetLSB(result, oldOverflow);
	}

	struct simd_int_t_holder {
		jsonifier_simd_int_t backslashes;
		jsonifier_simd_int_t whitespace;
		jsonifier_simd_int_t quotes;
		jsonifier_simd_int_t op;
	};

	template<size_t size> JSONIFIER_ALIGN constexpr std::array<char, size> escapeableArray00{ [] {
		constexpr const char values[]{ 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, '\t', '\n', 0x00u, '\\', 0x00u, 0x00u, 0x00u };
		std::array<char, size> returnValues{};
		for (uint64_t x = 0; x < size; ++x) {
			returnValues[x] = values[x % 16];
		}
		return returnValues;
	}() };

	template<size_t size> JSONIFIER_ALIGN constexpr std::array<char, size> escapeableArray01{ [] {
		constexpr const char values[]{ 0x00u, 0x00u, '"', 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, '\b', 0x00u, 0x00u, 0x00u, 0x0Cu, '\r', 0x00u, 0x00u };
		std::array<char, size> returnValues{};
		for (uint64_t x = 0; x < size; ++x) {
			returnValues[x] = values[x % 16];
		}
		return returnValues;
	}() };

	template<size_t size> JSONIFIER_ALIGN constexpr std::array<char, size> whitespaceArray{ [] {
		constexpr const char values[]{ 0x20u, 0x64u, 0x64u, 0x64u, 0x11u, 0x64u, 0x71u, 0x02u, 0x64u, '\t', '\n', 0x70u, 0x64u, '\r', 0x64u, 0x64u };
		std::array<char, size> returnValues{};
		for (uint64_t x = 0; x < size; ++x) {
			returnValues[x] = values[x % 16];
		}
		return returnValues;
	}() };

	template<size_t size> JSONIFIER_ALIGN constexpr std::array<char, size> opArray{ [] {
		constexpr const char values[]{ 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, ':', '{', ',', '}', 0x00u, 0x00u };
		std::array<char, size> returnValues{};
		for (uint64_t x = 0; x < size; ++x) {
			returnValues[x] = values[x % 16];
		}
		return returnValues;
	}() };

	JSONIFIER_ALWAYS_INLINE jsonifier_simd_int_t collectStructuralIndices(const jsonifier_simd_int_t* values) noexcept {
		JSONIFIER_ALIGN jsonifier_string_parsing_type valuesNew[stridesPerStep];

		jsonifier_simd_int_t simdValues{ gatherValues<jsonifier_simd_int_t>(opArray<bytesPerStep>.data()) };
		jsonifier_simd_int_t simdValue{ gatherValue<jsonifier_simd_int_t>(0x20) };
		valuesNew[0] = simd_internal::opCmpEq(opShuffle(simdValues, values[0]), opOr(simdValue, values[0]));
		valuesNew[1] = simd_internal::opCmpEq(opShuffle(simdValues, values[1]), opOr(simdValue, values[1]));
		valuesNew[2] = simd_internal::opCmpEq(opShuffle(simdValues, values[2]), opOr(simdValue, values[2]));
		valuesNew[3] = simd_internal::opCmpEq(opShuffle(simdValues, values[3]), opOr(simdValue, values[3]));
		valuesNew[4] = simd_internal::opCmpEq(opShuffle(simdValues, values[4]), opOr(simdValue, values[4]));
		valuesNew[5] = simd_internal::opCmpEq(opShuffle(simdValues, values[5]), opOr(simdValue, values[5]));
		valuesNew[6] = simd_internal::opCmpEq(opShuffle(simdValues, values[6]), opOr(simdValue, values[6]));
		valuesNew[7] = simd_internal::opCmpEq(opShuffle(simdValues, values[7]), opOr(simdValue, values[7]));
		return gatherValues<jsonifier_simd_int_t>(valuesNew);
	}

	JSONIFIER_ALWAYS_INLINE jsonifier_simd_int_t collectWhitespaceIndices(const jsonifier_simd_int_t* values) noexcept {
		JSONIFIER_ALIGN jsonifier_string_parsing_type valuesNew[stridesPerStep];
		jsonifier_simd_int_t simdValues{ gatherValues<jsonifier_simd_int_t>(whitespaceArray<bytesPerStep>.data()) };
		valuesNew[0] = simd_internal::opCmpEq(opShuffle(simdValues, values[0]), values[0]);
		valuesNew[1] = simd_internal::opCmpEq(opShuffle(simdValues, values[1]), values[1]);
		valuesNew[2] = simd_internal::opCmpEq(opShuffle(simdValues, values[2]), values[2]);
		valuesNew[3] = simd_internal::opCmpEq(opShuffle(simdValues, values[3]), values[3]);
		valuesNew[4] = simd_internal::opCmpEq(opShuffle(simdValues, values[4]), values[4]);
		valuesNew[5] = simd_internal::opCmpEq(opShuffle(simdValues, values[5]), values[5]);
		valuesNew[6] = simd_internal::opCmpEq(opShuffle(simdValues, values[6]), values[6]);
		valuesNew[7] = simd_internal::opCmpEq(opShuffle(simdValues, values[7]), values[7]);
		return gatherValues<jsonifier_simd_int_t>(valuesNew);
	}

	template<auto c> JSONIFIER_ALWAYS_INLINE jsonifier_simd_int_t collectValues(const jsonifier_simd_int_t* values) noexcept {
		JSONIFIER_ALIGN jsonifier_string_parsing_type valuesNew[stridesPerStep];
		jsonifier_simd_int_t simdValue{ gatherValue<jsonifier_simd_int_t>(c) };
		valuesNew[0] = simd_internal::opCmpEq(simdValue, values[0]);
		valuesNew[1] = simd_internal::opCmpEq(simdValue, values[1]);
		valuesNew[2] = simd_internal::opCmpEq(simdValue, values[2]);
		valuesNew[3] = simd_internal::opCmpEq(simdValue, values[3]);
		valuesNew[4] = simd_internal::opCmpEq(simdValue, values[4]);
		valuesNew[5] = simd_internal::opCmpEq(simdValue, values[5]);
		valuesNew[6] = simd_internal::opCmpEq(simdValue, values[6]);
		valuesNew[7] = simd_internal::opCmpEq(simdValue, values[7]);
		return gatherValues<jsonifier_simd_int_t>(valuesNew);
	}

	JSONIFIER_ALWAYS_INLINE simd_int_t_holder collectIndices(const jsonifier_simd_int_t* values) noexcept {
		simd_int_t_holder returnValues;
		returnValues.op			 = collectStructuralIndices(values);
		returnValues.quotes		 = collectValues<'"'>(values);
		returnValues.whitespace	 = collectWhitespaceIndices(values);
		returnValues.backslashes = collectValues<'\\'>(values);
		return returnValues;
	}

}

namespace jsonifier_internal {

	constexpr std::array<bool, 256> whitespaceTable{ [] {
		std::array<bool, 256> returnValues{};
		returnValues['\t']	 = true;
		returnValues['\x20'] = true;
		returnValues['\n']	 = true;
		returnValues['\r']	 = true;
		return returnValues;
	}() };

}