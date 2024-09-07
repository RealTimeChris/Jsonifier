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

#include <jsonifier/Config.hpp>
#include <type_traits>
#include <functional>
#include <optional>
#include <iostream>
#include <sstream>
#include <cassert>
#include <cstring>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <variant>
#include <chrono>
#include <cfloat>
#include <bitset>
#include <atomic>
#include <array>

namespace jsonifier_internal {

	template<typename derived_type> class parser;

	class json_structural_iterator;

	template<typename... value_types> class array_tuple : public std::tuple<value_types...> {};

	// from
	// https://stackoverflow.com/questions/16337610/how-to-know-if-a-type-is-a-specialization-of-stdvector
	template<typename, template<typename...> typename> constexpr bool is_specialization_v = false;

	template<template<typename...> class value_type, typename... arg_types> constexpr bool is_specialization_v<value_type<arg_types...>, value_type> = true;

	template<uint64_t index> using tag = std::integral_constant<uint64_t, index>;

	template<uint64_t index> constexpr tag<index> TagV{};

	template<typename value_type, typename... value_types> struct collect_first_type {
		using type = value_type;
	};

	template<typename... value_type> using unwrap_t = std::remove_cvref_t<typename collect_first_type<value_type...>::type>;

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

	template<typename type_list, uint64_t index> struct get_type_at_index;

	template<typename value_type, typename... rest> struct get_type_at_index<type_list<value_type, rest...>, 0> {
		using type = value_type;
	};

	template<typename value_type, typename... rest, uint64_t index> struct get_type_at_index<type_list<value_type, rest...>, index> {
		using type = typename get_type_at_index<type_list<rest...>, index - 1>::type;
	};

	template<const auto& function, uint64_t currentIndex = 0, typename variant_type, typename... arg_types>
	JSONIFIER_ALWAYS_INLINE constexpr void visit(variant_type&& variant, arg_types&&... args) {
		if constexpr (currentIndex < std::variant_size_v<unwrap_t<variant_type>>) {
			variant_type&& variantNew = std::forward<variant_type>(variant);
			if (variantNew.index() == currentIndex) [[unlikely]] {
				function(std::get<currentIndex>(variantNew), std::forward<arg_types>(args)...);
				return;
			}
			visit<function, currentIndex + 1>(variantNew, std::forward<arg_types>(args)...);
		}
	}

}

namespace jsonifier {

	template<typename value_type, uint64_t> class string_base;

	using string = string_base<char, 0>;

	class raw_json_data;

	template<typename value_type_new, uint64_t sizeVal = 0> class vector;

	template<typename value_type> struct core {};

	template<bool value, typename value_type> struct falseV {
		static_assert(value, "Sorry, but the static assert failed.");
		static constexpr bool boolVal{ value };
	};

	template<typename value_type> struct value {
		value_type val{};
	};

	template<typename value_type> value(value_type) -> value<value_type>;

	template<typename value_type> struct scalar_value {
		value_type val{};
	};

	template<typename value_type> scalar_value(value_type) -> scalar_value<value_type>;

	namespace concepts {

		template<typename value_type>
		concept simd_int_512_type = std::is_same_v<jsonifier_simd_int_512, jsonifier_internal::unwrap_t<value_type>>;
		template<typename value_type>
		concept simd_int_256_type = std::is_same_v<jsonifier_simd_int_256, jsonifier_internal::unwrap_t<value_type>>;
		template<typename value_type>
		concept simd_int_128_type = std::is_same_v<jsonifier_simd_int_128, jsonifier_internal::unwrap_t<value_type>>;
		template<typename value_type>
		concept simd_int_type = std::is_same_v<jsonifier_simd_int_t, jsonifier_internal::unwrap_t<value_type>>;

		template<typename value_type>
		concept range = requires(jsonifier_internal::unwrap_t<value_type> value) {
			typename jsonifier_internal::unwrap_t<value_type>::value_type;
			{ value.begin() } -> std::same_as<typename jsonifier_internal::unwrap_t<value_type>::const_iterator>;
			{ value.end() } -> std::same_as<typename jsonifier_internal::unwrap_t<value_type>::const_iterator>;
		} || requires(jsonifier_internal::unwrap_t<value_type> value) {
			typename jsonifier_internal::unwrap_t<value_type>::value_type;
			{ value.begin() } -> std::same_as<typename jsonifier_internal::unwrap_t<value_type>::iterator>;
			{ value.end() } -> std::same_as<typename jsonifier_internal::unwrap_t<value_type>::iterator>;
		};

