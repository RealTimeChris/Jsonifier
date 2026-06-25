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

#include <jsonifier-incl/containers/array.hpp>
#include <jsonifier-incl/utilities/concepts.hpp>

namespace jsonifier::internal {

	static constexpr array<bool, 256ULL> whitespaceTable{ []() constexpr {
		array<bool, 256ULL> returnValues{};
		returnValues[static_cast<uint64_t>('\t')] = true;
		returnValues[static_cast<uint64_t>(' ')]  = true;
		returnValues[static_cast<uint64_t>('\n')] = true;
		returnValues[static_cast<uint64_t>('\r')] = true;
		return returnValues;
	}() };

	static constexpr array<bool, 256> numberTable{ []() constexpr {
		array<bool, 256> returnValues{};
		returnValues['-'] = true;
		returnValues['0'] = true;
		returnValues['1'] = true;
		returnValues['2'] = true;
		returnValues['3'] = true;
		returnValues['4'] = true;
		returnValues['5'] = true;
		returnValues['6'] = true;
		returnValues['7'] = true;
		returnValues['8'] = true;
		returnValues['9'] = true;
		return returnValues;
	}() };

	template<concepts::uint_types value_type> constexpr value_type byteswap(value_type value) noexcept {
		if constexpr (sizeof(value_type) == 1) {
			return value;
		} else if constexpr (sizeof(value_type) == 2) {
			return static_cast<value_type>((value >> 8) | (value << 8));
		} else if constexpr (sizeof(value_type) == 4) {
			return static_cast<value_type>(((value & 0x000000FFu) << 24) | ((value & 0x0000FF00u) << 8) | ((value & 0x00FF0000u) >> 8) | ((value & 0xFF000000u) >> 24));
		} else if constexpr (sizeof(value_type) == 8) {
			return static_cast<value_type>(((value & 0x00000000000000FFull) << 56) | ((value & 0x000000000000FF00ull) << 40) | ((value & 0x0000000000FF0000ull) << 24) |
				((value & 0x00000000FF000000ull) << 8) | ((value & 0x000000FF00000000ull) >> 8) | ((value & 0x0000FF0000000000ull) >> 24) |
				((value & 0x00FF000000000000ull) >> 40) | ((value & 0xFF00000000000000ull) >> 56));
		} else {
			static_assert(sizeof(value_type) == 0, "byte_swap: unsupported type size");
		}
	}

	template<concepts::uint_types auto valueNew> struct integral_constant {
		using value_type				  = decltype(valueNew);
		static constexpr value_type value = valueNew;

		JSONIFIER_INLINE constexpr operator value_type() const noexcept {
			return value;
		}

		JSONIFIER_INLINE constexpr value_type operator()() const noexcept {
			return value;
		}
	};

	template<concepts::uint_types auto index> using tag = integral_constant<index>;

	template<typename value_type> JSONIFIER_INLINE constexpr jsonifier::internal::remove_reference_t<value_type>&& move(value_type&& value) noexcept {
		return static_cast<jsonifier::internal::remove_reference_t<value_type>&&>(value);
	}

	inline static std::ostream& operator<<(std::ostream& os, const std::source_location& location) {
		os << "File: " << location.file_name() << std::endl;
		os << "Line: " << location.line() << std::endl;
		return os;
	}

	template<uint64_t bytesProcessedNew, typename simd_type, typename integer_type_new, integer_type_new maskNew> struct type_holder {
		static constexpr uint64_t bytesProcessed{ bytesProcessedNew };
		static constexpr integer_type_new mask{ maskNew };
		using type		   = simd_type;
		using integer_type = integer_type_new;
	};

	template<typename value_type> struct get_int_type {
		using type = jsonifier::internal::conditional_t<std::is_unsigned_v<value_type>, uint8_t, int8_t>;
	};

	template<uint64_t... values> struct get_first_value;

	template<uint64_t value_new, uint64_t... values> struct get_first_value<value_new, values...> {
		using type = std::remove_cvref_t<decltype(value_new)>;
	};

	template<uint64_t... values> using get_first_value_t = get_first_value<values...>::type;

