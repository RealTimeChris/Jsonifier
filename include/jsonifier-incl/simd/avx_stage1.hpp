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
#pragma once

#include <jsonifier-incl/containers/array.hpp>
#include <jsonifier-incl/simd/avx.hpp>

namespace jsonifier::simd {

#if JSONIFIER_CHECK_FOR_INSTRUCTION(JSONIFIER_AVX512) || JSONIFIER_CHECK_FOR_INSTRUCTION(JSONIFIER_AVX2) || JSONIFIER_CHECK_FOR_INSTRUCTION(JSONIFIER_AVX)

	static constexpr internal::array<uint64_t, registersPerSixtyFourBits> shift_amounts{ [] {
		internal::array<uint64_t, registersPerSixtyFourBits> return_value{};
		for (uint64_t x = 0; x < registersPerSixtyFourBits; ++x) {
			return_value[x] = bytesPerStep * x;
		}
		return return_value;
	}() };

	inline static consteval uint64_t get_shift_amount(uint64_t index) {
		return shift_amounts[index % shift_amounts.size()];
	}

	template<typename derived_type> struct prefix_xor_op {
		JSONIFIER_INLINE static uint64_t impl(uint64_t bitmask) noexcept {
			const __m128i all_ones = _mm_set1_epi8(static_cast<char>(-1));
			const __m128i result   = _mm_clmulepi64_si128(_mm_set_epi64x(0, static_cast<int64_t>(bitmask)), all_ones, 0);
			return static_cast<uint64_t>(_mm_cvtsi128_si64(result));
		}
	};

	struct cmp_eq_op {
		JSONIFIER_INLINE static uint64_t impl(simd_array lhs, jsonifier_simd_int_t rhsBroadcast) noexcept {
			uint64_t result = simd::opCmpEqBitMask(lhs.get_value<0>(), rhsBroadcast);
			if constexpr (registersPerSixtyFourBits > 1) {
				result |= static_cast<uint64_t>(simd::opCmpEqBitMask(lhs.get_value<1>(), rhsBroadcast)) << get_shift_amount(1);
				if constexpr (registersPerSixtyFourBits > 2) {
					result |= static_cast<uint64_t>(simd::opCmpEqBitMask(lhs.get_value<2>(), rhsBroadcast)) << get_shift_amount(2);
					result |= static_cast<uint64_t>(simd::opCmpEqBitMask(lhs.get_value<3>(), rhsBroadcast)) << get_shift_amount(3);
				}
			}
			return result;
		}

		JSONIFIER_INLINE static uint64_t impl(simd_array lhs, simd_array rhs) noexcept {
			uint64_t result = simd::opCmpEqBitMask(lhs.get_value<0>(), rhs.get_value<0>());
			if constexpr (registersPerSixtyFourBits > 1) {
				result |= static_cast<uint64_t>(simd::opCmpEqBitMask(lhs.get_value<1>(), rhs.get_value<1>())) << get_shift_amount(1);
				if constexpr (registersPerSixtyFourBits > 2) {
					result |= static_cast<uint64_t>(simd::opCmpEqBitMask(lhs.get_value<2>(), rhs.get_value<2>())) << get_shift_amount(2);
					result |= static_cast<uint64_t>(simd::opCmpEqBitMask(lhs.get_value<3>(), rhs.get_value<3>())) << get_shift_amount(3);
				}
			}
			return result;
		}
	};

	struct unescaped_collector {
		JSONIFIER_INLINE static uint64_t impl(simd_array ltRhs) noexcept {
			uint64_t result = static_cast<uint64_t>(opBitMask(ltRhs.get_value<0>()));
			if constexpr (registersPerSixtyFourBits > 1) {
				result |= static_cast<uint64_t>(opBitMask(ltRhs.get_value<1>())) << get_shift_amount(1);
				if constexpr (registersPerSixtyFourBits > 2) {
					result |= static_cast<uint64_t>(opBitMask(ltRhs.get_value<2>())) << get_shift_amount(2);
					result |= static_cast<uint64_t>(opBitMask(ltRhs.get_value<3>())) << get_shift_amount(3);
				}
			}
			return result;
		}
	};

