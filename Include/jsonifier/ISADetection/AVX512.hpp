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

	template<typename value_type>
	concept avx_t = std::same_as<std::unwrap_ref_decay_t<value_type>, avx_int_512>;

	template<typename value_type> inline avx_int_128 gatherValues128(const value_type* str) {
		return _mm_loadu_epi8(str);
	}

	template<typename value_type> inline avx_int_256 gatherValues256(const value_type* str) {
		auto result = _mm256_loadu_epi8(str);
		return result;
	}

	template<typename value_type> inline avx_int_512 gatherValues512(const value_type* str) {
		auto result = _mm512_loadu_epi8(str);
		return result;
	}

	template<jsonifier::concepts::float_t value_type> inline avx_float_128 gatherValues128(const value_type* str) {
		return _mm_load_ps(str);
	}

	template<jsonifier::concepts::float_t value_type> inline avx_float_256 gatherValues256(const value_type* str) {
		return _mm256_load_ps(str);
	}

	template<jsonifier::concepts::float_t value_type> inline avx_float_512 gatherValues512(const value_type* str) {
		return _mm512_load_ps(str);
	}

	#define gatherValues(x) gatherValues512(x)

	template<> class simd_base_internal<256> {
	  public:
		inline simd_base_internal() = default;

		inline simd_base_internal& operator=(avx_int_256&& data) {
			value = std::forward<avx_int_256>(data);
			return *this;
		}

		inline simd_base_internal(avx_int_256&& data) {
			value = std::forward<avx_int_256>(data);
		}

		inline operator avx_int_256&&() {
			return std::forward<avx_int_256>(value);
		}

		template<uint64_t index = 0> inline void processValue(avx_int_128& val, uint64_t* valuesNewer, uint64_t& prevInstring) const {
			static const avx_int_128 allOnes{ _mm_set1_epi8('\xFF') };
			valuesNewer[index] = static_cast<uint64_t>(_mm_cvtsi128_si64(_mm_clmulepi64_si128(val, allOnes, index % 2)) ^ prevInstring);
			prevInstring	   = static_cast<int64_t>(valuesNewer[index]) >> 63;
		}

		inline simd_base_internal carrylessMultiplication(uint64_t& prevInstring) const {
			avx_int_128 valueLow{ _mm256_extracti128_si256(value, 0) };
			avx_int_128 valueHigh{ _mm256_extracti128_si256(value, 1) };
			alignas(JsonifierAlignment) uint64_t valuesNewer[SixtyFourBitsPerStep / 2]{};
			processValue<0>(valueLow, valuesNewer, prevInstring);
			processValue<1>(valueLow, valuesNewer, prevInstring);
			processValue<2>(valueHigh, valuesNewer, prevInstring);
			processValue<3>(valueHigh, valuesNewer, prevInstring);
			return gatherValues256(valuesNewer);
		}

	  protected:
		avx_int_256 value{};
	};

	template<> class simd_base_internal<512> {
	  public:
		inline simd_base_internal() = default;

		template<avx_t avx_type_new> inline simd_base_internal& operator=(avx_type_new&& data) {
			value = _mm512_load_si512(&data);
			return *this;
		}

		template<avx_t avx_type_new> inline simd_base_internal(avx_type_new&& data) {
			*this = data;
		}

		inline simd_base_internal& operator=(uint8_t other) {
			value = _mm512_set1_epi8(other);
			return *this;
		}

		inline explicit simd_base_internal(uint8_t other) {
			*this = other;
		}

		inline operator const avx_int_512&() const {
			return value;
		}

		inline explicit operator bool() const {
			return _mm512_test_epi64_mask(value, value) != 0;
		}

		template<typename simd_base_type> inline simd_base_internal operator|(simd_base_type&& other) const {
			return _mm512_or_si512(value, std::forward<simd_base_type>(other));
		}

		template<typename simd_base_type> inline simd_base_internal operator-(simd_base_type&& other) const {
			return _mm512_sub_epi8(value, std::forward<simd_base_type>(other));
		}

		template<typename simd_base_type> inline simd_base_internal operator&(simd_base_type&& other) const {
			return _mm512_and_si512(value, std::forward<simd_base_type>(other));
		}

		template<typename simd_base_type> inline simd_base_internal operator^(simd_base_type&& other) const {
			return _mm512_xor_si512(value, std::forward<simd_base_type>(other));
		}

		inline string_parsing_type operator==(const simd_base_internal& other) const {
			return _mm512_cmpeq_epi8_mask(value, other);
		}

		inline string_parsing_type operator==(const uint8_t& other) const {
			return _mm512_cmpeq_epi8_mask(value, _mm512_set1_epi8(other));
		}

		inline simd_base_internal operator~() const {
			return _mm512_xor_si512(*this, _mm512_set1_epi64(std::numeric_limits<uint64_t>::max()));
		}

		template<uint64_t index = 0> inline void convertWhitespaceToSimdBase(simd_base_internal valuesNew[StridesPerStep]) {
			if constexpr (index < StridesPerStep) {
				alignas(JsonifierAlignment) static constexpr uint8_t arrayNew[]{ ' ', 100, 100, 100, 17, 100, 113, 2, 100, '\t', '\n', 112, 100, '\r', 100, 100, ' ', 100, 100, 100,
					17, 100, 113, 2, 100, '\t', '\n', 112, 100, '\r', 100, 100, ' ', 100, 100, 100, 17, 100, 113, 2, 100, '\t', '\n', 112, 100, '\r', 100, 100, ' ', 100, 100, 100,
					17, 100, 113, 2, 100, '\t', '\n', 112, 100, '\r', 100, 100 };
				static const simd_base_internal whitespaceTable{ gatherValues(arrayNew) };
				addValues<index>(valuesNew[index].shuffle(whitespaceTable) == valuesNew[index]);
				convertWhitespaceToSimdBase<index + 1>(valuesNew);
			}
		}

		template<uint64_t index = 0> inline void convertBackslashesToSimdBase(simd_base_internal valuesNew[StridesPerStep]) {
			if constexpr (index < StridesPerStep) {
				static const simd_base_internal backslashes{ '\\' };
				addValues<index>(valuesNew[index] == backslashes);
				convertBackslashesToSimdBase<index + 1>(valuesNew);
			}
		}

		template<uint64_t index = 0> inline void convertStructuralsToSimdBase(simd_base_internal valuesNew[StridesPerStep]) {
			if constexpr (index < StridesPerStep) {
				alignas(JsonifierAlignment) static constexpr uint8_t arrayNew[]{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, ':', '{', ',', '}', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, ':', '{',
					',', '}', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, ':', '{', ',', '}', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, ':', '{', ',', '}', 0, 0 };
				static const simd_base_internal opTable{ gatherValues(arrayNew) };
				static const simd_base_internal chars{ 0x20 };
				addValues<index>(valuesNew[index].shuffle(opTable) == (valuesNew[index] | chars));
				convertStructuralsToSimdBase<index + 1>(valuesNew);
			}
		}

		template<uint64_t index = 0> inline void convertQuotesToSimdBase(simd_base_internal valuesNew[StridesPerStep]) {
			if constexpr (index < StridesPerStep) {
				static const simd_base_internal quotes{ '"' };
				addValues<index>(valuesNew[index] == quotes);
				convertQuotesToSimdBase<index + 1>(valuesNew);
			}
		}

		template<uint64_t index = 0> inline void insertUint64(uint64_t valueNew) {
			static_assert(index < SixtyFourBitsPerStep, "Sorry, but that index value is incorrect.");
			auto lower = _mm512_extracti64x4_epi64(value, index / 4);
			lower	   = _mm256_insert_epi64(lower, static_cast<int64_t>(valueNew), index % 4);
			value	   = _mm512_inserti64x4(value, lower, index / 4);
		}

		inline simd_base_internal bitAndNot(const simd_base_internal& other) const {
			return _mm512_andnot_si512(other, value);
		}

		inline simd_base_internal shuffle(const simd_base_internal& other) const {
			return _mm512_shuffle_epi8(other, value);
		}

		template<uint64_t index = 0> inline void addValues(string_parsing_type valuesToAdd) {
			insertUint64<index>(valuesToAdd);
		}

		template<string_parsing_type amount> inline simd_base_internal shl() const {
			simd_base_internal newValue{ _mm512_slli_epi64(*this, (amount % 64)) };
			simd_base_internal newerValue{ _mm512_srli_epi64(_mm512_permutexvar_epi64(_mm512_set_epi64(6, 5, 4, 3, 2, 1, 0, 7), *this), 64 - (amount % 64)) };
			static constexpr int64_t maxValue{ static_cast<int64_t>(std::numeric_limits<string_parsing_type>::max()) };
			simd_base_internal newestValue{ _mm512_set_epi64(maxValue, maxValue, maxValue, maxValue, maxValue, maxValue, maxValue, 0) };
			return simd_base_internal{ newValue | (newerValue & newestValue) };
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

		inline simd_base_internal& setLSB(bool valueNew) {
			if (valueNew) {
				*this = _mm512_or_si512(*this, _mm512_set_epi64(0, 0, 0, 0, 0, 0, 0, 0x1));
			} else {
				*this = _mm512_andnot_si512(_mm512_set_epi64(0, 0, 0, 0, 0, 0, 0, 0x1), *this);
			}
			return *this;
		}

		inline bool getMSB() const {
			avx_int_512 result = _mm512_and_si512(*this, _mm512_set_epi64(0x8000000000000000, 0, 0, 0, 0, 0, 0, 0));
			return static_cast<bool>(_mm512_test_epi64_mask(result, result));
		}

		inline simd_base carrylessMultiplication(string_parsing_type& prevInstring) const {
			simd_base_internal<256> lowValues{ _mm512_extracti64x4_epi64(value, 0) };
			simd_base_internal<256> highValues{ _mm512_extracti64x4_epi64(value, 1) };
			lowValues  = lowValues.carrylessMultiplication(prevInstring);
			highValues = highValues.carrylessMultiplication(prevInstring);
			simd_base returnValue{};
			returnValue = _mm512_inserti64x4(returnValue, lowValues, 0);
			returnValue = _mm512_inserti64x4(returnValue, highValues, 1);
			return returnValue;
		}

		inline simd_base_internal follows(bool& overflow) {
			simd_base_internal result = shl<1>();
			result.setLSB(overflow);
			overflow = getMSB();
			return result;
		}

		inline void printBits(string_parsing_type values, const std::string& valuesTitle) const {
			std::cout << valuesTitle;
			std::cout << std::bitset<64>{ values };
			std::cout << std::endl;
		}

		inline simd_base_internal& printBits(const std::string& valuesTitle) noexcept {
			uint8_t values[BytesPerStep]{};
			store(values);
			std::cout << valuesTitle;
			for (string_parsing_type x = 0; x < BytesPerStep; ++x) {
				for (string_parsing_type y = 0; y < 8; ++y) {
					std::cout << std::bitset<1>{ static_cast<string_parsing_type>(*(values + x)) >> y };
				}
			}
			std::cout << std::endl;
			return *this;
		}

	  protected:
		avx_int_512 value{};
	};

	inline simd_base makeSimdBase(int64_t value) {
		return _mm512_set1_epi64(value);
	}

#endif

}