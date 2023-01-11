#pragma once

#include <jsonifier/Base.hpp>
#include <jsonifier/TokenIterator.hpp>
#include <jsonifier/StringParsingUtils.hpp>
#include <jsonifier/NumberParsingUtils.hpp>

namespace Jsonifier {

	class RawJsonString;
	class Document;
	class Object;
	class Parser;
	class Array;
	class Value;

	class Jsonifier_Dll JsonIterator {
	  protected:
		TokenIterator iterator{};
		Parser* parser{};
		uint8_t* stringBuffer{};
		ErrorCode error{ Success };
		uint32_t currentDepth{};
		uint32_t* rootStructural{};

	  public:
		__forceinline JsonIterator() noexcept = default;
		__forceinline JsonIterator(JsonIterator&& other) noexcept;
		__forceinline JsonIterator& operator=(JsonIterator&& other) noexcept;
		__forceinline explicit JsonIterator(const JsonIterator& other) noexcept = default;
		__forceinline JsonIterator& operator=(const JsonIterator& other) noexcept = default;
		__forceinline ErrorCode skipChild(uint32_t parentDepth) noexcept;
		__forceinline bool atRoot() const noexcept;
		__forceinline uint32_t* rootPosition() const noexcept;
		__forceinline void assertAtDocumentDepth() const noexcept;
		__forceinline void assertAtRoot() const noexcept;
		__forceinline bool atEnd() const noexcept;
		__forceinline bool isAlive() const noexcept;
		__forceinline void abandon() noexcept;
		__forceinline const uint8_t* returnCurrentAndAdvance() noexcept;
		__forceinline bool isSingleToken() const noexcept;
		__forceinline const uint8_t* peek(int32_t delta = 0) const noexcept;
		__forceinline uint32_t peekLength(int32_t delta = 0) const noexcept;
		__forceinline const uint8_t* unsafePointer() const noexcept;
		__forceinline const uint8_t* peek(uint32_t* position) const noexcept;
		__forceinline uint32_t peekLength(uint32_t* position) const noexcept;
		__forceinline const uint8_t* peekLast() const noexcept;
		__forceinline void ascendTo(uint32_t parentDepth) noexcept;
		__forceinline void descendTo(uint32_t child_depth) noexcept;
		__forceinline void descendTo(uint32_t child_depth, int32_t delta) noexcept;
		__forceinline uint32_t depth() const noexcept;
		__forceinline uint8_t*& stringBufLoc() noexcept;
		__forceinline ErrorCode reportError(ErrorCode error) noexcept;
		template<int N> __forceinline bool copyToBuffer(const uint8_t* json, uint32_t maxLength, uint8_t (&tmpbuf)[N]) noexcept {
			if ((N < maxLength) || (N == 0)) {
				return false;
			}
			if (maxLength > N - 1) {
				maxLength = N - 1;
			}
			std::copy(json, json + maxLength, tmpbuf);
			tmpbuf[maxLength] = ' ';
			return true;
		}

		__forceinline uint32_t* position() const noexcept;
		__forceinline JsonifierResult<std::string_view> unescape(RawJsonString in) noexcept;
		__forceinline void reenterChild(uint32_t* position, uint32_t child_depth) noexcept;
		__forceinline std::string toString() const noexcept;
		__forceinline JsonifierResult<const char*> currentLocation() noexcept;
		__forceinline void rewind() noexcept;
		__forceinline bool balanced() const noexcept;

	  protected:
		__forceinline JsonIterator(Parser* parser) noexcept;
		__forceinline uint32_t* lastPosition() const noexcept;
		__forceinline uint32_t* endPosition() const noexcept;
		__forceinline uint32_t* end() const noexcept;

		friend class Document;
		friend class Object;
		friend class Array;
		friend class Value;
		friend class RawJsonString;
		friend class Parser;
		friend class ValueIterator;
	};

	template<> struct JsonifierResult<JsonIterator> : public JsonifierResultBase<JsonIterator> {
	  public:
		__forceinline JsonifierResult(JsonIterator&& Value) noexcept;
		__forceinline JsonifierResult(ErrorCode error) noexcept;
		__forceinline JsonifierResult() noexcept = default;
	};

}
