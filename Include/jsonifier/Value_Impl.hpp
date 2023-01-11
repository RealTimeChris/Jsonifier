#pragma once

#include <jsonifier/Field.hpp>
#include <jsonifier/Array.hpp>
#include <jsonifier/Object.hpp>
#include <jsonifier/ArrayIterator.hpp>
#include <jsonifier/ValueIterator.hpp>
#include <jsonifier/RawJsonString.hpp>

namespace Jsonifier {

	__forceinline JsonifierResult<Value>::JsonifierResult(Value&& value) noexcept : JsonifierResultBase<Value>(std::forward<Value>(value)) {
	}

	__forceinline JsonifierResult<Value>::JsonifierResult(ErrorCode error) noexcept : JsonifierResultBase<Value>(error) {
	}

	template<typename T> __forceinline ErrorCode Value::get(T& out) noexcept {
		return get<T>().get(out);
	}

	template<> __forceinline JsonifierResult<Array> Value::get() noexcept {
		return getArray();
	}

	template<> __forceinline JsonifierResult<Object> Value::get() noexcept {
		return getObject();
	}

	template<> __forceinline JsonifierResult<RawJsonString> Value::get() noexcept {
		return getRawJsonString();
	}

	template<> __forceinline JsonifierResult<std::string_view> Value::get() noexcept {
		return getString();
	}

	template<> __forceinline JsonifierResult<double> Value::get() noexcept {
		return getDouble();
	}

	template<> __forceinline JsonifierResult<uint64_t> Value::get() noexcept {
		return getUint64();
	}

	template<> __forceinline JsonifierResult<int64_t> Value::get() noexcept {
		return getInt64();
	}

	template<> __forceinline JsonifierResult<bool> Value::get() noexcept {
		return getBool();
	}

	template<> __forceinline JsonifierResult<Value> JsonifierResult<Value>::get<Value>() noexcept {
		if (error()) {
			return error();
		}
		return std::move(first);
	}

	template<> __forceinline JsonifierResult<Array> JsonifierResult<Value>::get<Array>() noexcept {
		if (error()) {
			return error();
		}
		return std::move(first.getArray());
	}

	template<> __forceinline ErrorCode JsonifierResult<Value>::get<Value>(Value& out) noexcept {
		if (error()) {
			return error();
		}
		out = first;
		return ErrorCode::Success;
	}

	__forceinline JsonifierResult<JsonType> JsonifierResult<Value>::type() noexcept {
		if (error()) {
			return error();
		}
		return first.type();
	}

	template<> __forceinline ErrorCode Value::get<Array>(Array& out) noexcept {
		if (error()) {
			return error();
		}
		out = getArray();
		return ErrorCode::Success;
	}

	template<> __forceinline ErrorCode JsonifierResult<Value>::get<Array>(Array& out) noexcept {
		if (error()) {
			return error();
		}
		out = first;
		return ErrorCode::Success;
	}

	template<typename T> __forceinline ErrorCode JsonifierResult<Value>::get(T& out) noexcept {
		if (error()) {
			return error();
		};
		auto result = first.get<T>(out);
		return result;
	}

	template<typename T> __forceinline JsonifierResult<T> Value::get() noexcept {
		static_assert(!sizeof(T), "The get method with given type is not implemented by the Jsonifier library.");
	}

	JsonifierResult<Value> __forceinline JsonifierResult<Value>::operator[](std::string_view key) noexcept {
		if (error()) {
			return error();
		}
		return first[key];
	}

	JsonifierResult<Value> __forceinline JsonifierResult<Value>::operator[](const char* key) noexcept {
		if (error()) {
			return error();
		}
		return first[key];
	}

	JsonifierResult<size_t> __forceinline JsonifierResult<Value>::countElements() & noexcept {
		if (error()) {
			return error();
		}
		return first.countElements();
	}

	JsonifierResult<size_t> __forceinline JsonifierResult<Value>::countFields() & noexcept {
		if (error()) {
			return error();
		}
		return first.countFields();
	}

	JsonifierResult<Value> __forceinline JsonifierResult<Value>::at(size_t index) noexcept {
		if (error()) {
			return error();
		}
		return first.at(index);
	}

	__forceinline JsonifierResult<ArrayIterator> JsonifierResult<Value>::begin() & noexcept {
		if (error()) {
			return error();
		}
		return first.begin();
	}

	__forceinline JsonifierResult<ArrayIterator> JsonifierResult<Value>::end() & noexcept {
		if (error()) {
			return error();
		}
		return {};
	}

	JsonifierResult<Value> __forceinline JsonifierResult<Value>::findField(std::string_view key) noexcept {
		if (error()) {
			return error();
		}
		return first.findField(key);
	}

