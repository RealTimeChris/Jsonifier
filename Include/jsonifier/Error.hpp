/*
	Jsonifier - For parsing and serializing Json - very rapidly.
	Copyright (C) 2023 Chris M. (RealTimeChris)

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	This library is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
	Lesser General Public License for more details.

	You should have received a copy of the GNU Lesser General Public
	License along with this library; if not, write to the Free Software
	Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301
	USA
*/
/// https://github.com/RealTimeChris/Jsonifier
/// Feb 3, 2023
#pragma once

#include <source_location>
#include <unordered_map>
#include <exception>
#include <string>

namespace Jsonifier {

	enum class ErrorCode { Success = 0, Parse_Error = 1, Number_Error = 2, Unknown_Key = 3, Incorrect_Type = 4, Setup_Error = 5 };

	inline std::unordered_map<ErrorCode, String> errorCodes{ { ErrorCode::Success, String{ "Success" } },
		{ ErrorCode::Parse_Error, String{ "Parse" } }, { ErrorCode::Number_Error, String{ "Number" } },
		{ ErrorCode::Unknown_Key, String{ "Unknown Key" } }, { ErrorCode::Incorrect_Type, String{ "Incorrect Type" } },
		{ ErrorCode::Setup_Error, String{ "Setup" } } };

	template<ErrorCode errorCode> class JsonifierError : public std::exception {
	  public:
		inline constexpr JsonifierError(const String& stringError, uint64_t indexNew = String::npos,
			std::source_location location = std::source_location::current()) {
			errorMessage = errorCodes[errorCode] + " Error at: " + String{ location.file_name() } + ":" + std::to_string(location.line()) + ":" +
				std::to_string(location.column());
			if (indexNew != String::npos) {
				errorMessage += ", at index: " + std::to_string(indexNew);
			}
			if (stringError.size() != 0) {
				errorMessage += String{ ", " } + String{ stringError };
			}
		}

		inline constexpr JsonifierError(String&& stringError, uint64_t indexNew = String::npos,
			std::source_location location = std::source_location::current()) {
			errorMessage = errorCodes[errorCode] + " Error at: " + String{ location.file_name() } + ":" + std::to_string(location.line()) + ":" +
				std::to_string(location.column());
			if (indexNew != String::npos) {
				errorMessage += ", at index: " + std::to_string(indexNew);
			}
			if (stringError.size() != 0) {
				errorMessage += String{ ", " } + String{ stringError };
			}
		}

		inline constexpr const char* what() const noexcept override {
			return errorMessage.c_str();
		}

	  protected:
		String errorMessage{};
	};
}
