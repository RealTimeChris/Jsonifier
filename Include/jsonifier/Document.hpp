#pragma once

#include <jsonifier/FoundationEntities.hpp>
#include <jsonifier/JsonifierResult.hpp>
#include <jsonifier/Value.hpp>

namespace Jsonifier {

	class ValueIterator;
	class ArrayIterator;

	class Jsonifier_Dll Document {
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
		JsonifierResult<bool> getBool() noexcept;
		JsonifierResult<double> getDouble() noexcept;
		JsonifierResult<std::string_view> getString() noexcept;
		JsonifierResult<RawJsonString> getRawJsonString() noexcept;
		JsonifierResult<Value> getValue() noexcept;
		JsonifierResult<bool> isNull() noexcept;
		template<typename T> JsonifierResult<T> get() & noexcept {
			static_assert(!sizeof(T), "The get method with given type is not implemented by the Jsonifier library.");
		}

	template<typename T> JsonifierResult<T> get() && noexcept {
			static_assert(!sizeof(T), "The get method with given type is not implemented by the Jsonifier library.");
		}

	template<typename T> ErrorCode get(T& out) & noexcept;
		template<typename T> ErrorCode get(T& out) && noexcept;
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
		operator Array() & noexcept(false);
		operator Object() & noexcept(false);
		operator uint64_t() noexcept(false);
		operator int64_t() noexcept(false);
		operator double() noexcept(false);
		operator std::string_view() noexcept(false);
		operator RawJsonString() noexcept(false);
		operator bool() noexcept(false);
		operator Value() noexcept(false);

	  protected:
		JsonifierResult<std::string_view> rawJsonToken() noexcept;
		void rewind() noexcept;
		bool isAlive() noexcept;
		JsonifierResult<const char*> currentLocation() noexcept;
		int32_t currentDepth() const noexcept;
		JsonifierResult<Value> atPointer(std::string_view jsonPointer) noexcept;
		JsonifierResult<std::string_view> rawJson() noexcept;
		ErrorCode consume() noexcept;

		Document(JsonIterator&& iteratorNew) noexcept;
		const uint8_t* text(uint32_t idx) const noexcept;

		ValueIterator resumeValueIterator() noexcept;
		ValueIterator getRootValueIterator() noexcept;
		JsonifierResult<Object> startOrResumeObject() noexcept;

		JsonIterator iterator;
		static constexpr size_t DOCUMENT_DEPTH = 0;
		friend class JsonifierResult<Document>;
		friend class ArrayIterator;
		friend class Parser;
		friend class Object;
		friend class Value;
		friend class Array;
		friend class Field;
		friend class Token;
	};

	template<> struct JsonifierResult<Document> : public ImplementationJsonifierResultBase<Document> {
	  public:
		inline JsonifierResult(Document&& Value) noexcept;
		inline JsonifierResult(ErrorCode error) noexcept;
		inline JsonifierResult() noexcept = default;
		inline ErrorCode rewind() noexcept;

		inline JsonifierResult<Array> getArray() & noexcept;
		inline JsonifierResult<Object> getObject() & noexcept;
		inline JsonifierResult<uint64_t> getUint64() noexcept;
		inline JsonifierResult<int64_t> getInt64() noexcept;
		inline JsonifierResult<double> getDouble() noexcept;
		inline JsonifierResult<std::string_view> getString() noexcept;
		inline JsonifierResult<RawJsonString> getRawJsonString() noexcept;
		inline JsonifierResult<bool> getBool() noexcept;
		inline JsonifierResult<Value> getValue() noexcept;
		inline JsonifierResult<bool> isNull() noexcept;

		template<typename T> inline JsonifierResult<T> get() & noexcept;
		template<typename T> inline JsonifierResult<T> get() && noexcept;

		template<typename T> inline ErrorCode get(T& out) & noexcept;
		template<typename T> inline ErrorCode get(T& out) && noexcept;

		inline operator Array() & noexcept(false);
		inline operator Object() & noexcept(false);
		inline operator uint64_t() noexcept(false);
		inline operator int64_t() noexcept(false);
		inline operator double() noexcept(false);
		inline operator std::string_view() noexcept(false);
		inline operator RawJsonString() noexcept(false);
		inline operator bool() noexcept(false);
		inline operator Value() noexcept(false);

		inline JsonifierResult<size_t> countElements() & noexcept;
		inline JsonifierResult<size_t> countFields() & noexcept;
		inline JsonifierResult<Value> at(size_t index) & noexcept;
		inline JsonifierResult<ArrayIterator> begin() & noexcept;
		inline JsonifierResult<ArrayIterator> end() & noexcept;
		inline JsonifierResult<Value> findField(std::string_view key) & noexcept;
		inline JsonifierResult<Value> findField(const char* key) & noexcept;
		inline JsonifierResult<Value> operator[](std::string_view key) & noexcept;
		inline JsonifierResult<Value> operator[](const char* key) & noexcept;
		inline JsonifierResult<Value> findFieldUnordered(std::string_view key) & noexcept;
		inline JsonifierResult<Value> findFieldUnordered(const char* key) & noexcept;
		inline JsonifierResult<JsonType> type() noexcept;
		inline JsonifierResult<bool> isScalar() noexcept;
		inline JsonifierResult<const char*> currentLocation() noexcept;
		inline int32_t currentDepth() const noexcept;
		inline JsonifierResult<std::string_view> rawJsonToken() noexcept;

		inline JsonifierResult<Value> atPointer(std::string_view jsonPointer) noexcept;
	};
}