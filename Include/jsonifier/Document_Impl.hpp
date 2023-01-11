#pragma once

#include <jsonifier/RawJsonString.hpp>
#include <jsonifier/Document.hpp>
#include <jsonifier/Object.hpp>
#include <jsonifier/Array.hpp>
#include <jsonifier/ArrayIterator.hpp>

namespace Jsonifier {

	template<typename T> __forceinline JsonifierResult<T> Document::get() & noexcept {
		static_assert(!sizeof(T), "The get method with given type is not implemented by the Jsonifier library.");
	}

	template<typename T> __forceinline JsonifierResult<T> Document::get() && noexcept {
		static_assert(!sizeof(T), "The get method with given type is not implemented by the Jsonifier library.");
	}

	template<> __forceinline JsonifierResult<Array> Document::get() & noexcept {
		return getArray();
	}

	template<> __forceinline JsonifierResult<Object> Document::get() & noexcept {
		return getObject();
	}

	template<> __forceinline JsonifierResult<RawJsonString> Document::get() & noexcept {
		return getRawJsonString();
	}

	template<> __forceinline JsonifierResult<std::string_view> Document::get() & noexcept {
		return getString();
	}

	template<> __forceinline JsonifierResult<double> Document::get() & noexcept {
		return getDouble();
	}

	template<> __forceinline JsonifierResult<uint64_t> Document::get() & noexcept {
		return getUint64();
	}

	template<> __forceinline JsonifierResult<int64_t> Document::get() & noexcept {
		return getInt64();
	}

	template<> __forceinline JsonifierResult<bool> Document::get() & noexcept {
		return getBool();
	}

	template<> __forceinline JsonifierResult<Value> Document::get() & noexcept {
		return getValue();
	}

	template<> __forceinline JsonifierResult<RawJsonString> Document::get() && noexcept {
		return getRawJsonString();
	}

	template<> __forceinline JsonifierResult<std::string_view> Document::get() && noexcept {
		return getString();
	}

	template<> __forceinline JsonifierResult<double> Document::get() && noexcept {
		return std::forward<Document>(*this).getDouble();
	}

	template<> __forceinline JsonifierResult<uint64_t> Document::get() && noexcept {
		return std::forward<Document>(*this).getUint64();
	}

	template<> __forceinline JsonifierResult<int64_t> Document::get() && noexcept {
		return std::forward<Document>(*this).getInt64();
	}

	template<> __forceinline JsonifierResult<bool> Document::get() && noexcept {
		return std::forward<Document>(*this).getBool();
	}

	template<> __forceinline JsonifierResult<Value> Document::get() && noexcept {
		return getValue();
	}

	template<typename T> ErrorCode Document::get(T& out) & noexcept {
		return get<T>().get(out);
	}

	template<typename T> ErrorCode Document::get(T& out) && noexcept {
		return std::forward<Document>(*this).get<T>().get(out);
	}

	template<typename T> JsonifierResult<T> JsonifierResult<Document>::get() & noexcept {
		if (error()) {
			return error();
		}
		return first.get<T>();
	}

	template<typename T> JsonifierResult<T> JsonifierResult<Document>::get() && noexcept {
		if (error()) {
			return error();
		}
		return std::forward<Document>(first).get<T>();
	}

	template<typename T> ErrorCode JsonifierResult<Document>::get(T& out) & noexcept {
		if (error()) {
			return error();
		}
		return first.get<T>(out);
	}

	template<typename T> ErrorCode JsonifierResult<Document>::get(T& out) && noexcept {
		if (error()) {
			return error();
		}
		return std::forward<Document>(first).get<T>(out);
	}

	template<> __forceinline JsonifierResult<Document> JsonifierResult<Document>::get<Document>() & noexcept = delete;

	template<> __forceinline JsonifierResult<Document> JsonifierResult<Document>::get<Document>() && noexcept {
		if (error()) {
			return error();
		}
		return std::forward<Document>(first);
	}

	template<> __forceinline ErrorCode JsonifierResult<Document>::get<Document>(Document& out) & noexcept = delete;

	template<> __forceinline ErrorCode JsonifierResult<Document>::get<Document>(Document& out) && noexcept {
		if (error()) {
			return error();
		}
		out = std::forward<Document>(first);
		return Success;
	}

	__forceinline Document::Document(JsonIterator&& iteratorNew) noexcept : iterator{ std::forward<JsonIterator>(iteratorNew) } {};

