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
#include <jsonifier/ISA/CompareValues.hpp>
#include <jsonifier/ISA/ShuffleValues.hpp>

namespace simd_internal {

	template<typename return_type, typename value_type, size_t N, size_t... indices>
	constexpr return_type createArray(const value_type (&newArray)[N], std::index_sequence<indices...>) {
		return return_type{ newArray[indices % N]... };
	}

	template<jsonifier::concepts::unsigned_type return_type, typename value_type, size_t N, size_t... indices>
	constexpr return_type createArray(const value_type (&newArray)[N], std::index_sequence<indices...>) {
		return_type returnValues{};
		std::copy(newArray, newArray + 1, &returnValues);
		return returnValues;
	}

	template<typename return_type> constexpr return_type simdFromValue(uint8_t value) {
#if defined(JSONIFIER_LINUX) || (defined(JSONIFIER_WIN) && defined(JSONIFIER_CLANG))
		constexpr uint64_t valueSize{ sizeof(uint64_t) };
		int64_t newArray[16 / sizeof(int64_t)]{};
		for (uint64_t x = 0; x < 16; ++x) {
			newArray[x / sizeof(int64_t)] |= static_cast<int64_t>(value) << ((x % 8) * 8);
		}
#elif defined(JSONIFIER_MAC)
		constexpr uint64_t valueSize{ sizeof(char) };
		unsigned char newArray[16]{};
		std::fill(newArray, newArray + 16, static_cast<char>(value));
#elif defined(JSONIFIER_WIN)
		constexpr uint64_t valueSize{ sizeof(char) };
		char newArray[16]{};
		std::fill(newArray, newArray + 16, static_cast<char>(value));
#endif
		return_type returnValue{ createArray<return_type>(newArray, std::make_index_sequence<sizeof(return_type) / valueSize>{}) };
		return returnValue;
	}

	template<typename return_type> constexpr return_type simdFromTable(const std::array<char, 16> valuesNew01) {
#if defined(JSONIFIER_LINUX) || (defined(JSONIFIER_WIN) && defined(JSONIFIER_CLANG))
		constexpr uint64_t valueSize{ sizeof(uint64_t) };
		int64_t newArray[16 / sizeof(int64_t)]{};
		for (uint64_t x = 0; x < 16; ++x) {
			newArray[x / sizeof(int64_t)] |= static_cast<int64_t>(valuesNew01[x % 16]) << ((x % 8) * 8);
		}
#elif defined(JSONIFIER_MAC)
		constexpr uint64_t valueSize{ sizeof(char) };
		unsigned char newArray[16]{};
		std::copy(valuesNew01.data(), valuesNew01.data() + std::size(newArray), newArray);
#elif defined(JSONIFIER_WIN)
		constexpr uint64_t valueSize{ sizeof(char) };
		char newArray[16]{};
		std::copy(valuesNew01.data(), valuesNew01.data() + std::size(newArray), newArray);
#endif
		return_type returnValue{ createArray<return_type>(newArray, std::make_index_sequence<sizeof(return_type) / valueSize>{}) };
		return returnValue;
	}

	struct simd_int_t_holder {
		simd_int_t backslashes;
		simd_int_t whitespace;
		simd_int_t quotes;
		simd_int_t op;
	};

	constexpr std::array<char, 16> escapeableArray00{ 0x00u, 0x00u, 0x22u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x08u, 0x00u, 0x00u, 0x00u, 0x0Cu, 0x0Du, 0x00u, 0x00u };
	constexpr std::array<char, 16> escapeableArray01{ 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x09u, 0x0Au, 0x00u, 0x5Cu, 0x00u, 0x00u, 0x00u };
	constexpr std::array<char, 16> whitespaceArray{ 0x20u, 0x64u, 0x64u, 0x64u, 0x11u, 0x64u, 0x71u, 0x02u, 0x64u, 0x09u, 0x0Au, 0x70u, 0x64u, 0x0Du, 0x64u, 0x64u };
	constexpr std::array<char, 16> opArray{ 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x3Au, 0x7Bu, 0x2Cu, 0x7Du, 0x00u, 0x00u };
	template<typename simd_type> constexpr simd_type escapeableTable00{ simdFromTable<simd_type>(escapeableArray00) };
	template<typename simd_type> constexpr simd_type escapeableTable01{ simdFromTable<simd_type>(escapeableArray01) };
	template<typename simd_type> constexpr simd_type whitespaceTable{ simdFromTable<simd_type>(whitespaceArray) };
	template<typename simd_type> constexpr simd_type backslashes{ simdFromValue<simd_type>(0x5Cu) };
	template<typename simd_type> constexpr simd_type opTable{ simdFromTable<simd_type>(opArray) };
	template<typename simd_type> constexpr simd_type quotes{ simdFromValue<simd_type>(0x22u) };
	template<typename simd_type> constexpr simd_type chars{ simdFromValue<simd_type>(0x20u) };

