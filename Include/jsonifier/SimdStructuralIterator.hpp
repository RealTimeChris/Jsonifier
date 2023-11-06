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

	template<typename derived_type, typename second_derived_type> class simd_structural_iterator {
	  public:
		friend class derailleur;

		using iterator_concept	= std::forward_iterator_tag;
		using iterator_category = std::forward_iterator_tag;
		using value_type		= uint8_t;
		using pointer_internal	= string_view_ptr*;
		using pointer			= string_view_ptr;
		using difference_type	= std::ptrdiff_t;
		using size_type			= uint64_t;

		JSONIFIER_INLINE simd_structural_iterator() noexcept : selfRef{ initializeSelfRef() } {};

		JSONIFIER_INLINE simd_structural_iterator& operator=(const simd_structural_iterator& other) noexcept {
			if (this != &other) {
				simd_structural_iterator{ other }.swap(*this);
			}
			return *this;
		};

		JSONIFIER_INLINE simd_structural_iterator(const simd_structural_iterator& other) noexcept
			: selfRef{ initializeSelfRef() }, currentIndex(other.currentIndex), rootIndex(other.rootIndex) {
			selfRef = *static_cast<second_derived_type*>(this);
		}

		JSONIFIER_INLINE value_type operator*() const {
			return (*currentIndex) ? **currentIndex : defaultValue;
		}

		JSONIFIER_INLINE pointer operator->() const {
			return *currentIndex;
		}

		JSONIFIER_INLINE simd_structural_iterator& operator++() {
			++currentIndex;
			return *this;
		}

		template<json_structural_type structuralType> JSONIFIER_INLINE void createError(std::source_location location = std::source_location::current()) {
			selfRef.errors.emplace_back(jsonifier_internal::createError<structuralType>(*this, location));
		}

		JSONIFIER_INLINE simd_structural_iterator operator++(int32_t) {
			simd_structural_iterator temp{ *this };
			++(*this);
			return temp;
		}

		JSONIFIER_INLINE pointer getEndPtr() const {
			auto newIndex = currentIndex;
			while (*(newIndex + 1) != nullptr) {
				++newIndex;
			}
			return *newIndex;
		}

		JSONIFIER_INLINE void reset(structural_index* rootIndexNew) {
			currentIndex = rootIndexNew;
			rootIndex	 = rootIndexNew;
		}

		JSONIFIER_INLINE pointer getRootPtr() const {
			return *rootIndex;
		}

		JSONIFIER_INLINE typename jsonifier::string_base<uint8_t>::pointer data() const {
			return selfRef.stringBuffer.data();
		}

		JSONIFIER_INLINE void resize(size_type newSize) {
			selfRef.stringBuffer.resize(newSize);
		}

		JSONIFIER_INLINE size_type size() const {
			return selfRef.stringBuffer.size();
		}

		JSONIFIER_INLINE buffer_string<uint8_t>& getCurrentString() {
			return selfRef.stringBuffer;
		}

		JSONIFIER_INLINE size_type getCurrentStringIndex() const {
			return static_cast<size_type>((*currentIndex) - (*rootIndex));
		}

		JSONIFIER_INLINE bool operator==(const simd_structural_iterator&) const {
			return !(*currentIndex);
		}

		JSONIFIER_INLINE void swap(simd_structural_iterator& other) {
			std::swap(currentIndex, other.currentIndex);
			std::swap(rootIndex, other.rootIndex);
		}

	  protected:
		second_derived_type& selfRef{ *static_cast<second_derived_type*>(this) };
		static constexpr value_type defaultValue{ 0x00ll };
		pointer_internal currentIndex{};
		pointer_internal rootIndex{};

		second_derived_type& initializeSelfRef() {
			return *static_cast<second_derived_type*>(this);
		}
	};
}