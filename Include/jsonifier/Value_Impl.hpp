#pragma once

#include <jsonifier/ImplementationJsonifierResultBase_Impl.hpp>
#include <jsonifier/Field_Impl.hpp>
#include <jsonifier/Array_Impl.hpp>
#include <jsonifier/Object_Impl.hpp>

namespace Jsonifier {

	inline JsonifierResult<Value>::JsonifierResult(Value&& value) noexcept : ImplementationJsonifierResultBase<Value>(std::forward<Value>(value)) {
	}

	inline JsonifierResult<Value>::JsonifierResult(ErrorCode error) noexcept : ImplementationJsonifierResultBase<Value>(error) {
	}

	inline JsonifierResult<size_t> JsonifierResult<Value>::countElements() & noexcept {
		if (error()) {
			return error();
		}
		return first.countElements();
	}

	inline JsonifierResult<size_t> JsonifierResult<Value>::countFields() & noexcept {
		if (error()) {
			return error();
		}
		return first.countFields();
	}

	inline JsonifierResult<Value> JsonifierResult<Value>::at(size_t index) noexcept {
		if (error()) {
			return error();
		}
		return first.at(index);
	}

	inline JsonifierResult<ArrayIterator> JsonifierResult<Value>::begin() & noexcept {
		if (error()) {
			return error();
		}
		return first.begin();
	}

	inline JsonifierResult<ArrayIterator> JsonifierResult<Value>::end() & noexcept {
		if (error()) {
			return error();
		}
		return {};
	}

	inline JsonifierResult<Value> JsonifierResult<Value>::findField(std::string_view key) noexcept {
		if (error()) {
			return error();
		}
		return first.findField(key);
	}

	inline JsonifierResult<Value> JsonifierResult<Value>::findField(const char* key) noexcept {
		if (error()) {
			return error();
		}
		return first.findField(key);
	}

	inline JsonifierResult<Value> JsonifierResult<Value>::findFieldUnordered(std::string_view key) noexcept {
		if (error()) {
			return error();
		}
		return first.findFieldUnordered(key);
	}

	inline JsonifierResult<Value> JsonifierResult<Value>::findFieldUnordered(const char* key) noexcept {
		if (error()) {
			return error();
		}
		return first.findFieldUnordered(key);
	}

	inline JsonifierResult<Value> JsonifierResult<Value>::operator[](std::string_view key) noexcept {
		if (error()) {
			return error();
		}
		return first[key];
	}

	inline JsonifierResult<Value> JsonifierResult<Value>::operator[](const char* key) noexcept {
		if (error()) {
			return error();
		}
		return first[key];
	}

	inline JsonifierResult<Array> JsonifierResult<Value>::getArray() noexcept {
		if (error()) {
			return error();
		}
		return first.getArray();
	}

	inline JsonifierResult<Object> JsonifierResult<Value>::getObject() noexcept {
		if (error()) {
			return error();
		}
		return first.getObject();
	}

	inline JsonifierResult<uint64_t> JsonifierResult<Value>::getUint64() noexcept {
		if (error()) {
			return error();
		}
		return first.getUint64();
	}

	inline JsonifierResult<int64_t> JsonifierResult<Value>::getInt64() noexcept {
		if (error()) {
			return error();
		}
		return first.getInt64();
	}

	inline JsonifierResult<double> JsonifierResult<Value>::getDouble() noexcept {
		if (error()) {
			return error();
		}
		auto result = first.getDouble();
		return result;
	}

	inline JsonifierResult<std::string_view> JsonifierResult<Value>::getString() noexcept {
		if (error()) {
			return error();
		}
		return first.getString();
	}

	inline JsonifierResult<RawJsonString> JsonifierResult<Value>::getRawJsonString() noexcept {
		if (error()) {
			return error();
		}
		return first.getRawJsonString();
	}

	inline JsonifierResult<bool> JsonifierResult<Value>::getBool() noexcept {
		if (error()) {
			return error();
		}
		return first.getBool();
	}

	inline JsonifierResult<bool> JsonifierResult<Value>::isNull() noexcept {
		if (error()) {
			return error();
		}
		return first.isNull();
	}

	template<typename T> inline JsonifierResult<T> JsonifierResult<Value>::get() noexcept {
		if (error()) {
			return error();
		}
		return first.get<T>();
	}

