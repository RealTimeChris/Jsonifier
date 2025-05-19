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

#include <jsonifier/ISA/SimdTypes.hpp>
#include <jsonifier/Tuple.hpp>
#include <source_location>
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

	template<class value_type> JSONIFIER_ALWAYS_INLINE constexpr value_type&& forward(std::remove_reference_t<value_type>& _Arg) noexcept {
		return static_cast<value_type&&>(_Arg);
	}

	template<class value_type> JSONIFIER_ALWAYS_INLINE constexpr value_type&& forward(std::remove_reference_t<value_type>&& _Arg) noexcept {
		static_assert(!std::is_lvalue_reference_v<value_type>, "bad jsonifier_internal::forward call");
		return static_cast<value_type&&>(_Arg);
	}

	template<class value_type> JSONIFIER_ALWAYS_INLINE constexpr std::remove_reference_t<value_type>&& move(value_type&& _Arg) noexcept {
		return static_cast<std::remove_reference_t<value_type>&&>(_Arg);
	}

	JSONIFIER_ALWAYS_INLINE std::ostream& operator<<(std::ostream& os, const std::source_location& location) {
		os << "File: " << location.file_name() << std::endl;
		os << "Line: " << location.line() << std::endl;
		return os;
	}

	JSONIFIER_ALWAYS_INLINE void printLocation(const std::source_location& location = std::source_location::current()) {
		std::cout << location;
	}

	template<auto memberPtrNew> struct json_entity;

	template<typename derived_type> class parser;

	// from
	// https://stackoverflow.com/questions/16337610/how-to-know-if-a-type-is-a-specialization-of-stdvector
	template<typename, template<typename...> typename> constexpr bool is_specialization_v = false;

	template<template<typename...> typename value_type, typename... arg_types> constexpr bool is_specialization_v<value_type<arg_types...>, value_type> = true;

	template<typename, template<auto...> typename> constexpr bool is_specialization_v_values = false;

	template<template<auto...> typename Template, auto... values> constexpr bool is_specialization_v_values<Template<values...>, Template> = true;

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

	template<typename value_type> struct get_int_type {
		using type = std::conditional_t<std::is_unsigned_v<std::remove_cvref_t<value_type>>, uint8_t, int8_t>;
	};

	template<const auto& function, typename... arg_types, size_t... indices>
	JSONIFIER_INLINE constexpr void forEachImpl(std::index_sequence<indices...>, arg_types&&... args) noexcept {
		void(args...);
		(function.operator()(std::integral_constant<size_t, indices>{}, std::integral_constant<size_t, sizeof...(indices)>{}, jsonifier_internal::forward<arg_types>(args)...),
			...);
	}

	template<size_t limit, const auto& function, typename... arg_types> JSONIFIER_INLINE constexpr void forEach(arg_types&&... args) noexcept {
		forEachImpl<function>(std::make_index_sequence<limit>{}, jsonifier_internal::forward<arg_types>(args)...);
	}

	template<const auto& function, uint64_t currentIndex = 0, typename variant_type, typename... arg_types>
	JSONIFIER_INLINE constexpr void visit(variant_type&& variant, arg_types&&... args) noexcept {
		if constexpr (currentIndex < std::variant_size_v<std::remove_cvref_t<variant_type>>) {
			variant_type&& variantNew = jsonifier_internal::forward<variant_type>(variant);
			if JSONIFIER_UNLIKELY (variantNew.index() == currentIndex) {
				function(std::get<currentIndex>(jsonifier_internal::forward<variant_type>(variantNew)), jsonifier_internal::forward<arg_types>(args)...);
				return;
			}
			visit<function, currentIndex + 1>(jsonifier_internal::forward<variant_type>(variantNew), jsonifier_internal::forward<arg_types>(args)...);
		}
	}

}

namespace jsonifier {

	enum class json_type {
		object	 = 0,
		array	 = 1,
		string	 = 2,
		number	 = 3,
		boolean	 = 4,
		null	 = 5,
		accessor = 6,
		unset	 = 7,
	};

	template<typename value_type> class string_view_base;

	template<typename value_type, uint64_t> class string_base;

