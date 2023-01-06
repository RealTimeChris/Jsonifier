#pragma once

#include "FoundationEntities.hpp"

namespace Jsonifier {

	class Jsonifier_Dll JsonIterator;
	class Jsonifier_Dll ArrayIterator;
	class Jsonifier_Dll Document;

	class Jsonifier_Dll EnumStringConverter {
	  public:
		inline EnumStringConverter(ErrorCode error) {
			this->code = error;
		}
		inline operator std::string() {
			switch (this->code) {
				case ErrorCode::Empty: {
					return "Empty";
				}
				case ErrorCode::Tape_Error: {
					return "Tape Error";
				}
				case ErrorCode::Depth_Error: {
					return "Depth Error";
				}
				case ErrorCode::Success: {
					return "Success";
				}
				case ErrorCode::Parse_Error: {
					return "Parse Error";
				}
				case ErrorCode::String_Error: {
					return "String Error";
				}
				case ErrorCode::TAtom_Error: {
					return "TAtom Error";
				}
				case ErrorCode::FAtom_Error: {
					return "FAtom Error";
				}
				case ErrorCode::NAtom_Error: {
					return "NAtom Error";
				}
				case ErrorCode::Mem_Alloc_Error: {
					return "Mem Alloc";
				}
				case ErrorCode::Invalid_Number: {
					return "Invalid Number";
				}
				default: {
					return "Unknown Error";
				}
			}
		}

	  protected:
		ErrorCode code{};
	};

	struct JsonifierException : public std::runtime_error, std::string {
		JsonifierException(const std::string&, std::source_location = std::source_location::current()) noexcept;
	};

	class Jsonifier_Dll ValueIterator {
	  protected:
		JsonIterator* jsonIterator{};
		uint32_t* rootStructural{};
		size_t currentDepth{ 0 };

	  public:
		inline ValueIterator() noexcept = default;
		inline void startDocument() noexcept;
		inline ErrorCode skipChild() noexcept;
		inline bool atEnd() const noexcept;
		inline bool atStart() const noexcept;
		inline bool isOpen() const noexcept;
		inline bool atFirstField() const noexcept;
		inline void abandon() noexcept;
		inline ValueIterator childValue() const noexcept;
		inline size_t depth() const noexcept;
		inline JsonifierResult<JsonType> type() const noexcept;
		inline JsonifierResult<bool> startObject() noexcept;
		inline JsonifierResult<bool> startRootObject() noexcept;
		inline JsonifierResult<bool> startedObject() noexcept;
		inline JsonifierResult<bool> startedRootObject() noexcept;
		inline JsonifierResult<bool> hasNextField() noexcept;
		inline JsonifierResult<RawJsonString> fieldKey() noexcept;
		inline ErrorCode fieldValue() noexcept;
		inline ErrorCode findField(const std::string_view key) noexcept;
		inline JsonifierResult<bool> findFieldRaw(const std::string_view key) noexcept;
		inline JsonifierResult<bool> findFieldUnorderedRaw(const std::string_view key) noexcept;
		inline JsonifierResult<bool> startArray() noexcept;
		inline JsonifierResult<bool> startRootArray() noexcept;
		inline JsonifierResult<bool> startedArray() noexcept;
		inline JsonifierResult<bool> startedRootArray() noexcept;
		inline JsonifierResult<bool> hasNextElement() noexcept;
		inline ValueIterator child() const noexcept;

		inline JsonifierResult<std::string_view> getString() noexcept;
		inline JsonifierResult<RawJsonString> getRawJsonString() noexcept;
		inline JsonifierResult<uint64_t> getUint64() noexcept;
		inline JsonifierResult<int64_t> getInt64() noexcept;
		inline JsonifierResult<double> getDouble() noexcept;
		inline JsonifierResult<bool> getBool() noexcept;
		inline JsonifierResult<bool> isNull() noexcept;

		inline JsonifierResult<std::string_view> getRootString() noexcept;
		inline JsonifierResult<RawJsonString> getRootRawJsonString() noexcept;
		inline JsonifierResult<uint64_t> getRootUint64() noexcept;
		inline JsonifierResult<int64_t> getRootInt64() noexcept;
		inline JsonifierResult<double> getRootDouble() noexcept;
		inline JsonifierResult<bool> getRootBool() noexcept;
		inline bool isRootNull() noexcept;

