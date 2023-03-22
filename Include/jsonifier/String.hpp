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
	License along with this library; if not, Write to the Free Software
	Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301
	USA
*/
/// https://github.com/RealTimeChris/Jsonifier
/// Feb 20, 2023
#pragma once

#include <jsonifier/Base.hpp>

namespace Jsonifier {

	inline void memcpy(void* destVector, void* sourceVector, size_t lengthNew) noexcept;

	class String {
	  public:
		using value_type = char;
		using difference_type = std::ptrdiff_t;
		using pointer = value_type*;
		using reference = value_type&;

		inline String() noexcept : string(nullptr), sizeVal(0), m_capacity(0) {
		}

		inline String& operator=(String&& other) noexcept {
			if (this != &other) {
				std::swap(*this, other);
			}
			return *this;
		}

		inline String(String&& other) noexcept {
			*this = std::move(other);
		}

		inline String& operator=(const String& other) {
			reserve(other.m_capacity);
			JsonifierCore::memcpy(string, other.string, other.sizeVal);
			sizeVal = other.sizeVal;
			return *this;
		}

		inline String(const String& other) noexcept {
			*this = other;
		}

		inline ~String() {
			free(string);
		}

		inline char& operator[](size_t index) {
			return string[index];
		}

		inline char& operator[](size_t index) const {
			return string[index];
		}

		inline constexpr size_t size() const noexcept {
			return sizeVal;
		}

		inline void resize(size_t newSize) {
			if (newSize > m_capacity) {
				reserve(newSize);
			}
			sizeVal = newSize;
		}

		inline String& operator=(const std::string& other) {
			reserve(other.size());
			JsonifierCore::memcpy(string, other.data(), other.size());
			sizeVal = other.size();
			return *this;
		}

		inline String(const std::string& other) noexcept {
			*this = other;
		}

		inline String& operator=(const std::string_view& other) {
			reserve(other.size());
			JsonifierCore::memcpy(string, other.data(), other.size());
			sizeVal = other.size();
			return *this;
		}

		inline String(const std::string_view& other) noexcept {
			*this = other;
		}

		inline String& operator=(const StringView& other) {
			reserve(other.size());
			JsonifierCore::memcpy(string, other.data(), other.size());
			sizeVal = other.size();
			return *this;
		}

		inline String(const StringView& other) noexcept {
			*this = other;
		}

		inline String& operator=(void* stringNew) noexcept {
			sizeVal = findSingleCharacterAvx2(stringNew, std::string::npos, '\0');
			JsonifierCore::memcpy(string, stringNew, sizeVal);
			return *this;
		}

		inline explicit String(void* stringNew) noexcept {
			*this = stringNew;
		}

		inline explicit String(void* stringNew, size_t sizeNew) noexcept {
			sizeVal = sizeNew;
			JsonifierCore::memcpy(string, stringNew, sizeVal);
		}

		inline size_t findFirstOf(char charToFind) noexcept {
			return findSingleCharacterAvx2(string, sizeVal, charToFind);
		}

		inline void push_back(char newChar) noexcept {
			if (sizeVal + 1 >= this->size()) {
				this->resize((sizeVal + 1) * 4);
			}
			this->string[sizeVal] = newChar;
			++sizeVal;
		}

		inline void checkForResize(size_t newSizeToAdd) noexcept {
			if (this->sizeVal + newSizeToAdd > this->capacity()) {
				this->resize((this->sizeVal + newSizeToAdd) * 2);
			}
		}

		inline void append(void* newString, size_t size) noexcept {
			if (this->sizeVal + size >= this->capacity()) {
				this->resize((this->sizeVal + size) * 2);
			}
			JsonifierCore::memcpy(this->string + sizeVal, newString, size);
			sizeVal += size;
		}

		inline void append(char newChar) noexcept {
			if (this->sizeVal + 1 >= this->capacity()) {
				this->resize((this->sizeVal + 1) * 2);
			}
			this->string[sizeVal++] = newChar;
		}

		inline void append(StringView newString) noexcept {
			if (this->sizeVal + newString.size() >= this->capacity()) {
				this->resize((this->sizeVal + newString.size()) * 2);
			}
			JsonifierCore::memcpy(this->string + sizeVal, newString.data(), newString.size());
			sizeVal += newString.size();
		}

		inline constexpr bool operator==(const StringView& rhs) const noexcept {
			if (sizeVal != rhs.size()) {
				return false;
			} else if (sizeVal) {
				return JsonifierCore::compare(string, reinterpret_cast<const void*>(rhs.data()), sizeVal);
			} else if (sizeVal == 0 && rhs.size() == 0) {
				return true;
			} else {
				return false;
			}
		}

		inline constexpr bool operator==(const std::string& rhs) const noexcept {
			if (sizeVal != rhs.size()) {
				return false;
			} else if (sizeVal) {
				return JsonifierCore::compare(string, reinterpret_cast<const void*>(rhs.data()), sizeVal);
			} else if (sizeVal == 0 && rhs.size() == 0) {
				return true;
			} else {
				return false;
			}
		}

		inline constexpr bool operator==(const std::string_view& rhs) const noexcept {
			if (sizeVal != rhs.size()) {
				return false;
			} else if (sizeVal) {
				return JsonifierCore::compare(string, reinterpret_cast<const void*>(rhs.data()), sizeVal);
			} else if (sizeVal == 0 && rhs.size() == 0) {
				return true;
			} else {
				return false;
			}
		}

		inline constexpr void clear() noexcept {
			sizeVal = 0;
		}

		inline constexpr operator std::string_view() const noexcept {
			return { string, sizeVal };
		}

		inline bool operator<(const String& other) const noexcept {
			size_t count01{ operator()(*this) };
			size_t count02{ operator()(other) };
			return count01 < count02;
		}

		inline constexpr char* data() const noexcept {
			return string;
		}

		inline constexpr char* data() noexcept {
			return string;
		}

		inline constexpr const size_t capacity() const noexcept {
			return m_capacity;
		}

		void reserve(size_t newCapacity) {
			if (!string) {
				string = static_cast<char*>(aligned_alloc(newCapacity, 32));
			} else {
				string = static_cast<char*>(realloc(string, newCapacity, 32));
			}
			m_capacity = newCapacity;
		}

	  private:
		static constexpr uint64_t fnvOffsetBasis{ 14695981039346656037ULL };
		static constexpr uint64_t fnvPrime{ 1099511628211ULL };
		inline size_t operator()(const String& stringNew) const noexcept {
			size_t value{ fnvOffsetBasis };
			for (size_t i = 0; i < stringNew.size(); ++i) {
				value ^= static_cast<std::uint64_t>(stringNew.data()[i]);
				value *= fnvPrime;
			}
			return value;
		}

		void swap(String& other) noexcept {
			std::swap(string, other.string);
			std::swap(sizeVal, other.sizeVal);
			std::swap(m_capacity, other.m_capacity);
		}

		char* string{};
		size_t sizeVal{};
		size_t m_capacity{};
	};

	inline StringView& StringView::operator=(const String& stringNew) noexcept {
		string = stringNew.data();
		sizeVal = stringNew.size();
		return *this;
	}

	inline constexpr StringView::StringView(const String& stringNew) noexcept {
		string = stringNew.data();
		sizeVal = stringNew.size();
	}

	inline std::ostream& operator<<(std::ostream& out, const String& string) noexcept {
		out << string.operator std::basic_string_view<char, std::char_traits<char>>();
		return out;
	}
}
