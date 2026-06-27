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
/// The code below drew heavy inspiration from Dr. Lemire's library, simdjson (https://github.com/simdjson/simdjson)
/// https://github.com/RealTimeChris/jsonifier
#pragma once

#include <jsonifier-incl/simd/neon.hpp>

namespace jsonifier::simd {

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

	struct unescaped_collector {
		JSONIFIER_INLINE static uint64_t impl(simd_array in_01) noexcept {
			static constexpr uint8x16_t bit_mask{ 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80 };
			uint8x16_t sum0 = vpaddq_u8(vandq_u8(in_01.get_value<0>(), bit_mask), vandq_u8(in_01.get_value<1>(), bit_mask));
			uint8x16_t sum1 = vpaddq_u8(vandq_u8(in_01.get_value<2>(), bit_mask), vandq_u8(in_01.get_value<3>(), bit_mask));
			sum0			= vpaddq_u8(sum0, sum1);
			sum0			= vpaddq_u8(sum0, sum0);
			return vgetq_lane_u64(vreinterpretq_u64_u8(sum0), 0);
		}
	};

	struct ws_collector {
		JSONIFIER_INLINE static uint64_t impl(simd_array in_01, jsonifier_simd_int_t whitespaceTableLocal) noexcept {
			const uint8x16_t d0_0 = in_01.get_value<0>();
			const uint8x16_t d0_1 = in_01.get_value<1>();
			const uint8x16_t d0_2 = in_01.get_value<2>();
			const uint8x16_t d0_3 = in_01.get_value<3>();
			const uint8x16_t match_ws_0 = vqtbx1q_u8(vceqq_u8(d0_0, vdupq_n_u8(' ')), whitespaceTableLocal, d0_0);
			const uint8x16_t match_ws_1 = vqtbx1q_u8(vceqq_u8(d0_1, vdupq_n_u8(' ')), whitespaceTableLocal, d0_1);
			const uint8x16_t match_ws_2 = vqtbx1q_u8(vceqq_u8(d0_2, vdupq_n_u8(' ')), whitespaceTableLocal, d0_2);
			const uint8x16_t match_ws_3 = vqtbx1q_u8(vceqq_u8(d0_3, vdupq_n_u8(' ')), whitespaceTableLocal, d0_3);
			static constexpr uint8x16_t bit_mask{ 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80 };
			uint8x16_t ws_sum0			= vpaddq_u8(vandq_u8(match_ws_0, bit_mask), vandq_u8(match_ws_1, bit_mask));
			uint8x16_t ws_sum1			= vpaddq_u8(vandq_u8(match_ws_2, bit_mask), vandq_u8(match_ws_3, bit_mask));
			ws_sum0						= vpaddq_u8(ws_sum0, ws_sum1);
			ws_sum0						= vpaddq_u8(ws_sum0, ws_sum0);
			return vgetq_lane_u64(vreinterpretq_u64_u8(ws_sum0), 0);
		}
	};

	struct op_collector {
		JSONIFIER_INLINE static uint64_t impl(simd_array in_01, jsonifier_simd_int_t opTable, jsonifier_simd_int_t) noexcept {
			const uint8x16_t d0_0		= in_01.get_value<0>();
			const uint8x16_t d0_1		= in_01.get_value<1>();
			const uint8x16_t d0_2		= in_01.get_value<2>();
			const uint8x16_t d0_3		= in_01.get_value<3>();
			const uint8x16_t match_op_0 = vceqq_u8(vqtbl1q_u8(opTable, vshrq_n_u8(vaddq_u8(d0_0, vdupq_n_u8(3)), 4)), d0_0);
			const uint8x16_t match_op_1 = vceqq_u8(vqtbl1q_u8(opTable, vshrq_n_u8(vaddq_u8(d0_1, vdupq_n_u8(3)), 4)), d0_1);
			const uint8x16_t match_op_2 = vceqq_u8(vqtbl1q_u8(opTable, vshrq_n_u8(vaddq_u8(d0_2, vdupq_n_u8(3)), 4)), d0_2);
			const uint8x16_t match_op_3 = vceqq_u8(vqtbl1q_u8(opTable, vshrq_n_u8(vaddq_u8(d0_3, vdupq_n_u8(3)), 4)), d0_3);
			static constexpr uint8x16_t bit_mask{ 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80 };
			uint8x16_t op_sum0 = vpaddq_u8(vandq_u8(match_op_0, bit_mask), vandq_u8(match_op_1, bit_mask));
			uint8x16_t op_sum1 = vpaddq_u8(vandq_u8(match_op_2, bit_mask), vandq_u8(match_op_3, bit_mask));
			op_sum0			   = vpaddq_u8(op_sum0, op_sum1);
			op_sum0			   = vpaddq_u8(op_sum0, op_sum0);
			return vgetq_lane_u64(vreinterpretq_u64_u8(op_sum0), 0);
		}
	};

