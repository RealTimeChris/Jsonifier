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
/// Feb 3, 2023
#pragma once

#include <jsonifier/Utilities/TypeEntities.hpp>
#include <jsonifier/Containers/Array.hpp>
#include <jsonifier/Simd/Fallback.hpp>
#include <jsonifier/Simd/Popcount.hpp>
#include <jsonifier/Simd/AVX.hpp>
#include <jsonifier/Simd/Neon.hpp>
#include <iostream>
#include <concepts>
#include <sstream>
#include <cstring>
#include <cstdint>
#include <bitset>

namespace jsonifier::simd {

#if defined(JSONIFIER_MAC)
	using avx_list = internal::type_list<internal::type_holder<16, internal::avx_type_wrapper<internal::avx_type::m128>, uint64_t, std::numeric_limits<uint64_t>::max()>,
		internal::type_holder<32, internal::avx_type_wrapper<internal::avx_type::m256>, uint32_t, std::numeric_limits<uint32_t>::max()>,
		internal::type_holder<64, internal::avx_type_wrapper<internal::avx_type::m512>, uint64_t, std::numeric_limits<uint64_t>::max()>>;
#else
	using avx_list = internal::type_list<internal::type_holder<16, internal::avx_type_wrapper<internal::avx_type::m128>, uint16_t, std::numeric_limits<uint16_t>::max()>,
		internal::type_holder<32, internal::avx_type_wrapper<internal::avx_type::m256>, uint32_t, std::numeric_limits<uint32_t>::max()>,
		internal::type_holder<64, internal::avx_type_wrapper<internal::avx_type::m512>, uint64_t, std::numeric_limits<uint64_t>::max()>>;
#endif

#if defined(JSONIFIER_MAC)
	using avx_integer_list =
		internal::type_list<internal::type_holder<8, uint64_t, uint64_t, 8>, internal::type_holder<16, internal::avx_type_wrapper<internal::avx_type::m128>, uint64_t, 16>,
			internal::type_holder<32, internal::avx_type_wrapper<internal::avx_type::m256>, uint32_t, 32>,
			internal::type_holder<64, internal::avx_type_wrapper<internal::avx_type::m512>, uint64_t, 64>>;
#else
	using avx_integer_list =
		internal::type_list<internal::type_holder<8, uint64_t, uint64_t, 8>, internal::type_holder<16, internal::avx_type_wrapper<internal::avx_type::m128>, uint16_t, 16>,
			internal::type_holder<32, internal::avx_type_wrapper<internal::avx_type::m256>, uint32_t, 32>,
			internal::type_holder<64, internal::avx_type_wrapper<internal::avx_type::m512>, uint64_t, 64>>;
#endif

	template<concepts::unsigned_t value_type> void printBits(value_type values, const string& valuesTitle);

	template<simd_int_type simd_type> const simd_type& printBits(const simd_type& value, const string& valuesTitle) noexcept;

	JSONIFIER_INLINE static uint64_t prefixXor(uint64_t prevInString) noexcept {
		prevInString ^= prevInString << 1;
		prevInString ^= prevInString << 2;
		prevInString ^= prevInString << 4;
		prevInString ^= prevInString << 8;
		prevInString ^= prevInString << 16;
		prevInString ^= prevInString << 32;
		return prevInString;
	}

