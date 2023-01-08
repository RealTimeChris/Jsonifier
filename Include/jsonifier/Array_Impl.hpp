#pragma once

#include <jsonifier/Array.hpp>

//
// ### Live States
//
// While iterating or looking up values, depth >= iterator->depth. at_start may vary. Error is
// always Success:
//
// - Start: This is the state when the Array is first found and the iterator is just past the `{`.
//   In this state, at_start == true.
// - Next: After we hand a scalar Value to the user, or an Array/Object which they then fully
//   iterate over, the iterator is at the `,` before the next Value (or `]`). In this state,
//   depth == iterator->depth, at_start == false, and error == Success.
// - Unfinished Business: When we hand an Array/Object to the user which they do not fully
//   iterate over, we need to finish that iteration by skipping child values until we reach the
//   Next state. In this state, depth > iterator->depth, at_start == false, and error == Success.
//
// ## Error States
//
// In error states, we will yield exactly one more Value before stopping. iterator->depth == depth
// and at_start is always false. We decrement after yielding the error, moving to the Finished
// state.
//
// - Chained Error: When the Array iterator is part of an error chain--for example, in
//   `for (auto tweet : doc["tweets"])`, where the tweet element may be missing or not be an
//   Array--we yield that error in the loop, exactly once. In this state, error != Success and
//   iterator->depth == depth, and at_start == false. We decrement depth when we yield the error.
// - Missing Comma Error: When the iterator ++ method discovers there is no comma between elements,
//   we flag that as an error and treat it exactly the same as a Chained Error. In this state,
//   error == Tape_Error, iterator->depth == depth, and at_start == false.
//
// ## Terminal State
//
// The terminal state has iterator->depth < depth. at_start is always false.
//
// - Finished: When we have reached a `]` or have reported an error, we are finished. We signal this
//   by decrementing depth. In this state, iterator->depth < depth, at_start == false, and
//   error == Success.
//
namespace Jsonifier {
	inline Array::Array(const ValueIterator& _iter) noexcept : iterator{ _iter } {
	}

	inline JsonifierResult<Array> Array::start(ValueIterator& iterator) noexcept {
		// We don't need to know if the Array is empty to start iteration, but we do want to know if there
		// is an error--thus `simdjson_unused`.
		bool has_value;
		JsonifierTry(iterator.start_array().get(has_value));
		return Array(iterator);
	}
	inline JsonifierResult<Array> Array::start_root(ValueIterator& iterator) noexcept {
		bool has_value;
		JsonifierTry(iterator.start_root_array().get(has_value));
		return Array(iterator);
	}
	inline JsonifierResult<Array> Array::started(ValueIterator& iterator) noexcept {
		bool has_value;
		JsonifierTry(iterator.started_array().get(has_value));
		return Array(iterator);
	}

	inline JsonifierResult<ArrayIterator> Array::begin() noexcept {
		return ArrayIterator(iterator);
	}
	inline JsonifierResult<ArrayIterator> Array::end() noexcept {
		return ArrayIterator(iterator);
	}
	inline ErrorCode Array::consume() noexcept {
		auto error = iterator.json_iter().skip_child(iterator.depth() - 1);
		if (error) {
			iterator.abandon();
		}
		return error;
	}

	inline JsonifierResult<std::string_view> Array::raw_json() noexcept {
		const uint8_t* starting_point{ iterator.peek_start() };
		auto error = consume();
		if (error) {
			return error;
		}
		// After 'consume()', we could be left pointing just beyond the Document, but that
		// is ok because we are not going to dereference the final pointer position, we just
		// use it to compute the length in bytes.
		const uint8_t* final_point{ iterator.jsonIterator->unsafe_pointer() };
		return std::string_view(reinterpret_cast<const char*>(starting_point), size_t(final_point - starting_point));
	}

