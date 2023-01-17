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
#include <jsonifier/Field.hpp>

namespace Jsonifier {

	class Jsonifier_Dll ObjectIterator : public NodeIterator {
	  public:
		__forceinline ObjectIterator() noexcept = default;

		__forceinline ObjectIterator(NodeIterator&& other) noexcept;

		__forceinline bool operator==(ObjectIterator& other) noexcept;

		__forceinline JsonifierResult<Field>& operator*() noexcept;

		__forceinline ObjectIterator operator++() noexcept;

	  protected:
		StructuralIndex currentStructural{};
		JsonifierResult<Field> data{};
	};

	template<> class JsonifierResult<ObjectIterator> : public JsonifierResultBase<ObjectIterator> {
	  public:
		__forceinline JsonifierResult() noexcept = default;

		__forceinline JsonifierResult(ObjectIterator&& jsonData) noexcept;

		__forceinline JsonifierResult(ErrorCode error) noexcept;

		__forceinline bool operator==(JsonifierResult<ObjectIterator>& other) noexcept;

		__forceinline JsonifierResult<ObjectIterator> operator++() noexcept;

		__forceinline JsonifierResult<Field>& operator*() noexcept;
	};
}