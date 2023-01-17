/*
	Jsonifier - For parsing and serializing Json - very rapidly.
	Copyright (C) 2023 Chris M. (RealTimeChris)

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	This library is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
	Lesser General Public License for more details.

	You should have received a copy of the GNU Lesser General Public
	License along with this library; if not, write to the Free Software
	Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301
	USA
*/
/// https://github.com/RealTimeChris/Jsonifier
/// Feb 3, 2023
#pragma once

#include <jsonifier/NodeIterator.hpp>
#include <jsonifier/Array.hpp>
#include <jsonifier/Object.hpp>
#include <jsonifier/NumberParsing.hpp>

namespace Jsonifier {

	__forceinline NodeIterator::NodeIterator(StructuralIndex newIndex, IteratorCore* newIteratorCore) noexcept {
		iteratorCore = newIteratorCore;
		rootStructural = newIndex;
	}

	__forceinline NodeIterator::NodeIterator(IteratorCore* iteratorCoreNew) noexcept {
		rootStructural = iteratorCoreNew->position();
		iteratorCore = iteratorCoreNew;
	}

	__forceinline StructuralIndex NodeIterator::skipUntilClosed(StructuralIndex index) noexcept {
		++index;
		size_t openCount = 1;
		size_t closeCount = 0;
		while (index < globalIter()->lastPosition() && openCount > closeCount) {
			switch (globalIter()->getStringView()[*index]) {
				case '[':
				case '{':
					++openCount;
					++index;
					break;
				case '}':
				case ']':
					++closeCount;
					++index;
					break;
				default:
					++index;
			}
		}
		return index;
	}

	__forceinline StructuralIndex NodeIterator::skipValue(StructuralIndex index) noexcept {
		while (index < globalIter()->lastPosition()) {
			switch (globalIter()->getStringView()[*index]) {
				case '{':
				case '[':
					index = skipUntilClosed(index);
					break;
				case ',':
				case '}':
				case ']':
					break;
				default: {
					++index;
					continue;
				}
			}
			break;
		}
		return index;
	}

	__forceinline JsonType NodeIterator::type(StructuralIndex currentIndex) noexcept {
		auto newValue = peek(currentIndex);
		if (newValue != nullptr) {
			switch (*newValue) {
				case '{':
					return JsonType::Object;
				case '[':
					return JsonType::Array;
				case '"':
					return JsonType::String;
				case 'n':
					return JsonType::Null;
				case 't':
				case 'f':
					return JsonType::Bool;
				case '-':
				case '0':
				case '1':
				case '2':
				case '3':
				case '4':
				case '5':
				case '6':
				case '7':
				case '8':
				case '9':
					return JsonType::Number;
				default:
					setError(Type_Error);
					return JsonType::Unset;
			}
		} else {
			setError(Type_Error);
			return JsonType::Unset;
		}
	}

	__forceinline bool NodeIterator::findField(const std::string_view key) noexcept {
		bool haveWeRestarted{ false };
		auto newPtr = rootPosition();
		size_t openCount = 1;
		size_t closeCount = 0;
		while (1) {
			if (*peek(newPtr) == '"' && *peek(newPtr + 1) == ':') {
				InStringPtr keyStart = peek(newPtr);
				size_t length = peek(newPtr + 1) - keyStart;
				std::string_view actualKey{ std::string_view(reinterpret_cast<const char*>(keyStart + 1), length - 2) };
				if (actualKey == key) {
					this->setPosition(newPtr + 2);
					return true;
				}
				++newPtr;
			}
			if (openCount <= closeCount || newPtr >= this->globalIter()->lastPosition()) {
				if (!haveWeRestarted) {
					haveWeRestarted = true;
					openCount = 1;
					closeCount = 0;
					newPtr = rootPosition();
					continue;
				}
				setError(No_Such_Field);
				return false;
			}
			switch (globalIter()->getStringView()[*newPtr]) {
				case '[':
				case '{':
					++openCount;
					++newPtr;
					break;
				case '}':
				case ']':
					++closeCount;
					++newPtr;
					break;
				default:
					++newPtr;
			}
		}
		return false;
	}

	__forceinline StructuralIndex NodeIterator::position() noexcept {
		return iteratorCore->position();
	}

	__forceinline StructuralIndex NodeIterator::rootPosition() noexcept {
		return rootStructural;
	}
	
	__forceinline IteratorCore* NodeIterator::globalIter() noexcept {
		return iteratorCore;
	}

	__forceinline bool NodeIterator::atGlobalEnd() noexcept {
		if (position() >= globalIter()->lastPosition()) {
			return true;
		} else {
			return false;
		}
	}

	__forceinline bool NodeIterator::resetValue() noexcept {
		setPosition(rootPosition());
		return startedValue();
	}

	__forceinline bool NodeIterator::startedValue() noexcept {
		switch (*peek(rootPosition())) {
			case '{': {
				if (*peek(position()) == '}') {
					returnCurrentAndAdvance();
					return false;
				}
				return true;
			}
			case '[': {
				if (*peek(position()) == ']') {
					returnCurrentAndAdvance();
					return false;
				}
				return true;
			}
			default: {
				return false;
			}
		}
	}

