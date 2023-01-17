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
#include <jsonifier/Object.hpp>

namespace Jsonifier {

	class Jsonifier_Dll Field : public Object {
	  public:
		friend class ObjectIterator;

		__forceinline Field() noexcept = default;

		__forceinline JsonifierResult<size_t> size() noexcept;
		__forceinline std::string_view key() noexcept;
		__forceinline Object value() noexcept;

	  protected:
		std::string_view first{};

		__forceinline Field(NodeIterator&&) noexcept;
	};

	template<> class JsonifierResult<Field> : public JsonifierResultBase<Field> {
	  public:
		friend class ObjectIterator;
		__forceinline JsonifierResult() noexcept = default;
		__forceinline JsonifierResult(Field&& value) noexcept;
		__forceinline JsonifierResult(ErrorCode error) noexcept;

		__forceinline JsonifierResult<std::string_view> key() noexcept;
		__forceinline JsonifierResult<JsonData> value() noexcept;
		__forceinline JsonifierResult<size_t> size() noexcept;
	};

}
