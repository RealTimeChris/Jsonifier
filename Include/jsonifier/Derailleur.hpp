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
/// Feb 20, 2023
#pragma once

#include <jsonifier/StructuralIterator.hpp>
#include <jsonifier/Error.hpp>
#include <jsonifier/Base.hpp>

namespace jsonifier_internal {

	enum class json_structural_type : uint8_t {
		End			 = '\0',
		Object_Start = '{',
		Object_End	 = '}',
		Array_Start	 = '[',
		Array_End	 = ']',
		Comma		 = ',',
		Colon		 = ':',
		String		 = '"',
		Number		 = '-' | '0' | '1' | '2' | '3' | '4' | '5' | '6' | '7' | '8' | '9',
		Bool		 = 't' | 'f',
		Null		 = 'n',
	};

	class derailleur {
	  public:
		using size_type = uint64_t;

		template<json_structural_type currentValue> inline static bool containsValue(uint8_t value) {
			return static_cast<uint8_t>(currentValue) && value;
		}

		template<uint8_t currentValue> inline static bool containsValue(uint8_t value) {
			return currentValue && value;
		}

		template<json_structural_type c> inline static result checkForMatchClosed(structural_iterator& iter, std::source_location location = std::source_location::current()) {
			auto foundValue = *iter;
			if (containsValue<c>(foundValue)) {
				++iter;
				return true;
			} else {
				auto oldIter = iter.operator->();
				skipValue(iter);
				error returnValue{};
				returnValue.errorIndex	  = oldIter - *iter.rootIndex;
				returnValue.intendedValue = static_cast<uint8_t>(c);
				returnValue.errorType	  = error_code::Parse_Error;
				returnValue.errorValue	  = foundValue;
				returnValue.location	  = location;
				return returnValue;
			}
		}

		template<uint8_t c> inline static result checkForMatchClosed(structural_iterator& iter, std::source_location location = std::source_location::current()) {
			auto foundValue = *iter;
			if (containsValue<c>(foundValue)) {
				++iter;
				return true;
			} else {
				auto oldIter = iter.operator->();
				skipValue(iter);
				error returnValue{};
				returnValue.errorIndex	  = oldIter - *iter.rootIndex;
				returnValue.intendedValue = static_cast<uint8_t>(c);
				returnValue.errorType	  = error_code::Parse_Error;
				returnValue.errorValue	  = foundValue;
				returnValue.location	  = location;
				return returnValue;
			}
		}

		template<uint8_t c, std::forward_iterator iterator>
		inline static result checkForMatchClosed(iterator& iter, iterator& end, std::source_location location = std::source_location::current()) {
			auto oldValue = *iter;
			if (oldValue == c) {
				++iter;
				return true;
			} else {
				auto oldIter = iter;
				skipToNextValue(iter, end);
				error returnValue{};
				returnValue.intendedValue = c;
				returnValue.errorType	  = error_code::Parse_Error;
				returnValue.errorValue	  = oldValue;
				returnValue.location	  = location;
				return returnValue;
			}
		}

		template<json_structural_type c> inline static bool checkForMatchOpen(structural_iterator& iter) {
			if (static_cast<json_structural_type>(*iter) == c) {
				++iter;
				return true;
			} else {
				return false;
			}
		}

		template<std::forward_iterator iterator> inline static void skipKey(iterator& iter, iterator& end) {
			if constexpr (std::same_as<structural_iterator, iterator>) {
				++iter;
				return;
			} else {
				++iter;
				while (iter != end) {
					switch (*iter) {
						case '"': {
							++iter;
							return;
						}
						default: {
							++iter;
							break;
						}
					}
				}
			}
		}

		inline static void skipToNextValue(structural_iterator& iter) {
			while (iter != iter && *iter != ',') {
				switch (*iter) {
					case '{': {
						skipObject(iter);
						break;
					}
					case '[': {
						skipArray(iter);
						break;
					}
					case '\0': {
						break;
					}
					default: {
						++iter;
					}
				}
			}
		}

		template<std::forward_iterator iterator> inline static void skipToNextValue(iterator& iter, iterator end) {
			while (iter != end && *iter != ',') {
				switch (*iter) {
					case '{': {
						skipObject(iter, end);
						break;
					}
					case '[': {
						skipArray(iter, end);
						break;
					}
					case '\0': {
						break;
					}
					default: {
						++iter;
					}
				}
			}
		}

