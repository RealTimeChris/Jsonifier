#pragma once

#include <jsonifier/ValueIterator.hpp>

namespace Jsonifier {

	inline ValueIterator::ValueIterator(JsonIterator* json_iter, uint32_t depth, uint32_t* start_position) noexcept
		: jsonIterator{ json_iter }, currentDepth{ depth }, rootStructural{ start_position } {
	}

	inline JsonifierResult<bool> ValueIterator::start_object() noexcept {
		JsonifierTry(start_container('{', "Not an Object", "Object"));
		return started_object();
	}

	inline JsonifierResult<bool> ValueIterator::start_root_object() noexcept {
		JsonifierTry(start_container('{', "Not an Object", "Object"));
		return started_root_object();
	}

	inline JsonifierResult<bool> ValueIterator::started_object() noexcept {
		assert_at_container_start();
		if (*jsonIterator->peek() == '}') {
			jsonIterator->returnCurrentAndAdvance();
			end_container();
			return false;
		}
		return true;
	}

	inline JsonifierResult<bool> ValueIterator::started_root_object() noexcept {
		if (*jsonIterator->peek_last() != '}') {
			jsonIterator->abandon();
			return reportError(Incomplete_Array_Or_Object, "missing } at end");
		}
		if ((*jsonIterator->peek(jsonIterator->end_position()) == '}') && (!jsonIterator->balanced())) {
			jsonIterator->abandon();
			return reportError(Incomplete_Array_Or_Object, "the Document is unbalanced");
		}
		return started_object();
	}

	inline ErrorCode ValueIterator::end_container() noexcept {
#if SIMDJSON_CHECK_EOF
		if (depth() > 1 && at_end()) {
			return reportError(Incomplete_Array_Or_Object, "missing parent ] or }");
		}
		// if (depth() <= 1 && !at_end()) { return reportError(Incomplete_Array_Or_Object, "missing [ or { at start"); }
#endif// SIMDJSON_CHECK_EOF
		jsonIterator->ascend_to(depth() - 1);
		return Success;
	}

	inline JsonifierResult<bool> ValueIterator::has_next_field() noexcept {
		assert_at_next();

		// It's illegal to call this unless there are more tokens: anything that ends in } or ] is
		// obligated to verify there are more tokens if they are not the top level.
		switch (*jsonIterator->returnCurrentAndAdvance()) {
			case '}':
				JsonifierTry(end_container());
				return false;
			case ',':
				return true;
			default:
				return reportError(Tape_Error, "Missing comma between Object fields");
		}
	}

	inline JsonifierResult<bool> ValueIterator::find_field_raw(const std::string_view key) noexcept {
		ErrorCode error;
		bool has_value;
		//
		// Initially, the Object can be in one of a few different places:
		//
		// 1. The start of the Object, at the first Field:
		//
		//    ```
		//    { "a": [ 1, 2 ], "b": [ 3, 4 ] }
		//      ^ (depth 2, index 1)
		//    ```
		if (at_first_field()) {
			has_value = true;

			//
			// 2. When a previous search did not yield a Value or the Object is empty:
			//
			//    ```
			//    { "a": [ 1, 2 ], "b": [ 3, 4 ] }
			//                                     ^ (depth 0)
			//    { }
			//        ^ (depth 0, index 2)
			//    ```
			//
		} else if (!is_open()) {
			return false;

			// 3. When a previous search found a Field or an iterator yielded a Value:
			//
			//    ```
			//    // When a Field was not fully consumed (or not even touched at all)
			//    { "a": [ 1, 2 ], "b": [ 3, 4 ] }
			//           ^ (depth 2)
			//    // When a Field was fully consumed
			//    { "a": [ 1, 2 ], "b": [ 3, 4 ] }
			//                   ^ (depth 1)
			//    // When the last Field was fully consumed
			//    { "a": [ 1, 2 ], "b": [ 3, 4 ] }
			//                                   ^ (depth 1)
			//    ```
			//
		} else {
			if ((error = skip_child())) {
				abandon();
				return error;
			}
			if ((error = has_next_field().get(has_value))) {
				abandon();
				return error;
			}
		}
		while (has_value) {
			// Get the key and colon, stopping at the Value.
			RawJsonString actual_key;
			// size_t max_key_length = jsonIterator->peekLength() - 2; // -2 for the two quotes
			// Note: jsonIterator->peekLength() - 2 might overflow if jsonIterator->peekLength() < 2.
			// field_key() advances the pointer and checks that '"' is found (corresponding to a key).
			// The depth is left unchanged by field_key().
			if ((error = field_key().get(actual_key))) {
				abandon();
				return error;
			};
			// field_value() will advance and check that we find a ':' separating the
			// key and the Value. It will also increment the depth by one.
			if ((error = field_value())) {
				abandon();
				return error;
			}
			// If it matches, stop and return
			// We could do it this way if we wanted to allow arbitrary
			// key content (including escaped quotes).
			//if (actual_key.unsafe_is_equal(max_key_length, key)) {
			// Instead we do the following which may trigger buffer overruns if the
			// user provides an adversarial key (containing a well placed unescaped quote
			// character and being longer than the number of bytes remaining in the JSON
			// input).
			if (actual_key.unsafe_is_equal(key)) {
				// If we return here, then we return while pointing at the ':' that we just checked.
				return true;
			}

			// The call to skip_child is meant to skip over the Value corresponding to the key.
			// After skip_child(), we are right before the next comma (',') or the final brace ('}').
			JsonifierTry(skip_child());// Skip the Value entirely
			// The has_next_field() advances the pointer and check that either ',' or '}' is found.
			// It returns true if ',' is found, false otherwise. If anything other than ',' or '}' is found,
			// then we are in error and we abort.
			if ((error = has_next_field().get(has_value))) {
				abandon();
				return error;
			}
		}

		// If the loop ended, we're out of fields to look at.
		return false;
	}

