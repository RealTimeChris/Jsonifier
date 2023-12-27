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

#include <jsonifier/ISADetection.hpp>

namespace jsonifier_internal {

	template<typename char_type01, typename char_type02> JSONIFIER_INLINE bool compareShort(const char_type01* lhs, const char_type02* rhs, uint64_t count) noexcept {
		if (count > 7) {
			static constexpr uint64_t n{ sizeof(uint64_t) };
			uint64_t v[2];
			while (count > n) {
				std::memcpy(v, lhs, n);
				std::memcpy(v + 1, rhs, n);
				if (v[0] != v[1]) {
					return false;
				}
				count -= n;
				lhs += n;
				rhs += n;
			}

			const auto shift = n - count;
			lhs -= shift;
			rhs -= shift;

			std::memcpy(v, lhs, n);
			std::memcpy(v + 1, rhs, n);
			return v[0] == v[1];
		}
		{
			static constexpr uint64_t n{ sizeof(uint32_t) };
			if (count >= n) {
				uint32_t v[2];
				std::memcpy(v, lhs, n);
				std::memcpy(v + 1, rhs, n);
				if (v[0] != v[1]) {
					return false;
				}
				count -= n;
				lhs += n;
				rhs += n;
			}
		}
		{
			static constexpr uint64_t n{ sizeof(uint16_t) };
			if (count >= n) {
				uint16_t v[2];
				std::memcpy(v, lhs, n);
				std::memcpy(v + 1, rhs, n);
				if (v[0] != v[1]) {
					return false;
				}
				count -= n;
				lhs += n;
				rhs += n;
			}
		}
		if (count && *lhs != *rhs) {
			return false;
		}
		return true;
	}

	template<typename char_type01, typename char_type02> JSONIFIER_INLINE bool compare(char_type01* string1, char_type02* string2, uint64_t lengthNew) {
#if JSONIFIER_CHECK_FOR_AVX(JSONIFIER_AVX512)
		{
			using integer_type						= typename jsonifier::concepts::get_type_at_index<avx_list, 2>::type::integer_type;
			using simd_type							= typename jsonifier::concepts::get_type_at_index<avx_list, 2>::type::type;
			static constexpr uint64_t vectorSize	= jsonifier::concepts::get_type_at_index<avx_list, 2>::type::bytesProcessed;
			static constexpr integer_type maskValue = jsonifier::concepts::get_type_at_index<avx_list, 2>::type::mask;
			simd_type value01{};
			simd_type value02{};
			while (lengthNew >= vectorSize) {
				value01 = simd_internal::gatherValuesU<simd_type>(string1);
				value02 = simd_internal::gatherValuesU<simd_type>(string2);
				if (simd_internal::opCmpEq(value01, value02) != maskValue) {
					return false;
				}
				lengthNew -= vectorSize;
				string1 += vectorSize;
				string2 += vectorSize;
			}
		}
#endif
#if JSONIFIER_CHECK_FOR_AVX(JSONIFIER_AVX2)
		{
			using integer_type						= typename jsonifier::concepts::get_type_at_index<avx_list, 1>::type::integer_type;
			using simd_type							= typename jsonifier::concepts::get_type_at_index<avx_list, 1>::type::type;
			static constexpr uint64_t vectorSize	= jsonifier::concepts::get_type_at_index<avx_list, 1>::type::bytesProcessed;
			static constexpr integer_type maskValue = jsonifier::concepts::get_type_at_index<avx_list, 1>::type::mask;
			simd_type value01{};
			simd_type value02{};
			while (lengthNew >= vectorSize) {
				value01 = simd_internal::gatherValuesU<simd_type>(string1);
				value02 = simd_internal::gatherValuesU<simd_type>(string2);
				if (simd_internal::opCmpEq(value01, value02) != maskValue) {
					return false;
				}
				lengthNew -= vectorSize;
				string1 += vectorSize;
				string2 += vectorSize;
			}
		}

#endif
#if JSONIFIER_CHECK_FOR_AVX(JSONIFIER_AVX)
		{
			using integer_type						= typename jsonifier::concepts::get_type_at_index<avx_list, 0>::type::integer_type;
			using simd_type							= typename jsonifier::concepts::get_type_at_index<avx_list, 0>::type::type;
			static constexpr uint64_t vectorSize	= jsonifier::concepts::get_type_at_index<avx_list, 0>::type::bytesProcessed;
			static constexpr integer_type maskValue = jsonifier::concepts::get_type_at_index<avx_list, 0>::type::mask;
			simd_type value01{};
			simd_type value02{};
			while (lengthNew >= vectorSize) {
				value01 = simd_internal::gatherValuesU<simd_type>(string1);
				value02 = simd_internal::gatherValuesU<simd_type>(string2);
				if (simd_internal::opCmpEq(value01, value02) != maskValue) {
					return false;
				}
				lengthNew -= vectorSize;
				string1 += vectorSize;
				string2 += vectorSize;
			}
		}
#endif
		return compareShort(string1, string2, lengthNew);
	}

	template<uint64_t Count, typename char_type01> JSONIFIER_INLINE bool compare(const char_type01* lhs, const char_type01* rhs) noexcept {
		static constexpr uint64_t n{ 8 };
		if constexpr (Count > n) {
			return compare(lhs, rhs, Count);
		} else if constexpr (Count == n) {
			uint64_t v[2];
			std::memcpy(v, lhs, Count);
			std::memcpy(v + 1, rhs, Count);
			return v[0] == v[1];
		} else if constexpr (Count > 4) {
			uint64_t v[2]{};
			std::memcpy(v, lhs, Count);
			std::memcpy(v + 1, rhs, Count);
			return v[0] == v[1];
		} else if constexpr (Count > 2) {
			uint32_t v[2]{};
			std::memcpy(v, lhs, Count);
			std::memcpy(v + 1, rhs, Count);
			return v[0] == v[1];
		} else if constexpr (Count == 2) {
			uint32_t v[2];
			std::memcpy(v, lhs, Count);
			std::memcpy(v + 1, rhs, Count);
			return v[0] == v[1];
		} else if constexpr (Count == 1) {
			return *lhs == *rhs;
		} else {
			return true;
		}
	}

}// namespace jsonifier_internal
