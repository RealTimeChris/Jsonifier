 #pragma once

#include <jsonifier/ImplementationJsonifierResultBase_Impl.hpp>
#include <jsonifier/Value_Impl.hpp>
#include <jsonifier/Array_Impl.hpp>
#include <jsonifier/Object_Impl.hpp>

namespace Jsonifier {

	inline JsonifierResult<Document>::JsonifierResult(Document&& Value) noexcept
		: ImplementationJsonifierResultBase<Document>(std::forward<Document>(Value)) {
	}

	inline JsonifierResult<Document>::JsonifierResult(ErrorCode error) noexcept : ImplementationJsonifierResultBase<Document>(error) {
	}

	inline JsonifierResult<size_t> JsonifierResult<Document>::countElements() & noexcept {
		if (error()) {
			return error();
		}
		return first.countElements();
	}

	inline JsonifierResult<size_t> JsonifierResult<Document>::countFields() & noexcept {
		if (error()) {
			return error();
		}
		return first.countFields();
	}

	inline JsonifierResult<Value> JsonifierResult<Document>::at(size_t index) & noexcept {
		if (error()) {
			return error();
		}
		return first.at(index);
	}

	inline ErrorCode JsonifierResult<Document>::rewind() noexcept {
		if (error()) {
			return error();
		}
		first.rewind();
		return ErrorCode::Success;
	}

	inline JsonifierResult<ArrayIterator> JsonifierResult<Document>::begin() & noexcept {
		if (error()) {
			return error();
		}
		return first.begin();
	}

	inline JsonifierResult<ArrayIterator> JsonifierResult<Document>::end() & noexcept {
		return {};
	}

	inline JsonifierResult<Value> JsonifierResult<Document>::findFieldUnordered(std::string_view key) & noexcept {
		if (error()) {
			return error();
		}
		return first.findFieldUnordered(key);
	}

	inline JsonifierResult<Value> JsonifierResult<Document>::findFieldUnordered(const char* key) & noexcept {
		if (error()) {
			return error();
		}
		return first.findFieldUnordered(key);
	}

	inline JsonifierResult<Value> JsonifierResult<Document>::operator[](std::string_view key) & noexcept {
		if (error()) {
			return error();
		}
		return first[key];
	}

	inline JsonifierResult<Value> JsonifierResult<Document>::operator[](const char* key) & noexcept {
		if (error()) {
			return error();
		}
		return first[key];
	}

	inline JsonifierResult<Value> JsonifierResult<Document>::findField(std::string_view key) & noexcept {
		if (error()) {
			return error();
		}
		return first.findField(key);
	}

	inline JsonifierResult<Value> JsonifierResult<Document>::findField(const char* key) & noexcept {
		if (error()) {
			return error();
		}
		return first.findField(key);
	}

	inline JsonifierResult<Array> JsonifierResult<Document>::getArray() & noexcept {
		if (error()) {
			return error();
		}
		return first.getArray();
	}

	template<> inline JsonifierResult<Array> Document::get() & noexcept {
		return getArray();
	}

	template<> inline JsonifierResult<Object> Document::get() & noexcept {
		return getObject();
	}

	template<> inline JsonifierResult<RawJsonString> Document::get() & noexcept {
		return getRawJsonString();
	}

	template<> inline JsonifierResult<std::string_view> Document::get() & noexcept {
		return getString();
	}

	template<> inline JsonifierResult<double> Document::get() & noexcept {
		return getDouble();
	}

	template<> inline JsonifierResult<uint64_t> Document::get() & noexcept {
		return getUint64();
	}

	template<> inline JsonifierResult<int64_t> Document::get() & noexcept {
		return getInt64();
	}

	template<> inline JsonifierResult<bool> Document::get() & noexcept {
		return getBool();
	}

	template<> inline JsonifierResult<Value> Document::get() & noexcept {
		return getValue();
	}

	template<> inline JsonifierResult<RawJsonString> Document::get() && noexcept {
		return getRawJsonString();
	}

	template<> inline JsonifierResult<std::string_view> Document::get() && noexcept {
		return getString();
	}

	template<> inline JsonifierResult<double> Document::get() && noexcept {
		return std::forward<Document>(*this).getDouble();
	}

	template<> inline JsonifierResult<uint64_t> Document::get() && noexcept {
		return std::forward<Document>(*this).getUint64();
	}

	template<> inline JsonifierResult<int64_t> Document::get() && noexcept {
		return std::forward<Document>(*this).getInt64();
	}

	template<> inline JsonifierResult<bool> Document::get() && noexcept {
		return std::forward<Document>(*this).getBool();
	}

	template<> inline JsonifierResult<Value> Document::get() && noexcept {
		return getValue();
	}

	template<typename T> inline ErrorCode Document::get(T& out) & noexcept {
		return get<T>().get(out);
	}

	template<typename T> inline ErrorCode Document::get(T& out) && noexcept {
		return std::forward<Document>(*this).get<T>().get(out);
	}

	inline JsonifierResult<Object> JsonifierResult<Document>::getObject() & noexcept {
		if (error()) {
			return error();
		}
		return first.getObject();
	}

