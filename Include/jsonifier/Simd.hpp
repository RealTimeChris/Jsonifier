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
#include <sstream>
#include <cmath>

namespace jsonifier_internal {

	class string_block_reader {
	  public:
		inline void reset(string_view_ptr stringViewNew, uint64_t lengthNew) noexcept {
			lengthMinusStep = lengthNew < BitsPerStep ? 0 : lengthNew - BitsPerStep;
			inString		= stringViewNew;
			length			= lengthNew;
			index			= 0;
		}

		inline uint64_t getRemainder(string_buffer_ptr dest) const noexcept {
			if (length == index) {
				return 0;
			}
			std::memset(dest, 0x20, BitsPerStep);
			std::memcpy(dest, inString + index, (length - index));
			return length - index;
		}

		inline string_view_ptr fullBlock() noexcept {
			string_view_ptr newPtr = inString + index;
			index += BitsPerStep;
			return newPtr;
		}

		inline bool hasFullBlock() const noexcept {
			return index < lengthMinusStep;
		}

	  protected:
		string_view_ptr inString{};
		uint64_t lengthMinusStep{};
		uint64_t length{};
		uint64_t index{};
	};

	class simd_string_reader : public simd_base {
	  public:
		using size_type = uint64_t;
		using allocator = alloc_wrapper<structural_index>;

		template<bool refreshString, jsonifier::concepts::string_t string_type> inline void reset(string_type&& stringViewNew) {
			if constexpr (refreshString) {
				currentParseBuffer = { reinterpret_cast<string_view_ptr>(stringViewNew.data()), stringViewNew.size() };
				structuralIndices.resize(roundUpToMultiple<8>(static_cast<uint64_t>(static_cast<float>(currentParseBuffer.size()) * 4.5f / 5.0f)));
				resetInternal();
			} else if (currentParseBuffer != stringViewNew) {
				currentParseBuffer = { reinterpret_cast<string_view_ptr>(stringViewNew.data()), stringViewNew.size() };
				structuralIndices.resize(roundUpToMultiple<8>(static_cast<uint64_t>(static_cast<float>(currentParseBuffer.size()) * 4.5f / 5.0f)));
				resetInternal();
			}
		}

		template<bool refreshString, jsonifier::concepts::string_t string_type> inline std::string resetWithErrorPrintOut(string_type&& stringViewNew, size_type errorIndex) {
			std::string returnValue{ "For the following string values: " };
			if (errorIndex < std::string{}.max_size()) {
				returnValue += std::string_view{ reinterpret_cast<const char*>(stringViewNew.data() + errorIndex), BitsPerStep };
				returnValue += "\n";
			}
			if constexpr (refreshString) {
				currentParseBuffer = { reinterpret_cast<string_view_ptr>(stringViewNew.data()), stringViewNew.size() };
				structuralIndices.resize(roundUpToMultiple<8>(static_cast<uint64_t>(static_cast<float>(currentParseBuffer.size()) * 4.5f / 5.0f)));
				return returnValue + resetInternalWithErrorPrintOut(errorIndex);
			} else if (currentParseBuffer != stringViewNew) {
				currentParseBuffer = { reinterpret_cast<string_view_ptr>(stringViewNew.data()), stringViewNew.size() };
				structuralIndices.resize(roundUpToMultiple<8>(static_cast<uint64_t>(static_cast<float>(currentParseBuffer.size()) * 4.5f / 5.0f)));
				return returnValue + resetInternalWithErrorPrintOut(errorIndex);
			}
		}

		inline void resetInternal() {
			stringBlockReader.reset(currentParseBuffer.data(), currentParseBuffer.size());
			stringIndex = 0;
			tapeIndex	= 0;
			generateJsonIndices();
		}

		inline std::string resetInternalWithErrorPrintOut(size_type errorIndex) {
			stringBlockReader.reset(currentParseBuffer.data(), currentParseBuffer.size());
			stringIndex = 0;
			tapeIndex	= 0;
			return generateJsonIndicesWithErrorPrintOut(errorIndex);
		}

