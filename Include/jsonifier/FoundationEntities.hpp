#pragma once

#include "NumberParsingUtils.hpp"
#include "JsonifierResult.hpp"

namespace Jsonifier {

	class Parser;
	class JsonIterator;
	class Object;
	class Array;
	class Field;

	enum class JsonType : uint8_t {
		Document = 0,
		Object = 1,
		Array = 2,
		String = 3,
		Float = 4,
		Number = 5,
		Uint64 = 6,
		Int64 = 7,
		Bool = 8,
		Null = 9
	};

	class RawJsonString {
	  public:
		inline RawJsonString() noexcept = default;
		inline RawJsonString(const uint8_t* _buf) noexcept;
		inline const char* raw() const noexcept;
		inline bool unsafeIsEqual(size_t length, std::string_view target) const noexcept;
		inline bool unsafeIsEqual(std::string_view target) const noexcept;
		inline bool unsafeIsEqual(const char* target) const noexcept;
		inline bool isEqual(std::string_view target) const noexcept;
		inline bool isEqual(const char* target) const noexcept;
		static inline bool isFreeFromUnescapedQuote(std::string_view target) noexcept;
		static inline bool isFreeFromUnescapedQuote(const char* target) noexcept;

	  private:
		inline void consume() noexcept {
			this->stringView = nullptr;
		}
		inline bool alive() const noexcept {
			return this->stringView != nullptr;
		}
		inline JsonifierResult<std::string_view> unescape(JsonIterator& iter) const noexcept;

		const uint8_t* stringView{};
		friend class Object;
		friend class Field;
		friend struct JsonifierResult<RawJsonString>;
	};

	class TokenIterator {
	  public:
		inline TokenIterator(TokenIterator&& other) noexcept = default;
		inline TokenIterator& operator=(TokenIterator&& other) noexcept = default;
		inline TokenIterator(const TokenIterator& other) noexcept = default;
		inline TokenIterator& operator=(const TokenIterator& other) noexcept = default;

		inline const uint8_t* returnCurrentAndAdvance() noexcept;
		inline uint32_t currentOffset() const noexcept;
		inline const uint8_t* peek(int32_t delta = 0) const noexcept;
		inline uint32_t peekLength(int32_t delta = 0) const noexcept;
		inline const uint8_t* peek(uint32_t* position) const noexcept;
		inline uint32_t peekLength(uint32_t* position) const noexcept;
		inline uint32_t* position() const noexcept;
		inline void setPosition(uint32_t* target_position) noexcept;
		inline bool operator==(const TokenIterator& other) const noexcept;
		inline bool operator!=(const TokenIterator& other) const noexcept;
		inline bool operator>(const TokenIterator& other) const noexcept;
		inline bool operator>=(const TokenIterator& other) const noexcept;
		inline bool operator<(const TokenIterator& other) const noexcept;
		inline bool operator<=(const TokenIterator& other) const noexcept;

	  protected:
		inline TokenIterator(const uint8_t* stringView, uint32_t* position) noexcept;
		inline uint32_t peekIndex(int32_t delta = 0) const noexcept;
		inline uint32_t peekIndex(uint32_t* position) const noexcept;

		const uint8_t* stringView{};
		uint32_t* currentPosition{};

		friend class JsonIterator;
		friend class ValueIterator;
		friend class Object;
	};

	class JsonIterator {
	  protected:
		TokenIterator token;
		Parser* parser{};
		uint8_t* stringBuffer{};
		ErrorCode error{ ErrorCode::Success };
		size_t currentDepth{ 1 };
		uint32_t* rootStructural{};

	  public:
		inline JsonIterator(JsonIterator&& other) noexcept;
		inline JsonIterator& operator=(JsonIterator&& other) noexcept;
		inline ErrorCode skipChild(size_t parentDepth) noexcept;
		inline bool atRoot() const noexcept;
		inline uint32_t* rootPosition() const noexcept;
		inline bool isSingleToken() const noexcept;
		inline void assertAtDocumentDepth() const noexcept;
		inline void assertAtRoot() const noexcept;
		inline bool atEnd() const noexcept;
		inline bool isAlive() const noexcept;
		inline void abandon() noexcept;
		inline const uint8_t* returnCurrentAndAdvance() noexcept;
		inline const uint8_t* peek(int32_t delta = 0) const noexcept;
		inline uint32_t peekLength(int32_t delta = 0) const noexcept;
		inline const uint8_t* unsafePointer() const noexcept;
		inline const uint8_t* peek(uint32_t* position) const noexcept;
		inline uint32_t peekLength(uint32_t* position) const noexcept;
		inline const uint8_t* peekLast() const noexcept;
		inline void ascendTo(size_t parentDepth) noexcept;
		inline void descendTo(size_t childDepth) noexcept;
		inline void descendTo(size_t childDepth, int32_t delta) noexcept;
		inline size_t depth() const noexcept;
		inline uint8_t*& stringBufLoc() noexcept;
		inline ErrorCode reportError(ErrorCode error, const char* message) noexcept;
		inline ErrorCode optionalError(ErrorCode error, const char* message) noexcept;

		template<int N> inline bool copyToBuffer(const uint8_t* json, uint32_t max_len, uint8_t (&tmpbuf)[N]) noexcept;

		inline uint32_t* position() const noexcept;
		inline JsonifierResult<std::string_view> unescape(RawJsonString in) noexcept;
		inline void reenterChild(uint32_t* position, size_t childDepth) noexcept;
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
		friend class document_stream;
		friend class Object;
		friend class Orray;
		friend class Value;
		friend class RawJsonString;
		friend class Parser;
		friend class ValueIterator;
	};

	template<> struct JsonifierResult<RawJsonString> : public JsonifierResultBase<RawJsonString> {
	  public:
		JsonifierResult(RawJsonString&& value) noexcept;
		JsonifierResult(ErrorCode error) noexcept;
		JsonifierResult() noexcept = default;
		~JsonifierResult() noexcept = default;

		JsonifierResult<const char*> raw() const noexcept;
		JsonifierResult<std::string_view> unescape(JsonIterator& iter) const noexcept;
	};

}