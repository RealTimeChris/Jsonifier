#pragma once

#include <jsonifier/Base.hpp>
#include <jsonifier/Value.hpp>
#include <jsonifier/RawJsonString.hpp>

namespace Jsonifier {

	class Jsonifier_Dll Field : public std::pair<RawJsonString, Value> {
	  public:
		__forceinline Field() noexcept = default;
		__forceinline JsonifierResult<std::string_view> unescapedKey() noexcept;
		__forceinline RawJsonString key() const noexcept;
		__forceinline Value& value() & noexcept;
		__forceinline Value value() && noexcept;

	  protected:
		__forceinline Field(RawJsonString key, Value&& value) noexcept;
		__forceinline static JsonifierResult<Field> start(ValueIterator& parentIter) noexcept;
		__forceinline static JsonifierResult<Field> start(const ValueIterator& parentIter, RawJsonString key) noexcept;
		friend struct JsonifierResult<Field>;
		friend class ObjectIterator;
	};

	template<> struct JsonifierResult<Field> : public JsonifierResultBase<Field> {
	  public:
		__forceinline JsonifierResult(Field&& Value) noexcept;
		__forceinline JsonifierResult(ErrorCode error) noexcept;
		__forceinline JsonifierResult() noexcept = default;

		__forceinline JsonifierResult<std::string_view> unescapedKey() noexcept;
		__forceinline JsonifierResult<RawJsonString> key() noexcept;
		__forceinline JsonifierResult<Value> value() noexcept;
	};

	__forceinline JsonifierResult<Field>::JsonifierResult(Field&& Value) noexcept : JsonifierResultBase<Field>(std::forward<Field>(Value)){};

	__forceinline JsonifierResult<Field>::JsonifierResult(ErrorCode error) noexcept : JsonifierResultBase<Field>(error){};

	__forceinline JsonifierResult<Value> JsonifierResult<Field>::value() noexcept {
		if (error()) {
			return error();
		}
		return std::move(first.value());
	}

}
