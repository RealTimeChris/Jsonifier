/*
	Jsonifier - For parsing and serializing Json - very rapidly.
	Copyright (C) 2023 Chris M. (RealTimeChris)

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	This library is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
	Lesser General Public License for more details.

	You should have received a copy of the GNU Lesser General Public
	License along with this library; if not, Write to the Free Software
	Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301
	USA
*/
/// https://github.com/RealTimeChris/Jsonifier
/// Feb 3, 2023
/// Most of the code in this header was copied from Simdjson - https://github.com/simdjson
#pragma once

#include <jsonifier/StringParsing.hpp>
#include <array>
#ifndef _WIN32
	#include <x86intrin.h>
#else
	#include <intrin.h>
#endif

namespace Jsonifier {

	const int32_t mantissaExplicitBits{ 52 };
	const int32_t decimalPointRange{ 2047 };
	const int32_t minimumExponent{ -1023 };
	const int32_t infinitePower{ 0x7FF };
	const int32_t smallestPower{ -342 };
	const int32_t largestPower{ 308 };
	const uint32_t maxDigits{ 768 };
	const int32_t signIndex{ 63 };

	struct Decimal {
		uint8_t digits[maxDigits]{};
		int32_t decimalPoint{};
		uint32_t numDigits{};
		bool truncated{};
		bool negative{};
	};

	struct AdjustedMantissa {
		uint64_t mantissa{};
		int32_t power2{};
	};

	struct Value128 {
		uint64_t high{};
		uint64_t low{};
	};

#ifndef _WIN32
	inline uint64_t __emulu(uint32_t x, uint32_t y) {
		return x * ( uint64_t )y;
	}

	inline uint64_t _umul128(uint64_t ab, uint64_t cd, uint64_t* hi) {
		uint64_t ad = __emulu(( uint32_t )(ab >> 32), ( uint32_t )cd);
		uint64_t bd = __emulu(( uint32_t )ab, ( uint32_t )cd);
		uint64_t adbc = ad + __emulu(( uint32_t )ab, ( uint32_t )(cd >> 32));
		uint64_t adbcCarry = !!(adbc < ad);
		uint64_t lo = bd + (adbc << 32);
		*hi = __emulu(( uint32_t )(ab >> 32), ( uint32_t )(cd >> 32)) + (adbc >> 32) + (adbcCarry << 32) + !!(lo < bd);
		return lo;
	}
#endif

	inline Value128 fullMultiplication(uint64_t value1, uint64_t value2) {
		Value128 answer{};
		answer.low = _umul128(value1, value2, &answer.high);
		return answer;
	}

	inline uint64_t round(Decimal& h) {
		if ((h.numDigits == 0) || (h.decimalPoint < 0)) {
			return 0;
		} else if (h.decimalPoint > 18) {
			return std::numeric_limits<uint64_t>::max();
		}
		uint32_t dp = uint32_t(h.decimalPoint);
		uint64_t size = 0;
		for (uint32_t i = 0; i < dp; i++) {
			size = (10 * size) + ((i < h.numDigits) ? h.digits[i] : 0);
		}
		bool roundUp = false;
		if (dp < h.numDigits) {
			roundUp = h.digits[dp] >= 5;
			if ((h.digits[dp] == 5) && (dp + 1 == h.numDigits)) {
				roundUp = h.truncated || ((dp > 0) && (1 & h.digits[dp - 1]));
			}
		}
		if (roundUp) {
			size++;
		}
		return size;
	}

	inline void trim(Decimal& h) {
		while ((h.numDigits > 0) && (h.digits[h.numDigits - 1] == 0)) {
			h.numDigits--;
		}
	}

	inline uint32_t numberOfDigitsDecimalLeftShift(Decimal& h, uint32_t shift) {
		shift &= 63;

		uint32_t xA = numberOfDigitsDecimalLeftShiftTableOne[shift];
		uint32_t xB = numberOfDigitsDecimalLeftShiftTableOne[shift + 1];
		uint32_t numNewDigits = xA >> 11;
		uint32_t pow5A = 0x7FF & xA;
		uint32_t pow5B = 0x7FF & xB;

		StringViewPtr pow5 = &numberOfDigitsDecimalLeftShiftTableTwo[pow5A];

		uint32_t i = 0;
		uint32_t size = pow5B - pow5A;
		for (; i < size; i++) {
			if (i >= h.numDigits) {
				return numNewDigits - 1;
			} else if (h.digits[i] == pow5[i]) {
				continue;
			} else if (h.digits[i] < pow5[i]) {
				return numNewDigits - 1;
			} else {
				return numNewDigits;
			}
		}
		return numNewDigits;
	}

	inline void decimalLeftShift(Decimal& h, uint32_t shift) {
		if (h.numDigits == 0) {
			return;
		}
		uint32_t numNewDigits = numberOfDigitsDecimalLeftShift(h, shift);
		int32_t readIndex = int32_t(h.numDigits - 1);
		uint32_t writeIndex = h.numDigits - 1 + numNewDigits;
		uint64_t size = 0;

		while (readIndex >= 0) {
			size += uint64_t(h.digits[readIndex]) << shift;
			uint64_t quotient = size / 10;
			uint64_t remainder = size - (10 * quotient);
			if (writeIndex < maxDigits) {
				h.digits[writeIndex] = uint8_t(remainder);
			} else if (remainder > 0) {
				h.truncated = true;
			}
			size = quotient;
			writeIndex--;
			readIndex--;
		}
		while (size > 0) {
			uint64_t quotient = size / 10;
			uint64_t remainder = size - (10 * quotient);
			if (writeIndex < maxDigits) {
				h.digits[writeIndex] = uint8_t(remainder);
			} else if (remainder > 0) {
				h.truncated = true;
			}
			size = quotient;
			writeIndex--;
		}
		h.numDigits += numNewDigits;
		if (h.numDigits > maxDigits) {
			h.numDigits = maxDigits;
		}
		h.decimalPoint += int32_t(numNewDigits);
		trim(h);
	}

	inline void decimalRightShift(Decimal& h, uint32_t shift) {
		uint32_t readIndex = 0;
		uint32_t writeIndex = 0;

		uint64_t size = 0;

		while ((size >> shift) == 0) {
			if (readIndex < h.numDigits) {
				size = (10 * size) + h.digits[readIndex++];
			} else if (size == 0) {
				return;
			} else {
				while ((size >> shift) == 0) {
					size = 10 * size;
					readIndex++;
				}
				break;
			}
		}
		h.decimalPoint -= int32_t(readIndex - 1);
		if (h.decimalPoint < -decimalPointRange) {
			h.numDigits = 0;
			h.decimalPoint = 0;
			h.negative = false;
			h.truncated = false;
			return;
		}
		uint64_t mask = (uint64_t(1) << shift) - 1;
		while (readIndex < h.numDigits) {
			uint8_t newDigit = uint8_t(size >> shift);
			size = (10 * (size & mask)) + h.digits[readIndex++];
			h.digits[writeIndex++] = newDigit;
		}
		while (size > 0) {
			uint8_t newDigit = uint8_t(size >> shift);
			size = 10 * (size & mask);
			if (writeIndex < maxDigits) {
				h.digits[writeIndex++] = newDigit;
			} else if (newDigit > 0) {
				h.truncated = true;
			}
		}
		h.numDigits = writeIndex;
		trim(h);
	}

	inline bool isInteger(char c) {
		return (c >= '0' && c <= '9');
	}

	inline double toDouble(uint64_t mantissa, uint64_t realExponent, bool negative) {
		mantissa &= ~(1ull << 52);
		mantissa |= realExponent << 52;
		mantissa |= ((static_cast<uint64_t>(negative)) << 63);
		return *reinterpret_cast<double*>(&mantissa);
	}

	inline AdjustedMantissa computeFloat(Decimal& d) {
		AdjustedMantissa answer;
		if (d.numDigits == 0) {
			answer.power2 = 0;
			answer.mantissa = 0;
			return answer;
		}
		if (d.decimalPoint < -324) {
			answer.power2 = 0;
			answer.mantissa = 0;
			return answer;
		} else if (d.decimalPoint >= 310) {
			answer.power2 = infinitePower;
			answer.mantissa = 0;
			return answer;
		}

		const uint8_t powers[19]{ 0, 3, 6, 9, 13, 16, 19, 23, 26, 29, 33, 36, 39, 43, 46, 49, 53, 56, 59 };
		const uint32_t numPowers{ 19 };
		const uint32_t maxShift{ 60 };

		int32_t exp2 = 0;
		while (d.decimalPoint > 0) {
			uint32_t size = uint32_t(d.decimalPoint);
			uint32_t shift = (size < numPowers) ? powers[size] : maxShift;
			decimalRightShift(d, shift);
			if (d.decimalPoint < -decimalPointRange) {
				answer.power2 = 0;
				answer.mantissa = 0;
				return answer;
			}
			exp2 += int32_t(shift);
		}
		while (d.decimalPoint <= 0) {
			uint32_t shift;
			if (d.decimalPoint == 0) {
				if (d.digits[0] >= 5) {
					break;
				}
				shift = (d.digits[0] < 2) ? 2 : 1;
			} else {
				uint32_t size = uint32_t(-d.decimalPoint);
				shift = (size < numPowers) ? powers[size] : maxShift;
			}
			decimalLeftShift(d, shift);
			if (d.decimalPoint > decimalPointRange) {
				answer.power2 = 0xFF;
				answer.mantissa = 0;
				return answer;
			}
			exp2 -= int32_t(shift);
		}
		exp2--;
		const int32_t minimumExponentNew = minimumExponent;
		while ((minimumExponentNew + 1) > exp2) {
			uint32_t size = uint32_t((minimumExponentNew + 1) - exp2);
			if (size > maxShift) {
				size = maxShift;
			}
			decimalRightShift(d, size);
			exp2 += int32_t(size);
		}
		if ((exp2 - minimumExponentNew) >= infinitePower) {
			answer.power2 = infinitePower;
			answer.mantissa = 0;
			return answer;
		}

		const int32_t mantissaSizeInBits = mantissaExplicitBits + 1;
		decimalLeftShift(d, mantissaSizeInBits);

		uint64_t mantissa = round(d);
		if (mantissa >= (uint64_t(1) << mantissaSizeInBits)) {
			decimalRightShift(d, 1);
			exp2 += 1;
			mantissa = round(d);
			if ((exp2 - minimumExponentNew) >= infinitePower) {
				answer.power2 = infinitePower;
				answer.mantissa = 0;
				return answer;
			}
		}
		answer.power2 = exp2 - minimumExponent;
		if (mantissa < (uint64_t(1) << mantissaExplicitBits)) {
			answer.power2--;
		}
		answer.mantissa = mantissa & ((uint64_t(1) << mantissaExplicitBits) - 1);
		return answer;
	}

	inline Decimal parseDecimal(const char*& p) {
		Decimal answer;
		answer.numDigits = 0;
		answer.decimalPoint = 0;
		answer.truncated = false;
		answer.negative = (*p == '-');
		if ((*p == '-') || (*p == '+')) {
			++p;
		}

		while (*p == '0') {
			++p;
		}
		while (isInteger(*p)) {
			if (answer.numDigits < maxDigits) {
				answer.digits[answer.numDigits] = uint8_t(*p - '0');
			}
			answer.numDigits++;
			++p;
		}
		if (*p == '.') {
			++p;
			const char* firstAfterPeriod = p;
			if (answer.numDigits == 0) {
				while (*p == '0') {
					++p;
				}
			}
			while (isInteger(*p)) {
				if (answer.numDigits < maxDigits) {
					answer.digits[answer.numDigits] = uint8_t(*p - '0');
				}
				answer.numDigits++;
				++p;
			}
			answer.decimalPoint = int32_t(firstAfterPeriod - p);
		}
		if (answer.numDigits > 0) {
			const char* preverse = p - 1;
			int32_t trailingZeros = 0;
			while ((*preverse == '0') || (*preverse == '.')) {
				if (*preverse == '0') {
					trailingZeros++;
				};
				--preverse;
			}
			answer.decimalPoint += int32_t(answer.numDigits);
			answer.numDigits -= uint32_t(trailingZeros);
		}
		if (answer.numDigits > maxDigits) {
			answer.numDigits = maxDigits;
			answer.truncated = true;
		}
		if (('e' == *p) || ('E' == *p)) {
			++p;
			bool negExp = false;
			if ('-' == *p) {
				negExp = true;
				++p;
			} else if ('+' == *p) {
				++p;
			}
			int32_t expNumber = 0;
			while (isInteger(*p)) {
				uint8_t digit = uint8_t(*p - '0');
				if (expNumber < 0x10000) {
					expNumber = 10 * expNumber + digit;
				}
				++p;
			}
			answer.decimalPoint += (negExp ? -expNumber : expNumber);
		}
		return answer;
	}

	inline bool computeFloat64(int64_t power, uint64_t i, bool negative, double& d) {
		if (-22 <= power && power <= 22 && i <= 9007199254740991) {
			d = double(i);
			if (power < 0) {
				d = d / powerOfTen[-power];
			} else {
				d = d * powerOfTen[power];
			}
			if (negative) {
				d = -d;
			}
			return true;
		}
		if (i == 0) {
			d = negative ? -0.0 : 0.0;
			return true;
		}
		int64_t exponent = (((152170ll + 65536ll) * power) >> 16ll) + 1024ll + 63ll;
		int32_t lz = _lzcnt_u64(i);
		i <<= lz;
		const uint32_t index = 2 * uint32_t(power - smallestPower);
		Value128 firstProduct = fullMultiplication(i, powerOfFive128[index]);
		if ((firstProduct.high & 0x1FF) == 0x1FF) {
			Value128 secondProduct = fullMultiplication(i, powerOfFive128[index + 1]);
			firstProduct.low += secondProduct.high;
			if (secondProduct.high > firstProduct.low) {
				firstProduct.high++;
			}
			if (firstProduct.low == 0xFFFFFFFFFFFFFFFF) {
				return false;
			}
		}
		uint64_t lower = firstProduct.low;
		uint64_t upper = firstProduct.high;
		uint64_t upperbit = upper >> 63;
		uint64_t mantissa = upper >> (upperbit + 9);
		lz += int32_t(1 ^ upperbit);
		int64_t realExponent = exponent - lz;
		if (realExponent <= 0) {
			if (-realExponent + 1 >= 64) {
				d = negative ? -0.0 : 0.0;
				return true;
			}
			mantissa >>= -realExponent + 1;
			mantissa += (mantissa & 1);
			mantissa >>= 1;
			realExponent = (mantissa < (uint64_t(1) << 52)) ? 0 : 1;
			d = toDouble(mantissa, realExponent, negative);
			return true;
		}
		if ((lower <= 1) && (power >= -4) && (power <= 23) && ((mantissa & 3) == 1)) {
			if ((mantissa << (upperbit + 64 - 53 - 2)) == upper) {
				mantissa &= ~1;
			}
		}

		mantissa += mantissa & 1;
		mantissa >>= 1;

		if (mantissa >= (1ull << 53)) {
			mantissa = (1ull << 52);
			realExponent++;
		}
		mantissa &= ~(1ull << 52);
		if (realExponent > 2046) {
			return false;
		}
		d = toDouble(mantissa, realExponent, negative);
		return true;
	}

	inline AdjustedMantissa parseLongMantissa(const char* first) {
		Decimal d = parseDecimal(first);
		return computeFloat(d);
	}

	inline bool isNegative(StringViewPtr source) {
		return (*source == '-');
	}

	inline double fromChars(const char* first) {
		bool negative = first[0] == '-';
		if (negative) {
			first++;
		}
		AdjustedMantissa am = parseLongMantissa(first);
		uint64_t word = am.mantissa;
		word |= uint64_t(am.power2) << mantissaExplicitBits;
		word = negative ? word | (uint64_t(1) << signIndex) : word;
		double value;
		JsonifierCore::memcpy(&value, &word, sizeof(double));
		return value;
	}

	inline bool parseFloatFallback(StringViewPtr ptr, double* outDouble) {
		*outDouble = fromChars(reinterpret_cast<const char*>(ptr));
		return !(*outDouble > (std::numeric_limits<double>::max)() || *outDouble < std::numeric_limits<double>::lowest());
	}

	template<typename I> inline bool parseDigit(const uint8_t c, I& i) {
		const uint8_t digit = static_cast<uint8_t>(c - '0');
		if (digit > 9) {
			return false;
		}
		i = 10 * i + digit;
		return true;
	}

	inline uint64_t parseUnsigned(StringViewPtr const source) {
		StringViewPtr p = source;
		StringViewPtr const startDigits = p;
		uint64_t i = 0;
		while (parseDigit(*p, i)) {
			p++;
		}

		size_t digitCount = size_t(p - startDigits);
		if ((digitCount == 0) || (digitCount > 20)) {
			throw Incorrect_Type;
		}
		if (('0' == *startDigits) && (digitCount > 1)) {
			throw Number_Error;
		}

		if (digitCount == 20) {
			if (source[0] != uint8_t('1') || i <= uint64_t(std::numeric_limits<int64_t>::max())) {
				throw Incorrect_Type;
			}
		}

		return i;
	};

	inline int64_t parseInteger(StringViewPtr source) {
		bool negative = (*source == '-');
		StringViewPtr p = source + uint8_t(negative);
		StringViewPtr const startDigits = p;
		uint64_t i = 0;
		while (parseDigit(*p, i)) {
			p++;
		}

		size_t digitCount = size_t(p - startDigits);
		size_t longestDigitCount = 19;
		if ((digitCount == 0) || (digitCount > longestDigitCount)) {
			throw Incorrect_Type;
		}
		if (('0' == *startDigits) && (digitCount > 1)) {
			throw Number_Error;
		}
		if (i > uint64_t(std::numeric_limits<int64_t>::max()) + uint64_t(negative)) {
			throw Incorrect_Type;
		}
		return negative ? (~i + 1) : i;
	}

	// Based on yyjson: https://github.com/ibireme/yyjson/blob/master/src/yyjson.c with some changes to rounding and
	// dirrect for floats
	// TODO: Subnormals are not handled right now.
	// TODO: Check overflow for intss
	// Wontfix: Numbers with more than 19 sigfigs may be off by 1ulp. No algorithm should be outputing more than 17
	// digits so I dont think roundtripping matters if you supply extra digits

	inline constexpr std::array<uint64_t, 21> powersOfTenInt{ 1ull, 10ull, 100ull, 1000ull, 10000ull, 100000ull, 1000000ull, 10000000ull,
		100000000ull, 1000000000ull, 10000000000ull, 100000000000ull, 1000000000000ull, 10000000000000ull, 100000000000000ull, 1000000000000000ull,
		10000000000000000ull, 100000000000000000ull, 1000000000000000000ull, 10000000000000000000ull };
	inline constexpr std::array<double, 23> powersOfTenFloat = { 1e0, 1e1, 1e2, 1e3, 1e4, 1e5, 1e6, 1e7, 1e8, 1e9, 1e10, 1e11, 1e12, 1e13, 1e14, 1e15,
		1e16, 1e17, 1e18, 1e19, 1e20, 1e21, 1e22 };

// https://stackoverflow.com/questions/28868367/getting-the-high-part-of-64-bit-integer-multiplication
#ifdef __SIZEOF_INT128__
   inline uint64_t mulhi64(uint64_t a, uint64_t b)
   {
#if defined(__GNUC__) || defined(__GNUG__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
      unsigned __int128 prod = a * static_cast<unsigned __int128>(b);
#pragma GCC diagnostic pop
#endif
      return prod >> 64;
   }
#elif defined(_M_X64) || defined(_M_ARM64)
#define mulhi64 __umulh
#else
   uint64_t mulhi64(uint64_t a, uint64_t b)
   {
      uint64_t a_lo = (uint32_t)a;
      uint64_t a_hi = a >> 32;
      uint64_t b_lo = (uint32_t)b;
      uint64_t b_hi = b >> 32;
      uint64_t a_x_b_hi = a_hi * b_hi;
      uint64_t a_x_b_mid = a_hi * b_lo;
      uint64_t b_x_a_mid = b_hi * a_lo;
      uint64_t a_x_b_lo = a_lo * b_lo;
      uint64_t carry_bit = ((uint64_t)(uint32_t)a_x_b_mid + (uint64_t)(uint32_t)b_x_a_mid + (a_x_b_lo >> 32)) >> 32;
      uint64_t multhi = a_x_b_hi + (a_x_b_mid >> 32) + (b_x_a_mid >> 32) + carry_bit;
      return multhi;
   }
#endif

