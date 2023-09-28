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
		using size_type = uint64_t;

		inline void reset(string_view_ptr stringViewNew, size_type lengthNew) {
			lengthMinusStep = lengthNew < stepSize ? 0 : lengthNew - stepSize;
			instring		= stringViewNew;
			length			= lengthNew;
			index			= 0;
		}

		inline size_type getRemainder(string_buffer_ptr dest) const {
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
			simd_base<StepSize> newPtr[StridesPerStep]{};
			simd_base<StepSize> nextIsEscaped{};
			simd_base<StepSize> currentValues{};
			simd_base<StepSize> whitespace{};
			simd_base<StepSize> backslash{};
			simd_base<StepSize> quotes{};
			simd_base<StepSize> op{};
			while (stringBlockReader.hasFullBlock()) {
				generateStructurals(stringBlockReader.fullBlock(), newPtr, currentValues, whitespace, op, quotes, nextIsEscaped, backslash);
				generateStructurals(stringBlockReader.fullBlock() + StepSize, newPtr, currentValues, whitespace, op, quotes, nextIsEscaped, backslash);
				stringBlockReader.advanceIndex();
			}
			uint8_t block[StepSize * 2];
			if (stringBlockReader.getRemainder(block) > 0) {
				generateStructurals(block, newPtr, currentValues, whitespace, op, quotes, nextIsEscaped, backslash);
				generateStructurals(block + StepSize, newPtr, currentValues, whitespace, op, quotes, nextIsEscaped, backslash);
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
		inline static const simd_base<StepSize> oddBits{ makeSimdBase(0xAAAAAAAAAAAAAAAAULL) };
		jsonifier::vector<structural_index> structuralIndices{};
		string_block_reader<StepSize * 2> stringBlockReader{};
		jsonifier::string_view_base<uint8_t> stringView{};
		size_type prevInstring{};
		size_type stringIndex{};
		size_type tapeIndex{};
		bool storedLSB01{};

		inline size_type roundUpToMultipleOfEight(size_type num) {
			size_type remainder = num % 8;
			if (remainder == 0) {
				return num;
			}
			return num + (8 - remainder);
		}

		template<size_type index = 0, size_type index02 = 0> inline size_type rollValuesIntoTape(size_type currentIndex, size_type newBits) {
			if constexpr (index < 8) {
				structuralIndices[index + (currentIndex * 8) + tapeIndex] = stringView.data() + static_cast<uint32_t>(tzCount(newBits) + (index02 * 64ULL) + stringIndex);
				newBits													  = blsr(newBits);
				return rollValuesIntoTape<index + 1, index02>(currentIndex, newBits);
			} else {
				return newBits;
			}
		}

		template<size_type index = 0> inline void collectStringValues(string_view_ptr valuesNew, simd_base<StepSize>* newPtr) {
			if constexpr (index < StridesPerStep) {
				newPtr[index] = load(valuesNew + (BytesPerStep * index));
				collectStringValues<index + 1>(valuesNew, newPtr);
			}
		}

		inline void generateStructurals(string_view_ptr valueNew, simd_base<StepSize>* newPtr, simd_base<StepSize>& currentValues, simd_base<StepSize>& whitespace,
			simd_base<StepSize>& op, simd_base<StepSize>& quotes, simd_base<StepSize>& nextIsEscaped, simd_base<StepSize>& backslash) {
			collectStringValues(valueNew, newPtr);
			whitespace.convertWhitespaceToSimdBase(newPtr);
			backslash.convertBackslashesToSimdBase(newPtr);
			op.convertStructuralsToSimdBase(newPtr);
			quotes.convertQuotesToSimdBase(newPtr);
			addTapeValues(collectStructurals(currentValues, quotes, whitespace, op, backslash, nextIsEscaped));
			stringIndex += StepSize;
		}

		template<size_type index = 0> inline void addTapeValues(simd_base<StepSize>&& structurals) {
			alignas(JSONIFIER_ALIGNMENT) size_type newBits[StridesPerStep]{};
			structurals.store<size_type>(newBits);
			if constexpr (index < StridesPerStep) {
				if (!newBits[index]) {
					addTapeValues<index + 1>(std::forward<simd_base<StepSize>>(structurals));
					return;
				}
				auto cnt			  = popcnt(newBits[index]);
				size_type rollsAmount = static_cast<size_type>(std::ceil(static_cast<float>(cnt) / 8.0f));
				for (size_type y = 0; y < rollsAmount; ++y) {
					newBits[index] = rollValuesIntoTape<0, index>(y, newBits[index]);
				}
				tapeIndex += cnt;
				addTapeValues<index + 1>(std::forward<simd_base<StepSize>>(structurals));
			}
		}

		inline simd_base<StepSize> collectEscapedCharacters(simd_base<StepSize>& currentValues, simd_base<StepSize>& backslash, simd_base<StepSize>& nextIsEscaped) {
			if (backslash.operator bool()) {
				currentValues				= backslash.bitAndNot(nextIsEscaped).shl<1>();
				currentValues				= currentValues | oddBits;
				currentValues				= currentValues - backslash.bitAndNot(nextIsEscaped);
				currentValues				= currentValues ^ oddBits;
				simd_base<StepSize> escaped = currentValues ^ (backslash | nextIsEscaped);
				nextIsEscaped.setLSB((currentValues & backslash).checkMSB());
				return escaped;
			} else {
				return {};
			}
		}

		inline simd_base<StepSize> collectStructurals(simd_base<StepSize>& currentValues, simd_base<StepSize>& quotes, simd_base<StepSize>& whitespace, simd_base<StepSize>& op,
			simd_base<StepSize>& backslash, simd_base<StepSize>& nextIsEscaped) {
			currentValues				   = collectEscapedCharacters(currentValues, backslash, nextIsEscaped);
			quotes						   = quotes.bitAndNot(currentValues);
			currentValues				   = quotes.carrylessMultiplication(prevInstring);
			simd_base<StepSize> stringTail = currentValues ^ quotes;
			simd_base<StepSize> scalar	   = ~(op | whitespace);
			currentValues				   = scalar.bitAndNot(quotes);
			currentValues				   = currentValues.follows(storedLSB01);
			currentValues				   = scalar.bitAndNot(currentValues);
			currentValues				   = op | currentValues;
			currentValues				   = currentValues.bitAndNot(stringTail);
			return currentValues;
		}
	};

}

#if defined(load)
	#undef load
#endif