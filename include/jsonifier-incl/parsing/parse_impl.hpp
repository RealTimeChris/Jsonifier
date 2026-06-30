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
/// Feb 20, 2023
#pragma once

#include <jsonifier-incl/utilities/number_utils.hpp>
#include <jsonifier-incl/utilities/string_utils.hpp>
#include <jsonifier-incl/parsing/parser.hpp>

namespace jsonifier::internal {

	enum class parse_result {
		inactive_member,
		active_member,
	};

	template<typename value_type, typename context_type, parse_options options> struct parse_types_impl {
		static constexpr auto membercount = core_tuple_size<value_type>;

		template<uint64_t index> inline static parse_result processIndex(value_type& value, context_type& context) {
			const auto keyStart = context.iter;
			if JSONIFIER_UNLIKELY (context.template advanceMaybe<json_structural_characters::key_start>()) {
				static constexpr auto tupleElem		= get_because_other_lib_authors_resolve<index>(core<value_type>::parseValue);
				static constexpr auto stringLiteral = escapedKeyLiteral<tupleElem.name>;
				static constexpr auto ptrNew		= tupleElem.memberPtr;
				static constexpr auto keySize		= stringLiteral.size();
				static constexpr auto keySizeNew	= keySize + 1;
				if JSONIFIER_LIKELY (((context.iter + keySize) < context.endIter) && (*(context.iter + keySize)) == quote &&
					string_literal_comparitor<decltype(stringLiteral), stringLiteral>::impl(context.iter)) {
					context.iter += keySizeNew;
					if constexpr (!options.minified) {
						context.skipWs();
					}
					if JSONIFIER_UNLIKELY (!context.template advanceTo<json_structural_characters::colon>()) {
						return parse_result::inactive_member;
					}
					if constexpr (concepts::has_excluded_keys<value_type>) {
						static constexpr auto key = stringLiteral.operator jsonifier::string_view();
						auto& keys				  = value.jsonifierExcludedKeys;
						if JSONIFIER_LIKELY (keys.find(static_cast<typename remove_cvref_t<decltype(keys)>::key_type>(key)) != keys.end()) {
							return parse_result::active_member;
						}
					}
					parse<options>::impl(value.*ptrNew, context);
					return parse_result::active_member;
				}
			}
			context.iter = keyStart;
			return parse_result::inactive_member;
		}
	};

	template<template<typename, typename, parse_options> typename parsing_type, typename value_type, typename context_type, parse_options options, uint64_t... indices>
	static constexpr auto generateFunctionPtrs(index_sequence<indices...>) noexcept {
		using function_type = decltype(&parsing_type<value_type, context_type, options>::template processIndex<0>);
		return array<function_type, sizeof...(indices)>{ { &parsing_type<value_type, context_type, options>::template processIndex<indices>... } };
	}

	template<template<typename, typename, parse_options> typename parsing_type, typename value_type, typename context_type, parse_options options, typename integer_sequence>
	struct generateFoldStatement;

	template<template<typename, typename, parse_options> typename parsing_type, typename value_type, typename context_type, parse_options options, uint64_t... indices>
	struct generateFoldStatement<parsing_type, value_type, context_type, options, integer_sequence<indices...>> {
		template<uint64_t index> JSONIFIER_INLINE static parse_result processIndex(value_type& value, context_type& context, uint64_t current_index) {
			if (index == current_index) {
				return parsing_type<value_type, context_type, options>::template processIndex<index>(value, context);
			} else {
				return parse_result::inactive_member;
			}
		}

		JSONIFIER_INLINE static parse_result impl(value_type& value, context_type& context, uint64_t current_index) {
			parse_result result{};
			((result = processIndex<indices>(value, context, current_index), result == parse_result::inactive_member) && ...);
			return result;
		}
	};

	template<template<typename, typename, parse_options> typename parsing_type, typename value_type, typename context_type, parse_options options, typename integer_sequence>
	struct generateDispatchTable;

	template<template<typename, typename, parse_options> typename parsing_type, typename value_type, typename context_type, parse_options options, uint64_t... indices>
	struct generateDispatchTable<parsing_type, value_type, context_type, options, integer_sequence<indices...>> {
		using fn_type = parse_result (*)(value_type&, context_type&);

		static constexpr array<fn_type, sizeof...(indices)> table{ { &parsing_type<value_type, context_type, options>::template processIndex<indices>... } };

		JSONIFIER_INLINE static parse_result impl(value_type& value, context_type& context, uint64_t current_index) {
			if constexpr (sizeof...(indices) <= JSONIFIER_DISPATCH_TABLE_COUNT) {
				parse_result result{ parse_result::inactive_member };
				static_cast<void>(
					((current_index == indices ? (result = parsing_type<value_type, context_type, options>::template processIndex<indices>(value, context), true) : false) || ...));
				return result;
			} else {
				return table[current_index](value, context);
			}
		}
	};

	template<uint64_t membercount> constexpr array<uint64_t, (membercount > 0 ? membercount : 1)> generateAntiHashStatesTable() {
		array<uint64_t, (membercount > 0 ? membercount : 1)> returnValues{};
		for (uint64_t x = 0; x < membercount; ++x) {
			returnValues[x] = x;
		}
		return returnValues;
	}

	template<uint64_t membercount, typename value_type>
	thread_local constinit static array<uint64_t, (membercount > 0 ? membercount : 1)> antiHashStates{ generateAntiHashStatesTable<membercount>() };

	template<template<typename, typename, parse_options> typename parsing_type, typename value_type, typename context_type, parse_options options>
	static constexpr auto functionPtrs{ generateFunctionPtrs<parsing_type, value_type, context_type, options>(make_index_sequence<core_tuple_size<value_type>>{}) };

	template<parse_options options, typename json_entity_type> struct json_entity_parse;

	template<parse_options options, typename json_entity_type>
		requires(!options.minified)
	struct json_entity_parse<options, json_entity_type> : public json_entity_type {
		static constexpr auto membercount{ core_tuple_size<typename json_entity_type::class_type> };