	inline JsonifierResult<bool> ValueIterator::find_field_unordered_raw(const std::string_view key) noexcept {
		/**
   * When find_field_unordered_raw is called, we can either be pointing at the
   * first key, pointing outside (at the closing brace) or if a key was matched
   * we can be either pointing right afterthe ':' right before the Value (that we need skip),
   * or we may have consumed the Value and we might be at a comma or at the
   * final brace (ready for a call to has_next_field()).
   */
		ErrorCode error;
		bool has_value;

		// First, we scan from that point to the end.
		// If we don't find a match, we may loop back around, and scan from the beginning to that point.
		uint32_t* search_start = jsonIterator->position();

		// We want to know whether we need to go back to the beginning.
		bool at_first = at_first_field();
		///////////////
		// Initially, the Object can be in one of a few different places:
		//
		// 1. At the first key:
		//
		//    ```
		//    { "a": [ 1, 2 ], "b": [ 3, 4 ] }
		//      ^ (depth 2, index 1)
		//    ```
		//
		if (at_first) {
			has_value = true;

			// 2. When a previous search did not yield a Value or the Object is empty:
			//
			//    ```
			//    { "a": [ 1, 2 ], "b": [ 3, 4 ] }
			//                                     ^ (depth 0)
			//    { }
			//        ^ (depth 0, index 2)
			//    ```
			//
		} else if (!is_open()) {
			JsonifierTry(reset_object().get(has_value));
			at_first = true;
			// 3. When a previous search found a Field or an iterator yielded a Value:
			//
			//    ```
			//    // When a Field was not fully consumed (or not even touched at all)
			//    { "a": [ 1, 2 ], "b": [ 3, 4 ] }
			//           ^ (depth 2)
			//    // When a Field was fully consumed
			//    { "a": [ 1, 2 ], "b": [ 3, 4 ] }
			//                   ^ (depth 1)
			//    // When the last Field was fully consumed
			//    { "a": [ 1, 2 ], "b": [ 3, 4 ] }
			//                                   ^ (depth 1)
			//    ```
			//
		} else {
			// If someone queried a key but they not did access the Value, then we are left pointing
			// at the ':' and we need to move forward through the Value... If the Value was
			// processed then skip_child() does not move the iterator (but may adjust the depth).
			if ((error = skip_child())) {
				abandon();
				return error;
			}
			search_start = jsonIterator->position();
			if ((error = has_next_field().get(has_value))) {
				abandon();
				return error;
			}
		}

		// After initial processing, we will be in one of two states:
		//
		// ```
		// // At the beginning of a Field
		// { "a": [ 1, 2 ], "b": [ 3, 4 ] }
		//   ^ (depth 1)
		// { "a": [ 1, 2 ], "b": [ 3, 4 ] }
		//                  ^ (depth 1)
		// // At the end of the Object
		// { "a": [ 1, 2 ], "b": [ 3, 4 ] }
		//                                  ^ (depth 0)
		// ```
		//
		// Next, we find a match starting from the current position.
		while (has_value) {
			assert(jsonIterator->currentDepth == currentDepth);// We must be at the start of a Field

			// Get the key and colon, stopping at the Value.
			RawJsonString actual_key;
			// size_t max_key_length = jsonIterator->peekLength() - 2; // -2 for the two quotes
			// Note: jsonIterator->peekLength() - 2 might overflow if jsonIterator->peekLength() < 2.
			// field_key() advances the pointer and checks that '"' is found (corresponding to a key).
			// The depth is left unchanged by field_key().
			if ((error = field_key().get(actual_key))) {
				abandon();
				return error;
			};
			// field_value() will advance and check that we find a ':' separating the
			// key and the Value. It will also increment the depth by one.
			if ((error = field_value())) {
				abandon();
				return error;
			}

			// If it matches, stop and return
			// We could do it this way if we wanted to allow arbitrary
			// key content (including escaped quotes).
			// if (actual_key.unsafe_is_equal(max_key_length, key)) {
			// Instead we do the following which may trigger buffer overruns if the
			// user provides an adversarial key (containing a well placed unescaped quote
			// character and being longer than the number of bytes remaining in the JSON
			// input).
			if (actual_key.unsafe_is_equal(key)) {
				// If we return here, then we return while pointing at the ':' that we just checked.
				return true;
			}

			// The call to skip_child is meant to skip over the Value corresponding to the key.
			// After skip_child(), we are right before the next comma (',') or the final brace ('}').
			JsonifierTry(skip_child());
			// The has_next_field() advances the pointer and check that either ',' or '}' is found.
			// It returns true if ',' is found, false otherwise. If anything other than ',' or '}' is found,
			// then we are in error and we abort.
			if ((error = has_next_field().get(has_value))) {
				abandon();
				return error;
			}
		}
		// Performance note: it maybe wasteful to rewind to the beginning when there might be
		// no other query following. Indeed, it would require reskipping the whole Object.
		// Instead, you can just stay where you are. If there is a new query, there is always time
		// to rewind.
		if (at_first) {
			return false;
		}

		// If we reach the end without finding a match, search the rest of the fields starting at the
		// beginning of the Object.
		// (We have already run through the Object before, so we've already validated its structure. We
		// don't check errors in this bit.)
		JsonifierTry(reset_object().get(has_value));
		while (true) {
			assert(has_value);// we should reach search_start before ever reaching the end of the Object
			assert(jsonIterator->currentDepth == currentDepth);// We must be at the start of a Field

			// Get the key and colon, stopping at the Value.
			RawJsonString actual_key;
			// size_t max_key_length = jsonIterator->peekLength() - 2; // -2 for the two quotes
			// Note: jsonIterator->peekLength() - 2 might overflow if jsonIterator->peekLength() < 2.
			// field_key() advances the pointer and checks that '"' is found (corresponding to a key).
			// The depth is left unchanged by field_key().
			error = field_key().get(actual_key);
			assert(!error);
			// field_value() will advance and check that we find a ':' separating the
			// key and the Value.  It will also increment the depth by one.
			error = field_value();
			assert(!error);

			// If it matches, stop and return
			// We could do it this way if we wanted to allow arbitrary
			// key content (including escaped quotes).
			// if (actual_key.unsafe_is_equal(max_key_length, key)) {
			// Instead we do the following which may trigger buffer overruns if the
			// user provides an adversarial key (containing a well placed unescaped quote
			// character and being longer than the number of bytes remaining in the JSON
			// input).
			if (actual_key.unsafe_is_equal(key)) {
				// If we return here, then we return while pointing at the ':' that we just checked.
				return true;
			}

			// The call to skip_child is meant to skip over the Value corresponding to the key.
			// After skip_child(), we are right before the next comma (',') or the final brace ('}').
			JsonifierTry(skip_child());
			// If we reached the end of the key-Value pair we started from, then we know
			// that the key is not there so we return false. We are either right before
			// the next comma or the final brace.
			if (jsonIterator->position() == search_start) {
				return false;
			}
			// The has_next_field() advances the pointer and check that either ',' or '}' is found.
			// It returns true if ',' is found, false otherwise. If anything other than ',' or '}' is found,
			// then we are in error and we abort.
			error = has_next_field().get(has_value);
			assert(!error);
			// If we make the mistake of exiting here, then we could be left pointing at a key
			// in the middle of an Object. That's not an allowable state.
		}
		// If the loop ended, we're out of fields to look at. The program should
		// never reach this point.
		return false;
	}

