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

#include <jsonifier/Object.hpp>
#include <jsonifier/ObjectIterator.hpp>

namespace Jsonifier {

	__forceinline Object::Object() noexcept {};

	__forceinline Object::Object(IteratorCore*iteratorCoreNew, StructuralIndex indexNew) noexcept {
		this->iteratorCore = iteratorCoreNew;
		this->rootStructural = indexNew;
	}

	__forceinline bool Object::contains(const char* key) noexcept {
		return this->data.contains(key);
	}

	__forceinline JsonData& Object::operator[](const char* key) noexcept {
		return data.at(key).accessFirst();
	}

	__forceinline ObjectIterator Object::begin() noexcept {
		if (!peek(rootStructural) || *peek(rootStructural) != '{') {
			setError(Incorrect_Type);
			return ObjectIterator{ static_cast<Object*>(this) };
		}
		return ObjectIterator{ static_cast<Object*>(this) };
	}

	__forceinline ObjectIterator Object::end() noexcept {
		return {};
	}

	__forceinline StructuralIndex Object::parseJson() noexcept {
		auto index = rootStructural;
		++index;
		size_t openCount{ 1 };
		size_t closeCount{ 0 };
		if (index >= iteratorCore->endPosition() || iteratorCore->getStringView()[*(index + 1)] == '}' ||
			iteratorCore->getStringView()[*(index)] == 'n') {
			return index;
		}
		while (index < iteratorCore->endPosition() && openCount > closeCount) {
			if (index >= iteratorCore->endPosition()) {
				return index;
			}
			switch (iteratorCore->getStringView()[*(index)]) {
				case '{': {
					++openCount;
					auto keyStart = index - 2;
					StringView key{ getStringKey(keyStart) };
					if (key.size() > 0) {
						Pair<JsonData, StringView> pair{ new Object{ iteratorCore, index }, std::move(key) };
						index = pair.accessFirst().operator Object&().parseJson();
						data.emplace(std::move(pair));
						++sizeVal;
					}
					break;
				}
				case '[': {
					++openCount;
					auto keyStart = index - 2;
					StringView key{ getStringKey(keyStart) };
					if (key.size() > 0) {
						Pair<JsonData, StringView> pair{ new Array{ iteratorCore, index }, std::move(key) };
						index = pair.accessFirst().operator Array&().parseJson();
						data.emplace(std::move(pair));
						++sizeVal;
					}
					break;
				}
				case '}': {
					++closeCount;
					if (closeCount + 2 >= openCount) {
						return index;
					}
					break;
				}
				case ']': {
					++closeCount;
					if (closeCount + 2 >= openCount) {
						return index;
					}
					break;
				}
				case '0':
				case '1':
				case '2':
				case '3':
				case '4':
				case '5':
				case '6':
				case '7':
				case '8':
				case '9':
				case 't':
				case 'f':
				case 'n':
				case '"':
				case '-': {
					if (this->iteratorCore->getStringView()[*(index - 1)] == ':') {
						auto keyStart = index - 2;
						StringView key{ getStringKey(keyStart) };
						if (key.size() > 0) {
							Pair<JsonData, StringView> pair{ new JsonDataBase{ iteratorCore, index }, std::move(key) };
							data.emplace(std::move(pair));
						}
					}
					break;
				}
				default: {
				};
			}
			++index;
		}
		return index;
	}

}
