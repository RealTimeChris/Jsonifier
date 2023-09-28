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

#if JSONIFIER_CHECK_FOR_INSTRUCTION(JSONIFIER_AVX) && !JSONIFIER_CHECK_FOR_INSTRUCTION(JSONIFIER_AVX2) && !JSONIFIER_CHECK_FOR_INSTRUCTION(JSONIFIER_AVX512)

	constexpr uint64_t StepSize{ 128 };
	constexpr uint64_t BytesPerStep{ StepSize / 8 };
	constexpr uint64_t StridesPerStep{ StepSize / 64 };
	using simd_base_real	  = simd_base<128>;
	using string_parsing_type = uint16_t;
	using avx_type			  = __m128i;
	using avx_float_type	  = __m128;

	template<typename value_type> inline __m128i gatherValues128(const value_type* str) {
		alignas(ALIGNMENT) float newArray[sizeof(__m128i) / sizeof(float)]{};
		std::memcpy(newArray, str, sizeof(__m128i));
		return _mm_castps_si128(_mm_load_ps(newArray));
	}

	template<float_t value_type> inline avx_float_type gatherValues128(const value_type* str) {
		return _mm_load_ps(str);
	}

	template<> struct alignas(16) simd_base<128> {
	  public:
		inline simd_base() = default;

		inline simd_base& operator=(avx_type&& data) {
			value = std::move(data);
			return *this;
		}

		inline simd_base(avx_type&& data) {
			*this = std::move(data);
		}

		inline simd_base& operator=(uint8_t other) {
			value = _mm_set1_epi8(other);
			return *this;
		}

		inline explicit simd_base(uint8_t other) {
			*this = other;
		}

		inline simd_base(string_view_ptr values) {
			value = gatherValues128<const uint8_t>(values);
		}

		inline operator avx_type&() {
			return value;
		}

		inline operator const avx_type&&() const {
			return std::forward<const avx_type>(value);
		}

		inline explicit operator bool() const {
			return !_mm_testz_si128(value, value);
		}

		inline operator string_parsing_type() const {
			return toBitMask();
		}

		template<typename simd_base_type> inline simd_base operator|(simd_base_type&& other) const {
			return _mm_or_si128(value, std::forward<simd_base_type>(other));
		}

		template<typename simd_base_type> inline simd_base operator-(simd_base_type&& other) const {
			return _mm_sub_epi8(value, std::forward<simd_base_type>(other));
		}

		template<typename simd_base_type> inline simd_base operator&(simd_base_type&& other) const {
			return _mm_and_si128(value, std::forward<simd_base_type>(other));
		}

		template<typename simd_base_type> inline simd_base operator^(simd_base_type&& other) const {
			return _mm_xor_si128(value, std::forward<simd_base_type>(other));
		}

		inline string_parsing_type operator==(const simd_base& other) const {
			simd_base newValue{ _mm_cmpeq_epi8(value, other) };
			return newValue.toBitMask();
		}

		inline string_parsing_type operator==(const uint8_t& other) const {
			simd_base newValue = _mm_cmpeq_epi8(value, _mm_set1_epi8(other));
			return newValue.toBitMask();
		}

		inline simd_base operator~() const {
			return _mm_xor_si128(*this, _mm_set1_epi64x(std::numeric_limits<uint64_t>::max()));
		}

		inline void convertWhitespaceToSimdBase(const simd_base* valuesNew) {
			alignas(ALIGNMENT) static constexpr uint8_t arrayNew[16]{ ' ', 100, 100, 100, 17, 100, 113, 2, 100, '\t', '\n', 112, 100, '\r', 100, 100 };
			simd_base whitespaceTable{ arrayNew };
			for (uint64_t x = 0; x < 8; ++x) {
				addValues(valuesNew[x].shuffle(whitespaceTable) == valuesNew[x], x);
			}
		};

		inline void convertBackslashesToSimdBase(const simd_base* valuesNew) {
			simd_base backslashes{ _mm_set1_epi8('\\') };
			for (uint64_t x = 0; x < 8; ++x) {
				addValues(valuesNew[x] == backslashes, x);
			}
		};

		inline void convertStructuralsToSimdBase(const simd_base* valuesNew) {
			alignas(ALIGNMENT) static constexpr uint8_t arrayNew[16]{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, ':', '{', ',', '}', 0, 0 };
			simd_base opTable{ arrayNew };
			simd_base chars{ uint8_t{ 0x20 } };
			for (uint64_t x = 0; x < 8; ++x) {
				addValues(valuesNew[x].shuffle(opTable) == (valuesNew[x] | chars), x);
			}
		};

		inline void convertQuotesToSimdBase(const simd_base* valuesNew) {
			simd_base quotes = _mm_set1_epi8('"');
			for (uint64_t x = 0; x < 8; ++x) {
				addValues(valuesNew[x] == quotes, x);
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

		inline void insertInt16(string_parsing_type valueNew, uint64_t index) {
			switch (index) {
				case 0: {
					value = _mm_insert_epi16(value, static_cast<int16_t>(valueNew), 0);
					break;
				}
				case 1: {
					value = _mm_insert_epi16(value, static_cast<int16_t>(valueNew), 1);
					break;
				}
				case 2: {
					value = _mm_insert_epi16(value, static_cast<int16_t>(valueNew), 2);
					break;
				}
				case 3: {
					value = _mm_insert_epi16(value, static_cast<int16_t>(valueNew), 3);
					break;
				}
				case 4: {
					value = _mm_insert_epi16(value, static_cast<int16_t>(valueNew), 4);
					break;
				}
				case 5: {
					value = _mm_insert_epi16(value, static_cast<int16_t>(valueNew), 5);
					break;
				}
				case 6: {
					value = _mm_insert_epi16(value, static_cast<int16_t>(valueNew), 6);
					break;
				}
				case 7: {
					value = _mm_insert_epi16(value, static_cast<int16_t>(valueNew), 7);
					break;
				}
				default: {
					value = _mm_insert_epi16(value, static_cast<int16_t>(valueNew), 0);
					break;
				}
			}
		}

		inline simd_base bitAndNot(const simd_base& other) const {
			return _mm_andnot_si128(other, value);
		}

		inline simd_base shuffle(const simd_base& other) const {
			return _mm_shuffle_epi8(other, value);
		}

		inline void addValues(string_parsing_type values, uint64_t index) {
			insertInt16(values, index);
		}

		template<uint64_t amount> inline simd_base shl() const {
			simd_base currentValues{};
			currentValues.insertInt64(static_cast<uint64_t>(getInt64(0)) << amount, 0);
			size_t shiftBetween = amount % 64;
			currentValues.insertInt64(static_cast<uint64_t>((getInt64(1)) << amount) | (static_cast<uint64_t>(getInt64(0)) >> (64 - shiftBetween)), 1);
			return currentValues;
		}

		inline string_parsing_type toBitMask() const {
			return static_cast<string_parsing_type>(_mm_movemask_epi8(*this));
		}

		inline void reset() {
			value = _mm_setzero_si128();
		}

		template<typename value_type> inline void store(value_type* storageLocation) {
			alignas(ALIGNMENT) float newArray[StridesPerStep * 2]{};
			_mm_store_ps(newArray, _mm_castsi128_ps(value));
			std::memcpy(storageLocation, newArray, sizeof(value));
		}

		inline simd_base& setLSB(bool valueNew) {
			if (valueNew) {
				*this = _mm_or_si128(*this, _mm_set_epi64x(0, 0x1));
			} else {
				*this = _mm_andnot_si128(_mm_set_epi64x(0, 0x1), *this);
			}
			return *this;
		}

		inline bool checkMSB() const {
			avx_type result = _mm_and_si128(*this, _mm_set_epi64x(0x8000000000000000, 0));
			return !_mm_testz_si128(result, result);
		}

		inline simd_base carrylessMultiplication(uint64_t& prevInString) const {
			__m128i allOnes{ _mm_set1_epi8('\xFF') };
			simd_base valuesNew{};
			__m128i valueLow{ value };
			valuesNew.insertInt64(_mm_cvtsi128_si64(_mm_clmulepi64_si128(valueLow, allOnes, 0)) ^ prevInString, 0);
			prevInString = uint64_t(static_cast<int64_t>(valuesNew.getInt64(0)) >> 63);
			valuesNew.insertInt64(_mm_cvtsi128_si64(_mm_clmulepi64_si128(valueLow, allOnes, 1)) ^ prevInString, 1);
			prevInString = uint64_t(static_cast<int64_t>(valuesNew.getInt64(1)) >> 63);
			return valuesNew;
		}

		inline bool collectCarries(simd_base<128>& other1, simd_base<128>& result) {
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

	inline simd_base_real makeSimdBase(uint64_t value) {
		return _mm_set1_epi64x(value);
	}

	#define load(value) gatherValues128<std::remove_pointer_t<decltype(value)>>(value)

#endif

}