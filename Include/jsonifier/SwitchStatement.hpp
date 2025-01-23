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

#define JSONIFIER_CASE(I) \
	case I: { \
		return base_type::template processIndex<I>(value, context); \
	}

#define JSONIFIER_SWITCH_START(X, ...) \
	if constexpr (size == X) { \
		switch (index) { \
			JSONIFIER_EVERY(JSONIFIER_CASE, __VA_ARGS__); \
			default: { \
				return false; \
			} \
		} \
	}

#define JSONIFIER_SWITCH(X, ...) \
	else if constexpr (size == X) { \
		switch (index) { \
			JSONIFIER_EVERY(JSONIFIER_CASE, __VA_ARGS__); \
			default: { \
				return false; \
			} \
		} \
	}

#define JSONIFIER_10 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10
#define JSONIFIER_20 JSONIFIER_10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20
#define JSONIFIER_30 JSONIFIER_20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30
#define JSONIFIER_40 JSONIFIER_30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40
#define JSONIFIER_50 JSONIFIER_40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50
#define JSONIFIER_60 JSONIFIER_50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60

	template<typename base_type, size_t size> struct switch_statement;

	template<typename base_type, size_t size>
		requires(size <= 64)
	struct switch_statement<base_type, size> : public base_type {
		template<typename value_type, typename context_type> static uint64_t impl(value_type& value, context_type& context, size_t index) noexcept {
			JSONIFIER_SWITCH_START(3, 0, 1, 2)
			JSONIFIER_SWITCH(4, 0, 1, 2, 3)
			JSONIFIER_SWITCH(5, 0, 1, 2, 3, 4)
			JSONIFIER_SWITCH(6, 0, 1, 2, 3, 4, 5)
			JSONIFIER_SWITCH(7, 0, 1, 2, 3, 4, 5, 6)
			JSONIFIER_SWITCH(8, 0, 1, 2, 3, 4, 5, 6, 7)
			JSONIFIER_SWITCH(9, 0, 1, 2, 3, 4, 5, 6, 7, 8)
			JSONIFIER_SWITCH(10, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9)
			JSONIFIER_SWITCH(11, JSONIFIER_10)
			JSONIFIER_SWITCH(12, JSONIFIER_10, 11)
			JSONIFIER_SWITCH(13, JSONIFIER_10, 11, 12)
			JSONIFIER_SWITCH(14, JSONIFIER_10, 11, 12, 13)
			JSONIFIER_SWITCH(15, JSONIFIER_10, 11, 12, 13, 14)
			JSONIFIER_SWITCH(16, JSONIFIER_10, 11, 12, 13, 14, 15)
			JSONIFIER_SWITCH(17, JSONIFIER_10, 11, 12, 13, 14, 15, 16)
			JSONIFIER_SWITCH(18, JSONIFIER_10, 11, 12, 13, 14, 15, 16, 17)
			JSONIFIER_SWITCH(19, JSONIFIER_10, 11, 12, 13, 14, 15, 16, 17, 18)
			JSONIFIER_SWITCH(20, JSONIFIER_10, 11, 12, 13, 14, 15, 16, 17, 18, 19)
			JSONIFIER_SWITCH(21, JSONIFIER_20)
			JSONIFIER_SWITCH(22, JSONIFIER_20, 21)
			JSONIFIER_SWITCH(23, JSONIFIER_20, 21, 22)
			JSONIFIER_SWITCH(24, JSONIFIER_20, 21, 22, 23)
			JSONIFIER_SWITCH(25, JSONIFIER_20, 21, 22, 23, 24)
			JSONIFIER_SWITCH(26, JSONIFIER_20, 21, 22, 23, 24, 25)
			JSONIFIER_SWITCH(27, JSONIFIER_20, 21, 22, 23, 24, 25, 26)
			JSONIFIER_SWITCH(28, JSONIFIER_20, 21, 22, 23, 24, 25, 26, 27)
			JSONIFIER_SWITCH(29, JSONIFIER_20, 21, 22, 23, 24, 25, 26, 27, 28)
			JSONIFIER_SWITCH(30, JSONIFIER_20, 21, 22, 23, 24, 25, 26, 27, 28, 29)
			JSONIFIER_SWITCH(31, JSONIFIER_30)
			JSONIFIER_SWITCH(32, JSONIFIER_30, 31) else {
				return false;
			}
		}
	};

}