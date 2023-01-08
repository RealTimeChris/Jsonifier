#pragma once

#include <jsonifier/Base.hpp>
#include <jsonifier/ValueIterator.hpp>

namespace Jsonifier {

	class Field;

	class Jsonifier_Dll ObjectIterator : public ValueIterator {
	  public:
		inline ObjectIterator() noexcept = default;
		inline JsonifierResult<Field> operator*() noexcept;
		inline bool operator==(const ObjectIterator&) const noexcept;
		inline bool operator!=(const ObjectIterator&) const noexcept;
		inline ObjectIterator& operator++() noexcept;

	  protected:
		inline ObjectIterator(const ValueIterator& iterator) noexcept;
		friend struct JsonifierResult<ObjectIterator>;
		friend class Object;
	};

	template<> struct JsonifierResult<ObjectIterator> : public ImplementationJsonifierResultBase<ObjectIterator> {
	  public:
		inline JsonifierResult(ObjectIterator&& value) noexcept;
		inline JsonifierResult(ErrorCode error) noexcept;
		inline JsonifierResult() noexcept = default;

		inline JsonifierResult<Field> operator*() noexcept;
		inline bool operator==(const JsonifierResult<ObjectIterator>&) const noexcept;
		inline bool operator!=(const JsonifierResult<ObjectIterator>&) const noexcept;
		inline JsonifierResult<ObjectIterator>& operator++() noexcept;
	};

}
