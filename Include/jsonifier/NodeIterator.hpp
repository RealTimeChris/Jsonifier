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
/// Feb 3, 2023
/// https://github.com/RealTimeChris/Jsonifier
/// \file NodeIterator.hpp
#pragma once

#include <jsonifier/IteratorCore.hpp>

namespace Jsonifier {

	class Object;
	class Array;

	class Jsonifier_Dll NodeIterator  {
	  public:
		friend class ObjectIterator;
		friend class ArrayIterator;
		friend class JsonData;
		friend class Parser;
		friend class Object;
		friend class Field;
		friend class Array;

		__forceinline bool isValuePresent(StructuralIndex index, char character, int32_t offset = 0) noexcept;
		__forceinline StructuralIndex skipUntilClosed(StructuralIndex index) noexcept;
		__forceinline StructuralIndex skipValue(StructuralIndex index) noexcept;
		__forceinline JsonType type(StructuralIndex currentIndex) noexcept;
		__forceinline InStringPtr peek(StructuralIndex position) noexcept;
		__forceinline bool findField(const std::string_view key) noexcept;
		__forceinline StructuralIndex findElement(size_t key) noexcept;
		__forceinline InStringPtr returnCurrentAndAdvance() noexcept;
		__forceinline std::string_view getRawJsonString() noexcept;
		__forceinline void setPosition(StructuralIndex) noexcept;
		__forceinline bool parseNull(InStringPtr json) noexcept;
		__forceinline bool parseBool(InStringPtr json) noexcept;
		__forceinline std::string_view findNextKey() noexcept;
		__forceinline StructuralIndex rootPosition() noexcept;
		__forceinline std::string_view getString() noexcept;
		__forceinline IteratorCore* globalIter() noexcept;
		__forceinline StructuralIndex position() noexcept;
		__forceinline void setError(ErrorCode) noexcept;
		__forceinline void skipContinuation() noexcept;
		__forceinline operator NodeIterator() noexcept;
		__forceinline ErrorCode reportError() noexcept;
		__forceinline uint64_t getUint64() noexcept;
		__forceinline bool startedValue() noexcept;
		__forceinline double getDouble() noexcept;
		__forceinline int64_t getInt64() noexcept;
		__forceinline Object getObject() noexcept;
		__forceinline bool atGlobalEnd() noexcept;
		__forceinline bool resetValue() noexcept;
		__forceinline Array getArray() noexcept;
		__forceinline bool getBool() noexcept;
		__forceinline bool atRoot() noexcept;
		__forceinline bool isNull() noexcept;

	  protected:
		StructuralIndex rootStructural{};
		IteratorCore* iteratorCore{};
		
		__forceinline NodeIterator() noexcept = default;
		__forceinline NodeIterator(StructuralIndex, IteratorCore*) noexcept;
		__forceinline NodeIterator(IteratorCore* iteratorCoreNew) noexcept;
	};

}