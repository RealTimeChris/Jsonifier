#pragma once

#include <jsonifier/FoundationEntities.hpp>
#include <jsonifier/Field.hpp>

namespace Jsonifier {

	class Value;

	class Jsonifier_Dll ObjectIterator :public ValueIterator{
	  public:
		ObjectIterator() noexcept = default;
		JsonifierResult<Field> operator*() noexcept;
		bool operator==(const ObjectIterator&) const noexcept;
		bool operator!=(const ObjectIterator&) const noexcept;
		ObjectIterator& operator++() noexcept;

	  protected:
		ObjectIterator(const ValueIterator& iteratorNew) noexcept;
		friend struct JsonifierResult<ObjectIterator>;
		friend class Object;
	};

	class Jsonifier_Dll Object {
	  public:
		Object() noexcept = default;

		JsonifierResult<ObjectIterator> begin() noexcept;
		JsonifierResult<ObjectIterator> end() noexcept;
		JsonifierResult<Value> findField(std::string_view key) & noexcept;
		JsonifierResult<Value> findField(std::string_view key) && noexcept;
		JsonifierResult<Value> findFieldUnordered(std::string_view key) & noexcept;
		JsonifierResult<Value> findFieldUnordered(std::string_view key) && noexcept;
		JsonifierResult<Value> operator[](std::string_view key) & noexcept;
		JsonifierResult<Value> operator[](std::string_view key) && noexcept;
		JsonifierResult<Value> atPointer(std::string_view jsonPointer) noexcept;
		JsonifierResult<bool> reset() & noexcept;
		JsonifierResult<bool> isEmpty() & noexcept;
		JsonifierResult<size_t> countFields() & noexcept;
		JsonifierResult<std::string_view> rawJson() noexcept;

	  protected:
		ErrorCode consume() noexcept;
		static JsonifierResult<Object> start(ValueIterator& iteratorNew) noexcept;
		static JsonifierResult<Object> startRoot(ValueIterator& iteratorNew) noexcept;
		static JsonifierResult<Object> started(ValueIterator& iteratorNew) noexcept;
		static Object resume(const ValueIterator& iteratorNew) noexcept;
		Object(const ValueIterator& iteratorNew) noexcept;

		ErrorCode findFieldRaw(const std::string_view key) noexcept;

		ValueIterator iteratorNew{};

		friend class Value;
		friend class Document;
		friend struct JsonifierResult<Object>;

		ValueIterator iterator{};

		friend class Value;
		friend class Document;
		friend struct JsonifierResult<Object>;
	};

	template<> struct JsonifierResult<ObjectIterator> : public ImplementationJsonifierResultBase<ObjectIterator> {
	  public:
		inline JsonifierResult(ObjectIterator&& Value) noexcept;
		inline JsonifierResult(ErrorCode error) noexcept;
		inline JsonifierResult() noexcept = default;
		inline JsonifierResult<Field> operator*() noexcept;
		inline bool operator==(const JsonifierResult<ObjectIterator>&) const noexcept;
		inline bool operator!=(const JsonifierResult<ObjectIterator>&) const noexcept;
		inline JsonifierResult<ObjectIterator>& operator++() noexcept;
	};

	template<> struct JsonifierResult<Object> : public ImplementationJsonifierResultBase<Object> {
	  public:
		inline JsonifierResult() noexcept = default;
		inline JsonifierResult(Object&& Value) noexcept;
		inline JsonifierResult(ErrorCode error) noexcept;

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