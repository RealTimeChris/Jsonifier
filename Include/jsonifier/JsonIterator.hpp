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
		inline JsonIterator() noexcept = default;
		inline JsonIterator(JsonIterator&& other) noexcept;
		inline JsonIterator& operator=(JsonIterator&& other) noexcept;
		inline explicit JsonIterator(const JsonIterator& other) noexcept = default;
		inline JsonIterator& operator=(const JsonIterator& other) noexcept = default;
		inline ErrorCode skipChild(uint32_t parentDepth) noexcept;
		inline bool atRoot() const noexcept;
		inline uint32_t* rootPosition() const noexcept;
		inline void assertAtDocumentDepth() const noexcept;
		inline void assertAtRoot() const noexcept;
		inline bool atEnd() const noexcept;
		inline bool isAlive() const noexcept;
		inline void abandon() noexcept;
		inline const uint8_t* returnCurrentAndAdvance() noexcept;
		inline bool isSingleToken() const noexcept;
		inline const uint8_t* peek(int32_t delta = 0) const noexcept;
		inline uint32_t peekLength(int32_t delta = 0) const noexcept;
		inline const uint8_t* unsafePointer() const noexcept;
		inline const uint8_t* peek(uint32_t* position) const noexcept;
		inline uint32_t peekLength(uint32_t* position) const noexcept;
		inline const uint8_t* peekLast() const noexcept;
		inline void ascendTo(uint32_t parentDepth) noexcept;
		inline void descendTo(uint32_t child_depth) noexcept;
		inline void descendTo(uint32_t child_depth, int32_t delta) noexcept;
		inline uint32_t depth() const noexcept;
		inline uint8_t*& stringBufLoc() noexcept;
		inline ErrorCode reportError(ErrorCode error, const char* message) noexcept;
		inline ErrorCode optionalError(ErrorCode error, const char* message) noexcept;
		template<int N> inline bool copyToBuffer(const uint8_t* json, uint32_t max_len, uint8_t (&tmpbuf)[N]) noexcept;
		inline uint32_t* position() const noexcept;
		inline JsonifierResult<std::string_view> unescape(RawJsonString in) noexcept;
		inline void reenterChild(uint32_t* position, uint32_t child_depth) noexcept;
		inline std::string toString() const noexcept;
		inline JsonifierResult<const char*> currentLocation() noexcept;
		inline void rewind() noexcept;
		inline bool balanced() const noexcept;

	  protected:
		inline JsonIterator(Parser* parser) noexcept;
		inline uint32_t* lastPosition() const noexcept;
		inline uint32_t* endPosition() const noexcept;
		inline uint32_t* end() const noexcept;

		friend class Document;
		friend class Object;
		friend class Array;
		friend class Value;
		friend class RawJsonString;
		friend class Parser;
		friend class ValueIterator;
	};

	template<> struct JsonifierResult<JsonIterator> : public ImplementationJsonifierResultBase<JsonIterator> {
	  public:
		inline JsonifierResult(JsonIterator&& Value) noexcept;
		inline JsonifierResult(ErrorCode error) noexcept;
		inline JsonifierResult() noexcept = default;
	};

}
