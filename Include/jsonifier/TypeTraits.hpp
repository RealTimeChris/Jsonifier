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


	template<typename value_type> struct remove_reference {
		using type				   = value_type;
	};

	template<typename value_type> struct remove_reference<value_type&> {
		using type				   = value_type;
	};

	template<typename value_type> struct remove_reference<value_type&&> {
		using type				   = value_type;
	};

	template<typename value_type> using remove_reference_t = typename remove_reference<value_type>::type;

	template<typename value_type> using _Remove_cvref_t = remove_cv_t<remove_reference_t<value_type>>;

	template<typename value_type> using remove_cvref_t = _Remove_cvref_t<value_type>;

	template<typename value_type> struct remove_cvref {
		using type = remove_cvref_t<value_type>;
	};

	template<size_t... indices> struct index_sequence {};

	template<size_t N, typename index_sequence> struct make_index_sequence_impl;

	template<size_t... indices> struct make_index_sequence_impl<0, index_sequence<indices...>> {
		using type = index_sequence<indices...>;
	};

	template<size_t N, size_t... indices> struct make_index_sequence_impl<N, index_sequence<indices...>> {
		using type = typename make_index_sequence_impl<N - 1, index_sequence<N - 1, indices...>>::type;
	};

	template<size_t N> using make_index_sequence = typename make_index_sequence_impl<N, index_sequence<>>::type;

}