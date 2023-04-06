/*
	Jsonifier - For parsing and serializing Json - very rapidly.
	Copyright (C) 2023 Chris M. (RealTimeChris)

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	This library is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
	Lesser General Public License for more details.

	You should have received a copy of the GNU Lesser General Public
	License along with this library; if not, write to the Free Software
	Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301
	USA
*/
/// https://github.com/RealTimeChris/Jsonifier
/// Feb 3, 2023
#pragma once

#include <jsonifier/StringView.hpp>
#include <jsonifier/String.hpp>
#include <immintrin.h>
#include <iostream>
#include <stdlib.h>
#include <memory>
#include <random>
#include <bitset>

namespace Jsonifier {

	struct SimdBase128 {
	  public:
		inline SimdBase128() noexcept = default;

		inline SimdBase128& operator=(SimdBase128&& other) noexcept = default;
		inline SimdBase128(SimdBase128&& other) noexcept = default;
		inline SimdBase128& operator=(const SimdBase128& other) noexcept = default;
		inline SimdBase128(const SimdBase128& other) noexcept = default;

		inline SimdBase128& operator=(__m128i&& other) noexcept {
			value = std::forward<__m128i>(other);
			return *this;
		}

		inline SimdBase128(__m128i&& other) noexcept {
			*this = std::move(other);
		}

		inline explicit SimdBase128(char other) noexcept {
			value = _mm_set1_epi8(other);
		}

		inline SimdBase128& operator=(const uint8_t values[16]) noexcept {
			value = _mm_loadu_epi8(values);
			return *this;
		}

		inline SimdBase128(const uint8_t values[16]) noexcept {
			*this = values;
		}

		inline operator const __m128i&&() const noexcept {
			return std::forward<const __m128i>(value);
		}

		inline SimdBase128 operator==(uint8_t other) const noexcept {
			return _mm_cmpeq_epi8(value, _mm_set1_epi8(other));
		}

		inline int32_t toBitMask() const noexcept {
			return _mm_movemask_epi8(*this);
		}

		inline void store(uint8_t destVector[16]) const noexcept {
			_mm_storeu_epi8(destVector, value);
		}

		inline void printBits(uint64_t values, const std::string& valuesTitle) const noexcept {
			std::cout << valuesTitle;
			std::cout << std::bitset<64>{ values };
			std::cout << std::endl;
		}

		inline SimdBase128 printBits(const std::string& valuesTitle) const noexcept {
			std::cout << valuesTitle;
			for (size_t x = 0; x < 32; ++x) {
				for (size_t y = 0; y < 8; ++y) {
					std::cout << std::bitset<1>{ static_cast<size_t>(*(reinterpret_cast<const int8_t*>(&value) + x)) >> y };
				}
			}
			std::cout << std::endl;
			return *this;
		}

	  protected:
		__m128i value{};
	};

	struct SimdBase256 {
	  public:
		inline SimdBase256() noexcept = default;

		inline SimdBase256& operator=(SimdBase256&& other) noexcept = default;
		inline SimdBase256(SimdBase256&& other) noexcept = default;
		inline SimdBase256& operator=(const SimdBase256& other) noexcept = default;
		inline SimdBase256(const SimdBase256& other) noexcept = default;

		inline SimdBase256& operator=(__m256i&& other) noexcept {
			value = std::forward<__m256i>(other);
			return *this;
		}

		inline SimdBase256(__m256i&& other) noexcept {
			*this = std::move(other);
		}

		inline SimdBase256& operator=(char other) noexcept {
			value = _mm256_set1_epi8(other);
			return *this;
		}

		inline explicit SimdBase256(char other) noexcept {
			*this = other;
		}

		inline SimdBase256& operator=(StringViewPtr values) noexcept {
			value = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(values));
			return *this;
		}

		inline SimdBase256(StringViewPtr values) noexcept {
			*this = values;
		}

		inline operator __m256i&() {
			return value;
		}

		inline operator const __m256i&&() const noexcept {
			return std::forward<const __m256i>(value);
		}

		inline operator bool() const noexcept {
			return !_mm256_testz_si256(value, value);
		}

		inline SimdBase256 operator|(SimdBase256&& other) noexcept {
			return _mm256_or_si256(value, std::forward<__m256i>(other));
		}

		inline SimdBase256 operator|(const SimdBase256& other) const noexcept {
			return _mm256_or_si256(value, other);
		}

