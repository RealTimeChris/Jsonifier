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

#include <jsonifier/TypeEntities.hpp>
#include <jsonifier/StringLiteral.hpp>
#include <jsonifier/StringView.hpp>
#include <jsonifier/Core.hpp>
#include <source_location>

namespace jsonifier_internal {

	/**
	 * @brief External template variable declaration.
	 *
	 * Declare an external template variable to be defined elsewhere.
	 *
	 * @tparam value_type The type of the external template variable.
	 */
	template<typename value_type> extern const value_type external;

	/**
	 * @brief Struct to remove member pointers.
	 *
	 * Define a struct to remove member pointers from the given type.
	 *
	 * @tparam value_type The type from which to remove member pointers.
	 */
	template<typename member_type> struct remove_member_pointer {
		using type = member_type;
	};

	template<typename value_type, typename member_type> struct remove_member_pointer<member_type value_type::*> {
		using type = value_type;
	};

	template<typename value_type, typename member_type, typename... arg_types> struct remove_member_pointer<member_type (value_type::*)(arg_types...)> {
		using type = value_type;
	};

	template<typename value_type> using remove_member_pointer_t = typename remove_member_pointer<value_type>::type;

#if defined(JSONIFIER_CLANG)
	constexpr auto pretty_function_tail = "]";
#elif defined(JSONIFIER_GNUCXX)
	constexpr auto pretty_function_tail = ";";
#endif

	/**
	 * @brief Get the name of a member pointer.
	 *
	 * function_typetion to extract the name of a member pointer.
	 *
	 * @tparam p The member pointer.
	 * @return The name of the member pointer.
	 */
#if defined(JSONIFIER_MSVC) && !defined(JSONIFIER_CLANG)
	template<typename value_type, auto p> consteval jsonifier::string_view getNameImpl() noexcept {
		jsonifier::string_view str = std::source_location::current().function_name();
		str						   = str.substr(str.find("->") + 2);
		return str.substr(0, str.find(">"));
	}
#else
	template<auto p> consteval jsonifier::string_view getNameImpl() noexcept {
		jsonifier::string_view str = std::source_location::current().function_name();
		str						   = str.substr(str.find("&") + 1);
		str						   = str.substr(0, str.find(pretty_function_tail));
		return str.substr(str.rfind("::") + 2);
	}
#endif

	template<auto p>
		requires(std::is_member_pointer_v<decltype(p)>)
	constexpr auto getName() noexcept {
#if defined(JSONIFIER_MSVC) && !defined(JSONIFIER_CLANG)
		using value_type		 = remove_member_pointer_t<std::remove_cvref_t<decltype(p)>>;
		constexpr auto pNew		 = p;
		constexpr auto newString = getNameImpl<value_type, &(external<value_type>.*pNew)>();
#else
		constexpr auto newString = getNameImpl<p>();
#endif
		return make_static<stringLiteralFromView<newString.size()>(newString)>::value.view();
	}

	template<typename value_type> static constexpr auto getJsonTypePre() {
		if constexpr (jsonifier::concepts::jsonifier_object_t<value_type> || jsonifier::concepts::map_t<value_type>) {
			return jsonifier::json_type::object;
		} else if constexpr (jsonifier::concepts::raw_array_t<value_type> || jsonifier::concepts::vector_t<value_type>) {
			return jsonifier::json_type::array;
		} else if constexpr (jsonifier::concepts::string_t<value_type> || jsonifier::concepts::string_view_t<value_type>) {
			return jsonifier::json_type::string;
		} else if constexpr (jsonifier::concepts::bool_t<value_type>) {
			return jsonifier::json_type::boolean;
		} else if constexpr (jsonifier::concepts::num_t<value_type> || jsonifier::concepts::enum_t<value_type>) {
			return jsonifier::json_type::number;
		} else if constexpr (jsonifier::concepts::always_null_t<value_type>) {
			return jsonifier::json_type::null;
		} else {
			return jsonifier::json_type::accessor;
		}
	}

	template<jsonifier::json_type type, typename value_type> static constexpr auto setJsonType() {
		if constexpr (type == jsonifier::json_type::object) {
			static_assert(( jsonifier::concepts::jsonifier_object_t<value_type> || jsonifier::concepts::map_t<value_type> || jsonifier::concepts::tuple_t<value_type> ),
				"Sorry, but that is not a valid value of type 'object'.");
		} else if constexpr (type == jsonifier::json_type::array) {
			static_assert(( jsonifier::concepts::vector_t<value_type> || jsonifier::concepts::raw_array_t<value_type> ), "Sorry, but that is not a valid value of type 'array'.");
		} else if constexpr (type == jsonifier::json_type::string) {
			static_assert(( jsonifier::concepts::string_t<value_type> || jsonifier::concepts::string_view_t<value_type> ),
				"Sorry, but that is not a valid value of type 'string'.");
		} else if constexpr (type == jsonifier::json_type::number) {
			static_assert(( jsonifier::concepts::num_t<value_type> || jsonifier::concepts::enum_t<value_type> ), "Sorry, but that is not a valid value of type 'number'.");
		} else if constexpr (type == jsonifier::json_type::boolean) {
			static_assert(( jsonifier::concepts::bool_t<value_type> ), "Sorry, but that is not a valid value of type 'boolean'.");
		} else if constexpr (type == jsonifier::json_type::null) {
			static_assert(( jsonifier::concepts::always_null_t<value_type> ), "Sorry, but that is not a valid value of type 'null'.");
		}
		return type;
	}

