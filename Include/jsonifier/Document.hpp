#pragma once

#include <jsonifier/Base.hpp>
#include <jsonifier/Value.hpp>

namespace Jsonifier {

	class Parser;
	class Array;
	class Object;
	class Value;
	class RawJsonString;
	class ArrayIterator;
	class ValueIterator;
	/**
 * A JSON Document. It holds a JsonIteratorinstance.
 *
 * Used by tokens to get text, and string buffer location.
 *
 * You must keep the Document around during iteration.
 */
	class Jsonifier_Dll Document {
	  public:
		/**
   * Create a new invalid Document.
   *
   * Exists so you can declare a variable and later assign to it before use.
   */
		inline Document() noexcept = default;
		inline Document(const Document& other) noexcept = delete;// pass your documents by reference, not by copy
		inline Document(Document&& other) noexcept = default;
		inline Document& operator=(const Document& other) noexcept = delete;
		inline Document& operator=(Document&& other) noexcept = default;

		/**
   * Cast this JSON Value to an Array.
   *
   * @returns An Object that can be used to iterate the Array.
   * @returns Incorrect_Type If the JSON Value is not an Array.
   */
		inline JsonifierResult<Array> getArray() & noexcept;
		/**
   * Cast this JSON Value to an Object.
   *
   * @returns An Object that can be used to look up or iterate fields.
   * @returns Incorrect_Type If the JSON Value is not an Object.
   */
		inline JsonifierResult<Object> getObject() & noexcept;
		/**
   * Cast this JSON Value to an unsigned integer.
   *
   * @returns A signed 64-bit integer.
   * @returns Incorrect_Type If the JSON Value is not a 64-bit unsigned integer.
   */
		inline JsonifierResult<uint64_t> getUint64() noexcept;
		/**
   * Cast this JSON Value (inside string) to an unsigned integer.
   *
   * @returns A signed 64-bit integer.
   * @returns Incorrect_Type If the JSON Value is not a 64-bit unsigned integer.
   */
		inline JsonifierResult<uint64_t> get_uint64_in_string() noexcept;
		/**
   * Cast this JSON Value to a signed integer.
   *
   * @returns A signed 64-bit integer.
   * @returns Incorrect_Type If the JSON Value is not a 64-bit integer.
   */
		inline JsonifierResult<int64_t> getInt64() noexcept;
		/**
   * Cast this JSON Value (inside string) to a signed integer.
   *
   * @returns A signed 64-bit integer.
   * @returns Incorrect_Type If the JSON Value is not a 64-bit integer.
   */
		inline JsonifierResult<int64_t> get_int64_in_string() noexcept;
		/**
   * Cast this JSON Value to a double.
   *
   * @returns A double.
   * @returns Incorrect_Type If the JSON Value is not a valid floating-point number.
   */
		inline JsonifierResult<double> getDouble() noexcept;

		/**
   * Cast this JSON Value (inside string) to a double.
   *
   * @returns A double.
   * @returns Incorrect_Type If the JSON Value is not a valid floating-point number.
   */
		inline JsonifierResult<double> get_double_in_string() noexcept;
		/**
   * Cast this JSON Value to a string.
   *
   * The string is guaranteed to be valid UTF-8.
   *
   * Important: Calling getString() twice on the same Document is an error.
   *
   * @returns An UTF-8 string. The string is stored in the parser and will be invalidated the next
   *          time it parses a Document or when it is destroyed.
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
   * Cast this JSON Value to a Value when the Document is an Object or an Array.
   *
   * @returns A Value if a JSON Array or Object cannot be found.
   * @returns SCALAR_DOCUMENT_AS_VALUE error is the Document is a scalar (see isScalar() function).
   */
		inline JsonifierResult<Value> get_value() noexcept;

		/**
   * Checks if this JSON Value is null.  If and only if the Value is
   * null, then it is consumed (we advance). If we find a token that
   * begins with 'n' but is not 'null', then an error is returned.
   *
   * @returns Whether the Value is null.
   * @returns Incorrect_Type If the JSON Value begins with 'n' and is not 'null'.
   */
		inline JsonifierResult<bool> isNull() noexcept;

		/**
   * Get this Value as the given type.
   *
   * Supported types: Object, Array, RawJsonString, string_view, uint64_t, int64_t, double, bool
   *
   * You may use getDouble(), getBool(), getUint64(), getInt64(),
   * getObject(), getArray(), getRawJsonString(), or getString() instead.
   *
   * @returns A Value of the given type, parsed from the JSON.
   * @returns Incorrect_Type If the JSON Value is not the given type.
   */
		template<typename T> inline JsonifierResult<T> get() & noexcept {
			// Unless the simdjson library provides an inline implementation, calling this method should
			// immediately fail.
			static_assert(!sizeof(T), "The get method with given type is not implemented by the simdjson library.");
		}
		/** @overload template<typename T> JsonifierResult<T> get() & noexcept */
		template<typename T> inline JsonifierResult<T> get() && noexcept {
			// Unless the simdjson library provides an inline implementation, calling this method should
			// immediately fail.
			static_assert(!sizeof(T), "The get method with given type is not implemented by the simdjson library.");
		}

