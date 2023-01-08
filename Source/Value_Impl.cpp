#pragma once

#include <jsonifier/ImplementationJsonifierResultBase_Impl.hpp>
#include <jsonifier/Field_Impl.hpp>
#include <jsonifier/Array_Impl.hpp>
#include <jsonifier/Value.hpp>
#include <jsonifier/Object_Impl.hpp>
#include <jsonifier/RawJsonString.hpp>

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

	inline JsonifierResult<Value> Value::atPointer(std::string_view json_pointer) noexcept {
		JsonType t{};
		JsonifierTry(type().get(t));
		switch (t) {
			case JsonType::Array:
				return (*this).getArray().atPointer(json_pointer);
			case JsonType::Object:
				return (*this).getObject().atPointer(json_pointer);
			default:
				return Invalid_Json_Pointer;
		}
	}

	inline JsonifierResult<Array> Value::getArray() noexcept {
		return Array::start(iterator);
	}

	inline Value::Value(const ValueIterator& _iter) noexcept : iterator{ _iter } {
	}

	inline JsonifierResult<JsonType> Value::type() noexcept {
		return iterator.type();
	}

	inline JsonifierResult<Object> Value::getObject() noexcept {
		return Object::start(iterator);
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
}