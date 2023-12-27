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

#include <jsonifier/SimdStructuralIterator.hpp>
#include <jsonifier/ISADetection.hpp>
#include <jsonifier/StringView.hpp>
#include <sstream>
#include <cmath>

namespace jsonifier_internal {

	class string_block_reader {
	  public:
		JSONIFIER_INLINE void reset(string_view_ptr stringViewNew, uint64_t lengthNew) noexcept {
			lengthMinusStep = lengthNew < BitsPerStep ? 0 : lengthNew - BitsPerStep;
			inString		= stringViewNew;
			length			= lengthNew;
			index			= 0;
		}

		JSONIFIER_INLINE uint64_t getRemainder(string_buffer_ptr dest) const noexcept {
			if (length == index) [[unlikely]] {
				return 0;
			}
			std::memset(dest, 0x20, BitsPerStep);
			std::memcpy(dest, inString + index, (length - index));
			return length - index;
		}

		JSONIFIER_INLINE string_view_ptr fullBlock() noexcept {
			string_view_ptr newPtr = inString + index;
			index += BitsPerStep;
			return newPtr;
		}

		JSONIFIER_INLINE bool hasFullBlock() const noexcept {
			return index < lengthMinusStep;
		}

	  protected:
		string_view_ptr inString{};
		uint64_t lengthMinusStep{};
		uint64_t length{};
		uint64_t index{};
	};

