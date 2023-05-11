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

#ifndef __APPLE__
	#include <source_location>
#else

namespace std {

	class source_location {
	  public:
		constexpr source_location() noexcept = default;

		constexpr source_location(int32_t lineNew = __LINE__, const char* const fileNew = __FILE__) : file{ fileNew } {
			lineVal = lineNew;
		}

		constexpr static std::source_location current(int32_t lineNew = __LINE__, const char* const fileNew = __FILE__) {
			return { lineNew, fileNew };
		}

		constexpr int32_t column() {
			return this->columnVal;
		}

		constexpr const char* const file_name() {
			return this->file;
		}

		constexpr const char* const function_name() {
			return nullptr;
		}

		constexpr int32_t line() {
			return this->lineVal;
		}

	  protected:
		int32_t columnVal{};
		int32_t lineVal{};
		const char* const file{};
	};
}
#endif
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
		inline constexpr JsonifierError(const Jsonifier::String& stringError, uint64_t indexNew = Jsonifier::String::npos,
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

		inline constexpr JsonifierError(Jsonifier::String&& stringError, uint64_t indexNew = Jsonifier::String::npos,
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
			return errorMessage.c_str();
		}

	  protected:
		Jsonifier::String errorMessage{};
	};
}
