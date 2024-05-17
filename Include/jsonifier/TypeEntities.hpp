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
/// Feb 3, 2023
#pragma once

#include <type_traits>
#include <concepts>
#include <optional>
#include <variant>
#include <string>
#include <vector>
#include <chrono>
#include <tuple>

namespace jsonifier_internal {

	template<typename derived_type> class parser;

	template<typename... value_types> class array_tuple : public std::tuple<value_types...> {};
}

namespace jsonifier {

	class raw_json_data;

	template<typename value_type_new, uint64_t sizeVal = 0> class vector;

	template<typename value_type> struct core {};

	template<typename value_type> struct value {
		value_type parseValue;
	};

	template<typename value_type> value(value_type) -> value<value_type>;

	template<typename value_type> struct scalar_value {
		value_type parseValue;
	};

	template<typename value_type> scalar_value(value_type) -> scalar_value<value_type>;

	namespace concepts {
		// from
		// https://stackoverflow.com/questions/16337610/how-to-know-if-a-type-is-a-specialization-of-stdvector
		template<typename, template<typename...> typename> constexpr bool is_specialization_v = false;

		template<template<typename...> class value_type, typename... Args> constexpr bool is_specialization_v<value_type<Args...>, value_type> = true;

		template<uint64_t index> using tag = std::integral_constant<uint64_t, index>;

		template<uint64_t index> constexpr tag<index> TagV{};

		template<typename value_type, typename... value_types> struct collect_first_type {
			using type = value_type;
		};

		template<typename... value_type> using unwrap_t = std::remove_cvref_t<typename collect_first_type<value_type...>::type>;

		template<typename value_type>
		concept range = requires(unwrap_t<value_type> value) {
			typename unwrap_t<value_type>::value_type;
			{ value.begin() } -> std::same_as<typename unwrap_t<value_type>::const_iterator>;
			{ value.end() } -> std::same_as<typename unwrap_t<value_type>::const_iterator>;
		} || requires(unwrap_t<value_type> value) {
			typename unwrap_t<value_type>::value_type;
			{ value.begin() } -> std::same_as<typename unwrap_t<value_type>::iterator>;
			{ value.end() } -> std::same_as<typename unwrap_t<value_type>::iterator>;
		};

		template<typename value_type>
		concept map_subscriptable = requires(unwrap_t<value_type> value) {
			{ value[std::declval<typename unwrap_t<value_type>::key_type>()] } -> std::same_as<const typename unwrap_t<value_type>::mapped_type&>;
		} || requires(unwrap_t<value_type> value) {
			{ value[std::declval<typename unwrap_t<value_type>::key_type>()] } -> std::same_as<typename unwrap_t<value_type>::mapped_type&>;
		};

		template<typename value_type>
		concept vector_subscriptable = requires(unwrap_t<value_type> value) {
			{ value[std::declval<typename unwrap_t<value_type>::size_type>()] } -> std::same_as<typename unwrap_t<value_type>::const_reference>;
		} || requires(unwrap_t<value_type> value) {
			{ value[std::declval<typename unwrap_t<value_type>::size_type>()] } -> std::same_as<typename unwrap_t<value_type>::reference>;
		};

		template<typename value_type>
		concept has_size = requires(unwrap_t<value_type> value) {
			{ value.size() } -> std::same_as<typename unwrap_t<value_type>::size_type>;
		};

		template<typename value_type>
		concept variant_t = is_specialization_v<unwrap_t<value_type>, std::variant>;

		template<typename value_type>
		concept is_fwd_iterator = std::forward_iterator<unwrap_t<value_type>>;

		template<typename value_type>
		concept has_resize = requires(unwrap_t<value_type> value) { value.resize(std::declval<typename unwrap_t<value_type>::size_type>()); };

		template<typename value_type>
		concept has_data = requires(unwrap_t<value_type> value) {
			{ value.data() } -> std::same_as<typename unwrap_t<value_type>::const_pointer>;
		} || requires(unwrap_t<value_type> value) {
			{ value.data() } -> std::same_as<typename unwrap_t<value_type>::pointer>;
		};

		template<typename value_type>
		concept stateless = std::is_empty_v<unwrap_t<value_type>>;

		template<typename value_type>
		concept void_t = std::is_void_v<unwrap_t<value_type>>;

		template<typename value_type>
		concept indexable = stateless<value_type> || requires(unwrap_t<value_type> value) { value[tag<0>()]; };

		template<typename value_type_01, typename value_type_02>
		concept related_ptr = ( std::derived_from<unwrap_t<value_type_01>, unwrap_t<value_type_02>> || std::is_base_of_v<unwrap_t<value_type_01>, unwrap_t<value_type_02>> ||
			std::is_same_v<unwrap_t<value_type_01>, unwrap_t<value_type_02>> )&&std::is_pointer_v<unwrap_t<value_type_01>>;

		template<typename value_type>
		concept bool_t = std::is_same_v<unwrap_t<value_type>, bool> || std::same_as<unwrap_t<value_type>, std::vector<bool>::reference> ||
			std::same_as<unwrap_t<value_type>, std::vector<bool>::const_reference>;

