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
	concept avx_int_128_t = std::same_as<jsonifier::concepts::unwrap_t<value_type>, avx_int_128>;

	template<typename value_type>
	concept avx_int_256_t = std::same_as<jsonifier::concepts::unwrap_t<value_type>, avx_int_256>;

	template<typename value_type>
	concept avx_int_512_t = std::same_as<jsonifier::concepts::unwrap_t<value_type>, avx_int_512>;

	template<typename value_type>
	concept avx_float_128_t = std::same_as<jsonifier::concepts::unwrap_t<value_type>, avx_float_128>;

	template<typename value_type>
	concept avx_float_256_t = std::same_as<jsonifier::concepts::unwrap_t<value_type>, avx_float_256>;

	template<typename value_type>
	concept avx_float_512_t = std::same_as<jsonifier::concepts::unwrap_t<value_type>, avx_float_512>;

	using avx_int_t = avx_int_512;

	template<avx_int_128_t avx_type, jsonifier::concepts::char_type char_type> inline avx_type gatherValues(char_type* str) {
		alignas(JsonifierAlignment) double valuesNew[sizeof(avx_type)]{};
		std::memcpy(valuesNew, str, sizeof(avx_type));
		return _mm_castpd_si128(_mm_load_pd(valuesNew));
	}

	template<avx_int_256_t avx_type, jsonifier::concepts::char_type char_type> inline avx_type gatherValues(char_type* str) {
		alignas(JsonifierAlignment) double valuesNew[sizeof(avx_type)]{};
		std::memcpy(valuesNew, str, sizeof(avx_type));
		return _mm256_castpd_si256(_mm256_load_pd(valuesNew));
	}

	template<avx_int_512_t avx_type, jsonifier::concepts::char_type char_type> inline avx_type gatherValues(char_type* str) {
		return _mm512_load_si512(str);
	}

	template<avx_int_128_t avx_type, jsonifier::concepts::float_t char_type> inline avx_type gatherValues(char_type* str) {
		return _mm_castpd_si128(_mm_load_pd(str));
	}

	template<avx_int_256_t avx_type, jsonifier::concepts::float_t char_type> inline avx_type gatherValues(char_type* str) {
		return _mm256_castpd_si256(_mm256_load_pd(str));
	}

	template<avx_int_512_t avx_type, jsonifier::concepts::float_t char_type> inline avx_type gatherValues(char_type* str) {
		return _mm512_castpd_si512(_mm512_load_pd(str));
	}

	template<avx_float_128_t avx_type, jsonifier::concepts::float_t char_type> inline avx_type gatherValues(char_type* str) {
		return _mm_load_pd(str);
	}

	template<avx_float_256_t avx_type, jsonifier::concepts::float_t char_type> inline avx_type gatherValues(char_type* str) {
		return _mm256_load_pd(str);
	}

	template<avx_float_512_t avx_type, jsonifier::concepts::float_t char_type> inline avx_type gatherValues(char_type* str) {
		return _mm256_load_pd(str);
	}

	template<avx_int_128_t return_type> constexpr return_type simdValues(uint8_t value) {
	#if !defined(_WIN32)
		int64_t newArray[sizeof(avx_int_128) / sizeof(uint64_t)]{};
		for (uint64_t x = 0; x < sizeof(avx_int_128) / sizeof(uint64_t); ++x) {
			newArray[x] |= static_cast<int64_t>(value);
			newArray[x] |= static_cast<int64_t>(value) << 8;
			newArray[x] |= static_cast<int64_t>(value) << 16;
			newArray[x] |= static_cast<int64_t>(value) << 24;
			newArray[x] |= static_cast<int64_t>(value) << 32;
			newArray[x] |= static_cast<int64_t>(value) << 40;
			newArray[x] |= static_cast<int64_t>(value) << 48;
			newArray[x] |= static_cast<int64_t>(value) << 56;
		}
		avx_int_128 returnValue{ newArray[0], newArray[1] };
	#else
		avx_int_128 returnValue{};
		for (uint64_t x = 0; x < sizeof(avx_int_128); ++x) {
			returnValue.m128i_u8[x] = value;
		}
	#endif
		return returnValue;
	}

	template<avx_int_512_t return_type> constexpr return_type simdTable(const uint8_t arrayNew[sizeof(avx_int_t)]) {
	#if !defined(_WIN32)
		int64_t newArray[sizeof(avx_int_t) / sizeof(uint64_t)]{};
		for (uint64_t x = 0; x < sizeof(avx_int_t) / sizeof(uint64_t); ++x) {
			newArray[x] |= static_cast<int64_t>(arrayNew[(x * 8) + 0]);
			newArray[x] |= static_cast<int64_t>(arrayNew[(x * 8) + 1]) << 8;
			newArray[x] |= static_cast<int64_t>(arrayNew[(x * 8) + 2]) << 16;
			newArray[x] |= static_cast<int64_t>(arrayNew[(x * 8) + 3]) << 24;
			newArray[x] |= static_cast<int64_t>(arrayNew[(x * 8) + 4]) << 32;
			newArray[x] |= static_cast<int64_t>(arrayNew[(x * 8) + 5]) << 40;
			newArray[x] |= static_cast<int64_t>(arrayNew[(x * 8) + 6]) << 48;
			newArray[x] |= static_cast<int64_t>(arrayNew[(x * 8) + 7]) << 56;
		}
		avx_int_t returnValue{ newArray[0], newArray[1], newArray[2], newArray[3] };
	#else
		avx_int_t returnValue{};
		for (uint64_t x = 0; x < sizeof(avx_int_t); ++x) {
			returnValue.m512i_u8[x] = arrayNew[x];
		}
	#endif
		return returnValue;
	}

	template<avx_int_512_t return_type> constexpr return_type simdValues(uint8_t value) {
	#if !defined(_WIN32)
		int64_t newArray[sizeof(avx_int_t) / sizeof(uint64_t)]{};
		for (uint64_t x = 0; x < sizeof(avx_int_t) / sizeof(uint64_t); ++x) {
			newArray[x] |= static_cast<int64_t>(value);
			newArray[x] |= static_cast<int64_t>(value) << 8;
			newArray[x] |= static_cast<int64_t>(value) << 16;
			newArray[x] |= static_cast<int64_t>(value) << 24;
			newArray[x] |= static_cast<int64_t>(value) << 32;
			newArray[x] |= static_cast<int64_t>(value) << 40;
			newArray[x] |= static_cast<int64_t>(value) << 48;
			newArray[x] |= static_cast<int64_t>(value) << 56;
		}
		avx_int_t returnValue{ newArray[0], newArray[1], newArray[2], newArray[3] };
	#else
		avx_int_t returnValue{};
		for (uint64_t x = 0; x < sizeof(avx_int_t); ++x) {
			returnValue.m512i_u8[x] = value;
		}
	#endif
		return returnValue;
	}

	inline string_parsing_type cmpEq(const avx_int_t& lhs, const avx_int_t& rhs) {
		return _mm512_cmpeq_epi8_mask(lhs, rhs);
	}

	inline string_parsing_type cmpEq(const avx_int_t& lhs, uint8_t rhs) {
		return _mm512_cmpeq_epi8_mask(lhs, _mm512_set1_epi8(static_cast<char>(rhs)));
	}

	inline avx_int_t opOr(const avx_int_t& lhs, const avx_int_t& rhs) {
		return _mm512_or_si512(lhs, rhs);
	}

	inline avx_int_t opSub(const avx_int_t& lhs, const avx_int_t& rhs) {
		return _mm512_subs_epi8(lhs, rhs);
	}

	inline avx_int_t opAnd(const avx_int_t& lhs, const avx_int_t& rhs) {
		return _mm512_and_si512(lhs, rhs);
	}

	inline avx_int_t opXor(const avx_int_t& lhs, const avx_int_t& rhs) {
		return _mm512_xor_si512(lhs, rhs);
	}

	inline void setLSB(avx_int_t& values, bool valueNew) {
		if (valueNew) {
			values = _mm512_or_si512(values, _mm512_set_epi64(0, 0, 0, 0, 0, 0, 0, 0x1));
		} else {
			values = _mm512_andnot_si512(_mm512_set_epi64(0, 0, 0, 0, 0, 0, 0, 0x1), values);
		}
	}

	template<uint64_t amount> inline void shl(avx_int_t& values) {
		avx_int_t newValue{ _mm512_slli_epi64(values, (amount % 64)) };
		avx_int_t newerValue{ _mm512_srli_epi64(_mm512_permutexvar_epi64(_mm512_set_epi64(6, 5, 4, 3, 2, 1, 0, 7), values), 64 - (amount % 64)) };
		static constexpr int64_t maxValue{ static_cast<int64_t>(std::numeric_limits<string_parsing_type>::max()) };
		static constexpr uint8_t newArray[]{ 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
			0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
			0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0, 0, 0, 0, 0, 0, 0, 0 };
		static constexpr avx_int_t newestValue{ simdTable<avx_int_t>(newArray) };
		values = opOr(newValue, opAnd(newerValue, newestValue));
	}

	inline bool getMSB(avx_int_t& values) {
		avx_int_t result = _mm512_and_si512(values, _mm512_set_epi64(0x8000000000000000, 0, 0, 0, 0, 0, 0, 0));
		return static_cast<bool>(_mm512_test_epi64_mask(result, result));
	}

	template<uint64_t index = 0> inline void insertUint64(avx_int_t& value, uint64_t valueNew) {
		static_assert(index < SixtyFourBitsPerStep, "Sorry, but that index value is incorrect.");
		auto lower = _mm512_extracti64x4_epi64(value, index / 4);
		lower	   = _mm256_insert_epi64(lower, static_cast<int64_t>(valueNew), index % 4);
		value	   = _mm512_inserti64x4(value, lower, index / 4);
	}

	template<uint64_t index = 0> inline void addValues(avx_int_t& values, string_parsing_type valuesToAdd) {
		insertUint64<index>(values, valuesToAdd);
	}

	inline avx_int_t shuffle(const avx_int_t& other, const avx_int_t& values) {
		return _mm512_shuffle_epi8(values, other);
	}

	template<uint64_t index = 0> inline void convertWhitespaceToSimdBase(avx_int_t& values, avx_int_t valuesNew[StridesPerStep]) {
		if constexpr (index < StridesPerStep) {
			static constexpr uint8_t arrayNew[]{ ' ', 100, 100, 100, 17, 100, 113, 2, 100, '\t', '\n', 112, 100, '\r', 100, 100, ' ', 100, 100, 100, 17, 100, 113, 2, 100, '\t',
				'\n', 112, 100, '\r', 100, 100, ' ', 100, 100, 100, 17, 100, 113, 2, 100, '\t', '\n', 112, 100, '\r', 100, 100, ' ', 100, 100, 100, 17, 100, 113, 2, 100, '\t',
				'\n', 112, 100, '\r', 100, 100 };
			static constexpr avx_int_t whitespaceTable{ simdTable<avx_int_t>(arrayNew) };
			addValues<index>(values, cmpEq(shuffle(valuesNew[index], whitespaceTable), valuesNew[index]));
			convertWhitespaceToSimdBase<index + 1>(values, valuesNew);
		}
	}

	template<uint64_t index = 0> inline void convertBackslashesToSimdBase(avx_int_t& values, avx_int_t valuesNew[StridesPerStep]) {
		if constexpr (index < StridesPerStep) {
			static constexpr avx_int_t backslashesVal{ simdValues<avx_int_t>('\\') };
			addValues<index>(values, cmpEq(valuesNew[index], backslashesVal));
			convertBackslashesToSimdBase<index + 1>(values, valuesNew);
		}
	}

	template<uint64_t index = 0> inline void convertStructuralsToSimdBase(avx_int_t& values, avx_int_t valuesNew[StridesPerStep]) {
		if constexpr (index < StridesPerStep) {
			static constexpr uint8_t arrayNew[]{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, ':', '{', ',', '}', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, ':', '{', ',', '}', 0, 0, 0, 0, 0, 0, 0, 0,
				0, 0, 0, 0, ':', '{', ',', '}', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, ':', '{', ',', '}', 0, 0 };
			static constexpr avx_int_t opTableVal{ simdTable<avx_int_t>(arrayNew) };
			static constexpr avx_int_t chars{ simdValues<avx_int_t>(0x20) };
			addValues<index>(values, cmpEq(shuffle(valuesNew[index], opTableVal), opOr(valuesNew[index], chars)));
			convertStructuralsToSimdBase<index + 1>(values, valuesNew);
		}
	}

	template<uint64_t index = 0> inline void convertQuotesToSimdBase(avx_int_t& values, avx_int_t valuesNew[StridesPerStep]) {
		if constexpr (index < StridesPerStep) {
			static constexpr avx_int_t quotesVal{ simdValues<avx_int_t>('"') };
			addValues<index>(values, cmpEq(valuesNew[index], quotesVal));
			convertQuotesToSimdBase<index + 1>(values, valuesNew);
		}
	}

	template<uint64_t index = 0> inline void processValue(const avx_int_128& allOnes, avx_int_128& val, double* valuesNewer, uint64_t& prevInstring) {
		static uint64_t newValue{};
		newValue	 = static_cast<uint64_t>(_mm_cvtsi128_si64(_mm_clmulepi64_si128(val, allOnes, index % 2)) ^ prevInstring);
		prevInstring = static_cast<uint64_t>(static_cast<int64_t>(newValue) >> 63);
		std::memcpy(valuesNewer + index, &newValue, sizeof(double));
	}

	inline avx_int_256 carrylessMultiplication256(avx_int_256& value, uint64_t& prevInstring) {
		static constexpr avx_int_128 allOnes{ simdValues<avx_int_128>(0xFF) };
		avx_int_128 valueLow{ _mm256_extracti128_si256(value, 0) };
		avx_int_128 valueHigh{ _mm256_extracti128_si256(value, 1) };
		alignas(JsonifierAlignment) double valuesNewer[SixtyFourBitsPerStep]{};
		processValue<0>(allOnes, valueLow, valuesNewer, prevInstring);
		processValue<1>(allOnes, valueLow, valuesNewer, prevInstring);
		processValue<2>(allOnes, valueHigh, valuesNewer, prevInstring);
		processValue<3>(allOnes, valueHigh, valuesNewer, prevInstring);
		return gatherValues<avx_int_256>(valuesNewer);
	}

	inline avx_int_t carrylessMultiplication(avx_int_t& value, string_parsing_type& prevInstring) {
		avx_int_256 lowValues{ _mm512_extracti64x4_epi64(value, 0) };
		avx_int_256 highValues{ _mm512_extracti64x4_epi64(value, 1) };
		lowValues  = carrylessMultiplication256(lowValues, prevInstring);
		highValues = carrylessMultiplication256(highValues, prevInstring);
		avx_int_t returnValue{};
		returnValue = _mm512_inserti64x4(returnValue, lowValues, 0);
		returnValue = _mm512_inserti64x4(returnValue, highValues, 1);
		return returnValue;
	}

	inline avx_int_t follows(avx_int_t& value, bool& overflow) {
		avx_int_t result = value;
		shl<1>(result);
		setLSB(result, overflow);
		overflow = getMSB(value);
		return result;
	}

	inline avx_int_t bitAndNot(const avx_int_t& other, const avx_int_t& values) {
		return _mm512_andnot_si512(values, other);
	}

	inline bool opBool(const avx_int_t& value) {
		return _mm512_test_epi64_mask(value, value) != 0;
	}

	inline avx_int_t opNot(const avx_int_t& value) {
		return _mm512_xor_si512(value, _mm512_set1_epi64(static_cast<int64_t>(std::numeric_limits<uint64_t>::max())));
	}

	template<typename value_type> inline void store(const avx_int_t& values, value_type* storageLocation) {
		_mm512_store_si512(storageLocation, values);
	}

	inline void printBits(uint64_t values, const std::string& valuesTitle) {
		std::cout << valuesTitle;
		std::cout << std::bitset<64>{ values };
		std::cout << std::endl;
	}

	inline const avx_int_t& printBits(avx_int_t& value, const std::string& valuesTitle) noexcept {
		uint8_t values[BytesPerStep]{};
		store(value, values);
		std::cout << valuesTitle;
		for (uint64_t x = 0; x < BytesPerStep; ++x) {
			for (uint64_t y = 0; y < 8; ++y) {
				std::cout << std::bitset<1>{ static_cast<uint64_t>(*(values + x)) >> y };
			}
		}
		std::cout << std::endl;
		return value;
	}

#endif

}