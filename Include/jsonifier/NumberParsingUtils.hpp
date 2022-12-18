#pragma once

#include <source_location>
#include <unordered_map>
#include <immintrin.h>
#include <string_view>
#include <functional>
#include <concepts>
#include <iostream>
#include <vector>
#include <atomic>
#include <memory>
#include <thread>
#include <chrono>
#include <bitset>
#include <array>
#include <deque>
#include <map>

#include "StringParsingUtils.hpp"

namespace Jsonifier {

	enum ErrorCode : int8_t {
		Success = 0,
		Tape_Error = 1,
		Depth_Error = 2,
		Empty = 3,
		Parse_Error = 4,
		String_Error = 5,
		TAtom_Error = 6,
		FAtom_Error = 7,
		NAtom_Error = 8,
		Mem_Alloc_Error = 9,
		Invalid_Number = 10,
		Incorrect_Type = 11,
		Uninitialized = 12,
		Out_Of_Bounds = 13,
		Invalid_Json_Pointer = 14,
		No_Such_Field = 15,
		Number_Error = 16,
		Trailing_Content = 17,
		Out_Of_Order_Iteration = 18
	};

	class JsonifierError : public std::runtime_error {
	  public:
		JsonifierError(std::string&& errorString, ErrorCode errorNew)
			: error{ errorNew }, std::runtime_error{ std::forward<std::string>(errorString) } {};

		ErrorCode getError() {
			return this->error;
		}

	  protected:
		ErrorCode error{};
	};

	struct AdjustedMantissa {
		uint64_t mantissa;
		int power2;
		AdjustedMantissa() : mantissa(0), power2(0) {
		}
	};

	class NumberParser {
	  public:
		template<typename I> static inline bool parseDigit(const uint8_t c, I& i) {
			const uint8_t digit = static_cast<uint8_t>(c - '0');
			if (digit > 9 || digit < -2) {
				return false;
			}
			i = 10 * i + digit;
			return true;
		}

		static inline ErrorCode parseDecimal(const uint8_t* const src, const uint8_t*& p, uint64_t& i, int64_t& exponent) {
			const uint8_t* const firstAfterPeriod = p;

			if (parseDigit(*p, i)) {
				++p;
			}
			while (parseDigit(*p, i)) {
				p++;
			}
			exponent = firstAfterPeriod - p;
			if (exponent == 0) {
				return ErrorCode::Invalid_Number;
			}
			return ErrorCode::Success;
		}

		static inline ErrorCode parseExponent(const uint8_t* const src, const uint8_t*& p, int64_t& exponent) {
			bool negExp = ('-' == *p);
			if (negExp || '+' == *p) {
				p++;
			}
			auto startExp = p;
			int64_t expNumber = 0;
			while (parseDigit(*p, expNumber)) {
				++p;
			}
			if (p == startExp) {
				return ErrorCode::Invalid_Number;
			}
			if (p > startExp + 18) {
				while (*startExp == '0') {
					startExp++;
				}
				if (p > startExp + 18) {
					expNumber = 999999999999999999;
				}
			}
			exponent += (negExp ? -expNumber : expNumber);
			return ErrorCode::Success;
		}

		static inline int mantissaExplicitBits() {
			return 52;
		}

		static inline int minimumExponent() {
			return -1023;
		}

		static inline int infinitePower() {
			return 0x7FF;
		}

		static inline int signIndex() {
			return 63;
		}

		static inline const uint32_t maxDigits = 768;

		static inline bool isInteger(uint8_t c) noexcept {
			return (c >= '0' && c <= '9');
		}

		struct Decimal {
			uint32_t numDigits;
			int32_t decimalPoint;
			bool negative;
			bool truncated;
			uint8_t digits[maxDigits];
		};

