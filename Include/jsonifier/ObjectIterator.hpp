#pragma once

#include <jsonifier/Base.hpp>
#include <jsonifier/ValueIterator.hpp>

namespace Jsonifier {

	class Field;

	class Jsonifier_Dll ObjectIterator : public ValueIterator {
	  public:
		ObjectIterator() noexcept = default;
		JsonifierResult<Field> operator*() noexcept;
		bool operator==(const ObjectIterator&) const noexcept;
		bool operator!=(const ObjectIterator&) const noexcept;
		ObjectIterator& operator++() noexcept;

	  protected:
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

}
