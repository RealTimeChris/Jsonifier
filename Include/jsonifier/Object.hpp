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
#include <jsonifier/HashMap.hpp>
#include <jsonifier/ObjectIterator.hpp>
#include <jsonifier/JsonDataBase.hpp>

namespace Jsonifier {

	class Jsonifier_Dll Object : public JsonDataBase {
	  public:
		friend class ObjectIterator;
		friend class JsonDataBase;
		friend class Array;
		__forceinline Object() noexcept;
		__forceinline Object(IteratorCore*, StructuralIndex) noexcept;
		__forceinline bool contains(const char*) noexcept;
		__forceinline JsonData& operator[](const char* key) noexcept;
		__forceinline ObjectIterator begin() noexcept;
		__forceinline ObjectIterator end() noexcept;

	  protected:
		HashMap<JsonData, StringView> data{ 3 };

		__forceinline StructuralIndex parseJson() noexcept;
	};
}