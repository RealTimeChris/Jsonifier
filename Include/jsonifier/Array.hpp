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
/// Nov 13, 2023
#pragma once

#include <jsonifier/TypeEntities.hpp>

#include <initializer_list>
#include <concepts>
#include <cstdint>
#include <cstring>
#include <array>

namespace jsonifier_internal {

#include <cstddef>
#include <initializer_list>
#include <stdexcept>
#include <iterator>
#include <algorithm>

	template<typename value_type_new, std::size_t sizeNew> class array {
	  public:
		using value_type	  = value_type_new;
		using size_type		  = std::size_t;
		using difference_type = std::ptrdiff_t;
		using pointer		  = value_type*;
		using const_pointer	  = const value_type*;
		using reference		  = value_type&;
		using const_reference = const value_type&;
		using iterator		  = value_type*;
		using const_iterator  = const value_type*;

		constexpr array() : data{} {};

		constexpr array(const std::initializer_list<value_type>& list) {
			if (list.size() > sizeNew) {
				throw std::out_of_range("Initializer list is too long");
			}
			std::copy(list.begin(), list.end(), data);
		}

		constexpr reference at(size_type pos) {
			if (pos >= sizeNew) {
				throw std::out_of_range("Index out of bounds");
			}
			return data[pos];
		}

		constexpr const_reference at(size_type pos) const {
			if (pos >= sizeNew) {
				throw std::out_of_range("Index out of bounds");
			}
			return data[pos];
		}

		constexpr reference operator[](size_type pos) {
			return data[pos];
		}

		constexpr const_reference operator[](size_type pos) const {
			return data[pos];
		}

		constexpr reference front() {
			return data[0];
		}

		constexpr const_reference front() const {
			return data[0];
		}

		constexpr reference back() {
			return data[sizeNew - 1];
		}

		constexpr const_reference back() const {
			return data[sizeNew - 1];
		}

		constexpr pointer dataPtr() noexcept {
			return data;
		}

		constexpr const_pointer dataPtr() const noexcept {
			return data;
		}

		constexpr iterator begin() noexcept {
			return data;
		}

		constexpr const_iterator begin() const noexcept {
			return data;
		}

		constexpr const_iterator cbegin() const noexcept {
			return data;
		}

		constexpr iterator end() noexcept {
			return data + sizeNew;
		}

		constexpr const_iterator end() const noexcept {
			return data + sizeNew;
		}

		constexpr const_iterator cend() const noexcept {
			return data + sizeNew;
		}

		constexpr size_type size() const noexcept {
			return sizeNew;
		}

		constexpr bool empty() const noexcept {
			return sizeNew == 0;
		}

		constexpr void fill(const value_type& value) {
			std::fill_n(data, sizeNew, value);
		}

		constexpr void swap(array& other) noexcept {
			std::swap_ranges(data, data + sizeNew, other.data);
		}

	  private:
		value_type data[sizeNew];
	};

	template<typename value_type, std::size_t size> constexpr void swap(array<value_type, size>& lhs, array<value_type, size>& rhs) noexcept {
		lhs.swap(rhs);
	}

}