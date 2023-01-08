#pragma once

#include <jsonifier/ValueIterator.hpp>

namespace Jsonifier {

	inline ValueIterator::ValueIterator(JsonIterator* jsonIter, uint32_t depth, uint32_t* startPosition) noexcept
		: jsonIterator{ jsonIter }, currentDepth{ depth }, rootStructural{ startPosition } {
	}

	inline JsonifierResult<bool> ValueIterator::startObject() noexcept {
		JsonifierTry(startContainer('{', "Not an Object", "Object"));
		return startedObject();
	}

	inline JsonifierResult<bool> ValueIterator::startRootObject() noexcept {
		JsonifierTry(startContainer('{', "Not an Object", "Object"));
		return startedRootObject();
	}

	inline JsonifierResult<bool> ValueIterator::startedObject() noexcept {
		assertAtContainerStart();
		if (*jsonIterator->peek() == '}') {
			jsonIterator->returnCurrentAndAdvance();
			endContainer();
			return false;
		}
		return true;
	}

	inline JsonifierResult<bool> ValueIterator::startedRootObject() noexcept {
		return startedObject();
	}

	inline ErrorCode ValueIterator::endContainer() noexcept {
		jsonIterator->ascendTo(depth() - 1);
		return Success;
	}

	inline JsonifierResult<bool> ValueIterator::hasNextField() noexcept {
		assertAtNext();
		switch (*jsonIterator->returnCurrentAndAdvance()) {
			case '}':
				JsonifierTry(endContainer());
				return false;
			case ',':
				return true;
			default:
				return reportError(Tape_Error, "Missing comma between Object fields");
		}
	}

	inline JsonifierResult<bool> ValueIterator::findFieldRaw(const std::string_view key) noexcept {
		ErrorCode error{};
		bool hasValue{};
		if (atFirstField()) {
			hasValue = true;
		} else if (!isOpen()) {
			return false;
		} else {
			if ((error = skipChild())) {
				abandon();
				return error;
			}
			if ((error = hasNextField().get(hasValue))) {
				abandon();
				return error;
			}
		}
		while (hasValue) {
			RawJsonString actualKey{};
			if ((error = fieldKey().get(actualKey))) {
				abandon();
				return error;
			};
			if ((error = fieldValue())) {
				abandon();
				return error;
			}
			if (actualKey.unsafeIsEqual(key)) {
				return true;
			}
			JsonifierTry(skipChild());
			if ((error = hasNextField().get(hasValue))) {
				abandon();
				return error;
			}
		}
		return false;
	}

	inline JsonifierResult<bool> ValueIterator::findFieldUnorderedRaw(const std::string_view key) noexcept {
		ErrorCode error{};
		bool hasValue{};
		uint32_t* search_start = jsonIterator->position();

		bool atFirst = atFirstField();
		if (atFirst) {
			hasValue = true;
		} else if (!isOpen()) {
			JsonifierTry(resetObject().get(hasValue));
			atFirst = true;
		} else {
			if ((error = skipChild())) {
				abandon();
				return error;
			}
			search_start = jsonIterator->position();
			if ((error = hasNextField().get(hasValue))) {
				abandon();
				return error;
			}
		}
		while (hasValue) {
			assert(jsonIterator->currentDepth == currentDepth);
			RawJsonString actualKey{};
			if ((error = fieldKey().get(actualKey))) {
				abandon();
				return error;
			};
			if ((error = fieldValue())) {
				abandon();
				return error;
			}
			if (actualKey.unsafeIsEqual(key)) {
				return true;
			}
			JsonifierTry(skipChild());
			if ((error = hasNextField().get(hasValue))) {
				abandon();
				return error;
			}
		}
		if (atFirst) {
			return false;
		}
		JsonifierTry(resetObject().get(hasValue));
		while (true) {
			assert(hasValue);
			assert(jsonIterator->currentDepth == currentDepth);
			RawJsonString actualKey{};
			error = fieldKey().get(actualKey);
			assert(!error);
			error = fieldValue();
			assert(!error);
			if (actualKey.unsafeIsEqual(key)) {
				return true;
			}
			JsonifierTry(skipChild());
			if (jsonIterator->position() == search_start) {
				return false;
			}
			error = hasNextField().get(hasValue);
			assert(!error);
		}
		return false;
	}

