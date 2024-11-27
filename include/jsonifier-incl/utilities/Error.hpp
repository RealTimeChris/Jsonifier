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

#include <jsonifier-incl/utilities/StringView.hpp>
#include <jsonifier-incl/utilities/Simd.hpp>
#include <source_location>
#include <unordered_map>
#include <exception>
#include <algorithm>
#include <string>

namespace jsonifier::internal {

	enum class status_classes : uint8_t {
		Unset		= 0,
		Parsing		= 1,
		Serializing = 2,
		Minifying	= 3,
		Prettifying = 4,
		Validating	= 5,
	};

	enum class parse_status : uint32_t {
		Success					  = 0,
		Missing_Object_Start	  = 1,
		Missing_Object_End		  = 2,
		Missing_Array_Start		  = 3,
		Missing_Array_End		  = 4,
		Missing_String_Start	  = 5,
		Missing_Colon			  = 6,
		Missing_Comma			  = 7,
		Invalid_Number_Value	  = 8,
		Invalid_Null_Value		  = 9,
		Invalid_Bool_Value		  = 10,
		Invalid_String_Characters = 11,
		No_Input				  = 12,
		Unfinished_Input		  = 13,
		Unexpected_String_End	  = 14,
		Count					  = 15,
	};

	enum class serialize_status : uint32_t {
		Success = 0,
		Count	= 1,
	};

	enum class minify_status : uint32_t {
		Success					   = 0,
		No_Input				   = 1,
		Invalid_String_Length	   = 2,
		Invalid_Number_Value	   = 3,
		Incorrect_Structural_Index = 4,
		Count					   = 5,
	};

	enum class prettify_status : uint32_t {
		Success					   = 0,
		No_Input				   = 1,
		Exceeded_Max_Depth		   = 2,
		Incorrect_Structural_Index = 3,
		Count					   = 4,
	};

	enum class validate_status : uint32_t {
		Success						   = 0,
		Missing_Object_Start		   = 1,
		Missing_Object_End			   = 2,
		Missing_Array_Start			   = 3,
		Missing_Array_End			   = 4,
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
		Count						   = 15,
	};

	static constexpr array<const char*, static_cast<size_t>(parse_status::Count)> parseStatusNames{ { "Success", "Missing_Object_Start", "Missing_Object_End",
		"Missing_Array_Start", "Missing_Array_End", "Missing_String_Start", "Missing_Colon", "Missing_Comma", "Invalid_Number_Value", "Invalid_Null_Value", "Invalid_Bool_Value",
		"Invalid_String_Characters", "No_Input", "Unfinished_Input", "Unexpected_String_End" } };

	static constexpr array<const char*, static_cast<size_t>(serialize_status::Count)> serializeErrorNames{ { "Success" } };

	static constexpr array<const char*, static_cast<size_t>(minify_status::Count)> minifyErrorNames{ { "Success", "No_Input", "Invalid_String_Length", "Invalid_Number_Value",
		"Incorrect_Structural_Index" } };

	static constexpr array<const char*, static_cast<size_t>(prettify_status::Count)> prettifyErrorNames{ { "Success", "No_Input", "Exceeded_Max_Depth",
		"Incorrect_Structural_Index" } };

	static constexpr array<const char*, static_cast<size_t>(validate_status::Count)> validateErrorNames{ { "Success", "Missing_Object_Start", "Missing_Object_End",
		"Missing_Array_Start", "Missing_Array_End", "Missing_String_Start", "Missing_Colon", "Missing_Comma", "Invalid_Number_Value", "Invalid_Null_Value", "Invalid_Bool_Value",
		"Invalid_String_Characters", "Invalid_Escape_Characters", "Missing_Comma_Or_Closing_Brace", "No_Input" } };

	template<typename error_type> constexpr const char* errorName(status_classes cls, error_type code_new) noexcept {
		uint64_t code{ static_cast<uint64_t>(code_new) };
		switch (static_cast<uint64_t>(cls)) {
			case static_cast<uint64_t>(status_classes::Parsing):
				return code < parseStatusNames.size() ? parseStatusNames[code] : "Unknown_Parse_Status";
			case static_cast<uint64_t>(status_classes::Serializing):
				return code < serializeErrorNames.size() ? serializeErrorNames[code] : "Unknown_Serialize_Status";
			case static_cast<uint64_t>(status_classes::Minifying):
				return code < minifyErrorNames.size() ? minifyErrorNames[code] : "Unknown_Minify_Status";
			case static_cast<uint64_t>(status_classes::Prettifying):
				return code < prettifyErrorNames.size() ? prettifyErrorNames[code] : "Unknown_Prettify_Status";
			case static_cast<uint64_t>(status_classes::Validating):
				return code < validateErrorNames.size() ? validateErrorNames[code] : "Unknown_Validate_Status";
			case static_cast<uint64_t>(status_classes::Unset):
				[[fallthrough]];
			default:
				return "Unset";
		}
	}

