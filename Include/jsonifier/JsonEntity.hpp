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

#include <jsonifier/Reflection.hpp>

namespace jsonifier_internal {

	template<typename value_type> static constexpr auto getJsonType() {
		if constexpr (jsonifier::concepts::jsonifier_object_t<value_type> || jsonifier::concepts::map_t<value_type>) {
			return jsonifier::json_type::object;
		} else if constexpr (jsonifier::concepts::raw_array_t<value_type> || jsonifier::concepts::tuple_t<value_type> || jsonifier::concepts::vector_t<value_type>) {
			return jsonifier::json_type::array;
		} else if constexpr (jsonifier::concepts::string_t<value_type> || jsonifier::concepts::string_view_t<value_type>) {
			return jsonifier::json_type::string;
		} else if constexpr (jsonifier::concepts::bool_t<value_type>) {
			return jsonifier::json_type::boolean;
		} else if constexpr (jsonifier::concepts::num_t<value_type> || jsonifier::concepts::enum_t<value_type>) {
			return jsonifier::json_type::number;
		} else if constexpr (jsonifier::concepts::always_null_t<value_type>) {
			return jsonifier::json_type::null;
		} else if constexpr (jsonifier::concepts::accessor_t<value_type>) {
			return jsonifier::json_type::accessor;
		} else {
			return jsonifier::json_type::custom;
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

	template<typename value_type>
	concept has_force_inline = requires() { std::remove_cvref_t<value_type>::forceInline; };

	template<typename value_type> constexpr bool getForceInlineAll() noexcept {
		if constexpr (has_force_inline<value_type>) {
			return value_type::forceInline;
		} else {
			return false;
		}
	}

	template<string_literal nameNew, auto memberPtrNew> struct json_entity_temp {
		using member_type = remove_class_pointer_t<std::remove_cvref_t<decltype(memberPtrNew)>>;
		using class_type  = remove_member_pointer_t<decltype(memberPtrNew)>;
		static constexpr jsonifier::json_type type{ getJsonType<member_type>() };
		static constexpr bool forceInlineAll{ getForceInlineAll<class_type>() };
		static constexpr member_type class_type::* memberPtr{ memberPtrNew };
		static constexpr string_literal name{ nameNew };
	};

	template<typename value_type, size_t currentDepth = 0, typename containing_type = void> static constexpr size_t maxFieldDepth() {
		using member_type = std::remove_cvref_t<value_type>;

		if constexpr (currentDepth < 32) {
			if constexpr (jsonifier::concepts::jsonifier_object_t<value_type>) {
				size_t maxDepth = 0;
				jsonifier_internal::forEach<std::tuple_size_v<std::remove_cvref_t<jsonifier_internal::core_tuple_type<value_type>>>>([&](const auto index, const auto maxIndex) {
					( void )maxIndex;
					using field_type = typename std::remove_cvref_t<decltype(get<index>(jsonifier::core<member_type>::parseValue))>::member_type;
					maxDepth		 = std::max(maxDepth, maxFieldDepth<field_type, currentDepth + 1, member_type>());
				});
				return 1 + maxDepth;
			} else if constexpr (jsonifier::concepts::raw_array_t<value_type> || jsonifier::concepts::vector_t<value_type>) {
				return 1 + maxFieldDepth<typename member_type::value_type, currentDepth + 1, containing_type>();
			} else if constexpr (jsonifier::concepts::map_t<value_type>) {
				return 1 + maxFieldDepth<typename member_type::mapped_type, currentDepth + 1, containing_type>();
			} else if constexpr (jsonifier::concepts::shared_ptr_t<value_type> || jsonifier::concepts::unique_ptr_t<value_type> || jsonifier::concepts::pointer_t<value_type>) {
				using pointee_type = std::remove_cvref_t<decltype(*member_type{})>;
				if constexpr (std::is_same_v<pointee_type, containing_type>) {
					return 1;
				} else {
					return 1 + maxFieldDepth<pointee_type, currentDepth + 1, containing_type>();
				}
			} else {
				return 1;
			}
		} else {
			return currentDepth;
		}
	}

	template<typename value_type>
	concept is_raw_core_type =
		requires() { jsonifier::core<std::remove_cvref_t<value_type>>::parseValue; } && !jsonifier::concepts::has_json_type<jsonifier::core<std::remove_cvref_t<value_type>>>;

	template<typename value_type>
	concept is_json_entity = requires {
		typename std::remove_cvref_t<value_type>::member_type;
		typename std::remove_cvref_t<value_type>::class_type;
		std::remove_cvref_t<value_type>::memberPtr;
		std::remove_cvref_t<value_type>::name;
		std::remove_cvref_t<value_type>::type;
	} && !std::is_member_pointer_v<std::remove_cvref_t<value_type>>;

	template<typename value_type>
	concept is_json_entity_temp = requires {
		typename std::remove_cvref_t<value_type>::member_type;
		typename std::remove_cvref_t<value_type>::class_type;
		std::remove_cvref_t<value_type>::memberPtr;
	} && !std::is_member_pointer_v<std::remove_cvref_t<value_type>>;

	template<typename value_type>
	concept convertible_to_json_entity = is_json_entity<value_type> || is_json_entity_temp<value_type> || std::is_member_pointer_v<value_type>;

	template<typename value_type, typename value_type02> constexpr jsonifier::json_type getJsonTypeFromEntity() {
		return getJsonType<std::remove_cvref_t<value_type02>>();
	}

	template<typename value_type> constexpr jsonifier::json_type getJsonTypeFromEntity() {
		return getJsonType<value_type>();
	}

	template<jsonifier::concepts::has_json_type value_type> constexpr jsonifier::json_type getJsonTypeFromEntity() {
		return jsonifier::core<std::remove_cvref_t<value_type>>::type;
	}

	template<typename value_type> using raw_core_type = std::remove_cvref_t<decltype(jsonifier::core<value_type>::parseValue)>;

	template<typename value_type>
	concept is_core_type = requires() { jsonifier::core<std::remove_cvref_t<value_type>>::parseValue; };

	template<is_core_type value_type> constexpr jsonifier::json_type getJsonTypeFromEntity() {
		return raw_core_type<std::remove_cvref_t<value_type>>::type;
	}

	template<auto... values> struct json_entity_pre;

	template<auto memberPtrNew> struct json_entity_pre<memberPtrNew> {
		using member_type = jsonifier_internal::remove_class_pointer_t<std::remove_cvref_t<decltype(memberPtrNew)>>;
		using class_type  = jsonifier_internal::remove_member_pointer_t<std::remove_cvref_t<decltype(memberPtrNew)>>;
		static constexpr auto nameTemp{ jsonifier_internal::getName<memberPtrNew>() };
		static constexpr jsonifier_internal::string_literal name{ jsonifier_internal::stringLiteralFromView<nameTemp.size()>(nameTemp) };
		static constexpr jsonifier::json_type type{ getJsonType<member_type>() };
		static constexpr auto memberPtr{ memberPtrNew };
	};

	template<auto memberPtrNew, jsonifier::json_type typeNew> struct json_entity_pre<memberPtrNew, typeNew> {
		using member_type = jsonifier_internal::remove_class_pointer_t<std::remove_cvref_t<decltype(memberPtrNew)>>;
		using class_type  = jsonifier_internal::remove_member_pointer_t<std::remove_cvref_t<decltype(memberPtrNew)>>;
		static constexpr auto nameTemp{ jsonifier_internal::getName<memberPtrNew>() };
		static constexpr jsonifier_internal::string_literal name{ jsonifier_internal::stringLiteralFromView<nameTemp.size()>(nameTemp) };
		static constexpr jsonifier::json_type type{ setJsonType<typeNew, member_type>() };
		static constexpr auto memberPtr{ memberPtrNew };
	};

	template<auto memberPtrNew, jsonifier_internal::string_literal nameNew> struct json_entity_pre<memberPtrNew, nameNew> {
		using member_type = jsonifier_internal::remove_class_pointer_t<std::remove_cvref_t<decltype(memberPtrNew)>>;
		using class_type  = jsonifier_internal::remove_member_pointer_t<std::remove_cvref_t<decltype(memberPtrNew)>>;
		static constexpr jsonifier::json_type type{ getJsonType<member_type>() };
		static constexpr jsonifier_internal::string_literal name{ nameNew };
		static constexpr auto memberPtr{ memberPtrNew };
	};

	template<auto memberPtrNew, jsonifier::json_type typeNew, jsonifier_internal::string_literal nameNew> struct json_entity_pre<memberPtrNew, typeNew, nameNew> {
		using member_type = jsonifier_internal::remove_class_pointer_t<std::remove_cvref_t<decltype(memberPtrNew)>>;
		using class_type  = jsonifier_internal::remove_member_pointer_t<std::remove_cvref_t<decltype(memberPtrNew)>>;
		static constexpr jsonifier::json_type type{ setJsonType<typeNew, member_type>() };
		static constexpr jsonifier_internal::string_literal name{ nameNew };
		static constexpr auto memberPtr{ memberPtrNew };
	};

	template<typename value_type>
	concept is_json_entity_pre = requires {
		typename std::remove_cvref_t<value_type>::member_type;
		typename std::remove_cvref_t<value_type>::class_type;
		std::remove_cvref_t<value_type>::memberPtr;
	} && !std::is_member_pointer_v<std::remove_cvref_t<value_type>>;

	template<typename value_type> constexpr bool getForceInline() noexcept {
		if constexpr (has_force_inline<value_type>) {
			return value_type::forceInline;
		} else {
			return false;
		}
	}

	template<size_t maxIndex, size_t index, auto value> constexpr auto createJsonEntityNewAuto() noexcept {
		if constexpr (is_json_entity_pre<decltype(value)>) {
			return value;
		} else {
			return json_entity_pre<value>{};
		}
	}

	template<typename current_type, typename containing_type> static constexpr bool isRecursive() {
		if constexpr (std::is_same_v<current_type, containing_type>) {
			return true;
		} else if constexpr (jsonifier::concepts::jsonifier_object_t<current_type>) {
			constexpr auto tuple = jsonifier::core<current_type>::parseValue;
			return []<size_t... Indices>(std::index_sequence<Indices...>) {
				return (isRecursive<typename std::remove_cvref_t<decltype(get<Indices>(tuple))>::member_type, containing_type>() || ...);
			}(std::make_index_sequence<std::tuple_size_v<std::remove_cvref_t<decltype(tuple)>>>{});
		} else if constexpr (jsonifier::concepts::shared_ptr_t<current_type> || jsonifier::concepts::unique_ptr_t<current_type> || jsonifier::concepts::pointer_t<current_type>) {
			using pointee_type = std::remove_cvref_t<decltype(*std::declval<current_type>())>;
			return isRecursive<pointee_type, containing_type>();
		} else if constexpr (jsonifier::concepts::map_t<current_type>) {
			using pointee_type = typename std::remove_cvref_t<current_type>::mapped_type;
			return isRecursive<pointee_type, containing_type>();
		} else {
			return false;
		}
	}

	template<typename value_type> struct base_json_entity {
		using member_type = std::remove_cvref_t<value_type>;
		static constexpr jsonifier::json_type type{ getJsonType<value_type>() };
		static constexpr bool forceInline{ true };
		static constexpr size_t index{ 0 };
	};

	template<typename value_type> constexpr auto getParseValue() {
		if constexpr (is_core_type<value_type>) {
			return jsonifier::core<value_type>::parseValue;
		} else {
			return jsonifier_internal::base_json_entity<value_type>{};
		}
	}

	template<auto memberPtrNew, jsonifier::json_type typeNew, jsonifier_internal::string_literal nameNew, size_t indexNew, bool forceInlineNew> struct json_entity {
		using member_type = jsonifier_internal::remove_class_pointer_t<std::remove_cvref_t<decltype(memberPtrNew)>>;
		using class_type  = jsonifier_internal::remove_member_pointer_t<std::remove_cvref_t<decltype(memberPtrNew)>>;
		static constexpr jsonifier::json_type type{ setJsonType<typeNew, member_type>() };
		static constexpr jsonifier_internal::string_literal name{ nameNew };
		static constexpr bool isRecursive{ jsonifier_internal::template isRecursive<member_type, class_type>() };
		static constexpr auto memberPtr{ memberPtrNew };
		static constexpr size_t index{ indexNew };
		static constexpr bool forceInline{ forceInlineNew ? forceInlineNew : ((index) < forceInlineLimitWidth) };
		static constexpr bool isItLast{ indexNew == jsonifier_internal::tuple_size_v<raw_core_type<class_type>> - 1 };

		JSONIFIER_FORCE_INLINE decltype(auto) operator[](tag<index>) const {
			return *this;
		}
	};

	template<auto... values, size_t... indices> constexpr auto createValueImpl(std::index_sequence<indices...>) {
		static_assert((convertible_to_json_entity<decltype(values)> && ...), "All arguments passed to createValue must be constructible to a json_entity.");
		return jsonifier_internal::makeTuple(createJsonEntityNewAuto<sizeof...(values), indices, values>()...);
	}
}

namespace jsonifier {

	template<auto testPtr, jsonifier::json_type type, jsonifier_internal::string_literal nameNew> constexpr auto createJsonEntity() {
		using member_type = jsonifier_internal::remove_class_pointer_t<decltype(testPtr)>;
		return jsonifier_internal::json_entity_pre<testPtr, jsonifier_internal::setJsonType<type, member_type>(), nameNew>{};
	}

	template<auto testPtr, jsonifier::json_type type> constexpr auto createJsonEntity() {
		using member_type = jsonifier_internal::remove_class_pointer_t<decltype(testPtr)>;
		return jsonifier_internal::json_entity_pre<testPtr, jsonifier_internal::setJsonType<type, member_type>()>{};
	}

	template<auto testPtr, jsonifier_internal::string_literal nameNew> constexpr auto createJsonEntity() {
		jsonifier_internal::json_entity_pre<testPtr, nameNew> jsonEntity{};
		return jsonEntity;
	}

	template<auto... values> constexpr auto createValue() noexcept {
		static_assert((jsonifier_internal::convertible_to_json_entity<decltype(values)> && ...), "All arguments passed to createValue must be constructible to a json_entity.");
		return jsonifier_internal::createValueImpl<values...>(std::make_index_sequence<sizeof...(values)>{});
	}

}
