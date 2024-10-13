/*
	MIT License

	Copyright (c) 2024 RealTimeChris

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

#if defined(JSONIFIER_AVX) || defined(JSONIFIER_AVX2) || defined(JSONIFIER_AVX512)

	template<jsonifier::concepts::simd_int_128_type simd_int_type_new, typename char_type> JSONIFIER_ALWAYS_INLINE simd_int_type_new gatherValues(char_type* str) noexcept {
		return _mm_load_si128(reinterpret_cast<const __m128i*>(str));
	}

	template<jsonifier::concepts::simd_int_128_type simd_int_type_new, typename char_type> JSONIFIER_ALWAYS_INLINE simd_int_type_new gatherValuesU(char_type* str) noexcept {
		return _mm_loadu_si128(reinterpret_cast<const __m128i*>(str));
	}

	template<jsonifier::concepts::simd_int_128_type simd_int_type_new, jsonifier::concepts::uint64_type char_type>
	JSONIFIER_ALWAYS_INLINE simd_int_type_new gatherValue(char_type str) noexcept {
		return _mm_set1_epi64x(str);
	}

	template<jsonifier::concepts::simd_int_128_type simd_int_type_new, typename char_type>
		requires(sizeof(char_type) == 1)
	JSONIFIER_ALWAYS_INLINE simd_int_type_new gatherValue(char_type str) noexcept {
		return _mm_set1_epi8(static_cast<char>(str));
	}

	template<jsonifier::concepts::simd_int_128_type simd_int_type_new, typename char_type>
	JSONIFIER_ALWAYS_INLINE void store(simd_int_type_new&& value, char_type* storageLocation) noexcept {
		_mm_store_si128(reinterpret_cast<__m128i*>(storageLocation), value);
	}

	template<jsonifier::concepts::simd_int_128_type simd_int_t01, jsonifier::concepts::simd_int_128_type simd_int_t02>
	JSONIFIER_ALWAYS_INLINE auto opCmpEq(simd_int_t01&& value, simd_int_t02&& other) noexcept {
		return static_cast<uint16_t>(_mm_movemask_epi8(_mm_cmpeq_epi8(value, other)));
	}

	template<jsonifier::concepts::simd_int_128_type simd_int_t01, jsonifier::concepts::simd_int_128_type simd_int_t02>
	JSONIFIER_ALWAYS_INLINE auto opCmpLt(simd_int_t01&& value, simd_int_t02&& other) noexcept {
		jsonifier_simd_int_128 offset		  = _mm_set1_epi8(static_cast<char>(0x80));
		jsonifier_simd_int_128 adjusted_value = _mm_add_epi8(value, offset);
		jsonifier_simd_int_128 adjusted_other = _mm_add_epi8(other, offset);
		return _mm_movemask_epi8(_mm_cmpgt_epi8(adjusted_other, adjusted_value));
	}

	template<jsonifier::concepts::simd_int_128_type simd_int_t01> JSONIFIER_ALWAYS_INLINE auto opBitMask(simd_int_t01&& value) noexcept {
		return static_cast<uint16_t>(_mm_movemask_epi8(value));
	}

	template<jsonifier::concepts::simd_int_128_type simd_int_t01, jsonifier::concepts::simd_int_128_type simd_int_t02>
	JSONIFIER_ALWAYS_INLINE auto opShuffle(simd_int_t01&& value, simd_int_t02&& other) noexcept {
		return _mm_shuffle_epi8(value, other);
	}

	template<jsonifier::concepts::simd_int_128_type simd_int_t01, jsonifier::concepts::simd_int_128_type simd_int_t02>
	JSONIFIER_ALWAYS_INLINE auto opXor(simd_int_t01&& value, simd_int_t02&& other) noexcept {
		return _mm_xor_si128(value, other);
	}

	template<jsonifier::concepts::simd_int_128_type simd_int_t01, jsonifier::concepts::simd_int_128_type simd_int_t02>
	JSONIFIER_ALWAYS_INLINE auto opSub64(simd_int_t01&& value, simd_int_t02&& other) noexcept {
		return _mm_sub_epi64(value, other);
	}

	template<jsonifier::concepts::simd_int_128_type simd_int_t01, jsonifier::concepts::simd_int_128_type simd_int_t02>
	JSONIFIER_ALWAYS_INLINE auto opAdd8(simd_int_t01&& value, simd_int_t02&& other) noexcept {
		return _mm_add_epi8(value, other);
	}

	template<jsonifier::concepts::simd_int_128_type simd_int_t01, jsonifier::concepts::simd_int_128_type simd_int_t02>
	JSONIFIER_ALWAYS_INLINE auto opAnd(simd_int_t01&& value, simd_int_t02&& other) noexcept {
		return _mm_and_si128(value, other);
	}

	template<jsonifier::concepts::simd_int_128_type simd_int_t01, jsonifier::concepts::simd_int_128_type simd_int_t02>
	JSONIFIER_ALWAYS_INLINE auto opOr(simd_int_t01&& value, simd_int_t02&& other) noexcept {
		return _mm_or_si128(value, other);
	}

	template<jsonifier::concepts::simd_int_128_type simd_int_t01, jsonifier::concepts::simd_int_128_type simd_int_t02>
	JSONIFIER_ALWAYS_INLINE auto opAndNot(simd_int_t01&& value, simd_int_t02&& other) noexcept {
		return _mm_andnot_si128(other, value);
	}

	template<jsonifier::concepts::simd_int_128_type simd_int_t01, jsonifier::concepts::simd_int_128_type simd_int_t02>
	JSONIFIER_ALWAYS_INLINE auto opTest(simd_int_t01&& value, simd_int_t02&& other) noexcept {
		return !_mm_testz_si128(value, other);
	}

	template<jsonifier::concepts::simd_int_128_type simd_type> JSONIFIER_ALWAYS_INLINE jsonifier_simd_int_128 opSetLSB(simd_type&& value, bool valueNew) noexcept {
	#if defined(JSONIFIER_WIN) || defined(JSONIFIER_LINUX)
		jsonifier_simd_int_128 mask{ 0x01u, '\0' };
	#else
		jsonifier_simd_int_128 mask{ 0x01u, '\0', '\0', '\0', '\0', '\0', '\0', '\0' };
	#endif
		return valueNew ? _mm_or_si128(value, mask) : _mm_andnot_si128(mask, value);
	}

	template<jsonifier::concepts::simd_int_128_type simd_type> JSONIFIER_ALWAYS_INLINE bool opGetMSB(simd_type&& value) noexcept {
		auto result = _mm_and_si128(value, _mm_set_epi64x(0x8000000000000000ll, 0x00ll));
		return !_mm_testz_si128(result, result);
	}

	#if defined(opNot)
		#undef opNot
	#endif
	#define opNot(x) _mm_xor_si128(x, _mm_set1_epi64x(0xFFFFFFFFFFFFFFFFll))

	#if defined(JSONIFIER_AVX2)

	template<jsonifier::concepts::simd_int_256_type simd_int_type_new, typename char_type> JSONIFIER_ALWAYS_INLINE simd_int_type_new gatherValues(char_type* str) noexcept {
		return _mm256_load_si256(reinterpret_cast<const __m256i*>(str));
	}

	template<jsonifier::concepts::simd_int_256_type simd_int_type_new, typename char_type> JSONIFIER_ALWAYS_INLINE simd_int_type_new gatherValuesU(char_type* str) noexcept {
		return _mm256_loadu_si256(reinterpret_cast<const __m256i*>(str));
	}

	template<jsonifier::concepts::simd_int_256_type simd_int_type_new, jsonifier::concepts::uint64_type char_type>
	JSONIFIER_ALWAYS_INLINE simd_int_type_new gatherValue(char_type str) noexcept {
		return _mm256_set1_epi64x(str);
	}

	template<jsonifier::concepts::simd_int_256_type simd_int_type_new, typename char_type>
		requires(sizeof(char_type) == 1)
	JSONIFIER_ALWAYS_INLINE simd_int_type_new gatherValue(char_type str) noexcept {
		return _mm256_set1_epi8(static_cast<char>(str));
	}

	template<jsonifier::concepts::simd_int_256_type simd_int_type_new, typename char_type>
	JSONIFIER_ALWAYS_INLINE void store(simd_int_type_new&& value, char_type* storageLocation) noexcept {
		_mm256_store_si256(reinterpret_cast<__m256i*>(storageLocation), value);
	}

	template<jsonifier::concepts::simd_int_256_type simd_int_t01, jsonifier::concepts::simd_int_256_type simd_int_t02>
	JSONIFIER_ALWAYS_INLINE auto opCmpEq(simd_int_t01&& value, simd_int_t02&& other) noexcept {
		return static_cast<uint32_t>(_mm256_movemask_epi8(_mm256_cmpeq_epi8(value, other)));
	}

	template<jsonifier::concepts::simd_int_256_type simd_int_t01, jsonifier::concepts::simd_int_256_type simd_int_t02>
	JSONIFIER_ALWAYS_INLINE auto opCmpLt(simd_int_t01&& value, simd_int_t02&& other) noexcept {
		jsonifier_simd_int_256 offset		  = _mm256_set1_epi8(static_cast<char>(0x80));
		jsonifier_simd_int_256 adjusted_value = _mm256_add_epi8(value, offset);
		jsonifier_simd_int_256 adjusted_other = _mm256_add_epi8(other, offset);
		return _mm256_movemask_epi8(_mm256_cmpgt_epi8(adjusted_other, adjusted_value));
	}

	template<jsonifier::concepts::simd_int_256_type simd_int_t01> JSONIFIER_ALWAYS_INLINE auto opBitMask(simd_int_t01&& value) noexcept {
		return static_cast<uint32_t>(_mm256_movemask_epi8(value));
	}

	template<jsonifier::concepts::simd_int_256_type simd_int_t01, jsonifier::concepts::simd_int_256_type simd_int_t02>
	JSONIFIER_ALWAYS_INLINE auto opShuffle(simd_int_t01&& value, simd_int_t02&& other) noexcept {
		return _mm256_shuffle_epi8(value, other);
	}

	template<jsonifier::concepts::simd_int_256_type simd_int_t01, jsonifier::concepts::simd_int_256_type simd_int_t02>
	JSONIFIER_ALWAYS_INLINE auto opXor(simd_int_t01&& value, simd_int_t02&& other) noexcept {
		return _mm256_xor_si256(value, other);
	}

	template<jsonifier::concepts::simd_int_256_type simd_int_t01, jsonifier::concepts::simd_int_256_type simd_int_t02>
	JSONIFIER_ALWAYS_INLINE auto opSub64(simd_int_t01&& value, simd_int_t02&& other) noexcept {
		return _mm256_sub_epi64(value, other);
	}

	template<jsonifier::concepts::simd_int_256_type simd_int_t01, jsonifier::concepts::simd_int_256_type simd_int_t02>
	JSONIFIER_ALWAYS_INLINE auto opAdd8(simd_int_t01&& value, simd_int_t02&& other) noexcept {
		return _mm256_add_epi8(value, other);
	}

	template<jsonifier::concepts::simd_int_256_type simd_int_t01, jsonifier::concepts::simd_int_256_type simd_int_t02>
	JSONIFIER_ALWAYS_INLINE auto opAnd(simd_int_t01&& value, simd_int_t02&& other) noexcept {
		return _mm256_and_si256(value, other);
	}

	template<jsonifier::concepts::simd_int_256_type simd_int_t01, jsonifier::concepts::simd_int_256_type simd_int_t02>
	JSONIFIER_ALWAYS_INLINE auto opOr(simd_int_t01&& value, simd_int_t02&& other) noexcept {
		return _mm256_or_si256(value, other);
	}

	template<jsonifier::concepts::simd_int_256_type simd_int_t01, jsonifier::concepts::simd_int_256_type simd_int_t02>
	JSONIFIER_ALWAYS_INLINE auto opAndNot(simd_int_t01&& value, simd_int_t02&& other) noexcept {
		return _mm256_andnot_si256(other, value);
	}

	template<jsonifier::concepts::simd_int_256_type simd_int_t01, jsonifier::concepts::simd_int_256_type simd_int_t02>
	JSONIFIER_ALWAYS_INLINE auto opTest(simd_int_t01&& value, simd_int_t02&& other) noexcept {
		return !_mm256_testz_si256(value, other);
	}

	template<jsonifier::concepts::simd_int_256_type simd_type> JSONIFIER_ALWAYS_INLINE jsonifier_simd_int_256 opSetLSB(simd_type&& value, bool valueNew) noexcept {
		#if defined(JSONIFIER_WIN) || defined(JSONIFIER_LINUX)
		jsonifier_simd_int_256 mask{ 0x01u, '\0', '\0', '\0' };
		#else
		jsonifier_simd_int_256 mask{ 0x01u, '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0' };
		#endif
		return valueNew ? _mm256_or_si256(value, mask) : _mm256_andnot_si256(mask, value);
	}

	template<jsonifier::concepts::simd_int_256_type simd_type> JSONIFIER_ALWAYS_INLINE bool opGetMSB(simd_type&& value) noexcept {
		auto result = _mm256_and_si256(value, _mm256_set_epi64x(0x8000000000000000ll, 0x00ll, 0x00ll, 0x00ll));
		return !_mm256_testz_si256(result, result);
	}

		#if defined(opNot)
			#undef opNot
		#endif
		#define opNot(x) _mm256_xor_si256(x, _mm256_set1_epi64x(0xFFFFFFFFFFFFFFFFll))

		#if defined(JSONIFIER_AVX512)

	template<jsonifier::concepts::simd_int_512_type simd_int_type_new, typename char_type> JSONIFIER_ALWAYS_INLINE simd_int_type_new gatherValues(char_type* str) noexcept {
		return _mm512_load_si512(reinterpret_cast<const __m512i*>(str));
	}

	template<jsonifier::concepts::simd_int_512_type simd_int_type_new, typename char_type> JSONIFIER_ALWAYS_INLINE simd_int_type_new gatherValuesU(char_type* str) noexcept {
		return _mm512_loadu_si512(reinterpret_cast<const __m512i*>(str));
	}

	template<jsonifier::concepts::simd_int_512_type simd_int_type_new, jsonifier::concepts::uint64_type char_type>
	JSONIFIER_ALWAYS_INLINE simd_int_type_new gatherValue(char_type str) noexcept {
		return _mm512_set1_epi64x(str);
	}

	template<jsonifier::concepts::simd_int_512_type simd_int_type_new, typename char_type>
		requires(sizeof(char_type) == 1)
	JSONIFIER_ALWAYS_INLINE simd_int_type_new gatherValue(char_type str) noexcept {
		return _mm512_set1_epi8(static_cast<char>(str));
	}

	template<jsonifier::concepts::simd_int_512_type simd_int_type_new, typename char_type>
	JSONIFIER_ALWAYS_INLINE void store(simd_int_type_new&& value, char_type* storageLocation) noexcept {
		_mm512_store_si512(reinterpret_cast<__m512i*>(storageLocation), value);
	}

	template<jsonifier::concepts::simd_int_512_type simd_int_t01, jsonifier::concepts::simd_int_512_type simd_int_t02>
	JSONIFIER_ALWAYS_INLINE auto opCmpEq(simd_int_t01&& value, simd_int_t02&& other) noexcept {
		return static_cast<uint16_t>(_mm512_movemask_epi8(_mm512_cmpeq_epi8(value, other)));
	}

	template<jsonifier::concepts::simd_int_512_type simd_int_t01> JSONIFIER_ALWAYS_INLINE auto opBitMask(simd_int_t01&& value) noexcept {
		return static_cast<uint16_t>(_mm512_movemask_epi8(value));
	}

	template<jsonifier::concepts::simd_int_512_type simd_int_t01, jsonifier::concepts::simd_int_512_type simd_int_t02>
	JSONIFIER_ALWAYS_INLINE auto opShuffle(simd_int_t01&& value, simd_int_t02&& other) noexcept {
		return _mm512_shuffle_epi8(value, other);
	}

	template<jsonifier::concepts::simd_int_512_type simd_int_t01, jsonifier::concepts::simd_int_512_type simd_int_t02>
	JSONIFIER_ALWAYS_INLINE auto opXor(simd_int_t01&& value, simd_int_t02&& other) noexcept {
		return _mm512_xor_si512(value, other);
	}

	template<jsonifier::concepts::simd_int_512_type simd_int_t01, jsonifier::concepts::simd_int_512_type simd_int_t02>
	JSONIFIER_ALWAYS_INLINE auto opSub64(simd_int_t01&& value, simd_int_t02&& other) noexcept {
		return _mm512_sub_epi64(value, other);
	}

	template<jsonifier::concepts::simd_int_512_type simd_int_t01, jsonifier::concepts::simd_int_512_type simd_int_t02>
	JSONIFIER_ALWAYS_INLINE auto opAdd8(simd_int_t01&& value, simd_int_t02&& other) noexcept {
		return _mm512_add_epi8(value, other);
	}

	template<jsonifier::concepts::simd_int_512_type simd_int_t01, jsonifier::concepts::simd_int_512_type simd_int_t02>
	JSONIFIER_ALWAYS_INLINE auto opAnd(simd_int_t01&& value, simd_int_t02&& other) noexcept {
		return _mm512_and_si512(value, other);
	}

	template<jsonifier::concepts::simd_int_512_type simd_int_t01, jsonifier::concepts::simd_int_512_type simd_int_t02>
	JSONIFIER_ALWAYS_INLINE auto opOr(simd_int_t01&& value, simd_int_t02&& other) noexcept {
		return _mm512512_or_si512(value, other);
	}

	template<jsonifier::concepts::simd_int_512_type simd_int_t01, jsonifier::concepts::simd_int_512_type simd_int_t02>
	JSONIFIER_ALWAYS_INLINE auto opAndNot(simd_int_t01&& value, simd_int_t02&& other) noexcept {
		return _mm512_andnot_si512(other, value);
	}

	template<jsonifier::concepts::simd_int_512_type simd_int_t01, jsonifier::concepts::simd_int_512_type simd_int_t02>
	JSONIFIER_ALWAYS_INLINE auto opTest(simd_int_t01&& value, simd_int_t02&& other) noexcept {
		return !_mm512_testz_si512(value, other);
	}

	template<jsonifier::concepts::simd_int_512_type simd_type> JSONIFIER_ALWAYS_INLINE jsonifier_simd_int_512 opSetLSB(simd_type&& value, bool valueNew) noexcept {
			#if defined(JSONIFIER_WIN) || defined(JSONIFIER_LINUX)
		jsonifier_simd_int_512 mask{ 0x01u, '\0', '\0', '\0', '\0', '\0', '\0', '\0' };
			#else
		jsonifier_simd_int_512 mask{ 0x01u, '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
			'\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0' };
			#endif
		return valueNew ? _mm512_or_si512(value, mask) : _mm512_andnot_si512(mask, value);
	}

	template<jsonifier::concepts::simd_int_512_type simd_type> JSONIFIER_ALWAYS_INLINE bool opGetMSB(simd_type&& value) noexcept {
		auto result = _mm512_and_si512(value, _mm512_set_epi64x(0x8000000000000000ll, 0x00ll));
		return !_mm512_testz_si512(result, result);
	}

			#if defined(opNot)
				#undef opNot
			#endif
			#define opNot(x) _mm512_xor_si512(x, _mm512_set1_epi64x(0xFFFFFFFFFFFFFFFFll))

		#endif

	#endif

#endif

}