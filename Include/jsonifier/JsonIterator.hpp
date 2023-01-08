#pragma once

#include <jsonifier/Base.hpp>
#include <jsonifier/TokenIterator.hpp>
#include <jsonifier/StringParsingUtils.hpp>
#include <jsonifier/NumberParsingUtils.hpp>

namespace Jsonifier {
	class Document;
	class Document_stream;
	class Object;
	class Array;
	class Value;
	class RawJsonString;
	class Parser;

	/**
 * Iterates through JSON tokens, keeping track of depth and string buffer.
 *
 * @private This is not intended for external use.
 */
	class Jsonifier_Dll JsonIterator {
	  protected:
		TokenIterator token{};
		Parser* parser{};
		/**
   * Next free location in the string buffer.
   *
   * Used by RawJsonString::unescape() to have a place to unescape strings to.
   */
		uint8_t* stringBufferLocation{};
		/**
   * JSON error, if there is one.
   *
   * Incorrect_Type and No_Such_Field are *not* stored here, ever.
   *
   * PERF NOTE: we *hope* this will be elided into control flow, as it is only used (a) in the first
   * iteration of the loop, or (b) for the final iteration after a missing comma is found in ++. If
   * this is not elided, we should make sure it's at least not using up a register. Failing that,
   * we should store it in Document so there's only one of them.
   */
		ErrorCode error{ Success };
		/**
   * Depth of the current token in the JSON.
   *
   * - 0 = finished with Document
   * - 1 = Document root Value (could be [ or {, not yet known)
   * - 2 = , or } inside root Array/Object
   * - 3 = key or Value inside root Array/Object.
   */
		uint32_t currentDepth{};
		/**
   * Beginning of the Document indexes.
   * Normally we have root == parser->getStructuralIndices().get()
   * but this may differ, especially in streaming mode (where we have several
   * Documents);
   */
		uint32_t* rootStructural{};
		/**
   * Normally, a JsonIterator operates over a single Document, but in
   * some cases, we may have a stream of Documents. This attribute is meant
   * as meta-data: the JsonIterator works the same irrespective of the
   * Value of this attribute.
   */
		bool _streaming{ false };

	  public:
		inline JsonIterator() noexcept = default;
		inline JsonIterator(JsonIterator&& other) noexcept;
		inline JsonIterator& operator=(JsonIterator&& other) noexcept;
		inline explicit JsonIterator(const JsonIterator& other) noexcept = default;
		inline JsonIterator& operator=(const JsonIterator& other) noexcept = default;
		/**
   * Skips a JSON Value, whether it is a scalar, Array or Object.
   */
		inline ErrorCode skip_child(uint32_t parent_depth) noexcept;

		/**
   * Tell whether the iterator is still at the start
   */
		inline bool at_root() const noexcept;

		/**
   * Tell whether we should be expected to run in streaming
   * mode (iterating over many Documents). It is pure metadata
   * that does not affect how the iterator works. It is used by
   * start_root_array() and start_root_object().
   */
		inline bool streaming() const noexcept;

		/**
   * Get the root Value iterator
   */
		inline uint32_t* root_position() const noexcept;
		/**
   * Assert that we are at the Document depth (== 1)
   */
		inline void assert_at_Document_depth() const noexcept;
		/**
   * Assert that we are at the root of the Document
   */
		inline void assert_at_root() const noexcept;

		/**
   * Tell whether the iterator is at the EOF mark
   */
		inline bool at_end() const noexcept;

		/**
   * Tell whether the iterator is live (has not been moved).
   */
		inline bool is_alive() const noexcept;

		/**
   * Abandon this iterator, setting depth to 0 (as if the Document is finished).
   */
		inline void abandon() noexcept;

		/**
   * Advance the current token without modifying depth.
   */
		inline const uint8_t* return_current_and_advance() noexcept;

		/**
   * Returns true if there is a single token in the index (i.e., it is
   * a JSON with a scalar Value such as a single number).
   *
   * @return whether there is a single token
   */
		inline bool is_single_token() const noexcept;