		inline ErrorCode error() const noexcept;
		inline uint8_t*& stringBufLoc() noexcept;
		inline const JsonIterator& jsonIter() const noexcept;
		inline JsonIterator& jsonIter() noexcept;

		inline void assertIsValid() const noexcept;
		inline bool isValid() const noexcept;

	  protected:
		inline JsonifierResult<bool> resetArray() noexcept;
		inline JsonifierResult<bool> resetObject() noexcept;
		inline void moveAtStart() noexcept;
		inline void moveAtContainerStart() noexcept;
		inline std::string toString() const noexcept;
		inline ValueIterator(JsonIterator* json_iter, size_t depth, uint32_t* start_index) noexcept;

		inline JsonifierResult<bool> parseNull(const uint8_t* json) const noexcept;
		inline JsonifierResult<bool> parseBool(const uint8_t* json) const noexcept;
		inline const uint8_t* peekStart() const noexcept;
		inline uint32_t peekStartLength() const noexcept;

		inline void advanceScalar(const char* type) noexcept;
		inline void advanceRootScalar(const char* type) noexcept;
		inline void advanceNonRootScalar(const char* type) noexcept;

		inline const uint8_t* peekScalar(const char* type) noexcept;
		inline const uint8_t* peekRootScalar(const char* type) noexcept;
		inline const uint8_t* peekNonRootScalar(const char* type) noexcept;


		inline ErrorCode startContainer(uint8_t start_char, const char* incorrect_type_message, const char* type) noexcept;
		inline ErrorCode endContainer() noexcept;
		inline JsonifierResult<const uint8_t*> advanceToValue() noexcept;

		inline ErrorCode incorrectTypeError(const char* message) const noexcept;
		inline ErrorCode errorUnlessMoreTokens(uint32_t tokens = 1) const noexcept;

		inline bool isAtStart() const noexcept;
		inline bool isAtIteratorStart() const noexcept;
		inline bool isAtKey() const noexcept;

		inline void assertAtStart() const noexcept;
		inline void assertAtContainerStart() const noexcept;
		inline void assertAtRoot() const noexcept;
		inline void assertAtChild() const noexcept;
		inline void assertAtNext() const noexcept;
		inline void assertAtNonRootStart() const noexcept;
		inline uint32_t* startPosition() const noexcept;

		inline uint32_t* position() const noexcept;
		inline uint32_t* lastPosition() const noexcept;
		inline uint32_t* endPosition() const noexcept;
		inline ErrorCode reportError(ErrorCode error, const char* message) noexcept;
		friend struct JsonifierResult<Document>;
		friend class Document;
		friend class Object;
		friend class Array;
		friend class Value;
	};

	class Jsonifier_Dll Value {
	  public:
		inline Value() noexcept = default;
		template<typename T> inline JsonifierResult<T> get() noexcept {
			static_assert(!sizeof(T), "The get method with given type is not implemented by the Jsonifier library.");
		}
		template<typename T> inline ErrorCode get(T& out) noexcept;
		inline JsonifierResult<Array> getArray() noexcept;
		inline JsonifierResult<Object> getObject() noexcept;
		inline JsonifierResult<uint64_t> getUint64() noexcept;
		inline JsonifierResult<int64_t> getInt64() noexcept;
		inline JsonifierResult<double> getDouble() noexcept;
		inline JsonifierResult<std::string_view> getString() noexcept;
		inline JsonifierResult<RawJsonString> getRawJsonString() noexcept;
		inline JsonifierResult<bool> getBool() noexcept;
		inline JsonifierResult<bool> isNull() noexcept;
		inline JsonifierResult<ArrayIterator> begin() & noexcept;
		inline JsonifierResult<ArrayIterator> end() & noexcept;
		inline JsonifierResult<size_t> countElements() & noexcept;
		inline JsonifierResult<size_t> countFields() & noexcept;
		inline JsonifierResult<Value> at(size_t index) noexcept;
		inline JsonifierResult<Value> findField(std::string_view key) noexcept;
		inline JsonifierResult<Value> findField(const char* key) noexcept;
		inline JsonifierResult<Value> findFieldUnordered(std::string_view key) noexcept;
		inline JsonifierResult<Value> findFieldUnordered(const char* key) noexcept;
		inline JsonifierResult<Value> operator[](std::string_view key) noexcept;
		inline JsonifierResult<Value> operator[](const char* key) noexcept;
		inline JsonifierResult<JsonType> type() noexcept;
		inline JsonifierResult<bool> isScalar() noexcept;
		inline std::string_view rawJsonToken() noexcept;
		inline JsonifierResult<const char*> currentLocation() noexcept;
		inline int32_t currentDepth() const noexcept;
		inline JsonifierResult<Value> atPointer(std::string_view json_pointer) noexcept;
		inline operator Array() noexcept(false);
		inline operator Object() noexcept(false);
		inline operator uint64_t() noexcept(false);
		inline operator int64_t() noexcept(false);
		inline operator double() noexcept(false);
		inline operator std::string_view() noexcept(false);
		inline operator RawJsonString() noexcept(false);
		inline operator bool() noexcept(false);

