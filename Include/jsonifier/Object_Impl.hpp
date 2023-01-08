
#pragma once

#include <jsonifier/Object.hpp>

namespace Jsonifier {

	inline JsonifierResult<Value> Object::findFieldUnordered(const std::string_view key) & noexcept {
		bool hasValue{};
		JsonifierTry(ValueIterator::findFieldUnorderedRaw(key).get(hasValue));
		if (!hasValue) {
			return No_Such_Field;
		}
		return Value(ValueIterator::child());
	}

	inline JsonifierResult<Value> Object::findFieldUnordered(const std::string_view key) && noexcept {
		bool hasValue{};
		JsonifierTry(ValueIterator::findFieldUnorderedRaw(key).get(hasValue));
		if (!hasValue) {
			return No_Such_Field;
		}
		return Value(ValueIterator::child());
	}

	inline JsonifierResult<Value> Object::operator[](const std::string_view key) & noexcept {
		return findFieldUnordered(key);
	}

	inline JsonifierResult<Value> Object::operator[](const std::string_view key) && noexcept {
		return std::forward<Object>(*this).findFieldUnordered(key);
	}

	inline JsonifierResult<Value> Object::findField(const std::string_view key) & noexcept {
		bool hasValue{};
		JsonifierTry(ValueIterator::findFieldRaw(key).get(hasValue));
		if (!hasValue) {
			return No_Such_Field;
		}
		return Value(ValueIterator::child());
	}

	inline JsonifierResult<Value> Object::findField(const std::string_view key) && noexcept {
		bool hasValue{};
		JsonifierTry(ValueIterator::findFieldRaw(key).get(hasValue));
		if (!hasValue) {
			return No_Such_Field;
		}
		return Value(ValueIterator::child());
	}

	inline JsonifierResult<Object> Object::start(ValueIterator& iterator) noexcept {
		JsonifierTry(iterator.startObject().error());
		return Object(iterator);
	}

	inline JsonifierResult<Object> Object::startRoot(ValueIterator& iterator) noexcept {
		JsonifierTry(iterator.startRootObject().error());
		return Object(iterator);
	}

	inline ErrorCode Object::consume() noexcept {
		if (ValueIterator::isAtKey()) {
			RawJsonString actualKey{};
			auto error = ValueIterator::fieldKey().get(actualKey);
			if (error) {
				ValueIterator::abandon();
				return error;
			};
			if ((error = ValueIterator::fieldValue())) {
				ValueIterator::abandon();
				return error;
			}
		}
		auto errorSkip = ValueIterator::jsonIter().skipChild(ValueIterator::depth() - 1);
		if (errorSkip) {
			ValueIterator::abandon();
		}
		return errorSkip;
	}

	inline JsonifierResult<std::string_view> Object::rawJson() noexcept {
		const uint8_t* startingPoint{ ValueIterator::peekStart() };
		auto error = consume();
		if (error) {
			return error;
		}
		const uint8_t* finalPoint{ ValueIterator::jsonIterator->peek(0) };
		return std::string_view(reinterpret_cast<const char*>(startingPoint), size_t(finalPoint - startingPoint));
	}

	inline JsonifierResult<Object> Object::started(ValueIterator& iterator) noexcept {
		JsonifierTry(iterator.startedObject().error());
		return Object(iterator);
	}

	inline Object Object::resume(const ValueIterator& iterator) noexcept {
		return iterator;
	}

	inline Object::Object(const ValueIterator& iteratorNew) noexcept : ValueIterator{ iteratorNew } {
	}

	inline JsonifierResult<ObjectIterator> Object::begin() noexcept {
		return ObjectIterator(*this);
	}

	inline JsonifierResult<ObjectIterator> Object::end() noexcept {
		return ObjectIterator(*this);
	}

