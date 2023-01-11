#pragma once

#include <jsonifier/RawJsonString.hpp>
#include <jsonifier/JsonIterator.hpp>
#include <jsonifier/Parser.hpp>

namespace Jsonifier {

	__forceinline JsonIterator::JsonIterator(JsonIterator&& other, std::source_location location) noexcept
		: iterator(std::forward<TokenIterator>(other.iterator)), parser{ other.parser }, stringBuffer{ other.stringBuffer }, error{ other.error },
		  currentDepth{ other.currentDepth }, rootStructural{ other.rootStructural } {
		//std::cout << "Error Report: \n"
		//<< "Caught in File: " << location.file_name() << " (" << std::to_string(location.line()) << ":" << std::to_string(location.column())
		//<< ")"
		//<< "\nThe Error: \n"
		//<< std::endl
		//<< std::endl;
		*this = std::move(other);
		//std::cout << "WERE BEING MOVED! 0101" << std::endl;
	}

	__forceinline JsonIterator& JsonIterator::operator=(JsonIterator&& other) noexcept {
		rootStructural = other.rootStructural;
		currentDepth = other.currentDepth;
		stringBuffer = other.stringBuffer;
		iterator = other.iterator;
		parser = other.parser;
		error = other.error;
		//std::cout << "WERE BEING MOVED!" << std::endl;
		return *this;
	}

	__forceinline JsonIterator::JsonIterator(Parser* parserNew) noexcept
		: iterator(parserNew->getStringView(), parserNew->getStructuralIndices()), parser{ parserNew },
		  stringBuffer{ parserNew->getStringBuffer() }, currentDepth{ 1 }, rootStructural{ parser->getStructuralIndices() } {
	}

	__forceinline void JsonIterator::rewind() noexcept {
		iterator.setPosition(rootPosition());
		stringBuffer = parser->getStringBuffer();
		currentDepth = 1;
	}

