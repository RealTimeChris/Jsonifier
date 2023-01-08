#pragma once

#include <jsonifier/Array.hpp>

namespace Jsonifier {

	inline Array::Array(const ValueIterator& _iter) noexcept : iterator{ _iter } {};

	inline JsonifierResult<Array> Array::start(ValueIterator& iterator) noexcept {
		bool hasValue;
		JsonifierTry(iterator.startArray().get(hasValue));
		return Array(iterator);
	}

	inline JsonifierResult<Array> Array::startRoot(ValueIterator& iterator) noexcept {
		bool hasValue;
		JsonifierTry(iterator.startRootArray().get(hasValue));
		return Array(iterator);
	}

	inline JsonifierResult<Array> Array::started(ValueIterator& iterator) noexcept {
		bool hasValue;
		JsonifierTry(iterator.startedArray().get(hasValue));
		return Array(iterator);
	}

	inline JsonifierResult<ArrayIterator> Array::begin() noexcept {
		return ArrayIterator(iterator);
	}

	inline JsonifierResult<ArrayIterator> Array::end() noexcept {
		return ArrayIterator(iterator);
	}

	inline ErrorCode Array::consume() noexcept {
		auto error = iterator.jsonIter().skipChild(iterator.depth() - 1);
		if (error) {
			iterator.abandon();
		}
		return error;
	}

	inline JsonifierResult<std::string_view> Array::rawJson() noexcept {
		const uint8_t* startingPoint{ iterator.peekStart() };
		auto error = consume();
		if (error) {
			return error;
		}
		const uint8_t* finalPoint{ iterator.jsonIterator->unsafePointer() };
		return std::string_view(reinterpret_cast<const char*>(startingPoint), size_t(finalPoint - startingPoint));
	}

	inline JsonifierResult<size_t> Array::countElements() & noexcept {
		size_t count{ 0 };
		for (auto v: *this) {
			count++;
		}
		if (iterator.error()) {
			return iterator.error();
		}
		iterator.resetArray();
		return count;
	}

	inline JsonifierResult<bool> Array::isEmpty() & noexcept {
		bool is_not_empty;
		auto error = iterator.resetArray().get(is_not_empty);
		if (error) {
			return error;
		}
		return !is_not_empty;
	}

	inline JsonifierResult<bool> Array::reset() & noexcept {
		return iterator.resetArray();
	}

	inline JsonifierResult<Value> Array::atPointer(std::string_view json_pointer) noexcept {
		if (json_pointer[0] != '/') {
			return Invalid_Json_Pointer;
		}
		json_pointer = json_pointer.substr(1);
		if (json_pointer == "-") {
			return Out_Of_Bounds;
		}
		size_t arrayIndex = 0;
		size_t i;
		for (i = 0; i < json_pointer.length() && json_pointer[i] != '/'; i++) {
			uint8_t digit = uint8_t(json_pointer[i] - '0');
			if (digit > 9) {
				return Incorrect_Type;
			}
			arrayIndex = arrayIndex * 10 + digit;
		}

		if (i > 1 && json_pointer[0] == '0') {
			return Invalid_Json_Pointer;
		}

		if (i == 0) {
			return Invalid_Json_Pointer;
		}
		auto child = at(arrayIndex);
		if (child.error()) {
			return child;
		}

		if (i < json_pointer.length()) {
			child = child.atPointer(json_pointer.substr(i));
		}
		return child;
	}

	inline JsonifierResult<Value> Array::at(size_t index) noexcept {
		size_t i = 0;
		for (auto Value: *this) {
			if (i == index) {
				return Value;
			}
			i++;
		}
		return Out_Of_Bounds;
	}

	inline JsonifierResult<Array>::JsonifierResult(Array&& Value) noexcept : ImplementationJsonifierResultBase<Array>(std::forward<Array>(Value)){}

	inline JsonifierResult<Array>::JsonifierResult(ErrorCode error) noexcept : ImplementationJsonifierResultBase<Array>(error){}

	inline JsonifierResult<ArrayIterator> JsonifierResult<Array>::begin() noexcept {
		if (error()) {
			return error();
		}
		return first.begin();
	}

	inline JsonifierResult<ArrayIterator> JsonifierResult<Array>::end() noexcept {
		if (error()) {
			return error();
		}
		return first.end();
	}

	inline JsonifierResult<size_t> JsonifierResult<Array>::countElements() & noexcept {
		if (error()) {
			return error();
		}
		return first.countElements();
	}

	inline JsonifierResult<bool> JsonifierResult<Array>::isEmpty() & noexcept {
		if (error()) {
			return error();
		}
		return first.isEmpty();
	}

	inline JsonifierResult<Value> JsonifierResult<Array>::at(size_t index) noexcept {
		if (error()) {
			return error();
		}
		return first.at(index);
	}

	inline JsonifierResult<Value> JsonifierResult<Array>::atPointer(std::string_view json_pointer) noexcept {
		if (error()) {
			return error();
		}
		return first.atPointer(json_pointer);
	}
}
