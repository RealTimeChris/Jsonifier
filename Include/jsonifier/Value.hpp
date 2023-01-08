#pragma once

#include <jsonifier/Base.hpp>
#include <jsonifier/ValueIterator.hpp>

namespace Jsonifier {
	class ArrayIterator;
	class Value;
	/**
 * An ephemeral JSON Value returned during iteration. It is only valid for as long as you do
 * not access more data in the JSON document.
 */
	class Jsonifier_Dll Value {
	  public:
		/**
   * Create a new invalid Value.
   *
   * Exists so you can declare a variable and later assign to it before use.
   */
		inline Value() noexcept = default;

		/**
   * Get this Value as the given type.
   *
   * Supported types: Object, array, RawJsonString, string_view, uint64_t, int64_t, double, bool
   *
   * You may use getDouble(), getBool(), getUint64(), getInt64(),
   * getObject(), getArray(), getRawJsonString(), or getString() instead.
   *
   * @returns A Value of the given type, parsed from the JSON.
   * @returns Incorrect_Type If the JSON Value is not the given type.
   */
		template<typename T> inline JsonifierResult<T> get() noexcept {
			// Unless the simdjson library provides an inline implementation, calling this method should
			// immediately fail.
			static_assert(!sizeof(T), "The get method with given type is not implemented by the simdjson library.");
		}

		/**
   * Get this Value as the given type.
   *
   * Supported types: Object, array, RawJsonString, string_view, uint64_t, int64_t, double, bool
   *
   * @param out This is set to a Value of the given type, parsed from the JSON. If there is an error, this may not be initialized.
   * @returns Incorrect_Type If the JSON Value is not an Object.
   * @returns Success If the parse succeeded and the out parameter was set to the Value.
   */
		template<typename T> inline ErrorCode get(T& out) noexcept;

		/**
   * Cast this JSON Value to an array.
   *
   * @returns An Object that can be used to iterate the array.
   * @returns Incorrect_Type If the JSON Value is not an array.
   */
		inline JsonifierResult<Array> getArray() noexcept;

		/**
   * Cast this JSON Value to an Object.
   *
   * @returns An Object that can be used to look up or iterate fields.
   * @returns Incorrect_Type If the JSON Value is not an Object.
   */
		inline JsonifierResult<Object> getObject() noexcept;

		/**
   * Cast this JSON Value to an unsigned integer.
   *
   * @returns A unsigned 64-bit integer.
   * @returns Incorrect_Type If the JSON Value is not a 64-bit unsigned integer.
   */
		inline JsonifierResult<uint64_t> getUint64() noexcept;
		inline JsonifierResult<int64_t> getInt64() noexcept;
		/**
   * Cast this JSON Value to a double.
   *
   * @returns A double.
   * @returns Incorrect_Type If the JSON Value is not a valid floating-point number.
   */
		inline JsonifierResult<double> getDouble() noexcept;
		/**
   * Cast this JSON Value to a string.
   *
   * The string is guaranteed to be valid UTF-8.
   *
   * Equivalent to get<std::string_view>().
   *
   * Important: a Value should be consumed once. Calling getString() twice on the same Value
   * is an error.
   *
   * @returns An UTF-8 string. The string is stored in the parser and will be invalidated the next
   *          time it parses a document or when it is destroyed.
   * @returns Incorrect_Type if the JSON Value is not a string.
   */
		inline JsonifierResult<std::string_view> getString() noexcept;

		/**
   * Cast this JSON Value to a RawJsonString.
   *
   * The string is guaranteed to be valid UTF-8, and may have escapes in it (e.g. \\ or \n).
   *
   * @returns A pointer to the raw JSON for the given string.
   * @returns Incorrect_Type if the JSON Value is not a string.
   */
		inline JsonifierResult<RawJsonString> getRawJsonString() noexcept;

