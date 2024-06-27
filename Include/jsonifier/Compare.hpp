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

#include <jsonifier/Config.hpp>

namespace jsonifier_internal {

	template<const uint8_t repeat, jsonifier::concepts::uint16_type return_type> constexpr return_type repeatByte() {
		return 0x0101ull * repeat;
	}

	template<const uint8_t repeat, jsonifier::concepts::uint32_type return_type> constexpr return_type repeatByte() {
		return 0x01010101ull * repeat;
	}

	template<const uint8_t repeat, jsonifier::concepts::uint64_type return_type> constexpr return_type repeatByte() {
		return 0x0101010101010101ull * repeat;
	}

	template<auto value> JSONIFIER_INLINE void memchar(const char*& data, size_t lengthNew) {
#if JSONIFIER_CHECK_FOR_AVX(JSONIFIER_AVX512)
		{
			using simd_type						 = typename jsonifier::concepts::get_type_at_index<simd_internal::avx_list, 2>::type::type;
			static constexpr uint64_t vectorSize = jsonifier::concepts::get_type_at_index<simd_internal::avx_list, 2>::type::bytesProcessed;
			simd_type search_value{ simd_internal::gatherValue<simd_type>(static_cast<uint8_t>(value)) }, chunk;
			while (lengthNew >= vectorSize) {
				chunk	  = simd_internal::gatherValuesU<simd_type>(data);
				auto mask = simd_internal::opCmpEq(chunk, search_value);
				if (mask != 0) {
					data += simd_internal::tzcnt(mask);
					return;
				};
				lengthNew -= vectorSize;
				data += vectorSize;
			}
		}
#endif

#if JSONIFIER_CHECK_FOR_AVX(JSONIFIER_AVX2)
		{
			using simd_type						 = typename jsonifier::concepts::get_type_at_index<simd_internal::avx_list, 1>::type::type;
			static constexpr uint64_t vectorSize = jsonifier::concepts::get_type_at_index<simd_internal::avx_list, 1>::type::bytesProcessed;
			simd_type search_value{ simd_internal::gatherValue<simd_type>(static_cast<uint8_t>(value)) }, chunk;
			while (lengthNew >= vectorSize) {
				chunk	  = simd_internal::gatherValuesU<simd_type>(data);
				auto mask = simd_internal::opCmpEq(chunk, search_value);
				if (mask != 0) {
					data += simd_internal::tzcnt(mask);
					return;
				};
				lengthNew -= vectorSize;
				data += vectorSize;
			}
		}
#endif

#if JSONIFIER_CHECK_FOR_AVX(JSONIFIER_AVX) || JSONIFIER_CHECK_FOR_INSTRUCTION(JSONIFIER_NEON)
		{
			using simd_type						 = typename jsonifier::concepts::get_type_at_index<simd_internal::avx_list, 0>::type::type;
			static constexpr uint64_t vectorSize = jsonifier::concepts::get_type_at_index<simd_internal::avx_list, 0>::type::bytesProcessed;
			simd_type search_value{ simd_internal::gatherValue<simd_type>(static_cast<uint8_t>(value)) }, chunk;
			while (lengthNew >= vectorSize) {
				chunk	  = simd_internal::gatherValuesU<simd_type>(data);
				auto mask = simd_internal::opCmpEq(chunk, search_value);
				if (mask != 0) {
					data += simd_internal::tzcnt(mask);
					return;
				};
				lengthNew -= vectorSize;
				data += vectorSize;
			}
		}
#endif
		{
			static constexpr uint64_t mask64  = repeatByte<0b01111111, uint64_t>();
			static constexpr uint64_t value64 = repeatByte<value, uint64_t>();
			static constexpr uint64_t hiBit = repeatByte<0b10000000, uint64_t>();
			uint64_t simdValue, lo7, quote, t0, next;
			while (lengthNew >= 8) {
				std::memcpy(&simdValue, data, sizeof(uint64_t));

				lo7	  = simdValue & mask64;
				quote = (lo7 ^ value64) + mask64;
				t0	  = ~(quote | simdValue);
				next  = simd_internal::tzcnt(static_cast<uint64_t>(t0 & hiBit)) >> 3u;

				if (next != 8) {
					data += next;
					return;
				} else {
					data += 8;
					lengthNew -= 8;
				}
			}
		}
		{
			static constexpr uint32_t mask32  = repeatByte<0b01111111, uint32_t>();
			static constexpr uint32_t value32 = repeatByte<value, uint32_t>();
			static constexpr uint32_t hiBit = repeatByte<0b10000000, uint32_t>();
			uint32_t simdValue, lo7, quote, t0, next;
			if (lengthNew >= 4) {
				std::memcpy(&simdValue, data, sizeof(uint32_t));

				lo7	  = simdValue & mask32;
				quote = (lo7 ^ value32) + mask32;
				t0	  = ~(quote | simdValue);
				next  = simd_internal::tzcnt(static_cast<uint32_t>(t0 & hiBit)) >> 3u;

				if (next != 4) {
					data += next;
					return;
				} else {
					data += 4;
					lengthNew -= 4;
				}
			}
		}
		{
			static constexpr uint16_t mask16  = repeatByte<0b01111111, uint16_t>();
			static constexpr uint16_t value16 = repeatByte<value, uint16_t>();
			static constexpr uint16_t hiBit = repeatByte<0b10000000, uint16_t>();
			uint16_t simdValue, lo7, quote, t0, next;
			if (lengthNew >= 2) {
				std::memcpy(&simdValue, data, sizeof(uint16_t));

				lo7	  = static_cast<uint16_t>(simdValue & mask16);
				quote = static_cast<uint16_t>((lo7 ^ value16) + mask16);
				t0	  = static_cast<uint16_t>(~(quote | simdValue));
				next  = static_cast<uint16_t>(simd_internal::tzcnt(static_cast<uint16_t>(t0 & hiBit)) >> 3u);

				if (next != 2) {
					data += next;
					return;
				} else {
					data += 2;
					lengthNew -= 2;
				}
			}
		}

		if (lengthNew > 0 && *data == value) {
			return;
		}

		data = nullptr;
	}

