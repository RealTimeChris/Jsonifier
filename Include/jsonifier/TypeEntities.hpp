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

#include <concepts>
#include <optional>
#include <variant>
#include <string>
#include <chrono>
#include <tuple>

#if !defined(jsonifier_inline)
	#define jsonifier_inline inline
#endif

#if !defined(jsonifier_constexpr)
	#define jsonifier_constexpr constexpr
#endif

namespace jsonifier {

	class raw_json_data;

	template<typename value_type> class vector;

	template<typename value_type> struct core {};

	template<typename value_type> struct array {
		value_type parseValue;
	};

	template<typename value_type> array(value_type) -> array<value_type>;

	template<typename value_type> struct object {
		value_type parseValue;
	};

	template<typename value_type> object(value_type) -> object<value_type>;
}

namespace jsonifier {

	namespace concepts {
		// From:
		// https://stackoverflow.com/questions/16337610/how-to-know-if-a-type-is-a-specialization-of-stdvector
		template<typename Test, template<typename...> typename Ref> struct is_specialization_v : std::false_type {};

		template<template<typename...> typename Ref, typename... Args> struct is_specialization_v<Ref<Args...>, Ref> : std::true_type {};

		template<size_t I> using Tag = std::integral_constant<size_t, I>;

		template<size_t I> jsonifier_constexpr Tag<I> TagV{};

		template<typename value_type> using unwrap = std::unwrap_ref_decay_t<value_type>;

		template<typename value_type>
		concept range = requires(value_type& value) {
			typename unwrap<value_type>::value_type;
			{ value.begin() } -> std::same_as<typename unwrap<value_type>::iterator>;
			{ value.end() } -> std::same_as<typename unwrap<value_type>::iterator>;
		};

		template<typename value_type>
		concept stateless = std::is_empty_v<unwrap<value_type>>;

		template<typename value_type>
		concept map_subscriptable = requires(value_type value) {
			{ value[std::declval<typename unwrap<value_type>::key_type>()] } -> std::same_as<const typename unwrap<value_type>::mapped_type&>;
		} || requires(value_type value) {
			{ value[std::declval<typename unwrap<value_type>::key_type>()] } -> std::same_as<typename unwrap<value_type>::mapped_type&>;
		};

		template<typename value_type>
		concept vector_subscriptable = requires(value_type value) {
			{ value[std::declval<uint64_t>()] } -> std::same_as<typename unwrap<value_type>::const_reference>;
		} || requires(value_type value) {
			{ value[std::declval<uint64_t>()] } -> std::same_as<typename unwrap<value_type>::reference>;
		};

		template<typename value_type>
		concept pair_t = requires(value_type value) {
			{ value.first } -> std::same_as<typename unwrap<value_type>::first_type>;
			{ value.second } -> std::same_as<typename unwrap<value_type>::second_type>;
		};

		template<typename value_type>
		concept has_size = requires(value_type value) {
			{ value.size() } -> std::same_as<typename unwrap<value_type>::size_type>;
		};

		template<typename value_type>
		concept has_data = requires(value_type data) {
			{ data.data() } -> std::same_as<typename unwrap<value_type>::const_pointer>;
		} || requires(value_type data) {
			{ data.data() } -> std::same_as<typename unwrap<value_type>::pointer>;
		};

		template<typename value_type>
		concept void_t = std::is_void_v<unwrap<value_type>>;

		template<typename value_type>
		concept indexable = stateless<value_type> || requires(value_type value) { value[Tag<0>()]; };
    
		template<typename value_type_01, typename value_type_02>
		concept related_ptr = ( std::derived_from<unwrap<value_type_01>, unwrap<value_type_02>> || std::is_base_of_v<unwrap<value_type_01>, unwrap<value_type_02>> ||
			std::same_as<unwrap<value_type_01>, unwrap<value_type_02>> )&&std::is_pointer_v<unwrap<value_type_01>>;

		template<typename value_type>
		concept bool_t = std::same_as<unwrap<value_type>, bool>;

		template<typename value_type>
		concept pointer_t = (std::is_pointer_v<unwrap<value_type>> || std::is_null_pointer_v<unwrap<value_type>>);

		template<typename value_type>
		concept signed_t = std::signed_integral<unwrap<value_type>> && !bool_t<value_type>;

		template<typename value_type>
		concept unsigned_t = std::unsigned_integral<unwrap<value_type>> && !bool_t<value_type>;

		template<typename value_type>
		concept unsigned_int16_t = unsigned_t<value_type> && sizeof(value_type) == 2;

		template<typename value_type>
		concept unsigned_int32_t = unsigned_t<value_type> && sizeof(value_type) == 4;

		template<typename value_type>
		concept unsigned_int64_t = unsigned_t<value_type> && sizeof(value_type) == 8;

		template<typename value_type>
		concept signed_int16_t = signed_t<value_type> && sizeof(value_type) == 2;

		template<typename value_type>
		concept signed_int32_t = signed_t<value_type> && sizeof(value_type) == 4;

		template<typename value_type>
		concept signed_int64_t = signed_t<value_type> && sizeof(value_type) == 8;

		template<typename value_type>
		concept float_t = std::floating_point<unwrap<value_type>>;

