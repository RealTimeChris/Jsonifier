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

#include <jsonifier/Core/Config.hpp>
#include <jsonifier/Containers/Iterator.hpp>

namespace jsonifier::internal {

	template<typename value_type_new, size_t sizeNew> class array {
	  public:
		static_assert(std::is_object_v<value_type_new>, "The C++ Standard forbids containers of non-object types because of [container.requirements].");

		using value_type			 = value_type_new;
		using size_type				 = size_t;
		using difference_type		 = ptrdiff_t;
		using pointer				 = value_type*;
		using const_pointer			 = const value_type*;
		using reference				 = value_type&;
		using const_reference		 = const value_type&;
		using iterator				 = array_iterator<value_type, sizeNew>;
		using const_iterator		 = const array_iterator<value_type, sizeNew>;
		using reverse_iterator		 = std::reverse_iterator<iterator>;
		using const_reverse_iterator = std::reverse_iterator<const_iterator>;

		JSONIFIER_INLINE constexpr void fill(const value_type& _Value) {
			std::fill_n(dataVal, sizeNew, _Value);
		}

		JSONIFIER_INLINE constexpr void swap(array& _Other) noexcept(std::is_nothrow_swappable<value_type>::value) {
			std::swap_ranges(dataVal, dataVal + sizeNew, _Other.dataVal);
		}

		JSONIFIER_INLINE constexpr iterator begin() noexcept {
			return iterator(dataVal);
		}

		JSONIFIER_INLINE constexpr const_iterator begin() const noexcept {
			return const_iterator(dataVal);
		}

		JSONIFIER_INLINE constexpr iterator end() noexcept {
			return iterator(dataVal);
		}

		JSONIFIER_INLINE constexpr const_iterator end() const noexcept {
			return const_iterator(dataVal);
		}

		JSONIFIER_INLINE constexpr reverse_iterator rbegin() noexcept {
			return reverse_iterator(end());
		}

		JSONIFIER_INLINE constexpr const_reverse_iterator rbegin() const noexcept {
			return const_reverse_iterator(end());
		}

		JSONIFIER_INLINE constexpr reverse_iterator rend() noexcept {
			return reverse_iterator(begin());
		}

		JSONIFIER_INLINE constexpr const_reverse_iterator rend() const noexcept {
			return const_reverse_iterator(begin());
		}

		JSONIFIER_INLINE constexpr const_iterator cbegin() const noexcept {
			return begin();
		}

		JSONIFIER_INLINE constexpr const_iterator cend() const noexcept {
			return end();
		}

		JSONIFIER_INLINE constexpr const_reverse_iterator crbegin() const noexcept {
			return rbegin();
		}

		JSONIFIER_INLINE constexpr const_reverse_iterator crend() const noexcept {
			return rend();
		}

		JSONIFIER_INLINE constexpr size_type size() const noexcept {
			return sizeNew;
		}

		JSONIFIER_INLINE constexpr size_type max_size() const noexcept {
			return sizeNew;
		}

		JSONIFIER_INLINE constexpr bool empty() const noexcept {
			return false;
		}

		JSONIFIER_INLINE constexpr reference at(size_type position) {
			if (sizeNew <= position) {
				std::runtime_error{ "invalid array<T, N> subscript" };
			}

			return dataVal[position];
		}

		JSONIFIER_INLINE constexpr const_reference at(size_type position) const {
			if (sizeNew <= position) {
				std::runtime_error{ "invalid array<T, N> subscript" };
			}

			return dataVal[position];
		}

		JSONIFIER_INLINE constexpr reference operator[](size_type position) noexcept {
			return dataVal[position];
		}

		JSONIFIER_INLINE constexpr const_reference operator[](size_type position) const noexcept {
			return dataVal[position];
		}

		JSONIFIER_INLINE constexpr reference front() noexcept {
			return dataVal[0];
		}

		JSONIFIER_INLINE constexpr const_reference front() const noexcept {
			return dataVal[0];
		}

		JSONIFIER_INLINE constexpr reference back() noexcept {
			return dataVal[sizeNew - 1];
		}

		JSONIFIER_INLINE constexpr const_reference back() const noexcept {
			return dataVal[sizeNew - 1];
		}

		JSONIFIER_INLINE constexpr value_type* data() noexcept {
			return dataVal;
		}

		JSONIFIER_INLINE constexpr const value_type* data() const noexcept {
			return dataVal;
		}

		value_type dataVal[sizeNew];
	};

