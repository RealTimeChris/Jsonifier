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

#include <jsonifier-incl/utilities/string_view.hpp>
#include <jsonifier-incl/utilities/type_entities.hpp>
#include <jsonifier-incl/simd/avx_stage1.hpp>
#include <jsonifier-incl/simd/neon_stage1.hpp>

#include <sstream>
#include <cmath>
#include <bitset>

namespace jsonifier::internal {

	struct string_block_reader {
		JSONIFIER_INLINE void reset(string_view_ptr stringViewNew, uint64_t lengthNew) noexcept {
			lengthMinusStep = lengthNew < bitsPerStep ? 0 : lengthNew - bitsPerStep;
			inString		= stringViewNew;
			length			= lengthNew;
			index			= 0;
		}

		JSONIFIER_INLINE string_view_ptr getRemainder() noexcept {
			if JSONIFIER_UNLIKELY (length == index) {
				return nullptr;
			}
			std::fill_n(block, bitsPerStep, static_cast<char>(0x20));
			std::copy_n(inString + index, length - index, block);
			return block;
		}

		JSONIFIER_INLINE uint64_t getRemainderBytes() const noexcept {
			return length - index;
		}

		JSONIFIER_INLINE string_view_ptr fullBlock() noexcept {
			string_view_ptr newPtr = inString + index;
			index += bitsPerStep;
			return newPtr;
		}

		JSONIFIER_INLINE bool hasFullBlock() const noexcept {
			return index < lengthMinusStep;
		}

	  protected:
		JSONIFIER_ALIGN(bytesPerStep) char block[bitsPerStep] {};
		string_view_ptr inString{};
		uint64_t lengthMinusStep{};
		uint64_t length{};
		uint64_t index{};
	};	

	template<auto...> struct write_indices_functor {
		using size_type = uint64_t;

		template<uint64_t index> JSONIFIER_INLINE static void impl(size_type base, size_type& bits, structural_index_ptr tape) noexcept {
			tape[index] = simd::tape_writer_op<void>::extractIndex(base, bits);
			bits		= simd::tape_writer_op<void>::advance(bits);
		}
	};

	template<uint64_t step> struct write_indices_stepped_functor {
		using size_type = uint64_t;

		template<uint64_t index> JSONIFIER_INLINE static bool impl(size_type base, size_type& bits, structural_index_ptr tape, uint64_t cnt) noexcept {
			if constexpr (index > 0) {
				if JSONIFIER_UNLIKELY (!((index) < cnt)) {
					return false;
				}
			}
			functor_runner<write_indices_functor, make_integer_sequence<step>>::impl(base, bits, tape + index);
			return true;
		}
	};

	template<typename derived_type, typename integer_sequence_type> struct add_tape_values;

	template<typename derived_type, uint64_t... indices> struct add_tape_values<derived_type, integer_sequence<indices...>> {
		using size_type = uint64_t;

		template<uint64_t index> JSONIFIER_INLINE static void drainLane(array<uint64_t, simdBlocksPerStep> bitsArr, array<uint64_t, simdBlocksPerStep> cnts, structural_index_ptr tape,
			size_type strIdx) noexcept {
			uint64_t bits = bitsArr[index];
			if JSONIFIER_UNLIKELY (!bits) {
				return;
			}
			const uint64_t cnt = cnts[index];
			static constexpr size_type bitTotal{ index * 64ull };
			const size_type base = bitTotal + strIdx;
			functor_runner<write_indices_stepped_functor, make_stepped_range_sequence<0, jsonifierTapeMax, jsonifierTapeStep>, jsonifierTapeStep>::impl(base, bits, tape,
				cnt);
			if JSONIFIER_UNLIKELY (jsonifierTapeMax < cnt) {
				for (uint64_t i = jsonifierTapeMax; i < cnt; ++i) {
					tape[i] = simd::tape_writer_op<void>::extractIndex(base, bits);
					bits	= simd::tape_writer_op<void>::advance(bits);
				}
			}
		}

		JSONIFIER_INLINE static void impl(array<uint64_t, simdBlocksPerStep> bitsArr, array<uint64_t, simdBlocksPerStep> cnts, structural_index_ptr tape, size_type strIdx) noexcept {
			uint64_t offset = 0;
			(((drainLane<indices>(bitsArr, cnts, tape + offset, strIdx)), offset += cnts[indices]), ...);
		}
	};

