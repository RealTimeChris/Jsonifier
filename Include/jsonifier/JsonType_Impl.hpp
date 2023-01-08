#pragma once

#include <jsonifier/JsonType.hpp>
#include <jsonifier/Parser.hpp>

namespace Jsonifier {

	inline std::ostream& operator<<(std::ostream& out, JsonType type) noexcept {
		switch (type) {
			case JsonType::Array:
				out << "Array";
				break;
			case JsonType::Object:
				out << "Object";
				break;
			case JsonType::Number:
				out << "number";
				break;
			case JsonType::String:
				out << "string";
				break;
			case JsonType::Bool:
				out << "boolean";
				break;
			case JsonType::Null:
				out << "null";
				break;
			default:
				assert(0);
		}
		return out;
	}

	inline std::ostream& operator<<(std::ostream& out, JsonifierResult<JsonType>& type) noexcept(false) {
		return out << type.value();
	}

	inline JsonifierResult<JsonType>::JsonifierResult(JsonType&& Value) noexcept
		: ImplementationJsonifierResultBase<JsonType>(std::forward<JsonType>(Value)){}

	inline JsonifierResult<JsonType>::JsonifierResult(ErrorCode error) noexcept : ImplementationJsonifierResultBase<JsonType>(error){}
}
