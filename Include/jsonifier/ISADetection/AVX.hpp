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
	concept simd_int_128_t = std::same_as<std::unwrap_ref_decay_t<value_type>, simd_int_128>;

	template<typename value_type>
	concept simd_float_128_t = std::same_as<std::unwrap_ref_decay_t<value_type>, simd_float_128>;

	using simd_int_t = simd_int_128;

	template<simd_int_128_t simd_type, typename char_type> jsonifier_inline simd_type gatherValues(char_type* str) {
		return _mm_load_si128(reinterpret_cast<const __m128i*>(str));
	}

	template<simd_int_128_t simd_type, typename char_type> jsonifier_inline simd_type gatherValuesU(char_type* str) {
		return _mm_loadu_si128(reinterpret_cast<const __m128i*>(str));
	}

	template<simd_float_128_t simd_type, jsonifier::concepts::float_t char_type> jsonifier_inline simd_type gatherValues(char_type* str) {
		return _mm_loadu_pd(str);
	}

	template<simd_int_128_t return_type> jsonifier_constexpr return_type simdTable(const uint8_t arrayNew03[sizeof(simd_int_t)]) {
	#if !defined(_WIN32)
		int64_t newArray[sizeof(simd_int_t) / sizeof(uint64_t)]{};
		for (uint64_t x = 0; x < sizeof(simd_int_t) / sizeof(uint64_t); ++x) {
			newArray[x] |= static_cast<int64_t>(arrayNew03[(x * 8) + 7]) << 56;
			newArray[x] |= static_cast<int64_t>(arrayNew03[(x * 8) + 6]) << 48;
			newArray[x] |= static_cast<int64_t>(arrayNew03[(x * 8) + 5]) << 40;
			newArray[x] |= static_cast<int64_t>(arrayNew03[(x * 8) + 4]) << 32;
			newArray[x] |= static_cast<int64_t>(arrayNew03[(x * 8) + 3]) << 24;
			newArray[x] |= static_cast<int64_t>(arrayNew03[(x * 8) + 2]) << 16;
			newArray[x] |= static_cast<int64_t>(arrayNew03[(x * 8) + 1]) << 8;
			newArray[x] |= static_cast<int64_t>(arrayNew03[(x * 8) + 0]);
		}
		simd_int_t returnValue{ newArray[0], newArray[1] };
	#else
		simd_int_t returnValue{};
		for (uint64_t x = 0; x < sizeof(simd_int_t); ++x) {
			returnValue.m128i_u8[x] = arrayNew03[x];
		}
	#endif
		return returnValue;
	}

	template<simd_int_128_t return_type> jsonifier_constexpr return_type simdValues(uint8_t value) {
	#if !defined(_WIN32)
		int64_t newArray[sizeof(simd_int_128) / sizeof(uint64_t)]{};
		for (uint64_t x = 0; x < sizeof(simd_int_128) / sizeof(uint64_t); ++x) {
			newArray[x] |= static_cast<int64_t>(value) << 56;
			newArray[x] |= static_cast<int64_t>(value) << 48;
			newArray[x] |= static_cast<int64_t>(value) << 40;
			newArray[x] |= static_cast<int64_t>(value) << 32;
			newArray[x] |= static_cast<int64_t>(value) << 24;
			newArray[x] |= static_cast<int64_t>(value) << 16;
			newArray[x] |= static_cast<int64_t>(value) << 8;
			newArray[x] |= static_cast<int64_t>(value);
		}
		simd_int_128 returnValue{ newArray[0], newArray[1] };
	#else
		simd_int_128 returnValue{};
		for (uint64_t x = 0; x < sizeof(simd_int_128); ++x) {
			returnValue.m128i_u8[x] = value;
		}
	#endif
		return returnValue;
	}

	template<> class simd_base_internal<128> {
	  public:
		static jsonifier_constexpr simd_int_t backslashesVal{ simdValues<simd_int_t>(0x5Cu) };
		static jsonifier_constexpr simd_int_t quotesVal{ simdValues<simd_int_t>(0x22u) };
		static jsonifier_constexpr uint8_t arrayNew00[16]{ 0x00u, 0x00u, 0x22u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x08u, 0x09u, 0x00u, 0x00u, 0x0Cu, 0x0Du, 0x00u, 0x00u };
		static jsonifier_constexpr uint8_t arrayNew01[16]{ 0x00u, 0x00u, 0x22u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x08u, 0x09u, 0x00u, 0x00u, 0x5Cu, 0x0Du, 0x00u, 0x00u };
		static jsonifier_constexpr uint8_t arrayNew02[16]{ 0x20u, 0x64u, 0x64u, 0x64u, 0x11u, 0x64u, 0x71u, 0x02u, 0x64u, 0x09u, 0x0Au, 0x70u, 0x64u, 0x0Du, 0x64u, 0x64u };
		static jsonifier_constexpr uint8_t arrayNew03[16]{ 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x3Au, 0x7Bu, 0x2Cu, 0x7Du, 0x00u, 0x00u };

		jsonifier_inline simd_base_internal() = default;

		jsonifier_inline static simd_int_t opOr(const simd_int_t& other, const simd_int_t& value) {
			return _mm_or_si128(value, other);
		}

		jsonifier_inline static simd_int_t opSub(const simd_int_t& other, const simd_int_t& value) {
			return _mm_sub_epi8(other, value);
		}

		jsonifier_inline static simd_int_t opAnd(const simd_int_t& other, const simd_int_t& value) {
			return _mm_and_si128(value, other);
		}

		jsonifier_inline static simd_int_t opXor(const simd_int_t& other, const simd_int_t& value) {
			return _mm_xor_si128(value, other);
		}

		jsonifier_inline static string_parsing_type cmpeq(const simd_int_t& other, const simd_int_t& value) {
			return static_cast<string_parsing_type>(_mm_movemask_epi8(_mm_cmpeq_epi8(value, other)));
		}

		jsonifier_inline static simd_int_t opNot(const simd_int_t& value) {
			return _mm_xor_si128(value, _mm_set1_epi64x(static_cast<int64_t>(std::numeric_limits<uint64_t>::max())));
		}

		jsonifier_inline static void convertEscapeablesToSimdBase(string_parsing_type& value, simd_int_t valuesNew) {
			static jsonifier_constexpr simd_int_t escapeTable01{ simdTable<simd_int_t>(arrayNew00) };
			static jsonifier_constexpr simd_int_t escapeTable02{ simdTable<simd_int_t>(arrayNew01) };
			value |= cmpeq(shuffle(valuesNew, escapeTable01), valuesNew);
			value |= cmpeq(shuffle(valuesNew, escapeTable02), valuesNew);
		}

		jsonifier_inline static void convertWhitespaceToSimdBase(simd_int_t& value, simd_int_t valuesNew[StridesPerStep]) {
			static jsonifier_constexpr simd_int_t whitespaceTable{ simdTable<simd_int_t>(arrayNew02) };
			insertUint16<0>(value, cmpeq(shuffle(valuesNew[0], whitespaceTable), valuesNew[0]));
			insertUint16<1>(value, cmpeq(shuffle(valuesNew[1], whitespaceTable), valuesNew[1]));
			insertUint16<2>(value, cmpeq(shuffle(valuesNew[2], whitespaceTable), valuesNew[2]));
			insertUint16<3>(value, cmpeq(shuffle(valuesNew[3], whitespaceTable), valuesNew[3]));
			insertUint16<4>(value, cmpeq(shuffle(valuesNew[4], whitespaceTable), valuesNew[4]));
			insertUint16<5>(value, cmpeq(shuffle(valuesNew[5], whitespaceTable), valuesNew[5]));
			insertUint16<6>(value, cmpeq(shuffle(valuesNew[6], whitespaceTable), valuesNew[6]));
			insertUint16<7>(value, cmpeq(shuffle(valuesNew[7], whitespaceTable), valuesNew[7]));
		}

		jsonifier_inline static void convertBackslashesToSimdBase(simd_int_t& value, simd_int_t valuesNew[StridesPerStep]) {
			insertUint16<0>(value, cmpeq(valuesNew[0], backslashesVal));
			insertUint16<1>(value, cmpeq(valuesNew[1], backslashesVal));
			insertUint16<2>(value, cmpeq(valuesNew[2], backslashesVal));
			insertUint16<3>(value, cmpeq(valuesNew[3], backslashesVal));
			insertUint16<4>(value, cmpeq(valuesNew[4], backslashesVal));
			insertUint16<5>(value, cmpeq(valuesNew[5], backslashesVal));
			insertUint16<6>(value, cmpeq(valuesNew[6], backslashesVal));
			insertUint16<7>(value, cmpeq(valuesNew[7], backslashesVal));
		}

		jsonifier_inline static void convertStructuralsToSimdBase(simd_int_t& value, simd_int_t valuesNew[StridesPerStep]) {
			static jsonifier_constexpr simd_int_t opTableVal{ simdTable<simd_int_t>(arrayNew03) };
			static jsonifier_constexpr simd_int_t chars{ simdValues<simd_int_t>(0x20u) };
			insertUint16<0>(value, cmpeq(shuffle(valuesNew[0], opTableVal), (opOr(valuesNew[0], chars))));
			insertUint16<1>(value, cmpeq(shuffle(valuesNew[1], opTableVal), (opOr(valuesNew[1], chars))));
			insertUint16<2>(value, cmpeq(shuffle(valuesNew[2], opTableVal), (opOr(valuesNew[2], chars))));
			insertUint16<3>(value, cmpeq(shuffle(valuesNew[3], opTableVal), (opOr(valuesNew[3], chars))));
			insertUint16<4>(value, cmpeq(shuffle(valuesNew[4], opTableVal), (opOr(valuesNew[4], chars))));
			insertUint16<5>(value, cmpeq(shuffle(valuesNew[5], opTableVal), (opOr(valuesNew[5], chars))));
			insertUint16<6>(value, cmpeq(shuffle(valuesNew[6], opTableVal), (opOr(valuesNew[6], chars))));
			insertUint16<7>(value, cmpeq(shuffle(valuesNew[7], opTableVal), (opOr(valuesNew[7], chars))));
		}

		jsonifier_inline static void convertQuotesToSimdBase(simd_int_t& value, simd_int_t valuesNew[StridesPerStep]) {
			insertUint16<0>(value, cmpeq(valuesNew[0], quotesVal));
			insertUint16<1>(value, cmpeq(valuesNew[1], quotesVal));
			insertUint16<2>(value, cmpeq(valuesNew[2], quotesVal));
			insertUint16<3>(value, cmpeq(valuesNew[3], quotesVal));
			insertUint16<4>(value, cmpeq(valuesNew[4], quotesVal));
			insertUint16<5>(value, cmpeq(valuesNew[5], quotesVal));
			insertUint16<6>(value, cmpeq(valuesNew[6], quotesVal));
			insertUint16<7>(value, cmpeq(valuesNew[7], quotesVal));
		}

		jsonifier_inline static bool opBool(const simd_int_t& value) {
			return !_mm_testz_si128(value, value);
		}

		template<uint64_t index = 0> jsonifier_inline static void insertUint64(simd_int_t& value, uint64_t valueNew) {
			static_assert(index < SixtyFourBitsPerStep, "Sorry, but that index value is incorrect.");
			value = _mm_insert_epi64(value, static_cast<int64_t>(valueNew), index);
		}

		template<uint64_t index = 0> jsonifier_inline static uint64_t getUint64(const simd_int_t& value) {
			static_assert(index < SixtyFourBitsPerStep, "Sorry, but that index value is incorrect.");
			return static_cast<uint64_t>(_mm_extract_epi64(value, index));
		}

		template<uint64_t index> jsonifier_inline static void insertUint16(simd_int_t& value, string_parsing_type valueNew) {
			static_assert(index < StridesPerStep, "Sorry, but that index value is incorrect.");
			value = _mm_insert_epi16(value, static_cast<int32_t>(valueNew), index);
		}

		jsonifier_inline static simd_int_t bitAndNot(const simd_int_t& value, const simd_int_t& other) {
			return _mm_andnot_si128(other, value);
		}

		jsonifier_inline static simd_int_t shuffle(const simd_int_t& value, const simd_int_t& other) {
			return _mm_shuffle_epi8(other, value);
		}

		template<uint64_t amount> jsonifier_inline static simd_int_t shl(const simd_int_t& value) {
			simd_int_t currentValues{};
			insertUint64<0>(currentValues, getUint64<0>(value) << amount);
			static constexpr uint64_t shiftBetween = amount % 64;
			insertUint64<1>(currentValues, (getUint64<1>(value) << amount) | (getUint64<0>(value) >> (64 - shiftBetween)));
			return currentValues;
		}

		template<typename value_type> jsonifier_inline static void storeu(const simd_int_t& value, value_type* storageLocation) {
			_mm_storeu_si128(reinterpret_cast<__m128i*>(storageLocation), value);
		}

		template<typename value_type> jsonifier_inline static void store(const simd_int_t& value, value_type* storageLocation) {
			_mm_store_si128(reinterpret_cast<__m128i*>(storageLocation), value);
		}

		jsonifier_inline static simd_int_t setLSB(const simd_int_t& value, bool valueNew) {
			if (valueNew) {
				return _mm_or_si128(value, _mm_set_epi64x(0x00, 0x1));
			} else {
				return _mm_andnot_si128(_mm_set_epi64x(0x00, 0x1), value);
			}
		}

		jsonifier_inline static bool getMSB(const simd_int_t& value) {
			simd_int_t result = _mm_and_si128(value, _mm_set_epi64x(0x8000000000000000, 0x00));
			return !_mm_testz_si128(result, result);
		}

		template<uint64_t index> jsonifier_inline static void processValue(const simd_int_128& allOnes, const simd_int_128& val, uint64_t& valuesNewer, uint64_t& prevInstring) {
			valuesNewer	 = static_cast<uint64_t>(_mm_cvtsi128_si64(_mm_clmulepi64_si128(val, allOnes, index % 2)) ^ prevInstring);
			prevInstring = static_cast<uint64_t>(static_cast<int64_t>(valuesNewer) >> 63);
		}

		jsonifier_inline static simd_int_t carrylessMultiplication(const simd_int_t& value, uint64_t& prevInstring) {
			static jsonifier_constexpr simd_int_128 allOnes{ simdValues<simd_int_128>(0xFFu) };
			alignas(BytesPerStep) uint64_t valuesNewer[SixtyFourBitsPerStep]{};
			processValue<0>(allOnes, value, valuesNewer[0], prevInstring);
			processValue<1>(allOnes, value, valuesNewer[1], prevInstring);
			return gatherValues<simd_int_t>(valuesNewer);
		}

		jsonifier_inline static simd_int_t follows(const simd_int_t& value, bool& overflow) {
			simd_int_t result = shl<1>(value);
			result			  = setLSB(result, overflow);
			overflow		  = getMSB(value);
			return result;
		}

		jsonifier_inline static void printBits(uint64_t values, const std::string& valuesTitle) {
			std::cout << valuesTitle;
			std::cout << std::bitset<64>{ values };
			std::cout << std::endl;
		}

		jsonifier_inline static const simd_int_t& printBits(const simd_int_t& value, const std::string& valuesTitle) noexcept {
			uint8_t values[BytesPerStep]{};
			storeu(value, values);
			std::cout << valuesTitle;
			for (string_parsing_type x = 0; x < BytesPerStep; ++x) {
				for (string_parsing_type y = 0; y < 8; ++y) {
					std::cout << std::bitset<1>{ static_cast<uint64_t>(*(values + x)) >> y };
				}
			}
			std::cout << std::endl;
			return value;
		}
	};


#endif

}