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

	JSONIFIER_INLINE jsonifier_simd_int_t collectStructuralsAsSimdBase(const jsonifier_simd_int_t* values, const jsonifier_simd_int_t& simdChars, const jsonifier_simd_int_t& opTable) {
		JSONIFIER_ALIGN jsonifier_string_parsing_type valuesNew[stridesPerStep];
		valuesNew[0] = opCmpEq(opShuffle(opTable, values[0]), opOr(simdChars, values[0]));
		valuesNew[1] = opCmpEq(opShuffle(opTable, values[1]), opOr(simdChars, values[1]));
		valuesNew[2] = opCmpEq(opShuffle(opTable, values[2]), opOr(simdChars, values[2]));
		valuesNew[3] = opCmpEq(opShuffle(opTable, values[3]), opOr(simdChars, values[3]));
		valuesNew[4] = opCmpEq(opShuffle(opTable, values[4]), opOr(simdChars, values[4]));
		valuesNew[5] = opCmpEq(opShuffle(opTable, values[5]), opOr(simdChars, values[5]));
		valuesNew[6] = opCmpEq(opShuffle(opTable, values[6]), opOr(simdChars, values[6]));
		valuesNew[7] = opCmpEq(opShuffle(opTable, values[7]), opOr(simdChars, values[7]));
		return gatherValues<jsonifier_simd_int_t>(valuesNew);
	}

	JSONIFIER_INLINE jsonifier_simd_int_t collectWhitespaceAsSimdBase(const jsonifier_simd_int_t* values, const jsonifier_simd_int_t& whitespaceTable) {
		JSONIFIER_ALIGN jsonifier_string_parsing_type valuesNew[stridesPerStep];
		valuesNew[0] = opCmpEq(opShuffle(whitespaceTable, values[0]), values[0]);
		valuesNew[1] = opCmpEq(opShuffle(whitespaceTable, values[1]), values[1]);
		valuesNew[2] = opCmpEq(opShuffle(whitespaceTable, values[2]), values[2]);
		valuesNew[3] = opCmpEq(opShuffle(whitespaceTable, values[3]), values[3]);
		valuesNew[4] = opCmpEq(opShuffle(whitespaceTable, values[4]), values[4]);
		valuesNew[5] = opCmpEq(opShuffle(whitespaceTable, values[5]), values[5]);
		valuesNew[6] = opCmpEq(opShuffle(whitespaceTable, values[6]), values[6]);
		valuesNew[7] = opCmpEq(opShuffle(whitespaceTable, values[7]), values[7]);
		return gatherValues<jsonifier_simd_int_t>(valuesNew);
	}

	JSONIFIER_INLINE jsonifier_simd_int_t collectBackslashes(const jsonifier_simd_int_t* values, const jsonifier_simd_int_t& simdChars) {
		JSONIFIER_ALIGN jsonifier_string_parsing_type valuesNew[stridesPerStep];
		valuesNew[0] = opCmpEq(simdChars, values[0]);
		valuesNew[1] = opCmpEq(simdChars, values[1]);
		valuesNew[2] = opCmpEq(simdChars, values[2]);
		valuesNew[3] = opCmpEq(simdChars, values[3]);
		valuesNew[4] = opCmpEq(simdChars, values[4]);
		valuesNew[5] = opCmpEq(simdChars, values[5]);
		valuesNew[6] = opCmpEq(simdChars, values[6]);
		valuesNew[7] = opCmpEq(simdChars, values[7]);
		return gatherValues<jsonifier_simd_int_t>(valuesNew);
	}

	JSONIFIER_INLINE jsonifier_simd_int_t collectQuotes(const jsonifier_simd_int_t* values, const jsonifier_simd_int_t& simdChars) {
		JSONIFIER_ALIGN jsonifier_string_parsing_type valuesNew[stridesPerStep];
		valuesNew[0] = opCmpEq(simdChars, values[0]);
		valuesNew[1] = opCmpEq(simdChars, values[1]);
		valuesNew[2] = opCmpEq(simdChars, values[2]);
		valuesNew[3] = opCmpEq(simdChars, values[3]);
		valuesNew[4] = opCmpEq(simdChars, values[4]);
		valuesNew[5] = opCmpEq(simdChars, values[5]);
		valuesNew[6] = opCmpEq(simdChars, values[6]);
		valuesNew[7] = opCmpEq(simdChars, values[7]);
		return gatherValues<jsonifier_simd_int_t>(valuesNew);
	}

	JSONIFIER_INLINE simd_int_t_holder collectIndices(const jsonifier_simd_int_t* values, const jsonifier_simd_int_t& opChars, const jsonifier_simd_int_t& opTable,
		const jsonifier_simd_int_t& quotes, const jsonifier_simd_int_t& backslashes, const jsonifier_simd_int_t& whiteSpaceTable) {
		simd_int_t_holder returnValues;
		returnValues.op			 = collectStructuralsAsSimdBase(values, opChars, opTable);
		returnValues.quotes		 = collectQuotes(values, quotes);
		returnValues.whitespace	 = collectWhitespaceAsSimdBase(values, whiteSpaceTable);
		returnValues.backslashes = collectBackslashes(values, backslashes);
		return returnValues;
	}

}