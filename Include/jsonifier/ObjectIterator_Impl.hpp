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

#include <jsonifier/ObjectIterator.hpp>

namespace Jsonifier {

	__forceinline ObjectIterator::ObjectIterator(NodeIterator&& other) noexcept {
		iteratorCore = other.iteratorCore;
		rootStructural = position();
		currentStructural = rootStructural;
		data = Field{ operator Jsonifier::NodeIterator() };
	}

	__forceinline JsonifierResult<Field>& ObjectIterator::operator*() noexcept {
		if (*peek(currentStructural) == ',') {
			++currentStructural;
		}
		setPosition(currentStructural);
		data = Field{ operator Jsonifier::NodeIterator() };
		return data;
	}

	__forceinline ObjectIterator ObjectIterator::operator++() noexcept {
		if (currentStructural > this->iteratorCore->lastPosition()) {
			currentStructural = nullptr;
			return std::move(*this);
		}
		currentStructural = skipValue(currentStructural);
		setPosition(currentStructural);
		return std::move(*this);
	}

	__forceinline bool ObjectIterator::operator==(ObjectIterator& other) noexcept {
		auto returnValue = !isValuePresent(currentStructural, ',');
		while (*peek(currentStructural) == ']' || *peek(currentStructural) == '}') {
			++currentStructural;
		}
		setPosition(currentStructural);
		if (returnValue) {
			returnCurrentAndAdvance();
		}
		return returnValue;
	}

	__forceinline JsonifierResult<ObjectIterator>::JsonifierResult(ObjectIterator&& jsonData) noexcept
		: JsonifierResultBase<ObjectIterator>(std::forward<ObjectIterator>(jsonData)){};


	__forceinline JsonifierResult<ObjectIterator>::JsonifierResult(ErrorCode error) noexcept : JsonifierResultBase<ObjectIterator>(error){};

	__forceinline JsonifierResult<Field>& JsonifierResult<ObjectIterator>::operator*() noexcept {
		return second.operator*();
	}

	__forceinline JsonifierResult<ObjectIterator> JsonifierResult<ObjectIterator>::operator++() noexcept {
		++second;
		return std::move(second);
	}

	__forceinline bool JsonifierResult<ObjectIterator>::operator==(JsonifierResult<ObjectIterator>& other) noexcept {
		return second == other.second;
	}

};
