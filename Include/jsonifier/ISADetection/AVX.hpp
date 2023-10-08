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

	template<typename value_type>
	concept avx_t = std::same_as<std::decay_t<value_type>, avx_int_128>;

	template<typename value_type> inline avx_int_128 gatherValues128(const value_type* str) {
		alignas(JsonifierAlignment) double newArray[sizeof(avx_int_128) / sizeof(double)]{};
		std::memcpy(newArray, str, sizeof(avx_int_128));
		return _mm_castpd_si128(_mm_load_pd(newArray));
	}

	template<jsonifier::concepts::float_t value_type> inline avx_float_128 gatherValues128(const value_type* str) {
		return _mm_load_ps(str);
	}

	template<> class simd_base_internal<128> {
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
			value = _mm_set1_epi8(other);
			return *this;
		}

		inline explicit simd_base_internal(uint8_t other) {
			*this = other;
		}

		inline simd_base_internal(const uint8_t values[BytesPerStep]) {
			value = gatherValues128(values);
		}

		inline operator const avx_int_128&() const {
			return value;
		}

		inline explicit operator bool() const {
			return !_mm_testz_si128(value, value);
		}

		template<typename simd_base_type> inline simd_base_internal operator|(simd_base_type&& other) const {
			return _mm_or_si128(value, std::forward<simd_base_type>(other));
		}

		template<typename simd_base_type> inline simd_base_internal operator-(simd_base_type&& other) const {
			return _mm_sub_epi8(value, std::forward<simd_base_type>(other));
		}

		template<typename simd_base_type> inline simd_base_internal operator&(simd_base_type&& other) const {
			return _mm_and_si128(value, std::forward<simd_base_type>(other));
		}

		template<typename simd_base_type> inline simd_base_internal operator^(simd_base_type&& other) const {
			return _mm_xor_si128(value, std::forward<simd_base_type>(other));
		}

		inline string_parsing_type operator==(const simd_base_internal& other) const {
			simd_base_internal newValue{ _mm_cmpeq_epi8(value, other) };
			return newValue.toBitMask();
		}

		inline string_parsing_type operator==(const uint8_t& other) const {
			simd_base_internal newValue = _mm_cmpeq_epi8(value, _mm_set1_epi8(other));
			return newValue.toBitMask();
		}

		inline simd_base_internal operator~() const {
			return _mm_xor_si128(*this, _mm_set1_epi64x(std::numeric_limits<uint64_t>::max()));
		}

		template<uint64_t index = 0> inline void convertWhitespaceToSimdBase(simd_base_internal valuesNew[StridesPerStep]) {
			if constexpr (index < StridesPerStep) {
				alignas(JsonifierAlignment) static constexpr uint8_t arrayNew[]{ ' ', 100, 100, 100, 17, 100, 113, 2, 100, '\t', '\n', 112, 100, '\r', 100, 100 };
				static const simd_base_internal whitespaceTable{ arrayNew };
				addValues<index>(valuesNew[index].shuffle(whitespaceTable) == valuesNew[index]);
				convertWhitespaceToSimdBase<index + 1>(valuesNew);
			}
		}

		template<uint64_t index = 0> inline void convertBackslashesToSimdBase(simd_base_internal valuesNew[StridesPerStep]) {
			if constexpr (index < StridesPerStep) {
				static const simd_base_internal backslashes{ _mm_set1_epi8('\\') };
				addValues<index>(valuesNew[index] == backslashes);
				convertBackslashesToSimdBase<index + 1>(valuesNew);
			}
		}

		template<uint64_t index = 0> inline void convertStructuralsToSimdBase(simd_base_internal valuesNew[StridesPerStep]) {
			if constexpr (index < StridesPerStep) {
				alignas(JsonifierAlignment) static constexpr uint8_t arrayNew[]{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, ':', '{', ',', '}', 0, 0 };
				static const simd_base_internal opTable{ arrayNew };
				static const simd_base_internal chars{ uint8_t{ 0x20 } };
				addValues<index>(valuesNew[index].shuffle(opTable) == (valuesNew[index] | chars));
				convertStructuralsToSimdBase<index + 1>(valuesNew);
			}
		}

		template<uint64_t index = 0> inline void convertQuotesToSimdBase(simd_base_internal valuesNew[StridesPerStep]) {
			if constexpr (index < StridesPerStep) {
				static const simd_base_internal quotes{ _mm_set1_epi8('"') };
				addValues<index>(valuesNew[index] == quotes);
				convertQuotesToSimdBase<index + 1>(valuesNew);
			}
		}

		template<uint64_t index = 0> inline uint64_t getUint64() const {
			static_assert(index < SixtyFourBytesPerStep, "Sorry, but that index value is incorrect.");
			return static_cast<uint64_t>(_mm_extract_epi64(value, index));
		}

		template<uint64_t index = 0> inline void insertUint64(uint64_t valueNew) {
			static_assert(index < SixtyFourBytesPerStep, "Sorry, but that index value is incorrect.");
			value = _mm_insert_epi64(value, static_cast<int64_t>(valueNew), index);
		}

		template<uint64_t index = 0> inline void insertUint16(string_parsing_type valueNew) {
			static_assert(index < StridesPerStep, "Sorry, but that index value is incorrect.");
			value = _mm_insert_epi16(value, static_cast<int16_t>(valueNew), index);
		}

		inline simd_base_internal bitAndNot(const simd_base_internal& other) const {
			return _mm_andnot_si128(other, value);
		}

		inline simd_base_internal shuffle(const simd_base_internal& other) const {
			return _mm_shuffle_epi8(other, value);
		}

		template<uint64_t index = 0> inline void addValues(string_parsing_type valuesToAdd) {
			insertUint16<index>(valuesToAdd);
		}

		template<uint64_t amount> inline simd_base_internal shl() const {
			simd_base_internal currentValues{};
			currentValues.insertUint64(getUint64() << amount);
			uint64_t shiftBetween = amount % 64;
			currentValues.insertUint64<1>((getUint64<1>() << amount) | (getUint64() >> (64 - shiftBetween)));
			return currentValues;
		}

		inline string_parsing_type toBitMask() const {
			return static_cast<string_parsing_type>(_mm_movemask_epi8(*this));
		}

		inline void reset() {
			value = _mm_setzero_si128();
		}

		template<typename value_type> inline void store(value_type* storageLocation) {
			alignas(JsonifierAlignment) double newArray[SixtyFourBytesPerStep]{};
			_mm_store_pd(newArray, _mm_castsi128_pd(value));
			std::memcpy(storageLocation, newArray, sizeof(value));
		}

		inline simd_base_internal& setLSB(bool valueNew) {
			if (valueNew) {
				*this = _mm_or_si128(*this, _mm_set_epi64x(0, 0x1));
			} else {
				*this = _mm_andnot_si128(_mm_set_epi64x(0, 0x1), *this);
			}
			return *this;
		}

		inline bool checkMSB() const {
			avx_int_128 result = _mm_and_si128(*this, _mm_set_epi64x(0x8000000000000000, 0));
			return !_mm_testz_si128(result, result);
		}

		inline simd_base_internal carrylessMultiplication(uint64_t& prevInString) const {
			avx_int_128 allOnes{ _mm_set1_epi8('\xFF') };
			simd_base_internal valuesNew{};
			avx_int_128 valueNew{ value };
			valuesNew.insertUint64(static_cast<uint64_t>(_mm_cvtsi128_si64(_mm_clmulepi64_si128(valueNew, allOnes, 0)) ^ prevInString));
			prevInString = uint64_t(static_cast<int64_t>(valuesNew.getUint64()) >> 63);
			valuesNew.insertUint64<1>(static_cast<uint64_t>(_mm_cvtsi128_si64(_mm_clmulepi64_si128(valueNew, allOnes, 1)) ^ prevInString));
			prevInString = uint64_t(static_cast<int64_t>(valuesNew.getUint64<1>()) >> 63);
			return valuesNew;
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
		avx_int_128 value{};
	};

	inline simd_base makeSimdBase(uint64_t value) {
		return _mm_set1_epi64x(value);
	}

	#define load(value) gatherValues128(value)

#endif

}