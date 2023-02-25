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

	class Jsonifier_Dll Array : public JsonDataBase {
	  public:
		friend class ArrayIterator;
		friend class JsonDataBase;
		friend class Object;

		__forceinline Array() noexcept = default;
		__forceinline Array(IteratorCore*, StructuralIndex) noexcept;

		__forceinline virtual JsonData& operator[](size_t) noexcept;
		__forceinline virtual JsonData& at(size_t index) noexcept;
		__forceinline virtual ArrayIterator begin() noexcept;
		__forceinline virtual ArrayIterator end() noexcept;

	  protected:
		Vector<JsonData> data{ 30 };
		__forceinline StructuralIndex parseJson() noexcept;
	};

}