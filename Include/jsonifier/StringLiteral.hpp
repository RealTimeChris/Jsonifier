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

		JSONIFIER_ALWAYS_INLINE constexpr string_literal() noexcept = default;

		JSONIFIER_ALWAYS_INLINE constexpr string_literal(const value_type_new (&str)[sizeVal]) noexcept {
			std::copy(str, str + length, values);
		}

		template<size_t sizeNew> JSONIFIER_ALWAYS_INLINE constexpr string_literal(const value_type (&str)[sizeNew]) noexcept {
			static_assert(sizeNew < sizeVal, "Sorry, but the additional string_literal is too large.");
			std::copy(str, str + sizeNew, values);
		}

		template<size_t sizeNew> JSONIFIER_ALWAYS_INLINE constexpr string_literal(const string_literal<sizeNew, value_type_new>& str) noexcept {
			static_assert(sizeNew < sizeVal, "Sorry, but the additional string_literal is too large.");
			std::copy(str.data(), str.data() + str.size(), values);
		}

		JSONIFIER_ALWAYS_INLINE constexpr const_pointer data() const noexcept {
			return values;
		}

		JSONIFIER_ALWAYS_INLINE constexpr pointer data() noexcept {
			return values;
		}

		template<size_type sizeNew> JSONIFIER_ALWAYS_INLINE constexpr auto operator+=(const string_literal<sizeNew, value_type_new>& str) const noexcept {
			string_literal<sizeNew + sizeVal - 1, value_type> newLiteral{};
			std::copy(values, values + size(), newLiteral.data());
			std::copy(str.data(), str.data() + sizeNew, newLiteral.data() + size());
			return newLiteral;
		}

		template<size_type sizeNew> JSONIFIER_ALWAYS_INLINE constexpr auto operator+=(const value_type (&str)[sizeNew]) const noexcept {
			string_literal<sizeNew + sizeVal - 1, value_type> newLiteral{};
			std::copy(values, values + size(), newLiteral.data());
			std::copy(str, str + sizeNew, newLiteral.data() + size());
			return newLiteral;
		}

		template<size_type sizeNew> JSONIFIER_ALWAYS_INLINE constexpr auto operator+(const string_literal<sizeNew, value_type_new>& str) const noexcept {
			string_literal<sizeNew + sizeVal - 1, value_type> newLiteral{};
			std::copy(values, values + size(), newLiteral.data());
			std::copy(str.data(), str.data() + sizeNew, newLiteral.data() + size());
			return newLiteral;
		}

		template<size_type sizeNew> JSONIFIER_ALWAYS_INLINE constexpr auto operator+(const value_type (&str)[sizeNew]) const noexcept {
			string_literal<sizeNew + sizeVal - 1, value_type> newLiteral{};
			std::copy(values, values + size(), newLiteral.data());
			std::copy(str, str + sizeNew, newLiteral.data() + size());
			return newLiteral;
		}

		template<size_type sizeNew>
		JSONIFIER_ALWAYS_INLINE constexpr friend auto operator+(const value_type (&lhs)[sizeNew], const string_literal<sizeVal, value_type>& rhs) noexcept {
			return string_literal<sizeNew, value_type>{ lhs } + rhs;
		}

		JSONIFIER_ALWAYS_INLINE constexpr reference operator[](size_type index) noexcept {
			return values[index];
		}

		JSONIFIER_ALWAYS_INLINE constexpr const_reference operator[](size_type index) const noexcept {
			return values[index];
		}

		JSONIFIER_ALWAYS_INLINE constexpr size_type size() const noexcept {
			return length;
		}

		JSONIFIER_ALWAYS_INLINE operator jsonifier::string() const noexcept {
			return { values, length };
		}

		JSONIFIER_ALWAYS_INLINE constexpr jsonifier::string_view view() const noexcept {
			return { values, length };
		}

		JSONIFIER_ALIGN value_type values[sizeVal]{};
	};

	template<size_t size, typename value_type_new>
	JSONIFIER_ALWAYS_INLINE std::ostream& operator<<(std::ostream& os, const string_literal<size, value_type_new>& input) noexcept {
		os << input.view();
		return os;
	}

	template<size_t N> JSONIFIER_ALWAYS_INLINE constexpr auto stringLiteralFromView(jsonifier::string_view str) noexcept {
		string_literal<N + 1, char> sl{};
		std::copy_n(str.data(), str.size(), sl.values);
		sl[N] = '\0';
		return sl;
	}

	JSONIFIER_ALWAYS_INLINE constexpr size_t countDigits(int64_t number) noexcept {
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

	template<int64_t number, size_t numDigits = countDigits(number)>
	JSONIFIER_ALWAYS_INLINE constexpr string_literal<numDigits + 1, char> toStringLiteral() noexcept {
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

	template<int64_t number> JSONIFIER_ALWAYS_INLINE constexpr jsonifier::string_view toStringView() noexcept {
		constexpr auto& lit = jsonifier_internal::make_static<toStringLiteral<number>()>::value;
		return jsonifier::string_view{ lit.data(), lit.size() };
	}

}
