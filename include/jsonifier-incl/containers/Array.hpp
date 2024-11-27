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

#include <jsonifier-incl/core/Config.hpp>
#include <jsonifier-incl/containers/Iterator.hpp>

namespace jsonifier::internal {

	template<typename value_type_new, uint64_t sizeNew> class array {
	  public:
		static_assert(std::is_object_v<value_type_new>, "The C++ Standard forbids containers of non-object types because of [container.requirements].");

		using value_type			 = value_type_new;
		using size_type				 = uint64_t;
		using difference_type		 = ptrdiff_t;
		using pointer				 = value_type*;
		using const_pointer			 = const value_type*;
		using reference				 = value_type&;
		using const_reference		 = const value_type&;
		using iterator				 = array_iterator<value_type, sizeNew>;
		using const_iterator		 = const array_iterator<value_type, sizeNew>;
		using reverse_iterator		 = std::reverse_iterator<iterator>;
		using const_reverse_iterator = std::reverse_iterator<const_iterator>;

		inline constexpr void fill(const value_type& value) {
			std::fill_n(dataVal, sizeNew, value);
		}

		inline constexpr void swap(array& other) noexcept(std::is_nothrow_swappable<value_type>::value) {
			std::swap_ranges(dataVal, dataVal + sizeNew, other.dataVal);
		}

		inline constexpr iterator begin() noexcept JSONIFIER_LIFETIME_BOUND {
			return iterator(dataVal);
		}

		inline constexpr const_iterator begin() const noexcept JSONIFIER_LIFETIME_BOUND {
			return const_iterator(dataVal);
		}

		inline constexpr iterator end() noexcept JSONIFIER_LIFETIME_BOUND {
			return iterator(dataVal);
		}

		inline constexpr const_iterator end() const noexcept JSONIFIER_LIFETIME_BOUND {
			return const_iterator(dataVal);
		}

		inline constexpr reverse_iterator rbegin() noexcept JSONIFIER_LIFETIME_BOUND {
			return reverse_iterator(end());
		}

		inline constexpr const_reverse_iterator rbegin() const noexcept JSONIFIER_LIFETIME_BOUND {
			return const_reverse_iterator(end());
		}

		inline constexpr reverse_iterator rend() noexcept JSONIFIER_LIFETIME_BOUND {
			return reverse_iterator(begin());
		}

		inline constexpr const_reverse_iterator rend() const noexcept JSONIFIER_LIFETIME_BOUND {
			return const_reverse_iterator(begin());
		}

		inline constexpr const_iterator cbegin() const noexcept JSONIFIER_LIFETIME_BOUND {
			return begin();
		}

		inline constexpr const_iterator cend() const noexcept JSONIFIER_LIFETIME_BOUND {
			return end();
		}

		inline constexpr const_reverse_iterator crbegin() const noexcept JSONIFIER_LIFETIME_BOUND {
			return rbegin();
		}

		inline constexpr const_reverse_iterator crend() const noexcept JSONIFIER_LIFETIME_BOUND {
			return rend();
		}

		inline constexpr size_type size() const noexcept {
			return sizeNew;
		}

		inline constexpr size_type max_size() const noexcept {
			return sizeNew;
		}

		inline constexpr bool empty() const noexcept {
			return false;
		}

		inline constexpr reference at(size_type position) JSONIFIER_LIFETIME_BOUND {
			if (sizeNew <= position) {
				std::runtime_error{ "invalid array<T, N> subscript" };
			}

			return dataVal[position];
		}

		inline constexpr const_reference at(size_type position) const JSONIFIER_LIFETIME_BOUND {
			if (sizeNew <= position) {
				std::runtime_error{ "invalid array<T, N> subscript" };
			}

			return dataVal[position];
		}

		inline constexpr reference operator[](size_type position) noexcept JSONIFIER_LIFETIME_BOUND {
			return dataVal[position];
		}

		inline constexpr const_reference operator[](size_type position) const noexcept JSONIFIER_LIFETIME_BOUND {
			return dataVal[position];
		}

		inline constexpr reference front() noexcept JSONIFIER_LIFETIME_BOUND {
			return dataVal[0];
		}

		inline constexpr const_reference front() const noexcept JSONIFIER_LIFETIME_BOUND {
			return dataVal[0];
		}

		inline constexpr reference back() noexcept JSONIFIER_LIFETIME_BOUND {
			return dataVal[sizeNew - 1];
		}

		inline constexpr const_reference back() const noexcept JSONIFIER_LIFETIME_BOUND {
			return dataVal[sizeNew - 1];
		}

		inline constexpr value_type* data() noexcept JSONIFIER_LIFETIME_BOUND {
			return dataVal;
		}

		inline constexpr const value_type* data() const noexcept JSONIFIER_LIFETIME_BOUND {
			return dataVal;
		}

		value_type dataVal[sizeNew];
	};

