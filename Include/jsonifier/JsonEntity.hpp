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

namespace jsonifier_internal {

	template<typename class_type_new, typename context_type> struct json_entity_base {
		JSONIFIER_ALWAYS_INLINE virtual bool processIndex(class_type_new& value, context_type& context) = 0;
		JSONIFIER_ALWAYS_INLINE virtual ~json_entity_base() noexcept									= default;
	};

	template<typename class_type_new, typename context_type, typename... bases> struct json_map : public bases... {
		template<typename current_type, typename... rest_types> constexpr auto generateBasePtrs(size_t currentIndex,
			std::array<jsonifier_internal::json_entity_base<class_type_new, context_type>*, sizeof...(bases)> arrayOfPtrsNew = {}) {
			if constexpr (sizeof...(rest_types) >= 1) {
				arrayOfPtrsNew[currentIndex] = static_cast<current_type*>(this);
				++currentIndex;
				return generateBasePtrs<rest_types...>(currentIndex, arrayOfPtrsNew);
			} else {
				arrayOfPtrsNew[currentIndex] = static_cast<current_type*>(this);
				++currentIndex;
				return arrayOfPtrsNew;
			}
		};
		std::array<jsonifier_internal::json_entity_base<class_type_new, context_type>*, sizeof...(bases)> arrayOfPtrs{ generateBasePtrs<bases...>(0) };
	};

	template<auto... valuesNew> struct value_holder {
		static constexpr jsonifier_internal::tuple values{ valuesNew };
	};

	template<template<jsonifier::concepts::value_holder_t, typename...> typename json_entity_type, auto index, auto tuple, jsonifier::concepts::value_holder_t values,
		typename... types>
	struct construct_json_entity {
		static constexpr auto originalTupleValue{ jsonifier_internal::get<index>(tuple) };
		using type = json_entity_type<values, types...>;
	};

	template<template<jsonifier::concepts::value_holder_t, typename...> typename json_entity_type, auto index, auto tuple, jsonifier::concepts::value_holder_t values,
		typename... types>
	using construct_json_entity_final_t = construct_json_entity<json_entity_type, index, tuple, values, types...>::type;

	template<template<jsonifier::concepts::value_holder_t, typename...> typename json_entity_type, jsonifier::concepts::value_holder_t values, typename index_sequence,
		typename... types>
	struct get_json_map;

	template<typename class_type, typename context_type, template<jsonifier::concepts::value_holder_t, typename...> typename json_entity_type, typename... types,
		jsonifier::concepts::value_holder_t values, auto tuple, size_t... I>
	struct get_json_map<class_type, context_type, json_entity_type, values, std::index_sequence<I...>, types...> {
		using type = json_map<class_type, context_type, construct_json_entity_final_t<json_entity_type, I, tuple, values, types...>...>;
	};

	template<template<jsonifier::concepts::value_holder_t, typename...> typename json_entity_type, jsonifier::parse_options options, typename class_type, typename context_type,
		typename buffer_type, auto tuple, bool minifiedOrInsideRepeated>
	using json_map_t = typename get_json_map<json_entity_type, options, class_type, context_type, buffer_type, tuple, minifiedOrInsideRepeated,
		jsonifier_internal::tag_range<jsonifier_internal::tuple_size_v<typename jsonifier_internal::core_tuple_type<class_type>::core_type>>>::type;

}// namespace jsonifier_internal