	template<typename simd_int_t01> JSONIFIER_INLINE static jsonifier_simd_int_t opClMul(const simd_int_t01& value, int64_t& prevInString) noexcept {
		JSONIFIER_ALIGN(bytesPerStep) uint64_t values[sixtyFourBitsPerStep];
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

	template<typename simd_int_t01, typename simd_int_t02> JSONIFIER_INLINE static jsonifier_simd_int_t opSub(const simd_int_t01& value, const simd_int_t02& other) noexcept {
		JSONIFIER_ALIGN(bytesPerStep) uint64_t values[sixtyFourBitsPerStep * 2];
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

	template<size_t amount, typename simd_int_t01> JSONIFIER_INLINE static jsonifier_simd_int_t opShl(const simd_int_t01& value) noexcept {
		JSONIFIER_ALIGN(bytesPerStep) uint64_t values[sixtyFourBitsPerStep * 2];
		simd::store(value, values);
		static constexpr uint64_t shiftAmount{ 64 - amount };
		values[sixtyFourBitsPerStep]	 = values[0] << amount;
		values[1 + sixtyFourBitsPerStep] = values[1] << amount | values[1 - 1] >> (shiftAmount);
		if constexpr (sixtyFourBitsPerStep > 2) {
			values[2 + sixtyFourBitsPerStep] = values[2] << amount | values[2 - 1] >> (shiftAmount);
			values[3 + sixtyFourBitsPerStep] = values[3] << amount | values[3 - 1] >> (shiftAmount);
		}
		if constexpr (sixtyFourBitsPerStep > 4) {
			values[4 + sixtyFourBitsPerStep] = values[4] << amount | values[4 - 1] >> (shiftAmount);
			values[5 + sixtyFourBitsPerStep] = values[5] << amount | values[5 - 1] >> (shiftAmount);
			values[6 + sixtyFourBitsPerStep] = values[6] << amount | values[6 - 1] >> (shiftAmount);
			values[7 + sixtyFourBitsPerStep] = values[7] << amount | values[7 - 1] >> (shiftAmount);
		}
		return simd::gatherValues<jsonifier_simd_int_t>(values + sixtyFourBitsPerStep);
	}

	template<typename simd_int_t01> JSONIFIER_INLINE static jsonifier_simd_int_t opFollows(const simd_int_t01& value, bool& overflow) noexcept {
		const bool oldOverflow = overflow;
		overflow			   = opGetMSB(value);
		return opSetLSB(opShl<1>(value), oldOverflow);
	}

	struct simd_int_t_holder {
		jsonifier_simd_int_t backslashes;
		jsonifier_simd_int_t whitespace;
		jsonifier_simd_int_t quotes;
		jsonifier_simd_int_t op;
	};

	template<size_t size> JSONIFIER_ALIGN(bytesPerStep)
	inline constexpr internal::array<uint8_t, size> escapeableArray00{ []() constexpr {
		constexpr const uint8_t values[]{ 0x00u, 0x00u, '"', 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, '\\', 0x00u, 0x00u, 0x00u };
		internal::array<uint8_t, size> returnValues{};
		for (uint64_t x = 0; x < size; ++x) {
			returnValues[x] = values[x % 16];
		}
		return returnValues;
	}() };

	template<size_t size> JSONIFIER_ALIGN(bytesPerStep)
	inline constexpr internal::array<uint8_t, size> escapeableArray01{ []() constexpr {
		constexpr const uint8_t values[]{ 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, '\b', 0x00u, 0x00u, 0x00u, 0x0Cu, '\r', 0x00u, 0x00u };
		internal::array<uint8_t, size> returnValues{};
		for (uint64_t x = 0; x < size; ++x) {
			returnValues[x] = values[x % 16];
		}
		return returnValues;
	}() };

	template<size_t size> JSONIFIER_ALIGN(bytesPerStep)
	inline constexpr internal::array<uint8_t, size> whitespaceArray{ []() constexpr {
		constexpr const uint8_t values[]{ 0x20u, 0x64u, 0x64u, 0x64u, 0x11u, 0x64u, 0x71u, 0x02u, 0x64u, '\t', '\n', 0x70u, 0x64u, '\r', 0x64u, 0x64u };
		internal::array<uint8_t, size> returnValues{};
		for (uint64_t x = 0; x < size; ++x) {
			returnValues[x] = values[x % 16];
		}
		return returnValues;
	}() };

	template<size_t size> JSONIFIER_ALIGN(bytesPerStep)
	inline constexpr internal::array<uint8_t, size> opArray{ []() constexpr {
		constexpr const uint8_t values[]{ 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, ':', '{', ',', '}', 0x00u, 0x00u };
		internal::array<uint8_t, size> returnValues{};
		for (uint64_t x = 0; x < size; ++x) {
			returnValues[x] = values[x % 16];
		}
		return returnValues;
	}() };

	JSONIFIER_INLINE static jsonifier_simd_int_t collectStructuralIndices(const jsonifier_simd_int_t* values) noexcept {
		JSONIFIER_ALIGN(bytesPerStep) jsonifier_string_parsing_type valuesNew[stridesPerStep];
		static constexpr auto opArrayPtr{ opArray<bytesPerStep>.data() };
		const jsonifier_simd_int_t simdValues{ gatherValues<jsonifier_simd_int_t>(opArrayPtr) };
		const jsonifier_simd_int_t simdValue{ gatherValue<jsonifier_simd_int_t>(static_cast<uint8_t>(0x20)) };
		valuesNew[0] = opCmpEqBitMask(opShuffle(simdValues, values[0]), opOr(simdValue, values[0]));
		valuesNew[1] = opCmpEqBitMask(opShuffle(simdValues, values[1]), opOr(simdValue, values[1]));
		valuesNew[2] = opCmpEqBitMask(opShuffle(simdValues, values[2]), opOr(simdValue, values[2]));
		valuesNew[3] = opCmpEqBitMask(opShuffle(simdValues, values[3]), opOr(simdValue, values[3]));
		valuesNew[4] = opCmpEqBitMask(opShuffle(simdValues, values[4]), opOr(simdValue, values[4]));
		valuesNew[5] = opCmpEqBitMask(opShuffle(simdValues, values[5]), opOr(simdValue, values[5]));
		valuesNew[6] = opCmpEqBitMask(opShuffle(simdValues, values[6]), opOr(simdValue, values[6]));
		valuesNew[7] = opCmpEqBitMask(opShuffle(simdValues, values[7]), opOr(simdValue, values[7]));
		return gatherValues<jsonifier_simd_int_t>(valuesNew);
	}

	JSONIFIER_INLINE static jsonifier_simd_int_t collectWhitespaceIndices(const jsonifier_simd_int_t* values) noexcept {
		JSONIFIER_ALIGN(bytesPerStep) jsonifier_string_parsing_type valuesNew[stridesPerStep];
		static constexpr auto whiteSpaceArrayPtr{ whitespaceArray<bytesPerStep>.data() };
		const jsonifier_simd_int_t simdValues{ gatherValues<jsonifier_simd_int_t>(whiteSpaceArrayPtr) };
		valuesNew[0] = opCmpEqBitMask(opShuffle(simdValues, values[0]), values[0]);
		valuesNew[1] = opCmpEqBitMask(opShuffle(simdValues, values[1]), values[1]);
		valuesNew[2] = opCmpEqBitMask(opShuffle(simdValues, values[2]), values[2]);
		valuesNew[3] = opCmpEqBitMask(opShuffle(simdValues, values[3]), values[3]);
		valuesNew[4] = opCmpEqBitMask(opShuffle(simdValues, values[4]), values[4]);
		valuesNew[5] = opCmpEqBitMask(opShuffle(simdValues, values[5]), values[5]);
		valuesNew[6] = opCmpEqBitMask(opShuffle(simdValues, values[6]), values[6]);
		valuesNew[7] = opCmpEqBitMask(opShuffle(simdValues, values[7]), values[7]);
		return gatherValues<jsonifier_simd_int_t>(valuesNew);
	}

	template<auto cNew> JSONIFIER_INLINE static jsonifier_simd_int_t collectValues(const jsonifier_simd_int_t* values) noexcept {
		static constexpr auto c{ cNew };
		JSONIFIER_ALIGN(bytesPerStep) jsonifier_string_parsing_type valuesNew[stridesPerStep];
		const jsonifier_simd_int_t simdValue{ gatherValue<jsonifier_simd_int_t>(c) };
		valuesNew[0] = opCmpEqBitMask(simdValue, values[0]);
		valuesNew[1] = opCmpEqBitMask(simdValue, values[1]);
		valuesNew[2] = opCmpEqBitMask(simdValue, values[2]);
		valuesNew[3] = opCmpEqBitMask(simdValue, values[3]);
		valuesNew[4] = opCmpEqBitMask(simdValue, values[4]);
		valuesNew[5] = opCmpEqBitMask(simdValue, values[5]);
		valuesNew[6] = opCmpEqBitMask(simdValue, values[6]);
		valuesNew[7] = opCmpEqBitMask(simdValue, values[7]);
		return gatherValues<jsonifier_simd_int_t>(valuesNew);
	}

	template<bool minified> JSONIFIER_INLINE static simd_int_t_holder collectIndices(const jsonifier_simd_int_t* values) noexcept {
		if constexpr (!minified) {
			return simd_int_t_holder{ .backslashes = collectValues<'\\'>(values),
				.whitespace						   = collectWhitespaceIndices(values),
				.quotes							   = collectValues<'"'>(values),
				.op								   = collectStructuralIndices(values) };
		} else {
			return simd_int_t_holder{ .backslashes = collectValues<'\\'>(values), .whitespace = {}, .quotes = collectValues<'"'>(values), .op = collectStructuralIndices(values) };
		}
	}

}

namespace jsonifier::internal {

	inline constexpr array<bool, 256> whitespaceTable{ []() constexpr {
		array<bool, 256> returnValues{};
		returnValues['\t'] = true;
		returnValues[' ']  = true;
		returnValues['\n'] = true;
		returnValues['\r'] = true;
		return returnValues;
	}() };

}