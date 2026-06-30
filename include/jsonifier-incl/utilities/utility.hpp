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
#pragma once

#include <jsonifier-incl/utilities/concepts.hpp>
#include <cstdint>
#include <vector>

namespace jsonifier::internal {

	template<template<auto> typename functor_type, typename integer_sequence> struct functor_executor;

	template<template<auto> typename functor_type, concepts::uint64_types auto... indices> struct functor_executor<functor_type, integer_sequence<indices...>> {
		template<typename container_type, typename... arg_types> JSONIFIER_INLINE static constexpr decltype(auto) impl(container_type& container, arg_types&&... args) noexcept {
			return ((functor_type<indices>::impl(container, forward<arg_types>(args)), ...));
		}

		template<typename container_type, typename arg_types> JSONIFIER_INLINE static constexpr decltype(auto) impl(container_type& container, arg_types&& args) noexcept {
			return ((functor_type<indices>::impl(container, forward<arg_types>(args)), ...));
		}

		template<typename... arg_types> JSONIFIER_INLINE static constexpr decltype(auto) impl(arg_types&&... args) noexcept {
			return ((functor_type<indices>::impl(forward<arg_types>(args)), ...));
		}

		template<typename container_type_01> JSONIFIER_INLINE static constexpr decltype(auto) impl_and(container_type_01& container_01, container_type_01& container_02) noexcept {
			return (functor_type<indices>::impl(container_01, container_02) && ...);
		}
	};

	static constexpr array<bool, 256> numberTerminators = [] {
		array<bool, 256> table{};
		table[',']	= true;
		table['}']	= true;
		table[']']	= true;
		table[' ']	= true;
		table['\t'] = true;
		table['\n'] = true;
		table['\r'] = true;
		return table;
	}();

	static constexpr array<parse_statuses, 256> failed_iterator_statuses{ [] {
		array<parse_statuses, 256> return_values{};
		return_values[static_cast<uint8_t>('\0')]									   = parse_statuses::unexpected_string_end;
		return_values[static_cast<uint8_t>(json_structural_characters::key_start)]	   = parse_statuses::invalid_string_characters;
		return_values[static_cast<uint8_t>(json_structural_characters::colon)]		   = parse_statuses::missing_colon;
		return_values[static_cast<uint8_t>(json_structural_characters::comma)]		   = parse_statuses::missing_comma;
		return_values[static_cast<uint8_t>(json_structural_characters::l_crl_bracket)] = parse_statuses::missing_object_start;
		return_values[static_cast<uint8_t>(json_structural_characters::r_crl_bracket)] = parse_statuses::missing_object_end;
		return_values[static_cast<uint8_t>(json_structural_characters::l_sqr_bracket)] = parse_statuses::missing_array_start;
		return_values[static_cast<uint8_t>(json_structural_characters::r_sqr_bracket)] = parse_statuses::missing_array_end;
		return return_values;
	}() };

}
