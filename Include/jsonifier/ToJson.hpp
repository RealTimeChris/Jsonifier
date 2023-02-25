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

	template<typename OTy = void> struct ToJson {};

	struct Write {
		template<class OTy> inline static void op(OTy& value, auto& buffer) {
			ToJson<std::decay_t<OTy>>::template op(value, buffer);
		}
	};

	template<JsonifierValueT OTy> struct ToJson<OTy> {
		template<typename OTy> inline static void op(OTy& value, auto& buffer) {
			using VTy = decltype(getMember(std::declval<OTy>(), CoreWrapperV<OTy>));
			ToJson<VTy>::template op(getMember(value, CoreWrapperV<OTy>), buffer);
		}
	};


}