		template<typename value_type>
		concept char_t = std::same_as<unwrap<value_type>, char> || std::same_as<unwrap<value_type>, wchar_t> || std::same_as<unwrap<value_type>, char8_t> ||
			std::same_as<unwrap<value_type>, char16_t> || std::same_as<unwrap<value_type>, char32_t>;

		template<typename value_type>
		concept num_t = ( float_t<value_type> || unsigned_t<value_type> || signed_t<value_type> )&&!char_t<value_type>;

		template<typename value_type>
		concept has_substr = requires(value_type value) {
			{ value.substr(std::declval<uint64_t>(), std::declval<uint64_t>()) };
		};

		template<typename value_type>
		concept string_t = has_substr<unwrap<value_type>> && has_data<unwrap<value_type>> && has_size<unwrap<value_type>> && !std::same_as<char, unwrap<value_type>> &&
			vector_subscriptable<unwrap<value_type>> && !pointer_t<value_type>;

		template<typename value_type>
		concept map_t = requires(value_type data) {
			typename unwrap<value_type>::mapped_type;
			typename unwrap<value_type>::key_type;
		} && range<value_type> && map_subscriptable<value_type>;

		template<typename value_type>
		concept has_emplace_back = requires(value_type data) {
			{ data.emplace_back(std::declval<typename unwrap<value_type>::value_type&&>()) } -> std::same_as<typename unwrap<value_type>::value_type&>;
		};

		template<typename value_type>
		concept has_release = requires(value_type value) {
			{ value.release() } -> std::same_as<typename unwrap<value_type>::pointer>;
		};

		template<typename value_type>
		concept unique_ptr_t = requires(value_type other) {
			typename unwrap<value_type>::element_type;
			typename unwrap<value_type>::deleter_type;
			{ other.release() } -> std::same_as<typename unwrap<value_type>::pointer>;
		} && has_release<value_type> && !std::is_copy_assignable_v<unwrap<value_type>>;

		template<typename value_type>
		concept shared_ptr_t = (requires(value_type other) {
			typename unwrap<value_type>::element_type;
			typename unwrap<value_type>::deleter_type;
			{ other.release() } -> std::same_as<typename unwrap<value_type>::pointer>;
		}) && has_release<value_type>;

		template<typename value_type>
		concept has_find = requires(value_type c) {
			{ c.find(std::declval<const typename unwrap<value_type>::key_type&>()) };
		};

		template<typename value_type>
		concept has_excluded_keys = requires(value_type value) {
			{ value.excludedKeys };
		};

		template<class value_type>
		concept always_null_t = std::same_as<value_type, std::nullptr_t> || std::same_as<value_type, std::monostate> || std::same_as<value_type, std::nullopt_t>;

		template<class value_type>
		concept nullable_t = !string_t<value_type> && requires(value_type t) {
			bool(t);
			{ *t };
		};

		template<class value_type>
		concept null_t = nullable_t<value_type> || always_null_t<value_type>;

		template<typename value_type>
		concept has_resize = requires(value_type value) { value.resize(0); };

		template<typename value_type>
		concept raw_json_t = std::same_as<unwrap<value_type>, jsonifier::raw_json_data>;

		template<typename value_type>
		concept jsonifier_t = requires { jsonifier::core<unwrap<value_type>>::parseValue; };

		struct empty_val {
			static jsonifier_constexpr std::tuple<> parseValue{};
		};

		template<typename value_type> jsonifier_constexpr auto coreWrapperV = [] {
			if jsonifier_constexpr (jsonifier_t<value_type>) {
				return jsonifier::core<unwrap<value_type>>::parseValue;
			} else {
				return empty_val{};
			}
		}();

		template<typename value_type> jsonifier_constexpr auto coreV = coreWrapperV<value_type>.parseValue;

		template<typename value_type> using core_t = unwrap<decltype(coreV<value_type>)>;

		template<typename value_type> using core_wrapper_t = unwrap<decltype(coreWrapperV<value_type>)>;

		template<typename value_type>
		concept jsonifier_array_t = jsonifier_t<value_type> && is_specialization_v<core_wrapper_t<value_type>, jsonifier::array>::value;

		template<typename value_type>
		concept jsonifier_object_t = jsonifier_t<value_type> && is_specialization_v<core_wrapper_t<value_type>, jsonifier::object>::value;

		template<typename value_type>
		concept enum_t = std::is_enum<unwrap<value_type>>::value;

		template<typename value_type>
		concept vector_t = ( !map_t<value_type> && vector_subscriptable<value_type> && has_data<value_type> )&&!jsonifier_array_t<value_type> && !has_substr<value_type>;

		template<typename value_type>
		concept raw_array_t = std::is_array_v<unwrap<value_type>>;

		template<typename value_type>
		concept buffer_like = has_resize<value_type> && vector_subscriptable<value_type> && has_data<value_type>;

		template<typename value_type>
		concept core_type = is_specialization_v<jsonifier::core<value_type>, jsonifier::core>::value;

		template<typename value_type>
		concept time_type = is_specialization_v<std::chrono::duration<value_type>, std::chrono::duration>::value;

		template<typename value_type>
		concept uint8_type = std::same_as<uint8_t, unwrap<value_type>>;

		template<typename value_type>
		concept char_type = uint8_type<value_type> || char_t<value_type>;

		template<typename value_type>
		concept integer_t = std::integral<unwrap<value_type>> && !bool_t<value_type>;

	}

}// namespace jsonifier_internal
