/*
	Jsonifier - For parsing and serializing Json - very rapidly.
	Copyright (C) 2023 Chris M. (RealTimeChris)

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	This library is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
	Lesser General Public License for more details.

	You should have received a copy of the GNU Lesser General Public
	License along with this library; if not, Write to the Free Software
	Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301
	USA
*/
/// https://github.com/RealTimeChris/Jsonifier
/// Feb 3, 2023
/// Most of the code in this header was copied from Simdjson - https://github.com/simdjson
#pragma once

#include <jsonifier/Base.hpp>
#include <jsonifier/Tables.hpp>
#include <jsonifier/Simd.hpp>
#include <codecvt>

namespace JsonifierInternal {

	struct SimdBase64;
	struct SimdBase128;
	struct SimdBase256;
	template<typename SimdBase> class BackslashAndQuote;

#if defined(INSTRUCTION_SET_TYPE_AVX512)
	template<> class BackslashAndQuote<SimdBase512> {
	  public:
		static const uint32_t bytesProcessed = 64;

		BackslashAndQuote<SimdBase512> inline static copyAndFind(StringViewPtr source, StringBufferPtr dest) {
			SimdBase512 values(reinterpret_cast<StringViewPtr>(source));
			values.store(dest);
			BackslashAndQuote returnData{};
			returnData.bsBits = values == '\\';
			returnData.quoteBits = values == '\"';
			return returnData;
		}

		inline bool hasQuoteFirst() {
			return ((bsBits - 1) & quoteBits) != 0;
		}

		inline bool hasBackslash() {
			return ((quoteBits - 1) & bsBits) != 0;
		}

		inline int64_t quoteIndex() {
			return _tzcnt_u32(quoteBits);
		}

		inline int64_t backslashIndex() {
			return _tzcnt_u32(bsBits);
		}

	  protected:
		uint64_t quoteBits{};
		uint64_t bsBits{};
	};
	constexpr int32_t stringParseLength{ 64 };
	using BackslashAndQuoteType = BackslashAndQuote<SimdBase512>;
#elif defined(INSTRUCTION_SET_TYPE_AVX2)
	template<> class BackslashAndQuote<SimdBase256> {
	  public:
		static const uint32_t bytesProcessed = 32;

		BackslashAndQuote<SimdBase256> inline static copyAndFind(StringViewPtr source, StringBufferPtr dest) {
			SimdBase256 values(reinterpret_cast<StringViewPtr>(source));
			values.store(dest);
			BackslashAndQuote returnData{};
			returnData.bsBits = static_cast<uint32_t>((values == '\\').toBitMask());
			returnData.quoteBits = static_cast<uint32_t>((values == '\"').toBitMask());
			return returnData;
		}

		inline bool hasQuoteFirst() {
			return ((bsBits - 1) & quoteBits) != 0;
		}

		inline bool hasBackslash() {
			return ((quoteBits - 1) & bsBits) != 0;
		}

		inline int32_t quoteIndex() {
			return _tzcnt_u32(quoteBits);
		}

		inline int32_t backslashIndex() {
			return _tzcnt_u32(bsBits);
		}

	  protected:
		uint32_t quoteBits{};
		uint32_t bsBits{};
	};
	constexpr int32_t stringParseLength{ 32 };
	using BackslashAndQuoteType = BackslashAndQuote<SimdBase256>;
#elif defined(INSTRUCTION_SET_TYPE_AVX)
	template<> class BackslashAndQuote<SimdBase128> {
	  public:
		static const uint32_t bytesProcessed = 16;

		BackslashAndQuote<SimdBase128> inline static copyAndFind(StringViewPtr source, StringBufferPtr dest) {
			SimdBase128 values(reinterpret_cast<StringViewPtr>(source));
			values.store(dest);
			BackslashAndQuote returnData{};
			returnData.bsBits = static_cast<uint16_t>((values == '\\').toBitMask());
			returnData.quoteBits = static_cast<uint16_t>((values == '\"').toBitMask());
			return returnData;
		}

		inline bool hasQuoteFirst() {
			return ((bsBits - 1) & quoteBits) != 0;
		}

		inline bool hasBackslash() {
			return ((quoteBits - 1) & bsBits) != 0;
		}

		inline int16_t quoteIndex() {
			return _tzcnt_u16(quoteBits);
		}

		inline int16_t backslashIndex() {
			return _tzcnt_u16(bsBits);
		}

	  protected:
		uint16_t quoteBits{};
		uint16_t bsBits{};
	};
	constexpr int32_t stringParseLength{ 16 };
	using BackslashAndQuoteType = BackslashAndQuote<SimdBase128>;
#else
	template<> class BackslashAndQuote<SimdBase64> {
	  public:
		static const uint32_t bytesProcessed = 8;

		BackslashAndQuote<SimdBase64> inline static copyAndFind(StringViewPtr source, StringBufferPtr dest) {
			SimdBase64 values(reinterpret_cast<StringViewPtr>(source));
			values.store(dest);
			BackslashAndQuote returnData{};
			returnData.bsBits = static_cast<uint8_t>((values == '\\').toBitMask());
			returnData.quoteBits = static_cast<uint8_t>((values == '\"').toBitMask());
			return returnData;
		}

		inline bool hasQuoteFirst() {
			return ((bsBits - 1) & quoteBits) != 0;
		}

		inline bool hasBackslash() {
			return ((quoteBits - 1) & bsBits) != 0;
		}

		inline int8_t quoteIndex() {
			return _tzcnt_u16(quoteBits);
		}

		inline int8_t backslashIndex() {
			return _tzcnt_u16(bsBits);
		}

	  protected:
		uint8_t quoteBits{};
		uint8_t bsBits{};
	};
	constexpr int32_t stringParseLength{ 8 };
	using BackslashAndQuoteType = BackslashAndQuote<SimdBase64>;
#endif	

