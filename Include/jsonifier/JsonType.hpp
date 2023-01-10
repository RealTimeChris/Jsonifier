#pragma once

#include <jsonifier/Base.hpp>

namespace Jsonifier {

	class ValueIterator;

	template<> struct JsonifierResult<JsonType> : public JsonifierResultBase<JsonType> {
	  public:
		__forceinline JsonifierResult(JsonType&& Value) noexcept;
		__forceinline JsonifierResult(ErrorCode error) noexcept;
		__forceinline JsonType value() noexcept;
		__forceinline JsonifierResult() noexcept = default;
		__forceinline ~JsonifierResult() noexcept = default;

	  protected:
		JsonType first{};
		ErrorCode second{};
	};

	__forceinline JsonType JsonifierResult<JsonType>::value() noexcept {
		return this->first;
	}

	__forceinline JsonifierResult<JsonType>::JsonifierResult(JsonType&& Value) noexcept {};


	__forceinline JsonifierResult<JsonType>::JsonifierResult(ErrorCode error) noexcept {};

	__forceinline std::ostream& operator<<(std::ostream& out, JsonType type) noexcept {
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

	__forceinline std::ostream& operator<<(std::ostream& out, JsonifierResult<JsonType>& type) noexcept(false) {
		return out << type.value();
	}

};