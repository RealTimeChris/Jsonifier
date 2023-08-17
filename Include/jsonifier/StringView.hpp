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
/// https://github.com/RealTimeChris/Jsonifier
/// Feb 20, 2023
#pragma once

#include <jsonifier/String.hpp>

#include <string_view>
#include <immintrin.h>
#include <type_traits>
#include <iostream>
#include <string>

namespace Jsonifier {

	template<typename ValueType> class StringViewBase {
	  public:
		using value_type			 = ValueType;
		using pointer				 = value_type*;
		using const_pointer			 = const value_type*;
		using reference				 = value_type&;
		using const_reference		 = const value_type&;
		using iterator               = JsonifierInternal::Iterator<StringViewBase>;
		using const_iterator         = JsonifierInternal::ConstIterator<StringViewBase>;
		using reverse_iterator       = std::reverse_iterator<iterator>;
		using difference_type		 = std::ptrdiff_t;
		using const_reverse_iterator = std::reverse_iterator<const_iterator>;
		using object_compare		 = std::equal_to<value_type>;
		using size_type				 = uint64_t;
		using allocator				 = JsonifierInternal::AllocWrapper<value_type>;
		using traits_type			 = std::char_traits<value_type>;

		const_pointer dataVal{};
		size_type sizeVal{};

		static constexpr auto npos{ static_cast<size_type>(-1) };

		constexpr StringViewBase() noexcept : dataVal(), sizeVal(0) {
		}

		constexpr StringViewBase(const StringViewBase&) noexcept = default;
		constexpr StringViewBase& operator=(const StringViewBase&) noexcept = default;

		constexpr StringViewBase& operator=(const StringBase<value_type>& stringNew) {
			dataVal = stringNew.data();
			sizeVal = stringNew.size();
			return *this;
		}

		constexpr StringViewBase(const StringBase<value_type>& stringNew) {
			*this = stringNew;
		}

		constexpr std::enable_if<std::same_as<char, value_type>, StringViewBase<value_type>>& operator=(const std::string& other) {
			dataVal = other.data();
			sizeVal = other.size();
			return *this;
		}

		constexpr StringViewBase(const std::string& other) {
			*this = other;
		};

		constexpr StringViewBase& operator=(const std::string_view& other) {
			dataVal = other.data();
			sizeVal = other.size();
			return *this;
		}

		constexpr StringViewBase(const std::string_view& other) {
			*this = other;
		};

		template<size_t strLength> constexpr StringViewBase(const value_type (&other)[strLength]) {
			dataVal = other;
			sizeVal = strLength;
		};

		constexpr StringViewBase(const_pointer _Ntcts) noexcept : dataVal(_Ntcts), sizeVal(traits_type::length(_Ntcts)) {
		}

		constexpr StringViewBase(const_pointer _Cts, const size_type countNew) noexcept : dataVal(_Cts), sizeVal(countNew){};

		constexpr const_iterator cbegin() const {
			return const_iterator(dataVal);
		}

		constexpr const_iterator cend() const {
			return const_iterator(dataVal + sizeVal);
		}

		constexpr const_iterator begin() const {
			return const_iterator(dataVal);
		}

		constexpr const_iterator end() const {
			return const_iterator(dataVal + sizeVal);
		}

		constexpr const_reverse_iterator crbegin() const {
			return reverse_iterator(end());
		}

		constexpr const_reverse_iterator crend() const {
			return reverse_iterator(begin());
		}

		constexpr reverse_iterator rbegin() const {
			return reverse_iterator(end());
		}

		constexpr reverse_iterator rend() const {
			return reverse_iterator(begin());
		}

		constexpr size_type size() const {
			return sizeVal;
		}

		constexpr size_type length() const {
			return sizeVal;
		}

		constexpr bool empty() const {
			return sizeVal == 0;
		}

		constexpr const_pointer data() const {
			return dataVal;
		}

		constexpr size_type max_size() const {
			return std::min(static_cast<size_t>(std::numeric_limits<std::ptrdiff_t>::max()), static_cast<size_t>(-1) / sizeof(value_type));
		}

		constexpr const_reference operator[](const size_type offsetNew) const {
			return dataVal[offsetNew];
		}

		constexpr const_reference front() const {
			return dataVal[0];
		}

		constexpr const_reference back() const {
			return dataVal[sizeVal - 1];
		}

		constexpr void swap(StringViewBase& other) {
			const StringViewBase temp{ other };
			other = *this;
			*this = temp;
		}

		constexpr StringViewBase substr(const size_type offsetNew = 0, size_type countNew = npos) const {
			return StringViewBase(dataVal + offsetNew, countNew);
		}

		inline explicit operator String() const {
			String returnValue{};
			returnValue.resize(sizeVal);
			std::memcpy(returnValue.data(), data(), returnValue.size());
			return returnValue;
		}

		inline explicit operator std::string() const {
			std::string returnString{};
			returnString.resize(size());
			std::memcpy(returnString.data(), data(), size());
			return returnString;
		}

		constexpr explicit operator std::string_view() const {
			return { data(), size() };
		}

		template<JsonifierInternal::HasDataAndSize StringType> constexpr bool operator==(const StringType& rhs) const {
			if (rhs.size() != size()) {
				return false;
			}
			return JsonifierInternal::JsonifierCoreInternal::compare(rhs.data(), data(), rhs.size());
		}

		template<size_type StrLength> constexpr bool operator==(const value_type (&rhs)[StrLength]) const {
			if (StrLength != size()) {
				return false;
			}
			return JsonifierInternal::JsonifierCoreInternal::compare(rhs, data(), StrLength);
		}

		constexpr ~StringViewBase() noexcept = default;
	};

	using StringView = StringViewBase<char>;

	inline std::basic_ostream<Jsonifier::StringViewBase<char>::value_type, Jsonifier::StringViewBase<char>::traits_type>& operator<<(
		std::basic_ostream<Jsonifier::StringViewBase<char>::value_type, Jsonifier::StringViewBase<char>::traits_type>& oStream,
		const Jsonifier::StringViewBase<char>& string) {
		return insertString<Jsonifier::StringViewBase<char>::value_type, Jsonifier::StringViewBase<char>::traits_type>(oStream, string.data(),
			string.size());
	}

	inline String operator+(const char* lhs, const StringView& rhs) {
		String newString(lhs);
		newString += rhs;
		return newString;
	}

	template<typename ValueType> inline std::string operator+=(const std::string& lhs, const StringViewBase<ValueType>& rhs) {
		String newString(lhs);
		newString += rhs;
		return newString;
	}

}
