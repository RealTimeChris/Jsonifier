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

	inline bool isTypeType(uint8_t c) {
		static constexpr uint8_t array01[]{ "0123456789-ftn\"{[" };
		return find(array01, std::size(array01), &c) != jsonifier::string::npos;
	}

	inline bool isDigitType(uint8_t c) {
		static constexpr uint8_t array01[]{ "0123456789-" };
		return find(array01, std::size(array01), &c) != jsonifier::string::npos;
	}

	inline jsonifier::string_view getValueType(uint8_t charToCheck) {
		static constexpr jsonifier::string_view array{ "Array" };
		static constexpr jsonifier::string_view object{ "Object" };
		static constexpr jsonifier::string_view boolean{ "Bool" };
		static constexpr jsonifier::string_view number{ "Number" };
		static constexpr jsonifier::string_view str{ "String" };
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

	enum class type_of_misread { Wrong_Type = 0, Damaged_Input = 1 };

	template<bool printErrors> class derailleur {
	  public:
		using size_type = uint64_t;

		template<uint8_t c, std::forward_iterator iterator>
		inline static bool checkForMatchClosed(iterator& iter, iterator& end, std::source_location location = std::source_location::current()) {
			if (*iter == c) {
				++iter;
				return true;
			} else {
				reportError<c>(iter, location);
				skipToNextValue(iter, end);
				return false;
			}
		}

		template<uint8_t c, std::forward_iterator iterator> inline static bool checkForMatchOpen(iterator& iter) {
			if (*iter == c) {
				++iter;
				return true;
			} else {
				return false;
			}
		}

		template<std::forward_iterator iterator> inline static void skipToNextValue(iterator& iter, iterator& end) {
			while (iter != end && *iter != static_cast<typename iterator::value_type>(',')) {
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

		template<std::forward_iterator iterator> inline static void skipValue(iterator& iter, iterator& end) {
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

		template<std::forward_iterator iterator> inline static size_type countValueElements(iterator iter, iterator end) {
			size_type currentDepth{ 1 };
			size_type currentCount{ 1 };
			if (*iter == ']' || *iter == '}') {
				++iter;
				return {};
			}
			while (currentDepth > 0 && iter != end) {
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

	  protected:
		template<std::forward_iterator iterator> inline static void skipObject(iterator& iter, iterator& end) {
			++iter;
			size_type currentDepth{ 1 };
			if (*iter == '}') {
				++iter;
				return;
			}
			while (currentDepth > 0 && iter != end) {
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

		template<std::forward_iterator iterator> inline static void skipArray(iterator& iter, iterator& end) {
			++iter;
			size_type currentDepth{ 1 };
			if (*iter == ']') {
				++iter;
				return;
			}
			while (currentDepth > 0 && iter != end) {
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

		template<uint8_t c, std::forward_iterator iterator> inline static void reportError(iterator& iter, std::source_location location) {
			if (printErrors) {
				if (collectMisReadType<c>(iter) == type_of_misread::Wrong_Type) {
					std::cout << "It seems you mismatched a value for a value of type: " << getValueType(c) << ", the found value was actually: " << getValueType(*iter)
							  << ", at index: " << iter.getCurrentIndex() << ", in file: " << location.file_name() << ", at: " << location.line() << ":" << location.column()
							  << ", in function: " << location.function_name() << "()." << std::endl;
				} else {
					std::cout << "Failed to collect a '" << c << "', instead found a '" << *iter << "'"
							  << ", at index: " << iter.getCurrentIndex() << ", in file: " << location.file_name() << ", at: " << location.line() << ":" << location.column()
							  << ", in function: " << location.function_name() << "()." << std::endl;
				}
			}
		}

		template<uint8_t c, std::forward_iterator iterator> inline static type_of_misread collectMisReadType(iterator& iter) {
			if (isTypeType(*iter) && isTypeType(c)) {
				return type_of_misread::Wrong_Type;
			} else {
				return type_of_misread::Damaged_Input;
			}
		}
	};

}
