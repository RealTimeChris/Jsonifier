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

	class ObjectIterator;

	class Jsonifier_Dll Object : public JsonData {
	  public:
		__forceinline Object() noexcept = default;
		__forceinline Object(NodeIterator&&) noexcept;

		__forceinline JsonifierResult<JsonData> operator[](std::string_view key) noexcept;
		__forceinline JsonifierResult<JsonData> operator[](const char* key) noexcept;
		__forceinline JsonifierResult<std::string_view> getRawJsonString() noexcept;
		__forceinline JsonifierResult<ObjectIterator> begin() noexcept;
		__forceinline JsonifierResult<ObjectIterator> end() noexcept;
		__forceinline JsonifierResult<size_t> size() noexcept;
	};

	template<> class JsonifierResult<Object> : public JsonifierResultBase<Object> {
	  public:
		__forceinline JsonifierResult(Object&& value) noexcept;
		__forceinline JsonifierResult(ErrorCode error) noexcept;

		__forceinline JsonifierResult<JsonData> operator[](std::string_view key) noexcept;
		__forceinline JsonifierResult<JsonData> operator[](const char* key) noexcept;
		__forceinline JsonifierResult<std::string_view> getRawJsonString() noexcept;
		__forceinline JsonifierResult<ObjectIterator> begin() noexcept;
		__forceinline JsonifierResult<ObjectIterator> end() noexcept;
		__forceinline JsonifierResult<size_t> size() noexcept;
	};

}