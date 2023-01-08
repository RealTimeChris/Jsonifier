#pragma once

#include <jsonifier/Base.hpp>
#include <jsonifier/ObjectIterator.hpp>
#include <jsonifier/Value.hpp>

namespace Jsonifier {
	class Value;
	class Jsonifier_Dll Object {
	  public:
		/**
   * Create a new invalid Object.
   *
   * Exists so you can declare a variable and later assign to it before use.
   */
		inline Object() noexcept = default;

		inline JsonifierResult<ObjectIterator> begin() noexcept;
		inline JsonifierResult<ObjectIterator> end() noexcept;
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
	   * If you have multiple fields with a matching key ({"x": 1,  "x": 1}) be mindful
	   * that only one Field is returned.
	   *
	   * **Raw Keys:** The lookup will be done against the *raw* key, and will not unescape keys.
	   * e.g. `Object["a"]` will match `{ "a": 1 }`, but will *not* match `{ "\u0061": 1 }`.
	   *
	   * You must consume the fields on an Object one at a time. A request for a new key
	   * invalidates previous Field values: it makes them unsafe. The Value instance you get
	   * from  `content["bids"]` becomes invalid when you call `content["asks"]`. The Array
	   * given by content["bids"].getArray() should not be accessed after you have called
	   * content["asks"].getArray(). You can detect such mistakes by first compiling and running
	   * the code in Debug mode (or with the macro `SIMDJSON_DEVELOPMENT_CHECKS` set to 1): an
	   * OUT_OF_ORDER_ITERATION error is generated.
	   *
	   * You are expected to access keys only once. You should access the Value corresponding to a
	   * key a single time. Doing Object["mykey"].toString() and then again Object["mykey"].toString()
	   * is an error.
	   *
	   * @param key The key to look up.
	   * @returns The Value of the Field, or No_Such_Field if the Field is not in the Object.
	   */
		inline JsonifierResult<Value> findField(std::string_view key) & noexcept;
		/** @overload inline JsonifierResult<Value> findField(std::string_view key) & noexcept; */
		inline JsonifierResult<Value> findField(std::string_view key) && noexcept;

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
	   * If you have multiple fields with a matching key ({"x": 1,  "x": 1}) be mindful
	   * that only one Field is returned.
	   *
	   * You must consume the fields on an Object one at a time. A request for a new key
	   * invalidates previous Field values: it makes them unsafe. The Value instance you get
	   * from  `content["bids"]` becomes invalid when you call `content["asks"]`. The Array
	   * given by content["bids"].getArray() should not be accessed after you have called
	   * content["asks"].getArray(). You can detect such mistakes by first compiling and running
	   * the code in Debug mode (or with the macro `SIMDJSON_DEVELOPMENT_CHECKS` set to 1): an
	   * OUT_OF_ORDER_ITERATION error is generated.
	   *
	   * You are expected to access keys only once. You should access the Value corresponding to a key
	   * a single time. Doing Object["mykey"].toString() and then again Object["mykey"].toString() is an error.
	   *
	   * @param key The key to look up.
	   * @returns The Value of the Field, or No_Such_Field if the Field is not in the Object.
	   */
		inline JsonifierResult<Value> findFieldUnordered(std::string_view key) & noexcept;
		/** @overload inline JsonifierResult<Value> findFieldUnordered(std::string_view key) & noexcept; */
		inline JsonifierResult<Value> findFieldUnordered(std::string_view key) && noexcept;
		/** @overload inline JsonifierResult<Value> findFieldUnordered(std::string_view key) & noexcept; */
		inline JsonifierResult<Value> operator[](std::string_view key) & noexcept;
		/** @overload inline JsonifierResult<Value> findFieldUnordered(std::string_view key) & noexcept; */
		inline JsonifierResult<Value> operator[](std::string_view key) && noexcept;

