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
			static_assert(jsonifier::concepts::bool_t<value_type>, "Sorry, but that is not a valid value of type 'boolean'.");
		} else if constexpr (type == jsonifier::json_type::null) {
			static_assert(jsonifier::concepts::always_null_t<value_type>, "Sorry, but that is not a valid value of type 'null'.");
		} else if (type == jsonifier::json_type::accessor) {
			static_assert(jsonifier::concepts::accessor_t<value_type>, "Sorry, but that is not a valid value of type 'accessor'.");
		}
		return type;
	}

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

	template<typename value_type> struct base_json_entity {
		using member_type = std::remove_cvref_t<value_type>;
		static constexpr jsonifier::json_type type{ getJsonType<member_type>() };
		static constexpr size_t index{ 0 };
	};

	template<auto memberPtrNew, string_literal nameNew, jsonifier::json_type typeNew> struct json_entity_temp {
		using member_type = remove_class_pointer_t<std::remove_cvref_t<decltype(memberPtrNew)>>;
		using class_type  = remove_member_pointer_t<std::remove_cvref_t<decltype(memberPtrNew)>>;
		static constexpr jsonifier::json_type type{ setJsonType<typeNew, member_type>() };
		static constexpr member_type class_type::* memberPtr{ memberPtrNew };
		static constexpr string_literal name{ nameNew };
	};

	template<auto memberPtrNew, string_literal nameNew, jsonifier::json_type typeNew, size_t indexNew, size_t maxIndex> struct json_entity {
		using member_type = remove_class_pointer_t<std::remove_cvref_t<decltype(memberPtrNew)>>;
		using class_type  = remove_member_pointer_t<std::remove_cvref_t<decltype(memberPtrNew)>>;
		static constexpr member_type class_type::* memberPtr{ memberPtrNew };
		static constexpr bool isItLast{ indexNew == maxIndex - 1 };
		static constexpr jsonifier::json_type type{ typeNew };
		static constexpr string_literal name{ nameNew };
		static constexpr size_t index{ indexNew };
	};

	template<typename value_type>
	concept is_base_json_entity = requires {
		typename std::remove_cvref_t<value_type>::member_type;
		std::remove_cvref_t<value_type>::type;
	} && !std::is_member_pointer_v<std::remove_cvref_t<value_type>>;

	template<typename value_type>
	concept is_json_entity_temp = requires {
		typename std::remove_cvref_t<value_type>::class_type;
		std::remove_cvref_t<value_type>::memberPtr;
	} && is_base_json_entity<value_type>;

	template<is_base_json_entity value_type01, typename value_type02> constexpr jsonifier::json_type getJsonTypeFromEntity() {
		if constexpr (std::is_same_v<typename std::remove_cvref_t<value_type01>::member_type, value_type02>) {
			return value_type01::type;
		} else {
			return getJsonType<value_type02>();
		}
	}

	template<typename value_type>
	concept convertible_to_json_entity = is_json_entity_temp<value_type> || std::is_member_pointer_v<value_type>;

	template<size_t maxIndex, size_t index, auto value> constexpr auto makeJsonEntityAuto() noexcept {
		if constexpr (is_json_entity_temp<decltype(value)>) {
			json_entity<value.memberPtr, value.name, value.type, index, maxIndex> jsonEntity{};
			return jsonEntity;
		} else {
			using member_type	   = remove_class_pointer_t<std::remove_cvref_t<decltype(value)>>;
			constexpr auto nameNew = getName<value>();
			json_entity<value, stringLiteralFromView<nameNew.size()>(nameNew), jsonifier_internal::getJsonType<member_type>(), index, maxIndex> jsonEntity{};
			return jsonEntity;
		}
	}

	template<auto... values, size_t... indices> constexpr auto createValueImpl(std::index_sequence<indices...>) {
		static_assert((convertible_to_json_entity<decltype(values)> && ...), "All arguments passed to createValue must be convertible to a json_entity.");
		return makeTuple(makeJsonEntityAuto<sizeof...(values), indices, values>()...);
	}

}

namespace jsonifier {

	template<auto memberPtr, jsonifier_internal::string_literal nameNew, json_type typeNew> constexpr auto makeJsonEntity() {
		using member_type = jsonifier_internal::remove_class_pointer_t<decltype(memberPtr)>;
		return jsonifier_internal::json_entity_temp<memberPtr, nameNew, typeNew>{};
	}

	template<auto memberPtr, jsonifier_internal::string_literal nameNew> constexpr auto makeJsonEntity() {
		using member_type = jsonifier_internal::remove_class_pointer_t<decltype(memberPtr)>;
		return jsonifier_internal::json_entity_temp<memberPtr, nameNew, jsonifier_internal::getJsonType<member_type>()>{};
	}

	template<auto memberPtr, json_type typeNew> constexpr auto makeJsonEntity() {
		using member_type = jsonifier_internal::remove_class_pointer_t<decltype(memberPtr)>;
		return jsonifier_internal::json_entity_temp<memberPtr, jsonifier_internal::getName<memberPtr>(), typeNew>{};
	}

	template<auto memberPtr> constexpr auto makeJsonEntity() {
		using member_type = jsonifier_internal::remove_class_pointer_t<decltype(memberPtr)>;
		return jsonifier_internal::json_entity_temp<memberPtr, jsonifier_internal::getName<memberPtr>(), jsonifier_internal::getJsonType<member_type>()>{};
	}

	template<auto... values> constexpr auto createValue() noexcept {
		return jsonifier_internal::createValueImpl<values...>(std::make_index_sequence<sizeof...(values)>{});
	}

}