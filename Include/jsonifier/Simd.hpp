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

#include "Base.hpp"

namespace Jsonifier {

	class Jsonifier_Dll SimdBase128 {
	  public:
		__forceinline SimdBase128() noexcept = default;

		__forceinline SimdBase128& operator=(const char other) noexcept {
			value = _mm_set1_epi8(other);
			return *this;
		}

		__forceinline SimdBase128(const char other) noexcept {
			*this = other;
		}

		__forceinline operator __m128i&() noexcept {
			return value;
		}

	  protected:
		__m128i value{};
	};

	class Jsonifier_Dll SimdBase256 {
	  public:
		__forceinline SimdBase256() noexcept {};

		__forceinline SimdBase256& operator=(__m256i&& other) noexcept {
			value = std::forward<__m256i>(other);
			return *this;
		}

		__forceinline SimdBase256(__m256i&& other) noexcept {
			*this = std::move(other);
		}

		__forceinline SimdBase256& operator=(char other) noexcept {
			value = _mm256_set1_epi8(other);
			return *this;
		}

		__forceinline SimdBase256(char other) noexcept {
			*this = other;
		}

		__forceinline SimdBase256& operator=(const uint8_t* values) noexcept {
			value = _mm256_load_si256(reinterpret_cast<const __m256i*>(values));
			return *this;
		}

		__forceinline SimdBase256(const uint8_t* values) noexcept {
			*this = values;
		}

		__forceinline SimdBase256& operator=(uint64_t* values) noexcept {
			insertInt64(values[0], 0);
			insertInt64(values[1], 1);
			insertInt64(values[2], 2);
			insertInt64(values[3], 3);
			return *this;
		}

		__forceinline SimdBase256(uint64_t* values) noexcept {
			*this = values;
		}

		__forceinline operator const __m256i&&() const noexcept {
			return std::forward<const __m256i>(value);
		}

		__forceinline SimdBase256 operator|(SimdBase256&& other) noexcept {
			return _mm256_or_si256(value, std::forward<SimdBase256>(other));
		}

		__forceinline SimdBase256 operator|(SimdBase256& other) const noexcept {
			return _mm256_or_si256(value, other);
		}

		__forceinline SimdBase256 operator&(SimdBase256& other) noexcept {
			return _mm256_and_si256(value, other);
		}

		__forceinline SimdBase256 operator^(SimdBase256& other) noexcept {
			return _mm256_xor_si256(value, other);
		}

		__forceinline SimdBase256 operator==(const SimdBase256& other) const noexcept {
			return _mm256_cmpeq_epi8(value, other);
		}

		__forceinline SimdBase256 operator==(uint8_t other) noexcept {
			return _mm256_cmpeq_epi8(value, _mm256_set1_epi8(other));
		}

		__forceinline SimdBase256 operator~() noexcept {
			return _mm256_xor_si256(*this, _mm256_set1_epi64x(-1ull));
		}

		__forceinline void convertWhitespaceToSimdBase256(const SimdBase256* valuesNew) noexcept {
			SimdBase256 whitespaceTable{ { ' ', 100, 100, 100, 17, 100, 113, 2, 100, '\t', '\n', 112, 100, '\r', 100, 100, ' ', 100, 100, 100, 17,
				100, 113, 2, 100, '\t', '\n', 112, 100, '\r', 100, 100 } };
			for (size_t x = 0; x < 8; ++x) {
				addValues(valuesNew[x].shuffle(whitespaceTable) == valuesNew[x], x);
			}
		};

		__forceinline void convertBackslashesToSimdBase256(const SimdBase256* valuesNew) noexcept {
			SimdBase256 backslashes{ _mm256_set1_epi8('\\') };
			for (size_t x = 0; x < 8; ++x) {
				addValues(valuesNew[x] == backslashes, x);
			}
		};

