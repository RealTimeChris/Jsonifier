/*
	MIT License

	Copyright (c) 2023 RealTimeChris

	Permission is hereby granted, free of charge, to any person obtaining a copy of this
	software and associated documentation files (the "Software"), to deal in the Software
	without restriction, including without limitation the rights to use, copy, modify, merge,
	publish, distribute, sublicense, and/or sell copies of the Software, and to permit
	persons to whom the Software is furnished to do so, subject to the following conditions:

	The above copyright notice and this permission notice shall be included in all copies or
	substantial portions of the Software.

	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
	INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
	PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE
	FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
	OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
	DEALINGS IN THE SOFTWARE.
*/
/// https://github.com/RealTimeChris/Jsonifier
/// Feb 3, 2023
#pragma once

#include <jsonifier/Tuple.hpp>
#include <variant>

namespace Jsonifier {
	/// A class to aid in registering a class/struct to be parsed/serialized.
	template<typename ValueType> struct Core {};
}

namespace JsonifierInternal {

	template<typename ValueType>
	concept JsonifierT = requires { Jsonifier::Core<std::decay_t<ValueType>>::parseValue; };

	struct EmptyVal {
		static inline constexpr Tuplet::Tuple<> parseValue{};
	};

	template<typename ValueType> inline constexpr auto CoreWrapperV = [] {
		if constexpr (JsonifierT<ValueType>) {
			return Jsonifier::Core<ValueType>::parseValue;
		} else {
			return EmptyVal{};
		}
	}();

	template<typename ValueType> inline constexpr auto CoreV = CoreWrapperV<std::decay_t<ValueType>>.parseValue;

	template<typename ValueType> using CoreT = std::decay_t<decltype(CoreV<ValueType>)>;

	template<typename ValueType> using CoreWrapperT = std::decay_t<decltype(CoreWrapperV<std::decay_t<ValueType>>)>;
}
