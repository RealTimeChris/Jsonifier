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

namespace JsonifierInternal {

	class StructuralIterator {
	  public:
		using iterator_category = std::forward_iterator_tag;
		using value_type = StringViewPtr;

		inline StructuralIterator() noexcept = default;

		inline StructuralIterator(SimdStringReader* stringReaderNew) noexcept {
			stringLength = stringReaderNew->getStringLength();
			currentIndex = stringReaderNew->getStructurals();
			rootIndex = stringReaderNew->getStructurals();
			stringView = stringReaderNew->getStringView();
		}

		inline value_type operator*() const noexcept {
			return stringView + *currentIndex;
		}

		inline StructuralIterator& operator++() {
			++currentIndex;
			return *this;
		}

		inline uint32_t getCurrentIndex() const noexcept {
			return *currentIndex;
		}

		inline bool operator==(const StructuralIterator& other) const noexcept {
			return checkForNullIndex() || checkForStringOverRun();
		}

		inline bool operator==(const uint8_t& other) const noexcept {
			return ***this == other;
		}

	  protected:
		StructuralIndex currentIndex{};
		StructuralIndex rootIndex{};
		StringViewPtr stringView{};
		size_t stringLength{};

		inline bool checkForNullIndex() const noexcept {
			return !currentIndex;
		}

		inline bool checkForStringOverRun() const noexcept {
			return *currentIndex >= stringLength;
		}
	};
}