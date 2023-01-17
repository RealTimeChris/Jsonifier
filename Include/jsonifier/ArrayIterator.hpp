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

#include <jsonifier/Base.hpp>

namespace Jsonifier {

	class JsonData;

	class Jsonifier_Dll ArrayIterator : public NodeIterator {
	  public:
		__forceinline ArrayIterator() noexcept = default;

		__forceinline ArrayIterator(NodeIterator&& other) noexcept;

		__forceinline JsonifierResult<JsonData>& operator*() noexcept;

		__forceinline bool operator==(ArrayIterator& other) noexcept;

		__forceinline ArrayIterator operator++() noexcept;

	  protected:
		StructuralIndex currentStructural{};
		JsonifierResult<JsonData> data{};
	};

	__forceinline NodeIterator::operator NodeIterator() noexcept {
		return NodeIterator{ position(), globalIter() };
	}

	template<> class JsonifierResult<ArrayIterator> : public JsonifierResultBase<ArrayIterator> {
	  public:
		__forceinline JsonifierResult() noexcept = default;

		__forceinline JsonifierResult(ArrayIterator&& jsonData) noexcept;

		__forceinline JsonifierResult(ErrorCode error) noexcept;

		__forceinline bool operator==(JsonifierResult<ArrayIterator>& other) noexcept;

		__forceinline JsonifierResult<ArrayIterator> operator++() noexcept;

		__forceinline JsonifierResult<JsonData>& operator*() noexcept;
	};
}