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

	inline std::unordered_map<error_classes, std::unordered_map<uint64_t, jsonifier::string_view>> errorMap{
		{ error_classes::Parsing,
			std::unordered_map<uint64_t, jsonifier::string_view>{
				{ { 0ull, "Success" }, { 1ull, "Missing_Object_Start" }, { 2ull, "Missing_Object_End" }, { 3ull, "Missing_Array_Start" }, { 4ull, "Missing_String_Start" },
					{ 5ull, "Missing_Colon" }, { 6ull, "Missing_Comma_Or_Object_End" }, { 7ull, "Missing_Comma_Or_Array_End" }, { 8ull, "Invalid_Number_Value" },
					{ 9ull, "Invalid_Null_Value" }, { 10ull, "Invalid_Bool_Value" }, { 11ull, "Invalid_String_Characters" }, { 12ull, "No_Input" } } } },
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
		String		 = 0x22u,
		Comma		 = 0x2Cu,
		Number		 = 0x2Du,
		Colon		 = 0x3Au,
		Array_Start	 = 0x5Bu,
		Array_End	 = 0x5Du,
		Null		 = 0x6Eu,
		Bool		 = 0x74u,
		Object_Start = 0x7Bu,
		Object_End	 = 0x7Du,
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
		friend class derailleur;

		JSONIFIER_INLINE error() noexcept = default;

		template<typename error_type, typename value_type> JSONIFIER_INLINE error(error_classes errorClassNew, const uint64_t errorIndexNew, uint64_t stringLengthNew,
			value_type* stringViewNew, error_type typeNew, std::source_location locationNew = std::source_location::current()) noexcept {
			errorType	 = static_cast<uint64_t>(typeNew);
			stringLength = stringLengthNew;
			errorClass	 = errorClassNew;
			stringView	 = stringViewNew;
			location	 = locationNew;
			errorIndex	 = errorIndexNew;
		}

		JSONIFIER_INLINE operator bool() {
			return errorType != 0;
		}

		JSONIFIER_INLINE bool operator==(const error& rhs) const {
			return errorType == rhs.errorType && errorIndex == rhs.errorIndex;
		}

		JSONIFIER_INLINE jsonifier::string getStringData(const jsonifier::string& errorString) const {
			if (errorIndex >= errorString.size()) {
				return {};
			}

			using V				   = std::remove_const_t<std::decay_t<decltype(errorString[0])>>;
			const auto start	   = std::begin(errorString) + static_cast<int64_t>(errorIndex);
			const auto rstart	   = std::rbegin(errorString) + static_cast<int64_t>(errorString.size()) - static_cast<int64_t>(errorIndex) - 1ll;
			const auto prevNewLine = std::find(std::min(rstart + 1, std::rend(errorString)), std::rend(errorString), static_cast<V>('\n'));
			const auto column	   = std::distance(rstart, prevNewLine);
			const auto nextNewLine = std::find(std::min(start + 1, std::end(errorString)), std::end(errorString), static_cast<V>('\n'));

			const auto offset = static_cast<int64_t>(errorIndex);
			auto contextBegin = std::begin(errorString) + static_cast<int64_t>(offset);
			auto contextEnd	  = nextNewLine;

			int64_t frontTruncation = 0;
			int64_t rearTruncation  = 0;

			if (std::distance(contextBegin, contextEnd) > 64) {
				if (column <= 32) {
					rearTruncation = 64;
					contextEnd	   = contextBegin + rearTruncation;
				} else {
					frontTruncation = column - 32;
					contextBegin += frontTruncation;
					if (std::distance(contextBegin, contextEnd) > 64) {
						rearTruncation = frontTruncation + 64;
						contextEnd	   = std::begin(errorString) + offset + rearTruncation;
					}
				}
			}

			jsonifier::string context = [&]() -> jsonifier::string {
				if constexpr (std::same_as<V, std::byte>) {
					return std::string{ reinterpret_cast<const char*>(&(*contextBegin)), reinterpret_cast<const char*>(&(*contextEnd)) };
				} else {
					return std::string{ contextBegin, contextEnd };
				}
			}();

			return context;
		}

		JSONIFIER_INLINE jsonifier::string reportError() const {
			jsonifier::string returnValue{ "Error of Type: " + errorMap[errorClass][errorType] + ", at errorIndex: " + jsonifier::toString(errorIndex) };
			if (stringView) {
				returnValue += "\nHere's some of the string's values:\n" + getStringData(stringView);
			}
			returnValue += jsonifier::string{ "\nIn file: " } + location.file_name() + ", at: " + jsonifier::toString(location.line()) + ":" +
				jsonifier::toString(location.column()) + ", in function: " + location.function_name() + "().\n";
			return returnValue;
		}

	  protected:
		std::source_location location{};
		string_view_ptr stringView{};
		error_classes errorClass{};
		uint64_t stringLength{};
		uint64_t errorIndex{};
		uint64_t errorType{};
	};

	template<typename error_type, typename value_type> JSONIFIER_INLINE error createError(error_classes errorClassNew, const uint64_t errorIndex, uint64_t stringLengthNew,
		value_type* stringViewNew, error_type errorNew, std::source_location locationNew = std::source_location::current()) {
		error newError(errorClassNew, errorIndex, stringLengthNew, stringViewNew, errorNew, locationNew);
		return newError;
	}

	JSONIFIER_INLINE std::ostream& operator<<(std::ostream& os, const error& errorNew) {
		os << errorNew.reportError();
		return os;
	}

}