	__forceinline bool JsonIterator::balanced() const noexcept {
		TokenIterator ti(this->iterator);
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

	__forceinline ErrorCode JsonIterator::skipChild(uint32_t parentDepth) noexcept {
		if (this->depth() <= parentDepth) {
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

		return reportError(Tape_Error);
	}

	__forceinline bool JsonIterator::atRoot() const noexcept {
		return position() == rootPosition();
	}

	__forceinline bool JsonIterator::isSingleToken() const noexcept {
		return parser->getTapeLength() == 1;
	}

	__forceinline uint32_t* JsonIterator::rootPosition() const noexcept {
		return rootStructural;
	}

	__forceinline void JsonIterator::assertAtDocumentDepth() const noexcept {
		assert(currentDepth == 1);
	}

	__forceinline void JsonIterator::assertAtRoot() const noexcept {
		assert(currentDepth == 1);
	}

	__forceinline bool JsonIterator::atEnd() const noexcept {
		return position() == endPosition();
	}

	__forceinline uint32_t* JsonIterator::endPosition() const noexcept {
		size_t structuralIndexCount{ parser->getTapeLength() };
		return &this->parser->getStructuralIndices()[structuralIndexCount - 1];
	}

	__forceinline std::string JsonIterator::toString() const noexcept {
		if (!isAlive()) {
			return "dead JsonIterator instance";
		}
		const char* current_structural = reinterpret_cast<const char*>(iterator.peek());
		return std::string("JsonIterator [ depth : ") + std::to_string(currentDepth) + std::string(", structural : '") +
			std::string(current_structural, 1) + std::string("', offset : ") + std::to_string(iterator.currentOffset()) +
			std::string("', error : ") + std::to_string(error) + std::string(" ]");
	}

	__forceinline JsonifierResult<const char*> JsonIterator::currentLocation() noexcept {
		if (!isAlive()) {
			if (!atRoot()) {
				return reinterpret_cast<const char*>(iterator.peek(-1));
			} else {
				return reinterpret_cast<const char*>(iterator.peek());
			}
		}
		if (atEnd()) {
			return Out_Of_Bounds;
		}
		return reinterpret_cast<const char*>(iterator.peek());
	}

	__forceinline bool JsonIterator::isAlive() const noexcept {
		return parser;
	}

	__forceinline void JsonIterator::abandon() noexcept {
		std::cout << "WERE ABANDONING!" << std::endl;
		this->parser = nullptr;
		currentDepth = 0;
	}

	__forceinline const uint8_t* JsonIterator::returnCurrentAndAdvance() noexcept {
		return iterator.returnCurrentAndAdvance();
	}

	__forceinline const uint8_t* JsonIterator::unsafePointer() const noexcept {
		return iterator.peek(0);
	}

	__forceinline const uint8_t* JsonIterator::peek(int32_t delta) const noexcept {
		return this->iterator.peek(delta);
	}

	__forceinline uint32_t JsonIterator::peekLength(int32_t delta) const noexcept {
		return iterator.peekLength(delta);
	}

	__forceinline const uint8_t* JsonIterator::peek(uint32_t* position) const noexcept {
		return iterator.peek(position);
	}

	__forceinline uint32_t JsonIterator::peekLength(uint32_t* position) const noexcept {
		return iterator.peekLength(position);
	}

	__forceinline uint32_t* JsonIterator::lastPosition() const noexcept {
		size_t structuralIndexCount{ parser->getTapeLength() };
		assert(structuralIndexCount > 0);
		return &parser->getStructuralIndices()[structuralIndexCount - 1];
	}

	__forceinline const uint8_t* JsonIterator::peekLast() const noexcept {
		return iterator.peek(lastPosition());
	}

	__forceinline void JsonIterator::ascendTo(uint32_t parentDepth) noexcept {
		assert(parentDepth >= 0 && parentDepth < INT32_MAX - 1);
		assert(currentDepth == parentDepth + 1);
		currentDepth = parentDepth;
	}

	__forceinline void JsonIterator::descendTo(uint32_t child_depth) noexcept {
		assert(child_depth >= 1 && child_depth < INT32_MAX);
		assert(currentDepth == child_depth - 1);
		currentDepth = child_depth;
	}

	__forceinline uint32_t JsonIterator::depth() const noexcept {
		return currentDepth;
	}

	__forceinline uint8_t*& JsonIterator::stringBufLoc() noexcept {
		return stringBuffer;
	}

	__forceinline ErrorCode JsonIterator::reportError(ErrorCode errorNew) noexcept {
		assert(errorNew != Success && errorNew != Uninitialized && errorNew != Incorrect_Type && errorNew != No_Such_Field);
		error = errorNew;
		return error;
	}

	__forceinline uint32_t* JsonIterator::position() const noexcept {
		return iterator.position();
	}

	__forceinline JsonifierResult<std::string_view> JsonIterator::unescape(RawJsonString in) noexcept {
		uint8_t* end = parseString(in.stringView, this->stringBuffer);
		if (!end) {
			return String_Error;
		}
		std::string_view result(reinterpret_cast<const char*>(this->stringBuffer), end - this->stringBuffer);
		this->stringBuffer = end;
		return result;
	}

	__forceinline void JsonIterator::reenterChild(uint32_t* position, uint32_t child_depth) noexcept {
		assert(child_depth >= 1 && child_depth < INT32_MAX);
		assert(currentDepth == child_depth - 1);
		iterator.setPosition(position);
		currentDepth = child_depth;
	}

	JsonifierResult<JsonIterator>::JsonifierResult(JsonIterator&& Value) noexcept
		: JsonifierResultBase<JsonIterator>(std::forward<JsonIterator>(Value)) {
	}

	JsonifierResult<JsonIterator>::JsonifierResult(ErrorCode error) noexcept : JsonifierResultBase<JsonIterator>(error) {
	}

}