	inline JsonifierResult<RawJsonString> ValueIterator::field_key() noexcept {
		assert_at_next();

		const uint8_t* key = jsonIterator->returnCurrentAndAdvance();
		if (*(key++) != '"') {
			return reportError(Tape_Error, "Object key is not a string");
		}
		return RawJsonString(key);
	}

	inline ErrorCode ValueIterator::field_value() noexcept {
		assert_at_next();

		if (*jsonIterator->returnCurrentAndAdvance() != ':') {
			return reportError(Tape_Error, "Missing colon in Object Field");
		}
		jsonIterator->descend_to(depth() + 1);
		return Success;
	}

	inline JsonifierResult<bool> ValueIterator::start_array() noexcept {
		JsonifierTry(start_container('[', "Not an Array", "Array"));
		return started_array();
	}

	inline JsonifierResult<bool> ValueIterator::start_root_array() noexcept {
		JsonifierTry(start_container('[', "Not an Array", "Array"));
		return started_root_array();
	}

	inline std::string ValueIterator::to_string() const noexcept {
		auto answer = std::string("ValueIterator [ depth : ") + std::to_string(currentDepth) + std::string(", ");
		if (jsonIterator != nullptr) {
			answer += jsonIterator->to_string();
		}
		answer += std::string(" ]");
		return answer;
	}