	__forceinline Document Document::start(JsonIterator&& iterator) noexcept {
		return Document(std::forward<JsonIterator>(iterator));
	}

	__forceinline void Document::rewind() noexcept {
		iterator.rewind();
	}

	__forceinline std::string Document::toDebugString() noexcept {
		return iterator.toString();
	}

	__forceinline JsonifierResult<const char*> Document::currentLocation() noexcept {
		return iterator.currentLocation();
	}

	__forceinline int32_t Document::currentDepth() const noexcept {
		return iterator.depth();
	}

	__forceinline bool Document::isAlive() noexcept {
		return iterator.isAlive();
	}

	__forceinline ValueIterator Document::resumeValueIterator() noexcept {
		return ValueIterator(&this->iterator, 1, iterator.rootPosition());
	}

	__forceinline ValueIterator Document::getRootValueIterator() noexcept {
		return resumeValueIterator();
	}

	__forceinline JsonifierResult<Object> Document::startOrResumeObject() noexcept {
		if (iterator.atRoot()) {
			return getObject();
		} else {
			return Object::resume(resumeValueIterator());
		}
	}

	__forceinline JsonifierResult<Value> Document::getValue() noexcept {
		iterator.assertAtDocumentDepth();
		switch (*iterator.peek()) {
			case '[':
			case '{':
				return Value(getRootValueIterator());
			default:
				return Scalar_Document_As_Value;
		}
	}

	__forceinline JsonifierResult<Array> Document::getArray() & noexcept {
		auto Value = getRootValueIterator();
		return Array::startRoot(Value);
	}

	__forceinline JsonifierResult<Object> Document::getObject() & noexcept {
		auto Value = getRootValueIterator();
		return Object::startRoot(Value);
	}

	__forceinline JsonifierResult<uint64_t> Document::getUint64() noexcept {
		return getRootValueIterator().getRootUint64();
	}

	__forceinline JsonifierResult<int64_t> Document::getInt64() noexcept {
		return getRootValueIterator().getRootInt64();
	}

	__forceinline JsonifierResult<double> Document::getDouble() noexcept {
		return getRootValueIterator().getRootDouble();
	}

	__forceinline JsonifierResult<std::string_view> Document::getString() noexcept {
		return getRootValueIterator().getRootString();
	}

	__forceinline JsonifierResult<RawJsonString> Document::getRawJsonString() noexcept {
		return getRootValueIterator().getRootRawJsonString();
	}

	__forceinline JsonifierResult<bool> Document::getBool() noexcept {
		return getRootValueIterator().getRootBool();
	}

	__forceinline JsonifierResult<bool> Document::isNull() noexcept {
		return getRootValueIterator().isRootNull();
	}

	__forceinline Document::operator Array() & noexcept(false) {
		return getArray();
	}

	__forceinline Document::operator Object() & noexcept(false) {
		return getObject();
	}

	__forceinline Document::operator uint64_t() noexcept(false) {
		return getUint64();
	}

	__forceinline Document::operator int64_t() noexcept(false) {
		return getInt64();
	}

	__forceinline Document::operator double() noexcept(false) {
		return getDouble();
	}

	__forceinline Document::operator std::string_view() noexcept(false) {
		return getString();
	}

	__forceinline Document::operator RawJsonString() noexcept(false) {
		return getRawJsonString();
	}

	__forceinline Document::operator bool() noexcept(false) {
		return getBool();
	}

	__forceinline Document::operator Value() noexcept(false) {
		return getValue();
	}

	__forceinline JsonifierResult<size_t> Document::countElements() & noexcept {
		auto a = getArray();
		JsonifierResult<size_t> answer = a.countElements();
		if (answer.error() == Success) {
			rewind();
		}
		return answer;
	}

	__forceinline JsonifierResult<size_t> Document::countFields() & noexcept {
		auto a = getObject();
		JsonifierResult<size_t> answer = a.countFields();
		if (answer.error() == Success) {
			rewind();
		}
		return answer;
	}

	__forceinline JsonifierResult<Value> Document::at(size_t index) & noexcept {
		auto a = getArray();
		return a.at(index);
	}

	__forceinline JsonifierResult<ArrayIterator> Document::begin() & noexcept {
		return getArray().begin();
	}

	__forceinline JsonifierResult<ArrayIterator> Document::end() & noexcept {
		return {};
	}

	__forceinline JsonifierResult<Value> Document::findField(std::string_view key) & noexcept {
		return startOrResumeObject().findField(key);
	}