		__forceinline void convertStructuralsToSimdBase256(const SimdBase256* valuesNew) noexcept {
			SimdBase256 opTable{ { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, ':', '{', ',', '}', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, ':', '{', ',', '}', 0, 0 } };
			SimdBase256 chars{ char{ 0x20 } };
			for (size_t x = 0; x < 8; ++x) {
				addValues({ valuesNew[x].shuffle(opTable) == (valuesNew[x] | chars) }, x);
			}
		};

		__forceinline void convertQuotesToSimdBase256(const SimdBase256* valuesNew) noexcept {
			SimdBase256 quotes = _mm256_set1_epi8('"');
			for (size_t x = 0; x < 8; ++x) {
				addValues(valuesNew[x] == quotes, x);
			}
		};

		__forceinline SimdBase256 bitAndNot(SimdBase256&& other) noexcept {
			return _mm256_andnot_si256(std::forward<SimdBase256>(other), value);
		}

		__forceinline SimdBase256 bitAndNot(SimdBase256& other) noexcept {
			return _mm256_andnot_si256(other, value);
		}

		__forceinline SimdBase256 shuffle(SimdBase256& other) const noexcept {
			return _mm256_shuffle_epi8(other, value);
		}

		__forceinline void addValues(const SimdBase256&& values, size_t index) noexcept {
			*(reinterpret_cast<int32_t*>(&value) + index) = _mm256_movemask_epi8(values);
		}

		template<size_t amount> __forceinline SimdBase256 shl() noexcept {
			return SimdBase256{ _mm256_slli_epi64(*this, (amount % 64)) } |
				_mm256_srli_epi64(_mm256_permute4x64_epi64(*this, 0b10010011), 64 - (amount % 64));
		}

		__forceinline int32_t toBitMask() noexcept {
			return _mm256_movemask_epi8(*this);
		}

		__forceinline void store(uint8_t dst[32]) noexcept {
			_mm256_store_si256(reinterpret_cast<__m256i*>(dst), value);
		}

		__forceinline int64_t getInt64(size_t index) noexcept {
			switch (index) {
				case 0: {
					return _mm256_extract_epi64(value, 0);
				}
				case 1: {
					return _mm256_extract_epi64(value, 1);
				}
				case 2: {
					return _mm256_extract_epi64(value, 2);
				}
				case 3: {
					return _mm256_extract_epi64(value, 3);
				}
				default: {
					return _mm256_extract_epi64(value, 0);
				}
			}
		}

		__forceinline void insertInt64(int64_t valueNew, size_t index) noexcept {
			switch (index) {
				case 0: {
					value = _mm256_insert_epi64(value, valueNew, 0);
					break;
				}
				case 1: {
					value = _mm256_insert_epi64(value, valueNew, 1);
					break;
				}
				case 2: {
					value = _mm256_insert_epi64(value, valueNew, 2);
					break;
				}
				case 3: {
					value = _mm256_insert_epi64(value, valueNew, 3);
					break;
				}
				default: {
					value = _mm256_insert_epi64(value, valueNew, 0);
					break;
				}
			}
		}

		__forceinline void setFirstBit(bool onOrOff) noexcept {
			if (onOrOff) {
				insertInt64(getInt64(0) | 1ll, 0);
			} else {
				insertInt64(getInt64(0) & ~1ll, 0);
			}
		}

		__forceinline bool checkLastBit() noexcept {
			return (getInt64(3) >> 63 & 1) << 63;
		}

