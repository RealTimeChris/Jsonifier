#pragma once

#include <jsonifier/JsonIterator.hpp>

namespace Jsonifier {
	class Document;
	class Object;
	class Array;
	class Value;
	class RawJsonString;
	class parser;

	/**
 * Iterates through a single JSON Value at a particular depth.
 *
 * Does not keep track of the type of Value: provides methods for objects, arrays and scalars and expects
 * the caller to call the right ones.
 *
 * @private This is not intended for external use.
 */
	class Jsonifier_Dll ValueIterator {
	  protected:
		/** The underlying JSON iterator */
		JsonIterator* jsonIterator{};
		/** The depth of this Value */
		uint32_t currentDepth{};
		/**
   * The starting token index for this Value
   */
		uint32_t* rootStructural{};

	  public:
		inline ValueIterator() noexcept = default;

		/**
   * Denote that we're starting a Document.
   */
		inline void start_Document() noexcept;

		/**
   * Skips a non-iterated or partially-iterated JSON Value, whether it is a scalar, Array or Object.
   *
   * Optimized for scalars.
   */
		inline ErrorCode skipChild() noexcept;

		/**
   * Tell whether the iterator is at the EOF mark
   */
		inline bool atEnd() const noexcept;

		/**
   * Tell whether the iterator is at the start of the Value
   */
		inline bool at_start() const noexcept;

		/**
   * Tell whether the Value is open--if the Value has not been used, or the Array/Object is still open.
   */
		inline bool is_open() const noexcept;

		/**
   * Tell whether the Value is at an Object's first Field (just after the {).
   */
		inline bool at_first_field() const noexcept;

		/**
   * Abandon all iteration.
   */
		inline void abandon() noexcept;

		/**
   * Get the child Value as a ValueIterator.
   */
		inline ValueIterator child_value() const noexcept;

		/**
   * Get the depth of this Value.
   */
		inline int32_t depth() const noexcept;

		/**
   * Get the JSON type of this Value.
   *
   * @error Tape_Error when the JSON Value is a bad token like "}" "," or "alse".
   */
		inline JsonifierResult<JsonType> type() const noexcept;

		/**
   * @addtogroup Object Object iteration
   *
   * Methods to iterate and find Object fields. These methods generally *assume* the Value is
   * actually an Object; the caller is responsible for keeping track of that fact.
   *
   * @{
   */

		/**
   * Start an Object iteration.
   *
   * @returns Whether the Object had any fields (returns false for empty).
   * @error Incorrect_Type if there is no opening {
   */
		inline JsonifierResult<bool> start_object() noexcept;
		/**
   * Start an Object iteration from the root.
   *
   * @returns Whether the Object had any fields (returns false for empty).
   * @error Incorrect_Type if there is no opening {
   * @error Tape_Error if there is no matching } at end of Document
   */
		inline JsonifierResult<bool> start_root_object() noexcept;

		/**
   * Start an Object iteration after the user has already checked and moved past the {.
   *
   * Does not move the iterator unless the Object is empty ({}).
   *
   * @returns Whether the Object had any fields (returns false for empty).
   * @error Incomplete_Array_Or_Object If there are no more tokens (implying the *parent*
   *        Array or Object is incomplete).
   */
		inline JsonifierResult<bool> started_object() noexcept;
		/**
   * Start an Object iteration from the root, after the user has already checked and moved past the {.
   *
   * Does not move the iterator unless the Object is empty ({}).
   *
   * @returns Whether the Object had any fields (returns false for empty).
   * @error Incomplete_Array_Or_Object If there are no more tokens (implying the *parent*
   *        Array or Object is incomplete).
   */
		inline JsonifierResult<bool> started_root_object() noexcept;

