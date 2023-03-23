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

#include <unordered_map>
#include <exception>

namespace Jsonifier {

	template<size_t N> struct StringLiteral;

	enum class ErrorCode { Success = 0, Parse_Error = 1, Number_Error = 2, Unknown_Key = 3, Incorrect_Type = 4, Setup_Error = 5 };

	template<StringLiteral string> class JsonifierError : public std::exception {
	  public:
		JsonifierError(const char* const stringError, uint64_t indexNew = std::string::npos) {
			errorMessage = static_cast<std::string>(string.sv()) + " Error";
			if (indexNew != std::string::npos) {
				errorMessage += ", at index: " + std::to_string(indexNew);
			}
			if (stringError) {
				errorMessage += std::string{ ", " } + stringError;
			}
		}

		const char* what() const noexcept override {
			return errorMessage.c_str();
		}

	  protected:
		std::string errorMessage{};
	};
}