	  protected:
		inline Value(const ValueIterator& iter) noexcept;
		inline void skip() noexcept;
		static inline Value start(const ValueIterator& iter) noexcept;
		static inline Value resume(const ValueIterator& iter) noexcept;
		inline JsonifierResult<Object> startOrResumeObject() noexcept;
		ValueIterator iterator;

		friend class Document;
		friend class ArrayIterator;
		friend class Field;
		friend class Object;
		friend struct JsonifierResult<Value>;
		friend struct JsonifierResult<Field>;
	};

	template<> struct JsonifierResult<Value> : public ImplementationJsonifierResultBase<Value> {
	  public:
		inline JsonifierResult(Value&& value) noexcept;
		inline JsonifierResult(ErrorCode error) noexcept;
		inline JsonifierResult() noexcept = default;

		inline JsonifierResult<Array> getArray() noexcept;
		inline JsonifierResult<Object> getObject() noexcept;

		inline JsonifierResult<uint64_t> getUint64() noexcept;
		inline JsonifierResult<int64_t> getInt64() noexcept;
		inline JsonifierResult<double> getDouble() noexcept;
		inline JsonifierResult<std::string_view> getString() noexcept;
		inline JsonifierResult<RawJsonString> getRawJsonString() noexcept;
		inline JsonifierResult<bool> getBool() noexcept;
		inline JsonifierResult<bool> isNull() noexcept;

		template<typename T> inline JsonifierResult<T> get() noexcept;

		template<typename T> inline ErrorCode get(T& out) noexcept;
		inline JsonifierResult<size_t> countElements() & noexcept;
		inline JsonifierResult<size_t> countFields() & noexcept;
		inline JsonifierResult<Value> at(size_t index) noexcept;
		inline JsonifierResult<ArrayIterator> begin() & noexcept;
		inline JsonifierResult<ArrayIterator> end() & noexcept;
		inline JsonifierResult<Value> findField(std::string_view key) noexcept;
		inline JsonifierResult<Value> findField(const char* key) noexcept;
		inline JsonifierResult<Value> findFieldUnordered(std::string_view key) noexcept;
		inline JsonifierResult<Value> findFieldUnordered(const char* key) noexcept;
		inline JsonifierResult<Value> operator[](std::string_view key) noexcept;
		inline JsonifierResult<Value> operator[](const char* key) noexcept;
		inline JsonifierResult<JsonType> type() noexcept;
		inline JsonifierResult<bool> isScalar() noexcept;
		inline JsonifierResult<std::string_view> rawJsonToken() noexcept;
		inline JsonifierResult<const char*> currentLocation() noexcept;
		inline JsonifierResult<int32_t> currentDepth() const noexcept;
		inline JsonifierResult<Value> atPointer(std::string_view json_pointer) noexcept;
		inline operator Array() noexcept(false);
		inline operator Object() noexcept(false);
		inline operator uint64_t() noexcept(false);
		inline operator int64_t() noexcept(false);
		inline operator double() noexcept(false);
		inline operator std::string_view() noexcept(false);
		inline operator RawJsonString() noexcept(false);
		inline operator bool() noexcept(false);
	};
}