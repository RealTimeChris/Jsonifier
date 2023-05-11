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

namespace JsonifierInternal {

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

	struct SimdBase64 {
	  public:
		inline SimdBase64() noexcept = default;

		inline SimdBase64& operator=(SimdBase64&& other) noexcept = default;
		inline SimdBase64(SimdBase64&& other) noexcept = default;
		inline SimdBase64& operator=(const SimdBase64& other) noexcept = default;
		inline SimdBase64(const SimdBase64& other) noexcept = default;

		inline SimdBase64& operator=(int64_t data) noexcept {
			this->value = data;
			return *this;
		}

		inline SimdBase64(int64_t data) noexcept {
			*this = data;
		}

		inline SimdBase64& operator=(char other) noexcept {
			this->value = _mm64_set1_epi8(other);
			return *this;
		}

		inline explicit SimdBase64(char other) noexcept {
			*this = other;
		}

		inline SimdBase64& operator=(const uint8_t values[8]) noexcept {
			value = _mm64_loadu_epi8(values);
			return *this;
		}

		inline SimdBase64(const uint8_t values[16]) noexcept {
			*this = values;
		}

		inline operator const uint64_t&() const noexcept {
			return value;
		}

		inline operator uint64_t&() noexcept {
			return value;
		}

		inline operator bool() const noexcept {
			return static_cast<bool>(value);
		}

		inline SimdBase64 operator|(SimdBase64&& other) noexcept {
			return value | other;
		}

		inline SimdBase64 operator|(const SimdBase64& other) const noexcept {
			return value | other.value;
		}

		inline SimdBase64 operator&(const SimdBase64& other) const noexcept {
			return value & other.value;
		}

		inline SimdBase64 operator^(const SimdBase64& other) const noexcept {
			return value ^ other.value;
		}

		inline SimdBase64 operator==(const SimdBase64& other) const noexcept {
			return value & other.value;
		}

		inline SimdBase64 operator==(const uint8_t& other) const noexcept {
			return value & _mm64_set1_epi8(other);
		}

		inline SimdBase64 operator~() noexcept {
			this->value = ~value;
			return *this;
		}

		inline SimdBase64 operator==(uint8_t other) const noexcept {
			int64_t newValue{};
			std::memset(&newValue, other, sizeof(int64_t));
			return value & newValue;
		}

		inline void reset() {
			value = 0;
		}