		inline SimdBase256 operator&(const SimdBase256& other) const noexcept {
			return _mm256_and_si256(value, other);
		}

		inline SimdBase256 operator^(const SimdBase256& other) const noexcept {
			return _mm256_xor_si256(value, other);
		}

		inline SimdBase256 operator==(const SimdBase256& other) const noexcept {
			return _mm256_cmpeq_epi8(value, other);
		}

		inline SimdBase256 operator==(const uint8_t& other) const noexcept {
			return _mm256_cmpeq_epi8(value, _mm256_set1_epi8(other));
		}

		inline SimdBase256 operator==(const char& other) const noexcept {
			return _mm256_cmpeq_epi8(value, _mm256_set1_epi8(other));
		}

		inline SimdBase256 operator~() const noexcept {
			return _mm256_xor_si256(*this, _mm256_set1_epi64x(-1ll));
		}

		inline void convertWhitespaceToSimdBase256(const SimdBase256* valuesNew) noexcept {
			uint8_t arrayNew[32]{ ' ', 100, 100, 100, 17, 100, 113, 2, 100, '\t', '\n', 112, 100, '\r', 100, 100, ' ', 100, 100, 100, 17, 100, 113, 2,
				100, '\t', '\n', 112, 100, '\r', 100, 100 };
			SimdBase256 whitespaceTable{ arrayNew };
			for (size_t x = 0; x < 8; ++x) {
				addValues(valuesNew[x].shuffle(whitespaceTable) == valuesNew[x], x);
			}
		};

		inline void convertBackslashesToSimdBase256(const SimdBase256* valuesNew) noexcept {
			SimdBase256 backslashes{ _mm256_set1_epi8('\\') };
			for (size_t x = 0; x < 8; ++x) {
				addValues(valuesNew[x] == backslashes, x);
			}
		};

		inline void convertStructuralsToSimdBase256(const SimdBase256* valuesNew) noexcept {
			uint8_t arrayNew[32]{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, ':', '{', ',', '}', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, ':', '{', ',', '}', 0, 0 };
			SimdBase256 opTable{ arrayNew };
			SimdBase256 chars{ char{ 0x20 } };
			for (size_t x = 0; x < 8; ++x) {
				addValues({ valuesNew[x].shuffle(opTable) == (valuesNew[x] | chars) }, x);
			}
		};

		inline void convertQuotesToSimdBase256(const SimdBase256* valuesNew) noexcept {
			SimdBase256 quotes = _mm256_set1_epi8('"');
			for (size_t x = 0; x < 8; ++x) {
				addValues(valuesNew[x] == quotes, x);
			}
		};

		inline SimdBase256 bitAndNot(SimdBase256&& other) const noexcept {
			return _mm256_andnot_si256(std::forward<__m256i>(other), value);
		}

		inline SimdBase256 bitAndNot(SimdBase256& other) const noexcept {
			return _mm256_andnot_si256(other, value);
		}

		inline SimdBase256 shuffle(SimdBase256& other) const noexcept {
			return _mm256_shuffle_epi8(other, value);
		}

		inline void addValues(SimdBase256&& values, size_t index) noexcept {
			*(reinterpret_cast<int32_t*>(&value) + index) = _mm256_movemask_epi8(values);
		}

		template<size_t amount> inline SimdBase256 shl() const noexcept {
			return SimdBase256{ _mm256_slli_epi64(*this, (amount % 64)) } |
				_mm256_srli_epi64(_mm256_permute4x64_epi64(*this, 0b10010011), 64 - (amount % 64));
		}

		inline int32_t toBitMask() const noexcept {
			return _mm256_movemask_epi8(*this);
		}

		inline void reset() noexcept {
			value = _mm256_setzero_si256();
		}

		inline void store(uint8_t destVector[32]) const noexcept {
			_mm256_storeu_si256(reinterpret_cast<__m256i*>(destVector), value);
		}

		inline void setMSB(bool value) noexcept {
			if (value) {
				*this = _mm256_or_si256(*this, _mm256_set_epi64x(0x8000000000000000, 0, 0, 0));
			} else {
				*this = _mm256_andnot_si256(_mm256_set_epi64x(0x8000000000000000, 0, 0, 0), *this);
			}
		}

