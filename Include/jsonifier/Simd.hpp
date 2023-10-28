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

#include <jsonifier/StructuralIterator.hpp>
#include <jsonifier/ISADetection.hpp>
#include <jsonifier/StringView.hpp>
#include <cmath>

namespace jsonifier_internal {

	template<uint64_t stepSize> class string_block_reader {
	  public:
		using size_type = uint64_t;

		jsonifier_inline void reset(string_view_ptr stringViewNew, size_type lengthNew) {
			lengthMinusStep = lengthNew < stepSize ? 0 : lengthNew - stepSize;
			instring		= stringViewNew;
			length			= lengthNew;
			index			= 0;
		}

		jsonifier_inline size_type getRemainderHelper02(string_buffer_ptr dest, size_type copyLength, size_type remaining) const {
			std::memset(dest + copyLength, 0x20, stepSize - copyLength);
			return remaining;
		}

		jsonifier_inline size_type getRemainderReal(string_buffer_ptr dest) const {
			size_type remaining	 = length - index;
			size_type copyLength = std::min(remaining, stepSize);

			std::memcpy(dest, instring + index, copyLength);
			return copyLength < stepSize ? getRemainderHelper02(dest, copyLength, remaining) : remaining;
		}

		jsonifier_inline size_type getRemainder() const {
			return length - index;
		}

		jsonifier_inline string_view_ptr fullBlock() {
			return instring + index;
		}

		jsonifier_inline void advanceIndex() {
			index += stepSize;
		}

		jsonifier_inline bool hasFullBlock() const {
			return index < lengthMinusStep;
		}

	  protected:
		size_type lengthMinusStep{};
		string_view_ptr instring{};
		size_type length{};
		size_type index{};
	};

	class simd_string_reader : public simd_base, alloc_wrapper<structural_index> {
	  public:
		using size_type = uint64_t;
		using allocator = alloc_wrapper<structural_index>;

		template<bool refreshString, jsonifier::concepts::string_t string_type> jsonifier_inline void reset(string_type&& stringViewNew) {
			if constexpr (refreshString) {
				stringView	 = reinterpret_cast<const uint8_t*>(stringViewNew.data());
				stringLength = stringViewNew.size();
				structuralIndices.resize(roundUpToMultiple<8>(static_cast<uint64_t>(static_cast<float>(stringLength) * 4.0f / 5.0f)));
				resetInternal();
			} else if (jsonifier::string_view_base<uint8_t>{ stringView, stringLength } != stringViewNew) {
				stringView	 = reinterpret_cast<const uint8_t*>(stringViewNew.data());
				stringLength = stringViewNew.size();
				structuralIndices.resize(roundUpToMultiple<8>(static_cast<uint64_t>(static_cast<float>(stringLength) * 4.0f / 5.0f)));
				resetInternal();
			}
		}

		jsonifier_inline void resetInternal() {
			stringBlockReader.reset(stringView, stringLength);
			overflow	 = false;
			prevIsEscaped	 = false;
			prevInstring = 0;
			stringIndex	 = 0;
			tapeIndex	 = 0;
			generateJsonIndices();
		}

		jsonifier_inline structural_iterator begin() {
			return structural_iterator{ structuralIndices.getIndices() };
		}

		jsonifier_inline structural_iterator end() {
			return structural_iterator{ structuralIndices.getIndices() + tapeIndex };
		}

		jsonifier_inline structural_index* getStructurals() {
			structuralIndices[tapeIndex] = nullptr;
			return structuralIndices.getIndices();
		}

		jsonifier_inline ~simd_string_reader(){};

	  protected:

		static constexpr simd_int_t oddBitsVal{ simdValues<simd_int_t>(0xAA) };
		structural_index_vector structuralIndices{};
		simd_int_t evenSeriesCodesAndOddBits{};
		simd_int_t newPtr[StridesPerStep]{};
		simd_int_t maybeEscapedAndOddBits{};
		simd_int_t escapeAndTerminalCode{};
		simd_int_t register00{};
		simd_int_t register01{};
		simd_int_t register02{};
		simd_int_t register03{};
		simd_int_t register04{};
		simd_int_t currentValues{};
		simd_int_t nextIsEscaped{};
		simd_int_t maybeEscaped{};
		simd_int_t stringTail{};
		simd_int_t escaped{};
		simd_int_t escape{};
		simd_int_t scalar{};
		string_block_reader<BitsPerStep> stringBlockReader{};
		string_view_ptr stringView{};
		size_type stringLength{};
		size_type prevInstring{};
		size_type stringIndex{};
		size_type tapeIndex{};
		bool overflow{};
		bool prevIsEscaped{};

		jsonifier_inline void generateJsonIndices() {
			while (stringBlockReader.hasFullBlock()) {
				generateStructurals(stringBlockReader.fullBlock());
				stringBlockReader.advanceIndex();
			}
			if (stringBlockReader.getRemainder() > 0) {
				uint8_t block[BitsPerStep];
				stringBlockReader.getRemainderReal(block);
				generateStructurals(block);
			}
		}

