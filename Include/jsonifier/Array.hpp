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

	class Jsonifier_Dll Array : public JsonData {
	  public:
		__forceinline Array() noexcept = default;
		__forceinline Array(NodeIterator&&) noexcept;

		__forceinline JsonifierResult<std::string_view> getRawJsonString() noexcept;
		__forceinline JsonifierResult<JsonData> operator[](size_t) noexcept;
		__forceinline JsonifierResult<JsonData> at(size_t index) noexcept;
		__forceinline JsonifierResult<ArrayIterator> begin() noexcept;
		__forceinline JsonifierResult<ArrayIterator> end() noexcept;
		__forceinline JsonifierResult<size_t> size() noexcept;
	};

	template<> class JsonifierResult<Array> : public JsonifierResultBase<Array> {
	  public:
		__forceinline JsonifierResult(Array&& value) noexcept;
		__forceinline JsonifierResult(ErrorCode error) noexcept;

		__forceinline JsonifierResult<std::string_view> getRawJsonString() noexcept;
		__forceinline JsonifierResult<JsonData> operator[](size_t) noexcept;
		__forceinline JsonifierResult<JsonData> at(size_t index) noexcept;
		__forceinline JsonifierResult<ArrayIterator> begin() noexcept;
		__forceinline JsonifierResult<ArrayIterator> end() noexcept;
		__forceinline JsonifierResult<size_t> size() noexcept;
	};

}