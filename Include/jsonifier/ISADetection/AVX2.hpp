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

	template<typename value_type>
	concept avx_t = std::same_as<std::decay_t<value_type>, avx_int_256>;

	template<typename value_type> inline avx_int_128 gatherValues128(const value_type* str) {
		alignas(JsonifierAlignment) double newArray[sizeof(avx_int_128) / sizeof(double)]{};
		std::memcpy(newArray, str, sizeof(avx_int_128));
		return _mm_castpd_si128(_mm_load_pd(newArray));
	}

	template<typename value_type> inline avx_int_256 gatherValues256(const value_type* str) {
		alignas(JsonifierAlignment) double newArray[sizeof(avx_int_256) / sizeof(double)]{};
		std::memcpy(newArray, str, sizeof(avx_int_256));
		return _mm256_castpd_si256(_mm256_load_pd(newArray));
	}

	template<jsonifier::concepts::float_t value_type> inline avx_float_128 gatherValues128(const value_type* str) {
		return _mm_load_ps(str);
	}

	template<jsonifier::concepts::float_t value_type> inline avx_float_256 gatherValues256(const value_type* str) {
		return _mm256_load_ps(str);
	}

	template<> class simd_base_internal<256> {
	  public:
		inline simd_base_internal() = default;

		template<avx_t avx_type_new> inline simd_base_internal& operator=(avx_type_new&& data) {
			value = std::forward<avx_type_new>(data);
			return *this;
		}

		template<avx_t avx_type_new> inline simd_base_internal(avx_type_new&& data) {
			*this = std::forward<avx_type_new>(data);
		}

		inline simd_base_internal& operator=(uint8_t other) {
			value = _mm256_set1_epi8(static_cast<char>(other));
			return *this;
		}

		inline explicit simd_base_internal(uint8_t other) {
			*this = other;
		}

		inline simd_base_internal(const uint8_t values[BytesPerStep]) {
			value = gatherValues256(values);
		}

		inline simd_base_internal(const uint64_t values[SixtyFourBytesPerStep]) {
			value = _mm256_setr_epi64x(static_cast<int64_t>(values[0]), static_cast<int64_t>(values[1]), static_cast<int64_t>(values[2]), static_cast<int64_t>(values[3]));
		}

		inline operator const avx_int_256&() const {
			return value;
		}

		inline explicit operator bool() const {
			return !_mm256_testz_si256(value, value);
		}

		template<typename simd_base_type> inline simd_base_internal operator|(simd_base_type&& other) const {
			return _mm256_or_si256(value, std::forward<simd_base_type>(other));
		}

		template<typename simd_base_type> inline simd_base_internal operator-(simd_base_type&& other) const {
			return _mm256_sub_epi8(value, std::forward<simd_base_type>(other));
		}

		template<typename simd_base_type> inline simd_base_internal operator&(simd_base_type&& other) const {
			return _mm256_and_si256(value, std::forward<simd_base_type>(other));
		}

		template<typename simd_base_type> inline simd_base_internal operator^(simd_base_type&& other) const {
			return _mm256_xor_si256(value, std::forward<simd_base_type>(other));
		}

		inline string_parsing_type operator==(const simd_base_internal& other) const {
			simd_base_internal newValue{ _mm256_cmpeq_epi8(value, other) };
			return newValue.toBitMask();
		}

		inline string_parsing_type operator==(const uint8_t& other) const {
			simd_base_internal newValue = _mm256_cmpeq_epi8(value, _mm256_set1_epi8(static_cast<char>(other)));
			return newValue.toBitMask();
		}

		inline simd_base_internal operator~() const {
			return _mm256_xor_si256(*this, _mm256_set1_epi64x(static_cast<int64_t>(std::numeric_limits<uint64_t>::max())));
		}

		template<uint64_t index = 0> inline void convertWhitespaceToSimdBase(simd_base_internal valuesNew[StridesPerStep]) {
			if constexpr (index < StridesPerStep) {
				alignas(JsonifierAlignment) static constexpr uint8_t arrayNew[]{ ' ', 100, 100, 100, 17, 100, 113, 2, 100, '\t', '\n', 112, 100, '\r', 100, 100, ' ', 100, 100, 100,
					17, 100, 113, 2, 100, '\t', '\n', 112, 100, '\r', 100, 100 };
				static const simd_base_internal whitespaceTable{ arrayNew };
				addValues<index>(valuesNew[index].shuffle(whitespaceTable) == valuesNew[index]);
				convertWhitespaceToSimdBase<index + 1>(valuesNew);
			}
		}

		template<uint64_t index = 0> inline void convertBackslashesToSimdBase(simd_base_internal valuesNew[StridesPerStep]) {
			if constexpr (index < StridesPerStep) {
				static const simd_base_internal backslashes{ _mm256_set1_epi8('\\') };
				addValues<index>(valuesNew[index] == backslashes);
				convertBackslashesToSimdBase<index + 1>(valuesNew);
			}
		}

		template<uint64_t index = 0> inline void convertStructuralsToSimdBase(simd_base_internal valuesNew[StridesPerStep]) {
			if constexpr (index < StridesPerStep) {
				alignas(JsonifierAlignment) static constexpr uint8_t arrayNew[]{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, ':', '{', ',', '}', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, ':', '{',
					',', '}', 0, 0 };
				static const simd_base_internal opTable{ arrayNew };
				static const simd_base_internal chars{ uint8_t{ 0x20 } };
				addValues<index>(valuesNew[index].shuffle(opTable) == (valuesNew[index] | chars));
				convertStructuralsToSimdBase<index + 1>(valuesNew);
			}
		}

		template<uint64_t index = 0> inline void convertQuotesToSimdBase(simd_base_internal valuesNew[StridesPerStep]) {
			if constexpr (index < StridesPerStep) {
				static const simd_base_internal quotes{ _mm256_set1_epi8('"') };
				addValues<index>(valuesNew[index] == quotes);
				convertQuotesToSimdBase<index + 1>(valuesNew);
			}
		}

		template<uint64_t index = 0> inline uint64_t getUint64() const {
			static_assert(index < SixtyFourBytesPerStep, "Sorry, but that index value is incorrect.");
			return static_cast<uint64_t>(_mm256_extract_epi64(value, index));
		}

		template<uint64_t index = 0> inline void insertUint64(uint64_t valueNew) {
			static_assert(index < SixtyFourBytesPerStep, "Sorry, but that index value is incorrect.");
			value = _mm256_insert_epi64(value, static_cast<int64_t>(valueNew), index);
		}

		template<uint64_t index = 0> inline void insertUint32(string_parsing_type valueNew) {
			static_assert(index < StridesPerStep, "Sorry, but that index value is incorrect.");
			value = _mm256_insert_epi32(value, static_cast<int32_t>(valueNew), index);
		}

		inline simd_base_internal bitAndNot(const simd_base_internal& other) const {
			return _mm256_andnot_si256(other, value);
		}

		inline simd_base_internal shuffle(const simd_base_internal& other) const {
			return _mm256_shuffle_epi8(other, value);
		}

		template<uint64_t index = 0> inline void addValues(string_parsing_type valuesToAdd) {
			insertUint32<index>(valuesToAdd);
		}

		template<uint64_t amount> inline simd_base_internal shl() const {
			return simd_base_internal{ _mm256_slli_epi64(*this, (amount % 64)) } | _mm256_srli_epi64(_mm256_permute4x64_epi64(*this, 0b10010011), 64 - (amount % 64));
		}

		inline string_parsing_type toBitMask() const {
			return static_cast<string_parsing_type>(_mm256_movemask_epi8(*this));
		}

		inline void reset() {
			value = _mm256_setzero_si256();
		}

		template<typename value_type> inline void store(value_type* storageLocation) {
			alignas(JsonifierAlignment) double newArray[SixtyFourBytesPerStep]{};
			_mm256_store_pd(newArray, _mm256_castsi256_pd(value));
			std::memcpy(storageLocation, newArray, sizeof(value));
		}

		inline simd_base_internal& setLSB(bool valueNew) {
			if (valueNew) {
				*this = _mm256_or_si256(*this, _mm256_set_epi64x(0, 0, 0, 0x1));
			} else {
				*this = _mm256_andnot_si256(_mm256_set_epi64x(0, 0, 0, 0x1), *this);
			}
			return *this;
		}

		inline bool checkMSB() const {
			avx_int_256 result = _mm256_and_si256(*this, _mm256_set_epi64x(0x8000000000000000, 0, 0, 0));
			return !_mm256_testz_si256(result, result);
		}

		template<uint64_t index = 0> inline void processValue(avx_int_128& val, uint64_t* valuesNewer, uint64_t& prevInstring) const {
			static const avx_int_128 allOnes{ _mm_set1_epi8('\xFF') };
			valuesNewer[index] = static_cast<uint64_t>(_mm_cvtsi128_si64(_mm_clmulepi64_si128(val, allOnes, index % 2)) ^ prevInstring);
			prevInstring	   = static_cast<uint64_t>(static_cast<int64_t>(valuesNewer[index]) >> 63);
		}

		inline simd_base_internal carrylessMultiplication(uint64_t& prevInstring) const {
			avx_int_128 valueLow{ _mm256_extracti128_si256(value, 0) };
			avx_int_128 valueHigh{ _mm256_extracti128_si256(value, 1) };
			alignas(JsonifierAlignment) uint64_t valuesNewer[SixtyFourBytesPerStep]{};
			processValue<0>(valueLow, valuesNewer, prevInstring);
			processValue<1>(valueLow, valuesNewer, prevInstring);
			processValue<2>(valueHigh, valuesNewer, prevInstring);
			processValue<3>(valueHigh, valuesNewer, prevInstring);
			return valuesNewer;
		}

		inline simd_base_internal follows(bool& overflow) const {
			simd_base_internal result = shl<1>();
			result.setLSB(overflow);
			overflow = checkMSB();
			return result;
		}

		inline void printBits(uint64_t values, const std::string& valuesTitle) const {
			std::cout << valuesTitle;
			std::cout << std::bitset<64>{ values };
			std::cout << std::endl;
		}

		inline simd_base_internal& printBits(const std::string& valuesTitle) noexcept {
			uint8_t values[BytesPerStep]{};
			store(values);
			std::cout << valuesTitle;
			for (uint64_t x = 0; x < BytesPerStep; ++x) {
				for (uint64_t y = 0; y < 8; ++y) {
					std::cout << std::bitset<1>{ static_cast<uint64_t>(*(values + x)) >> y };
				}
			}
			std::cout << std::endl;
			return *this;
		}

	  protected:
		avx_int_256 value{};
	};

	inline simd_base makeSimdBase(uint64_t value) {
		return _mm256_set1_epi64x(static_cast<int64_t>(value));
	}

	#define load(value) gatherValues256(value)

#endif

}