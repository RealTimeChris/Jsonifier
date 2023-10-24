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
	concept simd_int_128_t = std::same_as<std::unwrap_ref_decay_t<value_type>, simd_int_128>;

	template<typename value_type>
	concept simd_int_256_t = std::same_as<std::unwrap_ref_decay_t<value_type>, simd_int_256>;

	template<typename value_type>
	concept simd_int_512_t = std::same_as<std::unwrap_ref_decay_t<value_type>, simd_int_512>;

	template<typename value_type>
	concept simd_float_128_t = std::same_as<std::unwrap_ref_decay_t<value_type>, simd_float_128>;

	template<typename value_type>
	concept simd_float_256_t = std::same_as<std::unwrap_ref_decay_t<value_type>, simd_float_256>;

	template<typename value_type>
	concept simd_float_512_t = std::same_as<std::unwrap_ref_decay_t<value_type>, simd_float_512>;

	using simd_int_t = simd_int_512;

	template<simd_int_128_t simd_type, typename char_type> jsonifier_inline simd_type gatherValues(char_type* str) {
		return _mm_load_si128(reinterpret_cast<const __m128i*>(str));
	}

	template<simd_int_256_t simd_type, typename char_type> jsonifier_inline simd_type gatherValues(char_type* str) {
		return _mm256_load_si256(reinterpret_cast<const __m256i*>(str));
	}

	template<simd_int_512_t simd_type, typename char_type> jsonifier_inline simd_type gatherValues(char_type* str) {
		return _mm512_load_si512(reinterpret_cast<const __m512i*>(str));
	}

	template<simd_int_128_t simd_type, typename char_type> jsonifier_inline simd_type gatherValuesU(char_type* str) {
		return _mm_loadu_si128(reinterpret_cast<const __m128i*>(str));
	}

	template<simd_int_256_t simd_type, typename char_type> jsonifier_inline simd_type gatherValuesU(char_type* str) {
		return _mm256_loadu_si256(reinterpret_cast<const __m256i*>(str));
	}

	template<simd_int_512_t simd_type, typename char_type> jsonifier_inline simd_type gatherValuesU(char_type* str) {
		return _mm512_loadu_si512(reinterpret_cast<const __m512i*>(str));
	}

	template<simd_float_128_t simd_type, jsonifier::concepts::float_t char_type> jsonifier_inline simd_type gatherValues(char_type* str) {
		return _mm_load_pd(str);
	}

	template<simd_float_256_t simd_type, jsonifier::concepts::float_t char_type> jsonifier_inline simd_type gatherValues(char_type* str) {
		return _mm256_load_pd(str);
	}

	template<simd_float_512_t simd_type, jsonifier::concepts::float_t char_type> jsonifier_inline simd_type gatherValues(char_type* str) {
		return _mm512_load_pd(str);
	}

	template<simd_float_128_t simd_type, jsonifier::concepts::float_t char_type> jsonifier_inline simd_type gatherValuesU(char_type* str) {
		return _mm_loadu_pd(str);
	}

	template<simd_float_256_t simd_type, jsonifier::concepts::float_t char_type> jsonifier_inline simd_type gatherValuesU(char_type* str) {
		return _mm256_loadu_pd(str);
	}

	template<simd_float_512_t simd_type, jsonifier::concepts::float_t char_type> jsonifier_inline simd_type gatherValuesU(char_type* str) {
		return _mm512_loadu_pd(str);
	}

	template<simd_int_512_t return_type> jsonifier_constexpr return_type simdFromTable(const uint8_t arrayNew[sizeof(simd_int_t)]) {
	#if !defined(_WIN32)
		int64_t newArray[sizeof(simd_int_t) / sizeof(uint64_t)]{};
		for (uint64_t x = 0; x < sizeof(simd_int_t) / sizeof(uint64_t); ++x) {
			newArray[x] |= static_cast<int64_t>(arrayNew[(x * 8) + 7]) << 56;
			newArray[x] |= static_cast<int64_t>(arrayNew[(x * 8) + 6]) << 48;
			newArray[x] |= static_cast<int64_t>(arrayNew[(x * 8) + 5]) << 40;
			newArray[x] |= static_cast<int64_t>(arrayNew[(x * 8) + 4]) << 32;
			newArray[x] |= static_cast<int64_t>(arrayNew[(x * 8) + 3]) << 24;
			newArray[x] |= static_cast<int64_t>(arrayNew[(x * 8) + 2]) << 16;
			newArray[x] |= static_cast<int64_t>(arrayNew[(x * 8) + 1]) << 8;
			newArray[x] |= static_cast<int64_t>(arrayNew[(x * 8) + 0]);
		}
		simd_int_t returnValue{ newArray[0], newArray[1], newArray[2], newArray[3], newArray[4], newArray[5], newArray[6], newArray[7] };
	#else
		simd_int_t returnValue{};
		for (uint64_t x = 0; x < sizeof(simd_int_t); ++x) {
			returnValue.m512i_u8[x] = arrayNew[x];
		}
	#endif
		return returnValue;
	}

	template<simd_int_128_t return_type> jsonifier_constexpr return_type simdFromValue(uint8_t value) {
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

	template<simd_int_512_t return_type> jsonifier_constexpr return_type simdFromValue(uint8_t value) {
	#if !defined(_WIN32)
		int64_t newArray[sizeof(simd_int_512) / sizeof(uint64_t)]{};
		for (uint64_t x = 0; x < sizeof(simd_int_512) / sizeof(uint64_t); ++x) {
			newArray[x] |= static_cast<int64_t>(value) << 56;
			newArray[x] |= static_cast<int64_t>(value) << 48;
			newArray[x] |= static_cast<int64_t>(value) << 40;
			newArray[x] |= static_cast<int64_t>(value) << 32;
			newArray[x] |= static_cast<int64_t>(value) << 24;
			newArray[x] |= static_cast<int64_t>(value) << 16;
			newArray[x] |= static_cast<int64_t>(value) << 8;
			newArray[x] |= static_cast<int64_t>(value);
		}
		simd_int_512 returnValue{ newArray[0], newArray[1], newArray[2], newArray[3], newArray[4], newArray[5], newArray[6], newArray[7] };
	#else
		simd_int_512 returnValue{};
		for (uint64_t x = 0; x < sizeof(simd_int_512); ++x) {
			returnValue.m512i_u8[x] = value;
		}
	#endif
		return returnValue;
	}

	template<> class simd_base_internal<512> {
	  public:
		static jsonifier_constexpr uint8_t arrayNew03[]{ 0x00u, 0x00u, 0x22u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x08u, 0x09u, 0x00u, 0x00u, 0x0Cu, 0x0Du, 0x00u, 0x00u, 0x00u,
			0x00u, 0x22u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x08u, 0x09u, 0x00u, 0x00u, 0x0Cu, 0x0Du, 0x00u, 0x00u, 0x00u, 0x00u, 0x22u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x08u,
			0x09u, 0x00u, 0x00u, 0x0Cu, 0x0Du, 0x00u, 0x00u, 0x00u, 0x00u, 0x22u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x08u, 0x09u, 0x00u, 0x00u, 0x0Cu, 0x0Du, 0x00u, 0x00u };
		static jsonifier_constexpr uint8_t arrayNew00[]{ 0x00u, 0x00u, 0x22u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x08u, 0x09u, 0x00u, 0x00u, 0x5Cu, 0x0Du, 0x00u, 0x00u, 0x00u,
			0x00u, 0x22u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x08u, 0x09u, 0x00u, 0x00u, 0x5Cu, 0x0Du, 0x00u, 0x00u, 0x00u, 0x00u, 0x22u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x08u,
			0x09u, 0x00u, 0x00u, 0x5Cu, 0x0Du, 0x00u, 0x00u, 0x00u, 0x00u, 0x22u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x08u, 0x09u, 0x00u, 0x00u, 0x5Cu, 0x0Du, 0x00u, 0x00u };
		static jsonifier_constexpr simd_int_t escapeTable01{ simdFromTable<simd_int_t>(arrayNew03) };
		static jsonifier_constexpr simd_int_t escapeTable02{ simdFromTable<simd_int_t>(arrayNew00) };
		static jsonifier_constexpr uint8_t arrayNew01[]{ 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x3Au, 0x7Bu, 0x2Cu, 0x7Du, 0x00u, 0x00u, 0x00u,
			0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x3Au, 0x7Bu, 0x2Cu, 0x7Du, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u,
			0x00u, 0x3Au, 0x7Bu, 0x2Cu, 0x7Du, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x3Au, 0x7Bu, 0x2Cu, 0x7Du, 0x00u, 0x00u };
		static jsonifier_constexpr simd_int_t opTableVal{ simdFromTable<simd_int_t>(arrayNew01) };
		static jsonifier_constexpr simd_int_t chars{ simdFromValue<simd_int_t>(0x20u) };
		static jsonifier_constexpr uint8_t arrayNew02[]{ 0x20u, 0x64u, 0x64u, 0x64u, 0x11u, 0x64u, 0x71u, 0x02u, 0x64u, 0x09u, 0x0Au, 0x70u, 0x64u, 0x0Du, 0x64u, 0x64u, 0x20u,
			0x64u, 0x64u, 0x64u, 0x11u, 0x64u, 0x71u, 0x02u, 0x64u, 0x09u, 0x0Au, 0x70u, 0x64u, 0x0Du, 0x64u, 0x64u, 0x20u, 0x64u, 0x64u, 0x64u, 0x11u, 0x64u, 0x71u, 0x02u, 0x64u,
			0x09u, 0x0Au, 0x70u, 0x64u, 0x0Du, 0x64u, 0x64u, 0x20u, 0x64u, 0x64u, 0x64u, 0x11u, 0x64u, 0x71u, 0x02u, 0x64u, 0x09u, 0x0Au, 0x70u, 0x64u, 0x0Du, 0x64u, 0x64u };
		static jsonifier_constexpr simd_int_t whitespaceTable{ simdFromTable<simd_int_t>(arrayNew02) };
		static jsonifier_constexpr simd_int_t backslashes{ simdFromValue<simd_int_t>(0x5Cu) };
		static jsonifier_constexpr simd_int_t quotes{ simdFromValue<simd_int_t>(0x22u) };

		jsonifier_inline simd_base_internal() = default;

		jsonifier_inline static simd_int_t opOr(const simd_int_t& other, const simd_int_t& value) {
			return _mm512_or_si512(value, other);
		}

		jsonifier_inline static simd_int_t opSub(const simd_int_t& other, const simd_int_t& value) {
			return _mm512_subs_epu8(other, value);
		}

		jsonifier_inline static simd_int_t opAnd(const simd_int_t& other, const simd_int_t& value) {
			return _mm512_and_si512(value, other);
		}

		jsonifier_inline static simd_int_t opXor(const simd_int_t& other, const simd_int_t& value) {
			return _mm512_xor_si512(value, other);
		}

		jsonifier_inline static string_parsing_type cmpeq(const simd_int_t& other, const simd_int_t& value) {
			return static_cast<string_parsing_type>(_mm512_cmpeq_epi8_mask(value, other));
		}

		jsonifier_inline static simd_int_t opNot(const simd_int_t& value) {
			return _mm512_xor_si512(value, _mm512_set1_epi64(static_cast<int64_t>(std::numeric_limits<uint64_t>::max())));
		}

		jsonifier_inline static void convertEscapeablesToSimdBase(string_parsing_type& value, simd_int_t valuesNew) {
			value |= cmpeq(shuffle(valuesNew, escapeTable01), valuesNew);
			value |= cmpeq(shuffle(valuesNew, escapeTable02), valuesNew);
		}

		jsonifier_inline static bool opBool(const simd_int_t& value) {
			return _mm512_test_epi64_mask(value, value) != 0;
		}

		jsonifier_inline static simd_int_t follows(const simd_int_t& value, bool& overflow) {
			simd_int_t result = shl<1>(value);
			result			  = setLSB(result, overflow);
			overflow		  = getMSB(value);
			return result;
		}

		jsonifier_inline static void convertWhitespaceToSimdBase(simd_int_t& value, simd_int_t valuesNew[StridesPerStep]) {
			insertUint64<0>(value, cmpeq(shuffle(valuesNew[0], whitespaceTable), valuesNew[0]));
			insertUint64<1>(value, cmpeq(shuffle(valuesNew[1], whitespaceTable), valuesNew[1]));
			insertUint64<2>(value, cmpeq(shuffle(valuesNew[2], whitespaceTable), valuesNew[2]));
			insertUint64<3>(value, cmpeq(shuffle(valuesNew[3], whitespaceTable), valuesNew[3]));
			insertUint64<4>(value, cmpeq(shuffle(valuesNew[4], whitespaceTable), valuesNew[4]));
			insertUint64<5>(value, cmpeq(shuffle(valuesNew[5], whitespaceTable), valuesNew[5]));
			insertUint64<6>(value, cmpeq(shuffle(valuesNew[6], whitespaceTable), valuesNew[6]));
			insertUint64<7>(value, cmpeq(shuffle(valuesNew[7], whitespaceTable), valuesNew[7]));
		}

		jsonifier_inline static void convertBackslashesToSimdBase(simd_int_t& value, simd_int_t valuesNew[StridesPerStep]) {
			insertUint64<0>(value, cmpeq(valuesNew[0], backslashes));
			insertUint64<1>(value, cmpeq(valuesNew[1], backslashes));
			insertUint64<2>(value, cmpeq(valuesNew[2], backslashes));
			insertUint64<3>(value, cmpeq(valuesNew[3], backslashes));
			insertUint64<4>(value, cmpeq(valuesNew[4], backslashes));
			insertUint64<5>(value, cmpeq(valuesNew[5], backslashes));
			insertUint64<6>(value, cmpeq(valuesNew[6], backslashes));
			insertUint64<7>(value, cmpeq(valuesNew[7], backslashes));
		}

		jsonifier_inline static void convertStructuralsToSimdBase(simd_int_t& value, simd_int_t valuesNew[StridesPerStep]) {
			insertUint64<0>(value, cmpeq(shuffle(valuesNew[0], opTableVal), opOr(valuesNew[0], chars)));
			insertUint64<1>(value, cmpeq(shuffle(valuesNew[1], opTableVal), opOr(valuesNew[1], chars)));
			insertUint64<2>(value, cmpeq(shuffle(valuesNew[2], opTableVal), opOr(valuesNew[2], chars)));
			insertUint64<3>(value, cmpeq(shuffle(valuesNew[3], opTableVal), opOr(valuesNew[3], chars)));
			insertUint64<4>(value, cmpeq(shuffle(valuesNew[4], opTableVal), opOr(valuesNew[4], chars)));
			insertUint64<5>(value, cmpeq(shuffle(valuesNew[5], opTableVal), opOr(valuesNew[5], chars)));
			insertUint64<6>(value, cmpeq(shuffle(valuesNew[6], opTableVal), opOr(valuesNew[6], chars)));
			insertUint64<7>(value, cmpeq(shuffle(valuesNew[7], opTableVal), opOr(valuesNew[7], chars)));
		}

		jsonifier_inline static void convertQuotesToSimdBase(simd_int_t& value, simd_int_t valuesNew[StridesPerStep]) {
			insertUint64<0>(value, cmpeq(valuesNew[0], quotes));
			insertUint64<1>(value, cmpeq(valuesNew[1], quotes));
			insertUint64<2>(value, cmpeq(valuesNew[2], quotes));
			insertUint64<3>(value, cmpeq(valuesNew[3], quotes));
			insertUint64<4>(value, cmpeq(valuesNew[4], quotes));
			insertUint64<5>(value, cmpeq(valuesNew[5], quotes));
			insertUint64<6>(value, cmpeq(valuesNew[6], quotes));
			insertUint64<7>(value, cmpeq(valuesNew[7], quotes));
		}

		template<uint64_t index = 0> jsonifier_inline static void insertUint64(simd_int_t& value, uint64_t valueNew) {
			static_assert(index < SixtyFourBitsPerStep, "Sorry, but that index value is incorrect.");
			auto lower = _mm512_extracti64x4_epi64(value, index / 4);
			lower	   = _mm256_insert_epi64(lower, static_cast<int64_t>(valueNew), index % 4);
			value	   = _mm512_inserti64x4(value, lower, index / 4);
		}

		jsonifier_inline static simd_int_t bitAndNot(const simd_int_t& value, const simd_int_t& other) {
			return _mm512_andnot_si512(other, value);
		}

		jsonifier_inline static simd_int_t shuffle(const simd_int_t& value, const simd_int_t& other) {
			return _mm512_shuffle_epi8(other, value);
		}

		template<uint64_t index> jsonifier_inline static void addValues(simd_int_t& value, string_parsing_type valuesToAdd) {
			insertUint64<index>(value, valuesToAdd);
		}

		template<string_parsing_type amount> jsonifier_inline static simd_int_t shl(const simd_int_t& value) {
			simd_int_t newValue{ _mm512_slli_epi64(value, (amount % 64)) };
			simd_int_t newerValue{ _mm512_srli_epi64(_mm512_permutexvar_epi64(_mm512_set_epi64(6, 5, 4, 3, 2, 1, 0, 7), value), 64 - (amount % 64)) };
			static jsonifier_constexpr int64_t maxValue{ static_cast<int64_t>(std::numeric_limits<string_parsing_type>::max()) };
			simd_int_t newestValue{ _mm512_set_epi64(maxValue, maxValue, maxValue, maxValue, maxValue, maxValue, maxValue, 0) };
			return opOr(newValue, opAnd(newerValue, newestValue));
		}

		template<typename value_type> jsonifier_inline static void store(const simd_int_t& value, value_type* storageLocation) {
			_mm512_store_si512(storageLocation, value);
		}

		template<typename value_type> jsonifier_inline static void storeu(const simd_int_t& value, value_type* storageLocation) {
			_mm512_storeu_si512(storageLocation, value);
		}

		jsonifier_inline static simd_int_t setLSB(const simd_int_t& value, bool valueNew) {
			if (valueNew) {
				return _mm512_or_si512(value, _mm512_set_epi64(0, 0, 0, 0, 0, 0, 0, 0x01));
			} else {
				return _mm512_andnot_si512(_mm512_set_epi64(0, 0, 0, 0, 0, 0, 0, 0x01), value);
			}
		}

		jsonifier_inline static bool getMSB(const simd_int_t& value) {
			simd_int_512 result = _mm512_and_si512(value, _mm512_set_epi64(0x8000000000000000, 0, 0, 0, 0, 0, 0, 0));
			return static_cast<bool>(_mm512_test_epi64_mask(result, result));
		}

		template<uint64_t index> jsonifier_inline static void processValue(const simd_int_128& allOnes, simd_int_128& val, uint64_t& valuesNewer, uint64_t& prevInstring) {
			valuesNewer	 = static_cast<uint64_t>(_mm_cvtsi128_si64(_mm_clmulepi64_si128(val, allOnes, index % 2)) ^ prevInstring);
			prevInstring = static_cast<uint64_t>(static_cast<int64_t>(valuesNewer) >> 63);
		}

		jsonifier_inline static simd_int_256 carrylessMultiplication256(const simd_int_256& value, uint64_t& prevInstring) {
			static jsonifier_constexpr simd_int_128 allOnes{ simdFromValue<simd_int_128>(0xFFu) };
			simd_int_128 valueLow{ _mm256_extracti128_si256(value, 0) };
			simd_int_128 valueHigh{ _mm256_extracti128_si256(value, 1) };
			alignas(BytesPerStep) uint64_t valuesNewer[SixtyFourBitsPerStep / 2]{};
			processValue<0>(allOnes, valueLow, valuesNewer[0], prevInstring);
			processValue<1>(allOnes, valueLow, valuesNewer[1], prevInstring);
			processValue<2>(allOnes, valueHigh, valuesNewer[2], prevInstring);
			processValue<3>(allOnes, valueHigh, valuesNewer[3], prevInstring);
			return gatherValues<simd_int_256>(valuesNewer);
		}

		jsonifier_inline static simd_int_t carrylessMultiplication(const simd_int_t& value, string_parsing_type& prevInstring) {
			simd_int_256 lowValues{ _mm512_extracti64x4_epi64(value, 0) };
			simd_int_256 highValues{ _mm512_extracti64x4_epi64(value, 1) };
			lowValues  = carrylessMultiplication256(lowValues, prevInstring);
			highValues = carrylessMultiplication256(highValues, prevInstring);
			simd_int_t returnValue{};
			returnValue = _mm512_inserti64x4(returnValue, lowValues, 0);
			returnValue = _mm512_inserti64x4(returnValue, highValues, 1);
			return returnValue;
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