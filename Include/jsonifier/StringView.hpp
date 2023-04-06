/*
	Jsonifier - For parsing and serializing Json - very rapidly.
	Copyright (C) 2023 Chris M. (RealTimeChris)

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	This library is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
	Lesser General Public License for more details.

	You should have received a copy of the GNU Lesser General Public
	License along with this library; if not, write to the Free Software
	Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301
	USA
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

	using StringViewPtr = const uint8_t*;
	using StructuralIndex = uint32_t*;
	using StringBufferPtr = uint8_t*;

	class StringView {
	  public:
		using traits_type = std::char_traits<char>;
		using value_type = char;
		using pointer = char*;
		using const_pointer = const char*;
		using reference = char&;
		using const_reference = const char&;
		using iterator = StringIterator;
		using const_iterator = const iterator;
		using reverse_iterator = std::reverse_iterator<iterator>;
		using const_reverse_iterator = const reverse_iterator;
		using size_type = size_t;
		using difference_type = ptrdiff_t;

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

		inline constexpr StringView& operator=(const std::string& other) noexcept {
			dataVal = other.data();
			sizeVal = other.size();
			return *this;
		}

		inline constexpr StringView(const std::string& other) noexcept {
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

		inline constexpr StringView(const const_pointer _Ntcts) noexcept : dataVal(_Ntcts), sizeVal(traits_type::length(_Ntcts)) {
		}

		inline constexpr StringView(const const_pointer _Cts, const size_type countNew) noexcept : dataVal(_Cts), sizeVal(countNew){};

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
			return (std::min)(static_cast<size_t>(std::numeric_limits<std::ptrdiff_t>::max()), static_cast<size_t>(-1) / sizeof(char));
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

		inline constexpr explicit operator std::string() const noexcept {
			std::string returnValue{};
			returnValue.resize(sizeVal);
			std::copy(data(), data() + returnValue.size(), returnValue.data());
			return returnValue;
		}

		inline constexpr explicit operator std::string_view() const noexcept {
			return std::string_view{ data(), size() };
		}

		inline constexpr friend bool operator==(const StringView& lhs, const StringView& rhs) {
			if (lhs.size() != rhs.size()) {
				return false;
			}
			if (std::is_constant_evaluated()) {
				return stringConstCompare(lhs, rhs);
			} else {
				return JsonifierCoreInternal::compare(lhs.data(), rhs.data(), lhs.size());
			}
		}

		inline constexpr friend bool operator!=(const StringView& lhs, const StringView& rhs) {
			return !(lhs == rhs);
		}

		template<typename OTy> inline constexpr friend bool operator==(const StringView& lhs, const OTy& rhs) {
			if (lhs.size() != rhs.size()) {
				return false;
			}
			if (std::is_constant_evaluated()) {
				return stringConstCompare(lhs, rhs);
			} else {
				return JsonifierCoreInternal::compare(lhs.data(), rhs.data(), lhs.size());
			}
		}

		template<typename OTy> inline constexpr friend bool operator!=(const StringView& lhs, const OTy& rhs) {
			return !(lhs == rhs);
		}

		template<typename OTy> inline constexpr friend bool operator==(const OTy& lhs, const StringView& rhs) {
			if (lhs.size() != rhs.size()) {
				return false;
			}
			if (std::is_constant_evaluated()) {
				return stringConstCompare(lhs, rhs);
			} else {
				return JsonifierCoreInternal::compare(lhs.data(), rhs.data(), lhs.size());
			}
		}

		template<typename OTy> inline constexpr friend bool operator!=(const OTy& lhs, const StringView& rhs) {
			return !(lhs == rhs);
		}

		inline constexpr ~StringView() noexcept = default;

	  protected:
		const_pointer dataVal{};
		size_type sizeVal{};
	};

	inline std::ostream& operator<<(std::ostream& os, const StringView& string) {
		os << string.data();
		return os;
	}

	inline constexpr String& String::operator=(const StringView& other) {
		resize(other.size());
		std::copy(other.data(), other.data() + capacityVal, ptr.get());
		return *this;
	}

	inline constexpr String::String(const StringView& other) {
		*this = other;
	}

	template<size_t strLength> class StringLiteral {
	  public:
		static constexpr size_t sizeVal = (strLength > 0) ? (strLength - 1) : 0;

		inline constexpr StringLiteral() noexcept = default;

		inline constexpr StringLiteral(const char (&str)[strLength]) {
			std::copy_n(str, strLength, string);
		}

		inline constexpr const char* data() const noexcept {
			return string;
		}

		inline constexpr const char* begin() const noexcept {
			return string;
		}

		inline constexpr const char* end() const noexcept {
			return string + sizeVal;
		}

		inline constexpr size_t size() const noexcept {
			return sizeVal;
		}

		inline constexpr const StringView stringView() const noexcept {
			return { string, sizeVal };
		}

		char string[strLength];
	};

}
