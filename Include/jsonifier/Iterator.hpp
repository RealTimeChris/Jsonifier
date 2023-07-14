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
		using iterator_concept = std::bidirectional_iterator_tag;
		using value_type = ValueType;
		using difference_type = ptrdiff_t;
		using pointer = const value_type*;
		using reference = const value_type&;
		using size_type = size_t;

		inline constexpr Iterator() noexcept = default;

		inline constexpr Iterator(const pointer ptrNew, const size_type offsetNew) noexcept : dataVal(ptrNew), offset(offsetNew) {
		}

		inline constexpr reference operator*() const noexcept {
			return dataVal[offset];
		}

		inline constexpr pointer operator->() const noexcept {
			return dataVal + offset;
		}

		inline constexpr Iterator& operator++() noexcept {
			++offset;
			return *this;
		}

		inline constexpr Iterator operator++(int32_t) noexcept {
			Iterator temp{ *this };
			++*this;
			return temp;
		}

		inline constexpr Iterator& operator--() noexcept {
			--offset;
			return *this;
		}

		inline constexpr Iterator operator--(int32_t) noexcept {
			Iterator temp{ *this };
			--*this;
			return temp;
		}

		inline constexpr Iterator& operator+=(const difference_type offsetNew) noexcept {
			offset += static_cast<size_type>(offsetNew);
			return *this;
		}

		inline constexpr Iterator operator+(const difference_type offsetNew) const noexcept {
			Iterator temp{ *this };
			temp += offsetNew;
			return temp;
		}

		inline constexpr Iterator operator+(const difference_type offsetNew) noexcept {
			*this += offsetNew;
			return *this;
		}

		inline constexpr Iterator& operator-=(const difference_type offsetNew) noexcept {
			offset -= static_cast<size_type>(offsetNew);
			return *this;
		}

		inline constexpr Iterator operator-(const difference_type offsetNew) const noexcept {
			Iterator temp{ *this };
			temp -= offsetNew;
			return temp;
		}

		inline constexpr difference_type operator-(const Iterator& rhs) const noexcept {
			return static_cast<difference_type>(offset - rhs.offset);
		}

		inline constexpr reference operator[](const difference_type offsetNew) const noexcept {
			return *(*this + offsetNew);
		}

		inline constexpr bool operator==(const Iterator& rhs) const noexcept {
			return offset == rhs.offset;
		}

		inline constexpr bool operator!=(const Iterator& rhs) const noexcept {
			return !(*this == rhs);
		}

		inline constexpr bool operator<(const Iterator& rhs) const noexcept {
			return offset < rhs.offset;
		}

		inline constexpr bool operator>(const Iterator& rhs) const noexcept {
			return rhs < *this;
		}

		inline constexpr bool operator<=(const Iterator& rhs) const noexcept {
			return !(rhs < *this);
		}

		inline constexpr bool operator>=(const Iterator& rhs) const noexcept {
			return !(*this < rhs);
		}

	  protected:
		pointer dataVal = nullptr;
		size_type offset = 0;
	};

}
