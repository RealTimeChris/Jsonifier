#pragma once

#include <jsonifier/Array.hpp>
#include <jsonifier/Value.hpp>
#include <jsonifier/Value_Impl.hpp>

namespace Jsonifier {

	__forceinline Array::Array(const ValueIterator& iteratorNew) noexcept : iterator{ iteratorNew } {};

	__forceinline JsonifierResult<Array> Array::start(ValueIterator& iterator) noexcept {
		bool hasValue{};
		JsonifierTry(iterator.startArray().get(hasValue));
		return Array(iterator);
	}

	__forceinline JsonifierResult<Array> Array::startRoot(ValueIterator& iterator) noexcept {
		bool hasValue{};
		JsonifierTry(iterator.startRootArray().get(hasValue));
		return Array(iterator);
	}

	__forceinline JsonifierResult<Array> Array::started(ValueIterator& iterator) noexcept {
		bool hasValue{};
		JsonifierTry(iterator.startedArray().get(hasValue));
		return Array(iterator);
	}

	__forceinline JsonifierResult<ArrayIterator> Array::begin() noexcept {
		return ArrayIterator(this->iterator);
	}

	__forceinline JsonifierResult<ArrayIterator> Array::end() noexcept {
		return ArrayIterator(this->iterator);
	}

	__forceinline ErrorCode Array::consume() noexcept {
		auto error = iterator.jsonIter().skipChild(iterator.depth() - 1);
		if (error) {
			iterator.abandon();
		}
		return error;
	}

	__forceinline JsonifierResult<std::string_view> Array::rawJson() noexcept {
		const uint8_t* startingPoint{ iterator.peekStart() };
		auto error = consume();
		if (error) {
			return error;
		}
		const uint8_t* finalPoint{ iterator.jsonIterator->unsafePointer() };
		return std::string_view(reinterpret_cast<const char*>(startingPoint), size_t(finalPoint - startingPoint));
	}

	__forceinline JsonifierResult<size_t> Array::countElements() & noexcept {
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

	__forceinline JsonifierResult<bool> Array::isEmpty() & noexcept {
		bool isNotEmpty{};
		auto error = iterator.resetArray().get(isNotEmpty);
		if (error) {
			return error;
		}
		return !isNotEmpty;
	}

	__forceinline JsonifierResult<bool> Array::reset() & noexcept {
		return iterator.resetArray();
	}

	__forceinline JsonifierResult<Value> Array::atPointer(std::string_view jsonPointer) noexcept {
		if (jsonPointer[0] != '/') {
			return Invalid_Json_Pointer;
		}
		jsonPointer = jsonPointer.substr(1);
		if (jsonPointer == "-") {
			return Out_Of_Bounds;
		}

		size_t arrayIndex = 0;
		size_t i;
		for (i = 0; i < jsonPointer.length() && jsonPointer[i] != '/'; i++) {
			uint8_t digit = uint8_t(jsonPointer[i] - '0');
			if (digit > 9) {
				return Incorrect_Type;
			}
			arrayIndex = arrayIndex * 10 + digit;
		}

		if (i > 1 && jsonPointer[0] == '0') {
			return Invalid_Json_Pointer;
		}

		if (i == 0) {
			return Invalid_Json_Pointer;
		}
		auto child = at(arrayIndex);
		if (child.error()) {
			return child;
		}

		if (i < jsonPointer.length()) {
			child = child.atPointer(jsonPointer.substr(i));
		}
		return child;
	}

	__forceinline JsonifierResult<Value> Array::at(size_t index) noexcept {
		size_t i = 0;
		for (auto Value: *this) {
			if (i == index) {
				return Value;
			}
			i++;
		}
		return Out_Of_Bounds;
	}

	__forceinline JsonifierResult<Array>::JsonifierResult(Array&& Value) noexcept : JsonifierResultBase<Array>(std::forward<Array>(Value)){};

	__forceinline JsonifierResult<Array>::JsonifierResult(ErrorCode error) noexcept : JsonifierResultBase<Array>(error){};

	__forceinline JsonifierResult<ArrayIterator> JsonifierResult<Array>::begin() noexcept {
		if (error()) {
			return error();
		}
		return first.begin();
	}

	__forceinline JsonifierResult<ArrayIterator> JsonifierResult<Array>::end() noexcept {
		if (error()) {
			return error();
		}
		return first.end();
	}

	__forceinline JsonifierResult<size_t> JsonifierResult<Array>::countElements() & noexcept {
		if (error()) {
			return error();
		}
		return first.countElements();
	}

	__forceinline JsonifierResult<bool> JsonifierResult<Array>::isEmpty() & noexcept {
		if (error()) {
			return error();
		}
		return first.isEmpty();
	}

	__forceinline JsonifierResult<Value> JsonifierResult<Array>::at(size_t index) noexcept {
		if (error()) {
			return error();
		}
		return first.at(index);
	}

	__forceinline JsonifierResult<Value> JsonifierResult<Array>::atPointer(std::string_view jsonPointer) noexcept {
		if (error()) {
			return error();
		}
		return first.atPointer(jsonPointer);
	}
}
