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

#include <jsonifier/NodeIterator.hpp>

namespace Jsonifier {

	class ArrayIterator;

	class Jsonifier_Dll JsonData : protected NodeIterator {
	  public:
		friend class ArrayIterator;
		friend class NodeIterator;
		friend class Parser;
		friend class Object;
		friend class Array;

		__forceinline JsonData() noexcept = default;

		__forceinline JsonifierResult<std::string_view> getRawJsonString() noexcept;
		template<typename OTy> __forceinline ErrorCode get(OTy& out) noexcept;
		__forceinline operator std::string_view() noexcept(false);
		__forceinline JsonifierResult<JsonType> type() noexcept;
		__forceinline operator std::string() noexcept(false);
		__forceinline operator uint64_t() noexcept(false);
		__forceinline operator int64_t() noexcept(false);
		__forceinline operator Object() noexcept(false);
		__forceinline operator double() noexcept(false);
		__forceinline operator Array() noexcept(false);
		__forceinline operator bool() noexcept(false);

	  protected:
		__forceinline JsonData(NodeIterator&& iteratorNew) noexcept;
	};

	template<> class JsonifierResult<JsonData> : public JsonifierResultBase<JsonData> {
	  public:
		friend class ArrayIterator;
		friend class Array;

		__forceinline JsonifierResult() noexcept = default;

		__forceinline JsonifierResult(JsonData&& value) noexcept;
		__forceinline JsonifierResult(ErrorCode error) noexcept;


		__forceinline JsonifierResult<std::string_view> getRawJsonString() noexcept;
		template<typename OTy> __forceinline ErrorCode get(OTy& out) noexcept;
		__forceinline operator std::string_view() noexcept(false);
		__forceinline JsonifierResult<JsonType> type() noexcept;
		__forceinline operator std::string() noexcept(false);
		__forceinline operator uint64_t() noexcept(false);
		__forceinline operator int64_t() noexcept(false);
		__forceinline operator Object() noexcept(false);
		__forceinline operator double() noexcept(false);
		__forceinline operator Array() noexcept(false);
		__forceinline operator bool() noexcept(false);
	};

}
