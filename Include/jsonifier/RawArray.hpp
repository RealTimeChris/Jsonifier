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
/// Feb 20, 2023
#pragma once

#include <iterator>

namespace JsonifierInternal {

	template<typename ValueType, size_t N> struct RawArray {
	  public:
		using value_type	  = ValueType;
		using reference		  = value_type&;
		using const_reference = const value_type&;
		using pointer		  = value_type*;
		using const_pointer	  = const value_type*;
		using iterator		  = pointer;
		using const_iterator  = const_pointer;
		using size_type		  = uint64_t;
		using difference_type = std::ptrdiff_t;

		constexpr RawArray() noexcept						   = default;
		constexpr RawArray& operator=(RawArray&&) noexcept	   = default;
		constexpr RawArray(RawArray&&) noexcept				   = default;
		constexpr RawArray& operator=(const RawArray&) noexcept = default;
		constexpr RawArray(const RawArray&) noexcept			   = default;

		template<size_t M> constexpr RawArray(ValueType const (&init)[M]) : RawArray(init, std::make_index_sequence<N>()) {
			static_assert(M >= N);
		}

		constexpr RawArray(const std::initializer_list<ValueType>& other) {
			for (uint64_t x = 0; x < other.size(); ++x) {
				operator[](x) = std::move(other.begin()[x]);
			}
		}

		constexpr iterator begin() noexcept {
			return dataVal;
		}

		constexpr const_iterator begin() const noexcept {
			return dataVal;
		}

		constexpr iterator end() noexcept {
			return dataVal + N;
		}

		constexpr const_iterator end() const noexcept {
			return dataVal + N;
		}

		constexpr size_type size() const noexcept {
			return N;
		}

		constexpr size_type maxSize() const noexcept {
			return N;
		}

		constexpr reference operator[](uint64_t index) {
			return dataVal[index];
		}

		constexpr const_reference operator[](uint64_t index) const noexcept {
			return dataVal[index];
		}

		constexpr reference at(uint64_t index) noexcept {
			if (index > N) {
				std::abort();
			}
			return dataVal[index];
		}

		constexpr const_reference at(uint64_t index) const noexcept {
			if (index > N) {
				std::abort();
			}
			return dataVal[index];
		}

		constexpr reference front() {
			return dataVal[0];
		}

		constexpr const_reference front() const noexcept {
			return dataVal[0];
		}

		constexpr reference back() {
			return dataVal[N - 1];
		}

		constexpr const_reference back() const noexcept {
			return dataVal[N - 1];
		}

		constexpr value_type* data() noexcept {
			return dataVal;
		}

		constexpr const value_type* data() const noexcept {
			return dataVal;
		}

		constexpr void fill(const value_type& val) {
			for (uint64_t x = 0; x < N; ++x) {
				dataVal[x] = val;
			}
		}

		ValueType dataVal[N]{};

		template<size_t M, size_t... I> constexpr RawArray(ValueType const (&init)[M], std::index_sequence<I...>) : dataVal{ init[I]... } {
		}
	};

	template<typename ValueType> class RawArray<ValueType, 0> {
	  public:
		using value_type			 = ValueType;
		using reference				 = value_type&;
		using const_reference		 = const value_type&;
		using pointer				 = value_type*;
		using const_pointer			 = const value_type*;
		using iterator				 = pointer;
		using const_iterator		 = const_pointer;
		using reverse_iterator		 = std::reverse_iterator<iterator>;
		using const_reverse_iterator = std::reverse_iterator<const_iterator>;
		using size_type				 = uint64_t;
		using difference_type		 = std::ptrdiff_t;

		constexpr RawArray() noexcept = default;
	};

}