		template<typename value_type, typename context_type> inline static bool processIndex(value_type& value, context_type& context) {
			if JSONIFIER_UNLIKELY (context.template advanceMaybe<json_structural_characters::r_crl_bracket>()) {
				return false;
			}
			if constexpr (json_entity_type::index > 0) {
				if JSONIFIER_UNLIKELY (!context.template advanceMaybe<json_structural_characters::comma>()) {
					return true;
				}
			}
			if constexpr (options.knownOrder) {
				static constexpr auto stringLiteral = escapedKeyLiteral<json_entity_type::name>;
				static constexpr auto ptrNew		= json_entity_type::memberPtr;
				static constexpr auto keySize		= stringLiteral.size();
				static constexpr auto keySizeNew	= keySize + 1;
				if JSONIFIER_LIKELY (((context.iter + keySize) < context.endIter) && (*(context.iter + keySize)) == quote &&
					string_literal_comparitor<decltype(stringLiteral), stringLiteral>::impl(context.iter)) {
					context.iter += keySizeNew;
					if constexpr (!options.minified) {
						context.skipWs();
					}
					if JSONIFIER_UNLIKELY (!context.template advanceTo<json_structural_characters::colon>()) {
						return true;
					}
					if constexpr (concepts::has_excluded_keys<value_type>) {
						static constexpr auto key = stringLiteral.operator string_view();
						const auto& keys		  = value.jsonifierExcludedKeys;
						if JSONIFIER_LIKELY (keys.find(static_cast<typename remove_cvref_t<decltype(keys)>::key_type>(key)) != keys.end()) {
							return true;
						}
					}
					parse<options>::impl(value.*ptrNew, context);
					return true;
				}
			}
			if constexpr (membercount == 1) {
				if JSONIFIER_LIKELY (auto result = parse_types_impl<value_type, context_type, options>::template processIndex<0>(value, context);
					result == parse_result::active_member) {
					return true;
				}
			} else {
				if JSONIFIER_LIKELY (auto indexNew = antiHashStates<membercount, value_type>[json_entity_type::index]; indexNew < membercount) {
					if JSONIFIER_LIKELY (auto result = generateDispatchTable<parse_types_impl, value_type, context_type, options, make_integer_sequence<membercount>>::impl(value,
											 context, indexNew);
						result == parse_result::active_member) {
						return true;
					} else {
						if JSONIFIER_LIKELY (auto indexNew2 = hash_map<value_type, remove_cvref_t<decltype(context.iter)>>::findIndex(context.iter + 1, context.endIter);
							indexNew2 < membercount) {
							if JSONIFIER_LIKELY (auto result2 =
													 generateDispatchTable<parse_types_impl, value_type, context_type, options, make_integer_sequence<membercount>>::impl(value,
														 context, indexNew2);
								result2 == parse_result::active_member) {
								if constexpr (options.knownOrder) {
									antiHashStates<membercount, value_type>[json_entity_type::index] = indexNew2;
								}
								return true;
							}
						}
					}
				}
			}
			context.skipKey();
			if JSONIFIER_UNLIKELY (!context.template advanceTo<json_structural_characters::colon>()) {
				return true;
			}
			context.skipToNextValue();
			return true;
		}
	};

	template<uint64_t index, typename literal_type> JSONIFIER_INLINE static constexpr auto makeMemberLiteral(const literal_type& keyLiteral) noexcept {
		if constexpr (index > 0) {
			return string_literal{ "," } + string_literal{ "\"" } + keyLiteral + string_literal{ "\"" } + string_literal{ ":" };
		} else {
			return string_literal{ "\"" } + keyLiteral + string_literal{ "\"" } + string_literal{ ":" };
		}
	}

	template<parse_options options, typename json_entity_type>
		requires(options.minified)
	struct json_entity_parse<options, json_entity_type> : public json_entity_type {
		static constexpr auto membercount{ core_tuple_size<typename json_entity_type::class_type> };

		template<typename value_type, typename context_type> inline static bool processIndex(value_type& value, context_type& context) {
			if JSONIFIER_UNLIKELY (context.template advanceMaybe<json_structural_characters::r_crl_bracket>()) {
				return false;
			}
			if constexpr (options.knownOrder) {
				static constexpr auto stringLiteral		= escapedKeyLiteral<json_entity_type::name>;
				static constexpr auto ptrNew			= json_entity_type::memberPtr;
				static constexpr auto memberLiteral		= makeMemberLiteral<json_entity_type::index>(stringLiteral);
				static constexpr auto memberLiteralSize = memberLiteral.size();
				if JSONIFIER_LIKELY (((context.iter + memberLiteralSize) <= context.endIter) &&
					string_literal_comparitor<decltype(memberLiteral), memberLiteral>::impl(context.iter)) {
					context.iter += memberLiteralSize;
					if constexpr (concepts::has_excluded_keys<value_type>) {
						static constexpr auto key = stringLiteral.operator jsonifier::string_view();
						const auto& keys		  = value.jsonifierExcludedKeys;
						if JSONIFIER_LIKELY (keys.find(static_cast<typename remove_cvref_t<decltype(keys)>::key_type>(key)) != keys.end()) {
							return true;
						}
					}
					parse<options>::impl(value.*ptrNew, context);
					return true;
				}
			}
			if constexpr (json_entity_type::index > 0) {
				if JSONIFIER_UNLIKELY (!context.template advanceMaybe<json_structural_characters::comma>()) {
					return true;
				}
			}
			if JSONIFIER_LIKELY (auto indexNew = antiHashStates<membercount, value_type>[json_entity_type::index]; indexNew < membercount) {
				if JSONIFIER_LIKELY (auto result = generateDispatchTable<parse_types_impl, value_type, context_type, options, make_integer_sequence<membercount>>::impl(value,
										 context, indexNew);
					result == parse_result::active_member) {
					return true;
				} else {
					if JSONIFIER_LIKELY (auto indexNew2 = hash_map<value_type, remove_cvref_t<decltype(context.iter)>>::findIndex(context.iter + 1, context.endIter);
						indexNew2 < membercount) {
						if JSONIFIER_LIKELY (auto result2 = generateDispatchTable<parse_types_impl, value_type, context_type, options, make_integer_sequence<membercount>>::impl(
												 value, context, indexNew2);
							result2 == parse_result::active_member) {
							if constexpr (options.knownOrder) {
								antiHashStates<membercount, value_type>[json_entity_type::index] = indexNew2;
							}
							return true;
						}
					}
				}
			}
			context.skipKey();
			if JSONIFIER_UNLIKELY (!context.template advanceTo<json_structural_characters::colon>()) {
				return true;
			}
			context.skipToNextValue();
			return true;
		}
	};

