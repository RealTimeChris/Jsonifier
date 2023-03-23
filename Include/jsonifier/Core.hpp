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
	License along with this library; if not, Serialize to the Free Software
	Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301
	USA
*/
/// NOTE: Most of the code in this header was sampled heavily from Glaze library: https://github.com/stephenberry/glaze
/// https://github.com/RealTimeChris/Jsonifier
/// Feb 3, 2023
#pragma once

#include <jsonifier/Tuple.hpp>
#include <variant>

namespace Jsonifier {

	/// A class to aid in registering a class/struct to be parsed/serialized.
	template<typename OTy> struct Core {};

	template<typename OTy>
	concept LocalConstructT = requires { OTy::construct; };

	template<typename OTy>
	concept GlobalConstructT = requires { Core<OTy>::construct; };

	template<typename OTy>
	concept LocalCoreT = requires { OTy::value; };

	template<typename OTy>
	concept GlobalCoreT = requires { Core<OTy>::value; };

	template<typename OTy>
	concept JsonifierT = requires { Core<std::decay_t<OTy>>::value; } || LocalCoreT<std::decay_t<OTy>>;

	struct EmptyVal {
		static constexpr Tuplet::Tuple<> value{};
	};

	template<typename OTy> inline constexpr auto CoreWrapperV = [] {
		if constexpr (LocalCoreT<OTy>) {
			return OTy::value;
		} else if constexpr (GlobalCoreT<OTy>) {
			return Core<OTy>::value;
		} else {
			return EmptyVal{};
		}
	}();

	template<typename OTy> inline constexpr auto CoreV = CoreWrapperV<std::decay_t<OTy>>.value;

	template<typename OTy> using CoreT = std::decay_t<decltype(CoreV<OTy>)>;

	template<typename OTy> using CoreWrapperT = std::decay_t<decltype(CoreWrapperV<std::decay_t<OTy>>)>;

}
