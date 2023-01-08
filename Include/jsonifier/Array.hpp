#pragma once

#include <jsonifier/Base.hpp>
#include <jsonifier/ArrayIterator.hpp>

namespace Jsonifier {
	class Value;
	class document;

	class Jsonifier_Dll Array {
	  public:
		inline Array() noexcept = default;
		inline JsonifierResult<ArrayIterator> begin() noexcept;
		inline JsonifierResult<ArrayIterator> end() noexcept;
		inline JsonifierResult<size_t> countElements() & noexcept;
		inline JsonifierResult<bool> is_empty() & noexcept;
		inline JsonifierResult<bool> reset() & noexcept;
		inline JsonifierResult<Value> atPointer(std::string_view json_pointer) noexcept;
		inline JsonifierResult<std::string_view> raw_json() noexcept;
		inline JsonifierResult<Value> at(size_t index) noexcept;

	  protected:
		inline ErrorCode consume() noexcept;
		static inline JsonifierResult<Array> start(ValueIterator& iterator) noexcept;
		static inline JsonifierResult<Array> start_root(ValueIterator& iterator) noexcept;
		static inline JsonifierResult<Array> started(ValueIterator& iterator) noexcept;
		inline Array(const ValueIterator& iterator) noexcept;
		ValueIterator iterator{};

		friend class Value;
		friend class Document;
		friend struct JsonifierResult<Value>;
		friend struct JsonifierResult<Array>;
		friend class ArrayIterator;
	};



	template<> struct JsonifierResult<Array> : public ImplementationJsonifierResultBase<Array> {
	  public:
		inline JsonifierResult(Array&& Value) noexcept;///< @private
		inline JsonifierResult(ErrorCode error) noexcept;///< @private
		inline JsonifierResult() noexcept = default;

		inline JsonifierResult<ArrayIterator> begin() noexcept;
		inline JsonifierResult<ArrayIterator> end() noexcept;
		inline JsonifierResult<size_t> countElements() & noexcept;
		inline JsonifierResult<bool> is_empty() & noexcept;
		inline JsonifierResult<bool> reset() & noexcept;
		inline JsonifierResult<Value> at(size_t index) noexcept;
		inline JsonifierResult<Value> atPointer(std::string_view json_pointer) noexcept;
	};

}