		inline int64_t carrylessMultiplication(uint64_t& prevInString) noexcept {
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

		inline bool collectCarries(uint64_t& value2, uint64_t& result) {
			return _addcarry_u64(0, value, value2, reinterpret_cast<long long unsigned int*>(&result));
		}

		template<size_t amount> inline SimdBase64 shl() {
			return this->value << amount;
		}

		inline void addValues(SimdBase64&& values, size_t index) noexcept {
			*(reinterpret_cast<int8_t*>(&value) + index) = _mm64_movemask_epi8(values);
		}

		inline SimdBase64 bitAndNot(const SimdBase64& other) const noexcept {
			return other.value & ~value;
		}

		inline SimdBase64 shuffle(const SimdBase64& other) const noexcept {
			return _mm64_shuffle_epi8(other.value, value);
		}

		inline void convertWhitespaceToSimdBase(const SimdBase64* valuesNew) noexcept {
			uint8_t arrayNew[32]{ ' ', 100, 100, 100, 17, 100, 113, 2, 100, '\t', '\n', 112, 100, '\r', 100, 100, ' ', 100, 100, 100, 17, 100, 113, 2,
				100, '\t', '\n', 112, 100, '\r', 100, 100 };
			SimdBase64 whitespaceTable{ arrayNew };
			for (size_t x = 0; x < 8; ++x) {
				addValues(valuesNew[x].shuffle(whitespaceTable) == valuesNew[x], x);
			}
		};

		inline void convertBackslashesToSimdBase(const SimdBase64* valuesNew) noexcept {
			SimdBase64 backslashes{ _mm64_set1_epi8('\\') };
			for (size_t x = 0; x < 8; ++x) {
				addValues(valuesNew[x] == backslashes, x);
			}
		};

		inline void convertStructuralsToSimdBase(const SimdBase64* valuesNew) noexcept {
			uint8_t arrayNew[8]{ 0, 0, 0, 0, ':', '{', ',', '}' };
			SimdBase64 opTable{ arrayNew };
			SimdBase64 chars{ char{ 0x20 } };
			for (size_t x = 0; x < 8; ++x) {
				addValues({ valuesNew[x].shuffle(opTable) == (valuesNew[x] | chars) }, x);
			}
		};

		inline void convertQuotesToSimdBase(const SimdBase64* valuesNew) noexcept {
			SimdBase64 quotes = _mm64_set1_epi8('"');
			for (size_t x = 0; x < 8; ++x) {
				addValues(valuesNew[x] == quotes, x);
			}
		}

		inline int8_t toBitMask() noexcept {
			return _mm64_movemask_epi8(this->value);
		}

		inline void store(uint8_t destVector[8]) const noexcept {
			std::memcpy(destVector, &value, sizeof(int64_t));
		}

		inline SimdBase64& setLSB(bool value) noexcept {
			if (value) {
				this->value |= 0x01;
			} else {
				this->value &= ~0x01;
			}
			return *this;
		}

		inline bool checkLSB() const noexcept {
			return 0x01 & value;
		}

		inline bool checkMSB() const noexcept {
			return 0x8000000000000000 & value;
		}

		inline void printBits(uint64_t values, const Jsonifier::String& valuesTitle) const noexcept {
			std::cout << valuesTitle;
			std::cout << std::bitset<64>{ values };
			std::cout << std::endl;
		}

		inline SimdBase64 printBits(const Jsonifier::String& valuesTitle) const noexcept {
			std::cout << valuesTitle;
			for (size_t x = 0; x < 32; ++x) {
				for (size_t y = 0; y < 8; ++y) {
					std::cout << std::bitset<1>{ static_cast<size_t>(*(reinterpret_cast<const int8_t*>(&value) + x)) >> y };
				}
			}
			std::cout << std::endl;
			return *this;
		}

		inline SimdBase64 follows(bool& overflow) noexcept {
			SimdBase64 result = shl<1>();
			result.setLSB(overflow);
			overflow = checkMSB();
			return result;
		}

	  protected:
		uint64_t value{};

		inline int64_t _mm64_loadu_epi8(const uint8_t* value) const {
			int64_t returnValue{};
			std::memcpy(&returnValue, value, sizeof(int64_t));
			return returnValue;
		}

		inline int8_t _mm64_movemask_epi8(uint64_t value) const {
			constexpr uint64_t mask = 0x8080808080808080ULL;
			uint64_t maskedValue = ((value & mask) >> 7) * 0x8040201008040201ULL;
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
#if defined(INSTRUCTION_SET_TYPE_AVX) || defined(INSTRUCTION_SET_TYPE_AVX2) || defined(INSTRUCTION_SET_TYPE_AVX512)
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
			value = _mm_loadu_si128(reinterpret_cast<const __m128i*>(values));
			return *this;
		}

		inline SimdBase128(const uint8_t values[16]) noexcept {
			*this = values;
		}

		inline void reset() noexcept {
			value = _mm_setzero_si128();
		}

		inline operator bool() const noexcept {
			return !_mm_testz_si128(value, value);
		}

		inline SimdBase128 shuffle(const SimdBase128& other) const noexcept {
			return _mm_shuffle_epi8(other, value);
		}

		inline SimdBase128 operator|(SimdBase128&& other) noexcept {
			return _mm_or_si128(value, std::forward<__m128i>(other.value));
		}

		inline SimdBase128 operator|(const SimdBase128& other) const noexcept {
			return _mm_or_si128(value, other);
		}

		inline SimdBase128 operator&(const SimdBase128& other) const noexcept {
			return _mm_and_si128(value, other);
		}

		inline SimdBase128 operator^(const SimdBase128& other) const noexcept {
			return _mm_xor_si128(value, other);
		}

		inline SimdBase128 operator==(const SimdBase128& other) const noexcept {
			return _mm_cmpeq_epi8(value, other);
		}

		inline SimdBase128 operator==(const uint8_t& other) const noexcept {
			return _mm_cmpeq_epi8(value, _mm_set1_epi8(other));
		}

		inline SimdBase128 operator==(const char& other) const noexcept {
			return _mm_cmpeq_epi8(value, _mm_set1_epi8(other));
		}

		inline SimdBase128 operator~() const noexcept {
			return _mm_xor_si128(*this, _mm_set1_epi64x(-1ll));
		}

		inline SimdBase128& setLSB(bool value) noexcept {
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
			__m128i result = _mm_and_si128(*this, _mm_set_epi64x(0x8000000000000000, 0));
			return !_mm_testz_si128(result, result);
		}

		inline void convertWhitespaceToSimdBase(const SimdBase128* valuesNew) noexcept {
			uint8_t arrayNew[16]{ ' ', 100, 100, 100, 17, 100, 113, 2, 100, '\t', '\n', 112, 100, '\r', 100, 100 };
			SimdBase128 whitespaceTable{ arrayNew };
			for (size_t x = 0; x < 8; ++x) {
				addValues(valuesNew[x].shuffle(whitespaceTable) == valuesNew[x], x);
			}
		};

		inline void addValues(SimdBase128&& values, size_t index) noexcept {
			*(reinterpret_cast<int32_t*>(&value) + index) = _mm_movemask_epi8(values);
		}

		inline void convertBackslashesToSimdBase(const SimdBase128* valuesNew) noexcept {
			SimdBase128 backslashes{ _mm_set1_epi8('\\') };
			for (size_t x = 0; x < 8; ++x) {
				addValues(valuesNew[x] == backslashes, x);
			}
		};

		inline void convertStructuralsToSimdBase(const SimdBase128* valuesNew) noexcept {
			uint8_t arrayNew[16]{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, ':', '{', ',', '}', 0, 0 };
			SimdBase128 opTable{ arrayNew };
			SimdBase128 chars{ char{ 0x20 } };
			for (size_t x = 0; x < 8; ++x) {
				addValues({ valuesNew[x].shuffle(opTable) == (valuesNew[x] | chars) }, x);
			}
		};

		inline void convertQuotesToSimdBase(const SimdBase128* valuesNew) noexcept {
			SimdBase128 quotes = _mm_set1_epi8('"');
			for (size_t x = 0; x < 8; ++x) {
				addValues(valuesNew[x] == quotes, x);
			}
		}

		inline SimdBase128 carrylessMultiplication(uint64_t& prevInString) const noexcept {
			SimdBase128 allOnes{ '\xFF' };
			SimdBase128 valuesNew{};
			__m128i valueLow{ value };
			uint64_t* values = reinterpret_cast<uint64_t*>(&valuesNew);
			values[0] = _mm_cvtsi128_si64(_mm_clmulepi64_si128(valueLow, allOnes, 0)) ^ prevInString;
			prevInString = uint64_t(static_cast<int64_t>(values[0]) >> 63);
			values[1] = _mm_cvtsi128_si64(_mm_clmulepi64_si128(valueLow, allOnes, 1)) ^ prevInString;
			prevInString = uint64_t(static_cast<int64_t>(values[1]) >> 63);
			return valuesNew;
		}

		inline bool collectCarries(const SimdBase128& other1, SimdBase128& result) const noexcept {
			__m128i carry = _mm_set1_epi64x(0 ? 1 : 0);
			__m128i sum = _mm_add_epi64(*this, other1);
			sum = _mm_add_epi64(sum, carry);
			result = std::move(sum);
			return _mm_testz_si128(carry, sum) == 0;
		}

		inline SimdBase128 follows(bool& overflow) const noexcept {
			SimdBase128 result = shl<1>();
			result.setLSB(overflow);
			overflow = checkMSB();
			return result;
		}

		template<size_t amount> inline SimdBase128 shl() const noexcept {
			uint64_t newArray[2]{};
			newArray[0] = _mm_extract_epi64(value, 0);
			newArray[1] = _mm_extract_epi64(value, 1);
			uint64_t newerArray[2]{};
			newerArray[0] = newArray[0] << amount;
			newerArray[1] = newArray[1] << amount;
			SimdBase128 newValue{};
			newValue = _mm_insert_epi64(newValue, newerArray[0], 0);
			newValue = _mm_insert_epi64(newValue, newerArray[1], 1);
			return newValue;
		}

		inline operator const __m128i&&() const noexcept {
			return std::forward<const __m128i>(value);
		}

		inline SimdBase128 operator==(uint8_t other) const noexcept {
			return _mm_cmpeq_epi8(value, _mm_set1_epi8(other));
		}

		inline SimdBase128 bitAndNot(const SimdBase128& other) const noexcept {
			return _mm_andnot_si128(other, value);
		}

		inline int32_t toBitMask() const noexcept {
			return _mm_movemask_epi8(*this);
		}

		inline void store(uint8_t destVector[16]) const noexcept {
			_mm_storeu_epi8(destVector, value);
		}

		inline void printBits(uint64_t values, const Jsonifier::String& valuesTitle) const noexcept {
			std::cout << valuesTitle;
			std::cout << std::bitset<64>{ values };
			std::cout << std::endl;
		}

		inline SimdBase128 printBits(const Jsonifier::String& valuesTitle) const noexcept {
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

#if defined(INSTRUCTION_SET_TYPE_AVX2) || defined(INSTRUCTION_SET_TYPE_AVX512)
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

		inline void convertWhitespaceToSimdBase(const SimdBase256* valuesNew) noexcept {
			uint8_t arrayNew[32]{ ' ', 100, 100, 100, 17, 100, 113, 2, 100, '\t', '\n', 112, 100, '\r', 100, 100, ' ', 100, 100, 100, 17, 100, 113, 2,
				100, '\t', '\n', 112, 100, '\r', 100, 100 };
			SimdBase256 whitespaceTable{ arrayNew };
			for (size_t x = 0; x < 8; ++x) {
				addValues(valuesNew[x].shuffle(whitespaceTable) == valuesNew[x], x);
			}
		};

		inline void convertBackslashesToSimdBase(const SimdBase256* valuesNew) noexcept {
			SimdBase256 backslashes{ _mm256_set1_epi8('\\') };
			for (size_t x = 0; x < 8; ++x) {
				addValues(valuesNew[x] == backslashes, x);
			}
		};

		inline void convertStructuralsToSimdBase(const SimdBase256* valuesNew) noexcept {
			uint8_t arrayNew[32]{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, ':', '{', ',', '}', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, ':', '{', ',', '}', 0, 0 };
			SimdBase256 opTable{ arrayNew };
			SimdBase256 chars{ char{ 0x20 } };
			for (size_t x = 0; x < 8; ++x) {
				addValues({ valuesNew[x].shuffle(opTable) == (valuesNew[x] | chars) }, x);
			}
		};

		inline void convertQuotesToSimdBase(const SimdBase256* valuesNew) noexcept {
			SimdBase256 quotes = _mm256_set1_epi8('"');
			for (size_t x = 0; x < 8; ++x) {
				addValues(valuesNew[x] == quotes, x);
			}
		}

		inline SimdBase256 bitAndNot(const SimdBase256& other) const noexcept {
			return _mm256_andnot_si256(other, value);
		}

		inline SimdBase256 shuffle(const SimdBase256& other) const noexcept {
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

		inline SimdBase256& setLSB(bool value) noexcept {
			if (value) {
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

		inline SimdBase256 carrylessMultiplication(uint64_t& prevInString) const noexcept {
			SimdBase128 allOnes{ '\xFF' };
			SimdBase256 valuesNew{};
			__m128i valueLow{ _mm256_extracti128_si256(value, 0) };
			__m128i valueHigh{ _mm256_extracti128_si256(value, 1) };
			uint64_t* values = reinterpret_cast<uint64_t*>(&valuesNew);
			values[0] = _mm_cvtsi128_si64(_mm_clmulepi64_si128(valueLow, allOnes, 0)) ^ prevInString;
			prevInString = uint64_t(static_cast<int64_t>(values[0]) >> 63);
			values[1] = _mm_cvtsi128_si64(_mm_clmulepi64_si128(valueLow, allOnes, 1)) ^ prevInString;
			prevInString = uint64_t(static_cast<int64_t>(values[1]) >> 63);
			values[2] = _mm_cvtsi128_si64(_mm_clmulepi64_si128(valueHigh, allOnes, 0)) ^ prevInString;
			prevInString = uint64_t(static_cast<int64_t>(values[2]) >> 63);
			values[3] = _mm_cvtsi128_si64(_mm_clmulepi64_si128(valueHigh, allOnes, 1)) ^ prevInString;
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

		inline void printBits(uint64_t values, const Jsonifier::String& valuesTitle) const noexcept {
			std::cout << valuesTitle;
			std::cout << std::bitset<64>{ values };
			std::cout << std::endl;
		}

		inline SimdBase256 printBits(const Jsonifier::String& valuesTitle) const noexcept {
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
#if defined(INSTRUCTION_SET_TYPE_AVX512)
	struct SimdBase512 {
	  public:
		inline SimdBase512() noexcept = default;

		inline SimdBase512& operator=(SimdBase512&& other) noexcept = default;
		inline SimdBase512(SimdBase512&& other) noexcept = default;
		inline SimdBase512& operator=(const SimdBase512& other) noexcept = default;
		inline SimdBase512(const SimdBase512& other) noexcept = default;

		inline SimdBase512& operator=(__m512i&& other) noexcept {
			value = std::forward<__m512i>(other);
			return *this;
		}

		inline SimdBase512(__m512i&& other) noexcept {
			*this = std::move(other);
		}

		inline SimdBase512& operator=(char other) noexcept {
			value = _mm512_set1_epi8(other);
			return *this;
		}

		inline explicit SimdBase512(char other) noexcept {
			*this = other;
		}

		inline SimdBase512& operator=(const uint8_t values[64]) noexcept {
			value = _mm512_loadu_si512(reinterpret_cast<const __m512i*>(values));
			return *this;
		}

		inline SimdBase512(const uint8_t values[64]) noexcept {
			*this = values;
		}

		inline operator __m512i&() {
			return value;
		}

		inline operator const __m512i&&() const noexcept {
			return std::forward<const __m512i>(value);
		}

		inline operator bool() const noexcept {
			__mmask64 mask = _mm512_test_epi32_mask(value, value);
			return mask != 0;
		}

		inline SimdBase512 operator|(SimdBase512&& other) noexcept {
			return _mm512_or_si512(value, std::forward<__m512i>(other));
		}

		inline SimdBase512 operator|(const SimdBase512& other) const noexcept {
			return _mm512_or_si512(value, other);
		}

		inline SimdBase512 operator&(const SimdBase512& other) const noexcept {
			return _mm512_and_si512(value, other);
		}

		inline SimdBase512 operator^(const SimdBase512& other) const noexcept {
			return _mm512_xor_si512(value, other);
		}

		inline uint64_t operator==(const SimdBase512& other) const noexcept {
			return _mm512_cmpeq_epi8_mask(value, other);
		}

		inline uint64_t operator==(const uint8_t& other) const noexcept {
			return _mm512_cmpeq_epi8_mask(value, _mm512_set1_epi8(other));
		}

		inline uint64_t operator==(const char& other) const noexcept {
			return _mm512_cmpeq_epi8_mask(value, _mm512_set1_epi8(other));
		}

		inline SimdBase512 operator~() const noexcept {
			return _mm512_xor_si512(*this, _mm512_set1_epi64(-1ll));
		}

		inline void convertWhitespaceToSimdBase(const SimdBase512* valuesNew) noexcept {
			uint8_t arrayNew[64]{ ' ', 100, 100, 100, 17, 100, 113, 2, 100, '\t', '\n', 112, 100, '\r', 100, 100, ' ', 100, 100, 100, 17, 100, 113, 2,
				100, '\t', '\n', 112, 100, '\r', 100, 100, ' ', 100, 100, 100, 17, 100, 113, 2, 100, '\t', '\n', 112, 100, '\r', 100, 100, ' ', 100,
				100, 100, 17, 100, 113, 2, 100, '\t', '\n', 112, 100, '\r', 100, 100 };
			SimdBase512 whitespaceTable{ arrayNew };

			for (size_t x = 0; x < 8; ++x) {
				addValues(valuesNew[x].shuffle(whitespaceTable) == valuesNew[x], x);
			}
		}

		inline void convertBackslashesToSimdBase(const SimdBase512* valuesNew) noexcept {
			for (size_t x = 0; x < 8; ++x) {
				addValues(valuesNew[x] == '\\', x);
			}
		}

		inline void convertStructuralsToSimdBase(const SimdBase512* valuesNew) noexcept {
			uint8_t arrayNew[64]{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, ':', '{', ',', '}', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, ':', '{', ',', '}', 0, 0, 0,
				0, 0, 0, 0, 0, 0, 0, 0, 0, ':', '{', ',', '}', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, ':', '{', ',', '}', 0, 0 };
			SimdBase512 opTable{ arrayNew };
			SimdBase512 chars{ char{ 0x20 } };

			for (size_t x = 0; x < 8; ++x) {
				addValues(valuesNew[x].shuffle(opTable) == (valuesNew[x] | chars), x);
			}
		}

		inline void convertQuotesToSimdBase(const SimdBase512* valuesNew) noexcept {
			for (size_t x = 0; x < 8; ++x) {
				addValues(valuesNew[x] == '"', x);
			}
		}

		inline SimdBase512 bitAndNot(SimdBase512&& other) const noexcept {
			return _mm512_andnot_si512(std::forward<__m512i>(other), value);
		}

		inline SimdBase512 bitAndNot(SimdBase512& other) const noexcept {
			return _mm512_andnot_si512(other, value);
		}

		inline SimdBase512 shuffle(SimdBase512& other) const noexcept {
			return _mm512_shuffle_epi8(other, value);
		}

		inline void addValues(uint64_t&& values, size_t index) noexcept {
			*(reinterpret_cast<uint64_t*>(&value) + index) = values;
		}

		template<size_t amount> inline SimdBase512 shl() noexcept {
			//this->printBits("PRE LEFT SHIFT: ");
			SimdBase256 lowValues = _mm512_extracti64x4_epi64(value, 0);
			lowValues = lowValues.shl<amount>();
			SimdBase256 highValues = _mm512_extracti64x4_epi64(value, 1);
			highValues = highValues.shl<amount>();
			SimdBase512 newValues{};
			newValues = _mm512_inserti64x4(newValues, lowValues, 0);
			newValues = _mm512_inserti64x4(newValues, highValues, 1);
			*this = newValues;
			//this->printBits("POST LEFT SHIFT: ");
			return std::move(value);
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

		inline SimdBase512& setLSB(bool value) noexcept {
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

		inline SimdBase512 carrylessMultiplication(uint64_t& prevInString) const noexcept {
			SimdBase256 lowValues{ _mm512_extracti64x4_epi64(value, 0) };
			lowValues = lowValues.carrylessMultiplication(prevInString);
			SimdBase256 highValues{ _mm512_extracti64x4_epi64(value, 1) };
			highValues = highValues.carrylessMultiplication(prevInString);
			SimdBase512 returnValue{};
			returnValue = _mm512_inserti64x4(returnValue, lowValues, 0);
			returnValue = _mm512_inserti64x4(returnValue, highValues, 1);
			return returnValue;
		}

		inline bool collectCarries(const SimdBase512& other1, SimdBase512& result) const noexcept {
			__m512i carry = _mm512_set1_epi64(0 ? 1 : 0);
			__m512i sum = _mm512_add_epi64(*this, other1);
			sum = _mm512_add_epi64(sum, carry);
			result = std::move(sum);
			return _mm512_test_epi64_mask(carry, sum) == 0;
		}

		inline SimdBase512 follows(bool& overflow) noexcept {
			SimdBase512 result = shl<1>();
			result.setLSB(overflow);
			overflow = checkMSB();
			return result;
		}

		inline void printBits(uint64_t values, const Jsonifier::String& valuesTitle) const noexcept {
			std::cout << valuesTitle;
			std::cout << std::bitset<64>{ values };
			std::cout << std::endl;
		}

		inline SimdBase512 printBits(const Jsonifier::String& valuesTitle) const noexcept {
			std::cout << valuesTitle;
			for (size_t x = 0; x < 64; ++x) {
				for (size_t y = 0; y < 8; ++y) {
					std::cout << std::bitset<1>{ static_cast<size_t>(*(reinterpret_cast<const int8_t*>(&value) + x)) >> y };
				}
			}
			std::cout << std::endl;
			return *this;
		}

	  protected:
		__m512i value{};
	};
#endif 
#endif 
#endif 

#if defined(INSTRUCTION_SET_TYPE_AVX512)
	#define store(x, y) _mm512_store_si512(x, y)
	#define loadu(x) _mm512_loadu_si512(x)
	#define set(x) _mm512_set1_epi8(x)
	constexpr int32_t stepSize{ 512 };
	constexpr int32_t bytesPerStep{ stepSize / 8 };
	constexpr int32_t sixtyFourPer{ stepSize / 64 };
	using SimdBase = SimdBase512;
	using avxType = __m512i;
#elif defined(INSTRUCTION_SET_TYPE_AVX2)
	#define store(x, y) _mm256_store_si256(x, y)
	#define loadu(x) _mm256_loadu_si256(x)
	#define set(x) _mm256_set1_epi8(x)
	constexpr int32_t stepSize{ 256 };
	constexpr int32_t bytesPerStep{ stepSize / 8 };
	constexpr int32_t sixtyFourPer{ stepSize / 64 };
	using SimdBase = SimdBase256;
	using avxType = __m256i;
#elif defined(INSTRUCTION_SET_TYPE_AVX)
	#define store(x, y) _mm_store_si128(x, y)
	#define loadu(x) _mm_loadu_si128(x)
	#define set(x) _mm_set1_epi8(x)
	constexpr int32_t stepSize{ 128 };
	constexpr int32_t bytesPerStep{ stepSize / 8 };
	constexpr int32_t sixtyFourPer{ stepSize / 64 };
	using SimdBase = SimdBase128;
	using avxType = __m128i;
#else
	#define store(x, y) _mm64_store_si64(x, y)
	#define loadu(x) _mm64_loadu_si64(x)
	#define set(x) _mm64_set1_epi8(x)
	constexpr int32_t stepSize{ 64 };
	constexpr int32_t bytesPerStep{ stepSize / 8 };
	constexpr int32_t sixtyFourPer{ stepSize / 64 };
	using SimdBase = SimdBase64;
	using avxType = int64_t;
#endif

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
		using allocator = std::pmr::polymorphic_allocator<uint32_t>;

		inline SimdStringReader() noexcept = default;
		inline SimdStringReader& operator=(SimdStringReader&& other) noexcept = default;
		inline SimdStringReader(SimdStringReader&& other) noexcept = default;
		inline SimdStringReader& operator=(const SimdStringReader& other) noexcept = delete;
		inline SimdStringReader(const SimdStringReader& other) noexcept = delete;

		inline void reset(size_t stringLengthRawNew, StringViewPtr stringViewNew) noexcept {
			if (stringLengthRawNew > stringLengthRaw) [[likely]] {
				allocator{}.deallocate(structuralIndices, stringLengthRawAlloc);
				stringLengthRawAlloc = round(stringLengthRawNew + 3ull, stepSize);
				structuralIndices = allocator{}.allocate(stringLengthRawAlloc);
			} else [[unlikely]] {
				std::fill(structuralIndices, structuralIndices + stringLengthRawAlloc, uint32_t{});
			}
			stringLengthRaw = stringLengthRawNew;
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
		}

		inline void generateJsonIndices() noexcept {
			if (stringView) [[likely]] {
				StringBlockReader<stepSize> stringReader{ stringView, stringLengthRaw };
				while (stringReader.hasFullBlock()) {
					generateStructurals(stringReader.fullBlock());
				}
				uint8_t block[stepSize];
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
			return structuralIndices;
		}

		inline ~SimdStringReader() noexcept {
			if (structuralIndices && stringLengthRawAlloc) {
				allocator{}.deallocate(structuralIndices, stringLengthRawAlloc);
			}
		}

	  protected:
		StructuralIndex structuralIndices{};
		size_t stringLengthRawAlloc{};
		StringViewPtr stringView{};
		size_t stringLengthRaw{};
		uint64_t prevInString{};
		SimdBase prevEscaped{};
		SimdBase structurals{};
		SimdBase whitespace{};
		SimdBase backslash{};
		size_t stringIndex{};
		bool storedLSB01{};
		bool storedLSB02{};
		size_t tapeIndex{};
		SimdBase quotes{};
		SimdBase op{};

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
			SimdBase newPtr[8];
			for (size_t x = 0; x < 8; ++x) {
				newPtr[x] = loadu(reinterpret_cast<const avxType*>(valueNew + (bytesPerStep * x)));
			}
			whitespace.convertWhitespaceToSimdBase(newPtr);
			backslash.convertBackslashesToSimdBase(newPtr);
			op.convertStructuralsToSimdBase(newPtr);
			quotes.convertQuotesToSimdBase(newPtr);
			collectStructurals();
			addTapeValues();
			stringIndex += stepSize;
		}

		inline void addTapeValues() noexcept {
			alignas(64) int64_t newBits[sixtyFourPer];
			store(reinterpret_cast<avxType*>(newBits), structurals);
			for (size_t x = 0; x < sixtyFourPer; ++x) {
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

		inline SimdBase collectEscapedCharacters() {
			if (backslash) {
				SimdBase followsEscape = backslash.shl<1>() | prevEscaped;
				bool newStoredLSB = storedLSB02;
				storedLSB02 = followsEscape.checkLSB();
				SimdBase evenBits{ set(0xAA) };
				SimdBase oddSequenceStarts = backslash.bitAndNot(evenBits.bitAndNot(followsEscape));
				SimdBase sequencesStartingOnEvenBits{};
				prevEscaped.setLSB(oddSequenceStarts.collectCarries(backslash, sequencesStartingOnEvenBits));
				SimdBase invertMask = sequencesStartingOnEvenBits.shl<1>();
				return ((evenBits ^ invertMask) & followsEscape).setLSB(newStoredLSB);
			} else {
				SimdBase escaped = prevEscaped;
				prevEscaped.reset();
				return escaped;
			}
		}

		inline void collectStructurals() noexcept {
			SimdBase escaped = collectEscapedCharacters();
			quotes = quotes.bitAndNot(escaped);
			SimdBase inString = quotes.carrylessMultiplication(prevInString);
			SimdBase scalar = ~(op | whitespace);
			SimdBase nonQuoteScalar = scalar.bitAndNot(quotes);
			SimdBase stringTail = inString ^ quotes;
			SimdBase followsNonQuoteScalar = nonQuoteScalar.follows(storedLSB01);
			SimdBase potentialScalarStart = scalar.bitAndNot(followsNonQuoteScalar);
			SimdBase potentialStructuralStart = op | potentialScalarStart;
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