/*
	Jsonifier - For parsing and serializing Json - very rapidly.
	Copyright (C) 2023 Chris M. (RealTimeChris)

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	This library is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
	Lesser General Public License for more details.

	You should have received a copy of the GNU Lesser General Public
	License along with this library; if not, write to the Free Software
	Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301
	USA
*/
/// https://github.com/RealTimeChris/Jsonifier
/// Feb 3, 2023
#pragma once

#include <jsonifier/Simd.hpp>
#include <iterator>

namespace Jsonifier {

	class Parser;

	class SimdIteratorCore {
	  public:
		friend class Parser;
		using iterator_category = std::forward_iterator_tag;
		using difference_type = std::ptrdiff_t;
		using value_type = StringViewPtr;
		using reference = StringViewPtr&;
		using pointer = StringViewPtr*;

		inline SimdIteratorCore() noexcept = default;

		inline SimdIteratorCore(SimdStringReader* stringReaderNew) noexcept {
			stringLength = stringReaderNew->getStringLength();
			currentIndex = stringReaderNew->getStructurals();
			stringView = stringReaderNew->getStringView();
		}

		inline StringViewPtr operator*() const noexcept {
			return reinterpret_cast<StringViewPtr>(&stringView[*currentIndex]);
		}

		inline SimdIteratorCore operator++(int32_t) noexcept {
			this->operator++();
			return *this;
		}

		inline SimdIteratorCore& operator++() {
			++currentIndex;
			return *this;
		}

		inline uint32_t getCurrentIndex() {
			return *this->currentIndex;
		}

		inline SimdIteratorCore operator+=(int32_t indexToAdd) noexcept {
			SimdIteratorCore newIter{ *this };
			for (size_t x = 0; x < indexToAdd; ++x) {
				newIter.operator++();
			}
			return newIter;
		}

		inline size_t operator-(SimdIteratorCore indexToAdd) noexcept {
			return static_cast<size_t>(this->currentIndex - indexToAdd.currentIndex);
		}

		inline SimdIteratorCore operator+(int32_t indexToAdd) noexcept {
			for (size_t x = 0; x < indexToAdd; ++x) {
				this->operator++();
			}
			return *this;
		}

		inline friend bool operator<(const SimdIteratorCore& lhs, const SimdIteratorCore& rhs) noexcept {
			return lhs.currentIndex < rhs.currentIndex;
		}

		inline bool operator==(const SimdIteratorCore& other) const noexcept {
			return currentDepth == 0 || (*currentIndex) >= stringLength;
		}

	  protected:
		StructuralIndex currentIndex{};
		StringViewPtr stringView{};
		size_t currentDepth{ 1 };
		size_t stringLength{};
	};
}
