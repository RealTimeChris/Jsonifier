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

#include <jsonifier/Compare.hpp>
#include <limits>

namespace Jsonifier {

	class StringView;

	template<typename OTy1, typename OTy2> inline constexpr bool stringConstCompare(const OTy1& s0, const OTy2& s1) noexcept {
		if (s0.size() != s1.size()) [[unlikely]] {
			return false;
		}
		for (size_t x = 0; x < s0.size(); ++x) {
			if (s0[x] != s1[x]) [[unlikely]] {
				return false;
			}
		}
		return true;
	}

	template<typename OTy, const OTy& S> inline constexpr bool cxStringCompare(const OTy& key) noexcept {
		constexpr auto s = S;
		constexpr auto n = s.size();
		return (key.size() == n) && (key == s);
	}

	class StringIterator {
	  public:
		friend class StringView;
		friend class String;

		using iterator_concept = std::bidirectional_iterator_tag;
		using value_type = char;
		using difference_type = ptrdiff_t;
		using pointer = const value_type*;
		using reference = const value_type&;

		inline constexpr StringIterator() noexcept = default;

		inline constexpr reference operator*() const noexcept {
			return dataVal[offset];
		}

		inline constexpr pointer operator->() const noexcept {
			return dataVal + offset;
		}

		inline constexpr StringIterator& operator++() noexcept {
			++offset;
			return *this;
		}

		inline constexpr StringIterator operator++(int) noexcept {
			StringIterator temp{ *this };
			++*this;
			return temp;
		}

		inline constexpr StringIterator& operator--() noexcept {
			--offset;
			return *this;
		}

		inline constexpr StringIterator operator--(int) noexcept {
			StringIterator temp{ *this };
			--*this;
			return temp;
		}

		inline constexpr StringIterator& operator+=(const difference_type offsetNew) noexcept {
			offset += static_cast<size_t>(offsetNew);
			return *this;
		}

		inline constexpr StringIterator operator+(const difference_type offsetNew) const noexcept {
			StringIterator temp{ *this };
			temp += offsetNew;
			return temp;
		}

		friend inline constexpr StringIterator operator+(const difference_type offsetNew, StringIterator _Right) noexcept {
			_Right += offsetNew;
			return _Right;
		}

		inline constexpr StringIterator& operator-=(const difference_type offsetNew) noexcept {
			offset -= static_cast<size_t>(offsetNew);
			return *this;
		}

		inline constexpr StringIterator operator-(const difference_type offsetNew) const noexcept {
			StringIterator temp{ *this };
			temp -= offsetNew;
			return temp;
		}

		inline constexpr difference_type operator-(const StringIterator& _Right) const noexcept {
			return static_cast<difference_type>(offset - _Right.offset);
		}

		inline constexpr reference operator[](const difference_type offsetNew) const noexcept {
			return *(*this + offsetNew);
		}

		inline constexpr bool operator==(const StringIterator& _Right) const noexcept {
			return offset == _Right.offset;
		}

		inline constexpr bool operator!=(const StringIterator& _Right) const noexcept {
			return !(*this == _Right);
		}

		inline constexpr bool operator<(const StringIterator& _Right) const noexcept {
			return offset < _Right.offset;
		}

		inline constexpr bool operator>(const StringIterator& _Right) const noexcept {
			return _Right < *this;
		}

		inline constexpr bool operator<=(const StringIterator& _Right) const noexcept {
			return !(_Right < *this);
		}

		inline constexpr bool operator>=(const StringIterator& _Right) const noexcept {
			return !(*this < _Right);
		}

	  protected:
		pointer dataVal = nullptr;
		size_t offset = 0;

		inline constexpr StringIterator(const pointer _Data, const size_t _Size, const size_t offsetNew) noexcept
			: dataVal(_Data), offset(offsetNew) {
		}
	};

	class String {
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

		inline constexpr String() noexcept = default;

		inline constexpr static size_t npos{ std::numeric_limits<size_type>::max() };

		inline constexpr String& operator=(String&& other) noexcept {
			if (this != &other) {
				std::swap(capacityVal, other.capacityVal);
				std::swap(sizeVal, other.sizeVal);
				std::swap(ptr, other.ptr);
			}
			return *this;
		}

		inline explicit String(String&& other) noexcept {
			*this = std::move(other);
		}

		inline constexpr String& operator=(const String& other) noexcept {
			if (this != &other) {
				resize(other.size());
				std::copy(other.ptr.get(), other.ptr.get() + capacityVal, ptr.get());
			}
			return *this;
		}

		inline constexpr String(const String& other) noexcept {
			*this = other;
		}

		inline constexpr String& operator=(value_type other) noexcept {
			if (other == '\0') {
				emplace_back(' ');
			} else {
				emplace_back(other);
			}
			return *this;
		}

		inline constexpr String(value_type other) noexcept {
			*this = other;
		}

		inline constexpr String& operator=(const std::string& other) noexcept {
			resize(other.size());
			std::copy(other.data(), other.data() + capacityVal, ptr.get());
			return *this;
		}

		inline constexpr String(const std::string& other) noexcept {
			*this = other;
		}

		inline constexpr String& operator=(const std::string_view& other) noexcept {
			resize(other.size());
			std::copy(other.data(), other.data() + capacityVal, ptr.get());
			return *this;
		}

		inline constexpr String(const std::string_view& other) noexcept {
			*this = other;
		}