	template<typename... bases> struct parse_map : public bases... {
		template<typename json_entity_type, typename... arg_types> JSONIFIER_INLINE static bool iterateValuesImpl(arg_types&&... args) {
			return json_entity_type::processIndex(internal::forward<arg_types>(args)...);
		}

		template<typename... arg_types> JSONIFIER_INLINE static constexpr bool iterateValues([[maybe_unused]] arg_types&&... args) {
			return ((iterateValuesImpl<bases>(internal::forward<arg_types>(args)...)) && ...);
		}
	};

	template<parse_options options, typename value_type, typename context_type, typename index_sequence, typename... value_types> struct get_parse_base;

	template<parse_options options, typename value_type, typename context_type, uint64_t... index>
	struct get_parse_base<options, value_type, context_type, index_sequence<index...>> {
		using type = parse_map<json_entity_parse<options, remove_cvref_t<decltype(get_because_other_lib_authors_resolve<index>(core<value_type>::parseValue))>>...>;
	};

	template<parse_options options, typename value_type, typename context_type> using parse_base_t =
		typename get_parse_base<options, value_type, context_type, make_index_sequence<core_tuple_size<value_type>>>::type;	

	template<concepts::jsonifier_object_t value_type, typename context_type, parse_options options> struct parse_impl<value_type, context_type, options> {
		static constexpr auto membercount = core_tuple_size<value_type>;

		template<typename value_type_new> JSONIFIER_INLINE static void skipRemainingMembers(context_type& context) noexcept {
			while (!context.template advanceMaybe<json_structural_characters::r_crl_bracket>()) {
				if JSONIFIER_UNLIKELY (context.atEnd()) {
					context.template reportError<parse_statuses::unexpected_string_end>();
					return;
				}
				context.template advanceMaybe<json_structural_characters::comma>();
				context.skipKey();
				if JSONIFIER_UNLIKELY (!context.template advanceTo<json_structural_characters::colon>()) {
					return;
				}
				context.skipToNextValue();
			}
		}

		JSONIFIER_INLINE static void impl(value_type& value, context_type& context) noexcept {
			if JSONIFIER_UNLIKELY (!context.template advanceTo<json_structural_characters::l_crl_bracket>()) {
				return;
			}
			if JSONIFIER_LIKELY (context.template advanceMaybe<json_structural_characters::r_crl_bracket>()) {
				return;
			}
			if (parse_base_t<options, value_type, context_type>::iterateValues(value, context)) {
				skipRemainingMembers<value_type>(context);
			}
		}
	};

#if JSONIFIER_COMPILER_CLANG
	#pragma clang diagnostic push
	#pragma clang diagnostic ignored "-Wexit-time-destructors"
#endif
	template<typename key_type> JSONIFIER_INLINE static key_type& getKey() {
		thread_local static key_type key{};
		return key;
	}
#if JSONIFIER_COMPILER_CLANG
	#pragma clang diagnostic pop
#endif

	template<concepts::map_t value_type, typename context_type, parse_options options> struct parse_impl<value_type, context_type, options> {
		JSONIFIER_INLINE static void impl(value_type& value, context_type& context) noexcept {
			if JSONIFIER_UNLIKELY (!context.template advanceTo<json_structural_characters::l_crl_bracket>()) {
				return;
			}
			if JSONIFIER_UNLIKELY (context.template advanceMaybe<json_structural_characters::r_crl_bracket>()) {
				return;
			}
			while (true) {
				if JSONIFIER_UNLIKELY (context.currentChar() != '"') {
					context.template reportError<parse_statuses::invalid_string_characters>();
					return;
				}
				parse<options>::impl(getKey<typename value_type::key_type>(), context);
				if JSONIFIER_UNLIKELY (!context.template advanceTo<json_structural_characters::colon>()) {
					return;
				}
				parse<options>::impl(value[getKey<typename value_type::key_type>()], context);
				if JSONIFIER_UNLIKELY (context.template advanceMaybe<json_structural_characters::r_crl_bracket>()) {
					return;
				}
				if JSONIFIER_UNLIKELY (!context.template advanceTo<json_structural_characters::comma>()) {
					return;
				}
			}
		}
	};

	template<typename value_type> [[maybe_unused]] JSONIFIER_INLINE static auto getBeginIterVec(value_type& value JSONIFIER_LIFETIME_BOUND) {
		if constexpr (std::is_same_v<typename value_type::value_type, bool>) {
			return value.begin();
		} else {
			return value.data();
		}
	}

	template<typename value_type> [[maybe_unused]] JSONIFIER_INLINE static auto getEndIterVec(value_type& value JSONIFIER_LIFETIME_BOUND) {
		if constexpr (std::is_same_v<typename value_type::value_type, bool>) {
			return value.end();
		} else {
			return value.data() + value.size();
		}
	}

	template<concepts::vector_t value_type, typename context_type, parse_options optionsNew> struct parse_impl<value_type, context_type, optionsNew> {
		static constexpr parse_options options{ optionsNew };
		JSONIFIER_INLINE static void impl(value_type& value, context_type& context) noexcept {
			if JSONIFIER_UNLIKELY (!context.template advanceTo<json_structural_characters::l_sqr_bracket>()) {
				return;
			}
			if JSONIFIER_LIKELY (context.template advanceMaybe<json_structural_characters::r_sqr_bracket>()) {
				value.clear();
				return;
			}
			uint64_t parsedCount{};
			if (auto size = value.size(); size > 0) {
				auto newIter = getBeginIterVec(value);
				for (uint64_t i{}; i != size; ++i) {
					parse<options>::impl(newIter[static_cast<int64_t>(i)], context);
					++parsedCount;
					if JSONIFIER_LIKELY (context.template advanceMaybe<json_structural_characters::r_sqr_bracket>()) {
						value.resize(parsedCount);
						return;
					}
					if JSONIFIER_UNLIKELY (!context.template advanceTo<json_structural_characters::comma>()) {
						return;
					}
				}
			}
			while (!context.atEnd()) {
				parse<options>::impl(value.emplace_back(), context);
				if JSONIFIER_LIKELY (context.template advanceMaybe<json_structural_characters::r_sqr_bracket>()) {
					return;
				}
				if JSONIFIER_UNLIKELY (!context.template advanceTo<json_structural_characters::comma>()) {
					return;
				}
			}
			context.template reportError<parse_statuses::unexpected_string_end>();
		}
	};

