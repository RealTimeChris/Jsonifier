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

#include <jsonifier/ISADetection.hpp>
#include <jsonifier/Error.hpp>

namespace jsonifier_internal {

	template<typename value_type_new> class simd_structural_iterator {
	  public:
		friend class derailleur;
		template<json_structural_type value_type, typename derived_type> friend struct validate_impl;

		using iterator_concept	  = std::forward_iterator_tag;
		using iterator_category	  = std::forward_iterator_tag;
		using value_type		  = value_type_new;
		using value_type_internal = uint8_t;
		using pointer_internal	  = structural_index*;
		using pointer			  = value_type*;
		using string_type		  = jsonifier::string_base<value_type>;
		using size_type			  = uint64_t;

		JSONIFIER_INLINE simd_structural_iterator() noexcept = default;

		JSONIFIER_INLINE simd_structural_iterator(structural_index* startPtr, structural_index* endPtr, size_type stringLengthNew, string_type& bufferStringNew,
			jsonifier::vector<error>& errorsNew) noexcept
			: errors{ &errorsNew }, currentIndex{ startPtr }, rootIndex{ startPtr }, stringBuffer{ &bufferStringNew }, endIndex{ endPtr }, stringLength{ stringLengthNew } {};

		JSONIFIER_INLINE value_type operator*() const {
			return *currentIndex ? **currentIndex : defaultValue;
		}

		JSONIFIER_INLINE auto operator->() const {
			return *currentIndex;
		}

		JSONIFIER_INLINE simd_structural_iterator<value_type>& operator++() {
			++currentIndex;
			return *this;
		}

		template<error_code errorNew> JSONIFIER_INLINE void createError(std::source_location location = std::source_location::current()) {
			errors->emplace_back(jsonifier_internal::createError<errorNew>(*this, location));
		}

		template<json_structural_type typeNew> JSONIFIER_INLINE void createError(std::source_location location = std::source_location::current()) {
			errors->emplace_back(jsonifier_internal::createError<typeNew>(*this, location));
		}

		JSONIFIER_INLINE simd_structural_iterator<value_type> operator+(int32_t valueNew) {
			simd_structural_iterator<value_type> temp{ *this };
			for (uint64_t x = 0; x < valueNew; ++x) {
				++temp;
			}
			return temp;
		}

		JSONIFIER_INLINE simd_structural_iterator<value_type> operator++(int32_t) {
			simd_structural_iterator<value_type> temp{ *this };
			++(*this);
			return temp;
		}

		JSONIFIER_INLINE auto getEndPtr() const {
			return *endIndex;
		}

		JSONIFIER_INLINE auto getRootPtr() const {
			return *rootIndex;
		}

		JSONIFIER_INLINE string_type& getCurrentString() {
			return *stringBuffer;
		}

		JSONIFIER_INLINE jsonifier::vector<error>& getErrors() {
			return *errors;
		}

		JSONIFIER_INLINE size_type getRemainingStringLength() const {
			return stringLength - getCurrentStringIndex();
		}

		JSONIFIER_INLINE size_type getCurrentStringIndex() const {
			return *currentIndex - *rootIndex;
		}

		JSONIFIER_INLINE bool operator==(const simd_structural_iterator&) const {
			return *currentIndex == nullptr;
		}

		JSONIFIER_INLINE operator bool() const {
			return *currentIndex != nullptr;
		}

		JSONIFIER_INLINE void swap(simd_structural_iterator& other) {
			std::swap(stringBuffer, other.stringBuffer);
			std::swap(currentIndex, other.currentIndex);
			std::swap(stringLength, other.stringLength);
			std::swap(rootIndex, other.rootIndex);
			std::swap(endIndex, other.endIndex);
			std::swap(errors, other.errors);
		}

	  protected:
		static constexpr value_type defaultValue{ 0x00ll };
		jsonifier::vector<error>* errors{};
		pointer_internal currentIndex{};
		pointer_internal rootIndex{};
		string_type* stringBuffer{};
		pointer_internal endIndex{};
		size_type stringLength{};
	};

	template<typename value_type>
	concept simd_structural_iterator_t = jsonifier::concepts::is_specialization_v<jsonifier::concepts::unwrap_t<value_type>, simd_structural_iterator>;

}