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

#include <jsonifier/NumberUtils.hpp>
#include <jsonifier/StringView.hpp>
#include <jsonifier/Simd.hpp>
#include <source_location>
#include <unordered_map>
#include <exception>
#include <string>

namespace jsonifier_internal {

	enum class error_code : uint8_t {
		Success						   = 0,
		Incorrect_Type				   = 1,
		Setup_Error					   = 2,
		Damaged_Input				   = 3,
		Serialize_Error				   = 4,
		No_Input					   = 5,
		Requires_Array_Or_Object	   = 6,
		Missing_Colon				   = 7,
		Missing_Comma_Or_Closing_Brace = 8,
		Invalid_Escape_Characters	   = 9,
		Invalid_String_Characters	   = 10,
		Invalid_Null_Value			   = 11,
		Invalid_Bool_Value			   = 12,
		Invalid_Number_Value		   = 13,
		Broken_Array_Start			   = 14,
		Broken_Object_Start			   = 15,
		Prettify_Error				   = 16,
		Minify_Error				   = 17,
		Validate_Error				   = 18
	};

	inline std::unordered_map<error_code, jsonifier::string_view> errorMap{ { error_code::Success, "Success" }, { error_code::Incorrect_Type, "Incorrect Type" },
		{ error_code::Setup_Error, "Setup Error." }, { error_code::Damaged_Input, "Damaged Input" }, { error_code::Serialize_Error, "Serialize Error" },
		{ error_code::No_Input, "No Input" }, { error_code::Requires_Array_Or_Object, "Requires Array Or Object" }, { error_code::Missing_Colon, "Missing Colon" },
		{ error_code::Missing_Comma_Or_Closing_Brace, "Missing Comma Or Closing Brace" }, { error_code::Invalid_Escape_Characters, "Invalid Escape Characters" },
		{ error_code::Invalid_String_Characters, "Invalid String Characters" }, { error_code::Invalid_Null_Value, "Invalid Null Value" },
		{ error_code::Invalid_Bool_Value, "Invalid Bool Value" }, { error_code::Invalid_Number_Value, "Invalid Number Value" },
		{ error_code::Broken_Array_Start, "Broken Array Start" }, { error_code::Broken_Object_Start, "Broken Object Start" }, { error_code::Prettify_Error, "Prettify Error" },
		{ error_code::Minify_Error, "Minify Error" }, { error_code::Validate_Error, "Validate Error" } };


	enum json_structural_type : uint8_t {
		Jsonifier_Unset		   = 0x00u,
		Jsonifier_Object_Start = 0x7Bu,
		Jsonifier_Object_End   = 0x7Du,
		Jsonifier_Array_Start  = 0x5Bu,
		Jsonifier_Array_End	   = 0x5Du,
		Jsonifier_String	   = 0x22u,
		Jsonifier_Bool		   = 0x74u,
		Jsonifier_Number	   = 0x2Du,
		Jsonifier_Colon		   = 0x3Au,
		Jsonifier_Comma		   = 0x2Cu,
		Jsonifier_Null		   = 0x6Eu
	};

	JSONIFIER_INLINE bool isTypeType(uint8_t c) {
		static constexpr uint8_t array01[]{ "0123456789-ftn\"{[" };
		return jsonifier::string_view_base<uint8_t>{ array01, std::size(array01) }.find(c) != jsonifier::string::npos;
	}

	JSONIFIER_INLINE bool isDigitType(uint8_t c) {
		return numberTable[c];
	}

	JSONIFIER_INLINE jsonifier::string_view getValueType(uint8_t charToCheck) {
		static constexpr jsonifier::string_view array{ "Array" };
		static constexpr jsonifier::string_view object{ "Object" };
		static constexpr jsonifier::string_view boolean{ "Bool" };
		static constexpr jsonifier::string_view number{ "Number" };
		static constexpr jsonifier::string_view str{ "String" };
		static constexpr jsonifier::string_view null{ "Null" };
		static constexpr jsonifier::string_view unset{ "Unset" };
		if (isDigitType(charToCheck)) [[likely]] {
			return number;
		} else if (charToCheck == 0x74u || charToCheck == 0x66u) [[likely]] {
			return boolean;
		} else if (charToCheck == 0x7B) [[unlikely]] {
			return object;
		} else if (charToCheck == 0x5B) [[unlikely]] {
			return array;
		} else if (charToCheck == 0x22u) [[unlikely]] {
			return str;
		} else if (charToCheck == 0x6Eu) [[unlikely]] {
			return null;
		} else {
			return unset;
		}
	}

