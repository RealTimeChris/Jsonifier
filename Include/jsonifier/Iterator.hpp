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
/// Feb 3, 2023
#pragma once

#include <iterator>

namespace JsonifierInternal {

	template<typename ValueType> class Iterator {
	  public:
		using iterator_concept = std::contiguous_iterator_tag;
		using iterator_category = std::random_access_iterator_tag;
		using value_type = typename ValueType::value_type;
		using difference_type = typename ValueType::difference_type;
		using pointer = typename ValueType::pointer;
		using reference = value_type&;

		constexpr Iterator() noexcept : ptr() {
		}

		constexpr Iterator(pointer pointerNew) noexcept : ptr(pointerNew) {
		}

		constexpr reference operator*() const noexcept {
			return *ptr;
		}

		constexpr pointer operator->() const noexcept {
			return ptr;
		}

		constexpr Iterator& operator++() noexcept {
			++ptr;
			return *this;
		}

		constexpr Iterator operator++(int32_t) noexcept {
			Iterator temp = *this;
			++*this;
			return temp;
		}

		constexpr Iterator& operator--() noexcept {
			--ptr;
			return *this;
		}

		constexpr Iterator operator--(int32_t) noexcept {
			Iterator temp = *this;
			--*this;
			return temp;
		}

		constexpr Iterator& operator+=(const difference_type iter) noexcept {
			ptr += iter;
			return *this;
		}

		constexpr Iterator operator+(const difference_type iter) const noexcept {
			Iterator temp = *this;
			temp += iter;
			return temp;
		}

		constexpr Iterator& operator-=(const difference_type iter) noexcept {
			return *this += -iter;
		}

		constexpr Iterator operator-(const difference_type iter) const noexcept {
			Iterator temp = *this;
			temp -= iter;
			return temp;
		}

		constexpr difference_type operator-(const Iterator& _Right) const noexcept {
			return ptr - _Right.ptr;
		}

		constexpr reference operator[](const difference_type iter) const noexcept {
			return *(*this + iter);
		}

		constexpr bool operator==(const Iterator& _Right) const noexcept {
			return ptr == _Right.ptr;
		}

		constexpr bool operator>=(const Iterator& iter) const noexcept {
			return ptr >= iter.ptr;
		}

		constexpr bool operator <= (const Iterator& iter) const noexcept {
			return ptr >= iter.ptr;
		}

		constexpr bool operator>(const Iterator& iter) const noexcept {
			return ptr > iter.ptr;
		}

		constexpr bool operator<(const Iterator& iter) const noexcept {
			return ptr > iter.ptr;
		}

		pointer ptr;
	};

	template<typename ValueType> class ConstIterator  {
	  public:
		using iterator_concept = std::contiguous_iterator_tag;
		using iterator_category = std::random_access_iterator_tag;
		using value_type = typename ValueType::value_type;
		using difference_type = typename ValueType::difference_type;
		using const_pointer = typename ValueType::const_pointer;
		using reference = const value_type&;

		using pointer = typename ValueType::pointer;

		constexpr ConstIterator() noexcept : ptr() {
		}

		constexpr ConstIterator(const_pointer pointerNew) noexcept : ptr(pointerNew) {}

		constexpr reference operator*() const noexcept {
			return *ptr;
		}

		constexpr const_pointer operator->() const noexcept {
			return ptr;
		}

		constexpr ConstIterator& operator++() noexcept {
			++ptr;
			return *this;
		}

		constexpr ConstIterator operator++(int32_t) noexcept {
			ConstIterator temp = *this;
			++*this;
			return temp;
		}

		constexpr ConstIterator& operator--() noexcept {
			--ptr;
			return *this;
		}

		constexpr ConstIterator operator--(int32_t) noexcept {
			ConstIterator temp = *this;
			--*this;
			return temp;
		}

		constexpr ConstIterator& operator+=(const difference_type iter) noexcept {
			ptr += iter;
			return *this;
		}

		constexpr ConstIterator operator+(const difference_type iter) const noexcept {
			ConstIterator temp = *this;
			temp += iter;
			return temp;
		}

		constexpr ConstIterator& operator-=(const difference_type iter) noexcept {
			return *this += -iter;
		}

		constexpr ConstIterator operator-(const difference_type iter) const noexcept {
			ConstIterator temp = *this;
			temp -= iter;
			return temp;
		}

		constexpr difference_type operator-(const ConstIterator& _Right) const noexcept {
			return ptr - _Right.ptr;
		}

		constexpr reference operator[](const difference_type iter) const noexcept {
			return *(*this + iter);
		}

		constexpr bool operator==(const ConstIterator& _Right) const noexcept {
			return ptr == _Right.ptr;
		}

		constexpr bool operator>=(const ConstIterator& iter) const noexcept {
			return ptr >= iter.ptr;
		}

		constexpr bool operator<=(const ConstIterator& iter) const noexcept {
			return ptr >= iter.ptr;
		}

		constexpr bool operator>(const ConstIterator& iter) const noexcept {
			return ptr > iter.ptr;
		}

		constexpr bool operator<(const ConstIterator& iter) const noexcept {
			return ptr > iter.ptr;
		}

		const_pointer ptr;
	};

}
