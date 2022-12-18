#pragma once

#include "FoundationEntities.hpp"
#include "Value.hpp"

namespace Jsonifier {

	class ArrayIterator {
	  public:
		inline ArrayIterator() noexcept = default;
		inline JsonifierResult<Value> operator*() noexcept;
		inline bool operator==(const ArrayIterator&) const noexcept;
		inline bool operator!=(const ArrayIterator&) const noexcept;
		inline ArrayIterator& operator++() noexcept;

	  private:
		ValueIterator iterator{};

		inline ArrayIterator(const ValueIterator& iter) noexcept;

		friend class Array;
		friend class Value;
		friend struct JsonifierResult<ArrayIterator>;
	};

	class Array {
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

		friend class Value;
		friend class Document;
		friend struct JsonifierResult<Value>;
		friend struct JsonifierResult<Array>;
		friend class ArrayIterator;
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
	struct JsonifierResult<Array>
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