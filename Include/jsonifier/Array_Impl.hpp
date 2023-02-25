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

#include <jsonifier/Array.hpp>

namespace Jsonifier {

	__forceinline Array::Array(IteratorCore* iteratorCoreNew, StructuralIndex indexNew) noexcept {
		this->iteratorCore = iteratorCoreNew;
		this->rootStructural = indexNew;
	}

	__forceinline JsonData& Array::operator[](size_t index) noexcept {
		return at(index);
	}

	__forceinline JsonData& Array::at(size_t index) noexcept {
		return data[index];
	}

	__forceinline ArrayIterator Array::begin() noexcept {
		if (!peek(rootStructural) || *peek(rootStructural) != '[') {
			setError(Incorrect_Type);
			return ArrayIterator{ VectorIterator<JsonData>{ data.data() } };
		}
		return ArrayIterator{ VectorIterator<JsonData>{ data.data() } };
	}

	__forceinline ArrayIterator Array::end() noexcept {
		return ArrayIterator{ VectorIterator<JsonData>{ data.data() + sizeVal } };
	}

	__forceinline StructuralIndex Array::parseJson() noexcept {
		auto index = rootStructural;
		++index;
		size_t openCount{ 1 };
		size_t closeCount{ 0 };
		if (index >= iteratorCore->endPosition() || *peek(index + 1) == '}' ||
			*peek(index) == 'n') {
			return index;
		}
		while (index < iteratorCore->endPosition() && openCount > closeCount) {
			if (index >= iteratorCore->endPosition()) {
				return index;
			}

			switch (*peek(index)) {
				case '{': {
					auto newObject = new Object{ iteratorCore, index };
					index = newObject->parseJson();
					data.emplaceBack(JsonData{ newObject });
					++sizeVal;
					break;
				}
				case '[': {
					auto newObject = new Array{ iteratorCore, index };
					index = newObject->parseJson();
					data.emplaceBack(JsonData{ newObject });
					++sizeVal;
					break;
				}
				case '}': {
					++closeCount;
					if (closeCount >= openCount) {
						return index;
					}
					break;
				}
				case ']': {
					++closeCount;
					if (closeCount >= openCount) {
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
					data.emplaceBack(JsonData{ new JsonDataBase{ iteratorCore, index } });
					++sizeVal;
					++index;
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
