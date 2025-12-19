/*
	MIT License

	Copyright (c) 2024 RealTimeChris

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

#include <jsonifier/Utilities/StringView.hpp>
#include <jsonifier/Utilities/Simd.hpp>
#include <source_location>
#include <unordered_map>
#include <exception>
#include <algorithm>
#include <string>

namespace jsonifier::internal {

	enum class error_classes : uint8_t {
		Unset		= 0,
		Parsing		= 1,
		Serializing = 2,
		Minifying	= 3,
		Prettifying = 4,
		Validating	= 5,
	};

	enum class parse_errors : uint32_t {
		Success					  = 0,
		Missing_Object_Start	  = 1,
		Imbalanced_Object_Braces  = 2,
		Missing_Array_Start		  = 3,
		Imbalanced_Array_Brackets = 4,
		Missing_String_Start	  = 5,
		Missing_Colon			  = 6,
		Missing_Object_End		  = 7,
		Missing_Array_End		  = 8,
		Missing_Comma			  = 9,
		Invalid_Number_Value	  = 10,
		Invalid_Null_Value		  = 11,
		Invalid_Bool_Value		  = 12,
		Invalid_String_Characters = 13,
		No_Input				  = 14,
		Unfinished_Input		  = 15,
		Unexpected_String_End	  = 16,
	};

	enum class serialize_errors { Success = 0 };

	enum class minify_errors {
		Success					   = 0,
		No_Input				   = 1,
		Invalid_String_Length	   = 2,
		Invalid_Number_Value	   = 3,
		Incorrect_Structural_Index = 4,
	};

	enum class prettify_errors {
		Success					   = 0,
		No_Input				   = 1,
		Exceeded_Max_Depth		   = 2,
		Incorrect_Structural_Index = 3,
	};

	enum class validate_errors {
		Success						   = 0,
		Missing_Object_Start		   = 1,
		Imbalanced_Object_Braces	   = 2,
		Missing_Array_Start			   = 3,
		Imbalanced_Array_Brackets	   = 4,
		Missing_String_Start		   = 5,
		Missing_Colon				   = 6,
		Missing_Comma				   = 7,
		Invalid_Number_Value		   = 8,
		Invalid_Null_Value			   = 9,
		Invalid_Bool_Value			   = 10,
		Invalid_String_Characters	   = 11,
		Invalid_Escape_Characters	   = 12,
		Missing_Comma_Or_Closing_Brace = 13,
		No_Input					   = 14,
	};

	std::ostream& operator<<(std::ostream& os, parse_errors error) {
		os << simd::tzcnt(static_cast<uint64_t>(error));
		return os;
	}

	inline const std::unordered_map<error_classes, std::unordered_map<uint64_t, string_view>> errorMap{
		{ error_classes::Parsing,
			std::unordered_map<uint64_t, string_view>{
				{ static_cast<uint64_t>(parse_errors::Success), "Success" },
				{ static_cast<uint64_t>(parse_errors::Missing_Object_Start), "Missing_Object_Start" },
				{ static_cast<uint64_t>(parse_errors::Imbalanced_Object_Braces), "Imbalanced_Object_Braces" },
				{ static_cast<uint64_t>(parse_errors::Missing_Array_Start), "Missing_Array_Start" },
				{ static_cast<uint64_t>(parse_errors::Imbalanced_Array_Brackets), "Imbalanced_Array_Brackets" },
				{ static_cast<uint64_t>(parse_errors::Missing_String_Start), "Missing_String_Start" },
				{ static_cast<uint64_t>(parse_errors::Missing_Colon), "Missing_Colon" },
				{ static_cast<uint64_t>(parse_errors::Missing_Object_End), "Missing_Object_End" },
				{ static_cast<uint64_t>(parse_errors::Missing_Array_End), "Missing_Array_End" },
				{ static_cast<uint64_t>(parse_errors::Missing_Comma), "Missing_Comma" },
				{ static_cast<uint64_t>(parse_errors::Invalid_Number_Value), "Invalid_Number_Value" },
				{ static_cast<uint64_t>(parse_errors::Invalid_Null_Value), "Invalid_Null_Value" },
				{ static_cast<uint64_t>(parse_errors::Invalid_Bool_Value), "Invalid_Bool_Value" },
				{ static_cast<uint64_t>(parse_errors::Invalid_String_Characters), "Invalid_String_Characters" },
				{ static_cast<uint64_t>(parse_errors::No_Input), "No_Input" },
				{ static_cast<uint64_t>(parse_errors::Unfinished_Input), "Unfinished_Input" },
				{ static_cast<uint64_t>(parse_errors::Unexpected_String_End), "Unexpected_String_End" },
			} },
		{ error_classes::Serializing,
			std::unordered_map<uint64_t, string_view>{
				{ static_cast<uint64_t>(serialize_errors::Success), "Success" },
			} },
		{ error_classes::Minifying,
			std::unordered_map<uint64_t, string_view>{
				{ static_cast<uint64_t>(minify_errors::Success), "Success" },
				{ static_cast<uint64_t>(minify_errors::No_Input), "No_Input" },
				{ static_cast<uint64_t>(minify_errors::Invalid_String_Length), "Invalid_String_Length" },
				{ static_cast<uint64_t>(minify_errors::Invalid_Number_Value), "Invalid_Number_Value" },
				{ static_cast<uint64_t>(minify_errors::Incorrect_Structural_Index), "Incorrect_Structural_Index" },
			} },
		{ error_classes::Prettifying,
			std::unordered_map<uint64_t, string_view>{
				{ static_cast<uint64_t>(prettify_errors::Success), "Success" },
				{ static_cast<uint64_t>(prettify_errors::No_Input), "No_Input" },
				{ static_cast<uint64_t>(prettify_errors::Exceeded_Max_Depth), "Exceeded_Max_Depth" },
				{ static_cast<uint64_t>(prettify_errors::Incorrect_Structural_Index), "Incorrect_Structural_Index" },
			} },
		{ error_classes::Validating,
			std::unordered_map<uint64_t, string_view>{
				{ static_cast<uint64_t>(validate_errors::Success), "Success" },
				{ static_cast<uint64_t>(validate_errors::Missing_Object_Start), "Missing_Object_Start" },
				{ static_cast<uint64_t>(validate_errors::Imbalanced_Object_Braces), "Imbalanced_Object_Braces" },
				{ static_cast<uint64_t>(validate_errors::Missing_Array_Start), "Missing_Array_Start" },
				{ static_cast<uint64_t>(validate_errors::Imbalanced_Array_Brackets), "Imbalanced_Array_Brackets" },
				{ static_cast<uint64_t>(validate_errors::Missing_String_Start), "Missing_String_Start" },
				{ static_cast<uint64_t>(validate_errors::Missing_Colon), "Missing_Colon" },
				{ static_cast<uint64_t>(validate_errors::Missing_Comma), "Missing_Comma" },
				{ static_cast<uint64_t>(validate_errors::Invalid_Number_Value), "Invalid_Number_Value" },
				{ static_cast<uint64_t>(validate_errors::Invalid_Null_Value), "Invalid_Null_Value" },
				{ static_cast<uint64_t>(validate_errors::Invalid_Bool_Value), "Invalid_Bool_Value" },
				{ static_cast<uint64_t>(validate_errors::Invalid_String_Characters), "Invalid_String_Characters" },
				{ static_cast<uint64_t>(validate_errors::Invalid_Escape_Characters), "Invalid_Escape_Characters" },
				{ static_cast<uint64_t>(validate_errors::Missing_Comma_Or_Closing_Brace), "Missing_Comma_Or_Closing_Brace" },
				{ static_cast<uint64_t>(validate_errors::No_Input), "No_Input" },
			} },
	};

	enum class json_structural_type : int8_t {
		unset		 = 0,
		string		 = '"',
		comma		 = ',',
		number		 = '-',
		colon		 = ':',
		array_start	 = '[',
		array_end	 = ']',
		null		 = 'n',
		boolean		 = 't',
		object_start = '{',
		object_end	 = '}',
		error		 = -1,
		type_count	 = 12
	};

	inline constexpr array<bool, 256> numberTable{ []() constexpr {
		array<bool, 256> returnValues{};
		returnValues['-'] = true;
		returnValues['0'] = true;
		returnValues['1'] = true;
		returnValues['2'] = true;
		returnValues['3'] = true;
		returnValues['4'] = true;
		returnValues['5'] = true;
		returnValues['6'] = true;
		returnValues['7'] = true;
		returnValues['8'] = true;
		returnValues['9'] = true;
		return returnValues;
	}() };

	static string convertChar(char value) {
		switch (value) {
			[[unlikely]] case '\b': { return R"(\b)"; }
			[[unlikely]] case '\t': { return R"(\t)"; }
			[[unlikely]] case '\n': { return R"(\n)"; }
			[[unlikely]] case '\f': { return R"(\f)"; }
			[[unlikely]] case '\r': { return R"(\r)"; }
			[[unlikely]] case '"': { return R"(\")"; }
			[[unlikely]] case '\\': { return R"(\\)"; }
			[[unlikely]] case '\0': { return R"(\0)"; }
			[[likely]] default: { return string{ value }; };
		}
	}

	inline constexpr array<bool, 256> boolTable{ []() constexpr {
		array<bool, 256> returnValues{};
		returnValues['t'] = true;
		returnValues['f'] = true;
		return returnValues;
	}() };

	class error {
	  public:
		error(std::source_location sourceLocation, error_classes errorClassNew, int64_t errorIndexNew, int64_t stringLengthNew, string_view_ptr stringViewNew,
			uint64_t typeNew) noexcept {
			stringLength = static_cast<uint64_t>(stringLengthNew);
			location	 = sourceLocation;
			errorIndex	 = errorIndexNew;
			stringView	 = stringViewNew;
			errorClass	 = errorClassNew;
			errorType	 = typeNew;
			if (stringView) {
				formatError(stringView);
			}
		}

		template<error_classes errorClassNew, auto typeNew> static error constructError(int64_t errorIndexNew, int64_t stringLengthNew, string_view_ptr stringViewNew,
			const std::source_location& sourceLocation = std::source_location::current()) noexcept {
			return { sourceLocation, errorClassNew, errorIndexNew, stringLengthNew, stringViewNew, static_cast<uint64_t>(typeNew) };
		}

		operator uint64_t() const noexcept {
			return errorType;
		}

		operator parse_errors() const noexcept {
			return static_cast<parse_errors>(errorType);
		}

		operator bool() const noexcept {
			return errorType != 0;
		}

		bool operator==(const error& rhs) const noexcept {
			return errorType == rhs.errorType && errorIndex == rhs.errorIndex;
		}

		void formatError(const string_view& errorString) noexcept {
			if (static_cast<size_t>(errorIndex) >= errorString.size() || errorString.size() == 0ull) {
				return;
			}

			using V = std::decay_t<decltype(errorString[0])>;

			const auto start	   = std::begin(errorString) + errorIndex;
			line				   = size_t(std::count(std::begin(errorString), start, static_cast<V>('\n')) + 1);
			const auto rstart	   = std::rbegin(errorString) + static_cast<int64_t>(errorString.size()) - errorIndex - 1ll;
			const auto prevNewLine = std::find((std::min)(rstart + 1, std::rend(errorString)), std::rend(errorString), static_cast<V>('\n'));
			localIndex			   = std::distance(rstart, prevNewLine) - 1ll;
			auto endIndex{ std::end(errorString) - start >= 64 ? 64 : std::end(errorString) - start };
			context = string{ start, static_cast<size_t>(endIndex) };
			for (auto& c: context) {
				if (c == '\t') {
					c = ' ';
				}
			}
		}

		string collectValues(string outputValues, const string& inputValues, size_t currentIndex = 0) const {
			if (inputValues.size() > currentIndex && currentIndex <= 6) {
				outputValues += string{ "'" } + convertChar(inputValues[currentIndex]) + string{ "' " };
				return collectValues(outputValues, inputValues, currentIndex + 1);
			} else {
				return outputValues;
			}
		}

		string reportError() const noexcept {
			string returnValue{ "Error of Type: " + errorMap.at(errorClass).at(errorType) + ", at global index: " + std::to_string(errorIndex) + ", on line: " + std::to_string(line) +
				", at local index: " + std::to_string(localIndex) };
			if (stringView) {
				returnValue += "\nHere's some of the string's values: " + collectValues(string{}, context) + string{ "\nThe Values: " + context };
			}
			returnValue += string{ "\nIn file: " } + location.file_name() + ", at: " + std::to_string(location.line()) + ":" + std::to_string(location.column()) +
				", in function: " + location.function_name() + "().\n";
			return returnValue;
		}

	  protected:
		std::source_location location{};
		string_view_ptr stringView{};
		error_classes errorClass{};
		uint64_t stringLength{};
		int64_t errorIndex{};
		int64_t localIndex{};
		uint64_t errorType{};
		string context{};
		uint64_t line{};
	};

	std::ostream& operator<<(std::ostream& os, const error& errorNew) noexcept {
		os << errorNew.reportError();
		return os;
	}

}