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
	INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR a PARTICULAR
	PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE
	FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
	OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
	DEALINGS IN THE SOFTWARE.
*/
/// https://github.com/RealTimeChris/jsonifier
/// Feb 3, 2023
#pragma once

#include <jsonifier/ISADetection.hpp>
#include <jsonifier/StringView.hpp>
#include <cmath>

namespace jsonifier_internal {

	template<uint64_t stepSize> class string_block_reader {
	  public:
		using size_type = uint64_t;

		inline void reset(string_view_ptr stringViewNew, size_type lengthNew) {
			lengthMinusStep = lengthNew < stepSize ? 0 : lengthNew - stepSize;
			instring		= stringViewNew;
			length			= lengthNew;
			index			= 0;
		}

		inline size_type getRemainderHelper02(string_buffer_ptr dest, size_type copyLength, size_type remaining) const {
			std::memset(dest + copyLength, 0x20, stepSize - copyLength);
			return remaining;
		}

		inline size_type getRemainderHelper01(string_buffer_ptr dest) const {
			size_type remaining	 = length - index;
			size_type copyLength = std::min(remaining, stepSize);

			std::memcpy(dest, instring + index, copyLength);
			return copyLength < stepSize ? getRemainderHelper02(dest, copyLength, remaining) : remaining;
		}

		inline size_type getRemainder(string_buffer_ptr dest) const {
			return (length == index) ? 0 : getRemainderHelper01(dest);
		}

		inline string_view_ptr fullBlock() {
			return instring + index;
		}

		inline void advanceIndex() {
			index += stepSize;
		}

		inline bool hasFullBlock() const {
			return index < lengthMinusStep;
		}

	  protected:
		size_type lengthMinusStep{};
		string_view_ptr instring{};
		size_type length{};
		size_type index{};
	};

	class simd_string_reader {
	  public:
		using size_type = uint64_t;

		inline void reset(jsonifier::string_view_base<uint8_t> stringViewNew) {
			structuralIndices.clear();
			structuralIndices.resize(roundUpToMultipleOfEight(stringViewNew.size()));
			stringBlockReader.reset(stringViewNew.data(), stringViewNew.size());
			stringView	 = stringViewNew;
			storedLSB01	 = false;
			prevInstring = 0;
			stringIndex	 = 0;
			tapeIndex	 = 0;
			generateJsonIndices();
		}

		inline void generateJsonIndices() {
			avx_int_t newPtr[StridesPerStep]{};
			avx_int_t whitespace{};
			avx_int_t backslash{};
			avx_int_t quotes{};
			avx_int_t op{};
			while (stringBlockReader.hasFullBlock()) {
				generateStructurals(stringBlockReader.fullBlock(), newPtr, backslash, quotes, whitespace, op);
				stringBlockReader.advanceIndex();
			}
			uint8_t block[BitsPerStep * 2];
			if (stringBlockReader.getRemainder(block) > 0) {
				generateStructurals(block, newPtr, backslash, quotes, whitespace, op);
			}
		}

		inline size_type getTapeLength() {
			return tapeIndex - 1;
		}

		inline structural_index* getStructurals() {
			return structuralIndices.data();
		}

	  protected:
		static constexpr avx_int_t oddBitsVal{ simdValues<avx_int_t>(0xAA) };
		jsonifier::vector<structural_index> structuralIndices{};
		string_block_reader<BitsPerStep> stringBlockReader{};
		jsonifier::string_view_base<uint8_t> stringView{};
		size_type prevInstring{};
		size_type stringIndex{};
		size_type tapeIndex{};
		bool storedLSB01{};
		bool storedLSB02{};

		inline size_type roundUpToMultipleOfEight(size_type num) {
			size_type remainder = num % 8;
			return remainder == 0 ? num : num + (8 - remainder);
		}

		template<size_type index = 0, size_type index02 = 0> inline size_type rollValuesIntoTape(size_type currentIndex, size_type newBits) {
			if constexpr (index < 8) {
				structuralIndices[index + (currentIndex * 8) + tapeIndex] = stringView.data() + static_cast<uint32_t>(tzcnt(newBits) + (index02 * 64ULL) + stringIndex);
				newBits													  = blsr(newBits);
				return (newBits == 0) ? newBits : rollValuesIntoTape<index + 1, index02>(currentIndex, newBits);
			} else {
				return newBits;
			}
		}