	template<uint64_t... indices> struct integer_sequence {};

	template<uint64_t index, typename integer_sequence> struct make_integer_sequence_impl;

	template<uint64_t... indices> struct make_integer_sequence_impl<0, integer_sequence<indices...>> {
		using type = integer_sequence<indices...>;
	};

	template<uint64_t index, uint64_t... indices> struct make_integer_sequence_impl<index, integer_sequence<indices...>> {
		using type = typename make_integer_sequence_impl<index - 1, integer_sequence<index - 1, indices...>>::type;
	};

	template<typename sequence_01, typename sequence_02> struct merge_and_shift;

	template<uint64_t... indices_01, uint64_t... indices_02> struct merge_and_shift<integer_sequence<indices_01...>, integer_sequence<indices_02...>> {
		using type = integer_sequence<static_cast<decltype(indices_01)>(indices_01)..., static_cast<decltype(indices_02)>(indices_02 + sizeof...(indices_01))...>;
	};

	template<uint64_t size> struct make_sequence_impl {
		using type = typename merge_and_shift<typename make_sequence_impl<static_cast<decltype(size)>(size / 2ULL)>::type,
			typename make_sequence_impl<static_cast<decltype(size)>(size - size / 2ULL)>::type>::type;
	};

	template<uint64_t size>
		requires(size == 0ULL)
	struct make_sequence_impl<size> {
		using type = integer_sequence<>;
	};

	template<uint64_t size>
		requires(size == 1ULL)
	struct make_sequence_impl<size> {
		using type = integer_sequence<static_cast<decltype(size)>(0)>;
	};

	template<uint64_t size> using make_integer_sequence = typename make_sequence_impl<size>::type;

	template<typename integer_sequence, uint64_t offset> struct offset_sequence;

	template<uint64_t... indices, uint64_t offset> struct offset_sequence<integer_sequence<indices...>, offset> {
		using type = integer_sequence<static_cast<decltype(offset)>(indices + offset)...>;
	};

	template<typename integer_sequence, uint64_t step> struct step_sequence;

	template<uint64_t... indices, uint64_t step_new> struct step_sequence<integer_sequence<indices...>, step_new> {
		using type = integer_sequence<static_cast<decltype(step_new)>(indices* step_new)...>;
	};

	template<typename integer_sequence, uint64_t step> using step_sequence_t = typename step_sequence<integer_sequence, step>::type;

	template<uint64_t start, uint64_t end, uint64_t step>
		requires(end >= start && step > 0)
	using make_stepped_range_sequence =
		typename offset_sequence<step_sequence_t<make_integer_sequence<static_cast<decltype(end)>((end - start + step - 1) / step)>, step>, start>::type;

	template<typename value_type> JSONIFIER_INLINE constexpr value_type&& forward(remove_reference_t<value_type>& t JSONIFIER_LIFETIME_BOUND) noexcept {
		return static_cast<value_type&&>(t);
	}

	template<typename value_type>
		requires(std::is_rvalue_reference_v<value_type>)
	JSONIFIER_INLINE constexpr value_type&& forward(remove_reference_t<value_type>&& t) noexcept {
		static_assert(!std::is_lvalue_reference_v<value_type>, "value_type cannot be an lvalue reference (e.g., U&).");
		return static_cast<value_type&&>(t);
	}

	template<auto function, typename variant_type, typename... arg_types, uint64_t... indices>
	JSONIFIER_INLINE static constexpr void visitImpl(integer_sequence<indices...>, variant_type&& variant, arg_types&&... args) noexcept {
		const auto idx = variant.index();
		static_cast<void>(((idx == indices ? (function(std::get<indices>(internal::forward<variant_type>(variant)), internal::forward<arg_types>(args)...), true) : false) || ...));
	}

	template<auto function, typename variant_type, typename... arg_types> JSONIFIER_INLINE static constexpr void visit(variant_type&& variant, arg_types&&... args) noexcept {
		using seq_t = make_integer_sequence<std::variant_size_v<jsonifier::internal::remove_cvref_t<variant_type>>>;
		visitImpl<function>(seq_t{}, internal::forward<variant_type>(variant), internal::forward<arg_types>(args)...);
	}