		template<typename value_type>
		concept map_subscriptable = requires(jsonifier_internal::unwrap_t<value_type> value) {
			{ value[typename jsonifier_internal::unwrap_t<value_type>::key_type{}] } -> std::same_as<const typename jsonifier_internal::unwrap_t<value_type>::mapped_type&>;
		} || requires(jsonifier_internal::unwrap_t<value_type> value) {
			{ value[typename jsonifier_internal::unwrap_t<value_type>::key_type{}] } -> std::same_as<typename jsonifier_internal::unwrap_t<value_type>::mapped_type&>;
		};

		template<typename value_type>
		concept vector_subscriptable = requires(jsonifier_internal::unwrap_t<value_type> value) {
			{ value[typename jsonifier_internal::unwrap_t<value_type>::size_type{}] } -> std::same_as<typename jsonifier_internal::unwrap_t<value_type>::const_reference>;
		} || requires(jsonifier_internal::unwrap_t<value_type> value) {
			{ value[typename jsonifier_internal::unwrap_t<value_type>::size_type{}] } -> std::same_as<typename jsonifier_internal::unwrap_t<value_type>::reference>;
		};

		template<typename value_type>
		concept has_size = requires(jsonifier_internal::unwrap_t<value_type> value) {
			{ value.size() } -> std::same_as<typename jsonifier_internal::unwrap_t<value_type>::size_type>;
		};

		template<typename value_type>
		concept variant_t = jsonifier_internal::is_specialization_v<jsonifier_internal::unwrap_t<value_type>, std::variant>;

		template<typename value_type>
		concept has_resize = requires(jsonifier_internal::unwrap_t<value_type> value) { value.resize(typename jsonifier_internal::unwrap_t<value_type>::size_type{}); };

		template<typename value_type>
		concept has_data = requires(jsonifier_internal::unwrap_t<value_type> value) {
			{ value.data() } -> std::same_as<typename jsonifier_internal::unwrap_t<value_type>::const_pointer>;
		} || requires(jsonifier_internal::unwrap_t<value_type> value) {
			{ value.data() } -> std::same_as<typename jsonifier_internal::unwrap_t<value_type>::pointer>;
		};

		template<typename value_type>
		concept stateless = std::is_empty_v<jsonifier_internal::unwrap_t<value_type>>;

		template<typename value_type>
		concept void_t = std::is_void_v<jsonifier_internal::unwrap_t<value_type>>;

		template<typename value_type>
		concept has_ptr_operator = requires(jsonifier_internal::unwrap_t<value_type> value) { value.operator->(); };

		template<typename value_type>
		concept indexable = stateless<value_type> || requires(jsonifier_internal::unwrap_t<value_type> value) { value[jsonifier_internal::tag<0>()]; };

		template<typename value_type_01, typename value_type_02>
		concept related_ptr = (std::derived_from<jsonifier_internal::unwrap_t<value_type_01>, jsonifier_internal::unwrap_t<value_type_02>> ||
								  std::is_base_of_v<jsonifier_internal::unwrap_t<value_type_01>, jsonifier_internal::unwrap_t<value_type_02>> ||
								  std::is_same_v<jsonifier_internal::unwrap_t<value_type_01>, jsonifier_internal::unwrap_t<value_type_02>>) &&
			std::is_pointer_v<jsonifier_internal::unwrap_t<value_type_01>>;

		template<typename value_type>
		concept bool_t = std::is_same_v<jsonifier_internal::unwrap_t<value_type>, bool> || std::same_as<jsonifier_internal::unwrap_t<value_type>, std::vector<bool>::reference> ||
			std::same_as<jsonifier_internal::unwrap_t<value_type>, std::vector<bool>::const_reference>;

		template<typename value_type>
		concept always_null_t = std::is_same_v<jsonifier_internal::unwrap_t<value_type>, std::nullptr_t> ||
			std::is_same_v<jsonifier_internal::unwrap_t<value_type>, std::monostate> || std::is_same_v<jsonifier_internal::unwrap_t<value_type>, std::nullopt_t>;

		template<typename value_type>
		concept pointer_t = (std::is_pointer_v<jsonifier_internal::unwrap_t<value_type>> ||
								( std::is_null_pointer_v<jsonifier_internal::unwrap_t<value_type>> && !std::is_array_v<jsonifier_internal::unwrap_t<value_type>> )) &&
			!always_null_t<value_type>;

