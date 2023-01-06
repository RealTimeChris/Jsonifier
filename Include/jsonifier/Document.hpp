#pragma once

#include "FoundationEntities.hpp"
#include "JsonifierResult.hpp"

namespace Jsonifier {

	class ValueIterator;

	class Jsonifier_Dll Document {
	  public:
		inline Document() noexcept = default;
		inline Document(const Document& other) noexcept = delete;
		inline Document(Document&& other) noexcept = default;
		inline Document& operator=(const Document& other) noexcept = delete;
		inline Document& operator=(Document&& other) noexcept = default;
		inline JsonifierResult<Array> getArray() & noexcept;
		inline JsonifierResult<Object> getObject() & noexcept;
		inline JsonifierResult<uint64_t> getUint64() noexcept;
		inline JsonifierResult<int64_t> getInt64() noexcept;
		inline JsonifierResult<bool> getBool() noexcept;
		inline JsonifierResult<double> getDouble() noexcept;
		inline JsonifierResult<std::string_view> getString() noexcept;
		inline JsonifierResult<RawJsonString> getRawJsonString() noexcept;
		inline JsonifierResult<Value> getValue() noexcept;
		inline JsonifierResult<bool> isNull() noexcept;
		template<typename T> inline JsonifierResult<T> get() & noexcept {
			static_assert(!sizeof(T), "The get method with given type is not implemented by the Jsonifier library.");
		}
		template<typename T> inline JsonifierResult<T> get() && noexcept {
			static_assert(!sizeof(T), "The get method with given type is not implemented by the Jsonifier library.");
		}
		template<typename T> inline ErrorCode get(T& out) & noexcept;
		template<typename T> inline ErrorCode get(T& out) && noexcept;
		inline JsonifierResult<size_t> countElements() & noexcept;
		inline JsonifierResult<size_t> countFields() & noexcept;
		inline JsonifierResult<Value> at(size_t index) & noexcept;
		inline JsonifierResult<ArrayIterator> begin() & noexcept;
		inline JsonifierResult<ArrayIterator> end() & noexcept;
		inline JsonifierResult<Value> findField(std::string_view key) & noexcept;
		inline JsonifierResult<Value> findField(const char* key) & noexcept;
		inline JsonifierResult<Value> findFieldUnordered(std::string_view key) & noexcept;
		inline JsonifierResult<Value> findFieldUnordered(const char* key) & noexcept;
		inline JsonifierResult<Value> operator[](std::string_view key) & noexcept;
		inline JsonifierResult<Value> operator[](const char* key) & noexcept;
		inline JsonifierResult<JsonType> type() noexcept;
		inline JsonifierResult<bool> isScalar() noexcept;

	  protected:
		inline JsonifierResult<std::string_view> rawJsonToken() noexcept;
		inline void rewind() noexcept;
		inline bool isAlive() noexcept;
		inline JsonifierResult<const char*> currentLocation() noexcept;
		inline int32_t currentDepth() const noexcept;
		inline JsonifierResult<Value> atPointer(std::string_view json_pointer) noexcept;
		inline JsonifierResult<std::string_view> rawJson() noexcept;
		inline ErrorCode consume() noexcept;

		inline Document(JsonIterator&& iter) noexcept;
		inline const uint8_t* text(uint32_t idx) const noexcept;

		inline ValueIterator resumeValueIterator() noexcept;
		inline ValueIterator getRootValueIterator() noexcept;
		inline JsonifierResult<Object> startOrResumeObject() noexcept;
		static inline Document start(JsonIterator&& iter) noexcept;

		JsonIterator iterator{};
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

		inline JsonifierResult<Value> atPointer(std::string_view json_pointer) noexcept;
	};
}