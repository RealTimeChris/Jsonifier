#pragma once

#include <jsonifier/Base.hpp>
#include <jsonifier/ValueIterator.hpp>

namespace Jsonifier {
	class Field;

	class Jsonifier_Dll ObjectIterator {
	  public:
		/**
   * Create a new invalid ObjectIterator.
   *
   * Exists so you can declare a variable and later assign to it before use.
   */
		inline ObjectIterator() noexcept = default;

		//
		// Iterator interface
		//

		// Reads key and Value, yielding them to the user.
		// MUST ONLY BE CALLED ONCE PER ITERATION.
		inline JsonifierResult<Field> operator*() noexcept;
		// Assumes it's being compared with the end. true if depth < iterator->depth.
		inline bool operator==(const ObjectIterator&) const noexcept;
		// Assumes it's being compared with the end. true if depth >= iterator->depth.
		inline bool operator!=(const ObjectIterator&) const noexcept;
		// Checks for ']' and ','
		inline ObjectIterator& operator++() noexcept;

	  private:
		/**
   * The underlying JSON iterator.
   *
   * PERF NOTE: expected to be elided in favor of the parent Document: this is set when the Object
   * is first used, and never changes afterwards.
   */
		ValueIterator iterator{};

		inline ObjectIterator(const ValueIterator& iterator) noexcept;
		friend struct JsonifierResult<ObjectIterator>;
		friend class Object;
	};

	template<> struct JsonifierResult<ObjectIterator> : public ImplementationJsonifierResultBase<ObjectIterator> {
	  public:
		inline JsonifierResult(ObjectIterator&& Value) noexcept;///< @private
		inline JsonifierResult(ErrorCode error) noexcept;///< @private
		inline JsonifierResult() noexcept = default;

		//
		// Iterator interface
		//

		// Reads key and Value, yielding them to the user.
		inline JsonifierResult<Field> operator*() noexcept;// MUST ONLY BE CALLED ONCE PER ITERATION.
		// Assumes it's being compared with the end. true if depth < iterator->depth.
		inline bool operator==(const JsonifierResult<ObjectIterator>&) const noexcept;
		// Assumes it's being compared with the end. true if depth >= iterator->depth.
		inline bool operator!=(const JsonifierResult<ObjectIterator>&) const noexcept;
		// Checks for ']' and ','
		inline JsonifierResult<ObjectIterator>& operator++() noexcept;
	};

}
