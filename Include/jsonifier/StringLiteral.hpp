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
/// https://github.com/RealTimeChris/jsonifier
/// Sep 1, 2024
#pragma once

#include <jsonifier/StringView.hpp>

namespace jsonifier_internal {

	template<size_t sizeVal, typename value_type_new> struct string_literal {
		using value_type	  = value_type_new;
		using const_reference = const value_type&;
		using reference		  = value_type&;
		using const_pointer	  = const value_type*;
		using pointer		  = value_type*;
		using size_type		  = size_t;

		static constexpr size_type length{ sizeVal > 0 ? sizeVal - 1 : 0 };

		constexpr string_literal() noexcept = default;

		constexpr string_literal(const value_type_new (&str)[sizeVal]) noexcept {
			for (size_t x = 0; x < length; ++x) {
				values[x] = str[x];
			}
			values[length] = '\0';
		}

		JSONIFIER_ALWAYS_INLINE constexpr const_pointer data() const noexcept {
			return values;
		}

		JSONIFIER_ALWAYS_INLINE constexpr pointer data() noexcept {
			return values;
		}

		template<size_type sizeNew> constexpr auto operator+=(const string_literal<sizeNew, value_type_new>& str) const noexcept {
			string_literal<sizeNew + sizeVal - 1, value_type> newLiteral{};
			std::copy(values, values + size(), newLiteral.data());
			std::copy(str.data(), str.data() + sizeNew, newLiteral.data() + size());
			return newLiteral;
		}

		template<size_type sizeNew> constexpr auto operator+=(const value_type (&str)[sizeNew]) const noexcept {
			string_literal<sizeNew + sizeVal - 1, value_type> newLiteral{};
			std::copy(values, values + size(), newLiteral.data());
			std::copy(str, str + sizeNew, newLiteral.data() + size());
			return newLiteral;
		}

		template<size_type sizeNew> constexpr auto operator+(const string_literal<sizeNew, value_type_new>& str) const noexcept {
			string_literal<sizeNew + sizeVal - 1, value_type> newLiteral{};
			std::copy(values, values + size(), newLiteral.data());
			std::copy(str.data(), str.data() + sizeNew, newLiteral.data() + size());
			return newLiteral;
		}

		template<size_type sizeNew> constexpr auto operator+(const value_type (&str)[sizeNew]) const noexcept {
			string_literal<sizeNew + sizeVal - 1, value_type> newLiteral{};
			std::copy(values, values + size(), newLiteral.data());
			std::copy(str, str + sizeNew, newLiteral.data() + size());
			return newLiteral;
		}

		template<size_type sizeNew> constexpr friend auto operator+(const value_type (&lhs)[sizeNew], const string_literal<sizeVal, value_type>& str) noexcept {
			return string_literal<sizeNew, value_type>{ lhs } + str;
		}

		constexpr reference operator[](size_type index) noexcept {
			return values[index];
		}

		constexpr const_reference operator[](size_type index) const noexcept {
			return values[index];
		}

		JSONIFIER_ALWAYS_INLINE constexpr size_type size() const noexcept {
			return length;
		}

		JSONIFIER_ALWAYS_INLINE operator std::string() const noexcept {
			JSONIFIER_ALIGN std::string returnValues{ values, length };
			return returnValues;
		}

		JSONIFIER_ALWAYS_INLINE operator jsonifier::string() const noexcept {
			JSONIFIER_ALIGN jsonifier::string returnValues{ values, length };
			return returnValues;
		}

		JSONIFIER_ALWAYS_INLINE constexpr jsonifier::string_view view() const noexcept {
			JSONIFIER_ALIGN jsonifier::string_view returnValues{ values, length };
			return returnValues;
		}

		JSONIFIER_ALIGN value_type values[sizeVal]{};
	};

	template<string_literal string>
		requires(string.size() == 0)
	constexpr auto packValues() {
		return uint8_t{};
	}

	template<string_literal string>
		requires(string.size() == 1)
	constexpr auto packValues() {
		return uint8_t{ string[0] };
	}

	template<string_literal string>
		requires(string.size() == 2)
	constexpr auto packValues() {
		uint16_t returnValues{};
		for (size_t x = 0; x < 2; ++x) {
			returnValues |= (static_cast<uint16_t>(string[x]) << ((x % 8) * 8));
		}
		return returnValues;
	}

	template<string_literal string>
		requires(string.size() > 2 && string.size() <= 4)
	constexpr auto packValues() {
		uint32_t returnValues{};
		for (size_t x = 0; x < string.size(); ++x) {
			returnValues |= (static_cast<uint32_t>(string[x]) << ((x % 8) * 8));
		}
		return returnValues;
	}

