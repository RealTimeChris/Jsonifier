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

	template<typename value_type, typename char_type> struct integer_parser {
		JSONIFIER_ALWAYS_INLINE constexpr integer_parser() noexcept = default;

		JSONIFIER_ALWAYS_INLINE const uint8_t* parseFraction(value_type& value, const uint8_t* iter) const noexcept {
			uint8_t numTmp{ *iter };
			if JSONIFIER_LIKELY ((digiTable[numTmp])) {
				int64_t fracValue = (numTmp - zero);
				++iter;
				int8_t fracDigits{ 1 };
				auto iterNew = iter;
				fracValue	 = parseChars(fracValue, iter);
				fracDigits += static_cast<int8_t>(iter - iterNew);
				return (expTable[*iter]) ? (++iter, parseExponentPostFrac(value, iter, fracDigits, fracValue)) : iter;
			}
			JSONIFIER_UNLIKELY(else) {
				return nullptr;
			}
		}

		JSONIFIER_ALWAYS_INLINE const uint8_t* finishExponentPostFrac(value_type& value, const uint8_t* iter, const int8_t& expSign, int64_t& expValue, const int8_t& fracDigits,
			const int64_t& fracValue) const noexcept {
			if JSONIFIER_LIKELY ((expValue <= 19)) {
				const value_type powerExp = powerOfTenInt<value_type>[expValue];

				constexpr value_type doubleMax = std::numeric_limits<value_type>::max();
				constexpr value_type doubleMin = std::numeric_limits<value_type>::min();

				if (fracDigits + expValue >= 0) {
					expValue *= expSign;
					const auto fractionalCorrection =
						expValue > fracDigits ? fracValue * powerOfTenInt<value_type>[expValue - fracDigits] : fracValue / powerOfTenInt<value_type>[fracDigits - expValue];
					return (expSign > 0) ? ((value <= (doubleMax / powerExp)) ? (multiply(value, powerExp), value += fractionalCorrection, iter) : nullptr)
										 : ((value >= (doubleMin * powerExp)) ? (divide(value, powerExp), value += fractionalCorrection, iter) : nullptr);
				} else {
					return (expSign > 0) ? ((value <= (doubleMax / powerExp)) ? (multiply(value, powerExp), iter) : nullptr)
										 : ((value >= (doubleMin * powerExp)) ? (divide(value, powerExp), iter) : nullptr);
				}
				return iter;
			}
			JSONIFIER_UNLIKELY(else) {
				return nullptr;
			}
		}

		JSONIFIER_ALWAYS_INLINE const uint8_t* parseExponentPostFrac(value_type& value, const uint8_t* iter, int8_t fracDigits = 0, int64_t fracValue = 0) const noexcept {
			uint8_t numTmp{ *iter };
			int8_t expSign = 1;
			if (numTmp == minus) {
				expSign = -1;
				++iter;
				numTmp = *iter;

			} else if (numTmp == plus) {
				++iter;
				numTmp = *iter;
			}

			if JSONIFIER_LIKELY ((digiTable[numTmp])) {
				int64_t expValue = static_cast<int64_t>((numTmp - zero));
				++iter;
				expValue = parseChars(expValue, iter);
				return finishExponentPostFrac(value, iter, expSign, expValue, fracDigits, fracValue);
			}
			JSONIFIER_UNLIKELY(else) {
				return nullptr;
			}
		}

		JSONIFIER_ALWAYS_INLINE const uint8_t* finishExponent(value_type& value, const uint8_t* iter, const int8_t& expSign, int64_t& expValue) const noexcept {
			if JSONIFIER_LIKELY ((expValue <= 19)) {
				const value_type powerExp = powerOfTenInt<value_type>[expValue];

				constexpr value_type doubleMax = std::numeric_limits<value_type>::max();
				constexpr value_type doubleMin = std::numeric_limits<value_type>::min();
				expValue *= expSign;
				return (expSign > 0) ? ((value <= (doubleMax / powerExp)) ? (multiply(value, powerExp), iter) : nullptr)
									 : ((value >= (doubleMin * powerExp)) ? (divide(value, powerExp), iter) : nullptr);
				return iter;
			}
			JSONIFIER_UNLIKELY(else) {
				return nullptr;
			}
		}

		JSONIFIER_ALWAYS_INLINE const uint8_t* parseExponent(value_type& value, const uint8_t* iter) const noexcept {
			uint8_t numTmp{ *iter };
			int8_t expSign = 1;
			if (numTmp == minus) {
				expSign = -1;
				++iter;
				numTmp = *iter;

			} else if (numTmp == plus) {
				++iter;
				numTmp = *iter;
			}

			if JSONIFIER_LIKELY ((digiTable[numTmp])) {
				int64_t expValue = static_cast<int64_t>((numTmp - zero));
				++iter;
				expValue = parseChars(expValue, iter);
				return finishExponent(value, iter, expSign, expValue);
			}
			JSONIFIER_UNLIKELY(else) {
				return nullptr;
			}
		}

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
		};

		template<typename value_type02> JSONIFIER_ALWAYS_INLINE value_type02 parseChars(value_type02 value, const uint8_t*& iter) const noexcept {
			uint8_t numTmp{ *iter };
			if JSONIFIER_LIKELY ((!digiTable[numTmp])) {
				return value;
			}
			JSONIFIER_UNLIKELY(else) {
				value = value * 10 + (numTmp - zero);
				++iter;
				numTmp = *iter;
				if JSONIFIER_LIKELY ((!digiTable[numTmp])) {
					return value;
				}
				JSONIFIER_UNLIKELY(else) {
					value = value * 10 + (numTmp - zero);
					++iter;
					numTmp = *iter;
					if JSONIFIER_LIKELY ((!digiTable[numTmp])) {
						return value;
					}
					JSONIFIER_UNLIKELY(else) {
						value = value * 10 + (numTmp - zero);
						++iter;
						numTmp = *iter;
						if JSONIFIER_LIKELY ((!digiTable[numTmp])) {
							return value;
						}
						JSONIFIER_UNLIKELY(else) {
							value = value * 10 + (numTmp - zero);
							++iter;
							numTmp = *iter;
							if JSONIFIER_LIKELY ((!digiTable[numTmp])) {
								return value;
							}
							JSONIFIER_UNLIKELY(else) {
								value = value * 10 + (numTmp - zero);
								++iter;
								numTmp = *iter;
								if JSONIFIER_LIKELY ((!digiTable[numTmp])) {
									return value;
								}
								JSONIFIER_UNLIKELY(else) {
									value = value * 10 + (numTmp - zero);
									++iter;
									numTmp = *iter;
									if JSONIFIER_LIKELY ((!digiTable[numTmp])) {
										return value;
									}
									JSONIFIER_UNLIKELY(else) {
										value = value * 10 + (numTmp - zero);
										++iter;
										numTmp = *iter;
										if JSONIFIER_LIKELY ((!digiTable[numTmp])) {
											return value;
										}
										JSONIFIER_UNLIKELY(else) {
											value = value * 10 + (numTmp - zero);
											++iter;
											numTmp = *iter;
											if JSONIFIER_LIKELY ((!digiTable[numTmp])) {
												return value;
											}
											JSONIFIER_UNLIKELY(else) {
												value = value * 10 + (numTmp - zero);
												++iter;
												numTmp = *iter;
												if JSONIFIER_LIKELY ((!digiTable[numTmp])) {
													return value;
												}
												JSONIFIER_UNLIKELY(else) {
													value = value * 10 + (numTmp - zero);
													++iter;
													numTmp = *iter;
													if JSONIFIER_LIKELY ((!digiTable[numTmp])) {
														return value;
													}
													JSONIFIER_UNLIKELY(else) {
														value = value * 10 + (numTmp - zero);
														++iter;
														numTmp = *iter;
														if JSONIFIER_LIKELY ((!digiTable[numTmp])) {
															return value;
														}
														JSONIFIER_UNLIKELY(else) {
															value = value * 10 + (numTmp - zero);
															++iter;
															numTmp = *iter;
															if JSONIFIER_LIKELY ((!digiTable[numTmp])) {
																return value;
															}
															JSONIFIER_UNLIKELY(else) {
																value = value * 10 + (numTmp - zero);
																++iter;
																numTmp = *iter;
																if JSONIFIER_LIKELY ((!digiTable[numTmp])) {
																	return value;
																}
																JSONIFIER_UNLIKELY(else) {
																	value = value * 10 + (numTmp - zero);
																	++iter;
																	numTmp = *iter;
																	if JSONIFIER_LIKELY ((!digiTable[numTmp])) {
																		return value;
																	}
																	JSONIFIER_UNLIKELY(else) {
																		value = value * 10 + (numTmp - zero);
																		++iter;
																		numTmp = *iter;
																		if JSONIFIER_LIKELY ((!digiTable[numTmp])) {
																			return value;
																		}
																		JSONIFIER_UNLIKELY(else) {
																			value = value * 10 + (numTmp - zero);
																			++iter;
																			numTmp = *iter;
																			if constexpr (jsonifier::concepts::unsigned_type<value_type>) {
																				if JSONIFIER_LIKELY ((!digiTable[numTmp])) {
																					return value;
																				}
																				JSONIFIER_UNLIKELY(else) {
																					value = value * 10 + (numTmp - zero);
																					++iter;
																					numTmp = *iter;
																				}
																			}
																			return value;
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

		template<bool negative = false> JSONIFIER_ALWAYS_INLINE const uint8_t* parseInteger(value_type& value, const uint8_t* iter) const noexcept {
			uint8_t numTmp{ *iter };
			if JSONIFIER_LIKELY ((digiTable[numTmp])) {
				value = (numTmp - zero);
				++iter;
				numTmp = *iter;
				if (digiTable[numTmp]) {
					value = value * 10 + (numTmp - zero);
					++iter;
					if JSONIFIER_LIKELY ((iter[-2] != zero)) {
						value  = parseChars(value, iter);
						numTmp = *iter;
						if JSONIFIER_LIKELY ((!expFracDigitTable[numTmp])) {
							return iter;
						} else if (digiTable[numTmp]) {
							if constexpr (negative) {
								if JSONIFIER_UNLIKELY ((value > rawCompValsNeg<value_type>[numTmp])) {
									return nullptr;
								}
							} else {
								if JSONIFIER_UNLIKELY ((value > rawCompValsPos<value_type>[numTmp])) {
									return nullptr;
								}
							}
							value = value * 10 + (numTmp - zero);
							++iter;
							iter = !digiTable[*iter] ? iter : nullptr;
							return iter;
						} else if (expTable[numTmp]) {
							++iter;
							return parseExponent(value, iter);
						} else {
							++iter;
							return parseFraction(value, iter);
						}
					} else {
						return nullptr;
					}
				} else {
					if JSONIFIER_LIKELY ((!expFracTable[numTmp])) {
						return iter;
					} else {
						if (expTable[numTmp]) {
							++iter;
							return parseExponent(value, iter);
						} else {
							++iter;
							return parseFraction(value, iter);
						}
					}
				}
			}
			JSONIFIER_UNLIKELY(else) {
				return nullptr;
			}
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
					const uint8_t* resultPtr{ parseInteger(value, reinterpret_cast<const uint8_t*>(iter)) };
					if (resultPtr) {
						iter += resultPtr - reinterpret_cast<const uint8_t*>(iter);
						return true;
					} else {
						return false;
					}
				}

			} else {
				auto result = parseInteger(value, reinterpret_cast<const uint8_t*>(iter));
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