	template<typename rope_block> struct rope_detector {
		uint64_t nextIsEscaped{};
		uint64_t prevInString{};
		uint64_t prevScalar{};

		JSONIFIER_INLINE rope_block next(simd_array in_01, jsonifier_simd_int_t bsRegister, jsonifier_simd_int_t quoteRegister) noexcept {
			const uint64_t backslash_local = simd::cmp_eq_op::impl(in_01, bsRegister);
			const uint64_t quotes_local	   = simd::cmp_eq_op::impl(in_01, quoteRegister);
			const uint64_t escaped		   = nextEscapeAndTerminalCode(backslash_local);
			const uint64_t quotes		   = (quotes_local & ~escaped);
			const uint64_t inString		   = simd::prefix_xor_op<rope_detector>::impl(quotes) ^ prevInString;
			prevInString				   = static_cast<uint64_t>(static_cast<int64_t>(inString) >> 63);
			return { escaped, quotes, inString };
		}

		JSONIFIER_INLINE uint64_t nextEscapeAndTerminalCodeImpl(uint64_t potentialEscape) noexcept {
			static constexpr uint64_t oddBits{ 0xAAAAAAAAAAAAAAAAULL };
			const uint64_t maybeEscaped				 = potentialEscape << 1;
			const uint64_t maybeEscapedAndOddBits	 = maybeEscaped | oddBits;
			const uint64_t evenSeriesCodesAndOddBits = maybeEscapedAndOddBits - potentialEscape;
			return evenSeriesCodesAndOddBits ^ oddBits;
		}

		JSONIFIER_INLINE uint64_t nextEscapeAndTerminalCode(uint64_t backslash_local) noexcept {
			if (!backslash_local) {
				const uint64_t escaped = nextIsEscaped;
				nextIsEscaped		   = 0;
				return escaped;
			}
			const uint64_t escapeAndTerminalCode = nextEscapeAndTerminalCodeImpl(backslash_local & ~nextIsEscaped);
			const uint64_t escaped				 = escapeAndTerminalCode ^ (backslash_local | nextIsEscaped);
			nextIsEscaped						 = (escapeAndTerminalCode & backslash_local) >> 63;
			return escaped;
		}

		JSONIFIER_INLINE uint64_t followsNonquoteScalar(uint64_t nonquoteScalar) noexcept {
			const uint64_t shifted = (nonquoteScalar << 1) | prevScalar;
			prevScalar			   = nonquoteScalar >> 63;
			return shifted;
		}
	};

	struct ws_collector {
		JSONIFIER_INLINE static uint64_t impl(simd_array in_01, jsonifier_simd_int_t whitespaceTableLocal) noexcept {
			simd_array wsShuffle;
			wsShuffle.assign_value<0>(simd::opShuffle(whitespaceTableLocal, in_01.get_value<0>()));
			if constexpr (registersPerSixtyFourBits > 1) {
				wsShuffle.assign_value<1>(simd::opShuffle(whitespaceTableLocal, in_01.get_value<1>()));
				if constexpr (registersPerSixtyFourBits > 2) {
					wsShuffle.assign_value<2>(simd::opShuffle(whitespaceTableLocal, in_01.get_value<2>()));
					wsShuffle.assign_value<3>(simd::opShuffle(whitespaceTableLocal, in_01.get_value<3>()));
				}
			}
			return cmp_eq_op::impl(in_01, wsShuffle);
		}
	};

