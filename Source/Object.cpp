#include <jsonifier/Object_Impl.hpp>

namespace Jsonifier {
	ObjectIterator::ObjectIterator(const ValueIterator& _iterator) noexcept : ValueIterator{ _iterator } {
	}

	JsonifierResult<Field> ObjectIterator::operator*() noexcept {
		ErrorCode error = this->error();
		if (error) {
			this->abandon();
			return error;
		}
		auto result = Field::start(*this);
		if (result.error()) {
			this->abandon();
		}
		return result;
	}

	bool ObjectIterator::operator==(const ObjectIterator& other) const noexcept {
		return !(*this != other);
	}

	bool ObjectIterator::operator!=(const ObjectIterator&) const noexcept {
		return this->isOpen();
	}

	ObjectIterator& ObjectIterator::operator++() noexcept {
		if (!this->isOpen()) {
			return *this;
		}

		ErrorCode error;
		if ((error = this->skipChild())) {
			return *this;
		}

		bool hasValue{};
		if ((error = this->hasNextField().get(hasValue))) {
			return *this;
		};
		return *this;
	}

	JsonifierResult<Value> Object::findFieldUnordered(const std::string_view key) & noexcept {
		bool hasValue{};
		JsonifierTry(iterator.findFieldUnorderedRaw(key).get(hasValue));
		if (!hasValue) {
			return No_Such_Field;
		}
		return Value(iterator.child());
	}

	JsonifierResult<Value> Object::findFieldUnordered(const std::string_view key) && noexcept {
		bool hasValue{};
		JsonifierTry(iterator.findFieldUnorderedRaw(key).get(hasValue));
		if (!hasValue) {
			return No_Such_Field;
		}
		return Value(iterator.child());
	}

	JsonifierResult<Value> Object::operator[](const std::string_view key) & noexcept {
		return findFieldUnordered(key);
	}

	JsonifierResult<Value> Object::operator[](const std::string_view key) && noexcept {
		return std::forward<Object>(*this).findFieldUnordered(key);
	}

	JsonifierResult<Value> Object::findField(const std::string_view key) & noexcept {
		bool hasValue{};
		JsonifierTry(iterator.findFieldRaw(key).get(hasValue));
		if (!hasValue) {
			return No_Such_Field;
		}
		return Value(iterator.child());
	}

	JsonifierResult<Value> Object::findField(const std::string_view key) && noexcept {
		bool hasValue{};
		JsonifierTry(iterator.findFieldRaw(key).get(hasValue));
		if (!hasValue) {
			return No_Such_Field;
		}
		return Value(iterator.child());
	}

	JsonifierResult<Object> Object::start(ValueIterator& iterator) noexcept {
		JsonifierTry(iterator.startObject().error());
		return Object(iterator);
	}

	JsonifierResult<Object> Object::startRoot(ValueIterator& iterator) noexcept {
		JsonifierTry(iterator.startRootObject().error());
		return Object(iterator);
	}

	ErrorCode Object::consume() noexcept {
		if (iterator.isAtKey()) {
			RawJsonString actualKey{};
			auto error = iterator.fieldKey().get(actualKey);
			if (error) {
				iterator.abandon();
				return error;
			};
			if ((error = iterator.fieldValue())) {
				iterator.abandon();
				return error;
			}
		}
		auto errorSkip = iterator.jsonIter().skipChild(iterator.depth() - 1);
		if (errorSkip) {
			iterator.abandon();
		}
		return errorSkip;
	}

	JsonifierResult<std::string_view> Object::rawJson() noexcept {
		const uint8_t* startingPoint{ iterator.peekStart() };
		auto error = consume();
		if (error) {
			return error;
		}
		const uint8_t* finalPoint{ iterator.jsonIterator->peek(0) };
		return std::string_view(reinterpret_cast<const char*>(startingPoint), size_t(finalPoint - startingPoint));
	}

	JsonifierResult<Object> Object::started(ValueIterator& iterator) noexcept {
		JsonifierTry(iterator.startedObject().error());
		return Object(iterator);
	}

	Object Object::resume(const ValueIterator& iterator) noexcept {
		return iterator;
	}

	Object::Object(const ValueIterator& _iterator) noexcept : iterator{ _iterator } {
	}

	JsonifierResult<ObjectIterator> Object::begin() noexcept {
		return ObjectIterator(iterator);
	}

	JsonifierResult<ObjectIterator> Object::end() noexcept {
		return ObjectIterator(iterator);
	}

	JsonifierResult<Value> Object::atPointer(std::string_view jsonPointer) noexcept {
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

	JsonifierResult<size_t> Object::countFields() & noexcept {
		size_t count{ 0 };
		for (auto v: *this) {
			count++;
		}
		if (iterator.error()) {
			return iterator.error();
		}
		iterator.resetObject();
		return count;
	}

	JsonifierResult<bool> Object::isEmpty() & noexcept {
		bool isNotEmpty;
		auto error = iterator.resetObject().get(isNotEmpty);
		if (error) {
			return error;
		}
		return !isNotEmpty;
	}

	JsonifierResult<bool> Object::reset() & noexcept {
		return iterator.resetObject();
	}
}