		template<typename value_type>
		concept signed_type = std::signed_integral<jsonifier_internal::unwrap_t<value_type>> && !bool_t<value_type>;

		template<typename value_type>
		concept unsigned_type = std::unsigned_integral<jsonifier_internal::unwrap_t<value_type>> && !bool_t<value_type>;

		template<typename value_type>
		concept uint8_type = sizeof(jsonifier_internal::unwrap_t<value_type>) == 1 && unsigned_type<value_type>;

		template<typename value_type>
		concept uint16_type = sizeof(jsonifier_internal::unwrap_t<value_type>) == 2 && unsigned_type<value_type>;

		template<typename value_type>
		concept uint32_type = sizeof(jsonifier_internal::unwrap_t<value_type>) == 4 && unsigned_type<value_type>;

		template<typename value_type>
		concept uint64_type = sizeof(jsonifier_internal::unwrap_t<value_type>) == 8 && unsigned_type<value_type>;

		template<typename value_type>
		concept int8_type = sizeof(jsonifier_internal::unwrap_t<value_type>) == 1 && signed_type<value_type>;

		template<typename value_type>
		concept int16_type = sizeof(jsonifier_internal::unwrap_t<value_type>) == 2 && signed_type<value_type>;

		template<typename value_type>
		concept int32_type = sizeof(jsonifier_internal::unwrap_t<value_type>) == 4 && signed_type<value_type>;

		template<typename value_type>
		concept int64_type = sizeof(jsonifier_internal::unwrap_t<value_type>) == 8 && signed_type<value_type>;

		template<typename value_type>
		concept double_type = std::is_same_v<double, jsonifier_internal::unwrap_t<value_type>>;

		template<typename value_type>
		concept float_type = std::floating_point<jsonifier_internal::unwrap_t<value_type>>;

		template<typename value_type>
		concept json_structural_iterator_t = std::is_same_v<jsonifier_internal::unwrap_t<value_type>, jsonifier_internal::json_structural_iterator>;

		template<typename value_type>
		concept char_type = std::is_same_v<jsonifier_internal::unwrap_t<value_type>, char>;

		template<typename value_type>
		concept u_char_type = std::is_same_v<jsonifier_internal::unwrap_t<value_type>, uint8_t>;

		template<typename value_type>
		concept num_t = (float_type<value_type> || unsigned_type<value_type> || signed_type<value_type>) && !char_type<value_type>;

		template<typename value_type>
		concept has_substr = requires(jsonifier_internal::unwrap_t<value_type> value) {
			{
				value.substr(typename jsonifier_internal::unwrap_t<value_type>::size_type{}, typename jsonifier_internal::unwrap_t<value_type>::size_type{})
			} -> std::same_as<jsonifier_internal::unwrap_t<value_type>>;
		};

		template<typename value_type>
		concept string_t = has_substr<value_type> && has_data<value_type> && has_size<value_type> && !std::is_same_v<jsonifier_internal::unwrap_t<value_type>, char> &&
			vector_subscriptable<value_type> && !pointer_t<value_type>;

		template<typename value_type>
		concept map_t = requires(jsonifier_internal::unwrap_t<value_type> value) {
			typename jsonifier_internal::unwrap_t<value_type>::mapped_type;
			typename jsonifier_internal::unwrap_t<value_type>::key_type;
		} && range<value_type> && map_subscriptable<value_type>;

		template<typename value_type>
		concept pair_t = requires(jsonifier_internal::unwrap_t<value_type> value) {
			typename jsonifier_internal::unwrap_t<value_type>::first_type;
			typename jsonifier_internal::unwrap_t<value_type>::second_type;
		};

		template<typename value_type>
		concept has_emplace_back = requires(jsonifier_internal::unwrap_t<value_type> value) {
			{ value.emplace_back(typename jsonifier_internal::unwrap_t<value_type>::value_type{}) } -> std::same_as<typename jsonifier_internal::unwrap_t<value_type>::value_type&>;
		};

		template<typename value_type>
		concept has_control_byes = requires(jsonifier_internal::unwrap_t<value_type> value) {
			{ value.controlBytes };
		};

		template<typename value_type>
		concept has_reserve = requires(jsonifier_internal::unwrap_t<value_type> value) {
			{ value.reserve(typename jsonifier_internal::unwrap_t<value_type>::size_type{}) } -> std::same_as<void>;
		};