	inline JsonifierResult<bool> ValueIterator::started_array() noexcept {
		assert_at_container_start();
		if (*jsonIterator->peek() == ']') {
			jsonIterator->returnCurrentAndAdvance();
			JsonifierTry(end_container());
			return false;
		}
		jsonIterator->descend_to(depth() + 1);
		return true;
	}

	inline JsonifierResult<bool> ValueIterator::started_root_array() noexcept {
		if (*jsonIterator->peek_last() != ']') {
			jsonIterator->abandon();
			return reportError(Incomplete_Array_Or_Object, "missing ] at end");
		}
		if ((*jsonIterator->peek(jsonIterator->end_position()) == ']') && (!jsonIterator->balanced())) {
			jsonIterator->abandon();
			return reportError(Incomplete_Array_Or_Object, "the Document is unbalanced");
		}
		return started_array();
	}

	inline JsonifierResult<bool> ValueIterator::has_next_element() noexcept {
		assert_at_next();

		switch (*jsonIterator->returnCurrentAndAdvance()) {
			case ']':
				JsonifierTry(end_container());
				return false;
			case ',':
				jsonIterator->descend_to(depth() + 1);
				return true;
			default:
				return reportError(Tape_Error, "Missing comma between Array elements");
		}
	}

	inline JsonifierResult<bool> ValueIterator::parse_bool(const uint8_t* json) const noexcept {
		auto not_true = str4ncmp(json, "true");
		auto not_false = str4ncmp(json, "fals") | (json[4] ^ 'e');
		bool error = (not_true && not_false) || isNotStructuralOrWhitespace(json[not_true ? 5 : 4]);
		if (error) {
			return incorrect_type_error("Not a boolean");
		}
		return JsonifierResult<bool>(!not_true);
	}
	inline JsonifierResult<bool> ValueIterator::parse_null(const uint8_t* json) const noexcept {
		bool is_null_string = !str4ncmp(json, "null") && isStructuralOrWhitespace(json[4]);
		// if we start with 'n', we must be a null
		if (!is_null_string && json[0] == 'n') {
			return incorrect_type_error("Not a null but starts with n");
		}
		return is_null_string;
	}