	using string = string_base<char, 0>;

	using string_view = string_view_base<char>;

	class raw_json_data;

	template<typename value_type_new, uint64_t sizeVal = 0> class vector;

	template<typename value_type> struct core;

	template<typename value_type> struct value {
		value_type val{};
		json_type type{};

		constexpr value(value_type valNew, json_type typeNew) : val{ valNew }, type{ typeNew } {
		}

		constexpr value(value_type valNew) : val{ valNew }, type{} {
		}
	};

	template<typename value_type> value(value_type, json_type) -> value<value_type>;

	template<typename value_type> value(value_type) -> value<value_type>;

	struct skip {};

	template<auto... valuesNew> struct value_holder;

	namespace concepts {

		template<typename value_type, auto... values>
		concept value_holder_t = jsonifier_internal::is_specialization_v_values<value_type, value_holder>;

		template<typename value_type>
		concept skip_t = std::is_same_v<std::remove_cvref_t<value_type>, skip>;

		template<typename value_type>
		concept is_json_entity = requires {
			typename std::remove_cvref_t<value_type>::member_type;
			typename std::remove_cvref_t<value_type>::class_type;
			std::remove_cvref_t<value_type>::memberPtr;
			{ std::remove_cvref_t<value_type>::name };
			{ std::remove_cvref_t<value_type>::type };
		} && !std::is_member_pointer_v<std::remove_cvref_t<value_type>>;

		template<typename value_type>
		concept has_range = requires(std::remove_cvref_t<value_type> value) {
			{ value.begin() };
			{ value.end() };
		};

		template<typename value_type>
		concept map_subscriptable = requires(std::remove_cvref_t<value_type> value) {
			{ value[typename std::remove_cvref_t<value_type>::key_type{}] } -> std::same_as<const typename std::remove_cvref_t<value_type>::mapped_type&>;
		} || requires(std::remove_cvref_t<value_type> value) {
			{ value[typename std::remove_cvref_t<value_type>::key_type{}] } -> std::same_as<typename std::remove_cvref_t<value_type>::mapped_type&>;
		};

		template<typename value_type>
		concept vector_subscriptable = requires(std::remove_cvref_t<value_type> value) {
			{ value[typename std::remove_cvref_t<value_type>::size_type{}] } -> std::same_as<typename std::remove_cvref_t<value_type>::const_reference>;
		} || requires(std::remove_cvref_t<value_type> value) {
			{ value[typename std::remove_cvref_t<value_type>::size_type{}] } -> std::same_as<typename std::remove_cvref_t<value_type>::reference>;
		};

		template<typename value_type>
		concept has_size = requires(std::remove_cvref_t<value_type> value) {
			{ value.size() } -> std::same_as<typename std::remove_cvref_t<value_type>::size_type>;
		};

		template<typename value_type>
		concept has_empty = requires(std::remove_cvref_t<value_type> value) {
			{ value.empty() } -> std::same_as<bool>;
		};

		template<typename value_type>
		concept variant_t = requires(std::remove_cvref_t<value_type> var) {
			{ var.index() } -> std::same_as<size_t>;
			{ var.valueless_by_exception() } -> std::same_as<bool>;
			{ std::holds_alternative<decltype(std::get<0>(var))>(var) } -> std::same_as<bool>;
			{ std::get<0>(var) } -> std::same_as<decltype(std::get<0>(var))&>;
			{ std::get_if<0>(&var) } -> std::same_as<std::remove_cvref_t<decltype(std::get<0>(var))>*>;
		};

		template<typename value_type>
		concept has_resize = requires(std::remove_cvref_t<value_type> value) { value.resize(typename std::remove_cvref_t<value_type>::size_type{}); };

		template<typename value_type>
		concept has_reserve = requires(std::remove_cvref_t<value_type> value) { value.reserve(typename std::remove_cvref_t<value_type>::size_type{}); };

		template<typename value_type>
		concept has_data = requires(std::remove_cvref_t<value_type> value) {
			{ value.data() };
		};

		template<typename value_type>
		concept stateless = std::is_empty_v<std::remove_cvref_t<value_type>>;

