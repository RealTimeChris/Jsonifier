#pragma once

#include "FoundationEntities.hpp"
#include "Value.hpp"

namespace Jsonifier {

	class Jsonifier_Dll ArrayIterator {
	  public:
		inline ArrayIterator() noexcept = default;
		inline JsonifierResult<Value> operator*() noexcept;
		inline bool operator==(const ArrayIterator&) const noexcept;
		inline bool operator!=(const ArrayIterator&) const noexcept;
		inline ArrayIterator& operator++() noexcept;

	  private:
		ValueIterator iterator{};

		inline ArrayIterator(const ValueIterator& iter) noexcept;

		friend class Jsonifier_Dll Array;
		friend class Jsonifier_Dll Value;
		friend struct Jsonifier_Dll JsonifierResult<ArrayIterator>;
	};

	class Jsonifier_Dll Array {
	  public:
		inline Array() noexcept = default;
		inline JsonifierResult<ArrayIterator> begin() noexcept;
		inline JsonifierResult<ArrayIterator> end() noexcept;
		inline JsonifierResult<size_t> countElements() & noexcept;
		inline JsonifierResult<bool> isEmpty() & noexcept;
		inline JsonifierResult<bool> reset() & noexcept;
		inline JsonifierResult<Value> atPointer(std::string_view json_pointer) noexcept;
		inline JsonifierResult<std::string_view> rawJson() noexcept;
		inline JsonifierResult<Value> at(size_t index) noexcept;

	  protected:
		inline ErrorCode consume() noexcept;
		static inline JsonifierResult<Array> start(ValueIterator& iter) noexcept;
		static inline JsonifierResult<Array> startRoot(ValueIterator& iter) noexcept;
		static inline JsonifierResult<Array> started(ValueIterator& iter) noexcept;
		inline Array(const ValueIterator& iter) noexcept;
		ValueIterator iterator{};

		friend class Jsonifier_Dll Value;
		friend class Jsonifier_Dll Document;
		friend struct Jsonifier_Dll JsonifierResult<Value>;
		friend struct Jsonifier_Dll JsonifierResult<Array>;
		friend class Jsonifier_Dll ArrayIterator;
	};

	template<> struct JsonifierResult<ArrayIterator> : public JsonifierResultBase<ArrayIterator> {
	  public:
		inline JsonifierResult(ArrayIterator&& Value) noexcept;
		inline JsonifierResult(ErrorCode) noexcept;
		inline JsonifierResult() noexcept = default;

		inline JsonifierResult<Value> operator*() noexcept;
		inline bool operator==(const JsonifierResult<ArrayIterator>&) const noexcept;
		inline bool operator!=(const JsonifierResult<ArrayIterator>&) const noexcept;
		inline JsonifierResult<ArrayIterator>& operator++() noexcept;
	};

	template<>
	struct Jsonifier_Dll JsonifierResult<Array>
		: public JsonifierResultBase<Array> {
	  public:
		inline JsonifierResult(Array&& value) noexcept;///< @private
		inline JsonifierResult(ErrorCode error) noexcept;///< @private
		inline JsonifierResult() noexcept = default;

		inline JsonifierResult<ArrayIterator> begin() noexcept;
		inline JsonifierResult<ArrayIterator> end() noexcept;
		inline JsonifierResult<size_t> countElements() & noexcept;
		inline JsonifierResult<bool> isEmpty() & noexcept;
		inline JsonifierResult<bool> reset() & noexcept;
		inline JsonifierResult<Value> at(size_t index) noexcept;
		inline JsonifierResult<Value> atPointer(std::string_view json_pointer) noexcept;
	};

}