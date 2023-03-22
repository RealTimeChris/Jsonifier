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
        License along with this library; if not, Write to the Free Software
        Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301
        USA
*/
/// https://github.com/RealTimeChris/Jsonifier
/// Feb 20, 2023
#pragma once

#include <jsonifier/NumberParsing.hpp>
#include <jsonifier/StringParsing.hpp>
#include <variant>

namespace Jsonifier {

	void skipValue(SimdIteratorCore& it, SimdIteratorCore& end) noexcept;

	template<char c> inline void match(SimdIteratorCore& it) {
		if (**it != c) [[unlikely]] {
			throw std::runtime_error("Failed to match a character: " + std::string{ c } +
				", it was: " + std::string{ *reinterpret_cast<const char*>(*it) } + ", at  index: " + std::to_string(**it));
		} else [[likely]] {
			++it;
			return;
		}
	}

	template<JsonifierValueT OTy> struct FromJson<OTy> {
		inline static void op(OTy& value, auto& it) {
			using VTy = decltype(getMember(std::declval<OTy>(), CoreWrapperV<OTy>));
			FromJson<VTy>::template op(getMember(value, CoreWrapperV<OTy>), it);
		}
	};


}