	__forceinline JsonifierResult<Value> Document::findField(const char* key) & noexcept {
		return startOrResumeObject().findField(key);
	}

	__forceinline JsonifierResult<Value> Document::findFieldUnordered(std::string_view key) & noexcept {
		return startOrResumeObject().findFieldUnordered(key);
	}

	__forceinline JsonifierResult<Value> Document::findFieldUnordered(const char* key) & noexcept {
		return startOrResumeObject().findFieldUnordered(key);
	}

	__forceinline JsonifierResult<Value> Document::operator[](std::string_view key) & noexcept {
		return startOrResumeObject()[key];
	}

	__forceinline JsonifierResult<Value> Document::operator[](const char* key) & noexcept {
		return startOrResumeObject()[key];
	}

	__forceinline ErrorCode Document::consume() noexcept {
		auto error = iterator.skipChild(0);
		if (error) {
			iterator.abandon();
		}
		return error;
	}

	__forceinline JsonifierResult<std::string_view> Document::rawJson() noexcept {
		auto iteratorNew = getRootValueIterator();
		const uint8_t* startingPoint{ iteratorNew.peekStart() };
		auto error = consume();
		if (error) {
			return error;
		}
		const uint8_t* finalPoint{ iterator.unsafePointer() };
		return std::string_view(reinterpret_cast<const char*>(startingPoint), size_t(finalPoint - startingPoint));
	}

	__forceinline JsonifierResult<JsonType> Document::type() noexcept {
		return getRootValueIterator().type();
	}

	__forceinline JsonifierResult<bool> Document::isScalar() noexcept {
		JsonType thisType;
		auto error = type().get(thisType);
		if (error) {
			return error;
		}
		return !((thisType == JsonType::Array) || (thisType == JsonType::Object));
	}

	__forceinline JsonifierResult<std::string_view> Document::rawJsonToken() noexcept {
		auto iteratorNew = getRootValueIterator();
		return std::string_view(reinterpret_cast<const char*>(iteratorNew.peekStart()), iteratorNew.peekStartLength());
	}

