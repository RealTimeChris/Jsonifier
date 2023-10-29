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

	class string_block_reader {
	  public:
		jsonifier_inline void reset(const uint8_t* stringViewNew, uint64_t lengthNew) noexcept {
			lengthMinusStep = lengthNew < BitsPerStep ? 0 : lengthNew - BitsPerStep;
			inString		= stringViewNew;
			length			= lengthNew;
			index			= 0;
		}

		jsonifier_inline uint64_t getRemainder(string_buffer_ptr dest) const noexcept {
			if (length == index) {
				return 0;
			}
			std::memset(dest, 0x20, BitsPerStep);
			std::memcpy(dest, inString + index, (length - index));
			return length - index;
		}

		jsonifier_inline string_view_ptr fullBlock() noexcept {
			const uint8_t* newPtr = inString + index;
			index += BitsPerStep;
			return newPtr;
		}

		jsonifier_inline bool hasFullBlock() const noexcept {
			return index < lengthMinusStep;
		}

	  protected:
		const uint8_t* inString{};
		uint64_t lengthMinusStep{};
		uint64_t length{};
		uint64_t index{};
	};

	class simd_string_reader : public simd_base, alloc_wrapper<structural_index> {
	  public:
		using size_type = uint64_t;
		using allocator = alloc_wrapper<structural_index>;

		template<bool refreshString, jsonifier::concepts::string_t string_type> jsonifier_inline void reset(string_type&& stringViewNew) {
			if jsonifier_constexpr (refreshString) {
				stringView	 = reinterpret_cast<const uint8_t*>(stringViewNew.data());
				stringLength = stringViewNew.size();
				structuralIndices.resize(roundUpToMultiple<8>(static_cast<uint64_t>(static_cast<float>(stringLength) * 4.5f / 5.0f)));
				resetInternal();
			} else if (jsonifier::string_view_base<uint8_t>{ stringView, stringLength } != stringViewNew) {
				stringView	 = reinterpret_cast<const uint8_t*>(stringViewNew.data());
				stringLength = stringViewNew.size();
				structuralIndices.resize(roundUpToMultiple<8>(static_cast<uint64_t>(static_cast<float>(stringLength) * 4.5f / 5.0f)));
				resetInternal();
			}
		}

		jsonifier_inline void resetInternal() {
			stringBlockReader.reset(stringView, stringLength);
			overflow	 = false;
			prevInString = 0;
			stringIndex	 = 0;
			tapeIndex	 = 0;
			generateJsonIndices();
		}

		jsonifier_inline structural_iterator begin() {
			return structural_iterator{ getStructurals() };
		}

		jsonifier_inline structural_index* getStructurals() {
			structuralIndices[tapeIndex] = nullptr;
			return structuralIndices.getIndices();
		}

		jsonifier_inline ~simd_string_reader(){};

	  protected:

		static jsonifier_constexpr simd_int_t oddBitsVal{ simdFromValue<simd_int_t>(0xAA) };
		structural_index_vector structuralIndices{};
		simd_int_t evenSeriesCodesAndOddBits{};
		simd_int_t porentialStructuralStart{};
		simd_int_t newPtr[StridesPerStep]{};
		simd_int_t maybeEscapedAndOddBits{};
		simd_int_t followsNonQuoteScalar{};
		simd_int_t escapeAndTerminalCode{};
		simd_int_t potentialScalarStart{};
		simd_int_t nonQuoteScalar{};
		simd_int_t nextIsEscaped{};
		simd_int_t maybeEscaped{};
		simd_int_t backslashes{};
		simd_int_t structurals{};
		simd_int_t stringTail{};
		simd_int_t whitespace{};
		simd_int_t inString{};
		simd_int_t escaped{};
		simd_int_t quotes{};
		simd_int_t escape{};
		simd_int_t scalar{};
		simd_int_t op{};
		alignas(BytesPerStep) size_type newBits[SixtyFourBitsPerStep]{};
		string_block_reader stringBlockReader{};
		string_view_ptr stringView{};
		size_type stringLength{};
		size_type prevInString{};
		size_type stringIndex{};
		size_type tapeIndex{};
		bool overflow{};

		jsonifier_inline void generateJsonIndices() {
			while (stringBlockReader.hasFullBlock()) {
				generateStructurals(stringBlockReader.fullBlock());
			}
			uint8_t block[BitsPerStep]{};
			if (stringBlockReader.getRemainder(block) > 0) {
				generateStructurals(block);
			}
		}

		jsonifier_inline size_type getTapeLength() {
			return tapeIndex - 1;
		}

		template<size_type index = 0, size_type index02> jsonifier_inline size_type rollValuesIntoTape(size_type currentIndex, size_type newBits) {
			if jsonifier_constexpr (index < StridesPerStep) {
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
			collectWhitespaceAsSimdBase(whitespace, newPtr);
			collectBackslashesAsSimdBase(backslashes, newPtr);
			collectStructuralsAsSimdBase(op, newPtr);
			collectQuotesAsSimdBase(quotes, newPtr);
			structurals = collectStructurals();
			addTapeValues();
			stringIndex += BitsPerStep;
		}

		jsonifier_inline void addTapeValues() {
			store(structurals, newBits);
			addTapeValuesHelper<0>();
		}

		template<size_type index> jsonifier_inline void addTapeValuesHelper() {
			if jsonifier_constexpr (index < SixtyFourBitsPerStep) {
				if (!newBits[index]) {
					addTapeValuesHelper<index + 1>();
					return;
				}
				auto cnt			  = popcnt(newBits[index]);
				size_type rollsAmount = static_cast<size_type>(std::ceil(static_cast<float>(cnt) / 8.0f));
				for (size_type y = 0; y < rollsAmount; ++y) {
					newBits[index] = rollValuesIntoTape<0, index>(y, newBits[index]);
				}
				tapeIndex += cnt;
				addTapeValuesHelper<index + 1>();
			}
		}

		jsonifier_inline simd_int_t collectEscapedCharactersHelper(const simd_int_t& potentialEscape) noexcept {
			maybeEscaped			  = shl<1>(potentialEscape);
			maybeEscapedAndOddBits	  = opOr(maybeEscaped, oddBitsVal);
			evenSeriesCodesAndOddBits = opSub(maybeEscapedAndOddBits, potentialEscape);
			return opXor(evenSeriesCodesAndOddBits, oddBitsVal);
		}

		jsonifier_inline simd_int_t collectNonEmptyEscaped() noexcept {
			escapeAndTerminalCode = collectEscapedCharactersHelper(bitAndNot(backslashes, nextIsEscaped));
			escaped				  = opXor(escapeAndTerminalCode, opOr(backslashes, nextIsEscaped));
			escape				  = opAnd(escapeAndTerminalCode, backslashes);
			nextIsEscaped		  = setLSB(nextIsEscaped, getMSB(escape));
			return escaped;
		}

		jsonifier_inline simd_int_t collectEmptyEscaped() {
			auto escapedNew = nextIsEscaped;
			nextIsEscaped	= simd_int_t{};
			return escapedNew;
		}

		jsonifier_inline simd_int_t collectEscapedCharacters() {
			return opBool(backslashes) ? collectNonEmptyEscaped() : collectEmptyEscaped();
		}

		jsonifier_inline simd_int_t collectStructurals() {
			escaped					 = collectEscapedCharacters();
			quotes					 = bitAndNot(quotes, escaped);
			inString				 = carrylessMultiplication(quotes, prevInString);
			stringTail				 = opXor(inString, quotes);
			scalar					 = opNot(opOr(op, whitespace));
			nonQuoteScalar			 = bitAndNot(scalar, quotes);
			followsNonQuoteScalar	 = follows(nonQuoteScalar, overflow);
			potentialScalarStart	 = bitAndNot(scalar, followsNonQuoteScalar);
			porentialStructuralStart = opOr(op, potentialScalarStart);
			return bitAndNot(porentialStructuralStart, stringTail);
		}
	};

};