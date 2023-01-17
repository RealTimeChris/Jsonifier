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

#include <jsonifier/Base.hpp>

namespace Jsonifier {

	template<> class JsonifierResult<JsonType> : public JsonifierResultBase<JsonType> {
	  public:
		__forceinline JsonifierResult() noexcept = default;

		__forceinline JsonifierResult(JsonType data) noexcept : JsonifierResultBase<JsonType>(std::move(data)){};

		__forceinline JsonifierResult(ErrorCode error) noexcept : JsonifierResultBase<JsonType>(error){};
	};

	__forceinline std::ostream& operator<<(std::ostream& out, JsonType type) noexcept {
		switch (type) {
			case JsonType::Object:
				out << "Object";
				break;
			case JsonType::Array:
				out << "Array";
				break;
			case JsonType::Number:
				out << "Number";
				break;
			case JsonType::String:
				out << "String";
				break;
			case JsonType::Bool:
				out << "Bool";
				break;
			case JsonType::Null:
				out << "Null";
				break;
			default:
				out << "Unset";
				break;
		}
		return out;
	}

	__forceinline std::ostream& operator<<(std::ostream& out, JsonifierResult<JsonType>& type) noexcept(false) {
		return out << type.value();
	}

	__forceinline std::ostream& operator<<(std::ostream& out, JsonifierResult<std::string_view> type) noexcept(false) {
		return out << type.value();
	}

}
