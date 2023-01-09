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
		ValueIterator() noexcept = default;
		ErrorCode skipChild() noexcept;
		bool atEnd() const noexcept;
		bool atStart() const noexcept;
		bool isOpen() const noexcept;
		bool atFirstField() const noexcept;
		void abandon() noexcept;
		ValueIterator childValue() const noexcept;
		int32_t depth() const noexcept;
		JsonifierResult<JsonType> type() const noexcept;
		JsonifierResult<bool> startObject() noexcept;
		JsonifierResult<bool> startRootObject() noexcept;
		JsonifierResult<bool> startedObject() noexcept;
		JsonifierResult<bool> startedRootObject() noexcept;
		JsonifierResult<bool> hasNextField() noexcept;
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
		ValueIterator(JsonIterator* jsonIter, uint32_t depth, uint32_t* start_index) noexcept;

		JsonifierResult<bool> parseNull(const uint8_t* json) const noexcept;
		JsonifierResult<bool> parseBool(const uint8_t* json) const noexcept;
		const uint8_t* peekStart() const noexcept;
		uint32_t peekStartLength() const noexcept;

		void advanceScalar() noexcept;
		void advanceRootScalar() noexcept;
		void advanceNonRootScalar() noexcept;

		const uint8_t* peekScalar() noexcept;
		const uint8_t* peekRootScalar() noexcept;
		const uint8_t* peekNonRootScalar() noexcept;


		ErrorCode startContainer(uint8_t start_char) noexcept;
		ErrorCode endContainer() noexcept;
		JsonifierResult<const uint8_t*> advanceToValue() noexcept;

		ErrorCode incorrectTypeError() const noexcept;
		ErrorCode error_unless_more_tokens(uint32_t tokens = 1) const noexcept;

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
		ErrorCode reportError(ErrorCode error) noexcept;

		friend class Document;
		friend class Object;
		friend class Array;
		friend class Value;
	};

	template<> struct JsonifierResult<ValueIterator> : public JsonifierResultBase<ValueIterator> {
	  public:
		JsonifierResult(ValueIterator&& Value) noexcept;
		JsonifierResult(ErrorCode error) noexcept;
		JsonifierResult() noexcept = default;
	};

}
