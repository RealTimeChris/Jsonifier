#pragma once

#include <jsonifier/ArrayIterator.hpp>

namespace Jsonifier {

	JsonifierResult<ArrayIterator>::JsonifierResult(ArrayIterator&& value) noexcept
		: JsonifierResultBase<ArrayIterator>(std::forward<ArrayIterator>(value)) {
		first.assertIsValid();
	}

	JsonifierResult<ArrayIterator>::JsonifierResult(ErrorCode error) noexcept : JsonifierResultBase<ArrayIterator>(this->second){};

	JsonifierResult<Value> JsonifierResult<ArrayIterator>::operator*() noexcept {
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

	JsonifierResult<ArrayIterator>& JsonifierResult<ArrayIterator>::operator++() noexcept {
		if (error()) {
			second = Success;
			return *this;
		}
		++(first);
		return *this;
	}

	ArrayIterator::ArrayIterator(const ValueIterator& iteratorNew) noexcept : ValueIterator{ iteratorNew } {
	}

	JsonifierResult<Value> ArrayIterator::operator*() noexcept {
		if (error()) {
			abandon();
			return error();
		}
		return Value(child());
	}

	bool ArrayIterator::operator==(const ArrayIterator& other) const noexcept {
		return !(*this != other);
	}

	bool ArrayIterator::operator!=(const ArrayIterator&) const noexcept {
		return ValueIterator::isOpen();
	}

	ArrayIterator& ArrayIterator::operator++() noexcept {
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