	inline JsonifierResult<RawJsonString> ValueIterator::fieldKey() noexcept {
		assertAtNext();

		const uint8_t* key = jsonIterator->returnCurrentAndAdvance();
		if (*(key++) != '"') {
			return reportError(Tape_Error, "Object key is not a string");
		}
		return RawJsonString(key);
	}

	inline ErrorCode ValueIterator::fieldValue() noexcept {
		assertAtNext();

		if (*jsonIterator->returnCurrentAndAdvance() != ':') {
			return reportError(Tape_Error, "Missing colon in Object Field");
		}
		jsonIterator->descendTo(depth() + 1);
		return Success;
	}

	inline JsonifierResult<bool> ValueIterator::startArray() noexcept {
		JsonifierTry(startContainer('[', "Not an Array", "Array"));
		return startedArray();
	}

	inline JsonifierResult<bool> ValueIterator::startRootArray() noexcept {
		JsonifierTry(startContainer('[', "Not an Array", "Array"));
		return startedRootArray();
	}

	inline std::string ValueIterator::toString() const noexcept {
		auto answer = std::string("ValueIterator [ depth : ") + std::to_string(currentDepth) + std::string(", ");
		if (jsonIterator != nullptr) {
			answer += jsonIterator->toString();
		}
		answer += std::string(" ]");
		return answer;
	}

	inline JsonifierResult<bool> ValueIterator::startedArray() noexcept {
		assertAtContainerStart();
		if (*jsonIterator->peek() == ']') {
			jsonIterator->returnCurrentAndAdvance();
			JsonifierTry(endContainer());
			return false;
		}
		jsonIterator->descendTo(depth() + 1);
		return true;
	}

	inline JsonifierResult<bool> ValueIterator::startedRootArray() noexcept {
		if (*jsonIterator->peekLast() != ']') {
			jsonIterator->abandon();
			return reportError(Incomplete_Array_Or_Object, "missing ] at end");
		}
		if ((*jsonIterator->peek(jsonIterator->endPosition()) == ']') && (!jsonIterator->balanced())) {
			jsonIterator->abandon();
			return reportError(Incomplete_Array_Or_Object, "the Document is unbalanced");
		}
		return startedArray();
	}

	inline JsonifierResult<bool> ValueIterator::hasNextElement() noexcept {
		assertAtNext();

		switch (*jsonIterator->returnCurrentAndAdvance()) {
			case ']':
				JsonifierTry(endContainer());
				return false;
			case ',':
				jsonIterator->descendTo(depth() + 1);
				return true;
			default:
				return reportError(Tape_Error, "Missing comma between Array elements");
		}
	}

	inline JsonifierResult<bool> ValueIterator::parseBool(const uint8_t* json) const noexcept {
		auto not_true = str4ncmp(json, "true");
		auto not_false = str4ncmp(json, "fals") | (json[4] ^ 'e');
		bool error = (not_true && not_false) || isNotStructuralOrWhitespace(json[not_true ? 5 : 4]);
		if (error) {
			return incorrect_type_error("Not a boolean");
		}
		return JsonifierResult<bool>(!not_true);
	}

	inline JsonifierResult<bool> ValueIterator::parseNull(const uint8_t* json) const noexcept {
		bool is_null_string = !str4ncmp(json, "null") && isStructuralOrWhitespace(json[4]);
		if (!is_null_string && json[0] == 'n') {
			return incorrect_type_error("Not a null but starts with n");
		}
		return is_null_string;
	}

	inline JsonifierResult<std::string_view> ValueIterator::getString() noexcept {
		return getRawJsonString().unescape(jsonIter());
	}

	inline JsonifierResult<RawJsonString> ValueIterator::getRawJsonString() noexcept {
		auto json = peekScalar("string");
		if (*json != '"') {
			return incorrect_type_error("Not a string");
		}
		advanceScalar("string");
		return RawJsonString(json + 1);
	}

