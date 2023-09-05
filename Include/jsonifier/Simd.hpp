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
	INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
	PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE
	FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
	OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
	DEALINGS IN THE SOFTWARE.
*/
/// https://github.com/RealTimeChris/Jsonifier
/// Feb 3, 2023
#pragma once

#include <jsonifier/StringView.hpp>
#include <jsonifier/ISADetection.hpp>

namespace JsonifierInternal {

	class StringBlockReader {
	  public:
		inline StringBlockReader(const uint8_t* stringViewNew, uint64_t lengthNew) noexcept {
			lengthMinusStep = lengthNew < StepSize ? 0 : lengthNew - StepSize;
			inString		= stringViewNew;
			length			= lengthNew;
			index			= 0;
		}

		inline uint64_t getRemainder(StringBufferPtr dest) const noexcept {
			if (length == index) {
				return 0;
			}
			std::memset(dest, 0x20, StepSize * 2);
			std::memcpy(dest, inString + index, (length - index));
			return length - index;
		}

		inline StringViewPtr fullBlock() noexcept {
			const uint8_t* newPtr = inString + index;
			return newPtr;
		}

		inline void advanceIndex() {
			index += StepSize * 2;
		}

		inline bool hasFullBlock() const noexcept {
			return index + StepSize < lengthMinusStep;
		}

	  protected:
		const uint8_t* inString{};
		uint64_t lengthMinusStep{};
		uint64_t length{};
		uint64_t index{};
	};

	class SimdStringReader {
	  public:
		using size_type = uint64_t;

		inline SimdStringReader() noexcept										   = default;
		inline SimdStringReader& operator=(SimdStringReader&& other) noexcept	   = default;
		inline SimdStringReader(SimdStringReader&& other) noexcept				   = default;
		inline SimdStringReader& operator=(const SimdStringReader& other) noexcept = delete;
		inline SimdStringReader(const SimdStringReader& other) noexcept			   = delete;

		inline void reset(Jsonifier::StringViewBase<uint8_t> stringViewNew) noexcept {
			structuralIndices.resize(roundUpToMultipleOfEight(stringViewNew.size()));
			stringView = stringViewNew;
			nextIsEscaped.reset();
			storedLSB01 = false;
			whitespace.reset();
			backslash.reset();
			prevInString = 0;
			stringIndex	 = 0;
			quotes.reset();
			tapeIndex = 0;
			op.reset();
			generateJsonIndices();
		}

		inline void generateJsonIndices() noexcept {
			if (!stringView.empty()) [[likely]] {
				StringBlockReader stringReader{ stringView.data(), stringView.size() };
				while (stringReader.hasFullBlock()) {
					generateStructurals(stringReader.fullBlock());
					generateStructurals(stringReader.fullBlock() + StepSize);
					stringReader.advanceIndex();
				}
				uint8_t block[StepSize * 2];
				stringReader.getRemainder(block);
				generateStructurals(block);
				generateStructurals(block + StepSize);
			}
		}

		inline size_type getStringLength() noexcept {
			return stringView.size();
		}

		inline Jsonifier::StringViewBase<uint8_t> getStringView() noexcept {
			return stringView;
		}

		inline StructuralIndex* getStructurals() noexcept {
			return structuralIndices.data();
		}

		inline ~SimdStringReader() noexcept {};

	  protected:
		inline static const SimdBaseReal oddBits{ makeSimdBase(0xAAAAAAAAAAAAAAAAULL) };
		Jsonifier::Vector<uint32_t> structuralIndices{};
		Jsonifier::StringViewBase<uint8_t> stringView{};
		SimdBaseReal nextIsEscaped{};
		SimdBaseReal whitespace{};
		SimdBaseReal backslash{};
		uint64_t prevInString{};
		uint64_t stringIndex{};
		SimdBaseReal quotes{};
		uint64_t tapeIndex{};
		bool storedLSB01{};
		SimdBaseReal op{};

		inline uint64_t roundUpToMultipleOfEight(uint64_t num) {
			uint64_t remainder = num % 8;
			if (remainder == 0) {
				return num;
			}
			return num + (8 - remainder);
		}

