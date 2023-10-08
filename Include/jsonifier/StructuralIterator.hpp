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

#include <jsonifier/Simd.hpp>
#include <iterator>

namespace jsonifier_internal {

	class structural_iterator {
	  public:
		using iterator_category = std::forward_iterator_tag;
		using value_type		= uint8_t;
		using difference_type	= std::ptrdiff_t;
		using pointer			= string_view_ptr*;
		using reference			= value_type&;
		using size_type			= int64_t;

		inline structural_iterator() noexcept = default;

		inline structural_iterator(structural_index* rootIndexNew, size_type originalLength) {
			stringLength = originalLength;
			currentIndex = rootIndexNew;
			rootIndex	 = rootIndexNew;
		}

		inline value_type operator*() const {
			return **currentIndex;
		}

		inline string_view_ptr operator->() {
			return *currentIndex;
		}

		inline structural_iterator& operator++() {
			++currentIndex;
			return *this;
		}

		inline structural_iterator operator++(int32_t) {
			structural_iterator oldIter{ *this };
			++(*this);
			return oldIter;
		}

		inline size_type getCurrentIndex() const {
			return (*currentIndex) - (*rootIndex);
		}

		inline size_type getRemainingLength() const {
			return stringLength - getCurrentIndex();
		}

		inline bool operator==(const structural_iterator&) const {
			return checkForstringOverRun();
		}

		inline bool operator==(uint8_t other) const {
			if (checkForstringOverRun()) {
				return false;
			}
			return **currentIndex == other;
		}

	  protected:
		size_type stringLength{};
		pointer currentIndex{};
		pointer rootIndex{};

		inline bool checkForstringOverRun() const {
			auto currentIndexTemp = getCurrentIndex();
			return currentIndexTemp < 0 || currentIndexTemp >= stringLength;
		}
	};
}
