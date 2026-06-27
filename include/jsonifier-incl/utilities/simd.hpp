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

	template<uint64_t... values> struct get_first_value;

	template<uint64_t value_new, uint64_t... values> struct get_first_value<value_new, values...> {
		using type = std::remove_cvref_t<decltype(value_new)>;
	};

	template<uint64_t... values> using get_first_value_t = get_first_value<values...>::type;

	template<uint64_t... integers> struct integer_sequence {
		using value_type = get_first_value_t<integers...>;
		static constexpr value_type size() noexcept {
			return static_cast<value_type>(sizeof...(integers));
		}
	};

	template<typename sequence_01, typename sequence_02> struct merge_and_shift;

	template<uint64_t... indices_01, uint64_t... indices_02> struct merge_and_shift<integer_sequence<indices_01...>, integer_sequence<indices_02...>> {
		using type = integer_sequence<static_cast<decltype(indices_01)>(indices_01)..., static_cast<decltype(indices_02)>(indices_02 + sizeof...(indices_01))...>;
	};

	template<uint64_t size> struct make_sequence_impl {
		using type = typename merge_and_shift<typename make_sequence_impl<static_cast<decltype(size)>(size / 2ULL)>::type,
			typename make_sequence_impl<static_cast<decltype(size)>(size - size / 2ULL)>::type>::type;
	};

	template<uint64_t size>
		requires(size == 0ULL)
	struct make_sequence_impl<size> {
		using type = integer_sequence<>;
	};

	template<uint64_t size>
		requires(size == 1ULL)
	struct make_sequence_impl<size> {
		using type = integer_sequence<static_cast<decltype(size)>(0)>;
	};

	template<uint64_t size> using make_integer_sequence = typename make_sequence_impl<size>::type;

	template<typename integer_sequence, uint64_t offset> struct offset_sequence;

	template<uint64_t... indices, uint64_t offset> struct offset_sequence<integer_sequence<indices...>, offset> {
		using type = integer_sequence<static_cast<decltype(offset)>(indices + offset)...>;
	};

	template<typename integer_sequence, uint64_t step> struct step_sequence;

	template<uint64_t... indices, uint64_t step_new> struct step_sequence<integer_sequence<indices...>, step_new> {
		using type = integer_sequence<static_cast<decltype(step_new)>(indices* step_new)...>;
	};

	template<typename integer_sequence, uint64_t step> using step_sequence_t = typename step_sequence<integer_sequence, step>::type;

	template<uint64_t start, uint64_t end, uint64_t step>
		requires(end >= start && step > 0)
	using make_stepped_range_sequence =
		typename offset_sequence<step_sequence_t<make_integer_sequence<static_cast<decltype(end)>((end - start + step - 1) / step)>, step>, start>::type;

	template<template<auto...> typename functor_type, typename integer_sequence, auto...> struct functor_runner;

	template<template<auto...> typename functor_type, uint64_t... indices, auto... values> struct functor_runner<functor_type, integer_sequence<indices...>, values...> {
		template<typename... arg_types> JSONIFIER_INLINE static auto impl(arg_types&&... args) {
			(functor_type<values...>::template impl<indices>(forward<arg_types>(args)...), ...);
		}

		template<typename... arg_types> JSONIFIER_INLINE static auto implAnd(arg_types&&... args) {
			(functor_type<values...>::template impl<indices>(forward<arg_types>(args)...) && ...);
		}
	};

	template<auto...> struct write_indices_functor {
		using size_type = uint64_t;

		template<uint64_t index> JSONIFIER_INLINE static void impl(size_type base, size_type& bits, uint32_t* tape) {
			tape[index] = simd::tape_writer_op<void>::extractIndex(base, bits);
			bits		= simd::tape_writer_op<void>::advance(bits);
		}
	};

	template<uint64_t step> struct write_indices_stepped_functor {
		using size_type = uint64_t;

		template<uint64_t index> JSONIFIER_INLINE static bool impl(size_type base, size_type& bits, uint32_t* tape, uint64_t cnt) {
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

		template<uint64_t index> JSONIFIER_INLINE static void drainLane(array<uint64_t, sixtyFourBitsPerStep> bitsArr, array<uint64_t, sixtyFourBitsPerStep> cnts, uint32_t* tape,
			size_type strIdx) noexcept {
			uint64_t bits = bitsArr[index];
			if JSONIFIER_UNLIKELY (!bits) {
				return;
			}
			const uint64_t cnt = cnts[index];
			static constexpr size_type bitTotal{ index * 64ull };
			const size_type base = bitTotal + strIdx;
			functor_runner<write_indices_stepped_functor, make_stepped_range_sequence<0, 24, 4>, 4>::impl(base, bits, tape, cnt);
			if JSONIFIER_UNLIKELY (24 < cnt) {
				for (uint64_t i = 24; i < cnt; ++i) {
					tape[i] = simd::tape_writer_op<void>::extractIndex(base, bits);
					bits	= simd::tape_writer_op<void>::advance(bits);
				}
			}
		}

		JSONIFIER_INLINE static void impl(array<uint64_t, sixtyFourBitsPerStep> bitsArr, array<uint64_t, sixtyFourBitsPerStep> cnts, uint32_t* tape, size_type strIdx) noexcept {
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

	template<uint64_t initialBufferSize> struct simd_string_reader : public string_block_reader,
																	 public alloc_wrapper<uint32_t>,
																	 public add_tape_values<simd_string_reader<initialBufferSize>, make_integer_sequence<sixtyFourBitsPerStep>> {
		friend add_tape_values<simd_string_reader<initialBufferSize>, make_integer_sequence<sixtyFourBitsPerStep>>;
		static constexpr uint64_t initialTapeSize{ initialBufferSize * 8 / 10 };
		using allocator = alloc_wrapper<uint32_t>;

		JSONIFIER_INLINE simd_string_reader() noexcept {
			tape	 = allocator::allocate(initialTapeSize);
			capacity = initialTapeSize;
		}

		template<bool minified> JSONIFIER_INLINE void reset(const char* rootIter, uint64_t stringLength) noexcept {
			static constexpr uint64_t stepBytes = sixtyFourBitsPerStep * 64;

			const uint64_t neededCapacity = (stringLength * 8 / 10) + bitsPerStep;
			if (neededCapacity > capacity) {
				allocator::deallocate(tape, capacity);
				tape	 = allocator::allocate(neededCapacity);
				capacity = neededCapacity;
			}

			tapeCount							 = 0;
			string_block_reader::inString		 = reinterpret_cast<string_view_ptr>(rootIter);
			string_block_reader::length			 = stringLength;
			string_block_reader::lengthMinusStep = stringLength < stepBytes ? 0 : stringLength - stepBytes;
			string_block_reader::index			 = 0;
			scanner.prevInString				 = 0;
			scanner.prevScalar					 = 0;
			scanner.nextIsEscaped				 = 0;

			uint64_t unescapedCharsError{};

			if constexpr (minified) {
				const jsonifier_simd_int_t quoteRegister = simd::gatherValue<jsonifier_simd_int_t>('"');
				const jsonifier_simd_int_t bsRegister	 = simd::gatherValue<jsonifier_simd_int_t>('\\');
				const jsonifier_simd_int_t opTable		 = simd::gatherValues<jsonifier_simd_int_t>(simd::opArray<bytesPerStep>.data());
				const jsonifier_simd_int_t spaceMask	 = simd::gatherValue<jsonifier_simd_int_t>(static_cast<char>(0x20));
				while (string_block_reader::index + stepBytes <= string_block_reader::length) {
					processBlocks(reinterpret_cast<const uint8_t*>(rootIter) + string_block_reader::index, string_block_reader::index, unescapedCharsError, bsRegister,
						quoteRegister, opTable, spaceMask);
					string_block_reader::index += stepBytes;
				}

				if (string_block_reader::index < string_block_reader::length) {
					const uint64_t remaining = string_block_reader::length - string_block_reader::index;
					uint8_t remainder[stepBytes];
					std::fill_n(remainder, stepBytes, static_cast<uint8_t>(0x20));
					std::copy_n(reinterpret_cast<const uint8_t*>(rootIter) + string_block_reader::index, remaining, remainder);
					processBlocks(remainder, string_block_reader::index, unescapedCharsError, bsRegister, quoteRegister, opTable, spaceMask);

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
					processBlocks(reinterpret_cast<const uint8_t*>(rootIter) + string_block_reader::index, string_block_reader::index, unescapedCharsError, bsRegister,
						quoteRegister, opTable, spaceMask, whitespaceTable_local);
					string_block_reader::index += stepBytes;
				}

				if (string_block_reader::index < string_block_reader::length) {
					const uint64_t remaining = string_block_reader::length - string_block_reader::index;
					uint8_t remainder[stepBytes];
					std::fill_n(remainder, stepBytes, static_cast<uint8_t>(0x20));
					std::copy_n(reinterpret_cast<const uint8_t*>(rootIter) + string_block_reader::index, remaining, remainder);
					processBlocks(remainder, string_block_reader::index, unescapedCharsError, bsRegister, quoteRegister, opTable, spaceMask, whitespaceTable_local);

					const uint64_t excess = stepBytes - remaining;
					while (excess > 0 && tapeCount > 0 && tape[tapeCount - 1] >= string_block_reader::length) {
						--tapeCount;
					}
					string_block_reader::index += stepBytes;
				}
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
		simd::rope_detector<rope_block> scanner{};
		uint64_t tapeCount{};
		uint64_t capacity{};
		uint32_t* tape{};

		JSONIFIER_INLINE uint64_t getStructurals(simd_array in_01, const rope_block& block_local, jsonifier_simd_int_t opTable, jsonifier_simd_int_t spaceMask) noexcept {
			const uint64_t op			  = simd::op_collector::impl(in_01, opTable, spaceMask);
			const uint64_t scalar		  = ~(op | block_local.quotes);
			const uint64_t nonquoteScalar = scalar & ~block_local.quotes;
			const uint64_t follows		  = scanner.followsNonquoteScalar(nonquoteScalar);
			const uint64_t scalarStart	  = scalar & ~follows;
			return op | block_local.quotes | scalarStart;
		}

		JSONIFIER_INLINE uint64_t getStructurals(simd_array in_01, const rope_block& block_local, jsonifier_simd_int_t opTable, jsonifier_simd_int_t spaceMask,
			jsonifier_simd_int_t whitespaceTableLocal) noexcept {
			const uint64_t whitespace	  = simd::ws_collector::impl(in_01, whitespaceTableLocal);
			const uint64_t op			  = simd::op_collector::impl(in_01, opTable, spaceMask);
			const uint64_t scalar		  = ~(op | whitespace | block_local.quotes);
			const uint64_t nonquoteScalar = scalar & ~block_local.quotes;
			const uint64_t follows		  = scanner.followsNonquoteScalar(nonquoteScalar);
			const uint64_t scalarStart	  = scalar & ~follows;
			return op | block_local.quotes | scalarStart;
		}

		template<uint64_t I, typename... jsonifier_simd_int_types> JSONIFIER_INLINE void processBlocksImpl(array<uint64_t, sixtyFourBitsPerStep>& bitsArr,
			array<uint64_t, sixtyFourBitsPerStep>& cntsArr, const uint8_t* blockPtr, uint64_t& unescapedCharsError, jsonifier_simd_int_t bsRegister,
			jsonifier_simd_int_t quoteRegister, jsonifier_simd_int_t opTable, jsonifier_simd_int_t spaceMask, const jsonifier_simd_int_types&... args) noexcept {
			simd_array in_vals;
			in_vals.assign_value<0>(simd::gatherValuesU<jsonifier_simd_int_t>(blockPtr + I * 64));
			if constexpr (registersPerSixtyFourBits > 1) {
				in_vals.assign_value<1>(simd::gatherValuesU<jsonifier_simd_int_t>(blockPtr + I * 64 + bytesPerStep * 1));
				if constexpr (registersPerSixtyFourBits > 2) {
					in_vals.assign_value<2>(simd::gatherValuesU<jsonifier_simd_int_t>(blockPtr + I * 64 + bytesPerStep * 2));
					in_vals.assign_value<3>(simd::gatherValuesU<jsonifier_simd_int_t>(blockPtr + I * 64 + bytesPerStep * 3));
				}
			}
			const rope_block sb		   = scanner.next(in_vals, bsRegister, quoteRegister);
			const uint64_t structurals = getStructurals(in_vals, sb, opTable, spaceMask, args...) & ~sb.stringTail();
			bitsArr[I]				   = structurals;
			cntsArr[I]				   = simd::tape_writer_op<simd_string_reader>::correctedPopcount(structurals);
			simd_array ltRhs;
			ltRhs.assign_value<0>(simd::opCmpLtRaw(in_vals.get_value<0>(), spaceMask));
			if constexpr (registersPerSixtyFourBits > 1) {
				ltRhs.assign_value<1>(simd::opCmpLtRaw(in_vals.get_value<1>(), spaceMask));
				if constexpr (registersPerSixtyFourBits > 2) {
					ltRhs.assign_value<2>(simd::opCmpLtRaw(in_vals.get_value<2>(), spaceMask));
					ltRhs.assign_value<3>(simd::opCmpLtRaw(in_vals.get_value<3>(), spaceMask));
				}
			}
			const uint64_t unescaped = simd::unescaped_collector::impl(ltRhs);
			unescapedCharsError |= sb.nonQuoteInsideString(unescaped);
		}

		template<typename... jsonifier_simd_int_types> JSONIFIER_INLINE void processBlocks(const uint8_t* blockPtr, uint64_t stepBaseIndex, uint64_t& unescapedCharsError,
			jsonifier_simd_int_t bsRegister, jsonifier_simd_int_t quoteRegister, jsonifier_simd_int_t opTable, jsonifier_simd_int_t spaceMask,
			const jsonifier_simd_int_types&... args) noexcept {
			array<uint64_t, sixtyFourBitsPerStep> bitsArr{};
			array<uint64_t, sixtyFourBitsPerStep> cntsArr{};
			processBlocksImpl<0>(bitsArr, cntsArr, blockPtr, unescapedCharsError, bsRegister, quoteRegister, opTable, spaceMask, args...);
			processBlocksImpl<1>(bitsArr, cntsArr, blockPtr, unescapedCharsError, bsRegister, quoteRegister, opTable, spaceMask, args...);
			if constexpr (sixtyFourBitsPerStep > 2) {
				processBlocksImpl<2>(bitsArr, cntsArr, blockPtr, unescapedCharsError, bsRegister, quoteRegister, opTable, spaceMask, args...);
				processBlocksImpl<3>(bitsArr, cntsArr, blockPtr, unescapedCharsError, bsRegister, quoteRegister, opTable, spaceMask, args...);
				if constexpr (sixtyFourBitsPerStep > 4) {
					processBlocksImpl<4>(bitsArr, cntsArr, blockPtr, unescapedCharsError, bsRegister, quoteRegister, opTable, spaceMask, args...);
					processBlocksImpl<5>(bitsArr, cntsArr, blockPtr, unescapedCharsError, bsRegister, quoteRegister, opTable, spaceMask, args...);
					processBlocksImpl<6>(bitsArr, cntsArr, blockPtr, unescapedCharsError, bsRegister, quoteRegister, opTable, spaceMask, args...);
					processBlocksImpl<7>(bitsArr, cntsArr, blockPtr, unescapedCharsError, bsRegister, quoteRegister, opTable, spaceMask, args...);
				}
			}

			add_tape_values<simd_string_reader<initialBufferSize>, make_integer_sequence<sixtyFourBitsPerStep>>::impl(bitsArr, cntsArr, tape + tapeCount, stepBaseIndex);

			tapeCount += cntsArr[0];
			tapeCount += cntsArr[1];
			if constexpr (sixtyFourBitsPerStep > 2) {
				tapeCount += cntsArr[2];
				tapeCount += cntsArr[3];
				if constexpr (sixtyFourBitsPerStep > 4) {
					tapeCount += cntsArr[4];
					tapeCount += cntsArr[5];
					tapeCount += cntsArr[6];
					tapeCount += cntsArr[7];
				}
			}
		}
	};

}