	inline static void printBitsAligned(uint64_t bits, const char* label, const char* str = nullptr, uint64_t len = 0) noexcept {
		std::cout << label << ":" << std::endl;
		if (str && len > 0) {
			std::cout << "STR:  ";
			for (uint64_t i = 0; i < std::min<uint64_t>(len, 64); ++i) {
				char c = str[i];
				if (c == '\n' || c == '\r' || c == '\t')
					c = ' ';
				std::cout << c;
			}
			std::cout << std::endl;
		}
		std::cout << "BITS: ";
		for (uint64_t i = 0; i < 64; ++i) {
			std::cout << ((bits >> i) & 1ULL);
		}
		std::cout << std::endl;
		std::cout << "IDX:  ";
		for (uint64_t i = 0; i < 64; ++i) {
			std::cout << (i % 10);
		}
		std::cout << std::endl;
		std::cout << "TENS: ";
		for (uint64_t i = 0; i < 64; ++i) {
			std::cout << ((i / 10) % 10);
		}
		std::cout << std::endl << std::endl;
	}

	struct rope_block {
		uint64_t escaped{};
		uint64_t quotes{};
		uint64_t inString{};

		JSONIFIER_INLINE uint64_t stringTail() const noexcept {
			return inString ^ quotes;
		}

		JSONIFIER_INLINE uint64_t nonQuoteInsideString(uint64_t mask) const noexcept {
			return mask & inString;
		}

		JSONIFIER_INLINE uint64_t nonQuoteOutsideString(uint64_t mask) const noexcept {
			return mask & ~inString;
		}
	};

