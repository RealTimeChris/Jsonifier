/*
	MIT License

	Copyright (c) 2024 RealTimeChris

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
/// Feb 3, 2023
#pragma once

#include <cstdint>
#include <atomic>

#if JSONIFIER_COMPILER_MSVC
	#define JSONIFIER_VISUAL_STUDIO 1
	#if JSONIFIER_COMPILER_CLANG
		#define JSONIFIER_CLANG_VISUAL_STUDIO 1
	#else
		#define JSONIFIER_REGULAR_VISUAL_STUDIO 1
	#endif
#endif

#if defined(__has_builtin)
	#define JSONIFIER_HAS_BUILTIN(x) __has_builtin(x)
#else
	#define JSONIFIER_HAS_BUILTIN(x) 0
#endif

#if !defined(JSONIFIER_LIKELY)
	#define JSONIFIER_LIKELY(...) (__VA_ARGS__) [[likely]]
#endif

#if !defined(JSONIFIER_UNLIKELY)
	#define JSONIFIER_UNLIKELY(...) (__VA_ARGS__) [[unlikely]]
#endif

#if !defined(JSONIFIER_ELSE_UNLIKELY)
	#define JSONIFIER_ELSE_UNLIKELY(...) __VA_ARGS__ [[unlikely]]
#endif

#if !defined JSONIFIER_ALIGN
	#define JSONIFIER_ALIGN(b) alignas(b)
#endif

namespace jsonifier::internal {

	enum class json_structural_characters : char {
		post_primitive_state = 0,
		l_crl_bracket		 = '{',
		r_crl_bracket		 = '}',
		l_sqr_bracket		 = '[',
		r_sqr_bracket		 = ']',
		colon				 = ':',
		comma				 = ',',
		key_start			 = '"',
	};	

	enum class parse_statuses : uint8_t {
		success,
		missing_object_start,
		missing_object_end,
		missing_array_start,
		missing_array_end,
		invalid_string_characters,
		missing_colon,
		missing_comma,
		invalid_number_value,
		invalid_null_value,
		invalid_bool_value,
		no_input,
		unfinished_input,
		unexpected_string_end,
		unexpected_end_of_input,
		unexpected_token,
		illegal_control_character,
		count,
	};

}
