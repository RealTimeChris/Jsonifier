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

	template<typename value_type> JSONIFIER_ALWAYS_INLINE_VARIABLE std::array<value_type, 20> powerOfTenInt{ 1, 10, 100, 1000, 10000, 100000, 1000000, 10000000, 100000000,
		1000000000, 10000000000, 100000000000, 1000000000000, 10000000000000, 100000000000000, 1000000000000000, 10000000000000000, 100000000000000000, 1000000000000000000,
		static_cast<value_type>(10000000000000000000) };

	template<typename value_type> JSONIFIER_ALWAYS_INLINE_VARIABLE std::array<value_type, 256> rawCompValsPos{ [] {
		constexpr auto rawCompVal{ std::numeric_limits<value_type>::max() };
		std::array<value_type, 256> returnValues{};
		returnValues['9'] = (rawCompVal - 9) / 10;
		returnValues['8'] = (rawCompVal - 8) / 10;
		returnValues['7'] = (rawCompVal - 7) / 10;
		returnValues['6'] = (rawCompVal - 6) / 10;
		returnValues['5'] = (rawCompVal - 5) / 10;
		returnValues['4'] = (rawCompVal - 4) / 10;
		returnValues['3'] = (rawCompVal - 3) / 10;
		returnValues['2'] = (rawCompVal - 2) / 10;
		returnValues['1'] = (rawCompVal - 1) / 10;
		returnValues['0'] = (rawCompVal - 0) / 10;
		return returnValues;
	}() };

	template<typename value_type> JSONIFIER_ALWAYS_INLINE_VARIABLE std::array<value_type, 256> rawCompValsNeg{ [] {
		constexpr auto rawCompVal{ static_cast<size_t>(std::numeric_limits<value_type>::max()) + 1 };
		std::array<value_type, 256> returnValues{};
		returnValues['9'] = (rawCompVal - 9) / 10;
		returnValues['8'] = (rawCompVal - 8) / 10;
		returnValues['7'] = (rawCompVal - 7) / 10;
		returnValues['6'] = (rawCompVal - 6) / 10;
		returnValues['5'] = (rawCompVal - 5) / 10;
		returnValues['4'] = (rawCompVal - 4) / 10;
		returnValues['3'] = (rawCompVal - 3) / 10;
		returnValues['2'] = (rawCompVal - 2) / 10;
		returnValues['1'] = (rawCompVal - 1) / 10;
		returnValues['0'] = (rawCompVal - 0) / 10;
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

	JSONIFIER_ALWAYS_INLINE_VARIABLE std::array<bool, 256> expTable{ [] {
		std::array<bool, 256> returnValues{};
		returnValues['E'] = true;
		returnValues['e'] = true;
		return returnValues;
	}() };

	JSONIFIER_ALWAYS_INLINE_VARIABLE std::array<bool, 256> expFracDigitTable{ [] {
		std::array<bool, 256> returnValues{};
		returnValues['.'] = true;
		returnValues['E'] = true;
		returnValues['e'] = true;
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

	JSONIFIER_ALWAYS_INLINE_VARIABLE std::array<bool, 256> expFracTable{ [] {
		std::array<bool, 256> returnValues{};
		returnValues['.'] = true;
		returnValues['E'] = true;
		returnValues['e'] = true;
		return returnValues;
	}() };

	JSONIFIER_ALWAYS_INLINE_VARIABLE uint8_t decimal{ '.' };
	JSONIFIER_ALWAYS_INLINE_VARIABLE uint8_t minus{ '-' };
	JSONIFIER_ALWAYS_INLINE_VARIABLE uint8_t plus{ '+' };
	JSONIFIER_ALWAYS_INLINE_VARIABLE uint8_t zero{ '0' };
	JSONIFIER_ALWAYS_INLINE_VARIABLE uint8_t nine{ '9' };

#define isDigit(x) ((x <= nine) && (x >= zero))

	JSONIFIER_ALWAYS_INLINE uint16_t parse_2_chars(const uint8_t* string) noexcept {
		uint16_t value;
		std::memcpy(&value, string, sizeof(value));
		return ((value & 0x0f000f00) >> 8) + (value & 0x000f000f) * 10;
	}

	JSONIFIER_ALWAYS_INLINE uint32_t parse_3_chars(const uint8_t* string) noexcept {
		uint32_t value;
		std::memcpy(&value, string, 3);
		value = ((value & 0x00000f00) >> 8) + (value & 0x000f000f) * 10;
		return ((((value & 0x00ff0000) >> 16) + (value & 0x000000ff) * 100)) / 10;
	}

	JSONIFIER_ALWAYS_INLINE uint32_t parse_4_chars(const uint8_t* string) noexcept {
		uint32_t value;
		std::memcpy(&value, string, sizeof(value));
		value = ((value & 0x0f000f00) >> 8) + (value & 0x000f000f) * 10;
		return ((value >> 16) & 0xff) + (value & 0xff) * 100;
	}

	JSONIFIER_ALWAYS_INLINE uint64_t parse_5_chars(const uint8_t* string) noexcept {
		uint64_t value{};
		std::memcpy(&value, string, 5);
		value = ((value & 0x0f000f000f000f00) >> 8) + (value & 0x000f000f000f000f) * 10;
		value = ((value & 0x00ff000000ff0000) >> 16) + (value & 0x000000ff000000ff) * 100;
		return (((((value & 0x0000ffff00000000) >> 32) + (value & 0x000000000000ffff) * 10000)) / 1000);
	}

	JSONIFIER_ALWAYS_INLINE uint64_t parse_6_chars(const uint8_t* string) noexcept {
		uint64_t value{};
		std::memcpy(&value, string, 6);
		value = ((value & 0x0f000f000f000f00) >> 8) + (value & 0x000f000f000f000f) * 10;
		value = ((value & 0x00ff000000ff0000) >> 16) + (value & 0x000000ff000000ff) * 100;
		return (((((value & 0x0000ffff00000000) >> 32) + (value & 0x000000000000ffff) * 10000)) / 100);
	}

	JSONIFIER_ALWAYS_INLINE uint64_t parse_7_chars(const uint8_t* string) noexcept {
		uint64_t value{};
		std::memcpy(&value, string, 7);
		value = ((value & 0x0f000f000f000f00) >> 8) + (value & 0x000f000f000f000f) * 10;
		value = ((value & 0x00ff000000ff0000) >> 16) + (value & 0x000000ff000000ff) * 100;
		return (((((value & 0x0000ffff00000000) >> 32) + (value & 0x000000000000ffff) * 10000)) / 10);
	}

	JSONIFIER_ALWAYS_INLINE std::uint64_t parse_8_chars(const uint8_t* string) noexcept {
		std::uint64_t value;
		std::memcpy(&value, string, sizeof(value));
		std::uint64_t lower_digits = (value & 0x0f000f000f000f00) >> 8;
		std::uint64_t upper_digits = (value & 0x000f000f000f000f) * 10;
		value					   = lower_digits + upper_digits;
		lower_digits			   = (value & 0x00ff000000ff0000) >> 16;
		upper_digits			   = (value & 0x000000ff000000ff) * 100;
		value					   = lower_digits + upper_digits;
		return ((value & 0x0000ffff00000000) >> 32) + ((value & 0x000000000000ffff) * 10000);
	}

	template<size_t length, typename value_type> struct parse_trick {
		JSONIFIER_ALWAYS_INLINE static void impl(value_type& value, const uint8_t* s) noexcept {
			if constexpr (length == 20) {
				value = (parse_8_chars(s) * 1000000000000ULL) + (parse_8_chars(s + 8) * 10000) + parse_3_chars(s + 16) * 10 + (s[19] - zero);
			} else if constexpr (length == 19) {
				value = (parse_8_chars(s) * 100000000000ULL) + (parse_8_chars(s + 8) * 1000) + (parse_2_chars(s + 16) * 10) + (s[18] - zero);
			} else if constexpr (length == 18) {
				value = (parse_8_chars(s) * 10000000000ULL) + (parse_8_chars(s + 8) * 100) + parse_2_chars(s + 16);
			} else if constexpr (length == 17) {
				value = (parse_8_chars(s) * 1000000000ULL) + (parse_8_chars(s + 8) * 10) + (s[16] - zero);
			} else if constexpr (length == 16) {
				value = (parse_8_chars(s) * 100000000ULL) + parse_8_chars(s + 8);
			} else if constexpr (length == 15) {
				value = (parse_8_chars(s) * 10000000ULL) + (parse_4_chars(s + 8) * 1000) + (parse_2_chars(s + 12) * 10) + (s[14] - zero);
			} else if constexpr (length == 14) {
				value = (parse_8_chars(s) * 1000000ULL) + (parse_4_chars(s + 8) * 100) + parse_2_chars(s + 12);
			} else if constexpr (length == 13) {
				value = (parse_8_chars(s) * 100000ULL) + (parse_4_chars(s + 8) * 10) + (s[12] - zero);
			} else if constexpr (length == 12) {
				value = (parse_8_chars(s) * 10000ULL) + parse_4_chars(s + 8);
			} else if constexpr (length == 11) {
				value = (parse_8_chars(s) * 1000ULL) + (parse_2_chars(s + 8) * 10) + (s[10] - zero);
			} else if constexpr (length == 10) {
				value = (parse_8_chars(s) * 100ULL) + parse_2_chars(s + 8);
			} else if constexpr (length == 9) {
				value = (parse_8_chars(s) * 10ULL) + (s[8] - zero);
			} else if constexpr (length == 8) {
				value = parse_8_chars(s);
			} else if constexpr (length == 7) {
				value = parse_7_chars(s);
			} else if constexpr (length == 6) {
				value = parse_6_chars(s);
			} else if constexpr (length == 5) {
				value = parse_5_chars(s);
			} else if constexpr (length == 4) {
				value = parse_4_chars(s);
			} else if constexpr (length == 3) {
				value = parse_3_chars(s);
			} else if constexpr (length == 2) {
				value = parse_2_chars(s);
			} else if constexpr (length == 1) {
				value = (s[0] - zero);
			}
		}
	};

	template<typename value_type, size_t... indices> static constexpr auto generateFunctionPtrsImpl(std::index_sequence<indices...>) {
		using function_type = decltype(&parse_trick<0, value_type>::impl);
		return std::array<function_type, sizeof...(indices)>{ &parse_trick<indices, value_type>::impl... };
	}

	template<typename value_type> static constexpr auto generateFunctionPtrs() {
		return generateFunctionPtrsImpl<value_type>(std::make_index_sequence<21>{});
	}

	template<typename value_type> static constexpr auto functionPtrs{ generateFunctionPtrs<value_type>() };

	template<typename value_type, typename char_type> struct integer_parser {
		JSONIFIER_ALWAYS_INLINE constexpr integer_parser() noexcept = default;

		JSONIFIER_ALWAYS_INLINE uint64_t umul128Generic(const uint64_t& ab, const uint64_t& cd, uint64_t& hi) const noexcept {
			uint64_t a_high = ab >> 32;
			uint64_t a_low	= ab & 0xFFFFFFFF;
			uint64_t b_high = cd >> 32;
			uint64_t b_low	= cd & 0xFFFFFFFF;
			uint64_t ad		= a_high * static_cast<uint64_t>(b_low);
			uint64_t bd		= a_high * static_cast<uint64_t>(b_low);
			uint64_t adbc	= ad + a_low * static_cast<uint64_t>(b_high);
			uint64_t lo		= bd + (adbc << 32);
			uint64_t carry	= (lo < bd);
			hi				= a_high * static_cast<uint64_t>(b_high) + (adbc >> 32) + carry;
			return lo;
		}

		JSONIFIER_ALWAYS_INLINE bool multiply(value_type& value, const int64_t& expValue) const noexcept {
			JSONIFIER_ALIGN uint64_t values;
#if defined(_M_ARM64) && !defined(__MINGW32__)
			values = __umulh(value, expValue);
			value  = value * expValue;
#elif defined(FASTFLOAT_32BIT) || (defined(_WIN64) && !defined(__clang__) && !defined(_M_ARM64))
			value = _umul128(value, expValue, &values);
#elif defined(FASTFLOAT_64BIT) && defined(__SIZEOF_INT128__)
			__uint128_t r = (( __uint128_t )value) * expValue;
			value		  = static_cast<uint64_t>(r);
			values		  = static_cast<uint64_t>(r >> 64);
#else
			value = umul128Generic(value, expValue, values);
#endif
			return values == 0;
		};

		JSONIFIER_ALWAYS_INLINE bool divide(value_type& value, const int64_t& expValue) const noexcept {
			JSONIFIER_ALIGN uint64_t values;
#if defined(FASTFLOAT_32BIT) || (defined(_WIN64) && !defined(__clang__))
			value = _udiv128(0, value, expValue, &values);
#elif defined(FASTFLOAT_64BIT) && defined(__SIZEOF_INT128__)
			__uint128_t dividend = __uint128_t(value);
			value				 = static_cast<uint64_t>(dividend / expValue);
			values				 = static_cast<uint64_t>(dividend % expValue);
#else
			values = value % expValue;
			value  = value / expValue;
#endif
			return values == 0;
		}

		template<typename value_type02> JSONIFIER_ALWAYS_INLINE value_type02 parseCharsLength(value_type02 digits, const uint8_t* iter) const noexcept {
			if JSONIFIER_LIKELY ((!isDigit(iter[digits]))) {
				return digits;
			}
			JSONIFIER_UNLIKELY(else) {
				++digits;
				if JSONIFIER_LIKELY ((!isDigit(iter[digits]))) {
					return digits;
				}
				JSONIFIER_UNLIKELY(else) {
					++digits;

					if JSONIFIER_LIKELY ((!isDigit(iter[digits]))) {
						return digits;
					}
					JSONIFIER_UNLIKELY(else) {
						++digits;

						if JSONIFIER_LIKELY ((!isDigit(iter[digits]))) {
							return digits;
						}
						JSONIFIER_UNLIKELY(else) {
							++digits;

							if JSONIFIER_LIKELY ((!isDigit(iter[digits]))) {
								return digits;
							}
							JSONIFIER_UNLIKELY(else) {
								++digits;

								if JSONIFIER_LIKELY ((!isDigit(iter[digits]))) {
									return digits;
								}
								JSONIFIER_UNLIKELY(else) {
									++digits;

									if JSONIFIER_LIKELY ((!isDigit(iter[digits]))) {
										return digits;
									}
									JSONIFIER_UNLIKELY(else) {
										++digits;

										if JSONIFIER_LIKELY ((!isDigit(iter[digits]))) {
											return digits;
										}
										JSONIFIER_UNLIKELY(else) {
											++digits;

											if JSONIFIER_LIKELY ((!isDigit(iter[digits]))) {
												return digits;
											}
											JSONIFIER_UNLIKELY(else) {
												++digits;

												if JSONIFIER_LIKELY ((!isDigit(iter[digits]))) {
													return digits;
												}
												JSONIFIER_UNLIKELY(else) {
													++digits;

													if JSONIFIER_LIKELY ((!isDigit(iter[digits]))) {
														return digits;
													}
													JSONIFIER_UNLIKELY(else) {
														++digits;

														if JSONIFIER_LIKELY ((!isDigit(iter[digits]))) {
															return digits;
														}
														JSONIFIER_UNLIKELY(else) {
															++digits;

															if JSONIFIER_LIKELY ((!isDigit(iter[digits]))) {
																return digits;
															}
															JSONIFIER_UNLIKELY(else) {
																++digits;

																if JSONIFIER_LIKELY ((!isDigit(iter[digits]))) {
																	return digits;
																}
																JSONIFIER_UNLIKELY(else) {
																	++digits;

																	if JSONIFIER_LIKELY ((!isDigit(iter[digits]))) {
																		return digits;
																	}
																	JSONIFIER_UNLIKELY(else) {
																		++digits;

																		if JSONIFIER_LIKELY ((!isDigit(iter[digits]))) {
																			return digits;
																		}
																		JSONIFIER_UNLIKELY(else) {
																			++digits;

																			if constexpr (jsonifier::concepts::unsigned_type<value_type>) {
																				if JSONIFIER_LIKELY ((!isDigit(iter[digits]))) {
																					return digits;
																				}
																				JSONIFIER_UNLIKELY(else) {
																					++digits;
																				}
																			}
																			return digits;
																		}
																	}
																}
															}
														}
													}
												}
											}
										}
									}
								}
							}
						}
					}
				}
			}
		}

		JSONIFIER_ALWAYS_INLINE const uint8_t* finishFrac(value_type& value, int8_t intDigits, int8_t fracDigits, const uint8_t* iter) const noexcept {
			functionPtrs<value_type>[intDigits](value, iter);
			iter += intDigits + fracDigits + 1;
			return iter;
		}

		JSONIFIER_ALWAYS_INLINE const uint8_t* parseFraction(value_type& value, int8_t intDigits, const uint8_t* iter, const uint8_t* fracStart) const noexcept {
			if JSONIFIER_LIKELY ((isDigit(fracStart[0]))) {
				int8_t fracDigits{ 1 };
				fracDigits = parseCharsLength(fracDigits, fracStart);
				if (expTable[iter[intDigits + fracDigits]]) {
					int8_t expSign = 1;
					if (iter[intDigits] == minus) {
						expSign = -1;
						return parseExponentPostFrac(value, intDigits, iter, fracStart + 2, expSign, fracDigits, fracStart);
					} else if (iter[intDigits] == plus) {
						return parseExponentPostFrac(value, intDigits, iter, fracStart + 2, expSign, fracDigits, fracStart);
					} else {
						return parseExponentPostFrac(value, intDigits, iter, fracStart + 1, expSign, fracDigits, fracStart);
					}
					++iter;

				} else {
					finishFrac(value, intDigits, fracDigits, iter);
				}
			}
			JSONIFIER_UNLIKELY(else) {
				return nullptr;
			}
		}

		JSONIFIER_ALWAYS_INLINE const uint8_t* finishExponentPostFrac(value_type& value, const int8_t& intDigits, const uint8_t* intStart, const int8_t& expSign,
			const int8_t& expDigits, const uint8_t* expStart, const int8_t& fracDigits, const uint8_t* fracStart) const noexcept {
			functionPtrs<value_type>[intDigits](value, expStart);
			int64_t expValue;
			functionPtrs<int64_t>[expDigits](expValue, expStart);
			if JSONIFIER_LIKELY ((expValue <= 19)) {
				const value_type powerExp = powerOfTenInt<value_type>[expValue];

				constexpr value_type doubleMax = std::numeric_limits<value_type>::max();
				constexpr value_type doubleMin = std::numeric_limits<value_type>::min();

				if (fracDigits + expValue >= 0) {
					int64_t fracValue;
					functionPtrs<int64_t>[fracDigits](fracValue, fracStart);
					expValue *= expSign;
					const auto fractionalCorrection =
						expValue > fracDigits ? fracValue * powerOfTenInt<value_type>[expValue - fracDigits] : fracValue / powerOfTenInt<value_type>[fracDigits - expValue];
					return (expSign > 0)
						? ((value <= (doubleMax / powerExp)) ? (multiply(value, powerExp), value += fractionalCorrection, intStart + intDigits + fracDigits + expDigits) : nullptr)
						: ((value >= (doubleMin * powerExp)) ? (divide(value, powerExp), value += fractionalCorrection, intStart + intDigits + fracDigits + expDigits) : nullptr);
				} else {
					return (expSign > 0) ? ((value <= (doubleMax / powerExp)) ? (multiply(value, powerExp), intStart + intDigits + fracDigits + expDigits) : nullptr)
										 : ((value >= (doubleMin * powerExp)) ? (divide(value, powerExp), intStart + intDigits + fracDigits + expDigits) : nullptr);
				}
				return intStart;
			}
			JSONIFIER_UNLIKELY(else) {
				return nullptr;
			}
		}

		JSONIFIER_ALWAYS_INLINE const uint8_t* parseExponentPostFrac(value_type& value, const int8_t& intDigits, const uint8_t* iter, const uint8_t* expStart, int8_t expSign,
			const int8_t& fracDigits, const uint8_t* fracStart) const noexcept {
			if JSONIFIER_LIKELY ((isDigit(expStart[0]))) {
				int8_t expDigits{ 1 };
				expDigits = parseCharsLength(expDigits, expStart);
				return finishExponentPostFrac(value, intDigits, iter, expSign, expDigits, expStart, fracDigits, fracStart);
			}
			JSONIFIER_UNLIKELY(else) {
				return nullptr;
			}
		}

		template<int8_t sign = 0> JSONIFIER_ALWAYS_INLINE const uint8_t* finishExponent(value_type& value, const int8_t& intDigits, const uint8_t* iter, const int8_t& expSign,
			const int8_t& expDigits, const uint8_t* expStart) const noexcept {
			functionPtrs<value_type>[intDigits](value, iter);
			int64_t expValue;
			functionPtrs<int64_t>[expDigits](expValue, expStart);
			if JSONIFIER_LIKELY ((expValue <= 19)) {
				const value_type powerExp = powerOfTenInt<value_type>[expValue];

				constexpr value_type doubleMax = std::numeric_limits<value_type>::max();
				constexpr value_type doubleMin = std::numeric_limits<value_type>::min();
				expValue *= expSign;
				return (expSign > 0) ? ((value <= (doubleMax / powerExp)) ? (multiply(value, powerExp), iter + intDigits + expDigits + 1 + sign) : nullptr)
									 : ((value >= (doubleMin * powerExp)) ? (divide(value, powerExp), iter + intDigits + expDigits + 1 + sign) : nullptr);
				return iter;
			}
			JSONIFIER_UNLIKELY(else) {
				return nullptr;
			}
		}

		template<int8_t sign = 0> JSONIFIER_ALWAYS_INLINE const uint8_t* parseExponent(value_type& value, const int8_t& intDigits, const uint8_t* iter, const uint8_t* expStart,
			int8_t expSign) const noexcept {
			if JSONIFIER_LIKELY ((isDigit(expStart[0]))) {
				int8_t expDigits{ 1 };
				expDigits = parseCharsLength(expDigits, expStart);
				return finishExponent<sign>(value, intDigits, iter, expSign, expDigits, expStart);
			}
			JSONIFIER_UNLIKELY(else) {
				return nullptr;
			}
		}

		template<uint8_t Sign> JSONIFIER_ALWAYS_INLINE constexpr const uint8_t* parseInteger(value_type& value, const uint8_t* iter) const noexcept {
			int64_t intDigits{};
			if (digiTable[uint8_t(iter[intDigits])]) [[likely]] {
				++intDigits;
			} else [[unlikely]] {
				return nullptr;
			}

			if (digiTable[uint8_t(iter[intDigits])]) {
				++intDigits;
			} else {
				if (not expFracTable[uint8_t(iter[intDigits])]) {
					functionPtrs<value_type>[intDigits](value, iter);
					iter += intDigits;
					return iter;
				}
				goto finish;
			}

			if (digiTable[uint8_t(iter[intDigits])]) {
				++intDigits;
			} else {
				if (not expFracTable[uint8_t(iter[intDigits])]) {
					functionPtrs<value_type>[intDigits](value, iter);
					iter += intDigits;
					return iter;
				}
				goto finish;
			}

			if (*iter == zero) [[unlikely]] {
				return nullptr;
			}

			intDigits = parseCharsLength(intDigits, iter);

		finish:
			if (iter[intDigits] == 'e' || iter[intDigits] == 'E') {
				int8_t expSign = 1;
				if (iter[intDigits] == minus) {
					expSign = -1;
					return parseExponent<1>(value, intDigits, iter, iter + intDigits + 2, expSign);
				} else if (iter[intDigits] == plus) {
					return parseExponent<1>(value, intDigits, iter, iter + intDigits + 2, expSign);
				} else {
					return parseExponent(value, intDigits, iter, iter + intDigits + 1, expSign);
				}
			} else {
				if (iter[intDigits] == '.') [[unlikely]] {
					return parseFraction(value, intDigits, iter, iter + intDigits + 1);
				}
				functionPtrs<value_type>[intDigits](value, iter);
				iter += intDigits;
				return iter;
			}

			if (not digiTable[uint8_t(iter[intDigits])]) [[unlikely]] {
				return nullptr;
			}
			return nullptr;
		}

		JSONIFIER_ALWAYS_INLINE bool parseInt(value_type& value, char_type*& iter) const noexcept {
			if constexpr (jsonifier::concepts::signed_type<value_type>) {
				if (*iter == minus) {
					++iter;
					const uint8_t* resultPtr{ parseInteger<true>(value, reinterpret_cast<const uint8_t*>(iter)) };
					if (resultPtr) {
						iter += resultPtr - reinterpret_cast<const uint8_t*>(iter);
						value *= -1;
						return true;
					} else {
						return false;
					}
				} else {
					const uint8_t* resultPtr{ parseInteger<false>(value, reinterpret_cast<const uint8_t*>(iter)) };
					if (resultPtr) {
						iter += resultPtr - reinterpret_cast<const uint8_t*>(iter);
						return true;
					} else {
						return false;
					}
				}

			} else {
				auto result = parseInteger<false>(value, reinterpret_cast<const uint8_t*>(iter));
				if (result) {
					iter += result - reinterpret_cast<const uint8_t*>(iter);
					return true;
				} else {
					return false;
				}
			}
		}
	};
}