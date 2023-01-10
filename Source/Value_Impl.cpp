#pragma once

#include <jsonifier/Field.hpp>
#include <jsonifier/Array.hpp>
#include <jsonifier/Object.hpp>
#include <jsonifier/Value.hpp>
#include <jsonifier/ValueIterator.hpp>
#include <jsonifier/Value_Impl.hpp>

namespace Jsonifier {

	JsonifierResult<size_t> JsonifierResult<Value>::countElements() & noexcept {
		if (error()) {
			return error();
		}
		return first.countElements();
	}

	JsonifierResult<size_t> JsonifierResult<Value>::countFields() & noexcept {
		if (error()) {
			return error();
		}
		return first.countFields();
	}

	JsonifierResult<Value> JsonifierResult<Value>::at(size_t index) noexcept {
		if (error()) {
			return error();
		}
		return first.at(index);
	}

	JsonifierResult<ArrayIterator> JsonifierResult<Value>::begin() & noexcept {
		if (error()) {
			return error();
		}
		return first.begin();
	}

	JsonifierResult<ArrayIterator> JsonifierResult<Value>::end() & noexcept {
		if (error()) {
			return error();
		}
		return {};
	}

	JsonifierResult<Value> JsonifierResult<Value>::findField(std::string_view key) noexcept {
		if (error()) {
			return error();
		}
		return first.findField(key);
	}

	JsonifierResult<Value> JsonifierResult<Value>::findField(const char* key) noexcept {
		if (error()) {
			return error();
		}
		return first.findField(key);
	}

	JsonifierResult<Value> JsonifierResult<Value>::findFieldUnordered(std::string_view key) noexcept {
		if (error()) {
			return error();
		}
		return first.findFieldUnordered(key);
	}

	JsonifierResult<Value> JsonifierResult<Value>::findFieldUnordered(const char* key) noexcept {
		if (error()) {
			return error();
		}
		return first.findFieldUnordered(key);
	}

	JsonifierResult<Array> JsonifierResult<Value>::getArray() noexcept {
		if (error()) {
			return error();
		}
		return first.getArray();
	}

	JsonifierResult<Object> JsonifierResult<Value>::getObject() noexcept {
		if (error()) {
			return error();
		}
		return first.getObject();
	}

	JsonifierResult<uint64_t> JsonifierResult<Value>::getUint64() noexcept {
		if (error()) {
			return error();
		}
		return first.getUint64();
	}

	JsonifierResult<int64_t> JsonifierResult<Value>::getInt64() noexcept {
		if (error()) {
			return error();
		}
		return first.getInt64();
	}

	JsonifierResult<double> JsonifierResult<Value>::getDouble() noexcept {
		if (error()) {
			return error();
		}
		auto result = first.getDouble();
		return result;
	}

	JsonifierResult<std::string_view> JsonifierResult<Value>::getString() noexcept {
		if (error()) {
			return error();
		}
		return first.getString();
	}

	JsonifierResult<RawJsonString> JsonifierResult<Value>::getRawJsonString() noexcept {
		if (error()) {
			return error();
		}
		return first.getRawJsonString();
	}

	JsonifierResult<bool> JsonifierResult<Value>::getBool() noexcept {
		if (error()) {
			return error();
		}
		return first.getBool();
	}

	JsonifierResult<bool> JsonifierResult<Value>::isNull() noexcept {
		if (error()) {
			return error();
		}
		return first.isNull();
	}

	JsonifierResult<JsonType> JsonifierResult<Value>::type() noexcept {
		if (error()) {
			return error();
		}
		return first.type();
	}

	JsonifierResult<bool> JsonifierResult<Value>::isScalar() noexcept {
		if (error()) {
			return error();
		}
		return first.isScalar();
	}

	JsonifierResult<Value>::operator Array() noexcept(false) {
		if (error()) {
			throw error();
		}
		return first;
	}

	JsonifierResult<Value>::operator Object() noexcept(false) {
		if (error()) {
			throw error();
		}
		return first;
	}

	JsonifierResult<Value>::operator uint64_t() noexcept(false) {
		if (error()) {
			throw error();
		}
		return first;
	}

