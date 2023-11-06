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

#include <jsonifier/TypeEntities.hpp>
#include <iterator>

namespace jsonifier_internal {

	template<jsonifier::concepts::buffer_like string_type> struct serialization_iterator {
		friend class derailleur;

		using iterator_concept	= std::forward_iterator_tag;
		using iterator_category = std::forward_iterator_tag;
		using value_type		= uint64_t;
		using pointer			= uint8_t*;
		using difference_type	= std::ptrdiff_t;
		using size_type			= uint64_t;

		size_type currentSize{};

		inline serialization_iterator() noexcept = default;

		inline serialization_iterator(string_type& stringNew) : currentString{ &stringNew } {};

		inline value_type operator*() const {
			return currentString->size();
		}

		inline pointer operator->() const {
			return &(*currentString)[currentSize];
		}

		inline void potentiallyResize(size_type sizeNew) {
			if (currentSize + sizeNew >= currentString->size()) {
				currentString->resize((currentSize + sizeNew) * 2);
			}
		}

		template<typename char_type> inline void writeData(char_type* chars, size_type sizeNew) {
			std::memcpy(currentString->data() + currentSize, chars, sizeNew);
			currentSize += sizeNew;
		}

		inline void resize(size_type sizeNew) {
			currentString->resize(sizeNew);
		}

		inline pointer data() {
			return currentString->data();
		}

		inline void reset() {
			currentSize = 0;
		}

		template<typename char_type> inline serialization_iterator& operator+=(char_type newValue) {
			(*currentString)[currentSize++] = static_cast<typename string_type::value_type>(newValue);
			return *this;
		}

		inline serialization_iterator& operator++() {
			++currentSize;
			return *this;
		}

		inline serialization_iterator operator++(int32_t) {
			serialization_iterator temp{ *this };
			++(*this);
			return temp;
		}

		inline uint8_t& operator[](uint64_t index) {
			return (*currentString)[index];
		}

		inline bool operator==(const serialization_iterator& other) const {
			return currentSize == other.currentSize;
		}

		inline size_type size() {
			return currentString->size();
		}

	  protected:
		string_type* currentString{};
	};
}