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
#include <jsonifier/NumberUtils.hpp>
#include <jsonifier/StringUtils.hpp>
#include <jsonifier/Error.hpp>
#include <iterator>

namespace jsonifier_internal {

	template<typename derived_type, typename second_derived_type> class serialization_iterator {
	  public:
		friend class derailleur;

		using iterator_concept	= std::forward_iterator_tag;
		using iterator_category = std::forward_iterator_tag;
		using value_type		= uint64_t;
		using pointer			= uint8_t*;
		using difference_type	= std::ptrdiff_t;
		using size_type			= uint64_t;

		size_type currentSize{};

		JSONIFIER_INLINE serialization_iterator() noexcept : selfRef{ initializeSelfRef() } {};

		JSONIFIER_INLINE serialization_iterator& operator=(const serialization_iterator& other) noexcept {
			if (this != &other) {
				serialization_iterator{ other }.swap(*this);
			}
			return *this;
		};

		JSONIFIER_INLINE serialization_iterator(const serialization_iterator& other) noexcept : selfRef{ initializeSelfRef() }, currentSize{ other.currentSize } {};

		JSONIFIER_INLINE value_type operator*() const {
			return 0;
		}

		JSONIFIER_INLINE pointer operator->() const {
			return &selfRef.stringBuffer[currentSize];
		}

		JSONIFIER_INLINE void potentiallyResize(size_type sizeNew) {
			if (currentSize + sizeNew >= selfRef.stringBuffer.size()) {
				selfRef.stringBuffer.resize((currentSize + sizeNew) * 2);
			}
		}

		template<typename char_type, char_type c> JSONIFIER_INLINE void writeCharacter() {
			if (currentSize + 1 >= selfRef.stringBuffer.size()) {
				selfRef.stringBuffer.resize((currentSize + 1) * 2);
			}
			selfRef.stringBuffer[currentSize++] = static_cast<uint8_t>(c);
		}

		template<typename char_type> JSONIFIER_INLINE void writeCharacter(char_type c) {
			if (currentSize + 1 >= selfRef.stringBuffer.size()) {
				selfRef.stringBuffer.resize((currentSize + 1) * 2);
			}
			selfRef.stringBuffer[currentSize++] = static_cast<uint8_t>(c);
		}

		template<typename char_type> JSONIFIER_INLINE void writeCharacterUnchecked(char_type c) {
			selfRef.stringBuffer[currentSize++] = static_cast<uint8_t>(c);
		}

		template<const jsonifier::string_view& str> JSONIFIER_INLINE void writeCharacters() {
			constexpr auto n = str.size();
			if (currentSize + n >= selfRef.stringBuffer.size()) {
				selfRef.stringBuffer.resize((currentSize + n) * 2);
			}
			std::memcpy(selfRef.stringBuffer.data() + currentSize, str.data(), n);
			currentSize += n;
		}

		template<uint64_t size> JSONIFIER_INLINE void writeCharacters(const char (&str)[size]) {
			constexpr auto n = size - 1;
			if (currentSize + n >= selfRef.stringBuffer.size()) {
				selfRef.stringBuffer.resize((currentSize + n) * 2);
			}
			std::memcpy(selfRef.stringBuffer.data() + currentSize, str, n);
			currentSize += n;
		}

		template<typename char_type, char_type c> JSONIFIER_INLINE void writeCharacterUnchecked() {
			selfRef.stringBuffer[currentSize++] = static_cast<uint8_t>(c);
		}

		template<uint64_t size> JSONIFIER_INLINE void writeCharactersUnchecked(const char (&str)[size]) {
			std::memcpy(selfRef.stringBuffer.data() + currentSize, str, size - 1);
			currentSize += size - 1;
		}

		JSONIFIER_INLINE void resize(size_type sizeNew) {
			selfRef.stringBuffer.resize(sizeNew);
		}

		JSONIFIER_INLINE pointer data() const {
			return selfRef.stringBuffer.data();
		}

		JSONIFIER_INLINE void reset() {
			currentSize = 0;
		}

		JSONIFIER_INLINE serialization_iterator& operator++() {
			++currentSize;
			return *this;
		}

		JSONIFIER_INLINE serialization_iterator operator++(int32_t) {
			serialization_iterator temp{ *this };
			++(*this);
			return temp;
		}

		JSONIFIER_INLINE uint8_t& operator[](uint64_t index) const {
			return selfRef.stringBuffer[index];
		}

		JSONIFIER_INLINE bool operator==(const serialization_iterator& other) const {
			return currentSize == other.currentSize;
		}

		JSONIFIER_INLINE size_type size() const {
			return selfRef.stringBuffer.size();
		}

		JSONIFIER_INLINE void swap(serialization_iterator& other) {
			std::swap(currentSize, other.currentSize);
		}

	  protected:
		second_derived_type& selfRef{ initializeSelfRef() };

		second_derived_type& initializeSelfRef() {
			return *static_cast<second_derived_type*>(this);
		}
	};
}