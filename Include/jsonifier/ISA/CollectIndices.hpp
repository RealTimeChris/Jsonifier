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

	JSONIFIER_ALWAYS_INLINE jsonifier_simd_int_t collectStructuralsAsSimdBase(const jsonifier_simd_int_t* values) {
		JSONIFIER_ALIGN jsonifier_string_parsing_type valuesNew[stridesPerStep];
		const jsonifier_simd_int_t simdValue{ simd_internal::gatherValue<jsonifier_simd_int_t>(0x20) };
		const jsonifier_simd_int_t simdValues{ simd_internal::gatherValues<jsonifier_simd_int_t>(simd_internal::opArray<bytesPerStep>.data()) };
		valuesNew[0] = opCmpEq(opShuffle(simdValues, values[0]), opOr(simdValue, values[0]));
		valuesNew[1] = opCmpEq(opShuffle(simdValues, values[1]), opOr(simdValue, values[1]));
		valuesNew[2] = opCmpEq(opShuffle(simdValues, values[2]), opOr(simdValue, values[2]));
		valuesNew[3] = opCmpEq(opShuffle(simdValues, values[3]), opOr(simdValue, values[3]));
		valuesNew[4] = opCmpEq(opShuffle(simdValues, values[4]), opOr(simdValue, values[4]));
		valuesNew[5] = opCmpEq(opShuffle(simdValues, values[5]), opOr(simdValue, values[5]));
		valuesNew[6] = opCmpEq(opShuffle(simdValues, values[6]), opOr(simdValue, values[6]));
		valuesNew[7] = opCmpEq(opShuffle(simdValues, values[7]), opOr(simdValue, values[7]));
		return gatherValues<jsonifier_simd_int_t>(valuesNew);
	}

	JSONIFIER_ALWAYS_INLINE jsonifier_simd_int_t collectWhitespaceAsSimdBase(const jsonifier_simd_int_t* values) {
		JSONIFIER_ALIGN jsonifier_string_parsing_type valuesNew[stridesPerStep];
		const jsonifier_simd_int_t simdValues{ simd_internal::gatherValues<jsonifier_simd_int_t>(simd_internal::whitespaceArray<bytesPerStep>.data()) };
		valuesNew[0] = opCmpEq(opShuffle(simdValues, values[0]), values[0]);
		valuesNew[1] = opCmpEq(opShuffle(simdValues, values[1]), values[1]);
		valuesNew[2] = opCmpEq(opShuffle(simdValues, values[2]), values[2]);
		valuesNew[3] = opCmpEq(opShuffle(simdValues, values[3]), values[3]);
		valuesNew[4] = opCmpEq(opShuffle(simdValues, values[4]), values[4]);
		valuesNew[5] = opCmpEq(opShuffle(simdValues, values[5]), values[5]);
		valuesNew[6] = opCmpEq(opShuffle(simdValues, values[6]), values[6]);
		valuesNew[7] = opCmpEq(opShuffle(simdValues, values[7]), values[7]);
		return gatherValues<jsonifier_simd_int_t>(valuesNew);
	}

	template<auto c> JSONIFIER_ALWAYS_INLINE jsonifier_simd_int_t collectValues(const jsonifier_simd_int_t* values) {
		JSONIFIER_ALIGN jsonifier_string_parsing_type valuesNew[stridesPerStep];
		const jsonifier_simd_int_t simdValue{ simd_internal::gatherValue<jsonifier_simd_int_t>(c) };
		valuesNew[0] = opCmpEq(simdValue, values[0]);
		valuesNew[1] = opCmpEq(simdValue, values[1]);
		valuesNew[2] = opCmpEq(simdValue, values[2]);
		valuesNew[3] = opCmpEq(simdValue, values[3]);
		valuesNew[4] = opCmpEq(simdValue, values[4]);
		valuesNew[5] = opCmpEq(simdValue, values[5]);
		valuesNew[6] = opCmpEq(simdValue, values[6]);
		valuesNew[7] = opCmpEq(simdValue, values[7]);
		return gatherValues<jsonifier_simd_int_t>(valuesNew);
	}

	JSONIFIER_ALWAYS_INLINE simd_int_t_holder collectIndices(const jsonifier_simd_int_t* values) {
		simd_int_t_holder returnValues;
		returnValues.op			 = collectStructuralsAsSimdBase(values);
		returnValues.quotes		 = collectValues<'"'>(values);
		returnValues.whitespace	 = collectWhitespaceAsSimdBase(values);
		returnValues.backslashes = collectValues<'\\'>(values);
		return returnValues;
	}

}