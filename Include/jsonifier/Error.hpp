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

#include <jsonifier/StructuralIterator.hpp>
#include <jsonifier/NumberUtils.hpp>
#include <jsonifier/StringView.hpp>
#include <jsonifier/Expected.hpp>
#include <source_location>
#include <unordered_map>
#include <exception>
#include <string>

namespace jsonifier_internal {

	enum class error_code : uint8_t {
		Success					 = 0,
		Parse_Error				 = 1,
		Number_Error			 = 2,
		Unknown_Key				 = 3,
		Incorrect_Type			 = 4,
		Setup_Error				 = 5,
		Inadequate_String_Length = 6,
		Key_Parsing_Error		 = 7,
		Invalid_Escape			 = 8,
		Wrong_Type				 = 9,
		Damaged_Input			 = 10
	};

	enum class json_structural_type : uint8_t {
		Unset		 = 0x00u,
		Object_Start = 0x7Bu,
		Object_End	 = 0x7Du,
		Array_Start	 = 0x5Bu,
		Array_End	 = 0x5Du,
		String		 = 0x22u,
		Bool		 = 0x74u,
		Number		 = 0x2Du,
		Colon		 = 0x3Au,
		Comma		 = 0x2Cu,
		Null		 = 0x6Eu
	};

	jsonifier_inline static std::unordered_map<error_code, jsonifier::string> errorMap{ { error_code::Success, "Success" }, { error_code::Parse_Error, "Parse Error." },
		{ error_code::Number_Error, "Number Error." }, { error_code::Unknown_Key, "Unknown Key" }, { error_code::Incorrect_Type, "Incorrect Type" },
		{ error_code::Setup_Error, "Setup Error." }, { error_code::Inadequate_String_Length, "Inadequate String Length" }, { error_code::Key_Parsing_Error, "Key Parsing Error" } };

	jsonifier_inline bool isTypeType(uint8_t c) {
		static jsonifier_constexpr uint8_t array01[]{ "0123456789-ftn\"{[" };
		return find(array01, std::size(array01), &c, 1) != jsonifier::string::npos;
	}

	jsonifier_inline bool isDigitType(uint8_t c) {
		return numberTable[c];
	}

	jsonifier_inline jsonifier::string_view getValueType(uint8_t charToCheck) {
		static jsonifier_constexpr jsonifier::string_view array{ "Array" };
		static jsonifier_constexpr jsonifier::string_view object{ "Object" };
		static jsonifier_constexpr jsonifier::string_view boolean{ "Bool" };
		static jsonifier_constexpr jsonifier::string_view number{ "Number" };
		static jsonifier_constexpr jsonifier::string_view str{ "String" };
		static jsonifier_constexpr jsonifier::string_view null{ "Null" };
		static jsonifier_constexpr jsonifier::string_view unset{ "Unset" };
		if (isDigitType(charToCheck)) {
			return number;
		} else if (charToCheck == 0x74u || charToCheck == 0x66u) {
			return boolean;
		} else if (charToCheck == 0x7B) {
			return object;
		} else if (charToCheck == 0x5B) {
			return array;
		} else if (charToCheck == 0x22u) {
			return str;
		} else if (charToCheck == 0x6Eu) {
			return null;
		} else if (charToCheck == 0x00u) {
			return unset;
		} else {
			return {};
		}
	}

	jsonifier_inline static error_code collectMisReadType(uint8_t c, uint8_t currentValue) {
		if (isTypeType(currentValue) && isTypeType(c)) {
			return error_code::Wrong_Type;
		} else {
			return error_code::Damaged_Input;
		}
	}

	class error {
	  public:
		friend class derailleur;

		jsonifier_inline error() noexcept = default;

		jsonifier_inline error(structural_iterator iter, json_structural_type typeNew, std::source_location locationNew = std::source_location::current()) noexcept {
			intendedValue  = static_cast<uint8_t>(typeNew);
			errorIndex	   = static_cast<uint64_t>(iter.getCurrentStringIndex());
			errorIndexReal = roundDownToMultiple<BitsPerStep>(static_cast<int64_t>(iter.getCurrentStringIndex()));
			if (errorIndexReal < jsonifier::string::maxSize()) {
				stringView = iter.getRootPtr();
			}
			stringLength = static_cast<uint64_t>(iter.getEndPtr() - iter.getRootPtr());
			location	 = locationNew;
			errorValue	 = *iter;
			errorType	 = collectMisReadType(static_cast<uint8_t>(typeNew), errorValue);
		}

		jsonifier_inline error(structural_iterator& iter, error_code typeNew, std::source_location locationNew = std::source_location::current()) noexcept {
			errorIndex	   = static_cast<uint64_t>(iter.getCurrentStringIndex());
			errorIndexReal = roundDownToMultiple<BitsPerStep>(static_cast<int64_t>(iter.getCurrentStringIndex()));
			if (errorIndexReal < jsonifier::string::maxSize()) {
				stringView = iter.getRootPtr();
			}
			location   = locationNew;
			errorType  = typeNew;
			errorValue = *iter;
		}

