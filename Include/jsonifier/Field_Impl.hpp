#pragma once

#include <jsonifier/Field.hpp>

namespace Jsonifier {

	inline Field::Field() noexcept : std::pair<RawJsonString, Value>(){};

	inline Field::Field(RawJsonString key, Value&& value) noexcept : std::pair<RawJsonString, Value>(key, std::forward<Value>(value)) {
	}

	inline JsonifierResult<Field> Field::start(ValueIterator& parentIter) noexcept {
		RawJsonString key;
		JsonifierTry(parentIter.fieldKey().get(key));
		JsonifierTry(parentIter.fieldValue());
		return Field::start(parentIter, key);
	}

	inline JsonifierResult<Field> Field::start(const ValueIterator& parentIter, RawJsonString key) noexcept {
		return Field(key, parentIter.child());
	}

	inline JsonifierResult<std::string_view> Field::unescapedKey() noexcept {
		JsonifierResult<std::string_view> answer = first.unescape(second.jsonIter());
		first.consume();
		return answer;
	}

	inline RawJsonString Field::key() const noexcept {
		assert(first.stringView != nullptr);
		return first;
	}

	inline Value& Field::value() & noexcept {
		return second;
	}

	inline Value Field::value() && noexcept {
		return std::forward<Field>(*this).second;
	}

	inline JsonifierResult<Field>::JsonifierResult(Field&& Value) noexcept : JsonifierResultBase<Field>(std::forward<Field>(Value)) {
	}

	inline JsonifierResult<Field>::JsonifierResult(ErrorCode error) noexcept : JsonifierResultBase<Field>(error) {
	}

	inline JsonifierResult<RawJsonString> JsonifierResult<Field>::key() noexcept {
		if (error()) {
			return error();
		}
		return first.key();
	}

	inline JsonifierResult<std::string_view> JsonifierResult<Field>::unescapedKey() noexcept {
		if (error()) {
			return error();
		}
		return first.unescapedKey();
	}

	inline JsonifierResult<Value> JsonifierResult<Field>::value() noexcept {
		if (error()) {
			return error();
		}
		return std::move(first.value());
	}
}