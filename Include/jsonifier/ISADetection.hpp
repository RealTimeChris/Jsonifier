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

#include <immintrin.h>
#include <iostream>
#include <stdlib.h>
#include <memory>
#include <random>
#include <bitset>

namespace JsonifierInternal {

	using StringViewPtr = const uint8_t*;
	using StructuralIndex = const uint8_t*;
	using StringBufferPtr = uint8_t*;

	template<uint64_t StepSize> struct SimdBase {};

#ifdef T_AVX512

	template<typename CharacterType>
	concept CharType = requires(CharacterType) { sizeof(CharacterType) == 1; };

	template<typename ValueType>
	concept Avx256T = std::same_as<__m256i, ValueType> || std::same_as<__m256, ValueType>;

	template<typename ValueType>
	concept Avx512T = std::same_as<__m512i, ValueType> || std::same_as<__m512, ValueType>;

	template<> struct SimdBase<256> {
	  public:
		inline SimdBase() noexcept = default;

		inline SimdBase& operator=(SimdBase&& other) noexcept = default;
		inline SimdBase(SimdBase&& other) noexcept = default;
		inline SimdBase& operator=(const SimdBase& other) noexcept = default;
		inline SimdBase(const SimdBase& other) noexcept = default;

		template<Avx256T ValueType> inline SimdBase& operator=(ValueType&& data) noexcept {
			value = std::forward<ValueType>(data);
			return *this;
		}

		template<Avx256T ValueType> inline SimdBase(ValueType&& data) noexcept {
			value = std::forward<ValueType>(data);
		}

		inline operator const __m256i&&() const noexcept {
			return std::forward<const __m256i>(value);
		}

		inline uint64_t getUint64(uint64_t index) noexcept {
			switch (index) {
				case 0: {
					return static_cast<uint64_t>(_mm256_extract_epi64(value, 0));
				}
				case 1: {
					return static_cast<uint64_t>(_mm256_extract_epi64(value, 1));
				}
				case 2: {
					return static_cast<uint64_t>(_mm256_extract_epi64(value, 2));
				}
				case 3: {
					return static_cast<uint64_t>(_mm256_extract_epi64(value, 3));
				}
				default: {
					return static_cast<uint64_t>(_mm256_extract_epi64(value, 0));
				}
			}
		}

		inline void insertInt64(int64_t valueNew, uint64_t index) noexcept {
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

		inline SimdBase carrylessMultiplication(uint64_t& prevInString) const noexcept {
			__m128i allOnes{ _mm_set1_epi8('\xFF') };
			SimdBase valuesNew{};
			__m128i valueLow{ _mm256_extracti128_si256(value, 0) };
			__m128i valueHigh{ _mm256_extracti128_si256(value, 1) };
			valuesNew.insertInt64(_mm_cvtsi128_si64(_mm_clmulepi64_si128(valueLow, allOnes, 0)) ^ prevInString, 0);
			prevInString = uint64_t(static_cast<int64_t>(valuesNew.getUint64(0)) >> 63);
			valuesNew.insertInt64(_mm_cvtsi128_si64(_mm_clmulepi64_si128(valueLow, allOnes, 1)) ^ prevInString, 1);
			prevInString = uint64_t(static_cast<int64_t>(valuesNew.getUint64(1)) >> 63);
			valuesNew.insertInt64(_mm_cvtsi128_si64(_mm_clmulepi64_si128(valueHigh, allOnes, 0)) ^ prevInString, 2);
			prevInString = uint64_t(static_cast<int64_t>(valuesNew.getUint64(2)) >> 63);
			valuesNew.insertInt64(_mm_cvtsi128_si64(_mm_clmulepi64_si128(valueHigh, allOnes, 1)) ^ prevInString, 3);
			prevInString = uint64_t(static_cast<int64_t>(valuesNew.getUint64(3)) >> 63);
			return valuesNew;
		}

	  protected:
		__m256i value{};
	};

	constexpr int32_t StepSize{ 512 };
	constexpr int32_t BytesPerStep{ StepSize / 8 };
	constexpr int32_t SixtyFourPer{ StepSize / 64 };
	using SimdBaseReal = SimdBase<512>;
	using StringParsingType = uint64_t;
	using AvxType = __m512i;

	template<> struct SimdBase<512> {
	  public:
		inline SimdBase() noexcept = default;

		inline SimdBase& operator=(SimdBase&& other) noexcept = default;
		inline SimdBase(SimdBase&& other) noexcept = default;
		inline SimdBase& operator=(const SimdBase& other) noexcept = default;
		inline SimdBase(const SimdBase& other) noexcept = default;

		template<Avx512T ValueType> inline SimdBase& operator=(ValueType&& data) noexcept {
			value = std::forward<ValueType>(data);
			return *this;
		}

		template<Avx512T ValueType> inline SimdBase(ValueType&& data) noexcept {
			value = std::forward<ValueType>(data);
		}

		inline SimdBase& operator=(uint8_t other) noexcept {
			value = _mm512_set1_epi8(other);
			return *this;
		}

		inline explicit SimdBase(uint8_t other) noexcept {
			*this = other;
		}

		template<CharType ValueType> inline SimdBase& operator=(ValueType values[64]) noexcept {
			value = _mm512_load_si512(values);
			return *this;
		}

		template<CharType ValueType> inline SimdBase(ValueType values[64]) noexcept {
			*this = values;
		}

		inline operator __m512i&() {
			return value;
		}

		inline operator const __m512i&&() const noexcept {
			return std::forward<const __m512i>(value);
		}

		inline explicit operator bool() const noexcept {
			return _mm512_test_epi64_mask(value, value) != 0;
		}

		inline explicit operator uint64_t() const noexcept {
			return toBitMask();
		}

		inline SimdBase operator|(SimdBase&& other) noexcept {
			return _mm512_or_si512(value, std::forward<__m512i>(other));
		}

		inline SimdBase operator|(const SimdBase& other) const noexcept {
			return _mm512_or_si512(value, other);
		}

		inline SimdBase operator&(const SimdBase& other) const noexcept {
			return _mm512_and_si512(value, other);
		}

		inline SimdBase operator^(const SimdBase& other) const noexcept {
			return _mm512_xor_si512(value, other);
		}

		inline uint64_t operator==(const SimdBase& other) const noexcept {
			return _mm512_cmpeq_epi8_mask(value, other);
		}

		inline uint64_t operator==(const uint8_t& other) const noexcept {
			return _mm512_cmpeq_epi8_mask(value, _mm512_set1_epi8(other));
		}

		inline SimdBase operator~() const noexcept {
			return _mm512_xor_si512(*this, _mm512_set1_epi64(-1ll));
		}

		inline void convertWhitespaceToSimdBase(const SimdBase* valuesNew) noexcept {
			uint8_t arrayNew[64]{ ' ', 100, 100, 100, 17, 100, 113, 2, 100, '\t', '\n', 112, 100, '\r', 100, 100, ' ', 100, 100, 100, 17, 100, 113, 2,
				100, '\t', '\n', 112, 100, '\r', 100, 100, ' ', 100, 100, 100, 17, 100, 113, 2, 100, '\t', '\n', 112, 100, '\r', 100, 100, ' ', 100,
				100, 100, 17, 100, 113, 2, 100, '\t', '\n', 112, 100, '\r', 100, 100 };
			SimdBase whitespaceTable{ arrayNew };

			for (uint64_t x = 0; x < 8; ++x) {
				addValues(valuesNew[x].shuffle(whitespaceTable) == valuesNew[x], x);
			}
		}

		inline void convertBackslashesToSimdBase(const SimdBase* valuesNew) noexcept {
			for (uint64_t x = 0; x < 8; ++x) {
				addValues(valuesNew[x] == '\\', x);
			}
		}

		inline void convertStructuralsToSimdBase(const SimdBase* valuesNew) noexcept {
			uint8_t arrayNew[64]{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, ':', '{', ',', '}', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, ':', '{', ',', '}', 0, 0, 0,
				0, 0, 0, 0, 0, 0, 0, 0, 0, ':', '{', ',', '}', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, ':', '{', ',', '}', 0, 0 };
			SimdBase opTable{ arrayNew };
			SimdBase chars{ uint8_t{ 0x20 } };

			for (uint64_t x = 0; x < 8; ++x) {
				addValues(valuesNew[x].shuffle(opTable) == (valuesNew[x] | chars), x);
			}
		}

		inline void convertQuotesToSimdBase(const SimdBase* valuesNew) noexcept {
			for (uint64_t x = 0; x < 8; ++x) {
				addValues(valuesNew[x] == '"', x);
			}
		}

		inline SimdBase bitAndNot(const SimdBase& other) const noexcept {
			return _mm512_andnot_si512(other, value);
		}

		inline SimdBase shuffle(SimdBase& other) const noexcept {
			return _mm512_shuffle_epi8(other, value);
		}

		inline void insertInt64(int64_t valueNew, uint64_t index) noexcept {
			switch (index) {
				case 0: {
					auto lower = _mm512_extracti64x4_epi64(value, 0);
					lower = _mm256_insert_epi64(lower, valueNew, 0);
					value = _mm512_inserti64x4(value, lower, 0);
					break;
				}
				case 1: {
					auto lower = _mm512_extracti64x4_epi64(value, 0);
					lower = _mm256_insert_epi64(lower, valueNew, 1);
					value = _mm512_inserti64x4(value, lower, 0);
					break;
				}
				case 2: {
					auto lower = _mm512_extracti64x4_epi64(value, 0);
					lower = _mm256_insert_epi64(lower, valueNew, 2);
					value = _mm512_inserti64x4(value, lower, 0);
					break;
				}
				case 3: {
					auto lower = _mm512_extracti64x4_epi64(value, 0);
					lower = _mm256_insert_epi64(lower, valueNew, 3);
					value = _mm512_inserti64x4(value, lower, 0);
					break;
				}
				case 4: {
					auto lower = _mm512_extracti64x4_epi64(value, 1);
					lower = _mm256_insert_epi64(lower, valueNew, 0);
					value = _mm512_inserti64x4(value, lower, 1);
					break;
				}
				case 5: {
					auto lower = _mm512_extracti64x4_epi64(value, 1);
					lower = _mm256_insert_epi64(lower, valueNew, 1);
					value = _mm512_inserti64x4(value, lower, 1);
					break;
				}
				case 6: {
					auto lower = _mm512_extracti64x4_epi64(value, 1);
					lower = _mm256_insert_epi64(lower, valueNew, 2);
					value = _mm512_inserti64x4(value, lower, 1);
					break;
				}
				case 7: {
					auto lower = _mm512_extracti64x4_epi64(value, 1);
					lower = _mm256_insert_epi64(lower, valueNew, 3);
					value = _mm512_inserti64x4(value, lower, 1);
					break;
				}
				default: {
					auto lower = _mm512_extracti64x4_epi64(value, 0);
					lower = _mm256_insert_epi64(lower, valueNew, 0);
					value = _mm512_inserti64x4(value, lower, 0);
					break;
				}
			}
		}

		inline void addValues(uint64_t&& values, uint64_t index) noexcept {
			insertInt64(values, index);
		}

		template<uint64_t amount> inline SimdBase shl() const noexcept {
			SimdBase newValue{ _mm512_slli_epi64(*this, (amount % 64)) };
			SimdBase newerValue{ _mm512_srli_epi64(_mm512_permutexvar_epi64(_mm512_set_epi64(6, 5, 4, 3, 2, 1, 0, 7), *this), 64 - (amount % 64)) };
			uint64_t maxValue{ std::numeric_limits<uint64_t>::max() };
			SimdBase newestValue{ _mm512_set_epi64(maxValue, maxValue, maxValue, maxValue, maxValue, maxValue, maxValue, 0) };
			return SimdBase{ newValue | (newerValue & newestValue) };
		}

		inline int64_t toBitMask() const noexcept {
			return _mm512_movepi8_mask(*this);
		}

		inline void reset() noexcept {
			value = _mm512_setzero_si512();
		}

		template<typename ValueType> inline void store(ValueType* destVector) const noexcept {
			_mm512_store_si512(destVector, value);
		}

		inline SimdBase& setLSB(bool valueNew) noexcept {
			if (valueNew) {
				*this = _mm512_or_si512(*this, _mm512_set_epi64(0, 0, 0, 0, 0, 0, 0, 0x1));
			} else {
				*this = _mm512_andnot_si512(_mm512_set_epi64(0, 0, 0, 0, 0, 0, 0, 0x1), *this);
			}
			return *this;
		}

		inline bool checkLSB() const noexcept {
			__m512i result = _mm512_and_si512(*this, _mm512_set_epi64(0, 0, 0, 0, 0, 0, 0, 0x01));
			return _mm512_test_epi64_mask(result, result);
		}

		inline bool checkMSB() const noexcept {
			__m512i result = _mm512_and_si512(*this, _mm512_set_epi64(0x8000000000000000, 0, 0, 0, 0, 0, 0, 0));
			return _mm512_test_epi64_mask(result, result);
		}

		inline SimdBase carrylessMultiplication(uint64_t& prevInString) const noexcept {
			SimdBase<256> lowValues{ _mm512_extracti64x4_epi64(value, 0) };
			SimdBase<256> highValues{ _mm512_extracti64x4_epi64(value, 1) };
			lowValues = lowValues.carrylessMultiplication(prevInString);
			highValues = highValues.carrylessMultiplication(prevInString);
			SimdBase returnValue{};
			returnValue = _mm512_inserti64x4(returnValue, lowValues, 0);
			returnValue = _mm512_inserti64x4(returnValue, highValues, 1);
			return returnValue;
		}

		inline bool collectCarries(const SimdBase& other1, SimdBase& result) const noexcept {
			__m512i carry = _mm512_set1_epi64(0);
			__m512i sum = _mm512_add_epi64(*this, other1);
			sum = _mm512_add_epi64(sum, carry);
			result = std::move(sum);
			return _mm512_test_epi64_mask(carry, sum) == 0;
		}

		inline SimdBase follows(bool& overflow) noexcept {
			SimdBase result = shl<1>();
			result.setLSB(overflow);
			overflow = checkMSB();
			return result;
		}

		inline void printBits(uint64_t values, const std::string& valuesTitle) const noexcept {
			std::cout << valuesTitle;
			std::cout << std::bitset<64>{ values };
			std::cout << std::endl;
		}

		inline SimdBase printBits(const std::string& valuesTitle) const noexcept {
			std::cout << valuesTitle;
			for (uint64_t x = 0; x < 8; ++x) {
				std::cout << std::bitset<64>{ *(reinterpret_cast<const uint64_t*>(&value) + x) };
			}
			std::cout << std::endl;
			return *this;
		}

	  protected:
		__m512i value{};
	};

	#define load(x) _mm512_load_si512(x)
	#define set(x) _mm512_set1_epi8(x)
	#define tzCount(x) _tzcnt_u64(x)

#elif defined T_AVX2

	template<typename CharacterType>
	concept CharType = requires(CharacterType) { sizeof(CharacterType) == 1; };

	template<typename ValueType>
	concept Avx256T = std::same_as<__m256i, ValueType> || std::same_as<__m256, ValueType>;

	constexpr int32_t StepSize{ 256 };
	constexpr int32_t BytesPerStep{ StepSize / 8 };
	constexpr int32_t SixtyFourPer{ StepSize / 64 };
	using StringParsingType = uint32_t;
	using SimdBaseReal = SimdBase<256>;
	using AvxType = __m256i;

	template<> struct SimdBase<256> {
	  public:
		inline SimdBase() noexcept = default;

		inline SimdBase& operator=(SimdBase&& other) noexcept = default;
		inline SimdBase(SimdBase&& other) noexcept = default;
		inline SimdBase& operator=(const SimdBase& other) noexcept = delete;
		inline SimdBase(const SimdBase& other) noexcept = delete;

		template<Avx256T ValueType> inline SimdBase& operator=(ValueType&& data) noexcept {
			value = std::forward<ValueType>(data);
			return *this;
		}

		template<Avx256T ValueType> inline SimdBase(ValueType&& data) noexcept {
			value = std::forward<ValueType>(data);
		}

		inline SimdBase& operator=(uint8_t other) noexcept {
			value = _mm256_set1_epi8(other);
			return *this;
		}

		inline explicit SimdBase(uint8_t other) noexcept {
			*this = other;
		}

		template<CharType ValueType> inline SimdBase& operator=(ValueType values[32]) noexcept {
			value = gatherValues256<ValueType>(values);
			return *this;
		}

		template<CharType ValueType> inline SimdBase(ValueType values[32]) noexcept {
			*this = values;
		}

		inline operator __m256i&() {
			return value;
		}

		inline operator const __m256i&&() const noexcept {
			return std::forward<const __m256i>(value);
		}

		inline explicit operator bool() const noexcept {
			return !_mm256_testz_si256(value, value);
		}

		inline operator uint32_t() const noexcept {
			return toBitMask();
		}

		inline SimdBase operator|(SimdBase&& other) noexcept {
			return _mm256_or_si256(value, std::forward<__m256i>(other));
		}

		inline SimdBase operator|(const SimdBase& other) const noexcept {
			return _mm256_or_si256(value, other);
		}

		inline SimdBase operator&(const SimdBase& other) const noexcept {
			return _mm256_and_si256(value, other);
		}

		inline SimdBase operator^(const SimdBase& other) const noexcept {
			return _mm256_xor_si256(value, other);
		}

		inline StringParsingType operator==(const SimdBase& other) const noexcept {
			SimdBase newValue = _mm256_cmpeq_epi8(value, other);
			return newValue.toBitMask();
		}

		inline StringParsingType operator==(const uint8_t& other) const noexcept {
			SimdBase newValue = _mm256_cmpeq_epi8(value, _mm256_set1_epi8(other));
			return newValue.toBitMask();
		}

		inline SimdBase operator~() const noexcept {
			return _mm256_xor_si256(*this, _mm256_set1_epi64x(std::numeric_limits<uint64_t>::max()));
		}

		inline void convertWhitespaceToSimdBase(const SimdBase* valuesNew) noexcept {
			uint8_t arrayNew[32]{ ' ', 100, 100, 100, 17, 100, 113, 2, 100, '\t', '\n', 112, 100, '\r', 100, 100, ' ', 100, 100, 100, 17, 100, 113, 2,
				100, '\t', '\n', 112, 100, '\r', 100, 100 };
			SimdBase whitespaceTable{ arrayNew };
			for (uint64_t x = 0; x < 8; ++x) {
				addValues(valuesNew[x].shuffle(whitespaceTable) == valuesNew[x], x);
			}
		};

		inline void convertBackslashesToSimdBase(const SimdBase* valuesNew) noexcept {
			SimdBase backslashes{ _mm256_set1_epi8('\\') };
			for (uint64_t x = 0; x < 8; ++x) {
				addValues(valuesNew[x] == backslashes, x);
			}
		};

		inline void convertStructuralsToSimdBase(const SimdBase* valuesNew) noexcept {
			uint8_t arrayNew[32]{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, ':', '{', ',', '}', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, ':', '{', ',', '}', 0, 0 };
			SimdBase opTable{ arrayNew };
			SimdBase chars{ uint8_t{ 0x20 } };
			for (uint64_t x = 0; x < 8; ++x) {
				addValues(valuesNew[x].shuffle(opTable) == (valuesNew[x] | chars), x);
			}
		};

		inline void convertQuotesToSimdBase(const SimdBase* valuesNew) noexcept {
			SimdBase quotes = _mm256_set1_epi8('"');
			for (uint64_t x = 0; x < 8; ++x) {
				addValues(valuesNew[x] == quotes, x);
			}
		}

		inline uint64_t getUint64(uint64_t index) noexcept {
			switch (index) {
				case 0: {
					return static_cast<uint64_t>(_mm256_extract_epi64(value, 0));
				}
				case 1: {
					return static_cast<uint64_t>(_mm256_extract_epi64(value, 1));
				}
				case 2: {
					return static_cast<uint64_t>(_mm256_extract_epi64(value, 2));
				}
				case 3: {
					return static_cast<uint64_t>(_mm256_extract_epi64(value, 3));
				}
				default: {
					return static_cast<uint64_t>(_mm256_extract_epi64(value, 0));
				}
			}
		}

		inline int64_t getInt64(uint64_t index) noexcept {
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

		inline void insertInt64(int64_t valueNew, uint64_t index) noexcept {
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

		inline void insertInt32(int32_t valueNew, uint64_t index) noexcept {
			switch (index) {
				case 0: {
					value = _mm256_insert_epi32(value, valueNew, 0);
					break;
				}
				case 1: {
					value = _mm256_insert_epi32(value, valueNew, 1);
					break;
				}
				case 2: {
					value = _mm256_insert_epi32(value, valueNew, 2);
					break;
				}
				case 3: {
					value = _mm256_insert_epi32(value, valueNew, 3);
					break;
				}
				case 4: {
					value = _mm256_insert_epi32(value, valueNew, 4);
					break;
				}
				case 5: {
					value = _mm256_insert_epi32(value, valueNew, 5);
					break;
				}
				case 6: {
					value = _mm256_insert_epi32(value, valueNew, 6);
					break;
				}
				case 7: {
					value = _mm256_insert_epi32(value, valueNew, 7);
					break;
				}
				default: {
					value = _mm256_insert_epi32(value, valueNew, 0);
					break;
				}
			}
		}

		inline int32_t getInt32(uint64_t index) noexcept {
			switch (index) {
				case 0: {
					return _mm256_extract_epi32(value, 0);
				}
				case 1: {
					return _mm256_extract_epi32(value, 1);
				}
				case 2: {
					return _mm256_extract_epi32(value, 2);
				}
				case 3: {
					return _mm256_extract_epi32(value, 3);
				}
				case 4: {
					return _mm256_extract_epi32(value, 4);
				}
				case 5: {
					return _mm256_extract_epi32(value, 5);
				}
				case 6: {
					return _mm256_extract_epi32(value, 6);
				}
				case 7: {
					return _mm256_extract_epi32(value, 7);
				}
				default: {
					return _mm256_extract_epi32(value, 0);
				}
			}
		}

		inline SimdBase bitAndNot(const SimdBase& other) const noexcept {
			return _mm256_andnot_si256(other, value);
		}

		inline SimdBase shuffle(const SimdBase& other) const noexcept {
			return _mm256_shuffle_epi8(other, value);
		}

		inline void addValues(uint32_t values, uint64_t index) noexcept {
			insertInt32(values, index);
		}

		template<uint64_t amount> inline SimdBase shl() const noexcept {
			return SimdBase{ _mm256_slli_epi64(*this, (amount % 64)) } |
				_mm256_srli_epi64(_mm256_permute4x64_epi64(*this, 0b10010011), 64 - (amount % 64));
		}

		inline int32_t toBitMask() const noexcept {
			return _mm256_movemask_epi8(*this);
		}

		inline void reset() noexcept {
			value = _mm256_setzero_si256();
		}

		template<typename ValueType> inline void store(ValueType* storageLocation) {
			alignas(32) float newArray[8]{};
			_mm256_store_ps(newArray, _mm256_castsi256_ps(value));
			std::memcpy(storageLocation, newArray, sizeof(value));
		}

		inline SimdBase& setLSB(bool valueNew) noexcept {
			if (valueNew) {
				*this = _mm256_or_si256(*this, _mm256_set_epi64x(0, 0, 0, 0x1));
			} else {
				*this = _mm256_andnot_si256(_mm256_set_epi64x(0, 0, 0, 0x1), *this);
			}
			return *this;
		}

		inline bool checkLSB() const noexcept {
			__m256i result = _mm256_and_si256(*this, _mm256_set_epi64x(0, 0, 0, 0x01));
			return !_mm256_testz_si256(result, result);
		}

		inline bool checkMSB() const noexcept {
			__m256i result = _mm256_and_si256(*this, _mm256_set_epi64x(0x8000000000000000, 0, 0, 0));
			return !_mm256_testz_si256(result, result);
		}

		inline SimdBase carrylessMultiplication(uint64_t& prevInString) const noexcept {
			__m128i allOnes{ _mm_set1_epi8('\xFF') };
			SimdBase valuesNew{};
			__m128i valueLow{ _mm256_extracti128_si256(value, 0) };
			__m128i valueHigh{ _mm256_extracti128_si256(value, 1) };
			valuesNew.insertInt64(_mm_cvtsi128_si64(_mm_clmulepi64_si128(valueLow, allOnes, 0)) ^ prevInString, 0);
			prevInString = uint64_t(static_cast<int64_t>(valuesNew.getUint64(0)) >> 63);
			valuesNew.insertInt64(_mm_cvtsi128_si64(_mm_clmulepi64_si128(valueLow, allOnes, 1)) ^ prevInString, 1);
			prevInString = uint64_t(static_cast<int64_t>(valuesNew.getUint64(1)) >> 63);
			valuesNew.insertInt64(_mm_cvtsi128_si64(_mm_clmulepi64_si128(valueHigh, allOnes, 0)) ^ prevInString, 2);
			prevInString = uint64_t(static_cast<int64_t>(valuesNew.getUint64(2)) >> 63);
			valuesNew.insertInt64(_mm_cvtsi128_si64(_mm_clmulepi64_si128(valueHigh, allOnes, 1)) ^ prevInString, 3);
			prevInString = uint64_t(static_cast<int64_t>(valuesNew.getUint64(3)) >> 63);
			return valuesNew;
		}

		inline bool collectCarries(SimdBase<256>& other1, SimdBase<256>& result) noexcept {
			bool returnValue{};
			long long unsigned returnValue64{};
			for (uint64_t x = 0; x < 4; ++x) {
				if (_addcarry_u64(0, getInt64(x), other1.getInt64(x), &returnValue64)) {
					returnValue = true;
				} else {
					returnValue = false;
				}
				result.insertInt64(returnValue64, x);
			}
			return returnValue;
		}

		inline SimdBase follows(bool& overflow) const noexcept {
			SimdBase result = shl<1>();
			result.setLSB(overflow);
			overflow = checkMSB();
			return result;
		}

		inline void printBits(uint64_t values, const std::string& valuesTitle) const noexcept {
			std::cout << valuesTitle;
			std::cout << std::bitset<64>{ values };
			std::cout << std::endl;
		}

		inline SimdBase<256>& printBits(const std::string& valuesTitle) noexcept {
			std::cout << valuesTitle;
			for (uint64_t x = 0; x < 32; ++x) {
				for (uint64_t y = 0; y < 8; ++y) {
					std::cout << std::bitset<1>{ static_cast<uint64_t>(*(reinterpret_cast<int8_t*>(&value) + x)) >> y };
				}
			}
			std::cout << std::endl;
			return *this;
		}

	  protected:
		__m256i value{};
	};

	#define load(x) gatherValues256<std::remove_pointer_t<decltype(x)>>(x)
	#define set(x) _mm256_set1_epi8(x)
	#define tzCount(x) _tzcnt_u32(x)

#elif defined T_AVX

	template<typename CharacterType>
	concept CharType = requires(CharacterType) { sizeof(CharacterType) == 1; };

	template<typename ValueType>
	concept Avx128T = std::same_as<__m128i, ValueType> || std::same_as<__m128, ValueType>;

	constexpr int32_t StepSize{ 128 };
	constexpr int32_t BytesPerStep{ StepSize / 8 };
	constexpr int32_t SixtyFourPer{ StepSize / 64 };
	using StringParsingType = uint16_t;
	using SimdBaseReal = SimdBase<128>;
	using AvxType = __m128i;

	template<> struct SimdBase<128> {
	  public:
		inline SimdBase() noexcept = default;

		inline SimdBase& operator=(SimdBase&& other) noexcept = default;
		inline SimdBase(SimdBase&& other) noexcept = default;
		inline SimdBase& operator=(const SimdBase& other) noexcept = delete;
		inline SimdBase(const SimdBase& other) noexcept = delete;

		template<Avx128T ValueType> inline SimdBase& operator=(ValueType&& data) noexcept {
			value = std::forward<ValueType>(data);
			return *this;
		}

		template<Avx128T ValueType> inline SimdBase(ValueType&& data) noexcept {
			value = std::forward<ValueType>(data);
		}

		inline SimdBase& operator=(uint8_t other) noexcept {
			value = _mm_set1_epi8(other);
			return *this;
		}

		inline explicit SimdBase(uint8_t other) noexcept {
			*this = other;
		}

		inline SimdBase& operator=(const uint8_t* values) noexcept {
			value = gatherValues128<const uint8_t>(values);
			return *this;
		}

		inline SimdBase(const uint8_t* values) noexcept {
			*this = values;
		}

		inline operator __m128i&() {
			return value;
		}

		inline operator const __m128i&&() const noexcept {
			return std::forward<const __m128i>(value);
		}

		inline explicit operator bool() const noexcept {
			return !_mm_testz_si128(value, value);
		}

		inline operator uint32_t() const noexcept {
			return toBitMask();
		}

		inline SimdBase operator|(SimdBase&& other) noexcept {
			return _mm_or_si128(value, std::forward<__m128i>(other));
		}

		inline SimdBase operator|(const SimdBase& other) const noexcept {
			return _mm_or_si128(value, other);
		}

		inline SimdBase operator&(const SimdBase& other) const noexcept {
			return _mm_and_si128(value, other);
		}

		inline SimdBase operator^(const SimdBase& other) const noexcept {
			return _mm_xor_si128(value, other);
		}

		inline StringParsingType operator==(const SimdBase& other) const noexcept {
			SimdBase newValue = _mm_cmpeq_epi8(value, other);
			return newValue.toBitMask();
		}

		inline StringParsingType operator==(const uint8_t& other) const noexcept {
			SimdBase newValue = _mm_cmpeq_epi8(value, _mm_set1_epi8(other));
			return newValue.toBitMask();
		}

		inline SimdBase operator~() const noexcept {
			return _mm_xor_si128(*this, _mm_set1_epi64x(std::numeric_limits<uint64_t>::max()));
		}

		inline void convertWhitespaceToSimdBase(const SimdBase* valuesNew) noexcept {
			uint8_t arrayNew[16]{ ' ', 100, 100, 100, 17, 100, 113, 2, 100, '\t', '\n', 112, 100, '\r', 100, 100 };
			SimdBase whitespaceTable{ arrayNew };
			for (uint64_t x = 0; x < 8; ++x) {
				addValues(valuesNew[x].shuffle(whitespaceTable) == valuesNew[x], x);
			}
		};

		inline void convertBackslashesToSimdBase(const SimdBase* valuesNew) noexcept {
			SimdBase backslashes{ _mm_set1_epi8('\\') };
			for (uint64_t x = 0; x < 8; ++x) {
				addValues(valuesNew[x] == backslashes, x);
			}
		};

		inline void convertStructuralsToSimdBase(const SimdBase* valuesNew) noexcept {
			uint8_t arrayNew[16]{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, ':', '{', ',', '}', 0, 0 };
			SimdBase opTable{ arrayNew };
			SimdBase chars{ uint8_t{ 0x20 } };
			for (uint64_t x = 0; x < 8; ++x) {
				addValues(valuesNew[x].shuffle(opTable) == (valuesNew[x] | chars), x);
			}
		};

		inline void convertQuotesToSimdBase(const SimdBase* valuesNew) noexcept {
			SimdBase quotes = _mm_set1_epi8('"');
			for (uint64_t x = 0; x < 8; ++x) {
				addValues(valuesNew[x] == quotes, x);
			}
		}

		inline uint64_t getUint64(uint64_t index) const noexcept {
			switch (index) {
				case 0: {
					return static_cast<uint64_t>(_mm_extract_epi64(value, 0));
				}
				case 1: {
					return static_cast<uint64_t>(_mm_extract_epi64(value, 1));
				}
				default: {
					return static_cast<uint64_t>(_mm_extract_epi64(value, 0));
				}
			}
		}

		inline int64_t getInt64(uint64_t index) const noexcept {
			switch (index) {
				case 0: {
					return _mm_extract_epi64(value, 0);
				}
				case 1: {
					return _mm_extract_epi64(value, 1);
				}
				default: {
					return _mm_extract_epi64(value, 0);
				}
			}
		}

		inline void insertInt64(int64_t valueNew, uint64_t index) noexcept {
			switch (index) {
				case 0: {
					value = _mm_insert_epi64(value, valueNew, 0);
					break;
				}
				case 1: {
					value = _mm_insert_epi64(value, valueNew, 1);
					break;
				}
				default: {
					value = _mm_insert_epi64(value, valueNew, 0);
					break;
				}
			}
		}

		inline void insertInt32(int32_t valueNew, uint64_t index) noexcept {
			switch (index) {
				case 0: {
					value = _mm_insert_epi32(value, valueNew, 0);
					break;
				}
				case 1: {
					value = _mm_insert_epi32(value, valueNew, 1);
					break;
				}
				case 2: {
					value = _mm_insert_epi32(value, valueNew, 2);
					break;
				}
				case 3: {
					value = _mm_insert_epi32(value, valueNew, 3);
					break;
				}
				default: {
					value = _mm_insert_epi32(value, valueNew, 0);
					break;
				}
			}
		}

		inline void insertInt16(int16_t valueNew, uint64_t index) noexcept {
			switch (index) {
				case 0: {
					value = _mm_insert_epi16(value, valueNew, 0);
					break;
				}
				case 1: {
					value = _mm_insert_epi16(value, valueNew, 1);
					break;
				}
				case 2: {
					value = _mm_insert_epi16(value, valueNew, 2);
					break;
				}
				case 3: {
					value = _mm_insert_epi16(value, valueNew, 3);
					break;
				}
				case 4: {
					value = _mm_insert_epi16(value, valueNew, 4);
					break;
				}
				case 5: {
					value = _mm_insert_epi16(value, valueNew, 5);
					break;
				}
				case 6: {
					value = _mm_insert_epi16(value, valueNew, 6);
					break;
				}
				case 7: {
					value = _mm_insert_epi16(value, valueNew, 7);
					break;
				}
				default: {
					value = _mm_insert_epi16(value, valueNew, 0);
					break;
				}
			}
		}

		inline SimdBase bitAndNot(const SimdBase& other) const noexcept {
			return _mm_andnot_si128(other, value);
		}

		inline SimdBase shuffle(const SimdBase& other) const noexcept {
			return _mm_shuffle_epi8(other, value);
		}

		inline void addValues(uint16_t values, uint64_t index) noexcept {
			insertInt16(values, index);
		}

		template<uint64_t amount> inline SimdBase shl() const noexcept {
			SimdBase currentValues{};
			currentValues.insertInt64(getInt64(0) << amount % 64, 0);
			currentValues.insertInt64(getInt64(1) << amount % 64, 1);
			return currentValues;
		}

		inline int16_t toBitMask() const noexcept {
			return static_cast<int16_t>(_mm_movemask_epi8(*this));
		}

		inline void reset() noexcept {
			value = _mm_setzero_si128();
		}

		template<typename ValueType> inline void store(ValueType* storageLocation) {
			alignas(32) float newArray[4]{};
			_mm_store_ps(newArray, _mm_castsi128_ps(value));
			std::memcpy(storageLocation, newArray, sizeof(value));
		}

		inline SimdBase& setLSB(bool valueNew) noexcept {
			if (valueNew) {
				*this = _mm_or_si128(*this, _mm_set_epi64x(0, 0x1));
			} else {
				*this = _mm_andnot_si128(_mm_set_epi64x(0, 0x1), *this);
			}
			return *this;
		}

		inline bool checkLSB() const noexcept {
			__m128i result = _mm_and_si128(*this, _mm_set_epi64x(0, 0x01));
			return !_mm_testz_si128(result, result);
		}

		inline bool checkMSB() const noexcept {
			__m128i result = _mm_and_si128(*this, _mm_set_epi64x(0x8000000000000000, 0));
			return !_mm_testz_si128(result, result);
		}

		inline SimdBase carrylessMultiplication(uint64_t& prevInString) const noexcept {
			__m128i allOnes{ _mm_set1_epi8('\xFF') };
			SimdBase valuesNew{};
			__m128i valueLow{ value };
			valuesNew.insertInt64(_mm_cvtsi128_si64(_mm_clmulepi64_si128(valueLow, allOnes, 0)) ^ prevInString, 0);
			prevInString = uint64_t(static_cast<int64_t>(valuesNew.getUint64(0)) >> 63);
			valuesNew.insertInt64(_mm_cvtsi128_si64(_mm_clmulepi64_si128(valueLow, allOnes, 1)) ^ prevInString, 1);
			prevInString = uint64_t(static_cast<int64_t>(valuesNew.getUint64(1)) >> 63);
			return valuesNew;
		}

		inline bool collectCarries(SimdBase<128>& other1, SimdBase<128>& result) noexcept {
			bool returnValue{};
			long long unsigned returnValue64{};
			for (uint64_t x = 0; x < 2; ++x) {
				if (_addcarry_u64(0, getInt64(x), other1.getInt64(x), &returnValue64)) {
					returnValue = true;
				} else {
					returnValue = false;
				}
				result.insertInt64(returnValue64, x);
			}
			return returnValue;
		}

		inline SimdBase follows(bool& overflow) const noexcept {
			SimdBase result = shl<1>();
			result.setLSB(overflow);
			overflow = checkMSB();
			return result;
		}

		inline void printBits(uint64_t values, const std::string& valuesTitle) const noexcept {
			std::cout << valuesTitle;
			std::cout << std::bitset<64>{ values };
			std::cout << std::endl;
		}

		inline SimdBase<128>& printBits(const std::string& valuesTitle) noexcept {
			std::cout << valuesTitle;
			for (uint64_t x = 0; x < 16; ++x) {
				for (uint64_t y = 0; y < 8; ++y) {
					std::cout << std::bitset<1>{ static_cast<uint64_t>(*(reinterpret_cast<int8_t*>(&value) + x)) >> y };
				}
			}
			std::cout << std::endl;
			return *this;
		}

	  protected:
		__m128i value{};
	};

	#define load(x) gatherValues128<std::remove_pointer_t<decltype(x)>>(x)
	#define set(x) _mm_set1_epi8(x)
	#define tzCount(x) _tzcnt_u16(x)

#else

	template<typename CharacterType>
	concept CharType = requires(CharacterType) { sizeof(CharacterType) == 1; };

	constexpr int32_t StepSize{ 128 };
	constexpr int32_t BytesPerStep{ StepSize / 8 };
	constexpr int32_t SixtyFourPer{ StepSize / 64 };
	using StringParsingType = uint8_t;
	using SimdBaseReal = SimdBase<128>;
	using AvxType = int64_t;

	class Double64;

	uint16_t cmpeq_epi8_mask(const Double64& a, const Double64& b);

	class Double64 {
	  public:
		inline Double64() noexcept = default;

		inline Double64& operator=(int64_t values[2]) {
			value[0] = values[0];
			value[1] = values[1];
			return *this;
		}

		inline Double64(int64_t values[2]) {
			*this = values;
		}

		inline const int64_t& operator[](uint64_t index) const {
			return value[index];
		}

		inline int64_t& operator[](uint64_t index) {
			return value[index];
		}

		inline operator bool() const {
			return this->value[0] != 0 && this->value[1] != 0;
		}

		inline int16_t operator==(const Double64& other) const {
			return cmpeq_epi8_mask(*this, other);
		}

		inline Double64 shuffle_epi8(const Double64& mask) const {
			Double64 result;

			for (uint64_t j = 0; j < 2; ++j) {
				uint64_t mask_value = mask[j];
				uint64_t shuffled = 0;

				for (uint64_t k = 0; k < 8; ++k) {
					uint64_t src_index = (mask_value >> (k * 8)) & 0xFF;
					shuffled |= (this->value[j] >> (src_index * 8)) & 0xFFULL << (k * 8);
				}

				result[j] = shuffled;
			}

			return result;
		}

		inline void insertInt32(int32_t value, uint64_t index) {
			if (index == 0) {
				this->value[0] = (this->value[0] & 0xFFFFFFFF00000000) | static_cast<int64_t>(value);
			} else if (index == 1) {
				this->value[0] = (this->value[0] & 0x00000000FFFFFFFF) | (static_cast<int64_t>(value) << 32);
			} else if (index == 2) {
				this->value[1] = (this->value[1] & 0xFFFFFFFF00000000) | static_cast<int64_t>(value);
			} else if (index == 3) {
				this->value[1] = (this->value[1] & 0x00000000FFFFFFFF) | (static_cast<int64_t>(value) << 32);
			}
		}

		inline int32_t getInt32(uint64_t index) {
			if (index == 0) {
				return static_cast<int32_t>(this->value[0] >> 32);
			} else if (index == 1) {
				return static_cast<int32_t>(this->value[0]);
			} else if (index == 2) {
				return static_cast<int32_t>(this->value[1] >> 32);
			} else if (index == 3) {
				return static_cast<int32_t>(this->value[1]);
			}
		}

		inline Double64 operator|(const Double64& other) const {
			Double64 newValue{};
			newValue.value[0] = this->value[0] | other.value[0];
			newValue.value[1] = this->value[1] | other.value[1];
			return newValue;
		}

		inline Double64 operator&(const Double64& other) const {
			Double64 newValue{};
			newValue.value[0] = this->value[0] & other.value[0];
			newValue.value[1] = this->value[1] & other.value[1];
			return newValue;
		}

		inline Double64 operator^(const Double64& other) const {
			Double64 newValue{};
			newValue.value[0] = this->value[0] ^ other.value[0];
			newValue.value[1] = this->value[1] ^ other.value[1];
			return newValue;
		}

		inline Double64 operator~() const {
			Double64 newValue{};
			newValue.value[0] = ~this->value[0];
			newValue.value[1] = ~this->value[1];
			return newValue;
		}

	  protected:
		int64_t value[2]{};
	};

	inline int16_t countTrailingZeroesu16(uint64_t value) {
		if (value == 0) {
			return 16;
		}

		uint16_t count = 0;
		while ((value & 1) == 0) {
			value >>= 1;
			++count;
		}
		return count;
	}

	inline uint16_t movemask_epi8(const Double64& value) {
		uint16_t result = 0;

		for (uint64_t j = 0; j < 4; ++j) {
			for (uint64_t i = 0; i < 8; ++i) {
				uint64_t bit_index = j * 64 + i * 8 + 7;
				result |= ((value[j] >> bit_index) & 1) << i;
			}
		}

		return result;
	}

	inline Double64 setzero_x128() {
		return Double64{};
	}

	inline uint16_t cmpeq_epi8_mask(const Double64& a, const Double64& b) {
		uint16_t mask = 0;

		for (int i = 0; i < 16; ++i) {
			if (((a[i / 8] >> ((i % 8) * 8)) & 0xFF) == ((b[i / 8] >> ((i % 8) * 8)) & 0xFF)) {
				mask |= (1 << i);
			}
		}

		return mask;
	}

	inline Double64 mm_clmulepi64_si128(const Double64 a, const Double64 b, const int64_t imm8) {
		Double64 result;
		result[0] = 0;
		result[1] = 0;

		if (imm8 == 0x00) {
			uint64_t a0 = a[0];
			uint64_t b0 = b[0];

			uint64_t prod0 = 0;
			for (int i = 0; i < 64; ++i) {
				if ((a0 >> i) & 1) {
					prod0 ^= b0 << i;
				}
			}
			result[0] = prod0;
		} else if (imm8 == 0x10) {
			uint64_t a1 = a[1];
			uint64_t b0 = b[0];

			uint64_t prod1 = 0;
			for (int i = 0; i < 64; ++i) {
				if ((a1 >> i) & 1) {
					prod1 ^= b0 << i;
				}
			}
			result[0] = prod1;
		} else if (imm8 == 0x01) {
			uint64_t a0 = a[0];
			uint64_t b1 = b[1];

			uint64_t prod1 = 0;
			for (int i = 0; i < 64; ++i) {
				if ((a0 >> i) & 1) {
					prod1 ^= b1 << i;
				}
			}
			result[0] = prod1;
		} else if (imm8 == 0x11) {
			uint64_t a1 = a[1];
			uint64_t b1 = b[1];

			uint64_t prod0 = 0;
			for (int i = 0; i < 64; ++i) {
				if ((a1 >> i) & 1) {
					prod0 ^= b1 << i;
				}
			}
			result[0] = prod0;
		}

		return result;
	}

	inline Double64 set_epi64x(int64_t a, int64_t b) {
		Double64 newValue{};
		newValue[0] = a;
		newValue[1] = b;
		return newValue;
	}

	template<typename ValueType> inline void store_x128(const Double64& other, ValueType* storageLocation) {
		std::memcpy(storageLocation, &other, sizeof(int64_t) * 2);
	}

	template<typename ValueType> Double64 load_x128(ValueType* inputData) {
		Double64 currentValue{};
		std::memcpy(&currentValue, inputData, sizeof(int64_t) * 2);
		return currentValue;
	}

	template<typename ValueType> inline Double64 gatherValues128(const ValueType str[sizeof(Double64) / sizeof(ValueType)]) {
		return load_x128(str);
	}

	template<typename ValueType>
	concept Int128T = std::same_as<Double64, ValueType>;

	inline Double64 set1_x128(uint8_t input) {
		Double64 result = 0;
		memset(&result, input, sizeof(int64_t) * 2);
		return result;
	}

	template<> struct SimdBase<128> {
	  public:
		inline SimdBase() noexcept = default;

		inline SimdBase& operator=(SimdBase&& other) noexcept = default;
		inline SimdBase(SimdBase&& other) noexcept = default;
		inline SimdBase& operator=(const SimdBase& other) noexcept = delete;
		inline SimdBase(const SimdBase& other) noexcept = delete;

		template<Int128T ValueType> inline SimdBase& operator=(ValueType&& data) noexcept {
			value = std::forward<ValueType>(data);
			return *this;
		}

		template<Int128T ValueType> inline SimdBase(ValueType&& data) noexcept {
			value = std::forward<ValueType>(data);
		}

		inline SimdBase& operator=(uint8_t other) noexcept {
			value = set1_x128(other);
			return *this;
		}

		inline explicit SimdBase(uint8_t other) noexcept {
			*this = other;
		}

		template<CharType ValueType> inline SimdBase& operator=(ValueType values[16]) noexcept {
			value = gatherValues128<ValueType>(values);
			return *this;
		}

		template<CharType ValueType> inline SimdBase(ValueType values[16]) noexcept {
			*this = values;
		}

		inline operator Double64&() {
			return value;
		}

		inline operator const Double64&&() const noexcept {
			return std::forward<const Double64>(value);
		}

		inline explicit operator bool() const noexcept {
			return value;
		}

		inline operator uint16_t() const noexcept {
			return toBitMask();
		}

		inline SimdBase operator|(SimdBase&& other) noexcept {
			return value | other.value;
		}

		inline SimdBase operator|(const SimdBase& other) const noexcept {
			return value | other.value;
		}

		inline SimdBase operator&(const SimdBase& other) const noexcept {
			return value & other.value;
		}

		inline SimdBase operator^(const SimdBase& other) const noexcept {
			return value ^ other.value;
		}

		inline StringParsingType operator==(const SimdBase& other) const noexcept {
			return value == other.value;
		}

		inline StringParsingType operator==(const uint8_t& other) const noexcept {
			return value == set1_x128(other);
		}

		inline SimdBase operator~() const noexcept {
			return ~value;
		}

		inline void convertWhitespaceToSimdBase(const SimdBase* valuesNew) noexcept {
			uint8_t arrayNew[16]{ ' ', 100, 100, 100, 17, 100, 113, 2, 100, '\t', '\n', 112, 100, '\r', 100, 100 };
			SimdBase whitespaceTable{ arrayNew };
			for (uint64_t x = 0; x < 8; ++x) {
				addValues(valuesNew[x].shuffle(whitespaceTable) == valuesNew[x], x);
			}
		};

		inline void convertBackslashesToSimdBase(const SimdBase* valuesNew) noexcept {
			SimdBase backslashes{ set1_x128('\\') };
			for (uint64_t x = 0; x < 8; ++x) {
				addValues(valuesNew[x] == backslashes, x);
			}
		};

		inline void convertStructuralsToSimdBase(const SimdBase* valuesNew) noexcept {
			uint8_t arrayNew[16]{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, ':', '{', ',', '}', 0, 0 };
			SimdBase opTable{ arrayNew };
			SimdBase chars{ uint8_t{ 0x20 } };
			for (uint64_t x = 0; x < 8; ++x) {
				addValues(valuesNew[x].shuffle(opTable) == (valuesNew[x] | chars), x);
			}
		};

		inline void convertQuotesToSimdBase(const SimdBase* valuesNew) noexcept {
			SimdBase quotes = set1_x128('"');
			for (uint64_t x = 0; x < 8; ++x) {
				addValues(valuesNew[x] == quotes, x);
			}
		}

		inline uint64_t getUint64(uint64_t index) noexcept {
			return static_cast<uint64_t>(value[index]);
		}

		inline int64_t getInt64(uint64_t index) noexcept {
			return value[index];
		}

		inline void insertInt64(int64_t valueNew, uint64_t index) noexcept {
			value[index] = valueNew;
		}

		inline void insertInt32(int32_t valueNew, uint64_t index) noexcept {
			value.insertInt32(valueNew, index);
		}

		inline int32_t getInt32(uint64_t index) noexcept {
			return value.getInt32(index);
		}

		inline SimdBase bitAndNot(const SimdBase& other) const noexcept {
			return other.value & ~value;
		}

		inline SimdBase shuffle(const SimdBase& other) const noexcept {
			return value.shuffle_epi8(other);
		}

		inline void addValues(uint32_t values, uint64_t index) noexcept {
			insertInt32(values, index);
		}

		template<uint64_t amount> inline SimdBase shl() const noexcept {
			Double64 newValue{};
			newValue[0] = this->value[0] << amount % 64;
			newValue[1] = this->value[1] << amount % 64;
			return newValue;
		}

		inline int16_t toBitMask() const noexcept {
			return movemask_epi8(value);
		}

		inline void reset() noexcept {
			value = setzero_x128();
		}

		template<typename ValueType> inline void store(ValueType* storageLocation) {
			store_x128(value, storageLocation);
		}

		inline SimdBase& setLSB(bool valueNew) noexcept {
			if (valueNew) {
				*this = this->value | set_epi64x(0, 0x1);
			} else {
				*this = set_epi64x(0, 0x1) & ~this->value;
			}
			return *this;
		}

		inline bool checkLSB() const noexcept {
			Double64 result = this->value & set_epi64x(0, 0x01);
			return result;
		}

		inline bool checkMSB() const noexcept {
			Double64 result = this->value & set_epi64x(0x8000000000000000, 0);
			return result;
		}

		inline SimdBase carrylessMultiplication(uint64_t& prevInString) const noexcept {
			Double64 allOnes{ set1_x128('\xFF') };
			SimdBase valuesNew{};
			valuesNew.insertInt64(mm_clmulepi64_si128(value, allOnes, 0)[0] ^ prevInString, 0);
			prevInString = uint64_t(static_cast<int64_t>(valuesNew.getUint64(0)) >> 63);
			valuesNew.insertInt64(mm_clmulepi64_si128(value, allOnes, 1)[1] ^ prevInString, 1);
			prevInString = uint64_t(static_cast<int64_t>(valuesNew.getUint64(1)) >> 63);
			return valuesNew;
		}

		inline bool collectCarries(SimdBase<128>& other1, SimdBase<128>& result) noexcept {
			bool returnValue{};
			long long unsigned returnValue64{};
			for (uint64_t x = 0; x < 2; ++x) {
				if (_addcarry_u64(0, getInt64(x), other1.getInt64(x), &returnValue64)) {
					returnValue = true;
				} else {
					returnValue = false;
				}
				result.insertInt64(returnValue64, x);
			}
			return returnValue;
		}

		inline SimdBase follows(bool& overflow) const noexcept {
			SimdBase result = shl<1>();
			result.setLSB(overflow);
			overflow = checkMSB();
			return result;
		}

		inline void printBits(uint64_t values, const std::string& valuesTitle) const noexcept {
			std::cout << valuesTitle;
			std::cout << std::bitset<64>{ values };
			std::cout << std::endl;
		}

		inline SimdBase<128>& printBits(const std::string& valuesTitle) noexcept {
			std::cout << valuesTitle;
			for (uint64_t x = 0; x < 32; ++x) {
				for (uint64_t y = 0; y < 8; ++y) {
					std::cout << std::bitset<1>{ static_cast<uint64_t>(*(reinterpret_cast<int8_t*>(&value) + x)) >> y };
				}
			}
			std::cout << std::endl;
			return *this;
		}

	  protected:
		Double64 value{};
	};

	#define load(x) load_x128(x)
	#define set(x) set1_x128(x)
	#define tzCount(x) countTrailingZeroesu16(x)

#endif

}
