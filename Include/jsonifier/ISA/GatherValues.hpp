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

#include <jsonifier/TypeEntities.hpp>

namespace simd_internal {

	template<jsonifier::concepts::unsigned_type simd_int_type_new, typename char_type> JSONIFIER_INLINE static simd_int_type_new gatherValuesU(char_type* str) {
		simd_int_type_new returnValue{};
		std::memcpy(&returnValue, str, sizeof(simd_int_type_new));
		return returnValue;
	}

#if JSONIFIER_CHECK_FOR_INSTRUCTION(JSONIFIER_NEON)

	template<simd_int_128_type simd_int_type_new, jsonifier::concepts::uint16_type char_type> JSONIFIER_INLINE static simd_int_type_new gatherValues(char_type* str) {
		return vreinterpretq_u8_u16(vld1q_u16(str));
	}

	template<simd_int_128_type simd_int_type_new, jsonifier::concepts::uint64_type char_type> JSONIFIER_INLINE static simd_int_type_new gatherValues(char_type* str) {
		return vreinterpretq_u8_u64(vld1q_u64(str));
	}

	template<simd_int_128_type simd_int_type_new> JSONIFIER_INLINE static simd_int_type_new gatherValues(const void* str) {
		return vld1q_u8(static_cast<const uint8_t*>(str));
	}

	template<simd_int_128_type simd_int_type_new> JSONIFIER_INLINE static simd_int_type_new gatherValuesU(const void* str) {
		return vld1q_u8(static_cast<const uint8_t*>(str));
	}

	template<simd_int_128_type simd_int_type_new, typename char_type> JSONIFIER_INLINE static simd_int_type_new gatherValue(char_type str) {
		return vdupq_n_u8(str);
	}

	template<simd_int_128_type simd_int_type_new, typename char_type> JSONIFIER_INLINE static void store(const simd_int_type_new& value, char_type* storageLocation) {
		vst1q_u64(storageLocation, vreinterpretq_u64_u8(value));
	}

	template<simd_int_128_type simd_int_type_new, jsonifier::concepts::uint8_type char_type>
	JSONIFIER_INLINE static void store(const simd_int_type_new& value, char_type* storageLocation) {
		vst1q_u8(storageLocation, value);
	}

#elif JSONIFIER_CHECK_FOR_AVX(JSONIFIER_AVX)

	template<simd_int_128_type simd_int_type_new, typename char_type> JSONIFIER_INLINE static simd_int_type_new gatherValues(char_type* str) {
		return _mm_load_si128(reinterpret_cast<const __m128i*>(str));
	}

	template<simd_int_128_type simd_int_type_new, typename char_type> JSONIFIER_INLINE static simd_int_type_new gatherValuesU(char_type* str) {
		return _mm_loadu_si128(reinterpret_cast<const __m128i*>(str));
	}

	template<simd_int_128_type simd_int_type_new, typename char_type> JSONIFIER_INLINE static simd_int_type_new gatherValue(char_type str) {
		return _mm_set1_epi8(str);
	}

	template<simd_int_128_type simd_int_type_new, typename char_type> JSONIFIER_INLINE static void store(const simd_int_type_new& value, char_type* storageLocation) {
		_mm_store_si128(reinterpret_cast<__m128i*>(storageLocation), value);
	}

	#if JSONIFIER_CHECK_FOR_AVX(JSONIFIER_AVX2)

	template<simd_int_256_type simd_int_type_new, typename char_type> JSONIFIER_INLINE static simd_int_type_new gatherValues(char_type* str) {
		return _mm256_load_si256(reinterpret_cast<const __m256i*>(str));
	}

	template<simd_int_256_type simd_int_type_new, typename char_type> JSONIFIER_INLINE static simd_int_type_new gatherValuesU(char_type* str) {
		return _mm256_loadu_si256(reinterpret_cast<const __m256i*>(str));
	}

	template<simd_int_256_type simd_int_type_new, typename char_type> JSONIFIER_INLINE static simd_int_type_new gatherValue(char_type str) {
		return _mm256_set1_epi8(str);
	}

	template<simd_int_256_type simd_int_type_new, typename char_type> JSONIFIER_INLINE static void store(const simd_int_type_new& value, char_type* storageLocation) {
		_mm256_store_si256(reinterpret_cast<__m256i*>(storageLocation), value);
	}

		#if JSONIFIER_CHECK_FOR_AVX(JSONIFIER_AVX512)

	template<simd_int_512_type simd_int_type_new, typename char_type> JSONIFIER_INLINE static simd_int_type_new gatherValues(char_type* str) {
		return _mm512_load_si512(str);
	}

	template<simd_int_512_type simd_int_type_new, typename char_type> JSONIFIER_INLINE static simd_int_type_new gatherValuesU(char_type* str) {
		return _mm512_loadu_si512(reinterpret_cast<const __m512i*>(str));
	}

	template<simd_int_512_type simd_int_type_new, typename char_type> JSONIFIER_INLINE static simd_int_type_new gatherValue(char_type str) {
		return _mm512_set1_epi8(str);
	}

	template<simd_int_512_type simd_int_type_new, typename char_type> JSONIFIER_INLINE static void store(const simd_int_type_new& value, char_type* storageLocation) {
		_mm512_store_si512(storageLocation, value);
	}

		#endif

	#endif

#else

	template<simd_int_128_type simd_int_type_new, typename char_type> JSONIFIER_INLINE static simd_int_type_new gatherValues(char_type* str) {
		simd_int_t returnValue{};
		std::memcpy(&returnValue, str, sizeof(simd_int_t));
		return returnValue;
	}

	template<simd_int_128_type simd_int_type_new, typename char_type> JSONIFIER_INLINE static simd_int_type_new gatherValuesU(char_type* str) {
		simd_int_t returnValue{};
		std::memcpy(&returnValue, str, sizeof(simd_int_t));
		return returnValue;
	}

	template<simd_int_128_type simd_int_type_new, typename char_type> JSONIFIER_INLINE static simd_int_type_new gatherValue(char_type str) {
		simd_int_t returnValue{};
		std::memset(&returnValue, str, sizeof(simd_int_t));
		return returnValue;
	}

	template<typename char_type> JSONIFIER_INLINE static void store(const simd_int_t& value, char_type* storageLocation) {
		std::memcpy(storageLocation, &value, sizeof(simd_int_t));
	}

#endif

}