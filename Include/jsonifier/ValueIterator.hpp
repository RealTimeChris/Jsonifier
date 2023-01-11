#pragma once

#include <jsonifier/JsonIterator.hpp>

namespace Jsonifier {

	class Jsonifier_Dll ValueIterator {
	  protected:
		JsonIterator* jsonIterator{};
		uint32_t* rootStructural{};
		uint32_t currentDepth{};

	  public:
		__forceinline ValueIterator() noexcept = default;
		__forceinline ErrorCode skipChild() noexcept;
		__forceinline bool atEnd() const noexcept;
		__forceinline bool atStart() const noexcept;
		__forceinline bool isOpen() const noexcept;
		__forceinline bool atFirstField() const noexcept;
		__forceinline void abandon() noexcept;
		__forceinline ValueIterator childValue() const noexcept;
		__forceinline int32_t depth() const noexcept;
		__forceinline JsonifierResult<JsonType> type() const noexcept;
		__forceinline JsonifierResult<bool> startObject() noexcept;
		__forceinline JsonifierResult<bool> startRootObject() noexcept;
		__forceinline JsonifierResult<bool> startedObject() noexcept;
		__forceinline JsonifierResult<bool> startedRootObject() noexcept;
		__forceinline JsonifierResult<bool> hasNextField() noexcept;
		__forceinline JsonifierResult<RawJsonString> fieldKey() noexcept;
		__forceinline ErrorCode fieldValue() noexcept;
		__forceinline ErrorCode findField(const std::string_view key) noexcept;
		__forceinline JsonifierResult<bool> findFieldRaw(const std::string_view key) noexcept;
		__forceinline JsonifierResult<bool> findFieldUnorderedRaw(const std::string_view key) noexcept;
		__forceinline JsonifierResult<bool> startArray() noexcept;
		__forceinline JsonifierResult<bool> startRootArray() noexcept;
		__forceinline JsonifierResult<bool> startedArray() noexcept;
		__forceinline JsonifierResult<bool> startedRootArray() noexcept;
		__forceinline JsonifierResult<bool> hasNextElement() noexcept;
		__forceinline ValueIterator child() const noexcept;
		__forceinline JsonifierResult<std::string_view> getString() noexcept;
		__forceinline JsonifierResult<RawJsonString> getRawJsonString() noexcept;
		__forceinline JsonifierResult<uint64_t> getUint64() noexcept;
		__forceinline JsonifierResult<int64_t> getInt64() noexcept;
		__forceinline JsonifierResult<double> getDouble() noexcept;
		__forceinline JsonifierResult<bool> getBool() noexcept;
		__forceinline JsonifierResult<bool> isNull() noexcept;
		__forceinline JsonifierResult<std::string_view> getRootString() noexcept;
		__forceinline JsonifierResult<RawJsonString> getRootRawJsonString() noexcept;
		__forceinline JsonifierResult<uint64_t> getRootUint64() noexcept;
		__forceinline JsonifierResult<int64_t> getRootInt64() noexcept;
		__forceinline JsonifierResult<double> getRootDouble() noexcept;
		__forceinline JsonifierResult<bool> getRootBool() noexcept;
		__forceinline bool isRootNull() noexcept;

		__forceinline ErrorCode error() const noexcept;
		__forceinline uint8_t*& stringBufLoc() noexcept;
		__forceinline const JsonIterator& jsonIter() const noexcept;
		__forceinline JsonIterator& jsonIter() noexcept;

		__forceinline void assertIsValid() const noexcept;
		__forceinline bool isValid() const noexcept;

	  protected:
		__forceinline JsonifierResult<bool> resetArray() noexcept;
		__forceinline JsonifierResult<bool> resetObject() noexcept;
		__forceinline void moveAtStart() noexcept;
		__forceinline void moveAtContainerStart() noexcept;
		__forceinline std::string toString() const noexcept;
		__forceinline ValueIterator(JsonIterator* jsonIter, uint32_t depth, uint32_t* start_index) noexcept;
		__forceinline JsonifierResult<bool> parseNull(const uint8_t* json) const noexcept;
		__forceinline JsonifierResult<bool> parseBool(const uint8_t* json) const noexcept;
		__forceinline const uint8_t* peekStart() const noexcept;
		__forceinline uint32_t peekStartLength() const noexcept;

		__forceinline void advanceScalar() noexcept;
		__forceinline void advanceRootScalar() noexcept;
		__forceinline void advanceNonRootScalar() noexcept;

		__forceinline const uint8_t* peekScalar() noexcept;
		__forceinline const uint8_t* peekRootScalar() noexcept;
		__forceinline const uint8_t* peekNonRootScalar() noexcept;


		__forceinline ErrorCode startContainer(uint8_t start_char) noexcept;
		__forceinline ErrorCode endContainer() noexcept;
		__forceinline JsonifierResult<const uint8_t*> advanceToValue() noexcept;

		__forceinline ErrorCode incorrectTypeError() const noexcept;
		__forceinline ErrorCode error_unless_more_tokens(uint32_t tokens = 1) const noexcept;

		__forceinline bool isAtStart() const noexcept;
		__forceinline bool isAtIteratorStart() const noexcept;
		__forceinline bool isAtKey() const noexcept;

		__forceinline void assertAtStart() const noexcept;
		__forceinline void assertAtContainerStart() const noexcept;
		__forceinline void assertAtRoot() const noexcept;
		__forceinline void assertAtChild() const noexcept;
		__forceinline void assertAtNext() const noexcept;
		__forceinline void assertAtNonRootStart() const noexcept;
		__forceinline uint32_t* startPosition() const noexcept;
		__forceinline uint32_t* position() const noexcept;
		__forceinline uint32_t* lastPosition() const noexcept;
		__forceinline uint32_t* endPosition() const noexcept;
		__forceinline ErrorCode reportError(ErrorCode error) noexcept;

		friend class Field;
		friend class Document;
		friend class Object;
		friend class Array;
		friend class Value;
	};

	template<> struct JsonifierResult<ValueIterator> : public JsonifierResultBase<ValueIterator> {
	  public:
		__forceinline JsonifierResult(ValueIterator&& Value) noexcept;
		__forceinline JsonifierResult(ErrorCode error) noexcept;
		__forceinline JsonifierResult() noexcept = default;
	};

}
