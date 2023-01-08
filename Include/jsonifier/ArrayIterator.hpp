#pragma once

#include <jsonifier/Base.hpp>
#include <jsonifier/Value.hpp>

namespace Jsonifier {

	class Array;
	class Document;
	class Value;
	/**
 * A forward-only JSON Array.
 *
 * This is an input_iterator, meaning:
 * - It is forward-only
 * - * must be called exactly once per element.
 * - ++ must be called exactly once in between each * (*, ++, *, ++, * ...)
 */
	class Jsonifier_Dll ArrayIterator {
	  public:
		/** Create a new, invalid Array iterator. */
		inline ArrayIterator() noexcept = default;

		//
		// Iterator interface
		//

		/**
   * Get the current element.
   *
   * Part of the std::iterator interface.
   */
		inline JsonifierResult<Value> operator*() noexcept;// MUST ONLY BE CALLED ONCE PER ITERATION.
		/**
   * Check if we are at the end of the JSON.
   *
   * Part of the std::iterator interface.
   *
   * @return true if there are no more elements in the JSON Array.
   */
		inline bool operator==(const ArrayIterator&) const noexcept;
		/**
   * Check if there are more elements in the JSON Array.
   *
   * Part of the std::iterator interface.
   *
   * @return true if there are more elements in the JSON Array.
   */
		inline bool operator!=(const ArrayIterator&) const noexcept;
		/**
   * Move to the next element.
   *
   * Part of the std::iterator interface.
   */
		inline ArrayIterator& operator++() noexcept;

	  private:
		ValueIterator iterator{};

		inline ArrayIterator(const ValueIterator& iterator) noexcept;

		friend class Array;
		friend class Value;
		friend struct JsonifierResult<ArrayIterator>;
	};

	template<> struct JsonifierResult<ArrayIterator> : public ImplementationJsonifierResultBase<ArrayIterator> {
	  public:
		inline JsonifierResult(ArrayIterator&& Value) noexcept;///< @private
		inline JsonifierResult(ErrorCode error) noexcept;///< @private
		inline JsonifierResult() noexcept = default;

		//
		// Iterator interface
		//

		inline JsonifierResult<Value> operator*() noexcept;// MUST ONLY BE CALLED ONCE PER ITERATION.
		inline bool operator==(const JsonifierResult<ArrayIterator>&) const noexcept;
		inline bool operator!=(const JsonifierResult<ArrayIterator>&) const noexcept;
		inline JsonifierResult<ArrayIterator>& operator++() noexcept;
	};

}