	template<concepts::raw_array_t value_type, typename context_type, parse_options options> struct parse_impl<value_type, context_type, options> {
		JSONIFIER_INLINE static void impl(value_type& value, context_type& context) noexcept {
			if JSONIFIER_UNLIKELY (!context.template advanceTo<json_structural_characters::l_sqr_bracket>()) {
				return;
			}
			if JSONIFIER_UNLIKELY (context.template advanceMaybe<json_structural_characters::r_sqr_bracket>()) {
				return;
			}
			if JSONIFIER_LIKELY (const uint64_t n_local = std::size(value); n_local > 0) {
				auto iterNew = std::begin(value);
				for (uint64_t i = 0; i < n_local; ++i) {
					parse<options>::impl(*(iterNew++), context);
					if JSONIFIER_UNLIKELY (context.template advanceMaybe<json_structural_characters::r_sqr_bracket>()) {
						return;
					}
					if JSONIFIER_UNLIKELY (!context.template advanceTo<json_structural_characters::comma>()) {
						return;
					}
				}
			}
			while (!context.atEnd()) {
				context.skipToNextValue();
				if JSONIFIER_LIKELY (context.template advanceMaybe<json_structural_characters::r_sqr_bracket>()) {
					return;
				}
				if JSONIFIER_UNLIKELY (!context.template advanceTo<json_structural_characters::comma>()) {
					return;
				}
			}
			context.template reportError<parse_statuses::unexpected_string_end>();
		}
	};

	template<concepts::tuple_t value_type, typename context_type, parse_options options> struct parse_impl<value_type, context_type, options> {
		JSONIFIER_INLINE static void impl(value_type& value, context_type& context) noexcept {
			static constexpr auto membercount = tuple_size_v<value_type>;
			if JSONIFIER_UNLIKELY (!context.template advanceTo<json_structural_characters::l_sqr_bracket>()) {
				return;
			}
			if JSONIFIER_UNLIKELY (context.template advanceMaybe<json_structural_characters::r_sqr_bracket>()) {
				return;
			}
			if constexpr (membercount > 0) {
				parse<options>::impl(get<0>(value), context);
				parseObjects<membercount, 1>(value, context);
			}
			context.template advanceTo<json_structural_characters::r_sqr_bracket>();
		}

		template<uint64_t membercount, uint64_t index> JSONIFIER_INLINE static void parseObjects(value_type& value, context_type& context) {
			if constexpr (index < membercount) {
				if JSONIFIER_UNLIKELY (!context.template advanceMaybe<json_structural_characters::comma>()) {
					return;
				}
				parse<options>::impl(get<index>(value), context);
				return parseObjects<membercount, index + 1>(value, context);
			}
		}
	};

	template<concepts::string_t value_type, typename context_type, parse_options options> struct parse_impl<value_type, context_type, options> {
		using base = derailleur<options, context_type>;
		JSONIFIER_INLINE static void impl(value_type& value, context_type& context) noexcept {
			context.parseStringValue(value);
		}
	};

	template<concepts::char_t value_type, typename context_type, parse_options options> struct parse_impl<value_type, context_type, options> {
		JSONIFIER_INLINE static void impl(value_type& value, context_type& context) noexcept {
			value = static_cast<value_type>(*(context.iter + 1));
			context.iter += sizeof(value_type) + 2;
		}
	};

	template<concepts::enum_t value_type, typename context_type, parse_options options> struct parse_impl<value_type, context_type, options> {
		JSONIFIER_INLINE static void impl(value_type& value, context_type& context) noexcept {
			uint64_t newValue{};
			context.parseNumberValue(newValue);
			value = static_cast<value_type>(newValue);
		}
	};

	template<concepts::num_t value_type, typename context_type, parse_options options> struct parse_impl<value_type, context_type, options> {
		JSONIFIER_INLINE static void impl(value_type& value, context_type& context) noexcept {
			context.parseNumberValue(value);
		}
	};

	template<concepts::bool_t value_type, typename context_type, parse_options options> struct parse_impl<value_type, context_type, options> {
		JSONIFIER_INLINE static void impl(value_type& value, context_type& context) noexcept {
			context.parseBoolValue(value);
		}
	};

	template<concepts::always_null_t value_type, typename context_type, parse_options options> struct parse_impl<value_type, context_type, options> {
		JSONIFIER_INLINE static void impl(value_type&, context_type& context) noexcept {
			context.parseNullValue();
		}
	};

	template<concepts::variant_t value_type, typename context_type, parse_options options> struct parse_impl<value_type, context_type, options> {
		using base = derailleur<options, context_type>;
		template<json_type type, typename variant_type, uint64_t currentIndex = 0>
		JSONIFIER_INLINE static void iterateVariantTypes(variant_type&& variant, context_type& context) noexcept {
			if constexpr (currentIndex < std::variant_size_v<remove_cvref_t<variant_type>>) {
				using element_type = remove_cvref_t<decltype(std::get<currentIndex>(std::declval<remove_cvref_t<variant_type>>()))>;
				if constexpr (concepts::jsonifier_object_t<element_type> && type == json_type::object) {
					parse<options>::impl(variant.template emplace<element_type>(element_type{}), context);
				} else if constexpr ((concepts::vector_t<element_type> || concepts::raw_array_t<element_type>) && type == json_type::array) {
					parse<options>::impl(variant.template emplace<element_type>(element_type{}), context);
				} else if constexpr ((concepts::string_t<element_type> || concepts::string_view_t<element_type>) && type == json_type::string) {
					parse<options>::impl(variant.template emplace<element_type>(element_type{}), context);
				} else if constexpr (concepts::bool_t<element_type> && type == json_type::boolean) {
					parse<options>::impl(variant.template emplace<element_type>(element_type{}), context);
				} else if constexpr ((concepts::num_t<element_type> || concepts::enum_t<element_type>) && type == json_type::number) {
					parse<options>::impl(variant.template emplace<element_type>(element_type{}), context);
				} else if constexpr (concepts::always_null_t<element_type> && type == json_type::null) {
					parse<options>::impl(variant.template emplace<element_type>(element_type{}), context);
				} else if constexpr (concepts::accessor_t<element_type> && type == json_type::accessor) {
					parse<options>::impl(variant.template emplace<element_type>(element_type{}), context);
				} else {
					return iterateVariantTypes<type, variant_type, currentIndex + 1>(variant, context);
				}
			}
		}

		JSONIFIER_INLINE static void impl(value_type& value, context_type& context) noexcept {
			switch (*context.iter) {
				case '{': {
					iterateVariantTypes<json_type::object>(value, context);
					break;
				}
				case '[': {
					iterateVariantTypes<json_type::array>(value, context);
					break;
				}
				case '"': {
					iterateVariantTypes<json_type::string>(value, context);
					break;
				}
				case 't':
					[[fallthrough]];
				case 'f': {
					iterateVariantTypes<json_type::boolean>(value, context);
					break;
				}
				case '-':
					[[fallthrough]];
				case '0':
					[[fallthrough]];
				case '1':
					[[fallthrough]];
				case '2':
					[[fallthrough]];
				case '3':
					[[fallthrough]];
				case '4':
					[[fallthrough]];
				case '5':
					[[fallthrough]];
				case '6':
					[[fallthrough]];
				case '7':
					[[fallthrough]];
				case '8':
					[[fallthrough]];
				case '9': {
					iterateVariantTypes<json_type::number>(value, context);
					break;
				}
				case 'n': {
					iterateVariantTypes<json_type::null>(value, context);
					break;
				}
				default: {
					iterateVariantTypes<json_type::accessor>(value, context);
					break;
				}
			}
		}
	};