	JSONIFIER_INLINE static simd_int_t collectStructuralsAsSimdBase(const simd_int_t* values) {
		JSONIFIER_ALIGN string_parsing_type valuesNew[StridesPerStep];
		valuesNew[0] = opCmpEq(opShuffle(opTable<simd_int_t>, values[0]), opOr(chars<simd_int_t>, values[0]));
		valuesNew[1] = opCmpEq(opShuffle(opTable<simd_int_t>, values[1]), opOr(chars<simd_int_t>, values[1]));
		valuesNew[2] = opCmpEq(opShuffle(opTable<simd_int_t>, values[2]), opOr(chars<simd_int_t>, values[2]));
		valuesNew[3] = opCmpEq(opShuffle(opTable<simd_int_t>, values[3]), opOr(chars<simd_int_t>, values[3]));
		valuesNew[4] = opCmpEq(opShuffle(opTable<simd_int_t>, values[4]), opOr(chars<simd_int_t>, values[4]));
		valuesNew[5] = opCmpEq(opShuffle(opTable<simd_int_t>, values[5]), opOr(chars<simd_int_t>, values[5]));
		valuesNew[6] = opCmpEq(opShuffle(opTable<simd_int_t>, values[6]), opOr(chars<simd_int_t>, values[6]));
		valuesNew[7] = opCmpEq(opShuffle(opTable<simd_int_t>, values[7]), opOr(chars<simd_int_t>, values[7]));
		return gatherValues<simd_int_t>(valuesNew);
	}

	JSONIFIER_INLINE static simd_int_t collectWhitespaceAsSimdBase(const simd_int_t* values) {
		JSONIFIER_ALIGN string_parsing_type valuesNew[StridesPerStep];
		valuesNew[0] = opCmpEq(opShuffle(whitespaceTable<simd_int_t>, values[0]), values[0]);
		valuesNew[1] = opCmpEq(opShuffle(whitespaceTable<simd_int_t>, values[1]), values[1]);
		valuesNew[2] = opCmpEq(opShuffle(whitespaceTable<simd_int_t>, values[2]), values[2]);
		valuesNew[3] = opCmpEq(opShuffle(whitespaceTable<simd_int_t>, values[3]), values[3]);
		valuesNew[4] = opCmpEq(opShuffle(whitespaceTable<simd_int_t>, values[4]), values[4]);
		valuesNew[5] = opCmpEq(opShuffle(whitespaceTable<simd_int_t>, values[5]), values[5]);
		valuesNew[6] = opCmpEq(opShuffle(whitespaceTable<simd_int_t>, values[6]), values[6]);
		valuesNew[7] = opCmpEq(opShuffle(whitespaceTable<simd_int_t>, values[7]), values[7]);
		return gatherValues<simd_int_t>(valuesNew);
	}

	JSONIFIER_INLINE static simd_int_t collectBackslashesAsSimdBase(const simd_int_t* values) {
		JSONIFIER_ALIGN string_parsing_type valuesNew[StridesPerStep];
		valuesNew[0] = opCmpEq(backslashes<simd_int_t>, values[0]);
		valuesNew[1] = opCmpEq(backslashes<simd_int_t>, values[1]);
		valuesNew[2] = opCmpEq(backslashes<simd_int_t>, values[2]);
		valuesNew[3] = opCmpEq(backslashes<simd_int_t>, values[3]);
		valuesNew[4] = opCmpEq(backslashes<simd_int_t>, values[4]);
		valuesNew[5] = opCmpEq(backslashes<simd_int_t>, values[5]);
		valuesNew[6] = opCmpEq(backslashes<simd_int_t>, values[6]);
		valuesNew[7] = opCmpEq(backslashes<simd_int_t>, values[7]);
		return gatherValues<simd_int_t>(valuesNew);
	}

	JSONIFIER_INLINE static simd_int_t collectQuotesAsSimdBase(const simd_int_t* values) {
		JSONIFIER_ALIGN string_parsing_type valuesNew[StridesPerStep];
		valuesNew[0] = opCmpEq(quotes<simd_int_t>, values[0]);
		valuesNew[1] = opCmpEq(quotes<simd_int_t>, values[1]);
		valuesNew[2] = opCmpEq(quotes<simd_int_t>, values[2]);
		valuesNew[3] = opCmpEq(quotes<simd_int_t>, values[3]);
		valuesNew[4] = opCmpEq(quotes<simd_int_t>, values[4]);
		valuesNew[5] = opCmpEq(quotes<simd_int_t>, values[5]);
		valuesNew[6] = opCmpEq(quotes<simd_int_t>, values[6]);
		valuesNew[7] = opCmpEq(quotes<simd_int_t>, values[7]);
		return gatherValues<simd_int_t>(valuesNew);
	}

	template<bool doWeCollectWhitespace> JSONIFIER_INLINE simd_int_t_holder collectIndices(const simd_int_t* values) {
		simd_int_t_holder returnValues;
		returnValues.op		= collectStructuralsAsSimdBase(values);
		returnValues.quotes = collectQuotesAsSimdBase(values);
		if constexpr (doWeCollectWhitespace) {
			returnValues.whitespace = collectWhitespaceAsSimdBase(values);
		}
		returnValues.backslashes = collectBackslashesAsSimdBase(values);
		return returnValues;
	}

}
