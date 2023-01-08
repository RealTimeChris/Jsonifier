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
		TokenIterator token;
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
		inline ErrorCode skip_child(uint32_t parent_depth) noexcept;
		inline bool at_root() const noexcept;
		inline uint32_t* root_position() const noexcept;
		inline void assert_at_Document_depth() const noexcept;
		inline void assert_at_root() const noexcept;
		inline bool at_end() const noexcept;
		inline bool isAlive() const noexcept;
		inline void abandon() noexcept;
		inline const uint8_t* returnCurrentAndAdvance() noexcept;
		inline bool is_single_token() const noexcept;
		inline void assert_more_tokens(uint32_t required_tokens = 1) const noexcept;
		inline void assert_valid_position(uint32_t* position) const noexcept;
		inline const uint8_t* peek(int32_t delta = 0) const noexcept;
		inline uint32_t peekLength(int32_t delta = 0) const noexcept;
		inline const uint8_t* unsafe_pointer() const noexcept;
		inline const uint8_t* peek(uint32_t* position) const noexcept;
		inline uint32_t peekLength(uint32_t* position) const noexcept;
		inline const uint8_t* peek_last() const noexcept;
		inline void ascend_to(uint32_t parent_depth) noexcept;
		inline void descend_to(uint32_t child_depth) noexcept;
		inline void descend_to(uint32_t child_depth, int32_t delta) noexcept;
		inline uint32_t depth() const noexcept;
		inline uint8_t*& string_buf_loc() noexcept;
		inline ErrorCode reportError(ErrorCode error, const char* message) noexcept;
		inline ErrorCode optional_error(ErrorCode error, const char* message) noexcept;
		template<int N> inline bool copy_to_buffer(const uint8_t* json, uint32_t max_len, uint8_t (&tmpbuf)[N]) noexcept;
		inline uint32_t* position() const noexcept;
		inline JsonifierResult<std::string_view> unescape(RawJsonString in) noexcept;
		inline void reenter_child(uint32_t* position, uint32_t child_depth) noexcept;
		inline std::string to_string() const noexcept;
		inline JsonifierResult<const char*> currentLocation() noexcept;
		inline void rewind() noexcept;
		inline bool balanced() const noexcept;

	  protected:
		inline JsonIterator(Parser* parser) noexcept;
		inline uint32_t* last_position() const noexcept;
		inline uint32_t* end_position() const noexcept;
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
		inline JsonifierResult(JsonIterator&& Value) noexcept;///< @private
		inline JsonifierResult(ErrorCode error) noexcept;///< @private

		inline JsonifierResult() noexcept = default;
	};

}