		template<typename value_type>
		concept has_capacity = requires(jsonifier_internal::unwrap_t<value_type> value) {
			{ value.capacity() } -> std::same_as<typename jsonifier_internal::unwrap_t<value_type>::size_type>;
		};

		template<typename value_type>
		concept has_release = requires(jsonifier_internal::unwrap_t<value_type> value) {
			{ value.release() } -> std::same_as<typename jsonifier_internal::unwrap_t<value_type>::pointer>;
		};

		template<typename value_type>
		concept copyable = std::copyable<jsonifier_internal::unwrap_t<value_type>>;

		template<typename value_type>
		concept unique_ptr_t = requires(jsonifier_internal::unwrap_t<value_type> value) {
			typename jsonifier_internal::unwrap_t<value_type>::element_type;
			typename jsonifier_internal::unwrap_t<value_type>::deleter_type;
		} && has_release<value_type>;

		template<typename value_type>
		concept shared_ptr_t = requires(jsonifier_internal::unwrap_t<value_type> value) {
			typename jsonifier_internal::unwrap_t<value_type>::element_type;
			typename jsonifier_internal::unwrap_t<value_type>::deleter_type;
		} && has_release<value_type> && copyable<value_type>;

		template<typename value_type>
		concept has_find = requires(jsonifier_internal::unwrap_t<value_type> value) {
			{ value.find(typename jsonifier_internal::unwrap_t<value_type>::key_type{}) };
		};

		template<typename value_type>
		concept has_excluded_keys = requires(jsonifier_internal::unwrap_t<value_type> value) {
			{ value.jsonifierExcludedKeys };
		};

		template<typename value_type>
		concept nullable_t = !string_t<value_type> && requires(jsonifier_internal::unwrap_t<value_type> value) {
			bool(value);
			{ *value };
		};

		template<typename value_type>
		concept is_double_ptr = std::same_as<const char**, jsonifier_internal::unwrap_t<value_type>> || std::same_as<char**, jsonifier_internal::unwrap_t<value_type>>;

		template<typename value_type>
		concept array_tuple_t = requires(jsonifier_internal::unwrap_t<value_type> value) {
			std::tuple_size<jsonifier_internal::unwrap_t<value_type>>::value;
			std::get<0>(value);
		} && jsonifier_internal::is_specialization_v<jsonifier_internal::unwrap_t<value_type>, jsonifier_internal::array_tuple>;

		template<typename value_type>
		concept null_t = nullable_t<value_type> || always_null_t<value_type>;

		template<typename value_type>
		concept raw_json_t = std::is_same_v<jsonifier_internal::unwrap_t<value_type>, jsonifier::raw_json_data>;

		template<typename value_type01, typename value_type02>
		concept same_character_size = requires() {
			sizeof(typename jsonifier_internal::unwrap_t<value_type01>::value_type) == sizeof(typename jsonifier_internal::unwrap_t<value_type02>::value_type);
		} && string_t<value_type01> && string_t<value_type02>;

		template<typename value_type>
		concept tuple_t = requires(jsonifier_internal::unwrap_t<value_type> t) {
			std::tuple_size<jsonifier_internal::unwrap_t<value_type>>::value;
			std::get<0>(t);
		};

		template<typename value_type> using decay_keep_volatile_t = std::remove_const_t<std::remove_reference_t<value_type>>;

		template<typename value_type>
		concept optional_t = jsonifier_internal::is_specialization_v<jsonifier_internal::unwrap_t<value_type>, std::optional>;

		template<typename value_type>
		concept enum_t = std::is_enum_v<jsonifier_internal::unwrap_t<value_type>>;

		template<typename value_type>
		concept vector_t = !map_t<value_type> && vector_subscriptable<value_type> && !has_substr<value_type> && !array_tuple_t<value_type> &&
			!std::is_pointer_v<jsonifier_internal::unwrap_t<value_type>>;

		template<typename value_type>
		concept jsonifier_t = requires { jsonifier::core<jsonifier_internal::unwrap_t<value_type>>::parseValue; };

		template<typename value_type>
		concept is_core_type = jsonifier_t<value_type> || vector_t<value_type> || array_tuple_t<value_type> || map_t<value_type> || tuple_t<value_type>;

		template<typename value_type>
		concept has_view = requires(jsonifier_internal::unwrap_t<value_type> value) { value.view(); };

		template<typename value_type>
		concept convertible_to_string_view = std::convertible_to<jsonifier_internal::unwrap_t<value_type>, std::string_view>;