		/**
   * Get this Value as the given type.
   *
   * Supported types: Object, Array, RawJsonString, string_view, uint64_t, int64_t, double, bool, Value
   *
   * Be mindful that the Document instance must remain in scope while you are accessing Object, Array and Value instances.
   *
   * @param out This is set to a Value of the given type, parsed from the JSON. If there is an error, this may not be initialized.
   * @returns Incorrect_Type If the JSON Value is not an Object.
   * @returns Success If the parse succeeded and the out parameter was set to the Value.
   */
		template<typename T> inline ErrorCode get(T& out) & noexcept;
		/** @overload template<typename T> ErrorCode get(T &out) & noexcept */
		template<typename T> inline ErrorCode get(T& out) && noexcept;


		/**
   * Cast this JSON Value to an Array.
   *
   * @returns An Object that can be used to iterate the Array.
   * @exception simdjson_error(Incorrect_Type) If the JSON Value is not an Array.
   */
		inline operator Array() & noexcept(false);
		/**
   * Cast this JSON Value to an Object.
   *
   * @returns An Object that can be used to look up or iterate fields.
   * @exception simdjson_error(Incorrect_Type) If the JSON Value is not an Object.
   */
		inline operator Object() & noexcept(false);
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
   * @returns An UTF-8 string. The string is stored in the parser and will be invalidated the next
   *          time it parses a Document or when it is destroyed.
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
   * Cast this JSON Value to a Value.
   *
   * @returns A Value Value.
   * @exception if a JSON Value cannot be found
   */
		inline operator Value() noexcept(false);
		/**
   * This method scans the Array and counts the number of elements.
   * The countElements method should always be called before you have begun
   * iterating through the Array: it is expected that you are pointing at
   * the beginning of the Array.
   * The runtime complexity is linear in the size of the Array. After
   * calling this function, if successful, the Array is 'rewinded' at its
   * beginning as if it had never been accessed. If the JSON is malformed (e.g.,
   * there is a missing comma), then an error is returned and it is no longer
   * safe to continue.
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
   * the is_empty() method.
   */
		inline JsonifierResult<size_t> countFields() & noexcept;
		/**
   * Get the Value at the given index in the Array. This function has linear-time complexity.
   * This function should only be called once on an Array instance since the Array iterator is not reset between each call.
   *
   * @return The Value at the given index, or:
   *         - Out_Of_Bounds if the Array index is larger than an Array length
   */
		inline JsonifierResult<Value> at(size_t index) & noexcept;
		/**
   * Begin Array iteration.
   *
   * Part of the std::iterable interface.
   */
		inline JsonifierResult<ArrayIterator> begin() & noexcept;
		/**
   * Sentinel representing the end of the Array.
   *
   * Part of the std::iterable interface.
   */
		inline JsonifierResult<ArrayIterator> end() & noexcept;

