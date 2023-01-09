#pragma once

#include <jsonifier/RawJsonString.hpp>
#include <jsonifier/Document.hpp>
#include <jsonifier/Object.hpp>
#include <jsonifier/Array.hpp>
#include <jsonifier/Document_Impl.hpp>

namespace Jsonifier {

	Document::Document(JsonIterator&& iteratorNew) noexcept : JsonIterator{ std::forward<JsonIterator>(iteratorNew) } {
	}

	Document Document::start(JsonIterator&& iterator) noexcept {
		return Document(std::forward<JsonIterator>(iterator));
	}

	void Document::rewind() noexcept {
		JsonIterator::rewind();
	}

	std::string Document::toDebugString() noexcept {
		return JsonIterator::toString();
	}

	JsonifierResult<const char*> Document::currentLocation() noexcept {
		return JsonIterator::currentLocation();
	}

	int32_t Document::currentDepth() const noexcept {
		return JsonIterator::depth();
	}

	bool Document::isAlive() noexcept {
		return JsonIterator::isAlive();
	}

	ValueIterator Document::resumeValueIterator() noexcept {
		return ValueIterator(this, 1, JsonIterator::rootPosition());
	}

	ValueIterator Document::getRootValueIterator() noexcept {
		return resumeValueIterator();
	}

	JsonifierResult<Object> Document::startOrResumeObject() noexcept {
		if (JsonIterator::atRoot()) {
			return getObject();
		} else {
			return Object::resume(resumeValueIterator());
		}
	}

	JsonifierResult<Value> Document::getValue() noexcept {
		JsonIterator::assertAtDocumentDepth();
		switch (*JsonIterator::peek()) {
			case '[':
			case '{':
				return Value(getRootValueIterator());
			default:
				return Scalar_Document_As_Value;
		}
	}

	JsonifierResult<Array> Document::getArray() & noexcept {
		auto Value = getRootValueIterator();
		return Array::startRoot(Value);
	}

	JsonifierResult<Object> Document::getObject() & noexcept {
		auto Value = getRootValueIterator();
		return Object::startRoot(Value);
	}

	JsonifierResult<uint64_t> Document::getUint64() noexcept {
		return getRootValueIterator().getRootUint64();
	}

	JsonifierResult<int64_t> Document::getInt64() noexcept {
		return getRootValueIterator().getRootInt64();
	}

	JsonifierResult<double> Document::getDouble() noexcept {
		return getRootValueIterator().getRootDouble();
	}

	JsonifierResult<std::string_view> Document::getString() noexcept {
		return getRootValueIterator().getRootString();
	}

	JsonifierResult<RawJsonString> Document::getRawJsonString() noexcept {
		return getRootValueIterator().getRootRawJsonString();
	}

	JsonifierResult<bool> Document::getBool() noexcept {
		return getRootValueIterator().getRootBool();
	}

	JsonifierResult<bool> Document::isNull() noexcept {
		return getRootValueIterator().isRootNull();
	}

	Document::operator Array() & noexcept(false) {
		return getArray();
	}

	Document::operator Object() & noexcept(false) {
		return getObject();
	}

	Document::operator uint64_t() noexcept(false) {
		return getUint64();
	}

	Document::operator int64_t() noexcept(false) {
		return getInt64();
	}

	Document::operator double() noexcept(false) {
		return getDouble();
	}

	Document::operator std::string_view() noexcept(false) {
		return getString();
	}

	Document::operator RawJsonString() noexcept(false) {
		return getRawJsonString();
	}

	Document::operator bool() noexcept(false) {
		return getBool();
	}

	Document::operator Value() noexcept(false) {
		return getValue();
	}

	JsonifierResult<size_t> Document::countElements() & noexcept {
		auto a = getArray();
		JsonifierResult<size_t> answer = a.countElements();
		if (answer.error() == Success) {
			rewind();
		}
		return answer;
	}

	JsonifierResult<size_t> Document::countFields() & noexcept {
		auto a = getObject();
		JsonifierResult<size_t> answer = a.countFields();
		if (answer.error() == Success) {
			rewind();
		}
		return answer;
	}

	JsonifierResult<Value> Document::at(size_t index) & noexcept {
		auto a = getArray();
		return a.at(index);
	}