	template<string_literal string>
		requires(string.size() > 4 && string.size() <= 8)
	constexpr auto packValues() {
		uint64_t returnValues{};
		for (size_t x = 0; x < string.size(); ++x) {
			returnValues |= (static_cast<uint64_t>(string[x]) << ((x % 8) * 8));
		}
		return returnValues;
	}

	template<string_literal string>
		requires(string.size() != 0 && string.size() > 8)
	constexpr std::array<uint64_t, ((string.size() / 8 > 0) ? (string.size() / 8) + 1 : 1)> packValues() {
		std::array<uint64_t, ((string.size() / 8 > 0) ? (string.size() / 8) + 1 : 1)> returnValues{};
		for (size_t x = 0; x < string.size(); ++x) {
			if (x / 8 < (string.size() / 8) + 1) {
				returnValues[x / 8] |= (static_cast<uint64_t>(string[x]) << ((x % 8) * 8));
			}
		}
		return returnValues;
	}

	template<size_t N> constexpr auto stringLiteralFromView(jsonifier::string_view str) noexcept {
		string_literal<N + 1, char> sl{};
		std::copy_n(str.data(), str.size(), sl.values);
		sl[N] = '\0';
		return sl;
	}

	JSONIFIER_ALWAYS_INLINE bool isLower24Equal(uint32_t value1, uint32_t value2) {
		static constexpr uint32_t mask = 0xFFFFFF;
		return (value1 & mask) == (value2 & mask);
	}

	JSONIFIER_ALWAYS_INLINE bool isLower56Equal(uint64_t value1, uint64_t value2) {
		static constexpr uint64_t mask = 0x00FFFFFFFFFFFFFF;
		return (value1 & mask) == (value2 & mask);
	}

	JSONIFIER_ALWAYS_INLINE bool isLower48Equal(uint64_t value1, uint64_t value2) {
		static constexpr uint64_t mask = 0x0000FFFFFFFFFFFF;
		return (value1 & mask) == (value2 & mask);
	}

	JSONIFIER_ALWAYS_INLINE bool isLower40Equal(uint64_t value1, uint64_t value2) {
		static constexpr uint64_t mask = 0x000000FFFFFFFFFF;
		return (value1 & mask) == (value2 & mask);
	}

	JSONIFIER_ALWAYS_INLINE bool isLower16Equal(uint64_t value1, uint64_t value2) {
		static constexpr uint64_t mask = 0xFFFF;
		return (value1 & mask) == (value2 & mask);
	}

	JSONIFIER_ALWAYS_INLINE bool isLower8Equal(uint64_t value1, uint64_t value2) {
		static constexpr uint64_t mask = 0xFF;
		return (value1 & mask) == (value2 & mask);
	}

