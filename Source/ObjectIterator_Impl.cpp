#pragma once

#include <jsonifier/ObjectIterator.hpp>
#include <jsonifier/Field.hpp>

namespace Jsonifier {

	ObjectIterator::ObjectIterator(const ValueIterator& iteratorNew) noexcept : ValueIterator{ iteratorNew } {};

	JsonifierResult<Field> ObjectIterator::operator*() noexcept {
		ErrorCode error = ValueIterator::error();
		if (error) {
			ValueIterator::abandon();
			return error;
		}
		auto result = Field::start(*this);
		if (result.error()) {
			ValueIterator::abandon();
		}
		return result;
	}

	bool ObjectIterator::operator==(const ObjectIterator& other) const noexcept {
		return !(*this != other);
	}

	bool ObjectIterator::operator!=(const ObjectIterator&) const noexcept {
		return ValueIterator::isOpen();
	}

	ObjectIterator& ObjectIterator::operator++() noexcept {
		if (!ValueIterator::isOpen()) {
			return *this;
		}
		ErrorCode error{};
		if ((error = ValueIterator::skipChild())) {
			return *this;
		}

		bool hasValue{};
		if ((error = ValueIterator::hasNextField().get(hasValue))) {
			return *this;
		};
		return *this;
	}

	JsonifierResult<ObjectIterator>::JsonifierResult(ObjectIterator&& value) noexcept : JsonifierResultBase<ObjectIterator>{ std::move(value) } {
		this->first.assertIsValid();
	}

	JsonifierResult<ObjectIterator>::JsonifierResult(ErrorCode error) noexcept : JsonifierResultBase<ObjectIterator>{ error } {};

	JsonifierResult<Field> JsonifierResult<ObjectIterator>::operator*() noexcept {
		if (error()) {
			return error();
		}
		return *first;
	}

	bool JsonifierResult<ObjectIterator>::operator==(const JsonifierResult<ObjectIterator>& other) const noexcept {
		if (!first.ValueIterator::isValid()) {
			return !error();
		}
		return first == other.first;
	}

	bool JsonifierResult<ObjectIterator>::operator!=(const JsonifierResult<ObjectIterator>& other) const noexcept {
		if (!first.ValueIterator::isValid()) {
			return error();
		}
		return first != other.first;
	}

	JsonifierResult<ObjectIterator>& JsonifierResult<ObjectIterator>::operator++() noexcept {
		if (error()) {
			second = Success;
			return *this;
		}
		++first;
		return *this;
	}
}
