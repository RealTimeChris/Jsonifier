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
/// https://github.com/RealTimeChris/jsonifier
/// Feb 3, 2023
#pragma once

#include <iterator>

namespace jsonifier_internal {

	template<typename value_type_new> class iterator {
	  public:
		using iterator_concept	= std::contiguous_iterator_tag;
		using iterator_category = std::random_access_iterator_tag;
		using value_type		= value_type_new;
		using difference_type	= std::ptrdiff_t;
		using pointer			= value_type*;
		using reference			= value_type&;
		using size_type			= int64_t;

		constexpr iterator() noexcept = default;

		constexpr iterator(pointer pointerNew) : value{ pointerNew } {};

		constexpr reference operator*() const {
			return *value;
		}

		constexpr pointer operator->() const {
			return value;
		}

		constexpr iterator& operator++() {
			++value;
			return *this;
		}

		constexpr iterator operator++(int32_t) {
			iterator temp{ *this };
			++*this;
			return temp;
		}

		constexpr iterator& operator--() {
			--value;
			return *this;
		}

		constexpr iterator operator--(int32_t) {
			iterator temp{ *this };
			--*this;
			return temp;
		}

		constexpr iterator& operator+=(const difference_type iter) {
			value += iter;
			return *this;
		}

		constexpr iterator operator+(const difference_type iter) const {
			iterator temp{ *this };
			temp += iter;
			return temp;
		}

		constexpr iterator& operator-=(const difference_type iter) {
			return *this += -iter;
		}

		constexpr iterator operator-(const difference_type iter) const {
			iterator temp{ *this };
			temp -= iter;
			return temp;
		}

		constexpr reference operator[](const difference_type iter) const {
			return *(*this + iter);
		}

		constexpr difference_type operator-(const iterator& iter) const {
			return value - iter.value;
		}

		constexpr bool operator==(const iterator& iter) const {
			return value == iter.value;
		}

		constexpr bool operator>=(const iterator& iter) const {
			return value >= iter.value;
		}

		constexpr bool operator<=(const iterator& iter) const {
			return value <= iter.value;
		}

		constexpr bool operator>(const iterator& iter) const {
			return value > iter.value;
		}

		constexpr bool operator<(const iterator& iter) const {
			return value < iter.value;
		}

		pointer value;
	};

}// namespace jsonifier_internal
