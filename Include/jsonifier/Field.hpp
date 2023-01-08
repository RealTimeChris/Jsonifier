#pragma once

#include <jsonifier/Base.hpp>
#include <jsonifier/Value.hpp>
#include <jsonifier/RawJsonString.hpp>

namespace Jsonifier {
	class Value;
	/**
 * A JSON Field (key/Value pair) in an Object.
 *
 * Returned from Object iteration.
 *
 * Extends from std::pair<RawJsonString, Value> so you can use C++ algorithms that rely on pairs.
 */
	class Jsonifier_Dll Field : public std::pair<RawJsonString, Value> {
	  public:
		/**
   * Create a new invalid Field.
   *
   * Exists so you can declare a variable and later assign to it before use.
   */
		inline Field() noexcept;

		/**
   * Get the key as a string_view (for higher speed, consider raw_key).
   * We deliberately use a more cumbersome name (unescaped_key) to force users
   * to think twice about using it.
   *
   * This consumes the key: once you have called unescaped_key(), you cannot
   * call it again nor can you call key().
   */
		inline JsonifierResult<std::string_view> unescaped_key() noexcept;
		/**
   * Get the key as a RawJsonString. Can be used for direct comparison with
   * an unescaped C string: e.g., key() == "test".
   */
		inline RawJsonString key() const noexcept;
		/**
   * Get the Field Value.
   */
		inline Value& value() & noexcept;
		/**
   * @overload Value &value() & noexcept
   */
		inline Value value() && noexcept;

	  protected:
		inline Field(RawJsonString key, Value&& value) noexcept;
		static inline JsonifierResult<Field> start(ValueIterator& parent_iter) noexcept;
		static inline JsonifierResult<Field> start(const ValueIterator& parent_iter, RawJsonString key) noexcept;
		friend struct JsonifierResult<Field>;
		friend class ObjectIterator;
	};

	template<> struct JsonifierResult<Field> : public ImplementationJsonifierResultBase<Field> {
	  public:
		inline JsonifierResult(Field&& Value) noexcept;///< @private
		inline JsonifierResult(ErrorCode error) noexcept;///< @private
		inline JsonifierResult() noexcept = default;

		inline JsonifierResult<std::string_view> unescaped_key() noexcept;
		inline JsonifierResult<RawJsonString> key() noexcept;
		inline JsonifierResult<Value> value() noexcept;
	};

}
