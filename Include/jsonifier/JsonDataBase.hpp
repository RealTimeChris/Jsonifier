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
#include <jsonifier/StringView.hpp>

namespace Jsonifier {

	class Jsonifier_Dll JsonDataBase {
	  public:
		friend class JsonData;
		friend class Element;
		friend class Object;
		friend class Array;

		__forceinline JsonDataBase() noexcept = default;
		__forceinline JsonDataBase(IteratorCore*, StructuralIndex) noexcept;
		__forceinline JsonDataBase(IteratorCore* iteratorCoreNew) noexcept;

		__forceinline operator ErrorCode() noexcept;
		template<typename VTy> __forceinline ErrorCode get(VTy& out) noexcept;
		__forceinline operator std::string_view() noexcept;
		__forceinline operator std::string() noexcept;
		__forceinline StringView getRawJsonString() noexcept;
		__forceinline JsonType type() noexcept;
		__forceinline operator uint64_t() noexcept;
		__forceinline operator int64_t() noexcept;
		__forceinline operator double() noexcept;
		__forceinline operator Object() noexcept;
		__forceinline operator Array() noexcept;
		__forceinline operator bool() noexcept;
		__forceinline size_t size() noexcept;

	  protected:
		StructuralIndex rootStructural{};
		IteratorCore* iteratorCore{};
		size_t sizeVal{};

		__forceinline StructuralIndex skipUntilClosed(StructuralIndex index) noexcept;
		__forceinline StringView getStringKey(StructuralIndex indexNew) noexcept;
		__forceinline StructuralIndex skipValue(StructuralIndex index) noexcept;
		__forceinline StringViewPtr peek(StructuralIndex position) noexcept;
		__forceinline JsonType type(StructuralIndex currentIndex) noexcept;
		__forceinline bool parseNull(StringViewPtr json) noexcept;
		__forceinline bool parseBool(StringViewPtr json) noexcept;
		__forceinline std::string_view getString() noexcept;
		__forceinline void setError(ErrorCode) noexcept;
		__forceinline ErrorCode reportError() noexcept;
		__forceinline uint64_t getUint64() noexcept;
		__forceinline double getDouble() noexcept;
		__forceinline int64_t getInt64() noexcept;
		__forceinline Object getObject() noexcept;
		__forceinline Array getArray() noexcept;
		__forceinline bool getBool() noexcept;
	};
}