	template<bool doWeUseInitialBuffer> class simd_string_reader {
	  public:
		using size_type = uint64_t;
		using allocator = alloc_wrapper<structural_index>;
		static constexpr double multiplier{ 4.5f / 5.0f };

		template<bool refreshString, typename char_type> JSONIFIER_INLINE void reset(char_type* stringViewNew, size_type size) {
			if constexpr (refreshString) {
				currentParseBuffer = jsonifier::string_view_base{ reinterpret_cast<string_view_ptr>(stringViewNew), size };
				auto newSize	   = roundUpToMultiple<uint64_t, 8>(static_cast<uint64_t>(static_cast<double>(currentParseBuffer.size()) * multiplier));
				if (structuralIndices.size() < newSize) [[unlikely]] {
					structuralIndices.resize(newSize);
				}
				resetInternal();
			} else if (!jsonifier_core_internal::compare(currentParseBuffer.data(), stringViewNew, currentParseBuffer.size())) [[likely]] {
				currentParseBuffer = jsonifier::string_view_base{ reinterpret_cast<string_view_ptr>(stringViewNew), size };
				auto newSize	   = roundUpToMultiple<uint64_t, 8>(static_cast<uint64_t>(static_cast<double>(currentParseBuffer.size()) * multiplier));
				if (structuralIndices.size() < newSize) [[unlikely]] {
					structuralIndices.resize(newSize);
				}
				resetInternal();
			}
		}

		template<bool refreshString, typename char_type> JSONIFIER_INLINE std::string resetWithErrorPrintOut(char_type* stringViewNew, size_type size, size_type errorIndex) {
			std::string returnValue{ "For the following string values: " };
			if (errorIndex < std::string{}.max_size()) {
				returnValue += std::string_view{ reinterpret_cast<const char*>(stringViewNew + errorIndex), BitsPerStep };
				returnValue += "\n";
			}
			if constexpr (refreshString) {
				currentParseBuffer = { reinterpret_cast<string_view_ptr>(stringViewNew), size };
				auto newSize	   = roundUpToMultiple<uint64_t, 8>(static_cast<uint64_t>(static_cast<double>(currentParseBuffer.size()) * multiplier));
				if (structuralIndices.size() < newSize) {
					structuralIndices.resize(newSize);
				}
				return returnValue + resetInternalWithErrorPrintOut(errorIndex);
			} else if (currentParseBuffer != stringViewNew) {
				currentParseBuffer = { reinterpret_cast<string_view_ptr>(stringViewNew), size };
				auto newSize	   = roundUpToMultiple<uint64_t, 8>(static_cast<uint64_t>(static_cast<double>(currentParseBuffer.size()) * multiplier));
				if (structuralIndices.size() < newSize) {
					structuralIndices.resize(newSize);
				}
				return returnValue + resetInternalWithErrorPrintOut(errorIndex);
			}
		}

		JSONIFIER_INLINE void resetInternal() {
			stringBlockReader.reset(currentParseBuffer.data(), currentParseBuffer.size());
			overflow	 = false;
			prevInString = 0;
			stringIndex	 = 0;
			tapeIndex	 = 0;
			generateJsonIndices();
		}

		JSONIFIER_INLINE std::string resetInternalWithErrorPrintOut(size_type errorIndex) {
			stringBlockReader.reset(currentParseBuffer.data(), currentParseBuffer.size());
			stringIndex = 0;
			tapeIndex	= 0;
			return generateJsonIndicesWithErrorPrintOut(errorIndex);
		}

		JSONIFIER_INLINE auto end() {
			return structuralIndices.data() + (tapeIndex - 01);
		}

		JSONIFIER_INLINE auto getStringView() {
			return currentParseBuffer.data();
		}

		JSONIFIER_INLINE auto begin() {
			structuralIndices[tapeIndex] = std::numeric_limits<uint32_t>::max();
			return structuralIndices.data();
		}

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
		jsonifier::vector<structural_index, doWeUseInitialBuffer> structuralIndices{};
		JSONIFIER_ALIGN size_type newBits[SixtyFourBitsPerStep]{};
		jsonifier::string_view_base<uint8_t> currentParseBuffer{};
		string_block_reader stringBlockReader{};
		uint8_t block[BitsPerStep]{};
		size_type stringIndex{};
		int64_t prevInString{};
		size_type tapeIndex{};
		bool overflow{};

		JSONIFIER_INLINE void generateJsonIndices() {
			while (stringBlockReader.hasFullBlock()) {
				generateStructurals(stringBlockReader.fullBlock());
			}
			if (stringBlockReader.getRemainder(block) > 0) [[likely]] {
				generateStructurals(block);
			}
		}

		JSONIFIER_INLINE size_type getTapeLength() const {
			return tapeIndex - 1;
		}

		template<size_type index, size_t... indices> JSONIFIER_INLINE size_type rollValuesIntoTape(size_type currentIndex, size_type newBits, std::index_sequence<indices...>) {
			static constexpr uint64_t bitTotal{ index * 64ull };
			((structuralIndices[indices + (currentIndex * 8) + tapeIndex] = static_cast<uint32_t>(tzcnt(newBits) + bitTotal + stringIndex), newBits = blsr(newBits)), ...);
			return newBits;
		}

		template<size_t... indices> void collectStringValues(string_view_ptr values, std::index_sequence<indices...>) {
			((newPtr[indices] = gatherValuesU<simd_int_t>(values + (BytesPerStep * indices))), ...);
		}

		JSONIFIER_INLINE void generateStructurals(string_view_ptr values) {
			collectStringValues(values, std::make_index_sequence<StridesPerStep>{});
			whitespace	= simd_base::collectWhitespaceAsSimdBase(newPtr, std::make_index_sequence<StridesPerStep>{});
			backslashes = simd_base::collectBackslashesAsSimdBase(newPtr, std::make_index_sequence<StridesPerStep>{});
			op			= simd_base::collectStructuralsAsSimdBase(newPtr, std::make_index_sequence<StridesPerStep>{});
			quotes		= simd_base::collectQuotesAsSimdBase(newPtr, std::make_index_sequence<StridesPerStep>{});
			collectStructurals();
			addTapeValues(std::make_index_sequence<SixtyFourBitsPerStep>{});
			stringIndex += BitsPerStep;
		}

		template<size_t... indices> JSONIFIER_INLINE void addTapeValues(std::index_sequence<indices...>) {
			store(structurals, newBits);
			(addTapeValuesImpl<indices>(), ...);
		}

		template<uint64_t index> JSONIFIER_INLINE void addTapeValuesImpl() {
			if (!newBits[index]) [[unlikely]] {
				return;
			}
			auto cnt			  = popcnt(newBits[index]);
			size_type rollsAmount = static_cast<size_type>(std::ceil(static_cast<double>(cnt) / 8.0f));
			for (size_type y = 0; y < rollsAmount; ++y) {
				newBits[index] = rollValuesIntoTape<index>(y, newBits[index], std::make_index_sequence<StridesPerStep>{});
			}
			tapeIndex += cnt;
		}

		JSONIFIER_INLINE void collectNonEmptyEscaped() noexcept {
			simd_int_t potentialEscape			 = simd_base::opAndNot(backslashes, nextIsEscaped);
			simd_int_t maybeEscaped				 = simd_base::opShl<1>(potentialEscape, std::make_index_sequence<SixtyFourBitsPerStep - 1>{});
			simd_int_t maybeEscapedAndOddBits	 = simd_base::opOr(maybeEscaped, oddBitsVal);
			simd_int_t evenSeriesCodesAndOddBits = simd_base::opSub(maybeEscapedAndOddBits, potentialEscape, std::make_index_sequence<SixtyFourBitsPerStep>{});
			simd_int_t escapeAndTerminalCode	 = simd_base::opXor(evenSeriesCodesAndOddBits, oddBitsVal);
			escaped								 = simd_base::opXor(escapeAndTerminalCode, simd_base::opOr(backslashes, nextIsEscaped));
			nextIsEscaped						 = simd_base::opSetLSB(nextIsEscaped, simd_base::opGetMSB(simd_base::opAnd(escapeAndTerminalCode, backslashes)));
		}

		JSONIFIER_INLINE void collectEmptyEscaped() {
			auto escapedNew = nextIsEscaped;
			nextIsEscaped	= simd_base::reset();
			escaped			= escapedNew;
		}

		JSONIFIER_INLINE void collectEscapedCharacters() {
			return simd_base::opBool(backslashes) ? collectNonEmptyEscaped() : collectEmptyEscaped();
		}

		JSONIFIER_INLINE void collectStructurals() {
			collectEscapedCharacters();
			quotes								= simd_base::opAndNot(quotes, escaped);
			simd_int_t inString					= simd_base::opCLMul(quotes, prevInString, std::make_index_sequence<SixtyFourBitsPerStep>{});
			simd_int_t stringTail				= simd_base::opXor(inString, quotes);
			simd_int_t scalar					= simd_base::opNot(simd_base::opOr(op, whitespace));
			simd_int_t nonQuoteScalar			= simd_base::opAndNot(scalar, quotes);
			simd_int_t followsNonQuoteScalar	= simd_base::opFollows(nonQuoteScalar, overflow);
			simd_int_t potentialScalarStart		= simd_base::opAndNot(scalar, followsNonQuoteScalar);
			simd_int_t porentialStructuralStart = simd_base::opOr(op, potentialScalarStart);
			structurals							= simd_base::opAndNot(porentialStructuralStart, stringTail);
		}

		JSONIFIER_INLINE std::string collectEscapedCharactersWithErrorPrintOut(size_type errorIndex) {
			if (simd_base::opBool(backslashes)) {
				return collectNonEmptyEscapedWithErrorPrintOut(errorIndex);
			} else {
				collectEmptyEscaped();
				return {};
			}
		}

		JSONIFIER_INLINE std::string generateJsonIndicesWithErrorPrintOut(size_type errorIndex) {
			std::string returnValue{};
			while (stringBlockReader.hasFullBlock()) {
				returnValue += generateStructuralsWithErrorPrintOut(stringBlockReader.fullBlock(), errorIndex);
			}
			if (stringBlockReader.getRemainder(block) > 0) {
				returnValue += generateStructuralsWithErrorPrintOut(block, errorIndex);
			}
			return returnValue;
		}

		JSONIFIER_INLINE std::string generateStructuralsWithErrorPrintOut(string_view_ptr values, size_type errorIndex) {
			std::stringstream returnValue{};
			collectStringValues(values, std::make_index_sequence<StridesPerStep>{});
			whitespace	= simd_base::collectWhitespaceAsSimdBase(newPtr, std::make_index_sequence<StridesPerStep>{});
			backslashes = simd_base::collectBackslashesAsSimdBase(newPtr, std::make_index_sequence<StridesPerStep>{});
			op			= simd_base::collectStructuralsAsSimdBase(newPtr, std::make_index_sequence<StridesPerStep>{});
			quotes		= simd_base::collectQuotesAsSimdBase(newPtr, std::make_index_sequence<StridesPerStep>{});
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
			addTapeValues(std::make_index_sequence<SixtyFourBitsPerStep>{});
			stringIndex += BitsPerStep;
			return returnValue.str();
		}

		JSONIFIER_INLINE std::string collectNonEmptyEscapedWithErrorPrintOut(size_type errorIndex) noexcept {
			std::stringstream returnValue{};
			simd_int_t potentialEscape			 = simd_base::opAndNot(backslashes, nextIsEscaped);
			simd_int_t maybeEscaped				 = simd_base::opShl<1>(potentialEscape, std::make_index_sequence<SixtyFourBitsPerStep - 1>{});
			simd_int_t maybeEscapedAndOddBits	 = simd_base::opOr(maybeEscaped, oddBitsVal);
			simd_int_t evenSeriesCodesAndOddBits = simd_base::opSub(maybeEscapedAndOddBits, potentialEscape, std::make_index_sequence<SixtyFourBitsPerStep>{});
			simd_int_t escapeAndTerminalCode	 = simd_base::opXor(evenSeriesCodesAndOddBits, oddBitsVal);
			escaped								 = simd_base::opXor(escapeAndTerminalCode, simd_base::opOr(backslashes, nextIsEscaped));
			nextIsEscaped						 = simd_base::opSetLSB(nextIsEscaped, simd_base::opGetMSB(simd_base::opAnd(escapeAndTerminalCode, backslashes)));
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

		JSONIFIER_INLINE std::string collectStructuralsWithErrorPrintOut(size_type errorIndex) {
			std::stringstream returnValue{};
			std::string returnValueNew			= collectEscapedCharactersWithErrorPrintOut(errorIndex);
			quotes								= simd_base::opAndNot(quotes, escaped);
			simd_int_t inString					= simd_base::opCLMul(quotes, prevInString, std::make_index_sequence<SixtyFourBitsPerStep>{});
			simd_int_t stringTail				= simd_base::opXor(inString, quotes);
			simd_int_t scalar					= simd_base::opNot(simd_base::opOr(op, whitespace));
			simd_int_t nonQuoteScalar			= simd_base::opAndNot(scalar, quotes);
			simd_int_t followsNonQuoteScalar	= simd_base::opFollows(nonQuoteScalar, overflow);
			simd_int_t potentialScalarStart		= simd_base::opAndNot(scalar, followsNonQuoteScalar);
			simd_int_t porentialStructuralStart = simd_base::opOr(op, potentialScalarStart);
			structurals							= simd_base::opAndNot(porentialStructuralStart, stringTail);
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