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
		inline StringBlockReader(const uint8_t* stringViewNew, uint64_t lengthNew) noexcept : inString{ stringViewNew } {
			lengthMinusStep = lengthNew < StepSize ? 0 : lengthNew - StepSize;
			inString = stringViewNew;
			length = lengthNew;
			index = 0;
		}

		inline uint64_t getRemainder(StringBufferPtr dest) const noexcept {
			if (length == index) {
				return 0;
			}
			std::memset(dest, 0x20, StepSize);
			std::memcpy(dest, inString + index, (length - index));
			return length - index;
		}

		inline StringViewPtr fullBlock() noexcept {
			const uint8_t* newPtr = inString + index;
			index += StepSize;
			return newPtr;
		}

		inline bool hasFullBlock() const noexcept {
			return index < lengthMinusStep;
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

		inline SimdStringReader() noexcept = default;
		inline SimdStringReader& operator=(SimdStringReader&& other) noexcept = default;
		inline SimdStringReader(SimdStringReader&& other) noexcept = default;
		inline SimdStringReader& operator=(const SimdStringReader& other) noexcept = delete;
		inline SimdStringReader(const SimdStringReader& other) noexcept = delete;

		inline void reset(Jsonifier::StringViewBase<uint8_t> stringViewNew) noexcept {
			uint64_t sizeNew{ roundUpToMultipleOfEight(stringViewNew.size()) };
			structuralIndices.resize(sizeNew);
			stringView = stringViewNew;
			storedLSB01 = false;
			prevEscaped = false;
			structurals.reset();
			whitespace.reset();
			backslash.reset();
			prevInString = 0;
			stringIndex = 0;
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
				}
				uint8_t block[StepSize];
				stringReader.getRemainder(block);
				generateStructurals(block);
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
		Jsonifier::Vector<StructuralIndex> structuralIndices{};
		Jsonifier::StringViewBase<uint8_t> stringView{};
		SimdBaseReal structurals{};
		SimdBaseReal whitespace{};
		SimdBaseReal backslash{};
		uint64_t prevInString{};
		uint64_t stringIndex{};
		SimdBaseReal quotes{};
		uint64_t tapeIndex{};
		bool prevEscaped{};
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
			structuralIndices[(currentIndex * 8) + tapeIndex] =
				stringView.data() + static_cast<uint32_t>(_tzcnt_u64(newBits) + (x * 64ull) + stringIndex);
			newBits = _blsr_u64(newBits);
			structuralIndices[1 + (currentIndex * 8) + tapeIndex] =
				stringView.data() + static_cast<uint32_t>(_tzcnt_u64(newBits) + (x * 64ull) + stringIndex);
			newBits = _blsr_u64(newBits);
			structuralIndices[2 + (currentIndex * 8) + tapeIndex] =
				stringView.data() + static_cast<uint32_t>(_tzcnt_u64(newBits) + (x * 64ull) + stringIndex);
			newBits = _blsr_u64(newBits);
			structuralIndices[3 + (currentIndex * 8) + tapeIndex] =
				stringView.data() + static_cast<uint32_t>(_tzcnt_u64(newBits) + (x * 64ull) + stringIndex);
			newBits = _blsr_u64(newBits);
			structuralIndices[4 + (currentIndex * 8) + tapeIndex] =
				stringView.data() + static_cast<uint32_t>(_tzcnt_u64(newBits) + (x * 64ull) + stringIndex);
			newBits = _blsr_u64(newBits);
			structuralIndices[5 + (currentIndex * 8) + tapeIndex] =
				stringView.data() + static_cast<uint32_t>(_tzcnt_u64(newBits) + (x * 64ull) + stringIndex);
			newBits = _blsr_u64(newBits);
			structuralIndices[6 + (currentIndex * 8) + tapeIndex] =
				stringView.data() + static_cast<uint32_t>(_tzcnt_u64(newBits) + (x * 64ull) + stringIndex);
			newBits = _blsr_u64(newBits);
			structuralIndices[7 + (currentIndex * 8) + tapeIndex] =
				stringView.data() + static_cast<uint32_t>(_tzcnt_u64(newBits) + (x * 64ull) + stringIndex);
			newBits = _blsr_u64(newBits);
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
			collectStructurals();
			addTapeValues();
			stringIndex += StepSize;
		}

		inline void addTapeValues() noexcept {
			alignas(64) int64_t newBits[SixtyFourPer]{};
			structurals.store(newBits);
			for (uint64_t x = 0; x < SixtyFourPer; ++x) {
				if (!newBits[x]) {
					continue;
				}
				auto cnt = _mm_popcnt_u64(newBits[x]);
				uint64_t rollsAmount = static_cast<uint64_t>(ceil(static_cast<float>(cnt) / 8.0f));
				for (uint64_t y = 0; y < rollsAmount; ++y) {
					newBits[x] = rollValuesIntoTape(y, x, newBits[x]);
				}
				tapeIndex += cnt;
			}
		}

		inline SimdBaseReal collectEscapedCharacters() {
			if (backslash.operator bool()) {
				auto newPrevEscaped = prevEscaped;
				SimdBaseReal followsEscape = backslash.shl<1>();
				SimdBaseReal evenBits{ set(0b01010101) };
				SimdBaseReal oddSequenceStarts = backslash.bitAndNot(evenBits.bitAndNot(followsEscape));
				SimdBaseReal sequencesStartingOnEvenBits{};
				prevEscaped = oddSequenceStarts.collectCarries(backslash, sequencesStartingOnEvenBits);
				SimdBaseReal invertMask = sequencesStartingOnEvenBits.shl<1>();
				return std::move(((evenBits ^ invertMask) & followsEscape).setLSB(newPrevEscaped));
			} else {
				SimdBaseReal escaped{};
				escaped.setLSB(prevEscaped);
				prevEscaped = false;
				return escaped;
			}
		}

		inline void collectStructurals() noexcept {
			SimdBaseReal escaped = collectEscapedCharacters();
			quotes = quotes.bitAndNot(escaped);
			SimdBaseReal inString = quotes.carrylessMultiplication(prevInString);
			SimdBaseReal scalar = ~(op | whitespace);
			SimdBaseReal nonQuoteScalar = scalar.bitAndNot(quotes);
			SimdBaseReal stringTail = inString ^ quotes;
			SimdBaseReal followsNonQuoteScalar = nonQuoteScalar.follows(storedLSB01);
			SimdBaseReal potentialScalarStart = scalar.bitAndNot(followsNonQuoteScalar);
			SimdBaseReal potentialStructuralStart = op | potentialScalarStart;
			structurals = potentialStructuralStart.bitAndNot(stringTail);
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
