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

		constexpr const_pointer data() const noexcept {
			return values;
		}

		constexpr pointer data() noexcept {
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

		constexpr size_type size() const noexcept {
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

		constexpr jsonifier::string_view view() const noexcept {
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
	constexpr array<uint64_t, ((string.size() / 8 > 0) ? (string.size() / 8) + 1 : 1)> packValues() {
		array<uint64_t, ((string.size() / 8 > 0) ? (string.size() / 8) + 1 : 1)> returnValues{};
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

	template<string_literal string, size_t offset> constexpr auto offSetNewLiteral() noexcept {
		constexpr size_t originalSize = string.size();
		constexpr size_t newSize	  = (offset >= originalSize) ? 0 : originalSize - offset;
		string_literal<newSize + 1, char> sl{};
		if constexpr (newSize > 0) {
			std::copy_n(string.data() + offset, newSize, sl.values);
			sl.values[newSize] = '\0';
		}
		return sl;
	}

	template<string_literal string, size_t offset> constexpr auto offSetIntoLiteral() noexcept {
		constexpr size_t originalSize = string.size();
		constexpr size_t newSize	  = (offset >= originalSize) ? originalSize : offset;
		string_literal<newSize + 1, char> sl{};
		if constexpr (newSize > 0) {
			std::copy_n(string.data(), newSize, sl.values);
			sl.values[newSize] = '\0';
		}
		return sl;
	}

	template<string_literal stringNew> struct string_literal_comparitor {
		JSONIFIER_ALWAYS_INLINE static bool impl(const char* str) noexcept {
			static constexpr auto newLiteral{ stringNew };
			return newLiteral[0] == str[0];
		}
	};

	template<string_literal stringNew>
		requires(stringNew.size() > 0 && stringNew.size <= 8)
	struct string_literal_comparitor<stringNew> {
		static constexpr auto stringLiteral{ stringNew };
		static constexpr auto valuesNew{ packValues<stringLiteral>() };
		JSONIFIER_ALWAYS_INLINE static bool impl(const char* str) noexcept {
			static constexpr auto newCount{ stringLiteral.size() };
			if constexpr (newCount == 8) {
				static constexpr auto valuesNew{ packValues<stringLiteral>() };
				uint64_t l;
				std::memcpy(&l, str, 8);
				return l == valuesNew;
			} else if constexpr (newCount == 7) {
				static constexpr auto valuesNew{ packValues<stringLiteral>() };
				uint64_t l{};
				std::memcpy(&l, str, 7);
				return (valuesNew == l);
			} else if constexpr (newCount == 6) {
				static constexpr auto valuesNew{ packValues<stringLiteral>() };
				uint64_t l{};
				std::memcpy(&l, str, 6);
				return (valuesNew == l);
			} else if constexpr (newCount == 5) {
				static constexpr auto valuesNew{ packValues<stringLiteral>() };
				uint32_t l;
				std::memcpy(&l, str, 4);
				return (l == valuesNew) && (str[4] == stringLiteral[4]);
			} else if constexpr (newCount == 4) {
				static constexpr auto valuesNew{ packValues<stringLiteral>() };
				uint32_t l;
				std::memcpy(&l, str, 4);
				return l == valuesNew;
			} else if constexpr (newCount == 3) {
				static constexpr auto valuesNew{ packValues<stringLiteral>() };
				uint16_t l;
				std::memcpy(&l, str, 2);
				return (l == valuesNew) && (str[2] == stringLiteral[2]);
			} else if constexpr (newCount == 2) {
				static constexpr auto valuesNew{ packValues<stringLiteral>() };
				uint16_t l;
				std::memcpy(&l, str, 2);
				return l == valuesNew;
			} else if constexpr (newCount == 1) {
				return *str == stringLiteral[0];
			} else if constexpr (newCount == 0) {
				return true;
			}
		}
	};

#if JSONIFIER_CHECK_FOR_INSTRUCTION(JSONIFIER_AVX512)

	template<string_literal stringNew>
		requires(stringNew.size() > 32 && stringNew.size <= 64)
	struct string_literal_comparitor<stringNew> {
		JSONIFIER_ALWAYS_INLINE static bool impl(const char* str) noexcept {
			static constexpr auto newLiteral{ stringNew };
			static constexpr auto newLength{ stringNew.size() };
			static constexpr auto valuesNew{ packValues<newLiteral>() };
			jsonifier_simd_int_512 data1{};
			std::memcpy(&data1, str, newLength);
			jsonifier_simd_int_512 data2{};
			std::memcpy(&data2, &valuesNew, newLength);

			static constexpr auto maskBytes = []() constexpr {
				alignas(64) array<uint8_t, 64> maskBytes{};
				for (size_t i = 0; i < newLength; ++i) {
					maskBytes[i] = 0xFF;
				}
				return maskBytes;
			}();

			jsonifier_simd_int_512 mask = simd_internal::gatherValues<jsonifier_simd_int_512>(maskBytes.data());
			return simd_internal::opTest(simd_internal::opXor(simd_internal::opAnd(data1, mask), simd_internal::opAnd(data2, mask)));
		}
	};

#endif

#if JSONIFIER_CHECK_FOR_INSTRUCTION(JSONIFIER_AVX512) || JSONIFIER_CHECK_FOR_INSTRUCTION(JSONIFIER_AVX2)

	template<string_literal stringNew>
		requires(stringNew.size() > 16 && stringNew.size <= 32)
	struct string_literal_comparitor<stringNew> {
		JSONIFIER_ALWAYS_INLINE static bool impl(const char* str) noexcept {
			static constexpr auto newLiteral{ stringNew };
			static constexpr auto newLength{ stringNew.size() };
			static constexpr auto valuesNew{ packValues<newLiteral>() };
			jsonifier_simd_int_256 data1{};
			std::memcpy(&data1, str, newLength);
			jsonifier_simd_int_256 data2{};
			std::memcpy(&data2, &valuesNew, newLength);

			static constexpr auto maskBytes = []() constexpr {
				alignas(32) array<uint8_t, 32> maskBytes{};
				for (size_t i = 0; i < newLength; ++i) {
					maskBytes[i] = 0xFF;
				}
				return maskBytes;
			}();

			jsonifier_simd_int_256 mask = simd_internal::gatherValues<jsonifier_simd_int_256>(maskBytes.data());
			return simd_internal::opTest(simd_internal::opXor(simd_internal::opAnd(data1, mask), simd_internal::opAnd(data2, mask)));
		}
	};

#endif

#if JSONIFIER_CHECK_FOR_INSTRUCTION(JSONIFIER_AVX512) || JSONIFIER_CHECK_FOR_INSTRUCTION(JSONIFIER_AVX2) || JSONIFIER_CHECK_FOR_INSTRUCTION(JSONIFIER_AVX) || \
	JSONIFIER_CHECK_FOR_INSTRUCTION(JSONIFIER_NEON)

	template<string_literal stringNew>
		requires(stringNew.size() > 8 && stringNew.size <= 16)
	struct string_literal_comparitor<stringNew> {
		JSONIFIER_ALWAYS_INLINE static bool impl(const char* str) noexcept {
			static constexpr auto newLiteral{ stringNew };
			static constexpr auto newLength{ stringNew.size() };
			static constexpr auto valuesNew{ packValues<newLiteral>() };
			jsonifier_simd_int_128 data1{};
			std::memcpy(&data1, str, newLength);
			jsonifier_simd_int_128 data2{};
			std::memcpy(&data2, &valuesNew, newLength);

			static constexpr auto maskBytes = []() constexpr {
				alignas(16) array<uint8_t, 16> maskBytes{};
				for (size_t i = 0; i < newLength; ++i) {
					maskBytes[i] = 0xFF;
				}
				return maskBytes;
			}();

			jsonifier_simd_int_128 mask = simd_internal::gatherValues<jsonifier_simd_int_128>(maskBytes.data());
			return simd_internal::opTest(simd_internal::opXor(simd_internal::opAnd(data1, mask), simd_internal::opAnd(data2, mask)));
		}
	};

#endif

	template<string_literal stringNew>
		requires(stringNew.size() > bytesPerStep)
	struct string_literal_comparitor<stringNew> {
		JSONIFIER_ALWAYS_INLINE static bool impl(const char* str) noexcept {
			if constexpr (stringNew.size() > bytesPerStep) {
				static constexpr auto string{ offSetIntoLiteral<stringNew, bytesPerStep>() };
				if (!string_literal_comparitor<string>::impl(str)) {
					return false;
				} else {
					str += string.size();
					static constexpr auto stringNewer{ offSetNewLiteral<stringNew, bytesPerStep>() };
					return string_literal_comparitor<stringNewer>::impl(str);
				}
			} else {
				static constexpr auto stringNewer{ offSetNewLiteral<stringNew, stringNew.size()>() };
				return string_literal_comparitor<stringNewer>::impl(str);
			}
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