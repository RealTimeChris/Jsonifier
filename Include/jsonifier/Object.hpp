#pragma once

#include <jsonifier/Base.hpp>
#include <jsonifier/ObjectIterator.hpp>
#include <jsonifier/Value.hpp>

namespace Jsonifier {

	class Jsonifier_Dll Object {
	  public:
		__forceinline Object() noexcept = default;
		__forceinline JsonifierResult<ObjectIterator> begin() noexcept;
		__forceinline JsonifierResult<ObjectIterator> end() noexcept;
		__forceinline JsonifierResult<Value> findField(std::string_view key) & noexcept;
		__forceinline JsonifierResult<Value> findField(std::string_view key) && noexcept;
		__forceinline JsonifierResult<Value> findFieldUnordered(std::string_view key) & noexcept;
		__forceinline JsonifierResult<Value> findFieldUnordered(std::string_view key) && noexcept;
		__forceinline JsonifierResult<Value> operator[](std::string_view key) & noexcept;
		__forceinline JsonifierResult<Value> operator[](std::string_view key) && noexcept;
		__forceinline JsonifierResult<Value> atPointer(std::string_view jsonPointer) noexcept;
		__forceinline JsonifierResult<bool> reset() & noexcept;
		__forceinline JsonifierResult<bool> isEmpty() & noexcept;
		__forceinline JsonifierResult<size_t> countFields() & noexcept;
		__forceinline JsonifierResult<std::string_view> rawJson() noexcept;

	  protected:
		ValueIterator iterator{}; 
		__forceinline ErrorCode consume() noexcept;
		__forceinline static JsonifierResult<Object> start(ValueIterator& iterator) noexcept;
		__forceinline static JsonifierResult<Object> startRoot(ValueIterator& iterator) noexcept;
		__forceinline static JsonifierResult<Object> started(ValueIterator& iterator) noexcept;
		__forceinline static Object resume(const ValueIterator& iterator) noexcept;
		__forceinline Object(const ValueIterator& iterator) noexcept;
		__forceinline ErrorCode findFieldRaw(const std::string_view key) noexcept;

		friend class Value;
		friend class Document;
		friend struct JsonifierResult<Object>;
	};

	template<> struct JsonifierResult<Object> : public JsonifierResultBase<Object> {
	  public:
		__forceinline JsonifierResult(Object&& Value) noexcept;
		__forceinline JsonifierResult(ErrorCode error) noexcept;
		__forceinline JsonifierResult() noexcept = default;

		__forceinline JsonifierResult<ObjectIterator> begin() noexcept;
		__forceinline JsonifierResult<ObjectIterator> end() noexcept;
		__forceinline JsonifierResult<Value> findField(std::string_view key) & noexcept;
		__forceinline JsonifierResult<Value> findField(std::string_view key) && noexcept;
		__forceinline JsonifierResult<Value> findFieldUnordered(std::string_view key) & noexcept;
		__forceinline JsonifierResult<Value> findFieldUnordered(std::string_view key) && noexcept;
		__forceinline JsonifierResult<Value> operator[](std::string_view key) & noexcept;
		__forceinline JsonifierResult<Value> operator[](std::string_view key) && noexcept;
		__forceinline JsonifierResult<Value> atPointer(std::string_view jsonPointer) noexcept;
		__forceinline JsonifierResult<bool> reset() noexcept;
		__forceinline JsonifierResult<bool> isEmpty() noexcept;
		__forceinline JsonifierResult<size_t> countFields() & noexcept;
	};

}
