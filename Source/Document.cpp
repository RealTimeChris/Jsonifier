#include <jsonifier/Document_Impl.hpp>
#include <jsonifier/Value.hpp>
#include <jsonifier/Array.hpp>
#include <jsonifier/Object.hpp>

namespace Jsonifier {

	Document::Document(JsonIterator&& _iterator) noexcept : iterator{ std::forward<JsonIterator>(_iterator) } {
	}

	void Document::rewind() noexcept {
		iterator.rewind();
	}

	JsonifierResult<const char*> Document::currentLocation() noexcept {
		return iterator.currentLocation();
	}

	int32_t Document::currentDepth() const noexcept {
		return iterator.depth();
	}

	bool Document::isAlive() noexcept {
		return iterator.isAlive();
	}

	ValueIterator Document::resumeValueIterator() noexcept {
		return ValueIterator(&iterator, 1ull, iterator.rootPosition());
	}

	ValueIterator Document::getRootValueIterator() noexcept {
		return resumeValueIterator();
	}

	JsonifierResult<Object> Document::startOrResumeObject() noexcept {
		if (iterator.atRoot()) {
			return getObject();
		} else {
			return Object::resume(resumeValueIterator());
		}
	}

	JsonifierResult<Value> Document::getValue() noexcept {
		iterator.assertAtDocumentDepth();
		switch (*iterator.peek()) {
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
		auto error = iterator.skipChild(0);
		if (error) {
			iterator.abandon();
		}
		return error;
	}

	JsonifierResult<std::string_view> Document::rawJson() noexcept {
		auto _iterator = getRootValueIterator();
		const uint8_t* startingPoint{ _iterator.peekStart() };
		auto error = consume();
		if (error) {
			return error;
		}
		const uint8_t* finalPoint{ iterator.unsafePointer() };
		return std::string_view(reinterpret_cast<const char*>(startingPoint), size_t(finalPoint - startingPoint));
	}

	JsonifierResult<JsonType> Document::type() noexcept {
		return getRootValueIterator().type();
	}

	JsonifierResult<bool> Document::isScalar() noexcept {
		JsonType this_type;
		auto error = type().get(this_type);
		if (error) {
			return error;
		}
		return !((this_type == JsonType::Array) || (this_type == JsonType::Object));
	}

	JsonifierResult<std::string_view> Document::rawJsonToken() noexcept {
		auto _iterator = getRootValueIterator();
		return std::string_view(reinterpret_cast<const char*>(_iterator.peekStart()), _iterator.peekStartLength());
	}

	JsonifierResult<Value> Document::atPointer(std::string_view jsonPointer) noexcept {
		rewind();// Rewind the Document each time atPointer is called
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
}