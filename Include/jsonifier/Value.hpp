#pragma once

#include <jsonifier/Base.hpp>
#include <jsonifier/ValueIterator.hpp>
#include <jsonifier/Value.hpp>

namespace Jsonifier {

	class ArrayIterator;
	class Field;

	class Jsonifier_Dll Value : public ValueIterator {
	  public:
		Value() noexcept = default;
		template<typename T> JsonifierResult<T> get() noexcept;
		template<typename T> ErrorCode get(T& out) noexcept;
		JsonifierResult<Array> getArray() noexcept;
		JsonifierResult<Object> getObject() noexcept;
		JsonifierResult<uint64_t> getUint64() noexcept;
		JsonifierResult<int64_t> getInt64() noexcept;
		JsonifierResult<double> getDouble() noexcept;
		JsonifierResult<std::string_view> getString() noexcept;
		JsonifierResult<RawJsonString> getRawJsonString() noexcept;
		JsonifierResult<bool> getBool() noexcept;
		JsonifierResult<bool> isNull() noexcept;
		operator Array() noexcept(false);
		operator Object() noexcept(false);
		operator uint64_t() noexcept(false);
		operator int64_t() noexcept(false);
		operator double() noexcept(false);
		operator std::string_view() noexcept(false);
		operator RawJsonString() noexcept(false);
		operator bool() noexcept(false);
		JsonifierResult<ArrayIterator> begin() & noexcept;
		JsonifierResult<ArrayIterator> end() & noexcept;
		JsonifierResult<size_t> countElements() & noexcept;
		JsonifierResult<size_t> countFields() & noexcept;
		JsonifierResult<Value> at(size_t index) noexcept;
		JsonifierResult<Value> findField(std::string_view key) noexcept;
		JsonifierResult<Value> findField(const char* key) noexcept;
		JsonifierResult<Value> findFieldUnordered(std::string_view key) noexcept;
		JsonifierResult<Value> findFieldUnordered(const char* key) noexcept;
		JsonifierResult<Value> operator[](std::string_view key) noexcept;
		JsonifierResult<Value> operator[](const char* key) noexcept;
		JsonifierResult<JsonType> type() noexcept;
		JsonifierResult<bool> isScalar() noexcept;
		std::string_view rawJsonToken() noexcept;
		JsonifierResult<const char*> currentLocation() noexcept;
		int32_t currentDepth() const noexcept;
		JsonifierResult<Value> atPointer(std::string_view json_pointer) noexcept;

	  protected:
		Value(const ValueIterator& iter) noexcept;
		void skip() noexcept;
		static Value start(const ValueIterator& iter) noexcept;
		static Value resume(const ValueIterator& iter) noexcept;
		JsonifierResult<Object> startOrResumeObject() noexcept;

		friend class Document;
		friend class ArrayIterator;
		friend class Field;
		friend class Object;
		friend struct JsonifierResult<Value>;
		friend struct JsonifierResult<Field>;
	};

	template<> struct JsonifierResult<Value> : public JsonifierResultBase<Value> {
	  public:
		inline JsonifierResult(Value&& Value) noexcept;
		inline JsonifierResult(ErrorCode error) noexcept;
		JsonifierResult() noexcept = default;
		JsonifierResult<Array> getArray() noexcept;
		JsonifierResult<Object> getObject() noexcept;

		JsonifierResult<uint64_t> getUint64() noexcept;
		JsonifierResult<int64_t> getInt64() noexcept;
		JsonifierResult<double> getDouble() noexcept;
		JsonifierResult<std::string_view> getString() noexcept;
		JsonifierResult<RawJsonString> getRawJsonString() noexcept;
		JsonifierResult<bool> getBool() noexcept;
		JsonifierResult<bool> isNull() noexcept;

		template<typename T> JsonifierResult<T> get() noexcept;

		template<typename T> ErrorCode get(T& out) noexcept;

		operator Array() noexcept(false);
		operator Object() noexcept(false);
		operator uint64_t() noexcept(false);
		operator int64_t() noexcept(false);
		operator double() noexcept(false);
		operator std::string_view() noexcept(false);
		operator RawJsonString() noexcept(false);
		operator bool() noexcept(false);
		JsonifierResult<size_t> countElements() & noexcept;
		JsonifierResult<size_t> countFields() & noexcept;
		JsonifierResult<Value> at(size_t index) noexcept;
		JsonifierResult<ArrayIterator> begin() & noexcept;
		JsonifierResult<ArrayIterator> end() & noexcept;
		JsonifierResult<Value> findField(std::string_view key) noexcept;
		JsonifierResult<Value> findField(const char* key) noexcept;
		JsonifierResult<Value> findFieldUnordered(std::string_view key) noexcept;
		JsonifierResult<Value> findFieldUnordered(const char* key) noexcept;
		JsonifierResult<Value> operator[](std::string_view key) noexcept;
		JsonifierResult<Value> operator[](const char* key) noexcept;
		JsonifierResult<JsonType> type() noexcept;
		JsonifierResult<bool> isScalar() noexcept;
		JsonifierResult<std::string_view> rawJsonToken() noexcept;
		JsonifierResult<const char*> currentLocation() noexcept;
		JsonifierResult<int32_t> currentDepth() const noexcept;
		JsonifierResult<Value> atPointer(std::string_view json_pointer) noexcept;
	};

}
