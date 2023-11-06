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

#include <jsonifier/Base.hpp>
#include <jsonifier/Error.hpp>

namespace jsonifier_internal {

	enum class current_parsing_state {
		unset	= 0,
		object	= 1,
		array	= 2,
		string	= 3,
		boolean = 4,
		number	= 5,
		null	= 6,
	};

	constexpr std::array<bool, 128> getObjectTable() {
		constexpr uint8_t objectValues[]{ '{', '}', ':', '"', ',' };
		std::array<bool, 128> returnValues{};
		for (uint64_t x = 0; x < 128; ++x) {
			for (uint64_t y = 0; y < std::size(objectValues); ++y) {
				if (x == objectValues[y]) {
					returnValues[x] = true;
				}
			}
		}
		return returnValues;
	}

	constexpr std::array<bool, 128> getArrayTable() {
		constexpr uint8_t objectValues[]{ '[', ']', ',' };
		std::array<bool, 128> returnValues{};
		for (uint64_t x = 0; x < 128; ++x) {
			for (uint64_t y = 0; y < std::size(objectValues); ++y) {
				if (x == objectValues[y]) {
					returnValues[x] = true;
				}
			}
		}
		return returnValues;
	}

	template<current_parsing_state currentStateNew = current_parsing_state::unset> class serial_structural_iterator {
	  public:
		friend class derailleur;

		using iterator_concept	= std::forward_iterator_tag;
		using iterator_category = std::forward_iterator_tag;
		using value_type		= uint8_t;
		using pointer			= string_view_ptr;
		using difference_type	= std::ptrdiff_t;
		using size_type			= int64_t;

		inline serial_structural_iterator() noexcept = default;

		inline serial_structural_iterator(pointer& rootIndexNew, pointer& endIndexNew) : currentIndex{ rootIndexNew }, rootIndex{ rootIndexNew }, endIndex{ endIndexNew } {};

		inline value_type operator*() const {
			return *currentIndex;
		}

		inline pointer operator->() const {
			return currentIndex;
		}

		inline serial_structural_iterator& operator++() {
			return *this;
		}

		inline serial_structural_iterator operator++(int32_t) {
			serial_structural_iterator temp{ *this };
			++(*this);
			return temp;
		}

		inline pointer getEndPtr() {
			return endIndex - 1;
		}

		inline pointer getRootPtr() {
			return rootIndex;
		}

		inline bool areWeEscaped(pointer currentIndexNew, uint64_t currentCount = 0) {
			if (*currentIndexNew == 0x5Cu && currentIndexNew > rootIndex) {
				return areWeEscaped(--currentIndexNew, ++currentCount);
			} else {
				return currentCount % 1 == 0;
			}
		}

		inline size_type getCurrentStringIndex() const {
			return (currentIndex) - (rootIndex);
		}

		inline bool operator==(const serial_structural_iterator&) const {
			return currentIndex == endIndex;
		}

	  protected:
		value_type previouslyFoundStructural{ '\0' };
		current_parsing_state currentState{};
		pointer& currentIndex;
		pointer& rootIndex;
		pointer& endIndex;
	};
}