// BenchmarkSuite.h : Include file for standard system include files,
// or project specific include files.

#pragma once

#include <jsonifier/StringView.hpp>

namespace jsonifier_internal {

	template<size_t sizeVal> struct string_literal {
		using value_type	  = char;
		using const_reference = const value_type&;
		using reference		  = value_type&;
		using const_pointer	  = const value_type*;
		using pointer		  = value_type*;
		using size_type		  = size_t;

		static constexpr size_type length{ sizeVal > 0 ? sizeVal - 1 : 0 };

		JSONIFIER_ALWAYS_INLINE constexpr string_literal() noexcept = default;

		JSONIFIER_ALWAYS_INLINE constexpr string_literal(const value_type (&str)[sizeVal]) {
			std::copy(str, str + length, values);
		}

		JSONIFIER_ALWAYS_INLINE constexpr const_pointer data() const {
			return values;
		}

		JSONIFIER_ALWAYS_INLINE constexpr pointer data() {
			return values;
		}

		template<size_t sizeNew> JSONIFIER_ALWAYS_INLINE constexpr auto operator+(const string_literal<sizeNew>& str) const {
			string_literal<sizeNew + sizeVal - 1> newLiteral{};
			std::copy(values, values + size(), newLiteral.data());
			std::copy(str.data(), str.data() + sizeNew, newLiteral.data() + size());
			return newLiteral;
		}

		template<size_t sizeNew> JSONIFIER_ALWAYS_INLINE constexpr auto operator+(const value_type (&str)[sizeNew]) const {
			string_literal<sizeNew + sizeVal - 1> newLiteral{};
			std::copy(values, values + size(), newLiteral.data());
			std::copy(str, str + sizeNew, newLiteral.data() + size());
			return newLiteral;
		}

		template<size_t sizeNew> JSONIFIER_ALWAYS_INLINE constexpr auto operator+=(const string_literal<sizeNew>& str) const {
			string_literal<sizeNew + sizeVal - 1> newLiteral{};
			std::copy(values, values + size(), newLiteral.data());
			std::copy(str.data(), str.data() + sizeNew, newLiteral.data() + size());
			return newLiteral;
		}

		template<size_t sizeNew> JSONIFIER_ALWAYS_INLINE constexpr auto operator+=(const value_type (&str)[sizeNew]) const {
			string_literal<sizeNew + sizeVal - 1> newLiteral{};
			std::copy(values, values + size(), newLiteral.data());
			std::copy(str, str + sizeNew, newLiteral.data() + size());
			return newLiteral;
		}

		JSONIFIER_ALWAYS_INLINE constexpr reference operator[](size_type index) {
			return values[index];
		}

		JSONIFIER_ALWAYS_INLINE constexpr const_reference operator[](size_type index) const {
			return values[index];
		}

		JSONIFIER_ALWAYS_INLINE constexpr size_type size() const {
			return length;
		}

		JSONIFIER_ALWAYS_INLINE operator jsonifier::string() const {
			return { values, length };
		}

		JSONIFIER_ALWAYS_INLINE constexpr jsonifier::string_view view() const {
			return { values, length };
		}

		value_type values[sizeVal]{};
	};

	template<size_t size> JSONIFIER_ALWAYS_INLINE std::ostream& operator<<(std::ostream& os, const string_literal<size>& input) {
		os << input.operator jsonifier::string_view();
		return os;
	}

	template<size_t N> JSONIFIER_ALWAYS_INLINE constexpr auto stringLiteralFromView(jsonifier::string_view str) {
		string_literal<N + 1> sl{};
		std::copy_n(str.data(), str.size(), sl.values);
		sl[N] = '\0';
		return sl;
	}

	JSONIFIER_ALWAYS_INLINE constexpr size_t countDigits(uint32_t number) {
		size_t count = 0;
		do {
			++count;
			number /= 10;
		} while (number != 0);
		return count;
	}

	template<uint32_t number> JSONIFIER_ALWAYS_INLINE constexpr string_literal<countDigits(number) + 1> toStringLiteral() {
		constexpr size_t num_digits = countDigits(number);
		char buffer[num_digits + 1]{};
		char* ptr	  = buffer + num_digits;
		*ptr		  = '\0';
		uint32_t temp = number;
		do {
			*--ptr = '0' + (temp % 10);
			temp /= 10;
		} while (temp != 0);
		return string_literal<countDigits(number) + 1>{ buffer };
	}

	template<auto valueNew> struct make_static {
		static constexpr auto value{ valueNew };
	};

	constexpr char toLower(char input) {
		return (input >= 'A' && input <= 'Z') ? (input + 32) : input;
	}

	template<size_t size> constexpr auto toLower(string_literal<size> input) {
		string_literal<size> output{};
		for (size_t x = 0; x < size; ++x) {
			output[x] = toLower(input[x]);
		}
		return output;
	}

	template<uint32_t number> JSONIFIER_ALWAYS_INLINE constexpr jsonifier::string_view toStringView() {
		constexpr auto& lit = jsonifier_internal::make_static<toStringLiteral<number>()>::value;
		return jsonifier::string_view(lit.value.data(), lit.value.size() - 1);
	}

	template<string_literal... strings> JSONIFIER_ALWAYS_INLINE constexpr auto combineLiterals() {
		constexpr size_t newSize = { (strings.size() + ...) };
		char returnValue[newSize + 1]{};
		returnValue[newSize] = '\0';
		auto copyLambda		 = [&](const char* ptr, size_t newSize, size_t& currentOffset) {
			 std::copy(ptr, ptr + newSize, returnValue + currentOffset);
			 currentOffset += newSize;
		};
		size_t currentOffset{};
		(copyLambda(strings.data(), strings.size(), currentOffset), ...);
		return string_literal{ returnValue };
	}

}