		template<typename value_type>
		concept always_null_t =
			std::is_same_v<unwrap_t<value_type>, std::nullptr_t> || std::is_same_v<unwrap_t<value_type>, std::monostate> || std::is_same_v<unwrap_t<value_type>, std::nullopt_t>;

		template<typename value_type>
		concept pointer_t =
			( std::is_pointer_v<unwrap_t<value_type>> || std::is_null_pointer_v<unwrap_t<value_type>> && !std::is_array_v<unwrap_t<value_type>> )&&!always_null_t<value_type>;

		template<typename value_type>
		concept signed_type = std::signed_integral<unwrap_t<value_type>> && !bool_t<value_type>;

		template<typename value_type>
		concept unsigned_type = std::unsigned_integral<unwrap_t<value_type>> && !bool_t<value_type>;

		template<typename value_type>
		concept uint8_type = std::is_same_v<uint8_t, unwrap_t<value_type>>;

		template<typename value_type>
		concept uint16_type = std::is_same_v<uint16_t, unwrap_t<value_type>>;

		template<typename value_type>
		concept uint32_type = std::is_same_v<uint32_t, unwrap_t<value_type>>;

		template<typename value_type>
		concept uint64_type = std::is_same_v<uint64_t, unwrap_t<value_type>>;

		template<typename value_type>
		concept int8_type = std::is_same_v<int8_t, unwrap_t<value_type>>;

		template<typename value_type>
		concept int16_type = std::is_same_v<int16_t, unwrap_t<value_type>>;

		template<typename value_type>
		concept int32_type = std::is_same_v<int32_t, unwrap_t<value_type>>;

		template<typename value_type>
		concept int64_type = std::is_same_v<int64_t, unwrap_t<value_type>>;

		template<typename value_type>
		concept float_t = std::floating_point<unwrap_t<value_type>>;

		template<typename value_type>
		concept char_type = std::is_same_v<unwrap_t<value_type>, char>;

		template<typename value_type>
		concept u_char_type = std::is_same_v<unwrap_t<value_type>, unsigned char>;

		template<typename value_type>
		concept num_t = ( float_t<value_type> || unsigned_type<value_type> || signed_type<value_type> )&&!char_type<value_type>;

		template<typename value_type>
		concept has_substr = requires(unwrap_t<value_type> value) {
			{
				value.substr(std::declval<typename unwrap_t<value_type>::size_type>(), std::declval<typename unwrap_t<value_type>::size_type>())
			} -> std::same_as<unwrap_t<value_type>>;
		};

		template<typename value_type>
		concept string_t = has_substr<value_type> && has_data<value_type> && has_size<value_type> && !std::is_same_v<unwrap_t<value_type>, char> &&
			vector_subscriptable<value_type> && !pointer_t<value_type>;

		template<typename value_type>
		concept map_t = requires(unwrap_t<value_type> value) {
			typename unwrap_t<value_type>::mapped_type;
			typename unwrap_t<value_type>::key_type;
		} && range<value_type> && map_subscriptable<value_type>;

		template<typename value_type>
		concept pair_t = requires(unwrap_t<value_type> value) {
			typename unwrap_t<value_type>::first_type;
			typename unwrap_t<value_type>::second_type;
		};

		template<typename value_type>
		concept has_emplace_back = requires(unwrap_t<value_type> value) {
			{ value.emplace_back(std::declval<typename unwrap_t<value_type>::value_type&&>()) } -> std::same_as<typename unwrap_t<value_type>::value_type&>;
		};

		template<typename value_type>
		concept has_release = requires(unwrap_t<value_type> value) {
			{ value.release() } -> std::same_as<typename unwrap_t<value_type>::pointer>;
		};

		template<typename value_type>
		concept copyable = std::copyable<unwrap_t<value_type>>;

		template<typename value_type>
		concept unique_ptr_t = requires(unwrap_t<value_type> value) {
			typename unwrap_t<value_type>::element_type;
			typename unwrap_t<value_type>::deleter_type;
		} && has_release<value_type>;

		template<typename value_type>
		concept shared_ptr_t = requires(unwrap_t<value_type> value) {
			typename unwrap_t<value_type>::element_type;
			typename unwrap_t<value_type>::deleter_type;
		} && has_release<value_type> && copyable<value_type>;

		template<typename value_type>
		concept has_find = requires(unwrap_t<value_type> value) {
			{ value.find(std::declval<const typename unwrap_t<value_type>::key_type&>()) };
		};

		template<typename value_type>
		concept has_excluded_keys = requires(unwrap_t<value_type> value) {
			{ value.jsonifierExcludedKeys };
		};

		template<typename value_type>
		concept nullable_t = !string_t<value_type> && requires(unwrap_t<value_type> value) {
			bool(value);
			{ *value };
		};

		template<typename value_type>
		concept array_tuple_t = requires(unwrap_t<value_type> value) {
			std::tuple_size<unwrap_t<value_type>>::value;
			std::get<0>(value);
		};