		/**
   * Look up a Field by name on an Object (order-sensitive).
   *
   * The following code reads z, then y, then x, and thus will not retrieve x or y if fed the
   * JSON `{ "x": 1, "y": 2, "z": 3 }`:
   *
   * ```c++
   * simdjson::Parser parser;
   * auto obj = parser.parse(R"( { "x": 1, "y": 2, "z": 3 } )"_padded);
   * double z = obj.findField("z");
   * double y = obj.findField("y");
   * double x = obj.findField("x");
   * ```
   *
   * **Raw Keys:** The lookup will be done against the *raw* key, and will not unescape keys.
   * e.g. `Object["a"]` will match `{ "a": 1 }`, but will *not* match `{ "\u0061": 1 }`.
   *
   *
   * You must consume the fields on an Object one at a time. A request for a new key
   * invalidates previous Field values: it makes them unsafe. E.g., the Array
   * given by content["bids"].getArray() should not be accessed after you have called
   * content["asks"].getArray(). You can detect such mistakes by first compiling and running
   * the code in Debug mode (or with the macro `SIMDJSON_DEVELOPMENT_CHECKS` set to 1): an
   * OUT_OF_ORDER_ITERATION error is generated.
   *
   * You are expected to access keys only once. You should access the Value corresponding to
   * a key a single time. Doing Object["mykey"].to_string()and then again Object["mykey"].to_string()
   * is an error.
   *
   * @param key The key to look up.
   * @returns The Value of the Field, or No_Such_Field if the Field is not in the Object.
   */
		inline JsonifierResult<Value> findField(std::string_view key) & noexcept;
		/** @overload inline JsonifierResult<Value> findField(std::string_view key) & noexcept; */
		inline JsonifierResult<Value> findField(const char* key) & noexcept;

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
   * You must consume the fields on an Object one at a time. A request for a new key
   * invalidates previous Field values: it makes them unsafe. E.g., the Array
   * given by content["bids"].getArray() should not be accessed after you have called
   * content["asks"].getArray(). You can detect such mistakes by first compiling and running
   * the code in Debug mode (or with the macro `SIMDJSON_DEVELOPMENT_CHECKS` set to 1): an
   * OUT_OF_ORDER_ITERATION error is generated.
   *
   * You are expected to access keys only once. You should access the Value corresponding to a key
   * a single time. Doing Object["mykey"].to_string() and then again Object["mykey"].to_string()
   * is an error.
   *
   * @param key The key to look up.
   * @returns The Value of the Field, or No_Such_Field if the Field is not in the Object.
   */
		inline JsonifierResult<Value> findFieldUnordered(std::string_view key) & noexcept;
		/** @overload inline JsonifierResult<Value> findFieldUnordered(std::string_view key) & noexcept; */
		inline JsonifierResult<Value> findFieldUnordered(const char* key) & noexcept;
		/** @overload inline JsonifierResult<Value> findFieldUnordered(std::string_view key) & noexcept; */
		inline JsonifierResult<Value> operator[](std::string_view key) & noexcept;
		/** @overload inline JsonifierResult<Value> findFieldUnordered(std::string_view key) & noexcept; */
		inline JsonifierResult<Value> operator[](const char* key) & noexcept;

		/**
   * Get the type of this JSON Value. It does not validate or consume the Value.
   * E.g., you must still call "isNull()" to check that a Value is null even if
   * "type()" returns JsonType::null.
   *
   * NOTE: If you're only expecting a Value to be one type (a typical case), it's generally
   * better to just call .getDouble, .getString, etc. and check for Incorrect_Type (or just
   * let it throw an exception).
   *
   * @error Tape_Error when the JSON Value is a bad token like "}" "," or "alse".
   */
		inline JsonifierResult<JsonType> type() noexcept;

		/**
   * Checks whether the Document is a scalar (string, number, null, Boolean).
   * Returns false when there it is an Array or Object.
   *
   * @returns true if the type is string, number, null, Boolean
   * @error Tape_Error when the JSON Value is a bad token like "}" "," or "alse".
   */
		inline JsonifierResult<bool> isScalar() noexcept;


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
   * The string_view is *not* null-terminated. If this is a scalar (string, number,
   * boolean, or null), the character after the end of the string_view may be the padded buffer.
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
		inline JsonifierResult<std::string_view> rawJsonToken() noexcept;

		/**
   * Reset the iterator inside the Document instance so we are pointing back at the
   * beginning of the Document, as if it had just been created. It invalidates all
   * values, objects and arrays that you have created so far (including unescaped strings).
   */
		inline void rewind() noexcept;
		/**
   * Returns debugging information.
   */
		inline std::string to_debug_string() noexcept;
		/**
   * Some unrecoverable error conditions may render the Document instance unusable.
   * The is_alive() method returns true when the Document is still suitable.
   */
		inline bool is_alive() noexcept;

		/**
   * Returns the current location in the Document if in bounds.
   */
		inline JsonifierResult<const char*> currentLocation() noexcept;

		/**
   * Returns the current depth in the Document if in bounds.
   *
   * E.g.,
   *  0 = finished with Document
   *  1 = Document root Value (could be [ or {, not yet known)
   *  2 = , or } inside root Array/Object
   *  3 = key or Value inside root Array/Object.
   */
		inline int32_t currentDepth() const noexcept;

