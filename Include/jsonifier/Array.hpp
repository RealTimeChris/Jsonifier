#pragma once

#include <jsonifier/FoundationEntities.hpp>
#include <jsonifier/Value.hpp>

namespace Jsonifier {

	class Document;
	class Array;
	class ValueIterator;

	class Jsonifier_Dll ArrayIterator : public ValueIterator {
	  public:
		ArrayIterator() noexcept = default;
		JsonifierResult<Value> operator*() noexcept;
		bool operator==(const ArrayIterator&) const noexcept;
		bool operator!=(const ArrayIterator&) const noexcept;
		ArrayIterator& operator++() noexcept;

	  private:
		ArrayIterator(const ValueIterator& iteratorNew) noexcept;

		friend class Array;
		friend class Value;
		friend struct JsonifierResult<ArrayIterator>;
		friend struct JsonifierResult<Array>;
		friend struct JsonifierResult<Document>;
		friend struct Document;
		friend struct JsonifierResult<Value>;
	};

	class Jsonifier_Dll Array {
	  public:
		Array() noexcept = default;
		JsonifierResult<ArrayIterator> begin() noexcept;
		JsonifierResult<ArrayIterator> end() noexcept;
		JsonifierResult<size_t> countElements() & noexcept;
		JsonifierResult<bool> isEmpty() & noexcept;
		JsonifierResult<bool> reset() & noexcept;
		JsonifierResult<Value> atPointer(std::string_view jsonPointer) noexcept;
		JsonifierResult<std::string_view> rawJson() noexcept;
		JsonifierResult<Value> at(size_t index) noexcept;

	  protected:
		ErrorCode consume() noexcept;
		static JsonifierResult<Array> start(ValueIterator& iteratorNew) noexcept;
		static JsonifierResult<Array> startRoot(ValueIterator& iteratorNew) noexcept;
		static JsonifierResult<Array> started(ValueIterator& iteratorNew) noexcept;
		Array(const ValueIterator& iteratorNew) noexcept;
		ValueIterator iterator{};

		friend class Value;
		friend class Document;
		friend struct JsonifierResult<Value>;
		friend struct JsonifierResult<Array>;
		friend class ArrayIterator;
	};

	template<> struct JsonifierResult<ArrayIterator> : public ImplementationJsonifierResultBase<ArrayIterator> {
	  public:
		inline JsonifierResult(ArrayIterator&& Value) noexcept;
		inline JsonifierResult(ErrorCode) noexcept;
		inline JsonifierResult() noexcept = default;

		inline JsonifierResult<Value> operator*() noexcept;
		inline bool operator==(const JsonifierResult<ArrayIterator>&) const noexcept;
		inline bool operator!=(const JsonifierResult<ArrayIterator>&) const noexcept;
		inline JsonifierResult<ArrayIterator>& operator++() noexcept;
	};

	template<> struct JsonifierResult<Array> : public ImplementationJsonifierResultBase<Array> {
	  public:
		inline JsonifierResult(Array&& value) noexcept;
		inline JsonifierResult(ErrorCode error) noexcept;
		inline JsonifierResult() noexcept = default;

		inline JsonifierResult<ArrayIterator> begin() noexcept;
		inline JsonifierResult<ArrayIterator> end() noexcept;
		inline JsonifierResult<size_t> countElements() & noexcept;
		inline JsonifierResult<bool> isEmpty() & noexcept;
		inline JsonifierResult<bool> reset() & noexcept;
		inline JsonifierResult<Value> at(size_t index) noexcept;
		inline JsonifierResult<Value> atPointer(std::string_view jsonPointer) noexcept;
	};

}