		template<typename value_type>
		concept bool_t = std::same_as<std::remove_cvref_t<value_type>, bool> || std::same_as<std::remove_cvref_t<value_type>, std::vector<bool>::reference> ||
			std::same_as<std::remove_cvref_t<value_type>, std::vector<bool>::const_reference>;

		template<typename value_type>
		concept always_null_t = std::same_as<std::remove_cvref_t<value_type>, std::nullptr_t> || std::same_as<std::remove_cvref_t<value_type>, std::monostate> ||
			std::same_as<std::remove_cvref_t<value_type>, std::nullopt_t>;

		template<typename value_type>
		concept pointer_t = (std::is_pointer_v<std::remove_cvref_t<value_type>> ||
								( std::is_null_pointer_v<std::remove_cvref_t<value_type>> && !std::is_array_v<std::remove_cvref_t<value_type>> )) &&
			!always_null_t<value_type>;

		template<typename value_type>
		concept signed_t = std::signed_integral<std::remove_cvref_t<value_type>> && !bool_t<value_type>;

		template<typename value_type>
		concept unsigned_t = std::unsigned_integral<std::remove_cvref_t<value_type>> && !bool_t<value_type>;

		template<typename value_type>
		concept uns8_t = sizeof(std::remove_cvref_t<value_type>) == 1 && unsigned_t<value_type>;

		template<typename value_type>
		concept uns16_t = sizeof(std::remove_cvref_t<value_type>) == 2 && unsigned_t<value_type>;

		template<typename value_type>
		concept uns32_t = sizeof(std::remove_cvref_t<value_type>) == 4 && unsigned_t<value_type>;

		template<typename value_type>
		concept uns64_t = sizeof(std::remove_cvref_t<value_type>) == 8 && unsigned_t<value_type>;

		template<typename value_type>
		concept sig8_t = sizeof(std::remove_cvref_t<value_type>) == 1 && signed_t<value_type>;

		template<typename value_type>
		concept sig16_t = sizeof(std::remove_cvref_t<value_type>) == 2 && signed_t<value_type>;

		template<typename value_type>
		concept sig32_t = sizeof(std::remove_cvref_t<value_type>) == 4 && signed_t<value_type>;

		template<typename value_type>
		concept sig64_t = sizeof(std::remove_cvref_t<value_type>) == 8 && signed_t<value_type>;

		template<typename value_type>
		concept float_t = std::floating_point<std::remove_cvref_t<value_type>> && (std::numeric_limits<std::remove_cvref_t<value_type>>::radix == 2) &&
			std::numeric_limits<std::remove_cvref_t<value_type>>::is_iec559;

		template<typename value_type>
		concept void_t = std::is_void_v<std::remove_cvref_t<value_type>>;

		template<typename value_type>
		concept char_t = std::same_as<std::remove_cvref_t<value_type>, char>;

		template<typename value_type>
		concept num_t = (float_t<value_type> || unsigned_t<value_type> || signed_t<value_type>) && !char_t<value_type>;

		template<typename value_type>
		concept has_substr = requires(std::remove_cvref_t<value_type> value) {
			{
				value.substr(typename std::remove_cvref_t<value_type>::size_type{}, typename std::remove_cvref_t<value_type>::size_type{})
			} -> std::same_as<std::remove_cvref_t<value_type>>;
		};

		template<typename value_type>
		concept has_find = requires(std::remove_cvref_t<value_type> value) {
			{ value.find(typename std::remove_cvref_t<value_type>::value_type{}) } -> std::same_as<typename std::remove_cvref_t<value_type>::size_type>;
		} || requires(std::remove_cvref_t<value_type> value) {
			{ value.find(typename std::remove_cvref_t<value_type>::key_type{}) } -> std::same_as<typename std::remove_cvref_t<value_type>::iterator>;
		} || requires(std::remove_cvref_t<value_type> value) {
			{ value.find(typename std::remove_cvref_t<value_type>::key_type{}) } -> std::same_as<typename std::remove_cvref_t<value_type>::const_iterator>;
		};

		template<typename value_type>
		concept string_t = has_substr<value_type> && has_data<value_type> && has_size<value_type> && vector_subscriptable<value_type> && has_find<value_type>;

