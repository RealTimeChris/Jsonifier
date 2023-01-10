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

	class Jsonifier_Dll JsonIterator : public TokenIterator {
	  protected:
		Parser* parser{};
		uint8_t* stringBuffer{};
		ErrorCode error{ Success };
		uint32_t currentDepth{};
		uint32_t* rootStructural{};

	  public:
		JsonIterator() noexcept = default;
		JsonIterator(JsonIterator&& other) noexcept;
		JsonIterator& operator=(JsonIterator&& other) noexcept;
		explicit JsonIterator(const JsonIterator& other) noexcept = default;
		JsonIterator& operator=(const JsonIterator& other) noexcept = default;
		ErrorCode skipChild(uint32_t parentDepth) noexcept;
		bool atRoot() const noexcept;
		uint32_t* rootPosition() const noexcept;
		void assertAtDocumentDepth() const noexcept;
		void assertAtRoot() const noexcept;
		bool atEnd() const noexcept;
		bool isAlive() const noexcept;
		void abandon() noexcept;
		const uint8_t* returnCurrentAndAdvance() noexcept;
		bool isSingleToken() const noexcept;
		const uint8_t* peek(int32_t delta = 0) const noexcept;
		uint32_t peekLength(int32_t delta = 0) const noexcept;
		const uint8_t* unsafePointer() const noexcept;
		const uint8_t* peek(uint32_t* position) const noexcept;
		uint32_t peekLength(uint32_t* position) const noexcept;
		const uint8_t* peekLast() const noexcept;
		void ascendTo(uint32_t parentDepth) noexcept;
		void descendTo(uint32_t child_depth) noexcept;
		void descendTo(uint32_t child_depth, int32_t delta) noexcept;
		uint32_t depth() const noexcept;
		uint8_t*& stringBufLoc() noexcept;
		ErrorCode reportError(ErrorCode error) noexcept;
		template<int N> __forceinline bool copyToBuffer(const uint8_t* json, uint32_t max_len, uint8_t (&tmpbuf)[N]) noexcept {
			if ((N < max_len) || (N == 0)) {
				return false;
			}
			if (max_len > N - 1) {
				max_len = N - 1;
			}
			std::memcpy(tmpbuf, json, max_len);
			tmpbuf[max_len] = ' ';
			return true;
		}
		uint32_t* position() const noexcept;
		JsonifierResult<std::string_view> unescape(RawJsonString in) noexcept;
		void reenterChild(uint32_t* position, uint32_t child_depth) noexcept;
		std::string toString() const noexcept;
		JsonifierResult<const char*> currentLocation() noexcept;
		void rewind() noexcept;
		bool balanced() const noexcept;

	  protected:
		JsonIterator(Parser* parser) noexcept;
		uint32_t* lastPosition() const noexcept;
		uint32_t* endPosition() const noexcept;
		uint32_t* end() const noexcept;

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
		JsonifierResult(JsonIterator&& Value) noexcept;
		JsonifierResult(ErrorCode error) noexcept;
		JsonifierResult() noexcept = default;
	};

}
