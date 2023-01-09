#pragma once
#include <jsonifier/JsonIterator.hpp>
#include <jsonifier/Parser.hpp>

namespace Jsonifier {

	inline JsonIterator::JsonIterator(JsonIterator&& other) noexcept
		: TokenIterator(std::forward<TokenIterator>(other)), parser{ other.parser }, stringBuffer{ other.stringBuffer }, error{ other.error },
		  currentDepth{ other.currentDepth }, rootStructural{ other.rootStructural } {
		other.parser = nullptr;
	}

	inline JsonIterator& JsonIterator::operator=(JsonIterator&& other) noexcept {
		rootStructural = other.rootStructural;
		currentDepth = other.currentDepth;
		stringBuffer = other.stringBuffer;
		parser = other.parser;
		error = other.error;
		other.parser = nullptr;
		return *this;
	}

	inline JsonIterator::JsonIterator(Parser* parserNew) noexcept
		: TokenIterator(parserNew->getStringView(), parserNew->getStructuralIndices()), parser{ parserNew },
		  stringBuffer{ parserNew->getStringBuffer() }, currentDepth{ 1 }, rootStructural{ parser->getStructuralIndices() } {
	}

	inline void JsonIterator::rewind() noexcept {
		TokenIterator::setPosition(rootPosition());
		stringBuffer = parser->getStringBuffer();
		currentDepth = 1;
	}

	inline bool JsonIterator::balanced() const noexcept {
		TokenIterator ti(*this);
		int32_t count{ 0 };
		ti.setPosition(rootPosition());
		while (ti.peek() <= peekLast()) {
			switch (*ti.returnCurrentAndAdvance()) {
				case '[':
				case '{':
					count++;
					break;
				case ']':
				case '}':
					count--;
					break;
				default:
					break;
			}
		}
		return count == 0;
	}

	inline ErrorCode JsonIterator::skipChild(uint32_t parentDepth) noexcept {
		if (depth() <= parentDepth) {
			return Success;
		}
		switch (*returnCurrentAndAdvance()) {
			case '[':
			case '{':
			case ':':
				break;
			case ',':
				break;
			case ']':
			case '}':
				currentDepth--;
				if (depth() <= parentDepth) {
					return Success;
				}
				break;
			case '"':
				if (*peek() == ':') {
					returnCurrentAndAdvance();
					break;
				}
				[[fallthrough]];
			default:
				currentDepth--;
				if (depth() <= parentDepth) {
					return Success;
				}
				break;
		}

		while (position() < endPosition()) {
			switch (*returnCurrentAndAdvance()) {
				case '[':
				case '{':
					currentDepth++;
					break;
				case ']':
				case '}':
					currentDepth--;
					if (depth() <= parentDepth) {
						return Success;
					}
					break;
				default:
					break;
			}
		}

		return reportError(Tape_Error, "not enough close braces");
	}

	inline bool JsonIterator::atRoot() const noexcept {
		return position() == rootPosition();
	}

	inline bool JsonIterator::isSingleToken() const noexcept {
		return parser->getTapeLength() == 1;
	}

	inline uint32_t* JsonIterator::rootPosition() const noexcept {
		return rootStructural;
	}

	inline void JsonIterator::assertAtDocumentDepth() const noexcept {
		assert(currentDepth == 1);
	}

	inline void JsonIterator::assertAtRoot() const noexcept {
		assert(currentDepth == 1);
	}

	inline bool JsonIterator::atEnd() const noexcept {
		return position() == endPosition();
	}

	inline uint32_t* JsonIterator::endPosition() const noexcept {
		size_t structuralIndexCount{ parser->getTapeLength() };
		return &parser->getStructuralIndices()[structuralIndexCount - 1];
	}

	inline std::string JsonIterator::toString() const noexcept {
		if (!isAlive()) {
			return "dead JsonIterator instance";
		}
		const char* current_structural = reinterpret_cast<const char*>(TokenIterator::peek());
		return std::string("JsonIterator [ depth : ") + std::to_string(currentDepth) + std::string(", structural : '") +
			std::string(current_structural, 1) + std::string("', offset : ") + std::to_string(TokenIterator::currentOffset()) +
			std::string("', error : ") + std::to_string(error) + std::string(" ]");
	}