	JsonifierResult<Value> Document::atPointer(std::string_view jsonPointer) noexcept {
		if (jsonPointer.empty()) {
			return this->getValue();
		}
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

	__forceinline JsonifierResult<Document>::JsonifierResult(Document&& Value) noexcept
		: JsonifierResultBase<Document>(std::forward<Document>(Value)){}

	__forceinline JsonifierResult<Document>::JsonifierResult(ErrorCode error) noexcept : JsonifierResultBase<Document>(error){}

	__forceinline JsonifierResult<size_t> JsonifierResult<Document>::countElements() & noexcept {
		if (error()) {
			return error();
		}
		return first.countElements();
	}

	__forceinline JsonifierResult<size_t> JsonifierResult<Document>::countFields() & noexcept {
		if (error()) {
			return error();
		}
		return first.countFields();
	}

	__forceinline JsonifierResult<Value> JsonifierResult<Document>::at(size_t index) & noexcept {
		if (error()) {
			return error();
		}
		return first.at(index);
	}

	__forceinline ErrorCode JsonifierResult<Document>::error() noexcept {
		return JsonifierResultBase<Document>::error();
	}

	__forceinline ErrorCode JsonifierResult<Document>::rewind() noexcept {
		if (error()) {
			return error();
		}
		first.rewind();
		return Success;
	}

	__forceinline JsonifierResult<ArrayIterator> JsonifierResult<Document>::begin() & noexcept {
		if (error()) {
			return error();
		}
		return first.begin();
	}

	__forceinline JsonifierResult<ArrayIterator> JsonifierResult<Document>::end() & noexcept {
		return {};
	}

	__forceinline JsonifierResult<Value> JsonifierResult<Document>::findFieldUnordered(std::string_view key) & noexcept {
		if (error()) {
			return error();
		}
		return first.findFieldUnordered(key);
	}

	__forceinline JsonifierResult<Value> JsonifierResult<Document>::findFieldUnordered(const char* key) & noexcept {
		if (error()) {
			return error();
		}
		return first.findFieldUnordered(key);
	}

	__forceinline JsonifierResult<Value> JsonifierResult<Document>::operator[](std::string_view key) & noexcept {
		if (error()) {
			return error();
		}
		return first[key];
	}

	__forceinline JsonifierResult<Value> JsonifierResult<Document>::operator[](const char* key) & noexcept {
		if (error()) {
			return error();
		}
		return first[key];
	}

	__forceinline JsonifierResult<Value> JsonifierResult<Document>::findField(std::string_view key) & noexcept {
		if (error()) {
			return error();
		}
		return first.findField(key);
	}

	__forceinline JsonifierResult<Value> JsonifierResult<Document>::findField(const char* key) & noexcept {
		if (error()) {
			return error();
		}
		return first.findField(key);
	}

	__forceinline JsonifierResult<Array> JsonifierResult<Document>::getArray() & noexcept {
		if (error()) {
			return error();
		}
		return first.getArray();
	}

	__forceinline JsonifierResult<Object> JsonifierResult<Document>::getObject() & noexcept {
		if (error()) {
			return error();
		}
		return first.getObject();
	}

	__forceinline JsonifierResult<uint64_t> JsonifierResult<Document>::getUint64() noexcept {
		if (error()) {
			return error();
		}
		return first.getUint64();
	}

	__forceinline JsonifierResult<int64_t> JsonifierResult<Document>::getInt64() noexcept {
		if (error()) {
			return error();
		}
		return first.getInt64();
	}

	__forceinline JsonifierResult<double> JsonifierResult<Document>::getDouble() noexcept {
		if (error()) {
			return error();
		}
		return first.getDouble();
	}

	__forceinline JsonifierResult<std::string_view> JsonifierResult<Document>::getString() noexcept {
		if (error()) {
			return error();
		}
		return first.getString();
	}

	__forceinline JsonifierResult<RawJsonString> JsonifierResult<Document>::getRawJsonString() noexcept {
		if (error()) {
			return error();
		}
		return first.getRawJsonString();
	}

	__forceinline JsonifierResult<bool> JsonifierResult<Document>::getBool() noexcept {
		if (error()) {
			return error();
		}
		return first.getBool();
	}

	__forceinline JsonifierResult<Value> JsonifierResult<Document>::getValue() noexcept {
		if (error()) {
			return error();
		}
		return first.getValue();
	}

	__forceinline JsonifierResult<bool> JsonifierResult<Document>::isNull() noexcept {
		if (error()) {
			return error();
		}
		return first.isNull();
	}

	__forceinline JsonifierResult<JsonType> JsonifierResult<Document>::type() noexcept {
		if (error()) {
			return error();
		}
		return first.type();
	}

	__forceinline JsonifierResult<bool> JsonifierResult<Document>::isScalar() noexcept {
		if (error()) {
			return error();
		}
		return first.isScalar();
	}

	__forceinline JsonifierResult<Document>::operator Array() & noexcept(false) {
		if (error()) {
			throw error();
		}
		return first;
	}

	__forceinline JsonifierResult<Document>::operator Object() & noexcept(false) {
		if (error()) {
			throw error();
		}
		return first;
	}

	__forceinline JsonifierResult<Document>::operator uint64_t() noexcept(false) {
		if (error()) {
			throw error();
		}
		return first;
	}

	__forceinline JsonifierResult<Document>::operator int64_t() noexcept(false) {
		if (error()) {
			throw error();
		}
		return first;
	}

	__forceinline JsonifierResult<Document>::operator double() noexcept(false) {
		if (error()) {
			throw error();
		}
		return first;
	}

	__forceinline JsonifierResult<Document>::operator std::string_view() noexcept(false) {
		if (error()) {
			throw error();
		}
		return first;
	}

	__forceinline JsonifierResult<Document>::operator RawJsonString() noexcept(false) {
		if (error()) {
			throw error();
		}
		return first;
	}

	__forceinline JsonifierResult<Document>::operator bool() noexcept(false) {
		if (error()) {
			throw error();
		}
		return first;
	}

	__forceinline JsonifierResult<Document>::operator Value() noexcept(false) {
		if (error()) {
			throw error();
		}
		return first;
	}

	__forceinline JsonifierResult<const char*> JsonifierResult<Document>::currentLocation() noexcept {
		if (error()) {
			return error();
		}
		return first.currentLocation();
	}

	__forceinline int32_t JsonifierResult<Document>::currentDepth() noexcept {
		if (error()) {
			return error();
		}
		return first.currentDepth();
	}

	__forceinline JsonifierResult<std::string_view> JsonifierResult<Document>::rawJsonToken() noexcept {
		if (error()) {
			return error();
		}
		return first.rawJsonToken();
	}

	__forceinline JsonifierResult<Value> JsonifierResult<Document>::atPointer(std::string_view jsonPointer) noexcept {
		if (error()) {
			return error();
		}
		return first.atPointer(jsonPointer);
	}
}