	template<template<auto...> typename functor_type, typename integer_sequence, auto...> struct functor_runner;

	template<template<auto...> typename functor_type, uint64_t... indices, auto... values> struct functor_runner<functor_type, integer_sequence<indices...>, values...> {
		template<typename... arg_types> JSONIFIER_INLINE static auto impl([[maybe_unused]] arg_types&&... args) noexcept {
			return (functor_type<values...>::template impl<indices>(internal::forward<arg_types>(args)...), ...);
		}

		template<typename... arg_types> JSONIFIER_INLINE static auto implAnd([[maybe_unused]] arg_types&&... args) noexcept {
			return (functor_type<values...>::template impl<indices>(internal::forward<arg_types>(args)...) && ...);
		}
	};

	template<template<auto...> typename functor_type, uint64_t... indices, uint64_t offsetVal, auto... values>
	struct functor_runner<functor_type, offset_sequence<integer_sequence<indices...>, offsetVal>, values...> {
		template<typename... arg_types> JSONIFIER_INLINE static auto impl([[maybe_unused]] arg_types&&... args) noexcept {
			return (functor_type<values...>::template impl<indices + offsetVal>(internal::forward<arg_types>(args)...), ...);
		}
		template<typename... arg_types> JSONIFIER_INLINE static auto implAnd([[maybe_unused]] arg_types&&... args) noexcept {
			return (functor_type<values...>::template impl<indices + offsetVal>(internal::forward<arg_types>(args)...) && ...);
		}
	};

	template<typename value_type01, typename value_type02> JSONIFIER_INLINE constexpr value_type01 max(value_type01 value1, value_type02 value2) {
		return value1 > static_cast<value_type01>(value2) ? value1 : static_cast<value_type01>(value2);
	}

	template<typename value_type01, typename value_type02> JSONIFIER_INLINE constexpr value_type01 min(value_type01 value1, value_type02 value2) {
		return value1 < static_cast<value_type01>(value2) ? value1 : static_cast<value_type01>(value2);
	}

}

#include <jsonifier-incl/containers/tuple.hpp>

namespace jsonifier::simd {

#if JSONIFIER_CHECK_FOR_INSTRUCTION(JSONIFIER_NEON)
	using avx_integer_list = internal::type_list_t<internal::type_holder<64, internal::avx_type_wrapper<internal::avx_type::m512>, uint64_t, 64>,
		internal::type_holder<32, internal::avx_type_wrapper<internal::avx_type::m256>, uint32_t, 32>,
		internal::type_holder<16, internal::avx_type_wrapper<internal::avx_type::m128>, uint64_t, 16>>;
	using avx_list		   = internal::type_list_t<internal::type_holder<64, internal::avx_type_wrapper<internal::avx_type::m512>, uint64_t, std::numeric_limits<uint64_t>::max()>,
				internal::type_holder<32, internal::avx_type_wrapper<internal::avx_type::m256>, uint32_t, std::numeric_limits<uint32_t>::max()>,
				internal::type_holder<16, internal::avx_type_wrapper<internal::avx_type::m128>, uint64_t, std::numeric_limits<uint64_t>::max()>>;
#else
	using avx_integer_list = internal::type_list_t<internal::type_holder<64, internal::avx_type_wrapper<internal::avx_type::m512>, uint64_t, 64>,
		internal::type_holder<32, internal::avx_type_wrapper<internal::avx_type::m256>, uint32_t, 32>,
		internal::type_holder<16, internal::avx_type_wrapper<internal::avx_type::m128>, uint16_t, 16>>;
	using avx_list		   = internal::type_list_t<internal::type_holder<64, internal::avx_type_wrapper<internal::avx_type::m512>, uint64_t, std::numeric_limits<uint64_t>::max()>,
				internal::type_holder<32, internal::avx_type_wrapper<internal::avx_type::m256>, uint32_t, std::numeric_limits<uint32_t>::max()>,
				internal::type_holder<16, internal::avx_type_wrapper<internal::avx_type::m128>, uint16_t, std::numeric_limits<uint16_t>::max()>>;
#endif

}
