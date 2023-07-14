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
		using value_type = ValueType;
		using traits_type = std::char_traits<value_type>;
		using pointer = value_type*;
		using const_pointer = const value_type*;
		using reference = value_type&;
		using const_reference = const value_type&;
		using iterator = JsonifierInternal::Iterator<value_type>;
		using const_iterator = const iterator;
		using reverse_iterator = std::reverse_iterator<iterator>;
		using const_reverse_iterator = const reverse_iterator;
		using size_type = size_t;
		using difference_type = ptrdiff_t;

		const_pointer dataVal{};
		size_type sizeVal{};

		inline static constexpr auto npos{ static_cast<size_type>(-1) };

		inline constexpr StringViewBase() noexcept : dataVal(), sizeVal(0) {
		}

		inline constexpr StringViewBase(const StringViewBase&) noexcept = default;
		inline constexpr StringViewBase& operator=(const StringViewBase&) noexcept = default;

		StringViewBase& operator=(const StringBase<value_type>& stringNew) {
			dataVal = stringNew.data();
			sizeVal = stringNew.size();
			return *this;
		}

		StringViewBase(const StringBase<value_type>& stringNew) {
			*this = stringNew;
		}

		inline std::enable_if<std::same_as<char, value_type>, StringViewBase<value_type>>& operator=(const std::string& other) noexcept {
			dataVal = other.data();
			sizeVal = other.size();
			return *this;
		}

		inline StringViewBase(const std::string& other) noexcept {
			*this = other;
		};

		inline constexpr StringViewBase& operator=(const std::string_view& other) noexcept {
			dataVal = other.data();
			sizeVal = other.size();
			return *this;
		}

		inline constexpr StringViewBase(const std::string_view& other) noexcept {
			*this = other;
		};

		template<size_t strLength> inline StringViewBase(const value_type (&other)[strLength]) noexcept {
			dataVal = other;
			sizeVal = strLength;
		};

		inline constexpr StringViewBase(const_pointer _Ntcts) noexcept : dataVal(_Ntcts), sizeVal(traits_type::length(_Ntcts)) {
		}

		inline constexpr StringViewBase(const_pointer _Cts, const size_type countNew) noexcept : dataVal(_Cts), sizeVal(countNew){};

		inline constexpr const_iterator begin() const noexcept {
			return const_iterator(dataVal, 0);
		}

		inline constexpr const_iterator end() const noexcept {
			return const_iterator(dataVal, sizeVal);
		}

		inline constexpr size_type size() const noexcept {
			return sizeVal;
		}

		inline constexpr size_type length() const noexcept {
			return sizeVal;
		}

		inline constexpr bool empty() const noexcept {
			return sizeVal == 0;
		}

		inline constexpr const_pointer data() const noexcept {
			return dataVal;
		}

		inline constexpr size_type max_size() const noexcept {
			return std::min(static_cast<size_t>(std::numeric_limits<std::ptrdiff_t>::max()), static_cast<size_t>(-1) / sizeof(value_type));
		}

		inline constexpr const_reference operator[](const size_type offsetNew) const noexcept {
			return dataVal[offsetNew];
		}

		inline constexpr const_reference front() const noexcept {
			return dataVal[0];
		}

		inline constexpr const_reference back() const noexcept {
			return dataVal[sizeVal - 1];
		}

		inline constexpr void swap(StringViewBase& other) noexcept {
			const StringViewBase temp{ other };
			other = *this;
			*this = temp;
		}

		inline constexpr StringViewBase substr(const size_type offsetNew = 0, size_type countNew = npos) const noexcept {
			return StringViewBase(dataVal + offsetNew, countNew);
		}

		inline explicit operator String() const noexcept {
			String returnValue{};
			returnValue.resize(sizeVal);
			std::memcpy(returnValue.data(), data(), returnValue.size());
			return returnValue;
		}

		inline explicit operator std::string() const noexcept {
			std::string returnString{};
			returnString.resize(size());
			std::memcpy(returnString.data(), data(), size());
			return returnString;
		}

		inline explicit operator std::string_view() const noexcept {
			return { data(), size() };
		}

		template<typename ValueTypeNew> inline constexpr bool operator==(const ValueType& rhs) const {
			if (rhs.size() != size()) {
				return false;
			}
			return JsonifierInternal::JsonifierCoreInternal::compare(rhs.data(), data(), rhs.size());
		}

		inline constexpr ~StringViewBase() noexcept = default;
	};

	using StringView = StringViewBase<char>;

	inline std::basic_ostream<Jsonifier::StringViewBase<char>::value_type, Jsonifier::StringViewBase<char>::traits_type>& operator<<(
		std::basic_ostream<Jsonifier::StringViewBase<char>::value_type, Jsonifier::StringViewBase<char>::traits_type>& oStream,
		const Jsonifier::StringViewBase<char>& string) {
		return insertString<Jsonifier::StringViewBase<char>::value_type, Jsonifier::StringViewBase<char>::traits_type>(oStream, string.data(),
			string.size());
	}

}
