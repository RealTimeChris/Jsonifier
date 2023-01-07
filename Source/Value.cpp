#include <jsonifier/Value_Impl.hpp>
#include <jsonifier/Array.hpp>
#include <jsonifier/Object.hpp>

namespace Jsonifier {

	Value::Value(const ValueIterator& _iterator) noexcept : iterator{ _iterator } {
	}

	Value Value::start(const ValueIterator& iterator) noexcept {
		return iterator;
	}

	Value Value::resume(const ValueIterator& iterator) noexcept {
		return iterator;
	}

	JsonifierResult<Array> Value::getArray() noexcept {
		return Array::start(iterator);
	}

	JsonifierResult<Object> Value::getObject() noexcept {
		return Object::start(iterator);
	}

	JsonifierResult<Object> Value::startOrResumeObject() noexcept {
		if (iterator.atStart()) {
			return getObject();
		} else {
			return Object::resume(iterator);
		}
	}

	JsonifierResult<RawJsonString> Value::getRawJsonString() noexcept {
		return iterator.getRawJsonString();
	}

	JsonifierResult<std::string_view> Value::getString() noexcept {
		return iterator.getString();
	}

	JsonifierResult<double> Value::getDouble() noexcept {
		return iterator.getDouble();
	}

	JsonifierResult<uint64_t> Value::getUint64() noexcept {
		return iterator.getUint64();
	}

	JsonifierResult<int64_t> Value::getInt64() noexcept {
		return iterator.getInt64();
	}

	JsonifierResult<bool> Value::getBool() noexcept {
		return iterator.getBool();
	}

	JsonifierResult<bool> Value::isNull() noexcept {
		return iterator.isNull();
	}

	Value::operator Array() noexcept(false) {
		return getArray();
	}

	Value::operator Object() noexcept(false) {
		return getObject();
	}

	Value::operator uint64_t() noexcept(false) {
		return getUint64();
	}

	Value::operator int64_t() noexcept(false) {
		return getInt64();
	}

	Value::operator double() noexcept(false) {
		return getDouble();
	}

	Value::operator std::string_view() noexcept(false) {
		return getString();
	}

	Value::operator RawJsonString() noexcept(false) {
		return getRawJsonString();
	}

	Value::operator bool() noexcept(false) {
		return getBool();
	}

	JsonifierResult<ArrayIterator> Value::begin() & noexcept {
		return getArray().begin();
	}

	JsonifierResult<ArrayIterator> Value::end() & noexcept {
		return {};
	}

	JsonifierResult<size_t> Value::countElements() & noexcept {
		JsonifierResult<size_t> answer;
		auto a = getArray();
		answer = a.countElements();
		iterator.moveAtStart();
		return answer;
	}

	JsonifierResult<size_t> Value::countFields() & noexcept {
		JsonifierResult<size_t> answer;
		auto a = getObject();
		answer = a.countFields();
		iterator.moveAtStart();
		return answer;
	}

	JsonifierResult<Value> Value::at(size_t index) noexcept {
		auto a = getArray();
		return a.at(index);
	}

	JsonifierResult<Value> Value::findField(std::string_view key) noexcept {
		return startOrResumeObject().findField(key);
	}

	JsonifierResult<Value> Value::findField(const char* key) noexcept {
		return startOrResumeObject().findField(key);
	}

	JsonifierResult<Value> Value::findFieldUnordered(std::string_view key) noexcept {
		return startOrResumeObject().findFieldUnordered(key);
	}

	JsonifierResult<Value> Value::findFieldUnordered(const char* key) noexcept {
		return startOrResumeObject().findFieldUnordered(key);
	}

	JsonifierResult<Value> Value::operator[](std::string_view key) noexcept {
		return startOrResumeObject()[key];
	}

	int64_t totalTime{};
	int64_t iteratorations{};

	JsonifierResult<Value> Value::operator[](const char* key) noexcept {
		auto result = startOrResumeObject()[key];
		return result;
	}

	JsonifierResult<JsonType> Value::type() noexcept {
		return iterator.type();
	}

	JsonifierResult<bool> Value::isScalar() noexcept {
		JsonType this_type;
		auto error = type().get(this_type);
		if (error) {
			return error;
		}
		return !((this_type == JsonType::Array) || (this_type == JsonType::Object));
	}

	std::string_view Value::rawJsonToken() noexcept {
		return std::string_view(reinterpret_cast<const char*>(iterator.peekStart()), iterator.peekStartLength());
	}

	JsonifierResult<const char*> Value::currentLocation() noexcept {
		return iterator.jsonIter().currentLocation();
	}

	int32_t Value::currentDepth() const noexcept {
		return iterator.jsonIter().depth();
	}

	JsonifierResult<Value> Value::atPointer(std::string_view jsonPointer) noexcept {
		JsonType t;
		JsonifierTry(type().get(t));
		switch (t) {
			case JsonType::Array:
				return (*this).getArray().atPointer(jsonPointer);
			case JsonType::Object:
				return (*this).getObject().atPointer(jsonPointer);
			default:
				return Invalid_Json_Pointer;
		}
	}
}
