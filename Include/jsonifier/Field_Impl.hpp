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

#include <jsonifier/Field.hpp>
#include <jsonifier/Object.hpp>

namespace Jsonifier {

	__forceinline Field::Field(NodeIterator&& value) noexcept {
		rootStructural = value.rootStructural + 2;
		iteratorCore = value.iteratorCore;
		first = getString();
	};

	__forceinline JsonifierResult<size_t> Field::size() noexcept {
		return Object::size();
	}

	__forceinline std::string_view Field::key() noexcept {
		return first;
	}

	__forceinline Object Field::value() noexcept {
		return *reinterpret_cast<Object*>(this);
	}

	__forceinline JsonifierResult<Field>::JsonifierResult(Field&& JsonData) noexcept : JsonifierResultBase<Field>(std::forward<Field>(JsonData)){};

	__forceinline JsonifierResult<Field>::JsonifierResult(ErrorCode error) noexcept : JsonifierResultBase<Field>(error){};

	__forceinline JsonifierResult<std::string_view> JsonifierResult<Field>::key() noexcept {
		if (error()) {
			return error();
		}
		return second.key();
	}

	__forceinline JsonifierResult<JsonData> JsonifierResult<Field>::value() noexcept {
		if (error()) {
			return error();
		}
		return second.value();
	}

	__forceinline JsonifierResult<size_t> JsonifierResult<Field>::size() noexcept {
		if (error()) {
			return error();
		}
		return second.size();
	}

}