#pragma once

#include <jsonifier/Field.hpp>

namespace Jsonifier {

	Field::Field() noexcept : std::pair<RawJsonString, Value>(){};

	Field::Field(RawJsonString key, Value&& value) noexcept : std::pair<RawJsonString, Value>(key, std::forward<Value>(value)){};

	JsonifierResult<Field> Field::start(ValueIterator& parentIter) noexcept {
		RawJsonString key;
		JsonifierTry(parentIter.fieldKey().get(key));
		JsonifierTry(parentIter.fieldValue());
		return Field::start(parentIter, key);
	}

	JsonifierResult<Field> Field::start(const ValueIterator& parentIter, RawJsonString key) noexcept {
		return Field(key, parentIter.child());
	}

	JsonifierResult<std::string_view> Field::unescapedKey() noexcept {
		JsonifierResult<std::string_view> answer = first.unescape(second.jsonIter());
		first.consume();
		return answer;
	}

	RawJsonString Field::key() const noexcept {
		assert(first.stringView != nullptr);
		return first;
	}

	Value& Field::value() & noexcept {
		return second;
	}

	Value Field::value() && noexcept {
		return std::forward<Field>(*this).second;
	}

	JsonifierResult<Field>::JsonifierResult(Field&& Value) noexcept : JsonifierResultBase<Field>(std::forward<Field>(Value)) {
	}

	JsonifierResult<Field>::JsonifierResult(ErrorCode error) noexcept : JsonifierResultBase<Field>(error) {
	}

	JsonifierResult<RawJsonString> JsonifierResult<Field>::key() noexcept {
		if (error()) {
			return error();
		}
		return first.key();
	}

	JsonifierResult<std::string_view> JsonifierResult<Field>::unescapedKey() noexcept {
		if (error()) {
			return error();
		}
		return first.unescapedKey();
	}

	JsonifierResult<Value> JsonifierResult<Field>::value() noexcept {
		if (error()) {
			return error();
		}
		return std::move(first.value());
	}
}