	inline static uint32_t stringToUint32(StringViewPtr str) {
		uint32_t val{ *reinterpret_cast<const uint32_t*>(str) };
		return val;
	}

	inline static uint32_t str4ncmp(StringViewPtr source, const char* atom) {
		return stringToUint32(reinterpret_cast<StringViewPtr>(source)) ^ stringToUint32(reinterpret_cast<StringViewPtr>(atom));
	}

	inline static constexpr uint32_t isNotStructuralOrWhitespace(uint8_t c) {
		return structuralOrWhitespaceNegated[c];
	}

	inline static unsigned char hex2dec(uint8_t hex) {
		return ((hex & 0xf) + (hex >> 6) * 9);
	}

	inline static char32_t hex4ToChar32(const uint8_t* hex) {
		uint32_t value = hex2dec(hex[3]);
		value |= hex2dec(hex[2]) << 4;
		value |= hex2dec(hex[1]) << 8;
		value |= hex2dec(hex[0]) << 12;
		return value;
	}

	template<typename OTy> inline static bool readEscapedUnicode(OTy** value, auto** it) {
		char8_t buffer[4];
		char32_t codepoint = hex4ToChar32(*it);
		auto& facet = std::use_facet<std::codecvt<char32_t, char8_t, mbstate_t>>(std::locale());
		std::mbstate_t mbstate{};
		const char32_t* fromNext;
		char8_t* toNext;
		const auto result = facet.out(mbstate, &codepoint, &codepoint + 1, fromNext, buffer, buffer + 4, toNext);

		if (result != std::codecvt_base::ok) {
			return false;
		}

		if ((toNext - buffer) != 1) [[unlikely]] {
			return false;
		}
		**value = static_cast<char>(buffer[0]);
		++(*value);
		for (size_t x = 0; x < facet.encoding(); ++x) {
			++(*it);
		}
		std::advance(it, 4);
		return true;
	}

	inline static StringBufferPtr parseString(StringViewPtr source, StringBufferPtr dest, size_t length) {
		while (length > 0) {
			BackslashAndQuoteType bsQuote = BackslashAndQuoteType::copyAndFind(source, dest);
			if (bsQuote.hasQuoteFirst()) {
				return dest + bsQuote.quoteIndex();
			}
			if (bsQuote.hasBackslash()) {
				auto bsDist = bsQuote.backslashIndex();
				uint8_t escapeChar = source[bsDist + 1];
				if (escapeChar == 'u') {
					source += bsDist;
					dest += bsDist;
					length -= bsDist;
					if (!readEscapedUnicode(&dest, &source)) {
						return dest;
					}
				} else {
					uint8_t escapeResult = escapeMap[escapeChar];
					if (escapeResult == 0u) {
						return dest;
					}
					dest[bsDist] = escapeResult;
					source += (bsDist + 2ull);
					dest += (bsDist + 1ull);
					length -= (bsDist + 2ull);
				}
			} else {
				source += BackslashAndQuoteType::bytesProcessed;
				dest += BackslashAndQuoteType::bytesProcessed;
				length -= BackslashAndQuoteType::bytesProcessed;
			}
		}
		return dest;
	}

	inline static bool parseBool(StringViewPtr json) noexcept {
		uint32_t notTrue = str4ncmp(json, "true");
		uint32_t notFalse = str4ncmp(json, "fals") | (json[4] ^ 'e');
		bool error = (notTrue && notFalse) || isNotStructuralOrWhitespace(json[notTrue ? 5 : 4]);
		if (error) {
			return {};
		}
		return !notTrue;
	}
}