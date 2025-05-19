/*
	MIT License

	Copyright (c) 2024 RealTimeChris

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
/// Credit to simdjson for the core of the below algorithm: https://github.com/simdjson/simdjson
/// Feb 3, 2023
#pragma once

#include <jsonifier/StringView.hpp>
#include <jsonifier/TypeEntities.hpp>
#include <sstream>
#include <cmath>

namespace jsonifier_internal {

	class string_block_reader {
	  public:
		JSONIFIER_ALWAYS_INLINE void reset(string_view_ptr stringViewNew, uint64_t lengthNew) noexcept {
			lengthMinusStep = lengthNew < bitsPerStep ? 0 : lengthNew - bitsPerStep;
			inString		= stringViewNew;
			length			= lengthNew;
			index			= 0;
		}

		JSONIFIER_ALWAYS_INLINE string_view_ptr getRemainder() noexcept {
			if JSONIFIER_UNLIKELY (length == index) {
				return nullptr;
			}
			std::fill_n(block, bitsPerStep, static_cast<char>(0x20));
			std::copy_n(inString + index, length - index, block);
			return block;
		}

		JSONIFIER_ALWAYS_INLINE string_view_ptr fullBlock() noexcept {
			string_view_ptr newPtr = inString + index;
			index += bitsPerStep;
			return newPtr;
		}

		JSONIFIER_ALWAYS_INLINE bool hasFullBlock() const noexcept {
			return index < lengthMinusStep;
		}

	  protected:
		JSONIFIER_ALIGN char block[bitsPerStep]{};
		string_view_ptr inString{};
		uint64_t lengthMinusStep{};
		uint64_t length{};
		uint64_t index{};
	};

	template<bool doWeUseInitialBuffer> class simd_string_reader : public alloc_wrapper<structural_index> {
	  public:
		using size_type = uint64_t;
		using allocator = alloc_wrapper<structural_index>;
		static constexpr double multiplier{ 4.5f / 5.0f };

		JSONIFIER_ALWAYS_INLINE simd_string_reader() noexcept {
			if constexpr (doWeUseInitialBuffer) {
				resize(1024 * 1024 * 4);
			}
		}

		template<bool minified> JSONIFIER_ALWAYS_INLINE void reset(const void* stringViewNew, size_type size) noexcept {
			currentParseBuffer = jsonifier::string_view_base{ static_cast<string_view_ptr>(stringViewNew), size };
			auto newSize	   = roundUpToMultiple<8ull>(static_cast<size_type>(static_cast<double>(currentParseBuffer.size()) * multiplier));
			if JSONIFIER_UNLIKELY (structuralIndexCount < newSize) {
				resize(newSize * 2);
			}
			resetImpl<minified>();
		}

		JSONIFIER_ALWAYS_INLINE auto end() noexcept {
			return structuralIndices + tapeIndex;
		}

		JSONIFIER_ALWAYS_INLINE auto begin() noexcept {
			structuralIndices[tapeIndex] = currentParseBuffer.data() + currentParseBuffer.size();
			return structuralIndices;
		}

		JSONIFIER_ALWAYS_INLINE ~simd_string_reader() noexcept {
			clear();
		}

	  protected:
		JSONIFIER_ALIGN size_type newBits[sixtyFourBitsPerStep]{};
		jsonifier::string_view currentParseBuffer{};
		string_block_reader stringBlockReader{};
		structural_index* structuralIndices{};
		size_type structuralIndexCount{};
		size_type stringIndex{};
		int64_t prevInString{};
		size_type tapeIndex{};
		bool overflow{};

		template<bool minified> JSONIFIER_ALWAYS_INLINE void resetImpl() noexcept {
			stringBlockReader.reset(currentParseBuffer.data(), currentParseBuffer.size());
			overflow	 = false;
			prevInString = 0;
			stringIndex	 = 0;
			tapeIndex	 = 0;
			generateJsonIndices<minified>();
		}

		JSONIFIER_ALWAYS_INLINE void resize(size_type newSize) noexcept {
			clear();
			structuralIndices	 = allocator::allocate(newSize);
			structuralIndexCount = newSize;
			std::uninitialized_fill(structuralIndices, structuralIndices + structuralIndexCount, nullptr);
		}

		JSONIFIER_ALWAYS_INLINE void clear() noexcept {
			if (structuralIndices) {
				allocator::deallocate(structuralIndices);
				structuralIndices	 = nullptr;
				structuralIndexCount = 0;
			}
		}

		template<bool minified> JSONIFIER_ALWAYS_INLINE void generateJsonIndices() noexcept {
			simd_internal::simd_int_t_holder rawStructurals{};
			jsonifier_simd_int_t nextIsEscaped{};
			jsonifier_simd_int_t escaped{};
			while (stringBlockReader.hasFullBlock()) {
				generateStructurals<false, minified>(stringBlockReader.fullBlock(), escaped, nextIsEscaped, rawStructurals);
			}
			if JSONIFIER_LIKELY (auto newPtr = stringBlockReader.getRemainder(); newPtr) {
				generateStructurals<true, minified>(newPtr, escaped, nextIsEscaped, rawStructurals);
			}
		}

		template<size_type index> JSONIFIER_ALWAYS_INLINE size_type rollValuesIntoTape(size_type currentIndex, size_type newBits) noexcept {
			static constexpr size_type bitTotal{ index * 64ull };
			auto dataPtr										  = currentParseBuffer.data();
			structuralIndices[0 + (currentIndex * 8) + tapeIndex] = dataPtr + static_cast<uint32_t>(simd_internal::tzcnt(newBits) + bitTotal + stringIndex);
			newBits												  = blsr(newBits);
			structuralIndices[1 + (currentIndex * 8) + tapeIndex] = dataPtr + static_cast<uint32_t>(simd_internal::tzcnt(newBits) + bitTotal + stringIndex);
			newBits												  = blsr(newBits);
			structuralIndices[2 + (currentIndex * 8) + tapeIndex] = dataPtr + static_cast<uint32_t>(simd_internal::tzcnt(newBits) + bitTotal + stringIndex);
			newBits												  = blsr(newBits);
			structuralIndices[3 + (currentIndex * 8) + tapeIndex] = dataPtr + static_cast<uint32_t>(simd_internal::tzcnt(newBits) + bitTotal + stringIndex);
			newBits												  = blsr(newBits);
			structuralIndices[4 + (currentIndex * 8) + tapeIndex] = dataPtr + static_cast<uint32_t>(simd_internal::tzcnt(newBits) + bitTotal + stringIndex);
			newBits												  = blsr(newBits);
			structuralIndices[5 + (currentIndex * 8) + tapeIndex] = dataPtr + static_cast<uint32_t>(simd_internal::tzcnt(newBits) + bitTotal + stringIndex);
			newBits												  = blsr(newBits);
			structuralIndices[6 + (currentIndex * 8) + tapeIndex] = dataPtr + static_cast<uint32_t>(simd_internal::tzcnt(newBits) + bitTotal + stringIndex);
			newBits												  = blsr(newBits);
			structuralIndices[7 + (currentIndex * 8) + tapeIndex] = dataPtr + static_cast<uint32_t>(simd_internal::tzcnt(newBits) + bitTotal + stringIndex);
			newBits												  = blsr(newBits);
			return newBits;
		}

		template<size_type currentIndex = 0> JSONIFIER_ALWAYS_INLINE void prefetchStringValues(string_view_ptr values) noexcept {
			if constexpr (currentIndex < sixtyFourBitsPerStep) {
				jsonifierPrefetchImpl(values + (currentIndex * 64));
				prefetchStringValues<currentIndex + 1>(values);
			}
		}

		template<bool collectAligned> JSONIFIER_ALWAYS_INLINE void collectStringValues(string_view_ptr values, jsonifier_simd_int_t (&newPtr)[stridesPerStep]) noexcept {
			if constexpr (collectAligned) {
				newPtr[0] = simd_internal::gatherValues<jsonifier_simd_int_t>(values + (bytesPerStep * 0));
				newPtr[1] = simd_internal::gatherValues<jsonifier_simd_int_t>(values + (bytesPerStep * 1));
				newPtr[2] = simd_internal::gatherValues<jsonifier_simd_int_t>(values + (bytesPerStep * 2));
				newPtr[3] = simd_internal::gatherValues<jsonifier_simd_int_t>(values + (bytesPerStep * 3));
				newPtr[4] = simd_internal::gatherValues<jsonifier_simd_int_t>(values + (bytesPerStep * 4));
				newPtr[5] = simd_internal::gatherValues<jsonifier_simd_int_t>(values + (bytesPerStep * 5));
				newPtr[6] = simd_internal::gatherValues<jsonifier_simd_int_t>(values + (bytesPerStep * 6));
				newPtr[7] = simd_internal::gatherValues<jsonifier_simd_int_t>(values + (bytesPerStep * 7));
			} else {
				newPtr[0] = simd_internal::gatherValuesU<jsonifier_simd_int_t>(values + (bytesPerStep * 0));
				newPtr[1] = simd_internal::gatherValuesU<jsonifier_simd_int_t>(values + (bytesPerStep * 1));
				newPtr[2] = simd_internal::gatherValuesU<jsonifier_simd_int_t>(values + (bytesPerStep * 2));
				newPtr[3] = simd_internal::gatherValuesU<jsonifier_simd_int_t>(values + (bytesPerStep * 3));
				newPtr[4] = simd_internal::gatherValuesU<jsonifier_simd_int_t>(values + (bytesPerStep * 4));
				newPtr[5] = simd_internal::gatherValuesU<jsonifier_simd_int_t>(values + (bytesPerStep * 5));
				newPtr[6] = simd_internal::gatherValuesU<jsonifier_simd_int_t>(values + (bytesPerStep * 6));
				newPtr[7] = simd_internal::gatherValuesU<jsonifier_simd_int_t>(values + (bytesPerStep * 7));
			}
		}

		template<bool collectAligned, bool minified> JSONIFIER_ALWAYS_INLINE simd_internal::simd_int_t_holder getRawIndices(string_view_ptr values) noexcept {
			jsonifier_simd_int_t newPtr[stridesPerStep];
			collectStringValues<collectAligned>(values, newPtr);
			return simd_internal::collectIndices<minified>(newPtr);
		}

		template<bool collectAligned, bool minified> JSONIFIER_ALWAYS_INLINE void generateStructurals(string_view_ptr values, jsonifier_simd_int_t& escaped,
			jsonifier_simd_int_t& nextIsEscaped, simd_internal::simd_int_t_holder& rawStructurals) noexcept {
			rawStructurals = getRawIndices<collectAligned, minified>(values);
			collectStructurals<minified>(escaped, nextIsEscaped, rawStructurals);
			simd_internal::store(rawStructurals.op, newBits);
			addTapeValues();
			stringIndex += bitsPerStep;
		}

		template<size_type index = 0> JSONIFIER_ALWAYS_INLINE void addTapeValues() noexcept {
			if constexpr (index < sixtyFourBitsPerStep) {
				if JSONIFIER_UNLIKELY (!newBits[index]) {
					return addTapeValues<index + 1>();
				}
				auto cnt			  = popcnt(newBits[index]);
				size_type rollsAmount = static_cast<size_type>(std::ceil(static_cast<double>(cnt) / 8.0f));
				for (size_type y = 0; y < rollsAmount; ++y) {
					newBits[index] = rollValuesIntoTape<index>(y, newBits[index]);
				}
				tapeIndex += cnt;
				return addTapeValues<index + 1>();
			}
		}

		JSONIFIER_ALWAYS_INLINE void collectEscaped(jsonifier_simd_int_t& escaped, jsonifier_simd_int_t& nextIsEscaped, simd_internal::simd_int_t_holder& rawStructurals) noexcept {
			const jsonifier_simd_int_t simdValue{ simd_internal::gatherValue<jsonifier_simd_int_t>(static_cast<char>(0xAA)) };
			const jsonifier_simd_int_t potentialEscape = simd_internal::opAndNot(rawStructurals.backslashes, nextIsEscaped);
			jsonifier_simd_int_t maybeEscaped{};
			opShl(1, potentialEscape, maybeEscaped);
			jsonifier_simd_int_t escapeAndTerminalCode = simd_internal::opXor(simd_internal::opSub(simd_internal::opOr(maybeEscaped, simdValue), potentialEscape), simdValue);
			escaped									   = simd_internal::opXor(escapeAndTerminalCode, simd_internal::opOr(rawStructurals.backslashes, nextIsEscaped));
			nextIsEscaped = simd_internal::opSetLSB(nextIsEscaped, simd_internal::opGetMSB(simd_internal::opAnd(escapeAndTerminalCode, rawStructurals.backslashes)));
		}

		JSONIFIER_ALWAYS_INLINE void collectEmptyEscaped(jsonifier_simd_int_t& escaped, jsonifier_simd_int_t& nextIsEscaped) noexcept {
			const auto escapedNew = nextIsEscaped;
			nextIsEscaped		  = jsonifier_simd_int_t{};
			escaped				  = escapedNew;
		}

		JSONIFIER_ALWAYS_INLINE void collectEscapedCharacters(jsonifier_simd_int_t& escaped, jsonifier_simd_int_t& nextIsEscaped,
			simd_internal::simd_int_t_holder& rawStructurals) noexcept {
			return simd_internal::opTest(rawStructurals.backslashes) ? collectEscaped(escaped, nextIsEscaped, rawStructurals) : collectEmptyEscaped(escaped, nextIsEscaped);
		}

		template<bool minified> JSONIFIER_ALWAYS_INLINE void collectStructurals(jsonifier_simd_int_t& escaped, jsonifier_simd_int_t& nextIsEscaped,
			simd_internal::simd_int_t_holder& rawStructurals) noexcept {
			collectEscapedCharacters(escaped, nextIsEscaped, rawStructurals);
			rawStructurals.quotes = simd_internal::opAndNot(rawStructurals.quotes, escaped);
			jsonifier_simd_int_t scalar;
			if constexpr (!minified) {
				scalar = simd_internal::opNot(simd_internal::opOr(rawStructurals.op, rawStructurals.whitespace));
			} else {
				scalar = simd_internal::opNot(rawStructurals.op);
			}
			rawStructurals.op = simd_internal::opAndNot(
				simd_internal::opOr(rawStructurals.op, simd_internal::opAndNot(scalar, simd_internal::opFollows(simd_internal::opAndNot(scalar, rawStructurals.quotes), overflow))),
				simd_internal::opXor(simd_internal::opClMul(rawStructurals.quotes, prevInString), rawStructurals.quotes));
		}
	};

	inline static thread_local jsonifier_internal::simd_string_reader<true> section{};

};