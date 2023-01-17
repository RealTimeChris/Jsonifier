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

#include <jsonifier/Array.hpp>

namespace Jsonifier {

	__forceinline Array::Array(NodeIterator&& other) noexcept {
		rootStructural = other.rootStructural;
		iteratorCore = other.iteratorCore;
	}

	__forceinline JsonifierResult<std::string_view> Array::getRawJsonString() noexcept {
		return NodeIterator::getRawJsonString();
	}

	__forceinline JsonifierResult<JsonData> Array::operator[](size_t index) noexcept {
		return at(index);
	}

	__forceinline JsonifierResult<JsonData> Array::at(size_t index) noexcept {
		size_t currentDepth{};
		auto originalPosition = position();
		setPosition(rootPosition());
		for (auto& value: *this) {
			if (currentDepth == index) {
				auto returnData = JsonData{ NodeIterator{ static_cast<NodeIterator>(value.value()) } };
				setPosition(originalPosition);
				return returnData;
			}
			currentDepth++;
		}
		setPosition(originalPosition);
		setError(No_Such_Entity);
		return { std::move(*this) };
	}

	__forceinline JsonifierResult<ArrayIterator> Array::begin() noexcept {
		setPosition(rootPosition());
		if (*peek(position()) != '[') {
			setError(Incorrect_Type);
			return ArrayIterator{ operator Jsonifier::NodeIterator() };
		}
		setPosition(rootPosition() + 1);
		return ArrayIterator{ operator Jsonifier::NodeIterator() };
	}

	__forceinline JsonifierResult<ArrayIterator> Array::end() noexcept {
		return {};
	}

	__forceinline JsonifierResult<size_t> Array::size() noexcept {
		auto originalPosition = position();
		setPosition(rootPosition());
		size_t count{};
		for (auto& value: *this) {
			++count;
		}
		setPosition(originalPosition);
		return count;
	}

	__forceinline JsonifierResult<Array>::JsonifierResult(Array&& valueNew) noexcept : JsonifierResultBase<Array>(std::forward<Array>(valueNew)) {
	}

	__forceinline JsonifierResult<Array>::JsonifierResult(ErrorCode error) noexcept : JsonifierResultBase<Array>(error) {
	}

	__forceinline JsonifierResult<std::string_view> JsonifierResult<Array>::getRawJsonString() noexcept {
		return second.getRawJsonString();
	}

	__forceinline JsonifierResult<JsonData> JsonifierResult<Array>::operator[](size_t key) noexcept {
		return second.operator[](key);
	}

	__forceinline JsonifierResult<JsonData> JsonifierResult<Array>::at(size_t index) noexcept {
		return second.at(index);
	}

	__forceinline JsonifierResult<ArrayIterator> JsonifierResult<Array>::begin() noexcept {
		return second.begin();
	}

	__forceinline JsonifierResult<ArrayIterator> JsonifierResult<Array>::end() noexcept {
		return second.end();
	}

	__forceinline JsonifierResult<size_t> JsonifierResult<Array>::size() noexcept {
		return second.size();
	}

}
