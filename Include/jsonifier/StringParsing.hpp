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
/// https://github.com/RealTimeChris/Jsonifier
/// Feb 3, 2023
/// Most of the code in this header was sampled fromSimdjson - https://github.com/simdjson
#pragma once

#include <jsonifier/Base.hpp>
#include <jsonifier/Tables.hpp>
#include <jsonifier/Simd.hpp>
#include <codecvt>

namespace JsonifierInternal {

	template<typename SimdBase> class BackslashAndQuote;

	template<> class BackslashAndQuote<SimdBaseReal> {
	  public:
		static const uint32_t bytesProcessed = BytesPerStep;

		BackslashAndQuote<SimdBaseReal> inline static copyAndFind(StringViewPtr source, StringBufferPtr dest) {
			SimdBaseReal values(reinterpret_cast<StringViewPtr>(source));
			values.store(dest);
			BackslashAndQuote returnData{};
			returnData.bsBits = { values == char{ '\\' } };
			returnData.quoteBits = { values == char{ '\"' } };
			return returnData;
		}

		inline bool hasQuoteFirst() {
			return ((bsBits - 1) & quoteBits) != 0;
		}

		inline bool hasBackslash() {
			return ((quoteBits - 1) & bsBits) != 0;
		}

		inline StringParsingType quoteIndex() {
			return tzCount(quoteBits);
		}

		inline StringParsingType backslashIndex() {
			return tzCount(bsBits);
		}

	  protected:
		StringParsingType quoteBits{};
		StringParsingType bsBits{};
	};
	using BackslashAndQuoteType = BackslashAndQuote<SimdBaseReal>;

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

	template<typename ValueType> inline static bool readEscapedUnicode(ValueType** value, auto** it) {
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