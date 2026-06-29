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

#include <jsonifier-incl/containers/array.hpp>
#include <jsonifier-incl/simd/avx.hpp>

namespace jsonifier {

#if JSONIFIER_CHECK_FOR_INSTRUCTION(JSONIFIER_AVX512) || JSONIFIER_CHECK_FOR_INSTRUCTION(JSONIFIER_AVX2) || JSONIFIER_CHECK_FOR_INSTRUCTION(JSONIFIER_AVX)

	static constexpr uint8_t byte1HighTableRaw[]{ tooLong, tooLong, tooLong, tooLong, tooLong, tooLong, tooLong, tooLong, twoConts, twoConts, twoConts, twoConts,
		tooShort | overlong2, tooShort, tooShort | overlong3 | surrogate, tooShort | tooLarge | tooLarge1000 | overlong4 };

	JSONIFIER_ALIGN(bytesPerStep) static constexpr std::array<uint8_t, bytesPerStep> byte1HighTable{ [] {
		std::array<uint8_t, bytesPerStep> returnValue{};
		for (uint64_t x = 0; x < bytesPerStep; ++x) {
			returnValue[x] = byte1HighTableRaw[x % std::size(byte1HighTableRaw)];
		}
		return returnValue;
	}() };

	static constexpr uint8_t byte1LowTableRaw[]{ carry | overlong3 | overlong2 | overlong4, carry | overlong2, carry, carry, carry | tooLarge, carry | tooLarge | tooLarge1000,
		carry | tooLarge | tooLarge1000, carry | tooLarge | tooLarge1000, carry | tooLarge | tooLarge1000, carry | tooLarge | tooLarge1000, carry | tooLarge | tooLarge1000,
		carry | tooLarge | tooLarge1000, carry | tooLarge | tooLarge1000, carry | tooLarge | tooLarge1000 | surrogate, carry | tooLarge | tooLarge1000,
		carry | tooLarge | tooLarge1000 };

	JSONIFIER_ALIGN(bytesPerStep) static constexpr std::array<uint8_t, bytesPerStep> byte1LowTable{ [] {
		std::array<uint8_t, bytesPerStep> returnValue{};
		for (uint64_t x = 0; x < bytesPerStep; ++x) {
			returnValue[x] = byte1LowTableRaw[x % std::size(byte1LowTableRaw)];
		}
		return returnValue;
	}() };

	static constexpr uint8_t byte2HighTableRaw[]{ tooShort, tooShort, tooShort, tooShort, tooShort, tooShort, tooShort, tooShort,
		tooLong | overlong2 | twoConts | overlong3 | tooLarge1000 | overlong4, tooLong | overlong2 | twoConts | overlong3 | tooLarge,
		tooLong | overlong2 | twoConts | surrogate | tooLarge, tooLong | overlong2 | twoConts | surrogate | tooLarge, tooShort, tooShort, tooShort, tooShort };

	JSONIFIER_ALIGN(bytesPerStep) static constexpr std::array<uint8_t, bytesPerStep> byte2HighTable{ [] {
		std::array<uint8_t, bytesPerStep> return_value{};
		for (uint64_t x = 0; x < bytesPerStep; ++x) {
			return_value[x] = byte2HighTableRaw[x % std::size(byte2HighTableRaw)];
		}
		return return_value;
	}() };

	JSONIFIER_ALIGN(bytesPerStep) static constexpr uint8_t isIncompleteMax[64] = { 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
		255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
		255, 255, 255, 255, static_cast<uint8_t>(0xF0u - 1), static_cast<uint8_t>(0xE0u - 1), static_cast<uint8_t>(0xC0u - 1) };

	template<uint64_t n_blocks, uint64_t totalChunks> struct chunk_loader {
		template<uint64_t... i> static inline simd_array<totalChunks> loadImpl(const uint8_t* src, std::index_sequence<i...>) noexcept {
			simd_array<totalChunks> result;
			((result.template assignValue<i>(simd::gatherValuesU<jsonifier_simd_int_t>(std::bit_cast<const jsonifier_simd_int_t*>(src + i * bytesPerChunk)))), ...);
			return result;
		}

		static inline simd_array<totalChunks> load(const uint8_t* src) noexcept {
			return loadImpl(src, std::make_index_sequence<totalChunks>{});
		}
	};

