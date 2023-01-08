#pragma once

#include <jsonifier/Base.hpp>
#include <jsonifier/ObjectIterator.hpp>
#include <jsonifier/Value.hpp>

namespace Jsonifier {

	class Value;

	class Jsonifier_Dll Object : public ValueIterator {
	  public:
		inline Object() noexcept = default;
		inline JsonifierResult<ObjectIterator> begin() noexcept;
		inline JsonifierResult<ObjectIterator> end() noexcept;
		inline JsonifierResult<Value> findField(std::string_view key) & noexcept;
		inline JsonifierResult<Value> findField(std::string_view key) && noexcept;
		inline JsonifierResult<Value> findFieldUnordered(std::string_view key) & noexcept;
		inline JsonifierResult<Value> findFieldUnordered(std::string_view key) && noexcept;
		inline JsonifierResult<Value> operator[](std::string_view key) & noexcept;
		inline JsonifierResult<Value> operator[](std::string_view key) && noexcept;
		inline JsonifierResult<Value> atPointer(std::string_view jsonPointer) noexcept;
		inline JsonifierResult<bool> reset() & noexcept;
		inline JsonifierResult<bool> isEmpty() & noexcept;
		inline JsonifierResult<size_t> countFields() & noexcept;
		inline JsonifierResult<std::string_view> rawJson() noexcept;

	  protected:
		inline ErrorCode consume() noexcept;
		static inline JsonifierResult<Object> start(ValueIterator& iterator) noexcept;
		static inline JsonifierResult<Object> startRoot(ValueIterator& iterator) noexcept;
		static inline JsonifierResult<Object> started(ValueIterator& iterator) noexcept;
		static inline Object resume(const ValueIterator& iterator) noexcept;
		inline Object(const ValueIterator& iterator) noexcept;
		inline ErrorCode findFieldRaw(const std::string_view key) noexcept;

		friend class Value;
		friend class Document;
		friend struct JsonifierResult<Object>;
	};

	template<> struct JsonifierResult<Object> : public ImplementationJsonifierResultBase<Object> {
	  public:
		inline JsonifierResult(Object&& Value) noexcept;
		inline JsonifierResult(ErrorCode error) noexcept;
		inline JsonifierResult() noexcept = default;

		inline JsonifierResult<ObjectIterator> begin() noexcept;
		inline JsonifierResult<ObjectIterator> end() noexcept;
		inline JsonifierResult<Value> findField(std::string_view key) & noexcept;
		inline JsonifierResult<Value> findField(std::string_view key) && noexcept;
		inline JsonifierResult<Value> findFieldUnordered(std::string_view key) & noexcept;
		inline JsonifierResult<Value> findFieldUnordered(std::string_view key) && noexcept;
		inline JsonifierResult<Value> operator[](std::string_view key) & noexcept;
		inline JsonifierResult<Value> operator[](std::string_view key) && noexcept;
		inline JsonifierResult<Value> atPointer(std::string_view jsonPointer) noexcept;
		inline JsonifierResult<bool> reset() noexcept;
		inline JsonifierResult<bool> isEmpty() noexcept;
		inline JsonifierResult<size_t> countFields() & noexcept;
	};

}
