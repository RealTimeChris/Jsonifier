#pragma once

#include <jsonifier/Base.hpp>
#include <jsonifier/ArrayIterator.hpp>

namespace Jsonifier {

	class Value;
	class document;

	class Jsonifier_Dll Array : public ValueIterator {
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
		static JsonifierResult<Array> start(ValueIterator& iterator) noexcept;
		static JsonifierResult<Array> startRoot(ValueIterator& iterator) noexcept;
		static JsonifierResult<Array> started(ValueIterator& iterator) noexcept;
		Array(const ValueIterator& iterator) noexcept;

		friend class Value;
		friend class Document;
		friend struct JsonifierResult<Value>;
		friend struct JsonifierResult<Array>;
		friend class ArrayIterator;
	};

	template<> struct JsonifierResult<Array> : public JsonifierResultBase<Array> {
	  public:
		JsonifierResult(Array&& Value) noexcept;
		JsonifierResult(ErrorCode error) noexcept;
		JsonifierResult() noexcept = default;

		JsonifierResult<ArrayIterator> begin() noexcept;
		JsonifierResult<ArrayIterator> end() noexcept;
		JsonifierResult<size_t> countElements() & noexcept;
		JsonifierResult<bool> isEmpty() & noexcept;
		JsonifierResult<bool> reset() & noexcept;
		JsonifierResult<Value> at(size_t index) noexcept;
		JsonifierResult<Value> atPointer(std::string_view jsonPointer) noexcept;
	};

}
