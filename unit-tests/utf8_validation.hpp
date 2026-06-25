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

#include "common.hpp"

namespace utf8_validation_tests {

	struct utf8_test_case {
		std::vector<uint8_t> bytes;
		bool expected;
	};

	inline std::vector<uint8_t> repeated(uint8_t value, size_t count) {
		return std::vector<uint8_t>(count, value);
	}

	inline std::vector<uint8_t> repeatedSeq(std::vector<uint8_t> seq, size_t times) {
		std::vector<uint8_t> v;
		v.reserve(seq.size() * times);
		for (size_t i = 0; i < times; ++i) {
			v.insert(v.end(), seq.begin(), seq.end());
		}
		return v;
	}

	inline std::vector<uint8_t> paddedWithTrailing(size_t leadingAsciiCount, std::vector<uint8_t> trailer) {
		std::vector<uint8_t> v(leadingAsciiCount, 0x41);
		v.insert(v.end(), trailer.begin(), trailer.end());
		return v;
	}

	inline std::vector<uint8_t> asciiThenSeq(size_t asciiCount, std::vector<uint8_t> seq) {
		std::vector<uint8_t> v(asciiCount, 0x41);
		v.insert(v.end(), seq.begin(), seq.end());
		return v;
	}

	inline std::vector<uint8_t> asciiWrapped(size_t leadCount, std::vector<uint8_t> seq, size_t trailCount) {
		std::vector<uint8_t> v(leadCount, 0x41);
		v.insert(v.end(), seq.begin(), seq.end());
		v.insert(v.end(), trailCount, 0x41);
		return v;
	}

	inline std::vector<uint8_t> asciiWithSeqAt(size_t totalLength, size_t seqOffset, std::vector<uint8_t> seq) {
		std::vector<uint8_t> v(totalLength, 0x41);
		for (size_t i = 0; i < seq.size(); ++i) {
			v[seqOffset + i] = seq[i];
		}
		return v;
	}

	class reusable_ascii_buffer {
	  public:
		void resetTo(size_t totalLength) {
			static constexpr uint8_t asciiFill = 0x41;
			if (buffer.size() < totalLength) {
				buffer.assign(totalLength, asciiFill);
			} else {
				buffer.resize(totalLength);
				std::fill(buffer.begin(), buffer.end(), asciiFill);
			}
		}

		void placeSeqAt(size_t seqOffset, const std::vector<uint8_t>& seq) {
			for (size_t i = 0; i < seq.size(); ++i) {
				buffer[seqOffset + i] = seq[i];
			}
		}

		void clearSeqAt(size_t seqOffset, size_t seqLength) {
			for (size_t i = 0; i < seqLength; ++i) {
				buffer[seqOffset + i] = 0x41;
			}
		}

		const std::vector<uint8_t>& data() const {
			return buffer;
		}

	  private:
		std::vector<uint8_t> buffer;
	};

	template<rt_ut::string_literal testNameNew> inline static void utf8ValidationTest(std::vector<uint8_t> bytes, bool expected) {
		static constexpr rt_ut::string_literal testName{ testNameNew };
		rt_ut::unit_test<testName, true>::assert_eq(expected, jsonifier::validateUtf8, bytes.data(), bytes.size());
	}

	static constexpr jsonifier::parse_options utf8ValidatedOpts{ [] {
		jsonifier::parse_options opts{};
		opts.validateUtf8 = true;
		return opts;
	}() };

	inline static bool runValidatedStringParseWithScratch(const std::vector<uint8_t>& contentBytes, std::vector<uint8_t>& sourceScratch, std::vector<char>& destScratch) {
		sourceScratch.clear();
		sourceScratch.reserve(contentBytes.size() + 1);
		sourceScratch.insert(sourceScratch.end(), contentBytes.begin(), contentBytes.end());
		sourceScratch.push_back(static_cast<uint8_t>('"'));
		const size_t neededDestSize = sourceScratch.size() + 256;
		if (destScratch.size() < neededDestSize) {
			destScratch.assign(neededDestSize, 0);
		}
		const char* string1Start = std::bit_cast<const char*>(sourceScratch.data());
		char* string2			 = destScratch.data();
		const auto result		 = jsonifier::internal::string_parser<utf8ValidatedOpts>::impl(string1Start, string2, sourceScratch.size());
		return result != nullptr;
	}