		/**
   * Moves to the next Field in an Object.
   *
   * Looks for , and }. If } is found, the Object is finished and the iterator advances past it.
   * Otherwise, it advances to the next Value.
   *
   * @return whether there is another Field in the Object.
   * @error Tape_Error If there is a comma missing between fields.
   * @error Tape_Error If there is a comma, but not enough tokens remaining to have a key, :, and Value.
   */
		inline JsonifierResult<bool> has_next_field() noexcept;

		/**
   * Get the current Field's key.
   */
		inline JsonifierResult<RawJsonString> field_key() noexcept;

		/**
   * Pass the : in the Field and move to its Value.
   */
		inline ErrorCode field_value() noexcept;

		/**
   * Find the next Field with the given key.
   *
   * Assumes you have called next_field() or otherwise matched the previous Value.
   *
   * This means the iterator must be sitting at the next key:
   *
   * ```
   * { "a": 1, "b": 2 }
   *           ^
   * ```
   *
   * Key is *raw JSON,* meaning it will be matched against the verbatim JSON without attempting to
   * unescape it. This works well for typical ASCII and UTF-8 keys (almost all of them), but may
   * fail to match some keys with escapes (\u, \n, etc.).
   */
		inline ErrorCode findField(const std::string_view key) noexcept;

		/**
   * Find the next Field with the given key, *without* unescaping. This assumes Object order: it
   * will not find the Field if it was already passed when looking for some *other* Field.
   *
   * Assumes you have called next_field() or otherwise matched the previous Value.
   *
   * This means the iterator must be sitting at the next key:
   *
   * ```
   * { "a": 1, "b": 2 }
   *           ^
   * ```
   *
   * Key is *raw JSON,* meaning it will be matched against the verbatim JSON without attempting to
   * unescape it. This works well for typical ASCII and UTF-8 keys (almost all of them), but may
   * fail to match some keys with escapes (\u, \n, etc.).
   */
		inline JsonifierResult<bool> findFieldRaw(const std::string_view key) noexcept;

		/**
   * Find the Field with the given key without regard to order, and *without* unescaping.
   *
   * This is an unordered Object lookup: if the Field is not found initially, it will cycle around and scan from the beginning.
   *
   * Assumes you have called next_field() or otherwise matched the previous Value.
   *
   * This means the iterator must be sitting at the next key:
   *
   * ```
   * { "a": 1, "b": 2 }
   *           ^
   * ```
   *
   * Key is *raw JSON,* meaning it will be matched against the verbatim JSON without attempting to
   * unescape it. This works well for typical ASCII and UTF-8 keys (almost all of them), but may
   * fail to match some keys with escapes (\u, \n, etc.).
   */
		inline JsonifierResult<bool> find_field_unordered_raw(const std::string_view key) noexcept;

		/** @} */

		/**
   * @addtogroup Array Array iteration
   * Methods to iterate over Array elements. These methods generally *assume* the Value is actually
   * an Object; the caller is responsible for keeping track of that fact.
   * @{
   */

		/**
   * Check for an opening [ and start an Array iteration.
   *
   * @returns Whether the Array had any elements (returns false for empty).
   * @error Incorrect_Type If there is no [.
   */
		inline JsonifierResult<bool> startArray() noexcept;
		/**
   * Check for an opening [ and start an Array iteration while at the root.
   *
   * @returns Whether the Array had any elements (returns false for empty).
   * @error Incorrect_Type If there is no [.
   * @error Tape_Error if there is no matching ] at end of Document
   */
		inline JsonifierResult<bool> startRootArray() noexcept;

		/**
   * Start an Array iteration, after the user has already checked and moved past the [.
   *
   * Does not move the iterator unless the Array is empty ([]).
   *
   * @returns Whether the Array had any elements (returns false for empty).
   * @error Incomplete_Array_Or_Object If there are no more tokens (implying the *parent*
   *        Array or Object is incomplete).
   */
		inline JsonifierResult<bool> startedArray() noexcept;
		/**
   * Start an Array iteration from the root, after the user has already checked and moved past the [.
   *
   * Does not move the iterator unless the Array is empty ([]).
   *
   * @returns Whether the Array had any elements (returns false for empty).
   * @error Incomplete_Array_Or_Object If there are no more tokens (implying the *parent*
   *        Array or Object is incomplete).
   */
		inline JsonifierResult<bool> startedRootArray() noexcept;