		/**
   * Cast this JSON Value to a bool.
   *
   * @returns A bool Value.
   * @returns Incorrect_Type if the JSON Value is not true or false.
   */
		inline JsonifierResult<bool> getBool() noexcept;

		/**
   * Checks if this JSON Value is null. If and only if the Value is
   * null, then it is consumed (we advance). If we find a token that
   * begins with 'n' but is not 'null', then an error is returned.
   *
   * @returns Whether the Value is null.
   * @returns Incorrect_Type If the JSON Value begins with 'n' and is not 'null'.
   */
		inline JsonifierResult<bool> isNull() noexcept;

		/**
   * Cast this JSON Value to an array.
   *
   * @returns An Object that can be used to iterate the array.
   * @exception simdjson_error(Incorrect_Type) If the JSON Value is not an array.
   */
		inline operator Array() noexcept(false);
		/**
   * Cast this JSON Value to an Object.
   *
   * @returns An Object that can be used to look up or iterate fields.
   * @exception simdjson_error(Incorrect_Type) If the JSON Value is not an Object.
   */
		inline operator Object() noexcept(false);
		/**
   * Cast this JSON Value to an unsigned integer.
   *
   * @returns A signed 64-bit integer.
   * @exception simdjson_error(Incorrect_Type) If the JSON Value is not a 64-bit unsigned integer.
   */
		inline operator uint64_t() noexcept(false);
		/**
   * Cast this JSON Value to a signed integer.
   *
   * @returns A signed 64-bit integer.
   * @exception simdjson_error(Incorrect_Type) If the JSON Value is not a 64-bit integer.
   */
		inline operator int64_t() noexcept(false);
		/**
   * Cast this JSON Value to a double.
   *
   * @returns A double.
   * @exception simdjson_error(Incorrect_Type) If the JSON Value is not a valid floating-point number.
   */
		inline operator double() noexcept(false);
		/**
   * Cast this JSON Value to a string.
   *
   * The string is guaranteed to be valid UTF-8.
   *
   * Equivalent to get<std::string_view>().
   *
   * @returns An UTF-8 string. The string is stored in the parser and will be invalidated the next
   *          time it parses a document or when it is destroyed.
   * @exception simdjson_error(Incorrect_Type) if the JSON Value is not a string.
   */
		inline operator std::string_view() noexcept(false);
		/**
   * Cast this JSON Value to a RawJsonString.
   *
   * The string is guaranteed to be valid UTF-8, and may have escapes in it (e.g. \\ or \n).
   *
   * @returns A pointer to the raw JSON for the given string.
   * @exception simdjson_error(Incorrect_Type) if the JSON Value is not a string.
   */
		inline operator RawJsonString() noexcept(false);
		/**
   * Cast this JSON Value to a bool.
   *
   * @returns A bool Value.
   * @exception simdjson_error(Incorrect_Type) if the JSON Value is not true or false.
   */
		inline operator bool() noexcept(false);