	inline static bool runValidatedStringParse(const std::vector<uint8_t>& contentBytes) {
#if JSONIFIER_COMPILER_CLANG
	#pragma clang diagnostic push
	#pragma clang diagnostic ignored "-Wexit-time-destructors"
#endif
		static thread_local std::vector<uint8_t> sourceScratch;
		static thread_local std::vector<char> destScratch;
#if JSONIFIER_COMPILER_CLANG
	#pragma clang diagnostic pop
#endif
		return runValidatedStringParseWithScratch(contentBytes, sourceScratch, destScratch);
	}

	template<rt_ut::string_literal testNameNew> inline static void utf8StringParseTest(std::vector<uint8_t> bytes, bool expected) {
		static constexpr rt_ut::string_literal testName{ testNameNew };
		rt_ut::unit_test<testName, true>::assert_eq(expected, runValidatedStringParse, bytes);
	}

	inline static void runUtf8WidthTransitionSweep() {
		static constexpr size_t sweepLengthMin	= 24;
		static constexpr size_t sweepLengthMax	= 224;
		static constexpr size_t asciiTailLength = 20;

		const std::vector<std::vector<uint8_t>> completeSequences{
			{ 0xC3, 0xA9 },
			{ 0xE0, 0xA0, 0x80 },
			{ 0xF0, 0x90, 0x80, 0x80 },
		};

		const std::vector<std::vector<uint8_t>> danglingSequences{
			{ 0xC2 },
			{ 0xE0 },
			{ 0xE0, 0xA0 },
			{ 0xF0 },
			{ 0xF0, 0x90 },
			{ 0xF0, 0x90, 0x80 },
		};

		uint64_t acceptCases = 0;
		uint64_t acceptFails = 0;
		uint64_t rejectCases = 0;
		uint64_t rejectFails = 0;

		reusable_ascii_buffer contentBuffer;
		std::vector<uint8_t> sourceScratch;
		std::vector<char> destScratch;

		for (size_t bodyLength = sweepLengthMin; bodyLength <= sweepLengthMax; ++bodyLength) {
			const size_t totalLength = bodyLength + asciiTailLength;

			for (const auto& seq: completeSequences) {
				if (seq.size() > bodyLength) {
					continue;
				}
				contentBuffer.resetTo(totalLength);
				for (size_t offset = 0; offset + seq.size() <= bodyLength; ++offset) {
					contentBuffer.placeSeqAt(offset, seq);
					++acceptCases;
					if (!runValidatedStringParseWithScratch(contentBuffer.data(), sourceScratch, destScratch)) {
						++acceptFails;
						if (acceptFails <= 8) {
							std::cout << "FALSE POSITIVE: valid " << seq.size() << "-byte sequence rejected at offset " << offset << " of body length " << bodyLength << " (total "
									  << totalLength << ")" << std::endl;
						}
					}
					contentBuffer.clearSeqAt(offset, seq.size());
				}
			}

			for (const auto& seq: danglingSequences) {
				if (seq.size() > bodyLength) {
					continue;
				}
				contentBuffer.resetTo(totalLength);
				for (size_t offset = 0; offset + seq.size() <= bodyLength; ++offset) {
					contentBuffer.placeSeqAt(offset, seq);
					++rejectCases;
					if (runValidatedStringParseWithScratch(contentBuffer.data(), sourceScratch, destScratch)) {
						++rejectFails;
						if (rejectFails <= 8) {
							std::cout << "FALSE NEGATIVE: dangling " << (seq.size() == 1 ? "lead" : "partial sequence") << " (first byte 0x" << std::hex
									  << static_cast<uint32_t>(seq[0]) << std::dec << ") accepted at offset " << offset << " of body length " << bodyLength << " (total "
									  << totalLength << ")" << std::endl;
						}
					}
					contentBuffer.clearSeqAt(offset, seq.size());
				}
			}
		}

		std::cout << "Width-transition accept sweep: " << (acceptCases - acceptFails) << "/" << acceptCases << " passed" << std::endl;
		std::cout << "Width-transition reject sweep: " << (rejectCases - rejectFails) << "/" << rejectCases << " passed" << std::endl;
		static constexpr rt_ut::string_literal acceptSweepName{ "width_transition_accept_sweep" };
		static constexpr rt_ut::string_literal rejectSweepName{ "width_transition_reject_sweep" };
		rt_ut::unit_test<acceptSweepName, true>::assert_eq(
			uint64_t{ 0 },
			[](uint64_t fails) {
				return fails;
			},
			acceptFails);
		rt_ut::unit_test<rejectSweepName, true>::assert_eq(
			uint64_t{ 0 },
			[](uint64_t fails) {
				return fails;
			},
			rejectFails);
	}

