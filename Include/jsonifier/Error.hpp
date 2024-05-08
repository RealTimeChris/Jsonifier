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

#include <jsonifier/StringView.hpp>
#include <jsonifier/Simd.hpp>
#include <source_location>
#include <unordered_map>
#include <exception>
#include <algorithm>
#include <string>

namespace jsonifier_internal {

	enum class error_classes : uint8_t {
		Unset		= 0,
		Parsing		= 1,
		Serializing = 2,
		Minifying	= 3,
		Prettifying = 4,
		Validating	= 5,
	};

	enum class parse_errors {
		Success						= 0,
		Missing_Object_Start		= 1,
		Missing_Object_End			= 2,
		Missing_Array_Start			= 3,
		Missing_Array_End			= 4,
		Missing_String_Start		= 5,
		Missing_Colon				= 6,
		Missing_Comma_Or_Object_End = 7,
		Missing_Comma_Or_Array_End	= 8,
		Missing_Comma				= 9,
		Invalid_Number_Value		= 10,
		Invalid_Null_Value			= 11,
		Invalid_Bool_Value			= 12,
		Invalid_String_Characters	= 13,
		No_Input					= 14,
	};

	inline std::unordered_map<error_classes, std::unordered_map<uint64_t, jsonifier::string_view>> errorMap{
		{ error_classes::Parsing,
			std::unordered_map<uint64_t, jsonifier::string_view>{ { { 0ull, "Success" }, { 1ull, "Missing_Object_Start" }, { 2ull, "Missing_Object_End" },
				{ 3ull, "Missing_Array_Start" }, { 4ull, "Missing_Array_End" }, { 5ull, "Missing_String_Start" }, { 6ull, "Missing_Colon" },
				{ 7ull, "Missing_Comma_Or_Object_End" }, { 8ull, "Missing_Comma_Or_Array_End" }, { 9ull, "Missing_Comma" }, { 10ull, "Invalid_Number_Value" },
				{ 11ull, "Invalid_Null_Value" }, { 12ull, "Invalid_Bool_Value" }, { 13ull, "Invalid_String_Characters" }, { 14ull, "No_Input" } } } },
		{ error_classes::Serializing, std::unordered_map<uint64_t, jsonifier::string_view>{ { { 0ull, "Success" } } } },
		{ error_classes::Minifying,
			std::unordered_map<uint64_t, jsonifier::string_view>{ { 0ull, "Success" }, { 1ull, "No_Input" }, { 2ull, "Invalid_String_Length" }, { 3ull, "Invalid_Number_Value" },
				{ 4ull, "Incorrect_Structural_Index" } } },
		{ error_classes::Prettifying,
			std::unordered_map<uint64_t, jsonifier::string_view>{ { 0ull, "Success" }, { 1ull, "No_Input" }, { 2ull, "Exceeded_Max_Depth" },
				{ 3ull, "Incorrect_Structural_Index" } } },
		{ error_classes::Validating,
			std::unordered_map<uint64_t, jsonifier::string_view>{ { 0ull, "Success" }, { 1ull, "Missing_Object_Start" }, { 2ull, "Missing_Object_End" },
				{ 3ull, "Missing_Array_Start" }, { 4ull, "Missing_Array_End" }, { 5ull, "Missing_String_Start" }, { 6ull, "Missing_Colon" }, { 7ull, "Missing_Comma" },
				{ 8ull, "Invalid_Number_Value" }, { 9ull, "Invalid_Null_Value" }, { 10ull, "Invalid_Bool_Value" }, { 11ull, "Invalid_String_Characters" },
				{ 12ull, "Invalid_Escape_Characters" }, { 13ull, "Missing_Comma_Or_Closing_Brace" }, { 14ull, "No_Input" } } }
	};

	enum class json_structural_type : int8_t {
		Unset		 = 0,
		String		 = '"',
		Comma		 = ',',
		Number		 = '-',
		Colon		 = ':',
		Array_Start	 = '[',
		Array_End	 = ']',
		Null		 = 'n',
		Bool		 = 't',
		Object_Start = '{',
		Object_End	 = '}',
		Error		 = -1,
		Type_Count	 = 12
	};