		jsonifier_inline size_type getTapeLength() {
			return tapeIndex - 1;
		}

		template<size_type index = 0, size_type index02> jsonifier_inline size_type rollValuesIntoTape(size_type currentIndex, size_type newBits) {
			if constexpr (index < 8) {
				structuralIndices[index + (currentIndex * 8) + tapeIndex] = stringView + static_cast<uint32_t>(tzcnt(newBits) + (index02 * 64ull) + stringIndex);
				newBits													  = blsr(newBits);
				return (newBits == 0) ? newBits : rollValuesIntoTape<index + 1, index02>(currentIndex, newBits);
			} else {
				return newBits;
			}
		}

		template<size_type index> jsonifier_inline void collectStringValuesHelper(string_view_ptr valuesNew) {
			newPtr[index] = gatherValuesU<simd_int_t>(valuesNew + (BytesPerStep * index));
		}

		jsonifier_inline void collectStringValues(string_view_ptr valuesNew) {
			collectStringValuesHelper<0>(valuesNew);
			collectStringValuesHelper<1>(valuesNew);
			collectStringValuesHelper<2>(valuesNew);
			collectStringValuesHelper<3>(valuesNew);
			collectStringValuesHelper<4>(valuesNew);
			collectStringValuesHelper<5>(valuesNew);
			collectStringValuesHelper<6>(valuesNew);
			collectStringValuesHelper<7>(valuesNew);
		}

		jsonifier_inline void generateStructurals(string_view_ptr valueNew) {
			collectStringValues(valueNew);
			convertWhitespaceToSimdBase(register00, newPtr);
			convertBackslashesToSimdBase(register01, newPtr);
			convertStructuralsToSimdBase(register02, newPtr);
			convertQuotesToSimdBase(register03, newPtr);
			register04 = collectStructurals();
			addTapeValues();
			stringIndex += BitsPerStep;
		}

		jsonifier_inline void addTapeValues() {
			alignas(BytesPerStep) size_type newBits[StridesPerStep]{};
			store(register04, newBits);
			addTapeValuesHelper<0>(register04, newBits);
		}

		template<size_type index> jsonifier_inline void addTapeValuesHelper(const simd_int_t& register04, size_type* newBits) {
			if constexpr (index < StridesPerStep) {
				if (!newBits[index]) {
					addTapeValuesHelper<index + 1>(register04, newBits);
					return;
				}
				auto cnt			  = popcnt(newBits[index]);
				size_type rollsAmount = static_cast<size_type>(std::ceil(static_cast<float>(cnt) / 8.0f));
				for (size_type y = 0; y < rollsAmount; ++y) {
					newBits[index] = rollValuesIntoTape<0, index>(y, newBits[index]);
				}
				tapeIndex += cnt;
				addTapeValuesHelper<index + 1>(register04, newBits);
			}
		}

		jsonifier_inline simd_int_t collectEscapedCharactersHelper(const simd_int_t& potentialEscape) noexcept {
			maybeEscaped			  = shl<1>(potentialEscape);
			maybeEscapedAndOddBits	  = opOr(maybeEscaped, oddBitsVal);
			evenSeriesCodesAndOddBits = opSub(maybeEscapedAndOddBits, potentialEscape);
			return opXor(evenSeriesCodesAndOddBits, oddBitsVal);
		}

		jsonifier_inline simd_int_t collectNonEmptyEscaped() noexcept {
			nextIsEscaped		  = setLSB(simd_int_t{}, prevIsEscaped);
			escapeAndTerminalCode = collectEscapedCharactersHelper(bitAndNot(register01, nextIsEscaped));
			escaped				  = opXor(escapeAndTerminalCode, opOr(register01, nextIsEscaped));
			escape				  = opAnd(escapeAndTerminalCode, register01);
			prevIsEscaped			  = getMSB(escape);
			return escaped;
		}

		jsonifier_inline simd_int_t collectEmptyEscaped() {
			escaped		= setLSB(simd_int_t{}, prevIsEscaped);
			prevIsEscaped = false;
			return escaped;
		}

		jsonifier_inline simd_int_t collectEscapedCharacters() {
			return opBool(register01) ? collectNonEmptyEscaped() : collectEmptyEscaped();
		}

		jsonifier_inline simd_int_t collectStructurals() {
			currentValues = collectEscapedCharacters();
			printBits(currentValues, "ESCAPED BITS: FOR INDEX: " + std::to_string(stringIndex) + ": ");
			register03		  = bitAndNot(register03, currentValues);
			currentValues = carrylessMultiplication(register03, prevInstring);
			stringTail	  = opXor(currentValues, register03);
			scalar		  = opNot(opOr(register02, register00));
			currentValues = bitAndNot(scalar, register03);
			currentValues = follows(currentValues, overflow);
			currentValues = bitAndNot(scalar, currentValues);
			currentValues = opOr(register02, currentValues);
			return bitAndNot(currentValues, stringTail);
		}
	};

};