	inline JsonifierResult<std::string_view> ValueIterator::getString() noexcept {
		return getRawJsonString().unescape(json_iter());
	}
	inline JsonifierResult<RawJsonString> ValueIterator::getRawJsonString() noexcept {
		auto json = peek_scalar("string");
		if (*json != '"') {
			return incorrect_type_error("Not a string");
		}
		advance_scalar("string");
		return RawJsonString(json + 1);
	}
	inline JsonifierResult<uint64_t> ValueIterator::getUint64() noexcept {
		auto result = parseUnsigned(peek_non_root_scalar("uint64"));
		if (result.error() == Success) {
			advance_non_root_scalar("uint64");
		}
		return result;
	}
	inline JsonifierResult<int64_t> ValueIterator::getInt64() noexcept {
		auto result = parseInteger(peek_non_root_scalar("int64"));
		if (result.error() == Success) {
			advance_non_root_scalar("int64");
		}
		return result;
	}
	inline JsonifierResult<double> ValueIterator::getDouble() noexcept {
		auto result = parseDouble(peek_non_root_scalar("double"));
		if (result.error() == Success) {
			advance_non_root_scalar("double");
		}
		return result;
	}
	inline JsonifierResult<bool> ValueIterator::getBool() noexcept {
		auto result = parse_bool(peek_non_root_scalar("bool"));
		if (result.error() == Success) {
			advance_non_root_scalar("bool");
		}
		return result;
	}
	inline JsonifierResult<bool> ValueIterator::isNull() noexcept {
		bool is_null_value;
		JsonifierTry(parse_null(peek_non_root_scalar("null")).get(is_null_value));
		if (is_null_value) {
			advance_non_root_scalar("null");
		}
		return is_null_value;
	}


	inline JsonifierResult<std::string_view> ValueIterator::get_root_string() noexcept {
		return getString();
	}
	inline JsonifierResult<RawJsonString> ValueIterator::get_root_raw_json_string() noexcept {
		return getRawJsonString();
	}
	inline JsonifierResult<uint64_t> ValueIterator::get_root_uint64() noexcept {
		auto max_len = peek_start_length();
		auto json = peek_root_scalar("uint64");
		uint8_t tmpbuf[20 + 1];// <20 digits> is the longest possible unsigned integer
		if (!jsonIterator->copy_to_buffer(json, max_len, tmpbuf)) {
			return NUMBER_ERROR;
		}
		auto result = parseUnsigned(tmpbuf);
		if (result.error() == Success) {
			if (!jsonIterator->is_single_token()) {
				return Trailing_Content;
			}
			advance_root_scalar("uint64");
		}
		return result;
	}
	inline JsonifierResult<int64_t> ValueIterator::get_root_int64() noexcept {
		auto max_len = peek_start_length();
		auto json = peek_root_scalar("int64");
		uint8_t tmpbuf[20 + 1];// -<19 digits> is the longest possible integer
		if (!jsonIterator->copy_to_buffer(json, max_len, tmpbuf)) {
			return NUMBER_ERROR;
		}

		auto result = parseInteger(tmpbuf);
		if (result.error() == Success) {
			if (!jsonIterator->is_single_token()) {
				return Trailing_Content;
			}
			advance_root_scalar("int64");
		}
		return result;
	}
	inline JsonifierResult<double> ValueIterator::get_root_double() noexcept {
		auto max_len = peek_start_length();
		auto json = peek_root_scalar("double");
		// Per https://www.exploringbinary.com/maximum-number-of-decimal-digits-in-binary-floating-point-numbers/,
		// 1074 is the maximum number of significant fractional digits. Add 8 more digits for the biggest
		// number: -0.<fraction>e-308.
		uint8_t tmpbuf[1074 + 8 + 1];
		if (!jsonIterator->copy_to_buffer(json, max_len, tmpbuf)) {
			return NUMBER_ERROR;
		}
		auto result = parseDouble(tmpbuf);
		if (result.error() == Success) {
			if (!jsonIterator->is_single_token()) {
				return Trailing_Content;
			}
			advance_root_scalar("double");
		}
		return result;
	}