		template<size_type index = 0> inline void collectStringValues(string_view_ptr valuesNew, avx_int_t* newPtr) {
			if constexpr (index < StridesPerStep) {
				newPtr[index] = gatherValues<avx_int_t>(valuesNew + (BytesPerStep * index));
				collectStringValues<index + 1>(valuesNew, newPtr);
			}
		}

		inline void generateStructurals(string_view_ptr valueNew, avx_int_t* newPtr, avx_int_t& backslash, avx_int_t& quotes, avx_int_t& whitespace, avx_int_t& op) {
			collectStringValues(valueNew, newPtr);
			convertWhitespaceToSimdBase(whitespace, newPtr);
			convertBackslashesToSimdBase(backslash, newPtr);
			convertStructuralsToSimdBase(op, newPtr);
			convertQuotesToSimdBase(quotes, newPtr);
			addTapeValues(collectStructurals(backslash, quotes, whitespace, op));
			stringIndex += BitsPerStep;
		}

		inline void addTapeValues(const avx_int_t& structurals) {
			alignas(JsonifierAlignment) size_type newBits[StridesPerStep]{};
			store<size_type>(structurals, newBits);
			addTapeValuesHelper(std::move(structurals), newBits);
		}

		template<size_type index = 0> inline void addTapeValuesHelper(const avx_int_t& structurals, size_type* newBits) {
			if constexpr (index < StridesPerStep) {
				if (!newBits[index]) {
					addTapeValuesHelper<index + 1>(structurals, newBits);
					return;
				}
				auto cnt			  = popcnt(newBits[index]);
				size_type rollsAmount = static_cast<size_type>(std::ceil(static_cast<float>(cnt) / 8.0f));
				for (size_type y = 0; y < rollsAmount; ++y) {
					newBits[index] = rollValuesIntoTape<0, index>(y, newBits[index]);
				}
				tapeIndex += cnt;
				addTapeValuesHelper<index + 1>(structurals, newBits);
			}
		}

		inline avx_int_t collectNonEmptyEscaped(const avx_int_t& backslash) {
			avx_int_t nextIsEscaped{};
			setLSB(nextIsEscaped, storedLSB02);
			avx_int_t potentialEscape = bitAndNot(backslash, nextIsEscaped);
			shl<1>(potentialEscape);
			avx_int_t maybeEscaped			= potentialEscape;
			maybeEscaped					= opOr(maybeEscaped, oddBitsVal);
			maybeEscaped					= opSub(maybeEscaped, potentialEscape);
			avx_int_t escapeAndTerminalCode = opXor(maybeEscaped, oddBitsVal);
			escapeAndTerminalCode			= opAnd(escapeAndTerminalCode, backslash);
			storedLSB02						= getMSB(escapeAndTerminalCode);
			return opXor(escapeAndTerminalCode, opOr(backslash, nextIsEscaped));
		}

		inline avx_int_t collectEmptyEscaped() {
			avx_int_t escaped{};
			setLSB(escaped, storedLSB02);
			storedLSB02 = false;
			return escaped;
		}

		inline avx_int_t collectEscapedCharacters(avx_int_t& backslash) {
			return opBool(backslash) ? collectNonEmptyEscaped(backslash) : collectEmptyEscaped();
		}

		inline avx_int_t collectStructurals(avx_int_t& backslash, avx_int_t& quotes, avx_int_t& whitespace, avx_int_t& op) {
			avx_int_t currentValues = collectEscapedCharacters(backslash);
			quotes					= bitAndNot(quotes, currentValues);
			currentValues			= carrylessMultiplication(quotes, prevInstring);
			avx_int_t stringTail	= opXor(currentValues, quotes);
			avx_int_t scalar		= opNot(opOr(op, whitespace));
			currentValues			= bitAndNot(scalar, quotes);
			currentValues			= follows(currentValues, storedLSB01);
			currentValues			= bitAndNot(scalar, currentValues);
			currentValues			= opOr(op, currentValues);
			return bitAndNot(currentValues, stringTail);
		}
	};

}