	inline JsonifierResult<Value> Object::atPointer(std::string_view jsonPointer) noexcept {
		if (jsonPointer[0] != '/') {
			return Invalid_Json_Pointer;
		}
		jsonPointer = jsonPointer.substr(1);
		size_t slash = jsonPointer.find('/');
		std::string_view key = jsonPointer.substr(0, slash);
		JsonifierResult<Value> child;
		size_t escape = key.find('~');
		if (escape != std::string_view::npos) {
			std::string unescaped(key);
			do {
				switch (unescaped[escape + 1]) {
					case '0':
						unescaped.replace(escape, 2, "~");
						break;
					case '1':
						unescaped.replace(escape, 2, "/");
						break;
					default:
						return Invalid_Json_Pointer;
				}
				escape = unescaped.find('~', escape + 1);
			} while (escape != std::string::npos);
			child = findField(unescaped);
		} else {
			child = findField(key);
		}
		if (child.error()) {
			return child;
		}
		if (slash != std::string_view::npos) {
			child = child.atPointer(jsonPointer.substr(slash));
		}
		return child;
	}

	inline JsonifierResult<size_t> Object::countFields() & noexcept {
		size_t count{ 0 };
		for (auto v: *this) {
			count++;
		}
		if (ValueIterator::error()) {
			return ValueIterator::error();
		}
		ValueIterator::resetObject();
		return count;
	}

	inline JsonifierResult<bool> Object::isEmpty() & noexcept {
		bool isNotEmpty{};
		auto error = ValueIterator::resetObject().get(isNotEmpty);
		if (error) {
			return error;
		}
		return !isNotEmpty;
	}

	inline JsonifierResult<bool> Object::reset() & noexcept {
		return ValueIterator::resetObject();
	}

	inline JsonifierResult<Object>::JsonifierResult(Object&& Value) noexcept
		: ImplementationJsonifierResultBase<Object>(std::forward<Object>(Value)){}

	inline JsonifierResult<Object>::JsonifierResult(ErrorCode error) noexcept : ImplementationJsonifierResultBase<Object>(error){}

	inline JsonifierResult<ObjectIterator> JsonifierResult<Object>::begin() noexcept {
		if (error()) {
			return error();
		}
		return first.begin();
	}

	inline JsonifierResult<ObjectIterator> JsonifierResult<Object>::end() noexcept {
		if (error()) {
			return error();
		}
		return first.end();
	}

	inline JsonifierResult<Value> JsonifierResult<Object>::findFieldUnordered(std::string_view key) & noexcept {
		if (error()) {
			return error();
		}
		return first.findFieldUnordered(key);
	}

	inline JsonifierResult<Value> JsonifierResult<Object>::findFieldUnordered(std::string_view key) && noexcept {
		if (error()) {
			return error();
		}
		return std::forward<Object>(first).findFieldUnordered(key);
	}

	inline JsonifierResult<Value> JsonifierResult<Object>::operator[](std::string_view key) & noexcept {
		if (error()) {
			return error();
		}
		return first[key];
	}

	inline JsonifierResult<Value> JsonifierResult<Object>::operator[](std::string_view key) && noexcept {
		if (error()) {
			return error();
		}
		return std::forward<Object>(first)[key];
	}

	inline JsonifierResult<Value> JsonifierResult<Object>::findField(std::string_view key) & noexcept {
		if (error()) {
			return error();
		}
		return first.findField(key);
	}

	inline JsonifierResult<Value> JsonifierResult<Object>::findField(std::string_view key) && noexcept {
		if (error()) {
			return error();
		}
		return std::forward<Object>(first).findField(key);
	}

	inline JsonifierResult<Value> JsonifierResult<Object>::atPointer(std::string_view jsonPointer) noexcept {
		if (error()) {
			return error();
		}
		return first.atPointer(jsonPointer);
	}

	inline JsonifierResult<bool> JsonifierResult<Object>::reset() noexcept {
		if (error()) {
			return error();
		}
		return first.reset();
	}

	inline JsonifierResult<bool> JsonifierResult<Object>::isEmpty() noexcept {
		if (error()) {
			return error();
		}
		return first.isEmpty();
	}

	inline JsonifierResult<size_t> JsonifierResult<Object>::countFields() & noexcept {
		if (error()) {
			return error();
		}
		return first.countFields();
	}
}