	__forceinline bool NodeIterator::atRoot() noexcept {
		return this->position() == rootStructural;
	}

	__forceinline bool NodeIterator::isValuePresent(StructuralIndex index, char character, int32_t offset) noexcept {
		if (position() == rootPosition()) {
			return true;
		} else {
			return *peek(index + offset) == character;
		}
	}

	__forceinline InStringPtr NodeIterator::peek(StructuralIndex positionNew) noexcept {
		return &globalIter()->getStringView()[*positionNew];
	}

	__forceinline void NodeIterator::setError(ErrorCode errorNew) noexcept {
		iteratorCore->setError(errorNew);
	}

	__forceinline Object NodeIterator::getObject() noexcept {
		skipContinuation();
		JsonType typeNew{ type(rootPosition()) };
		if (typeNew != JsonType::Object) {
			setError(Incorrect_Type);
			return Object{ operator Jsonifier::NodeIterator() };
		}
		return Object{ operator Jsonifier::NodeIterator() };
	}

	__forceinline Array NodeIterator::getArray() noexcept {
		skipContinuation();
		JsonType typeVal{ type(rootPosition()) };
		if (typeVal != JsonType::Array) {
			setError(Incorrect_Type);
			return Array{ operator Jsonifier::NodeIterator() };
		}
		return Array{ operator Jsonifier::NodeIterator() };
	}

	__forceinline std::string_view NodeIterator::getString() noexcept {
		JsonType typeVal{ type(position()) };
		if (typeVal != JsonType::String) {
			setError(Incorrect_Type);
			return {};
		}
		uint8_t* end = parseString(peek(position()) + 1, iteratorCore->getStringBuffer());
		if (!end) {
			setError(String_Error);
			return {};
		}
		std::string_view result(reinterpret_cast<const char*>(iteratorCore->getStringBuffer()), end - iteratorCore->getStringBuffer());
		iteratorCore->getStringBuffer() = end;
		return result;
	}

	__forceinline double NodeIterator::getDouble() noexcept {
		JsonType typeVal{ type(position()) };
		if (typeVal != JsonType::Number) {
			setError(Incorrect_Type);
			return 0;
		}
		double returnValue{};
		JsonifierTry(parseDouble(peek(position())).get(returnValue));
		return returnValue;
	}

	__forceinline uint64_t NodeIterator::getUint64() noexcept {
		JsonType typeVal{ type(position()) };
		if (typeVal != JsonType::Number) {
			setError(Incorrect_Type);
			return 0;
		}
		uint64_t returnValue{};
		parseUnsigned(peek(position())).get(returnValue);
		return returnValue;
	}

	__forceinline int64_t NodeIterator::getInt64() noexcept {
		JsonType typeVal{ type(position()) };
		if (typeVal != JsonType::Number) {
			setError(Incorrect_Type);
			return 0;
		}
		int64_t returnValue{};
		parseInteger(peek(position())).get(returnValue);
		return returnValue;
	}

	__forceinline bool NodeIterator::getBool() noexcept {
		JsonType typeVal{ type(position()) };
		if (typeVal != JsonType::Bool) {
			setError(Incorrect_Type);
			return false;
		}
		return parseBool(peek(position()));
	}

	__forceinline InStringPtr NodeIterator::returnCurrentAndAdvance() noexcept {
		if (atGlobalEnd()) {
			globalIter()->setPosition(rootPosition());
		}
		auto originalPosition = position();
		globalIter()->setPosition(position() + 1);
		return &globalIter()->getStringView()[*originalPosition];
	}

	__forceinline bool NodeIterator::parseBool(const uint8_t* json) noexcept {
		auto not_true = str4ncmp(json, "true");
		auto not_false = str4ncmp(json, "fals") | (json[4] ^ 'e');
		bool error = (not_true && not_false) || isNotStructuralOrWhitespace(json[not_true ? 5 : 4]);
		if (error) {
			setError(Incorrect_Type);
			return {};
		}
		return JsonifierResult<bool>(!not_true);
	}

	__forceinline void NodeIterator::skipContinuation() noexcept {
		if (*peek(position()) == ',') {
			returnCurrentAndAdvance();
		}
	}

	__forceinline bool NodeIterator::parseNull(InStringPtr json) noexcept {
		bool isNullString = !str4ncmp(json, "null") && isStructuralOrWhitespace(json[4]);
		if (!isNullString && json[0] == 'n') {
			setError(Incorrect_Type);
			return {};
		}
		return isNullString;
	}

	__forceinline ErrorCode NodeIterator::reportError() noexcept {
		return iteratorCore->reportError();
	}

	__forceinline void NodeIterator::setPosition(StructuralIndex index) noexcept {
		iteratorCore->setPosition(index);
	}

	__forceinline std::string_view NodeIterator::getRawJsonString() noexcept {
		auto oldPtr = position();
		auto startPtr = peek(rootPosition());
		auto length = peek(skipValue(rootPosition())) - startPtr;
		if (!startPtr) {
			setError(String_Error);
			return {};
		}
		setPosition(oldPtr);
		return std::string_view{ reinterpret_cast<const char*>(startPtr), static_cast<size_t>(length) };
	}

}
