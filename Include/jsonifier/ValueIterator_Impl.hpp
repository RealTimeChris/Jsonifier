#pragma once

#include <jsonifier/ValueIterator.hpp>
#include <jsonifier/RawJsonString.hpp>

namespace Jsonifier {

	ValueIterator::ValueIterator(JsonIterator* jsonIter, uint32_t depth, uint32_t* startPosition) noexcept
		: jsonIterator{ jsonIter }, currentDepth{ depth }, rootStructural{ startPosition } {
	}

	JsonifierResult<bool> ValueIterator::startObject() noexcept {
		JsonifierTry(startContainer('{'));
		return startedObject();
	}

	JsonifierResult<bool> ValueIterator::startRootObject() noexcept {
		JsonifierTry(startContainer('{'));
		return startedRootObject();
	}

	JsonifierResult<bool> ValueIterator::startedObject() noexcept {
		assertAtContainerStart();
		if (*jsonIterator->peek() == '}') {
			jsonIterator->returnCurrentAndAdvance();
			endContainer();
			return false;
		}
		return true;
	}

	JsonifierResult<bool> ValueIterator::startedRootObject() noexcept {
		return startedObject();
	}

	ErrorCode ValueIterator::endContainer() noexcept {
		jsonIterator->ascendTo(depth() - 1);
		return Success;
	}

	JsonifierResult<bool> ValueIterator::hasNextField() noexcept {
		assertAtNext();
		switch (*jsonIterator->returnCurrentAndAdvance()) {
			case '}':
				JsonifierTry(endContainer());
				return false;
			case ',':
				return true;
			default:
				return reportError(Tape_Error);
		}
	}

