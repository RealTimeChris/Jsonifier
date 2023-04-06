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
        Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301
        USA
*/
/// https://github.com/RealTimeChris/Jsonifier
/// Feb 20, 2023
#pragma once

#include <jsonifier/StructuralIterator.hpp>
#include <jsonifier/Error.hpp>
#include <jsonifier/Base.hpp>

namespace Jsonifier {

	enum class TypeOfMisread { Wrong_Type = 0, Damaged_Input = 1 };

	template<bool printErrors> class Derailleur {
	  public:
		template<uint8_t c>
		inline static bool checkForMatchClosed(StructuralIterator& iter, std::source_location location = std::source_location::current()) {
			if (iter == c) {
				++iter;
				return true;
			} else {
				reportError<c>(iter, location);
				if (skipToNextValue<c>(iter)) {
					return true;
				} else {
					return false;
				}
			}
		};

		template<uint8_t c> inline static bool checkForMatchOpen(StructuralIterator& iter) {
			if (iter == c) {
				++iter;
				return true;
			} else {
				return false;
			}
		};

		template<std::forward_iterator ITy> inline static void skipValue(ITy& iter) noexcept {
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

	  protected:
		template<std::forward_iterator ITy> inline static void skipObject(ITy& iter) noexcept {
			++iter;
			size_t currentDepth{ 1 };
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

		template<std::forward_iterator ITy> inline static void skipArray(ITy& iter) noexcept {
			++iter;
			size_t currentDepth{ 1 };
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

		template<uint8_t c> inline static bool skipToNextValue(StructuralIterator& iter) {
			while (iter != iter) {
				if (iter == c) {
					++iter;
					return true;
				}
				++iter;
			}
			return false;
		};

		inline static StringView getValueType(uint8_t charToCheck) {
			static constexpr StringView array{ "Array" };
			static constexpr StringView object{ "Object" };
			static constexpr StringView boolean{ "Bool" };
			static constexpr StringView number{ "Number" };
			static constexpr StringView string{ "String" };
			static constexpr StringView null{ "Null" };
			if (charToCheck == '0' || charToCheck == '1' || charToCheck == '2' || charToCheck == '3' || charToCheck == '4' || charToCheck == '5' ||
				charToCheck == '6' || charToCheck == '7' || charToCheck == '8' || charToCheck == '9' || charToCheck == '-') {
				return number;
			} else if (charToCheck == 't' || charToCheck == 'f') {
				return boolean;
			} else if (charToCheck == '{') {
				return object;
			} else if (charToCheck == '[') {
				return array;
			} else if (charToCheck == '"') {
				return string;
			} else if (charToCheck == 'n') {
				return null;
			} else {
				return {};
			}
		}

		template<uint8_t c> inline static void reportError(StructuralIterator& iter, std::source_location location) {
			if (printErrors) {
				if (collectMisReadType<c>(iter) == TypeOfMisread::Wrong_Type) {
					std::cout << "It seems you mismatched a value for a value of type: " << getValueType(c)
							  << ", the found value was actually: " << getValueType(**iter) << ", at index: " << iter.getCurrentIndex()
							  << ", in file: " << location.file_name() << ", at: " << location.line() << ":" << location.column()
							  << ", in function: " << location.function_name() << "()." << std::endl;
				} else {
					std::cout << "Failed to collect a '" << c << "', at index: " << iter.getCurrentIndex() << " instead found a '" << **iter << "'"
							  << ", in file: " << location.file_name() << ", at: " << location.line() << ":" << location.column()
							  << ", in function: " << location.function_name() << "()." << std::endl;
				}
			}
		}

		template<uint8_t c> inline static TypeOfMisread collectMisReadType(StructuralIterator& iter) {
			if ((iter == '"' || iter == '-' || iter == '1' || iter == '2' || iter == '3' || iter == '4' || iter == '5' || iter == '6' ||
					iter == '7' || iter == '8' || iter == '9' || iter == '0' || iter == 't' || iter == 'f' || iter == 'n' || iter == '[' ||
					iter == '{' || iter == '2' || iter == '2' || iter == '2') &&
				(c == '"' || c == '-' || c == '1' || c == '2' || c == '3' || c == '4' || c == '5' || c == '6' || c == '7' || c == '8' || c == '9' ||
					c == '0' || c == 't' || c == 'f' || c == 'n' || c == '{' || c == '[')) {
				return TypeOfMisread::Wrong_Type;
			} else {
				return TypeOfMisread::Damaged_Input;
			}
		}
	};

}