	inline JsonifierResult<uint64_t> JsonifierResult<Document>::getUint64() noexcept {
		if (error()) {
			return error();
		}
		return first.getUint64();
	}

	inline JsonifierResult<int64_t> JsonifierResult<Document>::getInt64() noexcept {
		if (error()) {
			return error();
		}
		return first.getInt64();
	}

	inline JsonifierResult<double> JsonifierResult<Document>::getDouble() noexcept {
		if (error()) {
			return error();
		}
		return first.getDouble();
	}

	inline JsonifierResult<std::string_view> JsonifierResult<Document>::getString() noexcept {
		if (error()) {
			return error();
		}
		return first.getString();
	}

	inline JsonifierResult<RawJsonString> JsonifierResult<Document>::getRawJsonString() noexcept {
		if (error()) {
			return error();
		}
		return first.getRawJsonString();
	}

	inline JsonifierResult<bool> JsonifierResult<Document>::getBool() noexcept {
		if (error()) {
			return error();
		}
		return first.getBool();
	}

	inline JsonifierResult<Value> JsonifierResult<Document>::getValue() noexcept {
		if (error()) {
			return error();
		}
		return first.getValue();
	}

	inline JsonifierResult<bool> JsonifierResult<Document>::isNull() noexcept {
		if (error()) {
			return error();
		}
		return first.isNull();
	}

	template<typename T> inline JsonifierResult<T> JsonifierResult<Document>::get() & noexcept {
		if (error()) {
			return error();
		}
		return first.get<T>();
	}

	template<typename T> inline JsonifierResult<T> JsonifierResult<Document>::get() && noexcept {
		if (error()) {
			return error();
		}
		return std::forward<Document>(first).get<T>();
	}

	template<typename T> ErrorCode inline JsonifierResult<Document>::get(T& out) & noexcept {
		if (error()) {
			return error();
		}
		return first.get<T>(out);
	}

	template<typename T> ErrorCode inline JsonifierResult<Document>::get(T& out) && noexcept {
		if (error()) {
			return error();
		}
		return std::forward<Document>(first).get<T>(out);
	}

	template<> inline JsonifierResult<Document> JsonifierResult<Document>::get<Document>() & noexcept = delete;
	template<> inline JsonifierResult<Document> JsonifierResult<Document>::get<Document>() && noexcept {
		if (error()) {
			return error();
		}
		return std::forward<Document>(first);
	}

	template<> inline ErrorCode JsonifierResult<Document>::get<Document>(Document& out) & noexcept = delete;
	template<> inline ErrorCode JsonifierResult<Document>::get<Document>(Document& out) && noexcept {
		if (error()) {
			return error();
		}
		out = std::forward<Document>(first);
		return ErrorCode::Success;
	}

	inline JsonifierResult<JsonType> JsonifierResult<Document>::type() noexcept {
		if (error()) {
			return error();
		}
		return first.type();
	}

	inline JsonifierResult<bool> JsonifierResult<Document>::isScalar() noexcept {
		if (error()) {
			return error();
		}
		return first.isScalar();
	}

	inline JsonifierResult<Document>::operator Array() & noexcept(false) {
		if (error()) {
			throw error();
		}
		return first;
	}

	inline JsonifierResult<Document>::operator Object() & noexcept(false) {
		if (error()) {
			throw error();
		}
		return first;
	}

	inline JsonifierResult<Document>::operator uint64_t() noexcept(false) {
		if (error()) {
			throw error();
		}
		return first;
	}

	inline JsonifierResult<Document>::operator int64_t() noexcept(false) {
		if (error()) {
			throw error();
		}
		return first;
	}

	inline JsonifierResult<Document>::operator double() noexcept(false) {
		if (error()) {
			throw error();
		}
		return first;
	}

	inline JsonifierResult<Document>::operator std::string_view() noexcept(false) {
		if (error()) {
			throw error();
		}
		return first;
	}

	inline JsonifierResult<Document>::operator RawJsonString() noexcept(false) {
		if (error()) {
			throw error();
		}
		return first;
	}

	inline JsonifierResult<Document>::operator bool() noexcept(false) {
		if (error()) {
			throw error();
		}
		return first;
	}

	inline JsonifierResult<Document>::operator Value() noexcept(false) {
		if (error()) {
			throw error();
		}
		return first;
	}

	inline JsonifierResult<const char*> JsonifierResult<Document>::currentLocation() noexcept {
		if (error()) {
			return error();
		}
		return first.currentLocation();
	}

	inline int32_t JsonifierResult<Document>::currentDepth() const noexcept {
		if (error()) {
			return error();
		}
		return first.currentDepth();
	}

	inline JsonifierResult<std::string_view> JsonifierResult<Document>::rawJsonToken() noexcept {
		if (error()) {
			return error();
		}
		return first.rawJsonToken();
	}

	inline JsonifierResult<Value> JsonifierResult<Document>::atPointer(std::string_view jsonPointer) noexcept {
		if (error()) {
			return error();
		}
		return first.atPointer(jsonPointer);
	}
}