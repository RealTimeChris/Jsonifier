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
/// https://github.com/RealTimeChris/jsonifier
/// Feb 3, 2023
#pragma once

#include <jsonifier/ISADetection/ISADetectionBase.hpp>

namespace jsonifier_internal {

#if JSONIFIER_CHECK_FOR_INSTRUCTION(JSONIFIER_AVX512) && !JSONIFIER_CHECK_FOR_INSTRUCTION(JSONIFIER_AVX) && !JSONIFIER_CHECK_FOR_INSTRUCTION(JSONIFIER_AVX2)

	constexpr uint64_t StepSize{ 512 };
	constexpr uint64_t BytesPerStep{ StepSize / 8 };
	constexpr uint64_t StridesPerStep{ StepSize / 64 };
	using simd_base_real	   = simd_base<512>;
	using string_parsing_type  = uint64_t;
	using avx_type			   = __m512i;
	using avx_type_small	   = __m256i;
	using avx_float_type	   = __m512;
	using avx_float_type_small = __m256;

	template<typename value_type> inline __m128i gatherValues128(const value_type* str) {
		if constexpr (alignof(value_type) == 1) {
			return _mm_loadu_epi8(str);
		} else {
			alignas(ALIGNMENT) float newArray[sizeof(avx_type_small) / sizeof(float)]{};
			std::memcpy(newArray, str, sizeof(avx_type_small));
			return _mm_castps_si128(_mm_load_ps(newArray));
		}
	}

	template<typename value_type> inline avx_type_small gatherValues256(const value_type* str) {
		if constexpr (alignof(value_type) == 1) {
			return _mm256_loadu_epi8(str);
		} else {
			alignas(ALIGNMENT) float newArray[sizeof(avx_type) / sizeof(float)]{};
			std::memcpy(newArray, str, sizeof(avx_type));
			return _mm256_castps_si256(_mm256_load_ps(newArray));
		}
	}

	template<typename value_type> inline avx_type gatherValues512(const value_type* str) {
		if constexpr (alignof(value_type) == 1) {
			return _mm512_loadu_si512(str);
		} else {
			return _mm512_load_si512(str);
		}
	}

	template<float_t value_type> inline __m128 gatherValues128(const value_type* str) {
		return _mm_load_ps(str);
	}

	template<float_t value_type> inline avx_float_type_small gatherValues256(const value_type* str) {
		return _mm256_load_ps(str);
	}

	template<float_t value_type> inline avx_float_type gatherValues512(const value_type* str) {
		return _mm512_load_ps(str);
	}

