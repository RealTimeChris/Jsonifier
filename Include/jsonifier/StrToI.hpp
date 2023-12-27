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

	constexpr std::array<bool, 256> whitespaceTable{ false, false, false, false, false, false, false, false, false, true, true, false, false, true, true, false, false, false,
		false, false, false, false, false, false, false, false, false, false, false, false, false, false, true, false, false, false, false, false, false, false, false, false,
		false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,
		false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,
		false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,
		false, false, false, false, false, false, false, false, false, false, false, false, false, false };

	template<jsonifier::concepts::unsigned_type value_type, typename char_type> JSONIFIER_INLINE bool parseNumberImpl(value_type& value, char_type* cur, uint64_t length) {
		constexpr std::remove_const_t<char_type> zero{ '0' };
		constexpr std::remove_const_t<char_type> x{ 'x' };
		std::remove_const_t<char_type> currentChar{ *cur };
		uint64_t sig{};

		if (!digitTable[currentChar] || currentChar == zero && digitTable[*(cur + 1)] || *(cur + 1) == x) [[unlikely]] {
			return false;
		}

		while (digitTable[currentChar]) {
			sig = sig * 10 + (currentChar - zero);
			++cur;
			currentChar = *cur;
		}

		value = static_cast<value_type>(sig);
		return true;
	}

	template<jsonifier::concepts::integer_t value_type, typename char_type> JSONIFIER_INLINE bool parseNumber(value_type& value, char_type* curNew, uint64_t length) {
		static constexpr auto maximum = uint64_t((std::numeric_limits<value_type>::max)());
		if constexpr (jsonifier::concepts::unsigned_type<value_type>) {
			if constexpr (std::same_as<value_type, uint64_t>) {
				if (*curNew == '-') [[unlikely]] {
					return false;
				}
				if constexpr (std::is_volatile_v<decltype(value)>) {
					uint64_t i{};
					if (!parseNumberImpl<uint64_t, char_type>(i, curNew)) [[unlikely]] {
						return false;
					}
					value = i;
				} else {
					if (!parseNumberImpl<jsonifier::concepts::decay_keep_volatile_t<decltype(value)>, char_type>(value, curNew, length)) [[unlikely]] {
						return false;
					}
				}
			} else {
				uint64_t i{};
				if (*curNew == '-') [[unlikely]] {
					return false;
				}
				if (!parseNumberImpl<std::decay_t<decltype(i)>, char_type>(i, curNew, length)) [[unlikely]] {
					return false;
				}

				if (i > maximum) [[unlikely]] {
					return false;
				}
				value = static_cast<value_type>(i);
			}
		} else {
			uint64_t i{};
			int64_t sign = 1;
			if (*curNew == '-') {
				sign = -1;
				--length;
				++curNew;
			}

			if (!parseNumberImpl<jsonifier::concepts::decay_keep_volatile_t<decltype(i)>, char_type>(i, curNew, length)) [[unlikely]] {
				return false;
			}

			if (sign == -1) {
				static constexpr auto min_abs = uint64_t((std::numeric_limits<value_type>::max)()) + 1;
				if (i > min_abs) [[unlikely]] {
					return false;
				}
				value = static_cast<value_type>(sign * i);
			} else {
				if (i > maximum) [[unlikely]] {
					return false;
				}
				value = static_cast<value_type>(i);
			}
		}
		return true;
	}

	template<typename value_type, typename char_type> constexpr bool stoui64(value_type& res, const char_type* c) noexcept {
		if (!digitTable[*c]) [[unlikely]] {
			return false;
		}

		constexpr std::array<uint32_t, 4> max_digits_from_size = { 4, 6, 11, 20 };
		constexpr auto N									   = max_digits_from_size[static_cast<uint64_t>(std::bit_width(sizeof(value_type)) - 1)];

		std::array<uint8_t, N> digits{ 0 };
		auto next_digit	   = digits.begin();
		auto consume_digit = [&c, &next_digit, &digits]() {
			if (next_digit < digits.cend()) [[likely]] {
				*next_digit = static_cast<uint8_t>(*c - '0');
				++next_digit;
			}
			++c;
		};

		if (*c == '0') {
			++c;
			++next_digit;

			if (*c == '0') [[unlikely]] {
				return false;
			}
		}

		while (digitTable[*c]) {
			consume_digit();
		}
		auto n = std::distance(digits.begin(), next_digit);

		if (*c == '.') {
			++c;
			while (digitTable[*c]) {
				consume_digit();
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
			while (digitTable[*c] && exp < 128) {
				exp = static_cast<uint8_t>(10 * exp + (*c - '0'));
				++c;
			}
			n += negative ? -exp : exp;
		}

		res = 0;
		if (n < 0) [[unlikely]] {
			return true;
		}

		if constexpr (std::same_as<value_type, uint64_t>) {
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

	template<typename value_type, typename char_type> constexpr bool stoui64(value_type& res, char_type* curNew) noexcept {
		static_assert(sizeof(*curNew) == sizeof(char));
		const char_type* cur = reinterpret_cast<const char_type*>(&*curNew);
		if (stoui64(res, cur)) {
			return true;
		}
		return false;
	}
}