		inline structural_iterator begin() {
			structuralIndices[tapeIndex] = nullptr;
			return structuralIndices.data();
		}

		inline ~simd_string_reader(){};

	  protected:
		static constexpr simd_int_t oddBitsVal{ simdFromValue<simd_int_t>(0xAA) };
		simd_int_t newPtr[StridesPerStep]{};
		simd_int_t nextIsEscaped{};
		simd_int_t backslashes{};
		simd_int_t structurals{};
		simd_int_t whitespace{};
		simd_int_t escaped{};
		simd_int_t quotes{};
		simd_int_t op{};
		alignas(BytesPerStep) size_type newBits[SixtyFourBitsPerStep]{};
		jsonifier::string_view_base<uint8_t> currentParseBuffer{};
		bool prevInString{};
		bool overflow{};
		structural_index_vector structuralIndices{};
		string_block_reader stringBlockReader{};
		size_type stringIndex{};
		size_type tapeIndex{};


		inline void generateJsonIndices() {
			while (stringBlockReader.hasFullBlock()) {
				generateStructurals(stringBlockReader.fullBlock());
			}
			uint8_t block[BitsPerStep]{};
			if (stringBlockReader.getRemainder(block) > 0) {
				generateStructurals(block);
			}
		}

		inline size_type getTapeLength() {
			return tapeIndex - 1;
		}

		template<size_type index = 0, size_type index02> inline size_type rollValuesIntoTape(size_type currentIndex, size_type newBits) {
			if constexpr (index < StridesPerStep) {
				structuralIndices[index + (currentIndex * 8) + tapeIndex] = currentParseBuffer.data() + static_cast<uint32_t>(tzcnt(newBits) + (index02 * 64ull) + stringIndex);
				newBits													  = blsr(newBits);
				return (newBits == 0) ? newBits : rollValuesIntoTape<index + 1, index02>(currentIndex, newBits);
			} else {
				return newBits;
			}
		}

		template<uint64_t index> void collectStringValuesHelper(string_view_ptr values) {
			if constexpr (index < StridesPerStep) {
				newPtr[index] = gatherValuesU<simd_int_t>(values + (BytesPerStep * index));
				collectStringValuesHelper<index + 1>(values);
			}
		}

		inline void collectStringValues(string_view_ptr values) {
			collectStringValuesHelper<0>(values);
		}

		inline void generateStructurals(string_view_ptr values) {
			collectStringValues(values);
			collectWhitespaceAsSimdBase(whitespace, newPtr);
			collectBackslashesAsSimdBase(backslashes, newPtr);
			collectStructuralsAsSimdBase(op, newPtr);
			collectQuotesAsSimdBase(quotes, newPtr);
			collectStructurals();
			addTapeValues();
			stringIndex += BitsPerStep;
		}

		inline void addTapeValues() {
			store(structurals, newBits);
			addTapeValuesHelper<0>();
		}

