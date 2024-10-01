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
#include <jsonifier/FastFloat.hpp>
#include <jsonifier/StrToD.hpp>

#include <concepts>
#include <cstdint>
#include <cstring>
#include <array>

namespace jsonifier_internal {

	JSONIFIER_ALWAYS_INLINE_VARIABLE std::array<uint64_t, 20> powerOfTenInt{ 1ull, 10ull, 100ull, 1000ull, 10000ull, 100000ull, 1000000ull, 10000000ull, 100000000ull,
		1000000000ull, 10000000000ull, 100000000000ull, 1000000000000ull, 10000000000000ull, 100000000000000ull, 1000000000000000ull, 10000000000000000ull, 100000000000000000ull,
		1000000000000000000ull, 10000000000000000000ull };

	JSONIFIER_ALWAYS_INLINE_VARIABLE uint8_t digiTypeDot = 1 << 4;
	JSONIFIER_ALWAYS_INLINE_VARIABLE uint8_t digiTypeExp = 1 << 5;

	JSONIFIER_ALWAYS_INLINE_VARIABLE std::array<uint8_t, 256> digiTable = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00,
		0x08, 0x10, 0x00, 0x01, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

	JSONIFIER_ALWAYS_INLINE bool digiIsFp(uint8_t d) noexcept {
		return (digiTable[d] & (digiTypeDot | digiTypeExp)) != 0;
	}

	JSONIFIER_ALWAYS_INLINE constexpr bool isSafeAddition(uint64_t a, uint64_t b) noexcept {
		return a <= (std::numeric_limits<uint64_t>::max)() - b;
	}

	JSONIFIER_ALWAYS_INLINE constexpr bool isSafeMultiplication10(uint64_t a) noexcept {
		constexpr uint64_t b = (std::numeric_limits<uint64_t>::max)() / 10;
		return a <= b;
	}

	JSONIFIER_ALWAYS_INLINE_VARIABLE auto e_bit		   = static_cast<uint8_t>('E' ^ 'e');
	JSONIFIER_ALWAYS_INLINE_VARIABLE auto f64MinDecExp = (-324);
	JSONIFIER_ALWAYS_INLINE_VARIABLE char decimalPoint = '.';
	JSONIFIER_ALWAYS_INLINE_VARIABLE char smallE	   = 'e';
	JSONIFIER_ALWAYS_INLINE_VARIABLE char bigE		   = 'E';
	JSONIFIER_ALWAYS_INLINE_VARIABLE char minus		   = '-';
	JSONIFIER_ALWAYS_INLINE_VARIABLE char plus		   = '+';
	JSONIFIER_ALWAYS_INLINE_VARIABLE char zero		   = '0';

	JSONIFIER_ALWAYS_INLINE_VARIABLE std::array<uint64_t, 256> numberSubTable{ []() {
		std::array<uint64_t, 256> returnValues{};
		for (uint64_t x = 0; x < 256; ++x) {
			returnValues[x] = static_cast<uint64_t>(x - zero);
		}
		return returnValues;
	}() };

