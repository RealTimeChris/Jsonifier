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

	template<typename value_type_new, uint64_t N> class ctime_vector {
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

		jsonifier_constexpr ctime_vector() = default;

		jsonifier_constexpr ctime_vector(size_type count, const auto& value) : sizeVal(count) {
			for (size_type x = 0; x < N; ++x)
				data[x] = value;
		}

		jsonifier_constexpr iterator begin() {
			return data;
		}

		jsonifier_constexpr iterator end() {
			return data + sizeVal;
		}

		jsonifier_constexpr size_type size() const {
			return sizeVal;
		}

		jsonifier_constexpr reference operator[](size_type index) {
			return data[index];
		}

		jsonifier_constexpr const_reference operator[](size_type index) const {
			return data[index];
		}

		jsonifier_constexpr reference front() {
			return data[0];
		}

		jsonifier_constexpr reference back() {
			return data[sizeVal - 1];
		}

		jsonifier_constexpr const_reference front() const {
			return data[0];
		}

		jsonifier_constexpr const_reference back() const {
			return data[sizeVal - 1];
		}

		template<typename value_type_newer> jsonifier_constexpr void push_back(value_type_newer&& a) {
			data[sizeVal++] = std::forward<value_type>(a);
		}

		jsonifier_constexpr void pop_back() {
			--sizeVal;
		}

		jsonifier_constexpr void clear() {
			sizeVal = 0;
		}

	  protected:
		value_type data[N]{};
		size_type sizeVal = 0;
	};

}
