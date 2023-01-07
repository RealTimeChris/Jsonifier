#pragma once

#include <jsonifier/ImplementationJsonifierResultBase_Impl.hpp>
#include <jsonifier/Value_Impl.hpp>

namespace Jsonifier {

	inline JsonifierResult<ObjectIterator>::JsonifierResult(ObjectIterator&& value) noexcept
		: ImplementationJsonifierResultBase<ObjectIterator>(std::forward<ObjectIterator>(value)) {
		this->first.assertIsValid();
	}

	inline JsonifierResult<ObjectIterator>::JsonifierResult(ErrorCode error) noexcept : ImplementationJsonifierResultBase<ObjectIterator>({}, error) {
	}

	inline JsonifierResult<Field> JsonifierResult<ObjectIterator>::operator*() noexcept {
		if (error()) {
			return error();
		}
		return *this->first;
	}

	inline bool JsonifierResult<ObjectIterator>::operator==(const JsonifierResult<ObjectIterator>& other) const noexcept {
		if (!this->first.isValid()) {
			return !error();
		}
		return this->first == other.first;
	}

	inline bool JsonifierResult<ObjectIterator>::operator!=(const JsonifierResult<ObjectIterator>& other) const noexcept {
		if (!this->first.isValid()) {
			return error();
		}
		return this->first != other.first;
	}

	inline JsonifierResult<ObjectIterator>& JsonifierResult<ObjectIterator>::operator++() noexcept {
		if (error()) {
			this->second = ErrorCode::Success;
			return *this;
		}
		++this->first;
		return *this;
	}

	inline JsonifierResult<Object>::JsonifierResult(Object&& value) noexcept
		: ImplementationJsonifierResultBase<Object>(std::forward<Object>(value)){}

	inline JsonifierResult<Object>::JsonifierResult(ErrorCode error) noexcept : ImplementationJsonifierResultBase<Object>(error){}

	inline JsonifierResult<ObjectIterator> JsonifierResult<Object>::begin() noexcept {
		if (error()) {
			return error();
		}
		return this->first.begin();
	}

	inline JsonifierResult<ObjectIterator> JsonifierResult<Object>::end() noexcept {
		if (error()) {
			return error();
		}
		return this->first.end();
	}

	JsonifierResult<Value> JsonifierResult<Object>::findFieldUnordered(std::string_view key) & noexcept {
		if (error()) {
			return error();
		}
		return this->first.findFieldUnordered(key);
	}

	JsonifierResult<Value> JsonifierResult<Object>::findFieldUnordered(std::string_view key) && noexcept {
		if (error()) {
			return error();
		}
		return std::forward<Object>(this->first).findFieldUnordered(key);
	}

	JsonifierResult<Value> JsonifierResult<Object>::operator[](std::string_view key) & noexcept {
		if (error()) {
			return error();
		}
		return this->first[key];
	}

	JsonifierResult<Value> JsonifierResult<Object>::operator[](std::string_view key) && noexcept {
		if (error()) {
			return error();
		}
		return std::forward<Object>(this->first)[key];
	}

	JsonifierResult<Value> JsonifierResult<Object>::findField(std::string_view key) & noexcept {
		if (error()) {
			return error();
		}
		return this->first.findField(key);
	}

	JsonifierResult<Value> JsonifierResult<Object>::findField(std::string_view key) && noexcept {
		if (error()) {
			return error();
		}
		return std::forward<Object>(this->first).findField(key);
	}

	JsonifierResult<Value> JsonifierResult<Object>::atPointer(std::string_view jsonPointer) noexcept {
		if (error()) {
			return error();
		}
		return this->first.atPointer(jsonPointer);
	}

	JsonifierResult<bool> JsonifierResult<Object>::reset() noexcept {
		if (error()) {
			return error();
		}
		return this->first.reset();
	}

	JsonifierResult<bool> JsonifierResult<Object>::isEmpty() noexcept {
		if (error()) {
			return error();
		}
		return this->first.isEmpty();
	}

	JsonifierResult<size_t> JsonifierResult<Object>::countFields() & noexcept {
		if (error()) {
			return error();
		}
		return this->first.countFields();
	}
}
