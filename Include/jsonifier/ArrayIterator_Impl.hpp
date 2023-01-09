
#pragma once

#include <jsonifier/ArrayIterator.hpp>

namespace Jsonifier {

	inline JsonifierResult<ArrayIterator>::JsonifierResult(ArrayIterator&& value) noexcept
		: JsonifierResultBase<ArrayIterator>(std::forward<ArrayIterator>(value)) {
		first.assertIsValid();
	}

	inline JsonifierResult<ArrayIterator>::JsonifierResult(ErrorCode error) noexcept : JsonifierResultBase<ArrayIterator>(this->second){};

	inline JsonifierResult<Value> JsonifierResult<ArrayIterator>::operator*() noexcept {
		if (error()) {
			return error();
		}
		return *first;
	}

	bool JsonifierResult<ArrayIterator>::operator==(const JsonifierResult<ArrayIterator>& other) const noexcept {
		if (!first.isValid()) {
			return !error();
		}
		return first == other.first;
	}

	bool JsonifierResult<ArrayIterator>::operator!=(const JsonifierResult<ArrayIterator>& other) const noexcept {
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

	inline ArrayIterator::ArrayIterator(const ValueIterator& iteratorNew) noexcept : ValueIterator{ iteratorNew } {
	}

	inline JsonifierResult<Value> ArrayIterator::operator*() noexcept {
		if (error()) {
			abandon();
			return error();
		}
		return Value(child());
	}

	inline bool ArrayIterator::operator==(const ArrayIterator& other) const noexcept {
		return !(*this != other);
	}

	inline bool ArrayIterator::operator!=(const ArrayIterator&) const noexcept {
		return ValueIterator::isOpen();
	}

	inline ArrayIterator& ArrayIterator::operator++() noexcept {
		ErrorCode error{};
		if ((error = ValueIterator::error())) {
			return *this;
		}
		if ((error = ValueIterator::skipChild())) {
			return *this;
		}
		if ((error = ValueIterator::hasNextElement().error())) {
			return *this;
		}
		return *this;
	}
}
