#pragma once

#include <jsonifier/Array.hpp>
#include <jsonifier/Value.hpp>

namespace Jsonifier {

	Array::Array(const ValueIterator& iteratorNew) noexcept : ValueIterator{ iteratorNew } {};

	JsonifierResult<Array> Array::start(ValueIterator& iterator) noexcept {
		bool hasValue{};
		JsonifierTry(iterator.startArray().get(hasValue));
		return Array(iterator);
	}

	JsonifierResult<Array> Array::startRoot(ValueIterator& iterator) noexcept {
		bool hasValue{};
		JsonifierTry(iterator.startRootArray().get(hasValue));
		return Array(iterator);
	}

	JsonifierResult<Array> Array::started(ValueIterator& iterator) noexcept {
		bool hasValue{};
		JsonifierTry(iterator.startedArray().get(hasValue));
		return Array(iterator);
	}

	JsonifierResult<ArrayIterator> Array::begin() noexcept {
		return ArrayIterator(*this);
	}

	JsonifierResult<ArrayIterator> Array::end() noexcept {
		return ArrayIterator(*this);
	}

	ErrorCode Array::consume() noexcept {
		auto error = ValueIterator::jsonIter().skipChild(ValueIterator::depth() - 1);
		if (error) {
			ValueIterator::abandon();
		}
		return error;
	}

	JsonifierResult<std::string_view> Array::rawJson() noexcept {
		const uint8_t* startingPoint{ ValueIterator::peekStart() };
		auto error = consume();
		if (error) {
			return error;
		}
		const uint8_t* finalPoint{ ValueIterator::jsonIterator->unsafePointer() };
		return std::string_view(reinterpret_cast<const char*>(startingPoint), size_t(finalPoint - startingPoint));
	}

	JsonifierResult<size_t> Array::countElements() & noexcept {
		size_t count{ 0 };
		for (auto v: *this) {
			count++;
		}
		if (ValueIterator::error()) {
			return ValueIterator::error();
		}
		ValueIterator::resetArray();
		return count;
	}

	JsonifierResult<bool> Array::isEmpty() & noexcept {
		bool isNotEmpty{};
		auto error = ValueIterator::resetArray().get(isNotEmpty);
		if (error) {
			return error;
		}
		return !isNotEmpty;
	}

	JsonifierResult<bool> Array::reset() & noexcept {
		return ValueIterator::resetArray();
	}

	JsonifierResult<Value> Array::atPointer(std::string_view jsonPointer) noexcept {
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

	JsonifierResult<Value> Array::at(size_t index) noexcept {
		size_t i = 0;
		for (auto Value: *this) {
			if (i == index) {
				return Value;
			}
			i++;
		}
		return Out_Of_Bounds;
	}

	JsonifierResult<Array>::JsonifierResult(Array&& Value) noexcept : JsonifierResultBase<Array>(std::forward<Array>(Value)){};

	JsonifierResult<Array>::JsonifierResult(ErrorCode error) noexcept : JsonifierResultBase<Array>(error){};

	JsonifierResult<ArrayIterator> JsonifierResult<Array>::begin() noexcept {
		if (error()) {
			return error();
		}
		return first.begin();
	}

	JsonifierResult<ArrayIterator> JsonifierResult<Array>::end() noexcept {
		if (error()) {
			return error();
		}
		return first.end();
	}

	JsonifierResult<size_t> JsonifierResult<Array>::countElements() & noexcept {
		if (error()) {
			return error();
		}
		return first.countElements();
	}

	JsonifierResult<bool> JsonifierResult<Array>::isEmpty() & noexcept {
		if (error()) {
			return error();
		}
		return first.isEmpty();
	}

	JsonifierResult<Value> JsonifierResult<Array>::at(size_t index) noexcept {
		if (error()) {
			return error();
		}
		return first.at(index);
	}

	JsonifierResult<Value> JsonifierResult<Array>::atPointer(std::string_view jsonPointer) noexcept {
		if (error()) {
			return error();
		}
		return first.atPointer(jsonPointer);
	}
}