	inline static void runUtf8CorrectnessTests() {
		std::cout << "UTF8 Validation Tests" << std::endl;
		utf8ValidationTest<"empty">({}, true);
		utf8ValidationTest<"single_ascii">({ 0x41 }, true);
		utf8ValidationTest<"ascii_x16">(repeated(0x41, 16), true);
		utf8ValidationTest<"ascii_x64">(repeated(0x41, 64), true);
		utf8ValidationTest<"ascii_x256">(repeated(0x41, 256), true);
		utf8ValidationTest<"two_byte_U0080">({ 0xC2, 0x80 }, true);
		utf8ValidationTest<"two_byte_U07FF">({ 0xDF, 0xBF }, true);
		utf8ValidationTest<"three_byte_U0800">({ 0xE0, 0xA0, 0x80 }, true);
		utf8ValidationTest<"three_byte_UFFFF">({ 0xEF, 0xBF, 0xBF }, true);
		utf8ValidationTest<"four_byte_U10000">({ 0xF0, 0x90, 0x80, 0x80 }, true);
		utf8ValidationTest<"four_byte_U10FFFF">({ 0xF4, 0x8F, 0xBF, 0xBF }, true);
		utf8ValidationTest<"two_byte_x128">(repeatedSeq({ 0xC2, 0x80 }, 128), true);
		utf8ValidationTest<"three_byte_x64">(repeatedSeq({ 0xE0, 0xA0, 0x80 }, 64), true);
		utf8ValidationTest<"four_byte_x64">(repeatedSeq({ 0xF0, 0x90, 0x80, 0x80 }, 64), true);
		utf8ValidationTest<"two_byte_cross_chunk">(asciiWrapped(15, { 0xC2, 0x80 }, 15), true);
		utf8ValidationTest<"three_byte_cross_chunk">(asciiWrapped(14, { 0xE0, 0xA0, 0x80 }, 14), true);
		utf8ValidationTest<"four_byte_cross_chunk">(asciiWrapped(13, { 0xF0, 0x90, 0x80, 0x80 }, 14), true);
		utf8ValidationTest<"four_byte_cross_block_boundary">(asciiThenSeq(61, { 0xF0, 0x90, 0x80, 0x80 }), true);
		utf8ValidationTest<"mixed_ascii_plus_multibyte">({ 0x41, 0xC2, 0x80, 0x41, 0xE0, 0xA0, 0x80, 0x41, 0xF0, 0x90, 0x80, 0x80, 0x41, 0x41, 0x41, 0x41 }, true);
		utf8ValidationTest<"ascii_x64_then_two_byte">(asciiThenSeq(64, { 0xC2, 0x80 }), true);
		utf8ValidationTest<"ascii_x256_then_two_byte">(asciiThenSeq(256, { 0xC2, 0x80 }), true);
		utf8ValidationTest<"dangling_two_byte_lead">({ 0xC2 }, false);
		utf8ValidationTest<"dangling_three_byte_lead">({ 0xE0 }, false);
		utf8ValidationTest<"dangling_three_byte_lead_plus_1">({ 0xE0, 0xA0 }, false);
		utf8ValidationTest<"dangling_four_byte_lead">({ 0xF0 }, false);
		utf8ValidationTest<"dangling_four_byte_lead_plus_1">({ 0xF0, 0x90 }, false);
		utf8ValidationTest<"dangling_four_byte_lead_plus_2">({ 0xF0, 0x90, 0x80 }, false);
		utf8ValidationTest<"bad_cont_ascii_after_two_lead">({ 0xC2, 0x41 }, false);
		utf8ValidationTest<"bad_cont_ascii_after_three_lead">({ 0xE0, 0xA0, 0x41 }, false);
		utf8ValidationTest<"bad_cont_lead_after_lead">({ 0xC2, 0xC2, 0x80 }, false);
		utf8ValidationTest<"lone_continuation">({ 0x80 }, false);
		utf8ValidationTest<"lone_continuation_x4">({ 0x80, 0x80, 0x80, 0x80 }, false);
		utf8ValidationTest<"surrogate_UD800">({ 0xED, 0xA0, 0x80 }, false);
		utf8ValidationTest<"surrogate_UDFFF">({ 0xED, 0xBF, 0xBF }, false);
		utf8ValidationTest<"overlong_two_byte_U0000">({ 0xC0, 0x80 }, false);
		utf8ValidationTest<"overlong_two_byte_U007F">({ 0xC1, 0xBF }, false);
		utf8ValidationTest<"overlong_three_byte">({ 0xE0, 0x80, 0x80 }, false);
		utf8ValidationTest<"overlong_four_byte">({ 0xF0, 0x80, 0x80, 0x80 }, false);
		utf8ValidationTest<"too_large_U110000">({ 0xF4, 0x90, 0x80, 0x80 }, false);
		utf8ValidationTest<"too_large_0xF5">({ 0xF5, 0x80, 0x80, 0x80 }, false);
		utf8ValidationTest<"0xFF_byte">({ 0xFF }, false);
		utf8ValidationTest<"0xFE_byte">({ 0xFE }, false);
		utf8ValidationTest<"bad_cont_cross_chunk">(asciiWrapped(15, { 0xC2, 0x41 }, 15), false);
		utf8ValidationTest<"surrogate_cross_chunk">(asciiWrapped(14, { 0xED, 0xA0, 0x80 }, 14), false);
		utf8ValidationTest<"dangling_lead_at_block_end">(paddedWithTrailing(63, { 0xC2 }), false);
		utf8ValidationTest<"dangling_lead_at_step_end">(paddedWithTrailing(255, { 0xC2 }), false);
		utf8ValidationTest<"valid_then_lone_cont">({ 0xC2, 0x80, 0x80 }, false);

		std::cout << "UTF8 String-Parser Fused Validation Tests" << std::endl;
		utf8StringParseTest<"sp_ascii_short">(repeated(0x41, 8), true);
		utf8StringParseTest<"sp_ascii_x64">(repeated(0x41, 64), true);
		utf8StringParseTest<"sp_ascii_x200">(repeated(0x41, 200), true);
		utf8StringParseTest<"sp_two_byte_simple">(asciiWrapped(30, { 0xC3, 0xA9 }, 30), true);
		utf8StringParseTest<"sp_bad_cont_mid">(asciiWrapped(30, { 0xC2, 0x41 }, 30), false);
		utf8StringParseTest<"sp_surrogate_mid">(asciiWrapped(30, { 0xED, 0xA0, 0x80 }, 30), false);
		utf8StringParseTest<"sp_overlong_mid">(asciiWrapped(30, { 0xC0, 0x80 }, 30), false);
		utf8StringParseTest<"sp_lone_cont_mid">(asciiWrapped(30, { 0x80 }, 30), false);
		utf8StringParseTest<"sp_valid_lead_at_m3_of_32">(asciiWithSeqAt(52, 29, { 0xC3, 0xA9 }), true);
		utf8StringParseTest<"sp_valid_lead_at_m3_of_64">(asciiWithSeqAt(84, 61, { 0xC3, 0xA9 }), true);
		utf8StringParseTest<"sp_valid_lead_at_m2_of_32">(asciiWithSeqAt(52, 30, { 0xC3, 0xA9 }), true);
		utf8StringParseTest<"sp_valid_three_at_m3_of_32">(asciiWithSeqAt(52, 29, { 0xE0, 0xA0, 0x80 }), true);
		utf8StringParseTest<"sp_valid_four_at_m4_of_64">(asciiWithSeqAt(84, 60, { 0xF0, 0x90, 0x80, 0x80 }), true);
		utf8StringParseTest<"sp_valid_four_at_m3_of_64">(asciiWithSeqAt(84, 61, { 0xF0, 0x90, 0x80, 0x80 }), true);
		utf8StringParseTest<"sp_dangling_c2_at_m1_of_32">(asciiWithSeqAt(52, 31, { 0xC2 }), false);
		utf8StringParseTest<"sp_dangling_c2_at_m1_of_64">(asciiWithSeqAt(84, 63, { 0xC2 }), false);
		utf8StringParseTest<"sp_dangling_e0_at_m2_of_32">(asciiWithSeqAt(52, 30, { 0xE0, 0xA0 }), false);
		utf8StringParseTest<"sp_dangling_f0_at_m3_of_32">(asciiWithSeqAt(52, 29, { 0xF0, 0x90, 0x80 }), false);
		utf8StringParseTest<"sp_escape_then_multibyte_tail">(
			[] {
				std::vector<uint8_t> v(40, 0x41);
				v[10] = '\\';
				v[11] = 'n';
				v.insert(v.end(), { 0xC3, 0xA9 });
				v.insert(v.end(), 20, 0x41);
				return v;
			}(),
			true);
		utf8StringParseTest<"sp_escape_then_dangling_tail">(
			[] {
				std::vector<uint8_t> v(40, 0x41);
				v[10] = '\\';
				v[11] = 'n';
				v.push_back(0xC2);
				v.insert(v.end(), 20, 0x41);
				return v;
			}(),
			false);

		runUtf8WidthTransitionSweep();
	}

}