		struct empty {
			static constexpr std::tuple<> val{};
		};

		template<typename value_type> constexpr auto coreWrapperV = [] {
			if constexpr (jsonifier_t<value_type>) {
				return jsonifier::core<value_type>::parseValue;
			} else {
				return empty{};
			}
		}();

		template<typename value_type> constexpr auto coreV = coreWrapperV<decay_keep_volatile_t<value_type>>.val;

		template<typename value_type> using core_t = decay_keep_volatile_t<decltype(coreV<value_type>)>;

		template<typename value_type> using core_wrapper_t = decay_keep_volatile_t<decltype(coreWrapperV<std::decay_t<value_type>>)>;

		template<typename value_type>
		concept jsonifier_scalar_value_t = jsonifier_t<value_type> && jsonifier_internal::is_specialization_v<core_wrapper_t<value_type>, scalar_value>;

		template<typename value_type>
		concept jsonifier_value_t = jsonifier_t<value_type> && jsonifier_internal::is_specialization_v<core_wrapper_t<value_type>, value>;

		template<typename value_type>
		concept raw_array_t = ( std::is_array_v<jsonifier_internal::unwrap_t<value_type>> && !std::is_pointer_v<jsonifier_internal::unwrap_t<value_type>> ) ||
			(vector_subscriptable<value_type> && !vector_t<value_type> && !has_substr<value_type> && !array_tuple_t<value_type>);

		template<typename value_type>
		concept buffer_like = vector_subscriptable<value_type> && has_data<value_type> && has_resize<value_type>;

		template<typename value_type> constexpr bool printErrorFunction() noexcept {
			using fail_type = typename value_type::fail_type;
			return false;
		}

		template<is_core_type value_type> constexpr bool printErrorFunction() noexcept {
			return true;
		}

		template<typename value_type>
		concept time_type = jsonifier_internal::is_specialization_v<std::chrono::duration<jsonifier_internal::unwrap_t<value_type>>, std::chrono::duration>;

		template<typename value_type>
		concept char_t = uint8_type<value_type> || char_type<value_type>;

		template<typename value_type>
		concept integer_t = std::integral<jsonifier_internal::unwrap_t<value_type>> && !bool_t<value_type> && !std::floating_point<jsonifier_internal::unwrap_t<value_type>>;
	}

}// namespace jsonifier_internal

namespace std {

	template<> struct variant_size<jsonifier::concepts::empty> : integral_constant<uint64_t, 0> {};

	template<typename... value_types> struct variant_size<jsonifier::value<value_types...>> : integral_constant<uint64_t, sizeof...(value_types)> {};

	template<typename... value_types> struct variant_size<std::tuple<value_types...>> : integral_constant<uint64_t, sizeof...(value_types)> {};

	template<typename... value_types> struct tuple_size<jsonifier::value<value_types...>> : integral_constant<uint64_t, sizeof...(value_types)> {};

	template<> struct tuple_size<jsonifier::concepts::empty> : integral_constant<uint64_t, 0> {};

	template<typename... value_types> struct tuple_size<jsonifier_internal::array_tuple<value_types...>> : integral_constant<uint64_t, sizeof...(value_types)> {};
}

namespace jsonifier_internal {

	template<typename value_type01, typename value_type02> constexpr value_type01 max(value_type01 value1, value_type02 value2) {
		return value1 > static_cast<value_type01>(value2) ? value1 : static_cast<value_type01>(value2);
	}

	template<jsonifier::concepts::unsigned_type value_type> void printBits(value_type values, const std::string& valuesTitle) noexcept {
		std::cout << valuesTitle;
		std::cout << std::bitset<sizeof(value_type) * 8>{ values };
		std::cout << std::endl;
	}

	template<jsonifier::concepts::simd_int_type simd_type> const simd_type& printBits(const simd_type& value, const std::string& valuesTitle) noexcept {
		JSONIFIER_ALIGN uint8_t values[sizeof(simd_type)]{};
		std::stringstream theStream{};
		store(value, values);
		std::cout << valuesTitle;
		for (jsonifier_string_parsing_type x = 0; x < sizeof(simd_type); ++x) {
			for (jsonifier_string_parsing_type y = 0; y < 8; ++y) {
				std::cout << std::bitset<1>{ static_cast<uint64_t>(*(values + x)) >> y };
			}
		}
		std::cout << std::endl;
		return value;
	}