		inline int64_t rollValuesIntoTape(uint64_t currentIndex, uint64_t x, int64_t newBits) noexcept {
			structuralIndices[(currentIndex * 8) + tapeIndex]	  = static_cast<uint32_t>(tzCount64(newBits) + (x * 64ull) + stringIndex);
			newBits												  = blsr(newBits);
			structuralIndices[1 + (currentIndex * 8) + tapeIndex] = static_cast<uint32_t>(tzCount64(newBits) + (x * 64ull) + stringIndex);
			newBits												  = blsr(newBits);
			structuralIndices[2 + (currentIndex * 8) + tapeIndex] = static_cast<uint32_t>(tzCount64(newBits) + (x * 64ull) + stringIndex);
			newBits												  = blsr(newBits);
			structuralIndices[3 + (currentIndex * 8) + tapeIndex] = static_cast<uint32_t>(tzCount64(newBits) + (x * 64ull) + stringIndex);
			newBits												  = blsr(newBits);
			structuralIndices[4 + (currentIndex * 8) + tapeIndex] = static_cast<uint32_t>(tzCount64(newBits) + (x * 64ull) + stringIndex);
			newBits												  = blsr(newBits);
			structuralIndices[5 + (currentIndex * 8) + tapeIndex] = static_cast<uint32_t>(tzCount64(newBits) + (x * 64ull) + stringIndex);
			newBits												  = blsr(newBits);
			structuralIndices[6 + (currentIndex * 8) + tapeIndex] = static_cast<uint32_t>(tzCount64(newBits) + (x * 64ull) + stringIndex);
			newBits												  = blsr(newBits);
			structuralIndices[7 + (currentIndex * 8) + tapeIndex] = static_cast<uint32_t>(tzCount64(newBits) + (x * 64ull) + stringIndex);
			newBits												  = blsr(newBits);
			return newBits;
		}

		inline void generateStructurals(StringViewPtr valueNew) noexcept {
			SimdBaseReal newPtr[8]{};
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

		inline void addTapeValues(SimdBaseReal structurals) noexcept {
			alignas(ALIGNMENT) int64_t newBits[SixtyFourPer]{};
			structurals.store(newBits);
			for (uint64_t x = 0; x < SixtyFourPer; ++x) {
				if (!newBits[x]) {
					continue;
				}
				auto cnt			 = popcnt(newBits[x]);
				uint64_t rollsAmount = static_cast<uint64_t>(ceil(static_cast<float>(cnt) / 8.0f));
				for (uint64_t y = 0; y < rollsAmount; ++y) {
					newBits[x] = rollValuesIntoTape(y, x, newBits[x]);
				}
				tapeIndex += cnt;
			}
		}

		inline SimdBaseReal collectEscapedCharacters() {
			if (backslash.operator bool()) {
				SimdBaseReal currentValues = backslash.bitAndNot(nextIsEscaped).shl<1>();
				currentValues			   = currentValues | oddBits;
				currentValues			   = currentValues - backslash.bitAndNot(nextIsEscaped);
				currentValues			   = currentValues ^ oddBits;
				SimdBaseReal escaped	   = currentValues ^ (backslash | nextIsEscaped);
				SimdBaseReal escape		   = currentValues & backslash;
				nextIsEscaped.setLSB(escape.checkMSB());
				return escaped;

			} else {
				return {};
			}
		}

		inline SimdBaseReal collectStructurals() noexcept {
			SimdBaseReal currentValues = collectEscapedCharacters();
			quotes					   = quotes.bitAndNot(currentValues);
			currentValues			   = quotes.carrylessMultiplication(prevInString);
			SimdBaseReal stringTail	   = currentValues ^ quotes;
			SimdBaseReal scalar		   = ~(op | whitespace);
			currentValues			   = scalar.bitAndNot(quotes);
			currentValues			   = currentValues.follows(storedLSB01);
			currentValues			   = scalar.bitAndNot(currentValues);
			currentValues			   = op | currentValues;
			return currentValues.bitAndNot(stringTail);
		}
	};

}

#ifdef store
	#undef store
#endif

#ifdef load
	#undef load
#endif

#ifdef set
	#undef set
#endif