	JSONIFIER_INLINE error_code collectMisReadType(uint8_t c, uint8_t currentValue) {
		if (isTypeType(currentValue) && isTypeType(c)) [[likely]] {
			return error_code::Incorrect_Type;
		} else {
			return error_code::Damaged_Input;
		}
	}

	class error {
	  public:
		friend error createError(error_code errorCode);
		friend class derailleur;

		JSONIFIER_INLINE error() noexcept = default;
		template<jsonifier::concepts::is_fwd_iterator iterator>
		JSONIFIER_INLINE error(const iterator& iter, json_structural_type typeNew, std::source_location locationNew = std::source_location::current()) noexcept {
			intendedValue  = static_cast<uint8_t>(typeNew);
			errorIndex	   = static_cast<uint64_t>(iter.getCurrentStringIndex());
			errorIndexReal = roundDownToMultiple<BitsPerStep>(static_cast<int64_t>(iter.getCurrentStringIndex()));
			if (errorIndexReal < jsonifier::string{}.maxSize()) {
				stringView = iter.getRootPtr();
			}
			stringLength = static_cast<uint64_t>(iter.getEndPtr() - iter.getRootPtr());
			location	 = locationNew;
			if (iter) {
				errorValue = *iter;
			}
			errorType = collectMisReadType(static_cast<uint8_t>(typeNew), errorValue);
		}

		template<jsonifier::concepts::is_fwd_iterator iterator>
		JSONIFIER_INLINE error(const iterator& iter, error_code typeNew, std::source_location locationNew = std::source_location::current()) noexcept {
			errorIndex	   = static_cast<uint64_t>(iter.getCurrentStringIndex());
			errorIndexReal = roundDownToMultiple<BitsPerStep>(static_cast<int64_t>(iter.getCurrentStringIndex()));
			if (errorIndexReal < jsonifier::string{}.maxSize()) {
				stringView = reinterpret_cast<string_view_ptr>(iter.getRootPtr());
			}
			stringLength = static_cast<uint64_t>(iter.getEndPtr() - iter.getRootPtr());
			location	 = locationNew;
			errorType	 = typeNew;
			if (iter) {
				errorValue = *iter;
			}
		}

		template<error_code typeNew, jsonifier::concepts::is_fwd_iterator iterator>
		JSONIFIER_INLINE error(const iterator& iter, std::source_location locationNew = std::source_location::current()) noexcept {
			errorIndex	   = static_cast<uint64_t>(iter.getCurrentStringIndex());
			errorIndexReal = roundDownToMultiple<BitsPerStep>(static_cast<int64_t>(iter.getCurrentStringIndex()));
			if (errorIndexReal < jsonifier::string{}.maxSize()) {
				stringView = iter.getRootPtr();
			}
			stringLength = static_cast<uint64_t>(iter.getEndPtr() - iter.getRootPtr());
			location	 = locationNew;
			errorType	 = typeNew;
			if (iter) {
				errorValue = *iter;
			}
		}

		JSONIFIER_INLINE error& operator=(error_code errorNew) {
			errorType = errorNew;
			return *this;
		}

		JSONIFIER_INLINE error(error_code errorNew) {
			*this = errorNew;
		}
		JSONIFIER_INLINE operator bool() {
			return errorType != error_code::Success;
		}

		JSONIFIER_INLINE bool operator==(const error& rhs) const {
			return errorType == rhs.errorType && errorIndex == rhs.errorIndex && errorValue == rhs.errorValue && intendedValue == rhs.intendedValue;
		}

		jsonifier::string getStringData(const jsonifier::string& errorString) const {
			if (errorIndex >= errorString.size()) {
				return {};
			}

			using value_type	   = std::decay_t<decltype(errorString[0])>;
			const auto start	   = std::begin(errorString) + static_cast<int64_t>(errorIndex);
			const auto rstart	   = std::rbegin(errorString) + static_cast<int64_t>(errorString.size() - errorIndex - 1);
			const auto prevNewLine = std::find((std::min)(rstart + 1, std::rend(errorString)), std::rend(errorString), static_cast<value_type>('\n'));
			const auto column	   = static_cast<int64_t>(std::distance(rstart, prevNewLine));
			const auto nextNewLine = std::find((std::min)(start + 1, std::end(errorString)), std::end(errorString), static_cast<value_type>('\n'));

			const auto offset = static_cast<int64_t>(prevNewLine == std::rend(errorString) ? 0 : errorIndex - column + 1);
			auto contextBegin = std::begin(errorString) + static_cast<int64_t>(offset);
			auto contextEnd	  = nextNewLine;

			int64_t frontTruncation = 0;
			int64_t rearTruncation	= 0;

			if (std::distance(contextBegin, contextEnd) > 64) {
				if (column <= 32) {
					rearTruncation = 64;
					contextEnd	   = contextBegin + static_cast<int64_t>(rearTruncation);
				} else {
					frontTruncation = column - 32;
					contextBegin += frontTruncation;
					if (std::distance(contextBegin, contextEnd) > 64) {
						rearTruncation = frontTruncation + 64;
						contextEnd	   = std::begin(errorString) + offset + rearTruncation;
					}
				}
			}

			return jsonifier::string{ reinterpret_cast<const char*>(&(*contextBegin)),
				static_cast<uint64_t>(reinterpret_cast<const char*>(&(*contextEnd)) - reinterpret_cast<const char*>(&(*contextBegin))) };
		}

