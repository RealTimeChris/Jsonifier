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

#if !JSONIFIER_CHECK_FOR_INSTRUCTION(JSONIFIER_AVX) && !JSONIFIER_CHECK_FOR_INSTRUCTION(JSONIFIER_AVX2) && !JSONIFIER_CHECK_FOR_INSTRUCTION(JSONIFIER_AVX512)

	template<typename value_type>
	concept avx_int_128_t = std::same_as<std::unwrap_ref_decay_t<value_type>, avx_int_128>;

	template<typename value_type>
	concept simd_base_t = std::same_as<std::unwrap_ref_decay_t<value_type>, simd_base_internal<BitsPerStep>>;

	template<typename ReturnT> inline ReturnT gatherValues(jsonifier::concepts::float_t auto* str);

	inline uint64_t packUint64(const uint8_t* arr) {
		return (static_cast<uint64_t>(arr[7]) << 56) | (static_cast<uint64_t>(arr[6]) << 48) | (static_cast<uint64_t>(arr[5]) << 40) | (static_cast<uint64_t>(arr[4]) << 32) |
			(static_cast<uint64_t>(arr[3]) << 24) | (static_cast<uint64_t>(arr[2]) << 16) | (static_cast<uint64_t>(arr[1]) << 8) | static_cast<uint64_t>(arr[0]);
	}

	inline void packUint64Array(const uint8_t* input, avx_int_128& output) {
		output.values_uint64[0] = packUint64(const_cast<string_buffer_ptr>(input));
		output.values_uint64[1] = packUint64(const_cast<string_buffer_ptr>(input + 8));
	}

	template<avx_int_128_t avx_type> inline avx_type gatherValues(jsonifier::concepts::char_type auto* str) {
		alignas(JsonifierAlignment) avx_int_128 newArray{};
		packUint64Array(str, newArray);
		return newArray;
	}

	template<uint64_t index = 0, uint64_t index02 = 0> inline void movemaskEpi8Helper02(int16_t& result, const avx_int_128& a) {
		if constexpr (index < 8) {
			uint8_t value = (a.values_uint64[index02] >> (index * 8)) & 0xff;
			int16_t mask  = (value >> 7) & 1;
			result |= mask << (index02 * 8 + index);
			movemaskEpi8Helper02<index + 1, index02>(result, a);
		}
	}

	template<uint64_t index = 0> inline int16_t movemaskEpi8(const avx_int_128& a) {
		int16_t result{};
		if constexpr (index < 2) {
			movemaskEpi8Helper02<0, index>(result, a);
			result |= movemaskEpi8<index + 1>(a);
		}
		return result;
	}

	inline avx_int_128 orSi128(const avx_int_128& valOne, const avx_int_128& valTwo) {
		avx_int_128 value{};
		value.values_uint64[0] = valOne.values_uint64[0] | valTwo.values_uint64[0];
		value.values_uint64[1] = valOne.values_uint64[1] | valTwo.values_uint64[1];
		return value;
	}

	inline avx_int_128 andSi128(const avx_int_128& valOne, const avx_int_128& valTwo) {
		avx_int_128 value{};
		value.values_uint64[0] = valOne.values_uint64[0] & valTwo.values_uint64[0];
		value.values_uint64[1] = valOne.values_uint64[1] & valTwo.values_uint64[1];
		return value;
	}

	inline avx_int_128 andNotSi128(const avx_int_128& valOne, const avx_int_128& valTwo) {
		avx_int_128 result{};
		result.values_uint64[0] = valTwo.values_uint64[0] & (~valOne.values_uint64[0]);
		result.values_uint64[1] = valTwo.values_uint64[1] & (~valOne.values_uint64[1]);
		return result;
	}

	inline avx_int_128 xorSi128(const avx_int_128& valOne, const avx_int_128& valTwo) {
		avx_int_128 value{};
		value.values_uint64[0] = valOne.values_uint64[0] ^ valTwo.values_uint64[0];
		value.values_uint64[1] = valOne.values_uint64[1] ^ valTwo.values_uint64[1];
		return value;
	}

	inline avx_int_128 subEpi8(const avx_int_128& valOne, const avx_int_128& valTwo) {
		avx_int_128 result{};
		result.values_uint64[0] = valOne.values_uint64[0] - valTwo.values_uint64[0];
		result.values_uint64[1] = valOne.values_uint64[1] - valTwo.values_uint64[1];
		return result;
	}

	inline avx_int_128 notSi128(const avx_int_128& valOne) {
		avx_int_128 result{};
		result.values_uint64[0] = ~valOne.values_uint64[0];
		result.values_uint64[1] = ~valOne.values_uint64[1];
		return result;
	}

	template<uint64_t index = 0> inline avx_int_128 cmpeqEpi8(const avx_int_128& a, const avx_int_128& b) {
		avx_int_128 result{};
		if constexpr (index < 2) {
			cmpeqEpi8Helper<0, index>(result, a, b);
			result = orSi128(result, cmpeqEpi8<index + 1>(a, b));
		}
		return result;
	}

	template<uint64_t index = 0, uint64_t index02 = 0> inline void cmpeqEpi8Helper(avx_int_128& result, const avx_int_128& a, const avx_int_128& b) {
		if constexpr (index < 8) {
			uint8_t a8 = (a.values_uint64[index02] >> (index * 8)) & 0xff;
			uint8_t b8 = (b.values_uint64[index02] >> (index * 8)) & 0xff;
			result.values_uint64[index02] |= (a8 == b8 ? 0xFFULL : 0) << (index * 8);
			cmpeqEpi8Helper<index + 1, index02>(result, a, b);
		}
	}

	inline bool testzSi128(const avx_int_128& valOne, const avx_int_128& valTwo) {
		avx_int_128 result{};
		result.values_uint64[0] = valOne.values_uint64[0] & valTwo.values_uint64[0];
		result.values_uint64[1] = valOne.values_uint64[1] & valTwo.values_uint64[1];
		return result.values_uint64[0] == 0 && result.values_uint64[1] == 0;
	}

	inline avx_int_128 setEpi64x(uint64_t argOne, uint64_t argTwo) {
		avx_int_128 returnValue{};
		returnValue.values_uint64[0] = argTwo;
		returnValue.values_uint64[1] = argOne;
		return returnValue;
	}

	inline avx_int_128 set1Epi64x(uint64_t argOne) {
		avx_int_128 returnValue{};
		returnValue.values_uint64[0] = argOne;
		returnValue.values_uint64[1] = argOne;
		return returnValue;
	}

	inline avx_int_128& insertUint16(avx_int_128& value, uint64_t position, uint16_t newValue) {
		if (position < 0 || position >= 8) {
			return value;
		}
		value.values_uint16[position] = newValue;
		return value;
	}

	inline avx_int_128& insertUint64(avx_int_128& value, uint64_t position, uint64_t newValue) {
		if (position >= 0 && position < 2) {
			value.values_uint64[position] = newValue;
		}
		return value;
	}

	inline uint64_t extractUint64(avx_int_128 value, uint64_t index) {
		if (index >= 0 && index < 2) {
			return static_cast<uint64_t>(value.values_uint64[index]);
		}
		return 0;
	}

	template<uint64_t index = 0> inline avx_int_128 shuffleEpi8(const avx_int_128& a, const avx_int_128& b) {
		avx_int_128 result{};
		if constexpr (index < 16) {
			static constexpr uint64_t i		   = index * 8;
			static constexpr uint64_t bitIndex = i + 7;

			if ((b.values_uint64[bitIndex >> 6] & (1ULL << (bitIndex & 0x3F))) == (1ULL << (bitIndex & 0x3F))) {
				result.values_uint64[bitIndex >> 6] &= ~(1ULL << (bitIndex & 0x3F));
			} else {
				uint64_t indexNew = 0;
				shuffleEpi8Helper02<i + 3, i>(indexNew, b);

				uint64_t aIndex = indexNew * 8;
				shuffleEpi8Helper03<i, i + 7>(result, aIndex, a);
			}
			result = orSi128(result, shuffleEpi8<index + 1>(a, b));
		}
		return result;
	}

	template<int64_t index = 0, int64_t index02 = 0> inline void shuffleEpi8Helper02(uint64_t& result, const avx_int_128& b) {
		if constexpr (index >= index02) {
			result = (result << 1) | ((b.values_uint64[index >> 6] >> (index & 0x3F)) & 1);
			shuffleEpi8Helper02<index - 1, index02>(result, b);
		}
	}

	template<uint64_t index = 0, uint64_t index02 = 0> inline void shuffleEpi8Helper03(avx_int_128& result, uint64_t& result02, const avx_int_128& a) {
		if constexpr (index < index02) {
			result.values_uint64[index >> 6] |= ((a.values_uint64[result02 >> 6] >> (result02 & 0x3F)) & 1ULL) << (index & 0x3F);
			result02++;
			shuffleEpi8Helper03<index + 1, index02>(result, result02, a);
		}
	}

	inline avx_int_128 set1Epi8(int8_t newValue) {
		avx_int_128 returnValue{};
		std::memset(&returnValue, newValue, sizeof(avx_int_128));
		return returnValue;
	}

	template<avx_int_128_t return_type> constexpr return_type simdTable(const uint8_t arrayNew[sizeof(avx_int_128)]) {
		avx_int_128 returnValue{};
		for (uint64_t x = 0; x < sizeof(avx_int_128); ++x) {
			returnValue.values_uint8[x] = arrayNew[x];
		}
		return returnValue;
	}

	template<avx_int_128_t return_type> constexpr return_type simdValues(uint8_t value) {
		avx_int_128 returnValue{};
		for (uint64_t x = 0; x < sizeof(avx_int_128); ++x) {
			returnValue.values_uint8[x] = value;
		}
		return returnValue;
	}

	template<> class simd_base_internal<128> {
	  public:
		inline simd_base_internal() noexcept = default;

		inline simd_base_internal& operator=(simd_base_internal&& other) noexcept {
			value = std::move(other.value);
			return *this;
		}

		inline simd_base_internal(simd_base_internal&& other) noexcept {
			*this = std::move(other);
		}

		inline simd_base_internal& operator=(const simd_base_internal& other) {
			value = other.value;
			return *this;
		}

		inline simd_base_internal(const simd_base_internal& other) {
			*this = other;
		}

		template<avx_int_128_t avx_type_new> constexpr simd_base_internal& operator=(avx_type_new&& data) {
			value = data;
			return *this;
		}

		template<avx_int_128_t avx_type_new> constexpr simd_base_internal(avx_type_new&& data) {
			*this = data;
		}

		inline simd_base_internal& operator=(uint8_t other) {
			value = set1Epi8(other);
			return *this;
		}

		inline simd_base_internal(uint8_t other) {
			*this = other;
		}

		inline simd_base_internal(const uint8_t values_uint64[BytesPerStep]) {
			value = gatherValues<avx_int_128>(values_uint64);
		}

		inline explicit operator bool() {
			return !testzSi128(value, value);
		}

		inline operator avx_int_128&() {
			return value;
		}

		template<typename simd_base_type> inline simd_base_internal operator|(simd_base_type&& other) const {
			return orSi128(value, other.value);
		}

		template<typename simd_base_type> inline simd_base_internal operator-(simd_base_type&& other) const {
			return subEpi8(value, other.value);
		}

		template<typename simd_base_type> inline simd_base_internal operator&(simd_base_type&& other) const {
			return andSi128(value, other.value);
		}

		template<typename simd_base_type> inline simd_base_internal operator^(simd_base_type&& other) const {
			return xorSi128(value, other.value);
		}

		inline string_parsing_type operator==(const uint8_t other) const {
			simd_base_internal newValue = cmpeqEpi8(value, set1Epi8(other));
			return newValue.toBitMask();
		}

		inline string_parsing_type operator==(const simd_base_internal& other) const {
			simd_base_internal newValue = cmpeqEpi8(value, other.value);
			return newValue.toBitMask();
		}

		inline simd_base_internal operator~() {
			return notSi128(value);
		}

		template<uint64_t index = 0> inline void convertWhitespaceToSimdBase(simd_base_internal values_uint64New[StridesPerStep]) {
			if constexpr (index < StridesPerStep) {
				static constexpr uint8_t arrayNew[]{ ' ', 100, 100, 100, 17, 100, 113, 2, 100, '\t', '\n', 112, 100, '\r', 100, 100 };
				static constexpr simd_base_internal whitespaceTable{ simdTable<avx_int_128>(arrayNew) };
				addValues<index>(values_uint64New[index].shuffle(whitespaceTable) == values_uint64New[index]);
				convertWhitespaceToSimdBase<index + 1>(values_uint64New);
			}
		}

		template<uint64_t index = 0> inline void convertBackslashesToSimdBase(simd_base_internal values_uint64New[StridesPerStep]) {
			if constexpr (index < StridesPerStep) {
				static constexpr simd_base_internal backslashesVal{ simdValues<avx_int_128>('\\') };
				addValues<index>(values_uint64New[index] == backslashesVal);
				convertBackslashesToSimdBase<index + 1>(values_uint64New);
			}
		}

		template<uint64_t index = 0> inline void convertStructuralsToSimdBase(simd_base_internal values_uint64New[StridesPerStep]) {
			if constexpr (index < StridesPerStep) {
				static constexpr uint8_t arrayNew[]{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, ':', '{', ',', '}', 0, 0 };
				static constexpr simd_base_internal opTableVal{ simdTable<avx_int_128>(arrayNew) };
				static constexpr simd_base_internal chars{ simdValues<avx_int_128>(0x20) };
				addValues<index>(values_uint64New[index].shuffle(opTableVal) == (values_uint64New[index] | chars));
				convertStructuralsToSimdBase<index + 1>(values_uint64New);
			}
		}

		template<uint64_t index = 0> inline void convertQuotesToSimdBase(simd_base_internal values_uint64New[StridesPerStep]) {
			if constexpr (index < StridesPerStep) {
				static constexpr simd_base_internal quotesVal{ simdValues<avx_int_128>('"') };
				addValues<index>(values_uint64New[index] == quotesVal);
				convertQuotesToSimdBase<index + 1>(values_uint64New);
			}
		}

		inline uint64_t getUint64(uint64_t index) const {
			return extractUint64(value, index);
		}

		inline void insertUint64(uint64_t valueNew, uint64_t index) {
			value = jsonifier_internal::insertUint64(value, index, valueNew);
		}

		inline void insertUint16(uint16_t valueNew, uint64_t index) {
			value = jsonifier_internal::insertUint16(value, index, valueNew);
		}

		inline simd_base_internal bitAndNot(const simd_base_internal& other) {
			return andNotSi128(other.value, value);
		}

		inline simd_base_internal shuffle(const simd_base_internal& other) const {
			return shuffleEpi8(other.value, value);
		}

		template<uint64_t index = 0> inline void addValues(string_parsing_type values_uint64ToAdd) {
			insertUint16(values_uint64ToAdd, index);
		}

		template<uint64_t amount> inline simd_base_internal shl() const {
			simd_base_internal currentValues{};
			currentValues.insertUint64(getUint64(0) << amount, 0);
			uint64_t shiftBetween = amount % 64;
			currentValues.insertUint64((getUint64(1) << amount) | (getUint64(0) >> (64 - shiftBetween)), 1);
			return currentValues;
		}

		inline int16_t toBitMask() {
			return static_cast<int16_t>(movemaskEpi8(value));
		}

		inline void reset() {
			value.values_uint64[0] = 0;
			value.values_uint64[1] = 0;
		}

		template<typename ValueType> inline void store(ValueType* storageLocation) {
			std::memcpy(storageLocation, &value, sizeof(value));
		}

		inline void setLSB(bool valueNew) {
			if (valueNew) {
				*this = orSi128(*this, setEpi64x(0, 0x1));
			} else {
				*this = andNotSi128(setEpi64x(0, 0x1), *this);
			}
		}

		inline bool getMSB() const {
			avx_int_128 result = andSi128(value, setEpi64x(0x8000000000000000, 0));
			return !testzSi128(result, result);
		}

		inline uint64_t prefixXor(uint64_t prevInString) const {
			prevInString ^= prevInString << 1;
			prevInString ^= prevInString << 2;
			prevInString ^= prevInString << 4;
			prevInString ^= prevInString << 8;
			prevInString ^= prevInString << 16;
			prevInString ^= prevInString << 32;
			return prevInString;
		}

		inline simd_base_internal carrylessMultiplication(uint64_t& prevInString) const {
			simd_base_internal values_uint64New{};
			avx_int_128 valueLow{ value };
			values_uint64New.insertUint64(prefixXor(valueLow.values_uint64[0]) ^ prevInString, 0);
			prevInString = uint64_t(static_cast<int64_t>(values_uint64New.getUint64(0)) >> 63);
			values_uint64New.insertUint64(prefixXor(valueLow.values_uint64[1]) ^ prevInString, 1);
			prevInString = uint64_t(static_cast<int64_t>(values_uint64New.getUint64(1)) >> 63);
			return values_uint64New;
		}

		inline simd_base_internal follows(bool& overflow) const {
			simd_base_internal result = shl<1>();
			result.setLSB(overflow);
			overflow = getMSB();
			return result;
		}

		inline void printBits(uint64_t values_uint64, const std::string& values_uint64Title) const {
			std::cout << values_uint64Title;
			std::cout << std::bitset<64>{ values_uint64 };
			std::cout << std::endl;
		}

		inline simd_base_internal& printBits(const std::string& values_uint64Title) noexcept {
			uint8_t values_uint64[BytesPerStep]{};
			store(values_uint64);
			std::cout << values_uint64Title;
			for (uint64_t x = 0; x < BytesPerStep; ++x) {
				for (uint64_t y = 0; y < 8; ++y) {
					std::cout << std::bitset<1>{ static_cast<uint64_t>(*(values_uint64 + x)) >> y };
				}
			}
			std::cout << std::endl;
			return *this;
		}

	  protected:
		avx_int_128 value{};
	};

	template<simd_base_t simd_base_type> inline simd_base_type gatherValues(jsonifier::concepts::char_type auto* str) {
		alignas(JsonifierAlignment) avx_int_128 newArray{};
		packUint64Array(str, newArray);
		return newArray;
	}

	template<simd_base_t simd_base_type> constexpr simd_base_type simdValues(uint8_t values) {
		return simdValues<avx_int_128>(values);
	}

	inline simd_base makeSimdBase(uint64_t value) {
		return set1Epi64x(value);
	}

#endif

}