		template<typename value_type>
		concept null_t = nullable_t<value_type> || always_null_t<value_type>;

		template<typename value_type>
		concept raw_json_t = std::is_same_v<unwrap_t<value_type>, jsonifier::raw_json_data>;
		
		template<typename value_type01, typename value_type02>
		concept same_character_size = requires() { sizeof(typename unwrap_t<value_type01>::value_type) == sizeof(typename unwrap_t<value_type02>::value_type); } &&
			string_t<value_type01> && string_t<value_type02>;

		template<typename value_type>
		concept tuple_t = requires(unwrap_t<value_type> t) {
			std::tuple_size<unwrap_t<value_type>>::value;
			std::get<0>(t);
		};

		template<typename value_type> using decay_keep_volatile_t = std::remove_const_t<std::remove_reference_t<value_type>>;

		template<typename value_type>
		concept optional_t = is_specialization_v<unwrap_t<value_type>, std::optional>;

		template<typename value_type>
		concept enum_t = std::is_enum_v<unwrap_t<value_type>>;

		template<typename value_type>
		concept vector_t = !map_t<value_type> && vector_subscriptable<value_type> && !has_substr<value_type> && !array_tuple_t<value_type>;

		template<typename value_type>
		concept jsonifier_t = requires { jsonifier::core<unwrap_t<value_type>>::parseValue; };

		template<typename value_type>
		concept is_core_type = jsonifier_t<value_type> || vector_t<value_type>;

		struct empty {
			static constexpr std::tuple<> parseValue{};
		};

		template<typename value_type> constexpr auto core_value = [] {
			if constexpr (jsonifier_t<value_type>) {
				return jsonifier::core<unwrap_t<value_type>>::parseValue;
			} else {
				return empty::parseValue;
			}
		}();

		template<typename value_type> constexpr auto core_wrapper_value = core_value<value_type>.parseValue;

		template<typename value_type> using core_wrapper_type = decltype(core_wrapper_value<value_type>);

		template<typename value_type> using core_type = unwrap_t<decltype(core_value<value_type>)>;

		template<typename value_type>
		concept jsonifier_scalar_value_t = jsonifier_t<value_type> && is_specialization_v<core_type<value_type>, scalar_value>;

		template<typename value_type>
		concept jsonifier_value_t = jsonifier_t<value_type> && is_specialization_v<core_type<value_type>, value>;

		template<typename value_type>
		concept raw_array_t = ( std::is_array_v<unwrap_t<value_type>> && !std::is_pointer_v<unwrap_t<value_type>> ) ||
			vector_subscriptable<value_type> && !vector_t<value_type> && !has_substr<value_type> && !array_tuple_t<value_type>;

		template<typename value_type>
		concept buffer_like = vector_subscriptable<value_type> && has_data<value_type> && has_resize<value_type>;

		template<typename value_type> constexpr bool printErrorFunction() {
			using fail_type = typename value_type::fail_type;
			return false;
		}

		template<is_core_type value_type> constexpr bool printErrorFunction() {
			return true;
		}

		template<typename value_type>
		concept time_type = is_specialization_v<std::chrono::duration<unwrap_t<value_type>>, std::chrono::duration>;

		template<typename value_type>
		concept char_t = uint8_type<value_type> || char_type<value_type>;

		template<typename value_type>
		concept integer_t = std::integral<unwrap_t<value_type>> && !bool_t<value_type>;

		template<uint64_t bytesProcessedNew, typename simd_type, typename integer_type_new, integer_type_new maskNew> struct type_holder {
			static constexpr uint64_t bytesProcessed{ bytesProcessedNew };
			static constexpr integer_type_new mask{ maskNew };
			using type		   = simd_type;
			using integer_type = integer_type_new;
		};

		template<typename... types> struct type_list;

		template<typename value_type, typename... rest> struct type_list<value_type, rest...> {
			using current_type			   = value_type;
			using remaining_types		   = type_list<rest...>;
			static constexpr uint64_t size = 1 + sizeof...(rest);
		};

		template<typename value_type> struct type_list<value_type> {
			using current_type			   = value_type;
			static constexpr uint64_t size = 1;
		};

		template<typename type_list, uint64_t Index> struct get_type_at_index;

		template<typename value_type, typename... rest> struct get_type_at_index<type_list<value_type, rest...>, 0> {
			using type = value_type;
		};

		template<typename value_type, typename... rest, uint64_t Index> struct get_type_at_index<type_list<value_type, rest...>, Index> {
			using type = typename get_type_at_index<type_list<rest...>, Index - 1>::type;
		};
	}

}// namespace jsonifier_internal

namespace std {

	template<> struct variant_size<jsonifier::concepts::empty> : integral_constant<uint64_t, 0> {};

	template<> struct tuple_size<jsonifier::concepts::empty> : integral_constant<uint64_t, 0> {};

	template<typename... value_types> struct tuple_size<jsonifier_internal::array_tuple<value_types...>> : integral_constant<uint64_t, sizeof...(value_types)> {};
}
