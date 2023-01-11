#pragma once

#include <jsonifier/Base.hpp>
#include <jsonifier/ArrayIterator.hpp>

namespace Jsonifier {

	class Jsonifier_Dll Array {
	  public:
		__forceinline Array() noexcept = default;
		__forceinline JsonifierResult<ArrayIterator> begin() noexcept;
		__forceinline JsonifierResult<ArrayIterator> end() noexcept;
		__forceinline JsonifierResult<size_t> countElements() & noexcept;
		__forceinline JsonifierResult<bool> isEmpty() & noexcept;
		__forceinline JsonifierResult<bool> reset() & noexcept;
		__forceinline JsonifierResult<Value> atPointer(std::string_view jsonPointer) noexcept;
		__forceinline JsonifierResult<std::string_view> rawJson() noexcept;
		__forceinline JsonifierResult<Value> at(size_t index) noexcept;

	  protected:
		ValueIterator iterator{};
		__forceinline ErrorCode consume() noexcept;
		__forceinline static JsonifierResult<Array> start(ValueIterator& iterator) noexcept;
		__forceinline static JsonifierResult<Array> startRoot(ValueIterator& iterator) noexcept;
		__forceinline static JsonifierResult<Array> started(ValueIterator& iterator) noexcept;
		__forceinline Array(const ValueIterator& iterator) noexcept;

		friend class Value;
		friend class Document;
		friend struct JsonifierResult<Value>;
		friend struct JsonifierResult<Array>;
		friend class ArrayIterator;
	};

	template<> struct JsonifierResult<Array> : public JsonifierResultBase<Array> {
	  public:
		__forceinline JsonifierResult(Array&& Value) noexcept;
		__forceinline JsonifierResult(ErrorCode error) noexcept;
		__forceinline JsonifierResult() noexcept = default;
		__forceinline JsonifierResult<ArrayIterator> begin() noexcept;
		__forceinline JsonifierResult<ArrayIterator> end() noexcept;
		__forceinline JsonifierResult<size_t> countElements() & noexcept;
		__forceinline JsonifierResult<bool> isEmpty() & noexcept;
		__forceinline JsonifierResult<bool> reset() & noexcept;
		__forceinline JsonifierResult<Value> at(size_t index) noexcept;
		__forceinline JsonifierResult<Value> atPointer(std::string_view jsonPointer) noexcept;
	};

}
