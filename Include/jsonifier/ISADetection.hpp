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

#include <bitset>
#include <immintrin.h>
#include <iostream>
#include <memory>
#include <random>
#include <stdlib.h>

namespace JsonifierInternal {

	using StringViewPtr	  = const uint8_t*;
	using StructuralIndex = uint32_t;
	using StringBufferPtr = uint8_t*;

	template<uint64_t StepSize> struct SimdBase {};

#if defined T_AVX512

	template<typename CharacterType>
	concept CharType = requires(CharacterType) { sizeof(CharacterType) == 1; };

	template<typename ValueType>
	concept Avx256T = std::same_as<__m256i, ValueType> || std::same_as<__m256, ValueType>;

	template<typename ValueType>
	concept Avx512T = std::same_as<__m512i, ValueType> || std::same_as<__m512, ValueType>;

	template<> struct SimdBase<256> {
	  public:
		inline SimdBase() noexcept = default;

		inline SimdBase& operator=(SimdBase&& other) noexcept	   = default;
		inline SimdBase(SimdBase&& other) noexcept				   = default;
		inline SimdBase& operator=(const SimdBase& other) noexcept = default;
		inline SimdBase(const SimdBase& other) noexcept			   = default;

		template<Avx256T ValueType> inline SimdBase& operator=(ValueType&& data) {
			value = std::forward<ValueType>(data);
			return *this;
		}

		template<Avx256T ValueType> inline SimdBase(ValueType&& data) {
			value = std::forward<ValueType>(data);
		}

		inline operator const __m256i&&() const {
			return std::forward<const __m256i>(value);
		}

		inline uint64_t getUint64(uint64_t index) {
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

		inline void insertInt64(int64_t valueNew, uint64_t index) {
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

		inline SimdBase carrylessMultiplication(uint64_t& prevInString) const {
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

	constexpr uint64_t StepSize{ 512 };
	constexpr uint64_t BytesPerStep{ StepSize / 8 };
	constexpr uint64_t SixtyFourPer{ StepSize / 64 };
	using SimdBaseReal		= SimdBase<512>;
	using StringParsingType = uint64_t;
	using AvxType			= __m512i;

	template<> struct SimdBase<512> {
	  public:
		inline SimdBase() noexcept = default;

		inline SimdBase& operator=(SimdBase&& other) noexcept	   = default;
		inline SimdBase(SimdBase&& other) noexcept				   = default;
		inline SimdBase& operator=(const SimdBase& other) noexcept = default;
		inline SimdBase(const SimdBase& other) noexcept			   = default;

		template<Avx512T ValueType> inline SimdBase& operator=(ValueType&& data) {
			value = std::forward<ValueType>(data);
			return *this;
		}

		template<Avx512T ValueType> inline SimdBase(ValueType&& data) {
			value = std::forward<ValueType>(data);
		}

		inline SimdBase& operator=(uint8_t other) {
			value = _mm512_set1_epi8(other);
			return *this;
		}

		inline explicit SimdBase(uint8_t other) {
			*this = other;
		}

		template<typename ValueType> inline SimdBase(ValueType values[64]) {
			value = _mm512_loadu_epi64(values);
		}

		inline operator __m512i&() {
			return value;
		}

		inline operator const __m512i&&() const {
			return std::forward<const __m512i>(value);
		}

		inline explicit operator bool() const {
			return _mm512_test_epi64_mask(value, value) != 0;
		}

		inline explicit operator uint64_t() const {
			return toBitMask();
		}

		inline SimdBase operator|(SimdBase&& other) noexcept {
			return _mm512_or_si512(value, std::forward<__m512i>(other));
		}

		inline SimdBase operator|(const SimdBase& other) const {
			return _mm512_or_si512(value, other);
		}

		inline SimdBase operator&(const SimdBase& other) const {
			return _mm512_and_si512(value, other);
		}

		inline SimdBase operator-(const SimdBase& other) const {
			return _mm512_sub_epi8(value, other);
		}

		inline SimdBase operator^(const SimdBase& other) const {
			return _mm512_xor_si512(value, other);
		}

		inline uint64_t operator==(const SimdBase& other) const {
			return _mm512_cmpeq_epi8_mask(value, other);
		}

		inline uint64_t operator==(const uint8_t& other) const {
			return _mm512_cmpeq_epi8_mask(value, _mm512_set1_epi8(other));
		}

		inline SimdBase operator~() const {
			return _mm512_xor_si512(*this, _mm512_set1_epi64(-1ll));
		}

		inline void convertWhitespaceToSimdBase(const SimdBase* valuesNew) {
			alignas(ALIGNMENT) uint8_t arrayNew[64]{ ' ', 100, 100, 100, 17, 100, 113, 2, 100, '\t', '\n', 112, 100, '\r', 100, 100, ' ', 100, 100, 100, 17, 100, 113, 2, 100, '\t',
				'\n', 112, 100, '\r', 100, 100, ' ', 100, 100, 100, 17, 100, 113, 2, 100, '\t', '\n', 112, 100, '\r', 100, 100, ' ', 100, 100, 100, 17, 100, 113, 2, 100, '\t',
				'\n', 112, 100, '\r', 100, 100 };
			SimdBase whitespaceTable{ arrayNew };

			for (uint64_t x = 0; x < 8; ++x) {
				addValues(valuesNew[x].shuffle(whitespaceTable) == valuesNew[x], x);
			}
		}

		inline void convertBackslashesToSimdBase(const SimdBase* valuesNew) {
			for (uint64_t x = 0; x < 8; ++x) {
				addValues(valuesNew[x] == '\\', x);
			}
		}

		inline void convertStructuralsToSimdBase(const SimdBase* valuesNew) {
			alignas(ALIGNMENT) uint8_t arrayNew[64]{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, ':', '{', ',', '}', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, ':', '{', ',', '}', 0, 0, 0, 0, 0, 0, 0,
				0, 0, 0, 0, 0, ':', '{', ',', '}', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, ':', '{', ',', '}', 0, 0 };
			SimdBase opTable{ arrayNew };
			SimdBase chars{ uint8_t{ 0x20 } };

			for (uint64_t x = 0; x < 8; ++x) {
				addValues(valuesNew[x].shuffle(opTable) == (valuesNew[x] | chars), x);
			}
		}

		inline void convertQuotesToSimdBase(const SimdBase* valuesNew) {
			for (uint64_t x = 0; x < 8; ++x) {
				addValues(valuesNew[x] == '"', x);
			}
		}

		inline SimdBase bitAndNot(const SimdBase& other) const {
			return _mm512_andnot_si512(other, value);
		}

		inline SimdBase shuffle(SimdBase& other) const {
			return _mm512_shuffle_epi8(other, value);
		}

		inline void insertInt64(int64_t valueNew, uint64_t index) {
			switch (index) {
				case 0: {
					auto lower = _mm512_extracti64x4_epi64(value, 0);
					lower	   = _mm256_insert_epi64(lower, valueNew, 0);
					value	   = _mm512_inserti64x4(value, lower, 0);
					break;
				}
				case 1: {
					auto lower = _mm512_extracti64x4_epi64(value, 0);
					lower	   = _mm256_insert_epi64(lower, valueNew, 1);
					value	   = _mm512_inserti64x4(value, lower, 0);
					break;
				}
				case 2: {
					auto lower = _mm512_extracti64x4_epi64(value, 0);
					lower	   = _mm256_insert_epi64(lower, valueNew, 2);
					value	   = _mm512_inserti64x4(value, lower, 0);
					break;
				}
				case 3: {
					auto lower = _mm512_extracti64x4_epi64(value, 0);
					lower	   = _mm256_insert_epi64(lower, valueNew, 3);
					value	   = _mm512_inserti64x4(value, lower, 0);
					break;
				}
				case 4: {
					auto lower = _mm512_extracti64x4_epi64(value, 1);
					lower	   = _mm256_insert_epi64(lower, valueNew, 0);
					value	   = _mm512_inserti64x4(value, lower, 1);
					break;
				}
				case 5: {
					auto lower = _mm512_extracti64x4_epi64(value, 1);
					lower	   = _mm256_insert_epi64(lower, valueNew, 1);
					value	   = _mm512_inserti64x4(value, lower, 1);
					break;
				}
				case 6: {
					auto lower = _mm512_extracti64x4_epi64(value, 1);
					lower	   = _mm256_insert_epi64(lower, valueNew, 2);
					value	   = _mm512_inserti64x4(value, lower, 1);
					break;
				}
				case 7: {
					auto lower = _mm512_extracti64x4_epi64(value, 1);
					lower	   = _mm256_insert_epi64(lower, valueNew, 3);
					value	   = _mm512_inserti64x4(value, lower, 1);
					break;
				}
				default: {
					auto lower = _mm512_extracti64x4_epi64(value, 0);
					lower	   = _mm256_insert_epi64(lower, valueNew, 0);
					value	   = _mm512_inserti64x4(value, lower, 0);
					break;
				}
			}
		}

		inline void addValues(uint64_t&& values, uint64_t index) {
			insertInt64(values, index);
		}

		template<uint64_t amount> inline SimdBase shl() const {
			SimdBase newValue{ _mm512_slli_epi64(*this, (amount % 64)) };
			SimdBase newerValue{ _mm512_srli_epi64(_mm512_permutexvar_epi64(_mm512_set_epi64(6, 5, 4, 3, 2, 1, 0, 7), *this), 64 - (amount % 64)) };
			uint64_t maxValue{ std::numeric_limits<uint64_t>::max() };
			SimdBase newestValue{ _mm512_set_epi64(maxValue, maxValue, maxValue, maxValue, maxValue, maxValue, maxValue, 0) };
			return SimdBase{ newValue | (newerValue & newestValue) };
		}

		inline int64_t toBitMask() const {
			return _mm512_movepi8_mask(*this);
		}

		inline void reset() {
			value = _mm512_setzero_si512();
		}

		template<typename ValueType> inline void store(ValueType* destVector) const {
			_mm512_store_si512(destVector, value);
		}

		inline SimdBase& setLSB(bool valueNew) {
			if (valueNew) {
				*this = _mm512_or_si512(*this, _mm512_set_epi64(0, 0, 0, 0, 0, 0, 0, 0x1));
			} else {
				*this = _mm512_andnot_si512(_mm512_set_epi64(0, 0, 0, 0, 0, 0, 0, 0x1), *this);
			}
			return *this;
		}

		inline bool checkMSB() const {
			__m512i result = _mm512_and_si512(*this, _mm512_set_epi64(0x8000000000000000, 0, 0, 0, 0, 0, 0, 0));
			return _mm512_test_epi64_mask(result, result);
		}

		inline SimdBase carrylessMultiplication(uint64_t& prevInString) const {
			SimdBase<256> lowValues{ _mm512_extracti64x4_epi64(value, 0) };
			SimdBase<256> highValues{ _mm512_extracti64x4_epi64(value, 1) };
			lowValues  = lowValues.carrylessMultiplication(prevInString);
			highValues = highValues.carrylessMultiplication(prevInString);
			SimdBase returnValue{};
			returnValue = _mm512_inserti64x4(returnValue, lowValues, 0);
			returnValue = _mm512_inserti64x4(returnValue, highValues, 1);
			return returnValue;
		}

		inline bool collectCarries(const SimdBase& other1, SimdBase& result) const {
			__m512i carry = _mm512_set1_epi64(0);
			__m512i sum	  = _mm512_add_epi64(*this, other1);
			sum			  = _mm512_add_epi64(sum, carry);
			result		  = std::move(sum);
			return _mm512_test_epi64_mask(carry, sum) == 0;
		}

		inline SimdBase follows(bool& overflow) {
			SimdBase result = shl<1>();
			result.setLSB(overflow);
			overflow = checkMSB();
			return result;
		}

		inline void printBits(uint64_t values, const std::string& valuesTitle) const {
			std::cout << valuesTitle;
			std::cout << std::bitset<64>{ values };
			std::cout << std::endl;
		}

		inline SimdBase printBits(const std::string& valuesTitle) const {
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

	inline SimdBaseReal makeSimdBase(uint64_t value) {
		return _mm512_set1_epi64(value);
	}

	#define popcnt(x) _mm_popcnt_u64(x)
	#define blsr(x) _blsr_u64(x)
	#define load(x) _mm512_load_si512(x)
	#define tzCount(x) _tzcnt_u64(x)
	#define tzCount64(x) _tzcnt_u64(x)

#elif defined T_AVX2

	template<typename CharacterType>
	concept CharType = requires(CharacterType) { sizeof(CharacterType) == 1; };

	template<typename ValueType>
	concept Avx256T = std::same_as<__m256i, ValueType> || std::same_as<__m256, ValueType>;

	constexpr uint64_t StepSize{ 256 };
	constexpr uint64_t BytesPerStep{ StepSize / 8 };
	constexpr uint64_t SixtyFourPer{ StepSize / 64 };
	using StringParsingType = uint32_t;
	using SimdBaseReal		= SimdBase<256>;
	using AvxType			= __m256i;

	template<> struct SimdBase<256> {
	  public:
		inline SimdBase() noexcept = default;

		inline SimdBase& operator=(SimdBase&& other) noexcept	   = default;
		inline SimdBase(SimdBase&& other) noexcept				   = default;
		inline SimdBase& operator=(const SimdBase& other) noexcept = delete;
		inline SimdBase(const SimdBase& other) noexcept			   = delete;

		template<Avx256T ValueType> inline SimdBase& operator=(ValueType&& data) {
			value = std::forward<ValueType>(data);
			return *this;
		}

		template<Avx256T ValueType> inline SimdBase(ValueType&& data) {
			value = std::forward<ValueType>(data);
		}

		inline SimdBase& operator=(uint8_t other) {
			value = _mm256_set1_epi8(other);
			return *this;
		}

		inline explicit SimdBase(uint8_t other) {
			*this = other;
		}

		template<CharType ValueType> inline SimdBase& operator=(ValueType values[32]) {
			value = gatherValues256<ValueType>(values);
			return *this;
		}

		template<CharType ValueType> inline SimdBase(ValueType values[32]) {
			*this = values;
		}

		inline operator __m256i&() {
			return value;
		}

		inline operator const __m256i&&() const {
			return std::forward<const __m256i>(value);
		}

		inline explicit operator bool() const {
			return !_mm256_testz_si256(value, value);
		}

		inline operator uint32_t() const {
			return toBitMask();
		}

		inline SimdBase operator|(SimdBase&& other) noexcept {
			return _mm256_or_si256(value, std::forward<__m256i>(other));
		}

		inline SimdBase operator|(const SimdBase& other) const {
			return _mm256_or_si256(value, other);
		}

		inline SimdBase operator-(const SimdBase& other) const {
			return _mm256_sub_epi8(value, other);
		}

		inline SimdBase operator&(const SimdBase& other) const {
			return _mm256_and_si256(value, other);
		}

		inline SimdBase operator^(const SimdBase& other) const {
			return _mm256_xor_si256(value, other);
		}

		inline StringParsingType operator==(const SimdBase& other) const {
			SimdBase newValue = _mm256_cmpeq_epi8(value, other);
			return newValue.toBitMask();
		}

		inline StringParsingType operator==(const uint8_t& other) const {
			SimdBase newValue = _mm256_cmpeq_epi8(value, _mm256_set1_epi8(other));
			return newValue.toBitMask();
		}

		inline SimdBase operator~() const {
			return _mm256_xor_si256(*this, _mm256_set1_epi64x(std::numeric_limits<uint64_t>::max()));
		}

		inline void convertWhitespaceToSimdBase(const SimdBase* valuesNew) {
			alignas(ALIGNMENT) uint8_t arrayNew[32]{ ' ', 100, 100, 100, 17, 100, 113, 2, 100, '\t', '\n', 112, 100, '\r', 100, 100, ' ', 100, 100, 100, 17, 100, 113, 2, 100, '\t',
				'\n', 112, 100, '\r', 100, 100 };
			SimdBase whitespaceTable{ arrayNew };
			for (uint64_t x = 0; x < 8; ++x) {
				addValues(valuesNew[x].shuffle(whitespaceTable) == valuesNew[x], x);
			}
		};

		inline void convertBackslashesToSimdBase(const SimdBase* valuesNew) {
			SimdBase backslashes{ _mm256_set1_epi8('\\') };
			for (uint64_t x = 0; x < 8; ++x) {
				addValues(valuesNew[x] == backslashes, x);
			}
		};

		inline void convertStructuralsToSimdBase(const SimdBase* valuesNew) {
			alignas(ALIGNMENT) uint8_t arrayNew[32]{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, ':', '{', ',', '}', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, ':', '{', ',', '}', 0, 0 };
			SimdBase opTable{ arrayNew };
			SimdBase chars{ uint8_t{ 0x20 } };
			for (uint64_t x = 0; x < 8; ++x) {
				addValues(valuesNew[x].shuffle(opTable) == (valuesNew[x] | chars), x);
			}
		};

		inline void convertQuotesToSimdBase(const SimdBase* valuesNew) {
			SimdBase quotes = _mm256_set1_epi8('"');
			for (uint64_t x = 0; x < 8; ++x) {
				addValues(valuesNew[x] == quotes, x);
			}
		}

		inline uint64_t getUint64(uint64_t index) {
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

		inline int64_t getInt64(uint64_t index) {
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

		inline void insertInt64(int64_t valueNew, uint64_t index) {
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

		inline void insertInt32(uint32_t valueNew, uint64_t index) {
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

		inline SimdBase bitAndNot(const SimdBase& other) const {
			return _mm256_andnot_si256(other, value);
		}

		inline SimdBase shuffle(const SimdBase& other) const {
			return _mm256_shuffle_epi8(other, value);
		}

		inline void addValues(uint32_t values, uint64_t index) {
			insertInt32(values, index);
		}

		template<uint64_t amount> inline SimdBase shl() const {
			return SimdBase{ _mm256_slli_epi64(*this, (amount % 64)) } | _mm256_srli_epi64(_mm256_permute4x64_epi64(*this, 0b10010011), 64 - (amount % 64));
		}

		inline uint32_t toBitMask() const {
			return _mm256_movemask_epi8(*this);
		}

		inline void reset() {
			value = _mm256_setzero_si256();
		}

		template<typename ValueType> inline void store(ValueType* storageLocation) {
			alignas(ALIGNMENT) float newArray[8]{};
			_mm256_store_ps(newArray, _mm256_castsi256_ps(value));
			std::memcpy(storageLocation, newArray, sizeof(value));
		}

		inline SimdBase& setLSB(bool valueNew) {
			if (valueNew) {
				*this = _mm256_or_si256(*this, _mm256_set_epi64x(0, 0, 0, 0x1));
			} else {
				*this = _mm256_andnot_si256(_mm256_set_epi64x(0, 0, 0, 0x1), *this);
			}
			return *this;
		}

		inline bool checkMSB() const {
			__m256i result = _mm256_and_si256(*this, _mm256_set_epi64x(0x8000000000000000, 0, 0, 0));
			return !_mm256_testz_si256(result, result);
		}

		inline SimdBase carrylessMultiplication(uint64_t& prevInString) const {
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

		inline bool collectCarries(SimdBase<256>& other1, SimdBase<256>& result) {
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

		inline SimdBase follows(bool& overflow) const {
			SimdBase result = shl<1>();
			result.setLSB(overflow);
			overflow = checkMSB();
			return result;
		}

		inline void printBits(uint64_t values, const std::string& valuesTitle) const {
			std::cout << valuesTitle;
			std::cout << std::bitset<64>{ values };
			std::cout << std::endl;
		}

		inline SimdBase<256>& printBits(const std::string& valuesTitle) {
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

	inline SimdBaseReal makeSimdBase(uint64_t value) {
		return _mm256_set1_epi64x(value);
	}

	#define popcnt(x) _mm_popcnt_u64(x)
	#define blsr(x) _blsr_u64(x)
	#define load(x) gatherValues256<std::remove_pointer_t<decltype(x)>>(x)
	#define tzCount(x) _tzcnt_u32(x)
	#define tzCount64(x) _tzcnt_u64(x)

#elif defined T_AVX

	template<typename CharacterType>
	concept CharType = requires(CharacterType) { sizeof(CharacterType) == 1; };

	template<typename ValueType>
	concept Avx128T = std::same_as<__m128i, ValueType> || std::same_as<__m128, ValueType>;

	constexpr uint64_t StepSize{ 128 };
	constexpr uint64_t BytesPerStep{ StepSize / 8 };
	constexpr uint64_t SixtyFourPer{ StepSize / 64 };
	using StringParsingType = uint16_t;
	using SimdBaseReal		= SimdBase<128>;
	using AvxType			= __m128i;

	template<> struct SimdBase<128> {
	  public:
		inline SimdBase() noexcept = default;

		inline SimdBase& operator=(SimdBase&& other) noexcept	   = default;
		inline SimdBase(SimdBase&& other) noexcept				   = default;
		inline SimdBase& operator=(const SimdBase& other) noexcept = delete;
		inline SimdBase(const SimdBase& other) noexcept			   = delete;

		template<Avx128T ValueType> inline SimdBase& operator=(ValueType&& data) {
			value = std::forward<ValueType>(data);
			return *this;
		}

		template<Avx128T ValueType> inline SimdBase(ValueType&& data) {
			value = std::forward<ValueType>(data);
		}

		inline SimdBase& operator=(uint8_t other) {
			value = _mm_set1_epi8(other);
			return *this;
		}

		inline explicit SimdBase(uint8_t other) {
			*this = other;
		}

		inline SimdBase& operator=(const uint8_t values[16]) {
			value = gatherValues128<const uint8_t>(values);
			return *this;
		}

		inline SimdBase(const uint8_t* values) {
			*this = values;
		}

		inline operator __m128i&() {
			return value;
		}

		inline operator const __m128i&&() const {
			return std::forward<const __m128i>(value);
		}

		inline explicit operator bool() const {
			return !_mm_testz_si128(value, value);
		}

		inline operator uint16_t() const {
			return toBitMask();
		}

		inline SimdBase operator|(SimdBase&& other) noexcept {
			return _mm_or_si128(value, std::forward<__m128i>(other));
		}

		inline SimdBase operator-(SimdBase&& other) noexcept {
			return _mm_sub_epi8(value, std::forward<__m128i>(other));
		}

		inline SimdBase operator|(const SimdBase& other) const {
			return _mm_or_si128(value, other);
		}

		inline SimdBase operator&(const SimdBase& other) const {
			return _mm_and_si128(value, other);
		}

		inline SimdBase operator-(const SimdBase& other) const {
			return _mm_sub_epi8(value, other);
		}

		inline SimdBase operator^(const SimdBase& other) const {
			return _mm_xor_si128(value, other);
		}

		inline StringParsingType operator==(const SimdBase& other) const {
			SimdBase newValue = _mm_cmpeq_epi8(value, other);
			return newValue.toBitMask();
		}

		inline StringParsingType operator==(const uint8_t& other) const {
			SimdBase newValue = _mm_cmpeq_epi8(value, _mm_set1_epi8(other));
			return newValue.toBitMask();
		}

		inline SimdBase operator~() const {
			return _mm_xor_si128(*this, _mm_set1_epi64x(std::numeric_limits<uint64_t>::max()));
		}

		inline void convertWhitespaceToSimdBase(const SimdBase* valuesNew) {
			alignas(ALIGNMENT) uint8_t arrayNew[16]{ ' ', 100, 100, 100, 17, 100, 113, 2, 100, '\t', '\n', 112, 100, '\r', 100, 100 };
			SimdBase whitespaceTable{ arrayNew };
			for (uint64_t x = 0; x < 8; ++x) {
				addValues(valuesNew[x].shuffle(whitespaceTable) == valuesNew[x], x);
			}
		};

		inline void convertBackslashesToSimdBase(const SimdBase* valuesNew) {
			SimdBase backslashes{ _mm_set1_epi8('\\') };
			for (uint64_t x = 0; x < 8; ++x) {
				addValues(valuesNew[x] == backslashes, x);
			}
		};

		inline void convertStructuralsToSimdBase(const SimdBase* valuesNew) {
			alignas(ALIGNMENT) uint8_t arrayNew[16]{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, ':', '{', ',', '}', 0, 0 };
			SimdBase opTable{ arrayNew };
			SimdBase chars{ uint8_t{ 0x20 } };
			for (uint64_t x = 0; x < 8; ++x) {
				addValues(valuesNew[x].shuffle(opTable) == (valuesNew[x] | chars), x);
			}
		};

		inline void convertQuotesToSimdBase(const SimdBase* valuesNew) {
			SimdBase quotes = _mm_set1_epi8('"');
			for (uint64_t x = 0; x < 8; ++x) {
				addValues(valuesNew[x] == quotes, x);
			}
		}

		inline uint64_t getUint64(uint64_t index) const {
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

		inline int64_t getInt64(uint64_t index) const {
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

		inline void insertInt64(int64_t valueNew, uint64_t index) {
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

		inline void insertInt32(uint32_t valueNew, uint64_t index) {
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

		inline void insertInt16(int16_t valueNew, uint64_t index) {
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

		inline SimdBase bitAndNot(const SimdBase& other) const {
			return _mm_andnot_si128(other, value);
		}

		inline SimdBase shuffle(const SimdBase& other) const {
			return _mm_shuffle_epi8(other, value);
		}

		inline void addValues(uint16_t values, uint64_t index) {
			insertInt16(values, index);
		}

		template<uint64_t amount> inline SimdBase shl() const {
			SimdBase currentValues{};
			currentValues.insertInt64(getUint64(0) << amount, 0);
			size_t shiftBetween = amount % 64;
			currentValues.insertInt64((getUint64(1) << amount) | (getUint64(0) >> (64 - shiftBetween)), 1);
			return currentValues;
		}

		inline int16_t toBitMask() const {
			return static_cast<int16_t>(_mm_movemask_epi8(*this));
		}

		inline void reset() {
			value = _mm_setzero_si128();
		}

		template<typename ValueType> inline void store(ValueType* storageLocation) {
			alignas(ALIGNMENT) float newArray[4]{};
			_mm_store_ps(newArray, _mm_castsi128_ps(value));
			std::memcpy(storageLocation, newArray, sizeof(value));
		}

		inline SimdBase& setLSB(bool valueNew) {
			if (valueNew) {
				*this = _mm_or_si128(*this, _mm_set_epi64x(0, 0x1));
			} else {
				*this = _mm_andnot_si128(_mm_set_epi64x(0, 0x1), *this);
			}
			return *this;
		}

		inline bool checkMSB() const {
			__m128i result = _mm_and_si128(*this, _mm_set_epi64x(0x8000000000000000, 0));
			return !_mm_testz_si128(result, result);
		}

		inline SimdBase carrylessMultiplication(uint64_t& prevInString) const {
			__m128i allOnes{ _mm_set1_epi8('\xFF') };
			SimdBase valuesNew{};
			__m128i valueLow{ value };
			valuesNew.insertInt64(_mm_cvtsi128_si64(_mm_clmulepi64_si128(valueLow, allOnes, 0)) ^ prevInString, 0);
			prevInString = uint64_t(static_cast<int64_t>(valuesNew.getUint64(0)) >> 63);
			valuesNew.insertInt64(_mm_cvtsi128_si64(_mm_clmulepi64_si128(valueLow, allOnes, 1)) ^ prevInString, 1);
			prevInString = uint64_t(static_cast<int64_t>(valuesNew.getUint64(1)) >> 63);
			return valuesNew;
		}

		inline bool collectCarries(SimdBase<128>& other1, SimdBase<128>& result) {
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

		inline SimdBase follows(bool& overflow) const {
			SimdBase result = shl<1>();
			result.setLSB(overflow);
			overflow = checkMSB();
			return result;
		}

		inline void printBits(uint64_t values, const std::string& valuesTitle) const {
			std::cout << valuesTitle;
			std::cout << std::bitset<64>{ values };
			std::cout << std::endl;
		}

		inline const SimdBase<128>& printBits(const std::string& valuesTitle) const {
			std::cout << valuesTitle;
			for (uint64_t x = 0; x < 16; ++x) {
				for (uint64_t y = 0; y < 8; ++y) {
					std::cout << std::bitset<1>{ static_cast<uint64_t>(*(reinterpret_cast<const int8_t*>(&value) + x)) >> y };
				}
			}
			std::cout << std::endl;
			return *this;
		}

	  protected:
		__m128i value{};
	};

	inline SimdBaseReal makeSimdBase(uint64_t value) {
		return _mm_set1_epi64x(value);
	}

	#define popcnt(x) _mm_popcnt_u64(x)
	#define blsr(x) _blsr_u64(x)
	#define load(x) gatherValues128<std::remove_pointer_t<decltype(x)>>(x)
	#define tzCount(x) _tzcnt_u16(x)
	#define tzCount64(x) _tzcnt_u64(x)

#else

	template<typename CharacterType>
	concept CharType = requires(CharacterType) { sizeof(CharacterType) == 1; };

	template<typename ValueType>
	concept Avx128T = std::same_as<__m128I, ValueType>;

	constexpr uint64_t StepSize{ 128 };
	constexpr uint64_t BytesPerStep{ StepSize / 8 };
	constexpr uint64_t SixtyFourPer{ StepSize / 64 };
	using StringParsingType = uint16_t;
	using SimdBaseReal		= SimdBase<128>;
	using AvxType			= __m128I;

	inline int16_t movemaskEpi8(const __m128I& a) {
		int16_t result = 0;
		for (int16_t i = 0; i < 2; ++i) {
			for (int16_t j = 0; j < 8; ++j) {
				uint8_t value = (a.values[i] >> (j * 8)) & 0xFF;
				int16_t mask	  = (value >> 7) & 1;
				result |= mask << (i * 8 + j);
			}
		}
		return result;
	}

	inline __m128I orSi128(const __m128I& valOne, const __m128I& valTwo) {
		__m128I value{};
		value.values[0] = valOne.values[0] | valTwo.values[0];
		value.values[1] = valOne.values[1] | valTwo.values[1];
		return value;
	}

	inline __m128I andSi128(const __m128I& valOne, const __m128I& valTwo) {
		__m128I value{};
		value.values[0] = valOne.values[0] & valTwo.values[0];
		value.values[1] = valOne.values[1] & valTwo.values[1];
		return value;
	}

	inline __m128I andNotSi128(const __m128I& valOne, const __m128I& valTwo) {
		__m128I value{};
		value.values[0] = valOne.values[0] & ~valTwo.values[0];
		value.values[1] = valOne.values[1] & ~valTwo.values[1];
		return value;
	}

	inline __m128I xorSi128(const __m128I& valOne, const __m128I& valTwo) {
		__m128I value{};
		value.values[0] = valOne.values[0] ^ valTwo.values[0];
		value.values[1] = valOne.values[1] ^ valTwo.values[1];
		return value;
	}

	inline __m128I subEpi8(const __m128I& valOne, const __m128I& valTwo) {
		__m128I result;
		result.values[0] = valOne.values[0] - valTwo.values[0];
		result.values[1] = valOne.values[1] - valTwo.values[1];
		return result;
	}

	inline __m128I notSi128(const __m128I& valOne) {
		__m128I result{};

		for (int64_t i = 0; i < 2; ++i) {
			result.values[i] = ~valOne.values[i];
		}

		return result;
	}

	inline __m128I cmpeqEpi8(const __m128I& a,const __m128I& b) {
		__m128I result;
		for (int i = 0; i < 2; ++i) {
			for (int j = 0; j < 8; ++j) {
				uint8_t a8 = (a.values[i] >> (j * 8)) & 0xFF;
				uint8_t b8 = (b.values[i] >> (j * 8)) & 0xFF;
				result.values[i] |= (a8 == b8 ? 0xFFULL : 0) << (j * 8);
			}
		}
		return result;
	}

	inline bool testzSi128(const __m128I& valOne, const __m128I& valTwo) {
		__m128I result{};
		result.values[0] = valOne.values[0] & valTwo.values[0];
		result.values[1] = valOne.values[1] & valTwo.values[1];

		return (result.values[0] == 0) && (result.values[1] == 0);
	}

	inline __m128I setEpi64x(uint64_t argOne, uint64_t argTwo) {
		__m128I returnValue{};
		returnValue.values[0] = argOne;
		returnValue.values[1] = argTwo;
		return returnValue;
	}

	inline __m128I set1Epi64x(uint64_t argOne) {
		__m128I returnValue{};
		returnValue.values[0] = argOne;
		returnValue.values[1] = argOne;
		return returnValue;
	}

	inline __m128I insertUint16(__m128I value, int64_t position, uint16_t newValue) {
		if (position < 0 || position >= 8) {
			return value;
		}
		int arrayIndex	= (position < 4) ? 0 : 1;
		int shiftAmount = (position % 4) * 16;
		value.values[arrayIndex] &= ~(static_cast<uint64_t>(0xFFFF) << shiftAmount);
		value.values[arrayIndex] |= (static_cast<uint64_t>(newValue) << shiftAmount);
		return value;
	}

	inline __m128I insertUint32(__m128I value, int64_t position, uint32_t newValue) {
		if (position < 0 || position >= 4) {
			return value;
		}

		int arrayIndex = (position < 2) ? 0 : 1;

		int shiftAmount = (position % 2) * 32;

		value.values[arrayIndex] &= ~(static_cast<uint64_t>(0xFFFFFFFF) << shiftAmount);
		value.values[arrayIndex] |= (static_cast<uint64_t>(newValue) << shiftAmount);
		return value;
	}

	inline __m128I insertUint64(__m128I value, int64_t position, uint64_t newValue) {
		if (position >= 0 && position < 4) {
			int64_t index = position / 2;

			int64_t offset = (position % 2) * 64;

			value.values[index] &= ~(0xFFFFFFFFFFFFFFFFULL << offset);
			value.values[index] |= static_cast<uint64_t>(newValue) << offset;
		}
		return value;
	}

	inline uint32_t extractUint32(__m128I value, int64_t index) {
		if (index >= 0 && index < 4) {
			int64_t valueIndex = index / 2;

			int64_t offset = (index % 2) * 32;

			uint32_t extractedValue = static_cast<uint32_t>((value.values[valueIndex] >> offset) & 0xFFFFFFFFULL);
			return extractedValue;
		}

		return 0;
	}

	inline uint64_t extractUint64(__m128I value, int64_t index) {
		if (index >= 0 && index < 2) {
			int64_t valueIndex = index / 2;

			int64_t offset = (index % 2) * 64;

			uint64_t extractedValue = static_cast<uint64_t>((value.values[valueIndex] >> offset) & 0xFFFFFFFFFFFFFFFFULL);
			return extractedValue;
		}

		return 0;
	}

	inline __m128I shuffleM128I(const __m128I& value1, const __m128I& value2) {
		__m128I result;

		for (int64_t i = 0; i < 2; ++i) {
			result.values[i] = (value1.values[i] & 0xFF) | ((value2.values[i] & 0xFF) << 8);
			result.values[i] |= ((value1.values[i] & 0xFF00) << 8) | ((value2.values[i] & 0xFF00) << 16);
			result.values[i] |= ((value1.values[i] & 0xFF0000) << 16) | ((value2.values[i] & 0xFF0000) << 24);
			result.values[i] |= ((value1.values[i] & 0xFF000000) << 24) | ((value2.values[i] & 0xFF000000) << 32);
			result.values[i] |= ((value1.values[i] & 0xFF00000000) << 32) | ((value2.values[i] & 0xFF00000000) << 40);
			result.values[i] |= ((value1.values[i] & 0xFF0000000000) << 40) | ((value2.values[i] & 0xFF0000000000) << 48);
			result.values[i] |= ((value1.values[i] & 0xFF000000000000) << 48) | ((value2.values[i] & 0xFF000000000000) << 56);
		}

		return result;
	}

	inline __m128I set1Epi8(int8_t newValue) {
		__m128I returnValue{};
		std::memset(&returnValue, newValue, sizeof(__m128I));
		return returnValue;
	}

	inline SimdBaseReal makeSimdBase(uint64_t value);

	template<> struct SimdBase<128> {
	  public:
		inline SimdBase() noexcept = default;

		inline SimdBase& operator=(SimdBase&& other) noexcept	   = default;
		inline SimdBase(SimdBase&& other) noexcept				   = default;
		inline SimdBase& operator=(const SimdBase& other) noexcept = delete;
		inline SimdBase(const SimdBase& other) noexcept			   = delete;

		template<Avx128T ValueType> inline SimdBase& operator=(ValueType&& data) {
			value = std::forward<ValueType>(data);
			return *this;
		}

		template<Avx128T ValueType> inline SimdBase(ValueType&& data) {
			value = std::forward<ValueType>(data);
		}

		inline SimdBase& operator=(uint8_t other) {
			value = set1Epi8(other);
			return *this;
		}

		inline explicit SimdBase(uint8_t other) {
			*this = other;
		}

		template<CharType ValueType> inline SimdBase& operator=(ValueType values[16]) {
			value = gatherValues128<ValueType>(values);
			return *this;
		}

		template<CharType ValueType> inline SimdBase(ValueType values[16]) {
			*this = values;
		}

		inline operator __m128I&() {
			return value;
		}

		inline operator const __m128I&() const {
			return value;
		}

		inline operator const __m128I&&() const {
			return std::forward<const __m128I>(value);
		}

		inline explicit operator bool() const {
			return value.values[0] & value.values[1];
		}

		inline operator uint64_t() const {
			return toBitMask();
		}

		inline SimdBase operator|(SimdBase&& other) noexcept {
			return orSi128(value, std::forward<__m128I>(other));
		}

		inline SimdBase operator-(SimdBase&& other) noexcept {
			return subEpi8(value, std::forward<__m128I>(other));
		}

		inline SimdBase operator|(const SimdBase& other) const {
			return orSi128(value, other);
		}

		inline SimdBase operator&(const SimdBase& other) const {
			return andSi128(value, other);
		}

		inline SimdBase operator-(const SimdBase& other) const {
			return subEpi8(value, other);
		}

		inline SimdBase operator^(const SimdBase& other) const {
			return xorSi128(value, other);
		}

		inline StringParsingType operator==(const SimdBase& other) const {
			SimdBase newValue = cmpeqEpi8(value, other);
			//newValue.printBits("QUOTED BITS 01: ");
			return newValue.toBitMask();
		}

		inline StringParsingType operator==(const uint8_t& other) const {
			SimdBase newValue = cmpeqEpi8(value, set1Epi8(other));
			return newValue.toBitMask();
		}

		inline SimdBase operator~() const {
			return notSi128(value);
		}

		inline void convertWhitespaceToSimdBase(const SimdBase* valuesNew) {
			alignas(ALIGNMENT) uint8_t arrayNew[16]{ ' ', 100, 100, 100, 17, 100, 113, 2, 100, '\t', '\n', 112, 100, '\r', 100, 100 };
			SimdBase whitespaceTable{ arrayNew };
			for (uint64_t x = 0; x < 8; ++x) {
				addValues(valuesNew[x].shuffle(whitespaceTable) == valuesNew[x], x);
			}
		};

		inline void convertBackslashesToSimdBase(const SimdBase* valuesNew) {
			SimdBase backslashes{ set1Epi8('\\') };
			for (uint64_t x = 0; x < 8; ++x) {
				addValues(valuesNew[x] == backslashes, x);
			}
		};

		inline void convertStructuralsToSimdBase(const SimdBase* valuesNew) {
			alignas(ALIGNMENT) uint8_t arrayNew[16]{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, ':', '{', ',', '}', 0, 0 };
			SimdBase opTable{ arrayNew };
			SimdBase chars{ uint8_t{ 0x20 } };
			for (uint64_t x = 0; x < 8; ++x) {
				addValues(valuesNew[x].shuffle(opTable) == (valuesNew[x] | chars), x);
			}
		};

		inline void convertQuotesToSimdBase(const SimdBase* valuesNew) {
			SimdBase quotes{ set1Epi8('"') };
			for (uint64_t x = 0; x < 8; ++x) {
				addValues(valuesNew[x] == quotes, x);
			}
		}

		inline uint64_t getUint64(uint64_t index) const {
			return extractUint64(value, index);
		}

		inline int64_t getInt64(uint64_t index) const {
			return extractUint64(value, index);
		}

		inline void insertInt16(int16_t valueNew, uint64_t index) {
			value = insertUint16(value, index, valueNew);
		}

		inline void insertInt64(int64_t valueNew, uint64_t index) {
			value = insertUint64(value, index, valueNew);
		}

		inline SimdBase bitAndNot(const SimdBase& other) const {
			return andNotSi128(value, other.value);
		}

		inline SimdBase shuffle(const SimdBase& other) const {
			return shuffleM128I(other.value, value);
		}

		inline void addValues(uint16_t values, uint64_t index) {
			insertInt16(values, index);
		}

		template<uint64_t amount> inline SimdBase shl() const {
			SimdBase currentValues{};
			currentValues.insertInt64(getUint64(0) << amount, 0);
			size_t shiftBetween = amount % 64;
			currentValues.insertInt64((getUint64(1) << amount) | (getUint64(0) >> (64 - shiftBetween)), 1);
			return currentValues;
		}

		inline StringParsingType toBitMask() const {
			return movemaskEpi8(value);
		}

		inline void reset() {
			value.values[0] = 0;
			value.values[1] = 0;
		}

		template<typename ValueType> inline void store(ValueType* storageLocation) {
			std::memcpy(storageLocation, &value, sizeof(value));
		}

		inline SimdBase& setLSB(bool valueNew) {
			if (valueNew) {
				*this = orSi128(*this, setEpi64x(0, 0x1));
			} else {
				*this = andNotSi128(setEpi64x(0, 0x1), *this);
			}
			return *this;
		}

		inline bool checkMSB() const {
			__m128I result = andSi128(value, setEpi64x(0x8000000000000000, 0));
			return !testzSi128(result, result);
		}

		inline uint64_t prefixXor(uint64_t prevInString) {
			prevInString ^= prevInString << 1;
			prevInString ^= prevInString << 2;
			prevInString ^= prevInString << 4;
			prevInString ^= prevInString << 8;
			prevInString ^= prevInString << 16;
			prevInString ^= prevInString << 32;
			return prevInString;
		}

		inline SimdBase carrylessMultiplication(uint64_t& prevInString) {
			__m128I allOnes{ set1Epi8('\xFF') };
			SimdBase valuesNew{};
			__m128I valueLow{ value };
			valuesNew.insertInt64(prefixXor(valueLow.values[0]) ^ prevInString, 0);
			prevInString = uint64_t(static_cast<int64_t>(valuesNew.getUint64(0)) >> 63);
			valuesNew.insertInt64(prefixXor(valueLow.values[1]) ^ prevInString, 1);
			prevInString = uint64_t(static_cast<int64_t>(valuesNew.getUint64(1)) >> 63);
			return valuesNew;
		}

		inline bool collectCarries(SimdBase<128>& other1, SimdBase<128>& result) {
			bool returnValue{};
			long long unsigned returnValue128{};
			for (uint64_t x = 0; x < 2; ++x) {
				if (_addcarry_u64(0, getInt64(x), other1.getInt64(x), &returnValue128)) {
					returnValue = true;
				} else {
					returnValue = false;
				}
				result.insertInt64(returnValue128, x);
			}
			return returnValue;
		}

		inline SimdBase follows(bool& overflow) const {
			SimdBase result = shl<1>();
			result.setLSB(overflow);
			overflow = checkMSB();
			return result;
		}

		inline void printBits(uint64_t values, const std::string& valuesTitle) const {
			std::cout << valuesTitle;
			std::cout << std::bitset<128>{ values };
			std::cout << std::endl;
		}

		inline SimdBase<128>& printBits(const std::string& valuesTitle) {
			std::cout << valuesTitle;
			for (uint64_t x = 0; x < 2; ++x) {
				std::cout << std::bitset<64>{ value.values[x] };
			}
			std::cout << std::endl;
			return *this;
		}

	  protected:
		__m128I value{};
	};

	inline SimdBaseReal makeSimdBase(uint64_t value) {
		return set1Epi64x(value);
	}

	inline uint16_t tzcnt_u16(uint16_t value) {
		uint16_t count = 0;

		if (value == 0) {
			return 8;
		}

		while ((value & 1) == 0) {
			value >>= 1;
			count++;
		}

		return count;
	}

	inline uint64_t popcount_u64(uint64_t value) {
		uint64_t count = 0;

		while (value > 0) {
			count += value & 1;
			value >>= 1;
		}

		return count;
	}

	inline uint64_t blsr_u64(uint64_t value) {
		if (value == 0) {
			return 0;
		}

		uint64_t mask = 1ULL << 63;

		while ((value & mask) == 0) {
			mask >>= 1;
		}

		value &= ~mask;

		return value;
	}

	inline uint64_t tzcnt_u64(uint64_t value) {
		uint64_t count = 0;

		if (value == 0) {
			return 128;
		}

		while ((value & 1) == 0) {
			value >>= 1;
			count++;
		}

		return count;
	}

	#define popcnt(x) popcount_u64(x)
	#define blsr(x) blsr_u64(x)
	#define load(x) gatherValues128<std::remove_pointer_t<decltype(x)>>(x)
	#define tzCount(x) tzcnt_u16(x)
	#define tzCount64(x) tzcnt_u64(x)

#endif

}// namespace JsonifierInternal