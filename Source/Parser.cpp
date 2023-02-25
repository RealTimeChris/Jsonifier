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
/// \file Parser.cpp

#include <jsonifier/Jsonifier.hpp>

namespace Jsonifier {

	JsonDataBase Parser::parseJson(std::string_view string) noexcept {
		if (string.size() == 0) {
			iterator.setError(No_String_Error);
			return &iterator;
		}
		inString = reinterpret_cast<StringViewPtr>(string.data());
		stringLengthRaw = string.size();
		if (allocatedSpace < round(5 * stringLengthRaw / 3 + 256, 256)) {
			if (allocate() != Success) {
				iterator.setError(Mem_Alloc_Error);
				return &iterator;
			}
		}
		generateJsonIndices();
		iterator.reset(this);
		return std::forward<JsonDataBase>(&iterator);
	}

	JsonDataBase Parser::parseJson(const char* string, size_t stringLength) noexcept {
		if (stringLength == 0) {
			iterator.setError(No_String_Error);
			return &iterator;
		}
		inString = reinterpret_cast<StringViewPtr>(string);
		stringLengthRaw = stringLength;
		if (allocatedSpace < round(5 * stringLengthRaw / 3 + 256, 256)) {
			if (allocate() != Success) {
				iterator.setError(Mem_Alloc_Error);
				return &iterator;
			}
		}
		generateJsonIndices();
		iterator.reset(this);
		return std::forward<JsonDataBase>(&iterator);
	}

	JsonDataBase Parser::parseJson(const std::string& string) noexcept {
		if (string.size() == 0) {
			iterator.setError(No_String_Error);
			return &iterator;
		}
		inString = reinterpret_cast<StringViewPtr>(string.data());
		stringLengthRaw = string.size();
		if (allocatedSpace < round(5 * stringLengthRaw / 3 + 256, 256)) {
			if (allocate() != Success) {
				iterator.setError(Mem_Alloc_Error);
				return &iterator;
			}
		}
		generateJsonIndices();
		iterator.reset(this);
		return std::forward<JsonDataBase>(&iterator);
	}

	uint64_t Parser::round(int64_t a, int64_t size) noexcept {
		return (((a) + (( size )-1)) & ~(( size )-1));
	}

	ErrorCode Parser::allocate() noexcept {
		if (stringLengthRaw == 0) {
			return ErrorCode{ No_String_Error };
		}
		outString.reset(round(5 * stringLengthRaw / 3 + 256, 256));
		structuralIndices.reset(round(stringLengthRaw + 3, 256));
		allocatedSpace = round(5 * stringLengthRaw / 3 + 256, 256);
		if (!(outString && structuralIndices)) {
			outString.reset(0);
			return ErrorCode{ Mem_Alloc_Error };
		}

		return Success;
	}

	void Parser::generateJsonIndices() noexcept {
		if (inString) {
			StringBlockReader<256> stringReader{ inString, stringLengthRaw };
			SimdStringReader section{ structuralIndices };
			tapeLength = 0;
			while (stringReader.hasFullBlock()) {
				section.generateStructurals(stringReader.fullBlock());
			}
			uint8_t block[256];
			stringReader.getRemainder(block);
			section.generateStructurals(block);
			tapeLength = section.getTapeLength();
		}
	}
}
