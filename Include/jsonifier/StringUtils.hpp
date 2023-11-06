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
/// A lot of the code in this header was sampled from simdjson - https://github.com/simdjson
#pragma once

#include <jsonifier/Allocator.hpp>
#include <jsonifier/Tables.hpp>
#include <jsonifier/Base.hpp>
#include <jsonifier/Simd.hpp>

namespace jsonifier_internal {

	template<bool shortStringsSupport> class backslash_and_quote;

	template<> class backslash_and_quote<false> {
	  public:
		template<typename char_type01, typename char_type02> JSONIFIER_INLINE static string_parsing_type copyAndFind(const char_type01* source, char_type02* dest) {
			simd_int_t values(gatherValuesU<simd_int_t>(source));
			storeu(values, dest);
			return static_cast<string_parsing_type>(simd_base::cmpeq(values, simd_base::backslashes) | simd_base::cmpeq(values, simd_base::quotes));
		}
	};

	template<> class backslash_and_quote<true> {
	  public:
		template<typename char_type01, typename char_type02> JSONIFIER_INLINE static uint16_t copyAndFind(const char_type01* source, char_type02* dest) {
			simd_int_128 values(gatherValuesU<simd_int_128>(source));
			storeu(values, dest);
			return static_cast<uint16_t>(simd_base::cmpeq(values, backslashes128) | simd_base::cmpeq(values, quotes128));
		}

	  protected:
		static constexpr simd_int_128 backslashes128{ simdFromValue<simd_int_128>(0x5Cu) };
		static constexpr simd_int_128 quotes128{ simdFromValue<simd_int_128>(0x22u) };
	};

	template<bool shortStringsSupport> class escapeable;

	template<> class escapeable<false> {
	  public:
		template<typename char_type01, typename char_type02> JSONIFIER_INLINE static string_parsing_type copyAndFind(const char_type01* source, char_type02* dest) {
			simd_int_t values(gatherValuesU<simd_int_t>(source));
			storeu(values, dest);
			return static_cast<string_parsing_type>(
				simd_base::cmpeq(simd_base::opShuffle(escapeableChars01, values), values) | simd_base::cmpeq(simd_base::opShuffle(escapeableChars03, values), values));
		}

	  protected:
		static constexpr uint8_t escapeableChars00[]{ 0x00u, 0x00u, 0x22u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x08u, 0x00u, 0x00u, 0x00u, 0x0Cu, 0x0Du, 0x00u, 0x00u };
		static constexpr uint8_t escapeableChars02[]{ 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x09u, 0x0Au, 0x00u, 0x5Cu, 0x00u, 0x00u, 0x00u };
		static constexpr simd_int_t escapeableChars01{ simdFromTable<simd_int_t>(escapeableChars00) };
		static constexpr simd_int_t escapeableChars03{ simdFromTable<simd_int_t>(escapeableChars02) };
	};

	template<> class escapeable<true> {
	  public:
		template<typename char_type01, typename char_type02> JSONIFIER_INLINE static uint16_t copyAndFind(const char_type01* source, char_type02* dest) {
			simd_int_128 values(gatherValuesU<simd_int_128>(source));
			storeu(values, dest);
			return static_cast<uint16_t>(
				simd_base::cmpeq(simd_base::opShuffle(escapeableChars01, values), values) | simd_base::cmpeq(simd_base::opShuffle(escapeableChars03, values), values));
		}

	  protected:
		static constexpr uint8_t escapeableChars00[]{ 0x00u, 0x00u, 0x22u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x08u, 0x00u, 0x00u, 0x00u, 0x0Cu, 0x0Du, 0x00u, 0x00u };
		static constexpr uint8_t escapeableChars02[]{ 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x09u, 0x0Au, 0x00u, 0x5Cu, 0x00u, 0x00u, 0x00u };
		static constexpr simd_int_128 escapeableChars01{ simdFromTable<simd_int_128>(escapeableChars00) };
		static constexpr simd_int_128 escapeableChars03{ simdFromTable<simd_int_128>(escapeableChars02) };
	};

	template<typename char_type> JSONIFIER_INLINE uint32_t hexToU32NoCheck(const char_type* source) {
		uint32_t v1 = digitToVal32[630 + source[0]];
		uint32_t v2 = digitToVal32[420 + source[1]];
		uint32_t v3 = digitToVal32[210 + source[2]];
		uint32_t v4 = digitToVal32[0 + source[3]];
		return v1 | v2 | v3 | v4;
	}

	template<typename char_type> JSONIFIER_INLINE uint32_t codePointToUtf8(uint32_t codePoint, char_type* c) {
		if (codePoint <= 0x7F) {
			c[0] = static_cast<char_type>(codePoint);
			return 1;
		}
		uint32_t leadingZeros = lzcnt(codePoint);

		if (leadingZeros >= 11) {
			uint32_t pattern = pdep(0x3F00u, codePoint);
			pattern |= 0xC0ull;
			c[0] = static_cast<char_type>(pattern >> 8);
			c[1] = static_cast<char_type>(pattern & 0xFFu);
			return 2;
		} else if (leadingZeros >= 16) {
			uint32_t pattern = pdep(0x0F0800u, codePoint);
			pattern |= 0xE0ull;
			c[0] = static_cast<char_type>(pattern >> 16);
			c[1] = static_cast<char_type>(pattern >> 8);
			c[2] = static_cast<char_type>(pattern & 0xFFu);
			return 3;
		} else if (leadingZeros >= 21) {
			uint32_t pattern = pdep(0x01020400u, codePoint);
			pattern |= 0xF0ull;
			c[0] = static_cast<char_type>(pattern >> 24);
			c[1] = static_cast<char_type>(pattern >> 16);
			c[2] = static_cast<char_type>(pattern >> 8);
			c[3] = static_cast<char_type>(pattern & 0xFFu);
			return 4;
		}
		return 0;
	}