	JsonifierResult<bool> ValueIterator::findFieldRaw(const std::string_view key) noexcept {
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

	JsonifierResult<bool> ValueIterator::findFieldUnorderedRaw(const std::string_view key) noexcept {
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

	JsonifierResult<RawJsonString> ValueIterator::fieldKey() noexcept {
		assertAtNext();

		const uint8_t* key = jsonIterator->returnCurrentAndAdvance();
		if (*(key++) != '"') {
			return reportError(Tape_Error);
		}
		return RawJsonString(key);
	}

	ErrorCode ValueIterator::fieldValue() noexcept {
		assertAtNext();

		if (*jsonIterator->returnCurrentAndAdvance() != ':') {
			return reportError(Tape_Error);
		}
		jsonIterator->descendTo(depth() + 1);
		return Success;
	}

	JsonifierResult<bool> ValueIterator::startArray() noexcept {
		JsonifierTry(startContainer('['));
		return startedArray();
	}

	JsonifierResult<bool> ValueIterator::startRootArray() noexcept {
		JsonifierTry(startContainer('['));
		return startedRootArray();
	}

	std::string ValueIterator::toString() const noexcept {
		auto answer = std::string("ValueIterator [ depth : ") + std::to_string(currentDepth) + std::string(", ");
		if (jsonIterator != nullptr) {
			answer += jsonIterator->toString();
		}
		answer += std::string(" ]");
		return answer;
	}

	JsonifierResult<bool> ValueIterator::startedArray() noexcept {
		assertAtContainerStart();
		if (*jsonIterator->peek() == ']') {
			jsonIterator->returnCurrentAndAdvance();
			JsonifierTry(endContainer());
			return false;
		}
		jsonIterator->descendTo(depth() + 1);
		return true;
	}

	JsonifierResult<bool> ValueIterator::startedRootArray() noexcept {
		if (*jsonIterator->peekLast() != ']') {
			jsonIterator->abandon();
			return reportError(Incomplete_Array_Or_Object);
		}
		if ((*jsonIterator->peek(jsonIterator->endPosition()) == ']') && (!jsonIterator->balanced())) {
			jsonIterator->abandon();
			return reportError(Incomplete_Array_Or_Object);
		}
		return startedArray();
	}

	JsonifierResult<bool> ValueIterator::hasNextElement() noexcept {
		assertAtNext();

		switch (*jsonIterator->returnCurrentAndAdvance()) {
			case ']':
				JsonifierTry(endContainer());
				return false;
			case ',':
				jsonIterator->descendTo(depth() + 1);
				return true;
			default:
				return reportError(Tape_Error);
		}
	}

	JsonifierResult<bool> ValueIterator::parseBool(const uint8_t* json) const noexcept {
		auto not_true = str4ncmp(json, "true");
		auto not_false = str4ncmp(json, "fals") | (json[4] ^ 'e');
		bool error = (not_true && not_false) || isNotStructuralOrWhitespace(json[not_true ? 5 : 4]);
		if (error) {
			return incorrectTypeError();
		}
		return JsonifierResult<bool>(!not_true);
	}

	JsonifierResult<bool> ValueIterator::parseNull(const uint8_t* json) const noexcept {
		bool is_null_string = !str4ncmp(json, "null") && isStructuralOrWhitespace(json[4]);
		if (!is_null_string && json[0] == 'n') {
			return incorrectTypeError();
		}
		return is_null_string;
	}

	JsonifierResult<std::string_view> ValueIterator::getString() noexcept {
		return getRawJsonString().unescape(jsonIter());
	}

	JsonifierResult<RawJsonString> ValueIterator::getRawJsonString() noexcept {
		auto json = peekScalar();
		if (*json != '"') {
			return incorrectTypeError();
		}
		advanceScalar();
		return RawJsonString(json + 1);
	}

	JsonifierResult<uint64_t> ValueIterator::getUint64() noexcept {
		auto result = parseUnsigned(peekNonRootScalar());
		if (result.error() == Success) {
			advanceNonRootScalar();
		}
		return result;
	}

	JsonifierResult<int64_t> ValueIterator::getInt64() noexcept {
		auto result = parseInteger(peekNonRootScalar());
		if (result.error() == Success) {
			advanceNonRootScalar();
		}
		return result;
	}

	JsonifierResult<double> ValueIterator::getDouble() noexcept {
		auto result = parseDouble(peekNonRootScalar());
		if (result.error() == Success) {
			advanceNonRootScalar();
		}
		return result;
	}

	JsonifierResult<bool> ValueIterator::getBool() noexcept {
		auto result = parseBool(peekNonRootScalar());
		if (result.error() == Success) {
			advanceNonRootScalar();
		}
		return result;
	}

	JsonifierResult<bool> ValueIterator::isNull() noexcept {
		bool is_null_value;
		JsonifierTry(parseNull(peekNonRootScalar()).get(is_null_value));
		if (is_null_value) {
			advanceNonRootScalar();
		}
		return is_null_value;
	}

	JsonifierResult<std::string_view> ValueIterator::getRootString() noexcept {
		return getString();
	}

	JsonifierResult<RawJsonString> ValueIterator::getRootRawJsonString() noexcept {
		return getRawJsonString();
	}

	JsonifierResult<uint64_t> ValueIterator::getRootUint64() noexcept {
		auto max_len = peekStartLength();
		auto json = peekRootScalar();
		uint8_t tmpbuf[20 + 1];
		if (!jsonIterator->copyToBuffer(json, max_len, tmpbuf)) {
			return NUMBER_ERROR;
		}
		auto result = parseUnsigned(tmpbuf);
		if (result.error() == Success) {
			if (!jsonIterator->isSingleToken()) {
				return Trailing_Content;
			}
			advanceRootScalar();
		}
		return result;
	}

	JsonifierResult<int64_t> ValueIterator::getRootInt64() noexcept {
		auto max_len = peekStartLength();
		auto json = peekRootScalar();
		uint8_t tmpbuf[20 + 1];
		if (!jsonIterator->copyToBuffer(json, max_len, tmpbuf)) {
			return NUMBER_ERROR;
		}

		auto result = parseInteger(tmpbuf);
		if (!jsonIterator->isSingleToken()) {
			return Trailing_Content;
		}
		advanceRootScalar();
		return result;
	}

	JsonifierResult<double> ValueIterator::getRootDouble() noexcept {
		auto max_len = peekStartLength();
		auto json = peekRootScalar();
		uint8_t tmpbuf[1074 + 8 + 1];
		if (!jsonIterator->copyToBuffer(json, max_len, tmpbuf)) {
			return NUMBER_ERROR;
		}
		auto result = parseDouble(tmpbuf);
		if (result.error() == Success) {
			if (!jsonIterator->isSingleToken()) {
				return Trailing_Content;
			}
			advanceRootScalar();
		}
		return result;
	}

	JsonifierResult<bool> ValueIterator::getRootBool() noexcept {
		auto max_len = peekStartLength();
		auto json = peekRootScalar();
		uint8_t tmpbuf[5 + 1];
		if (!jsonIterator->copyToBuffer(json, max_len, tmpbuf)) {
			return incorrectTypeError();
		}
		auto result = parseBool(tmpbuf);
		if (result.error() == Success) {
			if (!jsonIterator->isSingleToken()) {
				return Trailing_Content;
			}
			advanceRootScalar();
		}
		return result;
	}

	bool ValueIterator::isRootNull() noexcept {
		if (!jsonIterator->isSingleToken()) {
			return false;
		}
		auto max_len = peekStartLength();
		auto json = peekRootScalar();
		bool result = (max_len >= 4 && !str4ncmp(json, "null") && (max_len == 4 || isStructuralOrWhitespace(json[5])));
		if (result) {
			advanceRootScalar();
		}
		return result;
	}

	ErrorCode ValueIterator::skipChild() noexcept {
		assert(jsonIterator->TokenIterator::currentPosition > rootStructural);
		assert(jsonIterator->currentDepth >= currentDepth);

		return jsonIterator->skipChild(depth());
	}

	ValueIterator ValueIterator::child() const noexcept {
		assertAtChild();
		return { jsonIterator, static_cast<size_t>(depth() + 1), jsonIterator->TokenIterator::position() };
	}

	bool ValueIterator::isOpen() const noexcept {
		return jsonIterator->depth() >= depth();
	}

	bool ValueIterator::atEnd() const noexcept {
		return jsonIterator->atEnd();
	}

	bool ValueIterator::atStart() const noexcept {
		return jsonIterator->TokenIterator::position() == startPosition();
	}

	bool ValueIterator::atFirstField() const noexcept {
		assert(jsonIterator->TokenIterator::currentPosition > rootStructural);
		return jsonIterator->TokenIterator::position() == startPosition() + 1;
	}

	void ValueIterator::abandon() noexcept {
		jsonIterator->abandon();
	}

	int32_t ValueIterator::depth() const noexcept {
		return currentDepth;
	}

	ErrorCode ValueIterator::error() const noexcept {
		return jsonIterator->error;
	}

	uint8_t*& ValueIterator::stringBufLoc() noexcept {
		return jsonIterator->stringBufLoc();
	}

	const JsonIterator& ValueIterator::jsonIter() const noexcept {
		return *jsonIterator;
	}

	JsonIterator& ValueIterator::jsonIter() noexcept {
		return *jsonIterator;
	}

	const uint8_t* ValueIterator::peekStart() const noexcept {
		return jsonIterator->peek(startPosition());
	}

	uint32_t ValueIterator::peekStartLength() const noexcept {
		return jsonIterator->peekLength(startPosition());
	}

	const uint8_t* ValueIterator::peekScalar() noexcept {
		if (!isAtStart()) {
			return peekStart();
		}

		assertAtStart();
		return jsonIterator->peek();
	}

	void ValueIterator::advanceScalar() noexcept {
		if (!isAtStart()) {
			return;
		}

		assertAtStart();
		jsonIterator->returnCurrentAndAdvance();
		jsonIterator->ascendTo(depth() - 1);
	}

	ErrorCode ValueIterator::startContainer(uint8_t start_char) noexcept {
		const uint8_t* json;
		if (!isAtStart()) {
			json = peekStart();
			if (*json != start_char) {
				return incorrectTypeError();
			}
		} else {
			assertAtStart();
			json = jsonIterator->peek();
			if (*json != start_char) {
				return incorrectTypeError();
			}
			jsonIterator->returnCurrentAndAdvance();
		}


		return Success;
	}

	const uint8_t* ValueIterator::peekRootScalar() noexcept {
		if (!isAtStart()) {
			return peekStart();
		}

		assertAtRoot();
		return jsonIterator->peek();
	}

	const uint8_t* ValueIterator::peekNonRootScalar() noexcept {
		//if (!isAtStart()) {
		//return peekStart();
		//}

		//assertAtNonRootStart();
		return jsonIterator->peek();
	}

	void ValueIterator::advanceRootScalar() noexcept {
		if (!isAtStart()) {
			return;
		}

		assertAtRoot();
		jsonIterator->returnCurrentAndAdvance();
		jsonIterator->ascendTo(depth() - 1);
	}

	void ValueIterator::advanceNonRootScalar() noexcept {
		if (!isAtStart()) {
			return;
		}

		assertAtNonRootStart();
		jsonIterator->returnCurrentAndAdvance();
		jsonIterator->ascendTo(depth() - 1);
	}

	ErrorCode ValueIterator::incorrectTypeError() const noexcept {
		return Incorrect_Type;
	}

	bool ValueIterator::isAtStart() const noexcept {
		return position() == startPosition();
	}

	bool ValueIterator::isAtKey() const noexcept {
		return currentDepth == jsonIterator->currentDepth && *jsonIterator->peek() == '"';
	}

	bool ValueIterator::isAtIteratorStart() const noexcept {
		auto delta = position() - startPosition();
		return delta == 1 || delta == 2;
	}

	void ValueIterator::assertAtStart() const noexcept {
		assert(jsonIterator->TokenIterator::currentPosition == rootStructural);
		assert(jsonIterator->currentDepth == currentDepth);
		assert(currentDepth > 0);
	}

	void ValueIterator::assertAtContainerStart() const noexcept {
		assert(jsonIterator->TokenIterator::currentPosition == rootStructural + 1);
		assert(jsonIterator->currentDepth == currentDepth);
		assert(currentDepth > 0);
	}

	void ValueIterator::assertAtNext() const noexcept {
		assert(jsonIterator->TokenIterator::currentPosition > rootStructural);
		assert(jsonIterator->currentDepth == currentDepth);
		assert(currentDepth > 0);
	}

	void ValueIterator::moveAtStart() noexcept {
		jsonIterator->currentDepth = currentDepth;
		jsonIterator->TokenIterator::setPosition(rootStructural);
	}

	void ValueIterator::moveAtContainerStart() noexcept {
		jsonIterator->currentDepth = currentDepth;
		jsonIterator->TokenIterator::setPosition(rootStructural + 1);
	}

	JsonifierResult<bool> ValueIterator::resetArray() noexcept {
		moveAtContainerStart();
		return startedArray();
	}

	JsonifierResult<bool> ValueIterator::resetObject() noexcept {
		moveAtContainerStart();
		return startedObject();
	}

	void ValueIterator::assertAtChild() const noexcept {
		assert(jsonIterator->TokenIterator::currentPosition > rootStructural);
		assert(jsonIterator->currentDepth == currentDepth + 1);
		assert(currentDepth > 0);
	}

	void ValueIterator::assertAtRoot() const noexcept {
		assertAtStart();
		assert(currentDepth == 1);
	}

	void ValueIterator::assertAtNonRootStart() const noexcept {
		assertAtStart();
		assert(currentDepth > 1);
	}

	void ValueIterator::assertIsValid() const noexcept {
		assert(jsonIterator != nullptr);
	}

	bool ValueIterator::isValid() const noexcept {
		return jsonIterator != nullptr;
	}

	JsonifierResult<JsonType> ValueIterator::type() const noexcept {
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

	uint32_t* ValueIterator::startPosition() const noexcept {
		return rootStructural;
	}

	uint32_t* ValueIterator::position() const noexcept {
		return jsonIterator->position();
	}

	uint32_t* ValueIterator::endPosition() const noexcept {
		return jsonIterator->endPosition();
	}

	uint32_t* ValueIterator::lastPosition() const noexcept {
		return jsonIterator->lastPosition();
	}

	ErrorCode ValueIterator::reportError(ErrorCode error) noexcept {
		return jsonIterator->reportError(error);
	}

	JsonifierResult<ValueIterator>::JsonifierResult(ValueIterator&& Value) noexcept
		: JsonifierResultBase<ValueIterator>(std::forward<ValueIterator>(Value)) {
	}

	JsonifierResult<ValueIterator>::JsonifierResult(ErrorCode error) noexcept : JsonifierResultBase<ValueIterator>(error) {
	}

}