	inline JsonifierResult<bool> ValueIterator::get_root_bool() noexcept {
		auto max_len = peek_start_length();
		auto json = peek_root_scalar("bool");
		uint8_t tmpbuf[5 + 1];
		if (!jsonIterator->copy_to_buffer(json, max_len, tmpbuf)) {
			return incorrect_type_error("Not a boolean");
		}
		auto result = parse_bool(tmpbuf);
		if (result.error() == Success) {
			if (!jsonIterator->is_single_token()) {
				return Trailing_Content;
			}
			advance_root_scalar("bool");
		}
		return result;
	}
	inline bool ValueIterator::is_root_null() noexcept {
		// If there is trailing content, then the Document is not null.
		if (!jsonIterator->is_single_token()) {
			return false;
		}
		auto max_len = peek_start_length();
		auto json = peek_root_scalar("null");
		bool result = (max_len >= 4 && !str4ncmp(json, "null") && (max_len == 4 || isStructuralOrWhitespace(json[5])));
		if (result) {
			advance_root_scalar("null");
		}
		return result;
	}

	inline ErrorCode ValueIterator::skip_child() noexcept {
		assert(jsonIterator->token.currentPosition > rootStructural);
		assert(jsonIterator->currentDepth >= currentDepth);

		return jsonIterator->skip_child(depth());
	}

	inline ValueIterator ValueIterator::child() const noexcept {
		assert_at_child();
		return { jsonIterator, static_cast<size_t>(depth() + 1), jsonIterator->token.position() };
	}

	inline bool ValueIterator::is_open() const noexcept {
		return jsonIterator->depth() >= depth();
	}

	inline bool ValueIterator::at_end() const noexcept {
		return jsonIterator->at_end();
	}

	inline bool ValueIterator::at_start() const noexcept {
		return jsonIterator->token.position() == start_position();
	}

