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
		inline StringBlockReader(StringViewPtr stringViewNew, size_t lengthNew) noexcept {
			lengthMinusStep = lengthNew < StepSize ? 0 : lengthNew - StepSize;
			inString = stringViewNew;
			length = lengthNew;
			index = 0;
		}

		inline size_t getRemainder(StringBufferPtr dest) const noexcept {
			if (length == index) {
				return 0;
			}
			memset(dest, 0x20, StepSize);
			memmove(dest, inString + index, (length - index));
			return length - index;
		}

		inline StringViewPtr fullBlock() noexcept {
			StringViewPtr newPtr = inString + index;
			index += StepSize;
			return newPtr;
		}

		inline bool hasFullBlock() const noexcept {
			return index < lengthMinusStep;
		}

	  protected:
		StringViewPtr inString{};
		size_t lengthMinusStep{};
		size_t length{};
		size_t index{};
	};

	class SimdStringReader {
	  public:
		inline SimdStringReader() noexcept = default;
		inline SimdStringReader& operator=(SimdStringReader&& other) noexcept = default;
		inline SimdStringReader(SimdStringReader&& other) noexcept = default;
		inline SimdStringReader& operator=(const SimdStringReader& other) noexcept = delete;
		inline SimdStringReader(const SimdStringReader& other) noexcept = delete;

		inline void reset(Jsonifier::StringView stringViewNew) noexcept {
			size_t sizeNew{ roundUpToMultipleOfEight(stringViewNew.size()) };
			if (sizeNew > structuralIndices.size()) {
				structuralIndices.resize(sizeNew);
			}
			structuralIndices.reset();
			stringView = stringViewNew;
			storedLSB01 = false;
			storedLSB02 = false;
			prevEscaped.reset();
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
				StringBlockReader stringReader{ reinterpret_cast<StringViewPtr>(stringView.data()), stringView.size() };
				while (stringReader.hasFullBlock()) {
					generateStructurals(stringReader.fullBlock());
				}
				uint8_t block[StepSize];
				stringReader.getRemainder(block);
				generateStructurals(block);
			}
		}

		inline Jsonifier::StringView& getStringView() noexcept {
			return stringView;
		}

		inline size_t getStringLength() const noexcept {
			return stringView.size();
		}

		inline StructuralIndex getStructurals() noexcept {
			return structuralIndices.data();
		}

		inline ~SimdStringReader() noexcept {};

	  protected:
		Jsonifier::Vector<uint32_t> structuralIndices{};
		Jsonifier::StringView stringView{};
		SimdBaseReal prevEscaped{};
		SimdBaseReal structurals{};
		SimdBaseReal whitespace{};
		SimdBaseReal backslash{};
		uint64_t prevInString{};
		SimdBaseReal quotes{};
		size_t stringIndex{};
		bool storedLSB01{};
		bool storedLSB02{};
		size_t tapeIndex{};
		SimdBaseReal op{};

		size_t roundUpToMultipleOfEight(size_t num) {
			size_t remainder = num % 8;
			if (remainder == 0) {
				return num;
			}
			return num + (8 - remainder);
		}

		inline int64_t rollValuesIntoTape(size_t currentIndex, size_t x, int64_t newBits) noexcept {
			structuralIndices[(currentIndex * 8) + tapeIndex] = static_cast<uint32_t>(_tzcnt_u64(newBits) + (x * 64ull) + stringIndex);
			newBits = _blsr_u64(newBits);
			structuralIndices[1 + (currentIndex * 8) + tapeIndex] = static_cast<uint32_t>(_tzcnt_u64(newBits) + (x * 64ull) + stringIndex);
			newBits = _blsr_u64(newBits);
			structuralIndices[2 + (currentIndex * 8) + tapeIndex] = static_cast<uint32_t>(_tzcnt_u64(newBits) + (x * 64ull) + stringIndex);
			newBits = _blsr_u64(newBits);
			structuralIndices[3 + (currentIndex * 8) + tapeIndex] = static_cast<uint32_t>(_tzcnt_u64(newBits) + (x * 64ull) + stringIndex);
			newBits = _blsr_u64(newBits);
			structuralIndices[4 + (currentIndex * 8) + tapeIndex] = static_cast<uint32_t>(_tzcnt_u64(newBits) + (x * 64ull) + stringIndex);
			newBits = _blsr_u64(newBits);
			structuralIndices[5 + (currentIndex * 8) + tapeIndex] = static_cast<uint32_t>(_tzcnt_u64(newBits) + (x * 64ull) + stringIndex);
			newBits = _blsr_u64(newBits);
			structuralIndices[6 + (currentIndex * 8) + tapeIndex] = static_cast<uint32_t>(_tzcnt_u64(newBits) + (x * 64ull) + stringIndex);
			newBits = _blsr_u64(newBits);
			structuralIndices[7 + (currentIndex * 8) + tapeIndex] = static_cast<uint32_t>(_tzcnt_u64(newBits) + (x * 64ull) + stringIndex);
			newBits = _blsr_u64(newBits);
			return newBits;
		}

		inline void generateStructurals(StringViewPtr valueNew) noexcept {
			SimdBaseReal newPtr[8];
			for (size_t x = 0; x < 8; ++x) {
				newPtr[x] = loadu(reinterpret_cast<const AvxType*>(valueNew + (BytesPerStep * x)));
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
			alignas(64) int64_t newBits[SixtyFourPer];
			store(reinterpret_cast<AvxType*>(newBits), structurals);
			for (size_t x = 0; x < SixtyFourPer; ++x) {
				if (!newBits[x]) {
					continue;
				}
				auto cnt = _mm_popcnt_u64(newBits[x]);
				size_t rollsAmount = static_cast<size_t>(ceil(static_cast<float>(cnt) / 8.0f));
				for (size_t y = 0; y < rollsAmount; ++y) {
					newBits[x] = rollValuesIntoTape(y, x, newBits[x]);
				}
				tapeIndex += cnt;
			}
		}

		inline SimdBaseReal collectEscapedCharacters() {
			if (backslash) {
				SimdBaseReal followsEscape = backslash.shl<1>() | prevEscaped;
				bool newStoredLSB = storedLSB02;
				storedLSB02 = followsEscape.checkLSB();
				SimdBaseReal evenBits{ set(0b01010101) };
				SimdBaseReal oddSequenceStarts = backslash.bitAndNot(evenBits.bitAndNot(followsEscape));
				SimdBaseReal sequencesStartingOnEvenBits{};
				prevEscaped.setLSB(oddSequenceStarts.collectCarries(backslash, sequencesStartingOnEvenBits));
				SimdBaseReal invertMask = sequencesStartingOnEvenBits.shl<1>();
				return ((evenBits ^ invertMask) & followsEscape).setLSB(newStoredLSB);
			} else {
				SimdBaseReal escaped = prevEscaped;
				prevEscaped.reset();
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

#ifdef loadu
	#undef loadu
#endif

#ifdef set
	#undef set
#endif