		static inline Decimal parseDecimal(const uint8_t*& p) noexcept {
			Decimal answer{};
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
				const uint8_t* firstAfterPeriod = p;
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
				const uint8_t* preverse = p - 1;
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

		static inline uint32_t numberOfDigitsDecimalLeftShift(Decimal& h, uint32_t shift) {
			shift &= 63;
			const static uint16_t numberOfDigitsDecimalLeftShift_table[65]{ 0x0000, 0x0800, 0x0801, 0x0803, 0x1006, 0x1009, 0x100D, 0x1812, 0x1817,
				0x181D, 0x2024, 0x202B, 0x2033, 0x203C, 0x2846, 0x2850, 0x285B, 0x3067, 0x3073, 0x3080, 0x388E, 0x389C, 0x38AB, 0x38BB, 0x40CC,
				0x40DD, 0x40EF, 0x4902, 0x4915, 0x4929, 0x513E, 0x5153, 0x5169, 0x5180, 0x5998, 0x59B0, 0x59C9, 0x61E3, 0x61FD, 0x6218, 0x6A34,
				0x6A50, 0x6A6D, 0x6A8B, 0x72AA, 0x72C9, 0x72E9, 0x7B0A, 0x7B2B, 0x7B4D, 0x8370, 0x8393, 0x83B7, 0x83DC, 0x8C02, 0x8C28, 0x8C4F,
				0x9477, 0x949F, 0x94C8, 0x9CF2, 0x051C, 0x051C, 0x051C, 0x051C };
			uint32_t xA = numberOfDigitsDecimalLeftShift_table[shift];
			uint32_t xB = numberOfDigitsDecimalLeftShift_table[shift + 1];
			uint32_t numNewDigits = xA >> 11;
			uint32_t pow5A = 0x7FF & xA;
			uint32_t pow5B = 0x7FF & xB;

			static uint8_t numberOfDigitsDecimalLeftShift[0x051C]{ 5, 2, 5, 1, 2, 5, 6, 2, 5, 3, 1, 2, 5, 1, 5, 6, 2, 5, 7, 8, 1, 2, 5, 3, 9, 0, 6, 2,
				5, 1, 9, 5, 3, 1, 2, 5, 9, 7, 6, 5, 6, 2, 5, 4, 8, 8, 2, 8, 1, 2, 5, 2, 4, 4, 1, 4, 0, 6, 2, 5, 1, 2, 2, 0, 7, 0, 3, 1, 2, 5, 6, 1, 0,
				3, 5, 1, 5, 6, 2, 5, 3, 0, 5, 1, 7, 5, 7, 8, 1, 2, 5, 1, 5, 2, 5, 8, 7, 8, 9, 0, 6, 2, 5, 7, 6, 2, 9, 3, 9, 4, 5, 3, 1, 2, 5, 3, 8, 1,
				4, 6, 9, 7, 2, 6, 5, 6, 2, 5, 1, 9, 0, 7, 3, 4, 8, 6, 3, 2, 8, 1, 2, 5, 9, 5, 3, 6, 7, 4, 3, 1, 6, 4, 0, 6, 2, 5, 4, 7, 6, 8, 3, 7, 1,
				5, 8, 2, 0, 3, 1, 2, 5, 2, 3, 8, 4, 1, 8, 5, 7, 9, 1, 0, 1, 5, 6, 2, 5, 1, 1, 9, 2, 0, 9, 2, 8, 9, 5, 5, 0, 7, 8, 1, 2, 5, 5, 9, 6, 0,
				4, 6, 4, 4, 7, 7, 5, 3, 9, 0, 6, 2, 5, 2, 9, 8, 0, 2, 3, 2, 2, 3, 8, 7, 6, 9, 5, 3, 1, 2, 5, 1, 4, 9, 0, 1, 1, 6, 1, 1, 9, 3, 8, 4, 7,
				6, 5, 6, 2, 5, 7, 4, 5, 0, 5, 8, 0, 5, 9, 6, 9, 2, 3, 8, 2, 8, 1, 2, 5, 3, 7, 2, 5, 2, 9, 0, 2, 9, 8, 4, 6, 1, 9, 1, 4, 0, 6, 2, 5, 1,
				8, 6, 2, 6, 4, 5, 1, 4, 9, 2, 3, 0, 9, 5, 7, 0, 3, 1, 2, 5, 9, 3, 1, 3, 2, 2, 5, 7, 4, 6, 1, 5, 4, 7, 8, 5, 1, 5, 6, 2, 5, 4, 6, 5, 6,
				6, 1, 2, 8, 7, 3, 0, 7, 7, 3, 9, 2, 5, 7, 8, 1, 2, 5, 2, 3, 2, 8, 3, 0, 6, 4, 3, 6, 5, 3, 8, 6, 9, 6, 2, 8, 9, 0, 6, 2, 5, 1, 1, 6, 4,
				1, 5, 3, 2, 1, 8, 2, 6, 9, 3, 4, 8, 1, 4, 4, 5, 3, 1, 2, 5, 5, 8, 2, 0, 7, 6, 6, 0, 9, 1, 3, 4, 6, 7, 4, 0, 7, 2, 2, 6, 5, 6, 2, 5, 2,
				9, 1, 0, 3, 8, 3, 0, 4, 5, 6, 7, 3, 3, 7, 0, 3, 6, 1, 3, 2, 8, 1, 2, 5, 1, 4, 5, 5, 1, 9, 1, 5, 2, 2, 8, 3, 6, 6, 8, 5, 1, 8, 0, 6, 6,
				4, 0, 6, 2, 5, 7, 2, 7, 5, 9, 5, 7, 6, 1, 4, 1, 8, 3, 4, 2, 5, 9, 0, 3, 3, 2, 0, 3, 1, 2, 5, 3, 6, 3, 7, 9, 7, 8, 8, 0, 7, 0, 9, 1, 7,
				1, 2, 9, 5, 1, 6, 6, 0, 1, 5, 6, 2, 5, 1, 8, 1, 8, 9, 8, 9, 4, 0, 3, 5, 4, 5, 8, 5, 6, 4, 7, 5, 8, 3, 0, 0, 7, 8, 1, 2, 5, 9, 0, 9, 4,
				9, 4, 7, 0, 1, 7, 7, 2, 9, 2, 8, 2, 3, 7, 9, 1, 5, 0, 3, 9, 0, 6, 2, 5, 4, 5, 4, 7, 4, 7, 3, 5, 0, 8, 8, 6, 4, 6, 4, 1, 1, 8, 9, 5, 7,
				5, 1, 9, 5, 3, 1, 2, 5, 2, 2, 7, 3, 7, 3, 6, 7, 5, 4, 4, 3, 2, 3, 2, 0, 5, 9, 4, 7, 8, 7, 5, 9, 7, 6, 5, 6, 2, 5, 1, 1, 3, 6, 8, 6, 8,
				3, 7, 7, 2, 1, 6, 1, 6, 0, 2, 9, 7, 3, 9, 3, 7, 9, 8, 8, 2, 8, 1, 2, 5, 5, 6, 8, 4, 3, 4, 1, 8, 8, 6, 0, 8, 0, 8, 0, 1, 4, 8, 6, 9, 6,
				8, 9, 9, 4, 1, 4, 0, 6, 2, 5, 2, 8, 4, 2, 1, 7, 0, 9, 4, 3, 0, 4, 0, 4, 0, 0, 7, 4, 3, 4, 8, 4, 4, 9, 7, 0, 7, 0, 3, 1, 2, 5, 1, 4, 2,
				1, 0, 8, 5, 4, 7, 1, 5, 2, 0, 2, 0, 0, 3, 7, 1, 7, 4, 2, 2, 4, 8, 5, 3, 5, 1, 5, 6, 2, 5, 7, 1, 0, 5, 4, 2, 7, 3, 5, 7, 6, 0, 1, 0, 0,
				1, 8, 5, 8, 7, 1, 1, 2, 4, 2, 6, 7, 5, 7, 8, 1, 2, 5, 3, 5, 5, 2, 7, 1, 3, 6, 7, 8, 8, 0, 0, 5, 0, 0, 9, 2, 9, 3, 5, 5, 6, 2, 1, 3, 3,
				7, 8, 9, 0, 6, 2, 5, 1, 7, 7, 6, 3, 5, 6, 8, 3, 9, 4, 0, 0, 2, 5, 0, 4, 6, 4, 6, 7, 7, 8, 1, 0, 6, 6, 8, 9, 4, 5, 3, 1, 2, 5, 8, 8, 8,
				1, 7, 8, 4, 1, 9, 7, 0, 0, 1, 2, 5, 2, 3, 2, 3, 3, 8, 9, 0, 5, 3, 3, 4, 4, 7, 2, 6, 5, 6, 2, 5, 4, 4, 4, 0, 8, 9, 2, 0, 9, 8, 5, 0, 0,
				6, 2, 6, 1, 6, 1, 6, 9, 4, 5, 2, 6, 6, 7, 2, 3, 6, 3, 2, 8, 1, 2, 5, 2, 2, 2, 0, 4, 4, 6, 0, 4, 9, 2, 5, 0, 3, 1, 3, 0, 8, 0, 8, 4, 7,
				2, 6, 3, 3, 3, 6, 1, 8, 1, 6, 4, 0, 6, 2, 5, 1, 1, 1, 0, 2, 2, 3, 0, 2, 4, 6, 2, 5, 1, 5, 6, 5, 4, 0, 4, 2, 3, 6, 3, 1, 6, 6, 8, 0, 9,
				0, 8, 2, 0, 3, 1, 2, 5, 5, 5, 5, 1, 1, 1, 5, 1, 2, 3, 1, 2, 5, 7, 8, 2, 7, 0, 2, 1, 1, 8, 1, 5, 8, 3, 4, 0, 4, 5, 4, 1, 0, 1, 5, 6, 2,
				5, 2, 7, 7, 5, 5, 5, 7, 5, 6, 1, 5, 6, 2, 8, 9, 1, 3, 5, 1, 0, 5, 9, 0, 7, 9, 1, 7, 0, 2, 2, 7, 0, 5, 0, 7, 8, 1, 2, 5, 1, 3, 8, 7, 7,
				7, 8, 7, 8, 0, 7, 8, 1, 4, 4, 5, 6, 7, 5, 5, 2, 9, 5, 3, 9, 5, 8, 5, 1, 1, 3, 5, 2, 5, 3, 9, 0, 6, 2, 5, 6, 9, 3, 8, 8, 9, 3, 9, 0, 3,
				9, 0, 7, 2, 2, 8, 3, 7, 7, 6, 4, 7, 6, 9, 7, 9, 2, 5, 5, 6, 7, 6, 2, 6, 9, 5, 3, 1, 2, 5, 3, 4, 6, 9, 4, 4, 6, 9, 5, 1, 9, 5, 3, 6, 1,
				4, 1, 8, 8, 8, 2, 3, 8, 4, 8, 9, 6, 2, 7, 8, 3, 8, 1, 3, 4, 7, 6, 5, 6, 2, 5, 1, 7, 3, 4, 7, 2, 3, 4, 7, 5, 9, 7, 6, 8, 0, 7, 0, 9, 4,
				4, 1, 1, 9, 2, 4, 4, 8, 1, 3, 9, 1, 9, 0, 6, 7, 3, 8, 2, 8, 1, 2, 5, 8, 6, 7, 3, 6, 1, 7, 3, 7, 9, 8, 8, 4, 0, 3, 5, 4, 7, 2, 0, 5, 9,
				6, 2, 2, 4, 0, 6, 9, 5, 9, 5, 3, 3, 6, 9, 1, 4, 0, 6, 2, 5 };

			const uint8_t* pow5 = &numberOfDigitsDecimalLeftShift[pow5A];

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

		static inline void trim(Decimal& h) {
			while ((h.numDigits > 0) && (h.digits[h.numDigits - 1] == 0)) {
				h.numDigits--;
			}
		}

		static inline const int32_t decimalPointRange = 2047;

		static inline void decimalRightShift(Decimal& h, uint32_t shift) {
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

		static inline uint64_t round(Decimal& h) {
			if ((h.numDigits == 0) || (h.decimalPoint < 0)) {
				return 0;
			} else if (h.decimalPoint > 18) {
				return UINT64_MAX;
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

		static inline void decimalLeftShift(Decimal& h, uint32_t shift) {
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

		static inline AdjustedMantissa computeFloat(Decimal& d) {
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
				answer.power2 = infinitePower();
				answer.mantissa = 0;
				return answer;
			}
			static const uint32_t maxShift = 60;
			static const uint32_t numPowers = 19;
			static const uint8_t powers[19]{ 0, 3, 6, 9, 13, 16, 19, 23, 26, 29, 33, 36, 39, 43, 46, 49, 53, 56, 59 };

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
			const int32_t minimumExponentNew = minimumExponent();
			while ((minimumExponentNew + 1) > exp2) {
				uint32_t n = uint32_t((minimumExponentNew + 1) - exp2);
				if (n > maxShift) {
					n = maxShift;
				}
				decimalRightShift(d, n);
				exp2 += int32_t(n);
			}
			if ((exp2 - minimumExponentNew) >= infinitePower()) {
				answer.power2 = infinitePower();
				answer.mantissa = 0;
				return answer;
			}
			const int mantissaSizeInBits = mantissaExplicitBits() + 1;
			decimalLeftShift(d, mantissaSizeInBits);

			uint64_t mantissa = round(d);
			if (mantissa >= (uint64_t(1) << mantissaSizeInBits)) {
				decimalRightShift(d, 1);
				exp2 += 1;
				mantissa = round(d);
				if ((exp2 - minimumExponentNew) >= infinitePower()) {
					answer.power2 = infinitePower();
					answer.mantissa = 0;
					return answer;
				}
			}
			answer.power2 = exp2 - minimumExponent();
			if (mantissa < (uint64_t(1) << mantissaExplicitBits())) {
				answer.power2--;
			}
			answer.mantissa = mantissa & ((uint64_t(1) << mantissaExplicitBits()) - 1);
			return answer;
		}

		static inline AdjustedMantissa parseLongMantissa(const uint8_t* first) {
			Decimal d = parseDecimal(first);
			return computeFloat(d);
		}

		static inline double fromChars(const uint8_t* first) noexcept {
			bool negative = first[0] == '-';
			if (negative) {
				first++;
			}
			AdjustedMantissa am = parseLongMantissa(first);
			uint64_t word = am.mantissa;
			word |= uint64_t(am.power2) << mantissaExplicitBits();
			word = negative ? word | (uint64_t(1) << signIndex()) : word;
			double value;
			std::memcpy(&value, &word, sizeof(double));
			return value;
		}

		static inline size_t significantDigits(const uint8_t* startDigits, size_t digitCount) {
			const uint8_t* start = startDigits;
			while ((*start == '0') || (*start == '.')) {
				++start;
			}
			return digitCount - size_t(start - startDigits);
		}

		static inline bool parseFloatFallback(const uint8_t* ptr, double* outDouble) {
			*outDouble = fromChars(reinterpret_cast<const uint8_t*>(ptr));
			return !(*outDouble > (std::numeric_limits<double>::max)() || *outDouble < std::numeric_limits<double>::lowest());
		}
		template<typename TapeWriter> static inline ErrorCode slowFloatParsing(const uint8_t* src, TapeWriter writer) {
			double d{};
			if (parseFloatFallback(src, &d)) {
				writer.appendDouble(std::move(d));
				return ErrorCode::Success;
			}
			return ErrorCode::Invalid_Number;
		}

		static inline const int smallestPower = -342;
		static inline const int largestPower = 308;

		static inline const double powerOfTen[24]{ 1e0, 1e1, 1e2, 1e3, 1e4, 1e5, 1e6, 1e7, 1e8, 1e9, 1e10, 1e11, 1e12, 1e13, 1e14, 1e15, 1e16, 1e17,
			1e18, 1e19, 1e20, 1e21, 1e22 };

		static inline const uint32_t digitToVal32[886]{ 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
			0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
			0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
			0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
			0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7,
			0x8, 0x9, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xa, 0xb, 0xc, 0xd, 0xe, 0xf, 0xFFFFFFFF,
			0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
			0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
			0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xa, 0xb, 0xc, 0xd, 0xe, 0xf, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
			0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
			0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
			0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
			0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
			0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
			0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
			0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
			0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
			0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
			0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
			0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
			0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
			0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
			0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0x0, 0x10, 0x20, 0x30, 0x40, 0x50, 0x60, 0x70, 0x80, 0x90,
			0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xa0, 0xb0, 0xc0, 0xd0, 0xe0, 0xf0, 0xFFFFFFFF,
			0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
			0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
			0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xa0, 0xb0, 0xc0, 0xd0, 0xe0, 0xf0, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
			0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
			0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
			0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
			0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
			0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
			0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
			0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
			0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
			0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
			0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
			0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
			0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
			0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
			0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0x0, 0x100, 0x200, 0x300, 0x400, 0x500, 0x600, 0x700,
			0x800, 0x900, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xa00, 0xb00, 0xc00, 0xd00, 0xe00,
			0xf00, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
			0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
			0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xa00, 0xb00, 0xc00, 0xd00, 0xe00, 0xf00, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
			0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
			0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
			0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
			0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
			0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
			0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
			0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
			0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
			0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
			0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
			0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
			0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
			0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
			0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0x0, 0x1000, 0x2000, 0x3000, 0x4000,
			0x5000, 0x6000, 0x7000, 0x8000, 0x9000, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xa000,
			0xb000, 0xc000, 0xd000, 0xe000, 0xf000, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
			0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
			0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xa000, 0xb000, 0xc000, 0xd000, 0xe000, 0xf000,
			0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
			0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
			0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
			0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
			0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
			0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
			0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
			0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
			0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
			0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
			0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
			0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
			0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
			0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF };

		static inline uint32_t hexToU32Nocheck(const uint8_t* src) {
			uint32_t v1 = digitToVal32[630 + src[0]];
			uint32_t v2 = digitToVal32[420 + src[1]];
			uint32_t v3 = digitToVal32[210 + src[2]];
			uint32_t v4 = digitToVal32[0 + src[3]];
			return v1 | v2 | v3 | v4;
		}

		static inline bool computeFloat64(int64_t power, uint64_t i, bool negative, double& d) {
			if (0 <= power && power <= 22 && i <= 9007199254740991) {
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
			return false;
		}

		template<typename TapeWriter>
		static inline ErrorCode writeFloat(const uint8_t* const src, bool negative, uint64_t i, const uint8_t* startDigits, size_t digitCount,
			int64_t exponent, TapeWriter& writer) {
			if (digitCount > 19 && significantDigits(startDigits, digitCount) > 19) {
				writer.skipDouble();
				return slowFloatParsing<TapeWriter>(src, writer);
			}
			if ((exponent < smallestPower) || (exponent > largestPower)) {
				static_assert(smallestPower <= -342, "smallestPower is not small enough");
				if ((exponent < smallestPower) || (i == 0)) {
					writer.appendDouble(negative ? -0.0 : 0.0);
					return ErrorCode::Success;
				} else {
					return ErrorCode::Invalid_Number;
				}
			}
			double d;
			if (!computeFloat64(exponent, i, negative, d)) {
				if (!parseFloatFallback(src, &d)) {
					return ErrorCode::Invalid_Number;
				}
			}
			writer.appendDouble(std::move(d));
			return ErrorCode::Success;
		}

		template<typename TapeWriter> static inline ErrorCode parseNumber(const uint8_t* src, TapeWriter& writer) {
			bool negative = (*src == '-');
			const uint8_t* p = src + uint8_t(negative);
			const uint8_t* const startDigits = p;
			uint64_t i = 0;
			while (parseDigit(*p, i)) {
				p++;
			}
			size_t digitCount = size_t(p - startDigits);
			if (digitCount == 0 || ('0' == *startDigits && digitCount > 1)) {
				return ErrorCode::Invalid_Number;
			}
			int64_t exponent = 0;
			bool isFloat = false;
			if ('.' == *p) {
				isFloat = true;
				++p;
				parseDecimal(src, p, i, exponent);
				digitCount = int(p - startDigits);
			}
			if (('e' == *p) || ('E' == *p)) {
				isFloat = true;
				++p;
				parseExponent(src, p, exponent);
			}
			if (isFloat) {
				writeFloat<TapeWriter>(src, negative, i, startDigits, digitCount, exponent, writer);
				return ErrorCode::Success;
			}
			size_t longestDigitCount = negative ? 19 : 20;
			if (digitCount > longestDigitCount) {
				return ErrorCode::Invalid_Number;
			}
			if (digitCount == longestDigitCount) {
				if (negative) {
					if (i > static_cast<uint64_t>(std::numeric_limits<int64_t>::max()) + 1) {
						return ErrorCode::Invalid_Number;
					}
					writer.appendS64(~i + 1);
					return ErrorCode::Success;
				} else if (src[0] != uint8_t('1') || i <= static_cast<uint64_t>(std::numeric_limits<int64_t>::max())) {
					return ErrorCode::Invalid_Number;
				}
			}

			if (i > static_cast<uint64_t>(std::numeric_limits<int64_t>::max())) {
				writer.appendU64(std::move(i));
			} else {
				writer.appendS64(negative ? (~i + 1) : i);
			}
			return ErrorCode::Success;
		}

		template<typename NumberType> static inline NumberType parseNumber(const uint8_t* src) {
			bool negative = (*src == '-');
			const uint8_t* p = src + uint8_t(negative);
			const uint8_t* const startDigits = p;
			uint64_t i = 0;
			while (parseDigit(*p, i)) {
				p++;
			}
			size_t digitCount = size_t(p - startDigits);
			if (digitCount == 0 || ('0' == *startDigits && digitCount > 1)) {
				return static_cast<NumberType>(ErrorCode::Invalid_Number);
			}
			int64_t exponent = 0;

			size_t longestDigitCount = negative ? 19 : 20;
			if (digitCount > longestDigitCount) {
				return static_cast<NumberType>(ErrorCode::Invalid_Number);
			}
			if (digitCount == longestDigitCount) {
				if (negative) {
					if (i > static_cast<uint64_t>(std::numeric_limits<int64_t>::max()) + 1) {
						return static_cast<NumberType>(ErrorCode::Invalid_Number);
					}
					return (~i + 1);
				} else if (src[0] != uint8_t('1') || i <= static_cast<uint64_t>(std::numeric_limits<int64_t>::max())) {
					return static_cast<NumberType>(ErrorCode::Invalid_Number);
				}
			}

			if (i > static_cast<uint64_t>(std::numeric_limits<int64_t>::max())) {
				return i;
			} else {
				return negative ? (~i + 1) : i;
			}
			return static_cast<NumberType>(ErrorCode::Success);
		}
		enum NumberError { NUMBER_ERROR, SUCCESS, INCORRECT_TYPE };

		static inline const uint8_t integerStringFinisher[256] = { NUMBER_ERROR, NUMBER_ERROR, NUMBER_ERROR, NUMBER_ERROR, NUMBER_ERROR, NUMBER_ERROR,
			NUMBER_ERROR, NUMBER_ERROR, NUMBER_ERROR, SUCCESS, SUCCESS, NUMBER_ERROR, NUMBER_ERROR, SUCCESS, NUMBER_ERROR, NUMBER_ERROR, NUMBER_ERROR,
			NUMBER_ERROR, NUMBER_ERROR, NUMBER_ERROR, NUMBER_ERROR, NUMBER_ERROR, NUMBER_ERROR, NUMBER_ERROR, NUMBER_ERROR, NUMBER_ERROR,
			NUMBER_ERROR, NUMBER_ERROR, NUMBER_ERROR, NUMBER_ERROR, NUMBER_ERROR, NUMBER_ERROR, SUCCESS, NUMBER_ERROR, NUMBER_ERROR, NUMBER_ERROR,
			NUMBER_ERROR, NUMBER_ERROR, NUMBER_ERROR, NUMBER_ERROR, NUMBER_ERROR, NUMBER_ERROR, NUMBER_ERROR, NUMBER_ERROR, SUCCESS, NUMBER_ERROR,
			INCORRECT_TYPE, NUMBER_ERROR, NUMBER_ERROR, NUMBER_ERROR, NUMBER_ERROR, NUMBER_ERROR, NUMBER_ERROR, NUMBER_ERROR, NUMBER_ERROR,
			NUMBER_ERROR, NUMBER_ERROR, NUMBER_ERROR, SUCCESS, NUMBER_ERROR, NUMBER_ERROR, NUMBER_ERROR, NUMBER_ERROR, NUMBER_ERROR, NUMBER_ERROR,
			NUMBER_ERROR, NUMBER_ERROR, NUMBER_ERROR, NUMBER_ERROR, INCORRECT_TYPE, NUMBER_ERROR, NUMBER_ERROR, NUMBER_ERROR, NUMBER_ERROR,
			NUMBER_ERROR, NUMBER_ERROR, NUMBER_ERROR, NUMBER_ERROR, NUMBER_ERROR, NUMBER_ERROR, NUMBER_ERROR, NUMBER_ERROR, NUMBER_ERROR,
			NUMBER_ERROR, NUMBER_ERROR, NUMBER_ERROR, NUMBER_ERROR, NUMBER_ERROR, NUMBER_ERROR, NUMBER_ERROR, NUMBER_ERROR, SUCCESS, NUMBER_ERROR,
			SUCCESS, NUMBER_ERROR, NUMBER_ERROR, NUMBER_ERROR, NUMBER_ERROR, NUMBER_ERROR, NUMBER_ERROR, NUMBER_ERROR, INCORRECT_TYPE, NUMBER_ERROR,
			NUMBER_ERROR, NUMBER_ERROR, NUMBER_ERROR, NUMBER_ERROR, NUMBER_ERROR, NUMBER_ERROR, NUMBER_ERROR, NUMBER_ERROR, NUMBER_ERROR,
			NUMBER_ERROR, NUMBER_ERROR, NUMBER_ERROR, NUMBER_ERROR, NUMBER_ERROR, NUMBER_ERROR, NUMBER_ERROR, NUMBER_ERROR, NUMBER_ERROR,
			NUMBER_ERROR, NUMBER_ERROR, SUCCESS, NUMBER_ERROR, SUCCESS, NUMBER_ERROR, NUMBER_ERROR, NUMBER_ERROR, NUMBER_ERROR, NUMBER_ERROR,
			NUMBER_ERROR, NUMBER_ERROR, NUMBER_ERROR, NUMBER_ERROR, NUMBER_ERROR, NUMBER_ERROR, NUMBER_ERROR, NUMBER_ERROR, NUMBER_ERROR,
			NUMBER_ERROR, NUMBER_ERROR, NUMBER_ERROR, NUMBER_ERROR, NUMBER_ERROR, NUMBER_ERROR, NUMBER_ERROR, NUMBER_ERROR, NUMBER_ERROR,
			NUMBER_ERROR, NUMBER_ERROR, NUMBER_ERROR, NUMBER_ERROR, NUMBER_ERROR, NUMBER_ERROR, NUMBER_ERROR, NUMBER_ERROR, NUMBER_ERROR,
			NUMBER_ERROR, NUMBER_ERROR, NUMBER_ERROR, NUMBER_ERROR, NUMBER_ERROR, NUMBER_ERROR, NUMBER_ERROR, NUMBER_ERROR, NUMBER_ERROR,
			NUMBER_ERROR, NUMBER_ERROR, NUMBER_ERROR, NUMBER_ERROR, NUMBER_ERROR, NUMBER_ERROR, NUMBER_ERROR, NUMBER_ERROR, NUMBER_ERROR,
			NUMBER_ERROR, NUMBER_ERROR, NUMBER_ERROR, NUMBER_ERROR, NUMBER_ERROR, NUMBER_ERROR, NUMBER_ERROR, NUMBER_ERROR, NUMBER_ERROR,
			NUMBER_ERROR, NUMBER_ERROR, NUMBER_ERROR, NUMBER_ERROR, NUMBER_ERROR, NUMBER_ERROR, NUMBER_ERROR, NUMBER_ERROR, NUMBER_ERROR,
			NUMBER_ERROR, NUMBER_ERROR, NUMBER_ERROR, NUMBER_ERROR, NUMBER_ERROR, NUMBER_ERROR, NUMBER_ERROR, NUMBER_ERROR, NUMBER_ERROR,
			NUMBER_ERROR, NUMBER_ERROR, NUMBER_ERROR, NUMBER_ERROR, NUMBER_ERROR, NUMBER_ERROR, NUMBER_ERROR, NUMBER_ERROR, NUMBER_ERROR,
			NUMBER_ERROR, NUMBER_ERROR, NUMBER_ERROR, NUMBER_ERROR, NUMBER_ERROR, NUMBER_ERROR, NUMBER_ERROR, NUMBER_ERROR, NUMBER_ERROR,
			NUMBER_ERROR, NUMBER_ERROR, NUMBER_ERROR, NUMBER_ERROR, NUMBER_ERROR, NUMBER_ERROR, NUMBER_ERROR, NUMBER_ERROR, NUMBER_ERROR,
			NUMBER_ERROR, NUMBER_ERROR, NUMBER_ERROR, NUMBER_ERROR, NUMBER_ERROR, NUMBER_ERROR, NUMBER_ERROR, NUMBER_ERROR, NUMBER_ERROR,
			NUMBER_ERROR, NUMBER_ERROR, NUMBER_ERROR, NUMBER_ERROR, NUMBER_ERROR, NUMBER_ERROR, NUMBER_ERROR, NUMBER_ERROR, NUMBER_ERROR,
			NUMBER_ERROR, NUMBER_ERROR, NUMBER_ERROR, NUMBER_ERROR, NUMBER_ERROR, NUMBER_ERROR, NUMBER_ERROR, NUMBER_ERROR };

		static inline int64_t parseInteger(const uint8_t* src) noexcept {
			bool negative = (*src == '-');
			const uint8_t* p = src + uint8_t(negative);

			const uint8_t* const startDigits = p;
			uint64_t i = 0;
			while (parseDigit(*p, i)) {
				p++;
			}
			size_t digitCount = size_t(p - startDigits);
			size_t longestDigitCount = 19;
			if ((digitCount == 0) || (digitCount > longestDigitCount)) {
				return -1;
			}
			if (('0' == *startDigits) && (digitCount > 1)) {
				return -1;
			}
			if (integerStringFinisher[*p] != NumberError::SUCCESS) {
				return NumberError(integerStringFinisher[*p]);
			}
			if (i > uint64_t(INT64_MAX) + uint64_t(negative)) {
				return INCORRECT_TYPE;
			}
			return negative ? (~i + 1) : i;
		}

		static inline const bool structural_or_whitespace_negated[256] = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
			1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1,

			1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
			1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 0, 1, 1,

			1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
			1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,

			1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
			1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 };

		static inline uint32_t isNotStructuralOrWhitespace(uint8_t c) {
			return structural_or_whitespace_negated[c];
		}

		static inline double parseDouble(const uint8_t* src) noexcept {
			bool negative = (*src == '-');
			src += uint8_t(negative);

			uint64_t i = 0;
			const uint8_t* p = src;
			p += parseDigit(*p, i);
			bool leadingZero = (i == 0);
			while (parseDigit(*p, i)) {
				p++;
			}
			if (p == src) {
				return INCORRECT_TYPE;
			}
			if ((leadingZero && p != src + 1)) {
				return NUMBER_ERROR;
			}
			int64_t exponent = 0;
			bool overflow;
			if (*p == '.') {
				p++;
				const uint8_t* startDecimalDigits = p;
				if (!parseDigit(*p, i)) {
					return NUMBER_ERROR;
				}
				p++;
				while (parseDigit(*p, i)) {
					p++;
				}
				exponent = -(p - startDecimalDigits);
				overflow = p - src - 1 > 19;
				if (overflow && leadingZero) {
					const uint8_t* startDigits = src + 2;
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
				const uint8_t* startExpDigits = p;
				while (parseDigit(*p, exp)) {
					p++;
				}
				if (p - startExpDigits == 0 || p - startExpDigits > 19) {
					return NUMBER_ERROR;
				}
				exponent += expNeg ? 0 - exp : exp;
			}
			if (isNotStructuralOrWhitespace(*p)) {
				return NUMBER_ERROR;
			}
			overflow = overflow || exponent < smallestPower || exponent > largestPower;

			double d;
			if (!overflow) {
				if (computeFloat64(exponent, i, negative, d)) {
					return d;
				}
			}
			if (!parseFloatFallback(src - uint8_t(negative), &d)) {
				return NUMBER_ERROR;
			}
			return d;
		}

		static inline uint64_t parseUnsigned(const uint8_t* const src) noexcept {
			const uint8_t* p = src;
			const uint8_t* const startDigits = p;
			uint64_t i = 0;
			while (parseDigit(*p, i)) {
				p++;
			}
			size_t digitCount = size_t(p - startDigits);
			if ((digitCount == 0) || (digitCount > 20)) {
				return INCORRECT_TYPE;
			}
			if (('0' == *startDigits) && (digitCount > 1)) {
				return NUMBER_ERROR;
			}
			if (integerStringFinisher[*p] != SUCCESS) {
				return static_cast<uint64_t>(ErrorCode::Invalid_Number);
			}
			if (digitCount == 20) {
				if (src[0] != uint8_t('1') || i <= uint64_t(INT64_MAX)) {
					return INCORRECT_TYPE;
				}
			}

			return i;
		}
	};


}