	inline JsonifierResult<uint64_t> ValueIterator::getUint64() noexcept {
		auto result = parseUnsigned(peekNonRootScalar("uint64"));
		if (result.error() == Success) {
			advanceNonRootScalar("uint64");
		}
		return result;
	}

	inline JsonifierResult<int64_t> ValueIterator::getInt64() noexcept {
		auto result = parseInteger(peekNonRootScalar("int64"));
		if (result.error() == Success) {
			advanceNonRootScalar("int64");
		}
		return result;
	}

	inline JsonifierResult<double> ValueIterator::getDouble() noexcept {
		auto result = parseDouble(peekNonRootScalar("double"));
		if (result.error() == Success) {
			advanceNonRootScalar("double");
		}
		return result;
	}

	inline JsonifierResult<bool> ValueIterator::getBool() noexcept {
		auto result = parseBool(peekNonRootScalar("bool"));
		if (result.error() == Success) {
			advanceNonRootScalar("bool");
		}
		return result;
	}

	inline JsonifierResult<bool> ValueIterator::isNull() noexcept {
		bool is_null_value;
		JsonifierTry(parseNull(peekNonRootScalar("null")).get(is_null_value));
		if (is_null_value) {
			advanceNonRootScalar("null");
		}
		return is_null_value;
	}

	inline JsonifierResult<std::string_view> ValueIterator::getRootString() noexcept {
		return getString();
	}

	inline JsonifierResult<RawJsonString> ValueIterator::getRootRawJsonString() noexcept {
		return getRawJsonString();
	}

	inline JsonifierResult<uint64_t> ValueIterator::getRootUint64() noexcept {
		auto max_len = peekStartLength();
		auto json = peekRootScalar("uint64");
		uint8_t tmpbuf[20 + 1];
		if (!jsonIterator->copyToBuffer(json, max_len, tmpbuf)) {
			return NUMBER_ERROR;
		}
		auto result = parseUnsigned(tmpbuf);
		if (result.error() == Success) {
			if (!jsonIterator->isSingleToken()) {
				return Trailing_Content;
			}
			advanceRootScalar("uint64");
		}
		return result;
	}

	inline JsonifierResult<int64_t> ValueIterator::getRootInt64() noexcept {
		auto max_len = peekStartLength();
		auto json = peekRootScalar("int64");
		uint8_t tmpbuf[20 + 1];
		if (!jsonIterator->copyToBuffer(json, max_len, tmpbuf)) {
			return NUMBER_ERROR;
		}

		auto result = parseInteger(tmpbuf);
		if (result.error() == Success) {
			if (!jsonIterator->isSingleToken()) {
				return Trailing_Content;
			}
			advanceRootScalar("int64");
		}
		return result;
	}

	inline JsonifierResult<double> ValueIterator::getRootDouble() noexcept {
		auto max_len = peekStartLength();
		auto json = peekRootScalar("double");
		uint8_t tmpbuf[1074 + 8 + 1];
		if (!jsonIterator->copyToBuffer(json, max_len, tmpbuf)) {
			return NUMBER_ERROR;
		}
		auto result = parseDouble(tmpbuf);
		if (result.error() == Success) {
			if (!jsonIterator->isSingleToken()) {
				return Trailing_Content;
			}
			advanceRootScalar("double");
		}
		return result;
	}

	inline JsonifierResult<bool> ValueIterator::getRootBool() noexcept {
		auto max_len = peekStartLength();
		auto json = peekRootScalar("bool");
		uint8_t tmpbuf[5 + 1];
		if (!jsonIterator->copyToBuffer(json, max_len, tmpbuf)) {
			return incorrect_type_error("Not a boolean");
		}
		auto result = parseBool(tmpbuf);
		if (result.error() == Success) {
			if (!jsonIterator->isSingleToken()) {
				return Trailing_Content;
			}
			advanceRootScalar("bool");
		}
		return result;
	}

	inline bool ValueIterator::isRootNull() noexcept {
		if (!jsonIterator->isSingleToken()) {
			return false;
		}
		auto max_len = peekStartLength();
		auto json = peekRootScalar("null");
		bool result = (max_len >= 4 && !str4ncmp(json, "null") && (max_len == 4 || isStructuralOrWhitespace(json[5])));
		if (result) {
			advanceRootScalar("null");
		}
		return result;
	}