	// Min decimal exponent in pow10SigTable.
	inline constexpr auto pow10SigTableMinExp = -343;
	// Max decimal exponent in pow10SigTable.
	inline constexpr auto pow10SigTableMaxExp = 324;
	// Min exact decimal exponent in pow10SigTable
	inline constexpr auto pow10SigTableMinExact = 0;
	// Max exact decimal exponent in pow10SigTable
	inline constexpr auto pow10SigTableMaxExact = 27;
	inline constexpr std::array<uint64_t, 668> pow10SigTable = { 0xBF29DCABA82FDEAE, 0xEEF453D6923BD65A, 0x9558B4661B6565F8, 0xBAAEE17FA23EBF76,
		0xE95A99DF8ACE6F53, 0x91D8A02BB6C10594, 0xB64EC836A47146F9, 0xE3E27A444D8D98B7, 0x8E6D8C6AB0787F72, 0xB208EF855C969F4F, 0xDE8B2B66B3BC4723,
		0x8B16FB203055AC76, 0xADDCB9E83C6B1793, 0xD953E8624B85DD78, 0x87D4713D6F33AA6B, 0xA9C98D8CCB009506, 0xD43BF0EFFDC0BA48, 0x84A57695FE98746D,
		0xA5CED43B7E3E9188, 0xCF42894A5DCE35EA, 0x818995CE7AA0E1B2, 0xA1EBFB4219491A1F, 0xCA66FA129F9B60A6, 0xFD00B897478238D0, 0x9E20735E8CB16382,
		0xC5A890362FDDBC62, 0xF712B443BBD52B7B, 0x9A6BB0AA55653B2D, 0xC1069CD4EABE89F8, 0xF148440A256E2C76, 0x96CD2A865764DBCA, 0xBC807527ED3E12BC,
		0xEBA09271E88D976B, 0x93445B8731587EA3, 0xB8157268FDAE9E4C, 0xE61ACF033D1A45DF, 0x8FD0C16206306BAB, 0xB3C4F1BA87BC8696, 0xE0B62E2929ABA83C,
		0x8C71DCD9BA0B4925, 0xAF8E5410288E1B6F, 0xDB71E91432B1A24A, 0x892731AC9FAF056E, 0xAB70FE17C79AC6CA, 0xD64D3D9DB981787D, 0x85F0468293F0EB4E,
		0xA76C582338ED2621, 0xD1476E2C07286FAA, 0x82CCA4DB847945CA, 0xA37FCE126597973C, 0xCC5FC196FEFD7D0C, 0xFF77B1FCBEBCDC4F, 0x9FAACF3DF73609B1,
		0xC795830D75038C1D, 0xF97AE3D0D2446F25, 0x9BECCE62836AC577, 0xC2E801FB244576D5, 0xF3A20279ED56D48A, 0x9845418C345644D6, 0xBE5691EF416BD60C,
		0xEDEC366B11C6CB8F, 0x94B3A202EB1C3F39, 0xB9E08A83A5E34F07, 0xE858AD248F5C22C9, 0x91376C36D99995BE, 0xB58547448FFFFB2D, 0xE2E69915B3FFF9F9,
		0x8DD01FAD907FFC3B, 0xB1442798F49FFB4A, 0xDD95317F31C7FA1D, 0x8A7D3EEF7F1CFC52, 0xAD1C8EAB5EE43B66, 0xD863B256369D4A40, 0x873E4F75E2224E68,
		0xA90DE3535AAAE202, 0xD3515C2831559A83, 0x8412D9991ED58091, 0xA5178FFF668AE0B6, 0xCE5D73FF402D98E3, 0x80FA687F881C7F8E, 0xA139029F6A239F72,
		0xC987434744AC874E, 0xFBE9141915D7A922, 0x9D71AC8FADA6C9B5, 0xC4CE17B399107C22, 0xF6019DA07F549B2B, 0x99C102844F94E0FB, 0xC0314325637A1939,
		0xF03D93EEBC589F88, 0x96267C7535B763B5, 0xBBB01B9283253CA2, 0xEA9C227723EE8BCB, 0x92A1958A7675175F, 0xB749FAED14125D36, 0xE51C79A85916F484,
		0x8F31CC0937AE58D2, 0xB2FE3F0B8599EF07, 0xDFBDCECE67006AC9, 0x8BD6A141006042BD, 0xAECC49914078536D, 0xDA7F5BF590966848, 0x888F99797A5E012D,
		0xAAB37FD7D8F58178, 0xD5605FCDCF32E1D6, 0x855C3BE0A17FCD26, 0xA6B34AD8C9DFC06F, 0xD0601D8EFC57B08B, 0x823C12795DB6CE57, 0xA2CB1717B52481ED,
		0xCB7DDCDDA26DA268, 0xFE5D54150B090B02, 0x9EFA548D26E5A6E1, 0xC6B8E9B0709F109A, 0xF867241C8CC6D4C0, 0x9B407691D7FC44F8, 0xC21094364DFB5636,
		0xF294B943E17A2BC4, 0x979CF3CA6CEC5B5A, 0xBD8430BD08277231, 0xECE53CEC4A314EBD, 0x940F4613AE5ED136, 0xB913179899F68584, 0xE757DD7EC07426E5,
		0x9096EA6F3848984F, 0xB4BCA50B065ABE63, 0xE1EBCE4DC7F16DFB, 0x8D3360F09CF6E4BD, 0xB080392CC4349DEC, 0xDCA04777F541C567, 0x89E42CAAF9491B60,
		0xAC5D37D5B79B6239, 0xD77485CB25823AC7, 0x86A8D39EF77164BC, 0xA8530886B54DBDEB, 0xD267CAA862A12D66, 0x8380DEA93DA4BC60, 0xA46116538D0DEB78,
		0xCD795BE870516656, 0x806BD9714632DFF6, 0xA086CFCD97BF97F3, 0xC8A883C0FDAF7DF0, 0xFAD2A4B13D1B5D6C, 0x9CC3A6EEC6311A63, 0xC3F490AA77BD60FC,
		0xF4F1B4D515ACB93B, 0x991711052D8BF3C5, 0xBF5CD54678EEF0B6, 0xEF340A98172AACE4, 0x9580869F0E7AAC0E, 0xBAE0A846D2195712, 0xE998D258869FACD7,
		0x91FF83775423CC06, 0xB67F6455292CBF08, 0xE41F3D6A7377EECA, 0x8E938662882AF53E, 0xB23867FB2A35B28D, 0xDEC681F9F4C31F31, 0x8B3C113C38F9F37E,
		0xAE0B158B4738705E, 0xD98DDAEE19068C76, 0x87F8A8D4CFA417C9, 0xA9F6D30A038D1DBC, 0xD47487CC8470652B, 0x84C8D4DFD2C63F3B, 0xA5FB0A17C777CF09,
		0xCF79CC9DB955C2CC, 0x81AC1FE293D599BF, 0xA21727DB38CB002F, 0xCA9CF1D206FDC03B, 0xFD442E4688BD304A, 0x9E4A9CEC15763E2E, 0xC5DD44271AD3CDBA,
		0xF7549530E188C128, 0x9A94DD3E8CF578B9, 0xC13A148E3032D6E7, 0xF18899B1BC3F8CA1, 0x96F5600F15A7B7E5, 0xBCB2B812DB11A5DE, 0xEBDF661791D60F56,
		0x936B9FCEBB25C995, 0xB84687C269EF3BFB, 0xE65829B3046B0AFA, 0x8FF71A0FE2C2E6DC, 0xB3F4E093DB73A093, 0xE0F218B8D25088B8, 0x8C974F7383725573,
		0xAFBD2350644EEACF, 0xDBAC6C247D62A583, 0x894BC396CE5DA772, 0xAB9EB47C81F5114F, 0xD686619BA27255A2, 0x8613FD0145877585, 0xA798FC4196E952E7,
		0xD17F3B51FCA3A7A0, 0x82EF85133DE648C4, 0xA3AB66580D5FDAF5, 0xCC963FEE10B7D1B3, 0xFFBBCFE994E5C61F, 0x9FD561F1FD0F9BD3, 0xC7CABA6E7C5382C8,
		0xF9BD690A1B68637B, 0x9C1661A651213E2D, 0xC31BFA0FE5698DB8, 0xF3E2F893DEC3F126, 0x986DDB5C6B3A76B7, 0xBE89523386091465, 0xEE2BA6C0678B597F,
		0x94DB483840B717EF, 0xBA121A4650E4DDEB, 0xE896A0D7E51E1566, 0x915E2486EF32CD60, 0xB5B5ADA8AAFF80B8, 0xE3231912D5BF60E6, 0x8DF5EFABC5979C8F,
		0xB1736B96B6FD83B3, 0xDDD0467C64BCE4A0, 0x8AA22C0DBEF60EE4, 0xAD4AB7112EB3929D, 0xD89D64D57A607744, 0x87625F056C7C4A8B, 0xA93AF6C6C79B5D2D,
		0xD389B47879823479, 0x843610CB4BF160CB, 0xA54394FE1EEDB8FE, 0xCE947A3DA6A9273E, 0x811CCC668829B887, 0xA163FF802A3426A8, 0xC9BCFF6034C13052,
		0xFC2C3F3841F17C67, 0x9D9BA7832936EDC0, 0xC5029163F384A931, 0xF64335BCF065D37D, 0x99EA0196163FA42E, 0xC06481FB9BCF8D39, 0xF07DA27A82C37088,
		0x964E858C91BA2655, 0xBBE226EFB628AFEA, 0xEADAB0ABA3B2DBE5, 0x92C8AE6B464FC96F, 0xB77ADA0617E3BBCB, 0xE55990879DDCAABD, 0x8F57FA54C2A9EAB6,
		0xB32DF8E9F3546564, 0xDFF9772470297EBD, 0x8BFBEA76C619EF36, 0xAEFAE51477A06B03, 0xDAB99E59958885C4, 0x88B402F7FD75539B, 0xAAE103B5FCD2A881,
		0xD59944A37C0752A2, 0x857FCAE62D8493A5, 0xA6DFBD9FB8E5B88E, 0xD097AD07A71F26B2, 0x825ECC24C873782F, 0xA2F67F2DFA90563B, 0xCBB41EF979346BCA,
		0xFEA126B7D78186BC, 0x9F24B832E6B0F436, 0xC6EDE63FA05D3143, 0xF8A95FCF88747D94, 0x9B69DBE1B548CE7C, 0xC24452DA229B021B, 0xF2D56790AB41C2A2,
		0x97C560BA6B0919A5, 0xBDB6B8E905CB600F, 0xED246723473E3813, 0x9436C0760C86E30B, 0xB94470938FA89BCE, 0xE7958CB87392C2C2, 0x90BD77F3483BB9B9,
		0xB4ECD5F01A4AA828, 0xE2280B6C20DD5232, 0x8D590723948A535F, 0xB0AF48EC79ACE837, 0xDCDB1B2798182244, 0x8A08F0F8BF0F156B, 0xAC8B2D36EED2DAC5,
		0xD7ADF884AA879177, 0x86CCBB52EA94BAEA, 0xA87FEA27A539E9A5, 0xD29FE4B18E88640E, 0x83A3EEEEF9153E89, 0xA48CEAAAB75A8E2B, 0xCDB02555653131B6,
		0x808E17555F3EBF11, 0xA0B19D2AB70E6ED6, 0xC8DE047564D20A8B, 0xFB158592BE068D2E, 0x9CED737BB6C4183D, 0xC428D05AA4751E4C, 0xF53304714D9265DF,
		0x993FE2C6D07B7FAB, 0xBF8FDB78849A5F96, 0xEF73D256A5C0F77C, 0x95A8637627989AAD, 0xBB127C53B17EC159, 0xE9D71B689DDE71AF, 0x9226712162AB070D,
		0xB6B00D69BB55C8D1, 0xE45C10C42A2B3B05, 0x8EB98A7A9A5B04E3, 0xB267ED1940F1C61C, 0xDF01E85F912E37A3, 0x8B61313BBABCE2C6, 0xAE397D8AA96C1B77,
		0xD9C7DCED53C72255, 0x881CEA14545C7575, 0xAA242499697392D2, 0xD4AD2DBFC3D07787, 0x84EC3C97DA624AB4, 0xA6274BBDD0FADD61, 0xCFB11EAD453994BA,
		0x81CEB32C4B43FCF4, 0xA2425FF75E14FC31, 0xCAD2F7F5359A3B3E, 0xFD87B5F28300CA0D, 0x9E74D1B791E07E48, 0xC612062576589DDA, 0xF79687AED3EEC551,
		0x9ABE14CD44753B52, 0xC16D9A0095928A27, 0xF1C90080BAF72CB1, 0x971DA05074DA7BEE, 0xBCE5086492111AEA, 0xEC1E4A7DB69561A5, 0x9392EE8E921D5D07,
		0xB877AA3236A4B449, 0xE69594BEC44DE15B, 0x901D7CF73AB0ACD9, 0xB424DC35095CD80F, 0xE12E13424BB40E13, 0x8CBCCC096F5088CB, 0xAFEBFF0BCB24AAFE,
		0xDBE6FECEBDEDD5BE, 0x89705F4136B4A597, 0xABCC77118461CEFC, 0xD6BF94D5E57A42BC, 0x8637BD05AF6C69B5, 0xA7C5AC471B478423, 0xD1B71758E219652B,
		0x83126E978D4FDF3B, 0xA3D70A3D70A3D70A, 0xCCCCCCCCCCCCCCCC, 0x8000000000000000, 0xA000000000000000, 0xC800000000000000, 0xFA00000000000000,
		0x9C40000000000000, 0xC350000000000000, 0xF424000000000000, 0x9896800000000000, 0xBEBC200000000000, 0xEE6B280000000000, 0x9502F90000000000,
		0xBA43B74000000000, 0xE8D4A51000000000, 0x9184E72A00000000, 0xB5E620F480000000, 0xE35FA931A0000000, 0x8E1BC9BF04000000, 0xB1A2BC2EC5000000,
		0xDE0B6B3A76400000, 0x8AC7230489E80000, 0xAD78EBC5AC620000, 0xD8D726B7177A8000, 0x878678326EAC9000, 0xA968163F0A57B400, 0xD3C21BCECCEDA100,
		0x84595161401484A0, 0xA56FA5B99019A5C8, 0xCECB8F27F4200F3A, 0x813F3978F8940984, 0xA18F07D736B90BE5, 0xC9F2C9CD04674EDE, 0xFC6F7C4045812296,
		0x9DC5ADA82B70B59D, 0xC5371912364CE305, 0xF684DF56C3E01BC6, 0x9A130B963A6C115C, 0xC097CE7BC90715B3, 0xF0BDC21ABB48DB20, 0x96769950B50D88F4,
		0xBC143FA4E250EB31, 0xEB194F8E1AE525FD, 0x92EFD1B8D0CF37BE, 0xB7ABC627050305AD, 0xE596B7B0C643C719, 0x8F7E32CE7BEA5C6F, 0xB35DBF821AE4F38B,
		0xE0352F62A19E306E, 0x8C213D9DA502DE45, 0xAF298D050E4395D6, 0xDAF3F04651D47B4C, 0x88D8762BF324CD0F, 0xAB0E93B6EFEE0053, 0xD5D238A4ABE98068,
		0x85A36366EB71F041, 0xA70C3C40A64E6C51, 0xD0CF4B50CFE20765, 0x82818F1281ED449F, 0xA321F2D7226895C7, 0xCBEA6F8CEB02BB39, 0xFEE50B7025C36A08,
		0x9F4F2726179A2245, 0xC722F0EF9D80AAD6, 0xF8EBAD2B84E0D58B, 0x9B934C3B330C8577, 0xC2781F49FFCFA6D5, 0xF316271C7FC3908A, 0x97EDD871CFDA3A56,
		0xBDE94E8E43D0C8EC, 0xED63A231D4C4FB27, 0x945E455F24FB1CF8, 0xB975D6B6EE39E436, 0xE7D34C64A9C85D44, 0x90E40FBEEA1D3A4A, 0xB51D13AEA4A488DD,
		0xE264589A4DCDAB14, 0x8D7EB76070A08AEC, 0xB0DE65388CC8ADA8, 0xDD15FE86AFFAD912, 0x8A2DBF142DFCC7AB, 0xACB92ED9397BF996, 0xD7E77A8F87DAF7FB,
		0x86F0AC99B4E8DAFD, 0xA8ACD7C0222311BC, 0xD2D80DB02AABD62B, 0x83C7088E1AAB65DB, 0xA4B8CAB1A1563F52, 0xCDE6FD5E09ABCF26, 0x80B05E5AC60B6178,
		0xA0DC75F1778E39D6, 0xC913936DD571C84C, 0xFB5878494ACE3A5F, 0x9D174B2DCEC0E47B, 0xC45D1DF942711D9A, 0xF5746577930D6500, 0x9968BF6ABBE85F20,
		0xBFC2EF456AE276E8, 0xEFB3AB16C59B14A2, 0x95D04AEE3B80ECE5, 0xBB445DA9CA61281F, 0xEA1575143CF97226, 0x924D692CA61BE758, 0xB6E0C377CFA2E12E,
		0xE498F455C38B997A, 0x8EDF98B59A373FEC, 0xB2977EE300C50FE7, 0xDF3D5E9BC0F653E1, 0x8B865B215899F46C, 0xAE67F1E9AEC07187, 0xDA01EE641A708DE9,
		0x884134FE908658B2, 0xAA51823E34A7EEDE, 0xD4E5E2CDC1D1EA96, 0x850FADC09923329E, 0xA6539930BF6BFF45, 0xCFE87F7CEF46FF16, 0x81F14FAE158C5F6E,
		0xA26DA3999AEF7749, 0xCB090C8001AB551C, 0xFDCB4FA002162A63, 0x9E9F11C4014DDA7E, 0xC646D63501A1511D, 0xF7D88BC24209A565, 0x9AE757596946075F,
		0xC1A12D2FC3978937, 0xF209787BB47D6B84, 0x9745EB4D50CE6332, 0xBD176620A501FBFF, 0xEC5D3FA8CE427AFF, 0x93BA47C980E98CDF, 0xB8A8D9BBE123F017,
		0xE6D3102AD96CEC1D, 0x9043EA1AC7E41392, 0xB454E4A179DD1877, 0xE16A1DC9D8545E94, 0x8CE2529E2734BB1D, 0xB01AE745B101E9E4, 0xDC21A1171D42645D,
		0x899504AE72497EBA, 0xABFA45DA0EDBDE69, 0xD6F8D7509292D603, 0x865B86925B9BC5C2, 0xA7F26836F282B732, 0xD1EF0244AF2364FF, 0x8335616AED761F1F,
		0xA402B9C5A8D3A6E7, 0xCD036837130890A1, 0x802221226BE55A64, 0xA02AA96B06DEB0FD, 0xC83553C5C8965D3D, 0xFA42A8B73ABBF48C, 0x9C69A97284B578D7,
		0xC38413CF25E2D70D, 0xF46518C2EF5B8CD1, 0x98BF2F79D5993802, 0xBEEEFB584AFF8603, 0xEEAABA2E5DBF6784, 0x952AB45CFA97A0B2, 0xBA756174393D88DF,
		0xE912B9D1478CEB17, 0x91ABB422CCB812EE, 0xB616A12B7FE617AA, 0xE39C49765FDF9D94, 0x8E41ADE9FBEBC27D, 0xB1D219647AE6B31C, 0xDE469FBD99A05FE3,
		0x8AEC23D680043BEE, 0xADA72CCC20054AE9, 0xD910F7FF28069DA4, 0x87AA9AFF79042286, 0xA99541BF57452B28, 0xD3FA922F2D1675F2, 0x847C9B5D7C2E09B7,
		0xA59BC234DB398C25, 0xCF02B2C21207EF2E, 0x8161AFB94B44F57D, 0xA1BA1BA79E1632DC, 0xCA28A291859BBF93, 0xFCB2CB35E702AF78, 0x9DEFBF01B061ADAB,
		0xC56BAEC21C7A1916, 0xF6C69A72A3989F5B, 0x9A3C2087A63F6399, 0xC0CB28A98FCF3C7F, 0xF0FDF2D3F3C30B9F, 0x969EB7C47859E743, 0xBC4665B596706114,
		0xEB57FF22FC0C7959, 0x9316FF75DD87CBD8, 0xB7DCBF5354E9BECE, 0xE5D3EF282A242E81, 0x8FA475791A569D10, 0xB38D92D760EC4455, 0xE070F78D3927556A,
		0x8C469AB843B89562, 0xAF58416654A6BABB, 0xDB2E51BFE9D0696A, 0x88FCF317F22241E2, 0xAB3C2FDDEEAAD25A, 0xD60B3BD56A5586F1, 0x85C7056562757456,
		0xA738C6BEBB12D16C, 0xD106F86E69D785C7, 0x82A45B450226B39C, 0xA34D721642B06084, 0xCC20CE9BD35C78A5, 0xFF290242C83396CE, 0x9F79A169BD203E41,
		0xC75809C42C684DD1, 0xF92E0C3537826145, 0x9BBCC7A142B17CCB, 0xC2ABF989935DDBFE, 0xF356F7EBF83552FE, 0x98165AF37B2153DE, 0xBE1BF1B059E9A8D6,
		0xEDA2EE1C7064130C, 0x9485D4D1C63E8BE7, 0xB9A74A0637CE2EE1, 0xE8111C87C5C1BA99, 0x910AB1D4DB9914A0, 0xB54D5E4A127F59C8, 0xE2A0B5DC971F303A,
		0x8DA471A9DE737E24, 0xB10D8E1456105DAD, 0xDD50F1996B947518, 0x8A5296FFE33CC92F, 0xACE73CBFDC0BFB7B, 0xD8210BEFD30EFA5A, 0x8714A775E3E95C78,
		0xA8D9D1535CE3B396, 0xD31045A8341CA07C, 0x83EA2B892091E44D, 0xA4E4B66B68B65D60, 0xCE1DE40642E3F4B9, 0x80D2AE83E9CE78F3, 0xA1075A24E4421730,
		0xC94930AE1D529CFC, 0xFB9B7CD9A4A7443C, 0x9D412E0806E88AA5, 0xC491798A08A2AD4E, 0xF5B5D7EC8ACB58A2, 0x9991A6F3D6BF1765, 0xBFF610B0CC6EDD3F,
		0xEFF394DCFF8A948E, 0x95F83D0A1FB69CD9, 0xBB764C4CA7A4440F, 0xEA53DF5FD18D5513, 0x92746B9BE2F8552C, 0xB7118682DBB66A77, 0xE4D5E82392A40515,
		0x8F05B1163BA6832D, 0xB2C71D5BCA9023F8, 0xDF78E4B2BD342CF6, 0x8BAB8EEFB6409C1A, 0xAE9672ABA3D0C320, 0xDA3C0F568CC4F3E8, 0x8865899617FB1871,
		0xAA7EEBFB9DF9DE8D, 0xD51EA6FA85785631, 0x8533285C936B35DE, 0xA67FF273B8460356, 0xD01FEF10A657842C, 0x8213F56A67F6B29B, 0xA298F2C501F45F42,
		0xCB3F2F7642717713, 0xFE0EFB53D30DD4D7, 0x9EC95D1463E8A506, 0xC67BB4597CE2CE48, 0xF81AA16FDC1B81DA, 0x9B10A4E5E9913128, 0xC1D4CE1F63F57D72,
		0xF24A01A73CF2DCCF, 0x976E41088617CA01, 0xBD49D14AA79DBC82, 0xEC9C459D51852BA2, 0x93E1AB8252F33B45, 0xB8DA1662E7B00A17, 0xE7109BFBA19C0C9D,
		0x906A617D450187E2, 0xB484F9DC9641E9DA, 0xE1A63853BBD26451, 0x8D07E33455637EB2, 0xB049DC016ABC5E5F, 0xDC5C5301C56B75F7, 0x89B9B3E11B6329BA,
		0xAC2820D9623BF429, 0xD732290FBACAF133, 0x867F59A9D4BED6C0, 0xA81F301449EE8C70, 0xD226FC195C6A2F8C, 0x83585D8FD9C25DB7, 0xA42E74F3D032F525,
		0xCD3A1230C43FB26F, 0x80444B5E7AA7CF85, 0xA0555E361951C366, 0xC86AB5C39FA63440, 0xFA856334878FC150, 0x9C935E00D4B9D8D2, 0xC3B8358109E84F07,
		0xF4A642E14C6262C8, 0x98E7E9CCCFBD7DBD, 0xBF21E44003ACDD2C, 0xEEEA5D5004981478, 0x95527A5202DF0CCB, 0xBAA718E68396CFFD, 0xE950DF20247C83FD,
		0x91D28B7416CDD27E, 0xB6472E511C81471D, 0xE3D8F9E563A198E5, 0x8E679C2F5E44FF8F, 0xB201833B35D63F73, 0xDE81E40A034BCF4F, 0x8B112E86420F6191,
		0xADD57A27D29339F6, 0xD94AD8B1C7380874, 0x87CEC76F1C830548, 0xA9C2794AE3A3C69A, 0xD433179D9C8CB841, 0x849FEEC281D7F328, 0xA5C7EA73224DEFF3,
		0xCF39E50FEAE16BEF, 0x81842F29F2CCE375, 0xA1E53AF46F801C53, 0xCA5E89B18B602368, 0xFCF62C1DEE382C42, 0x9E19DB92B4E31BA9 };