	struct empty_array_element {};

	template<class value_type_new> class array<value_type_new, 0> {
	  public:
		static_assert(std::is_object_v<value_type_new>, "The C++ Standard forbids containers of non-object types because of [container.requirements].");

		using value_type			 = value_type_new;
		using size_type				 = uint64_t;
		using difference_type		 = ptrdiff_t;
		using pointer				 = value_type*;
		using const_pointer			 = const value_type*;
		using reference				 = value_type&;
		using const_reference		 = const value_type&;
		using iterator				 = array_iterator<value_type, 0>;
		using const_iterator		 = const array_iterator<value_type, 0>;
		using reverse_iterator		 = std::reverse_iterator<iterator>;
		using const_reverse_iterator = std::reverse_iterator<const_iterator>;

		inline constexpr void fill(const value_type&) {
		}

		inline constexpr void swap(array&) noexcept {
		}

		inline constexpr iterator begin() noexcept {
			return iterator{};
		}

		inline constexpr const_iterator begin() const noexcept {
			return const_iterator{};
		}

		inline constexpr iterator end() noexcept {
			return iterator{};
		}

		inline constexpr const_iterator end() const noexcept {
			return const_iterator{};
		}

		inline constexpr reverse_iterator rbegin() noexcept {
			return reverse_iterator(end());
		}

		inline constexpr const_reverse_iterator rbegin() const noexcept {
			return const_reverse_iterator(end());
		}

		inline constexpr reverse_iterator rend() noexcept {
			return reverse_iterator(begin());
		}

		inline constexpr const_reverse_iterator rend() const noexcept {
			return const_reverse_iterator(begin());
		}

		inline constexpr const_iterator cbegin() const noexcept {
			return begin();
		}

		inline constexpr const_iterator cend() const noexcept {
			return end();
		}

		inline constexpr const_reverse_iterator crbegin() const noexcept {
			return rbegin();
		}

		inline constexpr const_reverse_iterator crend() const noexcept {
			return rend();
		}

		inline constexpr size_type size() const noexcept {
			return 0;
		}

		inline constexpr size_type max_size() const noexcept {
			return 0;
		}

		inline constexpr bool empty() const noexcept {
			return true;
		}

		inline constexpr reference at(size_type) {
			std::runtime_error{ "invalid array<T, N> subscript" };
		}

		inline constexpr const_reference at(size_type) const {
			std::runtime_error{ "invalid array<T, N> subscript" };
		}

		inline constexpr reference operator[](size_type) noexcept {
			return *data();
		}

		inline constexpr const_reference operator[](size_type) const noexcept {
			return *data();
		}

		inline constexpr reference front() noexcept {
			return *data();
		}

		inline constexpr const_reference front() const noexcept {
			return *data();
		}

		inline constexpr reference back() noexcept {
			return *data();
		}

		inline constexpr const_reference back() const noexcept {
			return *data();
		}

		inline constexpr value_type* data() noexcept {
			return nullptr;
		}

		inline constexpr const value_type* data() const noexcept {
			return nullptr;
		}

		conditional_t<std::disjunction_v<std::is_default_constructible<value_type>, std::is_default_constructible<value_type>>, value_type, empty_array_element> dataVal[1]{};
	};

}