	struct op_collector {
		JSONIFIER_INLINE static uint64_t impl(simd_array in_01, jsonifier_simd_int_t opTable, jsonifier_simd_int_t spaceMask) noexcept {
			simd_array orLhs;
			simd_array shuffleRhs;

			orLhs.assign_value<0>(simd::opOr(in_01.get_value<0>(), spaceMask));
			shuffleRhs.assign_value<0>(simd::opShuffle(opTable, in_01.get_value<0>()));
			if constexpr (registersPerSixtyFourBits > 1) {
				orLhs.assign_value<1>(simd::opOr(in_01.get_value<1>(), spaceMask));
				shuffleRhs.assign_value<1>(simd::opShuffle(opTable, in_01.get_value<1>()));
				if constexpr (registersPerSixtyFourBits > 2) {
					orLhs.assign_value<2>(simd::opOr(in_01.get_value<2>(), spaceMask));
					shuffleRhs.assign_value<2>(simd::opShuffle(opTable, in_01.get_value<2>()));
					orLhs.assign_value<3>(simd::opOr(in_01.get_value<3>(), spaceMask));
					shuffleRhs.assign_value<3>(simd::opShuffle(opTable, in_01.get_value<3>()));
				}
			}
			return cmp_eq_op::impl(orLhs, shuffleRhs);
		}
	};

	template<typename derived_type> struct tape_writer_op {
		JSONIFIER_INLINE static uint32_t extractIndex(uint64_t base, uint64_t bits) noexcept {
			return static_cast<uint32_t>(simd::postCmpTzcnt(bits) + base);
		}

		JSONIFIER_INLINE static uint64_t advance(uint64_t bits) noexcept {
			return blsr(bits);
		}

		JSONIFIER_INLINE static uint64_t correctedPopcount(uint64_t bits) noexcept {
			return static_cast<uint64_t>(popcnt(bits));
		}
	};

	template<uint64_t size> inline constexpr internal::array<uint8_t, size> generateEscapeableArray00() {
		constexpr const uint8_t values[]{ 0x00u, 0x00u, '"', 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, '\\', 0x00u, 0x00u, 0x00u };
		internal::array<uint8_t, size> returnValues{};
		for (uint64_t x = 0; x < size; ++x) {
			returnValues[x] = values[x % 16];
		}
		return returnValues;
	};

	template<uint64_t size> JSONIFIER_ALIGN(bytesPerStep) inline constexpr internal::array<uint8_t, size> escapeableArray00{ generateEscapeableArray00<size>() };

	template<uint64_t size> inline constexpr internal::array<uint8_t, size> generateEscapeableArray01() {
		constexpr const uint8_t values[]{ 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, '\b', 0x00u, 0x00u, 0x00u, 0x0Cu, '\r', 0x00u, 0x00u };
		internal::array<uint8_t, size> returnValues{};
		for (uint64_t x = 0; x < size; ++x) {
			returnValues[x] = values[x % 16];
		}
		return returnValues;
	};

	template<uint64_t size> JSONIFIER_ALIGN(bytesPerStep) inline constexpr internal::array<uint8_t, size> escapeableArray01{ generateEscapeableArray01<size>() };

	template<uint64_t size> inline constexpr internal::array<uint8_t, size> generateWhitespaceArray() {
		constexpr const uint8_t values[]{ 0x20u, 0x64u, 0x64u, 0x64u, 0x11u, 0x64u, 0x71u, 0x02u, 0x64u, '\t', '\n', 0x70u, 0x64u, '\r', 0x64u, 0x64u };
		internal::array<uint8_t, size> returnValues{};
		for (uint64_t x = 0; x < size; ++x) {
			returnValues[x] = values[x % 16];
		}
		return returnValues;
	};

	template<uint64_t size> JSONIFIER_ALIGN(bytesPerStep) inline constexpr internal::array<uint8_t, size> whitespaceArray{ generateWhitespaceArray<size>() };

	template<uint64_t size> inline constexpr internal::array<uint8_t, size> generateOpArray() {
		constexpr const uint8_t values[]{ 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, ':', '{', ',', '}', 0x00u, 0x00u };
		internal::array<uint8_t, size> returnValues{};
		for (uint64_t x = 0; x < size; ++x) {
			returnValues[x] = values[x % 16];
		}
		return returnValues;
	};

	template<uint64_t size> JSONIFIER_ALIGN(bytesPerStep) inline constexpr internal::array<uint8_t, size> opArray{ generateOpArray<size>() };

#endif

}
