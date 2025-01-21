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

	template<typename base_type, size_t size> struct switch_statement;

	template<typename base_type, size_t size>
		requires(size <= 12)
	struct switch_statement<base_type, size> : public base_type {
		template<typename value_type, typename context_type> JSONIFIER_INLINE static uint64_t impl(value_type& value, context_type& context, size_t index) noexcept {
			JSONIFIER_SWITCH_START(3, 0, 1, 2)
			JSONIFIER_SWITCH(4, 0, 1, 2, 3)
			JSONIFIER_SWITCH(5, 0, 1, 2, 3, 4)
			JSONIFIER_SWITCH(6, 0, 1, 2, 3, 4, 5)
			JSONIFIER_SWITCH(7, 0, 1, 2, 3, 4, 5, 6)
			JSONIFIER_SWITCH(8, 0, 1, 2, 3, 4, 5, 6, 7)
			JSONIFIER_SWITCH(9, 0, 1, 2, 3, 4, 5, 6, 7, 8)
			JSONIFIER_SWITCH(10, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9)
			JSONIFIER_SWITCH(11, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10)
			JSONIFIER_SWITCH(12, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11) else {
				return false;
			}
		}
	};

}