		inline constexpr String& operator=(const StringView& other);

		inline constexpr explicit String(const StringView& other);

		inline constexpr String& operator=(const_pointer other) noexcept {
			resize(std::char_traits<value_type>::length(other));
			std::copy(other, other + capacityVal, ptr.get());
			return *this;
		}

		inline constexpr String(const_pointer other) noexcept {
			*this = other;
		}

		inline constexpr String substr(const size_t offset, size_t length) noexcept {
			return {};
		}

		inline constexpr const_iterator begin() const noexcept {
			return const_iterator(ptr.get(), sizeVal, 0);
		}

		inline constexpr const_iterator end() const noexcept {
			return const_iterator(ptr.get(), sizeVal, sizeVal);
		}

		inline constexpr value_type& operator[](size_t index) const noexcept {
			return ptr[index];
		}

		inline constexpr const_pointer c_str() const noexcept {
			return ptr.get();
		}

		inline constexpr explicit operator std::string() const noexcept {
			return std::string(data(), size());
		}

		inline constexpr void emplace_back(value_type arg) {
			if (sizeVal + 1 >= capacityVal) {
				reserve((capacityVal + 1) * 2);
			}
			ptr[sizeVal] = arg;
			++sizeVal;
			ptr[sizeVal] = '\0';
		}

		inline constexpr size_type capacity() const noexcept {
			return capacityVal;
		}

		inline constexpr size_type size() const noexcept {
			return sizeVal;
		}

		inline constexpr pointer data() const noexcept {
			return ptr.get();
		}

		inline constexpr void reserve(size_type newCapacity) {
			if (newCapacity <= capacityVal) {
				return;
			}
			std::unique_ptr<value_type[]> new_ptr{ new value_type[newCapacity] };
			std::copy(begin(), end(), new_ptr.get());
			ptr = std::move(new_ptr);
			capacityVal = newCapacity;
		}

		inline constexpr void resize(size_type count) {
			if (count < sizeVal) {
				ptr[count] = '\0';
				sizeVal = count;
				return;
			}
			reserve(count + 1);
			std::fill(ptr.get() + sizeVal, ptr.get() + count + 1, value_type{});
			sizeVal = count;
		}

		inline constexpr void resize(size_type count, value_type ch) {
			if (count < sizeVal) {
				ptr[count] = '\0';
				sizeVal = count;
				return;
			}
			reserve(count + 1);
			std::fill(ptr.get() + sizeVal, ptr.get() + count, ch);
			ptr[count] = '\0';
			sizeVal = count;
		}

		inline constexpr friend bool operator==(const String& lhs, const String& rhs) noexcept {
			if (lhs.size() != rhs.size()) {
				return false;
			}
			if (std::is_constant_evaluated()) {
				return stringConstCompare(lhs, rhs);
			} else {
				return JsonifierCoreInternal::compare(lhs.data(), rhs.data(), lhs.size());
			}
		}

		inline constexpr friend bool operator!=(const String& lhs, const String& rhs) noexcept {
			return !(lhs == rhs);
		}

		template<typename OTy> inline constexpr friend bool operator==(const String& lhs, const OTy& rhs) noexcept {
			if (lhs.size() != rhs.size()) {
				return false;
			}
			if (std::is_constant_evaluated()) {
				return stringConstCompare(lhs, rhs);
			} else {
				return JsonifierCoreInternal::compare(lhs.data(), rhs.data(), lhs.size());
			}
		}

		template<typename OTy> inline constexpr friend bool operator!=(const String& lhs, const OTy& rhs) noexcept {
			return !(lhs == rhs);
		}

		template<typename OTy> inline constexpr friend bool operator==(const OTy& lhs, const String& rhs) noexcept {
			if (lhs.size() != rhs.size()) {
				return false;
			}
			if (std::is_constant_evaluated()) {
				return stringConstCompare(lhs, rhs);
			} else {
				return JsonifierCoreInternal::compare(lhs.data(), rhs.data(), lhs.size());
			}
		}

		template<typename OTy> inline constexpr friend bool operator!=(const OTy& lhs, const String& rhs) noexcept {
			return !(lhs == rhs);
		}

		inline constexpr String& operator+=(const String& rhs) noexcept {
			auto oldSize = size();
			resize(oldSize + rhs.size());
			std::copy(rhs.data(), rhs.data() + rhs.size(), data() + oldSize);
			return *this;
		}

		inline constexpr String& operator+=(const char* rhs) noexcept {
			auto rhsSize = std::char_traits<char>::length(rhs);
			auto oldSize = size();
			resize(oldSize + rhsSize);
			std::copy(rhs, rhs + rhsSize, data() + oldSize);
			return *this;
		}

		inline constexpr ~String(){};

	  protected:
		std::unique_ptr<value_type[]> ptr{};
		size_type capacityVal{};
		size_type sizeVal{};
	};

	inline std::ostream& operator<<(std::ostream& os, const String& string) noexcept {
		os << string.data();
		return os;
	}

	inline constexpr String operator+(const String& lhs, const String& rhs) noexcept {
		String newString(lhs);
		newString += rhs;
		return newString;
	}

	inline constexpr String operator+(const String& lhs, const char* rhs) noexcept {
		String newString(lhs);
		newString += rhs;
		return newString;
	}

	inline constexpr String operator+(const char* lhs, const String& rhs) noexcept {
		String newString(lhs);
		newString += rhs;
		return newString;
	}

}
