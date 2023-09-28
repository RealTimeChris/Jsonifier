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

#include <jsonifier/StringView.hpp>
#include <jsonifier/ISADetection.hpp>
#include <cmath>

namespace jsonifier_internal {

	template<uint64_t stepSize> class string_block_reader {
	  public:
		inline void reset(string_view_ptr stringViewNew, uint64_t lengthNew) {
			lengthMinusStep = lengthNew < stepSize ? 0 : lengthNew - stepSize;
			instring		= stringViewNew;
			length			= lengthNew;
			index			= 0;
		}

		inline uint64_t getRemainder(string_buffer_ptr dest) const {
			if (length == index) {
				return 0;
			}
			std::memset(dest, 0x20, stepSize);
			std::memcpy(dest, instring + index, (length - index));
			return length - index;
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
		uint64_t lengthMinusStep{};
		string_view_ptr instring{};
		uint64_t length{};
		uint64_t index{};
	};

	class simd_string_reader {
	  public:
		using size_type = uint64_t;

		inline void reset(jsonifier::string_view_base<uint8_t> stringViewNew) {
			structuralIndices.clear();
			structuralIndices.resize(roundUpToMultipleOfEight(stringViewNew.size()));
			stringBlockReader.reset(stringViewNew.data(), stringViewNew.size());
			stringView = stringViewNew;
			currentValues.reset();
			nextIsEscaped.reset();
			storedLSB01 = false;
			whitespace.reset();
			backslash.reset();
			prevInstring = 0;
			stringIndex	 = 0;
			tapeIndex	 = 0;
			quotes.reset();
			op.reset();
			generateJsonIndices();
		}

		inline void generateJsonIndices()  {
			while (stringBlockReader.hasFullBlock()) {
				generateStructurals(stringBlockReader.fullBlock());
				generateStructurals(stringBlockReader.fullBlock() + StepSize);
				stringBlockReader.advanceIndex();
			}
			uint8_t block[StepSize * 2];
			if (stringBlockReader.getRemainder(block) > 0) {
				generateStructurals(block);
				generateStructurals(block + StepSize);
			}
		}

		inline size_type getstringLength() {
			return stringView.size();
		}

		inline jsonifier::string_view_base<uint8_t> getStringView() {
			return stringView;
		}

		inline structural_index* getStructurals() {
			return structuralIndices.data();
		}

	  protected:
		inline static const simd_base_real oddBits{ makeSimdBase(0xAAAAAAAAAAAAAAAAULL) };
		jsonifier::vector<structural_index> structuralIndices{};
		string_block_reader<StepSize * 2> stringBlockReader{};
		jsonifier::string_view_base<uint8_t> stringView{};
		simd_base_real nextIsEscaped{};
		simd_base_real currentValues{};
		simd_base_real whitespace{};
		simd_base_real backslash{};
		simd_base_real newPtr[8]{};
		uint64_t prevInstring{};
		uint64_t stringIndex{};
		simd_base_real quotes{};
		uint64_t tapeIndex{};
		simd_base_real op{};
		bool storedLSB01{};

		inline uint64_t roundUpToMultipleOfEight(uint64_t num) {
			uint64_t remainder = num % 8;
			if (remainder == 0) {
				return num;
			}
			return num + (8 - remainder);
		}

		inline uint64_t rollValuesIntoTape(uint64_t currentIndex, uint64_t x, uint64_t newBits) {
			structuralIndices[(currentIndex * 8) + tapeIndex]	  = stringView.data() + static_cast<uint32_t>(tzCount(newBits) + (x * 64ULL) + stringIndex);
			newBits												  = blsr(newBits);
			structuralIndices[1 + (currentIndex * 8) + tapeIndex] = stringView.data() + static_cast<uint32_t>(tzCount(newBits) + (x * 64ULL) + stringIndex);
			newBits												  = blsr(newBits);
			structuralIndices[2 + (currentIndex * 8) + tapeIndex] = stringView.data() + static_cast<uint32_t>(tzCount(newBits) + (x * 64ULL) + stringIndex);
			newBits												  = blsr(newBits);
			structuralIndices[3 + (currentIndex * 8) + tapeIndex] = stringView.data() + static_cast<uint32_t>(tzCount(newBits) + (x * 64ULL) + stringIndex);
			newBits												  = blsr(newBits);
			structuralIndices[4 + (currentIndex * 8) + tapeIndex] = stringView.data() + static_cast<uint32_t>(tzCount(newBits) + (x * 64ULL) + stringIndex);
			newBits												  = blsr(newBits);
			structuralIndices[5 + (currentIndex * 8) + tapeIndex] = stringView.data() + static_cast<uint32_t>(tzCount(newBits) + (x * 64ULL) + stringIndex);
			newBits												  = blsr(newBits);
			structuralIndices[6 + (currentIndex * 8) + tapeIndex] = stringView.data() + static_cast<uint32_t>(tzCount(newBits) + (x * 64ULL) + stringIndex);
			newBits												  = blsr(newBits);
			structuralIndices[7 + (currentIndex * 8) + tapeIndex] = stringView.data() + static_cast<uint32_t>(tzCount(newBits) + (x * 64ULL) + stringIndex);
			newBits												  = blsr(newBits);
			return newBits;
		}

		inline void generateStructurals(string_view_ptr valueNew) {
			for (uint64_t x = 0; x < 8; ++x) {
				newPtr[x] = load(valueNew + (BytesPerStep * x));
			}
			whitespace.convertWhitespaceToSimdBase(newPtr);
			backslash.convertBackslashesToSimdBase(newPtr);
			op.convertStructuralsToSimdBase(newPtr);
			quotes.convertQuotesToSimdBase(newPtr);
			addTapeValues(collectStructurals());
			stringIndex += StepSize;
		}

		inline void addTapeValues(simd_base_real structurals) {
			alignas(ALIGNMENT) uint64_t newBits[StridesPerStep]{};
			structurals.store<uint64_t>(newBits);
			for (uint64_t x = 0; x < StridesPerStep; ++x) {
				if (!newBits[x]) {
					continue;
				}
				auto cnt			 = popcnt(newBits[x]);
				uint64_t rollsAmount = static_cast<uint64_t>(std::ceil(static_cast<float>(cnt) / 8.0f));
				for (uint64_t y = 0; y < rollsAmount; ++y) {
					newBits[x] = rollValuesIntoTape(y, x, newBits[x]);
				}
				tapeIndex += cnt;
			}
		}

		inline simd_base_real collectEscapedCharacters() {
			if (backslash.operator bool()) {
				currentValues		   = backslash.bitAndNot(nextIsEscaped).shl<1>();
				currentValues		   = currentValues | oddBits;
				currentValues		   = currentValues - backslash.bitAndNot(nextIsEscaped);
				currentValues		   = currentValues ^ oddBits;
				simd_base_real escaped = currentValues ^ (backslash | nextIsEscaped);
				nextIsEscaped.setLSB((currentValues & backslash).checkMSB());
				return escaped;

			} else {
				return {};
			}
		}

		inline simd_base_real collectStructurals() {
			currentValues			  = collectEscapedCharacters();
			quotes					  = quotes.bitAndNot(currentValues);
			currentValues			  = quotes.carrylessMultiplication(prevInstring);
			simd_base_real stringTail = currentValues ^ quotes;
			simd_base_real scalar	  = ~(op | whitespace);
			currentValues			  = scalar.bitAndNot(quotes);
			currentValues			  = currentValues.follows(storedLSB01);
			currentValues			  = scalar.bitAndNot(currentValues);
			currentValues			  = op | currentValues;
			return currentValues.bitAndNot(stringTail);
		}
	};

}

#if defined(load)
	#undef load
#endif