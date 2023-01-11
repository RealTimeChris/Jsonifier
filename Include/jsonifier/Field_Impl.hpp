#pragma once

#include <jsonifier/Field.hpp>
#include <jsonifier/Value_Impl.hpp>

namespace Jsonifier {

	__forceinline Field::Field(RawJsonString key, Value&& value) noexcept : std::pair<RawJsonString, Value>(key, std::forward<Value>(value)){};

	__forceinline JsonifierResult<Field> Field::start(ValueIterator& parentIter) noexcept {
		RawJsonString key;
		JsonifierTry(parentIter.fieldKey().get(key));
		JsonifierTry(parentIter.fieldValue());
		return Field::start(parentIter, key);
	}

	__forceinline JsonifierResult<Field> Field::start(const ValueIterator& parentIter, RawJsonString key) noexcept {
		return Field(key, parentIter.child());
	}

	__forceinline JsonifierResult<std::string_view> Field::unescapedKey() noexcept {
		JsonifierResult<std::string_view> answer = first.unescape(second.iterator.jsonIter());
		first.consume();
		return answer;
	}

	__forceinline RawJsonString Field::key() const noexcept {
		assert(first.stringView != nullptr);
		return first;
	}

	__forceinline Value& Field::value() & noexcept {
		return second;
	}

	__forceinline Value Field::value() && noexcept {
		return std::forward<Field>(*this).second;
	}

	__forceinline JsonifierResult<RawJsonString> JsonifierResult<Field>::key() noexcept {
		if (error()) {
			return error();
		}
		return first.key();
	}

	__forceinline JsonifierResult<std::string_view> JsonifierResult<Field>::unescapedKey() noexcept {
		if (error()) {
			return error();
		}
		return first.unescapedKey();
	}


}