	JsonifierResult<Value>::operator int64_t() noexcept(false) {
		if (error()) {
			throw error();
		}
		return first;
	}

	JsonifierResult<Value>::operator double() noexcept(false) {
		if (error()) {
			throw error();
		}
		return first;
	}

	JsonifierResult<Value>::operator std::string_view() noexcept(false) {
		if (error()) {
			throw error();
		}
		return first;
	}

	JsonifierResult<Value>::operator RawJsonString() noexcept(false) {
		if (error()) {
			throw error();
		}
		return first;
	}

	JsonifierResult<Value>::operator bool() noexcept(false) {
		if (error()) {
			throw error();
		}
		return first;
	}

	JsonifierResult<std::string_view> JsonifierResult<Value>::rawJsonToken() noexcept {
		if (error()) {
			return error();
		}
		return first.rawJsonToken();
	}

	JsonifierResult<const char*> JsonifierResult<Value>::currentLocation() noexcept {
		if (error()) {
			return error();
		}
		return first.currentLocation();
	}

	JsonifierResult<int32_t> JsonifierResult<Value>::currentDepth() const noexcept {
		if (error()) {
			return error();
		}
		return first.currentDepth();
	}

	JsonifierResult<Value> JsonifierResult<Value>::atPointer(std::string_view jsonPointer) noexcept {
		if (error()) {
			return error();
		}
		return first.atPointer(jsonPointer);
	}

	Value::Value(const ValueIterator& _iterator) noexcept : ValueIterator{ _iterator } {
	}

	Value Value::start(const ValueIterator& iterator) noexcept {
		return iterator;
	}

	Value Value::resume(const ValueIterator& iterator) noexcept {
		return iterator;
	}

	JsonifierResult<Array> Value::getArray() noexcept {
		return Array::start(*this);
	}

	JsonifierResult<Object> Value::getObject() noexcept {
		return Object::start(*this);
	}

	JsonifierResult<Object> Value::startOrResumeObject() noexcept {
		if (this->atStart()) {
			return getObject();
		} else {
			return Object::resume(*this);
		}
	}

	JsonifierResult<RawJsonString> Value::getRawJsonString() noexcept {
		return ValueIterator::getRawJsonString();
	}

	JsonifierResult<std::string_view> Value::getString() noexcept {
		return ValueIterator::getString();
	}

	JsonifierResult<double> Value::getDouble() noexcept {
		return ValueIterator::getDouble();
	}

	JsonifierResult<uint64_t> Value::getUint64() noexcept {
		return ValueIterator::getUint64();
	}

	JsonifierResult<int64_t> Value::getInt64() noexcept {
		return ValueIterator::getInt64();
	}

	JsonifierResult<bool> Value::getBool() noexcept {
		return ValueIterator::getBool();
	}

	JsonifierResult<bool> Value::isNull() noexcept {
		return ValueIterator::isNull();
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
		ValueIterator::moveAtStart();
		return answer;
	}

	JsonifierResult<size_t> Value::countFields() & noexcept {
		JsonifierResult<size_t> answer;
		auto a = getObject();
		answer = a.countFields();
		ValueIterator::moveAtStart();
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

	int64_t iteratorations{};

	JsonifierResult<Value> Value::operator[](const char* key) noexcept {
		auto result = startOrResumeObject()[key];
		return result;
	}

	JsonifierResult<JsonType> Value::type() noexcept {
		return ValueIterator::type();
	}

	JsonifierResult<bool> Value::isScalar() noexcept {
		JsonType thisType;
		auto error = type().get(thisType);
		if (error) {
			return error;
		}
		return !((thisType == JsonType::Array) || (thisType == JsonType::Object));
	}

	std::string_view Value::rawJsonToken() noexcept {
		return std::string_view(reinterpret_cast<const char*>(ValueIterator::peekStart()), ValueIterator::peekStartLength());
	}

	JsonifierResult<const char*> Value::currentLocation() noexcept {
		return ValueIterator::jsonIterator->currentLocation();
	}

	int32_t Value::currentDepth() const noexcept {
		return ValueIterator::jsonIterator->depth();
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