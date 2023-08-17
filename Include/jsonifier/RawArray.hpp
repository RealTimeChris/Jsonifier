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

	template<typename ValueType, size_t Count> struct RawArray {
	  public:
		using value_type = ValueType;
		using reference = value_type&;
		using const_reference = const value_type&;
		using pointer = value_type*;
		using const_pointer = const value_type*;
		using iterator = pointer;
		using const_iterator = const_pointer;
		using size_type = uint64_t;
		using difference_type = std::ptrdiff_t;

		constexpr RawArray() noexcept = default;
		constexpr RawArray& operator=(RawArray&&) noexcept = default;
		constexpr RawArray(RawArray&&) noexcept = default;
		constexpr RawArray& operator=(const RawArray&) noexcept = default;
		constexpr RawArray(const RawArray&) noexcept = default;

		template<size_t M> constexpr RawArray(ValueType const (&init)[M]) : RawArray(init, std::make_index_sequence<Count>()) {
			static_assert(M >= Count);
		}

		constexpr RawArray(const std::initializer_list<ValueType>& other) {
			for (size_t x = 0; x < other.size(); ++x) {
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
			return dataVal + Count;
		}

		constexpr const_iterator end() const {
			return dataVal + Count;
		}

		constexpr size_type size() const {
			return Count;
		}

		constexpr reference operator[](size_t index) {
			return dataVal[index];
		}

		constexpr const_reference operator[](size_t index) const {
			return dataVal[index];
		}

		constexpr reference at(size_t index) {
			if (index > Count) {
				std::abort();
			}
			return dataVal[index];
		}

		constexpr const_reference at(size_t index) const {
			if (index > Count) {
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
			return dataVal[Count - 1];
		}

		constexpr const_reference back() const {
			return dataVal[Count - 1];
		}

		constexpr value_type* data() {
			return dataVal;
		}

		constexpr const value_type* data() const {
			return dataVal;
		}

		constexpr void fill(const value_type& val) {
			for (size_t x = 0; x < Count; ++x) {
				dataVal[x] = val;
			}
		}

		ValueType dataVal[Count] = {};

		template<size_t M, size_t... Index> constexpr RawArray(ValueType const (&init)[M], std::index_sequence<Index...>)
			: dataVal{ init[Index]... } {};
	};

	template<typename ValueType> class RawArray<ValueType, 0> {
	  public:
		using value_type = ValueType;
		using reference = value_type&;
		using const_reference = const value_type&;
		using pointer = value_type*;
		using const_pointer = const value_type*;
		using iterator = pointer;
		using const_iterator = const_pointer;
		using reverse_iterator = std::reverse_iterator<iterator>;
		using const_reverse_iterator = std::reverse_iterator<const_iterator>;
		using size_type = uint64_t;
		using difference_type = std::ptrdiff_t;

		constexpr RawArray() noexcept = default;
	};

}