		template<typename value_type>
		concept string_view_t =
			has_substr<value_type> && has_data<value_type> && has_size<value_type> && vector_subscriptable<value_type> && has_find<value_type> && !has_resize<value_type>;

		template<typename value_type>
		concept map_t = map_subscriptable<value_type> && has_range<value_type> && has_size<value_type> && has_find<value_type> && has_empty<value_type>;

		template<typename value_type>
		concept pair_t = requires(std::remove_cvref_t<value_type> value) {
			typename std::remove_cvref_t<value_type>::first_type;
			typename std::remove_cvref_t<value_type>::second_type;
		};

		template<typename value_type>
		concept has_emplace_back = requires(std::remove_cvref_t<value_type> value) {
			{ value.emplace_back(typename std::remove_cvref_t<value_type>::value_type{}) } -> std::same_as<typename std::remove_cvref_t<value_type>::reference>;
		};

		template<typename value_type>
		concept has_release = requires(std::remove_cvref_t<value_type> value) {
			{ value.release() } -> std::same_as<typename std::remove_cvref_t<value_type>::pointer>;
		};

		template<typename value_type>
		concept has_reset = requires(std::remove_cvref_t<value_type> value) {
			{ value.reset() } -> std::same_as<void>;
		};

		template<typename value_type>
		concept has_get = requires(std::remove_cvref_t<value_type> value) {
			{ value.get() } -> std::same_as<typename std::remove_cvref_t<value_type>::element_type*>;
		};

		template<typename value_type>
		concept copyable = std::copyable<std::remove_cvref_t<value_type>>;

		template<typename value_type>
		concept unique_ptr_t = requires(std::remove_cvref_t<value_type> value) {
			typename std::remove_cvref_t<value_type>::element_type;
			typename std::remove_cvref_t<value_type>::deleter_type;
		} && has_release<value_type> && has_get<value_type>;

		template<typename value_type>
		concept shared_ptr_t = has_reset<value_type> && has_get<value_type> && copyable<value_type>;

		template<typename value_type>
		concept has_excluded_keys = requires(std::remove_cvref_t<value_type> value) {
			{ value.jsonifierExcludedKeys };
		};

		template<typename value_type>
		concept nullable_t = !string_t<value_type> && requires(std::remove_cvref_t<value_type> value) {
			bool(value);
			{ *value };
		};

		template<typename value_type>
		concept null_t = nullable_t<value_type> || always_null_t<value_type>;

		template<typename value_type>
		concept raw_json_t = std::same_as<std::remove_cvref_t<value_type>, jsonifier::raw_json_data>;

		template<typename value_type01, typename value_type02>
		concept same_character_size = requires() {
			sizeof(typename std::remove_cvref_t<value_type01>::value_type) == sizeof(typename std::remove_cvref_t<value_type02>::value_type);
		} && string_t<value_type01> && string_t<value_type02>;

		template<typename value_type> constexpr bool hasSizeEqualToZero{ std::tuple_size_v<std::remove_cvref_t<value_type>> == 0 };

		template<typename value_type>
		concept has_get_template = requires(std::remove_cvref_t<value_type> value) {
			{ std::get<0>(value) } -> std::same_as<decltype(std::get<0>(value))>;
		};

		template<typename value_type>
		concept tuple_t = requires() { std::tuple_size<std::remove_cvref_t<value_type>>::value; } && !has_data<value_type>;

		template<typename value_type> using decay_keep_volatile_t = std::remove_const_t<std::remove_reference_t<value_type>>;

		template<typename value_type>
		concept optional_t = requires(std::remove_cvref_t<value_type> opt) {
			{ opt.has_value() } -> std::same_as<bool>;
			{ opt.value() } -> std::same_as<typename std::remove_cvref_t<value_type>::value_type&>;
			{ *opt } -> std::same_as<typename std::remove_cvref_t<value_type>::value_type&>;
			{ opt.reset() } -> std::same_as<void>;
			{ opt.emplace(typename std::remove_cvref_t<value_type>::value_type{}) } -> std::same_as<typename std::remove_cvref_t<value_type>::value_type&>;
		};