	template<uint64_t initialBufferSize> struct simd_string_reader : simd::rope_detector<rope_block>,
																	 string_block_reader,
																	 alloc_wrapper<uint32_t>,
																	 add_tape_values<simd_string_reader<initialBufferSize>, make_integer_sequence<simdBlocksPerStep>> {
		friend add_tape_values<simd_string_reader<initialBufferSize>, make_integer_sequence<simdBlocksPerStep>>;
		static constexpr uint64_t initialTapeSize{ initialBufferSize * 8 / 10 };
		using allocator = alloc_wrapper<uint32_t>;
		bool utf8Valid{};
		JSONIFIER_INLINE simd_string_reader() noexcept {
			tape	 = allocator::allocate(initialTapeSize);
			capacity = initialTapeSize;
		}
		template<bool minified, bool validate_utf8 = true> JSONIFIER_INLINE bool reset(const char* rootIter, uint64_t stringLength) noexcept {
			static constexpr uint64_t stepBytes = simdBlocksPerStep * 64;
			const uint64_t neededCapacity		= (stringLength * 8 / 10) + bitsPerStep;
			if (neededCapacity > capacity) {
				allocator::deallocate(tape, capacity);
				tape	 = allocator::allocate(neededCapacity);
				capacity = neededCapacity;
			}
			tapeCount									   = 0;
			string_block_reader::inString				   = std::bit_cast<string_view_ptr>(rootIter);
			string_block_reader::length					   = stringLength;
			string_block_reader::lengthMinusStep		   = stringLength < stepBytes ? 0 : stringLength - stepBytes;
			string_block_reader::index					   = 0;
			simd::rope_detector<rope_block>::prevInString  = 0;
			simd::rope_detector<rope_block>::prevScalar	   = 0;
			simd::rope_detector<rope_block>::nextIsEscaped = 0;
			uint64_t unescapedCharsError{};
			utf8_checker checker{};
			if constexpr (validate_utf8) {
				checker.reset();
			}
			if constexpr (minified) {
				const jsonifier_simd_int_t quoteRegister = simd::gatherValue<jsonifier_simd_int_t>('"');
				const jsonifier_simd_int_t bsRegister	 = simd::gatherValue<jsonifier_simd_int_t>('\\');
				const jsonifier_simd_int_t opTable		 = simd::gatherValues<jsonifier_simd_int_t>(simd::opArray<bytesPerStep>.data());
				const jsonifier_simd_int_t spaceMask	 = simd::gatherValue<jsonifier_simd_int_t>(static_cast<char>(0x20));
				while (string_block_reader::index + stepBytes <= string_block_reader::length) {
					processBlocks<validate_utf8>(std::bit_cast<const uint8_t*>(rootIter) + string_block_reader::index, string_block_reader::index, unescapedCharsError, checker,
						bsRegister, quoteRegister, opTable, spaceMask);
					string_block_reader::index += stepBytes;
				}
				if (string_block_reader::index < string_block_reader::length) {
					const uint64_t remaining = string_block_reader::length - string_block_reader::index;
					uint8_t remainder[stepBytes];
					std::fill_n(remainder, stepBytes, static_cast<uint8_t>(0x20));
					std::copy_n(std::bit_cast<const uint8_t*>(rootIter) + string_block_reader::index, remaining, remainder);
					processBlocks<validate_utf8>(remainder, string_block_reader::index, unescapedCharsError, checker, bsRegister, quoteRegister, opTable, spaceMask);
					const uint64_t excess = stepBytes - remaining;
					while (excess > 0 && tapeCount > 0 && tape[tapeCount - 1] >= string_block_reader::length) {
						--tapeCount;
					}
					string_block_reader::index += stepBytes;
				}
			} else {
				const jsonifier_simd_int_t quoteRegister		 = simd::gatherValue<jsonifier_simd_int_t>('"');
				const jsonifier_simd_int_t bsRegister			 = simd::gatherValue<jsonifier_simd_int_t>('\\');
				const jsonifier_simd_int_t opTable				 = simd::gatherValues<jsonifier_simd_int_t>(simd::opArray<bytesPerStep>.data());
				const jsonifier_simd_int_t spaceMask			 = simd::gatherValue<jsonifier_simd_int_t>(static_cast<char>(0x20));
				const jsonifier_simd_int_t whitespaceTable_local = simd::gatherValues<jsonifier_simd_int_t>(simd::whitespaceArray<bytesPerStep>.data());
				while (string_block_reader::index + stepBytes <= string_block_reader::length) {
					processBlocks<validate_utf8>(std::bit_cast<const uint8_t*>(rootIter) + string_block_reader::index, string_block_reader::index, unescapedCharsError, checker,
						bsRegister, quoteRegister, opTable, spaceMask, whitespaceTable_local);
					string_block_reader::index += stepBytes;
				}
				if (string_block_reader::index < string_block_reader::length) {
					const uint64_t remaining = string_block_reader::length - string_block_reader::index;
					uint8_t remainder[stepBytes];
					std::fill_n(remainder, stepBytes, static_cast<uint8_t>(0x20));
					std::copy_n(std::bit_cast<const uint8_t*>(rootIter) + string_block_reader::index, remaining, remainder);
					processBlocks<validate_utf8>(remainder, string_block_reader::index, unescapedCharsError, checker, bsRegister, quoteRegister, opTable, spaceMask,
						whitespaceTable_local);
					const uint64_t excess = stepBytes - remaining;
					while (excess > 0 && tapeCount > 0 && tape[tapeCount - 1] >= string_block_reader::length) {
						--tapeCount;
					}
					string_block_reader::index += stepBytes;
				}
			}
			if constexpr (validate_utf8) {
				return !checker.errors();
			} else {
				return true;
			}
		}
		JSONIFIER_INLINE auto end() noexcept {
			return tape + tapeCount;
		}
		JSONIFIER_INLINE auto begin() noexcept {
			tape[tapeCount] = static_cast<uint32_t>(string_block_reader::length);
			return tape;
		}
		JSONIFIER_INLINE uint64_t getTapeCount() noexcept {
			return tapeCount;
		}
		JSONIFIER_INLINE ~simd_string_reader() noexcept {
			if (tape) {
				allocator::deallocate(tape, capacity);
				tape = nullptr;
			}
		}

	  protected:
		structural_index_ptr tape{};
		uint64_t tapeCount{};
		uint64_t capacity{};
		JSONIFIER_INLINE uint64_t getStructurals(simd_array<registersPerSixtyFourBits> in_01, jsonifier_simd_int_t opTable, jsonifier_simd_int_t spaceMask) noexcept {
			const uint64_t op			  = simd::op_collector::impl(in_01, opTable, spaceMask);
			const uint64_t scalar		  = ~(op | simd::rope_detector<rope_block>::quotes);
			const uint64_t nonquoteScalar = scalar & ~simd::rope_detector<rope_block>::quotes;
			const uint64_t follows		  = simd::rope_detector<rope_block>::followsNonquoteScalar(nonquoteScalar);
			const uint64_t scalarStart	  = scalar & ~follows;
			return op | simd::rope_detector<rope_block>::quotes | scalarStart;
		}
		JSONIFIER_INLINE uint64_t getStructurals(simd_array<registersPerSixtyFourBits> in_01, jsonifier_simd_int_t opTable, jsonifier_simd_int_t spaceMask,
			jsonifier_simd_int_t whitespaceTableLocal) noexcept {
			const uint64_t whitespace	  = simd::ws_collector::impl(in_01, whitespaceTableLocal);
			const uint64_t op			  = simd::op_collector::impl(in_01, opTable, spaceMask);
			const uint64_t scalar		  = ~(op | whitespace | simd::rope_detector<rope_block>::quotes);
			const uint64_t nonquoteScalar = scalar & ~simd::rope_detector<rope_block>::quotes;
			const uint64_t follows		  = simd::rope_detector<rope_block>::followsNonquoteScalar(nonquoteScalar);
			const uint64_t scalarStart	  = scalar & ~follows;
			return op | simd::rope_detector<rope_block>::quotes | scalarStart;
		}
		template<bool validate_utf8, uint64_t I, typename... jsonifier_simd_int_types> JSONIFIER_INLINE void processBlocksImpl(array<uint64_t, simdBlocksPerStep>& bitsArr,
			array<uint64_t, simdBlocksPerStep>& cntsArr, const uint8_t* blockPtr, uint64_t& unescapedCharsError, utf8_checker& checker, jsonifier_simd_int_t bsRegister,
			jsonifier_simd_int_t quoteRegister, jsonifier_simd_int_t opTable, jsonifier_simd_int_t spaceMask, jsonifier_simd_int_types... args) noexcept {
			simd_array<registersPerSixtyFourBits> in_vals;
			in_vals.assignValue<0>(simd::gatherValuesU<jsonifier_simd_int_t>(blockPtr + I * 64));
			if constexpr (registersPerSixtyFourBits > 1) {
				in_vals.assignValue<1>(simd::gatherValuesU<jsonifier_simd_int_t>(blockPtr + I * 64 + bytesPerStep * 1));
				if constexpr (registersPerSixtyFourBits > 2) {
					in_vals.assignValue<2>(simd::gatherValuesU<jsonifier_simd_int_t>(blockPtr + I * 64 + bytesPerStep * 2));
					in_vals.assignValue<3>(simd::gatherValuesU<jsonifier_simd_int_t>(blockPtr + I * 64 + bytesPerStep * 3));
				}
			}
			if constexpr (validate_utf8) {
				checker.checkChunk(in_vals.getValue<0>(), checker.prevInput);
				checker.prevInput = in_vals.getValue<0>();
				if constexpr (registersPerSixtyFourBits > 1) {
					checker.checkChunk(in_vals.getValue<1>(), checker.prevInput);
					checker.prevInput = in_vals.getValue<1>();
					if constexpr (registersPerSixtyFourBits > 2) {
						checker.checkChunk(in_vals.getValue<2>(), checker.prevInput);
						checker.prevInput = in_vals.getValue<2>();
						checker.checkChunk(in_vals.getValue<3>(), checker.prevInput);
						checker.prevInput = in_vals.getValue<3>();
					}
				}
			}
			simd::rope_detector<rope_block>::next(in_vals, bsRegister, quoteRegister);
			const uint64_t structurals = getStructurals(in_vals, opTable, spaceMask, args...) & ~simd::rope_detector<rope_block>::stringTail();
			bitsArr[I]				   = structurals;
			cntsArr[I]				   = simd::tape_writer_op<simd_string_reader>::correctedPopcount(structurals);
			simd_array<registersPerSixtyFourBits> ltRhs;
			ltRhs.assignValue<0>(simd::opCmpLtRaw(in_vals.getValue<0>(), spaceMask));
			if constexpr (registersPerSixtyFourBits > 1) {
				ltRhs.assignValue<1>(simd::opCmpLtRaw(in_vals.getValue<1>(), spaceMask));
				if constexpr (registersPerSixtyFourBits > 2) {
					ltRhs.assignValue<2>(simd::opCmpLtRaw(in_vals.getValue<2>(), spaceMask));
					ltRhs.assignValue<3>(simd::opCmpLtRaw(in_vals.getValue<3>(), spaceMask));
				}
			}
			const uint64_t unescaped = simd::unescaped_collector::impl(ltRhs);
			unescapedCharsError |= simd::rope_detector<rope_block>::nonQuoteInsideString(unescaped);
		}
		template<bool validate_utf8, typename... jsonifier_simd_int_types> JSONIFIER_INLINE void processBlocks(const uint8_t* blockPtr, uint64_t stepBaseIndex,
			uint64_t& unescapedCharsError, utf8_checker& checker, jsonifier_simd_int_t bsRegister, jsonifier_simd_int_t quoteRegister, jsonifier_simd_int_t opTable,
			jsonifier_simd_int_t spaceMask, jsonifier_simd_int_types... args) noexcept {
			array<uint64_t, simdBlocksPerStep> bitsArr{};
			array<uint64_t, simdBlocksPerStep> cntsArr{};
			processBlocksImpl<validate_utf8, 0>(bitsArr, cntsArr, blockPtr, unescapedCharsError, checker, bsRegister, quoteRegister, opTable, spaceMask, args...);
			if constexpr (simdBlocksPerStep > 1) {
				processBlocksImpl<validate_utf8, 1>(bitsArr, cntsArr, blockPtr, unescapedCharsError, checker, bsRegister, quoteRegister, opTable, spaceMask, args...);
				if constexpr (simdBlocksPerStep > 2) {
					processBlocksImpl<validate_utf8, 2>(bitsArr, cntsArr, blockPtr, unescapedCharsError, checker, bsRegister, quoteRegister, opTable, spaceMask, args...);
					processBlocksImpl<validate_utf8, 3>(bitsArr, cntsArr, blockPtr, unescapedCharsError, checker, bsRegister, quoteRegister, opTable, spaceMask, args...);
					if constexpr (simdBlocksPerStep > 4) {
						processBlocksImpl<validate_utf8, 4>(bitsArr, cntsArr, blockPtr, unescapedCharsError, checker, bsRegister, quoteRegister, opTable, spaceMask, args...);
						processBlocksImpl<validate_utf8, 5>(bitsArr, cntsArr, blockPtr, unescapedCharsError, checker, bsRegister, quoteRegister, opTable, spaceMask, args...);
						processBlocksImpl<validate_utf8, 6>(bitsArr, cntsArr, blockPtr, unescapedCharsError, checker, bsRegister, quoteRegister, opTable, spaceMask, args...);
						processBlocksImpl<validate_utf8, 7>(bitsArr, cntsArr, blockPtr, unescapedCharsError, checker, bsRegister, quoteRegister, opTable, spaceMask, args...);
					}
				}
			}
			add_tape_values<simd_string_reader<initialBufferSize>, make_integer_sequence<simdBlocksPerStep>>::impl(bitsArr, cntsArr, tape + tapeCount, stepBaseIndex);
			tapeCount += cntsArr[0];
			if constexpr (simdBlocksPerStep > 1) {
				tapeCount += cntsArr[1];
				if constexpr (simdBlocksPerStep > 2) {
					tapeCount += cntsArr[2];
					tapeCount += cntsArr[3];
					if constexpr (simdBlocksPerStep > 4) {
						tapeCount += cntsArr[4];
						tapeCount += cntsArr[5];
						tapeCount += cntsArr[6];
						tapeCount += cntsArr[7];
					}
				}
			}
		}
	};

}
