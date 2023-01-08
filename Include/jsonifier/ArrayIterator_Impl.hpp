
#pragma once

#include <jsonifier/ArrayIterator.hpp>

namespace Jsonifier {

	inline ArrayIterator::ArrayIterator(const ValueIterator& _iter) noexcept : iterator{ _iter } {
	}

	inline JsonifierResult<Value> ArrayIterator::operator*() noexcept {
		if (iterator.error()) {
			iterator.abandon();
			return iterator.error();
		}
		return Value(iterator.child());
	}
	inline bool ArrayIterator::operator==(const ArrayIterator& other) const noexcept {
		return !(*this != other);
	}
	inline bool ArrayIterator::operator!=(const ArrayIterator&) const noexcept {
		return iterator.is_open();
	}
	inline ArrayIterator& ArrayIterator::operator++() noexcept {
		ErrorCode error;
		// PERF NOTE this is a safety rail ... users should exit loops as soon as they receive an error, so we'll never get here.
		// However, it does not seem to make a perf difference, so we add it out of an abundance of caution.
		if ((error = iterator.error())) {
			return *this;
		}
		if ((error = iterator.skip_child())) {
			return *this;
		}
		if ((error = iterator.has_next_element().error())) {
			return *this;
		}
		return *this;
	}

		inline JsonifierResult<ArrayIterator>::JsonifierResult(ArrayIterator&& Value) noexcept
			: ImplementationJsonifierResultBase<ArrayIterator>(std::forward<ArrayIterator>(Value)) {
			first.iterator.assert_is_valid();
		}
		inline JsonifierResult<ArrayIterator>::JsonifierResult(ErrorCode error) noexcept
			: ImplementationJsonifierResultBase<ArrayIterator>({}, error) {
		}

		inline JsonifierResult<Value> JsonifierResult<ArrayIterator>::operator*() noexcept {
			if (error()) {
				return error();
			}
			return *first;
		}
		inline bool JsonifierResult<ArrayIterator>::operator==(const JsonifierResult<ArrayIterator>& other) const noexcept {
			if (!first.iterator.is_valid()) {
				return !error();
			}
			return first == other.first;
		}
		inline bool JsonifierResult<ArrayIterator>::operator!=(const JsonifierResult<ArrayIterator>& other) const noexcept {
			if (!first.iterator.is_valid()) {
				return error();
			}
			return first != other.first;
		}
		inline JsonifierResult<ArrayIterator>& JsonifierResult<ArrayIterator>::operator++() noexcept {
			// Clear the error if there is one, so we don't yield it twice
			if (error()) {
				second = Success;
				return *this;
			}
			++(first);
			return *this;
		}

	}
