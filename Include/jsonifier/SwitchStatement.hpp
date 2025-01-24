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
/// https://github.com/RealTimeChris/jsonifier
/// Feb 20, 2023
#pragma once

#include <jsonifier/Config.hpp>
#include <jsonifier/Core.hpp>

namespace jsonifier::internal {

// Sampled from Stephen Berry and his library, Glaze: https://github.com/stephenberry/Glaze
#define JSONIFIER_PARENS ()

#define JSONIFIER_EXPAND(...) JSONIFIER_EXPAND32(__VA_ARGS__)
#define JSONIFIER_EXPAND32(...) JSONIFIER_EXPAND16(JSONIFIER_EXPAND16(__VA_ARGS__))
#define JSONIFIER_EXPAND16(...) JSONIFIER_EXPAND8(JSONIFIER_EXPAND8(__VA_ARGS__))
#define JSONIFIER_EXPAND8(...) JSONIFIER_EXPAND4(JSONIFIER_EXPAND4(__VA_ARGS__))
#define JSONIFIER_EXPAND4(...) JSONIFIER_EXPAND2(JSONIFIER_EXPAND2(__VA_ARGS__))
#define JSONIFIER_EXPAND2(...) JSONIFIER_EXPAND1(JSONIFIER_EXPAND1(__VA_ARGS__))
#define JSONIFIER_EXPAND1(...) __VA_ARGS__

#define JSONIFIER_FOR_EACH(macro, ...) __VA_OPT__(JSONIFIER_EXPAND(JSONIFIER_FOR_EACH_HELPER(macro, __VA_ARGS__)))
#define JSONIFIER_FOR_EACH_HELPER(macro, a, ...) macro(a) __VA_OPT__(, ) __VA_OPT__(JSONIFIER_FOR_EACH_AGAIN JSONIFIER_PARENS(macro, __VA_ARGS__))
#define JSONIFIER_FOR_EACH_AGAIN() JSONIFIER_FOR_EACH_HELPER

#define JSONIFIER_EVERY(macro, ...) __VA_OPT__(JSONIFIER_EXPAND(JSONIFIER_EVERY_HELPER(macro, __VA_ARGS__)))
#define JSONIFIER_EVERY_HELPER(macro, a, ...) macro(a) __VA_OPT__(JSONIFIER_EVERY_AGAIN JSONIFIER_PARENS(macro, __VA_ARGS__))
#define JSONIFIER_EVERY_AGAIN() JSONIFIER_EVERY_HELPER

#define JSONIFIER_INVOKE(I) \
	base_type::template processIndex<I>(value, context); \

#define JSONIFIER_INVOKE_ALL_START(X, ...) \
	if constexpr (size == X) { \
		JSONIFIER_EVERY(JSONIFIER_INVOKE, __VA_ARGS__) \
	}

#define JSONIFIER_INVOKE_ALL(X, ...) \
	else if constexpr (size == X) { \
		JSONIFIER_EVERY(JSONIFIER_INVOKE, __VA_ARGS__) \
	}

#define JSONIFIER_10 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10
#define JSONIFIER_20 JSONIFIER_10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20
#define JSONIFIER_30 JSONIFIER_20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30
#define JSONIFIER_40 JSONIFIER_30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40
#define JSONIFIER_50 JSONIFIER_40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50
#define JSONIFIER_60 JSONIFIER_50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60

	template<typename base_type, size_t size> struct invoke_statement;

	template<typename base_type, size_t size>
		requires(size <= 64)
	struct invoke_statement<base_type, size> : public base_type {
		template<typename value_type, typename context_type> static uint64_t impl(value_type& value, context_type& context, size_t index) noexcept {
			JSONIFIER_INVOKE_ALL_START(3, 0, 1, 2)
			JSONIFIER_INVOKE_ALL(4, 0, 1, 2, 3)
			JSONIFIER_INVOKE_ALL(5, 0, 1, 2, 3, 4)
			JSONIFIER_INVOKE_ALL(6, 0, 1, 2, 3, 4, 5)
			JSONIFIER_INVOKE_ALL(7, 0, 1, 2, 3, 4, 5, 6)
			JSONIFIER_INVOKE_ALL(8, 0, 1, 2, 3, 4, 5, 6, 7)
			JSONIFIER_INVOKE_ALL(9, 0, 1, 2, 3, 4, 5, 6, 7, 8)
			JSONIFIER_INVOKE_ALL(10, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9)
			JSONIFIER_INVOKE_ALL(11, JSONIFIER_10)
			JSONIFIER_INVOKE_ALL(12, JSONIFIER_10, 11)
			JSONIFIER_INVOKE_ALL(13, JSONIFIER_10, 11, 12)
			JSONIFIER_INVOKE_ALL(14, JSONIFIER_10, 11, 12, 13)
			JSONIFIER_INVOKE_ALL(15, JSONIFIER_10, 11, 12, 13, 14)
			JSONIFIER_INVOKE_ALL(16, JSONIFIER_10, 11, 12, 13, 14, 15)
			JSONIFIER_INVOKE_ALL(17, JSONIFIER_10, 11, 12, 13, 14, 15, 16)
			JSONIFIER_INVOKE_ALL(18, JSONIFIER_10, 11, 12, 13, 14, 15, 16, 17)
			JSONIFIER_INVOKE_ALL(19, JSONIFIER_10, 11, 12, 13, 14, 15, 16, 17, 18)
			JSONIFIER_INVOKE_ALL(20, JSONIFIER_10, 11, 12, 13, 14, 15, 16, 17, 18)
			JSONIFIER_INVOKE_ALL(21, JSONIFIER_20)
			JSONIFIER_INVOKE_ALL(22, JSONIFIER_20, 21)
			JSONIFIER_INVOKE_ALL(23, JSONIFIER_20, 21, 22)
			JSONIFIER_INVOKE_ALL(24, JSONIFIER_20, 21, 22, 23)
			JSONIFIER_INVOKE_ALL(25, JSONIFIER_20, 21, 22, 23, 24)
			JSONIFIER_INVOKE_ALL(26, JSONIFIER_20, 21, 22, 23, 24, 25)
			JSONIFIER_INVOKE_ALL(27, JSONIFIER_20, 21, 22, 23, 24, 25, 26)
			JSONIFIER_INVOKE_ALL(28, JSONIFIER_20, 21, 22, 23, 24, 25, 26, 27)
			JSONIFIER_INVOKE_ALL(29, JSONIFIER_20, 21, 22, 23, 24, 25, 26, 27, 28)
			JSONIFIER_INVOKE_ALL(30, JSONIFIER_20, 21, 22, 23, 24, 25, 26, 27, 28, 29)
			JSONIFIER_INVOKE_ALL(31, JSONIFIER_30)
			JSONIFIER_INVOKE_ALL(32, JSONIFIER_30, 31) else {
				return false;
			}
		}
	};

#undef JSONIFIER_INVOKE
#undef JSONIFIER_INVOKE_ALL

}