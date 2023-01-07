#include <jsonifier/FoundationEntities.hpp>
#include <jsonifier/Array_Impl.hpp>

namespace Jsonifier {

	JsonifierResult<Value> ArrayIterator::operator*() noexcept {
		if (this->error()) {
			this->abandon();
			return this->error();
		}
		return Value(this->child());
	}

	bool ArrayIterator::operator==(const ArrayIterator& other) const noexcept {
		return !(*this != other);
	}

	bool ArrayIterator::operator!=(const ArrayIterator&) const noexcept {
		return this->isOpen();
	}

	ArrayIterator& ArrayIterator::operator++() noexcept {
		ErrorCode error;
		if ((error = this->error())) {
			return *this;
		}
		if ((error = this->skipChild())) {
			return *this;
		}
		if ((error = this->hasNextElement().error())) {
			return *this;
		}
		return *this;
	}

	Array::Array(const ValueIterator& _iterator) noexcept : iterator{ _iterator } {
	}

	JsonifierResult<Array> Array::start(ValueIterator& iterator) noexcept {
		bool hasValue{};
		JsonifierTry(iterator.startArray().get(hasValue));
		return Array(iterator);
	}

	JsonifierResult<Array> Array::startRoot(ValueIterator& iterator) noexcept {
		bool hasValue{};
		JsonifierTry(iterator.starRootArray().get(hasValue));
		return Array(iterator);
	}

	JsonifierResult<Array> Array::started(ValueIterator& iterator) noexcept {
		bool hasValue{};
		JsonifierTry(iterator.startedArray().get(hasValue));
		return Array(iterator);
	}

	JsonifierResult<ArrayIterator> Array::begin() noexcept {
		return ArrayIterator(iterator);
	}

	JsonifierResult<ArrayIterator> Array::end() noexcept {
		return ArrayIterator(iterator);
	}

	ErrorCode Array::consume() noexcept {
		auto error = iterator.jsonIter().skipChild(iterator.depth() - 1);
		if (error) {
			iterator.abandon();
		}
		return error;
	}

	JsonifierResult<std::string_view> Array::rawJson() noexcept {
		const uint8_t* startingPoint{ iterator.peekStart() };
		auto error = consume();
		if (error) {
			return error;
		}
		const uint8_t* finalPoint{ iterator.jsonIterator->unsafePointer() };
		return std::string_view(reinterpret_cast<const char*>(startingPoint), size_t(finalPoint - startingPoint));
	}

	JsonifierResult<size_t> Array::countElements() & noexcept {
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

	JsonifierResult<bool> Array::isEmpty() & noexcept {
		bool isNotEmpty;
		auto error = iterator.resetArray().get(isNotEmpty);
		if (error) {
			return error;
		}
		return !isNotEmpty;
	}

	JsonifierResult<bool> Array::reset() & noexcept {
		return iterator.resetArray();
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
		for (auto value: *this) {
			if (i == index) {
				return value;
			}
			i++;
		}
		return Out_Of_Bounds;
	}

}