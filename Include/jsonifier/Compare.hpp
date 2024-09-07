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

namespace jsonifier_internal {

	template<const uint8_t repeat, jsonifier::concepts::uint16_type return_type> JSONIFIER_ALWAYS_INLINE constexpr return_type repeatByte() noexcept {
		return 0x0101ull * repeat;
	}

	template<const uint8_t repeat, jsonifier::concepts::uint32_type return_type> JSONIFIER_ALWAYS_INLINE constexpr return_type repeatByte() noexcept {
		return 0x01010101ull * repeat;
	}

	template<const uint8_t repeat, jsonifier::concepts::uint64_type return_type> JSONIFIER_ALWAYS_INLINE constexpr return_type repeatByte() noexcept {
		return 0x0101010101010101ull * repeat;
	}

	template<char value, typename char_type> struct char_comparison {
		JSONIFIER_ALWAYS_INLINE static const char_type* memchar(const char_type* data, size_t lengthNew) noexcept {
#if JSONIFIER_CHECK_FOR_AVX(JSONIFIER_AVX512)
			{
				using simd_type						 = typename get_type_at_index<simd_internal::avx_list, 2>::type::type;
				using integer_type					 = typename get_type_at_index<simd_internal::avx_list, 2>::type::integer_type;
				static constexpr uint64_t vectorSize = get_type_at_index<simd_internal::avx_list, 0>::type::bytesProcessed;
				const simd_type search_value		 = simd_internal::gatherValue<simd_type>(static_cast<uint8_t>(value));
				integer_type mask;
				simd_type chunk;
				while (lengthNew >= vectorSize) {
					chunk = simd_internal::gatherValuesU<simd_type>(data);
					mask  = simd_internal::opCmpEq(chunk, search_value);
					if (mask != 0) [[unlikely]] {
						data += simd_internal::tzcnt(mask);
						return data;
					}
					lengthNew -= vectorSize;
					data += vectorSize;
				}
			}
#endif

#if JSONIFIER_CHECK_FOR_AVX(JSONIFIER_AVX2)
			{
				using simd_type						 = typename get_type_at_index<simd_internal::avx_list, 1>::type::type;
				using integer_type					 = typename get_type_at_index<simd_internal::avx_list, 1>::type::integer_type;
				static constexpr uint64_t vectorSize = get_type_at_index<simd_internal::avx_list, 0>::type::bytesProcessed;
				const simd_type search_value		 = simd_internal::gatherValue<simd_type>(static_cast<uint8_t>(value));
				integer_type mask;
				simd_type chunk;
				while (lengthNew >= vectorSize) {
					chunk = simd_internal::gatherValuesU<simd_type>(data);
					mask  = simd_internal::opCmpEq(chunk, search_value);
					if (mask != 0) [[unlikely]] {
						data += simd_internal::tzcnt(mask);
						return data;
					}
					lengthNew -= vectorSize;
					data += vectorSize;
				}
			}
#endif

#if JSONIFIER_CHECK_FOR_AVX(JSONIFIER_AVX) || JSONIFIER_CHECK_FOR_INSTRUCTION(JSONIFIER_NEON)
			{
				using simd_type						 = typename get_type_at_index<simd_internal::avx_list, 0>::type::type;
				using integer_type					 = typename get_type_at_index<simd_internal::avx_list, 0>::type::integer_type;
				static constexpr uint64_t vectorSize = get_type_at_index<simd_internal::avx_list, 0>::type::bytesProcessed;
				const simd_type search_value		 = simd_internal::gatherValue<simd_type>(static_cast<uint8_t>(value));
				integer_type mask;
				simd_type chunk;
				while (lengthNew >= vectorSize) {
					chunk = simd_internal::gatherValuesU<simd_type>(data);
					mask  = simd_internal::opCmpEq(chunk, search_value);
					if (mask != 0) [[unlikely]] {
						data += simd_internal::tzcnt(mask);
						return data;
					}
					lengthNew -= vectorSize;
					data += vectorSize;
				}
			}
#endif
			{
				static constexpr uint64_t mask64  = repeatByte<0b01111111, uint64_t>();
				static constexpr uint64_t value64 = repeatByte<value, uint64_t>();
				static constexpr uint64_t hiBit	  = repeatByte<0b10000000, uint64_t>();
				uint64_t simdValue, lo7, quote, t0, next;

				while (lengthNew >= 8) {
					std::memcpy(&simdValue, data, 8);

					lo7	  = simdValue & mask64;
					quote = (lo7 ^ value64) + mask64;
					t0	  = ~(quote | simdValue);
					next  = simd_internal::tzcnt(static_cast<uint64_t>(t0 & hiBit)) >> 3u;

					if (next != 8) [[unlikely]] {
						data += next;
						return data;
					}
					data += 8;
					lengthNew -= 8;
				}
			}

			if (lengthNew >= 4) {
				static constexpr uint32_t mask32  = repeatByte<0b01111111, uint32_t>();
				static constexpr uint32_t value32 = repeatByte<value, uint32_t>();
				static constexpr uint32_t hiBit	  = repeatByte<0b10000000, uint32_t>();
				uint32_t simdValue, lo7, quote, t0, next;
				std::memcpy(&simdValue, data, sizeof(uint32_t));

				lo7	  = simdValue & mask32;
				quote = (lo7 ^ value32) + mask32;
				t0	  = ~(quote | simdValue);
				next  = simd_internal::tzcnt(static_cast<uint32_t>(t0 & hiBit)) >> 3u;

				if (next != 4) [[unlikely]] {
					data += next;
					return data;
				}
				data += 4;
				lengthNew -= 4;
			}

			if (lengthNew >= 2) {
				static constexpr uint16_t mask16  = repeatByte<0b01111111, uint16_t>();
				static constexpr uint16_t value16 = repeatByte<value, uint16_t>();
				static constexpr uint16_t hiBit	  = repeatByte<0b10000000, uint16_t>();
				uint16_t simdValue, lo7, quote, t0, next;
				std::memcpy(&simdValue, data, sizeof(uint16_t));

				lo7	  = simdValue & mask16;
				quote = (lo7 ^ value16) + mask16;
				t0	  = ~(quote | simdValue);
				next  = simd_internal::tzcnt(static_cast<uint16_t>(t0 & hiBit)) >> 3u;

				if (next != 2) [[unlikely]] {
					data += next;
					return data;
				}
				data += 2;
				lengthNew -= 2;
			}

			if (lengthNew > 0 && *data == value) {
				++data;
				return data;
			}

			return static_cast<const char_type*>(nullptr);
		}
	};