	template<concepts::optional_t value_type, typename context_type, parse_options options> struct parse_impl<value_type, context_type, options> {
		JSONIFIER_INLINE static void impl(value_type& value, context_type& context) noexcept {
			if JSONIFIER_LIKELY (!context.atEnd() && context.currentChar() != 'n') {
				parse<options>::impl(value.emplace(), context);
			} else {
				context.parseNullValue();
			}
		}
	};

	template<concepts::shared_ptr_t value_type, typename context_type, parse_options options> struct parse_impl<value_type, context_type, options> {
		JSONIFIER_INLINE static void impl(value_type& value, context_type& context) noexcept {
			if JSONIFIER_LIKELY (!context.atEnd() && context.currentChar() != 'n') {
				using member_type = decltype(*value);
				if JSONIFIER_UNLIKELY (!value) {
					value = std::make_shared<jsonifier::internal::remove_pointer_t<remove_cvref_t<member_type>>>();
				}
				parse<options>::impl(*value, context);
			} else {
				context.parseNullValue();
			}
		}
	};

	template<concepts::unique_ptr_t value_type, typename context_type, parse_options options> struct parse_impl<value_type, context_type, options> {
		JSONIFIER_INLINE static void impl(value_type& value, context_type& context) noexcept {
			if JSONIFIER_LIKELY (!context.atEnd() && context.currentChar() != 'n') {
				using member_type = decltype(*value);
				if JSONIFIER_UNLIKELY (!value) {
					value = std::make_unique<jsonifier::internal::remove_pointer_t<remove_cvref_t<member_type>>>();
				}
				parse<options>::impl(*value, context);
			} else {
				context.parseNullValue();
			}
		}
	};

	template<concepts::pointer_t value_type, typename context_type, parse_options options> struct parse_impl<value_type, context_type, options> {
		JSONIFIER_INLINE static void impl(value_type& value, context_type& context) noexcept {
			if JSONIFIER_LIKELY (!context.atEnd() && context.currentChar() != 'n') {
				if JSONIFIER_UNLIKELY (!value) {
					value = new jsonifier::internal::remove_pointer_t<value_type>{};
				}
				parse<options>::impl(*value, context);
			} else {
				context.parseNullValue();
			}
		}
	};

	template<concepts::raw_json_t value_type, typename context_type, parse_options options> struct parse_impl<value_type, context_type, options> {
		JSONIFIER_INLINE static void impl(value_type& value, context_type& context) noexcept {
			const char* newPtr = context.iter;
			context.skipToNextValue();
			const char* endPtr = context.atEnd() ? context.endIter : context.iter;
			uint64_t newSize   = static_cast<uint64_t>(endPtr - newPtr);
			if constexpr (!options.minified) {
				context.skipWs();
			}
			if JSONIFIER_LIKELY (newSize > 0) {
				string newString{};
				newString.resize(newSize);
				std::memcpy(newString.data(), newPtr, newSize);
				value = value_type{ *context.parserPtr, newString };
			}
		}
	};

	template<concepts::skip_t value_type, typename context_type, parse_options options> struct parse_impl<value_type, context_type, options> {
		JSONIFIER_INLINE static void impl(value_type&, context_type& context) noexcept {
			context.skipToNextValue();
		}
	};

	template<typename value_type, typename context_type, parse_options options> struct parse_types_partial_impl {
		static constexpr auto membercount = core_tuple_size<value_type>;
		template<uint64_t index> inline static parse_result processIndex(value_type& value, context_type& context) {
			static constexpr auto tupleElem		= get_because_other_lib_authors_resolve<index>(core<value_type>::parseValue);
			static constexpr auto stringLiteral = escapedKeyLiteral<tupleElem.name>;
			static constexpr auto ptrNew		= tupleElem.memberPtr;
			static constexpr auto keySize		= stringLiteral.size();
			static constexpr auto keySizeNew	= keySize + 1;
			if JSONIFIER_LIKELY (((context.iter + 1) < context.endIter) && context.stringRoot[*context.iter + keySizeNew] == quote &&
				string_literal_comparitor<decltype(stringLiteral), stringLiteral>::impl((&context.stringRoot[*context.iter]) + 1)) {
				++context.iter;
				if JSONIFIER_LIKELY ((context.iter < context.endIter) && context.stringRoot[*context.iter] == colon) {
					++context.iter;
					if constexpr (concepts::has_excluded_keys<value_type>) {
						static constexpr auto key = stringLiteral.operator jsonifier::string_view();
						auto& keys				  = value.jsonifierExcludedKeys;
						if JSONIFIER_LIKELY (keys.find(static_cast<typename remove_cvref_t<decltype(keys)>::key_type>(key)) != keys.end()) {
							context.skipToNextValue();
							return parse_result::active_member;
						}
					}
					parse<options>::impl(value.*ptrNew, context);
					return parse_result::active_member;
				}
				JSONIFIER_ELSE_UNLIKELY(else) {
					context.template reportError<parse_statuses::missing_colon>();
					return parse_result::inactive_member;
				}
			}
			context.skipKey();
			if JSONIFIER_LIKELY ((context.iter < context.endIter) && context.stringRoot[*context.iter] == colon) {
				++context.iter;
				context.skipToNextValue();
			} else [[unlikely]] {
				context.template reportError<parse_statuses::missing_colon>();
			}
			return parse_result::inactive_member;
		}
	};