	template<uint64_t n_blocks, uint64_t totalChunks> struct ascii_checker {
		template<uint64_t... i> static inline jsonifier_simd_int_t orImpl(const simd_array<totalChunks>& chunks, std::index_sequence<i...>) noexcept {
			jsonifier_simd_int_t result = chunks.template getValue<0>();
			((result = simd::opOr(result, chunks.template getValue<i + 1>())), ...);
			return result;
		}

		static inline jsonifier_simd_int_t orAll(const simd_array<totalChunks>& chunks) noexcept {
			return orImpl(chunks, std::make_index_sequence<totalChunks - 1>{});
		}
	};

	struct utf8_checker {
		jsonifier_simd_int_t prevIncomplete;
		jsonifier_simd_int_t prevInput;
		jsonifier_simd_int_t lookupH;
		jsonifier_simd_int_t lookupL;
		jsonifier_simd_int_t lookup2;
		jsonifier_simd_int_t error;

		JSONIFIER_INLINE void reset() {
			prevIncomplete = jsonifier_simd_int_t{};
			prevInput	   = jsonifier_simd_int_t{};
			error		   = jsonifier_simd_int_t{};
			lookupH		   = simd::gatherValues<jsonifier_simd_int_t>(std::bit_cast<const jsonifier_simd_int_t*>(byte1HighTable.data()));
			lookupL		   = simd::gatherValues<jsonifier_simd_int_t>(std::bit_cast<const jsonifier_simd_int_t*>(byte1LowTable.data()));
			lookup2		   = simd::gatherValues<jsonifier_simd_int_t>(std::bit_cast<const jsonifier_simd_int_t*>(byte2HighTable.data()));
		}

		JSONIFIER_INLINE jsonifier_simd_int_t prev1(jsonifier_simd_int_t cur, jsonifier_simd_int_t prev) {
			return simd::opAlignR<15>(cur, simd::opPermute<0x21>(prev, cur));
		}

		JSONIFIER_INLINE jsonifier_simd_int_t prev2(jsonifier_simd_int_t cur, jsonifier_simd_int_t prev) {
			return simd::opAlignR<14>(cur, simd::opPermute<0x21>(prev, cur));
		}

		JSONIFIER_INLINE jsonifier_simd_int_t prev3(jsonifier_simd_int_t cur, jsonifier_simd_int_t prev) {
			return simd::opAlignR<13>(cur, simd::opPermute<0x21>(prev, cur));
		}

		JSONIFIER_INLINE jsonifier_simd_int_t checkSpecialCases(jsonifier_simd_int_t input, jsonifier_simd_int_t p1) {
			const jsonifier_simd_int_t loNibbleMask = simd::gatherValue<jsonifier_simd_int_t>(static_cast<char>(0x0Fu));
			jsonifier_simd_int_t b1Hi				= simd::opShuffle(lookupH, simd::opAnd(simd::opSrLi<4>(p1), loNibbleMask));
			jsonifier_simd_int_t b1Lo				= simd::opShuffle(lookupL, simd::opAnd(p1, loNibbleMask));
			jsonifier_simd_int_t b2Hi				= simd::opShuffle(lookup2, simd::opAnd(simd::opSrLi<4>(input), loNibbleMask));
			return simd::opAnd(simd::opAnd(b1Hi, b1Lo), b2Hi);
		}

		JSONIFIER_INLINE jsonifier_simd_int_t mustBe23Continuation(jsonifier_simd_int_t p2, jsonifier_simd_int_t p3) {
			jsonifier_simd_int_t is3Or4 = simd::opSubS(p2, simd::gatherValue<jsonifier_simd_int_t>(static_cast<char>(0xE0u - 0x80u)));
			jsonifier_simd_int_t is4	= simd::opSubS(p3, simd::gatherValue<jsonifier_simd_int_t>(static_cast<char>(0xF0u - 0x80u)));
			return simd::opOr(is3Or4, is4);
		}

		JSONIFIER_INLINE jsonifier_simd_int_t checkMultibyteLengths(jsonifier_simd_int_t input, jsonifier_simd_int_t prev, jsonifier_simd_int_t sc) {
			jsonifier_simd_int_t p2		= prev2(input, prev);
			jsonifier_simd_int_t p3		= prev3(input, prev);
			jsonifier_simd_int_t must23 = mustBe23Continuation(p2, p3);
			return simd::opXor(simd::opAnd(must23, simd::gatherValue<jsonifier_simd_int_t>(static_cast<char>(0x80u))), sc);
		}

