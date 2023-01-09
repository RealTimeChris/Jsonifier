#pragma once

#include <jsonifier/Base.hpp>
#include <jsonifier/Value.hpp>
#include <jsonifier/RawJsonString.hpp>

namespace Jsonifier {

	class Value;

	class Jsonifier_Dll Field : public std::pair<RawJsonString, Value> {
	  public:
		Field() noexcept;
		JsonifierResult<std::string_view> unescapedKey() noexcept;
		RawJsonString key() const noexcept;
		Value& value() & noexcept;
		Value value() && noexcept;

	  protected:
		Field(RawJsonString key, Value&& value) noexcept;
		static JsonifierResult<Field> start(ValueIterator& parentIter) noexcept;
		static JsonifierResult<Field> start(const ValueIterator& parentIter, RawJsonString key) noexcept;
		friend struct JsonifierResult<Field>;
		friend class ObjectIterator;
	};

	template<> struct JsonifierResult<Field> : public JsonifierResultBase<Field> {
	  public:
		JsonifierResult(Field&& Value) noexcept;
		JsonifierResult(ErrorCode error) noexcept;
		JsonifierResult() noexcept = default;

		JsonifierResult<std::string_view> unescapedKey() noexcept;
		JsonifierResult<RawJsonString> key() noexcept;
		JsonifierResult<Value> value() noexcept;
	};

}
