#pragma once
#include <jsonifier/JsonIterator.hpp>
#include <jsonifier/Parser.hpp>

namespace Jsonifier {

	inline JsonIterator::JsonIterator(JsonIterator&& other) noexcept
		: token(std::forward<TokenIterator>(other.token)), parser{ other.parser }, stringBufferLocation{ other.stringBufferLocation },
		  error{ other.error }, currentDepth{ other.currentDepth }, rootStructural{ other.rootStructural }, _streaming{ other._streaming } {
		other.parser = nullptr;
	}
	inline JsonIterator& JsonIterator::operator=(JsonIterator&& other) noexcept {
		token = other.token;
		parser = other.parser;
		stringBufferLocation = other.stringBufferLocation;
		error = other.error;
		currentDepth = other.currentDepth;
		rootStructural = other.rootStructural;
		_streaming = other._streaming;
		other.parser = nullptr;
		return *this;
	}

	inline JsonIterator::JsonIterator(Parser* _parser) noexcept
		: token(_parser->getStringView(), _parser->getStructuralIndices()), parser{ _parser }, stringBufferLocation{ parser->getStringBuffer() },
		  currentDepth{ 1 }, rootStructural{ parser->getStructuralIndices() }, _streaming{ false } {
	}

	inline void JsonIterator::rewind() noexcept {
		token.set_position(root_position());
		stringBufferLocation = parser->getStringBuffer();
		currentDepth = 1;
	}

	inline bool JsonIterator::balanced() const noexcept {
		TokenIterator ti(token);
		int32_t count{ 0 };
		ti.set_position(root_position());
		while (ti.peek() <= peek_last()) {
			switch (*ti.return_current_and_advance()) {
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
		switch (*return_current_and_advance()) {
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
					return_current_and_advance();
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
			switch (*return_current_and_advance()) {
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

		return report_error(Tape_Error, "not enough close braces");
	}

	inline bool JsonIterator::at_root() const noexcept {
		return position() == root_position();
	}

	inline bool JsonIterator::is_single_token() const noexcept {
		return parser->getTapeLength() == 1;
	}

	inline bool JsonIterator::streaming() const noexcept {
		return _streaming;
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
		size_t n_structural_indexes{ parser->getTapeLength() };
		return &parser->getStructuralIndices()[n_structural_indexes];
	}

	inline std::string JsonIterator::to_string() const noexcept {
		if (!is_alive()) {
			return "dead JsonIterator instance";
		}
		const char* current_structural = reinterpret_cast<const char*>(token.peek());
		return std::string("JsonIterator [ depth : ") + std::to_string(currentDepth) + std::string(", structural : '") +
			std::string(current_structural, 1) + std::string("', offset : ") + std::to_string(token.current_offset()) + std::string("', error : ") +
			std::to_string(error) + std::string(" ]");
	}

	inline JsonifierResult<const char*> JsonIterator::currentLocation() noexcept {
		if (!is_alive()) {// Unrecoverable error
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

	inline bool JsonIterator::is_alive() const noexcept {
		return parser;
	}

	inline void JsonIterator::abandon() noexcept {
		parser = nullptr;
		currentDepth = 0;
	}

	inline const uint8_t* JsonIterator::return_current_and_advance() noexcept {
#if SIMDJSON_CHECK_EOF
		assert_more_tokens();
#endif// SIMDJSON_CHECK_EOF
		return token.return_current_and_advance();
	}

	inline const uint8_t* JsonIterator::unsafe_pointer() const noexcept {
		// deliberately done without safety guard:
		return token.peek(0);
	}

	inline const uint8_t* JsonIterator::peek(int32_t delta) const noexcept {
#if SIMDJSON_CHECK_EOF
		assert_more_tokens(delta + 1);
#endif// SIMDJSON_CHECK_EOF
		return token.peek(delta);
	}

	inline uint32_t JsonIterator::peek_length(int32_t delta) const noexcept {
#if SIMDJSON_CHECK_EOF
		assert_more_tokens(delta + 1);
#endif// #if SIMDJSON_CHECK_EOF
		return token.peek_length(delta);
	}

	inline const uint8_t* JsonIterator::peek(uint32_t* position) const noexcept {
		// todo: currently we require end-of-string buffering, but the following
		// assert_valid_position should be turned on if/when we lift that condition.
		// assert_valid_position(position);
		// This is almost surely related to SIMDJSON_CHECK_EOF but given that SIMDJSON_CHECK_EOF
		// is ON by default, we have no choice but to disable it for real with a comment.
		return token.peek(position);
	}

	inline uint32_t JsonIterator::peek_length(uint32_t* position) const noexcept {
#if SIMDJSON_CHECK_EOF
		assert_valid_position(position);
#endif// SIMDJSON_CHECK_EOF
		return token.peek_length(position);
	}

	inline uint32_t* JsonIterator::last_position() const noexcept {
		// The following line fails under some compilers...
		// assert(parser->getTapeLength() > 0);
		// since it has side-effects.
		size_t n_structural_indexes{ parser->getTapeLength() };
		assert(n_structural_indexes > 0);
		return &parser->getStructuralIndices()[n_structural_indexes - 1];
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
		return stringBufferLocation;
	}

	inline ErrorCode JsonIterator::report_error(ErrorCode _error, const char* message) noexcept {
		assert(_error != Success && _error != Uninitialized && _error != Incorrect_Type && _error != No_Such_Field);
		error = _error;
		return error;
	}

	inline uint32_t* JsonIterator::position() const noexcept {
		return token.position();
	}

	JsonifierResult<std::string_view> JsonIterator::unescape(RawJsonString in) noexcept {
		uint8_t* end = parseString(in.stringView, this->stringBufferLocation);
		if (!end) {
			return String_Error;
		}
		std::string_view result(reinterpret_cast<const char*>(this->stringBufferLocation), end - this->stringBufferLocation);
		this->stringBufferLocation = end;
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
