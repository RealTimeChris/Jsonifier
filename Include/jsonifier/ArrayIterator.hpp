#pragma once

#include <jsonifier/Base.hpp>
#include <jsonifier/Value.hpp>

namespace Jsonifier {

	class Array;
	class Document;
	class Value;

	class Jsonifier_Dll ArrayIterator : public ValueIterator {
	  public:
		__forceinline ArrayIterator() noexcept = default;
		__forceinline ArrayIterator(const ValueIterator& iteratorNew) noexcept : ValueIterator{ iteratorNew } {
		}

		__forceinline JsonifierResult<Value> operator*() noexcept {
			if (error()) {
				abandon();
				return error();
			}
			return Value(child());
		}

		__forceinline bool operator==(const ArrayIterator& other) const noexcept {
			return !(*this != other);
		}

		__forceinline bool operator!=(const ArrayIterator&) const noexcept {
			return ValueIterator::isOpen();
		}

		__forceinline ArrayIterator& operator++() noexcept {
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

	  protected:
		friend class Array;
		friend class Value;
		friend struct JsonifierResult<ArrayIterator>;
	};

	template<> struct JsonifierResult<ArrayIterator> : public JsonifierResultBase<ArrayIterator> {
	  public:
		JsonifierResult(ArrayIterator&& Value) noexcept;
		JsonifierResult(ErrorCode error) noexcept;
		JsonifierResult() noexcept = default;

		JsonifierResult<Value> __forceinline operator*() noexcept {
			if (error()) {
				return error();
			}
			return *first;
		}

		__forceinline bool operator==(const JsonifierResult<ArrayIterator>& other) const noexcept {
			if (!first.isValid()) {
				return !error();
			}
			return first == other.first;
		}

		__forceinline bool operator!=(const JsonifierResult<ArrayIterator>& other) const noexcept {
			if (!first.isValid()) {
				return error();
			}
			return first != other.first;
		}

		__forceinline JsonifierResult<ArrayIterator>& operator++() noexcept {
			if (error()) {
				second = Success;
				return *this;
			}
			++(first);
			return *this;
		}
	};

}
