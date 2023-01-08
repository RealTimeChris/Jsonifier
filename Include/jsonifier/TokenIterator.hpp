#pragma once

#include <jsonifier/Base.hpp>
namespace Jsonifier {
	/**
 * Iterates through JSON tokens (`{` `}` `[` `]` `,` `:` `"<string>"` `123` `true` `false` `null`)
 * detected by stage 1.
 *
 * @private This is not intended for external use.
 */
	class Jsonifier_Dll TokenIterator {
	  public:
		/**
   * Create a new invalid TokenIterator.
   *
   * Exists so you can declare a variable and later assign to it before use.
   */
		inline TokenIterator() noexcept = default;
		inline TokenIterator(TokenIterator&& other) noexcept = default;
		inline TokenIterator& operator=(TokenIterator&& other) noexcept = default;
		inline TokenIterator(const TokenIterator& other) noexcept = default;
		inline TokenIterator& operator=(const TokenIterator& other) noexcept = default;

		/**
   * Advance to the next token (returning the current one).
   */
		inline const uint8_t* returnCurrentAndAdvance() noexcept;
		/**
   * Reports the current offset in bytes from the start of the underlying buffer.
   */
		inline uint32_t current_offset() const noexcept;
		/**
   * Get the JSON text for a given token (relative).
   *
   * This is not null-terminated; it is a view into the JSON.
   *
   * @param delta The relative position of the token to retrieve. e.g. 0 = current token,
   *              1 = next token, -1 = prev token.
   *
   * TODO consider a string_view, assuming the length will get stripped out by the optimizer when
   * it isn't used ...
   */
		inline const uint8_t* peek(int32_t delta = 0) const noexcept;
		/**
   * Get the maximum length of the JSON text for a given token.
   *
   * The length will include any whitespace at the end of the token.
   *
   * @param delta The relative position of the token to retrieve. e.g. 0 = current token,
   *              1 = next token, -1 = prev token.
   */
		inline uint32_t peekLength(int32_t delta = 0) const noexcept;

		/**
   * Get the JSON text for a given token.
   *
   * This is not null-terminated; it is a view into the JSON.
   *
   * @param position The position of the token.
   *
   */
		inline const uint8_t* peek(uint32_t* position) const noexcept;
		/**
   * Get the maximum length of the JSON text for a given token.
   *
   * The length will include any whitespace at the end of the token.
   *
   * @param position The position of the token.
   */
		inline uint32_t peekLength(uint32_t* position) const noexcept;

		/**
   * Return the current index.
   */
		inline uint32_t* position() const noexcept;
		/**
   * Reset to a previously saved index.
   */
		inline void set_position(uint32_t* target_position) noexcept;

		// NOTE: we don't support a full C++ iterator interface, because we expect people to make
		// different calls to advance the iterator based on *their own* state.

		inline bool operator==(const TokenIterator& other) const noexcept;
		inline bool operator!=(const TokenIterator& other) const noexcept;
		inline bool operator>(const TokenIterator& other) const noexcept;
		inline bool operator>=(const TokenIterator& other) const noexcept;
		inline bool operator<(const TokenIterator& other) const noexcept;
		inline bool operator<=(const TokenIterator& other) const noexcept;

	  protected:
		inline TokenIterator(const uint8_t* buf, uint32_t* position) noexcept;

		/**
   * Get the index of the JSON text for a given token (relative).
   *
   * This is not null-terminated; it is a view into the JSON.
   *
   * @param delta The relative position of the token to retrieve. e.g. 0 = current token,
   *              1 = next token, -1 = prev token.
   */
		inline uint32_t peekIndex(int32_t delta = 0) const noexcept;
		/**
   * Get the index of the JSON text for a given token.
   *
   * This is not null-terminated; it is a view into the JSON.
   *
   * @param position The position of the token.
   *
   */
		inline uint32_t peekIndex(uint32_t* position) const noexcept;

		const uint8_t* stringView{};
		uint32_t* currentPosition{};

		friend class JsonIterator;
		friend class ValueIterator;
		friend class Object;
	};

	template<> struct JsonifierResult<TokenIterator> : public ImplementationJsonifierResultBase<TokenIterator> {
	  public:
		inline JsonifierResult(TokenIterator&& Value) noexcept;///< @private
		inline JsonifierResult(ErrorCode error) noexcept;///< @private
		inline JsonifierResult() noexcept = default;
		inline ~JsonifierResult() noexcept = default;///< @private
	};

}