		/**
   * Moves to the next element in an Array.
   *
   * Looks for , and ]. If ] is found, the Array is finished and the iterator advances past it.
   * Otherwise, it advances to the next Value.
   *
   * @return Whether there is another element in the Array.
   * @error Tape_Error If there is a comma missing between elements.
   */
		inline JsonifierResult<bool> has_next_element() noexcept;

		/**
   * Get a child Value iterator.
   */
		inline ValueIterator child() const noexcept;

		/** @} */

		/**
   * @defgroup scalar Scalar values
   * @addtogroup scalar
   * @{
   */

		inline JsonifierResult<std::string_view> getString() noexcept;
		inline JsonifierResult<RawJsonString> getRawJsonString() noexcept;
		inline JsonifierResult<uint64_t> getUint64() noexcept;
		inline JsonifierResult<int64_t> getInt64() noexcept;
		inline JsonifierResult<double> getDouble() noexcept;
		inline JsonifierResult<bool> getBool() noexcept;
		inline JsonifierResult<bool> isNull() noexcept;
		inline JsonifierResult<bool> is_integer() noexcept;

		inline JsonifierResult<std::string_view> get_root_string() noexcept;
		inline JsonifierResult<RawJsonString> get_root_raw_json_string() noexcept;
		inline JsonifierResult<uint64_t> get_root_uint64() noexcept;
		inline JsonifierResult<int64_t> get_root_int64() noexcept;
		inline JsonifierResult<double> get_root_double() noexcept;
		inline JsonifierResult<bool> get_root_bool() noexcept;
		inline JsonifierResult<bool> is_root_integer() noexcept;
		inline bool is_root_null() noexcept;

		inline ErrorCode error() const noexcept;
		inline uint8_t*& stringBufLoc() noexcept;
		inline const JsonIterator& jsonIter() const noexcept;
		inline JsonIterator& jsonIter() noexcept;

		inline void assertIsValid() const noexcept;
		inline bool isValid() const noexcept;

		/** @} */
	  protected:
		/**
   * Restarts an Array iteration.
   * @returns Whether the Array has any elements (returns false for empty).
   */
		inline JsonifierResult<bool> resetArray() noexcept;
		/**
   * Restarts an Object iteration.
   * @returns Whether the Object has any fields (returns false for empty).
   */
		inline JsonifierResult<bool> reset_object() noexcept;
		/**
   * move_at_start(): moves us so that we are pointing at the beginning of
   * the container. It updates the index so that at_start() is true and it
   * syncs the depth. The user can then create a new container instance.
   *
   * Usage: used with Value::countElements().
   **/
		inline void move_at_start() noexcept;

		/**
   * move_at_container_start(): moves us so that we are pointing at the beginning of
   * the container so that assert_at_container_start() passes.
   *
   * Usage: used with resetArray() and reset_object().
   **/
		inline void move_at_container_start() noexcept;
		/* Useful for debugging and logging purposes. */
		inline std::string toString() const noexcept;
		inline ValueIterator(JsonIterator* jsonIter, uint32_t depth, uint32_t* start_index) noexcept;

		inline JsonifierResult<bool> parse_null(const uint8_t* json) const noexcept;
		inline JsonifierResult<bool> parse_bool(const uint8_t* json) const noexcept;
		inline const uint8_t* peekStart() const noexcept;
		inline uint32_t peek_start_length() const noexcept;