		/**
	   * Get the Value associated with the given JSON pointer. We use the RFC 6901
	   * https://tools.ietf.org/html/rfc6901 standard, interpreting the current node
	   * as the root of its own JSON document.
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
	   * Note that atPointer() called on the document automatically calls the document's rewind
	   * method between each call. It invalidates all previously accessed arrays, objects and values
	   * that have not been consumed. Yet it is not the case when calling atPointer on an Object
	   * instance: there is no rewind and no invalidation.
	   *
	   * You may call atPointer more than once on an Object, but each time the pointer is advanced
	   * to be within the Value matched by the key indicated by the JSON pointer query. Thus any preceding
	   * key (as well as the current key) can no longer be used with following JSON pointer calls.
	   *
	   * Also note that atPointer() relies on findField() which implies that we do not unescape keys when matching.
	   *
	   * @return The Value associated with the given JSON pointer, or:
	   *         - No_Such_Field if a Field does not exist in an Object
	   *         - Out_Of_Bounds if an Array index is larger than an Array length
	   *         - Incorrect_Type if a non-integer is used to access an Array
	   *         - Invalid_Json_Pointer if the JSON pointer is invalid and cannot be parsed
	   */
		inline JsonifierResult<Value> atPointer(std::string_view json_pointer) noexcept;

		/**
	   * Reset the iterator so that we are pointing back at the
	   * beginning of the Object. You should still consume values only once even if you
	   * can iterate through the Object more than once. If you unescape a string within
	   * the Object more than once, you have unsafe code. Note that rewinding an Object
	   * means that you may need to reparse it anew: it is not a free operation.
	   *
	   * @returns true if the Object contains some elements (not empty)
	   */
		inline JsonifierResult<bool> reset() & noexcept;
		/**
	   * This method scans the beginning of the Object and checks whether the
	   * Object is empty.
	   * The runtime complexity is constant time. After
	   * calling this function, if successful, the Object is 'rewinded' at its
	   * beginning as if it had never been accessed. If the JSON is malformed (e.g.,
	   * there is a missing comma), then an error is returned and it is no longer
	   * safe to continue.
	   */
		inline JsonifierResult<bool> is_empty() & noexcept;
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
	   *
	   * Performance hint: You should only call countFields() as a last
	   * resort as it may require scanning the document twice or more.
	   */
		inline JsonifierResult<size_t> countFields() & noexcept;
		/**
	   * Consumes the Object and returns a string_view instance corresponding to the
	   * Object as represented in JSON. It points inside the original byte Array containing
	   * the JSON document.
	   */
		inline JsonifierResult<std::string_view> raw_json() noexcept;

	  protected:
		/**
	   * Go to the end of the Object, no matter where you are right now.
	   */
		inline ErrorCode consume() noexcept;
		static inline JsonifierResult<Object> start(ValueIterator& iterator) noexcept;
		static inline JsonifierResult<Object> start_root(ValueIterator& iterator) noexcept;
		static inline JsonifierResult<Object> started(ValueIterator& iterator) noexcept;
		static inline Object resume(const ValueIterator& iterator) noexcept;
		inline Object(const ValueIterator& iterator) noexcept;

		inline ErrorCode find_field_raw(const std::string_view key) noexcept;

		ValueIterator iterator{};

		friend class Value;
		friend class Document;
		friend struct JsonifierResult<Object>;
	};

	template<> struct JsonifierResult<Object> : public ImplementationJsonifierResultBase<Object> {
	  public:
		inline JsonifierResult(Object&& Value) noexcept;///< @private
		inline JsonifierResult(ErrorCode error) noexcept;///< @private
		inline JsonifierResult() noexcept = default;

		inline JsonifierResult<ObjectIterator> begin() noexcept;
		inline JsonifierResult<ObjectIterator> end() noexcept;
		inline JsonifierResult<Value> findField(std::string_view key) & noexcept;
		inline JsonifierResult<Value> findField(std::string_view key) && noexcept;
		inline JsonifierResult<Value> findFieldUnordered(std::string_view key) & noexcept;
		inline JsonifierResult<Value> findFieldUnordered(std::string_view key) && noexcept;
		inline JsonifierResult<Value> operator[](std::string_view key) & noexcept;
		inline JsonifierResult<Value> operator[](std::string_view key) && noexcept;
		inline JsonifierResult<Value> atPointer(std::string_view json_pointer) noexcept;
		inline JsonifierResult<bool> reset() noexcept;
		inline JsonifierResult<bool> is_empty() noexcept;
		inline JsonifierResult<size_t> countFields() & noexcept;
	};

}