	inline bool ValueIterator::at_first_field() const noexcept {
		assert(jsonIterator->token.currentPosition > rootStructural);
		return jsonIterator->token.position() == start_position() + 1;
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
	inline uint8_t*& ValueIterator::string_buf_loc() noexcept {
		return jsonIterator->string_buf_loc();
	}
	inline const JsonIterator& ValueIterator::json_iter() const noexcept {
		return *jsonIterator;
	}
	inline JsonIterator& ValueIterator::json_iter() noexcept {
		return *jsonIterator;
	}

	inline const uint8_t* ValueIterator::peek_start() const noexcept {
		return jsonIterator->peek(start_position());
	}
	inline uint32_t ValueIterator::peek_start_length() const noexcept {
		return jsonIterator->peekLength(start_position());
	}

	inline const uint8_t* ValueIterator::peek_scalar(const char* type) noexcept {
		// If we're not at the position anymore, we don't want to advance the cursor.
		if (!is_at_start()) {
			return peek_start();
		}

		// Get the JSON and advance the cursor, decreasing depth to signify that we have retrieved the Value.
		assert_at_start();
		return jsonIterator->peek();
	}

	inline void ValueIterator::advance_scalar(const char* type) noexcept {
		// If we're not at the position anymore, we don't want to advance the cursor.
		if (!is_at_start()) {
			return;
		}

		// Get the JSON and advance the cursor, decreasing depth to signify that we have retrieved the Value.
		assert_at_start();
		jsonIterator->returnCurrentAndAdvance();
		jsonIterator->ascend_to(depth() - 1);
	}

	inline ErrorCode ValueIterator::start_container(uint8_t start_char, const char* incorrect_type_message, const char* type) noexcept {
		// If we're not at the position anymore, we don't want to advance the cursor.
		const uint8_t* json;
		if (!is_at_start()) {
			json = peek_start();
			if (*json != start_char) {
				return incorrect_type_error(incorrect_type_message);
			}
		} else {
			assert_at_start();
			/**
     * We should be prudent. Let us peek. If it is not the right type, we
     * return an error. Only once we have determined that we have the right
     * type are we allowed to advance!
     */
			json = jsonIterator->peek();
			if (*json != start_char) {
				return incorrect_type_error(incorrect_type_message);
			}
			jsonIterator->returnCurrentAndAdvance();
		}


		return Success;
	}


	inline const uint8_t* ValueIterator::peek_root_scalar(const char* type) noexcept {
		if (!is_at_start()) {
			return peek_start();
		}

		assert_at_root();
		return jsonIterator->peek();
	}
	inline const uint8_t* ValueIterator::peek_non_root_scalar(const char* type) noexcept {
		if (!is_at_start()) {
			return peek_start();
		}

		assert_at_non_root_start();
		return jsonIterator->peek();
	}

	inline void ValueIterator::advance_root_scalar(const char* type) noexcept {
		if (!is_at_start()) {
			return;
		}

		assert_at_root();
		jsonIterator->returnCurrentAndAdvance();
		jsonIterator->ascend_to(depth() - 1);
	}
	inline void ValueIterator::advance_non_root_scalar(const char* type) noexcept {
		if (!is_at_start()) {
			return;
		}

		assert_at_non_root_start();
		jsonIterator->returnCurrentAndAdvance();
		jsonIterator->ascend_to(depth() - 1);
	}

	inline ErrorCode ValueIterator::incorrect_type_error(const char* message) const noexcept {
		return Incorrect_Type;
	}

	inline bool ValueIterator::is_at_start() const noexcept {
		return position() == start_position();
	}

	inline bool ValueIterator::is_at_key() const noexcept {
		// Keys are at the same depth as the Object.
		// Note here that we could be safer and check that we are within an Object,
		// but we do not.
		return currentDepth == jsonIterator->currentDepth && *jsonIterator->peek() == '"';
	}

	inline bool ValueIterator::is_at_iterator_start() const noexcept {
		// We can legitimately be either at the first Value ([1]), or after the Array if it's empty ([]).
		auto delta = position() - start_position();
		return delta == 1 || delta == 2;
	}

	inline void ValueIterator::assert_at_start() const noexcept {
		assert(jsonIterator->token.currentPosition == rootStructural);
		assert(jsonIterator->currentDepth == currentDepth);
		assert(currentDepth > 0);
	}

	inline void ValueIterator::assert_at_container_start() const noexcept {
		assert(jsonIterator->token.currentPosition == rootStructural + 1);
		assert(jsonIterator->currentDepth == currentDepth);
		assert(currentDepth > 0);
	}

	inline void ValueIterator::assert_at_next() const noexcept {
		assert(jsonIterator->token.currentPosition > rootStructural);
		assert(jsonIterator->currentDepth == currentDepth);
		assert(currentDepth > 0);
	}

	inline void ValueIterator::move_at_start() noexcept {
		jsonIterator->currentDepth = currentDepth;
		jsonIterator->token.set_position(rootStructural);
	}

	inline void ValueIterator::move_at_container_start() noexcept {
		jsonIterator->currentDepth = currentDepth;
		jsonIterator->token.set_position(rootStructural + 1);
	}

	inline JsonifierResult<bool> ValueIterator::reset_array() noexcept {
		move_at_container_start();
		return started_array();
	}

	inline JsonifierResult<bool> ValueIterator::reset_object() noexcept {
		move_at_container_start();
		return started_object();
	}

	inline void ValueIterator::assert_at_child() const noexcept {
		assert(jsonIterator->token.currentPosition > rootStructural);
		assert(jsonIterator->currentDepth == currentDepth + 1);
		assert(currentDepth > 0);
	}

	inline void ValueIterator::assert_at_root() const noexcept {
		assert_at_start();
		assert(currentDepth == 1);
	}

	inline void ValueIterator::assert_at_non_root_start() const noexcept {
		assert_at_start();
		assert(currentDepth > 1);
	}

	inline void ValueIterator::assertIsValid() const noexcept {
		assert(jsonIterator != nullptr);
	}

	inline bool ValueIterator::isValid() const noexcept {
		return jsonIterator != nullptr;
	}

	inline JsonifierResult<JsonType> ValueIterator::type() const noexcept {
		switch (*peek_start()) {
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

	inline uint32_t* ValueIterator::start_position() const noexcept {
		return rootStructural;
	}

	inline uint32_t* ValueIterator::position() const noexcept {
		return jsonIterator->position();
	}

	inline uint32_t* ValueIterator::end_position() const noexcept {
		return jsonIterator->end_position();
	}

	inline uint32_t* ValueIterator::last_position() const noexcept {
		return jsonIterator->last_position();
	}

	inline ErrorCode ValueIterator::reportError(ErrorCode error, const char* message) noexcept {
		return jsonIterator->reportError(error, message);
	}
	inline JsonifierResult<ValueIterator>::JsonifierResult(ValueIterator&& Value) noexcept
		: ImplementationJsonifierResultBase<ValueIterator>(std::forward<ValueIterator>(Value)) {
	}
	inline JsonifierResult<ValueIterator>::JsonifierResult(ErrorCode error) noexcept : ImplementationJsonifierResultBase<ValueIterator>(error) {
	}

}
