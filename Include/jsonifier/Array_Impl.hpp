#pragma once

#include <jsonifier/ImplementationJsonifierResultBase_Impl.hpp>
#include <jsonifier/Value_Impl.hpp>

namespace Jsonifier {

	class ArrayIterator;

	inline JsonifierResult<Array>::JsonifierResult(Array&& Value) noexcept : ImplementationJsonifierResultBase<Array>(std::forward<Array>(Value)){}

	inline JsonifierResult<Array>::JsonifierResult(ErrorCode error) noexcept : ImplementationJsonifierResultBase<Array>(error) {}

	inline JsonifierResult<ArrayIterator>::JsonifierResult(ErrorCode error) noexcept : ImplementationJsonifierResultBase<ArrayIterator>({}, error){};

	inline ArrayIterator::ArrayIterator(const ValueIterator& _iterator) noexcept : ValueIterator{ _iterator } {
	}

	inline JsonifierResult<ArrayIterator>::JsonifierResult(ArrayIterator&& value) noexcept
		: ImplementationJsonifierResultBase<ArrayIterator>(std::forward<ArrayIterator>(value)) {
		first.assertIsValid();
	};

	inline JsonifierResult<Value> JsonifierResult<ArrayIterator>::operator*() noexcept {
		if (error()) {
			return error();
		}
		return *first;
	}

	inline bool JsonifierResult<ArrayIterator>::operator==(const JsonifierResult<ArrayIterator>& other) const noexcept {
		if (!first.isValid()) {
			return !error();
		}
		return first == other.first;
	}

	inline bool JsonifierResult<ArrayIterator>::operator!=(const JsonifierResult<ArrayIterator>& other) const noexcept {
		if (!first.isValid()) {
			return error();
		}
		return first != other.first;
	}

	inline JsonifierResult<ArrayIterator>& JsonifierResult<ArrayIterator>::operator++() noexcept {
		if (error()) {
			second = Success;
			return *this;
		}
		++(first);
		return *this;
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

	inline JsonifierResult<bool> JsonifierResult<Array>::isEmpty() & noexcept {
		if (error()) {
			return error();
		}
		return first.isEmpty();
	}

	inline JsonifierResult<Value> JsonifierResult<Array>::at(size_t index) noexcept {
		if (error()) {
			return error();
		}
		return first.at(index);
	}

	inline JsonifierResult<Value> JsonifierResult<Array>::atPointer(std::string_view jsonPointer) noexcept {
		if (error()) {
			return error();
		}
		return first.atPointer(jsonPointer);
	}
}