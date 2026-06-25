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

#include <jsonifier-incl/simd/neon.hpp>

namespace jsonifier {

#if JSONIFIER_CHECK_FOR_INSTRUCTION(JSONIFIER_NEON)

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
			bitmask ^= bitmask << 1;
			bitmask ^= bitmask << 2;
			bitmask ^= bitmask << 4;
			bitmask ^= bitmask << 8;
			bitmask ^= bitmask << 16;
			bitmask ^= bitmask << 32;
			return bitmask;
		}
	};

	template<typename derived_type> struct scalar_follow_op {
		uint64_t prevScalar{};

		JSONIFIER_INLINE uint64_t impl(uint64_t nonquoteScalar) noexcept {
			const uint64_t shifted = (nonquoteScalar << 1) | prevScalar;
			prevScalar			   = nonquoteScalar >> 63;
			return shifted;
		}
	};

	template<typename derived_type> struct escape_scanner_op {
		uint64_t nextIsEscaped{};

		JSONIFIER_INLINE uint64_t nextEscapeAndTerminalCode(uint64_t potentialEscape) noexcept {
			static constexpr uint64_t oddBits{ 0xAAAAAAAAAAAAAAAAULL };
			const uint64_t maybeEscaped				 = potentialEscape << 1;
			const uint64_t maybeEscapedAndOddBits	 = maybeEscaped | oddBits;
			const uint64_t evenSeriesCodesAndOddBits = maybeEscapedAndOddBits - potentialEscape;
			return evenSeriesCodesAndOddBits ^ oddBits;
		}

		JSONIFIER_INLINE uint64_t impl(uint64_t backslash_local) noexcept {
			if (!backslash_local) {
				const uint64_t escaped = nextIsEscaped;
				nextIsEscaped		   = 0;
				return escaped;
			}
			const uint64_t escapeAndTerminalCode = nextEscapeAndTerminalCode(backslash_local & ~nextIsEscaped);
			const uint64_t escaped				 = escapeAndTerminalCode ^ (backslash_local | nextIsEscaped);
			nextIsEscaped						 = (escapeAndTerminalCode & backslash_local) >> 63;
			return escaped;
		}
	};

	template<typename derived_type> struct mask_stitcher_op {
		JSONIFIER_INLINE static uint64_t impl(const jsonifier_simd_int_t (&lhs)[registersPerSixtyFourBits], const jsonifier_simd_int_t& rhsBroadcast) noexcept {
			uint64_t result = simd::opCmpEqBitMask(lhs[0], rhsBroadcast);
			if constexpr (registersPerSixtyFourBits > 1) {
				result |= static_cast<uint64_t>(simd::opCmpEqBitMask(lhs[1], rhsBroadcast)) << get_shift_amount(1);
				if constexpr (registersPerSixtyFourBits > 2) {
					result |= static_cast<uint64_t>(simd::opCmpEqBitMask(lhs[2], rhsBroadcast)) << get_shift_amount(2);
					result |= static_cast<uint64_t>(simd::opCmpEqBitMask(lhs[3], rhsBroadcast)) << get_shift_amount(3);
				}
			}
			return result;
		}

		JSONIFIER_INLINE static uint64_t impl(const jsonifier_simd_int_t (&lhs)[registersPerSixtyFourBits], const jsonifier_simd_int_t (&rhs)[registersPerSixtyFourBits]) noexcept {
			uint64_t result = simd::opCmpEqBitMask(lhs[0], rhs[0]);
			if constexpr (registersPerSixtyFourBits > 1) {
				result |= static_cast<uint64_t>(simd::opCmpEqBitMask(lhs[1], rhs[1])) << get_shift_amount(1);
				if constexpr (registersPerSixtyFourBits > 2) {
					result |= static_cast<uint64_t>(simd::opCmpEqBitMask(lhs[2], rhs[2])) << get_shift_amount(2);
					result |= static_cast<uint64_t>(simd::opCmpEqBitMask(lhs[3], rhs[3])) << get_shift_amount(3);
				}
			}
			return result;
		}
	};

	template<typename derived_type> struct tape_writer_op {
		JSONIFIER_INLINE static uint32_t extractIndex(uint64_t base, uint64_t bits) noexcept {
			return static_cast<uint32_t>(simd::tzcnt(bits) + base);
		}

		JSONIFIER_INLINE static uint64_t advance(uint64_t bits) noexcept {
			return blsr(bits);
		}

		JSONIFIER_INLINE static uint64_t correctedPopcount(uint64_t bits) noexcept {
			return static_cast<uint64_t>(popcnt(bits));
		}
	};

#endif

}
