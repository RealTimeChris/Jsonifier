#pragma once

#include <jsonifier/Base.hpp>
#include <jsonifier/ValueIterator.hpp>
#include <jsonifier/Field.hpp>

namespace Jsonifier {

	class Jsonifier_Dll ObjectIterator {
	  public:
		ObjectIterator() noexcept = default;
		JsonifierResult<Field> operator*() noexcept;
		bool operator==(const ObjectIterator&) const noexcept;
		bool operator!=(const ObjectIterator&) const noexcept;
		ObjectIterator& operator++() noexcept;

	  protected:
		ValueIterator iterator{}; 
		ObjectIterator(const ValueIterator& iterator) noexcept;
		friend struct JsonifierResult<ObjectIterator>;
		friend class Object;
	};

	template<> struct JsonifierResult<ObjectIterator> : public JsonifierResultBase<ObjectIterator> {
	  public:
		JsonifierResult(ObjectIterator&& value) noexcept;
		JsonifierResult(ErrorCode error) noexcept;
		JsonifierResult() noexcept = default;

		JsonifierResult<Field> operator*() noexcept;
		bool operator==(const JsonifierResult<ObjectIterator>&) const noexcept;
		bool operator!=(const JsonifierResult<ObjectIterator>&) const noexcept;
		JsonifierResult<ObjectIterator>& operator++() noexcept;
	};

	__forceinline ObjectIterator::ObjectIterator(const ValueIterator& iteratorNew) noexcept : iterator{ iteratorNew } {};

	__forceinline JsonifierResult<Field> ObjectIterator::operator*() noexcept {
		ErrorCode error = iterator.error();
		if (error) {
			iterator.abandon();
			return error;
		}
		auto result = Field::start(this->iterator);
		if (result.error()) {
			iterator.abandon();
		}
		return result;
	}

	__forceinline bool ObjectIterator::operator==(const ObjectIterator& other) const noexcept {
		return !(*this != other);
	}

	__forceinline bool ObjectIterator::operator!=(const ObjectIterator&) const noexcept {
		return iterator.isOpen();
	}

	__forceinline ObjectIterator& ObjectIterator::operator++() noexcept {
		if (!iterator.isOpen()) {
			return *this;
		}
		ErrorCode error{};
		if ((error = iterator.skipChild())) {
			return *this;
		}

		bool hasValue{};
		if ((error = iterator.hasNextField().get(hasValue))) {
			return *this;
		};
		return *this;
	}

	__forceinline JsonifierResult<ObjectIterator>::JsonifierResult(ObjectIterator&& value) noexcept
		: JsonifierResultBase<ObjectIterator>{ std::move(value) } {
		this->first.iterator.assertIsValid();
	}

	__forceinline JsonifierResult<ObjectIterator>::JsonifierResult(ErrorCode error) noexcept : JsonifierResultBase<ObjectIterator>{ error } {};

	__forceinline JsonifierResult<Field> JsonifierResult<ObjectIterator>::operator*() noexcept {
		if (error()) {
			return error();
		}
		return *first;
	}

	__forceinline bool JsonifierResult<ObjectIterator>::operator==(const JsonifierResult<ObjectIterator>& other) const noexcept {
		if (!first.iterator.isValid()) {
			return !error();
		}
		return first == other.first;
	}

	__forceinline bool JsonifierResult<ObjectIterator>::operator!=(const JsonifierResult<ObjectIterator>& other) const noexcept {
		if (!first.iterator.isValid()) {
			return error();
		}
		return first != other.first;
	}

	__forceinline JsonifierResult<ObjectIterator>& JsonifierResult<ObjectIterator>::operator++() noexcept {
		if (error()) {
			second = Success;
			return *this;
		}
		++first;
		return *this;
	}

}
