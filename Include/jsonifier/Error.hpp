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
#include <jsonifier/Optional.hpp>
#include <jsonifier/Expected.hpp>
#include <source_location>
#include <unordered_map>
#include <exception>
#include <string>

namespace jsonifier_internal {

	enum class type_of_misread { Wrong_Type = 0, Damaged_Input = 1 };

	enum class error_code {
		Success					 = 0,
		Parse_Error				 = 1,
		Number_Error			 = 2,
		Unknown_Key				 = 3,
		Incorrect_Type			 = 4,
		Setup_Error				 = 5,
		Inadequate_String_Length = 6,
		Key_Parsing_Error		 = 7
	};

	enum class json_structural_type {
		Object_Start = '{',
		Object_End	 = '}',
		Array_Start	 = '[',
		Array_End	 = ']',
		String		 = '"',
		Bool		 = 't',
		Number		 = '-',
		Colon		 = ':',
		Comma		 = ',',
		Null		 = 'n',
	};

	inline static std::unordered_map<error_code, jsonifier::string> errorMap{ { error_code::Success, "Success" }, { error_code::Parse_Error, "Parse Error." },
		{ error_code::Number_Error, "Number Error." }, { error_code::Unknown_Key, "Unknown Key" }, { error_code::Incorrect_Type, "Incorrect Type" },
		{ error_code::Setup_Error, "Setup Error." }, { error_code::Inadequate_String_Length, "Inadequate String Length" }, { error_code::Key_Parsing_Error, "Key Parsing Error" } };

	inline bool isTypeType(uint8_t c) {
		static constexpr uint8_t array01[]{ "0123456789-ftn\"{[" };
		return find(array01, std::size(array01), &c) != jsonifier::string::npos;
	}

	inline bool isDigitType(uint8_t c) {
		return validNumberValues[c];
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

	inline static type_of_misread collectMisReadType(uint8_t c, uint8_t currentValue) {
		if (isTypeType(currentValue) && isTypeType(c)) {
			return type_of_misread::Wrong_Type;
		} else {
			return type_of_misread::Damaged_Input;
		}
	}

	class error {
	  public:
		friend class derailleur;

		inline error() noexcept = default;

		inline error(structural_iterator& iter, json_structural_type typeNew, std::source_location locationNew = std::source_location::current()) noexcept {
			intendedValue = static_cast<uint8_t>(typeNew);
			errorIndex	  = iter.getCurrentStringIndex();
			errorType	  = error_code::Parse_Error;
			location	  = locationNew;
			errorValue	  = *iter;
		}

		inline error& operator=(error_code errorNew) {
			errorType = errorNew;
			return *this;
		}

		inline error(error_code errorNew) {
			*this = errorNew;
		}

		inline operator bool() {
			return errorType != error_code::Success;
		}

		inline bool operator==(const error& rhs) const {
			return errorType == rhs.errorType && errorIndex == rhs.errorIndex && errorValue == rhs.errorValue && intendedValue == rhs.intendedValue;
		}

		inline jsonifier::string reportError() const {
			if (collectMisReadType(intendedValue, errorValue) == type_of_misread::Wrong_Type) {
				return jsonifier::string{ "It seems you mismatched a value for a value of type: " + getValueType(intendedValue) +
					", the found value was actually: " + getValueType(errorValue) + ", at index: " + jsonifier::toString(errorIndex) + ", in file: " + location.file_name() +
					", at: " + jsonifier::toString(location.line()) + ":" + jsonifier::toString(location.column()) + ", in function: " + location.function_name() + "()." };
			} else {
				return jsonifier::string{ "Failed to collect a '" + jsonifier::string{ intendedValue } + "', instead found a '" + errorValue + "'" +
					", at index: " + jsonifier::toString(errorIndex) + ", in file: " + location.file_name() + ", at: " + jsonifier::toString(location.line()) + ":" +
					jsonifier::toString(location.column()) + ", in function: " + location.function_name() + "()." };
			}
		}

	  protected:
		std::source_location location{};
		uint8_t intendedValue{};
		error_code errorType{};
		uint64_t errorIndex{};
		uint8_t errorValue{};
	};

	template<json_structural_type typeNew> inline error createError(structural_iterator& iter, std::source_location locationNew = std::source_location::current()) {
		error newError{ iter, typeNew, locationNew };
		return newError;
	}

	inline std::ostream& operator<<(std::ostream& os, const error& errorNew) {
		os << errorNew.reportError();
		return os;
	}

}
