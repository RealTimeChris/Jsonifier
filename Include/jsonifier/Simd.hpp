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

		jsonifier_inline void reset(uint64_t stringLength, string_view_ptr stringViewNew) {
			lengthMinusStep = stringLength < stepSize ? 0 : stringLength - stepSize;
			stringView		= stringViewNew;
			length			= stringLength;
			index			= 0;
		}

		jsonifier_inline size_type getRemainderHelper02(string_buffer_ptr dest, size_type copyLength, size_type remaining) const {
			std::memset(dest + copyLength, 0x20, stepSize - copyLength);
			return remaining;
		}

		jsonifier_inline size_type getRemainderReal(string_buffer_ptr dest) const {
			size_type remaining	 = length - index;
			size_type copyLength = std::min(remaining, stepSize);

			std::memcpy(dest, stringView + index, copyLength);
			return copyLength < stepSize ? getRemainderHelper02(dest, copyLength, remaining) : remaining;
		}

		jsonifier_inline int64_t getRemainder() const {
			return static_cast<int64_t>(length - index);
		}

		jsonifier_inline string_view_ptr fullBlock() {
			return stringView + index;
		}

		jsonifier_inline void advanceIndex() {
			index += stepSize;
		}

		jsonifier_inline bool hasFullBlock() const {
			return index < lengthMinusStep;
		}

	  protected:
		string_view_ptr stringView{};
		size_type lengthMinusStep{};
		size_type length{};
		size_type index{};
	};

	class simd_structural_generator : public simd_base {
	  public:
		using size_type = uint64_t;

		static jsonifier_constexpr simd_int_t oddBitsVal{ simdFromValue<simd_int_t>(0xAA) };
		simd_int_t followsPotentialNonquoteScalar{};
		simd_int_t evenSeriesCodesAndOddBits{};
		simd_int_t newPtr[StridesPerStep]{};
		simd_int_t maybeEscapedAndOddBits{};
		simd_int_t escapeAndTerminalCode{};
		simd_int_t currentValues{};
		simd_int_t nextIsEscaped{};
		simd_int_t prevInScalar{};
		simd_int_t maybeEscaped{};
		simd_int_t structurals{};
		simd_int_t whitespace{};
		simd_int_t stringTail{};
		simd_int_t backslash{};
		simd_int_t escaped{};
		simd_int_t quotes{};
		simd_int_t escape{};
		simd_int_t scalar{};
		simd_int_t op{};
		bool storedLSB01{};
		bool storedLSB02{};
		string_block_reader<BitsPerStep> stringBlockReader{};
		structural_index_vector* structuralIndices{};
		string_view_ptr stringView{};
		size_type stringLength{};
		size_type stringIndex{};
		uint64_t prevInstring{};
		uint64_t newBits[4]{};
		size_type tapeIndex{};
		jsonifier_inline simd_structural_generator() noexcept = default;
		jsonifier_inline simd_structural_generator(size_type stringLengthNew, string_view_ptr stringViewNew, structural_index_vector* structuralIndicesNew) {
			structuralIndices = structuralIndicesNew;
			stringLength	  = stringLengthNew;
			stringView		  = stringViewNew;
			stringBlockReader.reset(stringLength, stringView);
			generateJsonIndices();
		}

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
			return tapeIndex;
		}

		template<size_type index = 0, size_type index02 = 0> jsonifier_inline size_type rollValuesIntoTape(size_type currentIndex, size_type newBits) {
			if constexpr (index < 8) {
				(*structuralIndices)[index + (currentIndex * 8) + tapeIndex] = stringView + static_cast<uint32_t>(tzcnt(newBits) + (index02 * 64ULL) + stringIndex);
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
			convertWhitespaceToSimdBase(whitespace, newPtr);
			convertBackslashesToSimdBase(backslash, newPtr);
			convertStructuralsToSimdBase(op, newPtr);
			convertQuotesToSimdBase(quotes, newPtr);
			structurals = collectStructurals();
			addTapeValues();
			stringIndex += BitsPerStep;
		}

		jsonifier_inline void addTapeValues() {
			alignas(BytesPerStep) size_type newBits[StridesPerStep]{};
			store(structurals, newBits);
			addTapeValuesHelper<0>(structurals, newBits);
		}

		jsonifier::vector<size_type> convertBitsToIndices(const simd_int_t& structurals) {
			jsonifier::vector<size_type> structuralIndices{};
			
			alignas(BytesPerStep) size_type newBits[StridesPerStep]{};
			size_type tapeIndex{};
			store(structurals, newBits);
			for (size_type x = 0; x < StridesPerStep; ++x) {
				if (!newBits[x]) {
					continue;
				}
				auto cnt = popcnt(newBits[x]);
				for (size_type y = 0; y < cnt; ++y) {
					auto newIndex = static_cast<uint32_t>(tzcnt(newBits[x]) + (x * 64ull) + stringIndex);
					if (newIndex >= stringLength) {
						return structuralIndices;
					}
					structuralIndices.emplace_back(newIndex - stringIndex);
					newBits[x] = blsr(newBits[x]);
				}
				tapeIndex += cnt;
			}
			structuralIndices.resize(tapeIndex);
			return structuralIndices;
		}

		template<size_type index = 0> jsonifier_inline void addTapeValuesHelper(simd_int_t& structurals, size_type* newBits) {
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

		jsonifier_inline simd_int_t collectNonEmptyEscaped(simd_int_t& backslash) {
			simd_int_t nextIsEscaped		 = setLSB(simd_int_t{}, storedLSB02);
			simd_int_t potentialEscape		 = bitAndNot(backslash, nextIsEscaped);
			simd_int_t maybeEscaped			 = shl<1>(potentialEscape);
			maybeEscaped					 = opOr(maybeEscaped, oddBitsVal);
			maybeEscaped					 = opSub(maybeEscaped, potentialEscape);
			simd_int_t escapeAndTerminalCode = opXor(maybeEscaped, oddBitsVal);
			storedLSB02						 = getMSB(opAnd(escapeAndTerminalCode, backslash));
			return opXor(escapeAndTerminalCode, opOr(backslash, nextIsEscaped));
		}

		jsonifier_inline simd_int_t collectEmptyEscaped() {
			simd_int_t escaped{ setLSB(simd_int_t{}, storedLSB02) };
			storedLSB02 = false;
			return escaped;
		}

		jsonifier_inline simd_int_t collectEscapedCharacters(simd_int_t& backslash) {
			return opBool(backslash) ? collectNonEmptyEscaped(backslash) : collectEmptyEscaped();
		}

		jsonifier_inline simd_int_t collectStructurals() {
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

	class simd_string_reader {
	  public:
		using size_type = uint64_t;

		template<bool refreshString, jsonifier::concepts::string_t string_type> jsonifier_inline void reset(string_type&& stringViewNew) {
			auto newSize   = stringViewNew.size();
			auto newerSize = roundUpToMultiple<8>(static_cast<size_type>(static_cast<float>(newSize) * 3.0f / 5.0f));
			if jsonifier_constexpr (refreshString) {
				stringView	 = reinterpret_cast<string_view_ptr>(stringViewNew.data());
				stringLength = newSize;
				if (structuralIndices.size() < newerSize) {
					structuralIndices.resize(newerSize);
				}
				resetInternal();
			} else if (structuralIndices.size() != newerSize) {
				stringView	 = reinterpret_cast<string_view_ptr>(stringViewNew.data());
				stringLength = newSize;
				if (structuralIndices.size() < newerSize) {
					structuralIndices.resize(newerSize);
				}
				resetInternal();
			}
		}

		jsonifier_inline structural_iterator begin() {
			return structural_iterator{ structuralIndices.data() };
		}

		jsonifier_inline ~simd_string_reader(){};

	  protected:
		structural_index_vector structuralIndices{};
		string_view_ptr stringView{};
		size_type stringLength{};

		jsonifier_inline void resetInternal() {
			simd_structural_generator structuralGenerator{ stringLength, stringView, &structuralIndices };
			auto tapeIndex				 = structuralGenerator.getTapeLength();
			structuralIndices[tapeIndex] = nullptr;
		}
	};

};