	JsonifierResult<Value> __forceinline JsonifierResult<Value>::findField(const char* key) noexcept {
		if (error()) {
			return error();
		}
		return first.findField(key);
	}

	JsonifierResult<Value> __forceinline JsonifierResult<Value>::findFieldUnordered(std::string_view key) noexcept {
		if (error()) {
			return error();
		}
		return first.findFieldUnordered(key);
	}

	JsonifierResult<Value> __forceinline JsonifierResult<Value>::findFieldUnordered(const char* key) noexcept {
		if (error()) {
			return error();
		}
		return first.findFieldUnordered(key);
	}

	__forceinline JsonifierResult<Array> JsonifierResult<Value>::getArray() noexcept {
		if (error()) {
			return error();
		}
		return first.getArray();
	}

	JsonifierResult<Object> __forceinline JsonifierResult<Value>::getObject() noexcept {
		if (error()) {
			return error();
		}
		return first.getObject();
	}

	JsonifierResult<uint64_t> __forceinline JsonifierResult<Value>::getUint64() noexcept {
		if (error()) {
			return error();
		}
		return first.getUint64();
	}

	JsonifierResult<int64_t> __forceinline JsonifierResult<Value>::getInt64() noexcept {
		if (error()) {
			return error();
		}
		return first.getInt64();
	}

	JsonifierResult<double> __forceinline JsonifierResult<Value>::getDouble() noexcept {
		if (error()) {
			return error();
		}
		auto result = first.getDouble();
		return result;
	}

	JsonifierResult<std::string_view> __forceinline JsonifierResult<Value>::getString() noexcept {
		if (error()) {
			return error();
		}
		return first.getString();
	}

	JsonifierResult<RawJsonString> __forceinline JsonifierResult<Value>::getRawJsonString() noexcept {
		if (error()) {
			return error();
		}
		return first.getRawJsonString();
	}

	JsonifierResult<bool> __forceinline JsonifierResult<Value>::getBool() noexcept {
		if (error()) {
			return error();
		}
		return first.getBool();
	}

	JsonifierResult<bool> __forceinline JsonifierResult<Value>::isNull() noexcept {
		if (error()) {
			return error();
		}
		return first.isNull();
	}

	JsonifierResult<bool> __forceinline JsonifierResult<Value>::isScalar() noexcept {
		if (error()) {
			return error();
		}
		return first.isScalar();
	}

	__forceinline JsonifierResult<Value>::operator Array() noexcept(false) {
		if (error()) {
			throw error();
		}
		return first;
	}

	__forceinline JsonifierResult<Value>::operator Object() noexcept(false) {
		if (error()) {
			throw error();
		}
		return first;
	}

	__forceinline JsonifierResult<Value>::operator uint64_t() noexcept(false) {
		if (error()) {
			throw error();
		}
		return first;
	}

	__forceinline JsonifierResult<Value>::operator int64_t() noexcept(false) {
		if (error()) {
			throw error();
		}
		return first;
	}

	__forceinline JsonifierResult<Value>::operator double() noexcept(false) {
		if (error()) {
			throw error();
		}
		return first;
	}

	__forceinline JsonifierResult<Value>::operator std::string_view() noexcept(false) {
		if (error()) {
			throw error();
		}
		return first;
	}

	__forceinline JsonifierResult<Value>::operator RawJsonString() noexcept(false) {
		if (error()) {
			throw error();
		}
		return first;
	}

	__forceinline JsonifierResult<Value>::operator bool() noexcept(false) {
		if (error()) {
			throw error();
		}
		return first;
	}

	__forceinline JsonifierResult<std::string_view> JsonifierResult<Value>::rawJsonToken() noexcept {
		if (error()) {
			return error();
		}
		return first.rawJsonToken();
	}

	__forceinline JsonifierResult<const char*> JsonifierResult<Value>::currentLocation() noexcept {
		if (error()) {
			return error();
		}
		return first.currentLocation();
	}

	__forceinline JsonifierResult<int32_t> JsonifierResult<Value>::currentDepth() const noexcept {
		if (error()) {
			return error();
		}
		return first.currentDepth();
	}

	__forceinline JsonifierResult<Value> JsonifierResult<Value>::atPointer(std::string_view jsonPointer) noexcept {
		if (error()) {
			return error();
		}
		return first.atPointer(jsonPointer);
	}

	__forceinline Value::Value(const ValueIterator& _iterator) noexcept : ValueIterator{ _iterator } {
	}

	__forceinline Value Value::start(const ValueIterator& iterator) noexcept {
		return iterator;
	}

	__forceinline Value Value::resume(const ValueIterator& iterator) noexcept {
		return iterator;
	}

