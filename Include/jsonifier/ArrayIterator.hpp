#pragma once

#include <jsonifier/Base.hpp>
#include <jsonifier/Value.hpp>

namespace Jsonifier {

	class Array;
	class Document;
	class Value;

	class Jsonifier_Dll ArrayIterator : public ValueIterator {
	  public:
		ArrayIterator() noexcept = default;
		JsonifierResult<Value> operator*() noexcept;
		bool operator==(const ArrayIterator&) const noexcept;
		bool operator!=(const ArrayIterator&) const noexcept;
		ArrayIterator& operator++() noexcept;

	  protected:
		ArrayIterator(const ValueIterator& iterator) noexcept;

		friend class Array;
		friend class Value;
		friend struct JsonifierResult<ArrayIterator>;
	};

	template<> struct JsonifierResult<ArrayIterator> : public JsonifierResultBase<ArrayIterator> {
	  public:
		JsonifierResult(ArrayIterator&& Value) noexcept;
		JsonifierResult(ErrorCode error) noexcept;
		JsonifierResult() noexcept = default;

		JsonifierResult<Value> operator*() noexcept;
		bool operator==(const JsonifierResult<ArrayIterator>&) const noexcept;
		bool operator!=(const JsonifierResult<ArrayIterator>&) const noexcept;
		JsonifierResult<ArrayIterator>& operator++() noexcept;
	};

}
