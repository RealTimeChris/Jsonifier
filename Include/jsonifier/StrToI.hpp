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
/// Note: Most of the code in this header was sampled from Glaze library: https://github.com/stephenberry/glaze
/// https://github.com/RealTimeChris/jsonifier
/// Nov 13, 2023
#pragma once

#include <jsonifier/Allocator.hpp>

#include <concepts>
#include <cstdint>
#include <cstring>
#include <array>

namespace jsonifier_internal {

	template<typename uint8_t> constexpr std::array<uint8_t, 256> digiTable{ 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u,
		0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u,
		0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x04u, 0x00u, '\b', 0x10u, 0x00u, 0x01u, 0x02u, 0x02u, 0x02u, 0x02u, 0x02u, 0x02u, 0x02u, 0x02u, 0x02u, 0x00u, 0x00u, 0x00u, 0x00u,
		0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x20u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u,
		0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x20u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u,
		0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u };

	constexpr uint8_t digiTypeZero	  = 1 << 0;
	constexpr uint8_t digiTypeNonZero = 1 << 1;
	constexpr uint8_t digiTypeDot	  = 1 << 4;
	constexpr uint8_t digiTypeExp	  = 1 << 5;

	template<typename uint8_t> JSONIFIER_INLINE bool digiIsType(uint8_t d, uint8_t type) noexcept {
		return (digiTable<uint8_t>[static_cast<size_t>(d)] & type) != 0;
	}

	template<typename uint8_t> JSONIFIER_INLINE bool digiIsFp(uint8_t d) noexcept {
		return digiIsType(d, uint8_t(digiTypeDot | digiTypeExp));
	}

	template<typename uint8_t> JSONIFIER_INLINE bool digiIsDigitOrFp(uint8_t d) noexcept {
		return digiIsType(d, uint8_t(digiTypeZero | digiTypeNonZero | digiTypeDot | digiTypeExp));
	}

	constexpr uint8_t zero{ '0' };

	constexpr std::array<size_t, 256> numberSubTable{ []() {
		std::array<size_t, 256> returnValues{};
		for (size_t x = 0; x < 256; ++x) {
			returnValues[x] = static_cast<size_t>(x - zero);
		}
		return returnValues;
	}() };

	constexpr std::array<bool, 256> digitTableBool{ []() {
		std::array<bool, 256> returnValues{};
		returnValues[0x30u] = true;
		returnValues[0x31u] = true;
		returnValues[0x32u] = true;
		returnValues[0x33u] = true;
		returnValues[0x34u] = true;
		returnValues[0x35u] = true;
		returnValues[0x36u] = true;
		returnValues[0x37u] = true;
		returnValues[0x38u] = true;
		returnValues[0x39u] = true;
		return returnValues;
	}() };

	JSONIFIER_INLINE constexpr bool isSafeAddition(size_t a, size_t b) noexcept {
		return a <= (std::numeric_limits<size_t>::max)() - b;
	}

	JSONIFIER_INLINE constexpr bool isSafeMultiplication10(size_t a) noexcept {
		constexpr size_t b = (std::numeric_limits<size_t>::max)() / 10;
		return a <= b;
	}

	template<size_t maxIndex, size_t index = 0, jsonifier::concepts::integer_t value_type, typename iterator> void opReal(value_type& value, iterator&& iter, size_t sig = 0) {
		if constexpr (index < maxIndex) {
			size_t numTmp = numberSubTable[static_cast<uint8_t>(iter[index])];
			sig				= numTmp + sig * 10;
			return opReal<maxIndex, index + 1>(value, iter, sig);
		}
		value = sig;
	}

	template<size_t index, jsonifier::concepts::integer_t value_type, typename iterator> struct int_parser {
		static void op(value_type& value, iterator&& iter) {
			return opReal<index>(value, iter);
		}
	};

	template<jsonifier::concepts::integer_t value_type_new, typename iterator> JSONIFIER_INLINE bool parseInt(value_type_new& value, iterator&& iter, size_t length) {
		static constexpr auto arrayOfParseIntPtrs = generateArrayOfFunctionPtrs<int_parser, value_type_new, iterator>(std::make_index_sequence<21>{});
		if (length < 21) {
			arrayOfParseIntPtrs[length](value, iter);
			return true;
		} else {
			return false;
		}
	}

	template<typename value_type, typename char_type> JSONIFIER_INLINE constexpr bool stoui64(value_type& res, const char_type* c) noexcept {
		if (!digitTableBool[static_cast<size_t>(*c)]) [[unlikely]] {
			return false;
		}

		constexpr std::array<uint32_t, 4> maxDigitsFromSize = { 4, 6, 11, 20 };
		constexpr auto N									= maxDigitsFromSize[static_cast<size_t>(std::bit_width(sizeof(value_type)) - 1)];

		std::array<uint8_t, N> digits{ 0 };
		auto nextDigit	  = digits.begin();
		auto consumeDigit = [&c, &nextDigit, &digits]() {
			if (nextDigit < digits.cend()) [[likely]] {
				*nextDigit = static_cast<uint8_t>(*c - 0x30u);
				++nextDigit;
			}
			++c;
		};

		if (*c == 0x30u) {
			++c;
			++nextDigit;

			if (*c == 0x30u) [[unlikely]] {
				return false;
			}
		}

		while (digitTableBool[static_cast<size_t>(*c)]) {
			consumeDigit();
		}
		auto n = std::distance(digits.begin(), nextDigit);

		if (*c == '.') {
			++c;
			while (digitTableBool[static_cast<size_t>(*c)]) {
				consumeDigit();
			}
		}

		if (*c == 'e' || *c == 'E') {
			++c;

			bool negative = false;
			if (*c == '+' || *c == '-') {
				negative = (*c == '-');
				++c;
			}
			uint8_t exp = 0;
			while (digitTableBool[static_cast<size_t>(*c)] && exp < 128) {
				exp = static_cast<uint8_t>(10 * exp + (*c - 0x30u));
				++c;
			}
			n += negative ? -exp : exp;
		}

		res = 0;
		if (n < 0) [[unlikely]] {
			return true;
		}

		if constexpr (std::is_same_v<value_type, size_t>) {
			if (n > 20) [[unlikely]] {
				return false;
			}

			if (n == 20) [[unlikely]] {
				for (auto k = 0; k < 19; ++k) {
					res = static_cast<value_type>(10) * res + static_cast<value_type>(digits[static_cast<size_t>(k)]);
				}

				if (isSafeMultiplication10(res)) [[likely]] {
					res *= 10;
				} else [[unlikely]] {
					return false;
				}
				if (isSafeAddition(res, digits.back())) [[likely]] {
					res += digits.back();
				} else [[unlikely]] {
					return false;
				}
			} else [[likely]] {
				for (auto k = 0; k < n; ++k) {
					res = static_cast<value_type>(10) * res + static_cast<value_type>(digits[static_cast<size_t>(k)]);
				}
			}
		} else {
			if (n >= N) [[unlikely]] {
				return false;
			} else [[likely]] {
				for (auto k = 0; k < n; ++k) {
					res = static_cast<value_type>(10) * res + static_cast<value_type>(digits[static_cast<size_t>(k)]);
				}
			}
		}

		return true;
	}
}