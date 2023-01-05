#pragma once

#include "FoundationEntities.hpp"
#include "Field.hpp"

namespace Jsonifier {

	class Jsonifier_Dll ObjectIterator {
	  public:
		inline ObjectIterator() noexcept = default;
		inline JsonifierResult<Field> operator*() noexcept;
		inline bool operator==(const ObjectIterator&) const noexcept;
		inline bool operator!=(const ObjectIterator&) const noexcept;
		inline ObjectIterator& operator++() noexcept;

	  protected:
		ValueIterator iterator{};

		inline ObjectIterator(const ValueIterator& iter) noexcept;
		friend struct Jsonifier_Dll JsonifierResult<ObjectIterator>;
		friend class Jsonifier_Dll Object;
	};

	class Jsonifier_Dll Object {
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
		inline JsonifierResult<Value> atPointer(std::string_view json_pointer) noexcept;
		inline JsonifierResult<bool> reset() & noexcept;
		inline JsonifierResult<bool>isEmpty() & noexcept;
		inline JsonifierResult<size_t> countFields() & noexcept;
		inline JsonifierResult<std::string_view> rawJson() noexcept;

	  protected:
		inline ErrorCode consume() noexcept;
		static inline JsonifierResult<Object> start(ValueIterator& iter) noexcept;
		static inline JsonifierResult<Object> startRoot(ValueIterator& iter) noexcept;
		static inline JsonifierResult<Object> started(ValueIterator& iter) noexcept;
		static inline Object resume(const ValueIterator& iter) noexcept;
		inline Object(const ValueIterator& iter) noexcept;

		inline ErrorCode findFieldRaw(const std::string_view key) noexcept;

		ValueIterator iter{};

		friend class Jsonifier_Dll Value;
		friend class Jsonifier_Dll Document;
		friend struct Jsonifier_Dll JsonifierResult<Object>;

		ValueIterator iterator;

		friend class Jsonifier_Dll Value;
		friend class Jsonifier_Dll Document;
		friend struct Jsonifier_Dll JsonifierResult<Object>;
	};

	template<> struct JsonifierResult<ObjectIterator> : public JsonifierResultBase<ObjectIterator> {
	  public:
		inline JsonifierResult(ObjectIterator&& Value) noexcept;
		inline JsonifierResult(ErrorCode error) noexcept;
		inline JsonifierResult() noexcept = default;
		inline JsonifierResult<Field> operator*() noexcept;
		inline bool operator==(const JsonifierResult<ObjectIterator>&) const noexcept;
		inline bool operator!=(const JsonifierResult<ObjectIterator>&) const noexcept;
		inline JsonifierResult<ObjectIterator>& operator++() noexcept;
	};

	template<> struct JsonifierResult<Object> : public JsonifierResultBase<Object> {
	  public:
		JsonifierResult() noexcept = default;
		JsonifierResult(Object&& Value) noexcept;
		JsonifierResult(ErrorCode error) noexcept;

		JsonifierResult<ObjectIterator> begin() noexcept;
		JsonifierResult<ObjectIterator> end() noexcept;
		JsonifierResult<Value> findField(std::string_view key) &noexcept;
		JsonifierResult<Value> findField(std::string_view key) &&noexcept;
		JsonifierResult<Value> findFieldUnordered(std::string_view key) &noexcept;
		JsonifierResult<Value> findFieldUnordered(std::string_view key) && noexcept;
		JsonifierResult<Value> operator[](std::string_view key) & noexcept;
		JsonifierResult<Value> operator[](std::string_view key) && noexcept;

		JsonifierResult<Value> atPointer(std::string_view jsonPointer) noexcept;
		inline JsonifierResult<bool> reset() noexcept;
		inline JsonifierResult<bool> isEmpty() noexcept;
		inline JsonifierResult<size_t> countFields() noexcept;
	};
}