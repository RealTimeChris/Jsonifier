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

		JSONIFIER_INLINE operator std::string() const noexcept {
			JSONIFIER_ALIGN std::string returnValues{ values, length };
			return returnValues;
		}

		JSONIFIER_INLINE operator jsonifier::string() const noexcept {
			JSONIFIER_ALIGN jsonifier::string returnValues{ values, length };
			return returnValues;
		}

		constexpr jsonifier::string_view view() const noexcept {
			JSONIFIER_ALIGN jsonifier::string_view returnValues{ values, length };
			return returnValues;
		}

		JSONIFIER_ALIGN value_type values[sizeVal]{};
	};

	template<size_t size> struct get_int_type_for_length {
		using type = std::conditional_t<(size >= 5), uint64_t, std::conditional_t<(size > 3 && size < 6), uint32_t, std::conditional_t<(size > 1 && size < 4), uint16_t, uint8_t>>>;
	};

	template<size_t size> using get_int_type_for_length_t = get_int_type_for_length<size>::type;

	template<string_literal string>
		requires(string.length == 0)
	constexpr auto packValues() {
		return uint8_t{};
	}

	template<string_literal string>
		requires(string.length > 0 && string.length <= 8)
	constexpr auto packValues() {
		JSONIFIER_ALIGN get_int_type_for_length_t<string.length> returnValues{};
		for (size_t x = 0; x < string.length; ++x) {
			returnValues |= (static_cast<uint64_t>(string[x]) << ((x % 8) * 8));
		}
		return returnValues;
	}

	template<size_t size> constexpr size_t getPackingSize() {
		if constexpr (size >= 64) {
			return 64;
		} else if constexpr (size >= 32) {
			return 32;
		} else {
			return 16;
		}
	}

	template<string_literal string>
		requires(string.length != 0 && string.length > 8)
	constexpr auto packValues() {
		JSONIFIER_ALIGN array<uint64_t, roundUpToMultiple<16>(getPackingSize<string.length>())> returnValues{};
		for (size_t x = 0; x < string.length; ++x) {
			if (x / 8 < (string.length / 8) + 1) {
				returnValues[x / 8] |= (static_cast<uint64_t>(string[x]) << ((x % 8) * 8));
			}
		}
		return returnValues;
	}

	template<typename value_type>
	concept equals_0 = std::remove_cvref_t<value_type>::length == 0;

	template<typename value_type>
	concept gt_0_lt_16 = std::remove_cvref_t<value_type>::length > 0 && std::remove_cvref_t<value_type>::length < 16;

	template<typename value_type>
	concept eq_16 = std::remove_cvref_t<value_type>::length == 16 && bytesPerStep >= 16;

	template<typename value_type>
	concept eq_32 = std::remove_cvref_t<value_type>::length == 32 && bytesPerStep >= 32;

	template<typename value_type>
	concept eq_64 = std::remove_cvref_t<value_type>::length == 64 && bytesPerStep >= 64;

	template<typename value_type>
	concept gt_16 = std::remove_cvref_t<value_type>::length > 16 && !eq_16<value_type> && !eq_32<value_type> && !eq_64<value_type>;

	template<size_t N> constexpr auto stringLiteralFromView(jsonifier::string_view str) noexcept {
		string_literal<N + 1, char> sl{};
		std::copy_n(str.data(), str.size(), sl.values);
		sl[N] = '\0';
		return sl;
	}

	template<string_literal string, size_t offset> constexpr auto offSetNewLiteral() noexcept {
		constexpr size_t originalSize = string.length;
		constexpr size_t newSize	  = (offset >= originalSize) ? 0 : originalSize - offset;
		string_literal<newSize + 1, char> sl{};
		if constexpr (newSize > 0) {
			std::copy_n(string.data() + offset, newSize, sl.values);
			sl.values[newSize] = '\0';
		}
		return sl;
	}

	template<string_literal string, size_t offset> constexpr auto offSetIntoLiteral() noexcept {
		constexpr size_t originalSize = string.length;
		constexpr size_t newSize	  = (offset >= originalSize) ? originalSize : offset;
		string_literal<newSize + 1, char> sl{};
		if constexpr (newSize > 0) {
			std::copy_n(string.data(), newSize, sl.values);
			sl.values[newSize] = '\0';
		}
		return sl;
	}

	template<typename sl_type, std::remove_cvref_t<sl_type> stringNew> struct string_literal_comparitor;

	template<equals_0 sl_type, std::remove_cvref_t<sl_type> stringNew> struct string_literal_comparitor<sl_type, stringNew> {
		JSONIFIER_INLINE static bool impl(string_view_ptr) noexcept {
			return true;
		}
	};

	template<gt_0_lt_16 sl_type, std::remove_cvref_t<sl_type> stringNew> struct string_literal_comparitor<sl_type, stringNew> {
		JSONIFIER_INLINE static bool impl(string_view_ptr str) noexcept {
			static constexpr auto stringLiteral{ stringNew };
			static constexpr auto newCount{ stringLiteral.size() };
			if constexpr (newCount > 8) {
				JSONIFIER_ALIGN static constexpr auto valuesNew{ packValues<stringLiteral>() };
				jsonifier_simd_int_128 data1{};
				std::memcpy(&data1, str, newCount);
				jsonifier_simd_int_128 data2;
				std::memcpy(&data2, valuesNew.data(), 16);
				return !simd_internal::opTest(simd_internal::opXor(data1, data2));
			} else if constexpr (newCount == 8) {
				static constexpr auto valuesNew{ packValues<stringLiteral>() };
				uint64_t l;
				std::memcpy(&l, str, 8);
				return !(l ^ valuesNew);
			} else if constexpr (newCount == 7) {
				static constexpr auto valuesNew{ packValues<stringLiteral>() };
				uint64_t l{};
				std::memcpy(&l, str, 7);
				return !(l ^ valuesNew);
			} else if constexpr (newCount == 6) {
				static constexpr auto valuesNew{ packValues<stringLiteral>() };
				uint64_t l{};
				std::memcpy(&l, str, 6);
				return !(l ^ valuesNew);
			} else if constexpr (newCount == 5) {
				static constexpr uint32_t valuesNew{ static_cast<uint32_t>(packValues<stringLiteral>()) };
				uint32_t l;
				std::memcpy(&l, str, 4);
				return !(l ^ valuesNew) && (str[4] == stringLiteral[4]);
			} else if constexpr (newCount == 4) {
				static constexpr uint32_t valuesNew{ static_cast<uint32_t>(packValues<stringLiteral>()) };
				uint32_t l;
				std::memcpy(&l, str, 4);
				return !(l ^ valuesNew);
			} else if constexpr (newCount == 3) {
				static constexpr uint16_t valuesNew{ static_cast<uint16_t>(packValues<stringLiteral>()) };
				uint16_t l;
				std::memcpy(&l, str, 2);
				return !(l ^ valuesNew) && (str[2] == stringLiteral[2]);
			} else if constexpr (newCount == 2) {
				static constexpr uint16_t valuesNew{ static_cast<uint16_t>(packValues<stringLiteral>()) };
				uint16_t l;
				std::memcpy(&l, str, 2);
				return !(l ^ valuesNew);
			} else if constexpr (newCount == 1) {
				return *str == stringLiteral[0];
			} else {
				return true;
			}
		};
	};

	template<eq_16 sl_type, std::remove_cvref_t<sl_type> stringNew> struct string_literal_comparitor<sl_type, stringNew> {
		JSONIFIER_INLINE static bool impl(string_view_ptr str) noexcept {
			static constexpr auto newLiteral{ stringNew };
			JSONIFIER_ALIGN static constexpr auto valuesNew{ packValues<newLiteral>() };
			jsonifier_simd_int_128 data1{ simd_internal::gatherValuesU<jsonifier_simd_int_128>(str) };
			jsonifier_simd_int_128 data2{ simd_internal::gatherValues<jsonifier_simd_int_128>(valuesNew.data()) };
			return !simd_internal::opTest(simd_internal::opXor(data1, data2));
		}
	};

