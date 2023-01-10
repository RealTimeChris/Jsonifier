#pragma once

#include <jsonifier/Base.hpp>
#include <jsonifier/ValueIterator.hpp>
#include <jsonifier/Value.hpp>

namespace Jsonifier {

	class ArrayIterator;
	class Field;

	class Jsonifier_Dll Value : public ValueIterator {
	  public:
		__forceinline Value() noexcept = default;
		template<typename T> __forceinline JsonifierResult<T> get() noexcept;
		template<typename T> __forceinline ErrorCode get(T& out) noexcept;
		__forceinline JsonifierResult<Array> getArray() noexcept;
		__forceinline JsonifierResult<Object> getObject() noexcept;
		__forceinline JsonifierResult<uint64_t> getUint64() noexcept;
		__forceinline JsonifierResult<int64_t> getInt64() noexcept;
		__forceinline JsonifierResult<double> getDouble() noexcept;
		__forceinline JsonifierResult<std::string_view> getString() noexcept;
		__forceinline JsonifierResult<RawJsonString> getRawJsonString() noexcept;
		__forceinline JsonifierResult<bool> getBool() noexcept;
		__forceinline JsonifierResult<bool> isNull() noexcept;
		__forceinline operator Array() noexcept(false);
		__forceinline operator Object() noexcept(false);
		__forceinline operator uint64_t() noexcept(false);
		__forceinline operator int64_t() noexcept(false);
		__forceinline operator double() noexcept(false);
		__forceinline operator std::string_view() noexcept(false);
		__forceinline operator RawJsonString() noexcept(false);
		__forceinline operator bool() noexcept(false);
		__forceinline JsonifierResult<ArrayIterator> begin() & noexcept;
		__forceinline JsonifierResult<ArrayIterator> end() & noexcept;
		__forceinline JsonifierResult<size_t> countElements() & noexcept;
		__forceinline JsonifierResult<size_t> countFields() & noexcept;
		__forceinline JsonifierResult<Value> at(size_t index) noexcept;
		__forceinline JsonifierResult<Value> findField(std::string_view key) noexcept;
		__forceinline JsonifierResult<Value> findField(const char* key) noexcept;
		__forceinline JsonifierResult<Value> findFieldUnordered(std::string_view key) noexcept;
		__forceinline JsonifierResult<Value> findFieldUnordered(const char* key) noexcept;
		__forceinline JsonifierResult<Value> operator[](std::string_view key) noexcept;
		__forceinline JsonifierResult<Value> operator[](const char* key) noexcept;
		__forceinline JsonifierResult<JsonType> type() noexcept;
		__forceinline JsonifierResult<bool> isScalar() noexcept;
		__forceinline std::string_view rawJsonToken() noexcept;
		__forceinline JsonifierResult<const char*> currentLocation() noexcept;
		__forceinline int32_t currentDepth() const noexcept;
		__forceinline JsonifierResult<Value> atPointer(std::string_view json_pointer) noexcept;

	  protected:
		__forceinline Value(const ValueIterator& iter) noexcept;
		__forceinline void skip() noexcept;
		__forceinline static Value start(const ValueIterator& iter) noexcept;
		__forceinline static Value resume(const ValueIterator& iter) noexcept;
		__forceinline JsonifierResult<Object> startOrResumeObject() noexcept;

		friend class Document;
		friend class ArrayIterator;
		friend class Field;
		friend class Object;
		friend struct JsonifierResult<Value>;
		friend struct JsonifierResult<Field>;
	};

	template<> struct JsonifierResult<Value> : public JsonifierResultBase<Value> {
	  public:
		__forceinline JsonifierResult(Value&& Value) noexcept;
		__forceinline JsonifierResult(ErrorCode error) noexcept;
		__forceinline JsonifierResult() noexcept = default;
		__forceinline JsonifierResult<Array> getArray() noexcept;
		__forceinline JsonifierResult<Object> getObject() noexcept;

		__forceinline JsonifierResult<uint64_t> getUint64() noexcept;
		__forceinline JsonifierResult<int64_t> getInt64() noexcept;
		__forceinline JsonifierResult<double> getDouble() noexcept;
		__forceinline JsonifierResult<std::string_view> getString() noexcept;
		__forceinline JsonifierResult<RawJsonString> getRawJsonString() noexcept;
		__forceinline JsonifierResult<bool> getBool() noexcept;
		__forceinline JsonifierResult<bool> isNull() noexcept;

		template<typename T> __forceinline JsonifierResult<T> get() noexcept;

		template<typename T> __forceinline ErrorCode get(T& out) noexcept;

		__forceinline operator Array() noexcept(false);
		__forceinline operator Object() noexcept(false);
		__forceinline operator uint64_t() noexcept(false);
		__forceinline operator int64_t() noexcept(false);
		__forceinline operator double() noexcept(false);
		__forceinline operator std::string_view() noexcept(false);
		__forceinline operator RawJsonString() noexcept(false);
		__forceinline operator bool() noexcept(false);
		__forceinline JsonifierResult<size_t> countElements() & noexcept;
		__forceinline JsonifierResult<size_t> countFields() & noexcept;
		__forceinline JsonifierResult<Value> at(size_t index) noexcept;
		__forceinline JsonifierResult<ArrayIterator> begin() & noexcept;
		__forceinline JsonifierResult<ArrayIterator> end() & noexcept;
		__forceinline JsonifierResult<Value> findField(std::string_view key) noexcept;
		__forceinline JsonifierResult<Value> findField(const char* key) noexcept;
		__forceinline JsonifierResult<Value> findFieldUnordered(std::string_view key) noexcept;
		__forceinline JsonifierResult<Value> findFieldUnordered(const char* key) noexcept;
		__forceinline JsonifierResult<Value> operator[](std::string_view key) noexcept;
		__forceinline JsonifierResult<Value> operator[](const char* key) noexcept;
		__forceinline JsonifierResult<JsonType> type() noexcept;
		__forceinline JsonifierResult<bool> isScalar() noexcept;
		__forceinline JsonifierResult<std::string_view> rawJsonToken() noexcept;
		__forceinline JsonifierResult<const char*> currentLocation() noexcept;
		__forceinline JsonifierResult<int32_t> currentDepth() const noexcept;
		__forceinline JsonifierResult<Value> atPointer(std::string_view json_pointer) noexcept;
	};

}
