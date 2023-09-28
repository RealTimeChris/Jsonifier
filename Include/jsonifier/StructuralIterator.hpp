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
		using value_type		= string_view_ptr;
		using pointer			= value_type*;
		using size_type			= int64_t;

		inline structural_iterator(pointer rootIndexNew, jsonifier::string_view_base<uint8_t> stringViewNew) {
			stringLength = stringViewNew.size();
			stringView	 = stringViewNew.data();
			currentIndex = rootIndexNew;
			rootIndex	 = rootIndexNew;
		}

		inline value_type operator*() const {
			return *currentIndex;
		}

		inline structural_iterator& operator++() {
			++currentIndex;
			return *this;
		}

		inline size_type getCurrentIndex() const {
			return *currentIndex - *rootIndex;
		}

		inline bool operator==(const structural_iterator&) const {
			return checkForNullIndex() || checkForstringOverRun();
		}

		inline bool operator==(uint8_t other) const {
			if (checkForNullIndex() || checkForstringOverRun()) {
				return false;
			}
			return ***this == other;
		}

	  protected:
		size_type stringLength{};
		value_type stringView{};
		pointer currentIndex{};
		pointer rootIndex{};

		inline bool checkForNullIndex() const {
			return !currentIndex;
		}

		inline bool checkForstringOverRun() const {
			auto currentIndexTemp = getCurrentIndex();
			return currentIndexTemp < 0 || currentIndexTemp >= stringLength;
		}
	};
}