		jsonifier_inline error& operator=(error_code errorNew) {
			errorType = errorNew;
			return *this;
		}

		jsonifier_inline error(error_code errorNew) {
			*this = errorNew;
		}

		jsonifier_inline operator bool() {
			return errorType != error_code::Success;
		}

		jsonifier_inline bool operator==(const error& rhs) const {
			return errorType == rhs.errorType && errorIndex == rhs.errorIndex && errorValue == rhs.errorValue && intendedValue == rhs.intendedValue;
		}

		jsonifier_inline jsonifier::string reportError() const {
#if defined(DEV)
			simd_string_reader section{};
			std::string resultString{};
			if (stringView) {
				resultString = section.resetWithErrorPrintOut<true>(jsonifier::string_view_base<uint8_t>{ stringView, stringLength }, errorIndexReal);
			}
#endif
			switch (errorType) {
				case error_code::Wrong_Type: {
					jsonifier::string returnValue{ "It seems you mismatched a value for a value of type: " + getValueType(intendedValue) +
						", the found value was actually: " + getValueType(errorValue) + ", at index: " + jsonifier::toString(errorIndex) + ", in file: " + location.file_name() +
						", at: " + jsonifier::toString(location.line()) + ":" + jsonifier::toString(location.column()) + ", in function: " + location.function_name() + "()." };
#if defined(DEV)
					if (stringView) {
						returnValue += "\nHere's some of the string's indices:\n" + resultString;
					}
#endif
					return returnValue;
				}
				case error_code::Damaged_Input: {
					jsonifier::string returnValue{ "Failed to collect a '" + jsonifier::string{ intendedValue } + "', instead found a '" + static_cast<char>(errorValue) + "'" +
						", at index: " + jsonifier::toString(errorIndex) + ", in file: " + location.file_name() + ", at: " + jsonifier::toString(location.line()) + ":" +
						jsonifier::toString(location.column()) + ", in function: " + location.function_name() + "()." };
#if defined(DEV)
					if (stringView) {
						returnValue += "\nHere's some of the string's indices:\n" + resultString;
					}
#endif
					return returnValue;
				}
				case error_code::Invalid_Escape: {
					jsonifier::string returnValue{ "Invalid escape at index: " + jsonifier::toString(errorIndex) + ", in file: " + location.file_name() +
						", at: " + jsonifier::toString(location.line()) + ":" + jsonifier::toString(location.column()) + ", in function: " + location.function_name() + "()." };
#if defined(DEV)
					if (stringView) {
						returnValue += "\nHere's some of the string's indices:\n" + resultString;
					}
#endif
					return returnValue;
				}
				case error_code::Parse_Error: {
					jsonifier::string returnValue{ "Parse Error at index: " + jsonifier::toString(errorIndex) + ", in file: " + location.file_name() +
						", at: " + jsonifier::toString(location.line()) + ":" + jsonifier::toString(location.column()) + ", in function: " + location.function_name() + "()." };
#if defined(DEV)
					if (stringView) {
						returnValue += "\nHere's some of the string's indices:\n" + resultString;
					}
#endif
					return returnValue;
				}
				case error_code::Success: {
					[[fallthrough]];
				}
				case error_code::Inadequate_String_Length: {
					[[fallthrough]];
				}
				case error_code::Incorrect_Type: {
					[[fallthrough]];
				}
				case error_code::Key_Parsing_Error: {
					[[fallthrough]];
				}
				case error_code::Number_Error: {
					[[fallthrough]];
				}
				case error_code::Unknown_Key: {
					[[fallthrough]];
				}
				case error_code::Setup_Error: {
					[[fallthrough]];
				}
				default: {
					return {};
				}
			}
		}

	  protected:
		std::source_location location{};
		string_view_ptr stringView{};
		uint64_t errorIndexReal{};
		uint64_t stringLength{};
		uint8_t intendedValue{};
		error_code errorType{};
		uint64_t errorIndex{};
		uint8_t errorValue{};
	};

	template<error_code typeNew> jsonifier_inline error createError(structural_iterator& iter, std::source_location locationNew = std::source_location::current()) {
		error newError{ iter, typeNew, locationNew };
		return newError;
	}

	template<json_structural_type typeNew> jsonifier_inline error createError(structural_iterator& iter, std::source_location locationNew = std::source_location::current()) {
		error newError{ iter, typeNew, locationNew };
		return newError;
	}

	jsonifier_inline std::ostream& operator<<(std::ostream& os, const error& errorNew) {
		os << errorNew.reportError();
		return os;
	}

}
