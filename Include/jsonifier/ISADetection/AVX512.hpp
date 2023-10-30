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
	concept simd_int_128_t = std::same_as<jsonifier::concepts::unwrap<value_type>, simd_int_128>;

	template<typename value_type>
	concept simd_int_256_t = std::same_as<jsonifier::concepts::unwrap<value_type>, simd_int_256>;

	template<typename value_type>
	concept simd_int_512_t = std::same_as<jsonifier::concepts::unwrap<value_type>, simd_int_512>;

	template<typename value_type>
	concept simd_float_128_t = std::same_as<jsonifier::concepts::unwrap<value_type>, simd_float_128>;

	template<typename value_type>
	concept simd_float_256_t = std::same_as<jsonifier::concepts::unwrap<value_type>, simd_float_256>;

	template<typename value_type>
	concept simd_float_512_t = std::same_as<jsonifier::concepts::unwrap<value_type>, simd_float_512>;

	using simd_int_t = simd_int_512;

	template<simd_int_128_t simd_type, typename char_type> jsonifier_inline simd_type gatherValues(char_type* str) {
		return _mm_load_si128(reinterpret_cast<const __m128i*>(str));
	}

	template<simd_int_256_t simd_type, typename char_type> jsonifier_inline simd_type gatherValues(char_type* str) {
		return _mm256_load_si256(reinterpret_cast<const __m256i*>(str));
	}

	template<simd_int_512_t simd_type, typename char_type> jsonifier_inline simd_type gatherValues(char_type* str) {
		return _mm512_load_si512(str);
	}

	template<simd_int_128_t simd_type, typename char_type> jsonifier_inline simd_type gatherValuesU(char_type* str) {
		return _mm_loadu_si128(reinterpret_cast<const __m128i*>(str));
	}

	template<simd_int_256_t simd_type, typename char_type> jsonifier_inline simd_type gatherValuesU(char_type* str) {
		return _mm256_loadu_si256(reinterpret_cast<const __m256i*>(str));
	}

	template<simd_int_512_t simd_type, typename char_type> jsonifier_inline simd_type gatherValuesU(char_type* str) {
		return _mm512_loadu_si512(str);
	}

	template<simd_float_128_t simd_type, jsonifier::concepts::float_t char_type> jsonifier_inline simd_type gatherValues(char_type* str) {
		return _mm_loadu_pd(str);
	}

	template<simd_float_256_t simd_type, jsonifier::concepts::float_t char_type> jsonifier_inline simd_type gatherValues(char_type* str) {
		return _mm256_loadu_pd(str);
	}

	template<simd_float_512_t simd_type, jsonifier::concepts::float_t char_type> jsonifier_inline simd_type gatherValues(char_type* str) {
		return _mm512_loadu_pd(str);
	}

	template<simd_int_512_t return_type> jsonifier_constexpr return_type simdFromTable(const uint8_t arrayNew01[sizeof(simd_int_t)]) {
	#if !defined(_WIN32)
		int64_t newArray[sizeof(simd_int_t) / sizeof(uint64_t)]{};
		for (uint64_t x = 0; x < sizeof(simd_int_t) / sizeof(uint64_t); ++x) {
			newArray[x] |= static_cast<int64_t>(arrayNew01[(x * 8) + 7]) << 56;
			newArray[x] |= static_cast<int64_t>(arrayNew01[(x * 8) + 6]) << 48;
			newArray[x] |= static_cast<int64_t>(arrayNew01[(x * 8) + 5]) << 40;
			newArray[x] |= static_cast<int64_t>(arrayNew01[(x * 8) + 4]) << 32;
			newArray[x] |= static_cast<int64_t>(arrayNew01[(x * 8) + 3]) << 24;
			newArray[x] |= static_cast<int64_t>(arrayNew01[(x * 8) + 2]) << 16;
			newArray[x] |= static_cast<int64_t>(arrayNew01[(x * 8) + 1]) << 8;
			newArray[x] |= static_cast<int64_t>(arrayNew01[(x * 8) + 0]);
		}
		simd_int_t returnValue{ newArray[0], newArray[1], newArray[2], newArray[3], newArray[4], newArray[5], newArray[6], newArray[7] };
	#else
		simd_int_t returnValue{};
		for (uint64_t x = 0; x < sizeof(simd_int_t); ++x) {
			returnValue.m512i_u8[x] = arrayNew01[x];
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
		int64_t newArray[sizeof(simd_int_t) / sizeof(uint64_t)]{};
		for (uint64_t x = 0; x < sizeof(simd_int_t) / sizeof(uint64_t); ++x) {
			newArray[x] |= static_cast<int64_t>(value) << 56;
			newArray[x] |= static_cast<int64_t>(value) << 48;
			newArray[x] |= static_cast<int64_t>(value) << 40;
			newArray[x] |= static_cast<int64_t>(value) << 32;
			newArray[x] |= static_cast<int64_t>(value) << 24;
			newArray[x] |= static_cast<int64_t>(value) << 16;
			newArray[x] |= static_cast<int64_t>(value) << 8;
			newArray[x] |= static_cast<int64_t>(value);
		}
		simd_int_t returnValue{ newArray[0], newArray[1], newArray[2], newArray[3], newArray[4], newArray[5], newArray[6], newArray[7] };
	#else
		simd_int_t returnValue{};
		for (uint64_t x = 0; x < sizeof(simd_int_t); ++x) {
			returnValue.m512i_u8[x] = value;
		}
	#endif
		return returnValue;
	}

	template<> class simd_base_internal<512> {
	  public:
		static jsonifier_constexpr simd_int_t backslashes{ simdFromValue<simd_int_t>(0x5Cu) };
		static jsonifier_constexpr simd_int_t quotes{ simdFromValue<simd_int_t>(0x22u) };
		static jsonifier_constexpr uint8_t arrayNew00[]{ 0x20u, 0x64u, 0x64u, 0x64u, 0x11u, 0x64u, 0x71u, 0x02u, 0x64u, 0x09u, 0x0Au, 0x70u, 0x64u, 0x0Du, 0x64u, 0x64u, 0x20u,
			0x64u, 0x64u, 0x64u, 0x11u, 0x64u, 0x71u, 0x02u, 0x64u, 0x09u, 0x0Au, 0x70u, 0x64u, 0x0Du, 0x64u, 0x64u, 0x20u, 0x64u, 0x64u, 0x64u, 0x11u, 0x64u, 0x71u, 0x02u, 0x64u,
			0x09u, 0x0Au, 0x70u, 0x64u, 0x0Du, 0x64u, 0x64u, 0x20u, 0x64u, 0x64u, 0x64u, 0x11u, 0x64u, 0x71u, 0x02u, 0x64u, 0x09u, 0x0Au, 0x70u, 0x64u, 0x0Du, 0x64u, 0x64u };
		static jsonifier_constexpr uint8_t arrayNew01[]{ 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x3Au, 0x7Bu, 0x2Cu, 0x7Du, 0x00u, 0x00u, 0x00u,
			0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x3Au, 0x7Bu, 0x2Cu, 0x7Du, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u,
			0x00u, 0x3Au, 0x7Bu, 0x2Cu, 0x7Du, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x3Au, 0x7Bu, 0x2Cu, 0x7Du, 0x00u, 0x00u };

		jsonifier_inline simd_base_internal() = default;

		jsonifier_inline static simd_int_t opOr(const simd_int_t& other, const simd_int_t& value) {
			return _mm512_or_si512(value, other);
		}

		jsonifier_inline static simd_int_t opSub(const simd_int_t& other, const simd_int_t& value) {
			return _mm512_sub_epi64(other, value);
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

		jsonifier_inline static void collectWhitespaceAsSimdBase(simd_int_t& value, simd_int_t valuesNew[StridesPerStep]) {
			static jsonifier_constexpr simd_int_t whitespaceTable{ simdFromTable<simd_int_t>(arrayNew00) };
			insertUint64<0>(value, cmpeq(shuffle(whitespaceTable, valuesNew[0]), valuesNew[0]));
			insertUint64<1>(value, cmpeq(shuffle(whitespaceTable, valuesNew[1]), valuesNew[1]));
			insertUint64<2>(value, cmpeq(shuffle(whitespaceTable, valuesNew[2]), valuesNew[2]));
			insertUint64<3>(value, cmpeq(shuffle(whitespaceTable, valuesNew[3]), valuesNew[3]));
			insertUint64<4>(value, cmpeq(shuffle(whitespaceTable, valuesNew[4]), valuesNew[4]));
			insertUint64<5>(value, cmpeq(shuffle(whitespaceTable, valuesNew[5]), valuesNew[5]));
			insertUint64<6>(value, cmpeq(shuffle(whitespaceTable, valuesNew[6]), valuesNew[6]));
			insertUint64<7>(value, cmpeq(shuffle(whitespaceTable, valuesNew[7]), valuesNew[7]));
		}

		jsonifier_inline static void collectBackslashesAsSimdBase(simd_int_t& value, simd_int_t valuesNew[StridesPerStep]) {
			insertUint64<0>(value, cmpeq(valuesNew[0], backslashes));
			insertUint64<1>(value, cmpeq(valuesNew[1], backslashes));
			insertUint64<2>(value, cmpeq(valuesNew[2], backslashes));
			insertUint64<3>(value, cmpeq(valuesNew[3], backslashes));
			insertUint64<4>(value, cmpeq(valuesNew[4], backslashes));
			insertUint64<5>(value, cmpeq(valuesNew[5], backslashes));
			insertUint64<6>(value, cmpeq(valuesNew[6], backslashes));
			insertUint64<7>(value, cmpeq(valuesNew[7], backslashes));
		}

		jsonifier_inline static void collectStructuralsAsSimdBase(simd_int_t& value, simd_int_t valuesNew[StridesPerStep]) {
			static jsonifier_constexpr simd_int_t opTableVal{ simdFromTable<simd_int_t>(arrayNew01) };
			static jsonifier_constexpr simd_int_t chars{ simdFromValue<simd_int_t>(0x20u) };
			insertUint64<0>(value, cmpeq(shuffle(opTableVal, valuesNew[0]), (opOr(valuesNew[0], chars))));
			insertUint64<1>(value, cmpeq(shuffle(opTableVal, valuesNew[1]), (opOr(valuesNew[1], chars))));
			insertUint64<2>(value, cmpeq(shuffle(opTableVal, valuesNew[2]), (opOr(valuesNew[2], chars))));
			insertUint64<3>(value, cmpeq(shuffle(opTableVal, valuesNew[3]), (opOr(valuesNew[3], chars))));
			insertUint64<4>(value, cmpeq(shuffle(opTableVal, valuesNew[4]), (opOr(valuesNew[4], chars))));
			insertUint64<5>(value, cmpeq(shuffle(opTableVal, valuesNew[5]), (opOr(valuesNew[5], chars))));
			insertUint64<6>(value, cmpeq(shuffle(opTableVal, valuesNew[6]), (opOr(valuesNew[6], chars))));
			insertUint64<7>(value, cmpeq(shuffle(opTableVal, valuesNew[7]), (opOr(valuesNew[7], chars))));
		}

		jsonifier_inline static void collectQuotesAsSimdBase(simd_int_t& value, simd_int_t valuesNew[StridesPerStep]) {
			insertUint64<0>(value, cmpeq(valuesNew[0], quotes));
			insertUint64<1>(value, cmpeq(valuesNew[1], quotes));
			insertUint64<2>(value, cmpeq(valuesNew[2], quotes));
			insertUint64<3>(value, cmpeq(valuesNew[3], quotes));
			insertUint64<4>(value, cmpeq(valuesNew[4], quotes));
			insertUint64<5>(value, cmpeq(valuesNew[5], quotes));
			insertUint64<6>(value, cmpeq(valuesNew[6], quotes));
			insertUint64<7>(value, cmpeq(valuesNew[7], quotes));
		}

		jsonifier_inline static bool opBool(const simd_int_t& value) {
			return _mm512_test_epi64_mask(value, value);
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
			return _mm512_shuffle_epi8(value, other);
		}

		template<uint64_t amount> static jsonifier_inline simd_int_t shl(const simd_int_t& value) {
			static jsonifier_constexpr uint64_t shiftBetween = amount % 64;
			alignas(BytesPerStep) uint64_t newArray00[SixtyFourBitsPerStep]{};
			alignas(BytesPerStep) uint64_t newArray01[SixtyFourBitsPerStep]{};
			store(value, newArray00);
			newArray01[0] = newArray00[0] << amount;
			newArray01[1] = (newArray00[1] << amount) | newArray00[0] >> (64 - shiftBetween);
			newArray01[2] = (newArray00[2] << amount) | newArray00[1] >> (64 - shiftBetween);
			newArray01[3] = (newArray00[3] << amount) | newArray00[2] >> (64 - shiftBetween);
			newArray01[4] = (newArray00[4] << amount) | newArray00[3] >> (64 - shiftBetween);
			newArray01[5] = (newArray00[5] << amount) | newArray00[4] >> (64 - shiftBetween);
			newArray01[6] = (newArray00[6] << amount) | newArray00[5] >> (64 - shiftBetween);
			newArray01[7] = (newArray00[7] << amount) | newArray00[6] >> (64 - shiftBetween);
			return gatherValues<simd_int_t>(newArray01);
		}

		template<typename value_type> jsonifier_inline static void storeu(const simd_int_t& value, value_type* storageLocation) {
			_mm512_storeu_si512(storageLocation, value);
		}

		template<typename value_type> jsonifier_inline static void store(const simd_int_t& value, value_type* storageLocation) {
			_mm512_store_si512(storageLocation, value);
		}

		template<typename value_type> jsonifier_inline static void store(const simd_int_256& value, value_type* storageLocation) {
			_mm256_store_si256(reinterpret_cast<__m256i*>(storageLocation), value);
		}

		jsonifier_inline static simd_int_t setLSB(const simd_int_t& value, bool valueNew) {
			if (valueNew) {
				return _mm512_or_si512(value, _mm512_set_epi64(0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01));
			} else {
				return _mm512_andnot_si512(_mm512_set_epi64(0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01), value);
			}
		}

		jsonifier_inline static simd_int_t setMSB(const simd_int_t& value, bool valueNew) {
			if (valueNew) {
				return _mm512_or_si512(value, _mm512_set_epi64(0x8000000000000000, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00));
			} else {
				return _mm512_andnot_si512(_mm512_set_epi64(0x8000000000000000, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00), value);
			}
		}

		jsonifier_inline static bool getMSB(const simd_int_t& value) {
			simd_int_t result = _mm512_and_si512(value, _mm512_set_epi64(0x8000000000000000, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00));
			return !_mm512_test_epi64_mask(result, result);
		}

		template<uint64_t index> jsonifier_inline static void processValue(const simd_int_128& allOnes, uint64_t& value, uint64_t& valuesNewer, uint64_t& prevInString) {
			valuesNewer	 = _mm_cvtsi128_si64(_mm_clmulepi64_si128(_mm_set_epi64x(0ULL, value), allOnes, 0)) ^ prevInString;
			prevInString = uint64_t(static_cast<int64_t>(valuesNewer) >> 63);
		}

		jsonifier_inline static simd_int_t carrylessMultiplication256(const simd_int_256& value, uint64_t& prevInString) {
			static jsonifier_constexpr simd_int_128 allOnes{ simdFromValue<simd_int_128>(0xFFu) };
			alignas(BytesPerStep) uint64_t valuesNewer01[SixtyFourBitsPerStep]{};
			alignas(BytesPerStep) uint64_t valuesNewer02[SixtyFourBitsPerStep]{};
			store(value, valuesNewer01);
			processValue<0>(allOnes, valuesNewer01[0], valuesNewer02[0], prevInString);
			processValue<1>(allOnes, valuesNewer01[1], valuesNewer02[1], prevInString);
			processValue<2>(allOnes, valuesNewer01[2], valuesNewer02[2], prevInString);
			processValue<3>(allOnes, valuesNewer01[3], valuesNewer02[3], prevInString);
			return gatherValues<simd_int_256>(valuesNewer02);
		}

		jsonifier_inline static simd_int_t follows(const simd_int_t& value, bool& overflow) {
			simd_int_t result = shl<1>(value);
			result			  = setLSB(result, overflow);
			overflow		  = getMSB(value);
			return result;
		}

		jsonifier_inline static simd_int_t carrylessMultiplication(const simd_int_t& value, string_parsing_type& prevInString) {
			simd_int_t returnValue{};
			simd_int_256 valuesLow{ _mm512_extracti64x4_epi64(value, 0) };
			simd_int_256 valuesHigh{ _mm512_extracti64x4_epi64(value, 1) };
			returnValue = _mm512_inserti64x4(returnValue, carrylessMultiplication256(valuesLow, prevInString), 0);
			returnValue = _mm512_inserti64x4(returnValue, carrylessMultiplication256(valuesHigh, prevInString), 1);
			return returnValue;
		}

		jsonifier_inline static simd_int_t follows(const simd_int_t& value, simd_int_t& overflow) {
			simd_int_t result = shl<1>(value);
			result			  = setLSB(result, getMSB(overflow));
			overflow		  = setMSB(overflow, getMSB(value));
			return result;
		}
	};

#endif

}