	template<parse_options options, typename json_entity_type> struct json_entity_parse_partial {
		static constexpr auto membercount{ core_tuple_size<typename json_entity_type::class_type> };

		template<typename value_type, typename context_type>
		inline static bool processIndex(value_type& value, context_type& context, uint64_t& remainingMembers) noexcept {
			if (remainingMembers == 0 || context.iter >= context.endIter || context.stringRoot[*context.iter] == rBrace) {
				return true;
			}
			if constexpr (json_entity_type::index > 0) {
				if JSONIFIER_LIKELY ((context.iter < context.endIter) && context.stringRoot[*context.iter] == comma) {
					++context.iter;
				}
			}
			if constexpr (options.knownOrder) {
				static constexpr auto stringLiteral = escapedKeyLiteral<json_entity_type::name>;
				static constexpr auto ptrNew		= json_entity_type::memberPtr;
				static constexpr auto keySize		= stringLiteral.size();
				static constexpr auto keySizeNew	= keySize + 1;
				if JSONIFIER_LIKELY (((context.iter + 1) < context.endIter) && context.stringRoot[*context.iter + keySizeNew] == quote &&
					string_literal_comparitor<decltype(stringLiteral), stringLiteral>::impl((&context.stringRoot[*context.iter]) + 1)) {
					++context.iter;
					if JSONIFIER_LIKELY ((context.iter < context.endIter) && context.stringRoot[*context.iter] == colon) {
						++context.iter;
						if constexpr (concepts::has_excluded_keys<value_type>) {
							static constexpr auto key = stringLiteral.operator jsonifier::string_view();
							auto& keys				  = value.jsonifierExcludedKeys;
							if JSONIFIER_LIKELY (keys.find(static_cast<typename remove_cvref_t<decltype(keys)>::key_type>(key)) != keys.end()) {
								context.skipToNextValue();
								--remainingMembers;
								return true;
							}
						}
						parse<options>::impl(value.*ptrNew, context);
						--remainingMembers;
						return true;
					}
					JSONIFIER_ELSE_UNLIKELY(else) {
						context.template reportError<parse_statuses::missing_colon>();
						return true;
					}
				}
			}

			if (const auto indexNew =
					hash_map<value_type, remove_cvref_t<decltype(context.stringRoot)>>::findIndex((&context.stringRoot[*context.iter]) + 1, context.stringRoot + *context.endIter);
				indexNew < membercount) {
				auto result = functionPtrs<parse_types_partial_impl, value_type, context_type, options>[indexNew](value, context);
				remainingMembers -= static_cast<uint64_t>(result);
			} else {
				context.skipKey();
				if JSONIFIER_LIKELY ((context.iter < context.endIter) && context.stringRoot[*context.iter] == colon) {
					++context.iter;
					context.skipToNextValue();
				} else [[unlikely]] {
					context.template reportError<parse_statuses::missing_colon>();
				}
			}
			return true;
		}
	};

	template<parse_options options, typename value_type, typename context_type, typename index_sequence, typename... value_types> struct get_parse_partial_base;

	template<parse_options options, typename value_type, typename context_type, uint64_t... index>
	struct get_parse_partial_base<options, value_type, context_type, index_sequence<index...>> {
		using type = parse_map<json_entity_parse_partial<options, remove_cvref_t<decltype(get_because_other_lib_authors_resolve<index>(core<value_type>::parseValue))>>...>;
	};

	template<parse_options options, typename value_type, typename context_type> using parse_base_partial_t =
		typename get_parse_partial_base<options, value_type, context_type, make_index_sequence<core_tuple_size<value_type>>>::type;

	template<concepts::jsonifier_object_t value_type, typename context_type, parse_options options> struct parse_partial_impl<value_type, context_type, options> {
		static constexpr auto memberCount = core_tuple_size<value_type>;

		template<typename value_type_new> inline static void skipRemainingMembers(context_type& context) noexcept {
			while (!context.template advanceMaybe<json_structural_characters::r_crl_bracket>()) {
				if JSONIFIER_UNLIKELY (context.atEnd()) {
					context.template reportError<parse_statuses::unexpected_string_end>();
					return;
				}
				context.template advanceMaybe<json_structural_characters::comma>();
				context.skipKey();
				if JSONIFIER_UNLIKELY (!context.template advanceTo<json_structural_characters::colon>()) {
					return;
				}
				context.skipToNextValue();
			}
		}

		inline static void impl(value_type& value, context_type& context) noexcept {
			if JSONIFIER_UNLIKELY (!context.template advanceTo<json_structural_characters::l_crl_bracket>()) {
				return;
			}
			if JSONIFIER_LIKELY (context.template advanceMaybe<json_structural_characters::r_crl_bracket>()) {
				return;
			}
			uint64_t remainingMembers{ memberCount };
			if (parse_base_partial_t<options, value_type, context_type>::iterateValues(value, context, remainingMembers)) {
				skipRemainingMembers<value_type>(context);
			}
		}
	};

	template<concepts::map_t value_type, typename context_type, parse_options options> struct parse_partial_impl<value_type, context_type, options> {
		JSONIFIER_INLINE static void impl(value_type& value, context_type& context) noexcept {
			if JSONIFIER_UNLIKELY (!context.template advanceTo<json_structural_characters::l_crl_bracket>()) {
				return;
			}
			if JSONIFIER_UNLIKELY (context.template advanceMaybe<json_structural_characters::r_crl_bracket>()) {
				return;
			}
			while (true) {
				if JSONIFIER_UNLIKELY (context.currentChar() != '"') {
					context.template reportError<parse_statuses::invalid_string_characters>();
					return;
				}
				parse<options>::impl(getKey<typename value_type::key_type>(), context);
				if JSONIFIER_UNLIKELY (!context.template advanceTo<json_structural_characters::colon>()) {
					return;
				}
				parse<options>::impl(value[getKey<typename value_type::key_type>()], context);
				if JSONIFIER_UNLIKELY (context.template advanceMaybe<json_structural_characters::r_crl_bracket>()) {
					return;
				}
				if JSONIFIER_UNLIKELY (!context.template advanceTo<json_structural_characters::comma>()) {
					return;
				}
			}
		}
	};

