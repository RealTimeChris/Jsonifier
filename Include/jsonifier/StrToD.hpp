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
/// Feb 3, 2023
#pragma once

#include <jsonifier/FastFloat.hpp>

namespace jsonifier_internal {

	JSONIFIER_ALWAYS_INLINE_VARIABLE uint8_t decimal{ '.' };
	JSONIFIER_ALWAYS_INLINE_VARIABLE uint8_t minus{ '-' };
	JSONIFIER_ALWAYS_INLINE_VARIABLE uint8_t plus{ '+' };
	JSONIFIER_ALWAYS_INLINE_VARIABLE uint8_t zero{ '0' };
	JSONIFIER_ALWAYS_INLINE_VARIABLE uint8_t nine{ '9' };

#define isDigit(x) ((x <= nine) && (x >= zero))

	template<jsonifier::concepts::float_t value_type> struct float_parser {
		static constexpr uint64_t minimalNineteenDigitInteger{ 1000000000000000000 };
		static constexpr char decimalNew = '.';
		static constexpr char smallE	 = 'e';
		static constexpr char bigE		 = 'E';
		static constexpr char minusNew	 = '-';
		static constexpr char plusNew	 = '+';
		static constexpr char zeroNew	 = '0';

		template<typename char_type> struct parsing_pack {
			jsonifier_fast_float::parsed_number_string_t<char_type> answer;
			char_type const* loactionOfE;
			char_type const* startDigits;
			char_type const* fracPtr;
			char_type const* fracEnd;
			char_type const* intPtr;
			char_type const* intEnd;
			size_t digitCount;
			int64_t expNumber;
			uint8_t digit;
		};

		template<typename char_type>
		JSONIFIER_ALWAYS_INLINE static char_type const* parseExponent(char_type const* iter, value_type& value, parsing_pack<char_type>& pack) noexcept {
			using namespace jsonifier_fast_float;
			pack.loactionOfE = iter;
			++iter;
			bool negExp = false;
			if (minusNew == *iter) {
				negExp = true;
				++iter;
			} else if (plusNew == *iter) {
				++iter;
			}
			if (!isDigit(*iter)) {
				iter = pack.loactionOfE;
			} else {
				while (isDigit(*iter)) {
					pack.digit = static_cast<uint8_t>(*iter - zeroNew);
					if (pack.expNumber < 0x10000000) {
						pack.expNumber = 10 * pack.expNumber + pack.digit;
					}
					++iter;
				}
				if (negExp) {
					pack.expNumber = -pack.expNumber;
				}
				pack.answer.exponent += pack.expNumber;
			}
			return finishParse(iter, value, pack);
		}

		template<typename char_type> JSONIFIER_ALWAYS_INLINE static char_type const* finishParse(char_type const* iter, value_type& value, parsing_pack<char_type>& pack) noexcept {
			using namespace jsonifier_fast_float;
			pack.answer.lastmatch = iter;
			pack.answer.valid	  = true;

			if (pack.digitCount > 19) {
				char_type const* start = pack.startDigits;
				while ((*start == zeroNew || *start == decimalNew)) {
					if (*start == zeroNew) {
						--pack.digitCount;
					}
					++start;
				}

				if (pack.digitCount > 19) {
					pack.answer.too_many_digits = true;
					pack.answer.mantissa		= 0;
					while ((pack.answer.mantissa < minimalNineteenDigitInteger) && (pack.intPtr != pack.intEnd)) {
						pack.answer.mantissa = pack.answer.mantissa * 10 + static_cast<uint64_t>(*pack.intPtr - zeroNew);
						++pack.intPtr;
					}
					if (pack.answer.mantissa >= minimalNineteenDigitInteger) {
						pack.answer.exponent = pack.intEnd - pack.intPtr + pack.expNumber;
					} else {
						pack.intPtr = pack.fracPtr;
						while ((pack.answer.mantissa < minimalNineteenDigitInteger) && (pack.intPtr != pack.fracEnd)) {
							pack.answer.mantissa = pack.answer.mantissa * 10 + static_cast<uint64_t>(*pack.intPtr - zeroNew);
							++pack.intPtr;
						}
						pack.answer.exponent = pack.fracPtr - pack.intPtr + pack.expNumber;
					}
				}
			}
			if JSONIFIER_LIKELY (pack.answer.valid) {
				return from_chars_advanced(pack.answer, value);
			} else {
				return nullptr;
			}
		}

		template<typename char_type>
		JSONIFIER_ALWAYS_INLINE static char_type const* parseFractional(char_type const* iter, value_type& value, parsing_pack<char_type>& pack) noexcept {
			using namespace jsonifier_fast_float;
			++iter;

			while (isDigit(*iter)) {
				pack.digit = static_cast<uint8_t>(*iter - zeroNew);
				++iter;
				pack.answer.mantissa = pack.answer.mantissa * 10 + pack.digit;
			}
			pack.answer.exponent = (pack.intEnd + 1) - iter;
			pack.fracEnd		 = iter;
			pack.fracPtr		 = pack.intEnd + 1;
			pack.digitCount -= pack.answer.exponent;

			if (pack.answer.exponent == 0) {
				return nullptr;
			}
			pack.expNumber = 0;

			if ((smallE == *iter) || (bigE == *iter)) {
				return parseExponent(iter, value, pack);
			}

			return finishParse(iter, value, pack);
		}

		template<typename char_type> JSONIFIER_ALWAYS_INLINE static char_type const* parseFloatImpl(char_type const* iter, value_type& value) noexcept {
			using namespace jsonifier_fast_float;
			static_assert(is_supported_float_t<value_type>(), "only some floating-point types are supported");
			static_assert(is_supported_char_t<char_type>(), "only char, wchar_t, char16_t and char32_t are supported");
			parsing_pack<char_type> pack;
			pack.answer.valid			= false;
			pack.answer.too_many_digits = false;
			pack.answer.negative		= (*iter == minusNew);
			pack.expNumber				= 0;
			if (pack.answer.negative) {
				++iter;

				if JSONIFIER_UNLIKELY (!isDigit(*iter)) {
					return nullptr;
				}
			}
			pack.startDigits = iter;

			while (isDigit(*iter)) {
				pack.digit = static_cast<uint64_t>(*iter - zeroNew);
				++iter;
				pack.answer.mantissa = 10 * pack.answer.mantissa + pack.digit;
			}

			pack.intEnd		= iter;
			pack.digitCount = static_cast<int64_t>(pack.intEnd - pack.startDigits);
			pack.intPtr		= pack.startDigits;

			if (pack.digitCount == 0 || (pack.startDigits[0] == zeroNew && pack.digitCount > 1)) {
				return nullptr;
			}

			if (*iter == decimalNew) {
				return parseFractional(iter, value, pack);
			}
			if ((smallE == *iter) || (bigE == *iter)) {
				return parseExponent(iter, value, pack);
			}
			return finishParse(iter, value, pack);
		}

		template<typename char_type> JSONIFIER_ALWAYS_INLINE static bool parseFloat(char_type const*& iter, value_type& value) noexcept {
			auto iterNew = parseFloatImpl(iter, value);
			return (iterNew) ? (iter = iterNew, true) : false;
		}
	};
}