#pragma once

#include <jsonifier/ObjectIterator.hpp>
#include <jsonifier/Field.hpp>

namespace Jsonifier {

	inline ObjectIterator::ObjectIterator(const ValueIterator& iteratorNew) noexcept : ValueIterator{ iteratorNew } {};

	inline JsonifierResult<Field> ObjectIterator::operator*() noexcept {
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

	inline bool ObjectIterator::operator==(const ObjectIterator& other) const noexcept {
		return !(*this != other);
	}

	inline bool ObjectIterator::operator!=(const ObjectIterator&) const noexcept {
		return ValueIterator::isOpen();
	}

	inline ObjectIterator& ObjectIterator::operator++() noexcept {
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

	inline JsonifierResult<ObjectIterator>::JsonifierResult(ObjectIterator&& value) noexcept
		: JsonifierResultBase<ObjectIterator>{ std::move(value) } {
		this->first.assertIsValid();
	}

	inline JsonifierResult<ObjectIterator>::JsonifierResult(ErrorCode error) noexcept : JsonifierResultBase<ObjectIterator>{ error } {};

	inline JsonifierResult<Field> JsonifierResult<ObjectIterator>::operator*() noexcept {
		if (error()) {
			return error();
		}
		return *first;
	}

	inline bool JsonifierResult<ObjectIterator>::operator==(const JsonifierResult<ObjectIterator>& other) const noexcept {
		if (!first.ValueIterator::isValid()) {
			return !error();
		}
		return first == other.first;
	}

	inline bool JsonifierResult<ObjectIterator>::operator!=(const JsonifierResult<ObjectIterator>& other) const noexcept {
		if (!first.ValueIterator::isValid()) {
			return error();
		}
		return first != other.first;
	}

	inline JsonifierResult<ObjectIterator>& JsonifierResult<ObjectIterator>::operator++() noexcept {
		if (error()) {
			second = Success;
			return *this;
		}
		++first;
		return *this;
	}
}
