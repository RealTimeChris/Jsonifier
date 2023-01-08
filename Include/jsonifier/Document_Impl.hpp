#pragma once

#include <jsonifier/Document.hpp>
#include <jsonifier/Object.hpp>
#include <jsonifier/Array.hpp>

namespace Jsonifier {

	inline Document::Document(JsonIterator&& iteratorNew) noexcept : JsonIterator{ std::forward<JsonIterator>(iteratorNew) } {}

	inline Document Document::start(JsonIterator&& iterator) noexcept {
		return Document(std::forward<JsonIterator>(iterator));
	}

	inline void Document::rewind() noexcept {
		JsonIterator::rewind();
	}

	inline std::string Document::toDebugString() noexcept {
		return JsonIterator::toString();
	}

	inline JsonifierResult<const char*> Document::currentLocation() noexcept {
		return JsonIterator::currentLocation();
	}

	inline int32_t Document::currentDepth() const noexcept {
		return JsonIterator::depth();
	}

	inline bool Document::isAlive() noexcept {
		return JsonIterator::isAlive();
	}

	inline ValueIterator Document::resumeValueIterator() noexcept {
		return ValueIterator(this, 1, JsonIterator::rootPosition());
	}

	inline ValueIterator Document::getRootValueIterator() noexcept {
		return resumeValueIterator();
	}

	inline JsonifierResult<Object> Document::startOrResumeObject() noexcept {
		if (JsonIterator::atRoot()) {
			return getObject();
		} else {
			return Object::resume(resumeValueIterator());
		}
	}

	inline JsonifierResult<Value> Document::getValue() noexcept {
		JsonIterator::assertAtDocumentDepth();
		switch (*JsonIterator::peek()) {
			case '[':
			case '{':
				return Value(getRootValueIterator());
			default:
				return Scalar_Document_As_Value;
		}
	}

	inline JsonifierResult<Array> Document::getArray() & noexcept {
		auto Value = getRootValueIterator();
		return Array::startRoot(Value);
	}

	inline JsonifierResult<Object> Document::getObject() & noexcept {
		auto Value = getRootValueIterator();
		return Object::startRoot(Value);
	}

	inline JsonifierResult<uint64_t> Document::getUint64() noexcept {
		return getRootValueIterator().getRootUint64();
	}

	inline JsonifierResult<int64_t> Document::getInt64() noexcept {
		return getRootValueIterator().getRootInt64();
	}

	inline JsonifierResult<double> Document::getDouble() noexcept {
		return getRootValueIterator().getRootDouble();
	}

	inline JsonifierResult<std::string_view> Document::getString() noexcept {
		return getRootValueIterator().getRootString();
	}

	inline JsonifierResult<RawJsonString> Document::getRawJsonString() noexcept {
		return getRootValueIterator().getRootRawJsonString();
	}

	inline JsonifierResult<bool> Document::getBool() noexcept {
		return getRootValueIterator().getRootBool();
	}

	inline JsonifierResult<bool> Document::isNull() noexcept {
		return getRootValueIterator().isRootNull();
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

	inline Document::operator Array() & noexcept(false) {
		return getArray();
	}

	inline Document::operator Object() & noexcept(false) {
		return getObject();
	}

	inline Document::operator uint64_t() noexcept(false) {
		return getUint64();
	}

	inline Document::operator int64_t() noexcept(false) {
		return getInt64();
	}

	inline Document::operator double() noexcept(false) {
		return getDouble();
	}

	inline Document::operator std::string_view() noexcept(false) {
		return getString();
	}

	inline Document::operator RawJsonString() noexcept(false) {
		return getRawJsonString();
	}

	inline Document::operator bool() noexcept(false) {
		return getBool();
	}

	inline Document::operator Value() noexcept(false) {
		return getValue();
	}

	inline JsonifierResult<size_t> Document::countElements() & noexcept {
		auto a = getArray();
		JsonifierResult<size_t> answer = a.countElements();
		if (answer.error() == Success) {
			rewind();
		}
		return answer;
	}

	inline JsonifierResult<size_t> Document::countFields() & noexcept {
		auto a = getObject();
		JsonifierResult<size_t> answer = a.countFields();
		if (answer.error() == Success) {
			rewind();
		}
		return answer;
	}

	inline JsonifierResult<Value> Document::at(size_t index) & noexcept {
		auto a = getArray();
		return a.at(index);
	}

	inline JsonifierResult<ArrayIterator> Document::begin() & noexcept {
		return getArray().begin();
	}

	inline JsonifierResult<ArrayIterator> Document::end() & noexcept {
		return {};
	}

	inline JsonifierResult<Value> Document::findField(std::string_view key) & noexcept {
		return startOrResumeObject().findField(key);
	}

	inline JsonifierResult<Value> Document::findField(const char* key) & noexcept {
		return startOrResumeObject().findField(key);
	}

	inline JsonifierResult<Value> Document::findFieldUnordered(std::string_view key) & noexcept {
		return startOrResumeObject().findFieldUnordered(key);
	}

	inline JsonifierResult<Value> Document::findFieldUnordered(const char* key) & noexcept {
		return startOrResumeObject().findFieldUnordered(key);
	}

	inline JsonifierResult<Value> Document::operator[](std::string_view key) & noexcept {
		return startOrResumeObject()[key];
	}

	inline JsonifierResult<Value> Document::operator[](const char* key) & noexcept {
		return startOrResumeObject()[key];
	}

	inline ErrorCode Document::consume() noexcept {
		auto error = JsonIterator::skipChild(0);
		if (error) {
			JsonIterator::abandon();
		}
		return error;
	}

	inline JsonifierResult<std::string_view> Document::rawJson() noexcept {
		auto iteratorNew = getRootValueIterator();
		const uint8_t* startingPoint{ iteratorNew.peekStart() };
		auto error = consume();
		if (error) {
			return error;
		}
		const uint8_t* finalPoint{ JsonIterator::unsafePointer() };
		return std::string_view(reinterpret_cast<const char*>(startingPoint), size_t(finalPoint - startingPoint));
	}

	inline JsonifierResult<JsonType> Document::type() noexcept {
		return getRootValueIterator().type();
	}

	inline JsonifierResult<bool> Document::isScalar() noexcept {
		JsonType thisType;
		auto error = type().get(thisType);
		if (error) {
			return error;
		}
		return !((thisType == JsonType::Array) || (thisType == JsonType::Object));
	}

	inline JsonifierResult<std::string_view> Document::rawJsonToken() noexcept {
		auto iteratorNew = getRootValueIterator();
		return std::string_view(reinterpret_cast<const char*>(iteratorNew.peekStart()), iteratorNew.peekStartLength());
	}

	inline JsonifierResult<Value> Document::atPointer(std::string_view jsonPointer) noexcept {
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

	inline JsonifierResult<Document>::JsonifierResult(Document&& Value) noexcept
		: ImplementationJsonifierResultBase<Document>(std::forward<Document>(Value)){}

	inline JsonifierResult<Document>::JsonifierResult(ErrorCode error) noexcept : ImplementationJsonifierResultBase<Document>(error){}

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
		return Success;
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
	template<typename T> inline ErrorCode JsonifierResult<Document>::get(T& out) & noexcept {
		if (error()) {
			return error();
		}
		return first.get<T>(out);
	}
	template<typename T> inline ErrorCode JsonifierResult<Document>::get(T& out) && noexcept {
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
		return Success;
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
