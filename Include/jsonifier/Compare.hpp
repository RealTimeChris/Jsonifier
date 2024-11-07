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

namespace jsonifier_internal {

	template<const uint8_t repeat, jsonifier::concepts::uns16_t return_type> constexpr return_type repeatByte() noexcept {
		return 0x0101ull * repeat;
	}

	template<const uint8_t repeat, jsonifier::concepts::uns32_t return_type> constexpr return_type repeatByte() noexcept {
		return 0x01010101ull * repeat;
	}

	template<const uint8_t repeat, jsonifier::concepts::uns64_t return_type> constexpr return_type repeatByte() noexcept {
		return 0x0101010101010101ull * repeat;
	}

	template<char value, typename char_type> struct char_comparison {
		JSONIFIER_ALWAYS_INLINE static const char_type* memchar(const char_type* data, size_t lengthNew) noexcept {
#if JSONIFIER_CHECK_FOR_INSTRUCTION(JSONIFIER_AVX512)
			if (lengthNew >= 64) {
				using simd_type					   = typename get_type_at_index<simd_internal::avx_list, 2>::type::type;
				using integer_type				   = typename get_type_at_index<simd_internal::avx_list, 2>::type::integer_type;
				static constexpr size_t vectorSize = get_type_at_index<simd_internal::avx_list, 2>::type::bytesProcessed;
				const simd_type search_value	   = simd_internal::gatherValue<simd_type>(static_cast<uint8_t>(value));
				integer_type mask;
				simd_type chunk;
				while (lengthNew >= vectorSize) {
					chunk = simd_internal::gatherValuesU<simd_type>(data);
					mask  = simd_internal::opCmpEq(chunk, search_value);
					if JSONIFIER_UNLIKELY (mask != 0) {
						data += simd_internal::postCmpTzcnt(mask);
						return data;
					}
					lengthNew -= vectorSize;
					data += vectorSize;
				}
			}
#endif

#if JSONIFIER_CHECK_FOR_INSTRUCTION(JSONIFIER_AVX512) || JSONIFIER_CHECK_FOR_INSTRUCTION(JSONIFIER_AVX2)
			if (lengthNew >= 32) {
				using simd_type					   = typename get_type_at_index<simd_internal::avx_list, 1>::type::type;
				using integer_type				   = typename get_type_at_index<simd_internal::avx_list, 1>::type::integer_type;
				static constexpr size_t vectorSize = get_type_at_index<simd_internal::avx_list, 1>::type::bytesProcessed;
				const simd_type search_value	   = simd_internal::gatherValue<simd_type>(static_cast<uint8_t>(value));
				integer_type mask;
				simd_type chunk;
				while (lengthNew >= vectorSize) {
					chunk = simd_internal::gatherValuesU<simd_type>(data);
					mask  = simd_internal::opCmpEq(chunk, search_value);
					if JSONIFIER_UNLIKELY (mask != 0) {
						data += simd_internal::postCmpTzcnt(mask);
						return data;
					}
					lengthNew -= vectorSize;
					data += vectorSize;
				}
			}
#endif
#if JSONIFIER_CHECK_FOR_INSTRUCTION(JSONIFIER_AVX512) || JSONIFIER_CHECK_FOR_INSTRUCTION(JSONIFIER_AVX2) || JSONIFIER_CHECK_FOR_INSTRUCTION(JSONIFIER_AVX) || \
	JSONIFIER_CHECK_FOR_INSTRUCTION(JSONIFIER_NEON)
			if (lengthNew >= 16) {
				using simd_type					   = typename get_type_at_index<simd_internal::avx_list, 0>::type::type;
				using integer_type				   = typename get_type_at_index<simd_internal::avx_list, 0>::type::integer_type;
				static constexpr size_t vectorSize = get_type_at_index<simd_internal::avx_list, 0>::type::bytesProcessed;
				const simd_type search_value	   = simd_internal::gatherValue<simd_type>(static_cast<uint8_t>(value));
				integer_type mask;
				simd_type chunk;
				while (lengthNew >= vectorSize) {
					chunk = simd_internal::gatherValuesU<simd_type>(data);
					mask  = simd_internal::opCmpEq(chunk, search_value);
					if JSONIFIER_UNLIKELY (mask != 0) {
						data += simd_internal::postCmpTzcnt(mask);
						return data;
					}
					lengthNew -= vectorSize;
					data += vectorSize;
				}
			}
#endif
			{
				static constexpr size_t n{ sizeof(size_t) };
				if (lengthNew >= n) {
					static constexpr auto valueNew{ jsonifier_internal::repeatByte<value, size_t>() };
					static constexpr auto highBits{ jsonifier_internal::repeatByte<0x80, size_t>() };
					static constexpr auto lowBits{ jsonifier_internal::repeatByte<0x01, size_t>() };
					size_t simdValue;
					while (lengthNew >= n) {
						std::memcpy(&simdValue, data, sizeof(size_t));
						const auto chunk = simdValue ^ valueNew;
						auto next		 = ((chunk - lowBits) & ~chunk) & highBits;
						if (next) {
							next = simd_internal::tzcnt(static_cast<size_t>(next)) >> 3u;
							data += next;
							return data;
						} else {
							lengthNew -= n;
							data += n;
						}
					}
				}
			}
			{
				static constexpr size_t n{ sizeof(uint32_t) };
				if (lengthNew >= n) {
					static constexpr auto valueNew{ jsonifier_internal::repeatByte<value, uint32_t>() };
					static constexpr auto highBits{ jsonifier_internal::repeatByte<0x80, uint32_t>() };
					static constexpr auto lowBits{ jsonifier_internal::repeatByte<0x01, uint32_t>() };
					uint32_t simdValue;
					std::memcpy(&simdValue, data, sizeof(uint32_t));
					const auto chunk = simdValue ^ valueNew;
					auto next		 = ((chunk - lowBits) & ~chunk) & highBits;
					if (next) {
						next = simd_internal::tzcnt(static_cast<uint32_t>(next)) >> 3u;
						data += next;
						return data;
					} else {
						lengthNew -= n;
						data += n;
					}
				}
			}
			{
				static constexpr size_t n{ sizeof(uint16_t) };
				if (lengthNew >= n) {
					static constexpr auto valueNew{ jsonifier_internal::repeatByte<value, uint16_t>() };
					static constexpr auto highBits{ jsonifier_internal::repeatByte<0x80, uint16_t>() };
					static constexpr auto lowBits{ jsonifier_internal::repeatByte<0x01, uint16_t>() };
					uint16_t simdValue;
					std::memcpy(&simdValue, data, sizeof(uint16_t));
					const auto chunk = simdValue ^ valueNew;
					auto next		 = ((chunk - lowBits) & ~chunk) & highBits;
					if (next) {
						next = simd_internal::tzcnt(static_cast<uint16_t>(next)) >> 3u;
						data += next;
						return data;
					} else {
						lengthNew -= n;
						data += n;
					}
				}
			}

			if (lengthNew > 0 && *data == value) {
				++data;
				return data;
			}
			return static_cast<decltype(data)>(nullptr);
		}
	};

