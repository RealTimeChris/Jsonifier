#pragma once

#include <jsonifier/Base.hpp>
#include <jsonifier/Value.hpp>

namespace Jsonifier {

	class Array;
	class Document;
	class Value;

	class Jsonifier_Dll ArrayIterator : public ValueIterator {
	  public:
		inline ArrayIterator() noexcept = default;
		inline ArrayIterator(const ValueIterator& iteratorNew) noexcept : ValueIterator{ iteratorNew } {
		}

		inline JsonifierResult<Value> operator*() noexcept {
			if (error()) {
				abandon();
				return error();
			}
			return Value(child());
		}

		inline bool operator==(const ArrayIterator& other) const noexcept {
			return !(*this != other);
		}

		inline bool operator!=(const ArrayIterator&) const noexcept {
			return ValueIterator::isOpen();
		}

		inline ArrayIterator& operator++() noexcept {
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

		JsonifierResult<Value> inline operator*() noexcept {
			if (error()) {
				return error();
			}
			return *first;
		}

		inline bool operator==(const JsonifierResult<ArrayIterator>& other) const noexcept {
			if (!first.isValid()) {
				return !error();
			}
			return first == other.first;
		}

		inline bool operator!=(const JsonifierResult<ArrayIterator>& other) const noexcept {
			if (!first.isValid()) {
				return error();
			}
			return first != other.first;
		}

		inline JsonifierResult<ArrayIterator>& operator++() noexcept {
			if (error()) {
				second = Success;
				return *this;
			}
			++(first);
			return *this;
		}
	};

}