		JSONIFIER_INLINE jsonifier::string reportError() const {
			switch (errorType) {
				case error_code::Incorrect_Type: {
					jsonifier::string returnValue{ "It seems you mismatched a value for a value of type: " + getValueType(intendedValue) +
						", the found value was actually: " + getValueType(errorValue) + ", at index: " + jsonifier::toString(errorIndex) + ", in file: " + location.file_name() +
						", at: " + jsonifier::toString(location.line()) + ":" + jsonifier::toString(location.column()) + ", in function: " + location.function_name() + "()." };
					if (stringView) {
						returnValue += "\nHere's some of the string's values:\n" + getStringData(stringView);
					}
					return returnValue;
				}
				case error_code::Damaged_Input: {
					jsonifier::string returnValue{ "Failed to collect a '" + jsonifier::string{ intendedValue } + "', instead found a '" + static_cast<char>(errorValue) + "'" +
						", at index: " + jsonifier::toString(errorIndex) + ", in file: " + location.file_name() + ", at: " + jsonifier::toString(location.line()) + ":" +
						jsonifier::toString(location.column()) + ", in function: " + location.function_name() + "()." };
					if (stringView) {
						returnValue += "\nHere's some of the string's values:\n" + getStringData(stringView);
					}
					return returnValue;
				}
				case error_code::No_Input: {
					return "There was no string being input.";
				}
				case error_code::Success: {
					[[fallthrough]];
				}
				case error_code::Serialize_Error: {
					jsonifier::string returnValue{ "Serialize error - you must provide a resizeable errorString." };
					return returnValue;
				}
				case error_code::Setup_Error:
				case error_code::Missing_Colon:
				case error_code::Missing_Comma_Or_Closing_Brace:
				case error_code::Invalid_Escape_Characters:
				case error_code::Invalid_String_Characters:
				case error_code::Invalid_Null_Value:
				case error_code::Invalid_Bool_Value:
				case error_code::Invalid_Number_Value:
				case error_code::Broken_Array_Start:
				case error_code::Broken_Object_Start:
				case error_code::Requires_Array_Or_Object:
				case error_code::Prettify_Error:
				case error_code::Validate_Error:
				case error_code::Minify_Error:
				default: {
					jsonifier::string returnValue{ "Error of Type: " + errorMap[errorType] + ", at index: " + jsonifier::toString(errorIndex) +
						", in file: " + location.file_name() + ", at: " + jsonifier::toString(location.line()) + ":" + jsonifier::toString(location.column()) +
						", in function: " + location.function_name() + "().\n" };
					if (stringView) {
						returnValue += "\nHere's some of the string's values:\n" + getStringData(stringView);
					}
					return returnValue;
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

	template<error_code typeNew, jsonifier::concepts::is_fwd_iterator iterator>
	JSONIFIER_INLINE error createError(const iterator& iter, std::source_location locationNew = std::source_location::current()) {
		error newError{ iter, typeNew, locationNew };
		return newError;
	}

	JSONIFIER_INLINE error createError(error_code errorCode) {
		error newError{};
		newError.errorType = errorCode;
		return newError;
	}

	template<json_structural_type typeNew, jsonifier::concepts::is_fwd_iterator iterator>
	JSONIFIER_INLINE error createError(const iterator& iter, std::source_location locationNew = std::source_location::current()) {
		error newError{ iter, typeNew, locationNew };
		return newError;
	}

	JSONIFIER_INLINE std::ostream& operator<<(std::ostream& os, const error& errorNew) {
		os << errorNew.reportError();
		return os;
	}

}
