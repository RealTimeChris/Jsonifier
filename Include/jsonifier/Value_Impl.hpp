#pragma once

#include <jsonifier/Field_Impl.hpp>
#include <jsonifier/Array_Impl.hpp>
#include <jsonifier/Object_Impl.hpp>
#include <jsonifier/Value.hpp>
#include <jsonifier/ValueIterator.hpp>

namespace Jsonifier {

	inline JsonifierResult<Value>::JsonifierResult(Value&& value) noexcept : JsonifierResultBase<Value>(std::forward<Value>(value)) {
	}

	inline JsonifierResult<Value>::JsonifierResult(ErrorCode error) noexcept : JsonifierResultBase<Value>(error) {
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
		};
		auto result = first.get<T>(out);
		return result;
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

	inline Value::Value(const ValueIterator& _iterator) noexcept : ValueIterator{ _iterator } {
	}

	inline Value Value::start(const ValueIterator& iterator) noexcept {
		return iterator;
	}

	inline Value Value::resume(const ValueIterator& iterator) noexcept {
		return iterator;
	}

	inline JsonifierResult<Array> Value::getArray() noexcept {
		return Array::start(*this);
	}

	inline JsonifierResult<Object> Value::getObject() noexcept {
		return Object::start(*this);
	}

	inline JsonifierResult<Object> Value::startOrResumeObject() noexcept {
		if (this->atStart()) {
			return getObject();
		} else {
			return Object::resume(*this);
		}
	}

	inline JsonifierResult<RawJsonString> Value::getRawJsonString() noexcept {
		return ValueIterator::getRawJsonString();
	}

	inline JsonifierResult<std::string_view> Value::getString() noexcept {
		return ValueIterator::getString();
	}

	inline JsonifierResult<double> Value::getDouble() noexcept {
		return ValueIterator::getDouble();
	}

	inline JsonifierResult<uint64_t> Value::getUint64() noexcept {
		return ValueIterator::getUint64();
	}

	inline JsonifierResult<int64_t> Value::getInt64() noexcept {
		return ValueIterator::getInt64();
	}

	inline JsonifierResult<bool> Value::getBool() noexcept {
		return ValueIterator::getBool();
	}

	inline JsonifierResult<bool> Value::isNull() noexcept {
		return ValueIterator::isNull();
	}

	inline Value::operator Array() noexcept(false) {
		return getArray();
	}

	inline Value::operator Object() noexcept(false) {
		return getObject();
	}

	inline Value::operator uint64_t() noexcept(false) {
		return getUint64();
	}

	inline Value::operator int64_t() noexcept(false) {
		return getInt64();
	}

	inline Value::operator double() noexcept(false) {
		return getDouble();
	}

	inline Value::operator std::string_view() noexcept(false) {
		return getString();
	}

	inline Value::operator RawJsonString() noexcept(false) {
		return getRawJsonString();
	}

	inline Value::operator bool() noexcept(false) {
		return getBool();
	}

	inline JsonifierResult<ArrayIterator> Value::begin() & noexcept {
		return getArray().begin();
	}

	inline JsonifierResult<ArrayIterator> Value::end() & noexcept {
		return {};
	}

	inline JsonifierResult<size_t> Value::countElements() & noexcept {
		JsonifierResult<size_t> answer;
		auto a = getArray();
		answer = a.countElements();
		ValueIterator::moveAtStart();
		return answer;
	}

	inline JsonifierResult<size_t> Value::countFields() & noexcept {
		JsonifierResult<size_t> answer;
		auto a = getObject();
		answer = a.countFields();
		ValueIterator::moveAtStart();
		return answer;
	}

	inline JsonifierResult<Value> Value::at(size_t index) noexcept {
		auto a = getArray();
		return a.at(index);
	}

	inline JsonifierResult<Value> Value::findField(std::string_view key) noexcept {
		return startOrResumeObject().findField(key);
	}

	inline JsonifierResult<Value> Value::findField(const char* key) noexcept {
		return startOrResumeObject().findField(key);
	}

	inline JsonifierResult<Value> Value::findFieldUnordered(std::string_view key) noexcept {
		return startOrResumeObject().findFieldUnordered(key);
	}

	inline JsonifierResult<Value> Value::findFieldUnordered(const char* key) noexcept {
		return startOrResumeObject().findFieldUnordered(key);
	}

	inline JsonifierResult<Value> Value::operator[](std::string_view key) noexcept {
		return startOrResumeObject()[key];
	}

	inline int64_t iteratorations{};

	inline JsonifierResult<Value> Value::operator[](const char* key) noexcept {
		auto result = startOrResumeObject()[key];
		return result;
	}

	inline JsonifierResult<JsonType> Value::type() noexcept {
		return ValueIterator::type();
	}

	inline JsonifierResult<bool> Value::isScalar() noexcept {
		JsonType thisType;
		auto error = type().get(thisType);
		if (error) {
			return error;
		}
		return !((thisType == JsonType::Array) || (thisType == JsonType::Object));
	}

	inline std::string_view Value::rawJsonToken() noexcept {
		return std::string_view(reinterpret_cast<const char*>(ValueIterator::peekStart()), ValueIterator::peekStartLength());
	}

	inline JsonifierResult<const char*> Value::currentLocation() noexcept {
		return ValueIterator::jsonIterator->currentLocation();
	}

	inline int32_t Value::currentDepth() const noexcept {
		return ValueIterator::jsonIterator->depth();
	}

	inline JsonifierResult<Value> Value::atPointer(std::string_view jsonPointer) noexcept {
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