	struct empty_array_element {};

	template<class value_type_new> class array<value_type_new, 0> {
	  public:
		static_assert(std::is_object_v<value_type_new>, "The C++ Standard forbids containers of non-object types because of [container.requirements].");

		using value_type			 = value_type_new;
		using size_type				 = size_t;
		using difference_type		 = ptrdiff_t;
		using pointer				 = value_type*;
		using const_pointer			 = const value_type*;
		using reference				 = value_type&;
		using const_reference		 = const value_type&;
		using iterator				 = array_iterator<value_type, 0>;
		using const_iterator		 = const array_iterator<value_type, 0>;
		using reverse_iterator		 = std::reverse_iterator<iterator>;
		using const_reverse_iterator = std::reverse_iterator<const_iterator>;

		JSONIFIER_INLINE constexpr void fill(const value_type&) {
		}

		JSONIFIER_INLINE constexpr void swap(array&) noexcept {
		}

		JSONIFIER_INLINE constexpr iterator begin() noexcept {
			return iterator{};
		}

		JSONIFIER_INLINE constexpr const_iterator begin() const noexcept {
			return const_iterator{};
		}

		JSONIFIER_INLINE constexpr iterator end() noexcept {
			return iterator{};
		}

		JSONIFIER_INLINE constexpr const_iterator end() const noexcept {
			return const_iterator{};
		}

		JSONIFIER_INLINE constexpr reverse_iterator rbegin() noexcept {
			return reverse_iterator(end());
		}

		JSONIFIER_INLINE constexpr const_reverse_iterator rbegin() const noexcept {
			return const_reverse_iterator(end());
		}

		JSONIFIER_INLINE constexpr reverse_iterator rend() noexcept {
			return reverse_iterator(begin());
		}

		JSONIFIER_INLINE constexpr const_reverse_iterator rend() const noexcept {
			return const_reverse_iterator(begin());
		}

		JSONIFIER_INLINE constexpr const_iterator cbegin() const noexcept {
			return begin();
		}

		JSONIFIER_INLINE constexpr const_iterator cend() const noexcept {
			return end();
		}

		JSONIFIER_INLINE constexpr const_reverse_iterator crbegin() const noexcept {
			return rbegin();
		}

		JSONIFIER_INLINE constexpr const_reverse_iterator crend() const noexcept {
			return rend();
		}

		JSONIFIER_INLINE constexpr size_type size() const noexcept {
			return 0;
		}

		JSONIFIER_INLINE constexpr size_type max_size() const noexcept {
			return 0;
		}

		JSONIFIER_INLINE constexpr bool empty() const noexcept {
			return true;
		}

		JSONIFIER_INLINE constexpr reference at(size_type) {
			std::runtime_error{ "invalid array<T, N> subscript" };
		}

		JSONIFIER_INLINE constexpr const_reference at(size_type) const {
			std::runtime_error{ "invalid array<T, N> subscript" };
		}

		JSONIFIER_INLINE constexpr reference operator[](size_type) noexcept {
			return *data();
		}

		JSONIFIER_INLINE constexpr const_reference operator[](size_type) const noexcept {
			return *data();
		}

		JSONIFIER_INLINE constexpr reference front() noexcept {
			return *data();
		}

		JSONIFIER_INLINE constexpr const_reference front() const noexcept {
			return *data();
		}

		JSONIFIER_INLINE constexpr reference back() noexcept {
			return *data();
		}

		JSONIFIER_INLINE constexpr const_reference back() const noexcept {
			return *data();
		}

		JSONIFIER_INLINE constexpr value_type* data() noexcept {
			return nullptr;
		}

		JSONIFIER_INLINE constexpr const value_type* data() const noexcept {
			return nullptr;
		}

		conditional_t<std::disjunction_v<std::is_default_constructible<value_type>, std::is_default_constructible<value_type>>, value_type, empty_array_element> dataVal[1]{};
	};

}