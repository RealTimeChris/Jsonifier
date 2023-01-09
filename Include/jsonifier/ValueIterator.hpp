#pragma once

#include <jsonifier/JsonIterator.hpp>

namespace Jsonifier {

	class Document;
	class Object;
	class Array;
	class Value;
	class RawJsonString;

	class Jsonifier_Dll ValueIterator {
	  protected:
		JsonIterator* jsonIterator{};
		uint32_t currentDepth{};
		uint32_t* rootStructural{};

	  public:
		inline ValueIterator() noexcept = default;
		inline ErrorCode skipChild() noexcept;
		inline bool atEnd() const noexcept;
		inline bool atStart() const noexcept;
		inline bool isOpen() const noexcept;
		inline bool atFirstField() const noexcept;
		inline void abandon() noexcept;
		inline ValueIterator childValue() const noexcept;
		inline int32_t depth() const noexcept;
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
		inline ValueIterator(JsonIterator* jsonIter, uint32_t depth, uint32_t* start_index) noexcept;

		inline JsonifierResult<bool> parseNull(const uint8_t* json) const noexcept;
		inline JsonifierResult<bool> parseBool(const uint8_t* json) const noexcept;
		inline const uint8_t* peekStart() const noexcept;
		inline uint32_t peekStartLength() const noexcept;

		inline void advanceScalar() noexcept;
		inline void advanceRootScalar() noexcept;
		inline void advanceNonRootScalar() noexcept;

		inline const uint8_t* peekScalar() noexcept;
		inline const uint8_t* peekRootScalar() noexcept;
		inline const uint8_t* peekNonRootScalar() noexcept;


		inline ErrorCode startContainer(uint8_t start_char) noexcept;
		inline ErrorCode endContainer() noexcept;
		inline JsonifierResult<const uint8_t*> advanceToValue() noexcept;

		inline ErrorCode incorrectTypeError() const noexcept;
		inline ErrorCode error_unless_more_tokens(uint32_t tokens = 1) const noexcept;

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
		inline ErrorCode reportError(ErrorCode error) noexcept;

		friend class Document;
		friend class Object;
		friend class Array;
		friend class Value;
	};

	template<> struct JsonifierResult<ValueIterator> : public JsonifierResultBase<ValueIterator> {
	  public:
		inline JsonifierResult(ValueIterator&& Value) noexcept;
		inline JsonifierResult(ErrorCode error) noexcept;
		inline JsonifierResult() noexcept = default;
	};

}
