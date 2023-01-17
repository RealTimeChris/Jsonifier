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

#include <jsonifier/Object.hpp>

namespace Jsonifier {

	__forceinline Object::Object(NodeIterator&& other) noexcept {
		rootStructural = other.rootStructural;
		iteratorCore = other.iteratorCore;
	}

	__forceinline JsonifierResult<JsonData> Object::operator[](std::string_view key) noexcept {
		setError(Success);
		bool hasValue{ findField(key) };
		if (reportError()) {
			return JsonData{ std::move(*this) };
		}
		if (!hasValue) {
			setError(No_Such_Field);
			return JsonData{ std::move(*this) };
		}
		if (*peek(position()) == ',') {
			returnCurrentAndAdvance();
		}
		NodeIterator iterator{ *this };
		iterator.rootStructural = position();
		return JsonData{ std::move(iterator) };
	}

	__forceinline JsonifierResult<JsonData> Object::operator[](const char* key) noexcept {
		setError(Success);
		bool hasValue{ findField(key) };
		if (reportError()) {
			return JsonData{ std::move(*this) };
		}
		if (!hasValue) {
			setError(No_Such_Field);
			return JsonData{ std::move(*this) };
		}
		if (*peek(position()) == ',') {
			returnCurrentAndAdvance();
		}
		NodeIterator iterator{ *this };
		iterator.rootStructural = position();
		return JsonData{ std::move(iterator) };
	}

	__forceinline JsonifierResult<std::string_view> Object::getRawJsonString() noexcept {
		return NodeIterator::getRawJsonString();
	}

	__forceinline JsonifierResult<ObjectIterator> Object::begin() noexcept {
		setPosition(rootPosition());
		if (*peek(position()) != '{') {
			setError(Incorrect_Type);
			return ObjectIterator{ operator Jsonifier::NodeIterator() };
		}
		setPosition(position() + 1);
		return ObjectIterator{ operator Jsonifier::NodeIterator() };
	}

	__forceinline JsonifierResult<ObjectIterator> Object::end() noexcept {
		return {};
	}

	__forceinline JsonifierResult<size_t> Object::size() noexcept {
		auto originalPosition = position();
		setPosition(rootPosition());
		size_t count{};
		for (auto& value: *this) {
			++count;
		}
		setPosition(originalPosition);
		return count;
	}

	__forceinline JsonifierResult<Object>::JsonifierResult(Object&& valueNew) noexcept : JsonifierResultBase<Object>(std::forward<Object>(valueNew)) {
	}

	__forceinline JsonifierResult<Object>::JsonifierResult(ErrorCode error) noexcept : JsonifierResultBase<Object>(error) {
	}

	__forceinline JsonifierResult<JsonData> JsonifierResult<Object>::operator[](std::string_view key) noexcept {
		return second.operator[](key);
	}

	__forceinline JsonifierResult<JsonData> JsonifierResult<Object>::operator[](const char* key) noexcept {
		return second.operator[](key);
	}

	__forceinline JsonifierResult<std::string_view> JsonifierResult<Object>::getRawJsonString() noexcept {
		return second.getRawJsonString();
	}

	__forceinline JsonifierResult<ObjectIterator> JsonifierResult<Object>::begin() noexcept {
		return second.begin();
	}

	__forceinline JsonifierResult<ObjectIterator> JsonifierResult<Object>::end() noexcept {
		return second.end();
	}

	__forceinline JsonifierResult<size_t> JsonifierResult<Object>::size() noexcept {
		return second.size();
	}

}
