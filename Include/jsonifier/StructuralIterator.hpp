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

#include <jsonifier/Simd.hpp>
#include <iterator>

namespace JsonifierInternal {

	class StructuralIterator {
	  public:
		using iterator_category = std::forward_iterator_tag;
		using value_type		= StringViewPtr;

		inline StructuralIterator() noexcept = default;

		inline StructuralIterator(SimdStringReader* stringReaderNew) noexcept {
			stringLength = stringReaderNew->getStringLength();
			currentIndex = stringReaderNew->getStructurals();
			rootIndex	 = currentIndex;
		}

		inline value_type operator*() const noexcept {
			return *currentIndex;
		}

		inline StructuralIterator& operator++() noexcept {
			++currentIndex;
			return *this;
		}

		inline int64_t getCurrentIndex() const noexcept {
			return *currentIndex - *rootIndex;
		}

		inline bool operator==(const StructuralIterator&) const noexcept {
			return checkForNullIndex() || checkForStringOverRun();
		}

		inline bool operator==(uint8_t other) const noexcept {
			return ***this == other;
		}

	  protected:
		StructuralIndex* currentIndex{};
		StructuralIndex* rootIndex{};
		int64_t stringLength{};

		inline bool checkForNullIndex() const noexcept {
			return !currentIndex;
		}

		inline bool checkForStringOverRun() const noexcept {
			auto currentIndexTemp = getCurrentIndex();
			return currentIndexTemp <= 0 || currentIndexTemp >= stringLength;
		}
	};
}