	template<> struct simd_base<256> {
	  public:
		inline simd_base() = default;

		inline simd_base& operator=(avx_type_small&& data) {
			value = std::forward<avx_type_small>(data);
			return *this;
		}

		inline simd_base(avx_type_small&& data) {
			value = std::forward<avx_type_small>(data);
		}

		inline operator const avx_type_small&&() const {
			return std::forward<const avx_type_small>(value);
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

		inline simd_base carrylessMultiplication(string_parsing_type& prevInstring) const {
			__m128i allOnes{ _mm_set1_epi8('\xFF') };
			simd_base valuesNew{};
			__m128i valueLow{ _mm256_extracti128_si256(value, 0) };
			__m128i valueHigh{ _mm256_extracti128_si256(value, 1) };
			valuesNew.insertInt64(_mm_cvtsi128_si64(_mm_clmulepi64_si128(valueLow, allOnes, 0)) ^ prevInstring, 0);
			prevInstring = string_parsing_type(valuesNew.getInt64(0) >> 63);
			valuesNew.insertInt64(_mm_cvtsi128_si64(_mm_clmulepi64_si128(valueLow, allOnes, 1)) ^ prevInstring, 1);
			prevInstring = string_parsing_type(valuesNew.getInt64(1) >> 63);
			valuesNew.insertInt64(_mm_cvtsi128_si64(_mm_clmulepi64_si128(valueHigh, allOnes, 0)) ^ prevInstring, 2);
			prevInstring = string_parsing_type(valuesNew.getInt64(2) >> 63);
			valuesNew.insertInt64(_mm_cvtsi128_si64(_mm_clmulepi64_si128(valueHigh, allOnes, 1)) ^ prevInstring, 3);
			prevInstring = string_parsing_type(valuesNew.getInt64(3) >> 63);
			return valuesNew;
		}

	  protected:
		avx_type_small value{};
	};

	template<> struct alignas(64) simd_base<512> {
	  public:
		inline simd_base() = default;

		inline simd_base& operator=(avx_type&& data) {
			value = std::forward<avx_type>(data);
			return *this;
		}

		inline simd_base(avx_type&& data) {
			value = std::forward<avx_type>(data);
		}

		inline simd_base& operator=(uint8_t other) {
			value = _mm512_set1_epi8(other);
			return *this;
		}

		inline explicit simd_base(uint8_t other) {
			*this = other;
		}

		inline simd_base(string_view_ptr values) {
			value = gatherValues512<const uint8_t>(values);
		}

		inline operator avx_type&() {
			return value;
		}

		inline operator const avx_type&&() const {
			return std::forward<const avx_type>(value);
		}

		inline explicit operator bool() const {
			return _mm512_test_epi64_mask(value, value) != 0;
		}

		inline explicit operator string_parsing_type() const {
			return toBitMask();
		}

		template<typename simd_base_type> inline simd_base operator|(simd_base_type&& other) const {
			return _mm512_or_si512(value, std::forward<simd_base_type>(other));
		}

		template<typename simd_base_type> inline simd_base operator-(simd_base_type&& other) const {
			return _mm512_sub_epi8(value, std::forward<simd_base_type>(other));
		}

		template<typename simd_base_type> inline simd_base operator&(simd_base_type&& other) const {
			return _mm512_and_si512(value, std::forward<simd_base_type>(other));
		}

		template<typename simd_base_type> inline simd_base operator^(simd_base_type&& other) const {
			return _mm512_xor_si512(value, std::forward<simd_base_type>(other));
		}

		inline string_parsing_type operator==(const simd_base& other) const {
			return _mm512_cmpeq_epi8_mask(value, other);
		}

		inline string_parsing_type operator==(uint8_t other) const {
			return _mm512_cmpeq_epi8_mask(value, _mm512_set1_epi8(other));
		}

		inline simd_base operator~() const {
			return _mm512_xor_si512(*this, _mm512_set1_epi64(-1ll));
		}

		inline void convertWhitespaceToSimdBase(const simd_base* valuesNew) {
			alignas(ALIGNMENT) static constexpr uint8_t arrayNew[64]{ ' ', 100, 100, 100, 17, 100, 113, 2, 100, '\t', '\n', 112, 100, '\r', 100, 100, ' ', 100, 100, 100, 17, 100,
				113, 2, 100, '\t', '\n', 112, 100, '\r', 100, 100, ' ', 100, 100, 100, 17, 100, 113, 2, 100, '\t', '\n', 112, 100, '\r', 100, 100, ' ', 100, 100, 100, 17, 100, 113,
				2, 100, '\t', '\n', 112, 100, '\r', 100, 100 };
			simd_base whitespaceTable{ arrayNew };

			for (string_parsing_type x = 0; x < 8; ++x) {
				addValues(valuesNew[x].shuffle(whitespaceTable) == valuesNew[x], x);
			}
		}

		inline void convertBackslashesToSimdBase(const simd_base* valuesNew) {
			for (string_parsing_type x = 0; x < 8; ++x) {
				addValues(valuesNew[x] == '\\', x);
			}
		}

		inline void convertStructuralsToSimdBase(const simd_base* valuesNew) {
			alignas(ALIGNMENT) static constexpr uint8_t arrayNew[64]{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, ':', '{', ',', '}', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, ':', '{', ',', '}', 0,
				0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, ':', '{', ',', '}', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, ':', '{', ',', '}', 0, 0 };
			simd_base opTable{ arrayNew };
			uint8_t newValue{ 0x20 };
			simd_base chars{ newValue };

			for (string_parsing_type x = 0; x < 8; ++x) {
				addValues(valuesNew[x].shuffle(opTable) == (valuesNew[x] | chars), x);
			}
		}

		inline void convertQuotesToSimdBase(const simd_base* valuesNew) {
			for (string_parsing_type x = 0; x < 8; ++x) {
				addValues(valuesNew[x] == '"', x);
			}
		}

		inline simd_base bitAndNot(const simd_base& other) const {
			return _mm512_andnot_si512(other, value);
		}

		inline simd_base shuffle(simd_base& other) const {
			return _mm512_shuffle_epi8(other, value);
		}

		inline void insertInt64(string_parsing_type valueNew, uint64_t index) {
			switch (index) {
				case 0: {
					auto lower = _mm512_extracti64x4_epi64(value, 0);
					lower	   = _mm256_insert_epi64(lower, static_cast<int64_t>(valueNew), 0);
					value	   = _mm512_inserti64x4(value, lower, 0);
					break;
				}
				case 1: {
					auto lower = _mm512_extracti64x4_epi64(value, 0);
					lower	   = _mm256_insert_epi64(lower, static_cast<int64_t>(valueNew), 1);
					value	   = _mm512_inserti64x4(value, lower, 0);
					break;
				}
				case 2: {
					auto lower = _mm512_extracti64x4_epi64(value, 0);
					lower	   = _mm256_insert_epi64(lower, static_cast<int64_t>(valueNew), 2);
					value	   = _mm512_inserti64x4(value, lower, 0);
					break;
				}
				case 3: {
					auto lower = _mm512_extracti64x4_epi64(value, 0);
					lower	   = _mm256_insert_epi64(lower, static_cast<int64_t>(valueNew), 3);
					value	   = _mm512_inserti64x4(value, lower, 0);
					break;
				}
				case 4: {
					auto lower = _mm512_extracti64x4_epi64(value, 1);
					lower	   = _mm256_insert_epi64(lower, static_cast<int64_t>(valueNew), 0);
					value	   = _mm512_inserti64x4(value, lower, 1);
					break;
				}
				case 5: {
					auto lower = _mm512_extracti64x4_epi64(value, 1);
					lower	   = _mm256_insert_epi64(lower, static_cast<int64_t>(valueNew), 1);
					value	   = _mm512_inserti64x4(value, lower, 1);
					break;
				}
				case 6: {
					auto lower = _mm512_extracti64x4_epi64(value, 1);
					lower	   = _mm256_insert_epi64(lower, static_cast<int64_t>(valueNew), 2);
					value	   = _mm512_inserti64x4(value, lower, 1);
					break;
				}
				case 7: {
					auto lower = _mm512_extracti64x4_epi64(value, 1);
					lower	   = _mm256_insert_epi64(lower, static_cast<int64_t>(valueNew), 3);
					value	   = _mm512_inserti64x4(value, lower, 1);
					break;
				}
				default: {
					auto lower = _mm512_extracti64x4_epi64(value, 0);
					lower	   = _mm256_insert_epi64(lower, static_cast<int64_t>(valueNew), 0);
					value	   = _mm512_inserti64x4(value, lower, 0);
					break;
				}
			}
		}

		inline void addValues(string_parsing_type values, uint64_t index) {
			insertInt64(values, index);
		}

		template<string_parsing_type amount> inline simd_base shl() const {
			simd_base newValue{ _mm512_slli_epi64(*this, (amount % 64)) };
			simd_base newerValue{ _mm512_srli_epi64(_mm512_permutexvar_epi64(_mm512_set_epi64(6, 5, 4, 3, 2, 1, 0, 7), *this), 64 - (amount % 64)) };
			int64_t maxValue{ static_cast<int64_t>(std::numeric_limits<string_parsing_type>::max()) };
			simd_base newestValue{ _mm512_set_epi64(maxValue, maxValue, maxValue, maxValue, maxValue, maxValue, maxValue, 0) };
			return simd_base{ newValue | (newerValue & newestValue) };
		}

		inline string_parsing_type toBitMask() const {
			return static_cast<string_parsing_type>(_mm512_movepi8_mask(*this));
		}

		inline void reset() {
			value = _mm512_setzero_si512();
		}

		template<typename value_type> inline void store(value_type* storageLocation) const {
			_mm512_store_si512(storageLocation, value);
		}

		inline simd_base& setLSB(bool valueNew) {
			if (valueNew) {
				*this = _mm512_or_si512(*this, _mm512_set_epi64(0, 0, 0, 0, 0, 0, 0, 0x1));
			} else {
				*this = _mm512_andnot_si512(_mm512_set_epi64(0, 0, 0, 0, 0, 0, 0, 0x1), *this);
			}
			return *this;
		}

		inline bool checkMSB() const {
			avx_type result = _mm512_and_si512(*this, _mm512_set_epi64(0x8000000000000000, 0, 0, 0, 0, 0, 0, 0));
			return static_cast<bool>(_mm512_test_epi64_mask(result, result));
		}

		inline simd_base carrylessMultiplication(string_parsing_type& prevInstring) const {
			simd_base<256> lowValues{ _mm512_extracti64x4_epi64(value, 0) };
			simd_base<256> highValues{ _mm512_extracti64x4_epi64(value, 1) };
			lowValues  = lowValues.carrylessMultiplication(prevInstring);
			highValues = highValues.carrylessMultiplication(prevInstring);
			simd_base returnValue{};
			returnValue = _mm512_inserti64x4(returnValue, lowValues, 0);
			returnValue = _mm512_inserti64x4(returnValue, highValues, 1);
			return returnValue;
		}

		inline bool collectCarries(const simd_base& other1, simd_base& result) const {
			avx_type carry = _mm512_set1_epi64(0);
			avx_type sum   = _mm512_add_epi64(*this, other1);
			sum			   = _mm512_add_epi64(sum, carry);
			result		   = std::move(sum);
			return _mm512_test_epi64_mask(carry, sum) == 0;
		}

		inline simd_base follows(bool& overflow) {
			simd_base result = shl<1>();
			result.setLSB(overflow);
			overflow = checkMSB();
			return result;
		}

		inline void printBits(string_parsing_type values, const std::string& valuesTitle) const {
			std::cout << valuesTitle;
			std::cout << std::bitset<64>{ values };
			std::cout << std::endl;
		}

		inline simd_base& printBits(const std::string& valuesTitle) {
			std::cout << valuesTitle;
			uint8_t valueBytes[BytesPerStep];
			std::memcpy(valueBytes, &value, BytesPerStep);

			for (string_parsing_type x = 0; x < BytesPerStep; ++x) {
				for (int64_t y = 7; y >= 0; --y) {
					uint8_t bit = (valueBytes[x] >> y) & 1;
					std::cout << std::bitset<1>{ bit };
				}
			}

			std::cout << std::endl;
			return *this;
		}

	  protected:
		avx_type value{};
	};

	inline simd_base_real makeSimdBase(int64_t value) {
		return _mm512_set1_epi64(value);
	}

	#define load(value) gatherValues512<std::remove_pointer_t<decltype(value)>>(value)

#endif

}