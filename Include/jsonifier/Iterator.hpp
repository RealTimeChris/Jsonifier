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
		using iterator_concept	= std::contiguous_iterator_tag;
		using iterator_category = std::random_access_iterator_tag;
		using value_type		= ValueType;
		using difference_type	= ptrdiff_t;
		using reference			= value_type&;
		using pointer			= value_type*;

		constexpr Iterator(pointer pointerNew) noexcept : value{ pointerNew } {};

		constexpr reference operator*() const {
			return *value;
		}

		constexpr pointer operator->() const {
			return value;
		}

		constexpr Iterator& operator++() {
			++value;
			return *this;
		}

		constexpr Iterator operator++(int32_t) {
			Iterator temp = *this;
			++*this;
			return temp;
		}

		constexpr Iterator& operator--() {
			--value;
			return *this;
		}

		constexpr Iterator operator--(int32_t) {
			Iterator temp = *this;
			--*this;
			return temp;
		}

		constexpr Iterator& operator+=(const difference_type iter) {
			value += iter;
			return *this;
		}

		constexpr Iterator operator+(const difference_type iter) const {
			Iterator temp = *this;
			temp += iter;
			return temp;
		}

		constexpr Iterator& operator-=(const difference_type iter) {
			return *this += -iter;
		}

		constexpr Iterator operator-(const difference_type iter) const {
			Iterator temp = *this;
			temp -= iter;
			return temp;
		}

		constexpr reference operator[](const difference_type iter) const {
			return *(*this + iter);
		}

		constexpr difference_type operator-(const Iterator& iter) const {
			return value - iter.value;
		}

		constexpr bool operator==(const Iterator& iter) const {
			return value == iter.value;
		}

		constexpr bool operator>=(const Iterator& iter) const {
			return value >= iter.value;
		}

		constexpr bool operator<=(const Iterator& iter) const {
			return value <= iter.value;
		}

		constexpr bool operator>(const Iterator& iter) const {
			return value > iter.value;
		}

		constexpr bool operator<(const Iterator& iter) const {
			return value < iter.value;
		}

		pointer value;
	};

}// namespace JsonifierInternal
