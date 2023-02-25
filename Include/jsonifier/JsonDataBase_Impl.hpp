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

#include <jsonifier/JsonDataBase.hpp>
#include <jsonifier/Array_Impl.hpp>
#include <jsonifier/NumberParsing.hpp>
#include <jsonifier/Object.hpp>
#include <jsonifier/ObjectIterator.hpp>

namespace Jsonifier {

	__forceinline JsonDataBase::JsonDataBase(IteratorCore* iteratorCoreNew, StructuralIndex indexNew) noexcept {
		iteratorCore = iteratorCoreNew;
		rootStructural = indexNew;
	}

	__forceinline JsonDataBase::JsonDataBase(IteratorCore* iteratorCoreNew) noexcept {
		rootStructural = iteratorCoreNew->rootPosition();
		iteratorCore = iteratorCoreNew;
	}

	__forceinline StructuralIndex JsonDataBase::skipUntilClosed(StructuralIndex index) noexcept {
		++index;
		size_t openCount = 1;
		size_t closeCount = 0;
		while (index <= iteratorCore->endPosition() && openCount > closeCount) {
			switch (*peek(index)) {
				case '[':
				case '{': {
					++openCount;
					break;
				}
				case '}':
				case ']': {
					++closeCount;
					if (closeCount >= openCount) {
						return index;
					}
					break;
				}
			}
			++index;
		}
		return index;
	}

