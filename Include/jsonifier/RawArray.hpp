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
/// Feb 20, 2023
#pragma once

#include <iterator>

namespace jsonifier_internal {

	template<typename value_type_new, size_t N> struct raw_array {
	  public:
		using value_type	  = value_type_new;
		using reference		  = value_type&;
		using const_reference = const value_type&;
		using pointer		  = value_type*;
		using const_pointer	  = const value_type*;
		using iterator		  = pointer;
		using const_iterator  = const_pointer;
		using size_type		  = uint64_t;
		using difference_type = std::ptrdiff_t;

		constexpr raw_array() = default;

		template<size_t M> constexpr raw_array(value_type const (&init)[M]) : raw_array(init, std::make_index_sequence<N>()) {
			static_assert(M >= N);
		}

		constexpr raw_array(const std::initializer_list<value_type>& other) {
			for (uint64_t x = 0; x < other.size(); ++x) {
				operator[](x) = std::move(other.begin()[x]);
			}
		}

		constexpr iterator begin() {
			return dataVal;
		}

		constexpr const_iterator begin() const {
			return dataVal;
		}

		constexpr iterator end() {
			return dataVal + N;
		}

		constexpr const_iterator end() const {
			return dataVal + N;
		}

		constexpr size_type size() const {
			return N;
		}

		constexpr reference operator[](uint64_t index) {
			return dataVal[index];
		}

		constexpr const_reference operator[](uint64_t index) const {
			return dataVal[index];
		}

		constexpr reference at(uint64_t index) {
			if (index > N) {
				std::abort();
			}
			return dataVal[index];
		}

		constexpr const_reference at(uint64_t index) const {
			if (index > N) {
				std::abort();
			}
			return dataVal[index];
		}

		constexpr reference front() {
			return dataVal[0];
		}

		constexpr const_reference front() const {
			return dataVal[0];
		}

		constexpr reference back() {
			return dataVal[N - 1];
		}

		constexpr const_reference back() const {
			return dataVal[N - 1];
		}

		constexpr pointer data() {
			return dataVal;
		}

		constexpr const_pointer data() const {
			return dataVal;
		}

		constexpr void fill(const value_type& val) {
			for (uint64_t x = 0; x < N; ++x) {
				dataVal[x] = val;
			}
		}

		alignas(JsonifierAlignment) value_type dataVal[N]{};

		template<size_t M, size_t... I> constexpr raw_array(value_type const (&init)[M], std::index_sequence<I...>) : dataVal{ init[I]... } {
		}
	};

	template<typename value_type_new> class raw_array<value_type_new, 0> {
	  public:
		using value_type			 = value_type_new;
		using reference				 = value_type&;
		using const_reference		 = const value_type&;
		using pointer				 = value_type*;
		using const_pointer			 = const pointer;
		using iterator				 = value_type*;
		using const_iterator		 = const_pointer;
		using reverse_iterator		 = std::reverse_iterator<iterator>;
		using const_reverse_iterator = std::reverse_iterator<const_iterator>;
		using size_type				 = uint64_t;
		using difference_type		 = std::ptrdiff_t;
	};

}