		/**
   * Begin array iteration.
   *
   * Part of the std::iterable interface.
   *
   * @returns Incorrect_Type If the JSON Value is not an array.
   */
		inline JsonifierResult<ArrayIterator> begin() & noexcept;
		/**
   * Sentinel representing the end of the array.
   *
   * Part of the std::iterable interface.
   */
		inline JsonifierResult<ArrayIterator> end() & noexcept;
		/**
   * This method scans the array and counts the number of elements.
   * The countElements method should always be called before you have begun
   * iterating through the array: it is expected that you are pointing at
   * the beginning of the array.
   * The runtime complexity is linear in the size of the array. After
   * calling this function, if successful, the array is 'rewinded' at its
   * beginning as if it had never been accessed. If the JSON is malformed (e.g.,
   * there is a missing comma), then an error is returned and it is no longer
   * safe to continue.
   *
   * Performance hint: You should only call countElements() as a last
   * resort as it may require scanning the document twice or more.
   */
		inline JsonifierResult<size_t> countElements() & noexcept;
		/**
   * This method scans the Object and counts the number of key-Value pairs.
   * The countFields method should always be called before you have begun
   * iterating through the Object: it is expected that you are pointing at
   * the beginning of the Object.
   * The runtime complexity is linear in the size of the Object. After
   * calling this function, if successful, the Object is 'rewinded' at its
   * beginning as if it had never been accessed. If the JSON is malformed (e.g.,
   * there is a missing comma), then an error is returned and it is no longer
   * safe to continue.
   *
   * To check that an Object is empty, it is more performant to use
   * the is_empty() method on the Object instance.
   *
   * Performance hint: You should only call countFields() as a last
   * resort as it may require scanning the document twice or more.
   */
		inline JsonifierResult<size_t> countFields() & noexcept;
		/**
   * Get the Value at the given index in the array. This function has linear-time complexity.
   * This function should only be called once on an array instance since the array iterator is not reset between each call.
   *
   * @return The Value at the given index, or:
   *         - Out_Of_Bounds if the array index is larger than an array length
   */
		inline JsonifierResult<Value> at(size_t index) noexcept;
		/**
   * Look up a Field by name on an Object (order-sensitive).
   *
   * The following code reads z, then y, then x, and thus will not retrieve x or y if fed the
   * JSON `{ "x": 1, "y": 2, "z": 3 }`:
   *
   * ```c++
   * simdjson::ondemand::parser parser;
   * auto obj = parser.parse(R"( { "x": 1, "y": 2, "z": 3 } )"_padded);
   * double z = obj.findField("z");
   * double y = obj.findField("y");
   * double x = obj.findField("x");
   * ```
   * If you have multiple fields with a matching key ({"x": 1,  "x": 1}) be mindful
   * that only one Field is returned.

   * **Raw Keys:** The lookup will be done against the *raw* key, and will not unescape keys.
   * e.g. `Object["a"]` will match `{ "a": 1 }`, but will *not* match `{ "\u0061": 1 }`.
   *
   * @param key The key to look up.
   * @returns The Value of the Field, or No_Such_Field if the Field is not in the Object.
   */
		inline JsonifierResult<Value> findField(std::string_view key) noexcept;
		/** @overload inline JsonifierResult<Value> findField(std::string_view key) noexcept; */
		inline JsonifierResult<Value> findField(const char* key) noexcept;

		/**
   * Look up a Field by name on an Object, without regard to key order.
   *
   * **Performance Notes:** This is a bit less performant than findField(), though its effect varies
   * and often appears negligible. It starts out normally, starting out at the last Field; but if
   * the Field is not found, it scans from the beginning of the Object to see if it missed it. That
   * missing case has a non-cache-friendly bump and lots of extra scanning, especially if the Object
   * in question is large. The fact that the extra code is there also bumps the executable size.
   *
   * It is the default, however, because it would be highly surprising (and hard to debug) if the
   * default behavior failed to look up a Field just because it was in the wrong order--and many
   * APIs assume this. Therefore, you must be explicit if you want to treat objects as out of order.
   *
   * If you have multiple fields with a matching key ({"x": 1,  "x": 1}) be mindful
   * that only one Field is returned.
   *
   * Use findField() if you are sure fields will be in order (or are willing to treat it as if the
   * Field wasn't there when they aren't).
   *
   * @param key The key to look up.
   * @returns The Value of the Field, or No_Such_Field if the Field is not in the Object.
   */
		inline JsonifierResult<Value> findFieldUnordered(std::string_view key) noexcept;
		/** @overload inline JsonifierResult<Value> findFieldUnordered(std::string_view key) noexcept; */
		inline JsonifierResult<Value> findFieldUnordered(const char* key) noexcept;
		/** @overload inline JsonifierResult<Value> findFieldUnordered(std::string_view key) noexcept; */
		inline JsonifierResult<Value> operator[](std::string_view key) noexcept;
		/** @overload inline JsonifierResult<Value> findFieldUnordered(std::string_view key) noexcept; */
		inline JsonifierResult<Value> operator[](const char* key) noexcept;