		JSONIFIER_INLINE jsonifier_simd_int_t checkIncomplete(jsonifier_simd_int_t input) {
			const jsonifier_simd_int_t maxVal = simd::gatherValues<jsonifier_simd_int_t>(std::bit_cast<const jsonifier_simd_int_t*>(isIncompleteMax + 32));
			return simd::opSubS(input, maxVal);
		}

		JSONIFIER_INLINE void checkChunk(jsonifier_simd_int_t input, jsonifier_simd_int_t prev) {
			jsonifier_simd_int_t p1 = prev1(input, prev);
			jsonifier_simd_int_t sc = checkSpecialCases(input, p1);
			error					= simd::opOr(error, checkMultibyteLengths(input, prev, sc));
		}

		template<uint64_t nBlocks, uint64_t totalChunks> struct chunk_processor {
			template<uint64_t... i>
			static inline void processImpl(utf8_checker& c, const simd_array<totalChunks>& chunks, jsonifier_simd_int_t& prev, std::index_sequence<i...>) noexcept {
				((c.checkChunk(chunks.template getValue<i>(), prev), prev = chunks.template getValue<i>()), ...);
			}

			static inline void process(utf8_checker& c, const simd_array<totalChunks>& chunks, jsonifier_simd_int_t& prev) noexcept {
				processImpl(c, chunks, prev, std::make_index_sequence<totalChunks>{});
			}
		};

		template<uint64_t nBlocks, uint64_t totalChunks> inline void checkStep(const uint8_t* src) {
			simd_array<totalChunks> chunks = chunk_loader<nBlocks, totalChunks>::load(src);

			jsonifier_simd_int_t asciiOr = ascii_checker<nBlocks, totalChunks>::orAll(chunks);

			if (simd::opBitMask(asciiOr) == 0) {
				error		   = simd::opOr(error, prevIncomplete);
				prevInput	   = chunks.template getValue<totalChunks - 1>();
				prevIncomplete = jsonifier_simd_int_t{};
				return;
			}

			jsonifier_simd_int_t prev = prevInput;
			chunk_processor<nBlocks, totalChunks>::process(*this, chunks, prev);

			prevInput	   = chunks.template getValue<totalChunks - 1>();
			prevIncomplete = checkIncomplete(chunks.template getValue<totalChunks - 1>());
		}

		JSONIFIER_INLINE bool errors() {
			return !simd::opTest(simd::opOr(error, prevIncomplete));
		}
	};

	template<uint64_t cpb, uint64_t bps> inline bool validateUtf8Impl(const uint8_t* src, uint64_t len) {
		if (len == 0)
			return true;

		constexpr uint64_t bytes_per_block = bytesPerChunk * cpb;
		constexpr uint64_t bytes_per_step  = bytes_per_block * bps;

		utf8_checker checker{};
		checker.reset();

		uint64_t i = 0;
		for (; i + bytes_per_step <= len; i += bytes_per_step)
			checker.checkStep<bps, bps * cpb>(src + i);

		for (; i + bytes_per_block <= len; i += bytes_per_block)
			checker.checkStep<1, cpb>(src + i);

		if (i < len) {
			JSONIFIER_ALIGN(bytesPerStep) uint8_t tmp[bytes_per_block];
			std::memset(tmp, 0x41, bytes_per_block);
			std::memcpy(tmp, src + i, len - i);
			checker.checkStep<1, cpb>(tmp);
		}

		return !checker.errors();
	}

	inline bool validateUtf8Ascii(const uint8_t* src, uint64_t len) {
		return validateUtf8Impl<chunksPerBlockAscii, blocksPerStepAscii>(src, len);
	}

	inline bool validateUtf8Mixed(const uint8_t* src, uint64_t len) {
		return validateUtf8Impl<chunksPerBlockMixed, blocksPerStepMixed>(src, len);
	}

	inline bool validateUtf8Multibyte(const uint8_t* src, uint64_t len) {
		return validateUtf8Impl<chunksPerBlockMultibyte, blocksPerStepMultibyte>(src, len);
	}

	inline bool validateUtf8(const uint8_t* src, uint64_t len) {
		return validateUtf8Impl<chunksPerBlockMixed, blocksPerStepMixed>(src, len);
	}

#endif

}