	template<concepts::vector_t value_type, typename context_type, parse_options optionsNew> struct parse_partial_impl<value_type, context_type, optionsNew> {
		static constexpr parse_options options{ optionsNew };
		inline static void impl(value_type& value, context_type& context) noexcept {
			if JSONIFIER_UNLIKELY (!context.template advanceTo<json_structural_characters::l_sqr_bracket>()) {
				return;
			}
			if JSONIFIER_LIKELY (context.template advanceMaybe<json_structural_characters::r_sqr_bracket>()) {
				value.clear();
				return;
			}
			uint64_t parsedCount{};
			if (auto size = value.size(); size > 0) {
				auto newIter = getBeginIterVec(value);
				for (uint64_t i{}; i != size; ++i) {
					parse<options>::impl(newIter[static_cast<int64_t>(i)], context);
					++parsedCount;
					if JSONIFIER_LIKELY (context.template advanceMaybe<json_structural_characters::r_sqr_bracket>()) {
						value.resize(parsedCount);
						return;
					}
					if JSONIFIER_UNLIKELY (!context.template advanceTo<json_structural_characters::comma>()) {
						return;
					}
				}
			}
			while (!context.atEnd()) {
				parse<options>::impl(value.emplace_back(), context);
				if JSONIFIER_LIKELY (context.template advanceMaybe<json_structural_characters::r_sqr_bracket>()) {
					return;
				}
				if JSONIFIER_UNLIKELY (!context.template advanceTo<json_structural_characters::comma>()) {
					return;
				}
			}
			context.template reportError<parse_statuses::unexpected_string_end>();
		}
	};

	template<concepts::raw_array_t value_type, typename context_type, parse_options options> struct parse_partial_impl<value_type, context_type, options> {
		JSONIFIER_INLINE static void impl(value_type& value, context_type& context) noexcept {
			if JSONIFIER_UNLIKELY (!context.template advanceTo<json_structural_characters::l_sqr_bracket>()) {
				return;
			}
			if JSONIFIER_UNLIKELY (context.template advanceMaybe<json_structural_characters::r_sqr_bracket>()) {
				return;
			}
			if JSONIFIER_LIKELY (const uint64_t n_local = std::size(value); n_local > 0) {
				auto iterNew = std::begin(value);
				for (uint64_t i = 0; i < n_local; ++i) {
					parse<options>::impl(*(iterNew++), context);
					if JSONIFIER_UNLIKELY (context.template advanceMaybe<json_structural_characters::r_sqr_bracket>()) {
						return;
					}
					if JSONIFIER_UNLIKELY (!context.template advanceTo<json_structural_characters::comma>()) {
						return;
					}
				}
			}
			while (!context.atEnd()) {
				context.skipToNextValue();
				if JSONIFIER_LIKELY (context.template advanceMaybe<json_structural_characters::r_sqr_bracket>()) {
					return;
				}
				if JSONIFIER_UNLIKELY (!context.template advanceTo<json_structural_characters::comma>()) {
					return;
				}
			}
			context.template reportError<parse_statuses::unexpected_string_end>();
		}
	};

	template<concepts::tuple_t value_type, typename context_type, parse_options options> struct parse_partial_impl<value_type, context_type, options> {
		JSONIFIER_INLINE static void impl(value_type& value, context_type& context) noexcept {
			static constexpr auto membercount = tuple_size_v<value_type>;
			if JSONIFIER_UNLIKELY (!context.template advanceTo<json_structural_characters::l_sqr_bracket>()) {
				return;
			}
			if JSONIFIER_UNLIKELY (context.template advanceMaybe<json_structural_characters::r_sqr_bracket>()) {
				return;
			}
			if constexpr (membercount > 0) {
				parse<options>::impl(get<0>(value), context);
				parseObjects<membercount, 1>(value, context);
			}
			context.template advanceTo<json_structural_characters::r_sqr_bracket>();
		}

		template<uint64_t membercount, uint64_t index> JSONIFIER_INLINE static void parseObjects(value_type& value, context_type& context) {
			if constexpr (index < membercount) {
				if JSONIFIER_UNLIKELY (!context.template advanceMaybe<json_structural_characters::comma>()) {
					return;
				}
				parse<options>::impl(get<index>(value), context);
				return parseObjects<membercount, index + 1>(value, context);
			}
		}
	};

	template<concepts::string_t value_type, typename context_type, parse_options options> struct parse_partial_impl<value_type, context_type, options> {
		using base = derailleur<options, context_type>;

		JSONIFIER_INLINE static void impl(value_type& value, context_type& context) noexcept {
			context.parseStringValue(value);
		}
	};

	template<concepts::char_t value_type, typename context_type, parse_options options> struct parse_partial_impl<value_type, context_type, options> {
		using base = derailleur<options, context_type>;

		JSONIFIER_INLINE static void impl(value_type& value, context_type& context) noexcept {
			value = static_cast<value_type>(context.stringRoot[*context.iter + 1]);
			++context.iter;
		}
	};

	template<concepts::enum_t value_type, typename context_type, parse_options options> struct parse_partial_impl<value_type, context_type, options> {
		JSONIFIER_INLINE static void impl(value_type& value, context_type& context) noexcept {
			uint64_t newValue{};
			context.parseNumberValue(newValue);
			value = static_cast<value_type>(newValue);
		}
	};

	template<concepts::num_t value_type, typename context_type, parse_options options> struct parse_partial_impl<value_type, context_type, options> {
		JSONIFIER_INLINE static void impl(value_type& value, context_type& context) noexcept {
			context.parseNumberValue(value);
		}
	};

	template<concepts::bool_t value_type, typename context_type, parse_options options> struct parse_partial_impl<value_type, context_type, options> {
		JSONIFIER_INLINE static void impl(value_type& value, context_type& context) noexcept {
			context.parseBoolValue(value);
		}
	};

	template<concepts::always_null_t value_type, typename context_type, parse_options options> struct parse_partial_impl<value_type, context_type, options> {
		JSONIFIER_INLINE static void impl(value_type&, context_type& context) noexcept {
			context.parseNullValue();
		}
	};

