#pragma once

#include <jsonifier/ObjectIterator.hpp>
#include <jsonifier/Field.hpp>

namespace Jsonifier{
inline ObjectIterator::ObjectIterator(const ValueIterator& _iter) noexcept : iterator{ _iter } {
}

inline JsonifierResult<Field> ObjectIterator::operator*() noexcept {
	ErrorCode error = iterator.error();
	if (error) {
		iterator.abandon();
		return error;
	}
	auto result = Field::start(iterator);
	// TODO this is a safety rail ... users should exit loops as soon as they receive an error.
	// Nonetheless, let's see if performance is OK with this if statement--the compiler may give it to us for free.
	if (result.error()) {
		iterator.abandon();
	}
	return result;
}
inline bool ObjectIterator::operator==(const ObjectIterator& other) const noexcept {
	return !(*this != other);
}
inline bool ObjectIterator::operator!=(const ObjectIterator&) const noexcept {
	return iterator.is_open();
}

inline ObjectIterator& ObjectIterator::operator++() noexcept {
	// TODO this is a safety rail ... users should exit loops as soon as they receive an error.
	// Nonetheless, let's see if performance is OK with this if statement--the compiler may give it to us for free.
	if (!iterator.is_open()) {
		return *this;
	}// Iterator will be released if there is an error

	ErrorCode error;
	if ((error = iterator.skipChild())) {
		return *this;
	}

	bool has_value;
	if ((error = iterator.has_next_field().get(has_value))) {
		return *this;
	};
	return *this;
}


//
// ### Live States
//
// While iterating or looking up values, depth >= iterator.depth. at_start may vary. Error is
// always Success:
//
// - Start: This is the state when the Object is first found and the iterator is just past the {.
//   In this state, at_start == true.
// - Next: After we hand a scalar Value to the user, or an Array/Object which they then fully
//   iterate over, the iterator is at the , or } before the next Value. In this state,
//   depth == iterator.depth, at_start == false, and error == Success.
// - Unfinished Business: When we hand an Array/Object to the user which they do not fully
//   iterate over, we need to finish that iteration by skipping child values until we reach the
//   Next state. In this state, depth > iterator.depth, at_start == false, and error == Success.
//
// ## Error States
//
// In error states, we will yield exactly one more Value before stopping. iterator.depth == depth
// and at_start is always false. We decrement after yielding the error, moving to the Finished
// state.
//
// - Chained Error: When the Object iterator is part of an error chain--for example, in
//   `for (auto tweet : doc["tweets"])`, where the tweet Field may be missing or not be an
//   Object--we yield that error in the loop, exactly once. In this state, error != Success and
//   iterator.depth == depth, and at_start == false. We decrement depth when we yield the error.
// - Missing Comma Error: When the iterator ++ method discovers there is no comma between fields,
//   we flag that as an error and treat it exactly the same as a Chained Error. In this state,
//   error == Tape_Error, iterator.depth == depth, and at_start == false.
//
// Errors that occur while reading a Field to give to the user (such as when the key is not a
// string or the Field is missing a colon) are yielded immediately. Depth is then decremented,
// moving to the Finished state without transitioning through an Error state at all.
//
// ## Terminal State
//
// The terminal state has iterator.depth < depth. at_start is always false.
//
// - Finished: When we have reached a }, we are finished. We signal this by decrementing depth.
//   In this state, iterator.depth < depth, at_start == false, and error == Success.
//
inline JsonifierResult<ObjectIterator>::JsonifierResult(ObjectIterator&& Value) noexcept
	: ImplementationJsonifierResultBase<ObjectIterator>(std::forward<ObjectIterator>(Value)) {
	first.iterator.assertIsValid();
}
inline JsonifierResult<ObjectIterator>::JsonifierResult(ErrorCode error) noexcept : ImplementationJsonifierResultBase<ObjectIterator>({}, error) {
}

inline JsonifierResult<Field> JsonifierResult<ObjectIterator>::operator*() noexcept {
	if (error()) {
		return error();
	}
	return *first;
}
// If we're iterating and there is an error, return the error once.
inline bool JsonifierResult<ObjectIterator>::operator==(const JsonifierResult<ObjectIterator>& other) const noexcept {
	if (!first.iterator.isValid()) {
		return !error();
	}
	return first == other.first;
}
// If we're iterating and there is an error, return the error once.
inline bool JsonifierResult<ObjectIterator>::operator!=(const JsonifierResult<ObjectIterator>& other) const noexcept {
	if (!first.iterator.isValid()) {
		return error();
	}
	return first != other.first;
}
// Checks for ']' and ','
inline JsonifierResult<ObjectIterator>& JsonifierResult<ObjectIterator>::operator++() noexcept {
	// Clear the error if there is one, so we don't yield it twice
	if (error()) {
		second = Success;
		return *this;
	}
	++first;
	return *this;
}

}
