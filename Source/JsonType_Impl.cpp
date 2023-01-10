#pragma once

#include <jsonifier/JsonType.hpp>
#include <jsonifier/Parser.hpp>

namespace Jsonifier {

	std::ostream& operator<<(std::ostream& out, JsonType type) noexcept {
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

	std::ostream& operator<<(std::ostream& out, JsonifierResult<JsonType>& type) noexcept(false) {
		return out << type.value();
	}

	JsonType __forceinline JsonifierResult<JsonType>::value() noexcept {
		return this->first;
	}

	JsonifierResult<JsonType>::JsonifierResult(JsonType&& Value) noexcept : JsonifierResultBase<JsonType>{ std::move(Value) } {};


	JsonifierResult<JsonType>::JsonifierResult(ErrorCode error) noexcept : JsonifierResultBase{ error } {};
}