		template<typename value_type>
		concept enum_t = std::is_enum_v<std::remove_cvref_t<value_type>>;

		template<typename value_type>
		concept vector_t = vector_subscriptable<value_type> && has_resize<value_type> && has_emplace_back<value_type>;

		template<typename value_type>
		concept jsonifier_t = requires { jsonifier::core<std::remove_cvref_t<value_type>>::parseValue; };

		template<typename value_type>
		concept is_core_t = jsonifier_t<value_type> || vector_t<value_type> || map_t<value_type> || tuple_t<value_type> || shared_ptr_t<value_type>;

		template<typename value_type>
		concept has_view = requires(std::remove_cvref_t<value_type> value) { value.view(); };

		template<typename value_type>
		concept convertible_to_string_view = std::convertible_to<std::remove_cvref_t<value_type>, std::string_view>;

		struct empty {
			static constexpr jsonifier_internal::tuple<> val{};
		};

		template<typename value_type> constexpr auto coreWrapperV = [] {
			if constexpr (jsonifier_t<value_type>) {
				return jsonifier::core<value_type>::parseValue;
			} else {
				return empty{};
			}
		}();

		template<typename value_type>
		concept has_member_t = requires { typename std::remove_cvref_t<value_type>::member_type; };

		template<typename value_type>
		concept has_json_type = requires {
			{ std::remove_cvref_t<value_type>::type } -> std::same_as<jsonifier::json_type>;
		};

		template<typename value_type>
		concept is_resizable = has_resize<value_type> && has_reserve<value_type> && !std::is_const_v<std::remove_reference_t<value_type>>;

		template<typename value_type> constexpr auto coreV = coreWrapperV<decay_keep_volatile_t<value_type>>.val;

		template<typename value_type> using core_wrapper_t = decay_keep_volatile_t<decltype(coreWrapperV<std::decay_t<value_type>>)>;

		template<typename value_type>
		concept jsonifier_object_t = jsonifier_t<value_type> && jsonifier_internal::is_specialization_v<core_wrapper_t<value_type>, value>;

		template<typename value_type>
		concept raw_array_t = ( std::is_array_v<std::remove_cvref_t<value_type>> && !std::is_pointer_v<std::remove_cvref_t<value_type>> ) ||
			(vector_subscriptable<value_type> && !vector_t<value_type> && !has_substr<value_type> && !tuple_t<value_type>);

		template<typename value_type>
		concept buffer_like = vector_subscriptable<value_type> && has_data<value_type> && has_resize<value_type>;

		template<typename value_type> constexpr bool printErrorFunction() noexcept {
			using fail_type = typename std::remove_cvref_t<value_type>::fail_type;
			return false;
		}

		template<is_core_t value_type> constexpr bool printErrorFunction() noexcept {
			return true;
		}

		template<typename value_type>
		concept accessor_t = optional_t<value_type> || variant_t<value_type> || pointer_t<value_type> || shared_ptr_t<value_type> || unique_ptr_t<value_type>;

		template<typename value_type>
		concept time_t = jsonifier_internal::is_specialization_v<std::chrono::duration<std::remove_cvref_t<value_type>>, std::chrono::duration>;

		template<typename value_type>
		concept integer_t = std::integral<std::remove_cvref_t<value_type>> && !bool_t<value_type> && !std::floating_point<std::remove_cvref_t<value_type>>;
	}

}// namespace jsonifier_internal

namespace jsonifier_internal {

	template<typename... value_types> struct tuple_size<jsonifier::value<value_types...>> : std::integral_constant<uint64_t, sizeof...(value_types)> {};

	template<> struct tuple_size<jsonifier::concepts::empty> : std::integral_constant<uint64_t, 0> {};
}

#if defined(max)
	#undef max
#endif

#if defined(min)
	#undef min
#endif

namespace jsonifier_internal {

	template<typename value_type01, typename value_type02> constexpr value_type01 max(value_type01 value1, value_type02 value2) {
		return value1 > static_cast<value_type01>(value2) ? value1 : static_cast<value_type01>(value2);
	}

