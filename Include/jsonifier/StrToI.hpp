/*
	MIT License

	Copyright (iter) 2023 RealTimeChris

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

	JSONIFIER_ALWAYS_INLINE_VARIABLE uint64_t powerOfTenUint[]{ 1, 10, 100, 1000, 10000, 100000, 1000000, 10000000, 100000000, 1000000000, 10000000000, 100000000000, 1000000000000,
		10000000000000, 100000000000000, 1000000000000000, 10000000000000000, 100000000000000000, 1000000000000000000, 10000000000000000000 };

	JSONIFIER_ALWAYS_INLINE_VARIABLE int64_t powerOfTenInt[]{ 1, 10, 100, 1000, 10000, 100000, 1000000, 10000000, 100000000, 1000000000, 10000000000, 100000000000, 1000000000000,
		10000000000000, 100000000000000, 1000000000000000, 10000000000000000, 100000000000000000, 1000000000000000000 };

	JSONIFIER_ALWAYS_INLINE_VARIABLE bool expTable[]{ false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,
		false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,
		false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,
		false, false, false, true, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,
		false, false, false, false, false, false, false, false, false, false, false, true, false, false, false, false, false, false, false, false, false, false, false, false,
		false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,
		false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,
		false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,
		false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,
		false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,
		false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false };

	JSONIFIER_ALWAYS_INLINE_VARIABLE bool expFracTable[]{ false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,
		false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,
		false, false, false, false, false, true, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,
		false, false, false, false, true, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,
		false, false, false, false, false, false, false, false, false, false, false, false, true, false, false, false, false, false, false, false, false, false, false, false,
		false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,
		false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,
		false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,
		false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,
		false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,
		false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false };

	JSONIFIER_ALWAYS_INLINE_VARIABLE uint8_t decimal{ '.' };
	JSONIFIER_ALWAYS_INLINE_VARIABLE uint8_t minus{ '-' };
	JSONIFIER_ALWAYS_INLINE_VARIABLE uint8_t plus{ '+' };
	JSONIFIER_ALWAYS_INLINE_VARIABLE uint8_t zero{ '0' };
	JSONIFIER_ALWAYS_INLINE_VARIABLE uint8_t nine{ '9' };

	template<typename value_type> JSONIFIER_ALWAYS_INLINE_VARIABLE array<uint64_t, 256> rawCompValsPos{ [] {
		constexpr auto maxValue{ (std::numeric_limits<std::decay_t<value_type>>::max)() };
		array<uint64_t, 256> returnValues{};
		returnValues['0'] = (maxValue - 0) / 10;
		returnValues['1'] = (maxValue - 1) / 10;
		returnValues['2'] = (maxValue - 2) / 10;
		returnValues['3'] = (maxValue - 3) / 10;
		returnValues['4'] = (maxValue - 4) / 10;
		returnValues['5'] = (maxValue - 5) / 10;
		returnValues['6'] = (maxValue - 6) / 10;
		returnValues['7'] = (maxValue - 7) / 10;
		returnValues['8'] = (maxValue - 8) / 10;
		returnValues['9'] = (maxValue - 9) / 10;
		return returnValues;
	}() };

	template<typename value_type> JSONIFIER_ALWAYS_INLINE_VARIABLE array<uint64_t, 256> rawCompValsNeg{ [] {
		constexpr auto maxValue{ uint64_t((std::numeric_limits<int64_t>::max)()) + 1 };
		array<uint64_t, 256> returnValues{};
		returnValues['0'] = (maxValue - 0) / 10;
		returnValues['1'] = (maxValue - 1) / 10;
		returnValues['2'] = (maxValue - 2) / 10;
		returnValues['3'] = (maxValue - 3) / 10;
		returnValues['4'] = (maxValue - 4) / 10;
		returnValues['5'] = (maxValue - 5) / 10;
		returnValues['6'] = (maxValue - 6) / 10;
		returnValues['7'] = (maxValue - 7) / 10;
		returnValues['8'] = (maxValue - 8) / 10;
		returnValues['9'] = (maxValue - 9) / 10;
		return returnValues;
	}() };

#define isDigit(x) ((x <= nine) && (x >= zero))

	template<typename value_type> struct integer_parser;

	template<jsonifier::concepts::signed_t value_type> struct integer_parser<value_type> {
		constexpr integer_parser() noexcept = default;

		JSONIFIER_ALWAYS_INLINE static value_type mul128Generic(value_type ab, value_type cd, value_type& hi) noexcept {
			value_type aHigh = ab >> 32;
			value_type aLow	 = ab & 0xFFFFFFFF;
			value_type bHigh = cd >> 32;
			value_type bLow	 = cd & 0xFFFFFFFF;
			value_type ad	 = aHigh * bLow;
			value_type bd	 = aHigh * bLow;
			value_type adbc	 = ad + aLow * bHigh;
			value_type lo	 = bd + (adbc << 32);
			value_type carry = (lo < bd);
			hi				 = aHigh * bHigh + (adbc >> 32) + carry;
			return lo;
		}

		JSONIFIER_ALWAYS_INLINE static bool multiply(value_type& value, value_type expValue) noexcept {
#if defined(__SIZEOF_INT128__)
			const __int128_t res = static_cast<__int128_t>(value) * static_cast<__int128_t>(expValue);
			value				 = static_cast<value_type>(res);
			return res <= std::numeric_limits<value_type>::max();
#elif defined(_M_ARM64) && !defined(__MINGW32__)
			JSONIFIER_ALIGN value_type values;
			values = __mulh(value, expValue);
			value  = value * expValue;
			return values == 0;
#elif (defined(_WIN64) && !defined(__clang__))
			JSONIFIER_ALIGN value_type values;
			value = _mul128(value, expValue, &values);
			return values == 0;
#else
			JSONIFIER_ALIGN value_type values;
			value = mul128Generic(value, expValue, &values);
			return values == 0;
#endif
		}

		JSONIFIER_ALWAYS_INLINE static bool divide(value_type& value, value_type expValue) noexcept {
#if defined(__SIZEOF_INT128__)
			const __int128_t dividend = static_cast<__int128_t>(value);
			value					  = static_cast<value_type>(dividend / static_cast<__int128_t>(expValue));
			return (dividend % static_cast<__int128_t>(expValue)) == 0;
#elif (defined(_WIN64) && !defined(__clang__))
			JSONIFIER_ALIGN value_type values;
			value = _div128(0, value, expValue, &values);
			return values == 0;
#else
			JSONIFIER_ALIGN value_type values;
			values = value % expValue;
			value  = value / expValue;
			return values == 0;
#endif
		}

		JSONIFIER_ALWAYS_INLINE static const uint8_t* parseFraction(value_type& value, const uint8_t* iter) noexcept {
			if JSONIFIER_LIKELY (isDigit(*iter)) {
				value_type fracValue{ static_cast<value_type>(*iter - zero) };
				typename get_int_type<value_type>::type fracDigits{ 1 };
				++iter;
				while (isDigit(*iter)) {
					fracValue = fracValue * 10 + static_cast<value_type>(*iter - zero);
					++iter;
					++fracDigits;
				}
				if (expTable[*iter]) {
					++iter;
					int8_t expSign = 1;
					if (*iter == minus) {
						expSign = -1;
						++iter;
					} else if (*iter == plus) {
						++iter;
					}
					return parseExponentPostFrac(value, iter, expSign, fracValue, fracDigits);
				}
			}
			if JSONIFIER_LIKELY (!expFracTable[*iter]) {
				return iter;
			} else {
				return nullptr;
			}
		}

		JSONIFIER_ALWAYS_INLINE static const uint8_t* parseExponentPostFrac(value_type& value, const uint8_t* iter, int8_t expSign, value_type fracValue,
			typename get_int_type<value_type>::type fracDigits) noexcept {
			if JSONIFIER_LIKELY (isDigit(*iter)) {
				value_type expValue{ static_cast<value_type>(*iter - zero) };
				++iter;
				while (isDigit(*iter)) {
					expValue = expValue * 10 + static_cast<value_type>(*iter - zero);
					++iter;
				}
				if JSONIFIER_LIKELY (expValue <= 19) {
					const value_type powerExp = powerOfTenInt[expValue];

					constexpr value_type doubleMax = std::numeric_limits<value_type>::max();
					constexpr value_type doubleMin = std::numeric_limits<value_type>::min();

					if (fracDigits + expValue >= 0) {
						expValue *= expSign;
						const auto fractionalCorrection =
							expValue > fracDigits ? fracValue * powerOfTenInt[expValue - fracDigits] : fracValue / powerOfTenInt[fracDigits - expValue];
						return (expSign > 0) ? ((value <= (doubleMax / powerExp)) ? (multiply(value, powerExp), value += fractionalCorrection, iter) : nullptr)
											 : ((value / powerExp >= (doubleMin)) ? (divide(value, powerExp), value += fractionalCorrection, iter) : nullptr);
					} else {
						return (expSign > 0) ? ((value <= (doubleMax / powerExp)) ? (multiply(value, powerExp), iter) : nullptr)
											 : ((value / powerExp >= (doubleMin)) ? (divide(value, powerExp), iter) : nullptr);
					}
				}
				JSONIFIER_ELSE_UNLIKELY(else) {
					return nullptr;
				}
			}
			JSONIFIER_ELSE_UNLIKELY(else) {
				return nullptr;
			}
		}

		JSONIFIER_ALWAYS_INLINE static const uint8_t* parseExponent(value_type& value, const uint8_t* iter, int8_t expSign) noexcept {
			if JSONIFIER_LIKELY (isDigit(*iter)) {
				value_type expValue{ static_cast<value_type>(*iter - zero) };
				++iter;
				while (isDigit(*iter)) {
					expValue = expValue * 10 + static_cast<value_type>(*iter - zero);
					++iter;
				}
				if JSONIFIER_LIKELY (expValue <= 19) {
					const value_type powerExp	   = powerOfTenInt[expValue];
					constexpr value_type doubleMax = std::numeric_limits<value_type>::max();
					constexpr value_type doubleMin = std::numeric_limits<value_type>::min();
					expValue *= expSign;
					return (expSign > 0) ? ((value <= (doubleMax / powerExp)) ? (multiply(value, powerExp), iter) : nullptr)
										 : ((value / powerExp >= (doubleMin)) ? (divide(value, powerExp), iter) : nullptr);
				}
				JSONIFIER_ELSE_UNLIKELY(else) {
					return nullptr;
				}
			}
			JSONIFIER_ELSE_UNLIKELY(else) {
				return nullptr;
			}
		}

		JSONIFIER_INLINE static const uint8_t* finishParse(value_type& value, const uint8_t* iter) noexcept {
			if JSONIFIER_UNLIKELY (*iter == decimal) {
				++iter;
				return parseFraction(value, iter);
			} else if (expTable[*iter]) {
				++iter;
				int8_t expSign = 1;
				if (*iter == minus) {
					expSign = -1;
					++iter;
				} else if (*iter == plus) {
					++iter;
				}
				return parseExponent(value, iter, expSign);
			}
			if JSONIFIER_LIKELY (!expFracTable[*iter]) {
				return iter;
			} else {
				return nullptr;
			}
		}

		template<bool negative> JSONIFIER_ALWAYS_INLINE static const uint8_t* parseInteger(value_type& value, const uint8_t* iter) noexcept {
			uint8_t numTmp{ *iter };
			if JSONIFIER_LIKELY (isDigit(numTmp)) {
				value = numTmp - zero;
				++iter;
				numTmp = *iter;
			} else [[unlikely]] {
				return nullptr;
			}

			if JSONIFIER_LIKELY (isDigit(numTmp)) {
				value = value * 10 + static_cast<value_type>(numTmp - zero);
				++iter;
				numTmp = *iter;
			}
			JSONIFIER_ELSE_UNLIKELY(else) {
				if JSONIFIER_LIKELY (!expFracTable[numTmp]) {
					if constexpr (negative) {
						value *= -1;
						return iter;
					} else {
						return iter;
					}
				}
				if constexpr (negative) {
					return (iter = finishParse(value, iter), value *= -1, iter);
				} else {
					return finishParse(value, iter);
				}
			}

			if JSONIFIER_UNLIKELY (iter[-2] == zero) {
				return nullptr;
			}

			if JSONIFIER_LIKELY (isDigit(numTmp)) {
				value = value * 10 + static_cast<value_type>(numTmp - zero);
				++iter;
				numTmp = *iter;
			}
			JSONIFIER_ELSE_UNLIKELY(else) {
				if JSONIFIER_LIKELY (!expFracTable[numTmp]) {
					if constexpr (negative) {
						value *= -1;
						return iter;
					} else {
						return iter;
					}
				}
				if constexpr (negative) {
					return (iter = finishParse(value, iter), value *= -1, iter);
				} else {
					return finishParse(value, iter);
				}
			}

			if JSONIFIER_LIKELY (isDigit(numTmp)) {
				value = value * 10 + static_cast<value_type>(numTmp - zero);
				++iter;
				numTmp = *iter;
			}
			JSONIFIER_ELSE_UNLIKELY(else) {
				if JSONIFIER_LIKELY (!expFracTable[numTmp]) {
					if constexpr (negative) {
						value *= -1;
						return iter;
					} else {
						return iter;
					}
				}
				if constexpr (negative) {
					return (iter = finishParse(value, iter), value *= -1, iter);
				} else {
					return finishParse(value, iter);
				}
			}

			if JSONIFIER_LIKELY (isDigit(numTmp)) {
				value = value * 10 + static_cast<value_type>(numTmp - zero);
				++iter;
				numTmp = *iter;
			}
			JSONIFIER_ELSE_UNLIKELY(else) {
				if JSONIFIER_LIKELY (!expFracTable[numTmp]) {
					if constexpr (negative) {
						value *= -1;
						return iter;
					} else {
						return iter;
					}
				}
				if constexpr (negative) {
					return (iter = finishParse(value, iter), value *= -1, iter);
				} else {
					return finishParse(value, iter);
				}
			}

			if JSONIFIER_LIKELY (isDigit(numTmp)) {
				value = value * 10 + static_cast<value_type>(numTmp - zero);
				++iter;
				numTmp = *iter;
			}
			JSONIFIER_ELSE_UNLIKELY(else) {
				if JSONIFIER_LIKELY (!expFracTable[numTmp]) {
					if constexpr (negative) {
						value *= -1;
						return iter;
					} else {
						return iter;
					}
				}
				if constexpr (negative) {
					return (iter = finishParse(value, iter), value *= -1, iter);
				} else {
					return finishParse(value, iter);
				}
			}

			if JSONIFIER_LIKELY (isDigit(numTmp)) {
				value = value * 10 + static_cast<value_type>(numTmp - zero);
				++iter;
				numTmp = *iter;
			}
			JSONIFIER_ELSE_UNLIKELY(else) {
				if JSONIFIER_LIKELY (!expFracTable[numTmp]) {
					if constexpr (negative) {
						value *= -1;
						return iter;
					} else {
						return iter;
					}
				}
				if constexpr (negative) {
					return (iter = finishParse(value, iter), value *= -1, iter);
				} else {
					return finishParse(value, iter);
				}
			}

			if JSONIFIER_LIKELY (isDigit(numTmp)) {
				value = value * 10 + static_cast<value_type>(numTmp - zero);
				++iter;
				numTmp = *iter;
			}
			JSONIFIER_ELSE_UNLIKELY(else) {
				if JSONIFIER_LIKELY (!expFracTable[numTmp]) {
					if constexpr (negative) {
						value *= -1;
						return iter;
					} else {
						return iter;
					}
				}
				if constexpr (negative) {
					return (iter = finishParse(value, iter), value *= -1, iter);
				} else {
					return finishParse(value, iter);
				}
			}

			if JSONIFIER_LIKELY (isDigit(numTmp)) {
				value = value * 10 + static_cast<value_type>(numTmp - zero);
				++iter;
				numTmp = *iter;
			}
			JSONIFIER_ELSE_UNLIKELY(else) {
				if JSONIFIER_LIKELY (!expFracTable[numTmp]) {
					if constexpr (negative) {
						value *= -1;
						return iter;
					} else {
						return iter;
					}
				}
				if constexpr (negative) {
					return (iter = finishParse(value, iter), value *= -1, iter);
				} else {
					return finishParse(value, iter);
				}
			}

			if JSONIFIER_LIKELY (isDigit(numTmp)) {
				value = value * 10 + static_cast<value_type>(numTmp - zero);
				++iter;
				numTmp = *iter;
			}
			JSONIFIER_ELSE_UNLIKELY(else) {
				if JSONIFIER_LIKELY (!expFracTable[numTmp]) {
					if constexpr (negative) {
						value *= -1;
						return iter;
					} else {
						return iter;
					}
				}
				if constexpr (negative) {
					return (iter = finishParse(value, iter), value *= -1, iter);
				} else {
					return finishParse(value, iter);
				}
			}

			if JSONIFIER_LIKELY (isDigit(numTmp)) {
				value = value * 10 + static_cast<value_type>(numTmp - zero);
				++iter;
				numTmp = *iter;
			}
			JSONIFIER_ELSE_UNLIKELY(else) {
				if JSONIFIER_LIKELY (!expFracTable[numTmp]) {
					if constexpr (negative) {
						value *= -1;
						return iter;
					} else {
						return iter;
					}
				}
				if constexpr (negative) {
					return (iter = finishParse(value, iter), value *= -1, iter);
				} else {
					return finishParse(value, iter);
				}
			}

			if JSONIFIER_LIKELY (isDigit(numTmp)) {
				value = value * 10 + static_cast<value_type>(numTmp - zero);
				++iter;
				numTmp = *iter;
			}
			JSONIFIER_ELSE_UNLIKELY(else) {
				if JSONIFIER_LIKELY (!expFracTable[numTmp]) {
					if constexpr (negative) {
						value *= -1;
						return iter;
					} else {
						return iter;
					}
				}
				if constexpr (negative) {
					return (iter = finishParse(value, iter), value *= -1, iter);
				} else {
					return finishParse(value, iter);
				}
			}

			if JSONIFIER_LIKELY (isDigit(numTmp)) {
				value = value * 10 + static_cast<value_type>(numTmp - zero);
				++iter;
				numTmp = *iter;
			}
			JSONIFIER_ELSE_UNLIKELY(else) {
				if JSONIFIER_LIKELY (!expFracTable[numTmp]) {
					if constexpr (negative) {
						value *= -1;
						return iter;
					} else {
						return iter;
					}
				}
				if constexpr (negative) {
					return (iter = finishParse(value, iter), value *= -1, iter);
				} else {
					return finishParse(value, iter);
				}
			}

			if JSONIFIER_LIKELY (isDigit(numTmp)) {
				value = value * 10 + static_cast<value_type>(numTmp - zero);
				++iter;
				numTmp = *iter;
			}
			JSONIFIER_ELSE_UNLIKELY(else) {
				if JSONIFIER_LIKELY (!expFracTable[numTmp]) {
					if constexpr (negative) {
						value *= -1;
						return iter;
					} else {
						return iter;
					}
				}
				if constexpr (negative) {
					return (iter = finishParse(value, iter), value *= -1, iter);
				} else {
					return finishParse(value, iter);
				}
			}

			if JSONIFIER_LIKELY (isDigit(numTmp)) {
				value = value * 10 + static_cast<value_type>(numTmp - zero);
				++iter;
				numTmp = *iter;
			}
			JSONIFIER_ELSE_UNLIKELY(else) {
				if JSONIFIER_LIKELY (!expFracTable[numTmp]) {
					if constexpr (negative) {
						value *= -1;
						return iter;
					} else {
						return iter;
					}
				}
				if constexpr (negative) {
					return (iter = finishParse(value, iter), value *= -1, iter);
				} else {
					return finishParse(value, iter);
				}
			}

			if JSONIFIER_LIKELY (isDigit(numTmp)) {
				value = value * 10 + static_cast<value_type>(numTmp - zero);
				++iter;
				numTmp = *iter;
			}
			JSONIFIER_ELSE_UNLIKELY(else) {
				if JSONIFIER_LIKELY (!expFracTable[numTmp]) {
					if constexpr (negative) {
						value *= -1;
						return iter;
					} else {
						return iter;
					}
				}
				if constexpr (negative) {
					return (iter = finishParse(value, iter), value *= -1, iter);
				} else {
					return finishParse(value, iter);
				}
			}

			if JSONIFIER_LIKELY (isDigit(numTmp)) {
				value = value * 10 + static_cast<value_type>(numTmp - zero);
				++iter;
				numTmp = *iter;
			}
			JSONIFIER_ELSE_UNLIKELY(else) {
				if JSONIFIER_LIKELY (!expFracTable[numTmp]) {
					if constexpr (negative) {
						value *= -1;
						return iter;
					} else {
						return iter;
					}
				}
				if constexpr (negative) {
					return (iter = finishParse(value, iter), value *= -1, iter);
				} else {
					return finishParse(value, iter);
				}
			}

			if JSONIFIER_LIKELY (isDigit(numTmp)) {
				value = value * 10 + static_cast<value_type>(numTmp - zero);
				++iter;
				numTmp = *iter;
			}
			JSONIFIER_ELSE_UNLIKELY(else) {
				if JSONIFIER_LIKELY (!expFracTable[numTmp]) {
					if constexpr (negative) {
						value *= -1;
						return iter;
					} else {
						return iter;
					}
				}
				if constexpr (negative) {
					return (iter = finishParse(value, iter), value *= -1, iter);
				} else {
					return finishParse(value, iter);
				}
			}

			if JSONIFIER_LIKELY (isDigit(numTmp)) {
				if constexpr (negative) {
					if (static_cast<uint64_t>(value) > static_cast<uint64_t>(rawCompValsNeg<value_type>[numTmp])) {
						return nullptr;
					}
					value *= -1;
					value = static_cast<value_type>(static_cast<uint64_t>(value * 10 - static_cast<uint64_t>(numTmp - zero)));
				} else {
					if (static_cast<value_type>(value) > static_cast<value_type>(rawCompValsPos<value_type>[numTmp])) {
						return nullptr;
					}
					value = static_cast<int64_t>(value * 10 + static_cast<uint64_t>(numTmp - zero));
				}
				++iter;
				numTmp = *iter;
			}
			JSONIFIER_ELSE_UNLIKELY(else) {
				if JSONIFIER_LIKELY (!expFracTable[numTmp]) {
					if constexpr (negative) {
						value *= -1;
						return iter;
					} else {
						return iter;
					}
				}
				if constexpr (negative) {
					return (iter = finishParse(value, iter), value *= -1, iter);
				} else {
					return finishParse(value, iter);
				}
			}

			if JSONIFIER_LIKELY (isDigit(numTmp)) {
				if constexpr (negative) {
					value = value * 10 - static_cast<value_type>(numTmp - zero);
				} else {
					value = value * 10 + static_cast<value_type>(numTmp - zero);
				}
				++iter;
				numTmp = *iter;
			}
			JSONIFIER_ELSE_UNLIKELY(else) {
				if JSONIFIER_LIKELY (!expFracTable[numTmp]) {
					return iter;
				}
			}
			return nullptr;
		}

		JSONIFIER_ALWAYS_INLINE static bool parseInt(value_type& value, const char*& iter) noexcept {
			if (*iter == minus) {
				++iter;
				const uint8_t* resultPtr{ parseInteger<true>(value, reinterpret_cast<const uint8_t*>(iter)) };
				if JSONIFIER_LIKELY (resultPtr) {
					iter += resultPtr - reinterpret_cast<const uint8_t*>(iter);
					return true;
				} else {
					value = 0;
					return false;
				}
			} else {
				const uint8_t* resultPtr{ parseInteger<false>(value, reinterpret_cast<const uint8_t*>(iter)) };
				if JSONIFIER_LIKELY (resultPtr) {
					iter += resultPtr - reinterpret_cast<const uint8_t*>(iter);
					return true;
				} else {
					value = 0;
					return false;
				}
			}
		}
	};

	template<jsonifier::concepts::unsigned_t value_type> struct integer_parser<value_type> {
		constexpr integer_parser() noexcept = default;

		JSONIFIER_ALWAYS_INLINE static value_type umul128Generic(value_type ab, value_type cd, value_type& hi) noexcept {
			value_type aHigh = ab >> 32;
			value_type aLow	 = ab & 0xFFFFFFFF;
			value_type bHigh = cd >> 32;
			value_type bLow	 = cd & 0xFFFFFFFF;
			value_type ad	 = aHigh * bLow;
			value_type bd	 = aHigh * bLow;
			value_type adbc	 = ad + aLow * bHigh;
			value_type lo	 = bd + (adbc << 32);
			value_type carry = (lo < bd);
			hi				 = aHigh * bHigh + (adbc >> 32) + carry;
			return lo;
		}

		JSONIFIER_ALWAYS_INLINE static bool multiply(value_type& value, value_type expValue) noexcept {
#if defined(__SIZEOF_INT128__)
			const __uint128_t res = static_cast<__uint128_t>(value) * static_cast<__uint128_t>(expValue);
			value				  = static_cast<value_type>(res);
			return res <= std::numeric_limits<value_type>::max();
#elif defined(_M_ARM64) && !defined(__MINGW32__)
			JSONIFIER_ALIGN value_type values;
			values = __umulh(value, expValue);
			value  = value * expValue;
			return values == 0;
#elif (defined(_WIN64) && !defined(__clang__))
			JSONIFIER_ALIGN value_type values;
			value = _umul128(value, expValue, &values);
			return values == 0;
#else
			JSONIFIER_ALIGN value_type values;
			value = umul128Generic(value, expValue, &values);
			return values == 0;
#endif
		}

		JSONIFIER_ALWAYS_INLINE static bool divide(value_type& value, value_type expValue) noexcept {
#if defined(__SIZEOF_INT128__)
			const __uint128_t dividend = static_cast<__uint128_t>(value);
			value					   = static_cast<value_type>(dividend / static_cast<__uint128_t>(expValue));
			return (dividend % static_cast<__uint128_t>(expValue)) == 0;
#elif (defined(_WIN64) && !defined(__clang__))
			JSONIFIER_ALIGN value_type values;
			value = _udiv128(0, value, expValue, &values);
			return values == 0;
#else
			JSONIFIER_ALIGN value_type values;
			values = value % expValue;
			value  = value / expValue;
			return values == 0;
#endif
		}

		JSONIFIER_ALWAYS_INLINE static const uint8_t* parseFraction(value_type& value, const uint8_t* iter) noexcept {
			if JSONIFIER_LIKELY (isDigit(*iter)) {
				value_type fracValue{ static_cast<value_type>(*iter - zero) };
				typename get_int_type<value_type>::type fracDigits{ 1 };
				++iter;
				while (isDigit(*iter)) {
					fracValue = fracValue * 10 + static_cast<value_type>(*iter - zero);
					++iter;
					++fracDigits;
				}
				if (expTable[*iter]) {
					++iter;
					int8_t expSign = 1;
					if (*iter == minus) {
						expSign = -1;
						++iter;
					} else if (*iter == plus) {
						++iter;
					}
					return parseExponentPostFrac(value, iter, expSign, fracValue, fracDigits);
				}
			}
			if JSONIFIER_LIKELY (!expFracTable[*iter]) {
				return iter;
			} else {
				return nullptr;
			}
		}

		JSONIFIER_ALWAYS_INLINE static const uint8_t* parseExponentPostFrac(value_type& value, const uint8_t* iter, int8_t expSign, value_type fracValue,
			typename get_int_type<value_type>::type fracDigits) noexcept {
			if JSONIFIER_LIKELY (isDigit(*iter)) {
				int64_t expValue{ *iter - zero };
				++iter;
				while (isDigit(*iter)) {
					expValue = expValue * 10 + *iter - zero;
					++iter;
				}
				if JSONIFIER_LIKELY (expValue <= 19) {
					const value_type powerExp = powerOfTenUint[expValue];

					constexpr value_type doubleMax = std::numeric_limits<value_type>::max();
					constexpr value_type doubleMin = std::numeric_limits<value_type>::min();

					if (fracDigits + expValue >= 0) {
						expValue *= expSign;
						const auto fractionalCorrection =
							expValue > fracDigits ? fracValue * powerOfTenUint[expValue - fracDigits] : fracValue / powerOfTenUint[fracDigits - expValue];
						return (expSign > 0) ? ((value <= (doubleMax / powerExp)) ? (multiply(value, powerExp), value += fractionalCorrection, iter) : nullptr)
											 : ((value / powerExp >= (doubleMin)) ? (divide(value, powerExp), value += fractionalCorrection, iter) : nullptr);
					} else {
						return (expSign > 0) ? ((value <= (doubleMax / powerExp)) ? (multiply(value, powerExp), iter) : nullptr)
											 : ((value / powerExp >= (doubleMin)) ? (divide(value, powerExp), iter) : nullptr);
					}
				}
				JSONIFIER_ELSE_UNLIKELY(else) {
					return nullptr;
				}
			}
			JSONIFIER_ELSE_UNLIKELY(else) {
				return nullptr;
			}
		}

		JSONIFIER_ALWAYS_INLINE static const uint8_t* parseExponent(value_type& value, const uint8_t* iter, int8_t expSign) noexcept {
			if JSONIFIER_LIKELY (isDigit(*iter)) {
				value_type expValue{ static_cast<value_type>(*iter - zero) };
				++iter;
				while (isDigit(*iter)) {
					expValue = expValue * 10 + static_cast<value_type>(*iter - zero);
					++iter;
				}
				if JSONIFIER_LIKELY (expValue <= 19) {
					const value_type powerExp	   = powerOfTenUint[expValue];
					constexpr value_type doubleMax = std::numeric_limits<value_type>::max();
					constexpr value_type doubleMin = std::numeric_limits<value_type>::min();
					expValue *= expSign;
					return (expSign > 0) ? ((value <= (doubleMax / powerExp)) ? (multiply(value, powerExp), iter) : nullptr)
										 : ((value / powerExp >= (doubleMin)) ? (divide(value, powerExp), iter) : nullptr);
				}
				JSONIFIER_ELSE_UNLIKELY(else) {
					return nullptr;
				}
			}
			JSONIFIER_ELSE_UNLIKELY(else) {
				return nullptr;
			}
		}

		JSONIFIER_INLINE static const uint8_t* finishParse(value_type& value, const uint8_t* iter) noexcept {
			if JSONIFIER_UNLIKELY (*iter == decimal) {
				++iter;
				return parseFraction(value, iter);
			} else if (expTable[*iter]) {
				++iter;
				int8_t expSign = 1;
				if (*iter == minus) {
					expSign = -1;
					++iter;
				} else if (*iter == plus) {
					++iter;
				}
				return parseExponent(value, iter, expSign);
			}
			if JSONIFIER_LIKELY (!expFracTable[*iter]) {
				return iter;
			} else {
				return nullptr;
			}
		}

		JSONIFIER_ALWAYS_INLINE static const uint8_t* parseInteger(value_type& value, const uint8_t* iter) noexcept {
			uint8_t numTmp{ *iter };
			if JSONIFIER_LIKELY (isDigit(numTmp)) {
				value = static_cast<value_type>(numTmp - zero);
				++iter;
				numTmp = *iter;
			} else [[unlikely]] {
				return nullptr;
			}

			if JSONIFIER_LIKELY (isDigit(numTmp)) {
				value = value * 10 + static_cast<value_type>(numTmp - zero);
				++iter;
				numTmp = *iter;
			}
			JSONIFIER_ELSE_UNLIKELY(else) {
				if JSONIFIER_LIKELY (!expFracTable[numTmp]) {
					return iter;
				}
				return finishParse(value, iter);
			}

			if JSONIFIER_UNLIKELY (iter[-2] == zero) {
				return nullptr;
			}

			if JSONIFIER_LIKELY (isDigit(numTmp)) {
				value = value * 10 + static_cast<value_type>(numTmp - zero);
				++iter;
				numTmp = *iter;
			}
			JSONIFIER_ELSE_UNLIKELY(else) {
				if JSONIFIER_LIKELY (!expFracTable[numTmp]) {
					return iter;
				}
				return finishParse(value, iter);
			}

			if JSONIFIER_LIKELY (isDigit(numTmp)) {
				value = value * 10 + static_cast<value_type>(numTmp - zero);
				++iter;
				numTmp = *iter;
			}
			JSONIFIER_ELSE_UNLIKELY(else) {
				if JSONIFIER_LIKELY (!expFracTable[numTmp]) {
					return iter;
				}
				return finishParse(value, iter);
			}

			if JSONIFIER_LIKELY (isDigit(numTmp)) {
				value = value * 10 + static_cast<value_type>(numTmp - zero);
				++iter;
				numTmp = *iter;
			}
			JSONIFIER_ELSE_UNLIKELY(else) {
				if JSONIFIER_LIKELY (!expFracTable[numTmp]) {
					return iter;
				}
				return finishParse(value, iter);
			}

			if JSONIFIER_LIKELY (isDigit(numTmp)) {
				value = value * 10 + static_cast<value_type>(numTmp - zero);
				++iter;
				numTmp = *iter;
			}
			JSONIFIER_ELSE_UNLIKELY(else) {
				if JSONIFIER_LIKELY (!expFracTable[numTmp]) {
					return iter;
				}
				return finishParse(value, iter);
			}

			if JSONIFIER_LIKELY (isDigit(numTmp)) {
				value = value * 10 + static_cast<value_type>(numTmp - zero);
				++iter;
				numTmp = *iter;
			}
			JSONIFIER_ELSE_UNLIKELY(else) {
				if JSONIFIER_LIKELY (!expFracTable[numTmp]) {
					return iter;
				}
				return finishParse(value, iter);
			}

			if JSONIFIER_LIKELY (isDigit(numTmp)) {
				value = value * 10 + static_cast<value_type>(numTmp - zero);
				++iter;
				numTmp = *iter;
			}
			JSONIFIER_ELSE_UNLIKELY(else) {
				if JSONIFIER_LIKELY (!expFracTable[numTmp]) {
					return iter;
				}
				return finishParse(value, iter);
			}

			if JSONIFIER_LIKELY (isDigit(numTmp)) {
				value = value * 10 + static_cast<value_type>(numTmp - zero);
				++iter;
				numTmp = *iter;
			}
			JSONIFIER_ELSE_UNLIKELY(else) {
				if JSONIFIER_LIKELY (!expFracTable[numTmp]) {
					return iter;
				}
				return finishParse(value, iter);
			}

			if JSONIFIER_LIKELY (isDigit(numTmp)) {
				value = value * 10 + static_cast<value_type>(numTmp - zero);
				++iter;
				numTmp = *iter;
			}
			JSONIFIER_ELSE_UNLIKELY(else) {
				if JSONIFIER_LIKELY (!expFracTable[numTmp]) {
					return iter;
				}
				return finishParse(value, iter);
			}

			if JSONIFIER_LIKELY (isDigit(numTmp)) {
				value = value * 10 + static_cast<value_type>(numTmp - zero);
				++iter;
				numTmp = *iter;
			}
			JSONIFIER_ELSE_UNLIKELY(else) {
				if JSONIFIER_LIKELY (!expFracTable[numTmp]) {
					return iter;
				}
				return finishParse(value, iter);
			}

			if JSONIFIER_LIKELY (isDigit(numTmp)) {
				value = value * 10 + static_cast<value_type>(numTmp - zero);
				++iter;
				numTmp = *iter;
			}
			JSONIFIER_ELSE_UNLIKELY(else) {
				if JSONIFIER_LIKELY (!expFracTable[numTmp]) {
					return iter;
				}
				return finishParse(value, iter);
			}

			if JSONIFIER_LIKELY (isDigit(numTmp)) {
				value = value * 10 + static_cast<value_type>(numTmp - zero);
				++iter;
				numTmp = *iter;
			}
			JSONIFIER_ELSE_UNLIKELY(else) {
				if JSONIFIER_LIKELY (!expFracTable[numTmp]) {
					return iter;
				}
				return finishParse(value, iter);
			}

			if JSONIFIER_LIKELY (isDigit(numTmp)) {
				value = value * 10 + static_cast<value_type>(numTmp - zero);
				++iter;
				numTmp = *iter;
			}
			JSONIFIER_ELSE_UNLIKELY(else) {
				if JSONIFIER_LIKELY (!expFracTable[numTmp]) {
					return iter;
				}
				return finishParse(value, iter);
			}

			if JSONIFIER_LIKELY (isDigit(numTmp)) {
				value = value * 10 + static_cast<value_type>(numTmp - zero);
				++iter;
				numTmp = *iter;
			}
			JSONIFIER_ELSE_UNLIKELY(else) {
				if JSONIFIER_LIKELY (!expFracTable[numTmp]) {
					return iter;
				}
				return finishParse(value, iter);
			}

			if JSONIFIER_LIKELY (isDigit(numTmp)) {
				value = value * 10 + static_cast<value_type>(numTmp - zero);
				++iter;
				numTmp = *iter;
			}
			JSONIFIER_ELSE_UNLIKELY(else) {
				if JSONIFIER_LIKELY (!expFracTable[numTmp]) {
					return iter;
				}
				return finishParse(value, iter);
			}

			if JSONIFIER_LIKELY (isDigit(numTmp)) {
				value = value * 10 + static_cast<value_type>(numTmp - zero);
				++iter;
				numTmp = *iter;
			}
			JSONIFIER_ELSE_UNLIKELY(else) {
				if JSONIFIER_LIKELY (!expFracTable[numTmp]) {
					return iter;
				}
				return finishParse(value, iter);
			}

			if JSONIFIER_LIKELY (isDigit(numTmp)) {
				value = value * 10 + static_cast<value_type>(numTmp - zero);
				++iter;
				numTmp = *iter;
			}
			JSONIFIER_ELSE_UNLIKELY(else) {
				if JSONIFIER_LIKELY (!expFracTable[numTmp]) {
					return iter;
				}
				return finishParse(value, iter);
			}

			if JSONIFIER_LIKELY (isDigit(numTmp)) {
				value = value * 10 + static_cast<value_type>(numTmp - zero);
				++iter;
				numTmp = *iter;
			}
			JSONIFIER_ELSE_UNLIKELY(else) {
				if JSONIFIER_LIKELY (!expFracTable[numTmp]) {
					return iter;
				}
				return finishParse(value, iter);
			}

			if JSONIFIER_LIKELY (isDigit(numTmp)) {
				if (value > rawCompValsPos<value_type>[numTmp]) {
					return nullptr;
				}
				value = value * 10 + static_cast<value_type>(numTmp - zero);
				++iter;
				numTmp = *iter;
			}
			JSONIFIER_ELSE_UNLIKELY(else) {
				if JSONIFIER_LIKELY (!expFracTable[numTmp]) {
					return iter;
				}
				return finishParse(value, iter);
			}

			if JSONIFIER_LIKELY (isDigit(numTmp)) {
				value = value * 10 + static_cast<value_type>(numTmp - zero);
				++iter;
				numTmp = *iter;
			}
			JSONIFIER_ELSE_UNLIKELY(else) {
				if JSONIFIER_LIKELY (!expFracTable[numTmp]) {
					return iter;
				}
			}
			return nullptr;
		}

		JSONIFIER_ALWAYS_INLINE static bool parseInt(value_type& value, const char*& iter) noexcept {
			auto resultPtr = parseInteger(value, reinterpret_cast<const uint8_t*>(iter));
			if JSONIFIER_LIKELY (resultPtr) {
				iter += resultPtr - reinterpret_cast<const uint8_t*>(iter);
				return true;
			} else {
				value = 0;
				return false;
			}
		}
	};
}