	JSONIFIER_ALWAYS_INLINE std::string printBits(bool value) noexcept {
		std::stringstream theStream{};
		theStream << std::boolalpha << value << std::endl;
		return theStream.str();
	}

	template<typename simd_type> JSONIFIER_ALWAYS_INLINE std::string printBits(const simd_type& value) noexcept {
		JSONIFIER_ALIGN uint8_t values[sizeof(simd_type)]{};
		std::stringstream theStream{};
		store(value, values);
		for (uint64_t x = 0; x < bytesPerStep; ++x) {
			for (uint64_t y = 0; y < 8; ++y) {
				theStream << std::bitset<1>{ static_cast<uint64_t>(*(values + x)) >> y };
			}
		}
		theStream << std::endl;
		return theStream.str();
	}

	template<jsonifier::concepts::time_type value_type> class stop_watch {
	  public:
		using hr_clock = std::chrono::high_resolution_clock;

		JSONIFIER_ALWAYS_INLINE stop_watch(uint64_t newTime) noexcept {
			totalNumberOfTimeUnits.store(value_type{ newTime }, std::memory_order_release);
		}

		JSONIFIER_ALWAYS_INLINE stop_watch(value_type newTime) noexcept {
			totalNumberOfTimeUnits.store(newTime, std::memory_order_release);
		}

		JSONIFIER_ALWAYS_INLINE stop_watch& operator=(stop_watch&& other) noexcept {
			if (this != &other) [[likely]] {
				totalNumberOfTimeUnits.store(other.totalNumberOfTimeUnits.load(std::memory_order_acquire), std::memory_order_release);
				startTimeInTimeUnits.store(other.startTimeInTimeUnits.load(std::memory_order_acquire), std::memory_order_release);
			}
			return *this;
		}

		JSONIFIER_ALWAYS_INLINE stop_watch(stop_watch&& other) noexcept {
			*this = std::move(other);
		}

		JSONIFIER_ALWAYS_INLINE stop_watch& operator=(const stop_watch& other) noexcept {
			if (this != &other) [[likely]] {
				totalNumberOfTimeUnits.store(other.totalNumberOfTimeUnits.load(std::memory_order_acquire), std::memory_order_release);
				startTimeInTimeUnits.store(other.startTimeInTimeUnits.load(std::memory_order_acquire), std::memory_order_release);
			}
			return *this;
		}

		JSONIFIER_ALWAYS_INLINE stop_watch(const stop_watch& other) noexcept {
			*this = other;
		}

		JSONIFIER_ALWAYS_INLINE bool hasTimeElapsed() noexcept {
			if (std::chrono::duration_cast<value_type>(hr_clock::now().time_since_epoch()) - startTimeInTimeUnits.load(std::memory_order_acquire) >=
				totalNumberOfTimeUnits.load(std::memory_order_acquire)) [[likely]] {
				return true;
			} else {
				return false;
			}
		}

		JSONIFIER_ALWAYS_INLINE void reset(value_type newTimeValue = value_type{}) noexcept {
			if (newTimeValue != value_type{}) [[likely]] {
				totalNumberOfTimeUnits.store(newTimeValue, std::memory_order_release);
				startTimeInTimeUnits.store(std::chrono::duration_cast<value_type>(hr_clock::now().time_since_epoch()), std::memory_order_release);
			} else {
				startTimeInTimeUnits.store(std::chrono::duration_cast<value_type>(hr_clock::now().time_since_epoch()), std::memory_order_release);
			}
		}

		JSONIFIER_ALWAYS_INLINE value_type getTotalWaitTime() const noexcept {
			return totalNumberOfTimeUnits.load(std::memory_order_acquire);
		}

		JSONIFIER_ALWAYS_INLINE value_type totalTimeElapsed() noexcept {
			return std::chrono::duration_cast<value_type>(hr_clock::now().time_since_epoch()) - startTimeInTimeUnits.load(std::memory_order_acquire);
		}

	  protected:
		std::atomic<value_type> totalNumberOfTimeUnits{};
		std::atomic<value_type> startTimeInTimeUnits{};
	};

	template<jsonifier::concepts::time_type value_type> stop_watch(value_type) -> stop_watch<value_type>;
}

#include <jsonifier/ISA/AVX.hpp>
#include <jsonifier/ISA/Lzcount.hpp>
#include <jsonifier/ISA/Popcount.hpp>
#include <jsonifier/ISA/Bmi2.hpp>
#include <jsonifier/ISA/Bmi.hpp>
#include <jsonifier/ISA/Fallback.hpp>