		/**
   * Get the type of this JSON Value. It does not validate or consume the Value.
   * E.g., you must still call "isNull()" to check that a Value is null even if
   * "type()" returns JsonType::null.
   *
   * NOTE: If you're only expecting a Value to be one type (a typical case), it's generally
   * better to just call .getDouble, .getString, etc. and check for Incorrect_Type (or just
   * let it throw an exception).
   *
   * @return The type of JSON Value (JsonType::array, JsonType::Object, JsonType::string,
   *     JsonType::number, JsonType::boolean, or JsonType::null).
   * @error Tape_Error when the JSON Value is a bad token like "}" "," or "alse".
   */
		inline JsonifierResult<JsonType> type() noexcept;

		/**
   * Checks whether the Value is a scalar (string, number, null, Boolean).
   * Returns false when there it is an array or Object.
   *
   * @returns true if the type is string, number, null, Boolean
   * @error Tape_Error when the JSON Value is a bad token like "}" "," or "alse".
   */
		inline JsonifierResult<bool> isScalar() noexcept;

		/**
   * Checks whether the Value is a negative number.
   *
   * @returns true if the number if negative.
   */
		inline bool is_negative() noexcept;
		/**
   * Checks whether the Value is an integer number. Note that
   * this requires to partially parse the number string. If
   * the Value is determined to be an integer, it may still
   * not parse properly as an integer in subsequent steps
   * (e.g., it might overflow).
   *
   * Performance note: if you call this function systematically
   * before parsing a number, you may have fallen for a performance
   * anti-pattern.
   *
   * @returns true if the number if negative.
  /**
   * Get the raw JSON for this token.
   *
   * The string_view will always point into the input buffer.
   *
   * The string_view will start at the beginning of the token, and include the entire token
   * *as well as all spaces until the next token (or EOF).* This means, for example, that a
   * string token always begins with a " and is always terminated by the final ", possibly
   * followed by a number of spaces.
   *
   * The string_view is *not* null-terminated. However, if this is a scalar (string, number,
   * boolean, or null), the character after the end of the string_view is guaranteed to be
   * a non-space token.
   *
   * Tokens include:
   * - {
   * - [
   * - "a string (possibly with UTF-8 or backslashed characters like \\\")".
   * - -1.2e-100
   * - true
   * - false
   * - null
   */
		inline std::string_view rawJsonToken() noexcept;

		/**
   * Returns the current location in the document if in bounds.
   */
		inline JsonifierResult<const char*> currentLocation() noexcept;

		/**
   * Returns the current depth in the document if in bounds.
   *
   * E.g.,
   *  0 = finished with document
   *  1 = document root Value (could be [ or {, not yet known)
   *  2 = , or } inside root array/Object
   *  3 = key or Value inside root array/Object.
   */
		inline int32_t currentDepth() const noexcept;

