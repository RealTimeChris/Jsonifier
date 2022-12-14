#pragma once

#include <jsonifier/Object.hpp>
#include <jsonifier/RawJsonString.hpp>
#include <jsonifier/Field.hpp>
#include <jsonifier/Value_Impl.hpp>

namespace Jsonifier {

	__forceinline JsonifierResult<Value> Object::findFieldUnordered(const std::string_view key) & noexcept {
		bool hasValue{};
		JsonifierTry(ValueIterator::findFieldUnorderedRaw(key).get(hasValue));
		if (!hasValue) {
			return No_Such_Field;
		}
		return Value(ValueIterator::child());
	}

	__forceinline JsonifierResult<Value> Object::findFieldUnordered(const std::string_view key) && noexcept {
		bool hasValue{};
		JsonifierTry(ValueIterator::findFieldUnorderedRaw(key).get(hasValue));
		if (!hasValue) {
			return No_Such_Field;
		}
		return Value(ValueIterator::child());
	}

	__forceinline JsonifierResult<Value> Object::operator[](const std::string_view key) & noexcept {
		return findFieldUnordered(key);
	}

	__forceinline JsonifierResult<Value> Object::operator[](const std::string_view key) && noexcept {
		return std::forward<Object>(*this).findFieldUnordered(key);
	}

	__forceinline JsonifierResult<Value> Object::findField(const std::string_view key) & noexcept {
		bool hasValue{};
		JsonifierTry(ValueIterator::findFieldRaw(key).get(hasValue));
		if (!hasValue) {
			return No_Such_Field;
		}
		return Value(ValueIterator::child());
	}

	__forceinline JsonifierResult<Value> Object::findField(const std::string_view key) && noexcept {
		bool hasValue{};
		JsonifierTry(ValueIterator::findFieldRaw(key).get(hasValue));
		if (!hasValue) {
			return No_Such_Field;
		}
		return Value(ValueIterator::child());
	}

	__forceinline JsonifierResult<Object> Object::start(ValueIterator& iterator) noexcept {
		JsonifierTry(iterator.startObject().error());
		return Object(iterator);
	}

	__forceinline JsonifierResult<Object> Object::startRoot(ValueIterator& iterator) noexcept {
		JsonifierTry(iterator.startRootObject().error());
		return Object(iterator);
	}

	ErrorCode Object::consume() noexcept {
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

	__forceinline JsonifierResult<std::string_view> Object::rawJson() noexcept {
		const uint8_t* startingPoint{ ValueIterator::peekStart() };
		auto error = consume();
		if (error) {
			return error;
		}
		const uint8_t* finalPoint{ ValueIterator::jsonIterator->peek(0) };
		return std::string_view(reinterpret_cast<const char*>(startingPoint), size_t(finalPoint - startingPoint));
	}

	__forceinline JsonifierResult<Object> Object::started(ValueIterator& iterator) noexcept {
		JsonifierTry(iterator.startedObject().error());
		return Object(iterator);
	}

	Object Object::resume(const ValueIterator& iterator) noexcept {
		return iterator;
	}

	Object::Object(const ValueIterator& iteratorNew) noexcept : ValueIterator{ iteratorNew } {
	}

	__forceinline JsonifierResult<ObjectIterator> Object::begin() noexcept {
		return ObjectIterator(*this);
	}

	__forceinline JsonifierResult<ObjectIterator> Object::end() noexcept {
		return ObjectIterator(*this);
	}

	__forceinline JsonifierResult<Value> Object::atPointer(std::string_view jsonPointer) noexcept {
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

	__forceinline JsonifierResult<size_t> Object::countFields() & noexcept {
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

	__forceinline JsonifierResult<bool> Object::isEmpty() & noexcept {
		bool isNotEmpty{};
		auto error = ValueIterator::resetObject().get(isNotEmpty);
		if (error) {
			return error;
		}
		return !isNotEmpty;
	}

	__forceinline JsonifierResult<bool> Object::reset() & noexcept {
		return ValueIterator::resetObject();
	}

	__forceinline JsonifierResult<Object>::JsonifierResult(Object&& Value) noexcept : JsonifierResultBase<Object>(std::forward<Object>(Value)) {
	}

	__forceinline JsonifierResult<Object>::JsonifierResult(ErrorCode error) noexcept : JsonifierResultBase<Object>(error) {
	}

	__forceinline JsonifierResult<ObjectIterator> JsonifierResult<Object>::begin() noexcept {
		if (error()) {
			return error();
		}
		return first.begin();
	}

	__forceinline JsonifierResult<ObjectIterator> JsonifierResult<Object>::end() noexcept {
		if (error()) {
			return error();
		}
		return first.end();
	}

	__forceinline JsonifierResult<Value> JsonifierResult<Object>::findFieldUnordered(std::string_view key) & noexcept {
		if (error()) {
			return error();
		}
		return first.findFieldUnordered(key);
	}

	__forceinline JsonifierResult<Value> JsonifierResult<Object>::findFieldUnordered(std::string_view key) && noexcept {
		if (error()) {
			return error();
		}
		return std::forward<Object>(first).findFieldUnordered(key);
	}

	__forceinline JsonifierResult<Value> JsonifierResult<Object>::operator[](std::string_view key) & noexcept {
		if (error()) {
			return error();
		}
		return first[key];
	}

	__forceinline JsonifierResult<Value> JsonifierResult<Object>::operator[](std::string_view key) && noexcept {
		if (error()) {
			return error();
		}
		return std::forward<Object>(first)[key];
	}

	__forceinline JsonifierResult<Value> JsonifierResult<Object>::findField(std::string_view key) & noexcept {
		if (error()) {
			return error();
		}
		return first.findField(key);
	}

	__forceinline JsonifierResult<Value> JsonifierResult<Object>::findField(std::string_view key) && noexcept {
		if (error()) {
			return error();
		}
		return std::forward<Object>(first).findField(key);
	}

	__forceinline JsonifierResult<Value> JsonifierResult<Object>::atPointer(std::string_view jsonPointer) noexcept {
		if (error()) {
			return error();
		}
		return first.atPointer(jsonPointer);
	}

	__forceinline JsonifierResult<bool> JsonifierResult<Object>::reset() noexcept {
		if (error()) {
			return error();
		}
		return first.reset();
	}

	__forceinline JsonifierResult<bool> JsonifierResult<Object>::isEmpty() noexcept {
		if (error()) {
			return error();
		}
		return first.isEmpty();
	}

	__forceinline JsonifierResult<size_t> JsonifierResult<Object>::countFields() & noexcept {
		if (error()) {
			return error();
		}
		return first.countFields();
	}
}