		__forceinline SimdBase256 carrylessMultiplication(uint64_t& prevInString) noexcept {
			SimdBase128 allOnes{ '\xFF' };
			SimdBase256 valuesNew{};
			auto values = reinterpret_cast<uint64_t*>(&valuesNew);
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

		__forceinline bool collectCarries(SimdBase256& other1, SimdBase256& result) noexcept {
			bool returnValue{};
			uint64_t returnValue64{};
			for (size_t x = 0; x < 4; ++x) {
				if (_addcarry_u64(0, getInt64(x), other1.getInt64(x), &returnValue64)) {
					returnValue = true;
				} else {
					returnValue = false;
				}
				result.insertInt64(returnValue64, x);
			}
			return returnValue;
		}

		__forceinline void printBits(uint64_t values, const std::string& valuesTitle) noexcept {
			using std::cout;
			cout << valuesTitle;
			cout << std::bitset<64>{ values };
			cout << std::endl;
		}

		__forceinline SimdBase256 printBits(const std::string& valuesTitle) noexcept {
			using std::cout;
			cout << valuesTitle;
			for (size_t x = 0; x < 32; ++x) {
				for (size_t y = 0; y < 8; ++y) {
					cout << std::bitset<1>{ static_cast<size_t>(*(reinterpret_cast<int8_t*>(&value) + x)) >> y };
				}
			}
			cout << std::endl;
			return std::move(*this);
		}

	  protected:
		__m256i value{};
	};

	template<size_t StepSize> class StringBlockReader {
	  public:
		__forceinline StringBlockReader(InStringPtr stringViewNew, size_t _len) noexcept {
			lengthMinusStep = _len < StepSize ? 0 : _len - StepSize;
			inString = stringViewNew;
			length = _len;
			index = 0;
		}

		__forceinline size_t getRemainder(OutStringPtr dst) noexcept {
			if (length == index) {
				return 0;
			}
			memset(dst, 0x20, StepSize);
			memmove(dst, inString + index, (length - index));
			return length - index;
		}

		__forceinline InStringPtr fullBlock() noexcept {
			auto newPtr = inString + index;
			index += StepSize;
			return newPtr;
		}

		__forceinline bool hasFullBlock() noexcept {
			return index < lengthMinusStep;
		}

	  protected:
		InStringPtr inString{};
		size_t lengthMinusStep{};
		size_t length{};
		size_t index{};
	};

	class Jsonifier_Dll SimdStringReader {
	  public:
		__forceinline SimdStringReader(StructuralIndex tapePtrsNew) noexcept : tapePtrs{ tapePtrsNew } {};

		__forceinline void generateStructurals(InStringPtr valueNew) noexcept {
			auto newPtr = reinterpret_cast<const SimdBase256*>(valueNew);
			whitespace.convertWhitespaceToSimdBase256(newPtr);
			backslash.convertBackslashesToSimdBase256(newPtr);
			op.convertStructuralsToSimdBase256(newPtr);
			quotes.convertQuotesToSimdBase256(newPtr);
			collectFinalStructurals();
			addTapeValues();
			stringIndex += 256;
		}

		__forceinline size_t getTapeLength() noexcept {
			return tapeIndex;
		}

	  protected:
		StructuralIndex tapePtrs{};
		SimdBase256 structurals{};
		SimdBase256 whitespace{};
		uint64_t prevInString{};
		SimdBase256 backslash{};
		size_t stringIndex{};
		SimdBase256 quotes{};
		bool prevInScalar{};
		bool prevEscaped{};
		size_t tapeIndex{};
		SimdBase256 op{};

