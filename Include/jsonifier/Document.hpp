#pragma once

#include <jsonifier/Base.hpp>
#include <jsonifier/Value.hpp>

namespace Jsonifier {

	class Jsonifier_Dll Document : public JsonIterator {
	  public:
		__forceinline Document() noexcept = default;
		__forceinline Document(const Document& other) noexcept = delete;
		__forceinline Document(Document&& other) noexcept = default;
		__forceinline Document& operator=(const Document& other) noexcept = delete;
		__forceinline Document& operator=(Document&& other) noexcept = default;
		__forceinline JsonifierResult<Array> getArray() & noexcept;
		__forceinline JsonifierResult<Object> getObject() & noexcept;
		__forceinline JsonifierResult<uint64_t> getUint64() noexcept;
		__forceinline JsonifierResult<int64_t> getInt64() noexcept;
		__forceinline JsonifierResult<double> getDouble() noexcept;
		__forceinline JsonifierResult<std::string_view> getString() noexcept;
		__forceinline JsonifierResult<RawJsonString> getRawJsonString() noexcept;
		__forceinline JsonifierResult<bool> getBool() noexcept;
		__forceinline JsonifierResult<Value> getValue() noexcept;
		__forceinline JsonifierResult<bool> isNull() noexcept;
		template<typename T> __forceinline JsonifierResult<T> get() & noexcept;
		template<typename T> __forceinline JsonifierResult<T> get() && noexcept;
		template<typename T> __forceinline ErrorCode get(T& out) & noexcept;
		template<typename T> __forceinline ErrorCode get(T& out) && noexcept;
		__forceinline operator Array() & noexcept(false);
		__forceinline operator Object() & noexcept(false);
		__forceinline operator uint64_t() noexcept(false);
		__forceinline operator int64_t() noexcept(false);
		__forceinline operator double() noexcept(false);
		__forceinline operator std::string_view() noexcept(false);
		__forceinline operator RawJsonString() noexcept(false);
		__forceinline operator bool() noexcept(false);
		__forceinline operator Value() noexcept(false);
		__forceinline JsonifierResult<size_t> countElements() & noexcept;
		__forceinline JsonifierResult<size_t> countFields() & noexcept;
		__forceinline JsonifierResult<Value> at(size_t index) & noexcept;
		__forceinline JsonifierResult<ArrayIterator> begin() & noexcept;
		__forceinline JsonifierResult<ArrayIterator> end() & noexcept;
		__forceinline JsonifierResult<Value> findField(std::string_view key) & noexcept;
		__forceinline JsonifierResult<Value> findField(const char* key) & noexcept;
		__forceinline JsonifierResult<Value> findFieldUnordered(std::string_view key) & noexcept;
		__forceinline JsonifierResult<Value> findFieldUnordered(const char* key) & noexcept;
		__forceinline JsonifierResult<Value> operator[](std::string_view key) & noexcept;
		__forceinline JsonifierResult<Value> operator[](const char* key) & noexcept;
		__forceinline JsonifierResult<JsonType> type() noexcept;
		__forceinline JsonifierResult<bool> isScalar() noexcept;
		__forceinline JsonifierResult<std::string_view> rawJsonToken() noexcept;
		__forceinline void rewind() noexcept;
		__forceinline std::string toDebugString() noexcept;
		__forceinline bool isAlive() noexcept;
		__forceinline JsonifierResult<const char*> currentLocation() noexcept;
		__forceinline int32_t currentDepth() const noexcept;
		__forceinline JsonifierResult<Value> atPointer(std::string_view jsonPointer) noexcept;
		__forceinline JsonifierResult<std::string_view> rawJson() noexcept;

	  protected:
		__forceinline ErrorCode consume() noexcept;

		__forceinline Document(JsonIterator&& iterator) noexcept;
		__forceinline const uint8_t* text(uint32_t idx) const noexcept;

		__forceinline ValueIterator resumeValueIterator() noexcept;
		__forceinline ValueIterator getRootValueIterator() noexcept;
		__forceinline JsonifierResult<Object> startOrResumeObject() noexcept;
		__forceinline static Document start(JsonIterator&& iterator) noexcept;
		static constexpr uint32_t DOCUMENT_DEPTH = 0;

		friend class ArrayIterator;
		friend class Parser;
		friend class Object;
		friend class Array;
		friend class Field;
		friend class document_stream;
	};

	template<> struct JsonifierResult<Document> : public JsonifierResultBase<Document> {
	  public:
		__forceinline JsonifierResult(Document&& Value) noexcept;
		__forceinline JsonifierResult(ErrorCode error) noexcept;
		__forceinline JsonifierResult() noexcept = default;
		__forceinline ErrorCode rewind() noexcept;
		__forceinline ErrorCode error() noexcept;
		__forceinline JsonifierResult<Array> getArray() & noexcept;
		__forceinline JsonifierResult<Object> getObject() & noexcept;
		__forceinline JsonifierResult<uint64_t> getUint64() noexcept;
		__forceinline JsonifierResult<int64_t> getInt64() noexcept;
		__forceinline JsonifierResult<double> getDouble() noexcept;
		__forceinline JsonifierResult<std::string_view> getString() noexcept;
		__forceinline JsonifierResult<RawJsonString> getRawJsonString() noexcept;
		__forceinline JsonifierResult<bool> getBool() noexcept;
		__forceinline JsonifierResult<Value> getValue() noexcept;
		__forceinline JsonifierResult<bool> isNull() noexcept;

		template<typename T> __forceinline JsonifierResult<T> get() & noexcept;
		template<typename T> __forceinline JsonifierResult<T> get() && noexcept;

		template<typename T> __forceinline ErrorCode get(T& out) & noexcept;
		template<typename T> __forceinline ErrorCode get(T& out) && noexcept;

		__forceinline operator Array() & noexcept(false);
		__forceinline operator Object() & noexcept(false);
		__forceinline operator uint64_t() noexcept(false);
		__forceinline operator int64_t() noexcept(false);
		__forceinline operator double() noexcept(false);
		__forceinline operator std::string_view() noexcept(false);
		__forceinline operator RawJsonString() noexcept(false);
		__forceinline operator bool() noexcept(false);
		__forceinline operator Value() noexcept(false);

		__forceinline JsonifierResult<size_t> countElements() & noexcept;
		__forceinline JsonifierResult<size_t> countFields() & noexcept;
		__forceinline JsonifierResult<Value> at(size_t index) & noexcept;
		__forceinline JsonifierResult<ArrayIterator> begin() & noexcept;
		__forceinline JsonifierResult<ArrayIterator> end() & noexcept;
		__forceinline JsonifierResult<Value> findField(std::string_view key) & noexcept;
		__forceinline JsonifierResult<Value> findField(const char* key) & noexcept;
		__forceinline JsonifierResult<Value> operator[](std::string_view key) & noexcept;
		__forceinline JsonifierResult<Value> operator[](const char* key) & noexcept;
		__forceinline JsonifierResult<Value> findFieldUnordered(std::string_view key) & noexcept;
		__forceinline JsonifierResult<Value> findFieldUnordered(const char* key) & noexcept;
		__forceinline JsonifierResult<JsonType> type() noexcept;
		__forceinline JsonifierResult<bool> isScalar() noexcept;
		__forceinline JsonifierResult<const char*> currentLocation() noexcept;
		__forceinline int32_t currentDepth() noexcept;
		__forceinline JsonifierResult<std::string_view> rawJsonToken() noexcept;

		__forceinline JsonifierResult<Value> atPointer(std::string_view jsonPointer) noexcept;
	};
}
