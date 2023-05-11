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

namespace JsonifierInternal {

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

		inline static void skipValue(StructuralIterator& iter) noexcept {
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

		inline static size_t countArrayElements(StructuralIterator iter) noexcept {
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
		inline static void skipObject(StructuralIterator& iter) noexcept {
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

		inline static void skipArray(StructuralIterator& iter) noexcept {
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

		inline static bool isTypeType(uint8_t c) {
			__m256i valueToCheck{ _mm256_set1_epi8(c) };
			__m256i valuesToCheckFor{ _mm256_set_epi8('0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '-', 'f', 't', 'n', '"', '{', '[', 0, 0, 0, 0,
				0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0) };
			__m256i comparison = _mm256_cmpeq_epi8(valuesToCheckFor, valueToCheck);
			return _mm256_movemask_epi8(comparison) != 0;
		}

		inline static bool isDigitType(uint8_t c) {
			__m128i valueToCheck{ _mm_set1_epi8(c) };
			__m128i valuesToCheckFor{ _mm_set_epi8('0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '-', 0, 0, 0, 0, 0) };
			__m128i comparison = _mm_cmpeq_epi8(valuesToCheckFor, valueToCheck);
			return _mm_movemask_epi8(comparison) != 0;
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

		inline static Jsonifier::StringView getValueType(uint8_t charToCheck) {
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
			if (isTypeType(**iter) && isTypeType(c)) {
				return TypeOfMisread::Wrong_Type;
			} else {
				return TypeOfMisread::Damaged_Input;
			}
		}
	};

}
