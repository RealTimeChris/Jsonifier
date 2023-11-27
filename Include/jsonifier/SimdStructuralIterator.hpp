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

	template<typename derived_type, typename second_derived_type, bool doWeUseInitialBuffer> class simd_structural_iterator {
	  public:
		friend class derailleur;

		using iterator_concept	= std::forward_iterator_tag;
		using iterator_category = std::forward_iterator_tag;
		using value_type		= uint8_t;
		using pointer_internal	= string_view_ptr*;
		using pointer			= string_view_ptr;
		using difference_type	= std::ptrdiff_t;
		using size_type			= uint64_t;

		JSONIFIER_INLINE simd_structural_iterator() noexcept {};

		JSONIFIER_INLINE simd_structural_iterator(structural_index* sectionNew, jsonifier::string_base<uint8_t, doWeUseInitialBuffer>& bufferStringNew,
			jsonifier::vector<error>& errorsNew) noexcept
			: stringBuffer{ &bufferStringNew }, errors{ &errorsNew }, rootIndex{ sectionNew } {};

		JSONIFIER_INLINE simd_structural_iterator& operator=(const simd_structural_iterator& other) noexcept {
			if (this != &other) {
				simd_structural_iterator{ other }.swap(*this);
			}
			return *this;
		};

		JSONIFIER_INLINE simd_structural_iterator(const simd_structural_iterator& other) noexcept
			: stringBuffer{ other.stringBuffer }, errors{ other.errors }, rootIndex{ other.rootIndex }, currentIndex{ other.currentIndex } {};

		JSONIFIER_INLINE value_type operator*() const {
			return (rootIndex[currentIndex]) ? *rootIndex[currentIndex] : defaultValue;
		}

		JSONIFIER_INLINE pointer operator->() const {
			return rootIndex[currentIndex];
		}

		JSONIFIER_INLINE simd_structural_iterator<derived_type, second_derived_type, doWeUseInitialBuffer>& operator++() {
			++currentIndex;
			return *this;
		}

		template<json_structural_type structuralType> JSONIFIER_INLINE void createError(std::source_location location = std::source_location::current()) {
			errors->emplace_back(jsonifier_internal::createError<structuralType>(*this, location));
		}

		JSONIFIER_INLINE simd_structural_iterator<derived_type, second_derived_type, doWeUseInitialBuffer> operator++(int32_t) {
			simd_structural_iterator<derived_type, second_derived_type, doWeUseInitialBuffer> temp{ *this };
			++(*this);
			return temp;
		}

		JSONIFIER_INLINE pointer getEndPtr() const {
			auto newIndex = rootIndex + currentIndex;
			while (*(newIndex + 1) != nullptr) {
				++newIndex;
			}
			return *newIndex;
		}

		JSONIFIER_INLINE size_type size() const {
			return stringBuffer->size();
		}

		JSONIFIER_INLINE void resize(size_type newSize) const {
			return stringBuffer->resize(newSize);
		}

		JSONIFIER_INLINE uint8_t* data() {
			return stringBuffer->data();
		}

		JSONIFIER_INLINE pointer getRootPtr() const {
			return *rootIndex;
		}

		JSONIFIER_INLINE jsonifier::string_base<uint8_t, doWeUseInitialBuffer>& getCurrentString() {
			return *stringBuffer;
		}

		JSONIFIER_INLINE size_type getCurrentStringIndex() const {
			return currentIndex;
		}

		JSONIFIER_INLINE bool operator==(const simd_structural_iterator&) const {
			return !(rootIndex[currentIndex]);
		}

		JSONIFIER_INLINE operator bool() const {
			return *(rootIndex + currentIndex) != nullptr;
		}

		JSONIFIER_INLINE void swap(simd_structural_iterator& other) {
			std::swap(stringBuffer, other.stringBuffer);
			std::swap(currentIndex, other.currentIndex);
			std::swap(rootIndex, other.rootIndex);
			std::swap(errors, other.errors);
		}

	  protected:
		jsonifier::string_base<uint8_t, doWeUseInitialBuffer>* stringBuffer{};
		static constexpr value_type defaultValue{ 0x00ll };
		jsonifier::vector<error>* errors{};
		pointer_internal rootIndex{};
		uint64_t currentIndex{};

		second_derived_type& initializeSelfRef() {
			return *static_cast<second_derived_type*>(this);
		}
	};
}