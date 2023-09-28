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

#if (!JSONIFIER_CHECK_FOR_INSTRUCTION(JSONIFIER_AVX)) && (!JSONIFIER_CHECK_FOR_INSTRUCTION(JSONIFIER_AVX2)) && (!JSONIFIER_CHECK_FOR_INSTRUCTION(JSONIFIER_AVX512))

	class __m128x2 {
	  public:
		uint64_t values[2]{};
	};

	constexpr uint64_t StepSize{ 128 };
	constexpr uint64_t BytesPerStep{ StepSize / 8 };
	constexpr uint64_t StridesPerStep{ StepSize / 64 };
	using string_parsing_type = uint16_t;
	using simd_base_real	  = simd_base<128>;
	using avx_type			  = __m128x2;

	inline uint64_t packUint64(uint8_t* arr) {
		return (static_cast<uint64_t>(arr[7]) << 56) | (static_cast<uint64_t>(arr[6]) << 48) | (static_cast<uint64_t>(arr[5]) << 40) | (static_cast<uint64_t>(arr[4]) << 32) |
			(static_cast<uint64_t>(arr[3]) << 24) | (static_cast<uint64_t>(arr[2]) << 16) | (static_cast<uint64_t>(arr[1]) << 8) | static_cast<uint64_t>(arr[0]);
	}

	inline void packUint64Array(string_view_ptr input, avx_type& output) {
		output.values[0] = packUint64(const_cast<uint8_t*>(input));
		output.values[1] = packUint64(const_cast<uint8_t*>(input + 8));
	}

	template<typename value_type> avx_type gatherValues128(const value_type* str) {
		alignas(ALIGNMENT) avx_type newArray{};
		packUint64Array(str, newArray);
		return newArray;
	}

	inline uint64_t packUint8(const uint8_t arr) {
		return (static_cast<uint64_t>(arr) << 56) | (static_cast<uint64_t>(arr) << 48) | (static_cast<uint64_t>(arr) << 40) | (static_cast<uint64_t>(arr) << 32) |
			(static_cast<uint64_t>(arr) << 24) | (static_cast<uint64_t>(arr) << 16) | (static_cast<uint64_t>(arr) << 8) | static_cast<uint64_t>(arr);
	}

	inline void packUint8Array(const uint8_t input, avx_type& output) {
		output.values[0] = packUint8(input);
		output.values[1] = packUint8(input);
	}

	inline uint16_t movemaskEpi8(const avx_type& a) {
		uint16_t result{};
		for (int16_t j = 0; j < 8; ++j) {
			uint8_t value = (a.values[0] >> (j * 8)) & 0xff;
			uint16_t mask = (value >> 7) & 1;
			result |= mask << (0 * 8 + j);
		}
		for (int16_t j = 0; j < 8; ++j) {
			uint8_t value = (a.values[1] >> (j * 8)) & 0xff;
			uint16_t mask = (value >> 7) & 1;
			result |= mask << (1 * 8 + j);
		}
		return result;
	}

	inline avx_type orSi128(const avx_type& valOne, const avx_type& valTwo) {
		avx_type value{};
		value.values[0] = valOne.values[0] | valTwo.values[0];
		value.values[1] = valOne.values[1] | valTwo.values[1];
		return value;
	}

	inline avx_type andSi128(const avx_type& valOne, const avx_type& valTwo) {
		avx_type value{};
		value.values[0] = valOne.values[0] & valTwo.values[0];
		value.values[1] = valOne.values[1] & valTwo.values[1];
		return value;
	}

	inline avx_type andNotSi128(const avx_type& valOne, const avx_type& valTwo) {
		avx_type result{};
		result.values[0] = valTwo.values[0] & (~valOne.values[0]);
		result.values[1] = valTwo.values[1] & (~valOne.values[1]);
		return result;
	}

	inline avx_type xorSi128(const avx_type& valOne, const avx_type& valTwo) {
		avx_type value{};
		value.values[0] = valOne.values[0] ^ valTwo.values[0];
		value.values[1] = valOne.values[1] ^ valTwo.values[1];
		return value;
	}

	inline avx_type subEpi8(const avx_type& valOne, const avx_type& valTwo) {
		avx_type result{};
		result.values[0] = valOne.values[0] - valTwo.values[0];
		result.values[1] = valOne.values[1] - valTwo.values[1];
		return result;
	}

	inline avx_type notSi128(const avx_type& valOne) {
		avx_type result{};
		result.values[0] = ~valOne.values[0];
		result.values[1] = ~valOne.values[1];
		return result;
	}

	inline avx_type cmpeqEpi8(const avx_type& a, const avx_type& b) {
		avx_type result{};
		for (int32_t i = 0; i < 2; ++i) {
			for (int32_t j = 0; j < 8; ++j) {
				uint8_t a8 = (a.values[i] >> (j * 8)) & 0xff;
				uint8_t b8 = (b.values[i] >> (j * 8)) & 0xff;
				result.values[i] |= (a8 == b8 ? 0xFFULL : 0) << (j * 8);
			}
		}
		return result;
	}

	inline bool testzSi128(const avx_type& valOne, const avx_type& valTwo) {
		avx_type result{};
		result.values[0] = valOne.values[0] & valTwo.values[0];
		result.values[1] = valOne.values[1] & valTwo.values[1];
		return result.values[0] == 0 && result.values[1] == 0;
	}

	inline avx_type setEpi64x(uint64_t argOne, uint64_t argTwo) {
		avx_type returnValue{};
		returnValue.values[0] = argTwo;
		returnValue.values[1] = argOne;
		return returnValue;
	}

	inline avx_type set1Epi64x(uint64_t argOne) {
		avx_type returnValue{};
		returnValue.values[0] = argOne;
		returnValue.values[1] = argOne;
		return returnValue;
	}

	inline avx_type& insertUint16(avx_type& value, uint64_t index, uint16_t newValue) {
		if (index >= 8) {
			return value;
		}
		uint64_t arrayIndex	 = (index < 4ULL) ? 0ULL : 1ULL;
		uint64_t shiftAmount = (index % 4ULL) * 16ULL;
		value.values[arrayIndex] &= ~(static_cast<uint64_t>(0xffff) << shiftAmount);
		value.values[arrayIndex] |= (static_cast<uint64_t>(newValue) << shiftAmount);
		return value;
	}

	inline avx_type& insertUint64(avx_type& value, uint64_t index, uint64_t newValue) {
		if (index < 2) {
			value.values[index] = newValue;
			return value;
		}
		return value;
	}

	inline uint64_t extractUint64(const avx_type& value, uint64_t index) {
		if (index < 2) {
			return value.values[index];
		}
		return 0;
	}

	inline avx_type shuffleEpi8(const avx_type& a, const avx_type& b) {
		avx_type dst{};
		for (int32_t j = 0; j < 16; j++) {
			int32_t i		 = j * 8;
			int32_t bitIndex = i + 7;

			if ((b.values[bitIndex >> 6] & (1ULL << (bitIndex & 0x3F))) == (1ULL << (bitIndex & 0x3F))) {
				dst.values[bitIndex >> 6] &= ~(1ULL << (bitIndex & 0x3F));
			} else {
				int32_t index = 0;
				for (int32_t m = i + 3; m >= i; m--) {
					index = (index << 1ull) | (static_cast<int32_t>(b.values[m >> 6] >> (m & 0x3F)) & 1);
				}

				int32_t aIndex = index * 8;
				for (int32_t n = i; n <= i + 7; n++) {
					dst.values[n >> 6] |= ((a.values[aIndex >> 6] >> (aIndex & 0x3F)) & 1ULL) << (n & 0x3F);
					aIndex++;
				}
			}
		}

		return dst;
	}

	inline avx_type set1Epi8(uint8_t newValue) {
		avx_type returnValue{};
		packUint8Array(newValue, returnValue);
		return returnValue;
	}

	template<> struct alignas(16) simd_base<128> {
	  public:
		inline simd_base() noexcept = default;

		inline simd_base& operator=(avx_type&& data) {
			value = std::forward<avx_type>(data);
			return *this;
		}

		inline simd_base(avx_type&& data) {
			*this = std::forward<avx_type>(data);
		}

		inline simd_base& operator=(uint8_t other) {
			value = set1Epi8(other);
			return *this;
		}

		inline explicit simd_base(uint8_t other) {
			*this = other;
		}

		inline simd_base(const uint8_t* values) {
			value = gatherValues128<const uint8_t>(values);
		}

		inline explicit operator bool() {
			return !testzSi128(value, value);
		}

		inline operator avx_type&() {
			return value;
		}

		inline simd_base operator|(simd_base&& other) noexcept {
			return orSi128(value, std::forward<avx_type>(other.value));
		}

		inline simd_base operator-(simd_base&& other) noexcept {
			return subEpi8(value, std::forward<avx_type>(other.value));
		}

		inline simd_base operator|(const simd_base& other) {
			auto valueNewer = other.value;
			return orSi128(value, valueNewer);
		}

		inline simd_base operator&(const simd_base& other) {
			auto valueNewer = other.value;
			return andSi128(value, valueNewer);
		}

		inline simd_base operator-(const simd_base& other) {
			auto valueNewer = other.value;
			return subEpi8(value, valueNewer);
		}

		inline simd_base operator^(const simd_base& other) {
			auto valueNewer = other.value;
			return xorSi128(value, valueNewer);
		}

		inline string_parsing_type operator==(uint8_t other) const {
			simd_base newValue = cmpeqEpi8(value, set1Epi8(other));
			return newValue.toBitMask();
		}

		inline string_parsing_type operator==(const simd_base& other) const {
			simd_base newValue = cmpeqEpi8(value, other.value);
			return newValue.toBitMask();
		}

		inline simd_base operator~() {
			return xorSi128(this->value, set1Epi64x(std::numeric_limits<uint64_t>::max()));
		}

		inline void convertWhitespaceToSimdBase(simd_base* valuesNew) {
			alignas(ALIGNMENT) constexpr uint8_t arrayNew[16]{ ' ', 100, 100, 100, 17, 100, 113, 2, 100, '\t', '\n', 112, 100, '\r', 100, 100 };
			simd_base whitespaceTable{ arrayNew };
			for (uint64_t x = 0; x < 8; ++x) {
				addValues(valuesNew[x].shuffle(whitespaceTable) == valuesNew[x], x);
			}
		};

		inline void convertBackslashesToSimdBase(simd_base* valuesNew) {
			simd_base backslashes{ set1Epi8('\\') };
			for (uint64_t x = 0; x < 8; ++x) {
				addValues(valuesNew[x] == backslashes, x);
			}
		};

		inline void convertStructuralsToSimdBase(simd_base* valuesNew) {
			alignas(ALIGNMENT) constexpr uint8_t arrayNew[16]{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, ':', '{', ',', '}', 0, 0 };
			simd_base opTable{ arrayNew };
			simd_base chars{ uint8_t{ 0x20 } };
			for (uint64_t x = 0; x < 8; ++x) {
				addValues(valuesNew[x].shuffle(opTable) == (valuesNew[x] | chars), x);
			}
		};

		inline void convertQuotesToSimdBase(simd_base* valuesNew) {
			simd_base quotes{ set1Epi8('"') };
			for (uint64_t x = 0; x < 8; ++x) {
				addValues(valuesNew[x] == quotes, x);
			}
		}

		inline uint64_t getUint64(uint64_t index) const {
			return extractUint64(value, index);
		}

		inline void insertUint64(uint64_t valueNewer, uint64_t index) {
			value = jsonifier_internal::insertUint64(value, index, valueNewer);
		}

		inline void insertUint16(uint16_t valueNewer, uint64_t index) {
			value = jsonifier_internal::insertUint16(value, index, valueNewer);
		}

		inline simd_base bitAndNot(const simd_base& other) {
			return andNotSi128(other.value, value);
		}

		inline simd_base shuffle(const simd_base& other) {
			return shuffleEpi8(other.value, value);
		}

		inline void addValues(uint16_t values, uint64_t index) {
			insertUint16(values, index);
		}

		template<uint64_t amount> inline simd_base shl() const {
			simd_base currentValues{};
			currentValues.insertUint64((getUint64(0) << amount), 0);
			uint64_t shiftBetween = amount % 64;
			currentValues.insertUint64(((getUint64(1) << amount) | (getUint64(0) >> (64 - shiftBetween))), 1);
			return currentValues;
		}

		inline uint16_t toBitMask() {
			return movemaskEpi8(this->value);
		}

		inline void reset() {
			value.values[0] = 0;
			value.values[1] = 0;
		}

		template<typename value_type> inline void store(value_type* storageLocation) {
			std::memcpy(storageLocation, &value, sizeof(value));
		}

		inline simd_base& setLSB(bool valueNewer) {
			if (valueNewer) {
				*this = orSi128(*this, setEpi64x(0, 0x1));
			} else {
				*this = andNotSi128(setEpi64x(0, 0x1), *this);
			}
			return *this;
		}

		inline bool checkMSB() const {
			avx_type result = andSi128(this->value, setEpi64x(0x8000000000000000, 0));
			return !testzSi128(result, result);
		}

		inline uint64_t prefixXor(uint64_t prevInstring) const {
			prevInstring ^= prevInstring << 1;
			prevInstring ^= prevInstring << 2;
			prevInstring ^= prevInstring << 4;
			prevInstring ^= prevInstring << 8;
			prevInstring ^= prevInstring << 16;
			prevInstring ^= prevInstring << 32;
			return prevInstring;
		}

		inline simd_base carrylessMultiplication(uint64_t& prevInstring) const {
			simd_base valuesNew{};
			avx_type valueLow{ value };
			valuesNew.insertUint64(prefixXor(valueLow.values[0]) ^ prevInstring, 0);
			prevInstring = uint64_t(static_cast<int64_t>(valuesNew.getUint64(0)) >> 63);
			valuesNew.insertUint64(prefixXor(valueLow.values[1]) ^ prevInstring, 1);
			prevInstring = uint64_t(static_cast<int64_t>(valuesNew.getUint64(1)) >> 63);
			return valuesNew;
		}

		inline bool collectCarries(simd_base<128>& other1, simd_base<128>& result) {
			bool returnValue{};
			long long unsigned returnValue64{};
			for (uint64_t x = 0; x < 2; ++x) {
				if (_addcarry_u64(0, getUint64(x), other1.getUint64(x), &returnValue64)) {
					returnValue = true;
				} else {
					returnValue = false;
				}
				result.insertUint64(returnValue64, x);
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
		return set1Epi64x(value);
	}

	#define load(value) gatherValues128<std::remove_pointer_t<decltype(value)>>(value)

#endif

}