		template<size_type index> inline void addTapeValuesHelper() {
			if constexpr (index < SixtyFourBitsPerStep) {
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

		inline void collectNonEmptyEscaped() noexcept {
			simd_int_t potentialEscape			 = opAndNot(backslashes, nextIsEscaped);
			simd_int_t maybeEscaped				 = shl<1>(potentialEscape);
			simd_int_t maybeEscapedAndOddBits	 = opOr(maybeEscaped, oddBitsVal);
			simd_int_t evenSeriesCodesAndOddBits = opSub(maybeEscapedAndOddBits, potentialEscape);
			simd_int_t escapeAndTerminalCode	 = opXor(evenSeriesCodesAndOddBits, oddBitsVal);
			escaped								 = opXor(escapeAndTerminalCode, opOr(backslashes, nextIsEscaped));
			nextIsEscaped						 = setLSB(nextIsEscaped, getMSB(opAnd(escapeAndTerminalCode, backslashes)));
		}

		inline void collectEmptyEscaped() {
			auto escapedNew = nextIsEscaped;
			nextIsEscaped	= simd_base::reset();
			escaped			= escapedNew;
		}

		inline void collectEscapedCharacters() {
			return opBool(backslashes) ? collectNonEmptyEscaped() : collectEmptyEscaped();
		}

		inline void collectStructurals() {
			collectEscapedCharacters();
			quotes								= opAndNot(quotes, escaped);
			simd_int_t inString					= carrylessMultiplication(quotes, prevInString);
			simd_int_t stringTail				= opXor(inString, quotes);
			simd_int_t scalar					= opNot(opOr(op, whitespace));
			simd_int_t nonQuoteScalar			= opAndNot(scalar, quotes);
			simd_int_t followsNonQuoteScalar	= follows(nonQuoteScalar, overflow);
			simd_int_t potentialScalarStart		= opAndNot(scalar, followsNonQuoteScalar);
			simd_int_t porentialStructuralStart = opOr(op, potentialScalarStart);
			structurals							= opAndNot(porentialStructuralStart, stringTail);
		}

		inline std::string collectEscapedCharactersWithErrorPrintOut(size_type errorIndex) {
			if (opBool(backslashes)) {
				return collectNonEmptyEscapedWithErrorPrintOut(errorIndex);
			} else {
				collectEmptyEscaped();
				return {};
			}
		}

		inline std::string generateJsonIndicesWithErrorPrintOut(size_type errorIndex) {
			std::string returnValue{};
			while (stringBlockReader.hasFullBlock()) {
				returnValue += generateStructuralsWithErrorPrintOut(stringBlockReader.fullBlock(), errorIndex);
			}
			uint8_t block[BitsPerStep]{};
			if (stringBlockReader.getRemainder(block) > 0) {
				returnValue += generateStructuralsWithErrorPrintOut(block, errorIndex);
			}
			return returnValue;
		}

		inline std::string generateStructuralsWithErrorPrintOut(string_view_ptr values, size_type errorIndex) {
			std::stringstream returnValue{};
			collectStringValues(values);
			collectWhitespaceAsSimdBase(whitespace, newPtr);
			collectBackslashesAsSimdBase(backslashes, newPtr);
			collectStructuralsAsSimdBase(op, newPtr);
			collectQuotesAsSimdBase(quotes, newPtr);
			if (stringIndex == errorIndex) {
				returnValue << "Whitespace Bits, for Index: " + std::to_string(stringIndex) + ": ";
				returnValue << printBits(whitespace).data();
				returnValue << "Backslash Bits, for Index: " + std::to_string(stringIndex) + ": ";
				returnValue << printBits(backslashes).data();
				returnValue << "Op Bits, for Index: " + std::to_string(stringIndex) + ": ";
				returnValue << printBits(op).data();
				returnValue << "Quote Bits, for Index: " + std::to_string(stringIndex) + ": ";
				returnValue << printBits(quotes).data();
			}
			std::string returnValueNew = collectStructuralsWithErrorPrintOut(errorIndex);
			if (stringIndex == errorIndex) {
				returnValue << returnValueNew;
			}
			addTapeValues();
			stringIndex += BitsPerStep;
			return returnValue.str();
		}

		inline std::string collectNonEmptyEscapedWithErrorPrintOut(size_type errorIndex) noexcept {
			std::stringstream returnValue{};
			simd_int_t potentialEscape			 = opAndNot(backslashes, nextIsEscaped);
			simd_int_t maybeEscaped				 = shl<1>(potentialEscape);
			simd_int_t maybeEscapedAndOddBits	 = opOr(maybeEscaped, oddBitsVal);
			simd_int_t evenSeriesCodesAndOddBits = opSub(maybeEscapedAndOddBits, potentialEscape);
			simd_int_t escapeAndTerminalCode	 = opXor(evenSeriesCodesAndOddBits, oddBitsVal);
			escaped								 = opXor(escapeAndTerminalCode, opOr(backslashes, nextIsEscaped));
			nextIsEscaped						 = setLSB(nextIsEscaped, getMSB(opAnd(escapeAndTerminalCode, backslashes)));
			if (stringIndex == errorIndex) {
				returnValue << "Potential Escape Bits, for Index: " + std::to_string(stringIndex) + ": ";
				returnValue << printBits(potentialEscape).data();
				returnValue << "Maybe Escaped Bits, for Index: " + std::to_string(stringIndex) + ": ";
				returnValue << printBits(maybeEscaped).data();
				returnValue << "Maybe Escaped And Odd Bits, for Index: " + std::to_string(stringIndex) + ": ";
				returnValue << printBits(maybeEscapedAndOddBits).data();
				returnValue << "Even Series Codes And Odd Bits, for Index: " + std::to_string(stringIndex) + ": ";
				returnValue << printBits(evenSeriesCodesAndOddBits).data();
				returnValue << "Escape And Terminal Code Bits, for Index: " + std::to_string(stringIndex) + ": ";
				returnValue << printBits(escapeAndTerminalCode).data();
				returnValue << "Next indices Escaped Bits, for Index: " + std::to_string(stringIndex) + ": ";
				returnValue << printBits(nextIsEscaped).data();
			}
			return returnValue.str();
		}

		inline std::string collectStructuralsWithErrorPrintOut(size_type errorIndex) {
			std::stringstream returnValue{};
			std::string returnValueNew			= collectEscapedCharactersWithErrorPrintOut(errorIndex);
			quotes								= opAndNot(quotes, escaped);
			simd_int_t inString					= carrylessMultiplication(quotes, prevInString);
			simd_int_t stringTail				= opXor(inString, quotes);
			simd_int_t scalar					= opNot(opOr(op, whitespace));
			simd_int_t nonQuoteScalar			= opAndNot(scalar, quotes);
			simd_int_t followsNonQuoteScalar	= follows(nonQuoteScalar, overflow);
			simd_int_t potentialScalarStart		= opAndNot(scalar, followsNonQuoteScalar);
			simd_int_t porentialStructuralStart = opOr(op, potentialScalarStart);
			structurals							= opAndNot(porentialStructuralStart, stringTail);
			if (stringIndex == errorIndex) {
				returnValue << returnValueNew;
				returnValue << "Escaped Bits, for Index: " + std::to_string(stringIndex) + ": ";
				returnValue << printBits(escaped).data();
				returnValue << "Quoted Bits, for Index: " + std::to_string(stringIndex) + ": ";
				returnValue << printBits(quotes).data();
				returnValue << "In String Bits, for Index: " + std::to_string(stringIndex) + ": ";
				returnValue << printBits(inString).data();
				returnValue << "String Tail Bits, for Index: " + std::to_string(stringIndex) + ": ";
				returnValue << printBits(stringTail).data();
				returnValue << "Scalar Bits, for Index: " + std::to_string(stringIndex) + ": ";
				returnValue << printBits(scalar).data();
				returnValue << "Overflow Bits, for Index: " + std::to_string(stringIndex) + ": ";
				returnValue << printBits(overflow).data();
				returnValue << "NonQuote Scalar Bits, for Index: " + std::to_string(stringIndex) + ": ";
				returnValue << printBits(nonQuoteScalar).data();
				returnValue << "Follows NonQuote Scalar Bits, for Index: " + std::to_string(stringIndex) + ": ";
				returnValue << printBits(followsNonQuoteScalar).data();
				returnValue << "Potential Scalar start Bits, for Index: " + std::to_string(stringIndex) + ": ";
				returnValue << printBits(potentialScalarStart).data();
				returnValue << "Potential Structural start Bits, for Index: " + std::to_string(stringIndex) + ": ";
				returnValue << printBits(porentialStructuralStart).data();
				returnValue << "Final Bits, for Index: " + std::to_string(stringIndex) + ": ";
				returnValue << printBits(structurals).data();
			}
			return returnValue.str();
		}
	};

};