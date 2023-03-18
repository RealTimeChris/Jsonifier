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

#include <string>
#include <immintrin.h>
#include <iostream>
#include <string_view>

namespace Jsonifier {

	using StructuralIndex = const uint8_t**;
	using StringViewPtr = const uint8_t*;
	using StringBufferPtr = uint8_t*;

	class String;

	inline size_t findSingleCharacter(const char* const string, size_t lengthNew, const char charToFind) noexcept;

	inline bool compare(const char* destVector, const char* sourceVector, size_t lengthNew) noexcept;

	class StringViewIterator {
	  public:
		inline constexpr StringViewIterator(const char* ptrNew) noexcept {
			ptr = ptrNew;
		}

		inline constexpr char operator*() noexcept {
			return *this->ptr;
		}

		inline constexpr StringViewIterator& operator++() noexcept {
			++ptr;
			return *this;
		}

		inline constexpr friend bool operator==(const StringViewIterator& lhs, const StringViewIterator& rhs) {
			return lhs.ptr == rhs.ptr;
		}

	  protected:
		const char* ptr{};
	};

	struct StringView {
	  public:
		using difference_type = std::ptrdiff_t;
		using value_type = char;
		using pointer = value_type*;
		using reference = value_type&;

		inline constexpr StringView& operator=(StringView&& other) noexcept {
			if (this != &other) {
				this->sizeVal = other.sizeVal;
				this->string = other.string;
				other.string = nullptr;
				other.sizeVal = 0;
			}
			return *this;
		}

		inline constexpr StringView(StringView&& other) noexcept {
			*this = std::move(other);
		}

		inline constexpr StringView& operator=(const StringView& other) {
			if (this != &other) {
				this->sizeVal = other.sizeVal;
				this->string = other.string;
			}
			return *this;
		}

		inline constexpr StringView(const StringView& other) {
			*this = other;
		}

		inline constexpr StringView() noexcept = default;

		inline StringView& operator=(const std::string& stringNew) noexcept {
			string = stringNew.data();
			sizeVal = stringNew.size();
			return *this;
		}

		inline constexpr StringView(const std::string& stringNew) noexcept {
			string = stringNew.data();
			sizeVal = stringNew.size();
		}

		inline StringView& operator=(const String& stringNew) noexcept;
		
		inline constexpr StringView(const String& stringNew) noexcept;

		inline constexpr StringView(const char*& stringNew) noexcept {
			sizeVal = std::char_traits<char>::length(stringNew);
			string = stringNew;
		}

		inline constexpr StringView(const std::string_view& stringNew) noexcept {
			string = stringNew.data();
			sizeVal = stringNew.size();
		}

		inline constexpr StringView(const char* stringNew, size_t sizeNew) noexcept : sizeVal{ sizeNew } {
			string = stringNew;
		}

		template<size_t Size> inline constexpr StringView(const char (&stringNew)[Size]) noexcept : sizeVal{ Size } {
			string = stringNew;
		}

		inline constexpr char operator[](size_t index) const noexcept {
			return string[index];
		}

		inline constexpr operator std::string_view() const noexcept {
			return { string, sizeVal };
		}

		inline bool operator<(const StringView& other) const noexcept {
			size_t count01{ operator()(*this) };
			size_t count02{ operator()(other) };
			return count01 < count02;
		}

		inline constexpr bool operator==(const StringView& rhs) const noexcept {
			if (sizeVal != rhs.sizeVal) {
				return false;
			} else if (sizeVal) {
				return compare(string, reinterpret_cast<const char*>(rhs.data()), sizeVal);
			} else if (sizeVal == 0 && rhs.size() == 0) {
				return true;
			} else {
				return false;
			}
		}

		inline constexpr StringViewIterator begin() const {
			return this->string;
		}

		inline constexpr StringViewIterator end() const {
			return &this->string[sizeVal];
		}

		inline constexpr const char* data() const noexcept {
			return string;
		}

		inline constexpr const char& back() const {
			return this->string[sizeVal - 1];
		}

		inline constexpr size_t size() const noexcept {
			return sizeVal;
		}

	  protected:
		mutable size_t sizeVal{};
		const char* string{};

		template<size_t Size> size_t inline constexpr findSingleCharacter(const char (&stringNew)[Size], char charToFind) {
			for (size_t x = 0; x < Size; ++x) {
				if (stringNew[x] == charToFind) {
					return x;
				}
			}
			return std::string::npos;
		}

		static constexpr uint64_t fnvOffsetBasis{ 14695981039346656037ULL };
		static constexpr uint64_t fnvPrime{ 1099511628211ULL };

		inline size_t operator()(const StringView& string) const noexcept {
			size_t value{ fnvOffsetBasis };
			for (size_t i = 0; i < string.size(); ++i) {
				value ^= static_cast<std::uint64_t>(string.data()[i]);
				value *= fnvPrime;
			}
			return value;
		}
	};

	inline std::ostream& operator<<(std::ostream& out, const StringView& string) noexcept {
		out << string.operator std::basic_string_view<char, std::char_traits<char>>();
		return out;
	}

}
