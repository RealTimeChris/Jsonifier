#pragma once

#include <jsonifier/Simd.hpp>
#include <jsonifier/NumberParsingUtils.hpp>
#include <jsonifier/StringParsingUtils.hpp>
#include <jsonifier/ImplementationJsonifierResultBase.hpp>

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

	class Jsonifier_Dll RawJsonString {
	  public:
		RawJsonString() noexcept = default;
		RawJsonString(const uint8_t* stringViewNew) noexcept;
		const char* raw() const noexcept;
		bool unsafeIsEqual(size_t length, std::string_view target) const noexcept;
		bool unsafeIsEqual(std::string_view target) const noexcept;
		bool unsafeIsEqual(const char* target) const noexcept;
		bool isEqual(std::string_view target) const noexcept;
		bool isEqual(const char* target) const noexcept;

	  private:
		void consume() noexcept {
			this->stringView = nullptr;
		}

		bool alive() const noexcept {
			return this->stringView != nullptr;
		}

		JsonifierResult<std::string_view> unescape(JsonIterator& iteratorNew) const noexcept;

		const uint8_t* stringView{};
		friend class Object;
		friend class Field;
		friend struct JsonifierResult<RawJsonString>;
		friend struct JsonifierResult<std::string_view>;
		friend struct JsonIterator;
	};

	class Jsonifier_Dll TokenIterator {
	  public:
		TokenIterator(TokenIterator&& other) noexcept = default;
		TokenIterator& operator=(TokenIterator&& other) noexcept = default;
		TokenIterator(const TokenIterator& other) noexcept = default;
		TokenIterator& operator=(const TokenIterator& other) noexcept = default;

		const uint8_t* returnCurrentAndAdvance() noexcept;
		uint32_t currentOffset() const noexcept;
		const uint8_t* peek(int32_t delta = 0) const noexcept;
		uint32_t peekLength(int32_t delta = 0) const noexcept;
		const uint8_t* peek(uint32_t* position) const noexcept;
		uint32_t peekLength(uint32_t* position) const noexcept;
		uint32_t* position() const noexcept;
		void setPosition(uint32_t* target_position) noexcept;
		bool operator==(const TokenIterator& other) const noexcept;
		bool operator!=(const TokenIterator& other) const noexcept;
		bool operator>(const TokenIterator& other) const noexcept;
		bool operator>=(const TokenIterator& other) const noexcept;
		bool operator<(const TokenIterator& other) const noexcept;
		bool operator<=(const TokenIterator& other) const noexcept;

	  protected:
		TokenIterator(const uint8_t* stringView, uint32_t* position) noexcept;
		uint32_t peekIndex(int32_t delta = 0) const noexcept;
		uint32_t peekIndex(uint32_t* position) const noexcept;

		const uint8_t* stringView{};
		uint32_t* currentPosition{};

		friend class JsonIterator;
		friend class ValueIterator;
		friend class Object;
	};

	class Jsonifier_Dll JsonIterator {
	  protected:
		TokenIterator token;
		Parser* parser{};
		uint8_t* stringBuffer{};
		ErrorCode error{ ErrorCode::Success };
		size_t currentDepth{ 1 };
		uint32_t* rootStructural{};

	  public:
		JsonIterator(JsonIterator&& other) noexcept;
		JsonIterator& operator=(JsonIterator&& other) noexcept;
		ErrorCode skipChild(size_t parentDepth) noexcept;
		bool atRoot() const noexcept;
		uint32_t* rootPosition() const noexcept;
		bool isSingleToken() const noexcept;
		void assertAtDocumentDepth() const noexcept;
		void assertAtRoot() const noexcept;
		bool atEnd() const noexcept;
		bool isAlive() const noexcept;
		void abandon() noexcept;
		const uint8_t* returnCurrentAndAdvance() noexcept;
		const uint8_t* peek(int32_t delta = 0) const noexcept;
		uint32_t peekLength(int32_t delta = 0) const noexcept;
		const uint8_t* unsafePointer() const noexcept;
		const uint8_t* peek(uint32_t* position) const noexcept;
		uint32_t peekLength(uint32_t* position) const noexcept;
		inline uint32_t* startPosition(size_t depth) const noexcept;
		inline void setStartPosition(size_t depth, uint32_t* position) noexcept;
		const uint8_t* peekLast() const noexcept;
		void ascendTo(size_t parentDepth) noexcept;
		void descendTo(size_t childDepth) noexcept;
		void descendTo(size_t childDepth, int32_t delta) noexcept;
		size_t depth() const noexcept;
		uint8_t*& stringBufLoc() noexcept;
		ErrorCode reportError(ErrorCode error, const char* message) noexcept;
		ErrorCode optionalError(ErrorCode error, const char* message) noexcept;

		template<int N> bool copyToBuffer(const uint8_t* json, uint32_t maxLength, uint8_t (&tmpbuf)[N]) noexcept;

		uint32_t* position() const noexcept;
		JsonifierResult<std::string_view> unescape(RawJsonString in) noexcept;
		void reenterChild(uint32_t* position, size_t childDepth) noexcept;
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
		friend class document_stream;
		friend class Object;
		friend class Orray;
		friend class Value;
		friend class RawJsonString;
		friend class Parser;
		friend class ValueIterator;
	};

	template<> struct JsonifierResult<RawJsonString> : public ImplementationJsonifierResultBase<RawJsonString> {
	  public:
		inline JsonifierResult(RawJsonString&& value) noexcept;
		inline JsonifierResult(ErrorCode error) noexcept;
		inline JsonifierResult() noexcept = default;
		inline ~JsonifierResult() noexcept = default;

		inline JsonifierResult<const char*> raw() const noexcept;
		inline JsonifierResult<std::string_view> unescape(JsonIterator& iteratorNew) const noexcept;
	};

	template<> struct JsonifierResult<TokenIterator> : public ImplementationJsonifierResultBase<TokenIterator> {
	  public:
		inline JsonifierResult(TokenIterator&& value) noexcept;
		inline JsonifierResult(ErrorCode error) noexcept;
		inline JsonifierResult() noexcept = default;
		inline ~JsonifierResult() noexcept = default;
	};

	template<> struct JsonifierResult<JsonIterator> : public ImplementationJsonifierResultBase<JsonIterator> {
	  public:
		inline JsonifierResult(JsonIterator&& value) noexcept;
		inline JsonifierResult(ErrorCode error) noexcept;

		inline JsonifierResult() noexcept = default;
	};

	template<> struct JsonifierResult<JsonType> : public ImplementationJsonifierResultBase<JsonType> {
	  public:
		inline JsonifierResult(JsonType&& value) noexcept;
		inline JsonifierResult(ErrorCode error) noexcept;
		inline JsonifierResult() noexcept = default;
		inline ~JsonifierResult() noexcept = default;
	};

	template<> inline BackslashAndQuote<SimdBase256> BackslashAndQuote<SimdBase256>::copyAndFind(const uint8_t* src, uint8_t* dst) {
		static_assert(256 >= (BYTES_PROCESSED - 1), "backslash and quote finder must process fewer than SIMDJSON_PADDING bytes");
		SimdBase256 v(src);
		v.store(dst);
		return {
			static_cast<uint32_t>((v == '\\').toBitMask()),
			static_cast<uint32_t>((v == '"').toBitMask()),
		};
	}

}