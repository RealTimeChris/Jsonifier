#pragma once

#include <jsonifier/Base.hpp>
#include <jsonifier/Value.hpp>

namespace Jsonifier {

	class Parser;
	class Array;
	class Object;
	class Value;
	class RawJsonString;
	class ArrayIterator;
	class ValueIterator;

	class Jsonifier_Dll Document : public JsonIterator {
	  public:
		Document() noexcept = default;
		Document(const Document& other) noexcept = delete;
		Document(Document&& other) noexcept = default;
		Document& operator=(const Document& other) noexcept = delete;
		Document& operator=(Document&& other) noexcept = default;
		JsonifierResult<Array> getArray() & noexcept;
		JsonifierResult<Object> getObject() & noexcept;
		JsonifierResult<uint64_t> getUint64() noexcept;
		JsonifierResult<int64_t> getInt64() noexcept;
		JsonifierResult<double> getDouble() noexcept;
		JsonifierResult<std::string_view> getString() noexcept;
		JsonifierResult<RawJsonString> getRawJsonString() noexcept;
		JsonifierResult<bool> getBool() noexcept;
		JsonifierResult<Value> getValue() noexcept;
		JsonifierResult<bool> isNull() noexcept;
		template<typename T> inline JsonifierResult<T> get() & noexcept;
		template<typename T> inline JsonifierResult<T> get() && noexcept;
		template<typename T> inline ErrorCode get(T& out) & noexcept;
		template<typename T> inline ErrorCode get(T& out) && noexcept;
		operator Array() & noexcept(false);
		operator Object() & noexcept(false);
		operator uint64_t() noexcept(false);
		operator int64_t() noexcept(false);
		operator double() noexcept(false);
		operator std::string_view() noexcept(false);
		operator RawJsonString() noexcept(false);
		operator bool() noexcept(false);
		operator Value() noexcept(false);
		JsonifierResult<size_t> countElements() & noexcept;
		JsonifierResult<size_t> countFields() & noexcept;
		JsonifierResult<Value> at(size_t index) & noexcept;
		JsonifierResult<ArrayIterator> begin() & noexcept;
		JsonifierResult<ArrayIterator> end() & noexcept;
		JsonifierResult<Value> findField(std::string_view key) & noexcept;
		JsonifierResult<Value> findField(const char* key) & noexcept;
		JsonifierResult<Value> findFieldUnordered(std::string_view key) & noexcept;
		JsonifierResult<Value> findFieldUnordered(const char* key) & noexcept;
		JsonifierResult<Value> operator[](std::string_view key) & noexcept;
		JsonifierResult<Value> operator[](const char* key) & noexcept;
		JsonifierResult<JsonType> type() noexcept;
		JsonifierResult<bool> isScalar() noexcept;
		JsonifierResult<std::string_view> rawJsonToken() noexcept;
		void rewind() noexcept;
		std::string toDebugString() noexcept;
		bool isAlive() noexcept;
		JsonifierResult<const char*> currentLocation() noexcept;
		int32_t currentDepth() const noexcept;
		JsonifierResult<Value> atPointer(std::string_view jsonPointer) noexcept;
		JsonifierResult<std::string_view> rawJson() noexcept;

	  protected:
		ErrorCode consume() noexcept;

		Document(JsonIterator&& iterator) noexcept;
		const uint8_t* text(uint32_t idx) const noexcept;

		ValueIterator resumeValueIterator() noexcept;
		ValueIterator getRootValueIterator() noexcept;
		JsonifierResult<Object> startOrResumeObject() noexcept;
		static Document start(JsonIterator&& iterator) noexcept;
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
		JsonifierResult(Document&& Value) noexcept;
		JsonifierResult(ErrorCode error) noexcept;
		JsonifierResult() noexcept = default;
		ErrorCode rewind() noexcept;
		ErrorCode error() noexcept;

		JsonifierResult<Array> getArray() & noexcept;
		JsonifierResult<Object> getObject() & noexcept;
		JsonifierResult<uint64_t> getUint64() noexcept;
		JsonifierResult<int64_t> getInt64() noexcept;
		JsonifierResult<double> getDouble() noexcept;
		JsonifierResult<std::string_view> getString() noexcept;
		JsonifierResult<RawJsonString> getRawJsonString() noexcept;
		JsonifierResult<bool> getBool() noexcept;
		JsonifierResult<Value> getValue() noexcept;
		JsonifierResult<bool> isNull() noexcept;

		template<typename T> inline JsonifierResult<T> get() & noexcept;
		template<typename T> inline JsonifierResult<T> get() && noexcept;

		template<typename T> inline ErrorCode get(T& out) & noexcept;
		template<typename T> inline ErrorCode get(T& out) && noexcept;

		operator Array() & noexcept(false);
		operator Object() & noexcept(false);
		operator uint64_t() noexcept(false);
		operator int64_t() noexcept(false);
		operator double() noexcept(false);
		operator std::string_view() noexcept(false);
		operator RawJsonString() noexcept(false);
		operator bool() noexcept(false);
		operator Value() noexcept(false);

		JsonifierResult<size_t> countElements() & noexcept;
		JsonifierResult<size_t> countFields() & noexcept;
		JsonifierResult<Value> at(size_t index) & noexcept;
		JsonifierResult<ArrayIterator> begin() & noexcept;
		JsonifierResult<ArrayIterator> end() & noexcept;
		JsonifierResult<Value> findField(std::string_view key) & noexcept;
		JsonifierResult<Value> findField(const char* key) & noexcept;
		JsonifierResult<Value> operator[](std::string_view key) & noexcept;
		JsonifierResult<Value> operator[](const char* key) & noexcept;
		JsonifierResult<Value> findFieldUnordered(std::string_view key) & noexcept;
		JsonifierResult<Value> findFieldUnordered(const char* key) & noexcept;
		JsonifierResult<JsonType> type() noexcept;
		JsonifierResult<bool> isScalar() noexcept;
		JsonifierResult<const char*> currentLocation() noexcept;
		int32_t currentDepth() noexcept;
		JsonifierResult<std::string_view> rawJsonToken() noexcept;

		JsonifierResult<Value> atPointer(std::string_view jsonPointer) noexcept;
	};


}
