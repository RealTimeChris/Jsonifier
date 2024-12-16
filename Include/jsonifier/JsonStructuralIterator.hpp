/*
	MIT License

	Copyright (c) 2024 RealTimeChris

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

namespace jsonifier_internal {

	struct json_structural_iterator {
		using iterator_category = std::forward_iterator_tag;
		using difference_type   = std::ptrdiff_t;
		using value_type        = char;
		using pointer           = const char*;
		using reference         = char&;

		JSONIFIER_ALWAYS_INLINE json_structural_iterator() noexcept = default;

		JSONIFIER_ALWAYS_INLINE json_structural_iterator(structural_index* iterNew) noexcept : iter{ iterNew }, root{ iterNew } {};

		JSONIFIER_ALWAYS_INLINE json_structural_iterator& operator++() noexcept {
			++iter;
			return *this;
		}

		JSONIFIER_ALWAYS_INLINE json_structural_iterator operator++(int) noexcept {
			json_structural_iterator tmp = *this;
			++(*this);
			return tmp;
		}

		JSONIFIER_ALWAYS_INLINE json_structural_iterator& operator--() noexcept {
			--iter;
			return *this;
		}

		JSONIFIER_ALWAYS_INLINE json_structural_iterator operator--(int) noexcept {
			json_structural_iterator tmp = *this;
			--(*this);
			return tmp;
		}

		JSONIFIER_ALWAYS_INLINE json_structural_iterator operator+(size_t value) noexcept {
			json_structural_iterator tmp = *this;
			for (size_t x = 0; x < value; ++x) {
				++tmp;
			}
			return tmp;
		}

		JSONIFIER_ALWAYS_INLINE string_view_ptr getPointerAt(size_t value) const noexcept {
			return static_cast<string_view_ptr>(*this) + value;
		}

		JSONIFIER_ALWAYS_INLINE value_type operator[](size_t index) noexcept {
			return *getPointerAt(index);
		}

		JSONIFIER_ALWAYS_INLINE value_type operator*() noexcept {
			return **iter;
		}

		JSONIFIER_ALWAYS_INLINE pointer operator->() const noexcept {
			return *iter;
		}

		JSONIFIER_ALWAYS_INLINE operator string_view_ptr() const noexcept {
			return *iter;
		}

		JSONIFIER_ALWAYS_INLINE int64_t operator-(const json_structural_iterator& other) const noexcept {
			return *iter - *other.iter;
		}

		JSONIFIER_ALWAYS_INLINE bool operator==(const json_structural_iterator& other) const noexcept {
			return iter == other.iter;
		}

		JSONIFIER_ALWAYS_INLINE bool operator<(const json_structural_iterator& other) const noexcept {
			return iter < other.iter;
		}

		JSONIFIER_ALWAYS_INLINE friend bool operator<(string_view_ptr lhs, const json_structural_iterator& other) noexcept {
			return lhs < *other.iter;
		}

		JSONIFIER_ALWAYS_INLINE bool operator!() const noexcept {
			return *iter == nullptr;
		}

	  protected:
		structural_index* iter{};
		structural_index* root{};
	};

};