	__forceinline StructuralIndex JsonDataBase::skipValue(StructuralIndex index) noexcept {
		while (index <= iteratorCore->endPosition()) {
			switch (*peek(index)) {
				case '{':
				case '[':
					return skipUntilClosed(index);
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

	__forceinline JsonType JsonDataBase::type(StructuralIndex currentIndex) noexcept {
		StringViewPtr newValue = peek(currentIndex);

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

	__forceinline StringView JsonDataBase::getStringKey(StructuralIndex index) noexcept {
		int64_t sizeVal = static_cast<int64_t>(*(index + 1)) - static_cast<int64_t>(*index);
		if (sizeVal >= 2) {
			return StringView{ reinterpret_cast<const char*>(&*peek(index)) + 1, static_cast<size_t>(sizeVal) - 2 };
		} else {
			return StringView{};
		}
	}

	__forceinline StringViewPtr JsonDataBase::peek(StructuralIndex positionNew) noexcept {
		if (!positionNew || !iteratorCore) {
			return nullptr;
		}
		return &iteratorCore->getStringView()[*positionNew];
	}

	__forceinline void JsonDataBase::setError(ErrorCode errorNew) noexcept {
		if (!iteratorCore) {
			return;
		}
		iteratorCore->setError(errorNew);
	}

	__forceinline Object JsonDataBase::getObject() noexcept {
		JsonType typeNew{ type(rootStructural) };
		Object newObject{ iteratorCore, rootStructural };
		if (typeNew != JsonType::Object) {
			setError(Incorrect_Type);
			return newObject;
		}
		newObject.parseJson();
		return newObject;
	}

	__forceinline Array JsonDataBase::getArray() noexcept {
		JsonType typeNew{ type(rootStructural) };
		Array newArray{ iteratorCore, rootStructural };
		if (typeNew != JsonType::Array) {
			setError(Incorrect_Type);
			return newArray;
		}
		newArray.parseJson();
		return newArray;
	}

	__forceinline std::string_view JsonDataBase::getString() noexcept {
		JsonType typeVal{ type(rootStructural) };
		if (typeVal != JsonType::String) {
			setError(Incorrect_Type);
			return {};
		}
		uint8_t* end = parseString(peek(rootStructural) + 1, iteratorCore->getStringBuffer());
		if (!end) {
			setError(String_Error);
			return {};
		}
		std::string_view result(reinterpret_cast<const char*>(iteratorCore->getStringBuffer()), end - iteratorCore->getStringBuffer());
		iteratorCore->getStringBuffer() = end;
		return result;
	}

	__forceinline double JsonDataBase::getDouble() noexcept {
		JsonType typeVal{ type(rootStructural) };
		if (typeVal != JsonType::Number) {
			setError(Incorrect_Type);
			return {};
		}
		double returnValue{};
		if (auto error = parseDouble(peek(rootStructural)).get(returnValue)) {
			setError(error);
		}
		return returnValue;
	}

	__forceinline uint64_t JsonDataBase::getUint64() noexcept {
		JsonType typeVal{ type(rootStructural) };
		if (typeVal != JsonType::Number) {
			setError(Incorrect_Type);
			return {};
		}
		uint64_t returnValue{};
		if (auto error = parseUnsigned(peek(rootStructural)).get(returnValue)) {
			setError(error);
		}
		return returnValue;
	}

	__forceinline int64_t JsonDataBase::getInt64() noexcept {
		JsonType typeVal{ type(rootStructural) };
		if (typeVal != JsonType::Number) {
			setError(Incorrect_Type);
			return {};
		}
		int64_t returnValue{};
		if (auto error = parseInteger(peek(rootStructural)).get(returnValue)) {
			setError(error);
		}
		return returnValue;
	}

	__forceinline bool JsonDataBase::getBool() noexcept {
		JsonType typeVal{ type(rootStructural) };
		if (typeVal != JsonType::Bool) {
			setError(Incorrect_Type);
			return {};
		}
		return parseBool(peek(rootStructural));
	}

	__forceinline bool JsonDataBase::parseBool(const uint8_t* json) noexcept {
		uint32_t notTrue = str4ncmp(json, "true");
		uint32_t notFalse = str4ncmp(json, "fals") | (json[4] ^ 'e');
		bool error = (notTrue && notFalse) || isNotStructuralOrWhitespace(json[notTrue ? 5 : 4]);
		if (error) {
			setError(Incorrect_Type);
			return {};
		}
		return !notTrue;
	}

	__forceinline bool JsonDataBase::parseNull(StringViewPtr json) noexcept {
		bool isNullString = !str4ncmp(json, "null") && isStructuralOrWhitespace(json[4]);
		if (!isNullString && json[0] == 'n') {
			setError(Incorrect_Type);
			return {};
		}
		return isNullString;
	}

	__forceinline ErrorCode JsonDataBase::reportError() noexcept {
		if (!iteratorCore) {
			return Uninitialized;
		}
		return iteratorCore->reportError();
	}

	__forceinline JsonType JsonDataBase::type() noexcept {
		return this->type(rootStructural);
	}

	__forceinline StringView JsonDataBase::getRawJsonString() noexcept {
		StructuralIndex oldPtr = rootStructural;
		StringViewPtr startPtr = peek(rootStructural);
		uint32_t length = peek(skipValue(rootStructural)) - startPtr + 1;
		if (!startPtr) {
			setError(String_Error);
			return {};
		}
		return StringView{ reinterpret_cast<const char*>(startPtr), static_cast<size_t>(length) };
	}

	template<> __forceinline ErrorCode JsonDataBase::get<std::string_view>(std::string_view& value) noexcept {
		auto resutlValue = getString();
		auto errorVal = reportError();
		if (!errorVal) {
			value = std::move(resutlValue);
		}
		return errorVal;
	}

	template<> __forceinline ErrorCode JsonDataBase::get<bool>(bool& value) noexcept {
		auto resutlValue = getBool();
		auto errorVal = reportError();
		if (!errorVal) {
			value = std::move(resutlValue);
		}
		return errorVal;
	}

	template<> __forceinline ErrorCode JsonDataBase::get<double>(double& value) noexcept {
		auto resutlValue = getDouble();
		auto errorVal = reportError();
		if (!errorVal) {
			value = std::move(resutlValue);
		}
		return errorVal;
	}

	template<> __forceinline ErrorCode JsonDataBase::get<int64_t>(int64_t& value) noexcept {
		auto resutlValue = getInt64();
		auto errorVal = reportError();
		if (!errorVal) {
			value = std::move(resutlValue);
		}
		return errorVal;
	}

	template<> __forceinline ErrorCode JsonDataBase::get<Array>(Array& value) noexcept {
		auto resutlValue = getArray();
		auto errorVal = reportError();
		if (!errorVal) {
			value = std::move(resutlValue);
		}
		return errorVal;
	}

	template<> __forceinline ErrorCode JsonDataBase::get<Object>(Object& value) noexcept {
		auto resutlValue = getObject();
		auto errorVal = reportError();
		if (!errorVal) {
			value = std::move(resutlValue);
		}
		return errorVal;
	}

	template<> __forceinline ErrorCode JsonDataBase::get<uint64_t>(uint64_t& value) noexcept {
		auto resutlValue = getUint64();
		auto errorVal = reportError();
		if (!errorVal) {
			value = std::move(resutlValue);
		}
		return errorVal;
	}

	template<> __forceinline ErrorCode JsonDataBase::get<JsonDataBase>(JsonDataBase& value) noexcept {
		auto resutlValue = *this;
		auto errorVal = reportError();
		if (!errorVal) {
			value = std::move(resutlValue);
		}
		return errorVal;
	}

	__forceinline size_t JsonDataBase::size() noexcept {
		return sizeVal;
	}

	__forceinline JsonDataBase::operator uint64_t() noexcept {
		setError(Success);
		auto returnValue = getUint64();
		if (reportError()) {
			return uint64_t{};
		}
		return returnValue;
	}

	__forceinline JsonDataBase::operator int64_t() noexcept {
		setError(Success);
		auto returnValue = getInt64();
		if (reportError()) {
			return int64_t{};
		}
		return returnValue;
	}

	__forceinline JsonDataBase::operator double() noexcept {
		setError(Success);
		auto returnValue = getDouble();
		if (reportError()) {
			return double{};
		}
		return returnValue;
	}

	__forceinline JsonDataBase::operator Array() noexcept {
		return getArray();
	}

	__forceinline JsonDataBase::operator ErrorCode() noexcept {
		return reportError();
	}

	__forceinline JsonDataBase::operator Object() noexcept {
		return getObject();
	}

	__forceinline JsonDataBase::operator std::string() noexcept {
		setError(Success);
		auto returnValue = getString();
		if (reportError()) {
			return std::string{};
		}
		return static_cast<std::string>(returnValue);
	}

	__forceinline JsonDataBase::operator std::string_view() noexcept {
		setError(Success);
		auto returnValue = getString();
		if (reportError()) {
			return std::string_view{};
		}
		return returnValue;
	}

	__forceinline JsonDataBase::operator bool() noexcept {
		setError(Success);
		auto returnValue = getBool();
		if (reportError()) {
			return bool{};
		}
		return returnValue;
	}

}
