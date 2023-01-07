#pragma once

#include <jsonifier/FoundationEntities.hpp>
#include <jsonifier/Value.hpp>

namespace Jsonifier {


	class Jsonifier_Dll Field : public std::pair<RawJsonString, Value> {
	  public:
		Field() noexcept;
		JsonifierResult<std::string_view> unescapedKey() noexcept;
		RawJsonString key() const noexcept;
		Value& value() & noexcept;
		Value value() && noexcept;

	  protected:
		Field(RawJsonString key, Value&& Value) noexcept;
		static JsonifierResult<Field> start(ValueIterator& parent_iter) noexcept;
		static JsonifierResult<Field> start(const ValueIterator& parent_iter, RawJsonString key) noexcept;
		friend struct JsonifierResult<Field>;
		friend class ObjectIterator;
	};

	template<> struct JsonifierResult<Field> : public ImplementationJsonifierResultBase<Field> {
	  public:
		inline JsonifierResult(Field&& value) noexcept;
		inline JsonifierResult(ErrorCode error) noexcept;
		inline JsonifierResult() noexcept = default;

		inline JsonifierResult<std::string_view> unescaped_key() noexcept;
		inline JsonifierResult<RawJsonString> key() noexcept;
		inline JsonifierResult<Value> value() noexcept;
	};
}