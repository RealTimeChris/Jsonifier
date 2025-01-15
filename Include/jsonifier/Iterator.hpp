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
/// Feb 3, 2023
#pragma once

#include <iterator>

namespace jsonifier::internal {

	template<typename value_type_new> class basic_iterator {
	  public:
		using iterator_concept	= std::contiguous_iterator_tag;
		using iterator_category = std::random_access_iterator_tag;
		using value_type		= value_type_new;
		using difference_type	= std::ptrdiff_t;
		using pointer			= value_type*;
		using reference			= value_type&;

		JSONIFIER_INLINE constexpr basic_iterator() noexcept : ptr() {
		}

		JSONIFIER_INLINE constexpr basic_iterator(pointer ptrNew) noexcept : ptr(ptrNew) {
		}

		JSONIFIER_INLINE constexpr reference operator*() const noexcept {
			return *ptr;
		}

		JSONIFIER_INLINE constexpr pointer operator->() const noexcept {
			return std::pointer_traits<pointer>::pointer_to(**this);
		}

		JSONIFIER_INLINE constexpr basic_iterator& operator++() noexcept {
			++ptr;
			return *this;
		}

		JSONIFIER_INLINE constexpr basic_iterator operator++(int32_t) noexcept {
			basic_iterator temp = *this;
			++*this;
			return temp;
		}

		JSONIFIER_INLINE constexpr basic_iterator& operator--() noexcept {
			--ptr;
			return *this;
		}

		JSONIFIER_INLINE constexpr basic_iterator operator--(int32_t) noexcept {
			basic_iterator temp = *this;
			--*this;
			return temp;
		}

		JSONIFIER_INLINE constexpr basic_iterator& operator+=(const difference_type offSet) noexcept {
			ptr += offSet;
			return *this;
		}

		JSONIFIER_INLINE constexpr basic_iterator operator+(const difference_type offSet) const noexcept {
			basic_iterator temp = *this;
			temp += offSet;
			return temp;
		}

		JSONIFIER_INLINE friend constexpr basic_iterator operator+(const difference_type offSet, basic_iterator _Next) noexcept {
			_Next += offSet;
			return _Next;
		}

		JSONIFIER_INLINE constexpr basic_iterator& operator-=(const difference_type offSet) noexcept {
			return *this += -offSet;
		}

		JSONIFIER_INLINE constexpr basic_iterator operator-(const difference_type offSet) const noexcept {
			basic_iterator temp = *this;
			temp -= offSet;
			return temp;
		}

		JSONIFIER_INLINE constexpr difference_type operator-(const basic_iterator& other) const noexcept {
			return static_cast<difference_type>(ptr - other.ptr);
		}

		JSONIFIER_INLINE constexpr reference operator[](const difference_type offSet) const noexcept {
			return *(*this + offSet);
		}

		JSONIFIER_INLINE constexpr bool operator==(const basic_iterator& other) const noexcept {
			return ptr == other.ptr;
		}

		JSONIFIER_INLINE constexpr std::strong_ordering operator<=>(const basic_iterator& other) const noexcept {
			return ptr <=> other.ptr;
		}

		JSONIFIER_INLINE constexpr bool operator!=(const basic_iterator& other) const noexcept {
			return !(*this == other);
		}

		JSONIFIER_INLINE constexpr bool operator<(const basic_iterator& other) const noexcept {
			return ptr < other.ptr;
		}

		JSONIFIER_INLINE constexpr bool operator>(const basic_iterator& other) const noexcept {
			return other < *this;
		}

		JSONIFIER_INLINE constexpr bool operator<=(const basic_iterator& other) const noexcept {
			return !(other < *this);
		}

		JSONIFIER_INLINE constexpr bool operator>=(const basic_iterator& other) const noexcept {
			return !(*this < other);
		}

		pointer ptr;
	};

}// namespace internal