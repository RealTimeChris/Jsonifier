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
#include <jsonifier/Core.hpp>
#include <source_location>

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
}

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

	template<typename member_type> struct remove_class_pointer {
		using type = member_type;
	};

	template<typename value_type, typename member_type> struct remove_class_pointer<member_type value_type::*> {
		using type = member_type;
	};

	template<typename value_type, typename member_type, typename... arg_types> struct remove_class_pointer<member_type (value_type::*)(arg_types...)> {
		using type = member_type;
	};

	template<typename value_type> using remove_class_pointer_t = typename remove_class_pointer<value_type>::type;

	template<typename class_type_new, typename member_type_new> struct raw_json_entity {
		using member_type = std::remove_cvref_t<member_type_new>;
		using class_type  = std::remove_cvref_t<class_type_new>;
		jsonifier::string_view name{};
		jsonifier::json_type type{};
		member_type class_type::*memberPtr{};

		constexpr raw_json_entity() noexcept = default;

		template<auto ptrNew> constexpr raw_json_entity() : memberPtr{ ptrNew }, name{ getName<ptrNew>() }, type{} {};

		constexpr raw_json_entity(member_type_new class_type_new::*ptr) noexcept : memberPtr{ ptr }, name{}, type{ getJsonType<member_type_new>() } {};

		constexpr raw_json_entity(member_type class_type::*ptr, jsonifier::string_view str, jsonifier::json_type typeNew) noexcept
			: memberPtr{ ptr }, name{ str }, type{ typeNew } {};

		constexpr auto& view() const noexcept {
			return name;
		}

		constexpr auto& ptr() const noexcept {
			return memberPtr;
		}
	};

	template<size_t index, typename class_type_new, typename member_type_new> struct json_entity {
		using member_type = member_type_new;
		using class_type  = class_type_new;
		jsonifier::string_view name{};
		jsonifier::json_type type{};
		mutable member_type class_type::*memberPtr{};

		constexpr json_entity() noexcept = default;

		template<auto ptrNew> constexpr json_entity() : memberPtr{ ptrNew }, name{ getName<ptrNew>() }, type{} {
		}

		constexpr json_entity(member_type_new class_type_new::*ptr) noexcept : memberPtr{ ptr }, name{}, type{ getJsonTypePre<member_type_new>() } {};

		constexpr json_entity(member_type class_type::*ptr, jsonifier::string_view str, jsonifier::json_type typeNew) noexcept : memberPtr{ ptr }, name{ str }, type{ typeNew } {};

		template<typename parse_context_type> constexpr bool processIndex(class_type& valueNew, parse_context_type& context) const {
			//static_assert(std::is_same_v<member_type, const std::string>, "member_type is not std::string");

			std::cout << "CURRENT TYPE: " << typeid(member_type).name() << std::endl;
			std::cout << "CURRENT TYPE: " << typeid(class_type).name() << std::endl;
			std::cout << "Type of ptr() member: " << typeid(ptr()).name() << std::endl;

			if constexpr (jsonifier::concepts::string_t<remove_class_pointer_t<member_type>>) {
				std::cout << "Processing index: (REAL) " << index << '\n';
				// Assigning to the actual member
				valueNew.*ptr() = "test";
			}
			return true;
		}

		constexpr decltype(auto) operator[](tag<index>) const& {
			std::cout << "Accessing index (REAL): " << index << '\n';
			return *this;
		}

		constexpr auto& view() const noexcept {
			return name;
		}

		constexpr auto& ptr() const noexcept {
			return memberPtr;
		}
	};

	template<typename member_type, typename class_type> constexpr auto makeRawJsonEntityAuto(jsonifier::string_view str, member_type class_type::*ptr) noexcept {
		return raw_json_entity<class_type, member_type>{ ptr, str, getJsonTypePre<member_type>() };
	}

	template<auto value> constexpr auto makeRawJsonEntityAuto() noexcept {
		if constexpr (jsonifier::concepts::is_json_entity<decltype(value)>) {
			return raw_json_entity{ value };
		} else {
			return makeRawJsonEntityAuto(getName<value>(), value);
		}
	}

	template<size_t index, typename class_type, typename member_type> constexpr auto makeJsonEntityAuto(const raw_json_entity<class_type, member_type>& rawJsonEntity) noexcept {
		json_entity<index, class_type, member_type> newEntity{};
		newEntity.memberPtr = rawJsonEntity.memberPtr;
		newEntity.name		= rawJsonEntity.name;
		newEntity.type		= rawJsonEntity.type;
		return newEntity;
	}

	template<typename value_type, typename... bases> struct json_map : bases... {
		using base_list = jsonifier_internal::tuple<bases...>;

		template<typename... bases_new> constexpr json_map(bases_new&&... args) : bases{ std::forward<bases_new>(args) }... {};

		using bases::operator[]...;

		template<size_t index, typename parse_context_type> constexpr void processIndex(value_type& value, parse_context_type& context) const {
			operator[](jsonifier_internal::tag<index>{}).processIndex(value, context);
		}

		template<typename parse_context_type> constexpr void processIndex(value_type& value, parse_context_type& context, size_t index) const {
			(*this.*function_pointers<parse_context_type>[index])(value, context);
		}


		template<typename parse_context_type, size_t... indices> static constexpr auto collectFunctionPtrs(std::index_sequence<indices...>) {
			using function_type = void (json_map::*)(value_type&, parse_context_type&) const;
			return std::array<function_type, sizeof...(indices)>{ &json_map::template processIndex<indices>... };
		}

		template<typename parse_context_type> static constexpr auto function_pointers = collectFunctionPtrs<parse_context_type>(std::make_index_sequence<sizeof...(bases)>{});
	};

	template<typename index_sequence, typename class_type, typename... value_type> struct get_json_map;

	template<size_t... I, typename class_type, typename... value_type> struct get_json_map<std::index_sequence<I...>, class_type, value_type...> {
		using type = json_map<class_type, json_entity<I, class_type, value_type>...>;
	};

	template<typename class_type, typename... value_type> using json_map_t = typename get_json_map<tag_range<sizeof...(value_type)>, class_type, value_type...>::type;

	template<typename value_type, size_t... indices> constexpr auto createJsonMapImpl(std::index_sequence<indices...>) {
		return json_map_t<value_type, typename std::remove_cvref_t<decltype(get<indices>(jsonifier::concepts::coreV<value_type>))>::member_type...>{ get<indices>(
			jsonifier::concepts::coreV<value_type>)... };
	}

	template<typename value_type> constexpr auto createJsonMap() {
		return createJsonMapImpl<value_type>(std::make_index_sequence<tuple_size_v<typename core_tuple_type<value_type>::core_type>>{});
	}

	template<auto... values, size_t... indices> constexpr auto createValuesImpl(std::index_sequence<indices...>) {
		return jsonifier::value{ makeTuple(makeJsonEntityAuto<indices>(makeRawJsonEntityAuto<values>())...) };
	}
}

