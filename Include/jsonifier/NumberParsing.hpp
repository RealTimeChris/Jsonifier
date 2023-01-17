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
	License along with this library; if not, write to the Free Software
	Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301
	USA
*/
/// https://github.com/RealTimeChris/Jsonifier
/// Feb 3, 2023
#pragma once

#include <jsonifier/StringParsing.hpp>

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
	__forceinline uint64_t __emulu(uint32_t x, uint32_t y) {
		return x * ( uint64_t )y;
	}
	__forceinline uint64_t _umul128(uint64_t ab, uint64_t cd, uint64_t* hi) {
		uint64_t ad = __emulu(( uint32_t )(ab >> 32), ( uint32_t )cd);
		uint64_t bd = __emulu(( uint32_t )ab, ( uint32_t )cd);
		uint64_t adbc = ad + __emulu(( uint32_t )ab, ( uint32_t )(cd >> 32));
		uint64_t adbcCarry = !!(adbc < ad);
		uint64_t lo = bd + (adbc << 32);
		*hi = __emulu(( uint32_t )(ab >> 32), ( uint32_t )(cd >> 32)) + (adbc >> 32) + (adbcCarry << 32) + !!(lo < bd);
		return lo;
	}
#endif

	__forceinline Value128 fullMultiplication(uint64_t value1, uint64_t value2) {
		Value128 answer{};
		answer.low = _umul128(value1, value2, &answer.high);
		return answer;
	}

	__forceinline uint64_t round(Decimal& h) {
		if ((h.numDigits == 0) || (h.decimalPoint < 0)) {
			return 0;
		} else if (h.decimalPoint > 18) {
			return std::numeric_limits<uint64_t>::max();
		}
		uint32_t dp = uint32_t(h.decimalPoint);
		uint64_t n = 0;
		for (uint32_t i = 0; i < dp; i++) {
			n = (10 * n) + ((i < h.numDigits) ? h.digits[i] : 0);
		}
		bool roundUp = false;
		if (dp < h.numDigits) {
			roundUp = h.digits[dp] >= 5;
			if ((h.digits[dp] == 5) && (dp + 1 == h.numDigits)) {
				roundUp = h.truncated || ((dp > 0) && (1 & h.digits[dp - 1]));
			}
		}
		if (roundUp) {
			n++;
		}
		return n;
	}

	__forceinline void trim(Decimal& h) {
		while ((h.numDigits > 0) && (h.digits[h.numDigits - 1] == 0)) {
			h.numDigits--;
		}
	}

	__forceinline uint32_t numberOfDigitsDecimalLeftShift(Decimal& h, uint32_t shift) {
		shift &= 63;

		uint32_t xA = numberOfDigitsDecimalLeftShiftTableOne[shift];
		uint32_t xB = numberOfDigitsDecimalLeftShiftTableOne[shift + 1];
		uint32_t numNewDigits = xA >> 11;
		uint32_t pow5A = 0x7FF & xA;
		uint32_t pow5B = 0x7FF & xB;

		InStringPtr pow5 = &numberOfDigitsDecimalLeftShiftTableTwo[pow5A];

		uint32_t i = 0;
		uint32_t n = pow5B - pow5A;
		for (; i < n; i++) {
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

	__forceinline void decimalLeftShift(Decimal& h, uint32_t shift) {
		if (h.numDigits == 0) {
			return;
		}
		uint32_t numNewDigits = numberOfDigitsDecimalLeftShift(h, shift);
		int32_t readIndex = int32_t(h.numDigits - 1);
		uint32_t writeIndex = h.numDigits - 1 + numNewDigits;
		uint64_t n = 0;

		while (readIndex >= 0) {
			n += uint64_t(h.digits[readIndex]) << shift;
			uint64_t quotient = n / 10;
			uint64_t remainder = n - (10 * quotient);
			if (writeIndex < maxDigits) {
				h.digits[writeIndex] = uint8_t(remainder);
			} else if (remainder > 0) {
				h.truncated = true;
			}
			n = quotient;
			writeIndex--;
			readIndex--;
		}
		while (n > 0) {
			uint64_t quotient = n / 10;
			uint64_t remainder = n - (10 * quotient);
			if (writeIndex < maxDigits) {
				h.digits[writeIndex] = uint8_t(remainder);
			} else if (remainder > 0) {
				h.truncated = true;
			}
			n = quotient;
			writeIndex--;
		}
		h.numDigits += numNewDigits;
		if (h.numDigits > maxDigits) {
			h.numDigits = maxDigits;
		}
		h.decimalPoint += int32_t(numNewDigits);
		trim(h);
	}

	__forceinline void decimalRightShift(Decimal& h, uint32_t shift) {
		uint32_t readIndex = 0;
		uint32_t writeIndex = 0;

		uint64_t n = 0;

		while ((n >> shift) == 0) {
			if (readIndex < h.numDigits) {
				n = (10 * n) + h.digits[readIndex++];
			} else if (n == 0) {
				return;
			} else {
				while ((n >> shift) == 0) {
					n = 10 * n;
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
			uint8_t newDigit = uint8_t(n >> shift);
			n = (10 * (n & mask)) + h.digits[readIndex++];
			h.digits[writeIndex++] = newDigit;
		}
		while (n > 0) {
			uint8_t newDigit = uint8_t(n >> shift);
			n = 10 * (n & mask);
			if (writeIndex < maxDigits) {
				h.digits[writeIndex++] = newDigit;
			} else if (newDigit > 0) {
				h.truncated = true;
			}
		}
		h.numDigits = writeIndex;
		trim(h);
	}

	__forceinline bool isInteger(char c) noexcept {
		return (c >= '0' && c <= '9');
	}

	__forceinline double toDouble(uint64_t mantissa, uint64_t realExponent, bool negative) {
		double d;
		mantissa &= ~(1ull << 52);
		mantissa |= realExponent << 52;
		mantissa |= ((static_cast<uint64_t>(negative)) << 63);
		std::memcpy(&d, &mantissa, sizeof(d));
		return d;
	}

	__forceinline AdjustedMantissa computeFloat(Decimal& d) {
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
			uint32_t n = uint32_t(d.decimalPoint);
			uint32_t shift = (n < numPowers) ? powers[n] : maxShift;
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
				uint32_t n = uint32_t(-d.decimalPoint);
				shift = (n < numPowers) ? powers[n] : maxShift;
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
			uint32_t n = uint32_t((minimumExponentNew + 1) - exp2);
			if (n > maxShift) {
				n = maxShift;
			}
			decimalRightShift(d, n);
			exp2 += int32_t(n);
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

	__forceinline Decimal parseDecimal(const char*& p) noexcept {
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

	__forceinline bool computeFloat64(int64_t power, uint64_t i, bool negative, double& d) {
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

	__forceinline AdjustedMantissa parseLongMantissa(const char* first) {
		Decimal d = parseDecimal(first);
		return computeFloat(d);
	}

	__forceinline bool isNegative(InStringPtr src) noexcept {
		return (*src == '-');
	}

	__forceinline double fromChars(const char* first) noexcept {
		bool negative = first[0] == '-';
		if (negative) {
			first++;
		}
		AdjustedMantissa am = parseLongMantissa(first);
		uint64_t word = am.mantissa;
		word |= uint64_t(am.power2) << mantissaExplicitBits;
		word = negative ? word | (uint64_t(1) << signIndex) : word;
		double value;
		std::memcpy(&value, &word, sizeof(double));
		return value;
	}

	__forceinline bool parseFloatFallback(InStringPtr ptr, double* outDouble) {
		*outDouble = fromChars(reinterpret_cast<const char*>(ptr));
		return !(*outDouble > (std::numeric_limits<double>::max)() || *outDouble < std::numeric_limits<double>::lowest());
	}

	template<typename I> __forceinline bool parseDigit(const uint8_t c, I& i) {
		const uint8_t digit = static_cast<uint8_t>(c - '0');
		if (digit > 9) {
			return false;
		}
		i = 10 * i + digit;
		return true;
	}

	__forceinline JsonifierResult<uint64_t> parseUnsigned(InStringPtr const src) noexcept {
		InStringPtr p = src;
		InStringPtr const startDigits = p;
		uint64_t i = 0;
		while (parseDigit(*p, i)) {
			p++;
		}

		size_t digitCount = size_t(p - startDigits);
		if ((digitCount == 0) || (digitCount > 20)) {
			return Incorrect_Type;
		}
		if (('0' == *startDigits) && (digitCount > 1)) {
			return Number_Error;
		}
		if (integerStringFinisher[*p] != Success) {
			return static_cast<ErrorCode>(integerStringFinisher[*p]);
		}

		if (digitCount == 20) {
			if (src[0] != uint8_t('1') || i <= uint64_t(std::numeric_limits<int64_t>::max())) {
				return Incorrect_Type;
			}
		}

		return i;
	}

	__forceinline JsonifierResult<int64_t> parseInteger(InStringPtr src) noexcept {
		bool negative = (*src == '-');
		InStringPtr p = src + uint8_t(negative);
		InStringPtr const startDigits = p;
		uint64_t i = 0;
		while (parseDigit(*p, i)) {
			p++;
		}

		size_t digitCount = size_t(p - startDigits);
		size_t longestDigitCount = 19;
		if ((digitCount == 0) || (digitCount > longestDigitCount)) {
			return Incorrect_Type;
		}
		if (('0' == *startDigits) && (digitCount > 1)) {
			return Number_Error;
		}
		if (integerStringFinisher[*p] != Success) {
			return Number_Error;
		}
		if (i > uint64_t(std::numeric_limits<int64_t>::max()) + uint64_t(negative)) {
			return Incorrect_Type;
		}
		return negative ? (~i + 1) : i;
	}

	__forceinline JsonifierResult<double> parseDouble(InStringPtr src) noexcept {
		bool negative = (*src == '-');
		src += uint8_t(negative);
		uint64_t i = 0;
		InStringPtr p = src;
		p += parseDigit(*p, i);
		bool leadingZero = (i == 0);
		while (parseDigit(*p, i)) {
			p++;
		}
		if (p == src) {
			return Incorrect_Type;
		}
		if ((leadingZero && p != src + 1)) {
			return Number_Error;
		}
		int64_t exponent = 0;
		bool overflow;
		if (*p == '.') {
			p++;
			InStringPtr startDecimalDigits = p;
			if (!parseDigit(*p, i)) {
				return Number_Error;
			}
			p++;
			while (parseDigit(*p, i)) {
				p++;
			}
			exponent = -(p - startDecimalDigits);
			overflow = p - src - 1 > 19;
			if (overflow && leadingZero) {
				InStringPtr startDigits = src + 2;
				while (*startDigits == '0') {
					startDigits++;
				}
				overflow = startDigits - src > 19;
			}
		} else {
			overflow = p - src > 19;
		}
		if (*p == 'e' || *p == 'E') {
			p++;
			bool expNeg = *p == '-';
			p += expNeg || *p == '+';

			uint64_t exp = 0;
			InStringPtr startExpDigits = p;
			while (parseDigit(*p, exp)) {
				p++;
			}
			if (p - startExpDigits == 0 || p - startExpDigits > 19) {
				return Number_Error;
			}

			exponent += expNeg ? 0 - exp : exp;
		}

		if (isNotStructuralOrWhitespace(*p)) {
			return Number_Error;
		}

		overflow = overflow || exponent < smallestPower || exponent > largestPower;
		double d;
		if (!overflow) {
			if (computeFloat64(exponent, i, negative, d)) {
				return d;
			}
		}
		if (!parseFloatFallback(src - uint8_t(negative), &d)) {
			return Number_Error;
		}
		return d;
	}
}