		/**
   * Get the Value associated with the given JSON pointer.  We use the RFC 6901
   * https://tools.ietf.org/html/rfc6901 standard.
   *
   *   Parser parser;
   *   auto json = R"({ "foo": { "a": [ 10, 20, 30 ] }})"_padded;
   *   auto doc = parser.iterate(json);
   *   doc.atPointer("/foo/a/1") == 20
   *
   * It is allowed for a key to be the empty string:
   *
   *   Parser parser;
   *   auto json = R"({ "": { "a": [ 10, 20, 30 ] }})"_padded;
   *   auto doc = parser.iterate(json);
   *   doc.atPointer("//a/1") == 20
   *
   * Note that atPointer() automatically calls rewind between each call. Thus
   * all values, objects and arrays that you have created so far (including unescaped strings)
   * are invalidated. After calling atPointer, you need to consume the result: string values
   * should be stored in your own variables, arrays should be decoded and stored in your own Array-like
   * structures and so forth.
   *
   * Also note that atPointer() relies on findField() which implies that we do not unescape keys when matching
   *
   * @return The Value associated with the given JSON pointer, or:
   *         - No_Such_Field if a Field does not exist in an Object
   *         - Out_Of_Bounds if an Array index is larger than an Array length
   *         - Incorrect_Type if a non-integer is used to access an Array
   *         - Invalid_Json_Pointer if the JSON pointer is invalid and cannot be parsed
   *         - SCALAR_DOCUMENT_AS_VALUE if the json_pointer is empty and the Document is not a scalar (see isScalar() function).
   */
		inline JsonifierResult<Value> atPointer(std::string_view json_pointer) noexcept;
		inline JsonifierResult<std::string_view> raw_json() noexcept;

	  protected:
		inline ErrorCode consume() noexcept;

		inline Document(JsonIterator&& iterator) noexcept;
		inline const uint8_t* text(uint32_t idx) const noexcept;

		inline ValueIterator resume_value_iterator() noexcept;
		inline ValueIterator get_root_value_iterator() noexcept;
		inline JsonifierResult<Object> startOrResumeObject() noexcept;
		static inline Document start(JsonIterator&& iterator) noexcept;
		JsonIterator iterator{};
		static constexpr uint32_t DOCUMENT_DEPTH = 0;

		friend class ArrayIterator;
		friend class Parser;
		friend class Object;
		friend class Array;
		friend class Field;
		friend class document_stream;
	};

	template<> struct JsonifierResult<Document> : public ImplementationJsonifierResultBase<Document> {
	  public:
		inline JsonifierResult(Document&& Value) noexcept;///< @private
		inline JsonifierResult(ErrorCode error) noexcept;///< @private
		inline JsonifierResult() noexcept = default;
		inline ErrorCode rewind() noexcept;

		inline JsonifierResult<Array> getArray() & noexcept;
		inline JsonifierResult<Object> getObject() & noexcept;
		inline JsonifierResult<uint64_t> getUint64() noexcept;
		inline JsonifierResult<int64_t> getInt64() noexcept;
		inline JsonifierResult<double> getDouble() noexcept;
		inline JsonifierResult<double> get_double_from_string() noexcept;
		inline JsonifierResult<std::string_view> getString() noexcept;
		inline JsonifierResult<RawJsonString> getRawJsonString() noexcept;
		inline JsonifierResult<bool> getBool() noexcept;
		inline JsonifierResult<Value> get_value() noexcept;
		inline JsonifierResult<bool> isNull() noexcept;

		template<typename T> inline JsonifierResult<T> get() & noexcept;
		template<typename T> inline JsonifierResult<T> get() && noexcept;

		template<typename T> inline ErrorCode get(T& out) & noexcept;
		template<typename T> inline ErrorCode get(T& out) && noexcept;


		inline operator Array() & noexcept(false);
		inline operator Object() & noexcept(false);
		inline operator uint64_t() noexcept(false);
		inline operator int64_t() noexcept(false);
		inline operator double() noexcept(false);
		inline operator std::string_view() noexcept(false);
		inline operator RawJsonString() noexcept(false);
		inline operator bool() noexcept(false);
		inline operator Value() noexcept(false);

		inline JsonifierResult<size_t> countElements() & noexcept;
		inline JsonifierResult<size_t> countFields() & noexcept;
		inline JsonifierResult<Value> at(size_t index) & noexcept;
		inline JsonifierResult<ArrayIterator> begin() & noexcept;
		inline JsonifierResult<ArrayIterator> end() & noexcept;
		inline JsonifierResult<Value> findField(std::string_view key) & noexcept;
		inline JsonifierResult<Value> findField(const char* key) & noexcept;
		inline JsonifierResult<Value> operator[](std::string_view key) & noexcept;
		inline JsonifierResult<Value> operator[](const char* key) & noexcept;
		inline JsonifierResult<Value> findFieldUnordered(std::string_view key) & noexcept;
		inline JsonifierResult<Value> findFieldUnordered(const char* key) & noexcept;
		inline JsonifierResult<JsonType> type() noexcept;
		inline JsonifierResult<bool> isScalar() noexcept;
		inline JsonifierResult<const char*> currentLocation() noexcept;
		inline int32_t currentDepth() const noexcept;
		/** @copydoc inline std::string_view Document::rawJsonToken() const noexcept */
		inline JsonifierResult<std::string_view> rawJsonToken() noexcept;

		inline JsonifierResult<Value> atPointer(std::string_view json_pointer) noexcept;
	};


}