	template<string_literal stringNew> struct string_literal_comparitor {
		JSONIFIER_ALWAYS_INLINE static bool impl(const char* str) noexcept {
			static constexpr auto string{ stringNew };
			static constexpr auto length{ string.size() };
			size_t lengthNew{ length };
			static constexpr jsonifier::string_view newerView{ string.data() + string.size() - length, length };
			static constexpr auto newerLiteral{ stringLiteralFromView<newerView.size()>(newerView) };
			if constexpr (length > 8) {
				static constexpr auto valuesNew{ packValues<newerLiteral>() };
				size_t currentIndex{};
				size_t v;
				while (lengthNew > 8) {
					std::memcpy(&v, str, sizeof(size_t));
					if (v != valuesNew[currentIndex]) {
						std::cout << "CURRENT SOUGHT STRING: " << jsonifier::string_view{ str, string.size() } << std::endl;
						std::cout << "CURRENT KEY: " << string << std::endl;
						return false;
					}
					++currentIndex;
					lengthNew -= 8;
					str += 8;
				}
			}
			static constexpr auto newLength{ length % 8 };
			static constexpr jsonifier::string_view newView{ newerLiteral.data() + newerLiteral.size() - newLength, newLength };
			static constexpr auto newLiteral{ stringLiteralFromView<newView.size()>(newView) };
			if constexpr (newLength == 8) {
				static constexpr auto valuesNew{ packValues<newLiteral>() };
				size_t v;
				std::memcpy(&v, str, sizeof(size_t));
				if (valuesNew == v) {
					return true;
				}
				std::cout << "CURRENT SOUGHT STRING: " << jsonifier::string_view{ str, string.size() } << std::endl;
				std::cout << "CURRENT KEY: " << string << std::endl;
				return false;
			} else if constexpr (newLength == 7) {
				static constexpr auto valuesNew{ packValues<newLiteral>() };
				uint64_t v;
				std::memcpy(&v, str, newLength);
				if (isLower56Equal(valuesNew, v)) {
					return true;
				}
				std::cout << "CURRENT SOUGHT STRING: " << jsonifier::string_view{ str, string.size() } << std::endl;
				std::cout << "CURRENT KEY: " << string << std::endl;
				return false;
			} else if constexpr (newLength == 6) {
				static constexpr auto valuesNew{ packValues<newLiteral>() };
				uint64_t v;
				std::memcpy(&v, str, newLength);
				if (isLower48Equal(valuesNew, v)) {
					return true;
				}
				std::cout << "CURRENT SOUGHT STRING: " << jsonifier::string_view{ str, string.size() } << std::endl;
				std::cout << "CURRENT KEY: " << string << std::endl;
				return false;
			} else if constexpr (newLength == 5) {
				static constexpr auto valuesNew{ packValues<newLiteral>() };
				uint64_t v;
				std::memcpy(&v, str, newLength);
				if (isLower40Equal(valuesNew, v)) {
					return true;
				}
				std::cout << "CURRENT SOUGHT STRING: " << jsonifier::string_view{ str, string.size() } << std::endl;
				std::cout << "CURRENT KEY: " << string << std::endl;
				return false;
			} else if constexpr (newLength == 4) {
				static constexpr auto valuesNew{ packValues<newLiteral>() };
				uint32_t v;
				std::memcpy(&v, str, newLength);
				if (valuesNew == v) {
					return true;
				}
				std::cout << "CURRENT SOUGHT STRING: " << jsonifier::string_view{ str, string.size() } << std::endl;
				std::cout << "CURRENT KEY: " << string << std::endl;
				return false;
			} else if constexpr (newLength == 3) {
				static constexpr auto valuesNew{ packValues<newLiteral>() };
				uint32_t v;
				std::memcpy(&v, str, newLength);
				if (isLower24Equal(valuesNew, v)) {
					return true;
				}
				std::cout << "CURRENT SOUGHT STRING: " << jsonifier::string_view{ str, string.size() } << std::endl;
				std::cout << "CURRENT KEY: " << string << std::endl;
				return false;
			} else if constexpr (newLength == 2) {
				static constexpr auto valuesNew{ packValues<newLiteral>() };
				uint16_t v;
				std::memcpy(&v, str, newLength);
				if (valuesNew == v) {
					return true;
				}
				std::cout << "CURRENT SOUGHT STRING: " << jsonifier::string_view{ str, string.size() } << std::endl;
				std::cout << "CURRENT KEY: " << string << std::endl;
				return true;
			} else if constexpr (newLength == 1) {
				static constexpr auto valuesNew{ packValues<newLiteral>() };
				if (valuesNew == *str) {
					return true;
				}
				std::cout << "CURRENT SOUGHT STRING: " << jsonifier::string_view{ str, string.size() } << std::endl;
				std::cout << "CURRENT KEY: " << string << std::endl;
				return false;
			}
			return true;
		}
	};

	template<size_t size, typename value_type_new> JSONIFIER_ALWAYS_INLINE std::ostream& operator<<(std::ostream& os, const string_literal<size, value_type_new>& input) noexcept {
		os << input.view();
		return os;
	}

	constexpr size_t countDigits(int64_t number) noexcept {
		size_t count = 0;
		if (number < 0) {
			number *= -1;
			++count;
		}
		do {
			++count;
			number /= 10;
		} while (number != 0);
		return count;
	}

	template<int64_t number, size_t numDigits = countDigits(number)> constexpr string_literal<numDigits + 1, char> toStringLiteral() noexcept {
		char buffer[numDigits + 1]{};
		char* ptr = buffer + numDigits;
		*ptr	  = '\0';
		int64_t temp{};
		if constexpr (number < 0) {
			temp			   = number * -1;
			*(ptr - numDigits) = '-';
		} else {
			temp = number;
		}
		do {
			*--ptr = '0' + (temp % 10);
			temp /= 10;
		} while (temp != 0);
		return string_literal<numDigits + 1, char>{ buffer };
	}

	template<auto valueNew> struct make_static {
		static constexpr auto value{ valueNew };
	};

	constexpr char toLower(char input) noexcept {
		return (input >= 'A' && input <= 'Z') ? (input + 32) : input;
	}

	template<size_t size, typename value_type> constexpr auto toLower(string_literal<size, value_type> input) noexcept {
		string_literal<size, value_type> output{};
		for (size_t x = 0; x < size; ++x) {
			output[x] = toLower(input[x]);
		}
		return output;
	}

	template<int64_t number> constexpr jsonifier::string_view toStringView() noexcept {
		constexpr auto& lit = jsonifier_internal::make_static<toStringLiteral<number>()>::value;
		return jsonifier::string_view{ lit.data(), lit.size() };
	}

}