	template<typename value_type01, typename value_type02> constexpr value_type01 min(value_type01 value1, value_type02 value2) {
		return value1 < static_cast<value_type01>(value2) ? value1 : static_cast<value_type01>(value2);
	}

	template<jsonifier::concepts::unsigned_t value_type> void printBits(value_type values, const std::string& valuesTitle) noexcept {
		std::cout << valuesTitle;
		std::cout << std::bitset<sizeof(value_type) * 8>{ values };
		std::cout << std::endl;
	}

	JSONIFIER_INLINE std::string printBits(bool value) noexcept {
		std::stringstream theStream{};
		theStream << std::boolalpha << value << std::endl;
		return theStream.str();
	}

	template<jsonifier::concepts::time_t value_type> class stop_watch {
	  public:
		using hr_clock = std::chrono::high_resolution_clock;

		JSONIFIER_INLINE stop_watch(uint64_t newTime) noexcept {
			totalNumberOfTimeUnits.store(value_type{ newTime }, std::memory_order_release);
		}

		JSONIFIER_INLINE stop_watch(value_type newTime) noexcept {
			totalNumberOfTimeUnits.store(newTime, std::memory_order_release);
		}

		JSONIFIER_INLINE stop_watch& operator=(stop_watch&& other) noexcept {
			if JSONIFIER_LIKELY (this != &other) {
				totalNumberOfTimeUnits.store(other.totalNumberOfTimeUnits.load(std::memory_order_acquire), std::memory_order_release);
				startTimeInTimeUnits.store(other.startTimeInTimeUnits.load(std::memory_order_acquire), std::memory_order_release);
			}
			return *this;
		}

		JSONIFIER_INLINE stop_watch(stop_watch&& other) noexcept {
			*this = jsonifier_internal::move(other);
		}

		JSONIFIER_INLINE stop_watch& operator=(const stop_watch& other) noexcept {
			if JSONIFIER_LIKELY (this != &other) {
				totalNumberOfTimeUnits.store(other.totalNumberOfTimeUnits.load(std::memory_order_acquire), std::memory_order_release);
				startTimeInTimeUnits.store(other.startTimeInTimeUnits.load(std::memory_order_acquire), std::memory_order_release);
			}
			return *this;
		}

		JSONIFIER_INLINE stop_watch(const stop_watch& other) noexcept {
			*this = other;
		}

		JSONIFIER_INLINE bool hasTimeElapsed() noexcept {
			if JSONIFIER_LIKELY (std::chrono::duration_cast<value_type>(hr_clock::now().time_since_epoch()) - startTimeInTimeUnits.load(std::memory_order_acquire) >=
				totalNumberOfTimeUnits.load(std::memory_order_acquire)) {
				return true;
			} else {
				return false;
			}
		}

		JSONIFIER_INLINE void reset(value_type newTimeValue = value_type{}) noexcept {
			if JSONIFIER_LIKELY (newTimeValue != value_type{}) {
				totalNumberOfTimeUnits.store(newTimeValue, std::memory_order_release);
				startTimeInTimeUnits.store(std::chrono::duration_cast<value_type>(hr_clock::now().time_since_epoch()), std::memory_order_release);
			} else {
				startTimeInTimeUnits.store(std::chrono::duration_cast<value_type>(hr_clock::now().time_since_epoch()), std::memory_order_release);
			}
		}

		JSONIFIER_INLINE value_type getTotalWaitTime() const noexcept {
			return totalNumberOfTimeUnits.load(std::memory_order_acquire);
		}

		JSONIFIER_INLINE value_type totalTimeElapsed() noexcept {
			return std::chrono::duration_cast<value_type>(hr_clock::now().time_since_epoch()) - startTimeInTimeUnits.load(std::memory_order_acquire);
		}

	  protected:
		std::atomic<value_type> totalNumberOfTimeUnits{};
		std::atomic<value_type> startTimeInTimeUnits{};
	};

	template<jsonifier::concepts::time_t value_type> stop_watch(value_type) -> stop_watch<value_type>;
}

#include <jsonifier/ISA/Lzcount.hpp>
#include <jsonifier/ISA/Popcount.hpp>