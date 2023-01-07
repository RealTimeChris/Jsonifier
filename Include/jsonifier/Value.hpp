#pragma once

#include <jsonifier/FoundationEntities.hpp>

namespace Jsonifier {

	class Jsonifier_Dll JsonIterator;
	class Jsonifier_Dll ArrayIterator;
	class Jsonifier_Dll Document;

	class Jsonifier_Dll EnumStringConverter {
	  public:
		EnumStringConverter(ErrorCode error) {
			this->code = error;
		}

		operator std::string() {
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
		size_t currentDepth{};
		uint32_t* rootStructural{};

	  public:
		ValueIterator() noexcept = default;
		void startDocument() noexcept;
		ErrorCode skipChild() noexcept;
		bool atEnd() const noexcept;
		bool atStart() const noexcept;
		bool isOpen() const noexcept;
		bool atFirstField() const noexcept;
		void abandon() noexcept;
		ValueIterator childValue() const noexcept;
		size_t depth() const noexcept;
		JsonifierResult<JsonType> type() const noexcept;
		JsonifierResult<bool> startObject() noexcept;
		JsonifierResult<bool> startRootObject() noexcept;
		JsonifierResult<bool> startedObject() noexcept;
		JsonifierResult<bool> startedRootObject() noexcept;
		JsonifierResult<bool> hasNextField() noexcept;
		JsonifierResult<bool> starRootArray() noexcept;
		JsonifierResult<RawJsonString> fieldKey() noexcept;
		ErrorCode fieldValue() noexcept;
		ErrorCode findField(const std::string_view key) noexcept;
		JsonifierResult<bool> findFieldRaw(const std::string_view key) noexcept;
		JsonifierResult<bool> findFieldUnorderedRaw(const std::string_view key) noexcept;
		JsonifierResult<bool> startArray() noexcept;
		JsonifierResult<bool> startRootArray() noexcept;
		JsonifierResult<bool> startedArray() noexcept;
		JsonifierResult<bool> startedRootArray() noexcept;
		JsonifierResult<bool> hasNextElement() noexcept;
		ValueIterator child() const noexcept;

		JsonifierResult<std::string_view> getString() noexcept;
		JsonifierResult<RawJsonString> getRawJsonString() noexcept;
		JsonifierResult<uint64_t> getUint64() noexcept;
		JsonifierResult<int64_t> getInt64() noexcept;
		JsonifierResult<double> getDouble() noexcept;
		JsonifierResult<bool> getBool() noexcept;
		JsonifierResult<bool> isNull() noexcept;

		JsonifierResult<std::string_view> getRootString() noexcept;
		JsonifierResult<RawJsonString> getRootRawJsonString() noexcept;
		JsonifierResult<uint64_t> getRootUint64() noexcept;
		JsonifierResult<int64_t> getRootInt64() noexcept;
		JsonifierResult<double> getRootDouble() noexcept;
		JsonifierResult<bool> getRootBool() noexcept;
		bool isRootNull() noexcept;

		ErrorCode error() const noexcept;
		uint8_t*& stringBufLoc() noexcept;
		const JsonIterator& jsonIter() const noexcept;
		JsonIterator& jsonIter() noexcept;

		void assertIsValid() const noexcept;
		bool isValid() const noexcept;

	  protected:
		JsonifierResult<bool> resetArray() noexcept;
		JsonifierResult<bool> resetObject() noexcept;
		void moveAtStart() noexcept;
		void moveAtContainerStart() noexcept;
		std::string toString() const noexcept;
		ValueIterator(JsonIterator* json_iter, size_t depth, uint32_t* start_index) noexcept;

		JsonifierResult<bool> parseNull(const uint8_t* json) const noexcept;
		JsonifierResult<bool> parseBool(const uint8_t* json) const noexcept;
		const uint8_t* peekStart() const noexcept;
		uint32_t peekStartLength() const noexcept;

		void advanceScalar(const char* type) noexcept;
		void advanceRootScalar(const char* type) noexcept;
		void advanceNonRootScalar(const char* type) noexcept;

		const uint8_t* peekScalar(const char* type) noexcept;
		const uint8_t* peekRootScalar(const char* type) noexcept;
		const uint8_t* peekNonRootScalar(const char* type) noexcept;


		ErrorCode startContainer(uint8_t start_char, const char* incorrect_type_message, const char* type) noexcept;
		ErrorCode endContainer() noexcept;
		JsonifierResult<const uint8_t*> advanceToValue() noexcept;

		ErrorCode incorrectTypeError(const char* message) const noexcept;
		ErrorCode errorUnlessMoreTokens(uint32_t tokens = 1) const noexcept;

		bool isAtStart() const noexcept;
		bool isAtIteratorStart() const noexcept;
		bool isAtKey() const noexcept;

		void assertAtStart() const noexcept;
		void assertAtContainerStart() const noexcept;
		void assertAtRoot() const noexcept;
		void assertAtChild() const noexcept;
		void assertAtNext() const noexcept;
		void assertAtNonRootStart() const noexcept;
		uint32_t* startPosition() const noexcept;

		uint32_t* position() const noexcept;
		uint32_t* lastPosition() const noexcept;
		uint32_t* endPosition() const noexcept;
		ErrorCode reportError(ErrorCode error, const char* message) noexcept;
		friend struct JsonifierResult<Document>;
		friend class Document;
		friend class Object;
		friend class Array;
		friend class Value;
	};

	class Jsonifier_Dll Value {
	  public:
		Value() noexcept = default;
		template<typename T> JsonifierResult<T> get() noexcept {
			static_assert(!sizeof(T), "The get method with given type is not implemented by the Jsonifier library.");
		}

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
		JsonifierResult<Value> atPointer(std::string_view jsonPointer) noexcept;
		operator Array() noexcept(false);
		operator Object() noexcept(false);
		operator uint64_t() noexcept(false);
		operator int64_t() noexcept(false);
		operator double() noexcept(false);
		operator std::string_view() noexcept(false);
		operator RawJsonString() noexcept(false);
		operator bool() noexcept(false);

	  protected:
		Value(const ValueIterator& iteratorNew) noexcept;
		void skip() noexcept;
		static Value start(const ValueIterator& iteratorNew) noexcept;
		static Value resume(const ValueIterator& iteratorNew) noexcept;
		JsonifierResult<Object> startOrResumeObject() noexcept;
		ValueIterator iterator{};

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
		inline JsonifierResult<Value> atPointer(std::string_view jsonPointer) noexcept;
		inline operator Array() noexcept(false);
		inline operator Object() noexcept(false);
		inline operator uint64_t() noexcept(false);
		inline operator int64_t() noexcept(false);
		inline operator double() noexcept(false);
		inline operator std::string_view() noexcept(false);
		inline operator RawJsonString() noexcept(false);
		inline operator bool() noexcept(false);
	};

	template<> struct JsonifierResult<ValueIterator> : public ImplementationJsonifierResultBase<ValueIterator> {
	  public:
		inline JsonifierResult(ValueIterator&& value) noexcept;
		inline JsonifierResult(ErrorCode error) noexcept;
		inline JsonifierResult() noexcept = default;
	};
}