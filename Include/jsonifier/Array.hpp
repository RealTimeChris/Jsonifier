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

#include <jsonifier/Config.hpp>
#include <jsonifier/Iterator.hpp>

namespace jsonifier_internal {

	template<typename value_type_new, size_t sizeNew> struct array {
		using value_type			 = value_type_new;
		using reference				 = value_type&;
		using const_reference		 = const value_type&;
		using size_type				 = size_t;
		using iterator				 = basic_iterator<value_type>;
		using const_iterator		 = basic_iterator<const value_type>;
		using reverse_iterator		 = std::reverse_iterator<iterator>;
		using const_reverse_iterator = std::reverse_iterator<const_iterator>;

		constexpr array() noexcept = default;

		constexpr array(std::initializer_list<value_type> initList) {
			for (size_t i = 0; i < initList.size() && i < sizeNew; ++i) {
				values[i] = initList.begin()[i];
			}
		}

		JSONIFIER_ALWAYS_INLINE constexpr iterator begin() noexcept {
			return iterator(values);
		}

		JSONIFIER_ALWAYS_INLINE constexpr const_iterator begin() const noexcept {
			return const_iterator(values);
		}

		JSONIFIER_ALWAYS_INLINE constexpr iterator end() noexcept {
			return iterator(values + sizeNew);
		}

		JSONIFIER_ALWAYS_INLINE constexpr const_iterator end() const noexcept {
			return const_iterator(values + sizeNew);
		}

		JSONIFIER_ALWAYS_INLINE constexpr reverse_iterator rbegin() noexcept {
			return reverse_iterator(end());
		}

		JSONIFIER_ALWAYS_INLINE constexpr const_reverse_iterator rbegin() const noexcept {
			return const_reverse_iterator(end());
		}

		JSONIFIER_ALWAYS_INLINE constexpr reverse_iterator rend() noexcept {
			return reverse_iterator(begin());
		}

		JSONIFIER_ALWAYS_INLINE constexpr const_reverse_iterator rend() const noexcept {
			return const_reverse_iterator(begin());
		}

		JSONIFIER_ALWAYS_INLINE constexpr const_reference operator[](size_type index) const noexcept {
			return values[index];
		}

		JSONIFIER_ALWAYS_INLINE constexpr reference operator[](size_type index) noexcept {
			return values[index];
		}

		JSONIFIER_ALWAYS_INLINE constexpr const value_type* data() const noexcept {
			return values;
		}

		JSONIFIER_ALWAYS_INLINE constexpr value_type* data() noexcept {
			return values;
		}

		constexpr void fill(const value_type& value) noexcept {
			for (size_t i = 0; i < sizeNew; ++i) {
				values[i] = value;
			}
		}

		JSONIFIER_ALWAYS_INLINE constexpr size_type size() const noexcept {
			return sizeNew;
		}

		value_type values[sizeNew]{};
	};

	template<typename value_type_new> struct array<value_type_new, 0> {
		using value_type			 = value_type_new;
		using reference				 = value_type&;
		using const_reference		 = const value_type&;
		using pointer				 = value_type*;
		using const_pointer			 = const value_type*;
		using const_iterator		 = const basic_iterator<value_type>;
		using iterator				 = basic_iterator<value_type>;
		using size_type				 = size_t;
		using reverse_iterator		 = std::reverse_iterator<iterator>;
		using const_reverse_iterator = std::reverse_iterator<const_iterator>;
		using difference_type		 = std::ptrdiff_t;

		constexpr array(std::initializer_list<value_type>) {
		}

		JSONIFIER_ALWAYS_INLINE constexpr const_pointer data() const noexcept {
			return nullptr;
		}

		JSONIFIER_ALWAYS_INLINE constexpr pointer data() noexcept {
			return nullptr;
		}

		JSONIFIER_ALWAYS_INLINE constexpr size_type size() const noexcept {
			return 0;
		}

		JSONIFIER_ALWAYS_INLINE constexpr const_reference operator[](size_type index) const noexcept {
			return dataVal[index];
		}

		JSONIFIER_ALWAYS_INLINE constexpr reference operator[](size_type index) noexcept {
			return dataVal[index];
		}

		JSONIFIER_ALWAYS_INLINE constexpr iterator begin() noexcept {
			return nullptr;
		}

		JSONIFIER_ALWAYS_INLINE constexpr iterator end() noexcept {
			return nullptr;
		}

		JSONIFIER_ALWAYS_INLINE constexpr const_iterator begin() const noexcept {
			return nullptr;
		}

		JSONIFIER_ALWAYS_INLINE constexpr const_iterator end() const noexcept {
			return nullptr;
		}

		constexpr void fill(const value_type&) noexcept {
		}

		value_type dataVal[0];
	};

}