		inline void setLSB(bool value) noexcept {
			if (value) {
				*this = _mm256_or_si256(*this, _mm256_set_epi64x(0, 0, 0, 0x1));
			} else {
				*this = _mm256_andnot_si256(_mm256_set_epi64x(0, 0, 0, 0x1), *this);
			}
		}

		inline bool checkLSB() const noexcept {
			__m256i result = _mm256_and_si256(*this, _mm256_set_epi64x(0, 0, 0, 0x01));
			return !_mm256_testz_si256(result, result);
		}

		inline bool checkMSB() const noexcept {
			__m256i result = _mm256_and_si256(*this, _mm256_set_epi64x(0x8000000000000000, 0, 0, 0));
			return !_mm256_testz_si256(result, result);
		}

		inline SimdBase256 carrylessMultiplication(uint64_t& prevInString) const noexcept {
			SimdBase128 allOnes{ '\xFF' };
			SimdBase256 valuesNew{};
			uint64_t* values = reinterpret_cast<uint64_t*>(&valuesNew);
			values[0] = _mm_cvtsi128_si64(_mm_clmulepi64_si128(_mm256_extracti128_si256(value, 0), allOnes, 0)) ^ prevInString;
			prevInString = uint64_t(static_cast<int64_t>(values[0]) >> 63);
			values[1] = _mm_cvtsi128_si64(_mm_clmulepi64_si128(_mm256_extracti128_si256(value, 0), allOnes, 1)) ^ prevInString;
			prevInString = uint64_t(static_cast<int64_t>(values[1]) >> 63);
			values[2] = _mm_cvtsi128_si64(_mm_clmulepi64_si128(_mm256_extracti128_si256(value, 1), allOnes, 0)) ^ prevInString;
			prevInString = uint64_t(static_cast<int64_t>(values[2]) >> 63);
			values[3] = _mm_cvtsi128_si64(_mm_clmulepi64_si128(_mm256_extracti128_si256(value, 1), allOnes, 1)) ^ prevInString;
			prevInString = uint64_t(static_cast<int64_t>(values[3]) >> 63);
			return valuesNew;
		}

		inline bool collectCarries(const SimdBase256& other1, SimdBase256& result) const noexcept {
			__m256i carry = _mm256_set1_epi64x(0 ? 1 : 0);
			__m256i sum = _mm256_add_epi64(*this, other1);
			sum = _mm256_add_epi64(sum, carry);
			result = std::move(sum);
			return _mm256_testz_si256(carry, sum) == 0;
		}

		inline SimdBase256 follows(bool& overflow) const noexcept {
			SimdBase256 result = shl<1>();
			result.setLSB(overflow);
			overflow = checkMSB();
			return result;
		}

		inline void printBits(uint64_t values, const std::string& valuesTitle) const noexcept {
			std::cout << valuesTitle;
			std::cout << std::bitset<64>{ values };
			std::cout << std::endl;
		}

		inline SimdBase256 printBits(const std::string& valuesTitle) const noexcept {
			std::cout << valuesTitle;
			for (size_t x = 0; x < 32; ++x) {
				for (size_t y = 0; y < 8; ++y) {
					std::cout << std::bitset<1>{ static_cast<size_t>(*(reinterpret_cast<const int8_t*>(&value) + x)) >> y };
				}
			}
			std::cout << std::endl;
			return *this;
		}

