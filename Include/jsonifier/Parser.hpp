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

#include <jsonifier/Simd.hpp>
#include <jsonifier/JsonData.hpp>

namespace Jsonifier {

	class Jsonifier_Dll Parser {
	  public:
		friend class IteratorCore;
		Parser& operator=(Parser&&) = delete;
		Parser(Parser&&) = delete;
		Parser& operator=(const Parser&) = delete;
		Parser(const Parser&) = delete;
		Parser(const std::string&) noexcept;
		Parser() noexcept = default;

		JsonifierResult<JsonData> parseJson(const char* string, size_t stringLength) noexcept;
		JsonifierResult<JsonData> parseJson(const std::string& string) noexcept;
		JsonifierResult<JsonData> parseJson(std::string_view string) noexcept;

	  protected:
		ObjectBuffer<uint32_t> structuralIndices{};
		ObjectBuffer<uint8_t> outString{};
		InStringPtr inString{};
		IteratorCore iterator{};
		size_t stringLengthRaw{};
		size_t allocatedSpace{};
		size_t tapeLength{};


		uint64_t round(int64_t array, int64_t n) noexcept;

		void generateJsonIndices() noexcept;

		ErrorCode allocate() noexcept;
	};
};
