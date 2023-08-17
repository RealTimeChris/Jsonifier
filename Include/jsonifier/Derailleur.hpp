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
/// https://github.com/RealTimeChris/Jsonifier
/// Feb 20, 2023
#pragma once

#include <jsonifier/StructuralIterator.hpp>
#include <jsonifier/Error.hpp>
#include <jsonifier/Base.hpp>

namespace JsonifierInternal {

	enum class TypeOfMisread { Wrong_Type = 0, Damaged_Input = 1 };

	template<bool printErrors> class Derailleur {
	  public:
		template<uint8_t c> static inline bool checkForMatchClosed(StructuralIterator& iter, std::source_location location = std::source_location::current()) {
			if (iter == c) {
				++iter;
				return true;
			} else {
				reportError<c>(iter, location);
				skipToNextValue<c>(iter);
				return false;
			}
		};

		template<uint8_t c> static inline bool checkForMatchOpen(StructuralIterator& iter) {
			if (iter == c) {
				++iter;
				return true;
			} else {
				return false;
			}
		};

		static inline void skipValue(StructuralIterator& iter) noexcept {
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

		static inline size_t countArrayElements(StructuralIterator iter) noexcept {
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
		static inline void skipObject(StructuralIterator& iter) noexcept {
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

		static inline void skipArray(StructuralIterator& iter) noexcept {
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

		static inline bool isTypeType(uint8_t c) {
			const uint8_t array01[]{ "0123456789-ftn\"{[" };
			return findSingleCharacter(array01, std::size(array01), c) != Jsonifier::String::npos;
		}

		static inline bool isDigitType(uint8_t c) {
			const uint8_t array01[]{ "0123456789-" };
			return findSingleCharacter(array01, std::size(array01), c) != Jsonifier::String::npos;
		}

		template<uint8_t c> static inline void skipToNextValue(StructuralIterator& iter) {
			while (iter != iter) {
				if (iter == ',') {
					return;
				}
				++iter;
			}
			return;
		};

		static inline Jsonifier::StringView getValueType(uint8_t charToCheck) {
			static constexpr Jsonifier::StringView array{ "Array" };
			static constexpr Jsonifier::StringView object{ "Object" };
			static constexpr Jsonifier::StringView boolean{ "Bool" };
			static constexpr Jsonifier::StringView number{ "Number" };
			static constexpr Jsonifier::StringView str{ "String" };
			static constexpr Jsonifier::StringView null{ "Null" };
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

		template<uint8_t c> static inline void reportError(StructuralIterator& iter, std::source_location location) {
			if (printErrors) {
				if (collectMisReadType<c>(iter) == TypeOfMisread::Wrong_Type) {
					std::cout << "It seems you mismatched a value for a value of type: " << getValueType(c) << ", the found value was actually: " << getValueType(**iter)
							  << ", at index: " << iter.getCurrentIndex() << ", in file: " << location.file_name() << ", at: " << location.line() << ":" << location.column()
							  << ", in function: " << location.function_name() << "()." << std::endl;
				} else {
					std::cout << "Failed to collect a '" << c << "', at index: " << iter.getCurrentIndex() << " instead found a '" << **iter << "'"
							  << ", in file: " << location.file_name() << ", at: " << location.line() << ":" << location.column() << ", in function: " << location.function_name()
							  << "()." << std::endl;
				}
			}
		}

		template<uint8_t c> static inline TypeOfMisread collectMisReadType(StructuralIterator& iter) {
			if (isTypeType(**iter) && isTypeType(c)) {
				return TypeOfMisread::Wrong_Type;
			} else {
				return TypeOfMisread::Damaged_Input;
			}
		}
	};

}
