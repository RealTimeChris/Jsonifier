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
	using StructuralIndex = uint32_t*;
	using StringBufferPtr = uint8_t*;

	template<size_t StepSize> struct SimdBase {};

#if AVX_TYPE == 124

	constexpr int32_t StepSize{ 512 };
	constexpr int32_t BytesPerStep{ StepSize / 8 };
	constexpr int32_t SixtyFourPer{ StepSize / 64 };
	using SimdBaseReal = SimdBase<512>;
	using StringParsingType = size_t;
	using AvxType = __m512i;

	template<> struct SimdBase<512> {
	  public:
		inline SimdBase() noexcept = default;

		inline SimdBase& operator=(SimdBase&& other) noexcept = default;
		inline SimdBase(SimdBase&& other) noexcept = default;
		inline SimdBase& operator=(const SimdBase& other) noexcept = default;
		inline SimdBase(const SimdBase& other) noexcept = default;

		inline SimdBase& operator=(__m512i&& other) noexcept {
			value = std::forward<__m512i>(other);
			return *this;
		}

		inline SimdBase(__m512i&& other) noexcept {
			*this = std::move(other);
		}

		inline SimdBase& operator=(char other) noexcept {
			value = _mm512_set1_epi8(other);
			return *this;
		}

		inline explicit SimdBase(char other) noexcept {
			*this = other;
		}

		inline SimdBase& operator=(const uint8_t values[64]) noexcept {
			value = _mm512_loadu_si512(reinterpret_cast<const __m512i*>(values));
			return *this;
		}

		inline explicit SimdBase(const uint8_t values[64]) noexcept {
			*this = values;
		}

		inline operator __m512i&() {
			return value;
		}

		inline operator const __m512i&&() const noexcept {
			return std::forward<const __m512i>(value);
		}

		inline explicit operator bool() const noexcept {
			__mmask64 mask = _mm512_test_epi64_mask(value, value);
			return mask != 0;
		}

		inline explicit operator size_t() const noexcept {
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

		inline size_t operator==(const SimdBase& other) const noexcept {
			return _mm512_cmpeq_epi8_mask(value, other);
		}

		inline size_t operator==(const uint8_t& other) const noexcept {
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

			for (size_t x = 0; x < 8; ++x) {
				addValues(valuesNew[x].shuffle(whitespaceTable) == valuesNew[x], x);
			}
		}

		inline void convertBackslashesToSimdBase(const SimdBase* valuesNew) noexcept {
			for (size_t x = 0; x < 8; ++x) {
				addValues(valuesNew[x] == '\\', x);
			}
		}

		inline void convertStructuralsToSimdBase(const SimdBase* valuesNew) noexcept {
			uint8_t arrayNew[64]{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, ':', '{', ',', '}', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, ':', '{', ',', '}', 0, 0, 0,
				0, 0, 0, 0, 0, 0, 0, 0, 0, ':', '{', ',', '}', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, ':', '{', ',', '}', 0, 0 };
			SimdBase opTable{ arrayNew };
			SimdBase chars{ char{ 0x20 } };

			for (size_t x = 0; x < 8; ++x) {
				addValues(valuesNew[x].shuffle(opTable) == (valuesNew[x] | chars), x);
			}
		}

		inline void convertQuotesToSimdBase(const SimdBase* valuesNew) noexcept {
			for (size_t x = 0; x < 8; ++x) {
				addValues(valuesNew[x] == '"', x);
			}
		}

		inline SimdBase bitAndNot(const SimdBase& other) const noexcept {
			return _mm512_andnot_si512(other, value);
		}

		inline SimdBase shuffle(SimdBase& other) const noexcept {
			return _mm512_shuffle_epi8(other, value);
		}

		inline void addValues(size_t&& values, size_t index) noexcept {
			*(reinterpret_cast<size_t*>(&value) + index) = values;
		}

		template<size_t amount> inline SimdBase shl() const noexcept {
			SimdBase newValue{ _mm512_slli_epi64(*this, (amount % 64)) };
			SimdBase newerValue{ _mm512_srli_epi64(_mm512_permutexvar_epi64(_mm512_set_epi64(6, 5, 4, 3, 2, 1, 0, 7), *this), 64 - (amount % 64)) };
			size_t maxValue{ std::numeric_limits<size_t>::max() };
			SimdBase newestValue{ _mm512_set_epi64(maxValue, maxValue, maxValue, maxValue, maxValue, maxValue, maxValue, 0) };
			return SimdBase{ newValue | (newerValue & newestValue) };
		}

		inline int64_t toBitMask() const noexcept {
			return _mm512_movepi8_mask(*this);
		}

		inline void reset() noexcept {
			value = _mm512_setzero_si512();
		}

		inline void store(uint8_t destVector[64]) const noexcept {
			_mm512_storeu_si512(reinterpret_cast<__m512i*>(destVector), value);
		}

		inline SimdBase& setLSB(bool value) noexcept {
			if (value) {
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

		inline __m256i carrylessMultiplication(__m256i valueNew, size_t& prevInString) const noexcept {
			__m128i allOnes{ _mm_set1_epi8('\xFF') };
			__m256i valuesNew{};
			__m128i valueLow{ _mm256_extracti128_si256(valueNew, 0) };
			__m128i valueHigh{ _mm256_extracti128_si256(valueNew, 1) };
			size_t* values = reinterpret_cast<size_t*>(&valuesNew);
			values[0] = _mm_cvtsi128_si64(_mm_clmulepi64_si128(valueLow, allOnes, 0)) ^ prevInString;
			prevInString = size_t(static_cast<int64_t>(values[0]) >> 63);
			values[1] = _mm_cvtsi128_si64(_mm_clmulepi64_si128(valueLow, allOnes, 1)) ^ prevInString;
			prevInString = size_t(static_cast<int64_t>(values[1]) >> 63);
			values[2] = _mm_cvtsi128_si64(_mm_clmulepi64_si128(valueHigh, allOnes, 0)) ^ prevInString;
			prevInString = size_t(static_cast<int64_t>(values[2]) >> 63);
			values[3] = _mm_cvtsi128_si64(_mm_clmulepi64_si128(valueHigh, allOnes, 1)) ^ prevInString;
			prevInString = size_t(static_cast<int64_t>(values[3]) >> 63);
			return valuesNew;
		}

		inline SimdBase carrylessMultiplication(size_t& prevInString) const noexcept {
			__m256i lowValues{ _mm512_extracti64x4_epi64(value, 0) };
			lowValues = carrylessMultiplication(lowValues, prevInString);
			__m256i highValues{ _mm512_extracti64x4_epi64(value, 1) };
			highValues = carrylessMultiplication(highValues, prevInString);
			SimdBase returnValue{};
			returnValue = _mm512_inserti64x4(returnValue, lowValues, 0);
			returnValue = _mm512_inserti64x4(returnValue, highValues, 1);
			return returnValue;
		}

		inline bool collectCarries(const SimdBase& other1, SimdBase& result) const noexcept {
			__m512i carry = _mm512_set1_epi64(0 ? 1 : 0);
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

		inline void printBits(size_t values, const std::string& valuesTitle) const noexcept {
			std::cout << valuesTitle;
			std::cout << std::bitset<64>{ values };
			std::cout << std::endl;
		}

		inline SimdBase printBits(const std::string& valuesTitle) const noexcept {
			std::cout << valuesTitle;
			for (size_t x = 0; x < 8; ++x) {
				std::cout << std::bitset<64>{ *(reinterpret_cast<const uint64_t*>(&value) + x) };
			}
			std::cout << std::endl;
			return *this;
		}

	  protected:
		__m512i value{};
	};

	#define store(x, y) _mm512_store_si512(x, y)
	#define loadu(x) _mm512_loadu_si512(x)
	#define set(x) _mm512_set1_epi8(x)
	#define tzCount(x) _tzcnt_u64(x)

#elif AVX_TYPE == 125

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
		inline SimdBase& operator=(const SimdBase& other) noexcept = default;
		inline SimdBase(const SimdBase& other) noexcept = default;

		inline SimdBase& operator=(__m256i&& other) noexcept {
			value = std::forward<__m256i>(other);
			return *this;
		}

		inline SimdBase(__m256i&& other) noexcept {
			*this = std::move(other);
		}

		inline SimdBase& operator=(char other) noexcept {
			value = _mm256_set1_epi8(other);
			return *this;
		}

		inline explicit SimdBase(char other) noexcept {
			*this = other;
		}

		inline SimdBase& operator=(StringViewPtr values) noexcept {
			value = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(values));
			return *this;
		}

		inline SimdBase(StringViewPtr values) noexcept {
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

		inline StringParsingType operator==(const char& other) const noexcept {
			SimdBase newValue = _mm256_cmpeq_epi8(value, _mm256_set1_epi8(other));
			return newValue.toBitMask();
		}

		inline SimdBase operator~() const noexcept {
			return _mm256_xor_si256(*this, _mm256_set1_epi64x(-1ll));
		}

		inline void convertWhitespaceToSimdBase(const SimdBase* valuesNew) noexcept {
			uint8_t arrayNew[32]{ ' ', 100, 100, 100, 17, 100, 113, 2, 100, '\t', '\n', 112, 100, '\r', 100, 100, ' ', 100, 100, 100, 17, 100, 113, 2,
				100, '\t', '\n', 112, 100, '\r', 100, 100 };
			SimdBase whitespaceTable{ arrayNew };
			for (size_t x = 0; x < 8; ++x) {
				addValues(valuesNew[x].shuffle(whitespaceTable) == valuesNew[x], x);
			}
		};

		inline void convertBackslashesToSimdBase(const SimdBase* valuesNew) noexcept {
			SimdBase backslashes{ _mm256_set1_epi8('\\') };
			for (size_t x = 0; x < 8; ++x) {
				addValues(valuesNew[x] == backslashes, x);
			}
		};

		inline void convertStructuralsToSimdBase(const SimdBase* valuesNew) noexcept {
			uint8_t arrayNew[32]{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, ':', '{', ',', '}', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, ':', '{', ',', '}', 0, 0 };
			SimdBase opTable{ arrayNew };
			SimdBase chars{ char{ 0x20 } };
			for (size_t x = 0; x < 8; ++x) {
				addValues(valuesNew[x].shuffle(opTable) == (valuesNew[x] | chars), x);
			}
		};

		inline void convertQuotesToSimdBase(const SimdBase* valuesNew) noexcept {
			SimdBase quotes = _mm256_set1_epi8('"');
			for (size_t x = 0; x < 8; ++x) {
				addValues(valuesNew[x] == quotes, x);
			}
		}

		inline SimdBase bitAndNot(const SimdBase& other) const noexcept {
			return _mm256_andnot_si256(other, value);
		}

		inline SimdBase shuffle(const SimdBase& other) const noexcept {
			return _mm256_shuffle_epi8(other, value);
		}

		inline void addValues(uint32_t values, size_t index) noexcept {
			*(reinterpret_cast<int32_t*>(&value) + index) = values;
		}

		template<size_t amount> inline SimdBase shl() const noexcept {
			return SimdBase{ _mm256_slli_epi64(*this, (amount % 64)) } |
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

		inline SimdBase carrylessMultiplication(size_t& prevInString) const noexcept {
			__m128i allOnes{ _mm_set1_epi8('\xFF') };
			SimdBase valuesNew{};
			__m128i valueLow{ _mm256_extracti128_si256(value, 0) };
			__m128i valueHigh{ _mm256_extracti128_si256(value, 1) };
			size_t* values = reinterpret_cast<size_t*>(&valuesNew);
			values[0] = _mm_cvtsi128_si64(_mm_clmulepi64_si128(valueLow, allOnes, 0)) ^ prevInString;
			prevInString = size_t(static_cast<int64_t>(values[0]) >> 63);
			values[1] = _mm_cvtsi128_si64(_mm_clmulepi64_si128(valueLow, allOnes, 1)) ^ prevInString;
			prevInString = size_t(static_cast<int64_t>(values[1]) >> 63);
			values[2] = _mm_cvtsi128_si64(_mm_clmulepi64_si128(valueHigh, allOnes, 0)) ^ prevInString;
			prevInString = size_t(static_cast<int64_t>(values[2]) >> 63);
			values[3] = _mm_cvtsi128_si64(_mm_clmulepi64_si128(valueHigh, allOnes, 1)) ^ prevInString;
			prevInString = size_t(static_cast<int64_t>(values[3]) >> 63);
			return valuesNew;
		}

		inline bool collectCarries(const SimdBase& other1, SimdBase& result) const noexcept {
			__m256i carry = _mm256_set1_epi64x(0 ? 1 : 0);
			__m256i sum = _mm256_add_epi64(*this, other1);
			sum = _mm256_add_epi64(sum, carry);
			result = std::move(sum);
			return _mm256_testz_si256(carry, sum) == 0;
		}

		inline SimdBase follows(bool& overflow) const noexcept {
			SimdBase result = shl<1>();
			result.setLSB(overflow);
			overflow = checkMSB();
			return result;
		}

		inline void printBits(size_t values, const std::string& valuesTitle) const noexcept {
			std::cout << valuesTitle;
			std::cout << std::bitset<64>{ values };
			std::cout << std::endl;
		}

		inline SimdBase printBits(const std::string& valuesTitle) const noexcept {
			std::cout << valuesTitle;
			for (size_t x = 0; x < 4; ++x) {
				std::cout << std::bitset<64>{ *(reinterpret_cast<const uint64_t*>(&value) + x) };
			}
			std::cout << std::endl;
			return *this;
		}

	  protected:
		__m256i value{};
	};

	#define store(x, y) _mm256_store_si256(x, y)
	#define loadu(x) _mm256_loadu_si256(x)
	#define set(x) _mm256_set1_epi8(x)
	#define tzCount(x) _tzcnt_u32(x)

#elif AVX_TYPE == 126

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
		inline SimdBase& operator=(const SimdBase& other) noexcept = default;
		inline SimdBase(const SimdBase& other) noexcept = default;

		inline SimdBase& operator=(__m128i&& other) noexcept {
			value = std::forward<__m128i>(other);
			return *this;
		}

		inline SimdBase(__m128i&& other) noexcept {
			*this = std::move(other);
		}

		inline explicit SimdBase(char other) noexcept {
			value = _mm_set1_epi8(other);
		}

		inline SimdBase& operator=(const uint8_t values[16]) noexcept {
			value = _mm_loadu_si128(reinterpret_cast<const __m128i*>(values));
			return *this;
		}

		inline SimdBase(const uint8_t values[16]) noexcept {
			*this = values;
		}

		inline void reset() noexcept {
			value = _mm_setzero_si128();
		}

		inline explicit operator bool() const noexcept {
			return !_mm_testz_si128(value, value);
		}

		inline operator uint16_t() const noexcept {
			return toBitMask();
		}

		inline SimdBase shuffle(const SimdBase& other) const noexcept {
			return _mm_shuffle_epi8(other, value);
		}

		inline SimdBase operator|(SimdBase&& other) noexcept {
			return _mm_or_si128(value, std::forward<__m128i>(other.value));
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

		inline StringParsingType operator==(const char& other) const noexcept {
			SimdBase newValue = _mm_cmpeq_epi8(value, _mm_set1_epi8(other));
			return newValue.toBitMask();
		}

		inline SimdBase operator~() const noexcept {
			return _mm_xor_si128(*this, _mm_set1_epi64x(-1ll));
		}

		inline SimdBase& setLSB(bool value) noexcept {
			if (value) {
				*this = _mm_or_si128(*this, _mm_set_epi64x(0, 0));
			} else {
				*this = _mm_andnot_si128(_mm_set_epi64x(0, 0), *this);
			}
			return *this;
		}

		inline bool checkLSB() const noexcept {
			__m128i result = _mm_and_si128(*this, _mm_set_epi64x(0, 0));
			return !_mm_testz_si128(result, result);
		}

		inline bool checkMSB() const noexcept {
			__m128i result = _mm_and_si128(*this, _mm_set_epi64x(0x8000000000000000, 1));
			return !_mm_testz_si128(result, result);
		}

		inline void convertWhitespaceToSimdBase(const SimdBase* valuesNew) noexcept {
			uint8_t arrayNew[16]{ ' ', 100, 100, 100, 17, 100, 113, 2, 100, '\t', '\n', 112, 100, '\r', 100, 100 };
			SimdBase whitespaceTable{ arrayNew };
			for (size_t x = 0; x < 8; ++x) {
				addValues(valuesNew[x].shuffle(whitespaceTable) == valuesNew[x], x);
			}
		};

		inline void addValues(uint16_t values, size_t index) noexcept {
			*(reinterpret_cast<int16_t*>(&value) + index) = values;
		}

		inline void convertBackslashesToSimdBase(const SimdBase* valuesNew) noexcept {
			SimdBase backslashes{ _mm_set1_epi8('\\') };
			for (size_t x = 0; x < 8; ++x) {
				addValues(valuesNew[x] == backslashes, x);
			}
		};

		inline void convertStructuralsToSimdBase(const SimdBase* valuesNew) noexcept {
			uint8_t arrayNew[16]{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, ':', '{', ',', '}', 0, 0 };
			SimdBase opTable{ arrayNew };
			SimdBase chars{ char{ 0x20 } };
			for (size_t x = 0; x < 8; ++x) {
				addValues({ valuesNew[x].shuffle(opTable) == (valuesNew[x] | chars) }, x);
			}
		};

		inline void convertQuotesToSimdBase(const SimdBase* valuesNew) noexcept {
			SimdBase quotes = _mm_set1_epi8('"');
			for (size_t x = 0; x < 8; ++x) {
				addValues(valuesNew[x] == quotes, x);
			}
		}

		inline SimdBase carrylessMultiplication(size_t& prevInString) const noexcept {
			SimdBase allOnes{ '\xFF' };
			SimdBase valuesNew{};
			__m128i valueLow{ value };
			size_t* values = reinterpret_cast<size_t*>(&valuesNew);
			values[0] = _mm_cvtsi128_si64(_mm_clmulepi64_si128(valueLow, allOnes, 0)) ^ prevInString;
			prevInString = size_t(static_cast<int64_t>(values[0]) >> 63);
			values[1] = _mm_cvtsi128_si64(_mm_clmulepi64_si128(valueLow, allOnes, 1)) ^ prevInString;
			prevInString = size_t(static_cast<int64_t>(values[1]) >> 63);
			return valuesNew;
		}

		inline bool collectCarries(const SimdBase& other1, SimdBase& result) const noexcept {
			__m128i carry = _mm_set1_epi64x(0 ? 1 : 0);
			__m128i sum = _mm_add_epi64(*this, other1);
			sum = _mm_add_epi64(sum, carry);
			result = std::move(sum);
			return _mm_testz_si128(carry, sum) == 0;
		}

		inline SimdBase follows(bool& overflow) const noexcept {
			SimdBase result = shl<1>();
			result.setLSB(overflow);
			overflow = checkMSB();
			return result;
		}

		template<size_t amount> inline SimdBase shl() const noexcept {
			int64_t shiftAmount = amount % 64;
			__m128i shifted = _mm_slli_epi64(value, shiftAmount);

			__m128i permuted = _mm_shuffle_epi32(value, 0b10010011);
			int64_t shiftRightAmount = 64 - (amount % 64);
			__m128i shiftedRight = _mm_srli_epi64(permuted, shiftRightAmount);

			return SimdBase{ std::move(shifted) } | SimdBase{ std::move(shiftedRight) };
		}

		inline operator const __m128i&&() const noexcept {
			return std::forward<const __m128i>(value);
		}

		inline SimdBase operator==(uint8_t other) const noexcept {
			return _mm_cmpeq_epi8(value, _mm_set1_epi8(other));
		}

		inline SimdBase bitAndNot(const SimdBase& other) const noexcept {
			return _mm_andnot_si128(other, value);
		}

		inline int32_t toBitMask() const noexcept {
			return _mm_movemask_epi8(*this);
		}

		inline void store(uint8_t destVector[16]) const noexcept {
			_mm_storeu_epi8(destVector, value);
		}

		inline void printBits(size_t values, const std::string& valuesTitle) const noexcept {
			std::cout << valuesTitle;
			std::cout << std::bitset<64>{ values };
			std::cout << std::endl;
		}

		inline SimdBase printBits(const std::string& valuesTitle) const noexcept {
			std::cout << valuesTitle;
			for (size_t x = 0; x < 2; ++x) {
				std::cout << std::bitset<64>{ *(reinterpret_cast<const uint64_t*>(&value) + x) };
			}
			std::cout << std::endl;
			return *this;
		}

	  protected:
		__m128i value{};
	};

	#define store(x, y) _mm_store_si128(x, y)
	#define loadu(x) _mm_loadu_si128(x)
	#define set(x) _mm_set1_epi8(x)
	#define tzCount(x) _tzcnt_u16(x)

#else

	constexpr int32_t StepSize{ 64 };
	constexpr int32_t BytesPerStep{ StepSize / 8 };
	constexpr int32_t SixtyFourPer{ StepSize / 64 };
	using StringParsingType = uint8_t;
	using SimdBaseReal = SimdBase<64>;
	using AvxType = int64_t;

	inline int64_t _mm64_set1_epi8(char value) {
		int64_t returnValue{};
		std::memset(&returnValue, value, sizeof(int64_t));
		return returnValue;
	}

	inline int64_t _mm64_loadu_si64(const int64_t* value) {
		return *value;
	}

	inline void _mm64_store_si64(int64_t* dest, int64_t src) {
		*dest = src;
	}

	inline uint8_t _tzcnt_u8(uint8_t value) {
		uint8_t count{};
		while ((value & 1) == 0 && count < 8) {
			value >>= 1;
			count++;
		}
		return count;
	}

	template<> struct SimdBase<64> {
	  public:
		inline SimdBase() noexcept = default;

		inline SimdBase& operator=(SimdBase&& other) noexcept = default;
		inline SimdBase(SimdBase&& other) noexcept = default;
		inline SimdBase& operator=(const SimdBase& other) noexcept = default;
		inline SimdBase(const SimdBase& other) noexcept = default;

		inline SimdBase& operator=(int64_t data) noexcept {
			value = data;
			return *this;
		}

		inline SimdBase(int64_t data) noexcept {
			*this = data;
		}

		inline SimdBase& operator=(char other) noexcept {
			value = _mm64_set1_epi8(other);
			return *this;
		}

		inline explicit SimdBase(char other) noexcept {
			*this = other;
		}

		inline SimdBase& operator=(const uint8_t values[8]) noexcept {
			value = _mm64_loadu_epi8(values);
			return *this;
		}

		inline SimdBase(const uint8_t values[16]) noexcept {
			*this = values;
		}

		inline explicit operator const size_t&() const noexcept {
			return value;
		}

		inline operator size_t&() noexcept {
			return value;
		}

		inline operator bool() const noexcept {
			return static_cast<bool>(value);
		}

		inline SimdBase<64> operator|(SimdBase&& other) noexcept {
			return value | other;
		}

		inline SimdBase<64> operator|(const SimdBase& other) const noexcept {
			return value | other.value;
		}

		inline SimdBase<64> operator&(const SimdBase& other) const noexcept {
			return value & other.value;
		}

		inline SimdBase<64> operator^(const SimdBase& other) const noexcept {
			return value ^ other.value;
		}

		inline StringParsingType operator==(const SimdBase& other) const noexcept {
			SimdBase newValue = value == other.value;
			return newValue.toBitMask();
		}

		inline StringParsingType operator==(const char& other) const noexcept {
			SimdBase newValue = value == _mm64_set1_epi8(other);
			return newValue.toBitMask();
		}

		inline SimdBase<64> operator~() noexcept {
			value = ~value;
			return *this;
		}

		inline void reset() {
			value = 0;
		}

		inline int64_t carrylessMultiplication(size_t& prevInString) noexcept {
			int64_t result = 0;
			int64_t allOnes = -1;

			int64_t valueLow = value & 0xFFFFFFFF;
			int64_t valueHigh = (value >> 32) & 0xFFFFFFFF;

			int64_t carry = (valueLow * allOnes) ^ prevInString;
			result = carry & 0x7FFFFFFFFFFFFFFF;

			prevInString = carry >> 63;

			carry = (valueHigh * allOnes) ^ prevInString;
			result ^= (carry & 0x7FFFFFFFFFFFFFFF);

			prevInString = carry >> 63;

			return result;
		}

		inline bool collectCarries(size_t& value2, size_t& result) {
			return _addcarry_u64(0, value, value2, reinterpret_cast<long long unsigned int*>(&result));
		}

		template<size_t amount> inline SimdBase shl() {
			return value << amount;
		}

		inline void addValues(StringParsingType&& values, size_t index) noexcept {
			*(reinterpret_cast<int8_t*>(&value) + index) = values;
		}

		inline SimdBase bitAndNot(const SimdBase& other) const noexcept {
			return other.value & ~value;
		}

		inline SimdBase shuffle(const SimdBase& other) const noexcept {
			return _mm64_shuffle_epi8(other.value, value);
		}

		inline void convertWhitespaceToSimdBase(const SimdBase* valuesNew) noexcept {
			uint8_t arrayNew[32]{ ' ', 100, 100, 100, 17, 100, 113, 2, 100, '\t', '\n', 112, 100, '\r', 100, 100, ' ', 100, 100, 100, 17, 100, 113, 2,
				100, '\t', '\n', 112, 100, '\r', 100, 100 };
			SimdBase whitespaceTable{ arrayNew };
			for (size_t x = 0; x < 8; ++x) {
				addValues(valuesNew[x].shuffle(whitespaceTable) == valuesNew[x], x);
			}
		};

		inline void convertBackslashesToSimdBase(const SimdBase* valuesNew) noexcept {
			SimdBase backslashes{ _mm64_set1_epi8('\\') };
			for (size_t x = 0; x < 8; ++x) {
				addValues(valuesNew[x] == backslashes, x);
			}
		};

		inline void convertStructuralsToSimdBase(const SimdBase* valuesNew) noexcept {
			uint8_t arrayNew[8]{ 0, 0, 0, 0, ':', '{', ',', '}' };
			SimdBase opTable{ arrayNew };
			SimdBase chars{ char{ 0x20 } };
			for (size_t x = 0; x < 8; ++x) {
				addValues({ valuesNew[x].shuffle(opTable) == (valuesNew[x] | chars) }, x);
			}
		};

		inline void convertQuotesToSimdBase(const SimdBase* valuesNew) noexcept {
			SimdBase quotes = _mm64_set1_epi8('"');
			for (size_t x = 0; x < 8; ++x) {
				addValues(valuesNew[x] == quotes, x);
			}
		}

		inline int8_t toBitMask() noexcept {
			return _mm64_movemask_epi8(value);
		}

		inline void store(uint8_t destVector[8]) const noexcept {
			std::memcpy(destVector, &value, sizeof(int64_t));
		}

		inline SimdBase& setLSB(bool valueNew) noexcept {
			if (valueNew) {
				value |= 0x01;
			} else {
				value &= ~0x01;
			}
			return *this;
		}

		inline bool checkLSB() const noexcept {
			return 0x01 & value;
		}

		inline bool checkMSB() const noexcept {
			return 0x8000000000000000 & value;
		}

		inline void printBits(size_t values, const std::string& valuesTitle) const noexcept {
			std::cout << valuesTitle;
			std::cout << std::bitset<64>{ values };
			std::cout << std::endl;
		}

		inline SimdBase printBits(const std::string& valuesTitle) const noexcept {
			std::cout << valuesTitle;
			for (size_t x = 0; x < 1; ++x) {
				std::cout << std::bitset<64>{ value };
			}
			std::cout << std::endl;
			return *this;
		}

		inline SimdBase follows(bool& overflow) noexcept {
			SimdBase result = shl<1>();
			result.setLSB(overflow);
			overflow = checkMSB();
			return result;
		}

	  protected:
		size_t value{};

		inline int64_t _mm64_loadu_epi8(const uint8_t* value) const {
			int64_t returnValue{};
			std::memcpy(&returnValue, value, sizeof(int64_t));
			return returnValue;
		}

		inline int8_t _mm64_movemask_epi8(size_t value) const {
			constexpr size_t mask = 0x8080808080808080ULL;
			size_t maskedValue = ((value & mask) >> 7) * 0x8040201008040201ULL;
			return static_cast<int8_t>(maskedValue >> 56);
		}

		inline int64_t _mm64_shuffle_epi8(int64_t value, int64_t mask) const {
			constexpr int64_t shuffleMask = 0x0101010101010101LL;
			int64_t result = 0;

			for (int i = 0; i < 8; ++i) {
				int shiftAmount = (mask >> (i * 8)) & 0xFF;
				int64_t byteMask = shuffleMask << (shiftAmount * 8);
				int64_t shuffledByte = (value & byteMask) >> (shiftAmount * 8);
				result |= shuffledByte << (i * 8);
			}

			return result;
		}
	};

	#define store(x, y) _mm64_store_si64(x, y)
	#define loadu(x) _mm64_loadu_si64(x)
	#define set(x) _mm64_set1_epi8(x)
	#define tzCount(x) _tzcnt_u8(x)

#endif

}