	template<size_t indexNew, typename member_type_new, typename class_type_new> struct json_entity {
		using member_type = std::remove_cvref_t<member_type_new>;
		using class_type  = std::remove_cvref_t<class_type_new>;
		static constexpr size_t index{ indexNew };
		member_type class_type::*memberPtr{};
		jsonifier::string_view name{};
		jsonifier::json_type type{};
		bool isItLast{ false };

		constexpr json_entity() noexcept = default;

		template<auto ptrNew> constexpr json_entity() : memberPtr{ ptrNew }, name{ getName<ptrNew>() }, type{} {
		}

		constexpr json_entity(member_type_new class_type_new::*ptr) noexcept : memberPtr{ ptr }, name{}, type{ getJsonTypePre<member_type_new>() } {
		}

		constexpr json_entity(member_type class_type::*ptr, jsonifier::string_view str, jsonifier::json_type typeNew) noexcept : memberPtr{ ptr }, name{ str }, type{ typeNew } {
		}

		constexpr auto& view() const noexcept {
			return name;
		}

		constexpr auto& ptr() const noexcept {
			return memberPtr;
		}
	};

	template<size_t index, typename member_type, typename class_type> json_entity(member_type, class_type) -> json_entity<index, member_type, class_type>;

	template<size_t index, typename member_type, typename class_type> json_entity(member_type class_type::*) -> json_entity<index, member_type, class_type>;

	template<typename T>
	concept is_json_entity = requires {
		typename T::member_type;
		typename T::class_type;
		T::memberPtr;
		{ T::name } -> std::same_as<jsonifier::string_view>;
		{ T::type } -> std::same_as<jsonifier::json_type>;
		{ T::index } -> std::same_as<size_t>;
	};

	template<typename T>
	concept is_data_member_ptr = requires { std::is_member_pointer_v<T>; };

	template<typename T>
	concept convertible_to_json_entity = is_json_entity<T> || is_data_member_ptr<T>;

	template<size_t maxIndex, size_t index, typename member_type, typename class_type>
	constexpr auto makeJsonEntityAuto(jsonifier::string_view str, member_type class_type::*ptr) noexcept {
		return json_entity<index, member_type, class_type>{ ptr, str, getJsonTypePre<member_type>() };
	}

	template<size_t maxIndex, size_t index, auto value> constexpr auto makeJsonEntityAuto() noexcept {
		if constexpr (jsonifier::concepts::is_json_entity<decltype(value)>) {
			using member_type = typename decltype(json_entity{ value })::member_type;
			using class_type  = typename decltype(json_entity{ value })::class_type;
			json_entity<index, member_type, class_type> jsonEntity{ value };
			if constexpr (!jsonifier::concepts::vector_t<typename decltype(jsonEntity)::member_type> && !jsonifier::concepts::map_t<typename decltype(jsonEntity)::member_type>) {
				if constexpr (maxIndex - index == 1 && maxIndex > 1) {
					jsonEntity.isItLast = true;
				}
			}
			return jsonEntity;
		} else {
			using member_type = typename decltype(makeJsonEntityAuto<maxIndex, index>(getName<value>(), value))::member_type;
			using class_type  = typename decltype(makeJsonEntityAuto<maxIndex, index>(getName<value>(), value))::class_type;
			json_entity<index, member_type, class_type> jsonEntity{ makeJsonEntityAuto<maxIndex, index>(getName<value>(), value) };
			if constexpr (!jsonifier::concepts::vector_t<typename decltype(jsonEntity)::member_type> && !jsonifier::concepts::map_t<typename decltype(jsonEntity)::member_type>) {
				if constexpr (maxIndex - index == 1 && maxIndex > 1) {
					jsonEntity.isItLast = true;
				}
			}
			return jsonEntity;
		}
	}

	template<auto... values, size_t... indices> constexpr auto createValueImpl(std::index_sequence<indices...>) {
		static_assert((convertible_to_json_entity<decltype(values)> && ...), "All arguments passed to createValue should be constructible from a json_entity.");
		return jsonifier::value{ makeTuple(makeJsonEntityAuto<sizeof...(values), indices, values>()...) };
	}

}

namespace jsonifier {

	template<auto testPtr, jsonifier_internal::string_literal nameNew> constexpr auto make_json_entity() {
		constexpr jsonifier_internal::string_literal name{ nameNew };
		return jsonifier_internal::makeJsonEntityAuto(jsonifier_internal::make_static<name>::value.view(), testPtr);
	}

	template<auto testPtr> constexpr auto make_json_entity() {
		return jsonifier_internal::makeJsonEntityAuto(jsonifier_internal::getName<testPtr>(), testPtr);
	}

	template<auto testPtr, json_type type, jsonifier_internal::string_literal nameNew> constexpr auto make_json_entity() {
		constexpr jsonifier_internal::string_literal name{ nameNew };
		auto jsonEntity = jsonifier_internal::makeJsonEntityAuto(jsonifier_internal::make_static<name>::value.view(), testPtr);
		jsonEntity.type = jsonifier_internal::setJsonType<type, typename decltype(jsonEntity)::member_type>();
		return jsonEntity;
	}

	template<auto testPtr, json_type type> constexpr auto make_json_entity() {
		auto jsonEntity = jsonifier_internal::makeJsonEntityAuto(jsonifier_internal::getName<testPtr>(), testPtr);
		jsonEntity.type = jsonifier_internal::setJsonType<type, typename decltype(jsonEntity)::member_type>();
		return jsonEntity;
	}

	template<auto... values> constexpr auto createValue() noexcept {
		return jsonifier_internal::createValueImpl<values...>(std::make_index_sequence<sizeof...(values)>{});
	}

}