		/**
   * Get the Value associated with the given JSON pointer.  We use the RFC 6901
   * https://tools.ietf.org/html/rfc6901 standard.
   *
   *   ondemand::parser parser;
   *   auto json = R"({ "foo": { "a": [ 10, 20, 30 ] }})"_padded;
   *   auto doc = parser.iterate(json);
   *   doc.atPointer("/foo/a/1") == 20
   *
   * It is allowed for a key to be the empty string:
   *
   *   ondemand::parser parser;
   *   auto json = R"({ "": { "a": [ 10, 20, 30 ] }})"_padded;
   *   auto doc = parser.iterate(json);
   *   doc.atPointer("//a/1") == 20
   *
   * Note that atPointer() called on the document automatically calls the document's rewind
   * method between each call. It invalidates all previously accessed arrays, objects and values
   * that have not been consumed.
   *
   * Calling atPointer() on non-document instances (e.g., arrays and objects) is not
   * standardized (by RFC 6901). We provide some experimental support for JSON pointers
   * on non-document instances.  Yet it is not the case when calling atPointer on an array
   * or an Object instance: there is no rewind and no invalidation.
   *
   * You may only call atPointer on an array after it has been created, but before it has
   * been first accessed. When calling atPointer on an array, the pointer is advanced to
   * the location indicated by the JSON pointer (in case of success). It is no longer possible
   * to call atPointer on the same array.
   *
   * You may call atPointer more than once on an Object, but each time the pointer is advanced
   * to be within the Value matched by the key indicated by the JSON pointer query. Thus any preceding
   * key (as well as the current key) can no longer be used with following JSON pointer calls.
   *
   * Also note that atPointer() relies on findField() which implies that we do not unescape keys when matching
   *
   * @return The Value associated with the given JSON pointer, or:
   *         - No_Such_Field if a Field does not exist in an Object
   *         - Out_Of_Bounds if an array index is larger than an array length
   *         - Incorrect_Type if a non-integer is used to access an array
   *         - Invalid_Json_Pointer if the JSON pointer is invalid and cannot be parsed
   */
		inline JsonifierResult<Value> atPointer(std::string_view json_pointer) noexcept;

	  protected:
		/**
   * Create a Value.
   */
		inline Value(const ValueIterator& iter) noexcept;

		/**
   * Skip this Value, allowing iteration to continue.
   */
		inline void skip() noexcept;

		/**
   * Start a Value at the current position.
   *
   * (It should already be started; this is just a self-documentation method.)
   */
		static inline Value start(const ValueIterator& iter) noexcept;

		/**
   * Resume a Value.
   */
		static inline Value resume(const ValueIterator& iter) noexcept;

		/**
   * Get the Object, starting or resuming it as necessary
   */
		inline JsonifierResult<Object> start_or_resume_object() noexcept;

		// inline void log_value(const char *type) const noexcept;
		// inline void log_error(const char *message) const noexcept;

		ValueIterator iterator{};

		friend class Document;
		friend class ArrayIterator;
		friend class Field;
		friend class Object;
		friend struct JsonifierResult<Value>;
		friend struct JsonifierResult<Field>;
	};


