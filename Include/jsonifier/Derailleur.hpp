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

	enum class type_of_misread { Wrong_Type = 0, Damaged_Input = 1 };

	template<bool printErrors> class derailleur {
	  public:
		template<uint8_t c> inline static bool checkForMatchClosed(structural_iterator& iter, std::source_location location = std::source_location::current()) {
			if (iter == c) {
				++iter;
				return true;
			} else {
				reportError<c>(iter, location);
				skipToNextValue(iter);
				return false;
			}
		}

		template<uint8_t c> inline static bool checkForMatchOpen(structural_iterator& iter) {
			if (iter == c) {
				++iter;
				return true;
			} else {
				return false;
			}
		}

		inline static void skipToNextValue(structural_iterator& iter) {
			while (iter != ',' && iter != iter) {
				switch (**iter) {
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

		inline static void skipValue(structural_iterator& iter) {
			switch (**iter) {
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

		inline static size_t countArrayElements(structural_iterator iter) {
			size_t currentDepth{ 1 };
			size_t currentCount{ 1 };
			if (iter == ']') {
				++iter;
				return {};
			}
			while (currentDepth > 0 && iter != iter) {
				switch (**iter) {
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
			uint64_t currentDepth{ 1 };
			if (iter == '}') {
				++iter;
				return;
			}
			while (currentDepth > 0 && iter != iter) {
				switch (**iter) {
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
			uint64_t currentDepth{ 1 };
			if (iter == ']') {
				++iter;
				return;
			}
			while (currentDepth > 0 && iter != iter) {
				switch (**iter) {
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

		inline static bool isTypeType(uint8_t c) {
			static constexpr uint8_t array01[]{ "0123456789-ftn\"{[" };
			return findFirstOf(array01, std::size(array01), c) != jsonifier::string::npos;
		}

		inline static bool isDigitType(uint8_t c) {
			static constexpr uint8_t array01[]{ "0123456789-" };
			return findFirstOf(array01, std::size(array01), c) != jsonifier::string::npos;
		}

		inline static jsonifier::string_view getValueType(uint8_t charToCheck) {
			static constexpr jsonifier::string_view array{ "array" };
			static constexpr jsonifier::string_view object{ "object" };
			static constexpr jsonifier::string_view boolean{ "Bool" };
			static constexpr jsonifier::string_view number{ "Number" };
			static constexpr jsonifier::string_view str{ "string" };
			static constexpr jsonifier::string_view null{ "Null" };
			if (isDigitType(charToCheck)) {
				return number;
			} else if (charToCheck == 't' || charToCheck == 'f') {
				return boolean;
			} else if (charToCheck == '{') {
				return object;
			} else if (charToCheck == '[') {
				return array;
			} else if (charToCheck == '"') {
				return str;
			} else if (charToCheck == 'n') {
				return null;
			} else {
				return {};
			}
		}

		template<uint8_t c> inline static void reportError(structural_iterator& iter, std::source_location location) {
			if (printErrors) {
				if (collectMisReadType<c>(iter) == type_of_misread::Wrong_Type) {
					std::cout << "It seems you mismatched a value for a value of type: " << getValueType(c) << ", the found value was actually: " << getValueType(**iter)
							  << ", at index: " << iter.getCurrentIndex() << ", in file: " << location.file_name() << ", at: " << location.line() << ":" << location.column()
							  << ", in function: " << location.function_name() << "()." << std::endl;
				} else {
					std::cout << "Failed to collect a '" << c << "', instead found a '" << **iter << "'"
							  << ", at index: " << iter.getCurrentIndex() << ", in file: " << location.file_name() << ", at: " << location.line() << ":" << location.column()
							  << ", in function: " << location.function_name() << "()." << std::endl;
				}
			}
		}

		template<uint8_t c> inline static type_of_misread collectMisReadType(structural_iterator& iter) {
			if (isTypeType(**iter) && isTypeType(c)) {
				return type_of_misread::Wrong_Type;
			} else {
				return type_of_misread::Damaged_Input;
			}
		}
	};

}