	inline static std::ostream& operator<<(std::ostream& os, parse_status error) {
		os << errorName(status_classes::Parsing, error);
		return os;
	}

	inline static std::ostream& operator<<(std::ostream& os, serialize_status error) {
		os << errorName(status_classes::Serializing, error);
		return os;
	}

	inline static std::ostream& operator<<(std::ostream& os, validate_status error) {
		os << errorName(status_classes::Validating, error);
		return os;
	}

	inline static std::ostream& operator<<(std::ostream& os, minify_status error) {
		os << errorName(status_classes::Minifying, error);
		return os;
	}

	inline static std::ostream& operator<<(std::ostream& os, prettify_status error) {
		os << errorName(status_classes::Prettifying, error);
		return os;
	}

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
			[[likely]] default: { return string{ value }; }
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
		error(std::source_location sourceLocation, status_classes errorClassNew, int64_t errorIndexNew, int64_t stringLengthNew, string_view_ptr stringViewNew,
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

		template<status_classes errorClassNew, auto typeNew> static error constructError(int64_t errorIndexNew, int64_t stringLengthNew, string_view_ptr stringViewNew,
			const std::source_location& sourceLocation = std::source_location::current()) noexcept {
			return { sourceLocation, errorClassNew, errorIndexNew, stringLengthNew, stringViewNew, static_cast<uint64_t>(typeNew) };
		}

		operator uint64_t() const noexcept {
			return errorType;
		}

		operator parse_status() const noexcept {
			return static_cast<parse_status>(errorType);
		}

		operator bool() const noexcept {
			return errorType != 0;
		}

		bool operator==(const error& rhs) const noexcept {
			return errorType == rhs.errorType && errorIndex == rhs.errorIndex;
		}

		void formatError(string_view_ptr errorString) noexcept {
			if (stringLength == 0ull) {
				return;
			}
			int64_t reportIndex = errorIndex;
			if (static_cast<uint64_t>(reportIndex) >= stringLength) {
				reportIndex = static_cast<int64_t>(stringLength) - 1;
			}
			string_view view{ errorString, stringLength };
			using V				   = std::decay_t<decltype(view[0])>;
			const auto start	   = std::begin(view) + reportIndex;
			line				   = uint64_t(std::count(std::begin(view), start, static_cast<V>('\n')) + 1);
			const auto rstart	   = std::rbegin(view) + static_cast<int64_t>(view.size()) - reportIndex - 1ll;
			const auto prevNewLine = std::find(( std::min )(rstart + 1, std::rend(view)), std::rend(view), static_cast<V>('\n'));
			localIndex			   = std::distance(rstart, prevNewLine) - 1ll;
			auto endIndex{ std::end(view) - start >= 64 ? 64 : std::end(view) - start };
			context = string{ start, static_cast<uint64_t>(endIndex) };
			for (auto& c: context) {
				if (c == '\t') {
					c = ' ';
				}
			}
		}

		string collectValues(const string& inputValues) const {
			string output{};
			for (uint64_t i = 0; i < 32 && i < inputValues.size(); ++i) {
				output += string{ "'" } + convertChar(inputValues[i]) + string{ "' " };
			}
			return output;
		}

		string reportError() const noexcept {
			string returnValue{ string{ "Error of Type: " } + errorName(errorClass, errorType) + ", at global index: " + std::to_string(errorIndex) +
				", on line: " + std::to_string(line) + ", at local index: " + std::to_string(localIndex) };
			if (stringView) {
				returnValue += "\nHere's some of the string's values: " + collectValues(context) + string{ "\nThe Values: " + context };
			}
			returnValue += string{ "\nIn file: " } + location.file_name() + ", at: " + std::to_string(location.line()) + ":" + std::to_string(location.column()) +
				", in function: " + location.function_name() + "().\n";
			return returnValue;
		}

	  protected:
		std::source_location location{};
		string_view_ptr stringView{};
		status_classes errorClass{};
		uint64_t stringLength{};
		int64_t errorIndex{};
		int64_t localIndex{};
		uint64_t errorType{};
		string context{};
		uint64_t line{};
	};

	inline static std::ostream& operator<<(std::ostream& os, const error& errorNew) noexcept {
		os << errorNew.reportError();
		return os;
	}

}