	JsonifierResult<ArrayIterator> Document::begin() & noexcept {
		return getArray().begin();
	}

	JsonifierResult<ArrayIterator> Document::end() & noexcept {
		return {};
	}

	JsonifierResult<Value> Document::findField(std::string_view key) & noexcept {
		return startOrResumeObject().findField(key);
	}

	JsonifierResult<Value> Document::findField(const char* key) & noexcept {
		return startOrResumeObject().findField(key);
	}

	JsonifierResult<Value> Document::findFieldUnordered(std::string_view key) & noexcept {
		return startOrResumeObject().findFieldUnordered(key);
	}

	JsonifierResult<Value> Document::findFieldUnordered(const char* key) & noexcept {
		return startOrResumeObject().findFieldUnordered(key);
	}

	JsonifierResult<Value> Document::operator[](std::string_view key) & noexcept {
		return startOrResumeObject()[key];
	}

	JsonifierResult<Value> Document::operator[](const char* key) & noexcept {
		return startOrResumeObject()[key];
	}

	ErrorCode Document::consume() noexcept {
		auto error = JsonIterator::skipChild(0);
		if (error) {
			JsonIterator::abandon();
		}
		return error;
	}

	JsonifierResult<std::string_view> Document::rawJson() noexcept {
		auto iteratorNew = getRootValueIterator();
		const uint8_t* startingPoint{ iteratorNew.peekStart() };
		auto error = consume();
		if (error) {
			return error;
		}
		const uint8_t* finalPoint{ JsonIterator::unsafePointer() };
		return std::string_view(reinterpret_cast<const char*>(startingPoint), size_t(finalPoint - startingPoint));
	}

	JsonifierResult<JsonType> Document::type() noexcept {
		return getRootValueIterator().type();
	}

	JsonifierResult<bool> Document::isScalar() noexcept {
		JsonType thisType;
		auto error = type().get(thisType);
		if (error) {
			return error;
		}
		return !((thisType == JsonType::Array) || (thisType == JsonType::Object));
	}

