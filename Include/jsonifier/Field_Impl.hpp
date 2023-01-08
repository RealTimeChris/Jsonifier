#pragma once

#include <jsonifier/Field.hpp>

namespace Jsonifier {
	// clang 6 doesn't think the default constructor can be noexcept, so we make it explicit
	inline Field::Field() noexcept : std::pair<RawJsonString, Value>() {
	}

	inline Field::Field(RawJsonString key, Value&& value) noexcept : std::pair<RawJsonString, Value>(key, std::forward<Value>(value)) {
	}

	inline JsonifierResult<Field> Field::start(ValueIterator& parent_iter) noexcept {
		RawJsonString key;
		JsonifierTry(parent_iter.field_key().get(key));
		JsonifierTry(parent_iter.field_value());
		return Field::start(parent_iter, key);
	}

	inline JsonifierResult<Field> Field::start(const ValueIterator& parent_iter, RawJsonString key) noexcept {
		return Field(key, parent_iter.child());
	}

	inline JsonifierResult<std::string_view> Field::unescaped_key() noexcept {
		assert(first.stringView != nullptr);// We would like to call .alive() but Visual Studio won't let us.
		JsonifierResult<std::string_view> answer = first.unescape(second.iterator.json_iter());
		first.consume();
		return answer;
	}

	inline RawJsonString Field::key() const noexcept {
		assert(first.stringView != nullptr);// We would like to call .alive() by Visual Studio won't let us.
		return first;
	}

	inline Value& Field::value() & noexcept {
		return second;
	}

	inline Value Field::value() && noexcept {
		return std::forward<Field>(*this).second;
	}
	inline JsonifierResult<Field>::JsonifierResult(Field&& Value) noexcept : ImplementationJsonifierResultBase<Field>(std::forward<Field>(Value)) {
	}
	inline JsonifierResult<Field>::JsonifierResult(ErrorCode error) noexcept : ImplementationJsonifierResultBase<Field>(error) {
	}

	inline JsonifierResult<RawJsonString> JsonifierResult<Field>::key() noexcept {
		if (error()) {
			return error();
		}
		return first.key();
	}
	inline JsonifierResult<std::string_view> JsonifierResult<Field>::unescaped_key() noexcept {
		if (error()) {
			return error();
		}
		return first.unescaped_key();
	}
	inline JsonifierResult<Value> JsonifierResult<Field>::value() noexcept {
		if (error()) {
			return error();
		}
		return std::move(first.value());
	}

	
}