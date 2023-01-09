#pragma once

#include <jsonifier/Base.hpp>
#include <jsonifier/ObjectIterator.hpp>
#include <jsonifier/Value.hpp>

namespace Jsonifier {

	class Value;

	class Jsonifier_Dll Object : public ValueIterator {
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
		static JsonifierResult<Object> start(ValueIterator& iterator) noexcept;
		static JsonifierResult<Object> startRoot(ValueIterator& iterator) noexcept;
		static JsonifierResult<Object> started(ValueIterator& iterator) noexcept;
		static Object resume(const ValueIterator& iterator) noexcept;
		Object(const ValueIterator& iterator) noexcept;
		ErrorCode findFieldRaw(const std::string_view key) noexcept;

		friend class Value;
		friend class Document;
		friend struct JsonifierResult<Object>;
	};

	template<> struct JsonifierResult<Object> : public JsonifierResultBase<Object> {
	  public:
		JsonifierResult(Object&& Value) noexcept;
		JsonifierResult(ErrorCode error) noexcept;
		JsonifierResult() noexcept = default;

		JsonifierResult<ObjectIterator> begin() noexcept;
		JsonifierResult<ObjectIterator> end() noexcept;
		JsonifierResult<Value> findField(std::string_view key) & noexcept;
		JsonifierResult<Value> findField(std::string_view key) && noexcept;
		JsonifierResult<Value> findFieldUnordered(std::string_view key) & noexcept;
		JsonifierResult<Value> findFieldUnordered(std::string_view key) && noexcept;
		JsonifierResult<Value> operator[](std::string_view key) & noexcept;
		JsonifierResult<Value> operator[](std::string_view key) && noexcept;
		JsonifierResult<Value> atPointer(std::string_view jsonPointer) noexcept;
		JsonifierResult<bool> reset() noexcept;
		JsonifierResult<bool> isEmpty() noexcept;
		JsonifierResult<size_t> countFields() & noexcept;
	};

}
