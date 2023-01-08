#pragma once
#include <jsonifier/JsonIterator.hpp>
#include <jsonifier/Parser.hpp>

namespace Jsonifier {

	inline JsonIterator::JsonIterator(JsonIterator&& other) noexcept
		: token(std::forward<TokenIterator>(other.token)), parser{ other.parser }, stringBuffer{ other.stringBuffer },
		  error{ other.error }, currentDepth{ other.currentDepth }, rootStructural{ other.rootStructural } {
		other.parser = nullptr;
	}
	inline JsonIterator& JsonIterator::operator=(JsonIterator&& other) noexcept {
		token = other.token;
		parser = other.parser;
		stringBuffer = other.stringBuffer;
		error = other.error;
		currentDepth = other.currentDepth;
		rootStructural = other.rootStructural;
		other.parser = nullptr;
		return *this;
	}

	inline JsonIterator::JsonIterator(Parser* _parser) noexcept
		: token(_parser->getStringView(), _parser->getStructuralIndices()), parser{ _parser }, stringBuffer{ parser->getStringBuffer() },
		  currentDepth{ 1 }, rootStructural{ parser->getStructuralIndices() }{
	}

	inline void JsonIterator::rewind() noexcept {
		token.set_position(root_position());
		stringBuffer = parser->getStringBuffer();
		currentDepth = 1;
	}