	template<typename rope_block> struct rope_detector : rope_block {
		uint64_t nextIsEscaped{};
		uint64_t prevInString{};
		uint64_t prevScalar{};

		JSONIFIER_INLINE static uint64_t toBitmask(uint8x16_t match_0, uint8x16_t match_1, uint8x16_t match_2, uint8x16_t match_3, uint8x16_t bit_mask) noexcept {
			uint8x16_t sum0 = vpaddq_u8(vandq_u8(match_0, bit_mask), vandq_u8(match_1, bit_mask));
			uint8x16_t sum1 = vpaddq_u8(vandq_u8(match_2, bit_mask), vandq_u8(match_3, bit_mask));
			sum0 = vpaddq_u8(sum0, sum1);
			sum0 = vpaddq_u8(sum0, sum0);
			return vgetq_lane_u64(vreinterpretq_u64_u8(sum0), 0);
		}

		JSONIFIER_INLINE void finishNextNoInString() noexcept {
			rope_block::inString = prevInString;
		}

		JSONIFIER_INLINE void finishNextInString() noexcept {
			const uint64_t inString = simd::prefix_xor_op<rope_detector>::impl(rope_block::quotes) ^ prevInString;
			prevInString			= static_cast<uint64_t>(static_cast<int64_t>(inString) >> 63);
			rope_block::inString	= inString;
		}

		JSONIFIER_INLINE void next(simd_array in_01, jsonifier_simd_int_t bsRegister, jsonifier_simd_int_t quoteRegister) noexcept {
			static constexpr uint8x16_t bit_mask{ 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80 };
			const uint8x16_t d0 = in_01.get_value<0>();
			const uint8x16_t d1 = in_01.get_value<1>();
			const uint8x16_t d2 = in_01.get_value<2>();
			const uint8x16_t d3 = in_01.get_value<3>();
			const uint64_t backslash_local = toBitmask(vceqq_u8(d0, bsRegister), vceqq_u8(d1, bsRegister), vceqq_u8(d2, bsRegister), vceqq_u8(d3, bsRegister), bit_mask);
			const uint64_t quotes_local = toBitmask(vceqq_u8(d0, quoteRegister), vceqq_u8(d1, quoteRegister), vceqq_u8(d2, quoteRegister), vceqq_u8(d3, quoteRegister), bit_mask);
			const uint64_t escaped = nextEscapeAndTerminalCode(backslash_local);
			const uint64_t quotes = (quotes_local & ~escaped);
			rope_block::escaped = escaped;
			rope_block::quotes	= quotes;
			return quotes ? finishNextInString() : finishNextNoInString();
		}

		JSONIFIER_INLINE uint64_t nextEscapeAndTerminalCodeImpl(uint64_t potentialEscape) noexcept {
			static constexpr uint64_t oddBits{ 0xAAAAAAAAAAAAAAAAULL };
			const uint64_t maybeEscaped = potentialEscape << 1;
			const uint64_t maybeEscapedAndOddBits = maybeEscaped | oddBits;
			const uint64_t evenSeriesCodesAndOddBits = maybeEscapedAndOddBits - potentialEscape;
			return evenSeriesCodesAndOddBits ^ oddBits;
		}

		JSONIFIER_INLINE uint64_t nextEscapeAndTerminalCode(uint64_t backslash_local) noexcept {
			if (!backslash_local) {
				const uint64_t escaped = nextIsEscaped;
				nextIsEscaped = 0;
				return escaped;			
			}
			const uint64_t escapeAndTerminalCode = nextEscapeAndTerminalCodeImpl(backslash_local & ~nextIsEscaped);
			const uint64_t escaped = escapeAndTerminalCode ^ (backslash_local | nextIsEscaped);
			nextIsEscaped = (escapeAndTerminalCode & backslash_local) >> 63;
			return escaped;
		}

		JSONIFIER_INLINE uint64_t followsNonquoteScalar(uint64_t nonquoteScalar) noexcept {
			const uint64_t shifted = (nonquoteScalar << 1) | prevScalar;
			prevScalar = nonquoteScalar >> 63;
			return shifted;
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

	template<uint64_t size> inline constexpr internal::array<uint8_t, size> generateWhitespaceArrayNeon() {
		constexpr const uint8_t values[]{ 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0xFFu, 0xFFu, 0x00u, 0x00u, 0xFFu, 0x00u, 0x00u };
		internal::array<uint8_t, size> returnValues{};
		for (uint64_t x = 0; x < size; ++x) {
			returnValues[x] = values[x % 16];
		}
		return returnValues;
	};

	template<uint64_t size> JSONIFIER_ALIGN(bytesPerStep) inline constexpr internal::array<uint8_t, size> whitespaceArray{ generateWhitespaceArrayNeon<size>() };

	template<uint64_t size> inline constexpr internal::array<uint8_t, size> generateOpArrayNeon() {
		constexpr const uint8_t values[]{ 0xFFu, 0x00u, ',', ':', 0x00u, '[', ']', '{', '}', 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u };
		internal::array<uint8_t, size> returnValues{};
		for (uint64_t x = 0; x < size; ++x) {
			returnValues[x] = values[x % 16];
		}
		return returnValues;
	};

	template<uint64_t size> JSONIFIER_ALIGN(bytesPerStep) inline constexpr internal::array<uint8_t, size> opArray{ generateOpArrayNeon<size>() };

#endif

}