		/**
   * The general idea of the advance_... methods and the peek_* methods
   * is that you first peek and check that you have desired type. If you do,
   * and only if you do, then you advance.
   *
   * We used to unconditionally advance. But this made reasoning about our
   * current state difficult.
   * Suppose you always advance. Look at the 'Value' matching the key
   * "shadowable" in the following example...
   *
   * ({"globals":{"a":{"shadowable":[}}}})
   *
   * If the user thinks it is a Boolean and asks for it, then we check the '[',
   * decide it is not a Boolean, but still move into the next character ('}'). Now
   * we are left pointing at '}' right after a '['. And we have not yet reported
   * an error, only that we do not have a Boolean.
   *
   * If, instead, you just stand your ground until it is content that you know, then
   * you will only even move beyond the '[' if the user tells you that you have an
   * Array. So you will be at the '}' character inside the Array and, hopefully, you
   * will then catch the error because an Array cannot start with '}', but the code
   * processing Boolean values does not know this.
   *
   * So the contract is: first call 'peek_...' and then call 'advance_...' only
   * if you have determined that it is a type you can handle.
   *
   * Unfortunately, it makes the code more verbose, longer and maybe more error prone.
   */

		inline void advance_scalar(const char* type) noexcept;
		inline void advance_root_scalar(const char* type) noexcept;
		inline void advance_non_root_scalar(const char* type) noexcept;

		inline const uint8_t* peek_scalar(const char* type) noexcept;
		inline const uint8_t* peek_root_scalar(const char* type) noexcept;
		inline const uint8_t* peek_non_root_scalar(const char* type) noexcept;


		inline ErrorCode start_container(uint8_t start_char, const char* incorrect_type_message, const char* type) noexcept;
		inline ErrorCode end_container() noexcept;

		/**
   * Advance to a place expecting a Value (increasing depth).
   *
   * @return The current token (the one left behind).
   * @error Tape_Error If the Document ended early.
   */
		inline JsonifierResult<const uint8_t*> advance_to_value() noexcept;

		inline ErrorCode incorrect_type_error(const char* message) const noexcept;
		inline ErrorCode error_unless_more_tokens(uint32_t tokens = 1) const noexcept;

		inline bool is_at_start() const noexcept;
		/**
   * is_at_iterator_start() returns true on an Array or Object after it has just been
   * created, whether the instance is empty or not.
   *
   * Usage: used by Array::begin() in debug mode (SIMDJSON_DEVELOPMENT_CHECKS)
   */
		inline bool is_at_iterator_start() const noexcept;

		/**
   * Assuming that we are within an Object, this returns true if we
   * are pointing at a key.
   *
   * Usage: the skipChild() method should never be used while we are pointing
   * at a key inside an Object.
   */
		inline bool is_at_key() const noexcept;

		inline void assert_at_start() const noexcept;
		inline void assert_at_container_start() const noexcept;
		inline void assertAtRoot() const noexcept;
		inline void assert_at_child() const noexcept;
		inline void assert_at_next() const noexcept;
		inline void assert_at_non_root_start() const noexcept;

		/** Get the starting position of this Value */
		inline uint32_t* start_position() const noexcept;

		/** @copydoc ErrorCode JsonIterator::position() const noexcept; */
		inline uint32_t* position() const noexcept;
		/** @copydoc ErrorCode JsonIterator::endPosition() const noexcept; */
		inline uint32_t* lastPosition() const noexcept;
		/** @copydoc ErrorCode JsonIterator::endPosition() const noexcept; */
		inline uint32_t* endPosition() const noexcept;
		/** @copydoc ErrorCode JsonIterator::reportError(ErrorCode error, const char *message) noexcept; */
		inline ErrorCode reportError(ErrorCode error, const char* message) noexcept;

		friend class Document;
		friend class Object;
		friend class Array;
		friend class Value;
	};// ValueIterator
	template<> struct JsonifierResult<ValueIterator> : public ImplementationJsonifierResultBase<ValueIterator> {
	  public:
		inline JsonifierResult(ValueIterator&& Value) noexcept;///< @private
		inline JsonifierResult(ErrorCode error) noexcept;///< @private
		inline JsonifierResult() noexcept = default;
	};

}
