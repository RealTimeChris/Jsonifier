#pragma once

#include <jsonifier/Base.hpp>
#include <jsonifier/Value.hpp>
#include <jsonifier/RawJsonString.hpp>

namespace Jsonifier {

	class Value;

	class Jsonifier_Dll Field : public std::pair<RawJsonString, Value> {
	  public:
		inline Field() noexcept;
		inline JsonifierResult<std::string_view> unescapedKey() noexcept;
		inline RawJsonString key() const noexcept;
		inline Value& value() & noexcept;
		inline Value value() && noexcept;

	  protected:
		inline Field(RawJsonString key, Value&& value) noexcept;
		static inline JsonifierResult<Field> start(ValueIterator& parentIter) noexcept;
		static inline JsonifierResult<Field> start(const ValueIterator& parentIter, RawJsonString key) noexcept;
		friend struct JsonifierResult<Field>;
		friend class ObjectIterator;
	};

	template<> struct JsonifierResult<Field> : public ImplementationJsonifierResultBase<Field> {
	  public:
		inline JsonifierResult(Field&& Value) noexcept;
		inline JsonifierResult(ErrorCode error) noexcept;
		inline JsonifierResult() noexcept = default;

		inline JsonifierResult<std::string_view> unescapedKey() noexcept;
		inline JsonifierResult<RawJsonString> key() noexcept;
		inline JsonifierResult<Value> value() noexcept;
	};

}