	inline JsonifierResult<size_t> Array::countElements() & noexcept {
		size_t count{ 0 };
		// Important: we do not consume any of the values.
		for (auto v: *this) {
			count++;
		}
		// The above loop will always succeed, but we want to report errors.
		if (iterator.error()) {
			return iterator.error();
		}
		// We need to move back at the start because we expect users to iterate through
		// the Array after counting the number of elements.
		iterator.reset_array();
		return count;
	}

	inline JsonifierResult<bool> Array::is_empty() & noexcept {
		bool is_not_empty;
		auto error = iterator.reset_array().get(is_not_empty);
		if (error) {
			return error;
		}
		return !is_not_empty;
	}

	inline JsonifierResult<bool> Array::reset() & noexcept {
		return iterator.reset_array();
	}

	inline JsonifierResult<Value> Array::atPointer(std::string_view json_pointer) noexcept {
		if (json_pointer[0] != '/') {
			return Invalid_Json_Pointer;
		}
		json_pointer = json_pointer.substr(1);
		// - means "the append position" or "the element after the end of the Array"
		// We don't support this, because we're returning a real element, not a position.
		if (json_pointer == "-") {
			return Out_Of_Bounds;
		}

		// Read the Array index
		size_t array_index = 0;
		size_t i;
		for (i = 0; i < json_pointer.length() && json_pointer[i] != '/'; i++) {
			uint8_t digit = uint8_t(json_pointer[i] - '0');
			// Check for non-digit in Array index. If it's there, we're trying to get a Field in an Object
			if (digit > 9) {
				return Incorrect_Type;
			}
			array_index = array_index * 10 + digit;
		}

		// 0 followed by other digits is invalid
		if (i > 1 && json_pointer[0] == '0') {
			return Invalid_Json_Pointer;
		}// "JSON pointer Array index has other characters after 0"

		// Empty string is invalid; so is a "/" with no digits before it
		if (i == 0) {
			return Invalid_Json_Pointer;
		}// "Empty string in JSON pointer Array index"
		// Get the child
		auto child = at(array_index);
		// If there is an error, it ends here
		if (child.error()) {
			return child;
		}

		// If there is a /, we're not done yet, call recursively.
		if (i < json_pointer.length()) {
			child = child.atPointer(json_pointer.substr(i));
		}
		return child;
	}

	inline JsonifierResult<Value> Array::at(size_t index) noexcept {
		size_t i = 0;
		for (auto Value: *this) {
			if (i == index) {
				return Value;
			}
			i++;
		}
		return Out_Of_Bounds;
	}


		inline JsonifierResult<Array>::JsonifierResult(Array&& Value) noexcept
			: ImplementationJsonifierResultBase<Array>(std::forward<Array>(Value)) {
		}
		inline JsonifierResult<Array>::JsonifierResult(ErrorCode error) noexcept : ImplementationJsonifierResultBase<Array>(error) {
		}

		inline JsonifierResult<ArrayIterator> JsonifierResult<Array>::begin() noexcept {
			if (error()) {
				return error();
			}
			return first.begin();
		}
		inline JsonifierResult<ArrayIterator> JsonifierResult<Array>::end() noexcept {
			if (error()) {
				return error();
			}
			return first.end();
		}
		inline JsonifierResult<size_t> JsonifierResult<Array>::countElements() & noexcept {
			if (error()) {
				return error();
			}
			return first.countElements();
		}
		inline JsonifierResult<bool> JsonifierResult<Array>::is_empty() & noexcept {
			if (error()) {
				return error();
			}
			return first.is_empty();
		}
		inline JsonifierResult<Value> JsonifierResult<Array>::at(size_t index) noexcept {
			if (error()) {
				return error();
			}
			return first.at(index);
		}
		inline JsonifierResult<Value> JsonifierResult<Array>::atPointer(std::string_view json_pointer) noexcept {
			if (error()) {
				return error();
			}
			return first.atPointer(json_pointer);
		}
	}