	template<> struct JsonifierResult<Value> : public ImplementationJsonifierResultBase<Value> {
	  public:
		inline JsonifierResult(Value&& Value) noexcept;///< @private
		inline JsonifierResult(ErrorCode error) noexcept;///< @private
		inline JsonifierResult() noexcept = default;

		inline JsonifierResult<Array> getArray() noexcept;
		inline JsonifierResult<Object> getObject() noexcept;

		inline JsonifierResult<uint64_t> getUint64() noexcept;
		inline JsonifierResult<uint64_t> get_uint64_in_string() noexcept;
		inline JsonifierResult<int64_t> getInt64() noexcept;
		inline JsonifierResult<int64_t> get_int64_in_string() noexcept;
		inline JsonifierResult<double> getDouble() noexcept;
		inline JsonifierResult<double> get_double_in_string() noexcept;
		inline JsonifierResult<std::string_view> getString() noexcept;
		inline JsonifierResult<RawJsonString> getRawJsonString() noexcept;
		inline JsonifierResult<bool> getBool() noexcept;
		inline JsonifierResult<bool> isNull() noexcept;

		template<typename T> inline JsonifierResult<T> get() noexcept;

		template<typename T> inline ErrorCode get(T& out) noexcept;


		inline operator Array() noexcept(false);
		inline operator Object() noexcept(false);
		inline operator uint64_t() noexcept(false);
		inline operator int64_t() noexcept(false);
		inline operator double() noexcept(false);
		inline operator std::string_view() noexcept(false);
		inline operator RawJsonString() noexcept(false);
		inline operator bool() noexcept(false);
		inline JsonifierResult<size_t> countElements() & noexcept;
		inline JsonifierResult<size_t> countFields() & noexcept;
		inline JsonifierResult<Value> at(size_t index) noexcept;
		inline JsonifierResult<ArrayIterator> begin() & noexcept;
		inline JsonifierResult<ArrayIterator> end() & noexcept;

		/**
   * Look up a Field by name on an Object (order-sensitive).
   *
   * The following code reads z, then y, then x, and thus will not retrieve x or y if fed the
   * JSON `{ "x": 1, "y": 2, "z": 3 }`:
   *
   * ```c++
   * simdjson::ondemand::parser parser;
   * auto obj = parser.parse(R"( { "x": 1, "y": 2, "z": 3 } )"_padded);
   * double z = obj.findField("z");
   * double y = obj.findField("y");
   * double x = obj.findField("x");
   * ```
   *
   * **Raw Keys:** The lookup will be done against the *raw* key, and will not unescape keys.
   * e.g. `Object["a"]` will match `{ "a": 1 }`, but will *not* match `{ "\u0061": 1 }`.
   *
   * @param key The key to look up.
   * @returns The Value of the Field, or No_Such_Field if the Field is not in the Object.
   */
		inline JsonifierResult<Value> findField(std::string_view key) noexcept;
		/** @overload inline JsonifierResult<Value> findField(std::string_view key) noexcept; */
		inline JsonifierResult<Value> findField(const char* key) noexcept;

		/**
   * Look up a Field by name on an Object, without regard to key order.
   *
   * **Performance Notes:** This is a bit less performant than findField(), though its effect varies
   * and often appears negligible. It starts out normally, starting out at the last Field; but if
   * the Field is not found, it scans from the beginning of the Object to see if it missed it. That
   * missing case has a non-cache-friendly bump and lots of extra scanning, especially if the Object
   * in question is large. The fact that the extra code is there also bumps the executable size.
   *
   * It is the default, however, because it would be highly surprising (and hard to debug) if the
   * default behavior failed to look up a Field just because it was in the wrong order--and many
   * APIs assume this. Therefore, you must be explicit if you want to treat objects as out of order.
   *
   * Use findField() if you are sure fields will be in order (or are willing to treat it as if the
   * Field wasn't there when they aren't).
   *
   * @param key The key to look up.
   * @returns The Value of the Field, or No_Such_Field if the Field is not in the Object.
   */
		inline JsonifierResult<Value> findFieldUnordered(std::string_view key) noexcept;
		/** @overload inline JsonifierResult<Value> findFieldUnordered(std::string_view key) noexcept; */
		inline JsonifierResult<Value> findFieldUnordered(const char* key) noexcept;
		/** @overload inline JsonifierResult<Value> findFieldUnordered(std::string_view key) noexcept; */
		inline JsonifierResult<Value> operator[](std::string_view key) noexcept;
		/** @overload inline JsonifierResult<Value> findFieldUnordered(std::string_view key) noexcept; */
		inline JsonifierResult<Value> operator[](const char* key) noexcept;

		/**
   * Get the type of this JSON Value.
   *
   * NOTE: If you're only expecting a Value to be one type (a typical case), it's generally
   * better to just call .getDouble, .getString, etc. and check for Incorrect_Type (or just
   * let it throw an exception).
   */
		inline JsonifierResult<JsonType> type() noexcept;
		inline JsonifierResult<bool> isScalar() noexcept;
		inline JsonifierResult<bool> is_negative() noexcept;
		inline JsonifierResult<bool> is_integer() noexcept;

		/** @copydoc inline std::string_view Value::rawJsonToken() const noexcept */
		inline JsonifierResult<std::string_view> rawJsonToken() noexcept;

		/** @copydoc inline JsonifierResult<const char *> currentLocation() noexcept */
		inline JsonifierResult<const char*> currentLocation() noexcept;
		/** @copydoc inline int32_t currentDepth() const noexcept */
		inline JsonifierResult<int32_t> currentDepth() const noexcept;
		inline JsonifierResult<Value> atPointer(std::string_view json_pointer) noexcept;
	};

}