	inline uint64_t sig2FromExp10(int32_t exp10) noexcept {
		return pow10SigTable[static_cast<size_t>(exp10) - pow10SigTableMinExp];
	}

	inline int32_t exp2FromExp10(int32_t exp10) noexcept {
		return (((exp10 * 217706 - 4128768) >> 16) + 126);
	}

	/*==============================================================================
    * Digit Character Matcher
    *============================================================================*/
	/** Digit type */
	using digi_type = uint8_t;
	/** Digit: '0'. */
	inline constexpr digi_type DIGI_TYPE_ZERO = 1 << 0;
	/** Digit: [1-9]. */
	inline constexpr digi_type DIGI_TYPE_NONZERO = 1 << 1;
	/** Plus sign (positive): '+'. */
	inline constexpr digi_type DIGI_TYPE_POS = 1 << 2;
	/** Minus sign (negative): '-'. */
	inline constexpr digi_type DIGI_TYPE_NEG = 1 << 3;
	/** Decimal point: '.' */
	inline constexpr digi_type DIGI_TYPE_DOT = 1 << 4;
	/** Exponent sign: 'e, 'E'. */
	inline constexpr digi_type DIGI_TYPE_EXP = 1 << 5;
	/** Digit type table (generate with misc/make_tables.c) */
	inline constexpr std::array<digi_type, 256> digiTable = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00, 0x08, 0x10, 0x00, 0x01, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
	/** Match a character with specified type. */
	inline bool digiIsType(uint8_t d, digi_type type) noexcept {
		return (digiTable[d] & type) != 0;
	}
	/** Match a floating point indicator: '.', 'e', 'E'. */
	inline bool digiIsFp(uint8_t d) noexcept {
		return digiIsType(d, digi_type(DIGI_TYPE_DOT | DIGI_TYPE_EXP));
	}
	/** Match a digit or floating point indicator: [0-9], '.', 'e', 'E'. */
	inline bool digiIsDigitOrFp(uint8_t d) noexcept {
		return digiIsType(d, digi_type(DIGI_TYPE_ZERO | DIGI_TYPE_NONZERO | DIGI_TYPE_DOT | DIGI_TYPE_EXP));
	}
/* Macros used for loop unrolling and other purpose. */
#define repeat2(x) \
	{ x x }
#define repeat3(x) \
	{ x x x }
#define repeat4(x) \
	{ x x x x }
#define repeat8(x) \
	{ x x x x x x x x }
#define repeat16(x) \
	{ x x x x x x x x x x x x x x x x }
#define repeat2_incr(x) \
	{ x(0) x(1) }
#define repeat4_incr(x) \
	{ x(0) x(1) x(2) x(3) }
#define repeat8_incr(x) \
	{ x(0) x(1) x(2) x(3) x(4) x(5) x(6) x(7) }
#define repeat16_incr(x) \
	{ x(0) x(1) x(2) x(3) x(4) x(5) x(6) x(7) x(8) x(9) x(10) x(11) x(12) x(13) x(14) x(15) }
#define repeat_in_1_18(x) \
	{ x(1) x(2) x(3) x(4) x(5) x(6) x(7) x(8) x(9) x(10) x(11) x(12) x(13) x(14) x(15) x(16) x(17) x(18) }
	inline constexpr auto e_bit = static_cast<uint8_t>('E' ^ 'e');
	/*==============================================================================
    * IEEE-754 Double Number Constants
    *============================================================================*/
	/* maximum decimal power of double number (1.7976931348623157e308) */
	inline constexpr auto F64_MAX_DEC_EXP = 308;
	/* minimum decimal power of double number (4.9406564584124654e-324) */
	inline constexpr auto F64_MIN_DEC_EXP = (-324);

	consteval uint32_t ceillog2(uint32_t x) {
		return x < 2 ? x : 1 + ceillog2(x >> 1);
	}

	struct BigIntT {
		std::vector<uint32_t> data = {};

		BigIntT(uint64_t num) {
			uint32_t lower_word = uint32_t(num);
			uint32_t upper_word = uint32_t(num >> 32);
			if (upper_word > 0) {
				data = { lower_word, upper_word };
			} else {
				data = { lower_word };
			}
		}

		void mulU32(uint32_t num) {
			uint32_t carry = 0;
			for (size_t i = 0; i < data.size(); i++) {
				uint64_t res = uint64_t(data[i]) * uint64_t(num) + uint64_t(carry);
				uint32_t lower_word = uint32_t(res);
				uint32_t upper_word = uint32_t(res >> 32);
				data[i] = lower_word;
				carry = upper_word;
			}
			if (carry != 0) {
				data.emplace_back(carry);
			}
		}

		void mulPow10(uint32_t pow10) {
			for (; pow10 >= 9; pow10 -= 9) {
				mulU32(static_cast<uint32_t>(powersOfTenInt[9]));
			}
			if (pow10) {
				mulU32(static_cast<uint32_t>(powersOfTenInt[pow10]));
			}
		}

		void mulPow2(uint32_t exp) {
			uint32_t shft = exp % 32;
			uint32_t move = exp / 32;
			uint32_t idx = static_cast<uint32_t>(data.size()) - 1;
			if (shft == 0) {
				data.resize(data.size() + move);
				for (; idx > 0; idx--) {
					data[static_cast<size_t>(idx) + static_cast<size_t>(move) - 1ull] = data[static_cast<size_t>(idx) - 1ull];
				}
				while (move)
					data[--move] = 0;
			} else {
				data.resize(data.size() + move + 1);
				++idx;
				for (; idx > 0; idx--) {
					uint32_t num = data[idx] << shft;
					num |= data[idx - 1ull] >> (32 - shft);
					data[static_cast<size_t>(idx) + static_cast<size_t>(move)] = num;
				}
				data[move] = data[0] << shft;
				if (data.back() == 0)
					data.resize(data.size() - 1);
				while (move)
					data[--move] = 0;
			}
		}

		auto operator<=>(const BigIntT& rhs) const {
			if (data.size() < rhs.data.size())
				return -1;
			if (data.size() > rhs.data.size())
				return 1;
			for (auto i = data.size() - 1; i > 0; --i) {
				;
				if (data[i] < rhs.data[i])
					return -1;
				if (data[i] > rhs.data[i])
					return 1;
			}
			return 0;
		}
	};

	#pragma once

#include <array>
#include <concepts>
#include <cstdint>
#include <cstring>

#if defined(_M_X64) || defined(_M_ARM64)
	#ifndef __linux__
		#include <intrin.h>
	#else
		#include <x86intrin.h>
	#endif
#endif

	// Source: https://github.com/ibireme/yyjson/blob/master/src/yyjson.c

	/** Multiplies two 64-bit unsigned integers (a * b),
       returns the 128-bit result as 'hi' and 'lo'. */
	inline void u128_mul(uint64_t a, uint64_t b, uint64_t *hi, uint64_t *lo)
   {
#ifdef __SIZEOF_INT128__
#if defined(__GNUC__) || defined(__GNUG__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
#endif
      unsigned __int128 m = static_cast<unsigned __int128>(a) * b;
#if defined(__GNUC__) || defined(__GNUG__)
#pragma GCC diagnostic pop
#endif
      *hi = uint64_t(m >> 64);
      *lo = uint64_t(m);
#elif defined(_M_X64)
      *lo = _umul128(a, b, hi);
#elif defined(_M_ARM64)
      *hi = __umulh(a, b);
      *lo = a * b;
#else
      uint32_t a0 = (uint32_t)(a), a1 = (uint32_t)(a >> 32);
      uint32_t b0 = (uint32_t)(b), b1 = (uint32_t)(b >> 32);
      uint64_t p00 = (uint64_t)a0 * b0, p01 = (uint64_t)a0 * b1;
      uint64_t p10 = (uint64_t)a1 * b0, p11 = (uint64_t)a1 * b1;
      uint64_t m0 = p01 + (p00 >> 32);
      uint32_t m00 = (uint32_t)(m0), m01 = (uint32_t)(m0 >> 32);
      uint64_t m1 = p10 + m00;
      uint32_t m10 = (uint32_t)(m1), m11 = (uint32_t)(m1 >> 32);
      *hi = p11 + m01 + m11;
      *lo = ((uint64_t)m10 << 32) | (uint32_t)p00;
#endif
   }

   /** Multiplies two 64-bit unsigned integers and add a value (a * b + c),
       returns the 128-bit result as 'hi' and 'lo'. */
   inline void u128_mul_add(uint64_t a, uint64_t b, uint64_t c, uint64_t *hi, uint64_t *lo)
   {
#ifdef __SIZEOF_INT128__
#if defined(__GNUC__) || defined(__GNUG__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
#endif
      unsigned __int128 m = static_cast<unsigned __int128>(a) * b + c;
#if defined(__GNUC__) || defined(__GNUG__)
#pragma GCC diagnostic pop
#endif
      *hi = uint64_t(m >> 64);
      *lo = uint64_t(m);
#else
      uint64_t h, l, t;
      u128_mul(a, b, &h, &l);
      t = l + c;
      h += ((t < l) | (t < c));
      *hi = h;
      *lo = t;
#endif
   }

   /** Multiplies 128-bit integer and returns highest 64-bit rounded value. */
   inline uint64_t roundToOdd(uint64_t hi, uint64_t lo, uint64_t cp)
   {
      uint64_t x_hi, x_lo, y_hi, y_lo;
      u128_mul(cp, lo, &x_hi, &x_lo);
      u128_mul_add(cp, hi, x_hi, &y_hi, &y_lo);
      return y_hi | (y_lo > 1);
   }

	template<class OTy> inline bool parseNumber(OTy& val, auto*& cur) noexcept {
		const uint8_t* sig_cut = nullptr; /* significant part cutting position for long number */
		[[maybe_unused]] const uint8_t* sig_end = nullptr; /* significant part ending position */
		const uint8_t* dot_pos = nullptr; /* decimal point position */
		uint32_t frac_zeros = 0;
		uint64_t sig = 0; /* significant part of the number */
		int32_t exp = 0; /* exponent part of the number */
		bool exp_sign; /* temporary exponent sign from literal part */
		int32_t exp_sig = 0; /* temporary exponent number from significant part */
		int32_t exp_lit = 0; /* temporary exponent number from exponent literal part */
		uint64_t num_tmp; /* temporary number for reading */
		const uint8_t* tmp; /* temporary cursor for reading */
		const uint8_t* hdr = cur;
		bool sign;
		sign = (*hdr == '-');
		cur += sign;
		auto apply_sign = [&](auto&& val) -> OTy {
			if constexpr (std::is_unsigned_v<OTy>) {
				return static_cast<OTy>(val);
			} else {
				return sign ? -static_cast<OTy>(val) : static_cast<OTy>(val);
			}
		};
		/* begin with non-zero digit */
		sig = uint64_t(*cur - '0');
		if (sig > 9) {
			if constexpr (std::integral<OTy>) {
				return false;
			} else if (*cur == 'n' && cur[1] == 'u' && cur[2] == 'l' && cur[3] == 'l') {
				cur += 4;
				val = std::numeric_limits<OTy>::quiet_NaN();
				return true;
			} else if ((*cur | e_bit) == 'n' && (cur[1] | e_bit) == 'a' && (cur[2] | e_bit) == 'n') {
				cur += 3;
				val = sign ? -std::numeric_limits<OTy>::quiet_NaN() : std::numeric_limits<OTy>::quiet_NaN();
				return true;
			} else {
				return false;
			}
		}
		constexpr auto zero = static_cast<uint8_t>('0');
#define expr_intg(i) \
	if ((num_tmp = cur[i] - zero) <= 9) [[likely]] \
		sig = num_tmp + sig * 10; \
	else { \
		goto digi_sepr_##i; \
	}
		repeat_in_1_18(expr_intg);
#undef expr_intg
		cur += 19; /* Skip continuous 19 digits */
		if (!digiIsDigitOrFp(*cur)) {
			val = static_cast<OTy>(sig);
			if constexpr (!std::is_unsigned_v<OTy>) {
				val *= sign ? -1 : 1;
			}
			return true;
		}
		goto digi_intg_more; /* read more digits in integral part */
		/* process first non-digit character */
#define expr_sepr(i) \
	digi_sepr_##i : if ((!digiIsFp(cur[i]))) [[likely]] { \
		cur += i; \
		val = apply_sign(sig); \
		return true; \
	} \
	dot_pos = cur + i; \
	if ((cur[i] == '.')) [[likely]] { \
		if (sig == 0) \
			while (cur[frac_zeros + i + 1] == zero) \
				++frac_zeros; \
		goto digi_frac_##i; \
	} \
	cur += i; \
	sig_end = cur; \
	goto digi_exp_more;
		repeat_in_1_18(expr_sepr)
#undef expr_sepr
		/* read fraction part */
#define expr_frac(i) \
	digi_frac_##i : if (((num_tmp = static_cast<uint64_t>(cur[i + 1 + frac_zeros] - zero)) <= 9)) [[likely]] sig = num_tmp + sig * 10; \
	else { \
		goto digi_stop_##i; \
	}
			repeat_in_1_18(expr_frac)
#undef expr_frac
				cur += 20 + frac_zeros; /* Skip 19 digits and 1 decimal point */
		if (uint8_t(*cur - zero) > 9)
			goto digi_frac_end; /* fraction part end */
		goto digi_frac_more; /* read more digits in fraction part */
		/* significant part end */
#define expr_stop(i) \
	digi_stop_##i : cur += i + 1 + frac_zeros; \
	goto digi_frac_end;
		repeat_in_1_18(expr_stop)
#undef expr_stop
			/* read more digits in integral part */
			digi_intg_more : static constexpr uint64_t U64_MAX = (std::numeric_limits<uint64_t>::max)();// todo
		if ((num_tmp = *cur - zero) < 10) {
			if (!digiIsDigitOrFp(cur[1])) {
				/* this number is an integer consisting of 20 digits */
				if ((sig < (U64_MAX / 10)) || (sig == (U64_MAX / 10) && num_tmp <= (U64_MAX % 10))) {
					sig = num_tmp + sig * 10;
					cur++;
					val = static_cast<OTy>(sig);
					if constexpr (!std::is_unsigned_v<OTy>) {
						val *= sign ? -1 : 1;
					}
					return true;
				}
			}
		}
		if ((e_bit | *cur) == 'e') {
			dot_pos = cur;
			goto digi_exp_more;
		}
		if (*cur == '.') {
			dot_pos = cur++;
			if (uint8_t(*cur - zero) > 9) {
				return false;
			}
		}
		/* read more digits in fraction part */
	digi_frac_more:
		sig_cut = cur; /* too large to fit in u64, excess digits need to be cut */
		sig += (*cur >= '5'); /* round */
		while (uint8_t(*++cur - zero) < 10) {
		}
		if (!dot_pos) {
			dot_pos = cur;
			if (*cur == '.') {
				if (uint8_t(*++cur - zero) > 9) {
					return false;
				}
				while (uint8_t(*++cur - zero) < 10) {
				}
			}
		}
		exp_sig = static_cast<int32_t>(dot_pos - sig_cut);
		exp_sig += (dot_pos < sig_cut);
		/* ignore trailing zeros */
		tmp = cur - 1;
		while (*tmp == '0' || *tmp == '.')
			tmp--;
		if (tmp < sig_cut) {
			sig_cut = nullptr;
		} else {
			sig_end = cur;
		}
		if ((e_bit | *cur) == 'e')
			goto digi_exp_more;
		goto digi_exp_finish;
		/* fraction part end */
	digi_frac_end:
		sig_end = cur;
		exp_sig = -int32_t((cur - dot_pos) - 1);
		if ((e_bit | *cur) != 'e') [[likely]] {
			if ((exp_sig < F64_MIN_DEC_EXP - 19)) [[unlikely]] {
				val = apply_sign(0);
				return true;
			}
			exp = exp_sig;
			goto digi_finish;
		} else {
			goto digi_exp_more;
		}
		/* read exponent part */
	digi_exp_more:
		exp_sign = (*++cur == '-');
		cur += (*cur == '+' || *cur == '-');
		if (uint8_t(*cur - zero) > 9) [[unlikely]] {
			goto digi_finish;
		}
		while (*cur == '0')
			cur++;
		/* read exponent literal */
		tmp = cur;
		uint8_t c;
		while (uint8_t(c = *cur - zero) < 10) {
			++cur;
			exp_lit = c + uint32_t(exp_lit) * 10;
		}
		// large exponent case
		if ((cur - tmp >= 6)) [[unlikely]] {
			if (sig == 0 || exp_sign) {
				val = apply_sign(0);
				val = static_cast<OTy>(sig);
				return true;
			} else {
				val = apply_sign(std::numeric_limits<OTy>::infinity());
				return true;
			}
		}
		exp_sig += exp_sign ? -exp_lit : exp_lit;
		// validate exponent value
	digi_exp_finish:
		if constexpr (std::integral<OTy>) {
			if (sig == 0) {
				val = ((sign && !std::is_unsigned_v<OTy>) ? -0 : 0);
				return true;
			}
			if (exp_sig < -20) {
				val = apply_sign(0);
				return true;
			} else if (exp_sig > 20) {
				val = apply_sign(std::numeric_limits<OTy>::infinity());
				return true;
			}
			exp = exp_sig;
		} else {
			if (sig == 0) {
				val = (sign ? -OTy{ 0 } : OTy{ 0 });
				return true;
			}
			if ((exp_sig < F64_MIN_DEC_EXP - 19)) [[unlikely]] {
				val = (sign ? -OTy{ 0 } : OTy{ 0 });
				return true;
			} else if ((exp_sig > F64_MAX_DEC_EXP)) [[unlikely]] {
				val = sign ? -std::numeric_limits<OTy>::infinity() : std::numeric_limits<OTy>::infinity();
				return true;
			}
			exp = exp_sig;
		}
		/* all digit read finished */
	digi_finish:

