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

#include <jsonifier/ArrayIterator.hpp>

namespace Jsonifier {

	__forceinline ArrayIterator::ArrayIterator(NodeIterator&& other) noexcept {
		iteratorCore = other.iteratorCore;
		rootStructural = position();
		currentStructural = rootStructural;
		data = JsonData{ operator Jsonifier::NodeIterator() };
	}

	__forceinline JsonifierResult<JsonData>& ArrayIterator::operator*() noexcept {
		if (*peek(currentStructural) == ',') {
			++currentStructural;
		}
		data = JsonData{ operator Jsonifier::NodeIterator() };
		data.second.rootStructural = currentStructural;
		setPosition(currentStructural);
		return data;
	}

	__forceinline bool ArrayIterator::operator==(ArrayIterator& other) noexcept {
		auto returnValue = !isValuePresent(currentStructural, ',');
		while (*peek(currentStructural) == ']' || *peek(currentStructural) == '}') {
			++currentStructural;
		}
		setPosition(currentStructural);
		return returnValue;
	}

	__forceinline ArrayIterator ArrayIterator::operator++() noexcept {
		currentStructural = skipValue(currentStructural);
		setPosition(currentStructural);
		return std::move(*this);
	}

	__forceinline JsonifierResult<ArrayIterator>::JsonifierResult(ArrayIterator&& jsonData) noexcept
		: JsonifierResultBase<ArrayIterator>(std::forward<ArrayIterator>(jsonData)){};

	__forceinline JsonifierResult<ArrayIterator>::JsonifierResult(ErrorCode error) noexcept : JsonifierResultBase<ArrayIterator>(error){};

	__forceinline bool JsonifierResult<ArrayIterator>::operator==(JsonifierResult<ArrayIterator>& other) noexcept {
		return second == other.second;
	}

	__forceinline JsonifierResult<ArrayIterator> JsonifierResult<ArrayIterator>::operator++() noexcept {
		++second;
		return std::move(second);
	}

	__forceinline JsonifierResult<JsonData>& JsonifierResult<ArrayIterator>::operator*() noexcept {
		return second.operator*();
	}

};