	inline JsonifierResult<const char*> JsonIterator::currentLocation() noexcept {
		if (!isAlive()) {
			if (!atRoot()) {
				return reinterpret_cast<const char*>(TokenIterator::peek(-1));
			} else {
				return reinterpret_cast<const char*>(TokenIterator::peek());
			}
		}
		if (atEnd()) {
			return Out_Of_Bounds;
		}
		return reinterpret_cast<const char*>(TokenIterator::peek());
	}

	inline bool JsonIterator::isAlive() const noexcept {
		return parser;
	}

	inline void JsonIterator::abandon() noexcept {
		parser = nullptr;
		currentDepth = 0;
	}

	inline const uint8_t* JsonIterator::returnCurrentAndAdvance() noexcept {
		return TokenIterator::returnCurrentAndAdvance();
	}

	inline const uint8_t* JsonIterator::unsafePointer() const noexcept {
		return TokenIterator::peek(0);
	}

	inline const uint8_t* JsonIterator::peek(int32_t delta) const noexcept {
		return TokenIterator::peek(delta);
	}

	inline uint32_t JsonIterator::peekLength(int32_t delta) const noexcept {
		return TokenIterator::peekLength(delta);
	}

	inline const uint8_t* JsonIterator::peek(uint32_t* position) const noexcept {
		return TokenIterator::peek(position);
	}

	inline uint32_t JsonIterator::peekLength(uint32_t* position) const noexcept {
		return TokenIterator::peekLength(position);
	}

	inline uint32_t* JsonIterator::lastPosition() const noexcept {
		size_t structuralIndexCount{ parser->getTapeLength() };
		assert(structuralIndexCount > 0);
		return &parser->getStructuralIndices()[structuralIndexCount - 1];
	}

	inline const uint8_t* JsonIterator::peekLast() const noexcept {
		return TokenIterator::peek(lastPosition());
	}

	inline void JsonIterator::ascendTo(uint32_t parentDepth) noexcept {
		assert(parentDepth >= 0 && parentDepth < INT32_MAX - 1);
		assert(currentDepth == parentDepth + 1);
		currentDepth = parentDepth;
	}

	inline void JsonIterator::descendTo(uint32_t child_depth) noexcept {
		assert(child_depth >= 1 && child_depth < INT32_MAX);
		assert(currentDepth == child_depth - 1);
		currentDepth = child_depth;
	}

	inline uint32_t JsonIterator::depth() const noexcept {
		return currentDepth;
	}

	inline uint8_t*& JsonIterator::stringBufLoc() noexcept {
		return stringBuffer;
	}

	inline ErrorCode JsonIterator::reportError(ErrorCode errorNew, const char* message) noexcept {
		assert(errorNew != Success && errorNew != Uninitialized && errorNew != Incorrect_Type && errorNew != No_Such_Field);
		error = errorNew;
		return error;
	}

	inline uint32_t* JsonIterator::position() const noexcept {
		return TokenIterator::position();
	}

	JsonifierResult<std::string_view> JsonIterator::unescape(RawJsonString in) noexcept {
		uint8_t* end = parseString(in.stringView, this->stringBuffer);
		if (!end) {
			return String_Error;
		}
		std::string_view result(reinterpret_cast<const char*>(this->stringBuffer), end - this->stringBuffer);
		this->stringBuffer = end;
		return result;
	}

	inline void JsonIterator::reenterChild(uint32_t* position, uint32_t child_depth) noexcept {
		assert(child_depth >= 1 && child_depth < INT32_MAX);
		assert(currentDepth == child_depth - 1);
		TokenIterator::setPosition(position);
		currentDepth = child_depth;
	}

	inline ErrorCode JsonIterator::optionalError(ErrorCode errorNew, const char* message) noexcept {
		assert(errorNew == Incorrect_Type || errorNew == No_Such_Field);
		return errorNew;
	}

	template<int N> inline bool JsonIterator::copyToBuffer(const uint8_t* json, uint32_t max_len, uint8_t (&tmpbuf)[N]) noexcept {
		if ((N < max_len) || (N == 0)) {
			return false;
		}
		if (max_len > N - 1) {
			max_len = N - 1;
		}
		std::memcpy(tmpbuf, json, max_len);
		tmpbuf[max_len] = ' ';
		return true;
	}

	inline JsonifierResult<JsonIterator>::JsonifierResult(JsonIterator&& Value) noexcept
		: JsonifierResultBase<JsonIterator>(std::forward<JsonIterator>(Value)) {
	}

	inline JsonifierResult<JsonIterator>::JsonifierResult(ErrorCode error) noexcept : JsonifierResultBase<JsonIterator>(error) {
	}

}