	inline ErrorCode ValueIterator::skipChild() noexcept {
		assert(jsonIterator->TokenIterator::currentPosition > rootStructural);
		assert(jsonIterator->currentDepth >= currentDepth);

		return jsonIterator->skipChild(depth());
	}

	inline ValueIterator ValueIterator::child() const noexcept {
		assertAtChild();
		return { jsonIterator, static_cast<size_t>(depth() + 1), jsonIterator->TokenIterator::position() };
	}

	inline bool ValueIterator::isOpen() const noexcept {
		return jsonIterator->depth() >= depth();
	}

	inline bool ValueIterator::atEnd() const noexcept {
		return jsonIterator->atEnd();
	}

	inline bool ValueIterator::atStart() const noexcept {
		return jsonIterator->TokenIterator::position() == startPosition();
	}

	inline bool ValueIterator::atFirstField() const noexcept {
		assert(jsonIterator->TokenIterator::currentPosition > rootStructural);
		return jsonIterator->TokenIterator::position() == startPosition() + 1;
	}

	inline void ValueIterator::abandon() noexcept {
		jsonIterator->abandon();
	}

	inline int32_t ValueIterator::depth() const noexcept {
		return currentDepth;
	}

	inline ErrorCode ValueIterator::error() const noexcept {
		return jsonIterator->error;
	}

	inline uint8_t*& ValueIterator::stringBufLoc() noexcept {
		return jsonIterator->stringBufLoc();
	}

	inline const JsonIterator& ValueIterator::jsonIter() const noexcept {
		return *jsonIterator;
	}

	inline JsonIterator& ValueIterator::jsonIter() noexcept {
		return *jsonIterator;
	}

	inline const uint8_t* ValueIterator::peekStart() const noexcept {
		return jsonIterator->peek(startPosition());
	}

	inline uint32_t ValueIterator::peekStartLength() const noexcept {
		return jsonIterator->peekLength(startPosition());
	}

	inline const uint8_t* ValueIterator::peekScalar(const char* type) noexcept {
		if (!isAtStart()) {
			return peekStart();
		}

		assertAtStart();
		return jsonIterator->peek();
	}

	inline void ValueIterator::advanceScalar(const char* type) noexcept {
		if (!isAtStart()) {
			return;
		}

		assertAtStart();
		jsonIterator->returnCurrentAndAdvance();
		jsonIterator->ascendTo(depth() - 1);
	}

	inline ErrorCode ValueIterator::startContainer(uint8_t start_char, const char* incorrect_type_message, const char* type) noexcept {
		const uint8_t* json;
		if (!isAtStart()) {
			json = peekStart();
			if (*json != start_char) {
				return incorrect_type_error(incorrect_type_message);
			}
		} else {
			assertAtStart();
			json = jsonIterator->peek();
			if (*json != start_char) {
				return incorrect_type_error(incorrect_type_message);
			}
			jsonIterator->returnCurrentAndAdvance();
		}


		return Success;
	}

	inline const uint8_t* ValueIterator::peekRootScalar(const char* type) noexcept {
		if (!isAtStart()) {
			return peekStart();
		}

		assertAtRoot();
		return jsonIterator->peek();
	}

	inline const uint8_t* ValueIterator::peekNonRootScalar(const char* type) noexcept {
		if (!isAtStart()) {
			return peekStart();
		}

		assertAtNonRootStart();
		return jsonIterator->peek();
	}

	inline void ValueIterator::advanceRootScalar(const char* type) noexcept {
		if (!isAtStart()) {
			return;
		}

		assertAtRoot();
		jsonIterator->returnCurrentAndAdvance();
		jsonIterator->ascendTo(depth() - 1);
	}

	inline void ValueIterator::advanceNonRootScalar(const char* type) noexcept {
		if (!isAtStart()) {
			return;
		}

		assertAtNonRootStart();
		jsonIterator->returnCurrentAndAdvance();
		jsonIterator->ascendTo(depth() - 1);
	}