	  protected:
		__m256i value{};
	};

	template<size_t StepSize> class StringBlockReader {
	  public:
		inline StringBlockReader(StringViewPtr stringViewNew, size_t _len) noexcept {
			lengthMinusStep = _len < StepSize ? 0 : _len - StepSize;
			inString = stringViewNew;
			length = _len;
			index = 0;
		}

		inline size_t getRemainder(StringBufferPtr dst) const noexcept {
			if (length == index) {
				return 0;
			}
			memset(dst, 0x20, StepSize);
			memmove(dst, inString + index, (length - index));
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

		inline void reset(size_t stringLengthRawNew, StringViewPtr stringViewNew) noexcept {
			if (stringLengthRawNew > stringLengthRaw) [[likely]] {
				stringLengthRawAlloc = round(stringLengthRawNew + 3ull, 256ull);
				structuralIndices = std::make_unique<uint32_t[]>(stringLengthRawAlloc);
			} else [[unlikely]] {
				std::memset(structuralIndices.get(), 0, sizeof(uint32_t) * stringLengthRawAlloc);
			}
			stringLengthRaw = stringLengthRawNew;
			stringView = stringViewNew;
			prevEscaped.reset();
			structurals.reset();
			whitespace.reset();
			backslash.reset();
			storedLSB01 = false;
			prevInString = 0;
			stringIndex = 0;
			quotes.reset();
			tapeIndex = 0;
			op.reset();
		}

		inline void generateJsonIndices() noexcept {
			if (stringView) [[likely]] {
				StringBlockReader<256> stringReader{ stringView, stringLengthRaw };
				while (stringReader.hasFullBlock()) {
					generateStructurals(stringReader.fullBlock());
				}
				uint8_t block[256];
				stringReader.getRemainder(block);
				generateStructurals(block);
			}
		}

		inline StringViewPtr getStringView() const noexcept {
			return stringView;
		}

		inline size_t getStringLength() const noexcept {
			return stringLengthRaw;
		}

		inline size_t getTapeLength() const noexcept {
			return tapeIndex;
		}

		inline StructuralIndex getStructurals() const noexcept {
			return structuralIndices.get();
		}

	  protected:
		std::unique_ptr<uint32_t[]> structuralIndices{};
		size_t stringLengthRawAlloc{};
		StringViewPtr stringView{};
		SimdBase256 prevEscaped{};
		SimdBase256 structurals{};
		size_t stringLengthRaw{};
		SimdBase256 whitespace{};
		uint64_t prevInString{};
		SimdBase256 backslash{};
		size_t stringIndex{};
		SimdBase256 quotes{};
		bool storedLSB01{};
		bool storedLSB02{};
		size_t tapeIndex{};
		SimdBase256 op{};

		inline uint64_t round(uint64_t a, uint64_t size) const noexcept {
			return (((a) + (( size )-1)) & ~(( size )-1));
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
			SimdBase256 newPtr[8];
			for (size_t x = 0; x < 8; ++x) {
				newPtr[x] = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(valueNew + (32 * x)));
			}
			whitespace.convertWhitespaceToSimdBase256(newPtr);
			backslash.convertBackslashesToSimdBase256(newPtr);
			op.convertStructuralsToSimdBase256(newPtr);
			quotes.convertQuotesToSimdBase256(newPtr);
			collectStructurals();
			addTapeValues();
			stringIndex += 256;
		}

		inline void addTapeValues() noexcept {
			alignas(32) int64_t newBits[4];
			_mm256_store_si256(reinterpret_cast<__m256i*>(newBits), structurals);
			for (size_t x = 0; x < 4; ++x) {
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

		inline void collectStructurals() noexcept {
			SimdBase256 escaped{};
			if (backslash) {
				backslash = backslash.bitAndNot(prevEscaped);
				SimdBase256 followsEscape = backslash.shl<1>() | prevEscaped;
				auto newStoredLSB = storedLSB02;
				storedLSB02 = followsEscape.checkLSB();
				followsEscape.setLSB(newStoredLSB);
				SimdBase256 evenBits{ _mm256_set1_epi8(0xAA) };
				SimdBase256 oddSequenceStarts = backslash.bitAndNot(evenBits.bitAndNot(followsEscape));
				SimdBase256 sequencesStartingOnEvenBits{};
				prevEscaped.setLSB(oddSequenceStarts.collectCarries(backslash, sequencesStartingOnEvenBits));
				SimdBase256 invertMask = sequencesStartingOnEvenBits.shl<1>();
				escaped = (evenBits ^ invertMask) & followsEscape;
				escaped.setLSB(followsEscape.checkLSB());
			} else {
				escaped = prevEscaped;
				prevEscaped.reset();
			}
			quotes = quotes.bitAndNot(escaped);
			SimdBase256 inString = quotes.carrylessMultiplication(prevInString);
			SimdBase256 scalar = ~(op | whitespace);
			SimdBase256 nonQuoteScalar = scalar.bitAndNot(quotes);
			SimdBase256 stringTail = inString ^ quotes;
			SimdBase256 followsNonQuoteScalar = nonQuoteScalar.follows(storedLSB01);
			SimdBase256 potentialScalarStart = scalar.bitAndNot(followsNonQuoteScalar);
			SimdBase256 potentialStructuralStart = op | potentialScalarStart;
			structurals = potentialStructuralStart.bitAndNot(stringTail);
		}
	};

}