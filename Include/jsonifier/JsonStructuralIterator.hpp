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


#include <jsonifier/Reflection.hpp>
#include <jsonifier/StringUtils.hpp>
#include <jsonifier/HashSet.hpp>
#include <jsonifier/StrToI.hpp>
#include <jsonifier/Error.hpp>

namespace jsonifier_internal {

	constexpr std::array<parse_errors, 256> errorMapNew{ [] {
		std::array<parse_errors, 256> returnValues{};
		returnValues['{'] = parse_errors::Missing_Object_Start;
		returnValues['}'] = parse_errors::Missing_Comma_Or_Object_End;
		returnValues['['] = parse_errors::Missing_Array_Start;
		returnValues[']'] = parse_errors::Missing_Comma_Or_Array_End;
		returnValues['"'] = parse_errors::Missing_String_Start;
		returnValues[','] = parse_errors::Missing_Comma;
		returnValues[':'] = parse_errors::Missing_Colon;
		returnValues['f'] = parse_errors::Invalid_Bool_Value;
		returnValues['t'] = parse_errors::Invalid_Bool_Value;
		returnValues['n'] = parse_errors::Invalid_Null_Value;
		returnValues['-'] = parse_errors::Invalid_Number_Value;
		returnValues['0'] = parse_errors::Invalid_Number_Value;
		returnValues['1'] = parse_errors::Invalid_Number_Value;
		returnValues['2'] = parse_errors::Invalid_Number_Value;
		returnValues['3'] = parse_errors::Invalid_Number_Value;
		returnValues['4'] = parse_errors::Invalid_Number_Value;
		returnValues['5'] = parse_errors::Invalid_Number_Value;
		returnValues['6'] = parse_errors::Invalid_Number_Value;
		returnValues['7'] = parse_errors::Invalid_Number_Value;
		returnValues['8'] = parse_errors::Invalid_Number_Value;
		returnValues['9'] = parse_errors::Invalid_Number_Value;
		return returnValues;
	}() };

	class json_structural_iterator {
	  public:
		friend class derailleur;
		template<json_structural_type value_type, typename derived_type> friend struct validate_impl;

		using iterator_concept	  = std::bidirectional_iterator_tag;
		using iterator_category	  = std::bidirectional_iterator_tag;
		using value_type		  = char;
		using value_type_internal = char;
		using pointer_internal	  = structural_index*;
		using pointer			  = value_type*;
		using size_type			  = uint64_t;

		JSONIFIER_INLINE json_structural_iterator() noexcept = default;

		JSONIFIER_INLINE json_structural_iterator(structural_index* startPtr, structural_index* endPtr) noexcept
			: currentIndex{ startPtr }, rootIndex{ startPtr }, endIndex{ endPtr } {};

		JSONIFIER_INLINE value_type operator*() const {
			return *currentIndex ? **currentIndex : defaultValue;
		}

		JSONIFIER_INLINE auto operator->() const {
			return *currentIndex;
		}

		JSONIFIER_INLINE json_structural_iterator& operator++() {
			++currentIndex;
			return *this;
		}

		JSONIFIER_INLINE json_structural_iterator& operator--() {
			--currentIndex;
			return *this;
		}

		JSONIFIER_INLINE json_structural_iterator operator+(uint64_t valueNew) {
			json_structural_iterator temp{ *this };
			for (uint64_t x = 0; x < valueNew; ++x) {
				++temp;
			}
			return temp;
		}

		JSONIFIER_INLINE operator const char*() const {
			return *currentIndex;
		}

		JSONIFIER_INLINE json_structural_iterator sub(uint64_t valueNew) {
			json_structural_iterator temp{ *this };
			for (uint64_t x = 0; x < valueNew; ++x) {
				--temp;
			}
			return temp;
		}

		JSONIFIER_INLINE int64_t operator-(const char* valueNew) {
			return *currentIndex - valueNew;
		}

		JSONIFIER_INLINE json_structural_iterator operator++(int32_t) {
			json_structural_iterator temp{ *this };
			++(*this);
			return temp;
		}

		JSONIFIER_INLINE auto getEndPtr() const {
			return *endIndex;
		}

		JSONIFIER_INLINE auto getRootPtr() const {
			return *rootIndex;
		}

		JSONIFIER_INLINE bool operator==(const json_structural_iterator&) const {
			return *currentIndex == nullptr;
		}

		JSONIFIER_INLINE operator bool() const {
			return *currentIndex != nullptr;
		}

		JSONIFIER_INLINE void swap(json_structural_iterator& other) {
			std::swap(currentIndex, other.currentIndex);
			std::swap(rootIndex, other.rootIndex);
			std::swap(endIndex, other.endIndex);
		}

	  protected:
		static constexpr value_type defaultValue{ 0x00ll };
		pointer_internal currentIndex{};
		pointer_internal rootIndex{};
		pointer_internal endIndex{};
	};

}