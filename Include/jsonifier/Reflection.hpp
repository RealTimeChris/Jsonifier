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

	template<typename class_type, typename member_type> struct remove_member_pointer<member_type class_type::*> {
		using type = class_type;
	};

	template<typename value_type> using remove_member_pointer_t = typename remove_member_pointer<value_type>::type;

	template<typename member_type> struct remove_class_pointer {
		using type = member_type;
	};

	template<typename class_type, typename member_type> struct remove_class_pointer<member_type class_type::*> {
		using type = member_type;
	};

	template<typename value_type> using remove_class_pointer_t = typename remove_class_pointer<value_type>::type;

#if defined(JSONIFIER_CLANG)
	constexpr auto pretty_function_tail = "]";
#elif defined(JSONIFIER_GNUCXX)
	constexpr auto pretty_function_tail = ";";
#endif

	/**
	 * @brief Get the name of a member pointer.
	 *
	 * Function to extract the name of a member pointer.
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
		using value_type		 = remove_member_pointer_t<decltype(p)>;
		constexpr auto pNew		 = p;
		constexpr auto newString = getNameImpl<value_type, &(external<value_type>.*pNew)>();
#else
		constexpr auto newString = getNameImpl<p>();
#endif
		return newString;
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
			static_assert(( jsonifier::concepts::vector_t<value_type> || jsonifier::concepts::raw_array_t<value_type> || jsonifier::concepts::tuple_t<value_type> ),
				"Sorry, but that is not a valid value of type 'array'.");
		} else if constexpr (type == jsonifier::json_type::string) {
			static_assert(( jsonifier::concepts::string_t<value_type> || jsonifier::concepts::string_view_t<value_type> ),
				"Sorry, but that is not a valid value of type 'string'.");
		} else if constexpr (type == jsonifier::json_type::number) {
			static_assert(( jsonifier::concepts::num_t<value_type> || jsonifier::concepts::enum_t<value_type> ), "Sorry, but that is not a valid value of type 'number'.");
		} else if constexpr (type == jsonifier::json_type::boolean) {
			static_assert(( jsonifier::concepts::bool_t<value_type> ), "Sorry, but that is not a valid value of type 'boolean'.");
		} else if constexpr (type == jsonifier::json_type::null) {
			static_assert(( jsonifier::concepts::always_null_t<value_type> ), "Sorry, but that is not a valid value of type 'null'.");
		} else if (type == jsonifier::json_type::accessor) {
			static_assert(( jsonifier::concepts::accessor_t<value_type> ), "Sorry, but that is not a valid value of type 'accessor'.");
		}
		return type;
	}

	template<string_literal nameNew, auto memberPtrNew> struct json_entity_temp {
		using member_type = remove_class_pointer_t<std::remove_cvref_t<decltype(memberPtrNew)>>;
		using class_type  = remove_member_pointer_t<decltype(memberPtrNew)>;
		static constexpr member_type class_type::*memberPtr{ memberPtrNew };
		static constexpr string_literal name{ nameNew };
		jsonifier::json_type type{ getJsonTypePre<member_type>() };
		bool isItLast{};
	};

	template<size_t indexNew, string_literal nameNew, auto memberPtrNew> struct json_entity {
		using member_type = remove_class_pointer_t<std::remove_cvref_t<decltype(memberPtrNew)>>;
		using class_type  = remove_member_pointer_t<decltype(memberPtrNew)>;
		static constexpr member_type class_type::*memberPtr{ memberPtrNew };
		static constexpr string_literal name{ nameNew };
		static constexpr size_t index{ indexNew };
		jsonifier::json_type type{ getJsonTypePre<member_type>() };
		bool isItLast{};

		constexpr json_entity() noexcept = default;

		static constexpr auto view() noexcept {
			return name.template view<jsonifier::string_view>();
		}

		template<typename class_type_new> JSONIFIER_FORCE_INLINE constexpr auto& accessor(class_type_new&& obj) const {
			return obj.*memberPtr;
		}
	};

	template<typename value_type>
	concept is_json_entity = requires {
		typename std::remove_cvref_t<value_type>::member_type;
		typename std::remove_cvref_t<value_type>::class_type;
		std::remove_cvref_t<value_type>::memberPtr;
		{ std::remove_cvref_t<value_type>::name };
		{ std::remove_cvref_t<value_type>::type };
	} && !std::is_member_pointer_v<std::remove_cvref_t<value_type>>;

	template<typename value_type>
	concept is_json_entity_temp = requires {
		typename std::remove_cvref_t<value_type>::member_type;
		typename std::remove_cvref_t<value_type>::class_type;
		std::remove_cvref_t<value_type>::memberPtr;
	} && !std::is_member_pointer_v<std::remove_cvref_t<value_type>>;

	template<typename value_type>
	concept convertible_to_json_entity = is_json_entity<value_type> || is_json_entity_temp<value_type> || std::is_member_pointer_v<value_type>;

	template<size_t maxIndex, size_t index, auto value> constexpr auto makeJsonEntityAuto() noexcept {
		if constexpr (is_json_entity_temp<decltype(value)>) {
			if constexpr (index == maxIndex - 1) {
				json_entity<index, value.name, value.memberPtr> jsonEntity{};
				jsonEntity.isItLast = true;
				jsonEntity.type		= jsonifier_internal::setJsonType<value.type, typename std::remove_cvref_t<decltype(value)>::member_type>();
				return jsonEntity;
			} else {
				json_entity<index, value.name, value.memberPtr> jsonEntity{};
				jsonEntity.type = jsonifier_internal::setJsonType<value.type, typename std::remove_cvref_t<decltype(value)>::member_type>();
				return jsonEntity;
			}
		} else if constexpr (is_json_entity<decltype(value)>) {
			if constexpr (index == maxIndex - 1) {
				json_entity<index, value.name, value.memberPtr> jsonEntity{};
				jsonEntity.isItLast = true;
				jsonEntity.type		= jsonifier_internal::setJsonType<value.type, typename std::remove_cvref_t<decltype(value)>::member_type>();
				return jsonEntity;
			} else {
				return value;
			}
		} else {
			if constexpr (index == maxIndex - 1) {
				constexpr auto newName{ getName<value>() };
				json_entity<index, stringLiteralFromView<newName.size()>(newName), value> jsonEntity{};
				jsonEntity.isItLast = true;
				return jsonEntity;
			} else {
				constexpr auto newName{ getName<value>() };
				return json_entity<index, stringLiteralFromView<newName.size()>(newName), value>{};
			}
		}
	}

	template<auto... values, size_t... indices> constexpr auto createValueImpl(std::index_sequence<indices...>) {
		static_assert((convertible_to_json_entity<decltype(values)> && ...), "All arguments passed to createValue must be constructible to a json_entity.");
		return makeTuple(makeJsonEntityAuto<sizeof...(values), indices, values>()...);
	}

}

namespace jsonifier {

	template<auto testPtr, jsonifier_internal::string_literal nameNew> constexpr auto makeJsonEntity() {
		return jsonifier_internal::json_entity_temp<nameNew, testPtr>{};
	}

	template<auto testPtr, json_type type, jsonifier_internal::string_literal nameNew> constexpr auto makeJsonEntity() {
		using member_type = jsonifier_internal::remove_class_pointer_t<decltype(testPtr)>;
		jsonifier_internal::json_entity_temp<nameNew, testPtr> jsonEntity{};
		jsonEntity.type = jsonifier_internal::setJsonType<type, member_type>();
		return jsonEntity;
	}

	template<auto testPtr, json_type type> constexpr auto makeJsonEntity() {
		using member_type = jsonifier_internal::remove_class_pointer_t<decltype(testPtr)>;
		jsonifier_internal::json_entity_temp<jsonifier_internal::getName<testPtr>(), testPtr> jsonEntity{};
		jsonEntity.type = jsonifier_internal::setJsonType<type, member_type>();
		return jsonEntity;
	}

	template<auto... values> constexpr auto createValue() noexcept {
		return jsonifier_internal::createValueImpl<values...>(std::make_index_sequence<sizeof...(values)>{});
	}

}