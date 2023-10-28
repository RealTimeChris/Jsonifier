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
		simd_int_t registers00to7[StridesPerStep]{};
		simd_int_t register08{};
		simd_int_t register09{};
		simd_int_t register10{};
		simd_int_t register11{};
		simd_int_t register12{};
		simd_int_t register13{};
		simd_int_t register14{};
		simd_int_t register15{};
		simd_int_t register16{};
		bool storedLSB01{};
		bool storedLSB02{};
		string_block_reader<BitsPerStep> stringBlockReader{};
		structural_index_vector* structuralIndices{};
		string_view_ptr stringView{};
		size_type stringLength{};
		size_type stringIndex{};
		uint64_t prevInstring{};
		uint64_t newBits[StridesPerStep]{};
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
			registers00to7[index] = gatherValuesU<simd_int_t>(valuesNew + (BytesPerStep * index));
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
			convertWhitespaceToSimdBase(register08, registers00to7);
			convertBackslashesToSimdBase(register09, registers00to7);
			convertStructuralsToSimdBase(register10, registers00to7);
			convertQuotesToSimdBase(register11, registers00to7);
			register12 = collectStructurals();
			addTapeValues();
			stringIndex += BitsPerStep;
		}

		jsonifier_inline void addTapeValues() {
			alignas(BytesPerStep) size_type newBits[StridesPerStep]{};
			store(register16, newBits);
			addTapeValuesHelper<0>(register16);
		}

		jsonifier::vector<size_type> convertBitsToIndices(simd_int_t& registerNew) {
			jsonifier::vector<size_type> structuralIndices{};
			
			alignas(BytesPerStep) size_type newBits[StridesPerStep]{};
			size_type tapeIndex{};
			store(registerNew, newBits);
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

		template<size_type index = 0> jsonifier_inline void addTapeValuesHelper(simd_int_t& registerNew) {
			if constexpr (index < StridesPerStep) {
				if (!newBits[index]) {
					addTapeValuesHelper<index + 1>(registerNew);
					return;
				}
				auto cnt			  = popcnt(newBits[index]);
				size_type rollsAmount = static_cast<size_type>(std::ceil(static_cast<float>(cnt) / 8.0f));
				for (size_type y = 0; y < rollsAmount; ++y) {
					newBits[index] = rollValuesIntoTape<0, index>(y, newBits[index]);
				}
				tapeIndex += cnt;
				addTapeValuesHelper<index + 1>(registerNew);
			}
		}

		jsonifier_inline simd_int_t collectNonEmptyEscaped() {
			register13						 = setLSB(simd_int_t{}, storedLSB02);
			register14						 = bitAndNot(register09, register13);
			register15						 = shl<1>(register14);
			register15						 = opOr(register15, oddBitsVal);
			register15						 = opSub(register15, register14);
			register16						 = opXor(register15, oddBitsVal);
			storedLSB02						 = getMSB(opAnd(register14, register09));
			return opXor(register16, opOr(register09, register13));
		}

		jsonifier_inline simd_int_t collectEmptyEscaped() {
			simd_int_t escaped{ setLSB(simd_int_t{}, storedLSB02) };
			storedLSB02 = false;
			return escaped;
		}

		jsonifier_inline simd_int_t collectEscapedCharacters() {
			return opBool(register09) ? collectNonEmptyEscaped() : collectEmptyEscaped();
		}

		jsonifier_inline simd_int_t collectStructurals() {
			register13 = collectEscapedCharacters();
			register11 = bitAndNot(register11, register13);
			register13 = carrylessMultiplication(register11, prevInstring);
			register14 = opXor(register13, register11);
			register15 = opNot(opOr(register10, register08));
			register13 = bitAndNot(register15, register11);
			register13 = follows(register13, storedLSB01);
			register13 = bitAndNot(register15, register13);
			register13 = opOr(register10, register13);
			return bitAndNot(register13, register14);
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