	template<typename char_type01, typename char_type02> JSONIFIER_INLINE bool handleUnicodeCodePoint(char_type01* srcPtr, char_type02* dstPtr) {
		uint32_t codePoint = hexToU32NoCheck(*srcPtr + 2);
		*srcPtr += 6;
		if (codePoint >= 0xD800 && codePoint < 0xDC00) {
			char_type01 srcData = *srcPtr;
			if (((srcData[0] << 8) | srcData[1]) != ((static_cast<std::remove_pointer_t<char_type01>>(0x5Cu) << 8) | static_cast<std::remove_pointer_t<char_type01>>(0x75u))) {
				codePoint = 0xFFFD;
			} else {
				uint32_t codePoint02 = hexToU32NoCheck(srcData + 2);
				uint32_t lowBit		 = codePoint02 - 0xDC00;
				if (lowBit >> 10) {
					codePoint = 0xFFFD;
				} else {
					codePoint = (((codePoint - 0xD800) << 10) | lowBit) + 0x10000;
					*srcPtr += 6;
				}
			}
		} else if (codePoint >= 0xdc00 && codePoint <= 0xdFFF) {
			codePoint = 0xFFFD;
		}
		uint64_t offset = codePointToUtf8(codePoint, *dstPtr);
		*dstPtr += offset;
		return offset > 0;
	}

	template<bool shortStringsSupport> constexpr uint64_t getStringLength() {
		return shortStringsSupport ? 16 : BytesPerStep;
	}

	template<bool shortStringsSupport, typename char_type01, typename char_type02>
	JSONIFIER_INLINE char_type02* parseString(const char_type01* source, char_type02* dest, uint64_t lengthNew) {
		while (static_cast<int64_t>(lengthNew) >= static_cast<int64_t>(0)) {
			string_parsing_type nextBackslashOrQuote = backslash_and_quote<shortStringsSupport>::copyAndFind(source, dest);
			if (nextBackslashOrQuote != 0) {
				nextBackslashOrQuote = tzcnt(nextBackslashOrQuote);
				auto escapeChar		 = source[nextBackslashOrQuote];
				if (escapeChar == 0x22u) {
					return dest + nextBackslashOrQuote;
				}
				escapeChar = source[nextBackslashOrQuote + 1];
				if (escapeChar == 0x75u) {
					source += nextBackslashOrQuote;
					dest += nextBackslashOrQuote;
					handleUnicodeCodePoint(&source, &dest);
				} else {
					char_type01 escapeResult = escapeMap[escapeChar];
					if (escapeResult == 0u) {
						return static_cast<char_type02*>(nullptr);
					}
					dest[nextBackslashOrQuote] = escapeResult;
					dest += nextBackslashOrQuote + 1ull;
					source += nextBackslashOrQuote + 2ull;
					lengthNew -= nextBackslashOrQuote + 2ull;
				}
			} else {
				dest += getStringLength<shortStringsSupport>();
				source += getStringLength<shortStringsSupport>();
				lengthNew -= getStringLength<shortStringsSupport>();
			}
		}
		return static_cast<char_type02*>(nullptr);
	}

	template<bool shortStringsSupport, typename char_type01, typename char_type02>
	JSONIFIER_INLINE char_type01* serializeString(char_type01* source, char_type02* dest, uint64_t lengthNew, uint64_t& indexNew) {
#if JSONIFIER_CHECK_FOR_INSTRUCTION(JSONIFIER_ANY_AVX)
		auto serializationLambda = [&](char_type01* source, char_type02*& dest, uint64_t remainingLength, auto&& comparitor) -> char_type01* {
			while (comparitor(static_cast<int64_t>(lengthNew), static_cast<int64_t>(remainingLength))) {
				string_parsing_type nextEscapeable = escapeable<shortStringsSupport>::copyAndFind(source, dest);
				if (nextEscapeable != 0) {
					nextEscapeable			 = tzcnt(nextEscapeable);
					char_type01 escapeResult = static_cast<char_type01>(escapeableChars[static_cast<char_type01>(source[nextEscapeable])]);
					if (escapeResult == 0u) {
						lengthNew -= nextEscapeable;
						indexNew += nextEscapeable;
						source += nextEscapeable;
						dest += nextEscapeable;
						return source;
					}
					dest[nextEscapeable]							   = 0x5Cu;
					dest[static_cast<uint64_t>(nextEscapeable) + 1ull] = static_cast<char_type02>(escapeResult);
					dest += static_cast<uint64_t>(nextEscapeable) + 2ull;
					indexNew += static_cast<uint64_t>(nextEscapeable) + 2ull;
					lengthNew -= (nextEscapeable + 1ull);
					source += nextEscapeable + 1ull;
				} else {
					lengthNew -= remainingLength > 0 ? remainingLength : 1;
					indexNew += remainingLength > 0 ? remainingLength : 1;
					source += remainingLength > 0 ? remainingLength : 1;
					dest += remainingLength > 0 ? remainingLength : 1;
				}
			}
			return source;
		};

		source = serializationLambda(source, dest, getStringLength<shortStringsSupport>(), std::greater_equal{});
		std::remove_const_t<char_type01> newValues[getStringLength<shortStringsSupport>()]{};
		std::memcpy(newValues, source, lengthNew);
		return serializationLambda(newValues, dest, 0, std::greater{});
#else
		return source;
#endif
	}

	template<typename char_type> JSONIFIER_INLINE bool parseBool(char_type* json) {
		char_type values[5]{ "true" };
		return std::memcmp(values, json, 4) == 0;
	}
}// namespace jsonifier_internal