namespace jsonifier {

	template<auto testPtr, jsonifier_internal::string_literal nameNew> constexpr auto make_json_entity() {
		constexpr jsonifier_internal::string_literal name{ nameNew };
		return jsonifier_internal::makeRawJsonEntityAuto(jsonifier_internal::make_static<name>::value.view(), testPtr);
	}

	template<auto testPtr> constexpr auto make_json_entity() {
		return jsonifier_internal::makeRawJsonEntityAuto(jsonifier_internal::getName<testPtr>(), testPtr);
	}

	template<auto testPtr, json_type type, jsonifier_internal::string_literal nameNew> constexpr auto make_json_entity() {
		constexpr jsonifier_internal::string_literal name{ nameNew };
		auto jsonEntity = jsonifier_internal::makeRawJsonEntityAuto(jsonifier_internal::make_static<name>::value.view(), testPtr);
		jsonEntity.type = jsonifier_internal::setJsonType<type, typename decltype(jsonEntity)::member_type>();
		return jsonEntity;
	}

	template<auto testPtr, json_type type> constexpr auto make_json_entity() {
		auto jsonEntity = jsonifier_internal::makeRawJsonEntityAuto(jsonifier_internal::getName<testPtr>(), testPtr);
		jsonEntity.type = jsonifier_internal::setJsonType<type, typename decltype(jsonEntity)::member_type>();
		return jsonEntity;
	}

	template<auto... values> constexpr auto createValue() noexcept {
		static_assert((jsonifier::concepts::convertible_to_raw_json_entity<decltype(values)> && ...),
			"All arguments passed to createValue should be convertible to or a json_entity.");
		return jsonifier_internal::createValuesImpl<values...>(std::make_index_sequence<sizeof...(values)>{});
	}

}