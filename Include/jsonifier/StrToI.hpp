/*
	MIT License

	Copyright (c) 2024 RealTimeChris

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

	constexpr std::array<uint8_t, 256> digiTable{ 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u,
		0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u,
		0x00u, 0x04u, 0x00u, 0x08u, 0x10u, 0x00u, 0x01u, 0x02u, 0x02u, 0x02u, 0x02u, 0x02u, 0x02u, 0x02u, 0x02u, 0x02u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u,
		0x00u, 0x00u, 0x00u, 0x20u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u,
		0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x20u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u,
		0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u };

	JSONIFIER_ALWAYS_INLINE bool digiIsFp(uint8_t d) noexcept {
		static constexpr uint8_t digiTypeDot = 1 << 4;
		static constexpr uint8_t digiTypeExp = 1 << 5;
		return (digiTable[d] & (digiTypeDot | digiTypeExp)) != 0;
	}

	constexpr std::array<uint64_t, 256> numberSubTable = []() {
		std::array<uint64_t, 256> returnValues{};
		for (uint64_t i = 0; i < 256; ++i) {
			returnValues[i] = i >= '0' && i <= '9' ? i - '0' : 10;
		}
		return returnValues;
	}();

	template<bool negative, size_t currentIndex, typename iterator, jsonifier::concepts::int64_type value_type>
	JSONIFIER_ALWAYS_INLINE bool parseIntImpl(iterator& iter, value_type& value, value_type numTmp = {}) {
		static constexpr uint64_t maxUint64{ std::numeric_limits<uint64_t>::max() };
		static constexpr int64_t minInt64{ std::numeric_limits<int64_t>::min() };
		static constexpr size_t maxInteger{ 9 };
		static constexpr size_t maxIndex{ 19 };
		static constexpr uint8_t zero{ '0' };
		static constexpr size_t ten{ 10 };
		if constexpr (currentIndex < maxIndex) {
			numTmp = numberSubTable[static_cast<uint8_t>(iter[currentIndex])];
			if (numTmp <= maxInteger) [[likely]] {
				if constexpr (negative) {
					if (static_cast<uint64_t>(value) <= ((minInt64 - numTmp) / ten)) [[likely]] {
						value = numTmp + value * ten;
						return parseIntImpl<negative, currentIndex + 1>(iter, value, numTmp);
					} else [[unlikely]] {
						return false;
					}
				} else {
					if (static_cast<uint64_t>(value) <= static_cast<uint64_t>(maxUint64 - numTmp) / ten) [[likely]] {
						value = numTmp + value * ten;
						return parseIntImpl<negative, currentIndex + 1>(iter, value, numTmp);
					} else [[unlikely]] {
						return false;
					}
				}
			}
		}
		 
		if constexpr (currentIndex > 1) {
			if (iter[0] == zero) [[unlikely]] {
				return false;
			}
		}

		if (!digiIsFp(uint8_t(iter[currentIndex + 1]))) [[likely]] {
			iter += currentIndex;
			return true;
		}

		return false;
	}

	template<jsonifier::concepts::int64_type value_type, typename iterator> JSONIFIER_ALWAYS_INLINE bool parseInt(iterator&& iter, value_type& value) {
		static constexpr int64_t negativeOne{ -1 };
		static constexpr size_t maxInteger{ 9 };
		const bool negative{ (*iter == '-') };
		iter += static_cast<int64_t>(negative);
		value = numberSubTable[static_cast<uint8_t>(*iter)];

		if (value > maxInteger) [[unlikely]] {
			return false;
		}

		if (negative) {
			auto result = parseIntImpl<true, 1>(iter, value);
			value *= negativeOne;
			return result;
		} else {
			return parseIntImpl<false, 1>(iter, value);
		}
	}

	template<size_t currentIndex, typename iterator, jsonifier::concepts::uint64_type value_type>
	JSONIFIER_ALWAYS_INLINE bool parseUintImpl(iterator& iter, value_type& value, value_type numTmp = {}) {
		static constexpr uint64_t maxUint64{ std::numeric_limits<uint64_t>::max() };
		static constexpr size_t maxInteger{ 9 };
		static constexpr size_t maxIndex{ 20 };
		static constexpr uint8_t zero{ '0' };
		static constexpr size_t ten{ 10 };
		if constexpr (currentIndex < maxIndex) {
			numTmp = numberSubTable[static_cast<uint8_t>(iter[currentIndex])];
			if (numTmp <= maxInteger) [[likely]] {
				if (value <= (maxUint64 - numTmp) / ten) [[likely]] {
					value = numTmp + value * ten;
					return parseUintImpl<currentIndex + 1>(iter, value, numTmp);
				} else [[unlikely]] {
					return false;
				}
			}
		}

		if constexpr (currentIndex > 1) {
			if (iter[0] == zero) [[unlikely]] {
				return false;
			}
		}

		if (!digiIsFp(uint8_t(iter[currentIndex + 1]))) [[likely]] {
			iter += currentIndex;
			return true;
		}

		return false;
	}

	template<jsonifier::concepts::uint64_type value_type, typename iterator> JSONIFIER_ALWAYS_INLINE bool parseUint(iterator&& iter, value_type& value) {
		static constexpr size_t maxInteger{ 9 };
		value = numberSubTable[static_cast<uint8_t>(*iter)];

		if (value > maxInteger) [[unlikely]] {
			return false;
		}

		return parseUintImpl<1>(iter, value);
	}

	constexpr std::array<bool, 256> digitTableBool{ []() {
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

	JSONIFIER_ALWAYS_INLINE constexpr bool isSafeAddition(uint64_t a, uint64_t b) noexcept {
		return a <= (std::numeric_limits<uint64_t>::max)() - b;
	}

	JSONIFIER_ALWAYS_INLINE constexpr bool isSafeMultiplication10(uint64_t a) noexcept {
		constexpr uint64_t b = (std::numeric_limits<uint64_t>::max)() / 10;
		return a <= b;
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