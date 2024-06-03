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

	constexpr bool isSafeAddition(uint64_t a, uint64_t b) noexcept {
		return a <= (std::numeric_limits<uint64_t>::max)() - b;
	}

	constexpr bool isSafeMultiplication10(uint64_t a) noexcept {
		constexpr uint64_t b = (std::numeric_limits<uint64_t>::max)() / 10;
		return a <= b;
	}

	template<jsonifier::concepts::integer_t value_type_new> JSONIFIER_INLINE bool parseInt(value_type_new& value, auto& iter) {
		using value_type		  = jsonifier::concepts::unwrap_t<value_type_new>;
		constexpr auto isVolatile = std::is_volatile_v<std::remove_reference_t<decltype(value)>>;
		using char_type			  = decltype(iter);
		uint64_t sig			  = uint64_t(numberSubTable[static_cast<uint64_t>(*iter)]);
		uint64_t numTmp;

		if (sig > 9) [[unlikely]] {
			return false;
		}

		constexpr auto zero = uint8_t('0');
#define expr_intg(i) \
	if (numTmp = numberSubTable[iter[i]]; numTmp <= 9) [[likely]] \
		sig = numTmp + sig * 10; \
	else { \
		if constexpr (i > 1) { \
			if (*iter == zero) \
				return false; \
		} \
		goto digi_sepr_##i; \
	}
		repeat_in_1_18(expr_intg);
#undef expr_intg

		if (*iter == zero)
			return false;

		iter += 19;
		if (!digiIsDigitOrFp(*iter)) {
			value = static_cast<value_type>(sig);
			return true;
		}

#define expr_sepr(i) \
	digi_sepr_##i : if (!digiIsFp(uint8_t(iter[i]))) [[likely]] { \
		iter += i; \
		value = sig; \
		return true; \
	} \
	iter += i; \
	return false;
		repeat_in_1_18(expr_sepr)
#undef expr_sepr
			return false;
	}

	template<typename value_type, typename char_type> constexpr bool stoui64(value_type& res, const char_type* c) noexcept {
		if (!digitTableBool[static_cast<uint64_t>(*c)]) [[unlikely]] {
			return false;
		}

		constexpr std::array<uint32_t, 4> maxDigitsFromSize = { 4, 6, 11, 20 };
		constexpr auto N									= maxDigitsFromSize[static_cast<uint64_t>(std::bit_width(sizeof(value_type)) - 1)];

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

		while (digitTableBool[static_cast<uint64_t>(*c)]) {
			consumeDigit();
		}
		auto n = std::distance(digits.begin(), nextDigit);

		if (*c == '.') {
			++c;
			while (digitTableBool[static_cast<uint64_t>(*c)]) {
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
			while (digitTableBool[static_cast<uint64_t>(*c)] && exp < 128) {
				exp = static_cast<uint8_t>(10 * exp + (*c - 0x30u));
				++c;
			}
			n += negative ? -exp : exp;
		}

		res = 0;
		if (n < 0) [[unlikely]] {
			return true;
		}

		if constexpr (std::is_same_v<value_type, uint64_t>) {
			if (n > 20) [[unlikely]] {
				return false;
			}

			if (n == 20) [[unlikely]] {
				for (auto k = 0; k < 19; ++k) {
					res = static_cast<value_type>(10) * res + static_cast<value_type>(digits[static_cast<uint64_t>(k)]);
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
					res = static_cast<value_type>(10) * res + static_cast<value_type>(digits[static_cast<uint64_t>(k)]);
				}
			}
		} else {
			if (n >= N) [[unlikely]] {
				return false;
			} else [[likely]] {
				for (auto k = 0; k < n; ++k) {
					res = static_cast<value_type>(10) * res + static_cast<value_type>(digits[static_cast<uint64_t>(k)]);
				}
			}
		}

		return true;
	}
}