	inline ErrorCode ValueIterator::incorrect_type_error(const char* message) const noexcept {
		return Incorrect_Type;
	}

	inline bool ValueIterator::isAtStart() const noexcept {
		return position() == startPosition();
	}

	inline bool ValueIterator::isAtKey() const noexcept {
		return currentDepth == jsonIterator->currentDepth && *jsonIterator->peek() == '"';
	}

	inline bool ValueIterator::isAtIteratorStart() const noexcept {
		auto delta = position() - startPosition();
		return delta == 1 || delta == 2;
	}

	inline void ValueIterator::assertAtStart() const noexcept {
		assert(jsonIterator->TokenIterator::currentPosition == rootStructural);
		assert(jsonIterator->currentDepth == currentDepth);
		assert(currentDepth > 0);
	}

	inline void ValueIterator::assertAtContainerStart() const noexcept {
		assert(jsonIterator->TokenIterator::currentPosition == rootStructural + 1);
		assert(jsonIterator->currentDepth == currentDepth);
		assert(currentDepth > 0);
	}

	inline void ValueIterator::assertAtNext() const noexcept {
		assert(jsonIterator->TokenIterator::currentPosition > rootStructural);
		assert(jsonIterator->currentDepth == currentDepth);
		assert(currentDepth > 0);
	}

	inline void ValueIterator::moveAtStart() noexcept {
		jsonIterator->currentDepth = currentDepth;
		jsonIterator->TokenIterator::setPosition(rootStructural);
	}

	inline void ValueIterator::moveAtContainerStart() noexcept {
		jsonIterator->currentDepth = currentDepth;
		jsonIterator->TokenIterator::setPosition(rootStructural + 1);
	}

	inline JsonifierResult<bool> ValueIterator::resetArray() noexcept {
		moveAtContainerStart();
		return startedArray();
	}

	inline JsonifierResult<bool> ValueIterator::resetObject() noexcept {
		moveAtContainerStart();
		return startedObject();
	}

	inline void ValueIterator::assertAtChild() const noexcept {
		assert(jsonIterator->TokenIterator::currentPosition > rootStructural);
		assert(jsonIterator->currentDepth == currentDepth + 1);
		assert(currentDepth > 0);
	}

	inline void ValueIterator::assertAtRoot() const noexcept {
		assertAtStart();
		assert(currentDepth == 1);
	}

	inline void ValueIterator::assertAtNonRootStart() const noexcept {
		assertAtStart();
		assert(currentDepth > 1);
	}

	inline void ValueIterator::assertIsValid() const noexcept {
		assert(jsonIterator != nullptr);
	}

	inline bool ValueIterator::isValid() const noexcept {
		return jsonIterator != nullptr;
	}

	inline JsonifierResult<JsonType> ValueIterator::type() const noexcept {
		switch (*peekStart()) {
			case '{':
				return JsonType::Object;
			case '[':
				return JsonType::Array;
			case '"':
				return JsonType::String;
			case 'n':
				return JsonType::Null;
			case 't':
			case 'f':
				return JsonType::Bool;
			case '-':
			case '0':
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
			case '9':
				return JsonType::Number;
			default:
				return Tape_Error;
		}
	}

	inline uint32_t* ValueIterator::startPosition() const noexcept {
		return rootStructural;
	}

	inline uint32_t* ValueIterator::position() const noexcept {
		return jsonIterator->position();
	}

	inline uint32_t* ValueIterator::endPosition() const noexcept {
		return jsonIterator->endPosition();
	}

	inline uint32_t* ValueIterator::lastPosition() const noexcept {
		return jsonIterator->lastPosition();
	}

	inline ErrorCode ValueIterator::reportError(ErrorCode error, const char* message) noexcept {
		return jsonIterator->reportError(error, message);
	}

	inline JsonifierResult<ValueIterator>::JsonifierResult(ValueIterator&& Value) noexcept
		: ImplementationJsonifierResultBase<ValueIterator>(std::forward<ValueIterator>(Value)){}

	inline JsonifierResult<ValueIterator>::JsonifierResult(ErrorCode error) noexcept : ImplementationJsonifierResultBase<ValueIterator>(error){}

}