	inline bool JsonIterator::balanced() const noexcept {
		TokenIterator ti(token);
		int32_t count{ 0 };
		ti.set_position(root_position());
		while (ti.peek() <= peek_last()) {
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

	inline ErrorCode JsonIterator::skip_child(uint32_t parent_depth) noexcept {
		if (depth() <= parent_depth) {
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
				if (depth() <= parent_depth) {
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
				if (depth() <= parent_depth) {
					return Success;
				}
				break;
		}

		while (position() < end_position()) {
			switch (*returnCurrentAndAdvance()) {
				case '[':
				case '{':
					currentDepth++;
					break;
				case ']':
				case '}':
					currentDepth--;
					if (depth() <= parent_depth) {
						return Success;
					}
					break;
				default:
					break;
			}
		}

		return reportError(Tape_Error, "not enough close braces");
	}

	inline bool JsonIterator::at_root() const noexcept {
		return position() == root_position();
	}

	inline bool JsonIterator::is_single_token() const noexcept {
		return parser->getTapeLength() == 1;
	}

	inline uint32_t* JsonIterator::root_position() const noexcept {
		return rootStructural;
	}

	inline void JsonIterator::assert_at_Document_depth() const noexcept {
		assert(currentDepth == 1);
	}

	inline void JsonIterator::assert_at_root() const noexcept {
		assert(currentDepth == 1);
#ifndef SIMDJSON_CLANG_VISUAL_STUDIO
		// Under Visual Studio, the next assert fails with: the argument
		// has side effects that will be discarded.
		assert(token.position() == rootStructural);
#endif
	}

	inline void JsonIterator::assert_more_tokens(uint32_t required_tokens) const noexcept {
		assert_valid_position(token.currentPosition + required_tokens - 1);
	}

	inline void JsonIterator::assert_valid_position(uint32_t* position) const noexcept {
#ifndef SIMDJSON_CLANG_VISUAL_STUDIO
		assert(position >= &parser->getStructuralIndices()[0]);
		assert(position < &parser->getStructuralIndices()[parser->getTapeLength()]);
#endif
	}

	inline bool JsonIterator::at_end() const noexcept {
		return position() == end_position();
	}
	inline uint32_t* JsonIterator::end_position() const noexcept {
		size_t structuralIndexCount{ parser->getTapeLength() };
		return &parser->getStructuralIndices()[structuralIndexCount - 1];
	}

	inline std::string JsonIterator::to_string() const noexcept {
		if (!isAlive()) {
			return "dead JsonIterator instance";
		}
		const char* current_structural = reinterpret_cast<const char*>(token.peek());
		return std::string("JsonIterator [ depth : ") + std::to_string(currentDepth) + std::string(", structural : '") +
			std::string(current_structural, 1) + std::string("', offset : ") + std::to_string(token.currentOffset()) + std::string("', error : ") +
			std::to_string(error) + std::string(" ]");
	}

	inline JsonifierResult<const char*> JsonIterator::currentLocation() noexcept {
		if (!isAlive()) {// Unrecoverable error
			if (!at_root()) {
				return reinterpret_cast<const char*>(token.peek(-1));
			} else {
				return reinterpret_cast<const char*>(token.peek());
			}
		}
		if (at_end()) {
			return Out_Of_Bounds;
		}
		return reinterpret_cast<const char*>(token.peek());
	}

	inline bool JsonIterator::isAlive() const noexcept {
		return parser;
	}

	inline void JsonIterator::abandon() noexcept {
		parser = nullptr;
		currentDepth = 0;
	}

	inline const uint8_t* JsonIterator::returnCurrentAndAdvance() noexcept {
		return token.returnCurrentAndAdvance();
	}

	inline const uint8_t* JsonIterator::unsafe_pointer() const noexcept {
		return token.peek(0);
	}

	inline const uint8_t* JsonIterator::peek(int32_t delta) const noexcept {
		return token.peek(delta);
	}

	inline uint32_t JsonIterator::peekLength(int32_t delta) const noexcept {
		return token.peekLength(delta);
	}

	inline const uint8_t* JsonIterator::peek(uint32_t* position) const noexcept {
		return token.peek(position);
	}

	inline uint32_t JsonIterator::peekLength(uint32_t* position) const noexcept {
		return token.peekLength(position);
	}

	inline uint32_t* JsonIterator::last_position() const noexcept {
		size_t structuralIndexCount{ parser->getTapeLength() };
		assert(structuralIndexCount > 0);
		return &parser->getStructuralIndices()[structuralIndexCount - 1];
	}
	inline const uint8_t* JsonIterator::peek_last() const noexcept {
		return token.peek(last_position());
	}

	inline void JsonIterator::ascend_to(uint32_t parent_depth) noexcept {
		assert(parent_depth >= 0 && parent_depth < INT32_MAX - 1);
		assert(currentDepth == parent_depth + 1);
		currentDepth = parent_depth;
	}

	inline void JsonIterator::descend_to(uint32_t child_depth) noexcept {
		assert(child_depth >= 1 && child_depth < INT32_MAX);
		assert(currentDepth == child_depth - 1);
		currentDepth = child_depth;
	}

	inline uint32_t JsonIterator::depth() const noexcept {
		return currentDepth;
	}

	inline uint8_t*& JsonIterator::string_buf_loc() noexcept {
		return stringBuffer;
	}

	inline ErrorCode JsonIterator::reportError(ErrorCode _error, const char* message) noexcept {
		assert(_error != Success && _error != Uninitialized && _error != Incorrect_Type && _error != No_Such_Field);
		error = _error;
		return error;
	}

	inline uint32_t* JsonIterator::position() const noexcept {
		return token.position();
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

	inline void JsonIterator::reenter_child(uint32_t* position, uint32_t child_depth) noexcept {
		assert(child_depth >= 1 && child_depth < INT32_MAX);
		assert(currentDepth == child_depth - 1);
		token.set_position(position);
		currentDepth = child_depth;
	}

	inline ErrorCode JsonIterator::optional_error(ErrorCode _error, const char* message) noexcept {
		assert(_error == Incorrect_Type || _error == No_Such_Field);
		return _error;
	}

	template<int N> inline bool JsonIterator::copy_to_buffer(const uint8_t* json, uint32_t max_len, uint8_t (&tmpbuf)[N]) noexcept {
		// Let us guard against silly cases:
		if ((N < max_len) || (N == 0)) {
			return false;
		}
		// Truncate whitespace to fit the buffer.
		if (max_len > N - 1) {
			// if (jsoncharutils::is_not_structural_or_whitespace(json[N-1])) { return false; }
			max_len = N - 1;
		}

		// Copy to the buffer.
		std::memcpy(tmpbuf, json, max_len);
		tmpbuf[max_len] = ' ';
		return true;
	}

	inline JsonifierResult<JsonIterator>::JsonifierResult(JsonIterator&& Value) noexcept
		: ImplementationJsonifierResultBase<JsonIterator>(std::forward<JsonIterator>(Value)) {
	}
	inline JsonifierResult<JsonIterator>::JsonifierResult(ErrorCode error) noexcept : ImplementationJsonifierResultBase<JsonIterator>(error) {
	}

}
