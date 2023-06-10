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
/// https://github.com/RealTimeChris/Jsonifier
/// Feb 3, 2023
#pragma once

#include <source_location>
#include <unordered_map>
#include <exception>
#include <string>

namespace JsonifierInternal {

	enum class ErrorCode { Success = 0, Parse_Error = 1, Number_Error = 2, Unknown_Key = 3, Incorrect_Type = 4, Setup_Error = 5 };

	inline std::unordered_map<ErrorCode, Jsonifier::String> errorCodes{ { ErrorCode::Success, Jsonifier::String{ "Success" } },
		{ ErrorCode::Parse_Error, Jsonifier::String{ "Parse" } }, { ErrorCode::Number_Error, Jsonifier::String{ "Number" } },
		{ ErrorCode::Unknown_Key, Jsonifier::String{ "Unknown Key" } }, { ErrorCode::Incorrect_Type, Jsonifier::String{ "Incorrect Type" } },
		{ ErrorCode::Setup_Error, Jsonifier::String{ "Setup" } } };

	template<ErrorCode errorCode> class JsonifierError : public std::exception {
	  public:
		inline constexpr JsonifierError(const Jsonifier::String& stringError, size_t indexNew = Jsonifier::String::npos,
			std::source_location location = std::source_location::current()) {
			errorMessage = errorCodes[errorCode] + " Error at: " + Jsonifier::String{ location.file_name() } + ":" + std::to_string(location.line()) +
				":" + std::to_string(location.column());
			if (indexNew != Jsonifier::String::npos) {
				errorMessage += ", at index: " + std::to_string(indexNew);
			}
			if (stringError.size() != 0) {
				errorMessage += Jsonifier::String{ ", " } + Jsonifier::String{ stringError };
			}
		}

		inline constexpr JsonifierError(Jsonifier::String&& stringError, size_t indexNew = Jsonifier::String::npos,
			std::source_location location = std::source_location::current()) {
			errorMessage = errorCodes[errorCode] + " Error at: " + Jsonifier::String{ location.file_name() } + ":" + std::to_string(location.line()) +
				":" + std::to_string(location.column());
			if (indexNew != Jsonifier::String::npos) {
				errorMessage += ", at index: " + std::to_string(indexNew);
			}
			if (stringError.size() != 0) {
				errorMessage += Jsonifier::String{ ", " } + Jsonifier::String{ stringError };
			}
		}

		inline constexpr const char* what() const noexcept override {
			return errorMessage.data();
		}

	  protected:
		Jsonifier::String errorMessage{};
	};
}