	JSONIFIER_ALWAYS_INLINE_VARIABLE std::array<bool, 256> digitTableBool{ []() {
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

	JSONIFIER_ALWAYS_INLINE bool digiFinish(auto& exp, auto& val, auto& sig) {
		if (exp <= -20) [[unlikely]] {
			val = unwrap_t<decltype(val)>(0);
			return true;
		}

		val = static_cast<unwrap_t<decltype(val)>>(sig);
		if (exp >= 0) {
			val *= static_cast<unwrap_t<decltype(val)>>(powerOfTenInt[exp]);
		} else {
			val /= static_cast<unwrap_t<decltype(val)>>(powerOfTenInt[-exp]);
		}
		return true;
	}

	JSONIFIER_ALWAYS_INLINE bool digiExpFinish(auto& sig, auto& val, auto& expSig, auto& exp) {
		if (sig == 0) {
			val = 0;
			return true;
		}
		if (expSig == 19) {
			val *= static_cast<unwrap_t<decltype(val)>>(powerOfTenInt[expSig - 1]);
			if (isSafeMultiplication10(val)) [[likely]] {
				return bool(val *= 10);
			} else [[unlikely]] {
				return false;
			}
		} else if (expSig >= 20) [[unlikely]] {
			return false;
		}
		exp = expSig;
		return digiFinish(exp, val, sig);
	};

	JSONIFIER_ALWAYS_INLINE bool digiExpMore(auto& expSign, auto& cur, auto& tmp, auto& val, auto& expSig, auto& expLit, auto& sig, auto& exp) {
		expSign = (*++cur == minus);
		cur += (*cur == plus || *cur == minus);
		if (uint8_t(numberSubTable[*cur]) > 9) {
			return false;
		}
		while (*cur == zero)
			++cur;
		tmp = cur;
		uint8_t c;
		while (uint8_t(c = numberSubTable[*cur]) < 10) {
			++cur;
			expLit = c + uint32_t(expLit) * 10;
		}
		if ((cur - tmp >= 6)) [[unlikely]] {
			if (sig == 0 || expSign) {
				val = 0;
				return true;
			} else {
				return false;
			}
		}
		expSig += expSign ? -expLit : expLit;
		return digiExpFinish(sig, val, expSig, exp);
	};


	JSONIFIER_ALWAYS_INLINE bool digiFracEnd(auto& expSign, auto& cur, auto& tmp, auto& val, auto& expSig, auto& expLit, auto& sig, auto& exp, auto sigEnd, auto dotPos) {
		sigEnd = cur;
		expSig = -int32_t((cur - dotPos) - 1);
		if (expSig == 0) [[unlikely]]
			return false;
		if ((e_bit | *cur) != smallE) [[likely]] {
			if ((expSig < f64MinDecExp - 19)) [[unlikely]] {
				val = 0;
				return true;
			}
			exp = expSig;
			return digiFinish(exp, val, sig);
		} else {
			return digiExpMore(expSign, cur, tmp, val, expSig, expLit, sig, exp);
		}
	};

	JSONIFIER_ALWAYS_INLINE bool digiStop(auto& expSign, auto& cur, auto& tmp, auto& val, auto& expSig, auto& expLit, auto& sig, auto& exp, auto sigEnd, auto dotPos, auto& fracZeros,
		size_t i) {
		cur += i + 1 + fracZeros;
		return digiFracEnd(expSign, cur, tmp, val, expSig, expLit, sig, exp, sigEnd, dotPos);
	};

	JSONIFIER_ALWAYS_INLINE bool digiFrac(auto& expSign, auto& cur, auto& tmp, auto& val, auto& expSig, auto& expLit, auto& sig, auto& exp, auto sigEnd, auto dotPos, auto& fracZeros,
		auto& numTmp, size_t i) {
		while (i < 21) {
			if (((numTmp = uint64_t(numberSubTable[cur[i + 1 + fracZeros]])) <= 9)) [[likely]] {
				sig = numTmp + sig * 10;

			} else {
				return digiStop(expSign, cur, tmp, val, expSig, expLit, sig, exp, sigEnd, dotPos, fracZeros, i);
			}
			++i;
		}
		return true;
	};

	JSONIFIER_ALWAYS_INLINE bool digiSepr(auto& expSign, auto& cur, auto& tmp, auto& val, auto& expSig, auto& expLit, auto& sig, auto& exp, auto sigEnd, auto dotPos, auto& fracZeros,
		auto& numTmp, size_t i) {
		if ((!digiIsFp(uint8_t(cur[i])))) [[likely]] {
			cur += i;
			val = sig;
			return true;
		}
		dotPos = cur + i;
		if ((cur[i] == decimalPoint)) [[likely]] {
			if (sig == 0)
				while (cur[fracZeros + i + 1] == zero)
					++fracZeros;
			return digiFrac(expSign, cur, tmp, val, expSig, expLit, sig, exp, sigEnd, dotPos, fracZeros, numTmp, i);
		}
		cur += i;
		sigEnd = cur;
		return digiExpMore(expSign, cur, tmp, val, expSig, expLit, sig, exp);
	};

	JSONIFIER_ALWAYS_INLINE bool exprIntg(auto& expSign, auto& cur, auto& tmp, auto& val, auto& expSig, auto& expLit, auto& sig, auto& exp, auto sigEnd, auto dotPos, auto& fracZeros,
		auto& numTmp, size_t i) {
		while (i < 21) {
			if ((numTmp = numberSubTable[cur[i]]) <= 9) [[likely]] {
				sig = numTmp + sig * 10;
			} else {
				if (i > 1) {
					if (*cur == zero)
						return false;
				}
				return digiSepr(expSign, cur, tmp, val, expSig, expLit, sig, exp, sigEnd, dotPos, fracZeros, numTmp, i);
			};
			++i;
		}
		return true;
	};

	template<jsonifier::concepts::integer_t value_type, typename char_type> JSONIFIER_ALWAYS_INLINE bool parseInt(value_type& value, char_type*& cur) noexcept {
		constexpr auto isVolatile = std::is_volatile_v<std::remove_reference_t<decltype(value)>>;
		const char_type* sig_cut;
		[[maybe_unused]] const char_type* sigEnd{};
		const char_type* dotPos{};
		uint32_t fracZeros = 0;
		uint64_t sig	   = uint64_t(numberSubTable[*cur]);
		int32_t exp		   = 0;
		bool expSign;
		int32_t expSig = 0;
		int32_t expLit = 0;
		uint64_t numTmp;
		const char_type* tmp;

		if (sig > 9) [[unlikely]] {
			return false;
		}

		return exprIntg(expSign, cur, tmp, value, expSig, expLit, sig, exp, sigEnd, dotPos, fracZeros, numTmp, 0);
	}

	template<typename value_type, typename char_type> JSONIFIER_ALWAYS_INLINE constexpr bool stoui64(value_type& res, const char_type* c) noexcept {
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

		if (*c == decimalPoint) {
			++c;
			while (digitTableBool[static_cast<uint64_t>(*c)]) {
				consumeDigit();
			}
		}

		if (*c == smallE || *c == bigE) {
			++c;

			bool negative = false;
			if (*c == plus || *c == minus) {
				negative = (*c == minus);
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