#if JSONIFIER_CHECK_FOR_INSTRUCTION(JSONIFIER_AVX512) || JSONIFIER_CHECK_FOR_INSTRUCTION(JSONIFIER_AVX2)

	template<eq_32 sl_type, std::remove_cvref_t<sl_type> stringNew> struct string_literal_comparitor<sl_type, stringNew> {
		JSONIFIER_INLINE static bool impl(string_view_ptr str) noexcept {
			static constexpr auto newLiteral{ stringNew };
			JSONIFIER_ALIGN static constexpr auto valuesNew{ packValues<newLiteral>() };
			jsonifier_simd_int_256 data1{ simd_internal::gatherValuesU<jsonifier_simd_int_256>(str) };
			jsonifier_simd_int_256 data2{ simd_internal::gatherValues<jsonifier_simd_int_256>(valuesNew.data()) };
			return !simd_internal::opTest(simd_internal::opXor(data1, data2));
		}
	};

#endif

#if JSONIFIER_CHECK_FOR_INSTRUCTION(JSONIFIER_AVX512)
	template<eq_64 value_type, std::remove_cvref_t<value_type> stringNew> struct string_literal_comparitor<value_type, stringNew> {
		JSONIFIER_INLINE static bool impl(string_view_ptr str) noexcept {
			static constexpr auto newLiteral{ stringNew };
			JSONIFIER_ALIGN static constexpr auto valuesNew{ packValues<newLiteral>() };
			jsonifier_simd_int_512 data1{ simd_internal::gatherValuesU<jsonifier_simd_int_512>(str) };
			jsonifier_simd_int_512 data2{ simd_internal::gatherValues<jsonifier_simd_int_512>(valuesNew.data()) };
			return !simd_internal::opTest(simd_internal::opXor(data1, data2));
		}
	};
#endif

	constexpr auto getOffsetIntoLiteralSize(size_t inputSize) noexcept {
		if (inputSize >= 64 && bytesPerStep >= 64) {
			return 64;
		} else if (inputSize >= 32 && bytesPerStep >= 32) {
			return 32;
		} else {
			return 16;
		}
	}

	template<gt_16 sl_type, std::remove_cvref_t<sl_type> stringNew> struct string_literal_comparitor<sl_type, stringNew> {
		JSONIFIER_INLINE static bool impl(string_view_ptr str) noexcept {
			static constexpr auto string{ offSetIntoLiteral<stringNew, getOffsetIntoLiteralSize(stringNew.size())>() };
			if (!string_literal_comparitor<decltype(string), string>::impl(str)) {
				return false;
			} else {
				static constexpr auto stringSize = string.size();
				str += stringSize;
				static constexpr auto stringNewer{ offSetNewLiteral<stringNew, stringSize>() };
				return string_literal_comparitor<decltype(stringNewer), stringNewer>::impl(str);
			}
		}
	};

	template<size_t size, typename value_type_new> JSONIFIER_INLINE std::ostream& operator<<(std::ostream& os, const string_literal<size, value_type_new>& input) noexcept {
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
		string_buffer_ptr ptr = buffer + numDigits;
		*ptr				  = '\0';
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
		constexpr auto& lit = make_static<toStringLiteral<number>()>::value;
		return jsonifier::string_view{ lit.data(), lit.size() };
	}

}