	constexpr std::array<bool, 256> numberTable{ [] {
		std::array<bool, 256> returnValues{};
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

	JSONIFIER_INLINE bool isNumberType(uint8_t c) {
		return numberTable[c];
	}

	constexpr std::array<bool, 256> boolTable{ [] {
		std::array<bool, 256> returnValues{};
		returnValues['t'] = true;
		returnValues['f'] = true;
		return returnValues;
	}() };

	class error {
	  public:
		JSONIFIER_INLINE error(std::source_location sourceLocation, error_classes errorClassNew, int64_t errorIndexNew, int64_t stringLengthNew, const char* stringViewNew,
			uint64_t typeNew) noexcept {
			stringLength = static_cast<uint64_t>(stringLengthNew);
			errorIndex	 = static_cast<uint64_t>(errorIndexNew);
			location	 = sourceLocation;
			stringView	 = stringViewNew;
			errorClass	 = errorClassNew;
			errorType	 = typeNew;
			if (stringView) {
				formatError(stringView);
			}
		}

		template<const std::source_location& sourceLocation, error_classes errorClassNew, auto typeNew>
		JSONIFIER_INLINE static error constructError(int64_t errorIndexNew, int64_t stringLengthNew, const char* stringViewNew) {
			return { sourceLocation, errorClassNew, errorIndexNew, stringLengthNew, stringViewNew, static_cast<uint64_t>(typeNew) };
		}

		JSONIFIER_INLINE operator bool() {
			return errorType != 0;
		}

		JSONIFIER_INLINE bool operator==(const error& rhs) const {
			return errorType == rhs.errorType && errorIndex == rhs.errorIndex;
		}

		JSONIFIER_INLINE void formatError(const jsonifier::string_view& errorString) {
			if (errorIndex >= errorString.size() || errorString.size() == 0) {
				return;
			}

			using V = jsonifier::concepts::unwrap_t<decltype(errorString[0])>;

			const auto start	   = std::begin(errorString) + static_cast<int64_t>(errorIndex);
			line				   = static_cast<uint64_t>(std::count(std::begin(errorString), start, static_cast<V>('\n')) + 1ll);
			const auto rstart	   = std::rbegin(errorString) + static_cast<int64_t>(errorString.size()) - static_cast<int64_t>(errorIndex) - 1ll;
			const auto prevNewLine = std::find(std::min(rstart, std::rend(errorString)), std::rend(errorString), static_cast<V>('\n'));
			localIndex			   = std::distance(rstart, prevNewLine);
			const auto nextNewLine = std::find(std::min(start + 1, std::end(errorString)), std::end(errorString), static_cast<V>('\n'));

			const auto offset = (prevNewLine == std::rend(errorString) ? 0ll : static_cast<int64_t>(errorIndex) - static_cast<int64_t>(localIndex) + 1ll);
			auto contextBegin = std::begin(errorString) + offset;
			auto contextEnd	  = nextNewLine;

			int64_t frontTruncation = 0;
			int64_t rearTruncation	= 0;

			if (std::distance(contextBegin, contextEnd) > 64) {
				if (localIndex <= 32) {
					rearTruncation = 64;
					contextEnd	   = contextBegin + rearTruncation;
				} else {
					frontTruncation = localIndex;
					contextBegin += frontTruncation;
					if (std::distance(contextBegin, contextEnd) > 64) {
						rearTruncation = frontTruncation + 64;
						contextEnd	   = std::begin(errorString) + offset + rearTruncation;
					}
				}
			}

			context = jsonifier::string{ contextBegin, static_cast<uint64_t>(contextEnd - contextBegin) };
		}

		JSONIFIER_INLINE jsonifier::string reportError() const {
			jsonifier::string returnValue{ "Error of Type: " + errorMap[errorClass][errorType] + ", at global index: " + std::to_string(errorIndex) +
				", on line: " + std::to_string(line) + ", at local index: " + std::to_string(localIndex) };
			if (stringView) {
				returnValue += "\nHere's some of the string's values:\n" + context;
			}
			returnValue += jsonifier::string{ "\nIn file: " } + location.file_name() + ", at: " + std::to_string(location.line()) + ":" + std::to_string(location.column()) +
				", in function: " + location.function_name() + "().\n";
			return returnValue;
		}

	  protected:
		std::source_location location{};
		string_view_ptr stringView{};
		jsonifier::string context{};
		error_classes errorClass{};
		uint64_t stringLength{};
		uint64_t errorIndex{};
		int64_t localIndex{};
		uint64_t errorType{};
		uint64_t line{};
	};

	JSONIFIER_INLINE std::ostream& operator<<(std::ostream& os, const error& errorNew) {
		os << errorNew.reportError();
		return os;
	}

}