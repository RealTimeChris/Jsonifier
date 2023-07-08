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

	class StringView {
	  public:
		using traits_type = std::char_traits<char>;
		using value_type = char;
		using pointer = char*;
		using const_pointer = const char*;
		using reference = char&;
		using const_reference = const char&;
		using iterator = JsonifierInternal::Iterator<char>;
		using const_iterator = const iterator;
		using reverse_iterator = std::reverse_iterator<iterator>;
		using const_reverse_iterator = const reverse_iterator;
		using size_type = size_t;
		using difference_type = ptrdiff_t;

		const_pointer dataVal{};
		size_type sizeVal{};

		inline static constexpr auto npos{ static_cast<size_type>(-1) };

		inline constexpr StringView() noexcept : dataVal(), sizeVal(0) {
		}

		inline constexpr StringView(const StringView&) noexcept = default;
		inline constexpr StringView& operator=(const StringView&) noexcept = default;

		inline constexpr StringView& operator=(const String& other) noexcept {
			dataVal = other.data();
			sizeVal = other.size();
			return *this;
		}

		inline constexpr StringView(const String& other) noexcept {
			*this = other;
		};

		inline StringView& operator=(const std::string& other) noexcept {
			dataVal = other.data();
			sizeVal = other.size();
			return *this;
		}

		inline StringView(const std::string& other) noexcept {
			*this = other;
		};

		inline constexpr StringView& operator=(const std::string_view& other) noexcept {
			dataVal = other.data();
			sizeVal = other.size();
			return *this;
		}

		inline constexpr StringView(const std::string_view& other) noexcept {
			*this = other;
		};

		template<size_t strLength> inline StringView(const char (&other)[strLength]) noexcept {
			dataVal = other;
			sizeVal = strLength;
		};

		inline constexpr StringView(const_pointer _Ntcts) noexcept : dataVal(_Ntcts), sizeVal(traits_type::length(_Ntcts)) {
		}

		inline constexpr StringView(const_pointer _Cts, const size_type countNew) noexcept : dataVal(_Cts), sizeVal(countNew){};

		inline constexpr const_iterator begin() const noexcept {
			return const_iterator(dataVal, sizeVal, 0);
		}

		inline constexpr const_iterator end() const noexcept {
			return const_iterator(dataVal, sizeVal, sizeVal);
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
			return std::min(static_cast<size_t>(std::numeric_limits<std::ptrdiff_t>::max()), static_cast<size_t>(-1) / sizeof(char));
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

		inline constexpr void swap(StringView& other) noexcept {
			const StringView temp{ other };
			other = *this;
			*this = temp;
		}

		inline constexpr StringView substr(const size_type offsetNew = 0, size_type countNew = npos) const noexcept {
			return StringView(dataVal + offsetNew, countNew);
		}

		inline explicit operator String() const noexcept {
			String returnValue{};
			returnValue.resize(sizeVal);
			std::memcpy(returnValue.data(), data(), returnValue.size());
			return returnValue;
		}

		inline explicit operator std::string() const noexcept {
			return { data(), size() };
		}

		inline explicit operator std::string_view() const noexcept {
			return { data(), size() };
		}

		template<typename ValueType> inline constexpr bool operator==(const ValueType& rhs) const {
			if (rhs.size() != size()) {
				return false;
			}
			return JsonifierInternal::JsonifierCoreInternal::compare(rhs.data(), data(), rhs.size());
		}

		inline constexpr ~StringView() noexcept = default;
	};

	inline std::basic_ostream<Jsonifier::StringView::value_type, Jsonifier::StringView::traits_type>& operator<<(
		std::basic_ostream<Jsonifier::StringView::value_type, Jsonifier::StringView::traits_type>& oStream, const Jsonifier::StringView& string) {
		return insertString<Jsonifier::StringView::value_type, Jsonifier::StringView::traits_type>(oStream, string.data(), string.size());
	}

}