	__forceinline JsonifierResult<Array> Value::getArray() noexcept {
		return Array::start(*this);
	}

	__forceinline JsonifierResult<Object> Value::getObject() noexcept {
		return Object::start(*this);
	}

	__forceinline JsonifierResult<Object> Value::startOrResumeObject() noexcept {
		if (this->atStart()) {
			return getObject();
		} else {
			return Object::resume(*this);
		}
	}

	__forceinline JsonifierResult<RawJsonString> Value::getRawJsonString() noexcept {
		return ValueIterator::getRawJsonString();
	}

	__forceinline JsonifierResult<std::string_view> Value::getString() noexcept {
		return ValueIterator::getString();
	}

	__forceinline JsonifierResult<double> Value::getDouble() noexcept {
		return ValueIterator::getDouble();
	}

	__forceinline JsonifierResult<uint64_t> Value::getUint64() noexcept {
		return ValueIterator::getUint64();
	}

	__forceinline JsonifierResult<int64_t> Value::getInt64() noexcept {
		return ValueIterator::getInt64();
	}

	__forceinline JsonifierResult<bool> Value::getBool() noexcept {
		return ValueIterator::getBool();
	}

	__forceinline JsonifierResult<bool> Value::isNull() noexcept {
		return ValueIterator::isNull();
	}

	__forceinline Value::operator Array() noexcept(false) {
		return getArray();
	}

	__forceinline Value::operator Object() noexcept(false) {
		return getObject();
	}

	__forceinline Value::operator uint64_t() noexcept(false) {
		return getUint64();
	}

	__forceinline Value::operator int64_t() noexcept(false) {
		return getInt64();
	}

	__forceinline Value::operator double() noexcept(false) {
		return getDouble();
	}

	__forceinline Value::operator std::string_view() noexcept(false) {
		return getString();
	}

	__forceinline Value::operator RawJsonString() noexcept(false) {
		return getRawJsonString();
	}

	__forceinline Value::operator bool() noexcept(false) {
		return getBool();
	}

	__forceinline JsonifierResult<ArrayIterator> Value::begin() & noexcept {
		return getArray().begin();
	}

	__forceinline JsonifierResult<ArrayIterator> Value::end() & noexcept {
		return {};
	}

	__forceinline JsonifierResult<size_t> Value::countElements() & noexcept {
		JsonifierResult<size_t> answer;
		auto a = getArray();
		answer = a.countElements();
		ValueIterator::moveAtStart();
		return answer;
	}

	__forceinline JsonifierResult<size_t> Value::countFields() & noexcept {
		JsonifierResult<size_t> answer;
		auto a = getObject();
		answer = a.countFields();
		ValueIterator::moveAtStart();
		return answer;
	}

	__forceinline JsonifierResult<Value> Value::at(size_t index) noexcept {
		auto a = getArray();
		return a.at(index);
	}

	__forceinline JsonifierResult<Value> Value::findField(std::string_view key) noexcept {
		return startOrResumeObject().findField(key);
	}

	__forceinline JsonifierResult<Value> Value::findField(const char* key) noexcept {
		return startOrResumeObject().findField(key);
	}

	__forceinline JsonifierResult<Value> Value::findFieldUnordered(std::string_view key) noexcept {
		return startOrResumeObject().findFieldUnordered(key);
	}

	__forceinline JsonifierResult<Value> Value::findFieldUnordered(const char* key) noexcept {
		return startOrResumeObject().findFieldUnordered(key);
	}

	__forceinline JsonifierResult<Value> Value::operator[](std::string_view key) noexcept {
		return startOrResumeObject()[key];
	}

	__forceinline JsonifierResult<Value> Value::operator[](const char* key) noexcept {
		auto result = startOrResumeObject()[key];
		return result;
	}

	__forceinline JsonifierResult<JsonType> Value::type() noexcept {
		return ValueIterator::type();
	}

	__forceinline JsonifierResult<bool> Value::isScalar() noexcept {
		JsonType thisType{};
		auto error = type().get(thisType);
		if (error) {
			return error;
		}
		return !((thisType == JsonType::Array) || (thisType == JsonType::Object));
	}

	__forceinline std::string_view Value::rawJsonToken() noexcept {
		return std::string_view(reinterpret_cast<const char*>(ValueIterator::peekStart()), ValueIterator::peekStartLength());
	}

	__forceinline JsonifierResult<const char*> Value::currentLocation() noexcept {
		return ValueIterator::jsonIterator->currentLocation();
	}

	__forceinline int32_t Value::currentDepth() const noexcept {
		return ValueIterator::jsonIterator->depth();
	}

	__forceinline JsonifierResult<Value> Value::atPointer(std::string_view jsonPointer) noexcept {
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