	template<typename char_t01, typename char_t02> struct comparison {
		JSONIFIER_ALWAYS_INLINE static bool compare(const char_t01* lhs, char_t02* rhs, size_t lengthNew) noexcept {
#if JSONIFIER_CHECK_FOR_INSTRUCTION(JSONIFIER_AVX512)
			if (lengthNew >= 64) {
				using simd_type					   = typename get_type_at_index<simd_internal::avx_list, 2>::type::type;
				static constexpr size_t vectorSize = get_type_at_index<simd_internal::avx_list, 2>::type::bytesProcessed;
				static constexpr size_t mask	   = get_type_at_index<simd_internal::avx_list, 2>::type::mask;
				simd_type value01, value02;
				while (lengthNew >= vectorSize) {
					value01 = simd_internal::gatherValuesU<simd_type>(lhs);
					value02 = simd_internal::gatherValuesU<simd_type>(rhs);
					if (simd_internal::opCmpEq(value01, value02) != mask) {
						return false;
					};
					lengthNew -= vectorSize;
					lhs += vectorSize;
					rhs += vectorSize;
				}
			}
#endif
#if JSONIFIER_CHECK_FOR_INSTRUCTION(JSONIFIER_AVX512) || JSONIFIER_CHECK_FOR_INSTRUCTION(JSONIFIER_AVX2)
			if (lengthNew >= 32) {
				using simd_type					   = typename get_type_at_index<simd_internal::avx_list, 1>::type::type;
				static constexpr size_t vectorSize = get_type_at_index<simd_internal::avx_list, 1>::type::bytesProcessed;
				static constexpr size_t mask	   = get_type_at_index<simd_internal::avx_list, 1>::type::mask;
				simd_type value01, value02;
				while (lengthNew >= vectorSize) {
					value01 = simd_internal::gatherValuesU<simd_type>(lhs);
					value02 = simd_internal::gatherValuesU<simd_type>(rhs);
					if (simd_internal::opCmpEq(value01, value02) != mask) {
						return false;
					};
					lengthNew -= vectorSize;
					lhs += vectorSize;
					rhs += vectorSize;
				}
			}
#endif
#if JSONIFIER_CHECK_FOR_INSTRUCTION(JSONIFIER_AVX512) || JSONIFIER_CHECK_FOR_INSTRUCTION(JSONIFIER_AVX2) || JSONIFIER_CHECK_FOR_INSTRUCTION(JSONIFIER_AVX) || \
	JSONIFIER_CHECK_FOR_INSTRUCTION(JSONIFIER_NEON)
			if (lengthNew >= 16) {
				using simd_type					   = typename get_type_at_index<simd_internal::avx_list, 0>::type::type;
				static constexpr size_t vectorSize = get_type_at_index<simd_internal::avx_list, 0>::type::bytesProcessed;
				static constexpr size_t mask	   = get_type_at_index<simd_internal::avx_list, 0>::type::mask;
				simd_type value01, value02;
				while (lengthNew >= vectorSize) {
					value01 = simd_internal::gatherValuesU<simd_type>(lhs);
					value02 = simd_internal::gatherValuesU<simd_type>(rhs);
					if (simd_internal::opCmpEq(value01, value02) != mask) {
						return false;
					};
					lengthNew -= vectorSize;
					lhs += vectorSize;
					rhs += vectorSize;
				}
			}
#endif
			{
				static constexpr size_t n{ sizeof(size_t) };
				if (lengthNew >= n) {
					size_t v[2];
					while (lengthNew >= n) {
						std::memcpy(v, lhs, n);
						std::memcpy(v + 1, rhs, n);
						if (v[0] != v[1]) {
							return false;
						}
						lengthNew -= n;
						lhs += n;
						rhs += n;
					}
				}
			}
			{
				static constexpr size_t n{ sizeof(uint32_t) };
				if (lengthNew >= n) {
					uint32_t v[2];
					std::memcpy(v, lhs, n);
					std::memcpy(v + 1, rhs, n);
					if (v[0] != v[1]) {
						return false;
					}
					lengthNew -= n;
					lhs += n;
					rhs += n;
				}
			}
			{
				static constexpr size_t n{ sizeof(uint16_t) };
				if (lengthNew >= n) {
					uint16_t v[2];
					std::memcpy(v, lhs, n);
					std::memcpy(v + 1, rhs, n);
					if (v[0] != v[1]) {
						return false;
					}
					lengthNew -= n;
					lhs += n;
					rhs += n;
				}
			}
			if (lengthNew && *lhs != *rhs) {
				return false;
			}
			return true;
		}
	};

};