	template<typename char_type01, typename char_type02> JSONIFIER_ALWAYS_INLINE bool compare(char_type01* lhs, char_type02* rhs, uint64_t lengthNew) noexcept {
#if JSONIFIER_CHECK_FOR_AVX(JSONIFIER_AVX512)
		{
			using simd_type						 = typename get_type_at_index<simd_internal::avx_list, 2>::type::type;
			static constexpr uint64_t vectorSize = get_type_at_index<simd_internal::avx_list, 2>::type::bytesProcessed;
			simd_type value01, value02;
			while (lengthNew >= vectorSize) {
				value01 = simd_internal::gatherValuesU<simd_type>(lhs);
				value02 = simd_internal::gatherValuesU<simd_type>(rhs);
				if (!simd_internal::opCmpEq(value01, value02)) {
					return false;
				};
				lengthNew -= vectorSize;
				lhs += vectorSize;
				rhs += vectorSize;
			}
		}
#endif

#if JSONIFIER_CHECK_FOR_AVX(JSONIFIER_AVX2)
		{
			using simd_type						 = typename get_type_at_index<simd_internal::avx_list, 1>::type::type;
			static constexpr uint64_t vectorSize = get_type_at_index<simd_internal::avx_list, 1>::type::bytesProcessed;
			simd_type value01, value02;
			while (lengthNew >= vectorSize) {
				value01 = simd_internal::gatherValuesU<simd_type>(lhs);
				value02 = simd_internal::gatherValuesU<simd_type>(rhs);
				if (!simd_internal::opCmpEq(value01, value02)) {
					return false;
				};
				lengthNew -= vectorSize;
				lhs += vectorSize;
				rhs += vectorSize;
			}
		}
#endif

#if JSONIFIER_CHECK_FOR_AVX(JSONIFIER_AVX) || JSONIFIER_CHECK_FOR_INSTRUCTION(JSONIFIER_NEON)
		{
			using simd_type						 = typename get_type_at_index<simd_internal::avx_list, 0>::type::type;
			static constexpr uint64_t vectorSize = get_type_at_index<simd_internal::avx_list, 0>::type::bytesProcessed;
			simd_type value01, value02;
			while (lengthNew >= vectorSize) {
				value01 = simd_internal::gatherValuesU<simd_type>(lhs);
				value02 = simd_internal::gatherValuesU<simd_type>(rhs);
				if (!simd_internal::opCmpEq(value01, value02)) {
					return false;
				};
				lengthNew -= vectorSize;
				lhs += vectorSize;
				rhs += vectorSize;
			}
		}
#endif

		if (lengthNew > 7) {
			static constexpr uint64_t n{ sizeof(uint64_t) };
			uint64_t v[2];
			while (lengthNew > n) {
				std::memcpy(v, lhs, n);
				std::memcpy(v + 1, rhs, n);
				if (v[0] != v[1]) {
					return false;
				}
				lengthNew -= n;
				lhs += n;
				rhs += n;
			}

			auto shift = n - lengthNew;
			lhs -= shift;
			rhs -= shift;

			std::memcpy(v, lhs, 8);
			std::memcpy(v + 1, rhs, 8);
			return v[0] == v[1];
		}
		{
			static constexpr uint64_t n{ sizeof(uint32_t) };
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
			static constexpr uint64_t n{ sizeof(uint16_t) };
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

	template<uint64_t count, typename char_type01, typename char_type02> struct comparison {
		JSONIFIER_ALWAYS_INLINE static bool compare(const char_type01* lhs, const char_type02* rhs) noexcept {
			size_t lengthNew{ count };
#if JSONIFIER_CHECK_FOR_AVX(JSONIFIER_AVX512)
			if constexpr (count >= 64) {
				{
					using simd_type						 = typename get_type_at_index<simd_internal::avx_list, 2>::type::type;
					static constexpr uint64_t vectorSize = get_type_at_index<simd_internal::avx_list, 2>::type::bytesProcessed;
					simd_type value01, value02;
					while (lengthNew >= vectorSize) {
						value01 = simd_internal::gatherValues<simd_type>(lhs);
						value02 = simd_internal::gatherValuesU<simd_type>(rhs);
						if (!simd_internal::opCmpEq(value01, value02)) {
							return false;
						};
						lengthNew -= vectorSize;
						lhs += vectorSize;
						rhs += vectorSize;
					}
				}
			}
#endif
			constexpr size_t newCount01{ count % 64 };

#if JSONIFIER_CHECK_FOR_AVX(JSONIFIER_AVX2)
			if constexpr (newCount01 >= 32) {
				{
					using simd_type						 = typename get_type_at_index<simd_internal::avx_list, 1>::type::type;
					static constexpr uint64_t vectorSize = get_type_at_index<simd_internal::avx_list, 1>::type::bytesProcessed;
					simd_type value01, value02;
					while (lengthNew >= vectorSize) {
						value01 = simd_internal::gatherValues<simd_type>(lhs);
						value02 = simd_internal::gatherValuesU<simd_type>(rhs);
						if (!simd_internal::opCmpEq(value01, value02)) {
							return false;
						};
						lengthNew -= vectorSize;
						lhs += vectorSize;
						rhs += vectorSize;
					}
				}
			}
#endif
			constexpr size_t newCount02{ newCount01 % 32 };

#if JSONIFIER_CHECK_FOR_AVX(JSONIFIER_AVX) || JSONIFIER_CHECK_FOR_INSTRUCTION(JSONIFIER_NEON)
			if constexpr (newCount02 >= 16) {
				{
					using simd_type						 = typename get_type_at_index<simd_internal::avx_list, 0>::type::type;
					static constexpr uint64_t vectorSize = get_type_at_index<simd_internal::avx_list, 0>::type::bytesProcessed;
					simd_type value01, value02;
					while (lengthNew >= vectorSize) {
						value01 = simd_internal::gatherValues<simd_type>(lhs);
						value02 = simd_internal::gatherValuesU<simd_type>(rhs);
						if (!simd_internal::opCmpEq(value01, value02)) {
							return false;
						};
						lengthNew -= vectorSize;
						lhs += vectorSize;
						rhs += vectorSize;
					}
				}
			}
#endif

			constexpr size_t newCount03{ newCount02 % 16 };
			if constexpr (newCount03 > 8) {
				uint64_t v[2];
				while (lengthNew > 8) {
					std::memcpy(v, lhs, sizeof(uint64_t));
					std::memcpy(v + 1, rhs, sizeof(uint64_t));
					if (v[0] != v[1]) {
						return false;
					}
					lengthNew -= 8;
					lhs += 8;
					rhs += 8;
				}

				const auto shift = 8 - lengthNew;
				lhs -= shift;
				rhs -= shift;
				std::memcpy(v, lhs, sizeof(uint64_t));
				std::memcpy(v + 1, rhs, sizeof(uint64_t));
				return v[0] == v[1];
			} else if constexpr (newCount03 == 8) {
				uint64_t v[2];
				std::memcpy(v, lhs, sizeof(uint64_t));
				std::memcpy(v + 1, rhs, sizeof(uint64_t));
				return v[0] == v[1];
			} else if constexpr (newCount03 > 4) {
				uint64_t v[2]{};
				std::memcpy(v, lhs, newCount03);
				std::memcpy(v + 1, rhs, newCount03);
				return v[0] == v[1];
			} else if constexpr (newCount03 == 4) {
				uint32_t v[2];
				std::memcpy(v, lhs, newCount03);
				std::memcpy(v + 1, rhs, newCount03);
				return v[0] == v[1];
			} else if constexpr (newCount03 == 3) {
				uint32_t v[2]{};
				std::memcpy(v, lhs, newCount03);
				std::memcpy(v + 1, rhs, newCount03);
				return v[0] == v[1];
			} else if constexpr (newCount03 == 2) {
				uint16_t v[2];
				std::memcpy(v, lhs, newCount03);
				std::memcpy(v + 1, rhs, newCount03);
				return v[0] == v[1];
			} else if constexpr (newCount03 == 1) {
				return *lhs == *rhs;
			} else {
				return true;
			}
		}
	};

}// namespace jsonifier_internal