	template<typename char_type01, typename char_type02> JSONIFIER_INLINE bool compare(char_type01* lhs, char_type02* rhs, uint64_t lengthNew) {
#if JSONIFIER_CHECK_FOR_AVX(JSONIFIER_AVX512)
		{
			using simd_type						 = typename jsonifier::concepts::get_type_at_index<simd_internal::avx_list, 2>::type::type;
			static constexpr uint64_t vectorSize = jsonifier::concepts::get_type_at_index<simd_internal::avx_list, 2>::type::bytesProcessed;
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
			using simd_type						 = typename jsonifier::concepts::get_type_at_index<simd_internal::avx_list, 1>::type::type;
			static constexpr uint64_t vectorSize = jsonifier::concepts::get_type_at_index<simd_internal::avx_list, 1>::type::bytesProcessed;
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
			using simd_type						 = typename jsonifier::concepts::get_type_at_index<simd_internal::avx_list, 0>::type::type;
			static constexpr uint64_t vectorSize = jsonifier::concepts::get_type_at_index<simd_internal::avx_list, 0>::type::bytesProcessed;
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

			std::memcpy(v, lhs, n);
			std::memcpy(v + 1, rhs, n);
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

	template<uint64_t count, class char_type> constexpr bool compare(const char_type* lhs, const char_type* rhs) noexcept {
		if constexpr (count > 8) {
			uint64_t countNew{ count };
			uint64_t v[2];
			while (countNew > 8) {
				std::memcpy(v, lhs, 8);
				std::memcpy(v + 1, rhs, 8);
				if (v[0] != v[1]) {
					return false;
				}
				countNew -= 8;
				lhs += 8;
				rhs += 8;
			}

			const auto shift = 8 - countNew;
			lhs -= shift;
			rhs -= shift;

			std::memcpy(v, lhs, 8);
			std::memcpy(v + 1, rhs, 8);
			return v[0] == v[1];
		} else if constexpr (count == 8) {
			uint64_t v[2];
			std::memcpy(v, lhs, count);
			std::memcpy(v + 1, rhs, count);
			return v[0] == v[1];
		} else if constexpr (count > 4) {
			uint64_t v[2]{};
			std::memcpy(v, lhs, count);
			std::memcpy(v + 1, rhs, count);
			return v[0] == v[1];
		} else if constexpr (count == 4) {
			uint32_t v[2];
			std::memcpy(v, lhs, count);
			std::memcpy(v + 1, rhs, count);
			return v[0] == v[1];
		} else if constexpr (count == 3) {
			uint32_t v[2]{};
			std::memcpy(v, lhs, count);
			std::memcpy(v + 1, rhs, count);
			return v[0] == v[1];
		} else if constexpr (count == 2) {
			uint16_t v[2];
			std::memcpy(v, lhs, count);
			std::memcpy(v + 1, rhs, count);
			return v[0] == v[1];
		} else if constexpr (count == 1) {
			return *lhs == *rhs;
		} else {
			return true;
		}
	}

}// namespace jsonifier_internal
