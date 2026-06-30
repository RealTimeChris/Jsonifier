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

	template<template<auto> typename functor_type, uint64_types auto... indices> struct functor_executor<functor_type, integer_sequence<indices...>> {
		template<typename container_type, typename... arg_types> JSONIFIER_INLINE static constexpr decltype(auto) impl(container_type& container, arg_types&&... args) noexcept {
			return ((functor_type<indices>::impl(container, forward<arg_types>(args)), ...));
		}

		template<typename container_type, typename arg_types> JSONIFIER_INLINE static constexpr decltype(auto) impl(container_type& container, arg_types&& args) noexcept {
			return ((functor_type<indices>::impl(container, forward<arg_types>(args)), ...));
		}

		template<typename container_type_01> JSONIFIER_INLINE static constexpr decltype(auto) impl(container_type_01& container_01, container_type_01& container_02) noexcept {
			return (functor_type<indices>::impl(container_01, container_02), ...);
		}

		template<typename container_type_01> JSONIFIER_INLINE static constexpr decltype(auto) impl_and(container_type_01& container_01, container_type_01& container_02) noexcept {
			return (functor_type<indices>::impl(container_01, container_02) && ...);
		}
	};

	static constexpr array<bool, 256> numberTerminators = [] {
		array<bool, 256> table{};
		table[',']						  = true;
		table['}']						  = true;
		table[']']						  = true;
		table[' ']						  = true;
		table['\t']						  = true;
		table['\n']						  = true;
		table['\r']						  = true;
		table[static_cast<uint8_t>('\0')] = true;
		return table;
	}();


}