		if constexpr (std::integral<OTy>) {
			val = static_cast<OTy>(sig);
			if constexpr (!std::is_unsigned_v<OTy>) {
				val *= sign ? -1 : 1;
			}
			if (exp >= 0) {
				val *= OTy(powersOfTenInt[exp]);
			} else {
				val /= OTy(powersOfTenInt[-exp]);
			}
			return true;
		} else {
			if constexpr (std::is_same_v<double, OTy>) {
				// numbers must be exactly representable in this fast path
				if (sig < (uint64_t(1) << 53) && std::abs(exp) <= 22) {
					val = static_cast<OTy>(sig);
					if constexpr (!std::is_unsigned_v<OTy>) {
						val *= sign ? -1 : 1;
					}
					if (exp >= 0) {
						val *= powersOfTenFloat[exp];
					} else {
						val /= powersOfTenFloat[-exp];
					}
					return true;
				}
			} else {
				if (sig < (uint64_t(1) << 24) && std::abs(exp) <= 8) {
					val = static_cast<OTy>(sig);
					if constexpr (!std::is_unsigned_v<OTy>) {
						val *= sign ? -1 : 1;
					}
					if (exp >= 0) {
						val *= static_cast<OTy>(powersOfTenFloat[exp]);
					} else {
						val /= static_cast<OTy>(powersOfTenFloat[-exp]);
					}
					return true;
				}
			}

			static_assert(std::numeric_limits<OTy>::is_iec559);
			static_assert(std::numeric_limits<OTy>::radix == 2);
			static_assert(std::is_same_v<float, std::decay_t<OTy>> || std::is_same_v<double, std::decay_t<OTy>>);
			static_assert(sizeof(float) == 4 && sizeof(double) == 8);

			using raw_t = std::conditional_t<std::is_same_v<float, std::decay_t<OTy>>, uint32_t, uint64_t>;
			const auto sig_leading_zeros = std::countl_zero(sig);
			const auto sig_norm = sig << sig_leading_zeros;
			const auto sig2_norm = sig2FromExp10(exp);
			const auto sig_product = mulhi64(sig_norm, sig2_norm) + 1;
			const auto sig_product_starts_with_1 = sig_product >> 63;
			auto mantisa = sig_product << (2 - sig_product_starts_with_1);
			constexpr uint64_t round_mask = uint64_t(1) << 63 >> (std::numeric_limits<OTy>::digits - 1);
			constexpr uint32_t exponent_bits = ceillog2(std::numeric_limits<OTy>::max_exponent - std::numeric_limits<OTy>::min_exponent + 1);
			constexpr uint32_t mantisa_shift = exponent_bits + 1 + 64 - 8 * sizeof(raw_t);
			int32_t exp2 = exp2FromExp10(exp) + static_cast<uint32_t>(-sig_leading_zeros + sig_product_starts_with_1);

			if (exp2 < std::numeric_limits<OTy>::min_exponent - 1) [[unlikely]] {
				// TODO handle subnormal numbers
				val = sign ? -OTy(0) : OTy(0);
				return true;
			} else if (exp2 > std::numeric_limits<OTy>::max_exponent - 1) [[unlikely]] {
				val = sign ? -std::numeric_limits<OTy>::infinity() : std::numeric_limits<OTy>::infinity();
				return true;
			}

			uint64_t round = 0;
			if (round_mask & mantisa) {
				if (mantisa << (std::numeric_limits<OTy>::digits) == 0) {
					// We added one to the product so this is the case were the trailing bits were 1.
					// This is a problem since the product could underestimate by a bit and uness there is a zero bit to fall
					// into we cant be sure if we need to round or not
					auto sig_upper = (mantisa >> (mantisa_shift - 1)) | (uint64_t(1) << 63 >> (mantisa_shift - 2)) | 1;
					int32_t exp2_upper = exp2 - std::numeric_limits<OTy>::digits;

					BigIntT big_comp{ sig_upper };
					BigIntT big_full{ sig };// Not dealing will ulp from sig_cut since we only care about roundtriping
						// machine doubles and only a human would use so many sigfigs
					if (exp >= 0) {
						big_full.mulPow10(exp);
					} else {
						big_comp.mulPow10(-exp);
					}
					if (exp2_upper >= 0) {
						big_comp.mulPow2(exp2_upper);
					} else {
						big_full.mulPow2(-exp2_upper);
					}
					auto cmp = big_full <=> big_comp;
					if (cmp != 0) [[likely]] {
						// round down or round up
						round = (cmp > 0);
					} else {
						// falls midway, round to even
						round = (mantisa & (round_mask << 1)) != 0;
					}
				} else if ((exp < pow10SigTableMinExact || exp > pow10SigTableMaxExact)// If there are ones after 64 bits in sig2 then there will be
					// ones after the rounding bit in the product
					|| (mantisa & (round_mask << 1))// Odd nums need to round up regardless of if the rest is nonzero or not
					|| (static_cast<size_t>(std::countr_zero(sig_norm) + std::countr_zero(sig2_norm)) <
						   128 - std::numeric_limits<OTy>::digits - (2 - sig_product_starts_with_1))// Check where the least significant one is
				) {
					round = 1;
				}
			}

			auto num = raw_t(sign) << (sizeof(raw_t) * 8 - 1) | raw_t(mantisa >> mantisa_shift) |
				(raw_t(exp2 + std::numeric_limits<OTy>::max_exponent - 1) << (std::numeric_limits<OTy>::digits - 1));
			num += raw_t(round);
			JsonifierCore::memcpy(&val, &num, sizeof(OTy));
			return true;
		}
	}

	inline constexpr char charTable[200] = { '0', '0', '0', '1', '0', '2', '0', '3', '0', '4', '0', '5', '0', '6', '0', '7', '0', '8', '0', '9', '1',
		'0', '1', '1', '1', '2', '1', '3', '1', '4', '1', '5', '1', '6', '1', '7', '1', '8', '1', '9', '2', '0', '2', '1', '2', '2', '2', '3', '2',
		'4', '2', '5', '2', '6', '2', '7', '2', '8', '2', '9', '3', '0', '3', '1', '3', '2', '3', '3', '3', '4', '3', '5', '3', '6', '3', '7', '3',
		'8', '3', '9', '4', '0', '4', '1', '4', '2', '4', '3', '4', '4', '4', '5', '4', '6', '4', '7', '4', '8', '4', '9', '5', '0', '5', '1', '5',
		'2', '5', '3', '5', '4', '5', '5', '5', '6', '5', '7', '5', '8', '5', '9', '6', '0', '6', '1', '6', '2', '6', '3', '6', '4', '6', '5', '6',
		'6', '6', '7', '6', '8', '6', '9', '7', '0', '7', '1', '7', '2', '7', '3', '7', '4', '7', '5', '7', '6', '7', '7', '7', '8', '7', '9', '8',
		'0', '8', '1', '8', '2', '8', '3', '8', '4', '8', '5', '8', '6', '8', '7', '8', '8', '8', '9', '9', '0', '9', '1', '9', '2', '9', '3', '9',
		'4', '9', '5', '9', '6', '9', '7', '9', '8', '9', '9' };

	/* This struct can help compiler generate 2-byte load/store */
	/* instructions on platforms that support unaligned access. */
	struct pair {
		char c1, c2;
	};

	template<class OTy>
	requires std::same_as<OTy, uint32_t>
	inline auto* toChars(auto* buf, OTy val) noexcept {
		/* The maximum value of uint32_t is 4294967295 (10 digits), */
		/* these digits are named as 'aabbccddee' here.             */
		uint32_t aa, bb, cc, dd, ee, aabb, bbcc, ccdd, ddee, aabbcc;

		/* Leading zero count in the first pair.                    */
		uint32_t lz;

		/* Although most compilers may convert the "division by     */
		/* constant value" into "multiply and shift", manual        */
		/* conversion can still help some compilers generate        */
		/* fewer and better instructions.                           */

		if (val < 100) { /* 1-2 digits: aa */
			lz = val < 10;
			JsonifierCore::memcpy(&buf[0], &charTable[val * 2 + lz], 2);
			buf -= lz;
			return buf + 2;

		} else if (val < 10000) { /* 3-4 digits: aabb */
			aa = (val * 5243) >> 19; /* (val / 100) */
			bb = val - aa * 100; /* (val % 100) */
			lz = aa < 10;
			JsonifierCore::memcpy(&buf[0], &charTable[aa * 2 + lz], 2);
			buf -= lz;
			JsonifierCore::memcpy(&buf[2], &charTable[2 * bb], 2);

			return buf + 4;

		} else if (val < 1000000) { /* 5-6 digits: aabbcc */
			aa = uint32_t((uint64_t(val) * 429497) >> 32); /* (val / 10000) */
			bbcc = val - aa * 10000; /* (val % 10000) */
			bb = (bbcc * 5243) >> 19; /* (bbcc / 100) */
			cc = bbcc - bb * 100; /* (bbcc % 100) */
			lz = aa < 10;
			JsonifierCore::memcpy(buf, charTable + aa * 2 + lz, 2);
			buf -= lz;
			JsonifierCore::memcpy(buf + 2, charTable + bb * 2, 2);
			JsonifierCore::memcpy(buf + 4, charTable + cc * 2, 2);
			return buf + 6;

		} else if (val < 100000000) { /* 7~8 digits: aabbccdd */
			/* (val / 10000) */
			aabb = uint32_t((uint64_t(val) * 109951163) >> 40);
			ccdd = val - aabb * 10000; /* (val % 10000) */
			aa = (aabb * 5243) >> 19; /* (aabb / 100) */
			cc = (ccdd * 5243) >> 19; /* (ccdd / 100) */
			bb = aabb - aa * 100; /* (aabb % 100) */
			dd = ccdd - cc * 100; /* (ccdd % 100) */
			lz = aa < 10;
			JsonifierCore::memcpy(buf, charTable + aa * 2 + lz, 2);
			buf -= lz;
			JsonifierCore::memcpy(buf + 2, charTable + bb * 2, 2);
			JsonifierCore::memcpy(buf + 4, charTable + cc * 2, 2);
			JsonifierCore::memcpy(buf + 6, charTable + dd * 2, 2);
			return buf + 8;

		} else { /* 9~10 digits: aabbccddee */
			/* (val / 10000) */
			aabbcc = uint32_t((uint64_t(val) * 3518437209ul) >> 45);
			/* (aabbcc / 10000) */
			aa = uint32_t((uint64_t(aabbcc) * 429497) >> 32);
			ddee = val - aabbcc * 10000; /* (val % 10000) */
			bbcc = aabbcc - aa * 10000; /* (aabbcc % 10000) */
			bb = (bbcc * 5243) >> 19; /* (bbcc / 100) */
			dd = (ddee * 5243) >> 19; /* (ddee / 100) */
			cc = bbcc - bb * 100; /* (bbcc % 100) */
			ee = ddee - dd * 100; /* (ddee % 100) */
			lz = aa < 10;
			JsonifierCore::memcpy(buf, charTable + aa * 2 + lz, 2);
			buf -= lz;
			JsonifierCore::memcpy(buf + 2, charTable + bb * 2, 2);
			JsonifierCore::memcpy(buf + 4, charTable + cc * 2, 2);
			JsonifierCore::memcpy(buf + 6, charTable + dd * 2, 2);
			JsonifierCore::memcpy(buf + 8, charTable + ee * 2, 2);
			return buf + 10;
		}
	}

	template<class OTy>
	requires std::same_as<OTy, int32_t>
	inline auto* toChars(auto* buf, OTy val) noexcept {
		uint32_t neg = uint32_t(-val);
		size_t sign = val < 0;
		*buf = '-';
		return toChars(buf + sign, sign ? uint32_t(neg) : uint32_t(val));
	}

	inline auto* toCharsU64Len8(auto* buf, uint32_t val) noexcept {
		/* 8 digits: aabbccdd */
		uint32_t aa, bb, cc, dd, aabb, ccdd;
		aabb = uint32_t((uint64_t(val) * 109951163) >> 40); /* (val / 10000) */
		ccdd = val - aabb * 10000; /* (val % 10000) */
		aa = (aabb * 5243) >> 19; /* (aabb / 100) */
		cc = (ccdd * 5243) >> 19; /* (ccdd / 100) */
		bb = aabb - aa * 100; /* (aabb % 100) */
		dd = ccdd - cc * 100; /* (ccdd % 100) */
		JsonifierCore::memcpy(buf, charTable + aa * 2, 2);
		JsonifierCore::memcpy(buf + 2, charTable + bb * 2, 2);
		JsonifierCore::memcpy(buf + 4, charTable + cc * 2, 2);
		JsonifierCore::memcpy(buf + 6, charTable + dd * 2, 2);
		return buf + 8;
	}

	inline auto* toCharsU64Len4(auto* buf, uint32_t val) noexcept {
		/* 4 digits: aabb */
		uint32_t aa, bb;
		aa = (val * 5243) >> 19; /* (val / 100) */
		bb = val - aa * 100; /* (val % 100) */
		JsonifierCore::memcpy(buf, charTable + aa * 2, 2);
		JsonifierCore::memcpy(buf + 2, charTable + bb * 2, 2);
		return buf + 4;
	}

	inline auto* toCharsU64Len18(auto* buf, uint32_t val) noexcept {
		uint32_t aa, bb, cc, dd, aabb, bbcc, ccdd, lz;

		if (val < 100) { /* 1-2 digits: aa */
			lz = val < 10;
			JsonifierCore::memcpy(buf, charTable + val * 2 + lz, 2);
			buf -= lz;
			return buf + 2;

		} else if (val < 10000) { /* 3-4 digits: aabb */
			aa = (val * 5243) >> 19; /* (val / 100) */
			bb = val - aa * 100; /* (val % 100) */
			lz = aa < 10;
			JsonifierCore::memcpy(buf, charTable + aa * 2 + lz, 2);
			buf -= lz;
			JsonifierCore::memcpy(buf + 2, charTable + bb * 2, 2);
			return buf + 4;

		} else if (val < 1000000) { /* 5-6 digits: aabbcc */
			aa = uint32_t((uint64_t(val) * 429497) >> 32); /* (val / 10000) */
			bbcc = val - aa * 10000; /* (val % 10000) */
			bb = (bbcc * 5243) >> 19; /* (bbcc / 100) */
			cc = bbcc - bb * 100; /* (bbcc % 100) */
			lz = aa < 10;
			JsonifierCore::memcpy(buf, charTable + aa * 2 + lz, 2);
			buf -= lz;
			JsonifierCore::memcpy(buf + 2, charTable + bb * 2, 2);
			JsonifierCore::memcpy(buf + 4, charTable + cc * 2, 2);
			return buf + 6;

		} else { /* 7-8 digits: aabbccdd */
			/* (val / 10000) */
			aabb = uint32_t((uint64_t(val) * 109951163) >> 40);
			ccdd = val - aabb * 10000; /* (val % 10000) */
			aa = (aabb * 5243) >> 19; /* (aabb / 100) */
			cc = (ccdd * 5243) >> 19; /* (ccdd / 100) */
			bb = aabb - aa * 100; /* (aabb % 100) */
			dd = ccdd - cc * 100; /* (ccdd % 100) */
			lz = aa < 10;
			JsonifierCore::memcpy(buf, charTable + aa * 2 + lz, 2);
			buf -= lz;
			JsonifierCore::memcpy(buf + 2, charTable + bb * 2, 2);
			JsonifierCore::memcpy(buf + 4, charTable + cc * 2, 2);
			JsonifierCore::memcpy(buf + 6, charTable + dd * 2, 2);
			return buf + 8;
		}
	}

	inline auto* toCharsU64Len58(auto* buf, uint32_t val) noexcept {
		uint32_t aa, bb, cc, dd, aabb, bbcc, ccdd, lz;

		if (val < 1000000) { /* 5-6 digits: aabbcc */
			aa = uint32_t((uint64_t(val) * 429497) >> 32); /* (val / 10000) */
			bbcc = val - aa * 10000; /* (val % 10000) */
			bb = (bbcc * 5243) >> 19; /* (bbcc / 100) */
			cc = bbcc - bb * 100; /* (bbcc % 100) */
			lz = aa < 10;
			JsonifierCore::memcpy(buf, charTable + aa * 2 + lz, 2);
			buf -= lz;
			JsonifierCore::memcpy(buf + 2, charTable + bb * 2, 2);
			JsonifierCore::memcpy(buf + 4, charTable + cc * 2, 2);
			return buf + 6;

		} else { /* 7-8 digits: aabbccdd */
			/* (val / 10000) */
			aabb = uint32_t((uint64_t(val) * 109951163) >> 40);
			ccdd = val - aabb * 10000; /* (val % 10000) */
			aa = (aabb * 5243) >> 19; /* (aabb / 100) */
			cc = (ccdd * 5243) >> 19; /* (ccdd / 100) */
			bb = aabb - aa * 100; /* (aabb % 100) */
			dd = ccdd - cc * 100; /* (ccdd % 100) */
			lz = aa < 10;
			JsonifierCore::memcpy(buf, charTable + aa * 2 + lz, 2);
			buf -= lz;
			JsonifierCore::memcpy(buf + 2, charTable + bb * 2, 2);
			JsonifierCore::memcpy(buf + 4, charTable + cc * 2, 2);
			JsonifierCore::memcpy(buf + 6, charTable + dd * 2, 2);
			return buf + 8;
		}
	}

	template<class OTy>
	requires std::same_as<OTy, uint64_t>
	inline auto* toChars(auto* buf, OTy val) noexcept {
		uint64_t tmp, hgh;
		uint32_t mid, low;

		if (val < 100000000) { /* 1-8 digits */
			buf = toCharsU64Len18(buf, uint32_t(val));
			return buf;

		} else if (val < 100000000ull * 100000000ull) { /* 9-16 digits */
			hgh = val / 100000000;
			low = uint32_t(val - hgh * 100000000); /* (val % 100000000) */
			buf = toCharsU64Len18(buf, uint32_t(hgh));
			buf = toCharsU64Len8(buf, low);
			return buf;

		} else { /* 17-20 digits */
			tmp = val / 100000000;
			low = uint32_t(val - tmp * 100000000); /* (val % 100000000) */
			hgh = uint32_t(tmp / 10000);
			mid = uint32_t(tmp - hgh * 10000); /* (tmp % 10000) */
			buf = toCharsU64Len58(buf, uint32_t(hgh));
			buf = toCharsU64Len4(buf, mid);
			buf = toCharsU64Len8(buf, low);
			return buf;
		}
	}

	template<class OTy>
	requires std::same_as<OTy, int64_t>
	inline auto* toChars(auto* buf, OTy val) noexcept {
		uint64_t neg = uint64_t(-val);
		size_t sign = val < 0;
		*buf = '-';
		return toChars(buf + sign, sign ? uint64_t(neg) : uint64_t(val));
	}

	//Source: https://github.com/ibireme/yyjson/blob/master/src/yyjson.c

	/*==============================================================================
    * Power10 Lookup Table
    *============================================================================*/

	/** Minimum decimal exponent in pow10SigTable. */
	inline constexpr auto POW10_SIG_TABLE_128_MIN_EXP = -343;

	/** Maximum decimal exponent in pow10SigTable. */
	inline constexpr auto POW10_SIG_TABLE_128_MAX_EXP = 324;

	/** Minimum exact decimal exponent in pow10SigTable */
	inline constexpr auto POW10_SIG_TABLE_128_MIN_EXACT_EXP = 0;

	/** Maximum exact decimal exponent in pow10SigTable */
	inline constexpr auto POW10_SIG_TABLE_128_MAX_EXACT_EXP = 55;

	// TODO: Remove duplicate table
	inline constexpr std::array<uint64_t, 1336> pow10_sig_table_128 = {
		0xBF29DCABA82FDEAE, 0x7432EE873880FC33, /* ~= 10^-343 */
		0xEEF453D6923BD65A, 0x113FAA2906A13B3F, /* ~= 10^-342 */
		0x9558B4661B6565F8, 0x4AC7CA59A424C507, /* ~= 10^-341 */
		0xBAAEE17FA23EBF76, 0x5D79BCF00D2DF649, /* ~= 10^-340 */
		0xE95A99DF8ACE6F53, 0xF4D82C2C107973DC, /* ~= 10^-339 */
		0x91D8A02BB6C10594, 0x79071B9B8A4BE869, /* ~= 10^-338 */
		0xB64EC836A47146F9, 0x9748E2826CDEE284, /* ~= 10^-337 */
		0xE3E27A444D8D98B7, 0xFD1B1B2308169B25, /* ~= 10^-336 */
		0x8E6D8C6AB0787F72, 0xFE30F0F5E50E20F7, /* ~= 10^-335 */
		0xB208EF855C969F4F, 0xBDBD2D335E51A935, /* ~= 10^-334 */
		0xDE8B2B66B3BC4723, 0xAD2C788035E61382, /* ~= 10^-333 */
		0x8B16FB203055AC76, 0x4C3BCB5021AFCC31, /* ~= 10^-332 */
		0xADDCB9E83C6B1793, 0xDF4ABE242A1BBF3D, /* ~= 10^-331 */
		0xD953E8624B85DD78, 0xD71D6DAD34A2AF0D, /* ~= 10^-330 */
		0x87D4713D6F33AA6B, 0x8672648C40E5AD68, /* ~= 10^-329 */
		0xA9C98D8CCB009506, 0x680EFDAF511F18C2, /* ~= 10^-328 */
		0xD43BF0EFFDC0BA48, 0x0212BD1B2566DEF2, /* ~= 10^-327 */
		0x84A57695FE98746D, 0x014BB630F7604B57, /* ~= 10^-326 */
		0xA5CED43B7E3E9188, 0x419EA3BD35385E2D, /* ~= 10^-325 */
		0xCF42894A5DCE35EA, 0x52064CAC828675B9, /* ~= 10^-324 */
		0x818995CE7AA0E1B2, 0x7343EFEBD1940993, /* ~= 10^-323 */
		0xA1EBFB4219491A1F, 0x1014EBE6C5F90BF8, /* ~= 10^-322 */
		0xCA66FA129F9B60A6, 0xD41A26E077774EF6, /* ~= 10^-321 */
		0xFD00B897478238D0, 0x8920B098955522B4, /* ~= 10^-320 */
		0x9E20735E8CB16382, 0x55B46E5F5D5535B0, /* ~= 10^-319 */
		0xC5A890362FDDBC62, 0xEB2189F734AA831D, /* ~= 10^-318 */
		0xF712B443BBD52B7B, 0xA5E9EC7501D523E4, /* ~= 10^-317 */
		0x9A6BB0AA55653B2D, 0x47B233C92125366E, /* ~= 10^-316 */
		0xC1069CD4EABE89F8, 0x999EC0BB696E840A, /* ~= 10^-315 */
		0xF148440A256E2C76, 0xC00670EA43CA250D, /* ~= 10^-314 */
		0x96CD2A865764DBCA, 0x380406926A5E5728, /* ~= 10^-313 */
		0xBC807527ED3E12BC, 0xC605083704F5ECF2, /* ~= 10^-312 */
		0xEBA09271E88D976B, 0xF7864A44C633682E, /* ~= 10^-311 */
		0x93445B8731587EA3, 0x7AB3EE6AFBE0211D, /* ~= 10^-310 */
		0xB8157268FDAE9E4C, 0x5960EA05BAD82964, /* ~= 10^-309 */
		0xE61ACF033D1A45DF, 0x6FB92487298E33BD, /* ~= 10^-308 */
		0x8FD0C16206306BAB, 0xA5D3B6D479F8E056, /* ~= 10^-307 */
		0xB3C4F1BA87BC8696, 0x8F48A4899877186C, /* ~= 10^-306 */
		0xE0B62E2929ABA83C, 0x331ACDABFE94DE87, /* ~= 10^-305 */
		0x8C71DCD9BA0B4925, 0x9FF0C08B7F1D0B14, /* ~= 10^-304 */
		0xAF8E5410288E1B6F, 0x07ECF0AE5EE44DD9, /* ~= 10^-303 */
		0xDB71E91432B1A24A, 0xC9E82CD9F69D6150, /* ~= 10^-302 */
		0x892731AC9FAF056E, 0xBE311C083A225CD2, /* ~= 10^-301 */
		0xAB70FE17C79AC6CA, 0x6DBD630A48AAF406, /* ~= 10^-300 */
		0xD64D3D9DB981787D, 0x092CBBCCDAD5B108, /* ~= 10^-299 */
		0x85F0468293F0EB4E, 0x25BBF56008C58EA5, /* ~= 10^-298 */
		0xA76C582338ED2621, 0xAF2AF2B80AF6F24E, /* ~= 10^-297 */
		0xD1476E2C07286FAA, 0x1AF5AF660DB4AEE1, /* ~= 10^-296 */
		0x82CCA4DB847945CA, 0x50D98D9FC890ED4D, /* ~= 10^-295 */
		0xA37FCE126597973C, 0xE50FF107BAB528A0, /* ~= 10^-294 */
		0xCC5FC196FEFD7D0C, 0x1E53ED49A96272C8, /* ~= 10^-293 */
		0xFF77B1FCBEBCDC4F, 0x25E8E89C13BB0F7A, /* ~= 10^-292 */
		0x9FAACF3DF73609B1, 0x77B191618C54E9AC, /* ~= 10^-291 */
		0xC795830D75038C1D, 0xD59DF5B9EF6A2417, /* ~= 10^-290 */
		0xF97AE3D0D2446F25, 0x4B0573286B44AD1D, /* ~= 10^-289 */
		0x9BECCE62836AC577, 0x4EE367F9430AEC32, /* ~= 10^-288 */
		0xC2E801FB244576D5, 0x229C41F793CDA73F, /* ~= 10^-287 */
		0xF3A20279ED56D48A, 0x6B43527578C1110F, /* ~= 10^-286 */
		0x9845418C345644D6, 0x830A13896B78AAA9, /* ~= 10^-285 */
		0xBE5691EF416BD60C, 0x23CC986BC656D553, /* ~= 10^-284 */
		0xEDEC366B11C6CB8F, 0x2CBFBE86B7EC8AA8, /* ~= 10^-283 */
		0x94B3A202EB1C3F39, 0x7BF7D71432F3D6A9, /* ~= 10^-282 */
		0xB9E08A83A5E34F07, 0xDAF5CCD93FB0CC53, /* ~= 10^-281 */
		0xE858AD248F5C22C9, 0xD1B3400F8F9CFF68, /* ~= 10^-280 */
		0x91376C36D99995BE, 0x23100809B9C21FA1, /* ~= 10^-279 */
		0xB58547448FFFFB2D, 0xABD40A0C2832A78A, /* ~= 10^-278 */
		0xE2E69915B3FFF9F9, 0x16C90C8F323F516C, /* ~= 10^-277 */
		0x8DD01FAD907FFC3B, 0xAE3DA7D97F6792E3, /* ~= 10^-276 */
		0xB1442798F49FFB4A, 0x99CD11CFDF41779C, /* ~= 10^-275 */
		0xDD95317F31C7FA1D, 0x40405643D711D583, /* ~= 10^-274 */
		0x8A7D3EEF7F1CFC52, 0x482835EA666B2572, /* ~= 10^-273 */
		0xAD1C8EAB5EE43B66, 0xDA3243650005EECF, /* ~= 10^-272 */
		0xD863B256369D4A40, 0x90BED43E40076A82, /* ~= 10^-271 */
		0x873E4F75E2224E68, 0x5A7744A6E804A291, /* ~= 10^-270 */
		0xA90DE3535AAAE202, 0x711515D0A205CB36, /* ~= 10^-269 */
		0xD3515C2831559A83, 0x0D5A5B44CA873E03, /* ~= 10^-268 */
		0x8412D9991ED58091, 0xE858790AFE9486C2, /* ~= 10^-267 */
		0xA5178FFF668AE0B6, 0x626E974DBE39A872, /* ~= 10^-266 */
		0xCE5D73FF402D98E3, 0xFB0A3D212DC8128F, /* ~= 10^-265 */
		0x80FA687F881C7F8E, 0x7CE66634BC9D0B99, /* ~= 10^-264 */
		0xA139029F6A239F72, 0x1C1FFFC1EBC44E80, /* ~= 10^-263 */
		0xC987434744AC874E, 0xA327FFB266B56220, /* ~= 10^-262 */
		0xFBE9141915D7A922, 0x4BF1FF9F0062BAA8, /* ~= 10^-261 */
		0x9D71AC8FADA6C9B5, 0x6F773FC3603DB4A9, /* ~= 10^-260 */
		0xC4CE17B399107C22, 0xCB550FB4384D21D3, /* ~= 10^-259 */
		0xF6019DA07F549B2B, 0x7E2A53A146606A48, /* ~= 10^-258 */
		0x99C102844F94E0FB, 0x2EDA7444CBFC426D, /* ~= 10^-257 */
		0xC0314325637A1939, 0xFA911155FEFB5308, /* ~= 10^-256 */
		0xF03D93EEBC589F88, 0x793555AB7EBA27CA, /* ~= 10^-255 */
		0x96267C7535B763B5, 0x4BC1558B2F3458DE, /* ~= 10^-254 */
		0xBBB01B9283253CA2, 0x9EB1AAEDFB016F16, /* ~= 10^-253 */
		0xEA9C227723EE8BCB, 0x465E15A979C1CADC, /* ~= 10^-252 */
		0x92A1958A7675175F, 0x0BFACD89EC191EC9, /* ~= 10^-251 */
		0xB749FAED14125D36, 0xCEF980EC671F667B, /* ~= 10^-250 */
		0xE51C79A85916F484, 0x82B7E12780E7401A, /* ~= 10^-249 */
		0x8F31CC0937AE58D2, 0xD1B2ECB8B0908810, /* ~= 10^-248 */
		0xB2FE3F0B8599EF07, 0x861FA7E6DCB4AA15, /* ~= 10^-247 */
		0xDFBDCECE67006AC9, 0x67A791E093E1D49A, /* ~= 10^-246 */
		0x8BD6A141006042BD, 0xE0C8BB2C5C6D24E0, /* ~= 10^-245 */
		0xAECC49914078536D, 0x58FAE9F773886E18, /* ~= 10^-244 */
		0xDA7F5BF590966848, 0xAF39A475506A899E, /* ~= 10^-243 */
		0x888F99797A5E012D, 0x6D8406C952429603, /* ~= 10^-242 */
		0xAAB37FD7D8F58178, 0xC8E5087BA6D33B83, /* ~= 10^-241 */
		0xD5605FCDCF32E1D6, 0xFB1E4A9A90880A64, /* ~= 10^-240 */
		0x855C3BE0A17FCD26, 0x5CF2EEA09A55067F, /* ~= 10^-239 */
		0xA6B34AD8C9DFC06F, 0xF42FAA48C0EA481E, /* ~= 10^-238 */
		0xD0601D8EFC57B08B, 0xF13B94DAF124DA26, /* ~= 10^-237 */
		0x823C12795DB6CE57, 0x76C53D08D6B70858, /* ~= 10^-236 */
		0xA2CB1717B52481ED, 0x54768C4B0C64CA6E, /* ~= 10^-235 */
		0xCB7DDCDDA26DA268, 0xA9942F5DCF7DFD09, /* ~= 10^-234 */
		0xFE5D54150B090B02, 0xD3F93B35435D7C4C, /* ~= 10^-233 */
		0x9EFA548D26E5A6E1, 0xC47BC5014A1A6DAF, /* ~= 10^-232 */
		0xC6B8E9B0709F109A, 0x359AB6419CA1091B, /* ~= 10^-231 */
		0xF867241C8CC6D4C0, 0xC30163D203C94B62, /* ~= 10^-230 */
		0x9B407691D7FC44F8, 0x79E0DE63425DCF1D, /* ~= 10^-229 */
		0xC21094364DFB5636, 0x985915FC12F542E4, /* ~= 10^-228 */
		0xF294B943E17A2BC4, 0x3E6F5B7B17B2939D, /* ~= 10^-227 */
		0x979CF3CA6CEC5B5A, 0xA705992CEECF9C42, /* ~= 10^-226 */
		0xBD8430BD08277231, 0x50C6FF782A838353, /* ~= 10^-225 */
		0xECE53CEC4A314EBD, 0xA4F8BF5635246428, /* ~= 10^-224 */
		0x940F4613AE5ED136, 0x871B7795E136BE99, /* ~= 10^-223 */
		0xB913179899F68584, 0x28E2557B59846E3F, /* ~= 10^-222 */
		0xE757DD7EC07426E5, 0x331AEADA2FE589CF, /* ~= 10^-221 */
		0x9096EA6F3848984F, 0x3FF0D2C85DEF7621, /* ~= 10^-220 */
		0xB4BCA50B065ABE63, 0x0FED077A756B53A9, /* ~= 10^-219 */
		0xE1EBCE4DC7F16DFB, 0xD3E8495912C62894, /* ~= 10^-218 */
		0x8D3360F09CF6E4BD, 0x64712DD7ABBBD95C, /* ~= 10^-217 */
		0xB080392CC4349DEC, 0xBD8D794D96AACFB3, /* ~= 10^-216 */
		0xDCA04777F541C567, 0xECF0D7A0FC5583A0, /* ~= 10^-215 */
		0x89E42CAAF9491B60, 0xF41686C49DB57244, /* ~= 10^-214 */
		0xAC5D37D5B79B6239, 0x311C2875C522CED5, /* ~= 10^-213 */
		0xD77485CB25823AC7, 0x7D633293366B828B, /* ~= 10^-212 */
		0x86A8D39EF77164BC, 0xAE5DFF9C02033197, /* ~= 10^-211 */
		0xA8530886B54DBDEB, 0xD9F57F830283FDFC, /* ~= 10^-210 */
		0xD267CAA862A12D66, 0xD072DF63C324FD7B, /* ~= 10^-209 */
		0x8380DEA93DA4BC60, 0x4247CB9E59F71E6D, /* ~= 10^-208 */
		0xA46116538D0DEB78, 0x52D9BE85F074E608, /* ~= 10^-207 */
		0xCD795BE870516656, 0x67902E276C921F8B, /* ~= 10^-206 */
		0x806BD9714632DFF6, 0x00BA1CD8A3DB53B6, /* ~= 10^-205 */
		0xA086CFCD97BF97F3, 0x80E8A40ECCD228A4, /* ~= 10^-204 */
		0xC8A883C0FDAF7DF0, 0x6122CD128006B2CD, /* ~= 10^-203 */
		0xFAD2A4B13D1B5D6C, 0x796B805720085F81, /* ~= 10^-202 */
		0x9CC3A6EEC6311A63, 0xCBE3303674053BB0, /* ~= 10^-201 */
		0xC3F490AA77BD60FC, 0xBEDBFC4411068A9C, /* ~= 10^-200 */
		0xF4F1B4D515ACB93B, 0xEE92FB5515482D44, /* ~= 10^-199 */
		0x991711052D8BF3C5, 0x751BDD152D4D1C4A, /* ~= 10^-198 */
		0xBF5CD54678EEF0B6, 0xD262D45A78A0635D, /* ~= 10^-197 */
		0xEF340A98172AACE4, 0x86FB897116C87C34, /* ~= 10^-196 */
		0x9580869F0E7AAC0E, 0xD45D35E6AE3D4DA0, /* ~= 10^-195 */
		0xBAE0A846D2195712, 0x8974836059CCA109, /* ~= 10^-194 */
		0xE998D258869FACD7, 0x2BD1A438703FC94B, /* ~= 10^-193 */
		0x91FF83775423CC06, 0x7B6306A34627DDCF, /* ~= 10^-192 */
		0xB67F6455292CBF08, 0x1A3BC84C17B1D542, /* ~= 10^-191 */
		0xE41F3D6A7377EECA, 0x20CABA5F1D9E4A93, /* ~= 10^-190 */
		0x8E938662882AF53E, 0x547EB47B7282EE9C, /* ~= 10^-189 */
		0xB23867FB2A35B28D, 0xE99E619A4F23AA43, /* ~= 10^-188 */
		0xDEC681F9F4C31F31, 0x6405FA00E2EC94D4, /* ~= 10^-187 */
		0x8B3C113C38F9F37E, 0xDE83BC408DD3DD04, /* ~= 10^-186 */
		0xAE0B158B4738705E, 0x9624AB50B148D445, /* ~= 10^-185 */
		0xD98DDAEE19068C76, 0x3BADD624DD9B0957, /* ~= 10^-184 */
		0x87F8A8D4CFA417C9, 0xE54CA5D70A80E5D6, /* ~= 10^-183 */
		0xA9F6D30A038D1DBC, 0x5E9FCF4CCD211F4C, /* ~= 10^-182 */
		0xD47487CC8470652B, 0x7647C3200069671F, /* ~= 10^-181 */
		0x84C8D4DFD2C63F3B, 0x29ECD9F40041E073, /* ~= 10^-180 */
		0xA5FB0A17C777CF09, 0xF468107100525890, /* ~= 10^-179 */
		0xCF79CC9DB955C2CC, 0x7182148D4066EEB4, /* ~= 10^-178 */
		0x81AC1FE293D599BF, 0xC6F14CD848405530, /* ~= 10^-177 */
		0xA21727DB38CB002F, 0xB8ADA00E5A506A7C, /* ~= 10^-176 */
		0xCA9CF1D206FDC03B, 0xA6D90811F0E4851C, /* ~= 10^-175 */
		0xFD442E4688BD304A, 0x908F4A166D1DA663, /* ~= 10^-174 */
		0x9E4A9CEC15763E2E, 0x9A598E4E043287FE, /* ~= 10^-173 */
		0xC5DD44271AD3CDBA, 0x40EFF1E1853F29FD, /* ~= 10^-172 */
		0xF7549530E188C128, 0xD12BEE59E68EF47C, /* ~= 10^-171 */
		0x9A94DD3E8CF578B9, 0x82BB74F8301958CE, /* ~= 10^-170 */
		0xC13A148E3032D6E7, 0xE36A52363C1FAF01, /* ~= 10^-169 */
		0xF18899B1BC3F8CA1, 0xDC44E6C3CB279AC1, /* ~= 10^-168 */
		0x96F5600F15A7B7E5, 0x29AB103A5EF8C0B9, /* ~= 10^-167 */
		0xBCB2B812DB11A5DE, 0x7415D448F6B6F0E7, /* ~= 10^-166 */
		0xEBDF661791D60F56, 0x111B495B3464AD21, /* ~= 10^-165 */
		0x936B9FCEBB25C995, 0xCAB10DD900BEEC34, /* ~= 10^-164 */
		0xB84687C269EF3BFB, 0x3D5D514F40EEA742, /* ~= 10^-163 */
		0xE65829B3046B0AFA, 0x0CB4A5A3112A5112, /* ~= 10^-162 */
		0x8FF71A0FE2C2E6DC, 0x47F0E785EABA72AB, /* ~= 10^-161 */
		0xB3F4E093DB73A093, 0x59ED216765690F56, /* ~= 10^-160 */
		0xE0F218B8D25088B8, 0x306869C13EC3532C, /* ~= 10^-159 */
		0x8C974F7383725573, 0x1E414218C73A13FB, /* ~= 10^-158 */
		0xAFBD2350644EEACF, 0xE5D1929EF90898FA, /* ~= 10^-157 */
		0xDBAC6C247D62A583, 0xDF45F746B74ABF39, /* ~= 10^-156 */
		0x894BC396CE5DA772, 0x6B8BBA8C328EB783, /* ~= 10^-155 */
		0xAB9EB47C81F5114F, 0x066EA92F3F326564, /* ~= 10^-154 */
		0xD686619BA27255A2, 0xC80A537B0EFEFEBD, /* ~= 10^-153 */
		0x8613FD0145877585, 0xBD06742CE95F5F36, /* ~= 10^-152 */
		0xA798FC4196E952E7, 0x2C48113823B73704, /* ~= 10^-151 */
		0xD17F3B51FCA3A7A0, 0xF75A15862CA504C5, /* ~= 10^-150 */
		0x82EF85133DE648C4, 0x9A984D73DBE722FB, /* ~= 10^-149 */
		0xA3AB66580D5FDAF5, 0xC13E60D0D2E0EBBA, /* ~= 10^-148 */
		0xCC963FEE10B7D1B3, 0x318DF905079926A8, /* ~= 10^-147 */
		0xFFBBCFE994E5C61F, 0xFDF17746497F7052, /* ~= 10^-146 */
		0x9FD561F1FD0F9BD3, 0xFEB6EA8BEDEFA633, /* ~= 10^-145 */
		0xC7CABA6E7C5382C8, 0xFE64A52EE96B8FC0, /* ~= 10^-144 */
		0xF9BD690A1B68637B, 0x3DFDCE7AA3C673B0, /* ~= 10^-143 */
		0x9C1661A651213E2D, 0x06BEA10CA65C084E, /* ~= 10^-142 */
		0xC31BFA0FE5698DB8, 0x486E494FCFF30A62, /* ~= 10^-141 */
		0xF3E2F893DEC3F126, 0x5A89DBA3C3EFCCFA, /* ~= 10^-140 */
		0x986DDB5C6B3A76B7, 0xF89629465A75E01C, /* ~= 10^-139 */
		0xBE89523386091465, 0xF6BBB397F1135823, /* ~= 10^-138 */
		0xEE2BA6C0678B597F, 0x746AA07DED582E2C, /* ~= 10^-137 */
		0x94DB483840B717EF, 0xA8C2A44EB4571CDC, /* ~= 10^-136 */
		0xBA121A4650E4DDEB, 0x92F34D62616CE413, /* ~= 10^-135 */
		0xE896A0D7E51E1566, 0x77B020BAF9C81D17, /* ~= 10^-134 */
		0x915E2486EF32CD60, 0x0ACE1474DC1D122E, /* ~= 10^-133 */
		0xB5B5ADA8AAFF80B8, 0x0D819992132456BA, /* ~= 10^-132 */
		0xE3231912D5BF60E6, 0x10E1FFF697ED6C69, /* ~= 10^-131 */
		0x8DF5EFABC5979C8F, 0xCA8D3FFA1EF463C1, /* ~= 10^-130 */
		0xB1736B96B6FD83B3, 0xBD308FF8A6B17CB2, /* ~= 10^-129 */
		0xDDD0467C64BCE4A0, 0xAC7CB3F6D05DDBDE, /* ~= 10^-128 */
		0x8AA22C0DBEF60EE4, 0x6BCDF07A423AA96B, /* ~= 10^-127 */
		0xAD4AB7112EB3929D, 0x86C16C98D2C953C6, /* ~= 10^-126 */
		0xD89D64D57A607744, 0xE871C7BF077BA8B7, /* ~= 10^-125 */
		0x87625F056C7C4A8B, 0x11471CD764AD4972, /* ~= 10^-124 */
		0xA93AF6C6C79B5D2D, 0xD598E40D3DD89BCF, /* ~= 10^-123 */
		0xD389B47879823479, 0x4AFF1D108D4EC2C3, /* ~= 10^-122 */
		0x843610CB4BF160CB, 0xCEDF722A585139BA, /* ~= 10^-121 */
		0xA54394FE1EEDB8FE, 0xC2974EB4EE658828, /* ~= 10^-120 */
		0xCE947A3DA6A9273E, 0x733D226229FEEA32, /* ~= 10^-119 */
		0x811CCC668829B887, 0x0806357D5A3F525F, /* ~= 10^-118 */
		0xA163FF802A3426A8, 0xCA07C2DCB0CF26F7, /* ~= 10^-117 */
		0xC9BCFF6034C13052, 0xFC89B393DD02F0B5, /* ~= 10^-116 */
		0xFC2C3F3841F17C67, 0xBBAC2078D443ACE2, /* ~= 10^-115 */
		0x9D9BA7832936EDC0, 0xD54B944B84AA4C0D, /* ~= 10^-114 */
		0xC5029163F384A931, 0x0A9E795E65D4DF11, /* ~= 10^-113 */
		0xF64335BCF065D37D, 0x4D4617B5FF4A16D5, /* ~= 10^-112 */
		0x99EA0196163FA42E, 0x504BCED1BF8E4E45, /* ~= 10^-111 */
		0xC06481FB9BCF8D39, 0xE45EC2862F71E1D6, /* ~= 10^-110 */
		0xF07DA27A82C37088, 0x5D767327BB4E5A4C, /* ~= 10^-109 */
		0x964E858C91BA2655, 0x3A6A07F8D510F86F, /* ~= 10^-108 */
		0xBBE226EFB628AFEA, 0x890489F70A55368B, /* ~= 10^-107 */
		0xEADAB0ABA3B2DBE5, 0x2B45AC74CCEA842E, /* ~= 10^-106 */
		0x92C8AE6B464FC96F, 0x3B0B8BC90012929D, /* ~= 10^-105 */
		0xB77ADA0617E3BBCB, 0x09CE6EBB40173744, /* ~= 10^-104 */
		0xE55990879DDCAABD, 0xCC420A6A101D0515, /* ~= 10^-103 */
		0x8F57FA54C2A9EAB6, 0x9FA946824A12232D, /* ~= 10^-102 */
		0xB32DF8E9F3546564, 0x47939822DC96ABF9, /* ~= 10^-101 */
		0xDFF9772470297EBD, 0x59787E2B93BC56F7, /* ~= 10^-100 */
		0x8BFBEA76C619EF36, 0x57EB4EDB3C55B65A, /* ~= 10^-99 */
		0xAEFAE51477A06B03, 0xEDE622920B6B23F1, /* ~= 10^-98 */
		0xDAB99E59958885C4, 0xE95FAB368E45ECED, /* ~= 10^-97 */
		0x88B402F7FD75539B, 0x11DBCB0218EBB414, /* ~= 10^-96 */
		0xAAE103B5FCD2A881, 0xD652BDC29F26A119, /* ~= 10^-95 */
		0xD59944A37C0752A2, 0x4BE76D3346F0495F, /* ~= 10^-94 */
		0x857FCAE62D8493A5, 0x6F70A4400C562DDB, /* ~= 10^-93 */
		0xA6DFBD9FB8E5B88E, 0xCB4CCD500F6BB952, /* ~= 10^-92 */
		0xD097AD07A71F26B2, 0x7E2000A41346A7A7, /* ~= 10^-91 */
		0x825ECC24C873782F, 0x8ED400668C0C28C8, /* ~= 10^-90 */
		0xA2F67F2DFA90563B, 0x728900802F0F32FA, /* ~= 10^-89 */
		0xCBB41EF979346BCA, 0x4F2B40A03AD2FFB9, /* ~= 10^-88 */
		0xFEA126B7D78186BC, 0xE2F610C84987BFA8, /* ~= 10^-87 */
		0x9F24B832E6B0F436, 0x0DD9CA7D2DF4D7C9, /* ~= 10^-86 */
		0xC6EDE63FA05D3143, 0x91503D1C79720DBB, /* ~= 10^-85 */
		0xF8A95FCF88747D94, 0x75A44C6397CE912A, /* ~= 10^-84 */
		0x9B69DBE1B548CE7C, 0xC986AFBE3EE11ABA, /* ~= 10^-83 */
		0xC24452DA229B021B, 0xFBE85BADCE996168, /* ~= 10^-82 */
		0xF2D56790AB41C2A2, 0xFAE27299423FB9C3, /* ~= 10^-81 */
		0x97C560BA6B0919A5, 0xDCCD879FC967D41A, /* ~= 10^-80 */
		0xBDB6B8E905CB600F, 0x5400E987BBC1C920, /* ~= 10^-79 */
		0xED246723473E3813, 0x290123E9AAB23B68, /* ~= 10^-78 */
		0x9436C0760C86E30B, 0xF9A0B6720AAF6521, /* ~= 10^-77 */
		0xB94470938FA89BCE, 0xF808E40E8D5B3E69, /* ~= 10^-76 */
		0xE7958CB87392C2C2, 0xB60B1D1230B20E04, /* ~= 10^-75 */
		0x90BD77F3483BB9B9, 0xB1C6F22B5E6F48C2, /* ~= 10^-74 */
		0xB4ECD5F01A4AA828, 0x1E38AEB6360B1AF3, /* ~= 10^-73 */
		0xE2280B6C20DD5232, 0x25C6DA63C38DE1B0, /* ~= 10^-72 */
		0x8D590723948A535F, 0x579C487E5A38AD0E, /* ~= 10^-71 */
		0xB0AF48EC79ACE837, 0x2D835A9DF0C6D851, /* ~= 10^-70 */
		0xDCDB1B2798182244, 0xF8E431456CF88E65, /* ~= 10^-69 */
		0x8A08F0F8BF0F156B, 0x1B8E9ECB641B58FF, /* ~= 10^-68 */
		0xAC8B2D36EED2DAC5, 0xE272467E3D222F3F, /* ~= 10^-67 */
		0xD7ADF884AA879177, 0x5B0ED81DCC6ABB0F, /* ~= 10^-66 */
		0x86CCBB52EA94BAEA, 0x98E947129FC2B4E9, /* ~= 10^-65 */
		0xA87FEA27A539E9A5, 0x3F2398D747B36224, /* ~= 10^-64 */
		0xD29FE4B18E88640E, 0x8EEC7F0D19A03AAD, /* ~= 10^-63 */
		0x83A3EEEEF9153E89, 0x1953CF68300424AC, /* ~= 10^-62 */
		0xA48CEAAAB75A8E2B, 0x5FA8C3423C052DD7, /* ~= 10^-61 */
		0xCDB02555653131B6, 0x3792F412CB06794D, /* ~= 10^-60 */
		0x808E17555F3EBF11, 0xE2BBD88BBEE40BD0, /* ~= 10^-59 */
		0xA0B19D2AB70E6ED6, 0x5B6ACEAEAE9D0EC4, /* ~= 10^-58 */
		0xC8DE047564D20A8B, 0xF245825A5A445275, /* ~= 10^-57 */
		0xFB158592BE068D2E, 0xEED6E2F0F0D56712, /* ~= 10^-56 */
		0x9CED737BB6C4183D, 0x55464DD69685606B, /* ~= 10^-55 */
		0xC428D05AA4751E4C, 0xAA97E14C3C26B886, /* ~= 10^-54 */
		0xF53304714D9265DF, 0xD53DD99F4B3066A8, /* ~= 10^-53 */
		0x993FE2C6D07B7FAB, 0xE546A8038EFE4029, /* ~= 10^-52 */
		0xBF8FDB78849A5F96, 0xDE98520472BDD033, /* ~= 10^-51 */
		0xEF73D256A5C0F77C, 0x963E66858F6D4440, /* ~= 10^-50 */
		0x95A8637627989AAD, 0xDDE7001379A44AA8, /* ~= 10^-49 */
		0xBB127C53B17EC159, 0x5560C018580D5D52, /* ~= 10^-48 */
		0xE9D71B689DDE71AF, 0xAAB8F01E6E10B4A6, /* ~= 10^-47 */
		0x9226712162AB070D, 0xCAB3961304CA70E8, /* ~= 10^-46 */
		0xB6B00D69BB55C8D1, 0x3D607B97C5FD0D22, /* ~= 10^-45 */
		0xE45C10C42A2B3B05, 0x8CB89A7DB77C506A, /* ~= 10^-44 */
		0x8EB98A7A9A5B04E3, 0x77F3608E92ADB242, /* ~= 10^-43 */
		0xB267ED1940F1C61C, 0x55F038B237591ED3, /* ~= 10^-42 */
		0xDF01E85F912E37A3, 0x6B6C46DEC52F6688, /* ~= 10^-41 */
		0x8B61313BBABCE2C6, 0x2323AC4B3B3DA015, /* ~= 10^-40 */
		0xAE397D8AA96C1B77, 0xABEC975E0A0D081A, /* ~= 10^-39 */
		0xD9C7DCED53C72255, 0x96E7BD358C904A21, /* ~= 10^-38 */
		0x881CEA14545C7575, 0x7E50D64177DA2E54, /* ~= 10^-37 */
		0xAA242499697392D2, 0xDDE50BD1D5D0B9E9, /* ~= 10^-36 */
		0xD4AD2DBFC3D07787, 0x955E4EC64B44E864, /* ~= 10^-35 */
		0x84EC3C97DA624AB4, 0xBD5AF13BEF0B113E, /* ~= 10^-34 */
		0xA6274BBDD0FADD61, 0xECB1AD8AEACDD58E, /* ~= 10^-33 */
		0xCFB11EAD453994BA, 0x67DE18EDA5814AF2, /* ~= 10^-32 */
		0x81CEB32C4B43FCF4, 0x80EACF948770CED7, /* ~= 10^-31 */
		0xA2425FF75E14FC31, 0xA1258379A94D028D, /* ~= 10^-30 */
		0xCAD2F7F5359A3B3E, 0x096EE45813A04330, /* ~= 10^-29 */
		0xFD87B5F28300CA0D, 0x8BCA9D6E188853FC, /* ~= 10^-28 */
		0x9E74D1B791E07E48, 0x775EA264CF55347D, /* ~= 10^-27 */
		0xC612062576589DDA, 0x95364AFE032A819D, /* ~= 10^-26 */
		0xF79687AED3EEC551, 0x3A83DDBD83F52204, /* ~= 10^-25 */
		0x9ABE14CD44753B52, 0xC4926A9672793542, /* ~= 10^-24 */
		0xC16D9A0095928A27, 0x75B7053C0F178293, /* ~= 10^-23 */
		0xF1C90080BAF72CB1, 0x5324C68B12DD6338, /* ~= 10^-22 */
		0x971DA05074DA7BEE, 0xD3F6FC16EBCA5E03, /* ~= 10^-21 */
		0xBCE5086492111AEA, 0x88F4BB1CA6BCF584, /* ~= 10^-20 */
		0xEC1E4A7DB69561A5, 0x2B31E9E3D06C32E5, /* ~= 10^-19 */
		0x9392EE8E921D5D07, 0x3AFF322E62439FCF, /* ~= 10^-18 */
		0xB877AA3236A4B449, 0x09BEFEB9FAD487C2, /* ~= 10^-17 */
		0xE69594BEC44DE15B, 0x4C2EBE687989A9B3, /* ~= 10^-16 */
		0x901D7CF73AB0ACD9, 0x0F9D37014BF60A10, /* ~= 10^-15 */
		0xB424DC35095CD80F, 0x538484C19EF38C94, /* ~= 10^-14 */
		0xE12E13424BB40E13, 0x2865A5F206B06FB9, /* ~= 10^-13 */
		0x8CBCCC096F5088CB, 0xF93F87B7442E45D3, /* ~= 10^-12 */
		0xAFEBFF0BCB24AAFE, 0xF78F69A51539D748, /* ~= 10^-11 */
		0xDBE6FECEBDEDD5BE, 0xB573440E5A884D1B, /* ~= 10^-10 */
		0x89705F4136B4A597, 0x31680A88F8953030, /* ~= 10^-9 */
		0xABCC77118461CEFC, 0xFDC20D2B36BA7C3D, /* ~= 10^-8 */
		0xD6BF94D5E57A42BC, 0x3D32907604691B4C, /* ~= 10^-7 */
		0x8637BD05AF6C69B5, 0xA63F9A49C2C1B10F, /* ~= 10^-6 */
		0xA7C5AC471B478423, 0x0FCF80DC33721D53, /* ~= 10^-5 */
		0xD1B71758E219652B, 0xD3C36113404EA4A8, /* ~= 10^-4 */
		0x83126E978D4FDF3B, 0x645A1CAC083126E9, /* ~= 10^-3 */
		0xA3D70A3D70A3D70A, 0x3D70A3D70A3D70A3, /* ~= 10^-2 */
		0xCCCCCCCCCCCCCCCC, 0xCCCCCCCCCCCCCCCC, /* ~= 10^-1 */
		0x8000000000000000, 0x0000000000000000, /* == 10^0 */
		0xA000000000000000, 0x0000000000000000, /* == 10^1 */
		0xC800000000000000, 0x0000000000000000, /* == 10^2 */
		0xFA00000000000000, 0x0000000000000000, /* == 10^3 */
		0x9C40000000000000, 0x0000000000000000, /* == 10^4 */
		0xC350000000000000, 0x0000000000000000, /* == 10^5 */
		0xF424000000000000, 0x0000000000000000, /* == 10^6 */
		0x9896800000000000, 0x0000000000000000, /* == 10^7 */
		0xBEBC200000000000, 0x0000000000000000, /* == 10^8 */
		0xEE6B280000000000, 0x0000000000000000, /* == 10^9 */
		0x9502F90000000000, 0x0000000000000000, /* == 10^10 */
		0xBA43B74000000000, 0x0000000000000000, /* == 10^11 */
		0xE8D4A51000000000, 0x0000000000000000, /* == 10^12 */
		0x9184E72A00000000, 0x0000000000000000, /* == 10^13 */
		0xB5E620F480000000, 0x0000000000000000, /* == 10^14 */
		0xE35FA931A0000000, 0x0000000000000000, /* == 10^15 */
		0x8E1BC9BF04000000, 0x0000000000000000, /* == 10^16 */
		0xB1A2BC2EC5000000, 0x0000000000000000, /* == 10^17 */
		0xDE0B6B3A76400000, 0x0000000000000000, /* == 10^18 */
		0x8AC7230489E80000, 0x0000000000000000, /* == 10^19 */
		0xAD78EBC5AC620000, 0x0000000000000000, /* == 10^20 */
		0xD8D726B7177A8000, 0x0000000000000000, /* == 10^21 */
		0x878678326EAC9000, 0x0000000000000000, /* == 10^22 */
		0xA968163F0A57B400, 0x0000000000000000, /* == 10^23 */
		0xD3C21BCECCEDA100, 0x0000000000000000, /* == 10^24 */
		0x84595161401484A0, 0x0000000000000000, /* == 10^25 */
		0xA56FA5B99019A5C8, 0x0000000000000000, /* == 10^26 */
		0xCECB8F27F4200F3A, 0x0000000000000000, /* == 10^27 */
		0x813F3978F8940984, 0x4000000000000000, /* == 10^28 */
		0xA18F07D736B90BE5, 0x5000000000000000, /* == 10^29 */
		0xC9F2C9CD04674EDE, 0xA400000000000000, /* == 10^30 */
		0xFC6F7C4045812296, 0x4D00000000000000, /* == 10^31 */
		0x9DC5ADA82B70B59D, 0xF020000000000000, /* == 10^32 */
		0xC5371912364CE305, 0x6C28000000000000, /* == 10^33 */
		0xF684DF56C3E01BC6, 0xC732000000000000, /* == 10^34 */
		0x9A130B963A6C115C, 0x3C7F400000000000, /* == 10^35 */
		0xC097CE7BC90715B3, 0x4B9F100000000000, /* == 10^36 */
		0xF0BDC21ABB48DB20, 0x1E86D40000000000, /* == 10^37 */
		0x96769950B50D88F4, 0x1314448000000000, /* == 10^38 */
		0xBC143FA4E250EB31, 0x17D955A000000000, /* == 10^39 */
		0xEB194F8E1AE525FD, 0x5DCFAB0800000000, /* == 10^40 */
		0x92EFD1B8D0CF37BE, 0x5AA1CAE500000000, /* == 10^41 */
		0xB7ABC627050305AD, 0xF14A3D9E40000000, /* == 10^42 */
		0xE596B7B0C643C719, 0x6D9CCD05D0000000, /* == 10^43 */
		0x8F7E32CE7BEA5C6F, 0xE4820023A2000000, /* == 10^44 */
		0xB35DBF821AE4F38B, 0xDDA2802C8A800000, /* == 10^45 */
		0xE0352F62A19E306E, 0xD50B2037AD200000, /* == 10^46 */
		0x8C213D9DA502DE45, 0x4526F422CC340000, /* == 10^47 */
		0xAF298D050E4395D6, 0x9670B12B7F410000, /* == 10^48 */
		0xDAF3F04651D47B4C, 0x3C0CDD765F114000, /* == 10^49 */
		0x88D8762BF324CD0F, 0xA5880A69FB6AC800, /* == 10^50 */
		0xAB0E93B6EFEE0053, 0x8EEA0D047A457A00, /* == 10^51 */
		0xD5D238A4ABE98068, 0x72A4904598D6D880, /* == 10^52 */
		0x85A36366EB71F041, 0x47A6DA2B7F864750, /* == 10^53 */
		0xA70C3C40A64E6C51, 0x999090B65F67D924, /* == 10^54 */
		0xD0CF4B50CFE20765, 0xFFF4B4E3F741CF6D, /* == 10^55 */
		0x82818F1281ED449F, 0xBFF8F10E7A8921A4, /* ~= 10^56 */
		0xA321F2D7226895C7, 0xAFF72D52192B6A0D, /* ~= 10^57 */
		0xCBEA6F8CEB02BB39, 0x9BF4F8A69F764490, /* ~= 10^58 */
		0xFEE50B7025C36A08, 0x02F236D04753D5B4, /* ~= 10^59 */
		0x9F4F2726179A2245, 0x01D762422C946590, /* ~= 10^60 */
		0xC722F0EF9D80AAD6, 0x424D3AD2B7B97EF5, /* ~= 10^61 */
		0xF8EBAD2B84E0D58B, 0xD2E0898765A7DEB2, /* ~= 10^62 */
		0x9B934C3B330C8577, 0x63CC55F49F88EB2F, /* ~= 10^63 */
		0xC2781F49FFCFA6D5, 0x3CBF6B71C76B25FB, /* ~= 10^64 */
		0xF316271C7FC3908A, 0x8BEF464E3945EF7A, /* ~= 10^65 */
		0x97EDD871CFDA3A56, 0x97758BF0E3CBB5AC, /* ~= 10^66 */
		0xBDE94E8E43D0C8EC, 0x3D52EEED1CBEA317, /* ~= 10^67 */
		0xED63A231D4C4FB27, 0x4CA7AAA863EE4BDD, /* ~= 10^68 */
		0x945E455F24FB1CF8, 0x8FE8CAA93E74EF6A, /* ~= 10^69 */
		0xB975D6B6EE39E436, 0xB3E2FD538E122B44, /* ~= 10^70 */
		0xE7D34C64A9C85D44, 0x60DBBCA87196B616, /* ~= 10^71 */
		0x90E40FBEEA1D3A4A, 0xBC8955E946FE31CD, /* ~= 10^72 */
		0xB51D13AEA4A488DD, 0x6BABAB6398BDBE41, /* ~= 10^73 */
		0xE264589A4DCDAB14, 0xC696963C7EED2DD1, /* ~= 10^74 */
		0x8D7EB76070A08AEC, 0xFC1E1DE5CF543CA2, /* ~= 10^75 */
		0xB0DE65388CC8ADA8, 0x3B25A55F43294BCB, /* ~= 10^76 */
		0xDD15FE86AFFAD912, 0x49EF0EB713F39EBE, /* ~= 10^77 */
		0x8A2DBF142DFCC7AB, 0x6E3569326C784337, /* ~= 10^78 */
		0xACB92ED9397BF996, 0x49C2C37F07965404, /* ~= 10^79 */
		0xD7E77A8F87DAF7FB, 0xDC33745EC97BE906, /* ~= 10^80 */
		0x86F0AC99B4E8DAFD, 0x69A028BB3DED71A3, /* ~= 10^81 */
		0xA8ACD7C0222311BC, 0xC40832EA0D68CE0C, /* ~= 10^82 */
		0xD2D80DB02AABD62B, 0xF50A3FA490C30190, /* ~= 10^83 */
		0x83C7088E1AAB65DB, 0x792667C6DA79E0FA, /* ~= 10^84 */
		0xA4B8CAB1A1563F52, 0x577001B891185938, /* ~= 10^85 */
		0xCDE6FD5E09ABCF26, 0xED4C0226B55E6F86, /* ~= 10^86 */
		0x80B05E5AC60B6178, 0x544F8158315B05B4, /* ~= 10^87 */
		0xA0DC75F1778E39D6, 0x696361AE3DB1C721, /* ~= 10^88 */
		0xC913936DD571C84C, 0x03BC3A19CD1E38E9, /* ~= 10^89 */
		0xFB5878494ACE3A5F, 0x04AB48A04065C723, /* ~= 10^90 */
		0x9D174B2DCEC0E47B, 0x62EB0D64283F9C76, /* ~= 10^91 */
		0xC45D1DF942711D9A, 0x3BA5D0BD324F8394, /* ~= 10^92 */
		0xF5746577930D6500, 0xCA8F44EC7EE36479, /* ~= 10^93 */
		0x9968BF6ABBE85F20, 0x7E998B13CF4E1ECB, /* ~= 10^94 */
		0xBFC2EF456AE276E8, 0x9E3FEDD8C321A67E, /* ~= 10^95 */
		0xEFB3AB16C59B14A2, 0xC5CFE94EF3EA101E, /* ~= 10^96 */
		0x95D04AEE3B80ECE5, 0xBBA1F1D158724A12, /* ~= 10^97 */
		0xBB445DA9CA61281F, 0x2A8A6E45AE8EDC97, /* ~= 10^98 */
		0xEA1575143CF97226, 0xF52D09D71A3293BD, /* ~= 10^99 */
		0x924D692CA61BE758, 0x593C2626705F9C56, /* ~= 10^100 */
		0xB6E0C377CFA2E12E, 0x6F8B2FB00C77836C, /* ~= 10^101 */
		0xE498F455C38B997A, 0x0B6DFB9C0F956447, /* ~= 10^102 */
		0x8EDF98B59A373FEC, 0x4724BD4189BD5EAC, /* ~= 10^103 */
		0xB2977EE300C50FE7, 0x58EDEC91EC2CB657, /* ~= 10^104 */
		0xDF3D5E9BC0F653E1, 0x2F2967B66737E3ED, /* ~= 10^105 */
		0x8B865B215899F46C, 0xBD79E0D20082EE74, /* ~= 10^106 */
		0xAE67F1E9AEC07187, 0xECD8590680A3AA11, /* ~= 10^107 */
		0xDA01EE641A708DE9, 0xE80E6F4820CC9495, /* ~= 10^108 */
		0x884134FE908658B2, 0x3109058D147FDCDD, /* ~= 10^109 */
		0xAA51823E34A7EEDE, 0xBD4B46F0599FD415, /* ~= 10^110 */
		0xD4E5E2CDC1D1EA96, 0x6C9E18AC7007C91A, /* ~= 10^111 */
		0x850FADC09923329E, 0x03E2CF6BC604DDB0, /* ~= 10^112 */
		0xA6539930BF6BFF45, 0x84DB8346B786151C, /* ~= 10^113 */
		0xCFE87F7CEF46FF16, 0xE612641865679A63, /* ~= 10^114 */
		0x81F14FAE158C5F6E, 0x4FCB7E8F3F60C07E, /* ~= 10^115 */
		0xA26DA3999AEF7749, 0xE3BE5E330F38F09D, /* ~= 10^116 */
		0xCB090C8001AB551C, 0x5CADF5BFD3072CC5, /* ~= 10^117 */
		0xFDCB4FA002162A63, 0x73D9732FC7C8F7F6, /* ~= 10^118 */
		0x9E9F11C4014DDA7E, 0x2867E7FDDCDD9AFA, /* ~= 10^119 */
		0xC646D63501A1511D, 0xB281E1FD541501B8, /* ~= 10^120 */
		0xF7D88BC24209A565, 0x1F225A7CA91A4226, /* ~= 10^121 */
		0x9AE757596946075F, 0x3375788DE9B06958, /* ~= 10^122 */
		0xC1A12D2FC3978937, 0x0052D6B1641C83AE, /* ~= 10^123 */
		0xF209787BB47D6B84, 0xC0678C5DBD23A49A, /* ~= 10^124 */
		0x9745EB4D50CE6332, 0xF840B7BA963646E0, /* ~= 10^125 */
		0xBD176620A501FBFF, 0xB650E5A93BC3D898, /* ~= 10^126 */
		0xEC5D3FA8CE427AFF, 0xA3E51F138AB4CEBE, /* ~= 10^127 */
		0x93BA47C980E98CDF, 0xC66F336C36B10137, /* ~= 10^128 */
		0xB8A8D9BBE123F017, 0xB80B0047445D4184, /* ~= 10^129 */
		0xE6D3102AD96CEC1D, 0xA60DC059157491E5, /* ~= 10^130 */
		0x9043EA1AC7E41392, 0x87C89837AD68DB2F, /* ~= 10^131 */
		0xB454E4A179DD1877, 0x29BABE4598C311FB, /* ~= 10^132 */
		0xE16A1DC9D8545E94, 0xF4296DD6FEF3D67A, /* ~= 10^133 */
		0x8CE2529E2734BB1D, 0x1899E4A65F58660C, /* ~= 10^134 */
		0xB01AE745B101E9E4, 0x5EC05DCFF72E7F8F, /* ~= 10^135 */
		0xDC21A1171D42645D, 0x76707543F4FA1F73, /* ~= 10^136 */
		0x899504AE72497EBA, 0x6A06494A791C53A8, /* ~= 10^137 */
		0xABFA45DA0EDBDE69, 0x0487DB9D17636892, /* ~= 10^138 */
		0xD6F8D7509292D603, 0x45A9D2845D3C42B6, /* ~= 10^139 */
		0x865B86925B9BC5C2, 0x0B8A2392BA45A9B2, /* ~= 10^140 */
		0xA7F26836F282B732, 0x8E6CAC7768D7141E, /* ~= 10^141 */
		0xD1EF0244AF2364FF, 0x3207D795430CD926, /* ~= 10^142 */
		0x8335616AED761F1F, 0x7F44E6BD49E807B8, /* ~= 10^143 */
		0xA402B9C5A8D3A6E7, 0x5F16206C9C6209A6, /* ~= 10^144 */
		0xCD036837130890A1, 0x36DBA887C37A8C0F, /* ~= 10^145 */
		0x802221226BE55A64, 0xC2494954DA2C9789, /* ~= 10^146 */
		0xA02AA96B06DEB0FD, 0xF2DB9BAA10B7BD6C, /* ~= 10^147 */
		0xC83553C5C8965D3D, 0x6F92829494E5ACC7, /* ~= 10^148 */
		0xFA42A8B73ABBF48C, 0xCB772339BA1F17F9, /* ~= 10^149 */
		0x9C69A97284B578D7, 0xFF2A760414536EFB, /* ~= 10^150 */
		0xC38413CF25E2D70D, 0xFEF5138519684ABA, /* ~= 10^151 */
		0xF46518C2EF5B8CD1, 0x7EB258665FC25D69, /* ~= 10^152 */
		0x98BF2F79D5993802, 0xEF2F773FFBD97A61, /* ~= 10^153 */
		0xBEEEFB584AFF8603, 0xAAFB550FFACFD8FA, /* ~= 10^154 */
		0xEEAABA2E5DBF6784, 0x95BA2A53F983CF38, /* ~= 10^155 */
		0x952AB45CFA97A0B2, 0xDD945A747BF26183, /* ~= 10^156 */
		0xBA756174393D88DF, 0x94F971119AEEF9E4, /* ~= 10^157 */
		0xE912B9D1478CEB17, 0x7A37CD5601AAB85D, /* ~= 10^158 */
		0x91ABB422CCB812EE, 0xAC62E055C10AB33A, /* ~= 10^159 */
		0xB616A12B7FE617AA, 0x577B986B314D6009, /* ~= 10^160 */
		0xE39C49765FDF9D94, 0xED5A7E85FDA0B80B, /* ~= 10^161 */
		0x8E41ADE9FBEBC27D, 0x14588F13BE847307, /* ~= 10^162 */
		0xB1D219647AE6B31C, 0x596EB2D8AE258FC8, /* ~= 10^163 */
		0xDE469FBD99A05FE3, 0x6FCA5F8ED9AEF3BB, /* ~= 10^164 */
		0x8AEC23D680043BEE, 0x25DE7BB9480D5854, /* ~= 10^165 */
		0xADA72CCC20054AE9, 0xAF561AA79A10AE6A, /* ~= 10^166 */
		0xD910F7FF28069DA4, 0x1B2BA1518094DA04, /* ~= 10^167 */
		0x87AA9AFF79042286, 0x90FB44D2F05D0842, /* ~= 10^168 */
		0xA99541BF57452B28, 0x353A1607AC744A53, /* ~= 10^169 */
		0xD3FA922F2D1675F2, 0x42889B8997915CE8, /* ~= 10^170 */
		0x847C9B5D7C2E09B7, 0x69956135FEBADA11, /* ~= 10^171 */
		0xA59BC234DB398C25, 0x43FAB9837E699095, /* ~= 10^172 */
		0xCF02B2C21207EF2E, 0x94F967E45E03F4BB, /* ~= 10^173 */
		0x8161AFB94B44F57D, 0x1D1BE0EEBAC278F5, /* ~= 10^174 */
		0xA1BA1BA79E1632DC, 0x6462D92A69731732, /* ~= 10^175 */
		0xCA28A291859BBF93, 0x7D7B8F7503CFDCFE, /* ~= 10^176 */
		0xFCB2CB35E702AF78, 0x5CDA735244C3D43E, /* ~= 10^177 */
		0x9DEFBF01B061ADAB, 0x3A0888136AFA64A7, /* ~= 10^178 */
		0xC56BAEC21C7A1916, 0x088AAA1845B8FDD0, /* ~= 10^179 */
		0xF6C69A72A3989F5B, 0x8AAD549E57273D45, /* ~= 10^180 */
		0x9A3C2087A63F6399, 0x36AC54E2F678864B, /* ~= 10^181 */
		0xC0CB28A98FCF3C7F, 0x84576A1BB416A7DD, /* ~= 10^182 */
		0xF0FDF2D3F3C30B9F, 0x656D44A2A11C51D5, /* ~= 10^183 */
		0x969EB7C47859E743, 0x9F644AE5A4B1B325, /* ~= 10^184 */
		0xBC4665B596706114, 0x873D5D9F0DDE1FEE, /* ~= 10^185 */
		0xEB57FF22FC0C7959, 0xA90CB506D155A7EA, /* ~= 10^186 */
		0x9316FF75DD87CBD8, 0x09A7F12442D588F2, /* ~= 10^187 */
		0xB7DCBF5354E9BECE, 0x0C11ED6D538AEB2F, /* ~= 10^188 */
		0xE5D3EF282A242E81, 0x8F1668C8A86DA5FA, /* ~= 10^189 */
		0x8FA475791A569D10, 0xF96E017D694487BC, /* ~= 10^190 */
		0xB38D92D760EC4455, 0x37C981DCC395A9AC, /* ~= 10^191 */
		0xE070F78D3927556A, 0x85BBE253F47B1417, /* ~= 10^192 */
		0x8C469AB843B89562, 0x93956D7478CCEC8E, /* ~= 10^193 */
		0xAF58416654A6BABB, 0x387AC8D1970027B2, /* ~= 10^194 */
		0xDB2E51BFE9D0696A, 0x06997B05FCC0319E, /* ~= 10^195 */
		0x88FCF317F22241E2, 0x441FECE3BDF81F03, /* ~= 10^196 */
		0xAB3C2FDDEEAAD25A, 0xD527E81CAD7626C3, /* ~= 10^197 */
		0xD60B3BD56A5586F1, 0x8A71E223D8D3B074, /* ~= 10^198 */
		0x85C7056562757456, 0xF6872D5667844E49, /* ~= 10^199 */
		0xA738C6BEBB12D16C, 0xB428F8AC016561DB, /* ~= 10^200 */
		0xD106F86E69D785C7, 0xE13336D701BEBA52, /* ~= 10^201 */
		0x82A45B450226B39C, 0xECC0024661173473, /* ~= 10^202 */
		0xA34D721642B06084, 0x27F002D7F95D0190, /* ~= 10^203 */
		0xCC20CE9BD35C78A5, 0x31EC038DF7B441F4, /* ~= 10^204 */
		0xFF290242C83396CE, 0x7E67047175A15271, /* ~= 10^205 */
		0x9F79A169BD203E41, 0x0F0062C6E984D386, /* ~= 10^206 */
		0xC75809C42C684DD1, 0x52C07B78A3E60868, /* ~= 10^207 */
		0xF92E0C3537826145, 0xA7709A56CCDF8A82, /* ~= 10^208 */
		0x9BBCC7A142B17CCB, 0x88A66076400BB691, /* ~= 10^209 */
		0xC2ABF989935DDBFE, 0x6ACFF893D00EA435, /* ~= 10^210 */
		0xF356F7EBF83552FE, 0x0583F6B8C4124D43, /* ~= 10^211 */
		0x98165AF37B2153DE, 0xC3727A337A8B704A, /* ~= 10^212 */
		0xBE1BF1B059E9A8D6, 0x744F18C0592E4C5C, /* ~= 10^213 */
		0xEDA2EE1C7064130C, 0x1162DEF06F79DF73, /* ~= 10^214 */
		0x9485D4D1C63E8BE7, 0x8ADDCB5645AC2BA8, /* ~= 10^215 */
		0xB9A74A0637CE2EE1, 0x6D953E2BD7173692, /* ~= 10^216 */
		0xE8111C87C5C1BA99, 0xC8FA8DB6CCDD0437, /* ~= 10^217 */
		0x910AB1D4DB9914A0, 0x1D9C9892400A22A2, /* ~= 10^218 */
		0xB54D5E4A127F59C8, 0x2503BEB6D00CAB4B, /* ~= 10^219 */
		0xE2A0B5DC971F303A, 0x2E44AE64840FD61D, /* ~= 10^220 */
		0x8DA471A9DE737E24, 0x5CEAECFED289E5D2, /* ~= 10^221 */
		0xB10D8E1456105DAD, 0x7425A83E872C5F47, /* ~= 10^222 */
		0xDD50F1996B947518, 0xD12F124E28F77719, /* ~= 10^223 */
		0x8A5296FFE33CC92F, 0x82BD6B70D99AAA6F, /* ~= 10^224 */
		0xACE73CBFDC0BFB7B, 0x636CC64D1001550B, /* ~= 10^225 */
		0xD8210BEFD30EFA5A, 0x3C47F7E05401AA4E, /* ~= 10^226 */
		0x8714A775E3E95C78, 0x65ACFAEC34810A71, /* ~= 10^227 */
		0xA8D9D1535CE3B396, 0x7F1839A741A14D0D, /* ~= 10^228 */
		0xD31045A8341CA07C, 0x1EDE48111209A050, /* ~= 10^229 */
		0x83EA2B892091E44D, 0x934AED0AAB460432, /* ~= 10^230 */
		0xA4E4B66B68B65D60, 0xF81DA84D5617853F, /* ~= 10^231 */
		0xCE1DE40642E3F4B9, 0x36251260AB9D668E, /* ~= 10^232 */
		0x80D2AE83E9CE78F3, 0xC1D72B7C6B426019, /* ~= 10^233 */
		0xA1075A24E4421730, 0xB24CF65B8612F81F, /* ~= 10^234 */
		0xC94930AE1D529CFC, 0xDEE033F26797B627, /* ~= 10^235 */
		0xFB9B7CD9A4A7443C, 0x169840EF017DA3B1, /* ~= 10^236 */
		0x9D412E0806E88AA5, 0x8E1F289560EE864E, /* ~= 10^237 */
		0xC491798A08A2AD4E, 0xF1A6F2BAB92A27E2, /* ~= 10^238 */
		0xF5B5D7EC8ACB58A2, 0xAE10AF696774B1DB, /* ~= 10^239 */
		0x9991A6F3D6BF1765, 0xACCA6DA1E0A8EF29, /* ~= 10^240 */
		0xBFF610B0CC6EDD3F, 0x17FD090A58D32AF3, /* ~= 10^241 */
		0xEFF394DCFF8A948E, 0xDDFC4B4CEF07F5B0, /* ~= 10^242 */
		0x95F83D0A1FB69CD9, 0x4ABDAF101564F98E, /* ~= 10^243 */
		0xBB764C4CA7A4440F, 0x9D6D1AD41ABE37F1, /* ~= 10^244 */
		0xEA53DF5FD18D5513, 0x84C86189216DC5ED, /* ~= 10^245 */
		0x92746B9BE2F8552C, 0x32FD3CF5B4E49BB4, /* ~= 10^246 */
		0xB7118682DBB66A77, 0x3FBC8C33221DC2A1, /* ~= 10^247 */
		0xE4D5E82392A40515, 0x0FABAF3FEAA5334A, /* ~= 10^248 */
		0x8F05B1163BA6832D, 0x29CB4D87F2A7400E, /* ~= 10^249 */
		0xB2C71D5BCA9023F8, 0x743E20E9EF511012, /* ~= 10^250 */
		0xDF78E4B2BD342CF6, 0x914DA9246B255416, /* ~= 10^251 */
		0x8BAB8EEFB6409C1A, 0x1AD089B6C2F7548E, /* ~= 10^252 */
		0xAE9672ABA3D0C320, 0xA184AC2473B529B1, /* ~= 10^253 */
		0xDA3C0F568CC4F3E8, 0xC9E5D72D90A2741E, /* ~= 10^254 */
		0x8865899617FB1871, 0x7E2FA67C7A658892, /* ~= 10^255 */
		0xAA7EEBFB9DF9DE8D, 0xDDBB901B98FEEAB7, /* ~= 10^256 */
		0xD51EA6FA85785631, 0x552A74227F3EA565, /* ~= 10^257 */
		0x8533285C936B35DE, 0xD53A88958F87275F, /* ~= 10^258 */
		0xA67FF273B8460356, 0x8A892ABAF368F137, /* ~= 10^259 */
		0xD01FEF10A657842C, 0x2D2B7569B0432D85, /* ~= 10^260 */
		0x8213F56A67F6B29B, 0x9C3B29620E29FC73, /* ~= 10^261 */
		0xA298F2C501F45F42, 0x8349F3BA91B47B8F, /* ~= 10^262 */
		0xCB3F2F7642717713, 0x241C70A936219A73, /* ~= 10^263 */
		0xFE0EFB53D30DD4D7, 0xED238CD383AA0110, /* ~= 10^264 */
		0x9EC95D1463E8A506, 0xF4363804324A40AA, /* ~= 10^265 */
		0xC67BB4597CE2CE48, 0xB143C6053EDCD0D5, /* ~= 10^266 */
		0xF81AA16FDC1B81DA, 0xDD94B7868E94050A, /* ~= 10^267 */
		0x9B10A4E5E9913128, 0xCA7CF2B4191C8326, /* ~= 10^268 */
		0xC1D4CE1F63F57D72, 0xFD1C2F611F63A3F0, /* ~= 10^269 */
		0xF24A01A73CF2DCCF, 0xBC633B39673C8CEC, /* ~= 10^270 */
		0x976E41088617CA01, 0xD5BE0503E085D813, /* ~= 10^271 */
		0xBD49D14AA79DBC82, 0x4B2D8644D8A74E18, /* ~= 10^272 */
		0xEC9C459D51852BA2, 0xDDF8E7D60ED1219E, /* ~= 10^273 */
		0x93E1AB8252F33B45, 0xCABB90E5C942B503, /* ~= 10^274 */
		0xB8DA1662E7B00A17, 0x3D6A751F3B936243, /* ~= 10^275 */
		0xE7109BFBA19C0C9D, 0x0CC512670A783AD4, /* ~= 10^276 */
		0x906A617D450187E2, 0x27FB2B80668B24C5, /* ~= 10^277 */
		0xB484F9DC9641E9DA, 0xB1F9F660802DEDF6, /* ~= 10^278 */
		0xE1A63853BBD26451, 0x5E7873F8A0396973, /* ~= 10^279 */
		0x8D07E33455637EB2, 0xDB0B487B6423E1E8, /* ~= 10^280 */
		0xB049DC016ABC5E5F, 0x91CE1A9A3D2CDA62, /* ~= 10^281 */
		0xDC5C5301C56B75F7, 0x7641A140CC7810FB, /* ~= 10^282 */
		0x89B9B3E11B6329BA, 0xA9E904C87FCB0A9D, /* ~= 10^283 */
		0xAC2820D9623BF429, 0x546345FA9FBDCD44, /* ~= 10^284 */
		0xD732290FBACAF133, 0xA97C177947AD4095, /* ~= 10^285 */
		0x867F59A9D4BED6C0, 0x49ED8EABCCCC485D, /* ~= 10^286 */
		0xA81F301449EE8C70, 0x5C68F256BFFF5A74, /* ~= 10^287 */
		0xD226FC195C6A2F8C, 0x73832EEC6FFF3111, /* ~= 10^288 */
		0x83585D8FD9C25DB7, 0xC831FD53C5FF7EAB, /* ~= 10^289 */
		0xA42E74F3D032F525, 0xBA3E7CA8B77F5E55, /* ~= 10^290 */
		0xCD3A1230C43FB26F, 0x28CE1BD2E55F35EB, /* ~= 10^291 */
		0x80444B5E7AA7CF85, 0x7980D163CF5B81B3, /* ~= 10^292 */
		0xA0555E361951C366, 0xD7E105BCC332621F, /* ~= 10^293 */
		0xC86AB5C39FA63440, 0x8DD9472BF3FEFAA7, /* ~= 10^294 */
		0xFA856334878FC150, 0xB14F98F6F0FEB951, /* ~= 10^295 */
		0x9C935E00D4B9D8D2, 0x6ED1BF9A569F33D3, /* ~= 10^296 */
		0xC3B8358109E84F07, 0x0A862F80EC4700C8, /* ~= 10^297 */
		0xF4A642E14C6262C8, 0xCD27BB612758C0FA, /* ~= 10^298 */
		0x98E7E9CCCFBD7DBD, 0x8038D51CB897789C, /* ~= 10^299 */
		0xBF21E44003ACDD2C, 0xE0470A63E6BD56C3, /* ~= 10^300 */
		0xEEEA5D5004981478, 0x1858CCFCE06CAC74, /* ~= 10^301 */
		0x95527A5202DF0CCB, 0x0F37801E0C43EBC8, /* ~= 10^302 */
		0xBAA718E68396CFFD, 0xD30560258F54E6BA, /* ~= 10^303 */
		0xE950DF20247C83FD, 0x47C6B82EF32A2069, /* ~= 10^304 */
		0x91D28B7416CDD27E, 0x4CDC331D57FA5441, /* ~= 10^305 */
		0xB6472E511C81471D, 0xE0133FE4ADF8E952, /* ~= 10^306 */
		0xE3D8F9E563A198E5, 0x58180FDDD97723A6, /* ~= 10^307 */
		0x8E679C2F5E44FF8F, 0x570F09EAA7EA7648, /* ~= 10^308 */
		0xB201833B35D63F73, 0x2CD2CC6551E513DA, /* ~= 10^309 */
		0xDE81E40A034BCF4F, 0xF8077F7EA65E58D1, /* ~= 10^310 */
		0x8B112E86420F6191, 0xFB04AFAF27FAF782, /* ~= 10^311 */
		0xADD57A27D29339F6, 0x79C5DB9AF1F9B563, /* ~= 10^312 */
		0xD94AD8B1C7380874, 0x18375281AE7822BC, /* ~= 10^313 */
		0x87CEC76F1C830548, 0x8F2293910D0B15B5, /* ~= 10^314 */
		0xA9C2794AE3A3C69A, 0xB2EB3875504DDB22, /* ~= 10^315 */
		0xD433179D9C8CB841, 0x5FA60692A46151EB, /* ~= 10^316 */
		0x849FEEC281D7F328, 0xDBC7C41BA6BCD333, /* ~= 10^317 */
		0xA5C7EA73224DEFF3, 0x12B9B522906C0800, /* ~= 10^318 */
		0xCF39E50FEAE16BEF, 0xD768226B34870A00, /* ~= 10^319 */
		0x81842F29F2CCE375, 0xE6A1158300D46640, /* ~= 10^320 */
		0xA1E53AF46F801C53, 0x60495AE3C1097FD0, /* ~= 10^321 */
		0xCA5E89B18B602368, 0x385BB19CB14BDFC4, /* ~= 10^322 */
		0xFCF62C1DEE382C42, 0x46729E03DD9ED7B5, /* ~= 10^323 */
		0x9E19DB92B4E31BA9, 0x6C07A2C26A8346D1 /* ~= 10^324 */
	};

	/**
    Get the cached pow10 value from pow10SigTable.
    @param exp10 The exponent of pow(10, e). This value must in range
                 POW10_SIG_TABLE_MIN_EXP to POW10_SIG_TABLE_MAX_EXP.
    @param hi    The highest 64 bits of pow(10, e).
    @param lo    The lower 64 bits after `hi`.
    */
	inline void pow10TableGetSig128(int32_t exp10, uint64_t* hi, uint64_t* lo) {
		int32_t idx = exp10 - (POW10_SIG_TABLE_128_MIN_EXP);
		*hi = pow10_sig_table_128[idx * 2ull];
		*lo = pow10_sig_table_128[idx * 2ull + 1ull];
	}

	inline constexpr std::array<uint64_t, 21> exp10_int{ 1ull, 10ull, 100ull, 1000ull, 10000ull, 100000ull, 1000000ull, 10000000ull, 100000000ull,
		1000000000ull, 10000000000ull, 100000000000ull, 1000000000000ull, 10000000000000ull, 100000000000000ull, 1000000000000000ull,
		10000000000000000ull, 100000000000000000ull, 1000000000000000000ull, 10000000000000000000ull };

	/**
    Convert double number from binary to decimal.
    The output significand is shortest decimal but may have trailing zeros.

    This function use the Schubfach algorithm:
    Raffaello Giulietti, The Schubfach way to render doubles, 2020.
    https://drive.google.com/open?id=1luHhyQF9zKlM8yJ1nebU0OgVYhfC6CBN
    https://github.com/abolz/Drachennest

    See also:
    Dragonbox: A New Floating-Point Binary-to-Decimal Conversion Algorithm, 2020.
    https://github.com/jk-jeon/dragonbox/blob/master/other_files/Dragonbox.pdf
    https://github.com/jk-jeon/dragonbox

    @param sig_raw The raw value of significand in IEEE 754 format.
    @param exp_raw The raw value of exponent in IEEE 754 format.
    @param sig_bin The decoded value of significand in binary.
    @param exp_bin The decoded value of exponent in binary.
    @param sig_dec The output value of significand in decimal.
    @param exp_dec The output value of exponent in decimal.
    @warning The input double number should not be 0, inf, nan.
    */
	inline void f64BinToDec(uint64_t sig_raw, int32_t exp_raw, uint64_t sig_bin, int32_t exp_bin, uint64_t* sig_dec, int32_t* exp_dec) {
		bool is_even, lower_bound_closer, u_inside, w_inside, round_up;
		uint64_t s, sp, cb, cbl, cbr, vb, vbl, vbr, pow10hi, pow10lo, upper, lower, mid;
		int32_t k, h, exp10;

		is_even = !(sig_bin & 1);
		lower_bound_closer = (sig_raw == 0 && exp_raw > 1);

		cbl = 4 * sig_bin - 2 + lower_bound_closer;
		cb = 4 * sig_bin;
		cbr = 4 * sig_bin + 2;

		/* exp_bin: [-1074, 971]                                                  */
		/* k = lower_bound_closer ? floor(log10(pow(2, exp_bin)))                 */
		/*                        : floor(log10(pow(2, exp_bin) * 3.0 / 4.0))     */
		/*   = lower_bound_closer ? floor(exp_bin * log10(2))                     */
		/*                        : floor(exp_bin * log10(2) + log10(3.0 / 4.0))  */
		k = (exp_bin * 315653 - (lower_bound_closer ? 131237 : 0)) >> 20;

		/* k: [-324, 292]                                                         */
		/* h = exp_bin + floor(log2(pow(10, e)))                                  */
		/*   = exp_bin + floor(log2(10) * e)                                      */
		exp10 = -k;
		h = exp_bin + ((exp10 * 217707) >> 16) + 1;

		pow10TableGetSig128(exp10, &pow10hi, &pow10lo);
		pow10lo += (exp10 < POW10_SIG_TABLE_128_MIN_EXACT_EXP || exp10 > POW10_SIG_TABLE_128_MAX_EXACT_EXP);
		vbl = roundToOdd(pow10hi, pow10lo, cbl << h);
		vb = roundToOdd(pow10hi, pow10lo, cb << h);
		vbr = roundToOdd(pow10hi, pow10lo, cbr << h);

		lower = vbl + !is_even;
		upper = vbr - !is_even;

		s = vb / 4;
		if (s >= 10) {
			sp = s / 10;
			u_inside = (lower <= 40 * sp);
			w_inside = (upper >= 40 * sp + 40);
			if (u_inside != w_inside) {
				*sig_dec = sp + w_inside;
				*exp_dec = k + 1;
				return;
			}
		}

		u_inside = (lower <= 4 * s);
		w_inside = (upper >= 4 * s + 4);

		mid = 4 * s + 2;
		round_up = (vb > mid) || (vb == mid && (s & 1) != 0);

		*sig_dec = s + ((u_inside != w_inside) ? w_inside : round_up);
		*exp_dec = k;
	}

	/** Trailing zero count table for number 0 to 99.
    (generate with misc/make_tables.c) */
	inline constexpr uint8_t dec_trailing_zero_table[] = { 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

	/**
    Write an unsigned integer with a length of 15 to 17 with trailing zero trimmed.
    These digits are named as "aabbccddeeffgghhii" here.
    For example, input 1234567890123000, output "1234567890123".
    */
	inline char* writeU64Len15To17Trim(char* buf, uint64_t sig) {
		bool lz; /* leading zero */
		uint32_t tz1, tz2, tz; /* trailing zero */

		uint32_t abbccddee = uint32_t(sig / 100000000);
		uint32_t ffgghhii = uint32_t(sig - uint64_t(abbccddee) * 100000000);
		uint32_t abbcc = abbccddee / 10000; /* (abbccddee / 10000) */
		uint32_t ddee = abbccddee - abbcc * 10000; /* (abbccddee % 10000) */
		uint32_t abb = uint32_t((uint64_t(abbcc) * 167773) >> 24); /* (abbcc / 100) */
		uint32_t a = (abb * 41) >> 12; /* (abb / 100) */
		uint32_t bb = abb - a * 100; /* (abb % 100) */
		uint32_t cc = abbcc - abb * 100; /* (abbcc % 100) */

		/* Write abbcc */
		buf[0] = uint8_t(a + '0');
		buf += a > 0;
		lz = bb < 10 && a == 0;
		JsonifierCore::memcpy(buf, charTable + (bb * 2ull + lz), 2ull);
		buf -= lz;
		JsonifierCore::memcpy(buf + 2ull, charTable + 2ull * cc, 2ull);

		if (ffgghhii) {
			uint32_t dd = (ddee * 5243) >> 19; /* (ddee / 100) */
			uint32_t ee = ddee - dd * 100; /* (ddee % 100) */
			uint32_t ffgg = uint32_t((uint64_t(ffgghhii) * 109951163) >> 40); /* (val / 10000) */
			uint32_t hhii = ffgghhii - ffgg * 10000; /* (val % 10000) */
			uint32_t ff = (ffgg * 5243) >> 19; /* (aabb / 100) */
			uint32_t gg = ffgg - ff * 100; /* (aabb % 100) */
			//((uint16_t *)buf)[2] = ((const uint16_t *)charTable)[dd];
			JsonifierCore::memcpy(buf + 4ull, charTable + 2ull * dd, 2ull);
			//((uint16_t *)buf)[3] = ((const uint16_t *)charTable)[ee];
			JsonifierCore::memcpy(buf + 6ull, charTable + 2ull * ee, 2ull);
			//((uint16_t *)buf)[4] = ((const uint16_t *)charTable)[ff];
			JsonifierCore::memcpy(buf + 8ull, charTable + 2ull * ff, 2ull);
			//((uint16_t *)buf)[5] = ((const uint16_t *)charTable)[gg];
			JsonifierCore::memcpy(buf + 10ull, charTable + 2ull * gg, 2ull);
			if (hhii) {
				uint32_t hh = (hhii * 5243) >> 19; /* (ccdd / 100) */
				uint32_t ii = hhii - hh * 100; /* (ccdd % 100) */
				//((uint16_t *)buf)[6] = ((const uint16_t *)charTable)[hh];
				JsonifierCore::memcpy(buf + 12ull, charTable + 2ull * hh, 2ull);
				//((uint16_t *)buf)[7] = ((const uint16_t *)charTable)[ii];
				JsonifierCore::memcpy(buf + 14ull, charTable + 2ull * ii, 2ull);
				tz1 = dec_trailing_zero_table[hh];
				tz2 = dec_trailing_zero_table[ii];
				tz = ii ? tz2 : (tz1 + 2);
				buf += 16 - tz;
				return buf;
			} else {
				tz1 = dec_trailing_zero_table[ff];
				tz2 = dec_trailing_zero_table[gg];
				tz = gg ? tz2 : (tz1 + 2);
				buf += 12 - tz;
				return buf;
			}
		} else {
			if (ddee) {
				uint32_t dd = (ddee * 5243) >> 19; /* (ddee / 100) */
				uint32_t ee = ddee - dd * 100; /* (ddee % 100) */
				//((uint16_t *)buf)[2] = ((const uint16_t *)charTable)[dd];
				JsonifierCore::memcpy(buf + 4ull, charTable + 2ull * dd, 2ull);
				//((uint16_t *)buf)[3] = ((const uint16_t *)charTable)[ee];
				JsonifierCore::memcpy(buf + 6ull, charTable + 2ull * ee, 2ull);
				tz1 = dec_trailing_zero_table[dd];
				tz2 = dec_trailing_zero_table[ee];
				tz = ee ? tz2 : (tz1 + 2);
				buf += 8 - tz;
				return buf;
			} else {
				tz1 = dec_trailing_zero_table[bb];
				tz2 = dec_trailing_zero_table[cc];
				tz = cc ? tz2 : (tz1 + tz2);
				buf += 4 - tz;
				return buf;
			}
		}
	}

	consteval uint32_t numbits(uint32_t x) {
		return x < 2 ? x : 1 + numbits(x >> 1);
	}

	template<typename OTy>
	requires std::same_as<OTy, float> || std::same_as<OTy, double>
	inline char* toChars(char* buffer, OTy val) noexcept {
		static_assert(std::numeric_limits<OTy>::is_iec559);
		static_assert(std::numeric_limits<OTy>::radix == 2);
		static_assert(std::is_same_v<float, OTy> || std::is_same_v<double, OTy>);
		static_assert(sizeof(float) == 4 && sizeof(double) == 8);
		using raw_t = std::conditional_t<std::is_same_v<float, OTy>, uint32_t, uint64_t>;

		raw_t raw;
		JsonifierCore::memcpy(&raw, &val, sizeof(OTy));

		/* decode from raw bytes from IEEE-754 double format. */
		constexpr uint32_t exponent_bits = numbits(std::numeric_limits<OTy>::max_exponent - std::numeric_limits<OTy>::min_exponent + 1);
		constexpr raw_t sig_mask = raw_t(-1) >> (exponent_bits + 1);
		bool sign = (raw >> (sizeof(OTy) * 8 - 1));
		uint64_t sig_raw = raw & sig_mask;
		int32_t exp_raw = raw << 1 >> (sizeof(raw_t) * 8 - exponent_bits);

		if (exp_raw == (uint32_t(1) << exponent_bits) - 1) [[unlikely]] {
			// NaN or Infinity
			JsonifierCore::memcpy(buffer, "null", 4);
			return buffer + 4;
		}
		if (sign) {
			*buffer = '-';
			++buffer;
		}
		if ((raw << 1) != 0) [[likely]] {
			uint64_t sig_bin;
			int32_t exp_bin;
			if (exp_raw == 0) [[unlikely]] {
				// subnormal
				sig_bin = sig_raw;
				exp_bin = 1 - (std::numeric_limits<OTy>::max_exponent - 1) - (std::numeric_limits<OTy>::digits - 1);
			} else {
				sig_bin = sig_raw | uint64_t(1ull << (std::numeric_limits<OTy>::digits - 1));
				exp_bin = int32_t(exp_raw) - (std::numeric_limits<OTy>::max_exponent - 1) - (std::numeric_limits<OTy>::digits - 1);
			}

			// if constexpr (std::same_as<OTy, float>) {
			//    constexpr auto shift = std::numeric_limits<double>::digits - std::numeric_limits<float>::digits;
			//    sig_bin <<= shift;
			//    exp_bin -= shift;
			// }

			/* binary to decimal */
			uint64_t sig_dec;
			int32_t exp_dec;
			f64BinToDec(sig_raw, exp_raw, sig_bin, exp_bin, &sig_dec, &exp_dec);
			if constexpr (std::same_as<OTy, float>) {
				sig_dec *= 100000000;
				exp_dec -= 8;
			}

			int32_t sig_len = 17;
			sig_len -= (sig_dec < 100000000ull * 100000000ull);
			sig_len -= (sig_dec < 100000000ull * 10000000ull);

			/* the decimal point position relative to the first digit */
			int32_t dot_pos = sig_len + exp_dec;

			if (-6 < dot_pos && dot_pos <= 21) {
				/* no need to Write exponent part */
				if (dot_pos <= 0) {
					auto num_hdr = buffer + (2 - dot_pos);
					auto num_end = writeU64Len15To17Trim(num_hdr, sig_dec);
					buffer[0] = '0';
					buffer[1] = '.';
					buffer += 2;
					for (; buffer < num_hdr; ++buffer)
						*buffer = '0';
					return num_end;
				} else {
					/* dot after first digit */
					/* such as 1.234, 1234.0, 123400000000000000000.0 */
					memset(buffer, '0', 8);
					memset(buffer + 8, '0', 8);
					memset(buffer + 16, '0', 8);
					auto num_hdr = buffer + 1;
					auto num_end = writeU64Len15To17Trim(num_hdr, sig_dec);
					for (int i = 0; i < dot_pos; i++)
						buffer[i] = buffer[i + 1];
					buffer[dot_pos] = '.';
					return ((num_end - num_hdr) <= dot_pos) ? buffer + dot_pos : num_end;
				}
			} else {
				/* Write with scientific notation */
				/* such as 1.234e56 */
				auto end = writeU64Len15To17Trim(buffer + 1, sig_dec);
				end -= (end == buffer + 2); /* remove '.0', e.g. 2.0e34 -> 2e34 */
				exp_dec += sig_len - 1;
				buffer[0] = buffer[1];
				buffer[1] = '.';
				end[0] = 'E';
				buffer = end + 1;
				buffer[0] = '-';
				buffer += exp_dec < 0;
				exp_dec = std::abs(exp_dec);
				if (exp_dec < 100) {
					uint32_t lz = exp_dec < 10;
					//*(uint16_t *)buffer = *(const uint16_t *)(charTable + (exp_dec * 2 + lz));
					JsonifierCore::memcpy(buffer, charTable + (exp_dec * 2 + lz), 2);
					return buffer + 2 - lz;
				} else {
					uint32_t hi = (uint32_t(exp_dec) * 656) >> 16; /* exp / 100 */
					uint32_t lo = uint32_t(exp_dec) - hi * 100; /* exp % 100 */
					buffer[0] = uint8_t(hi) + '0';
					JsonifierCore::memcpy(&buffer[1], charTable + (lo * 2), 2);
					return buffer + 3;
				}
			}
		} else [[unlikely]] {
			*buffer = '0';
			return buffer + 1;
		}
	}

}