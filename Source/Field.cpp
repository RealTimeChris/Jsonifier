#include <jsonifier/Field_Impl.hpp>

namespace Jsonifier {

	Field::Field() noexcept : std::pair<RawJsonString, Value>() {
	}

	Field::Field(RawJsonString key, Value&& value) noexcept : std::pair<RawJsonString, Value>(key, std::forward<Value>(value)) {
	}

	JsonifierResult<Field> Field::start(ValueIterator& parent_iterator) noexcept {
		RawJsonString key;
		JsonifierTry(parent_iterator.fieldKey().get(key));
		JsonifierTry(parent_iterator.fieldValue());
		return Field::start(parent_iterator, key);
	}

	JsonifierResult<Field> Field::start(const ValueIterator& parent_iterator, RawJsonString key) noexcept {
		return Field(key, parent_iterator.child());
	}

	JsonifierResult<std::string_view> Field::unescapedKey() noexcept {
		assert(first.stringView != nullptr);
		JsonifierResult<std::string_view> answer = first.unescape(second.iterator.jsonIter());
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

}
