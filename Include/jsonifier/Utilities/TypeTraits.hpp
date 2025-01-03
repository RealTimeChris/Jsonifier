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

#include <jsonifier/Core/Config.hpp>
#include <cstdint>

namespace jsonifier::internal {

	template<typename value_type> struct remove_cv {
		using type = value_type;
	};

	template<typename value_type> struct remove_cv<const value_type> {
		using type = value_type;
	};

	template<typename value_type> struct remove_cv<volatile value_type> {
		using type = value_type;
	};

	template<typename value_type> struct remove_cv<const volatile value_type> {
		using type = value_type;
	};

	template<typename value_type> using remove_cv_t = typename remove_cv<value_type>::type;

	template<typename value_type> struct remove_pointer {
		using type = value_type;
	};

	template<typename value_type> struct remove_pointer<value_type*> {
		using type = value_type;
	};

	template<typename value_type> using remove_pointer_t = typename remove_pointer<value_type>::type;

	template<typename value_type> struct remove_reference {
		using type = value_type;
	};

	template<typename value_type> struct remove_reference<value_type&> {
		using type = value_type;
	};

	template<typename value_type> struct remove_reference<value_type&&> {
		using type = value_type;
	};

	template<typename value_type> using remove_reference_t = typename remove_reference<value_type>::type;

	template<typename value_type> using remove_cvref_t = remove_cv_t<remove_reference_t<value_type>>;

	template<size_t... indices> struct index_sequence {};

	template<size_t index, typename index_sequence> struct make_index_sequence_impl;

	template<size_t... indices> struct make_index_sequence_impl<0, index_sequence<indices...>> {
		using type = index_sequence<indices...>;
	};

	template<size_t index, size_t... indices> struct make_index_sequence_impl<index, index_sequence<indices...>> {
		using type = typename make_index_sequence_impl<index - 1, index_sequence<index - 1, indices...>>::type;
	};

	template<size_t index> using make_index_sequence = typename make_index_sequence_impl<index, index_sequence<>>::type;

	template<class value_type_new, value_type_new valueNew> struct integral_constant {
		static constexpr value_type_new value = valueNew;

		using value_type = value_type_new;
		using type		 = integral_constant;

		JSONIFIER_INLINE constexpr operator value_type() const noexcept {
			return value;
		}

		JSONIFIER_INLINE constexpr value_type operator()() const noexcept {
			return value;
		}
	};

	template<bool condition, typename type01, typename type02> struct conditional;

	template<typename type01, typename type02> struct conditional<true, type01, type02> {
		using type = type01;
	};

	template<typename type01, typename type02> struct conditional<false, type01, type02> {
		using type = type02;
	};

	template<bool condition, typename type01, typename type02> using conditional_t = conditional<condition, type01, type02>::type;

}