		/**
   * Assert that there are at least the given number of tokens left.
   *
   * Has no effect in release builds.
   */
		inline void assert_more_tokens(uint32_t required_tokens = 1) const noexcept;
		/**
   * Assert that the given position addresses an actual token (is within bounds).
   *
   * Has no effect in release builds.
   */
		inline void assert_valid_position(uint32_t* position) const noexcept;
		/**
   * Get the JSON text for a given token (relative).
   *
   * This is not null-terminated; it is a view into the JSON.
   *
   * @param delta The relative position of the token to retrieve. e.g. 0 = next token, -1 = prev token.
   *
   * TODO consider a string_view, assuming the length will get stripped out by the optimizer when
   * it isn't used ...
   */
		inline const uint8_t* peek(int32_t delta = 0) const noexcept;
		/**
   * Get the maximum length of the JSON text for the current token (or relative).
   *
   * The length will include any whitespace at the end of the token.
   *
   * @param delta The relative position of the token to retrieve. e.g. 0 = next token, -1 = prev token.
   */
		inline uint32_t peek_length(int32_t delta = 0) const noexcept;
		/**
   * Get a pointer to the current location in the input buffer.
   *
   * This is not null-terminated; it is a view into the JSON.
   *
   * You may be pointing outside of the input buffer: it is not generally
   * safe to dereference this pointer.
   */
		inline const uint8_t* unsafe_pointer() const noexcept;
		/**
   * Get the JSON text for a given token.
   *
   * This is not null-terminated; it is a view into the JSON.
   *
   * @param position The position of the token to retrieve.
   *
   * TODO consider a string_view, assuming the length will get stripped out by the optimizer when
   * it isn't used ...
   */
		inline const uint8_t* peek(uint32_t* position) const noexcept;
		/**
   * Get the maximum length of the JSON text for the current token (or relative).
   *
   * The length will include any whitespace at the end of the token.
   *
   * @param position The position of the token to retrieve.
   */
		inline uint32_t peek_length(uint32_t* position) const noexcept;
		/**
   * Get the JSON text for the last token in the Document.
   *
   * This is not null-terminated; it is a view into the JSON.
   *
   * TODO consider a string_view, assuming the length will get stripped out by the optimizer when
   * it isn't used ...
   */
		inline const uint8_t* peek_last() const noexcept;

		/**
   * Ascend one level.
   *
   * Validates that the depth - 1 == parent_depth.
   *
   * @param parent_depth the expected parent depth.
   */
		inline void ascend_to(uint32_t parent_depth) noexcept;

		/**
   * Descend one level.
   *
   * Validates that the new depth == child_depth.
   *
   * @param child_depth the expected child depth.
   */
		inline void descend_to(uint32_t child_depth) noexcept;
		inline void descend_to(uint32_t child_depth, int32_t delta) noexcept;

		/**
   * Get current depth.
   */
		inline uint32_t depth() const noexcept;

		/**
   * Get current (writeable) location in the string buffer.
   */
		inline uint8_t*& string_buf_loc() noexcept;

		/**
   * Report an unrecoverable error, preventing further iteration.
   *
   * @param error The error to report. Must not be Success, Uninitialized, Incorrect_Type, or No_Such_Field.
   * @param message An error message to report with the error.
   */
		inline ErrorCode report_error(ErrorCode error, const char* message) noexcept;

		/**
   * Log error, but don't stop iteration.
   * @param error The error to report. Must be Incorrect_Type, or No_Such_Field.
   * @param message An error message to report with the error.
   */
		inline ErrorCode optional_error(ErrorCode error, const char* message) noexcept;

		template<int N> inline bool copy_to_buffer(const uint8_t* json, uint32_t max_len, uint8_t (&tmpbuf)[N]) noexcept;

		inline uint32_t* position() const noexcept;
		/**
   * Write the RawJsonString to the string buffer and return a string_view.
   * Each RawJsonString should be unescaped once, or else the string buffer might
   * overflow.
   */
		inline JsonifierResult<std::string_view> unescape(RawJsonString in) noexcept;
		inline void reenter_child(uint32_t* position, uint32_t child_depth) noexcept;



		/* Useful for debugging and logging purposes. */
		inline std::string to_string() const noexcept;

		/**
   * Returns the current location in the Document if in bounds.
   */
		inline JsonifierResult<const char*> currentLocation() noexcept;

		/**
   * Updates this json iterator so that it is back at the beginning of the Document,
   * as if it had just been created.
   */
		inline void rewind() noexcept;
		/**
   * This checks whether the {,},[,] are balanced so that the Document
   * ends with proper zero depth. This requires scanning the whole Document
   * and it may be expensive. It is expected that it will be rarely called.
   * It does not attempt to match { with } and [ with ].
   */
		inline bool balanced() const noexcept;

	  protected:
		inline JsonIterator(Parser* parser) noexcept;
		/// The last token before the end
		inline uint32_t* last_position() const noexcept;
		/// The token *at* the end. This points at gibberish and should only be used for comparison.
		inline uint32_t* end_position() const noexcept;
		/// The end of the buffer.
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
