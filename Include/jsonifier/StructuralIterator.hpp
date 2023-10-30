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

#include <jsonifier/Base02.hpp>

namespace jsonifier_internal {

	class structural_iterator {
	  public:
		friend class derailleur;

		using iterator_category = std::forward_iterator_tag;
		using value_type		= uint8_t;
		using difference_type	= std::ptrdiff_t;
		using pointer			= string_view_ptr*;
		using reference			= value_type&;
		using size_type			= int64_t;

		jsonifier_inline structural_iterator() noexcept = default;

		jsonifier_inline structural_iterator(structural_index* rootIndexNew) {
			currentIndex = rootIndexNew;
			rootIndex	 = rootIndexNew;
		}

		jsonifier_inline value_type operator*() const {
			return (*currentIndex) ? **currentIndex : defaultValue;
		}

		jsonifier_inline string_view_ptr operator->() const {
			return *currentIndex;
		}

		jsonifier_inline structural_iterator& operator++() {
			++currentIndex;
			return *this;
		}

		jsonifier_inline structural_iterator operator++(int32_t) {
			structural_iterator oldIter{ *this };
			++(*this);
			return oldIter;
		}

		jsonifier_inline string_view_ptr getEndPtr() {
			auto newIndex = currentIndex;
			while (*(newIndex + 1) != nullptr) {
				++newIndex;
			}
			return *newIndex;
		}

		jsonifier_inline string_view_ptr getRootPtr() {
			return *rootIndex;
		}

		jsonifier_inline size_type getCurrentStringIndex() const {
			return (*currentIndex) - (*rootIndex);
		}

		jsonifier_inline bool operator==(const structural_iterator&) const {
			return !(*currentIndex);
		}

	  protected:
		static jsonifier_constexpr uint8_t defaultValue{ 0x00 };
		pointer currentIndex{};
		pointer rootIndex{};
	};
}