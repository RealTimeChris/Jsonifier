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

#if JSONIFIER_CHECK_FOR_INSTRUCTION(JSONIFIER_AVX2) && !JSONIFIER_CHECK_FOR_INSTRUCTION(JSONIFIER_AVX) && !JSONIFIER_CHECK_FOR_INSTRUCTION(JSONIFIER_AVX512)

	constexpr uint64_t StepSize{ 256 };
	constexpr uint64_t BytesPerStep{ StepSize / 8 };
	constexpr uint64_t StridesPerStep{ StepSize / 64 };
	using simd_base_real	   = simd_base<256>;
	using string_parsing_type  = uint32_t;
	using avx_type			   = __m256i;
	using avx_type_small	   = __m128i;
	using avx_float_type	   = __m256;
	using avx_float_type_small = __m128;

	template<typename value_type> inline avx_type_small gatherValues128(const value_type* str) {
		alignas(ALIGNMENT) float newArray[sizeof(avx_type_small) / sizeof(float)]{};
		std::memcpy(newArray, str, sizeof(avx_type_small));
		return _mm_castps_si128(_mm_load_ps(newArray));
	}

	template<typename value_type> inline avx_type gatherValues256(const value_type* str) {
		alignas(ALIGNMENT) float newArray[sizeof(avx_type) / sizeof(float)]{};
		std::memcpy(newArray, str, sizeof(avx_type));
		return _mm256_castps_si256(_mm256_load_ps(newArray));
	}

	template<float_t value_type> inline avx_float_type_small gatherValues128(const value_type* str) {
		return _mm_load_ps(str);
	}

	template<float_t value_type> inline avx_float_type gatherValues256(const value_type* str) {
		return _mm256_load_ps(str);
	}

	template<> struct alignas(32) simd_base<256> {
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
			value = _mm256_set1_epi8(other);
			return *this;
		}

		inline explicit simd_base(uint8_t other) {
			*this = other;
		}

		inline simd_base(string_view_ptr values) {
			value = gatherValues256(values);
		}

		inline operator avx_type&() {
			return value;
		}

		inline operator const avx_type&&() const {
			return std::forward<const avx_type>(value);
		}

		inline explicit operator bool() const {
			return !_mm256_testz_si256(value, value);
		}

		template<typename simd_base_type> inline simd_base operator|(simd_base_type&& other) const {
			return _mm256_or_si256(value, std::forward<simd_base_type>(other));
		}

		template<typename simd_base_type> inline simd_base operator-(simd_base_type&& other) const {
			return _mm256_sub_epi8(value, std::forward<simd_base_type>(other));
		}

		template<typename simd_base_type> inline simd_base operator&(simd_base_type&& other) const {
			return _mm256_and_si256(value, std::forward<simd_base_type>(other));
		}

		template<typename simd_base_type> inline simd_base operator^(simd_base_type&& other) const {
			return _mm256_xor_si256(value, std::forward<simd_base_type>(other));
		}

		inline string_parsing_type operator==(const simd_base& other) const {
			simd_base newValue{ _mm256_cmpeq_epi8(value, other) };
			return newValue.toBitMask();
		}

		inline string_parsing_type operator==(uint8_t other) const {
			simd_base newValue = _mm256_cmpeq_epi8(value, _mm256_set1_epi8(other));
			return newValue.toBitMask();
		}

		inline simd_base operator~() const {
			return _mm256_xor_si256(*this, _mm256_set1_epi64x(static_cast<int64_t>(std::numeric_limits<uint64_t>::max())));
		}

		inline void convertWhitespaceToSimdBase(const simd_base* valuesNew) {
			alignas(ALIGNMENT) static constexpr uint8_t arrayNew[32]{ ' ', 100, 100, 100, 17, 100, 113, 2, 100, '\t', '\n', 112, 100, '\r', 100, 100, ' ', 100, 100, 100, 17, 100,
				113, 2, 100, '\t', '\n', 112, 100, '\r', 100, 100 };
			simd_base whitespaceTable{ arrayNew };
			for (uint64_t x = 0; x < 8; ++x) {
				addValues(valuesNew[x].shuffle(whitespaceTable) == valuesNew[x], x);
			}
		};

		inline void convertBackslashesToSimdBase(const simd_base* valuesNew) {
			simd_base backslashes{ _mm256_set1_epi8('\\') };
			for (uint64_t x = 0; x < 8; ++x) {
				addValues(valuesNew[x] == backslashes, x);
			}
		};

		inline void convertStructuralsToSimdBase(const simd_base* valuesNew) {
			alignas(
				ALIGNMENT) static constexpr uint8_t arrayNew[32]{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, ':', '{', ',', '}', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, ':', '{', ',', '}', 0, 0 };
			simd_base opTable{ arrayNew };
			simd_base chars{ uint8_t{ 0x20 } };
			for (uint64_t x = 0; x < 8; ++x) {
				addValues(valuesNew[x].shuffle(opTable) == (valuesNew[x] | chars), x);
			}
		};

		inline void convertQuotesToSimdBase(const simd_base* valuesNew) {
			simd_base quotes = _mm256_set1_epi8('"');
			for (uint64_t x = 0; x < 8; ++x) {
				addValues(valuesNew[x] == quotes, x);
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

		inline void insertInt32(string_parsing_type valueNew, uint64_t index) {
			switch (index) {
				case 0: {
					value = _mm256_insert_epi32(value, static_cast<int32_t>(valueNew), 0);
					break;
				}
				case 1: {
					value = _mm256_insert_epi32(value, static_cast<int32_t>(valueNew), 1);
					break;
				}
				case 2: {
					value = _mm256_insert_epi32(value, static_cast<int32_t>(valueNew), 2);
					break;
				}
				case 3: {
					value = _mm256_insert_epi32(value, static_cast<int32_t>(valueNew), 3);
					break;
				}
				case 4: {
					value = _mm256_insert_epi32(value, static_cast<int32_t>(valueNew), 4);
					break;
				}
				case 5: {
					value = _mm256_insert_epi32(value, static_cast<int32_t>(valueNew), 5);
					break;
				}
				case 6: {
					value = _mm256_insert_epi32(value, static_cast<int32_t>(valueNew), 6);
					break;
				}
				case 7: {
					value = _mm256_insert_epi32(value, static_cast<int32_t>(valueNew), 7);
					break;
				}
				default: {
					value = _mm256_insert_epi32(value, static_cast<int32_t>(valueNew), 0);
					break;
				}
			}
		}

		inline simd_base bitAndNot(const simd_base& other) const {
			return _mm256_andnot_si256(other, value);
		}

		inline simd_base shuffle(const simd_base& other) const {
			return _mm256_shuffle_epi8(other, value);
		}

		inline void addValues(string_parsing_type values, uint64_t index) {
			insertInt32(values, index);
		}

		template<uint64_t amount> inline simd_base shl() const {
			return simd_base{ _mm256_slli_epi64(*this, (amount % 64)) } | _mm256_srli_epi64(_mm256_permute4x64_epi64(*this, 0b10010011), 64 - (amount % 64));
		}

		inline string_parsing_type toBitMask() const {
			return static_cast<string_parsing_type>(_mm256_movemask_epi8(*this));
		}

		inline void reset() {
			value = _mm256_setzero_si256();
		}

		template<typename value_type> inline void store(value_type* storageLocation) {
			alignas(ALIGNMENT) float newArray[StridesPerStep * 2]{};
			_mm256_store_ps(newArray, _mm256_castsi256_ps(value));
			std::memcpy(storageLocation, newArray, sizeof(value));
		}

		inline simd_base& setLSB(bool valueNew) {
			if (valueNew) {
				*this = _mm256_or_si256(*this, _mm256_set_epi64x(0, 0, 0, 0x1));
			} else {
				*this = _mm256_andnot_si256(_mm256_set_epi64x(0, 0, 0, 0x1), *this);
			}
			return *this;
		}

		inline bool checkMSB() const {
			avx_type result = _mm256_and_si256(*this, _mm256_set_epi64x(0x8000000000000000, 0, 0, 0));
			return !_mm256_testz_si256(result, result);
		}

		inline simd_base carrylessMultiplication(uint64_t& prevInstring) const {
			avx_type_small allOnes{ _mm_set1_epi8('\xFF') };
			simd_base valuesNew{};
			avx_type_small valueLow{ _mm256_extracti128_si256(value, 0) };
			avx_type_small valueHigh{ _mm256_extracti128_si256(value, 1) };
			valuesNew.insertInt64(_mm_cvtsi128_si64(_mm_clmulepi64_si128(valueLow, allOnes, 0)) ^ prevInstring, 0);
			prevInstring = uint64_t(valuesNew.getInt64(0) >> 63);
			valuesNew.insertInt64(_mm_cvtsi128_si64(_mm_clmulepi64_si128(valueLow, allOnes, 1)) ^ prevInstring, 1);
			prevInstring = uint64_t(valuesNew.getInt64(1) >> 63);
			valuesNew.insertInt64(_mm_cvtsi128_si64(_mm_clmulepi64_si128(valueHigh, allOnes, 0)) ^ prevInstring, 2);
			prevInstring = uint64_t(valuesNew.getInt64(2) >> 63);
			valuesNew.insertInt64(_mm_cvtsi128_si64(_mm_clmulepi64_si128(valueHigh, allOnes, 1)) ^ prevInstring, 3);
			prevInstring = uint64_t(valuesNew.getInt64(3) >> 63);
			return valuesNew;
		}

		inline simd_base follows(bool& overflow) const {
			simd_base result = shl<1>();
			result.setLSB(overflow);
			overflow = checkMSB();
			return result;
		}

		inline void printBits(uint64_t values, const std::string& valuesTitle) const {
			std::cout << valuesTitle;
			std::cout << std::bitset<64>{ values };
			std::cout << std::endl;
		}

		inline simd_base& printBits(const std::string& valuesTitle) {
			std::cout << valuesTitle;
			uint8_t valueBytes[BytesPerStep];
			std::memcpy(valueBytes, &value, BytesPerStep);

			for (uint64_t x = 0; x < BytesPerStep; ++x) {
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
		return _mm256_set1_epi64x(value);
	}

	#define load(value) gatherValues256<std::remove_pointer_t<decltype(value)>>(value)

#endif

}