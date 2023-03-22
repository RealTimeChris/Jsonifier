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

	template<class OTy> struct Core {};

	template<class OTy>
	concept LocalConstructT = requires {
		OTy::construct;
	};

	template<class OTy>
	concept GlobalConstructT = requires {
		Core<OTy>::construct;
	};

	template<class OTy>
	concept LocalCoreT = requires {
		OTy::value;
	};

	template<class OTy>
	concept GlobalCoreT = requires {
		Core<OTy>::value;
	};

	template<class OTy>
	concept JsonifierT = requires {
		Core<std::decay_t<OTy>>::value;
	}
	|| LocalCoreT<std::decay_t<OTy>>;

	struct Empty {
		static constexpr Tuplet::Tuple<> value{};
	};

	template<class OTy> inline constexpr auto CoreWrapperV = [] {
		if constexpr (LocalCoreT<OTy>) {
			return OTy::value;
		} else if constexpr (GlobalCoreT<OTy>) {
			return Core<OTy>::value;
		} else {
			return Empty{};
		}
	}();

	template<class OTy> inline constexpr auto CoreV = CoreWrapperV<std::decay_t<OTy>>.value;

	template<class OTy> using CoreT = std::decay_t<decltype(CoreV<OTy>)>;

	template<class OTy> using CoreWrapperT = std::decay_t<decltype(CoreWrapperV<std::decay_t<OTy>>)>;

}
