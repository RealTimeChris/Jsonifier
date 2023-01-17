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

#include <jsonifier/JsonData.hpp>
#include <jsonifier/Array_Impl.hpp>
#include <jsonifier/NumberParsing.hpp>

namespace Jsonifier {

	__forceinline JsonData::JsonData(NodeIterator&& other) noexcept {
		rootStructural = other.rootStructural;
		iteratorCore = other.iteratorCore;
	}

	template<> __forceinline ErrorCode JsonData::get<std::string_view>(std::string_view& value) noexcept {
		auto resutlValue = getString();
		auto errorVal = reportError();
		if (!errorVal) {
			value = std::move(resutlValue);
		}
		return errorVal;
	}

	template<> __forceinline ErrorCode JsonData::get<bool>(bool& value) noexcept {
		auto resutlValue = getBool();
		auto errorVal = reportError();
		if (!errorVal) {
			value = std::move(resutlValue);
		}
		return errorVal;
	}

	template<> __forceinline ErrorCode JsonData::get<double>(double& value) noexcept {
		auto resutlValue = getDouble();
		auto errorVal = reportError();
		if (!errorVal) {
			value = std::move(resutlValue);
		}
		return errorVal;
	}

	template<> __forceinline ErrorCode JsonData::get<int64_t>(int64_t& value) noexcept {
		auto resutlValue = getInt64();
		auto errorVal = reportError();
		if (!errorVal) {
			value = std::move(resutlValue);
		}
		return errorVal;
	}

	template<> __forceinline ErrorCode JsonData::get<Array>(Array& value) noexcept {
		auto resutlValue = getArray();
		auto errorVal = reportError();
		if (!errorVal) {
			value = std::move(resutlValue);
		}
		return errorVal;
	}

	template<> __forceinline ErrorCode JsonData::get<Object>(Object& value) noexcept {
		auto resutlValue = getObject();
		auto errorVal = reportError();
		if (!errorVal) {
			value = std::move(resutlValue);
		}
		return errorVal;
	}

	template<> __forceinline ErrorCode JsonData::get<uint64_t>(uint64_t& value) noexcept {
		auto resutlValue = getUint64();
		auto errorVal = reportError();
		if (!errorVal) {
			value = std::move(resutlValue);
		}
		return errorVal;
	}

	template<> __forceinline ErrorCode JsonData::get<JsonData>(JsonData& value) noexcept {
		auto resutlValue = *this;
		auto errorVal = reportError();
		if (!errorVal) {
			value = std::move(resutlValue);
		}
		return errorVal;
	}

	__forceinline JsonifierResult<std::string_view> JsonData::getRawJsonString() noexcept {
		return NodeIterator::getRawJsonString();
	}

	__forceinline JsonifierResult<JsonType> JsonData::type() noexcept {
		return NodeIterator::type(rootPosition());
	}

	__forceinline JsonData::operator uint64_t() noexcept(false) {
		auto returnValue = getUint64();
		if (reportError()) {
			throw reportError();
		}
		return returnValue;
	}

	__forceinline JsonData::operator int64_t() noexcept(false) {
		auto returnValue = getInt64();
		if (reportError()) {
			throw reportError();
		}
		return returnValue;
	}

	__forceinline JsonData::operator double() noexcept(false) {
		auto returnValue = getDouble();
		if (reportError()) {
			throw reportError();
		}
		return returnValue;
	}

	__forceinline JsonData::operator Array() noexcept(false) {
		auto returnValue = getArray();
		if (reportError()) {
			throw reportError();
		}
		return returnValue;
	}

	__forceinline JsonData::operator Object() noexcept(false) {
		auto returnValue = getObject();
		if (reportError()) {
			throw reportError();
		}
		return returnValue;
	}

	__forceinline JsonData::operator std::string() noexcept(false) {
		auto returnValue = getString();
		if (reportError()) {
			throw reportError();
		}
		return static_cast<std::string>(returnValue);
	}

	__forceinline JsonData::operator std::string_view() noexcept(false) {
		auto returnValue = getString();
		if (reportError()) {
			throw reportError();
		}
		return returnValue;
	}

	__forceinline JsonData::operator bool() noexcept(false) {
		auto returnValue = getBool();
		if (reportError()) {
			throw reportError();
		}
		return returnValue;
	}

	__forceinline JsonifierResult<JsonData>::JsonifierResult(JsonData&& valueNew) noexcept
		: JsonifierResultBase<JsonData>(std::forward<JsonData>(valueNew)) {
	}

	__forceinline JsonifierResult<JsonData>::JsonifierResult(ErrorCode error) noexcept : JsonifierResultBase<JsonData>(error) {
	}

	__forceinline JsonifierResult<JsonType> JsonifierResult<JsonData>::type() noexcept {
		return second.type();
	}

	__forceinline JsonifierResult<JsonData>::operator Object() noexcept(false) {
		return second;
	}

	__forceinline JsonifierResult<JsonData>::operator Array() noexcept(false) {
		return second;
	}

	__forceinline JsonifierResult<JsonData>::operator uint64_t() noexcept(false) {
		return second;
	}

	__forceinline JsonifierResult<JsonData>::operator int64_t() noexcept(false) {
		return second;
	}

	__forceinline JsonifierResult<JsonData>::operator double() noexcept(false) {
		return second;
	}

	__forceinline JsonifierResult<JsonData>::operator std::string_view() noexcept(false) {
		return second;
	}

	__forceinline JsonifierResult<JsonData>::operator std::string() noexcept(false) {
		return second;
	}

	__forceinline JsonifierResult<JsonData>::operator bool() noexcept(false) {
		return second;
	}

	template<typename OTy> __forceinline ErrorCode JsonifierResult<JsonData>::get(OTy& newValue) noexcept {
		return second.get(newValue);
	}

	__forceinline JsonifierResult<std::string_view> JsonifierResult<JsonData>::getRawJsonString() noexcept {
		return second.getRawJsonString();
	}
}
