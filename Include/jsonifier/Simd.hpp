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
			std::memcpy(dest, inString + index, length - index);
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
		static constexpr size_t PrefetchDistance = 128;

		JSONIFIER_INLINE simd_string_reader() {
			if constexpr (doWeUseInitialBuffer) {
				structuralIndices.assign(static_cast<uint64_t>(static_cast<double>(1024 * 1024 * 4) * 0.85f), nullptr);
			}
		}

		template<bool refreshString = true, bool minified = false, typename char_type> JSONIFIER_INLINE void reset(char_type* stringViewNew, size_type size) {
			if constexpr (refreshString) {
				currentParseBuffer = jsonifier::string_view_base{ reinterpret_cast<string_view_ptr>(stringViewNew), size };
				auto newSize	   = roundUpToMultiple<8ull>(static_cast<uint64_t>(static_cast<double>(currentParseBuffer.size()) * multiplier));
				if (structuralIndices.size() < newSize) [[unlikely]] {
					structuralIndices.resize(newSize * 2);
				}
				resetInternal<minified>();
			} else if (!compare(currentParseBuffer.data(), stringViewNew, currentParseBuffer.size())) [[likely]] {
				currentParseBuffer = jsonifier::string_view_base{ reinterpret_cast<string_view_ptr>(stringViewNew), size };
				auto newSize	   = roundUpToMultiple<8ull>(static_cast<uint64_t>(static_cast<double>(currentParseBuffer.size()) * multiplier));
				if (structuralIndices.size() < newSize) [[unlikely]] {
					structuralIndices.resize(newSize * 2);
				}
				resetInternal<minified>();
			}
		}

		template<bool refreshString, typename char_type> JSONIFIER_INLINE std::string resetWithErrorPrintOut(char_type* stringViewNew, size_type size, size_type errorIndex) {
			std::string returnValue{ "For the following string values: " };
			if (static_cast<int64_t>(errorIndex) < std::string{}.max_size()) {
				returnValue += std::string_view{ reinterpret_cast<const char*>(stringViewNew + errorIndex), 24 };
				returnValue += "\n";
			}
			if constexpr (refreshString) {
				currentParseBuffer = { reinterpret_cast<string_view_ptr>(stringViewNew), size };
				auto newSize	   = roundUpToMultiple<8ull>(static_cast<uint64_t>(static_cast<double>(currentParseBuffer.size()) * multiplier));
				if (structuralIndices.size() < newSize) {
					structuralIndices.resize(newSize * 2);
				}
				return returnValue + resetInternalWithErrorPrintOut(errorIndex);
			} else if (currentParseBuffer != stringViewNew) {
				currentParseBuffer = { reinterpret_cast<string_view_ptr>(stringViewNew), size };
				auto newSize	   = roundUpToMultiple<8ull>(static_cast<uint64_t>(static_cast<double>(currentParseBuffer.size()) * multiplier));
				if (structuralIndices.size() < newSize) {
					structuralIndices.resize(newSize * 2);
				}
				return returnValue + resetInternalWithErrorPrintOut(errorIndex);
			}
		}

		JSONIFIER_INLINE auto end() {
			return structuralIndices.data() + tapeIndex;
		}

		JSONIFIER_INLINE auto begin() {
			if (structuralIndices.size() > tapeIndex) {
				structuralIndices[tapeIndex]					= nullptr;
				structuralIndices[structuralIndices.size() - 1] = nullptr;
			}
			return structuralIndices.data();
		}

	  protected:
		static constexpr simd_int_t oddBitsVal{ simd_internal::simdFromValue<simd_int_t>(0xAA) };
		simd_internal::simd_int_t_holder rawStructurals{};
		simd_int_t newPtr[StridesPerStep]{}; 
		simd_int_t nextIsEscaped{};
		simd_int_t escaped{};
		JSONIFIER_ALIGN size_type newBits[SixtyFourBitsPerStep]{};
		jsonifier::string_view_base<uint8_t> currentParseBuffer{};
		jsonifier::vector<structural_index> structuralIndices{};
		JSONIFIER_ALIGN uint8_t block[BitsPerStep]{};
		string_block_reader stringBlockReader{};
		size_type stringIndex{};
		int64_t prevInString{};
		size_type tapeIndex{};
		bool overflow{};

		template<bool minified> JSONIFIER_INLINE void resetInternal() {
			stringBlockReader.reset(currentParseBuffer.data(), currentParseBuffer.size());
			overflow	 = false;
			prevInString = 0;
			stringIndex	 = 0;
			tapeIndex	 = 0;
			generateJsonIndices<minified>();
		}

		JSONIFIER_INLINE std::string resetInternalWithErrorPrintOut(size_type errorIndex) {
			stringBlockReader.reset(currentParseBuffer.data(), currentParseBuffer.size());
			stringIndex = 0;
			tapeIndex	= 0;
			return generateJsonIndicesWithErrorPrintOut(errorIndex);
		}

		template<bool minified> JSONIFIER_INLINE void generateJsonIndices() {
			while (stringBlockReader.hasFullBlock()) {
				generateStructurals<minified, false>(stringBlockReader.fullBlock());
			}
			if (stringBlockReader.getRemainder(block) > 0) [[likely]] {
				generateStructurals<minified, true>(block);
			}
		}

		template<size_type index> JSONIFIER_INLINE size_type rollValuesIntoTape(size_type currentIndex, size_type newBits) {
			static constexpr uint64_t bitTotal{ index * 64ull };
			structuralIndices[0 + (currentIndex * 8) + tapeIndex] = currentParseBuffer.data() + static_cast<uint32_t>(simd_internal::tzcnt(newBits) + bitTotal + stringIndex);
			newBits												  = simd_internal::blsr(newBits);
			structuralIndices[1 + (currentIndex * 8) + tapeIndex] = currentParseBuffer.data() + static_cast<uint32_t>(simd_internal::tzcnt(newBits) + bitTotal + stringIndex);
			newBits												  = simd_internal::blsr(newBits);
			structuralIndices[2 + (currentIndex * 8) + tapeIndex] = currentParseBuffer.data() + static_cast<uint32_t>(simd_internal::tzcnt(newBits) + bitTotal + stringIndex);
			newBits												  = simd_internal::blsr(newBits);
			structuralIndices[3 + (currentIndex * 8) + tapeIndex] = currentParseBuffer.data() + static_cast<uint32_t>(simd_internal::tzcnt(newBits) + bitTotal + stringIndex);
			newBits												  = simd_internal::blsr(newBits);
			structuralIndices[4 + (currentIndex * 8) + tapeIndex] = currentParseBuffer.data() + static_cast<uint32_t>(simd_internal::tzcnt(newBits) + bitTotal + stringIndex);
			newBits												  = simd_internal::blsr(newBits);
			structuralIndices[5 + (currentIndex * 8) + tapeIndex] = currentParseBuffer.data() + static_cast<uint32_t>(simd_internal::tzcnt(newBits) + bitTotal + stringIndex);
			newBits												  = simd_internal::blsr(newBits);
			structuralIndices[6 + (currentIndex * 8) + tapeIndex] = currentParseBuffer.data() + static_cast<uint32_t>(simd_internal::tzcnt(newBits) + bitTotal + stringIndex);
			newBits												  = simd_internal::blsr(newBits);
			structuralIndices[7 + (currentIndex * 8) + tapeIndex] = currentParseBuffer.data() + static_cast<uint32_t>(simd_internal::tzcnt(newBits) + bitTotal + stringIndex);
			newBits												  = simd_internal::blsr(newBits);
			return newBits;
		}

		template<bool collectAligned> JSONIFIER_INLINE void collectStringValues(string_view_ptr values) {
			if constexpr (collectAligned) {
				newPtr[0] = simd_internal::gatherValues<simd_int_t>(values + (BytesPerStep * 0));
				newPtr[1] = simd_internal::gatherValues<simd_int_t>(values + (BytesPerStep * 1));
				newPtr[2] = simd_internal::gatherValues<simd_int_t>(values + (BytesPerStep * 2));
				newPtr[3] = simd_internal::gatherValues<simd_int_t>(values + (BytesPerStep * 3));
				newPtr[4] = simd_internal::gatherValues<simd_int_t>(values + (BytesPerStep * 4));
				newPtr[5] = simd_internal::gatherValues<simd_int_t>(values + (BytesPerStep * 5));
				newPtr[6] = simd_internal::gatherValues<simd_int_t>(values + (BytesPerStep * 6));
				newPtr[7] = simd_internal::gatherValues<simd_int_t>(values + (BytesPerStep * 7));
			} else {
				newPtr[0] = simd_internal::gatherValuesU<simd_int_t>(values + (BytesPerStep * 0));
				newPtr[1] = simd_internal::gatherValuesU<simd_int_t>(values + (BytesPerStep * 1));
				newPtr[2] = simd_internal::gatherValuesU<simd_int_t>(values + (BytesPerStep * 2));
				newPtr[3] = simd_internal::gatherValuesU<simd_int_t>(values + (BytesPerStep * 3));
				newPtr[4] = simd_internal::gatherValuesU<simd_int_t>(values + (BytesPerStep * 4));
				newPtr[5] = simd_internal::gatherValuesU<simd_int_t>(values + (BytesPerStep * 5));
				newPtr[6] = simd_internal::gatherValuesU<simd_int_t>(values + (BytesPerStep * 6));
				newPtr[7] = simd_internal::gatherValuesU<simd_int_t>(values + (BytesPerStep * 7));
			}
		}

		template<bool minified, bool collectAligned> JSONIFIER_INLINE void generateStructurals(string_view_ptr values) {
			collectStringValues<collectAligned>(values);
			rawStructurals = simd_internal::collectIndices<minified>(newPtr);
			collectStructurals<minified>();
			simd_internal::store(rawStructurals.op, newBits);
			addTapeValues();
			stringIndex += BitsPerStep;
		}

		template<uint64_t index = 0> JSONIFIER_INLINE void addTapeValues() {
			if constexpr (index < SixtyFourBitsPerStep) {
				if (!newBits[index]) [[unlikely]] {
					return addTapeValues<index + 1>();
				}
				auto cnt			  = simd_internal::popcnt(newBits[index]);
				size_type rollsAmount = static_cast<size_type>(std::ceil(static_cast<double>(cnt) / 8.0f));
				for (size_type y = 0; y < rollsAmount; ++y) {
					newBits[index] = rollValuesIntoTape<index>(y, newBits[index]);
				}
				tapeIndex += cnt;
				return addTapeValues<index + 1>();
			}
		}

		JSONIFIER_INLINE void collectNonEmptyEscaped() noexcept {
			simd_int_t potentialEscape			 = simd_internal::opAndNot(rawStructurals.backslashes, nextIsEscaped);
			simd_int_t maybeEscaped				 = simd_internal::opShl<1>(potentialEscape);
			simd_int_t maybeEscapedAndOddBits	 = simd_internal::opOr(maybeEscaped, oddBitsVal);
			simd_int_t evenSeriesCodesAndOddBits = simd_internal::opSub(maybeEscapedAndOddBits, potentialEscape);
			simd_int_t escapeAndTerminalCode	 = simd_internal::opXor(evenSeriesCodesAndOddBits, oddBitsVal);
			escaped								 = simd_internal::opXor(escapeAndTerminalCode, simd_internal::opOr(rawStructurals.backslashes, nextIsEscaped));
			nextIsEscaped = simd_internal::opSetLSB(nextIsEscaped, simd_internal::opGetMSB(simd_internal::opAnd(escapeAndTerminalCode, rawStructurals.backslashes)));
		}

		JSONIFIER_INLINE void collectEmptyEscaped() {
			auto escapedNew = nextIsEscaped;
			nextIsEscaped	= simd_internal::reset();
			escaped			= escapedNew;
		}

		JSONIFIER_INLINE void collectEscapedCharacters() {
			return simd_internal::opBool(rawStructurals.backslashes) ? collectNonEmptyEscaped() : collectEmptyEscaped();
		}

		template<bool minified> JSONIFIER_INLINE void collectStructurals() {
			collectEscapedCharacters();
			rawStructurals.quotes = simd_internal::opAndNot(rawStructurals.quotes, escaped);
			simd_int_t inString	  = simd_internal::opClMul(rawStructurals.quotes, prevInString);
			simd_int_t stringTail = simd_internal::opXor(inString, rawStructurals.quotes);
			simd_int_t scalar;
			if constexpr (!minified) {
				scalar = simd_internal::opNot(simd_internal::opOr(rawStructurals.op, rawStructurals.whitespace));
			} else {
				scalar = simd_internal::opNot(rawStructurals.op);
			}
			simd_int_t nonQuoteScalar			= simd_internal::opAndNot(scalar, rawStructurals.quotes);
			simd_int_t followsNonQuoteScalar	= simd_internal::opFollows(nonQuoteScalar, overflow);
			simd_int_t potentialScalarStart		= simd_internal::opAndNot(scalar, followsNonQuoteScalar);
			simd_int_t porentialStructuralStart = simd_internal::opOr(rawStructurals.op, potentialScalarStart);
			rawStructurals.op					= simd_internal::opAndNot(porentialStructuralStart, stringTail);
		}

		JSONIFIER_INLINE std::string collectEscapedCharactersWithErrorPrintOut(size_type errorIndex) {
			if (simd_internal::opBool(rawStructurals.backslashes)) {
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
			collectStringValues(values);
			rawStructurals = simd_internal::collectIndices<true>(newPtr);
			if (stringIndex == errorIndex) {
				returnValue << "Whitespace Bits, for Index: " + std::to_string(stringIndex) + ": ";
				returnValue << printBits(rawStructurals.whitespace).data();
				returnValue << "Backslash Bits, for Index: " + std::to_string(stringIndex) + ": ";
				returnValue << printBits(rawStructurals.backslashes).data();
				returnValue << "Op Bits, for Index: " + std::to_string(stringIndex) + ": ";
				returnValue << printBits(rawStructurals.op).data();
				returnValue << "Quote Bits, for Index: " + std::to_string(stringIndex) + ": ";
				returnValue << printBits(rawStructurals.quotes).data();
			}
			std::string returnValueNew = collectStructuralsWithErrorPrintOut(errorIndex);
			if (stringIndex == errorIndex) {
				returnValue << returnValueNew;
			}
			addTapeValues();
			stringIndex += BitsPerStep;
			return returnValue.str();
		}

		JSONIFIER_INLINE std::string collectNonEmptyEscapedWithErrorPrintOut(size_type errorIndex) noexcept {
			std::stringstream returnValue{};
			simd_int_t potentialEscape			 = simd_internal::opAndNot(rawStructurals.backslashes, nextIsEscaped);
			simd_int_t maybeEscaped				 = simd_internal::opShl<1>(potentialEscape);
			simd_int_t maybeEscapedAndOddBits	 = simd_internal::opOr(maybeEscaped, oddBitsVal);
			simd_int_t evenSeriesCodesAndOddBits = simd_internal::opSub(maybeEscapedAndOddBits, potentialEscape);
			simd_int_t escapeAndTerminalCode	 = simd_internal::opXor(evenSeriesCodesAndOddBits, oddBitsVal);
			escaped								 = simd_internal::opXor(escapeAndTerminalCode, simd_internal::opOr(rawStructurals.backslashes, nextIsEscaped));
			nextIsEscaped = simd_internal::opSetLSB(nextIsEscaped, simd_internal::opGetMSB(simd_internal::opAnd(escapeAndTerminalCode, rawStructurals.backslashes)));
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
			rawStructurals.quotes				= simd_internal::opAndNot(rawStructurals.quotes, escaped);
			simd_int_t inString					= simd_internal::opClMul(rawStructurals.quotes, prevInString);
			simd_int_t stringTail				= simd_internal::opXor(inString, rawStructurals.quotes);
			simd_int_t scalar					= simd_internal::opNot(simd_internal::opOr(rawStructurals.op, rawStructurals.whitespace));
			simd_int_t nonQuoteScalar			= simd_internal::opAndNot(scalar, rawStructurals.quotes);
			simd_int_t followsNonQuoteScalar	= simd_internal::opFollows(nonQuoteScalar, overflow);
			simd_int_t potentialScalarStart		= simd_internal::opAndNot(scalar, followsNonQuoteScalar);
			simd_int_t porentialStructuralStart = simd_internal::opOr(rawStructurals.op, potentialScalarStart);
			rawStructurals.op						= simd_internal::opAndNot(porentialStructuralStart, stringTail);
			if (stringIndex == errorIndex) {
				returnValue << returnValueNew;
				returnValue << "Escaped Bits, for Index: " + std::to_string(stringIndex) + ": ";
				returnValue << printBits(escaped).data();
				returnValue << "Quoted Bits, for Index: " + std::to_string(stringIndex) + ": ";
				returnValue << printBits(rawStructurals.quotes).data();
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
				returnValue << printBits(rawStructurals.op).data();
			}
			return returnValue.str();
		}
	};

};