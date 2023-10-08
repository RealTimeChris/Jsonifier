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
			simd_base newPtr[StridesPerStep]{};
			simd_base whitespace{};
			simd_base backslash{};
			simd_base quotes{};
			simd_base op{};
			while (stringBlockReader.hasFullBlock()) {
				generateStructurals(stringBlockReader.fullBlock(), newPtr, backslash, quotes, whitespace, op);
				generateStructurals(stringBlockReader.fullBlock() + BitPerStep, newPtr, backslash, quotes, whitespace, op);
				stringBlockReader.advanceIndex();
			}
			uint8_t block[BitPerStep * 2];
			if (stringBlockReader.getRemainder(block) > 0) {
				generateStructurals(block, newPtr, backslash, quotes, whitespace, op);
				generateStructurals(block + BitPerStep, newPtr, backslash, quotes, whitespace, op);
			}
		}

		inline size_type getTapeLength() {
			return tapeIndex;
		}

		inline structural_index* getStructurals() {
			return structuralIndices.data();
		}

	  protected:
		inline static const simd_base oddBits{ makeSimdBase(0xAAAAAAAAAAAAAAAAULL) };
		jsonifier::vector<structural_index> structuralIndices{};
		string_block_reader<BitPerStep * 2> stringBlockReader{};
		jsonifier::string_view_base<uint8_t> stringView{};
		size_type prevInstring{};
		size_type stringIndex{};
		size_type tapeIndex{};
		bool storedLSB01{};
		bool storedLSB02{};

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

		template<size_type index = 0> inline void collectStringValues(string_view_ptr valuesNew, simd_base* newPtr) {
			if constexpr (index < StridesPerStep) {
				newPtr[index] = gatherValues(valuesNew + (BytesPerStep * index));
				collectStringValues<index + 1>(valuesNew, newPtr);
			}
		}

		inline void generateStructurals(string_view_ptr valueNew, simd_base* newPtr, simd_base& backslash, simd_base& quotes, simd_base& whitespace, simd_base& op
			) {
			collectStringValues(valueNew, newPtr);
			whitespace.convertWhitespaceToSimdBase(newPtr);
			backslash.convertBackslashesToSimdBase(newPtr);
			op.convertStructuralsToSimdBase(newPtr);
			quotes.convertQuotesToSimdBase(newPtr);
			addTapeValues(collectStructurals(backslash, quotes, whitespace, op));
			stringIndex += BitPerStep;
		}

		void addTapeValues(simd_base&& structurals) {
			alignas(JsonifierAlignment) size_type newBits[StridesPerStep]{};
			structurals.store<size_type>(newBits);
			addTapeValuesHelper(std::move(structurals), newBits);
		}

		template<size_type index = 0> inline void addTapeValuesHelper(simd_base&& structurals, size_type* newBits) {
			if constexpr (index < StridesPerStep) {
				if (!newBits[index]) {
					addTapeValuesHelper<index + 1>(std::forward<simd_base>(structurals), newBits);
					return;
				}
				auto cnt			  = popcnt(newBits[index]);
				size_type rollsAmount = static_cast<size_type>(std::ceil(static_cast<float>(cnt) / 8.0f));
				for (size_type y = 0; y < rollsAmount; ++y) {
					newBits[index] = rollValuesIntoTape<0, index>(y, newBits[index]);
				}
				tapeIndex += cnt;
				addTapeValuesHelper<index + 1>(std::forward<simd_base>(structurals), newBits);
			}
		}

		inline simd_base collectEscapedCharacters(simd_base& backslash) {
			if (backslash.operator bool()) {
				simd_base nextIsEscaped{};
				nextIsEscaped.setLSB(storedLSB02);
				simd_base potentialEscape			= backslash.bitAndNot(nextIsEscaped);
				simd_base maybeEscaped				= potentialEscape.shl<1>();
				maybeEscaped						= maybeEscaped | oddBits;
				maybeEscaped						= maybeEscaped - potentialEscape;
				simd_base escapeAndTerminalCode		= maybeEscaped ^ oddBits;
				storedLSB02							= (escapeAndTerminalCode & backslash).getMSB();
				return escapeAndTerminalCode ^ (backslash | nextIsEscaped);
			} else {
				simd_base escaped{};
				escaped.setLSB(storedLSB02);
				storedLSB02 = false;
				return escaped;
			}
		}

		inline simd_base collectStructurals(simd_base& backslash, simd_base& quotes, simd_base& whitespace, simd_base& op) {
			simd_base currentValues = collectEscapedCharacters(backslash);
			quotes					= quotes.bitAndNot(currentValues);
			currentValues			= quotes.carrylessMultiplication(prevInstring);
			simd_base stringTail	= currentValues ^ quotes;
			simd_base scalar		= ~(op | whitespace);
			currentValues			= scalar.bitAndNot(quotes);
			currentValues			= currentValues.follows(storedLSB01);
			currentValues			= scalar.bitAndNot(currentValues);
			currentValues			= op | currentValues;
			return currentValues.bitAndNot(stringTail);
		}
	};

}