	JsonifierResult<std::string_view> Document::rawJsonToken() noexcept {
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

	JsonifierResult<Document>::JsonifierResult(Document&& Value) noexcept : JsonifierResultBase<Document>(std::forward<Document>(Value)) {
	}

	JsonifierResult<Document>::JsonifierResult(ErrorCode error) noexcept : JsonifierResultBase<Document>(error) {
	}

	JsonifierResult<size_t> JsonifierResult<Document>::countElements() & noexcept {
		if (error()) {
			return error();
		}
		return first.countElements();
	}

	JsonifierResult<size_t> JsonifierResult<Document>::countFields() & noexcept {
		if (error()) {
			return error();
		}
		return first.countFields();
	}

	JsonifierResult<Value> JsonifierResult<Document>::at(size_t index) & noexcept {
		if (error()) {
			return error();
		}
		return first.at(index);
	}

	ErrorCode JsonifierResult<Document>::error() noexcept {
		return JsonifierResultBase<Document>::error();
	}

	ErrorCode JsonifierResult<Document>::rewind() noexcept {
		if (error()) {
			return error();
		}
		first.rewind();
		return Success;
	}

	JsonifierResult<ArrayIterator> JsonifierResult<Document>::begin() & noexcept {
		if (error()) {
			return error();
		}
		return first.begin();
	}

	JsonifierResult<ArrayIterator> JsonifierResult<Document>::end() & noexcept {
		return {};
	}

	JsonifierResult<Value> JsonifierResult<Document>::findFieldUnordered(std::string_view key) & noexcept {
		if (error()) {
			return error();
		}
		return first.findFieldUnordered(key);
	}

	JsonifierResult<Value> JsonifierResult<Document>::findFieldUnordered(const char* key) & noexcept {
		if (error()) {
			return error();
		}
		return first.findFieldUnordered(key);
	}

	JsonifierResult<Value> JsonifierResult<Document>::operator[](std::string_view key) & noexcept {
		if (error()) {
			return error();
		}
		return first[key];
	}

	JsonifierResult<Value> JsonifierResult<Document>::operator[](const char* key) & noexcept {
		if (error()) {
			return error();
		}
		return first[key];
	}

	JsonifierResult<Value> JsonifierResult<Document>::findField(std::string_view key) & noexcept {
		if (error()) {
			return error();
		}
		return first.findField(key);
	}

	JsonifierResult<Value> JsonifierResult<Document>::findField(const char* key) & noexcept {
		if (error()) {
			return error();
		}
		return first.findField(key);
	}

	JsonifierResult<Array> JsonifierResult<Document>::getArray() & noexcept {
		if (error()) {
			return error();
		}
		return first.getArray();
	}

	JsonifierResult<Object> JsonifierResult<Document>::getObject() & noexcept {
		if (error()) {
			return error();
		}
		return first.getObject();
	}

	JsonifierResult<uint64_t> JsonifierResult<Document>::getUint64() noexcept {
		if (error()) {
			return error();
		}
		return first.getUint64();
	}

	JsonifierResult<int64_t> JsonifierResult<Document>::getInt64() noexcept {
		if (error()) {
			return error();
		}
		return first.getInt64();
	}

	JsonifierResult<double> JsonifierResult<Document>::getDouble() noexcept {
		if (error()) {
			return error();
		}
		return first.getDouble();
	}

	JsonifierResult<std::string_view> JsonifierResult<Document>::getString() noexcept {
		if (error()) {
			return error();
		}
		return first.getString();
	}

	JsonifierResult<RawJsonString> JsonifierResult<Document>::getRawJsonString() noexcept {
		if (error()) {
			return error();
		}
		return first.getRawJsonString();
	}

	JsonifierResult<bool> JsonifierResult<Document>::getBool() noexcept {
		if (error()) {
			return error();
		}
		return first.getBool();
	}

	JsonifierResult<Value> JsonifierResult<Document>::getValue() noexcept {
		if (error()) {
			return error();
		}
		return first.getValue();
	}

	JsonifierResult<bool> JsonifierResult<Document>::isNull() noexcept {
		if (error()) {
			return error();
		}
		return first.isNull();
	}

	JsonifierResult<JsonType> JsonifierResult<Document>::type() noexcept {
		if (error()) {
			return error();
		}
		return first.type();
	}

	JsonifierResult<bool> JsonifierResult<Document>::isScalar() noexcept {
		if (error()) {
			return error();
		}
		return first.isScalar();
	}

	JsonifierResult<Document>::operator Array() & noexcept(false) {
		if (error()) {
			throw error();
		}
		return first;
	}

	JsonifierResult<Document>::operator Object() & noexcept(false) {
		if (error()) {
			throw error();
		}
		return first;
	}

	JsonifierResult<Document>::operator uint64_t() noexcept(false) {
		if (error()) {
			throw error();
		}
		return first;
	}

	JsonifierResult<Document>::operator int64_t() noexcept(false) {
		if (error()) {
			throw error();
		}
		return first;
	}

	JsonifierResult<Document>::operator double() noexcept(false) {
		if (error()) {
			throw error();
		}
		return first;
	}

	JsonifierResult<Document>::operator std::string_view() noexcept(false) {
		if (error()) {
			throw error();
		}
		return first;
	}

	JsonifierResult<Document>::operator RawJsonString() noexcept(false) {
		if (error()) {
			throw error();
		}
		return first;
	}

	JsonifierResult<Document>::operator bool() noexcept(false) {
		if (error()) {
			throw error();
		}
		return first;
	}

	JsonifierResult<Document>::operator Value() noexcept(false) {
		if (error()) {
			throw error();
		}
		return first;
	}

	JsonifierResult<const char*> JsonifierResult<Document>::currentLocation() noexcept {
		if (error()) {
			return error();
		}
		return first.currentLocation();
	}

	int32_t JsonifierResult<Document>::currentDepth() noexcept {
		if (error()) {
			return error();
		}
		return first.currentDepth();
	}

	JsonifierResult<std::string_view> JsonifierResult<Document>::rawJsonToken() noexcept {
		if (error()) {
			return error();
		}
		return first.rawJsonToken();
	}

	JsonifierResult<Value> JsonifierResult<Document>::atPointer(std::string_view jsonPointer) noexcept {
		if (error()) {
			return error();
		}
		return first.atPointer(jsonPointer);
	}
}