	template<concepts::variant_t value_type, typename context_type, parse_options options> struct parse_partial_impl<value_type, context_type, options> {
		using base = derailleur<options, context_type>;

		template<json_type type, typename variant_type, uint64_t currentIndex = 0>
		JSONIFIER_INLINE static void iterateVariantTypes(variant_type&& variant, context_type& context) noexcept {
			if constexpr (currentIndex < std::variant_size_v<remove_cvref_t<variant_type>>) {
				using element_type = remove_cvref_t<decltype(std::get<currentIndex>(std::declval<remove_cvref_t<variant_type>>()))>;
				if constexpr (concepts::jsonifier_object_t<element_type> && type == json_type::object) {
					parse<options>::impl(variant.template emplace<element_type>(element_type{}), context);
				} else if constexpr ((concepts::vector_t<element_type> || concepts::raw_array_t<element_type>) && type == json_type::array) {
					parse<options>::impl(variant.template emplace<element_type>(element_type{}), context);
				} else if constexpr ((concepts::string_t<element_type> || concepts::string_view_t<element_type>) && type == json_type::string) {
					parse<options>::impl(variant.template emplace<element_type>(element_type{}), context);
				} else if constexpr (concepts::bool_t<element_type> && type == json_type::boolean) {
					parse<options>::impl(variant.template emplace<element_type>(element_type{}), context);
				} else if constexpr ((concepts::num_t<element_type> || concepts::enum_t<element_type>) && type == json_type::number) {
					parse<options>::impl(variant.template emplace<element_type>(element_type{}), context);
				} else if constexpr (concepts::always_null_t<element_type> && type == json_type::null) {
					parse<options>::impl(variant.template emplace<element_type>(element_type{}), context);
				} else if constexpr (concepts::accessor_t<element_type> && type == json_type::accessor) {
					parse<options>::impl(variant.template emplace<element_type>(element_type{}), context);
				} else {
					return iterateVariantTypes<type, variant_type, currentIndex + 1>(variant, context);
				}
			}
		}

		JSONIFIER_INLINE static void impl(value_type& value, context_type& context) noexcept {
			switch (*context.iter) {
				case '{': {
					iterateVariantTypes<json_type::object>(value, context);
					break;
				}
				case '[': {
					iterateVariantTypes<json_type::array>(value, context);
					break;
				}
				case '"': {
					iterateVariantTypes<json_type::string>(value, context);
					break;
				}
				case 't':
					[[fallthrough]];
				case 'f': {
					iterateVariantTypes<json_type::boolean>(value, context);
					break;
				}
				case '-':
					[[fallthrough]];
				case '0':
					[[fallthrough]];
				case '1':
					[[fallthrough]];
				case '2':
					[[fallthrough]];
				case '3':
					[[fallthrough]];
				case '4':
					[[fallthrough]];
				case '5':
					[[fallthrough]];
				case '6':
					[[fallthrough]];
				case '7':
					[[fallthrough]];
				case '8':
					[[fallthrough]];
				case '9': {
					iterateVariantTypes<json_type::number>(value, context);
					break;
				}
				case 'n': {
					iterateVariantTypes<json_type::null>(value, context);
					break;
				}
				default: {
					iterateVariantTypes<json_type::accessor>(value, context);
					break;
				}
			}
		}
	};

	template<concepts::optional_t value_type, typename context_type, parse_options options> struct parse_partial_impl<value_type, context_type, options> {
		JSONIFIER_INLINE static void impl(value_type& value, context_type& context) noexcept {
			if JSONIFIER_LIKELY (!context.atEnd() && context.currentChar() != 'n') {
				parse<options>::impl(value.emplace(), context);
			} else {
				context.parseNullValue();
				value.reset();
			}
		}
	};

	template<concepts::shared_ptr_t value_type, typename context_type, parse_options options> struct parse_partial_impl<value_type, context_type, options> {
		JSONIFIER_INLINE static void impl(value_type& value, context_type& context) noexcept {
			if JSONIFIER_LIKELY (!context.atEnd() && context.currentChar() != 'n') {
				using member_type = decltype(*value);
				if JSONIFIER_UNLIKELY (!value) {
					value = std::make_shared<jsonifier::internal::remove_pointer_t<remove_cvref_t<member_type>>>();
				}
				parse<options>::impl(*value, context);
			} else {
				context.parseNullValue();
			}
		}
	};

	template<concepts::unique_ptr_t value_type, typename context_type, parse_options options> struct parse_partial_impl<value_type, context_type, options> {
		JSONIFIER_INLINE static void impl(value_type& value, context_type& context) noexcept {
			if JSONIFIER_LIKELY (!context.atEnd() && context.currentChar() != 'n') {
				using member_type = decltype(*value);
				if JSONIFIER_UNLIKELY (!value) {
					value = std::make_unique<jsonifier::internal::remove_pointer_t<remove_cvref_t<member_type>>>();
				}
				parse<options>::impl(*value, context);
			} else {
				context.parseNullValue();
			}
		}
	};

	template<concepts::pointer_t value_type, typename context_type, parse_options options> struct parse_partial_impl<value_type, context_type, options> {
		JSONIFIER_INLINE static void impl(value_type& value, context_type& context) noexcept {
			if JSONIFIER_LIKELY (!context.atEnd() && context.currentChar() != 'n') {
				if JSONIFIER_UNLIKELY (!value) {
					value = new jsonifier::internal::remove_pointer_t<value_type>{};
				}
				parse<options>::impl(*value, context);
			} else {
				context.parseNullValue();
			}
		}
	};

	template<concepts::raw_json_t value_type, typename context_type, parse_options options> struct parse_partial_impl<value_type, context_type, options> {
		JSONIFIER_INLINE static void impl(value_type& value, context_type& context) noexcept {
			const char* newPtr = &context.stringRoot[*context.iter];
			const char* endPtr = context.atEnd() ? &context.stringRoot[*context.endIter] : &context.stringRoot[*context.iter];
			uint64_t newSize   = static_cast<uint64_t>(endPtr - newPtr);
			if constexpr (!options.minified) {
				while (newSize > 0 && (newPtr[newSize - 1] == ' ' || newPtr[newSize - 1] == '\t' || newPtr[newSize - 1] == '\n' || newPtr[newSize - 1] == '\r')) {
					--newSize;
				}
			}
			if JSONIFIER_LIKELY (newSize > 0) {
				string newString{};
				newString.resize(newSize);
				std::memcpy(newString.data(), newPtr, newSize);
				value = value_type{ *context.parserPtr, newString };
			}
		}
	};

	template<concepts::skip_t value_type, typename context_type, parse_options options> struct parse_partial_impl<value_type, context_type, options> {
		JSONIFIER_INLINE static void impl(value_type&, context_type& context) noexcept {
			context.skipToNextValue();
		}
	};
}