	template<typename T> inline ErrorCode JsonifierResult<Value>::get(T& out) noexcept {
		if (error()) {
			return error();
		}
		return first.get<T>(out);
	}

	template<> inline JsonifierResult<Value> JsonifierResult<Value>::get<Value>() noexcept {
		if (error()) {
			return error();
		}
		return std::move(first);
	}

	template<> inline ErrorCode JsonifierResult<Value>::get<Value>(Value& out) noexcept {
		if (error()) {
			return error();
		}
		out = first;
		return ErrorCode::Success;
	}

	inline JsonifierResult<JsonType> JsonifierResult<Value>::type() noexcept {
		if (error()) {
			return error();
		}
		return first.type();
	}

	inline JsonifierResult<bool> JsonifierResult<Value>::isScalar() noexcept {
		if (error()) {
			return error();
		}
		return first.isScalar();
	}

	inline JsonifierResult<Value>::operator Array() noexcept(false) {
		if (error()) {
			throw error();
		}
		return first;
	}

	inline JsonifierResult<Value>::operator Object() noexcept(false) {
		if (error()) {
			throw error();
		}
		return first;
	}

	inline JsonifierResult<Value>::operator uint64_t() noexcept(false) {
		if (error()) {
			throw error();
		}
		return first;
	}

	inline JsonifierResult<Value>::operator int64_t() noexcept(false) {
		if (error()) {
			throw error();
		}
		return first;
	}

	inline JsonifierResult<Value>::operator double() noexcept(false) {
		if (error()) {
			throw error();
		}
		return first;
	}

	inline JsonifierResult<Value>::operator std::string_view() noexcept(false) {
		if (error()) {
			throw error();
		}
		return first;
	}

	inline JsonifierResult<Value>::operator RawJsonString() noexcept(false) {
		if (error()) {
			throw error();
		}
		return first;
	}

	inline JsonifierResult<Value>::operator bool() noexcept(false) {
		if (error()) {
			throw error();
		}
		return first;
	}

	inline JsonifierResult<std::string_view> JsonifierResult<Value>::rawJsonToken() noexcept {
		if (error()) {
			return error();
		}
		return first.rawJsonToken();
	}

	inline JsonifierResult<const char*> JsonifierResult<Value>::currentLocation() noexcept {
		if (error()) {
			return error();
		}
		return first.currentLocation();
	}

	inline JsonifierResult<int32_t> JsonifierResult<Value>::currentDepth() const noexcept {
		if (error()) {
			return error();
		}
		return first.currentDepth();
	}

	inline JsonifierResult<Value> JsonifierResult<Value>::atPointer(std::string_view jsonPointer) noexcept {
		if (error()) {
			return error();
		}
		return first.atPointer(jsonPointer);
	}

	template<> inline JsonifierResult<Array> Value::get() noexcept {
		return getArray();
	}

	template<> inline JsonifierResult<Object> Value::get() noexcept {
		return getObject();
	}

	template<> inline JsonifierResult<RawJsonString> Value::get() noexcept {
		return getRawJsonString();
	}

	template<> inline JsonifierResult<std::string_view> Value::get() noexcept {
		return getString();
	}

	template<> inline JsonifierResult<double> Value::get() noexcept {
		return getDouble();
	}

	template<> inline JsonifierResult<uint64_t> Value::get() noexcept {
		return getUint64();
	}

	template<> inline JsonifierResult<int64_t> Value::get() noexcept {
		return getInt64();
	}

	template<> inline JsonifierResult<bool> Value::get() noexcept {
		return getBool();
	}

	template<typename T> inline ErrorCode Value::get(T& out) noexcept {
		return get<T>().get(out);
	}

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
		if (iterator.at_start()) {
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
		iterator.move_at_start();
		return answer;
	}

	JsonifierResult<size_t> Value::countFields() & noexcept {
		JsonifierResult<size_t> answer;
		auto a = getObject();
		answer = a.countFields();
		iterator.move_at_start();
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
		return std::string_view(reinterpret_cast<const char*>(iterator.peek_start()), iterator.peek_start_length());
	}

	JsonifierResult<const char*> Value::currentLocation() noexcept {
		return iterator.jsonIterator->currentLocation();
	}

	int32_t Value::currentDepth() const noexcept {
		return iterator.jsonIterator->depth();
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