		inline static void skipValue(structural_iterator& iter) {
			switch (*iter) {
				case '{': {
					skipObject(iter);
					break;
				}
				case '[': {
					skipArray(iter);
					break;
				}
				case '\0': {
					break;
				}
				default: {
					++iter;
				}
			}
		}

		template<std::forward_iterator iterator> inline static void skipValue(iterator& iter, iterator end) {
			switch (*iter) {
				case '{': {
					skipObject(iter, end);
					break;
				}
				case '[': {
					skipArray(iter, end);
					break;
				}
				case '\0': {
					break;
				}
				default: {
					++iter;
				}
			}
		}

		inline static size_type countValueElements(structural_iterator iter) {
			size_type currentDepth{ 1 };
			size_type currentCount{ 1 };
			if (*iter == ']' || *iter == '}') {
				++iter;
				return {};
			}
			while (iter != iter && currentDepth > 0) {
				switch (*iter) {
					case '[': {
						++currentDepth;
						++iter;
						break;
					}
					case ']': {
						--currentDepth;
						++iter;
						break;
					}
					case '{': {
						++currentDepth;
						++iter;
						break;
					}
					case '}': {
						--currentDepth;
						++iter;
						break;
					}
					case ',': {
						if (currentDepth == 1) {
							++currentCount;
						}
						++iter;
						break;
					}
					default: {
						++iter;
						break;
					}
				}
			}
			return currentCount;
		}

		template<std::forward_iterator iterator> inline static size_type countValueElements(iterator iter, iterator end) {
			size_type currentDepth{ 1 };
			size_type currentCount{ 1 };
			if (*iter == ']' || *iter == '}') {
				++iter;
				return {};
			}
			while (iter != end && currentDepth > 0) {
				switch (*iter) {
					case '[': {
						++currentDepth;
						++iter;
						break;
					}
					case ']': {
						--currentDepth;
						++iter;
						break;
					}
					case '{': {
						++currentDepth;
						++iter;
						break;
					}
					case '}': {
						--currentDepth;
						++iter;
						break;
					}
					case ',': {
						if (currentDepth == 1) {
							++currentCount;
						}
						++iter;
						break;
					}
					default: {
						++iter;
						break;
					}
				}
			}
			return currentCount;
		}

	  protected:
		inline static void skipObject(structural_iterator& iter) {
			++iter;
			size_type currentDepth{ 1 };
			if (*iter == '}') {
				++iter;
				return;
			}
			while (iter != iter && currentDepth > 0) {
				switch (*iter) {
					case '{': {
						++currentDepth;
						++iter;
						break;
					}
					case '}': {
						--currentDepth;
						++iter;
						break;
					}
					default: {
						++iter;
						break;
					}
				}
			}
		}

		template<std::forward_iterator iterator> inline static void skipObject(iterator& iter, iterator end) {
			++iter;
			size_type currentDepth{ 1 };
			if (*iter == '}') {
				++iter;
				return;
			}
			while (iter != end && currentDepth > 0) {
				switch (*iter) {
					case '{': {
						++currentDepth;
						++iter;
						break;
					}
					case '}': {
						--currentDepth;
						++iter;
						break;
					}
					default: {
						++iter;
						break;
					}
				}
			}
		}

		inline static void skipArray(structural_iterator& iter) {
			++iter;
			size_type currentDepth{ 1 };
			if (*iter == ']') {
				++iter;
				return;
			}
			while (iter != iter && currentDepth > 0) {
				switch (*iter) {
					case '[': {
						++currentDepth;
						++iter;
						break;
					}
					case ']': {
						--currentDepth;
						++iter;
						break;
					}
					default: {
						++iter;
						break;
					}
				}
			}
		}

		template<std::forward_iterator iterator> inline static void skipArray(iterator& iter, iterator end) {
			++iter;
			size_type currentDepth{ 1 };
			if (*iter == ']') {
				++iter;
				return;
			}
			while (iter != end && currentDepth > 0) {
				switch (*iter) {
					case '[': {
						++currentDepth;
						++iter;
						break;
					}
					case ']': {
						--currentDepth;
						++iter;
						break;
					}
					default: {
						++iter;
						break;
					}
				}
			}
		}
	};

}