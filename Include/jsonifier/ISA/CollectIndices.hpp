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

#include <jsonifier/ISA/CompareValues.hpp>
#include <jsonifier/ISA/GatherValues.hpp>
#include <jsonifier/ISA/ShuffleValues.hpp>

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

namespace simd_internal {

	using avx_list = jsonifier::concepts::type_list<jsonifier::concepts::type_holder<16, simd_int_128, uint16_t, std::numeric_limits<uint16_t>::max()>,
		jsonifier::concepts::type_holder<32, simd_int_256, uint32_t, std::numeric_limits<uint32_t>::max()>,
		jsonifier::concepts::type_holder<64, simd_int_512, uint64_t, std::numeric_limits<uint64_t>::max()>>;

	using avx_integer_list =
		jsonifier::concepts::type_list<jsonifier::concepts::type_holder<8, uint64_t, uint64_t, 8>, jsonifier::concepts::type_holder<16, simd_int_128, uint16_t, 16>,
			jsonifier::concepts::type_holder<32, simd_int_256, uint32_t, 32>, jsonifier::concepts::type_holder<64, simd_int_512, uint64_t, 64>>;

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
		uint8_t newArray[16]{};
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
		uint8_t newArray[16]{};
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

	constexpr std::array<char, 16> escapeableArray00{ 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, '\t', '\n', 0x00u, '\\', 0x00u, 0x00u, 0x00u };
	constexpr std::array<char, 16> escapeableArray01{ 0x00u, 0x00u, '"', 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, '\b', 0x00u, 0x00u, 0x00u, 0x0Cu, '\r', 0x00u, 0x00u };
	constexpr std::array<char, 16> whitespaceArray{ 0x20u, 0x64u, 0x64u, 0x64u, 0x11u, 0x64u, 0x71u, 0x02u, 0x64u, '\t', '\n', 0x70u, 0x64u, '\r', 0x64u, 0x64u };
	constexpr std::array<char, 16> opArray{ 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, ':', '{', ',', '}', 0x00u, 0x00u };
	template<typename simd_type> constexpr simd_type escapeableTable00{ simdFromTable<simd_type>(escapeableArray00) };
	template<typename simd_type> constexpr simd_type escapeableTable01{ simdFromTable<simd_type>(escapeableArray01) };
	template<typename simd_type> constexpr simd_type whitespaceTable{ simdFromTable<simd_type>(whitespaceArray) };
	template<char c, typename simd_type> constexpr simd_type simdChars{ simdFromValue<simd_type>(c) };
	template<typename simd_type> constexpr simd_type opTable{ simdFromTable<simd_type>(opArray) };

	JSONIFIER_INLINE simd_int_t collectStructuralsAsSimdBase(const simd_int_t* values) {
		JSONIFIER_ALIGN string_parsing_type valuesNew[stridesPerStep];
		valuesNew[0] = opCmpEq(opShuffle(opTable<simd_int_t>, values[0]), opOr(simdChars<0x20u, simd_int_t>, values[0]));
		valuesNew[1] = opCmpEq(opShuffle(opTable<simd_int_t>, values[1]), opOr(simdChars<0x20u, simd_int_t>, values[1]));
		valuesNew[2] = opCmpEq(opShuffle(opTable<simd_int_t>, values[2]), opOr(simdChars<0x20u, simd_int_t>, values[2]));
		valuesNew[3] = opCmpEq(opShuffle(opTable<simd_int_t>, values[3]), opOr(simdChars<0x20u, simd_int_t>, values[3]));
		valuesNew[4] = opCmpEq(opShuffle(opTable<simd_int_t>, values[4]), opOr(simdChars<0x20u, simd_int_t>, values[4]));
		valuesNew[5] = opCmpEq(opShuffle(opTable<simd_int_t>, values[5]), opOr(simdChars<0x20u, simd_int_t>, values[5]));
		valuesNew[6] = opCmpEq(opShuffle(opTable<simd_int_t>, values[6]), opOr(simdChars<0x20u, simd_int_t>, values[6]));
		valuesNew[7] = opCmpEq(opShuffle(opTable<simd_int_t>, values[7]), opOr(simdChars<0x20u, simd_int_t>, values[7]));
		return gatherValues<simd_int_t>(valuesNew);
	}

	JSONIFIER_INLINE simd_int_t collectWhitespaceAsSimdBase(const simd_int_t* values) {
		JSONIFIER_ALIGN string_parsing_type valuesNew[stridesPerStep];
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

	template<char c> JSONIFIER_INLINE simd_int_t collectValuesAsSimdBase(const simd_int_t* values) {
		JSONIFIER_ALIGN string_parsing_type valuesNew[stridesPerStep];
		valuesNew[0] = opCmpEq(simdChars<c, simd_int_t>, values[0]);
		valuesNew[1] = opCmpEq(simdChars<c, simd_int_t>, values[1]);
		valuesNew[2] = opCmpEq(simdChars<c, simd_int_t>, values[2]);
		valuesNew[3] = opCmpEq(simdChars<c, simd_int_t>, values[3]);
		valuesNew[4] = opCmpEq(simdChars<c, simd_int_t>, values[4]);
		valuesNew[5] = opCmpEq(simdChars<c, simd_int_t>, values[5]);
		valuesNew[6] = opCmpEq(simdChars<c, simd_int_t>, values[6]);
		valuesNew[7] = opCmpEq(simdChars<c, simd_int_t>, values[7]);
		return gatherValues<simd_int_t>(valuesNew);
	}

	JSONIFIER_INLINE simd_int_t_holder collectIndices(const simd_int_t* values) {
		simd_int_t_holder returnValues;
		returnValues.op			 = collectStructuralsAsSimdBase(values);
		returnValues.quotes		 = collectValuesAsSimdBase<'"'>(values);
		returnValues.whitespace	 = collectWhitespaceAsSimdBase(values);
		returnValues.backslashes = collectValuesAsSimdBase<'\\'>(values);
		return returnValues;
	}

}