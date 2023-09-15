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

	constexpr uint64_t StepSize{ 128 };
	constexpr uint64_t BytesPerStep{ StepSize / 8 };
	constexpr uint64_t SixtyFourBytesPerStep{ StepSize / 64 };
	constexpr uint64_t StridesPerStep{ StepSize / BytesPerStep };
	using string_parsing_type = uint16_t;

	template<typename value_type>
	concept avx_t = std::same_as<ref_unwrap<value_type>, avx_int_128>;

	inline uint64_t packUint64(uint8_t* arr) {
		return (static_cast<uint64_t>(arr[7]) << 56) | (static_cast<uint64_t>(arr[6]) << 48) | (static_cast<uint64_t>(arr[5]) << 40) | (static_cast<uint64_t>(arr[4]) << 32) |
			(static_cast<uint64_t>(arr[3]) << 24) | (static_cast<uint64_t>(arr[2]) << 16) | (static_cast<uint64_t>(arr[1]) << 8) | static_cast<uint64_t>(arr[0]);
	}

	inline void packUint64Array(string_view_ptr input, avx_int_128& output) {
		output.values[0] = packUint64(const_cast<uint8_t*>(input));
		output.values[1] = packUint64(const_cast<uint8_t*>(input + 8));
	}

	template<typename value_type> inline avx_int_128 gatherValues128(const value_type* str) {
		alignas(JSONIFIER_ALIGNMENT) avx_int_128 newArray{};
		packUint64Array(str, newArray);
		return newArray;
	}

	template<uint64_t index = 0, uint64_t index02 = 0> inline void movemaskEpi8Helper02(int16_t& result, const avx_int_128& a) {
		if constexpr (index < 8) {
			uint8_t value = (a.values[index02] >> (index * 8)) & 0xff;
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
		value.values[0] = valOne.values[0] | valTwo.values[0];
		value.values[1] = valOne.values[1] | valTwo.values[1];
		return value;
	}

	inline avx_int_128 andSi128(const avx_int_128& valOne, const avx_int_128& valTwo) {
		avx_int_128 value{};
		value.values[0] = valOne.values[0] & valTwo.values[0];
		value.values[1] = valOne.values[1] & valTwo.values[1];
		return value;
	}

	inline avx_int_128 andNotSi128(const avx_int_128& valOne, const avx_int_128& valTwo) {
		avx_int_128 result{};
		result.values[0] = valTwo.values[0] & (~valOne.values[0]);
		result.values[1] = valTwo.values[1] & (~valOne.values[1]);
		return result;
	}

	inline avx_int_128 xorSi128(const avx_int_128& valOne, const avx_int_128& valTwo) {
		avx_int_128 value{};
		value.values[0] = valOne.values[0] ^ valTwo.values[0];
		value.values[1] = valOne.values[1] ^ valTwo.values[1];
		return value;
	}

	inline avx_int_128 subEpi8(const avx_int_128& valOne, const avx_int_128& valTwo) {
		avx_int_128 result{};
		result.values[0] = valOne.values[0] - valTwo.values[0];
		result.values[1] = valOne.values[1] - valTwo.values[1];
		return result;
	}

	inline avx_int_128 notSi128(const avx_int_128& valOne) {
		avx_int_128 result{};
		result.values[0] = ~valOne.values[0];
		result.values[1] = ~valOne.values[1];
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
			uint8_t a8 = (a.values[index02] >> (index * 8)) & 0xff;
			uint8_t b8 = (b.values[index02] >> (index * 8)) & 0xff;
			result.values[index02] |= (a8 == b8 ? 0xFFULL : 0) << (index * 8);
			cmpeqEpi8Helper<index + 1, index02>(result, a, b);
		}
	}

	inline bool testzSi128(const avx_int_128& valOne, const avx_int_128& valTwo) {
		avx_int_128 result{};
		result.values[0] = valOne.values[0] & valTwo.values[0];
		result.values[1] = valOne.values[1] & valTwo.values[1];
		return result.values[0] == 0 && result.values[1] == 0;
	}

	inline avx_int_128 setEpi64x(uint64_t argOne, uint64_t argTwo) {
		avx_int_128 returnValue{};
		returnValue.values[0] = argTwo;
		returnValue.values[1] = argOne;
		return returnValue;
	}

	inline avx_int_128 set1Epi64x(uint64_t argOne) {
		avx_int_128 returnValue{};
		returnValue.values[0] = argOne;
		returnValue.values[1] = argOne;
		return returnValue;
	}

	inline avx_int_128& insertUint16(avx_int_128& value, uint64_t position, uint16_t newValue) {
		if (position < 0 || position >= 8) {
			return value;
		}
		int32_t arrayIndex	= (position < 4) ? 0 : 1;
		int32_t shiftAmount = (position % 4) * 16;
		value.values[arrayIndex] &= ~(static_cast<uint64_t>(0xffff) << shiftAmount);
		value.values[arrayIndex] |= (static_cast<uint64_t>(newValue) << shiftAmount);
		return value;
	}

	inline avx_int_128& insertUint64(avx_int_128& value, uint64_t position, uint64_t newValue) {
		if (position == 0) {
			value.values[0] = static_cast<uint64_t>(newValue);
		} else if (position == 1) {
			value.values[1] = static_cast<uint64_t>(newValue);
		}
		return value;
	}

	inline uint64_t extractUint64(avx_int_128 value, uint64_t index) {
		if (index >= 0 && index < 2) {
			return static_cast<uint64_t>(value.values[index]);
		}
		return 0;
	}

	template<uint64_t index = 0> inline avx_int_128 shuffleEpi8(const avx_int_128& a, const avx_int_128& b) {
		avx_int_128 result{};
		if constexpr (index < 16) {
			static constexpr int32_t i		  = index * 8;
			static constexpr int32_t bitIndex = i + 7;

			if ((b.values[bitIndex >> 6] & (1ULL << (bitIndex & 0x3F))) == (1ULL << (bitIndex & 0x3F))) {
				result.values[bitIndex >> 6] &= ~(1ULL << (bitIndex & 0x3F));
			} else {
				int32_t indexNew = 0;
				shuffleEpi8Helper02<i + 3, i>(indexNew, b);

				int32_t aIndex = indexNew * 8;
				shuffleEpi8Helper03<i, i + 7>(result, aIndex, a);
			}
			result = orSi128(result, shuffleEpi8<index + 1>(a, b));
		}
		return result;
	}

	template<int32_t index = 0, int32_t index02 = 0> inline void shuffleEpi8Helper02(int32_t& result, const avx_int_128& b) {
		if constexpr (index >= index02) {
			result = (result << 1) | ((b.values[index >> 6] >> (index & 0x3F)) & 1);
			shuffleEpi8Helper02<index - 1, index02>(result, b);
		}
	}

	template<int32_t index = 0, int32_t index02 = 0> inline void shuffleEpi8Helper03(avx_int_128& result, int32_t& result02, const avx_int_128& a) {
		if constexpr (index < index02) {
			result.values[index >> 6] |= ((a.values[result02 >> 6] >> (result02 & 0x3F)) & 1ULL) << (index & 0x3F);
			result02++;
			shuffleEpi8Helper03<index + 1, index02>(result, result02, a);
		}
	}

	inline avx_int_128 set1Epi8(int8_t newValue) {
		avx_int_128 returnValue{};
		std::memset(&returnValue, newValue, sizeof(avx_int_128));
		return returnValue;
	}

	template<> class simd_base_internal<128> {
	  public:
		inline simd_base_internal() noexcept = default;

		template<avx_t avx_type_new> inline simd_base_internal& operator=(avx_type_new&& data) {
			value = std::forward<avx_type_new>(data);
			return *this;
		}

		template<avx_t avx_type_new> inline simd_base_internal(avx_type_new&& data) {
			*this = std::forward<avx_type_new>(data);
		}

		inline simd_base_internal& operator=(uint8_t other) {
			value = set1Epi8(other);
			return *this;
		}

		inline simd_base_internal(uint8_t other) {
			*this = other;
		}

		inline simd_base_internal(const uint8_t values[BytesPerStep]) {
			value = gatherValues128(values);
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
			return notSi128(this->value);
		}

		template<uint64_t index = 0> inline void convertWhitespaceToSimdBase(const simd_base_internal* valuesNew) {
			if constexpr (index < StridesPerStep) {
				alignas(JSONIFIER_ALIGNMENT) static constexpr uint8_t arrayNew[]{ ' ', 100, 100, 100, 17, 100, 113, 2, 100, '\t', '\n', 112, 100, '\r', 100, 100 };
				static const simd_base_internal whitespaceTable{ arrayNew };
				addValues<index>(valuesNew[index].shuffle(whitespaceTable) == valuesNew[index]);
				convertWhitespaceToSimdBase<index + 1>(valuesNew);
			}
		}

		template<uint64_t index = 0> inline void convertBackslashesToSimdBase(const simd_base_internal* valuesNew) {
			if constexpr (index < StridesPerStep) {
				static const simd_base_internal backslashes{ set1Epi8('\\') };
				addValues<index>(valuesNew[index] == backslashes);
				convertBackslashesToSimdBase<index + 1>(valuesNew);
			}
		}

		template<uint64_t index = 0> inline void convertStructuralsToSimdBase(const simd_base_internal* valuesNew) {
			if constexpr (index < StridesPerStep) {
				alignas(JSONIFIER_ALIGNMENT) static constexpr uint8_t arrayNew[]{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, ':', '{', ',', '}', 0, 0 };
				static const simd_base_internal opTable{ arrayNew };
				static const simd_base_internal chars{ uint8_t{ 0x20 } };
				addValues<index>(valuesNew[index].shuffle(opTable) == (valuesNew[index] | chars));
				convertStructuralsToSimdBase<index + 1>(valuesNew);
			}
		}

		template<uint64_t index = 0> inline void convertQuotesToSimdBase(const simd_base_internal* valuesNew) {
			if constexpr (index < StridesPerStep) {
				static const simd_base_internal quotes{ set1Epi8('"') };
				addValues<index>(valuesNew[index] == quotes);
				convertQuotesToSimdBase<index + 1>(valuesNew);
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

		template<uint64_t index = 0> inline void addValues(string_parsing_type valuesToAdd) {
			insertUint16(valuesToAdd, index);
		}

		template<uint64_t amount> inline simd_base_internal shl() const {
			simd_base_internal currentValues{};
			currentValues.insertUint64(getUint64(0) << amount, 0);
			uint64_t shiftBetween = amount % 64;
			currentValues.insertUint64((getUint64(1) << amount) | (getUint64(0) >> (64 - shiftBetween)), 1);
			return currentValues;
		}

		inline int16_t toBitMask() {
			return static_cast<int16_t>(movemaskEpi8(this->value));
		}

		inline void reset() {
			value.values[0] = 0;
			value.values[1] = 0;
		}

		template<typename ValueType> inline void store(ValueType* storageLocation) {
			std::memcpy(storageLocation, &value, sizeof(value));
		}

		inline simd_base_internal& setLSB(bool valueNew) {
			if (valueNew) {
				*this = orSi128(*this, setEpi64x(0, 0x1));
			} else {
				*this = andNotSi128(setEpi64x(0, 0x1), *this);
			}
			return *this;
		}

		inline bool checkMSB() const {
			avx_int_128 result = andSi128(this->value, setEpi64x(0x8000000000000000, 0));
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
			simd_base_internal valuesNew{};
			avx_int_128 valueLow{ value };
			valuesNew.insertUint64(prefixXor(valueLow.values[0]) ^ prevInString, 0);
			prevInString = uint64_t(static_cast<int64_t>(valuesNew.getUint64(0)) >> 63);
			valuesNew.insertUint64(prefixXor(valueLow.values[1]) ^ prevInString, 1);
			prevInString = uint64_t(static_cast<int64_t>(valuesNew.getUint64(1)) >> 63);
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

	inline simd_base<StepSize> makeSimdBase(uint64_t value) {
		return set1Epi64x(value);
	}

	#define load(value) gatherValues128(value)

#endif

}