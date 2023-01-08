
#pragma once

#include <jsonifier/Base.hpp>
namespace Jsonifier {
	class Object;
	class parser;
	class JsonIterator;

	/**
 * A string escaped per JSON rules, terminated with quote ("). They are used to represent
 * unescaped keys inside JSON Documents.
 *
 * (In other words, a pointer to the beginning of a string, just after the start quote, inside a
 * JSON file.)
 *
 * This class is deliberately simplistic and has little functionality. You can
 * compare a RawJsonString instance with an unescaped C string, but
 * that is nearly all you can do.
 *
 * The RawJsonString is unescaped. If you wish to write an unescaped version of it to your own
 * buffer, you may do so using the parser.unescape(string, buff) method, using an Parser
 * instance. Doing so requires you to have a sufficiently large buffer.
 *
 * The RawJsonString instances originate typically from Field instance which in turn represent
 * key-Value pairs from Object instances. From a Field instance, you get the RawJsonString
 * instance by calling key(). You can, if you want a more usable string_view instance, call
 * the unescaped_key() method on the Field instance. You may also create a RawJsonString from
 * any other string Value, with the Value.getRawJsonString() method. Again, you can get
 * a more usable string_view instance by calling getString().
 *
 */
	class Jsonifier_Dll RawJsonString {
	  public:
		/**
   * Create a new invalid RawJsonString.
   *
   * Exists so you can declare a variable and later assign to it before use.
   */
		inline RawJsonString() noexcept = default;

		/**
   * Create a new invalid RawJsonString pointed at the given location in the JSON.
   *
   * The given location must be just *after* the beginning quote (") in the JSON file.
   *
   * It *must* be terminated by a ", and be a valid JSON string.
   */
		inline RawJsonString(const uint8_t* _buf) noexcept;
		/**
   * Get the raw pointer to the beginning of the string in the JSON (just after the ").
   *
   * It is possible for this function to return a null pointer if the instance
   * has outlived its existence.
   */
		inline const char* raw() const noexcept;

		/**
   * This compares the current instance to the std::string_view target: returns true if
   * they are byte-by-byte equal (no escaping is done) on target.size() characters,
   * and if the RawJsonString instance has a quote character at byte index target.size().
   * We never read more than length + 1 bytes in the RawJsonString instance.
   * If length is smaller than target.size(), this will return false.
   *
   * The std::string_view instance may contain any characters. However, the caller
   * is responsible for setting length so that length bytes may be read in the
   * RawJsonString.
   *
   * Performance: the comparison may be done using memcmp which may be efficient
   * for long strings.
   */
		inline bool unsafe_is_equal(size_t length, std::string_view target) const noexcept;

		/**
   * This compares the current instance to the std::string_view target: returns true if
   * they are byte-by-byte equal (no escaping is done).
   * The std::string_view instance should not contain unescaped quote characters:
   * the caller is responsible for this check. See is_free_from_unescaped_quote.
   *
   * Performance: the comparison is done byte-by-byte which might be inefficient for
   * long strings.
   *
   * If target is a compile-time constant, and your compiler likes you,
   * you should be able to do the following without performance penalty...
   *
   *   static_assert(RawJsonString::is_free_from_unescaped_quote(target), "");
   *   s.unsafe_is_equal(target);
   */
		inline bool unsafe_is_equal(std::string_view target) const noexcept;

		/**
   * This compares the current instance to the C string target: returns true if
   * they are byte-by-byte equal (no escaping is done).
   * The provided C string should not contain an unescaped quote character:
   * the caller is responsible for this check. See is_free_from_unescaped_quote.
   *
   * If target is a compile-time constant, and your compiler likes you,
   * you should be able to do the following without performance penalty...
   *
   *   static_assert(RawJsonString::is_free_from_unescaped_quote(target), "");
   *   s.unsafe_is_equal(target);
   */
		inline bool unsafe_is_equal(const char* target) const noexcept;

		/**
   * This compares the current instance to the std::string_view target: returns true if
   * they are byte-by-byte equal (no escaping is done).
   */
		inline bool is_equal(std::string_view target) const noexcept;

		/**
   * This compares the current instance to the C string target: returns true if
   * they are byte-by-byte equal (no escaping is done).
   */
		inline bool is_equal(const char* target) const noexcept;

		/**
   * Returns true if target is free from unescaped quote. If target is known at
   * compile-time, we might expect the computation to happen at compile time with
   * many compilers (not all!).
   */
		static inline bool is_free_from_unescaped_quote(std::string_view target) noexcept;
		static inline bool is_free_from_unescaped_quote(const char* target) noexcept;

	  private:
		/**
   * This will set the inner pointer to zero, effectively making
   * this instance unusable.
   */
		inline void consume() noexcept {
			stringView = nullptr;
		}

		/**
   * Checks whether the inner pointer is non-null and thus usable.
   */
		inline bool alive() const noexcept {
			return stringView != nullptr;
		}

		/**
   * Unescape this JSON string, replacing \\ with \, \n with newline, etc.
   *
   * ## IMPORTANT: string_view lifetime
   *
   * The string_view is only valid until the next parse() call on the parser.
   *
   * @param iterator A JsonIterator, which contains a buffer where the string will be written.
   */
		inline JsonifierResult<std::string_view> unescape(JsonIterator& iterator) const noexcept;

		const uint8_t* stringView{};
		friend class Object;
		friend class Field;
		friend class Parser;
		friend struct JsonifierResult<RawJsonString>;
		friend class JsonIterator;
	};

	inline std::ostream& operator<<(std::ostream&, const RawJsonString&) noexcept;

	/**
 * Comparisons between RawJsonString and std::string_view instances are potentially unsafe: the user is responsible
 * for providing a string with no unescaped quote. Note that unescaped quotes cannot be present in valid JSON strings.
 */
	inline bool operator==(const RawJsonString& a, std::string_view c) noexcept;
	inline bool operator==(std::string_view c, const RawJsonString& a) noexcept;
	inline bool operator!=(const RawJsonString& a, std::string_view c) noexcept;
	inline bool operator!=(std::string_view c, const RawJsonString& a) noexcept;

	template<> struct JsonifierResult<RawJsonString> : public ImplementationJsonifierResultBase<RawJsonString> {
	  public:
		inline JsonifierResult(RawJsonString&& Value) noexcept;///< @private
		inline JsonifierResult(ErrorCode error) noexcept;///< @private
		inline JsonifierResult() noexcept = default;
		inline ~JsonifierResult() noexcept = default;///< @private

		inline JsonifierResult<const char*> raw() const noexcept;
		inline JsonifierResult<std::string_view> unescape(JsonIterator& iterator) const noexcept;
	};

}