		__forceinline int64_t rollValuesIntoTape(size_t currentIndex, size_t x, int64_t newBits) noexcept {
			tapePtrs[(currentIndex * 8) + tapeIndex] = static_cast<uint32_t>(_tzcnt_u64(newBits) + (x * 64ull) + stringIndex);
			newBits = _blsr_u64(newBits);
			tapePtrs[1 + (currentIndex * 8) + tapeIndex] = static_cast<uint32_t>(_tzcnt_u64(newBits) + (x * 64ull) + stringIndex);
			newBits = _blsr_u64(newBits);
			tapePtrs[2 + (currentIndex * 8) + tapeIndex] = static_cast<uint32_t>(_tzcnt_u64(newBits) + (x * 64ull) + stringIndex);
			newBits = _blsr_u64(newBits);
			tapePtrs[3 + (currentIndex * 8) + tapeIndex] = static_cast<uint32_t>(_tzcnt_u64(newBits) + (x * 64ull) + stringIndex);
			newBits = _blsr_u64(newBits);
			tapePtrs[4 + (currentIndex * 8) + tapeIndex] = static_cast<uint32_t>(_tzcnt_u64(newBits) + (x * 64ull) + stringIndex);
			newBits = _blsr_u64(newBits);
			tapePtrs[5 + (currentIndex * 8) + tapeIndex] = static_cast<uint32_t>(_tzcnt_u64(newBits) + (x * 64ull) + stringIndex);
			newBits = _blsr_u64(newBits);
			tapePtrs[6 + (currentIndex * 8) + tapeIndex] = static_cast<uint32_t>(_tzcnt_u64(newBits) + (x * 64ull) + stringIndex);
			newBits = _blsr_u64(newBits);
			tapePtrs[7 + (currentIndex * 8) + tapeIndex] = static_cast<uint32_t>(_tzcnt_u64(newBits) + (x * 64ull) + stringIndex);
			newBits = _blsr_u64(newBits);
			return newBits;
		}

		__forceinline void addTapeValues() noexcept {
			for (size_t x = 0; x < 4; ++x) {
				auto newBits = structurals.getInt64(x);
				if (!newBits) {
					continue;
				}
				int32_t cnt = static_cast<int32_t>(__popcnt64(newBits));
				if (cnt > 0) {
					newBits = rollValuesIntoTape(0, x, newBits);
					if (cnt > 8) {
						newBits = rollValuesIntoTape(1, x, newBits);
						if (cnt > 16) {
							newBits = rollValuesIntoTape(2, x, newBits);
							if (cnt > 24) {
								newBits = rollValuesIntoTape(3, x, newBits);
								if (cnt > 32) {
									newBits = rollValuesIntoTape(4, x, newBits);
									if (cnt > 40) {
										newBits = rollValuesIntoTape(5, x, newBits);
										if (cnt > 48) {
											newBits = rollValuesIntoTape(6, x, newBits);
											if (cnt > 56) {
												newBits = rollValuesIntoTape(7, x, newBits);
											}
										}
									}
								}
							}
						}
					}
				}
				tapeIndex += cnt;
			}
		}

		__forceinline void collectFinalStructurals() noexcept {
			backslash.setFirstBit(prevEscaped);
			SimdBase256 followsEscape = backslash.shl<1>() | prevEscaped;
			SimdBase256 evenBits{ _mm256_set1_epi8(0b01010101) };
			SimdBase256 oddSequenceStarts = backslash.bitAndNot(evenBits.bitAndNot(followsEscape));
			SimdBase256 sequencesStartingOnEvenBits{};
			prevEscaped = backslash.collectCarries(oddSequenceStarts, sequencesStartingOnEvenBits);
			SimdBase256 invert_mask = sequencesStartingOnEvenBits.shl<1>();
			SimdBase256 escaped = (evenBits ^ invert_mask) & followsEscape;
			quotes = quotes.bitAndNot(escaped);
			SimdBase256 inString = quotes.carrylessMultiplication(prevInString);
			SimdBase256 scalar = ~(op | whitespace);
			SimdBase256 nonQuoteScalar = ~(op | whitespace).bitAndNot(quotes);
			bool prevInScalarNew = prevInScalar;
			SimdBase256 shiftMask{ _mm256_set_epi64x(static_cast<int64_t>(static_cast<uint64_t>(0ull) - static_cast<uint64_t>(1ull << 62)), 0ull,
				0ull, 0ull) };
			prevInScalar = (nonQuoteScalar & shiftMask).checkLastBit();
			SimdBase256 followsNonQuoteScalar = nonQuoteScalar.shl<1>();
			followsNonQuoteScalar.setFirstBit(prevInScalarNew);
			structurals = (op | scalar.bitAndNot(followsNonQuoteScalar)).bitAndNot(inString ^ quotes);
		}
	};

}