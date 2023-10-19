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

	class simd_string_reader : public simd_base {
	  public:
		using size_type = uint64_t;

		inline void reset(string_view_ptr stringViewNew, size_type sizeNew) {
			auto newSize = roundUpToMultiple<8>(sizeNew * 5 / 6);
			if (newSize > structuralIndices.size()) {
				structuralIndices.resize(newSize);
			}
			stringBlockReader.reset(stringViewNew, sizeNew);
			stringView	 = stringViewNew;
			storedLSB01	 = false;
			prevInstring = 0;
			stringIndex	 = 0;
			tapeIndex	 = 0;
			generateJsonIndices();
		}

		inline void generateJsonIndices() {
			simd_int_t newPtr[StridesPerStep]{};
			simd_int_t whitespace{};
			simd_int_t backslash{};
			simd_int_t quotes{};
			simd_int_t op{};
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
			structuralIndices.at(tapeIndex) = nullptr;
			return structuralIndices.data();
		}

	  protected:
		static constexpr simd_int_t oddBitsVal{ simdValues<simd_int_t>(0xAA) };
		jsonifier::vector<structural_index> structuralIndices{};
		string_block_reader<BitsPerStep> stringBlockReader{};
		string_view_ptr stringView{};
		size_type prevInstring{};
		size_type stringIndex{};
		size_type tapeIndex{};
		bool storedLSB01{};
		bool storedLSB02{};

		template<size_type index = 0, size_type index02> inline size_type rollValuesIntoTape(size_type currentIndex, size_type newBits) {
			if constexpr (index < 8) {
				structuralIndices[index + (currentIndex * 8) + tapeIndex] = stringView + static_cast<uint32_t>(tzcnt(newBits) + (index02 * 64ULL) + stringIndex);
				newBits													  = blsr(newBits);
				return (newBits == 0) ? newBits : rollValuesIntoTape<index + 1, index02>(currentIndex, newBits);
			} else {
				return newBits;
			}
		}

		template<size_type index> inline void collectStringValuesHelper(string_view_ptr valuesNew, simd_int_t* newPtr) {
			newPtr[index] = gatherValues<simd_int_t>(valuesNew + (BytesPerStep * index));
		}

		inline void collectStringValues(string_view_ptr valuesNew, simd_int_t* newPtr) {
			collectStringValuesHelper<0>(valuesNew, newPtr);
			collectStringValuesHelper<1>(valuesNew, newPtr);
			collectStringValuesHelper<2>(valuesNew, newPtr);
			collectStringValuesHelper<3>(valuesNew, newPtr);
			collectStringValuesHelper<4>(valuesNew, newPtr);
			collectStringValuesHelper<5>(valuesNew, newPtr);
			collectStringValuesHelper<6>(valuesNew, newPtr);
			collectStringValuesHelper<7>(valuesNew, newPtr);
		}

		inline void generateStructurals(string_view_ptr valueNew, simd_int_t* newPtr, simd_int_t& backslash, simd_int_t& quotes, simd_int_t& whitespace, simd_int_t& op) {
			collectStringValues(valueNew, newPtr);
			convertWhitespaceToSimdBase(whitespace, newPtr);
			convertBackslashesToSimdBase(backslash, newPtr);
			convertStructuralsToSimdBase(op, newPtr);
			convertQuotesToSimdBase(quotes, newPtr);
			addTapeValues(collectStructurals(backslash, quotes, whitespace, op));
			stringIndex += BitsPerStep;
		}

		inline void addTapeValues(const simd_int_t& structurals) {
			alignas(JsonifierAlignment) size_type newBits[StridesPerStep]{};
			store(structurals, newBits);
			addTapeValuesHelper<0>(structurals, newBits);
		}

		template<size_type index> inline void addTapeValuesHelper(const simd_int_t& structurals, size_type* newBits) {
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

		inline simd_int_t collectEscapedCharactersHelper(const simd_int_t& potentialEscape) noexcept {
			simd_int_t maybeEscaped				 = shl<1>(potentialEscape);
			simd_int_t maybeEscapedAndOddBits	 = opOr(maybeEscaped, oddBitsVal);
			simd_int_t evenSeriesCodesAndOddBits = opSub(maybeEscapedAndOddBits, potentialEscape);
			return opXor(evenSeriesCodesAndOddBits, oddBitsVal);
		}

		inline simd_int_t collectNonEmptyEscaped(const simd_int_t& backslash) noexcept {
			simd_int_t nextIsEscaped{ setLSB(simd_int_t{}, storedLSB02) };
			simd_int_t escapeAndTerminalCode = collectEscapedCharactersHelper(bitAndNot(backslash, nextIsEscaped));
			simd_int_t escaped				 = opXor(escapeAndTerminalCode, opOr(backslash, nextIsEscaped));
			simd_int_t escape				 = opAnd(escapeAndTerminalCode, backslash);
			storedLSB02						 = getMSB(escape);
			return escaped;
		}

		inline simd_int_t collectEmptyEscaped() {
			simd_int_t escaped{ setLSB(simd_int_t{}, storedLSB02) };
			storedLSB02 = false;
			return escaped;
		}

		inline simd_int_t collectEscapedCharacters(simd_int_t& backslash) {
			return opBool(backslash) ? collectNonEmptyEscaped(backslash) : collectEmptyEscaped();
		}

		inline simd_int_t collectStructurals(simd_int_t& backslash, simd_int_t& quotes, simd_int_t& whitespace, simd_int_t& op) {
			simd_int_t currentValues = collectEscapedCharacters(backslash);
			quotes					 = bitAndNot(quotes, currentValues);
			currentValues			 = carrylessMultiplication(quotes, prevInstring);
			simd_int_t stringTail	 = opXor(currentValues, quotes);
			simd_int_t scalar		 = opNot(opOr(op, whitespace));
			currentValues			 = bitAndNot(scalar, quotes);
			currentValues			 = follows(currentValues, storedLSB01);
			currentValues			 = bitAndNot(scalar, currentValues);
			currentValues			 = opOr(op, currentValues);
			return bitAndNot(currentValues, stringTail);
		}
	};

};