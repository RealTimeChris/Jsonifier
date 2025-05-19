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

	JSONIFIER_INLINE std::ostream& operator<<(std::ostream& os, parse_errors error) {
		os << simd_internal::tzcnt(static_cast<uint64_t>(error));
		return os;
	}

	inline std::unordered_map<error_classes, std::unordered_map<uint64_t, jsonifier::string_view>> errorMap{
		{ error_classes::Parsing,
			std::unordered_map<uint64_t, jsonifier::string_view>{ { 0ull, "Success" }, { 1ull, "Missing_Object_Start" }, { 2ull, "Imbalanced_Object_Braces" },
				{ 3ull, "Missing_Array_Start" }, { 4ull, "Imbalanced_Array_Brackets" }, { 5ull, "Missing_String_Start" }, { 6ull, "Missing_Colon" }, { 7ull, "Missing_Object_End" },
				{ 8ull, "Missing_Array_End" }, { 9ull, "Missing_Comma" }, { 10ull, "Invalid_Number_Value" }, { 11ull, "Invalid_Null_Value" }, { 12ull, "Invalid_Bool_Value" },
				{ 13ull, "Invalid_String_Characters" }, { 14ull, "No_Input" }, { 15ull, "Unfinished_Input" }, { 16ull, "Unexpected_String_End" } } },
		{ error_classes::Serializing, std::unordered_map<uint64_t, jsonifier::string_view>{ { { 0ull, "Success" } } } },
		{ error_classes::Minifying,
			std::unordered_map<uint64_t, jsonifier::string_view>{ { 0ull, "Success" }, { 1ull, "No_Input" }, { 2ull, "Invalid_String_Length" }, { 3ull, "Invalid_Number_Value" },
				{ 4ull, "Incorrect_Structural_Index" } } },
		{ error_classes::Prettifying,
			std::unordered_map<uint64_t, jsonifier::string_view>{ { 0ull, "Success" }, { 1ull, "No_Input" }, { 2ull, "Exceeded_Max_Depth" },
				{ 3ull, "Incorrect_Structural_Index" } } },
		{ error_classes::Validating,
			std::unordered_map<uint64_t, jsonifier::string_view>{ { 0ull, "Success" }, { 1ull, "Missing_Object_Start" }, { 2ull, "Imbalanced_Object_Braces" },
				{ 3ull, "Missing_Array_Start" }, { 4ull, "Imbalanced_Array_Brackets" }, { 5ull, "Missing_String_Start" }, { 6ull, "Missing_Colon" }, { 7ull, "Missing_Comma" },
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

	constexpr array<bool, 256> numberTable{ [] {
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

	JSONIFIER_INLINE bool isNumberType(uint8_t c) noexcept {
		return numberTable[c];
	}

	JSONIFIER_INLINE std::string convertChar(char value) {
		switch (value) {
			[[unlikely]] case '\b': { return R"(\b)"; }
			[[unlikely]] case '\t': { return R"(\t)"; }
			[[unlikely]] case '\n': { return R"(\n)"; }
			[[unlikely]] case '\f': { return R"(\f)"; }
			[[unlikely]] case '\r': { return R"(\r)"; }
			[[unlikely]] case '"': { return R"(\")"; }
			[[unlikely]] case '\\': { return R"(\\)"; }
			[[unlikely]] case '\0': { return R"(\0)"; }
			[[likely]] default: { return std::string{ value }; };
		}
	}

	constexpr array<bool, 256> boolTable{ [] {
		array<bool, 256> returnValues{};
		returnValues['t'] = true;
		returnValues['f'] = true;
		return returnValues;
	}() };

	class error {
	  public:
		JSONIFIER_INLINE error(std::source_location sourceLocation, error_classes errorClassNew, int64_t errorIndexNew, int64_t stringLengthNew, string_view_ptr stringViewNew,
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

		template<error_classes errorClassNew, auto typeNew> JSONIFIER_INLINE static error constructError(int64_t errorIndexNew, int64_t stringLengthNew,
			string_view_ptr stringViewNew, const std::source_location& sourceLocation = std::source_location::current()) noexcept {
			return { sourceLocation, errorClassNew, errorIndexNew, stringLengthNew, stringViewNew, static_cast<uint64_t>(typeNew) };
		}

		JSONIFIER_INLINE operator uint64_t() const noexcept {
			return errorType;
		}

		JSONIFIER_INLINE operator parse_errors() const noexcept {
			return static_cast<parse_errors>(errorType);
		}

		JSONIFIER_INLINE operator bool() const noexcept {
			return errorType != 0;
		}

		JSONIFIER_INLINE bool operator==(const error& rhs) const noexcept {
			return errorType == rhs.errorType && errorIndex == rhs.errorIndex;
		}

		JSONIFIER_INLINE void formatError(const jsonifier::string_view& errorString) noexcept {
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
			context = jsonifier::string{ start, static_cast<size_t>(endIndex) };
			for (auto& c: context) {
				if (c == '\t') {
					c = ' ';
				}
			}
		}

		JSONIFIER_INLINE jsonifier::string collectValues(jsonifier::string outputValues, const jsonifier::string& inputValues, size_t currentIndex = 0) const {
			if (inputValues.size() > currentIndex && currentIndex <= 6) {
				outputValues += jsonifier::string{ "'" } + convertChar(inputValues[currentIndex]) + jsonifier::string{ "' " };
				return collectValues(outputValues, inputValues, currentIndex + 1);
			} else {
				return outputValues;
			}
		}

		JSONIFIER_INLINE jsonifier::string reportError() const noexcept {
			jsonifier::string returnValue{ "Error of Type: " + errorMap[errorClass][errorType] + ", at global index: " + std::to_string(errorIndex) +
				", on line: " + std::to_string(line) + ", at local index: " + std::to_string(localIndex) };
			if (stringView) {
				returnValue += "\nHere's some of the string's values: " + collectValues(jsonifier::string{}, context) + std::string{ "\nThe Values: " + context };
			}
			returnValue += jsonifier::string{ "\nIn file: " } + location.file_name() + ", at: " + std::to_string(location.line()) + ":" + std::to_string(location.column()) +
				", in function: " + location.function_name() + "().\n";
			return returnValue;
		}

	  protected:
		std::source_location location{};
		jsonifier::string context{};
		error_classes errorClass{};
		uint64_t stringLength{};
		int64_t errorIndex{};
		int64_t localIndex{};
		uint64_t errorType{};
		uint64_t line{};
		string_view_ptr stringView{};
		uint8_t padding01[7]{};
	};

	JSONIFIER_INLINE std::ostream& operator<<(std::ostream& os, const error& errorNew) noexcept {
		os << errorNew.reportError();
		return os;
	}

}