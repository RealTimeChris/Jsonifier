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

#define repeat1For18(x) \
	{ x(1) x(2) x(3) x(4) x(5) x(6) x(7) x(8) x(9) x(10) x(11) x(12) x(13) x(14) x(15) x(16) x(17) x(18) }

	JSONIFIER_ALWAYS_INLINE_VARIABLE std::array<size_t, 20> powerOfTenInt{ 1ull, 10ull, 100ull, 1000ull, 10000ull, 100000ull, 1000000ull, 10000000ull, 100000000ull, 1000000000ull,
		10000000000ull, 100000000000ull, 1000000000000ull, 10000000000000ull, 100000000000000ull, 1000000000000000ull, 10000000000000000ull, 100000000000000000ull,
		1000000000000000000ull, 10000000000000000000ull };

	template<typename value_type> JSONIFIER_ALWAYS_INLINE_VARIABLE std::array<value_type, 10> rawCompValsPos{ [] {
		constexpr auto rawCompVal{ std::numeric_limits<value_type>::max() };
		std::array<value_type, 10> returnValues{};
		returnValues[0] = (rawCompVal - 0) / 10;
		returnValues[1] = (rawCompVal - 1) / 10;
		returnValues[2] = (rawCompVal - 2) / 10;
		returnValues[3] = (rawCompVal - 3) / 10;
		returnValues[4] = (rawCompVal - 4) / 10;
		returnValues[5] = (rawCompVal - 5) / 10;
		returnValues[6] = (rawCompVal - 6) / 10;
		returnValues[7] = (rawCompVal - 7) / 10;
		returnValues[8] = (rawCompVal - 8) / 10;
		returnValues[9] = (rawCompVal - 9) / 10;
		return returnValues;
	}() };

	template<typename value_type> JSONIFIER_ALWAYS_INLINE_VARIABLE std::array<value_type, 10> rawCompValsNeg{ [] {
		constexpr auto rawCompVal{ size_t(std::numeric_limits<value_type>::max()) + 1 };
		std::array<value_type, 10> returnValues{};
		returnValues[0] = (rawCompVal - 0) / 10;
		returnValues[1] = (rawCompVal - 1) / 10;
		returnValues[2] = (rawCompVal - 2) / 10;
		returnValues[3] = (rawCompVal - 3) / 10;
		returnValues[4] = (rawCompVal - 4) / 10;
		returnValues[5] = (rawCompVal - 5) / 10;
		returnValues[6] = (rawCompVal - 6) / 10;
		returnValues[7] = (rawCompVal - 7) / 10;
		returnValues[8] = (rawCompVal - 8) / 10;
		returnValues[9] = (rawCompVal - 9) / 10;
		return returnValues;
	}() };

	JSONIFIER_ALWAYS_INLINE_VARIABLE std::array<bool, 256> digiExpDecTable{ []() noexcept {
		std::array<bool, 256> returnValues{};
		returnValues['0'] = true;
		returnValues['1'] = true;
		returnValues['2'] = true;
		returnValues['3'] = true;
		returnValues['4'] = true;
		returnValues['5'] = true;
		returnValues['6'] = true;
		returnValues['7'] = true;
		returnValues['8'] = true;
		returnValues['9'] = true;
		returnValues['.'] = true;
		returnValues['E'] = true;
		returnValues['e'] = true;
		return returnValues;
	}() };

	JSONIFIER_ALWAYS_INLINE_VARIABLE std::array<bool, 256> digiTable{ [] {
		std::array<bool, 256> returnValues{};
		returnValues['0'] = true;
		returnValues['1'] = true;
		returnValues['2'] = true;
		returnValues['3'] = true;
		returnValues['4'] = true;
		returnValues['5'] = true;
		returnValues['6'] = true;
		returnValues['7'] = true;
		returnValues['8'] = true;
		returnValues['9'] = true;
		return returnValues;
	}() };

	JSONIFIER_ALWAYS_INLINE_VARIABLE std::array<bool, 256> nonZeroDigiTable{ [] {
		std::array<bool, 256> returnValues{};
		returnValues['1'] = true;
		returnValues['2'] = true;
		returnValues['3'] = true;
		returnValues['4'] = true;
		returnValues['5'] = true;
		returnValues['6'] = true;
		returnValues['7'] = true;
		returnValues['8'] = true;
		returnValues['9'] = true;
		return returnValues;
	}() };

	JSONIFIER_ALWAYS_INLINE_VARIABLE std::array<bool, 256> expTable{ [] {
		std::array<bool, 256> returnValues{};
		returnValues['E'] = true;
		returnValues['e'] = true;
		return returnValues;
	}() };

	JSONIFIER_ALWAYS_INLINE_VARIABLE std::array<bool, 256> expFracTable{ [] {
		std::array<bool, 256> returnValues{};
		returnValues['.'] = true;
		returnValues['E'] = true;
		returnValues['e'] = true;
		return returnValues;
	}() };

	JSONIFIER_ALWAYS_INLINE_VARIABLE std::array<bool, 256> plusOrMinusTable{ [] {
		std::array<bool, 256> returnValues{};
		returnValues['+'] = true;
		returnValues['-'] = true;
		return returnValues;
	}() };

	JSONIFIER_ALWAYS_INLINE_VARIABLE char decimal{ '.' };
	JSONIFIER_ALWAYS_INLINE_VARIABLE char minus{ '-' };
	JSONIFIER_ALWAYS_INLINE_VARIABLE char plus{ '+' };
	JSONIFIER_ALWAYS_INLINE_VARIABLE char zero{ '0' };

	struct JSONIFIER_ALIGN value128 final {
		uint64_t high;
		uint64_t low;
	};

	JSONIFIER_ALWAYS_INLINE constexpr uint64_t emulu(uint32_t x, uint32_t y) noexcept {
		return x * ( uint64_t )y;
	}

	JSONIFIER_ALWAYS_INLINE constexpr uint64_t umul128Generic(uint64_t ab, uint64_t cd, uint64_t* hi) noexcept {
		uint64_t a_high = ab >> 32;
		uint64_t a_low	= ab & 0xFFFFFFFF;
		uint64_t b_high = cd >> 32;
		uint64_t b_low	= cd & 0xFFFFFFFF;
		uint64_t ad		= emulu(a_high, b_low);
		uint64_t bd		= emulu(a_low, b_low);
		uint64_t adbc	= ad + emulu(a_low, b_high);
		uint64_t lo		= bd + (adbc << 32);
		uint64_t carry	= (lo < bd);
		*hi				= emulu(a_high, b_high) + (adbc >> 32) + carry;
		return lo;
	}

	template<typename value_type, typename char_type> struct integer_parser {
		JSONIFIER_ALWAYS_INLINE bool parseFraction(value_type& value, char_type*& iter) noexcept {
			if JSONIFIER_LIKELY ((digiTable[static_cast<uint8_t>(*iter)])) {
				int64_t fracValue = static_cast<uint8_t>(*iter - '0');
				++iter;
				int8_t fracDigits{ static_cast<int8_t>(parseChars(fracValue, iter) + 1) };
				return (expTable[static_cast<uint8_t>(*iter)]) ? (++iter, parseExponent(value, iter, fracDigits, fracValue)) : true;
			}
			JSONIFIER_UNLIKELY(else) {
				return false;
			}
		}

		template<typename value_type02> JSONIFIER_ALWAYS_INLINE int64_t parseChars(value_type02& value, char_type*& iter) noexcept {
			uint8_t numTmp = static_cast<uint8_t>(*iter);
			if ((digiTable[static_cast<uint8_t>(numTmp)])) {
				value = static_cast<value_type>(numTmp - '0') + value * 10;
				++iter;
				numTmp = static_cast<uint8_t>(*iter);
			} else {
				return 0;
			}
			if ((digiTable[static_cast<uint8_t>(numTmp)])) {
				value = static_cast<value_type>(numTmp - '0') + value * 10;
				++iter;
				numTmp = static_cast<uint8_t>(*iter);
			} else {
				return 1;
			}
			if ((digiTable[static_cast<uint8_t>(numTmp)])) {
				value = static_cast<value_type>(numTmp - '0') + value * 10;
				++iter;
				numTmp = static_cast<uint8_t>(*iter);
			} else {
				return 2;
			}
			if ((digiTable[static_cast<uint8_t>(numTmp)])) {
				value = static_cast<value_type>(numTmp - '0') + value * 10;
				++iter;
				numTmp = static_cast<uint8_t>(*iter);
			} else {
				return 3;
			}
			if ((digiTable[static_cast<uint8_t>(numTmp)])) {
				value = static_cast<value_type>(numTmp - '0') + value * 10;
				++iter;
				numTmp = static_cast<uint8_t>(*iter);
			} else {
				return 4;
			}
			if ((digiTable[static_cast<uint8_t>(numTmp)])) {
				value = static_cast<value_type>(numTmp - '0') + value * 10;
				++iter;
				numTmp = static_cast<uint8_t>(*iter);
			} else {
				return 5;
			}
			if ((digiTable[static_cast<uint8_t>(numTmp)])) {
				value = static_cast<value_type>(numTmp - '0') + value * 10;
				++iter;
				numTmp = static_cast<uint8_t>(*iter);
			} else {
				return 6;
			}
			if ((digiTable[static_cast<uint8_t>(numTmp)])) {
				value = static_cast<value_type>(numTmp - '0') + value * 10;
				++iter;
				numTmp = static_cast<uint8_t>(*iter);
			} else {
				return 7;
			}
			if ((digiTable[static_cast<uint8_t>(numTmp)])) {
				value = static_cast<value_type>(numTmp - '0') + value * 10;
				++iter;
				numTmp = static_cast<uint8_t>(*iter);
			} else {
				return 8;
			}
			if ((digiTable[static_cast<uint8_t>(numTmp)])) {
				value = static_cast<value_type>(numTmp - '0') + value * 10;
				++iter;
				numTmp = static_cast<uint8_t>(*iter);
			} else {
				return 9;
			}
			if ((digiTable[static_cast<uint8_t>(numTmp)])) {
				value = static_cast<value_type>(numTmp - '0') + value * 10;
				++iter;
				numTmp = static_cast<uint8_t>(*iter);
			} else {
				return 10;
			}
			if ((digiTable[static_cast<uint8_t>(numTmp)])) {
				value = static_cast<value_type>(numTmp - '0') + value * 10;
				++iter;
				numTmp = static_cast<uint8_t>(*iter);
			} else {
				return 11;
			}
			if ((digiTable[static_cast<uint8_t>(numTmp)])) {
				value = static_cast<value_type>(numTmp - '0') + value * 10;
				++iter;
				numTmp = static_cast<uint8_t>(*iter);
			} else {
				return 12;
			}
			if ((digiTable[static_cast<uint8_t>(numTmp)])) {
				value = static_cast<value_type>(numTmp - '0') + value * 10;
				++iter;
				numTmp = static_cast<uint8_t>(*iter);
			} else {
				return 13;
			}
			if ((digiTable[static_cast<uint8_t>(numTmp)])) {
				value = static_cast<value_type>(numTmp - '0') + value * 10;
				++iter;
				numTmp = static_cast<uint8_t>(*iter);
			} else {
				return 14;
			}
			if ((digiTable[static_cast<uint8_t>(numTmp)])) {
				value = static_cast<value_type>(numTmp - '0') + value * 10;
				++iter;
				numTmp = static_cast<uint8_t>(*iter);
			} else {
				return 15;
			}
			if constexpr (jsonifier::concepts::unsigned_type<value_type>) {
				if ((digiTable[static_cast<uint8_t>(numTmp)])) {
					value = static_cast<value_type>(numTmp - '0') + value * 10;
					++iter;
					numTmp = static_cast<uint8_t>(*iter);
				} else {
					return 16;
				}
				return 17;
			}
			return 16;
		}

		JSONIFIER_ALWAYS_INLINE bool parseExponent(value_type& value, char_type*& iter, int8_t fracDigits = 0, int64_t fracValue = 0) noexcept {
			int8_t expSign = 1;
			if (*iter == minus) {
				expSign = -1;
				++iter;
			} else if (*iter == plus) {
				++iter;
			}

			if JSONIFIER_LIKELY ((digiTable[static_cast<uint8_t>(*iter)])) {
				int64_t expValue = static_cast<uint8_t>(*iter - '0');
				++iter;
				parseChars(expValue, iter);
				return parseFinish(value, iter, expSign, expValue, fracDigits, fracValue);
			}
			JSONIFIER_UNLIKELY(else) {
				return false;
			}
		}

		JSONIFIER_ALWAYS_INLINE bool multiply(value_type& value, const int64_t& expValue) noexcept {
			uint64_t values[2];
#if defined(__SIZEOF_INT128__)
			const __uint128_t res = __uint128_t(value) * expValue;
			value				  = value_type(res);
			return res <= (std::numeric_limits<value_type>::max)();
#elif defined(_M_ARM64) && !defined(__MINGW32__)
			values[1] = __umulh(value, expValue);
			values[0] = value * expValue;
#elif defined(FASTFLOAT_32BIT) || (defined(_WIN64) && !defined(__clang__))
			values[0] = _umul128(value, expValue, values + 1);
#elif defined(FASTFLOAT_64BIT) && defined(__SIZEOF_INT128__)
			values[0] = (( __uint128_t )value) * expValue;
			values[1] = uint64_t(r >> 64);
#else
			values[0] = umul128Generic(value, expValue, values + 1);
#endif
			value = value_type(values[0]);
			return values[1] == 0;
		};

		JSONIFIER_ALWAYS_INLINE bool divide(value_type& value, const int64_t& expValue) noexcept {
#if defined(__SIZEOF_INT128__)
			const __uint128_t res = __uint128_t(value) / expValue;
			value				  = value_type(res);
			return res <= (std::numeric_limits<value_type>::max)();
#else
			if (expValue == 0) {
				return true;
			}
			value = value_type(value / expValue);
			return value % expValue == 0;
#endif
		};

		JSONIFIER_ALWAYS_INLINE bool parseFinish(value_type& value, char_type*& iter, const int8_t& expSign, int64_t& expValue, const int8_t& fracDigits,
			const int64_t& fracValue) noexcept {
			if JSONIFIER_LIKELY ((expValue <= 19)) {
				const auto powerExp = powerOfTenInt[expValue];

				static constexpr value_type doubleMax = std::numeric_limits<value_type>::max();
				static constexpr value_type doubleMin = std::numeric_limits<value_type>::min();

				if (fracDigits + expValue >= 0) {
					expValue *= expSign;
					const auto fractionalCorrection = expValue > fracDigits ? fracValue * powerOfTenInt[expValue - fracDigits] : fracValue / powerOfTenInt[fracDigits - expValue];
					return (expSign > 0) ? ((value <= (doubleMax / powerExp)) ? (multiply(value, powerExp), value += fractionalCorrection, true) : false)
										 : ((value >= (doubleMin * powerExp)) ? (divide(value, powerExp), value += fractionalCorrection, true) : false);
				} else {
					return (expSign > 0) ? ((value <= (doubleMax / powerExp)) ? multiply(value, powerExp) : false)
										 : ((value >= (doubleMin * powerExp)) ? divide(value, powerExp) : false);
				}
				return true;
			}
			JSONIFIER_UNLIKELY(else) {
				return false;
			}
		}

		template<bool positive = true> JSONIFIER_ALWAYS_INLINE bool parseInteger(value_type& value, char_type*& iter) noexcept {
			static constexpr auto maxLoopIndex{ jsonifier::concepts::signed_type<value_type> ? 16ull : 17ull };
			if JSONIFIER_LIKELY ((digiTable[static_cast<uint8_t>(*iter)])) {
				value = static_cast<value_type>(*iter - '0');
				++iter;

				if JSONIFIER_LIKELY ((digiTable[static_cast<uint8_t>(*iter)])) {
					value = static_cast<value_type>(*iter - '0') + value * 10;
					++iter;
				}
				JSONIFIER_UNLIKELY(else) {
					return (!expFracTable[static_cast<uint8_t>(*iter)]) ? true : (*iter == decimal) ? (++iter, parseFraction(value, iter)) : (++iter, parseExponent(value, iter));
				}

				if JSONIFIER_LIKELY ((nonZeroDigiTable[static_cast<uint8_t>(*(iter - 2))])) {
					parseChars(value, iter);
					if JSONIFIER_UNLIKELY ((digiTable[static_cast<uint8_t>(*iter)])) {
						value_type numTmp = static_cast<value_type>(*iter - '0');
						if constexpr (positive) {
							if JSONIFIER_UNLIKELY ((value > rawCompValsPos<value_type>[numTmp])) [[unlikely]] {
								return false;
							}
						} else {
							if JSONIFIER_UNLIKELY ((value > rawCompValsNeg<value_type>[numTmp])) [[unlikely]] {
								return false;
							}
						}
						value = numTmp + value * 10;
						++iter;
						return !digiTable[static_cast<uint8_t>(*iter)];
					} else {
						return (!expFracTable[static_cast<uint8_t>(*iter)]) ? true
							: (*iter == decimal)							? (++iter, parseFraction(value, iter))
																			: (++iter, parseExponent(value, iter));
					}
				}
				JSONIFIER_UNLIKELY(else) {
					return false;
				}
			}
			JSONIFIER_UNLIKELY(else) {
				return false;
			}
		}

		JSONIFIER_ALWAYS_INLINE bool parseInt(value_type& value, char_type*& iter) noexcept {
			if constexpr (jsonifier::concepts::signed_type<value_type>) {
				static constexpr auto maxValue = size_t((std::numeric_limits<value_type>::max)());
				static constexpr auto minValue = size_t((std::numeric_limits<value_type>::max)()) + 1;
				bool result;
				return ((*iter == minus ? (++iter, result = integer_parser<value_type, char_type>::parseInteger<false>(value, iter), value *= -1, result)
										: integer_parser<value_type, char_type>::parseInteger(value, iter)));
			} else {
				return integer_parser<value_type, char_type>::parseInteger(value, iter);
			}
		}
	};
}