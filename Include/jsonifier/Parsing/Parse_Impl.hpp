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

#include <jsonifier/Utilities/NumberUtils.hpp>
#include <jsonifier/Utilities/StringUtils.hpp>
#include <jsonifier/Parsing/Parser.hpp>

#include <memory>

namespace jsonifier::internal {

	template<typename value_type, typename context_type, parse_options options, bool minifiedOrInsideRepeated> struct parse_types_impl {
		using base						  = derailleur<options, context_type>;
		inline static constexpr auto memberCount = core_tuple_size<value_type>;
		template<size_t index> inline static bool processIndex(value_type& value, context_type& context) {
			static constexpr auto tupleElem						= get<index>(core<value_type>::parseValue);
			static constexpr auto stringLiteral					= tupleElem.name;
			static constexpr auto ptrNew						= tupleElem.memberPtr;
			static constexpr auto keySize						= stringLiteral.size();
			static constexpr auto keySizeNew					= keySize + 1;
			if JSONIFIER_LIKELY (((context.iter + keySize) < context.endIter) && (*(context.iter + keySize)) == quote &&
				string_literal_comparitor<decltype(stringLiteral), stringLiteral>::impl(context.iter)) {
				context.iter += keySizeNew;
				if constexpr (!minifiedOrInsideRepeated) {
					JSONIFIER_SKIP_WS();
				}
				if JSONIFIER_LIKELY ((context.iter < context.endIter) && *context.iter == colon) {
					++context.iter;
					if constexpr (!minifiedOrInsideRepeated) {
						JSONIFIER_SKIP_WS();
					}
					if constexpr (concepts::has_excluded_keys<value_type>) {
						static constexpr auto key = stringLiteral.operator jsonifier::string_view();
						auto& keys				  = value.jsonifierExcludedKeys;
						if JSONIFIER_LIKELY (keys.find(static_cast<typename remove_cvref_t<decltype(keys)>::key_type>(key)) != keys.end()) {
							base::skipToNextValue(context);
							return true;
						}
					}
					parse<options, minifiedOrInsideRepeated>::impl(value.*ptrNew, context);
					return true;
				}
				JSONIFIER_ELSE_UNLIKELY(else) {
					context.parserPtr->template reportError<parse_errors::Missing_Colon>(context);
					base::skipToNextValue(context);
				}
			}
			return false;
		}
	};

	template<typename value_type, typename context_type, parse_options options, bool minifiedOrInsideRepeated> struct parse_types_partial_impl {
		using base						  = derailleur<options, context_type>;
		static constexpr auto memberCount = core_tuple_size<value_type>;
		template<size_t index> inline static bool processIndex(value_type& value, context_type& context) {
			static constexpr auto tupleElem						= get<index>(core<value_type>::parseValue);
			static constexpr auto stringLiteral					= tupleElem.name;
			static constexpr auto ptrNew						= tupleElem.memberPtr;
			static constexpr auto keySize						= stringLiteral.size();
			static constexpr auto keySizeNew					= keySize + 1;
			if JSONIFIER_LIKELY (((context.iter + 1) < context.endIter) && (*((*context.iter) + keySizeNew)) == quote &&
				string_literal_comparitor<decltype(stringLiteral), stringLiteral>::impl((*context.iter) + 1)) {
				++context.iter;
				if JSONIFIER_LIKELY ((context.iter < context.endIter) && **context.iter == colon) {
					++context.iter;
					if constexpr (concepts::has_excluded_keys<value_type>) {
						static constexpr auto key = stringLiteral.operator jsonifier::string_view();
						auto& keys				  = value.jsonifierExcludedKeys;
						if JSONIFIER_LIKELY (keys.find(static_cast<typename remove_cvref_t<decltype(keys)>::key_type>(key)) != keys.end()) {
							base::skipToNextValue(context);
							return false;
						}
					}
					parse<options, minifiedOrInsideRepeated>::impl(value.*ptrNew, context);
					return true;
				}
				JSONIFIER_ELSE_UNLIKELY(else) {
					context.parserPtr->template reportError<parse_errors::Missing_Colon>(context);
					base::skipToNextValue(context);
				}
			}
			return false;
		}
	};

	template<template<typename, typename, parse_options, bool> typename parsing_type, typename value_type, typename context_type, parse_options options,
		bool minifiedOrInsideRepeated, size_t... indices>
	inline static constexpr auto generateFunctionPtrs(index_sequence<indices...>) noexcept {
		using function_type = decltype(&parse_types_impl<value_type, context_type, options, minifiedOrInsideRepeated>::template processIndex<0>);
		return array<function_type, sizeof...(indices)>{ { &parsing_type<value_type, context_type, options, minifiedOrInsideRepeated>::template processIndex<indices>... } };
	}

	template<parse_options options, bool newLines = true> struct check_for_entry_comma {
		template<typename context_type> JSONIFIER_INLINE static void impl(context_type& context, string_view_ptr wsStart, size_t wsSize) {
			using base = derailleur<options, context_type>;
			if JSONIFIER_LIKELY (*context.iter == comma) {
				++context.iter;
				if constexpr (!options.minified) {
					JSONIFIER_SKIP_MATCHING_WS()
				}
			}
			JSONIFIER_ELSE_UNLIKELY(else) {
				context.parserPtr->template reportError<parse_errors::Missing_Comma>(context);
				base::skipToNextValue(context);
				return;
			}
		}

		template<typename context_type> JSONIFIER_INLINE static void impl(context_type& context) {
			using base = derailleur<options, context_type>;
			if JSONIFIER_LIKELY (*context.iter == comma) {
				++context.iter;
			}
			JSONIFIER_ELSE_UNLIKELY(else) {
				context.parserPtr->template reportError<parse_errors::Missing_Comma>(context);
				base::skipToNextValue(context);
				return;
			}
		}

		template<partial_reading_context_t context_type> JSONIFIER_INLINE static void impl(context_type& context) {
			using base = derailleur<options, context_type>;
			if JSONIFIER_LIKELY (**context.iter == comma) {
				++context.iter;
			}
			JSONIFIER_ELSE_UNLIKELY(else) {
				context.parserPtr->template reportError<parse_errors::Missing_Comma>(context);
				base::skipToNextValue(context);
				return;
			}
		}
	};

	template<template<typename, typename, parse_options, bool> typename parsing_type, typename value_type, typename context_type, parse_options options,
		bool minifiedOrInsideRepeated>
	inline static constexpr auto functionPtrs{ generateFunctionPtrs<parsing_type, value_type, context_type, options, minifiedOrInsideRepeated>(
		make_index_sequence<core_tuple_size<value_type>>{}) };

	template<parse_options options, typename json_entity_type, bool minified> struct json_entity_parse;

	template<size_t memberCount, typename value_type> inline static thread_local array<uint64_t, (memberCount > 0 ? memberCount : 1)> antiHashStates{ []() constexpr {
		array<uint64_t, (memberCount > 0 ? memberCount : 1)> returnValues{};
		for (uint64_t x = 0; x < memberCount; ++x) {
			returnValues[x] = x;
		}
		return returnValues;
	}() };

	template<parse_options options, typename json_entity_type> struct json_entity_parse<options, json_entity_type, false> : public json_entity_type {
		static constexpr auto memberCount{ core_tuple_size<typename json_entity_type::class_type> };

		constexpr json_entity_parse() noexcept = default;

		template<typename value_type, typename context_type>
		JSONIFIER_INLINE static void processIndexImpl(value_type& value, context_type& context, string_view_ptr wsStart, size_t wsSize) {
			using base = derailleur<options, context_type>;

			if constexpr (memberCount == 1) {
				if JSONIFIER_LIKELY (parse_types_impl<value_type, context_type, options, false>::template processIndex<0>(value, context)) {
					return;
				}
			} else if constexpr (options.knownOrder) {
				static constexpr auto stringLiteral					= json_entity_type::name;
				static constexpr auto ptrNew						= json_entity_type::memberPtr;
				static constexpr auto keySize						= stringLiteral.size();
				static constexpr auto keySizeNew					= keySize + 1;
				if JSONIFIER_LIKELY (((context.iter + keySize) < context.endIter) && (*(context.iter + keySize)) == quote &&
					string_literal_comparitor<decltype(stringLiteral), stringLiteral>::impl(context.iter)) {
					context.iter += keySizeNew;
					JSONIFIER_SKIP_WS();
					if JSONIFIER_LIKELY ((context.iter < context.endIter) && *context.iter == colon) {
						++context.iter;
						JSONIFIER_SKIP_WS();
						if constexpr (concepts::has_excluded_keys<value_type>) {
							static constexpr auto key = stringLiteral.operator string_view();
							auto& keys				  = value.jsonifierExcludedKeys;
							if JSONIFIER_LIKELY (keys.find(static_cast<typename remove_cvref_t<decltype(keys)>::key_type>(key)) != keys.end()) {
								base::skipToNextValue(context);
								return;
							}
						}
						parse<options, false>::impl(value.*ptrNew, context);
						return;
					}
					JSONIFIER_ELSE_UNLIKELY(else) {
						context.parserPtr->template reportError<parse_errors::Missing_Colon>(context);
						base::skipToNextValue(context);
					}
				} else {
					if JSONIFIER_LIKELY (functionPtrs<parse_types_impl, value_type, context_type, options, false>[antiHashStates<memberCount, value_type>[json_entity_type::index]](
											 value, context)) {
						return;
					}
				}
			}
			if constexpr (memberCount == 2) {
				const auto indexNew = hash_map<value_type, remove_cvref_t<decltype(context.iter)>>::findIndex(context.iter, context.endIter);
				if (indexNew == 0) {
					if JSONIFIER_LIKELY (parse_types_impl<value_type, context_type, options, false>::template processIndex<0>(value, context)) {
						if constexpr (options.knownOrder) {
							antiHashStates<memberCount, value_type>[json_entity_type::index] = indexNew;
						}
						return;
					}
				} else {
					if JSONIFIER_LIKELY (parse_types_impl<value_type, context_type, options, false>::template processIndex<1>(value, context)) {
						if constexpr (options.knownOrder) {
							antiHashStates<memberCount, value_type>[json_entity_type::index] = indexNew;
						}
						return;
					}
				}
			} else {
				if JSONIFIER_LIKELY (const auto indexNew = hash_map<value_type, remove_cvref_t<decltype(context.iter)>>::findIndex(context.iter, context.endIter);
					indexNew < memberCount) {
					if JSONIFIER_LIKELY (functionPtrs<parse_types_impl, value_type, context_type, options, false>[indexNew](value, context)) {
						if constexpr (options.knownOrder) {
							antiHashStates<memberCount, value_type>[json_entity_type::index] = indexNew;
						}
						return;
					}
				}
			}
			base::template skipKeyStarted<value_type>(context);
			++context.iter;
			JSONIFIER_SKIP_WS();
			if JSONIFIER_LIKELY ((context.iter < context.endIter) && *context.iter == ':') {
				++context.iter;
				JSONIFIER_SKIP_WS();
				base::skipToNextValue(context);
				return processIndex<value_type, context_type, true>(value, context, wsStart, wsSize);
			} else [[unlikely]] {
				context.parserPtr->template reportError<parse_errors::Missing_Colon>(context);
				base::skipToNextValue(context);
				return;
			}
		}

		template<typename value_type, typename context_type, bool haveWeStarted = false>
		inline static void processIndex(value_type& value, context_type& context, string_view_ptr wsStart, size_t wsSize) {
			using base = derailleur<options, context_type>;
			if constexpr (memberCount > 0 && json_entity_type::index < memberCount) {
				if JSONIFIER_LIKELY (context.iter < context.endIter) {
					if (*context.iter != rBrace) {
						if constexpr (haveWeStarted || json_entity_type::index > 0) {
							check_for_entry_comma<options>::impl(context, wsStart, wsSize);
						}
						if JSONIFIER_LIKELY ((context.iter < context.endIter) && *context.iter == quote) {
							++context.iter;
							processIndexImpl(value, context, wsStart, wsSize);
						}
						JSONIFIER_ELSE_UNLIKELY(else) {
							context.parserPtr->template reportError<parse_errors::Missing_String_Start>(context);
							base::template skipKeyStarted<value_type>(context);
							++context.iter;
							JSONIFIER_SKIP_WS();
							if JSONIFIER_LIKELY ((context.iter < context.endIter) && *context.iter == ':') {
								++context.iter;
								JSONIFIER_SKIP_WS();
							} else [[unlikely]] {
								context.parserPtr->template reportError<parse_errors::Missing_Colon>(context);
								base::skipToNextValue(context);
								return;
							}
							base::skipToNextValue(context);
							return;
						}
					} else {
						return;
					}
				}
				JSONIFIER_ELSE_UNLIKELY(else) {
					context.parserPtr->template reportError<parse_errors::Unexpected_String_End>(context);
					base::skipToNextValue(context);
					return;
				}
			}
			return;
		}
	};

	template<parse_options options, typename json_entity_type> struct json_entity_parse<options, json_entity_type, true> : public json_entity_type {
		static constexpr auto memberCount{ core_tuple_size<typename json_entity_type::class_type> };

		constexpr json_entity_parse() noexcept = default;

		template<typename value_type, typename context_type> inline static void processIndexImpl(value_type& value, context_type& context) {
			using base = derailleur<options, context_type>;
			if constexpr (memberCount == 1) {
				if JSONIFIER_LIKELY (parse_types_impl<value_type, context_type, options, true>::template processIndex<0>(value, context)) {
					return;
				}
			} else if constexpr (options.knownOrder) {
				static constexpr auto stringLiteral					= json_entity_type::name;
				static constexpr auto ptrNew						= json_entity_type::memberPtr;
				static constexpr auto keySize						= stringLiteral.size();
				static constexpr auto keySizeNew					= keySize + 1;
				if JSONIFIER_LIKELY (((context.iter + keySize) < context.endIter) && (*(context.iter + keySize)) == quote &&
					string_literal_comparitor<decltype(stringLiteral), stringLiteral>::impl(context.iter)) {
					context.iter += keySizeNew;
					if JSONIFIER_LIKELY ((context.iter < context.endIter) && *context.iter == colon) {
						++context.iter;
						if constexpr (concepts::has_excluded_keys<value_type>) {
							static constexpr auto key = stringLiteral.operator string_view();
							const auto& keys		  = value.jsonifierExcludedKeys;
							if JSONIFIER_LIKELY (keys.find(static_cast<typename remove_cvref_t<decltype(keys)>::key_type>(key)) != keys.end()) {
								base::skipToNextValue(context);
								return;
							}
						}
						parse<options, true>::impl(value.*ptrNew, context);
						return;
					}
					JSONIFIER_ELSE_UNLIKELY(else) {
						context.parserPtr->template reportError<parse_errors::Missing_Colon>(context);
						base::skipToNextValue(context);
					}
				} else {
					if JSONIFIER_LIKELY (functionPtrs<parse_types_impl, value_type, context_type, options, true>[antiHashStates<memberCount, value_type>[json_entity_type::index]](
											 value, context)) {
						return;
					}
				}
			}
			if constexpr (memberCount == 2) {
				const auto indexNew = hash_map<value_type, remove_cvref_t<decltype(context.iter)>>::findIndex(context.iter, context.endIter);
				if (indexNew == 0) {
					if JSONIFIER_LIKELY (parse_types_impl<value_type, context_type, options, true>::template processIndex<0>(value, context)) {
						if constexpr (options.knownOrder) {
							antiHashStates<memberCount, value_type>[json_entity_type::index] = indexNew;
						}
						return;
					}
				} else {
					if JSONIFIER_LIKELY (parse_types_impl<value_type, context_type, options, true>::template processIndex<1>(value, context)) {
						if constexpr (options.knownOrder) {
							antiHashStates<memberCount, value_type>[json_entity_type::index] = indexNew;
						}
						return;
					}
				}
			} else {
				if JSONIFIER_LIKELY (const auto indexNew = hash_map<value_type, remove_cvref_t<decltype(context.iter)>>::findIndex(context.iter, context.endIter);
					indexNew < memberCount) {
					if JSONIFIER_LIKELY (functionPtrs<parse_types_impl, value_type, context_type, options, true>[indexNew](value, context)) {
						if constexpr (options.knownOrder) {
							antiHashStates<memberCount, value_type>[json_entity_type::index] = indexNew;
						}
						return;
					}
				}
			}
			base::template skipKeyStarted<value_type>(context);
			++context.iter;
			if JSONIFIER_LIKELY ((context.iter < context.endIter) && *context.iter == ':') {
				++context.iter;
				base::skipToNextValue(context);
				return processIndex<value_type, context_type, true>(value, context);
			} else [[unlikely]] {
				context.parserPtr->template reportError<parse_errors::Missing_Colon>(context);
				base::skipToNextValue(context);
				return;
			}
		}

		template<typename value_type, typename context_type, bool haveWeStarted = false> JSONIFIER_INLINE static void processIndex(value_type& value, context_type& context) {
			using base = derailleur<options, context_type>;
			if constexpr (memberCount > 0 && json_entity_type::index < memberCount) {
				if JSONIFIER_LIKELY (context.iter < context.endIter) {
					if (*context.iter != rBrace) {
						if constexpr (haveWeStarted || json_entity_type::index > 0) {
							check_for_entry_comma<options>::impl(context);
						}
						if JSONIFIER_LIKELY ((context.iter < context.endIter) && *context.iter == quote) {
							++context.iter;
							processIndexImpl(value, context);
						}
						JSONIFIER_ELSE_UNLIKELY(else) {
							context.parserPtr->template reportError<parse_errors::Missing_String_Start>(context);
							base::template skipKeyStarted<value_type>(context);
							++context.iter;
							if JSONIFIER_LIKELY ((context.iter < context.endIter) && *context.iter == ':') {
								++context.iter;
							} else [[unlikely]] {
								context.parserPtr->template reportError<parse_errors::Missing_Colon>(context);
								base::skipToNextValue(context);
								return;
							}
							base::skipToNextValue(context);
							return;
						}
					} else {
						return;
					}
				}
				JSONIFIER_ELSE_UNLIKELY(else) {
					context.parserPtr->template reportError<parse_errors::Unexpected_String_End>(context);
					base::skipToNextValue(context);
					return;
				}
			}
			return;
		}
	};

	template<typename... bases> struct parse_map : public bases... {
		template<typename json_entity_type, typename... arg_types> inline static void iterateValuesImpl(arg_types&&... args) {
			json_entity_type::processIndex(internal::forward<arg_types>(args)...);
		}

		template<typename... arg_types> static constexpr void iterateValues(arg_types&&... args) {
			(( void )(args), ...);
			((iterateValuesImpl<bases>(internal::forward<arg_types>(args)...)), ...);
		}
	};

	template<parse_options options, typename value_type, typename context_type, bool minifiedOrInsideRepeated, typename index_sequence, typename... value_types>
	struct get_parse_base;

	template<parse_options options, typename value_type, typename context_type, bool minifiedOrInsideRepeated, size_t... index>
	struct get_parse_base<options, value_type, context_type, minifiedOrInsideRepeated, index_sequence<index...>> {
		using type = parse_map<json_entity_parse<options, remove_cvref_t<decltype(get<index>(core<value_type>::parseValue))>, minifiedOrInsideRepeated>...>;
	};

	template<parse_options options, typename value_type, typename context_type, bool minifiedOrInsideRepeated> using parse_base_t =
		typename get_parse_base<options, value_type, context_type, minifiedOrInsideRepeated, make_index_sequence<core_tuple_size<value_type>>>::type;

	template<parse_options options, typename json_entity_type, bool minified> struct json_entity_parse_partial;

	template<parse_options options, typename json_entity_type, bool minifiedOrInsideRepeated> struct json_entity_parse_partial : public json_entity_type {
		inline static constexpr auto memberCount{ core_tuple_size<typename json_entity_type::class_type> };

		constexpr json_entity_parse_partial() noexcept = default;

		template<typename value_type, typename context_type, bool haveWeStarted = false>
		JSONIFIER_INLINE static void processIndex(value_type& value, context_type& context) noexcept {
			using base = derailleur<options, context_type>;
			if constexpr (json_entity_type::index < memberCount) {
				if (**context.iter != rBrace) {
					if JSONIFIER_LIKELY (context.iter < context.endIter) {
						if constexpr (haveWeStarted || json_entity_type::index > 0) {
							check_for_entry_comma<options>::impl(context);
						}

						if JSONIFIER_LIKELY ((context.iter < context.endIter) && **context.iter == quote) {
							if constexpr (memberCount == 1) {
								if JSONIFIER_LIKELY (parse_types_partial_impl<value_type, context_type, options, minifiedOrInsideRepeated>::template processIndex<0>(value,
														 context)) {
									return;
								}
							} else if constexpr (memberCount == 2) {
								const auto indexNew = hash_map<value_type, remove_cvref_t<decltype(*context.iter)>>::findIndex((*context.iter) + 1, *context.endIter);
								if (indexNew == 0) {
									if JSONIFIER_LIKELY (parse_types_partial_impl<value_type, context_type, options, minifiedOrInsideRepeated>::template processIndex<0>(value,
															 context)) {
										return;
									}
								} else {
									if JSONIFIER_LIKELY (parse_types_partial_impl<value_type, context_type, options, minifiedOrInsideRepeated>::template processIndex<1>(value,
															 context)) {
										return;
									}
								}
							} else {
								if JSONIFIER_LIKELY (const auto indexNew =
														 hash_map<value_type, remove_cvref_t<decltype(*context.iter)>>::findIndex((*context.iter) + 1, *context.endIter);
									indexNew < memberCount) {
									if JSONIFIER_LIKELY (functionPtrs<parse_types_partial_impl, value_type, context_type, options, minifiedOrInsideRepeated>[indexNew](value,
															 context)) {
										return;
									}
								}
							}
							base::template skipKey<value_type>(context);
							if JSONIFIER_LIKELY ((context.iter < context.endIter) && **context.iter == ':') {
								++context.iter;
							} else [[unlikely]] {
								context.parserPtr->template reportError<parse_errors::Missing_Colon>(context);
								base::skipToNextValue(context);
								return;
							}
							base::skipToNextValue(context);
							return processIndex<value_type, context_type, true>(value, context);
						}
						JSONIFIER_ELSE_UNLIKELY(else) {
							context.parserPtr->template reportError<parse_errors::Missing_String_Start>(context);
							base::template skipKey<value_type>(context);
							if JSONIFIER_LIKELY ((context.iter < context.endIter) && **context.iter == ':') {
								++context.iter;
							} else [[unlikely]] {
								context.parserPtr->template reportError<parse_errors::Missing_Colon>(context);
								base::skipToNextValue(context);
								return;
							}
							base::skipToNextValue(context);
							return;
						}
					} else {
						context.parserPtr->template reportError<parse_errors::Unexpected_String_End>(context);
						base::skipToNextValue(context);
						return;
					}
				}
			}
		}
	};

	template<typename... bases> struct parse_partial_map : public bases... {
		template<typename json_entity_type, typename... arg_types> inline static void iterateValuesImpl(arg_types&&... args) {
			json_entity_type::processIndex(internal::forward<arg_types>(args)...);
		}

		template<typename... arg_types> static constexpr void iterateValues(arg_types&&... args) {
			(( void )(args), ...);
			((iterateValuesImpl<bases>(internal::forward<arg_types>(args)...)), ...);
		}
	};

	template<parse_options options, typename value_type, typename context_type, bool minifiedOrInsideRepeated, typename index_sequence, typename... value_types>
	struct get_parse_partial_base;

	template<parse_options options, typename value_type, typename context_type, bool minifiedOrInsideRepeated, size_t... index>
	struct get_parse_partial_base<options, value_type, context_type, minifiedOrInsideRepeated, index_sequence<index...>> {
		using type = parse_partial_map<json_entity_parse_partial<options, remove_cvref_t<decltype(get<index>(core<value_type>::parseValue))>, minifiedOrInsideRepeated>...>;
	};

	template<parse_options options, typename value_type, typename context_type, bool minifiedOrInsideRepeated> using parse_partial_base_t =
		typename get_parse_partial_base<options, value_type, context_type, minifiedOrInsideRepeated, make_index_sequence<core_tuple_size<value_type>>>::type;

	template<typename value_type, typename context_type, parse_options options, auto tupleElem, bool minifiedOrInsideRepeated> struct parse_types;

	template<typename value_type, typename context_type, parse_options options, auto tupleElem, bool minifiedOrInsideRepeated> struct parse_types_partial {
		using base						  = derailleur<options, context_type>;
		inline static constexpr auto memberCount = core_tuple_size<value_type>;
	};

	template<concepts::jsonifier_object_t value_type, typename context_type, parse_options options> struct parse_impl<value_type, context_type, options, false> {
		using base = derailleur<options, context_type>;

		JSONIFIER_INLINE static void parseObjectOpening(value_type& value, context_type& context) noexcept {
			if JSONIFIER_LIKELY (context.iter < context.endIter) {
				if JSONIFIER_LIKELY (*context.iter == lBrace) {
					++context.iter;
					++context.currentObjectDepth;
					string_view_ptr wsStart = context.iter;
					JSONIFIER_SKIP_WS();
					size_t wsSize = static_cast<size_t>(context.iter - wsStart);
					parse_base_t<options, value_type, context_type, false>::iterateValues(value, context, wsStart, wsSize);
				}
				JSONIFIER_ELSE_UNLIKELY(else) {
					context.parserPtr->template reportError<parse_errors::Missing_Object_Start>(context);
					base::skipToNextValue(context);
				}
			}
			JSONIFIER_ELSE_UNLIKELY(else) {
				context.parserPtr->template reportError<parse_errors::Unexpected_String_End>(context);
				base::skipToNextValue(context);
			}
		}

		JSONIFIER_INLINE static void parseObjectEnding(context_type& context) noexcept {
			if JSONIFIER_LIKELY (context.iter < context.endIter) {
				if JSONIFIER_LIKELY (*context.iter == rBrace) {
					++context.iter;
					JSONIFIER_SKIP_WS();
					--context.currentObjectDepth;
				}
				JSONIFIER_ELSE_UNLIKELY(else) {
					base::template skipToEndOfValue<'{', '}'>(context);
				}
			}
			JSONIFIER_ELSE_UNLIKELY(else) {
				context.parserPtr->template reportError<parse_errors::Unexpected_String_End>(context);
				base::skipToNextValue(context);
			}
		}

		JSONIFIER_INLINE static void impl(value_type& value, context_type& context) noexcept {
			parseObjectOpening(value, context);
			parseObjectEnding(context);
		}
	};

	template<concepts::jsonifier_object_t value_type, typename context_type, parse_options options> struct parse_impl<value_type, context_type, options, true> {
		using base = derailleur<options, context_type>;

		JSONIFIER_INLINE static void parseObjectOpening(value_type& value, context_type& context) noexcept {
			if JSONIFIER_LIKELY (context.iter < context.endIter) {
				if JSONIFIER_LIKELY (*context.iter == lBrace) {
					++context.iter;
					++context.currentObjectDepth;
					parse_base_t<options, value_type, context_type, true>::iterateValues(value, context);
				}
				JSONIFIER_ELSE_UNLIKELY(else) {
					context.parserPtr->template reportError<parse_errors::Missing_Object_Start>(context);
					base::skipToNextValue(context);
				}
			}
			JSONIFIER_ELSE_UNLIKELY(else) {
				context.parserPtr->template reportError<parse_errors::Unexpected_String_End>(context);
				base::skipToNextValue(context);
			}
		}

		JSONIFIER_INLINE static void parseObjectEnding(context_type& context) noexcept {
			if JSONIFIER_LIKELY (context.iter < context.endIter) {
				if JSONIFIER_LIKELY (*context.iter == rBrace) {
					++context.iter;
					--context.currentObjectDepth;
				}
				JSONIFIER_ELSE_UNLIKELY(else) {
					base::template skipToEndOfValue<'{', '}'>(context);
				}
			}
			JSONIFIER_ELSE_UNLIKELY(else) {
				context.parserPtr->template reportError<parse_errors::Unexpected_String_End>(context);
				base::skipToNextValue(context);
			}
		}

		JSONIFIER_INLINE static void impl(value_type& value, context_type& context) noexcept {
			parseObjectOpening(value, context);
			parseObjectEnding(context);
		}
	};

	template<concepts::map_t value_type, typename context_type, parse_options options> struct parse_impl<value_type, context_type, options, false> {
		using base = derailleur<options, context_type>;

		JSONIFIER_INLINE static void impl(value_type& value, context_type& context) noexcept {
			static thread_local typename value_type::key_type key{};
			if JSONIFIER_LIKELY (context.iter + 1 < context.endIter) {
				if JSONIFIER_LIKELY (*context.iter == lBrace) {
					++context.iter;
					++context.currentObjectDepth;
					if JSONIFIER_LIKELY (*context.iter != rBrace) {
						const auto wsStart = context.iter;
						JSONIFIER_SKIP_WS();
						size_t wsSize{ static_cast<size_t>(context.iter - wsStart) };
						parse<options, false>::impl(key, context);

						if JSONIFIER_LIKELY ((context.iter < context.endIter) && *context.iter == colon) {
							++context.iter;
							JSONIFIER_SKIP_WS();
							parse<options, false>::impl(value[key], context);
						}
						JSONIFIER_ELSE_UNLIKELY(else) {
							context.parserPtr->template reportError<parse_errors::Missing_Colon>(context);
							base::skipToNextValue(context);
							return;
						}
						if (whitespaceTable[static_cast<uint8_t>(*(context.iter + wsSize))]) {
							return parseObjects<true>(value, context, wsStart, wsSize);
						} else {
							return parseObjects<false>(value, context, wsStart, wsSize);
						}
					} else {
						++context.iter;
						JSONIFIER_SKIP_WS();
						--context.currentObjectDepth;
					}
				}
				JSONIFIER_ELSE_UNLIKELY(else) {
					context.parserPtr->template reportError<parse_errors::Missing_Object_Start>(context);
					base::skipToNextValue(context);
					return;
				}
			}
			JSONIFIER_ELSE_UNLIKELY(else) {
				context.parserPtr->template reportError<parse_errors::Unexpected_String_End>(context);
				base::skipToNextValue(context);
				return;
			}
		}

		template<bool newLines> JSONIFIER_INLINE static void parseObjects(value_type& value, context_type& context, const auto wsStart = {}, size_t wsSize = {}) {
			static thread_local typename value_type::key_type key{};
			while
				JSONIFIER_LIKELY((context.iter < context.endIter) && *context.iter != rBrace) {
					if (*context.iter == comma) {
						++context.iter;
						JSONIFIER_SKIP_MATCHING_WS();
						parse<options, false>::impl(key, context);

						if JSONIFIER_LIKELY ((context.iter < context.endIter) && *context.iter == colon) {
							++context.iter;
							JSONIFIER_SKIP_WS();
							parse<options, false>::impl(value[key], context);
						} else {
							context.parserPtr->template reportError<parse_errors::Missing_Colon>(context);
							base::skipToNextValue(context);
							return;
						}
					} else {
						context.parserPtr->template reportError<parse_errors::Missing_Comma>(context);
						base::skipToNextValue(context);
						return;
					}
				}
			++context.iter;
			--context.currentObjectDepth;
			JSONIFIER_SKIP_WS();
		}
	};

	template<concepts::map_t value_type, typename context_type, parse_options options> struct parse_impl<value_type, context_type, options, true> {
		using base = derailleur<options, context_type>;

		JSONIFIER_INLINE static void impl(value_type& value, context_type& context) noexcept {
			static thread_local typename value_type::key_type key{};
			if JSONIFIER_LIKELY (context.iter + 1 < context.endIter) {
				if JSONIFIER_LIKELY (*context.iter == lBrace) {
					++context.iter;
					++context.currentObjectDepth;
					if JSONIFIER_LIKELY (*context.iter != rBrace) {
						parse<options, true>::impl(key, context);

						if JSONIFIER_LIKELY ((context.iter < context.endIter) && *context.iter == colon) {
							++context.iter;
							parse<options, true>::impl(value[key], context);
						}
						JSONIFIER_ELSE_UNLIKELY(else) {
							context.parserPtr->template reportError<parse_errors::Missing_Colon>(context);
							base::skipToNextValue(context);
							return;
						}

						while ((context.iter < context.endIter) && *context.iter != rBrace) {
							if (*context.iter == comma) {
								++context.iter;
								parse<options, true>::impl(key, context);

								if JSONIFIER_LIKELY ((context.iter < context.endIter) && *context.iter == colon) {
									++context.iter;
									parse<options, true>::impl(value[key], context);
								} else {
									context.parserPtr->template reportError<parse_errors::Missing_Colon>(context);
									base::skipToNextValue(context);
									return;
								}
							} else {
								context.parserPtr->template reportError<parse_errors::Missing_Comma>(context);
								base::skipToNextValue(context);
								return;
							}
						}
						++context.iter;
						--context.currentObjectDepth;
					} else {
						++context.iter;
						--context.currentObjectDepth;
					}
				}
				JSONIFIER_ELSE_UNLIKELY(else) {
					context.parserPtr->template reportError<parse_errors::Missing_Object_Start>(context);
					base::skipToNextValue(context);
					return;
				}
			}
			JSONIFIER_ELSE_UNLIKELY(else) {
				context.parserPtr->template reportError<parse_errors::Unexpected_String_End>(context);
				base::skipToNextValue(context);
				return;
			}
		}
	};

	template<typename value_type> JSONIFIER_INLINE static auto getBeginIterVec(value_type& value) {
		if constexpr (std::is_same_v<typename value_type::value_type, bool>) {
			return value.begin();
		} else {
			return value.data();
		}
	}

	template<typename value_type> JSONIFIER_INLINE static auto getEndIterVec(value_type& value) {
		if constexpr (std::is_same_v<typename value_type::value_type, bool>) {
			return value.end();
		} else {
			return value.data() + value.size();
		}
	}

	JSONIFIER_INLINE void noop() noexcept {};

	template<concepts::vector_t value_type, typename context_type, parse_options optionsNew> struct parse_impl<value_type, context_type, optionsNew, false> {
		inline static constexpr parse_options options{ optionsNew };
		using base = derailleur<options, context_type>;

		JSONIFIER_INLINE static void impl(value_type& value, context_type& context) noexcept {
			if JSONIFIER_LIKELY (context.iter + 1 < context.endIter) {
				if JSONIFIER_LIKELY (*context.iter == lBracket) {
					++context.currentArrayDepth;
					++context.iter;
					if JSONIFIER_LIKELY (*context.iter != rBracket) {
						const auto wsStart = context.iter;
						JSONIFIER_SKIP_WS();
						size_t wsSize{ static_cast<size_t>(context.iter - wsStart) };
						if (auto size = value.size(); size > 0) {
							parseObjectsWithSize<true>(value, context, size, wsStart, wsSize);
						} else {
							parseObjects<true>(value, context, wsStart, wsSize);
						}
					}
					++context.iter;
					JSONIFIER_SKIP_WS();
					--context.currentArrayDepth;
				}
				JSONIFIER_ELSE_UNLIKELY(else) {
					context.parserPtr->template reportError<parse_errors::Missing_Array_Start>(context);
					base::skipToNextValue(context);
				}
			}
			JSONIFIER_ELSE_UNLIKELY(else) {
				context.parserPtr->template reportError<parse_errors::Unexpected_String_End>(context);
				base::skipToNextValue(context);
				return;
			}
		}

		template<bool newLines>
		JSONIFIER_INLINE static void parseObjectsWithSize(value_type& value, context_type& context, size_t size, const auto wsStart = {}, size_t wsSize = {}) {
			auto newIter = getBeginIterVec(value);

			for (size_t i{}; i != size; ++i) {
				parse<options, false>::impl(newIter[static_cast<int64_t>(i)], context);

				if JSONIFIER_LIKELY (context.iter < context.endIter) {
					if JSONIFIER_LIKELY (*context.iter == comma) {
						++context.iter;
						JSONIFIER_SKIP_MATCHING_WS();
					} else {
						if JSONIFIER_LIKELY (*context.iter == rBracket) {
							return (i + 1 < size) ? value.resize(i + 1) : noop();
						} else {
							context.parserPtr->template reportError<parse_errors::Missing_Array_End>(context);
							base::skipToNextValue(context);
							return;
						}
					}
				}
				JSONIFIER_ELSE_UNLIKELY(else) {
					context.parserPtr->template reportError<parse_errors::Unexpected_String_End>(context);
					base::skipToNextValue(context);
					return;
				}
			}
			parseObjects<newLines>(value, context, wsStart, wsSize);
		}

		template<bool newLines> JSONIFIER_INLINE static void parseObjects(value_type& value, context_type& context, const auto wsStart = {}, size_t wsSize = {}) {
			parse<options, false>::impl(value.emplace_back(), context);

			while (context.iter < context.endIter && *context.iter == comma) {
				++context.iter;
				JSONIFIER_SKIP_MATCHING_WS()
				parse<options, false>::impl(value.emplace_back(), context);
			}
			if JSONIFIER_LIKELY (context.iter < context.endIter && *context.iter == rBracket) {
				return;
			} else {
				context.parserPtr->template reportError<parse_errors::Missing_Array_End>(context);
				base::skipToNextValue(context);
				return;
			}
		}
	};

	template<concepts::raw_array_t value_type, typename context_type, parse_options options> struct parse_impl<value_type, context_type, options, false> {
		using base = derailleur<options, context_type>;

		JSONIFIER_INLINE static void impl(value_type& value, context_type& context) noexcept {
			if JSONIFIER_LIKELY (context.iter + 1 < context.endIter) {
				if JSONIFIER_LIKELY (*context.iter == lBracket) {
					++context.currentArrayDepth;
					++context.iter;
					if JSONIFIER_LIKELY (*context.iter != rBracket) {
						const auto wsStart = context.iter;
						JSONIFIER_SKIP_WS();
						size_t wsSize{ static_cast<size_t>(context.iter - wsStart) };
						if (whitespaceTable[static_cast<uint8_t>(*(context.iter + wsSize))]) {
							return parseObjects<true>(value, context, wsStart, wsSize);
						} else {
							return parseObjects<false>(value, context, wsStart, wsSize);
						}
					} else {
						++context.iter;
						JSONIFIER_SKIP_WS();
						--context.currentArrayDepth;
					}
				}
				JSONIFIER_ELSE_UNLIKELY(else) {
					context.parserPtr->template reportError<parse_errors::Missing_Array_Start>(context);
					base::skipToNextValue(context);
				}
			}
			JSONIFIER_ELSE_UNLIKELY(else) {
				context.parserPtr->template reportError<parse_errors::Unexpected_String_End>(context);
				base::skipToNextValue(context);
				return;
			}
		}

		template<bool newLines> JSONIFIER_INLINE static void parseObjects(value_type& value, context_type& context, const auto wsStart = {}, size_t wsSize = {}) {
			if JSONIFIER_LIKELY (const size_t n = std::size(value); n > 0) {
				auto iterNew = std::begin(value);

				for (size_t i = 0; i < n; ++i) {
					parse<options, false>::impl(*(iterNew++), context);

					if JSONIFIER_LIKELY (context.iter < context.endIter) {
						if JSONIFIER_LIKELY (*context.iter == comma) {
							++context.iter;
							JSONIFIER_SKIP_MATCHING_WS();
						}
						JSONIFIER_ELSE_UNLIKELY(else) {
							if JSONIFIER_LIKELY (*context.iter == rBracket) {
								++context.iter;
								JSONIFIER_SKIP_WS()
								--context.currentArrayDepth;
								return;
							}
							JSONIFIER_ELSE_UNLIKELY(else) {
								context.parserPtr->template reportError<parse_errors::Missing_Array_End>(context);
								base::skipToNextValue(context);
								return;
							}
						}
					}
					JSONIFIER_ELSE_UNLIKELY(else) {
						context.parserPtr->template reportError<parse_errors::Unexpected_String_End>(context);
						base::skipToNextValue(context);
						return;
					}
				}
			}
		}
	};

	template<concepts::tuple_t value_type, typename context_type, parse_options options> struct parse_impl<value_type, context_type, options, false> {
		using base = derailleur<options, context_type>;

		JSONIFIER_INLINE static void impl(value_type& value, context_type& context) noexcept {
			static constexpr auto memberCount = tuple_size_v<value_type>;
			if JSONIFIER_LIKELY (context.iter + 1 < context.endIter) {
				if JSONIFIER_LIKELY (*context.iter == lBracket) {
					++context.iter;
					JSONIFIER_SKIP_WS()
					++context.currentArrayDepth;
					if JSONIFIER_LIKELY (*context.iter != rBracket) {
						if constexpr (memberCount > 0) {
							parse<options, false>::impl(get<0>(value), context);
							parseObjects<memberCount, 1>(value, context);
						}
					}
					++context.iter;
					JSONIFIER_SKIP_WS()
					--context.currentArrayDepth;
				}
				JSONIFIER_ELSE_UNLIKELY(else) {
					context.parserPtr->template reportError<parse_errors::Missing_Array_Start>(context);
					base::skipToNextValue(context);
					return;
				}
			}
			JSONIFIER_ELSE_UNLIKELY(else) {
				context.parserPtr->template reportError<parse_errors::Unexpected_String_End>(context);
				base::skipToNextValue(context);
				return;
			}
		}

		template<size_t memberCount, size_t index> JSONIFIER_INLINE static void parseObjects(value_type& value, context_type& context) {
			if constexpr (index < memberCount) {
				if JSONIFIER_LIKELY ((context.iter < context.endIter) && *context.iter != rBracket) {
					if JSONIFIER_LIKELY (*context.iter == comma) {
						++context.iter;
						JSONIFIER_SKIP_WS()
						parse<options, false>::impl(get<index>(value), context);
						return parseObjects<memberCount, index + 1>(value, context);
					} else {
						context.parserPtr->template reportError<parse_errors::Missing_Comma>(context);
						base::skipToNextValue(context);
						return;
					}
				} else {
					++context.iter;
				}
			} else {
				++context.iter;
			}
		}
	};

	template<concepts::vector_t value_type, typename context_type, parse_options options> struct parse_impl<value_type, context_type, options, true> {
		using base = derailleur<options, context_type>;

		JSONIFIER_INLINE static void impl(value_type& value, context_type& context) noexcept {
			if JSONIFIER_LIKELY (context.iter + 1 < context.endIter) {
				if JSONIFIER_LIKELY (*context.iter == lBracket) {
					++context.currentArrayDepth;
					++context.iter;
					if JSONIFIER_LIKELY (*context.iter != rBracket) {
						if (auto size = value.size(); size > 0) {
							parseObjectsWithSize(value, context, size);
						} else {
							parseObjects(value, context);
						}
					}
					++context.iter;
					JSONIFIER_SKIP_WS();
					--context.currentArrayDepth;
				}
				JSONIFIER_ELSE_UNLIKELY(else) {
					context.parserPtr->template reportError<parse_errors::Missing_Array_Start>(context);
					base::skipToNextValue(context);
				}
			}
			JSONIFIER_ELSE_UNLIKELY(else) {
				context.parserPtr->template reportError<parse_errors::Unexpected_String_End>(context);
				base::skipToNextValue(context);
				return;
			}
		}

		JSONIFIER_INLINE static void parseObjectsWithSize(value_type& value, context_type& context, size_t size) {
			auto newIter = getBeginIterVec(value);

			for (size_t i{}; i != size; ++i) {
				parse<options, true>::impl(newIter[static_cast<int64_t>(i)], context);

				if JSONIFIER_LIKELY (context.iter < context.endIter) {
					if JSONIFIER_LIKELY (*context.iter == comma) {
						++context.iter;
					} else {
						if JSONIFIER_LIKELY (*context.iter == rBracket) {
							return (i + 1 < size) ? value.resize(i + 1) : noop();
						} else {
							context.parserPtr->template reportError<parse_errors::Missing_Array_End>(context);
							base::skipToNextValue(context);
							return;
						}
					}
				}
				JSONIFIER_ELSE_UNLIKELY(else) {
					context.parserPtr->template reportError<parse_errors::Unexpected_String_End>(context);
					base::skipToNextValue(context);
					return;
				}
			}
			parseObjects(value, context);
		}

		JSONIFIER_INLINE static void parseObjects(value_type& value, context_type& context) {
			parse<options, true>::impl(value.emplace_back(), context);

			while (context.iter < context.endIter && *context.iter == comma) {
				++context.iter;
				parse<options, true>::impl(value.emplace_back(), context);
			}
			if JSONIFIER_LIKELY (context.iter < context.endIter && *context.iter == rBracket) {
				return;
			} else {
				context.parserPtr->template reportError<parse_errors::Missing_Array_End>(context);
				base::skipToNextValue(context);
				return;
			}
		}
	};

	template<concepts::raw_array_t value_type, typename context_type, parse_options options> struct parse_impl<value_type, context_type, options, true> {
		using base = derailleur<options, context_type>;

		JSONIFIER_INLINE static void impl(value_type& value, context_type& context) noexcept {
			if JSONIFIER_LIKELY (context.iter + 1 < context.endIter) {
				if JSONIFIER_LIKELY (*context.iter == lBracket) {
					++context.currentArrayDepth;
					++context.iter;
					if JSONIFIER_LIKELY (*context.iter != rBracket) {
						auto iterNew = std::begin(value);

						for (size_t i = 0; i < value.size(); ++i) {
							parse<options, true>::impl(*(iterNew++), context);

							if JSONIFIER_LIKELY ((context.iter < context.endIter) && *context.iter == comma) {
								++context.iter;
							}
							JSONIFIER_ELSE_UNLIKELY(else) {
								if JSONIFIER_LIKELY ((context.iter < context.endIter) && *context.iter == rBracket) {
									++context.iter;
									--context.currentArrayDepth;
									return;
								}
								JSONIFIER_ELSE_UNLIKELY(else) {
									context.parserPtr->template reportError<parse_errors::Missing_Array_End>(context);
									base::skipToNextValue(context);
									return;
								}
							}
						}
					} else {
						++context.iter;
						--context.currentArrayDepth;
					}
				}
				JSONIFIER_ELSE_UNLIKELY(else) {
					context.parserPtr->template reportError<parse_errors::Missing_Array_Start>(context);
					base::skipToNextValue(context);
				}
			}
			JSONIFIER_ELSE_UNLIKELY(else) {
				context.parserPtr->template reportError<parse_errors::Unexpected_String_End>(context);
				base::skipToNextValue(context);
				return;
			}
		}
	};

	template<concepts::tuple_t value_type, typename context_type, parse_options options> struct parse_impl<value_type, context_type, options, true> {
		using base = derailleur<options, context_type>;

		JSONIFIER_INLINE static void impl(value_type& value, context_type& context) noexcept {
			static constexpr auto memberCount = tuple_size_v<value_type>;
			if JSONIFIER_LIKELY (context.iter + 1 < context.endIter) {
				if JSONIFIER_LIKELY (*context.iter == lBracket) {
					++context.iter;
					++context.currentArrayDepth;
					if JSONIFIER_LIKELY (*context.iter != rBracket) {
						if constexpr (memberCount > 0) {
							parse<options, true>::impl(get<0>(value), context);
							parseObjects<memberCount, 1>(value, context);
						}
					}
					++context.iter;
					--context.currentArrayDepth;
				}
				JSONIFIER_ELSE_UNLIKELY(else) {
					context.parserPtr->template reportError<parse_errors::Missing_Array_Start>(context);
					base::skipToNextValue(context);
					return;
				}
			}
			JSONIFIER_ELSE_UNLIKELY(else) {
				context.parserPtr->template reportError<parse_errors::Unexpected_String_End>(context);
				base::skipToNextValue(context);
				return;
			}
		}

		template<size_t memberCount, size_t index> JSONIFIER_INLINE static void parseObjects(value_type& value, context_type& context) {
			if constexpr (index < memberCount) {
				if JSONIFIER_LIKELY ((context.iter < context.endIter) && *context.iter != rBracket) {
					if JSONIFIER_LIKELY (*context.iter == comma) {
						++context.iter;
						parse<options, true>::impl(get<index>(value), context);
						return parseObjects<memberCount, index + 1>(value, context);
					} else {
						context.parserPtr->template reportError<parse_errors::Missing_Comma>(context);
						base::skipToNextValue(context);
						return;
					}
				} else {
					++context.iter;
				}
			} else {
				++context.iter;
			}
		}
	};

	template<concepts::string_t value_type, typename context_type, parse_options options, bool minified> struct parse_impl<value_type, context_type, options, minified> {
		using base = derailleur<options, context_type>;
		JSONIFIER_INLINE static void impl(value_type& value, context_type& context) noexcept {
			base::parseString(value, context);
			if constexpr (!minified) {
				JSONIFIER_SKIP_WS();
			}
		}
	};

	template<concepts::char_t value_type, typename context_type, parse_options options, bool minified> struct parse_impl<value_type, context_type, options, minified> {
		using base = derailleur<options, context_type>;
		JSONIFIER_INLINE static void impl(value_type& value, context_type& context) noexcept {
			value = static_cast<value_type>(*(context.iter + 1));
			++context.iter;
			if constexpr (!minified) {
				JSONIFIER_SKIP_WS();
			}
		}
	};

	template<concepts::enum_t value_type, typename context_type, parse_options options, bool minified> struct parse_impl<value_type, context_type, options, minified> {
		using base = derailleur<options, context_type>;
		JSONIFIER_INLINE static void impl(value_type& value, context_type& context) noexcept {
			size_t newValue{};
			if JSONIFIER_LIKELY (parseNumber(newValue, context.iter, context.endIter)) {
				value = static_cast<value_type>(newValue);
				if constexpr (!minified) {
					JSONIFIER_SKIP_WS();
				}
				return;
			}
			JSONIFIER_ELSE_UNLIKELY(else) {
				context.parserPtr->template reportError<parse_errors::Invalid_Number_Value>(context);
				base::skipToNextValue(context);
				return;
			}
		}
	};

	template<concepts::num_t value_type, typename context_type, parse_options options, bool minified> struct parse_impl<value_type, context_type, options, minified> {
		using base = derailleur<options, context_type>;
		JSONIFIER_INLINE static void impl(value_type& value, context_type& context) noexcept {
			if JSONIFIER_LIKELY (parseNumber(value, context.iter, context.endIter)) {
				if constexpr (!minified) {
					JSONIFIER_SKIP_WS();
				}
				return;
			}
			JSONIFIER_ELSE_UNLIKELY(else) {
				context.parserPtr->template reportError<parse_errors::Invalid_Number_Value>(context);
				base::skipToNextValue(context);
				return;
			}
		}
	};

	template<concepts::bool_t value_type, typename context_type, parse_options options, bool minified> struct parse_impl<value_type, context_type, options, minified> {
		using base = derailleur<options, context_type>;
		JSONIFIER_INLINE static void impl(value_type& value, context_type& context) noexcept {
			if JSONIFIER_LIKELY ((context.iter + 3) < context.endIter && parseBool(value, context.iter)) {
				if constexpr (!minified) {
					JSONIFIER_SKIP_WS();
				}
				return;
			}
			JSONIFIER_ELSE_UNLIKELY(else) {
				context.parserPtr->template reportError<parse_errors::Invalid_Bool_Value>(context);
				base::skipToNextValue(context);
				return;
			}
		}
	};

	template<concepts::always_null_t value_type, typename context_type, parse_options options, bool minified> struct parse_impl<value_type, context_type, options, minified> {
		using base = derailleur<options, context_type>;
		JSONIFIER_INLINE static void impl(value_type&, context_type& context) noexcept {
			if JSONIFIER_LIKELY (parseNull(context.iter)) {
				if constexpr (!minified) {
					JSONIFIER_SKIP_WS();
				}
				return;
			}
			JSONIFIER_ELSE_UNLIKELY(else) {
				context.parserPtr->template reportError<parse_errors::Invalid_Null_Value>(context);
				base::skipToNextValue(context);
				return;
			}
		}
	};

	template<concepts::variant_t value_type, typename context_type, parse_options options, bool minified> struct parse_impl<value_type, context_type, options, minified> {
		using base = derailleur<options, context_type>;
		template<json_type type, typename variant_type, size_t currentIndex = 0>
		JSONIFIER_INLINE static void iterateVariantTypes(variant_type&& variant, context_type& context) noexcept {
			if constexpr (currentIndex < std::variant_size_v<remove_cvref_t<variant_type>>) {
				using element_type = remove_cvref_t<decltype(std::get<currentIndex>(std::declval<remove_cvref_t<variant_type>>()))>;
				if constexpr (concepts::jsonifier_object_t<element_type> && type == json_type::object) {
					parse<options, minified>::impl(variant.template emplace<element_type>(element_type{}), context);
				} else if constexpr ((concepts::vector_t<element_type> || concepts::raw_array_t<element_type>) && type == json_type::array) {
					parse<options, minified>::impl(variant.template emplace<element_type>(element_type{}), context);
				} else if constexpr ((concepts::string_t<element_type> || concepts::string_view_t<element_type>) && type == json_type::string) {
					parse<options, minified>::impl(variant.template emplace<element_type>(element_type{}), context);
				} else if constexpr (concepts::bool_t<element_type> && type == json_type::boolean) {
					parse<options, minified>::impl(variant.template emplace<element_type>(element_type{}), context);
				} else if constexpr ((concepts::num_t<element_type> || concepts::enum_t<element_type>) && type == json_type::number) {
					parse<options, minified>::impl(variant.template emplace<element_type>(element_type{}), context);
				} else if constexpr (concepts::always_null_t<element_type> && type == json_type::null) {
					parse<options, minified>::impl(variant.template emplace<element_type>(element_type{}), context);
				} else if constexpr (concepts::accessor_t<element_type> && type == json_type::accessor) {
					parse<options, minified>::impl(variant.template emplace<element_type>(element_type{}), context);
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

	template<concepts::optional_t value_type, typename context_type, parse_options options, bool minified> struct parse_impl<value_type, context_type, options, minified> {
		using base = derailleur<options, context_type>;
		JSONIFIER_INLINE static void impl(value_type& value, context_type& context) noexcept {
			if JSONIFIER_LIKELY ((context.iter < context.endIter) && *context.iter != n) {
				parse<options, minified>::impl(value.emplace(), context);
			} else {
				if JSONIFIER_LIKELY (parseNull(context.iter)) {
					if constexpr (!minified) {
						JSONIFIER_SKIP_WS();
					}
					return;
				}
				JSONIFIER_ELSE_UNLIKELY(else) {
					context.parserPtr->template reportError<parse_errors::Invalid_Null_Value>(context);
					base::skipToNextValue(context);
					return;
				}
			}
		}
	};

	template<concepts::shared_ptr_t value_type, typename context_type, parse_options options, bool minified> struct parse_impl<value_type, context_type, options, minified> {
		using base = derailleur<options, context_type>;
		JSONIFIER_INLINE static void impl(value_type& value, context_type& context) noexcept {
			if JSONIFIER_LIKELY ((context.iter < context.endIter) && *context.iter != n) {
				using member_type = decltype(*value);
				if JSONIFIER_UNLIKELY (!value) {
					value = std::make_shared<jsonifier::internal::remove_pointer_t<remove_cvref_t<member_type>>>();
				}
				parse<options, minified>::impl(*value, context);
			} else {
				if JSONIFIER_LIKELY (parseNull(context.iter)) {
					if constexpr (!minified) {
						JSONIFIER_SKIP_WS();
					}
					return;
				}
				JSONIFIER_ELSE_UNLIKELY(else) {
					context.parserPtr->template reportError<parse_errors::Invalid_Null_Value>(context);
					base::skipToNextValue(context);
					return;
				}
			}
		}
	};

	template<concepts::unique_ptr_t value_type, typename context_type, parse_options options, bool minified> struct parse_impl<value_type, context_type, options, minified> {
		using base = derailleur<options, context_type>;
		JSONIFIER_INLINE static void impl(value_type& value, context_type& context) noexcept {
			if JSONIFIER_LIKELY ((context.iter < context.endIter) && *context.iter != n) {
				using member_type = decltype(*value);
				if JSONIFIER_UNLIKELY (!value) {
					value = std::make_unique<jsonifier::internal::remove_pointer_t<remove_cvref_t<member_type>>>();
				}
				parse<options, minified>::impl(*value, context);
			} else {
				if JSONIFIER_LIKELY (parseNull(context.iter)) {
					if constexpr (!minified) {
						JSONIFIER_SKIP_WS();
					}
					return;
				}
				JSONIFIER_ELSE_UNLIKELY(else) {
					context.parserPtr->template reportError<parse_errors::Invalid_Null_Value>(context);
					base::skipToNextValue(context);
					return;
				}
			}
		}
	};

	template<concepts::pointer_t value_type, typename context_type, parse_options options, bool minified> struct parse_impl<value_type, context_type, options, minified> {
		using base = derailleur<options, context_type>;
		JSONIFIER_INLINE static void impl(value_type& value, context_type& context) noexcept {
			if JSONIFIER_LIKELY ((context.iter < context.endIter) && *context.iter != n) {
				if JSONIFIER_UNLIKELY (!value) {
					value = new jsonifier::internal::remove_pointer_t<value_type>{};
				}
				parse<options, minified>::impl(*value, context);
			} else {
				if JSONIFIER_LIKELY (parseNull(context.iter)) {
					if constexpr (!minified) {
						JSONIFIER_SKIP_WS();
					}
					return;
				}
				JSONIFIER_ELSE_UNLIKELY(else) {
					context.parserPtr->template reportError<parse_errors::Invalid_Null_Value>(context);
					base::skipToNextValue(context);
					return;
				}
			}
		}
	};

	template<concepts::raw_json_t value_type, typename context_type, parse_options options, bool minified> struct parse_impl<value_type, context_type, options, minified> {
		using base = derailleur<options, context_type>;

		JSONIFIER_INLINE static void impl(value_type& value, context_type& context) noexcept {
			auto newPtr = context.iter;
			base::skipToNextValue(context);
			uint64_t newSize = context.iter - newPtr;
			if JSONIFIER_LIKELY (newSize > 0) {
				string newString{};
				newString.resize(static_cast<size_t>(newSize));
				std::memcpy(newString.data(), newPtr, static_cast<size_t>(newSize));
				value = value_type{ *context.parserPtr, newString };
			}
			return;
		}
	};

	template<concepts::skip_t value_type, typename context_type, parse_options options, bool minified> struct parse_impl<value_type, context_type, options, minified> {
		using base = derailleur<options, context_type>;

		JSONIFIER_INLINE static void impl(value_type& value, context_type& context) noexcept {
			base::skipToNextValue(context);
		}
	};

	template<concepts::jsonifier_object_t value_type, typename context_type, parse_options options, bool minifiedOrInsideRepeated>
	struct parse_partial_impl<value_type, context_type, options, minifiedOrInsideRepeated> {
		using base = derailleur<options, context_type>;

		JSONIFIER_INLINE static void impl(value_type& value, context_type& context) noexcept {
			if JSONIFIER_LIKELY (context.iter + 1 < context.endIter) {
				if JSONIFIER_LIKELY (**context.iter == lBrace) {
					++context.iter;
					++context.currentObjectDepth;
					parse_partial_base_t<options, value_type, context_type, minifiedOrInsideRepeated>::iterateValues(value, context);
					if JSONIFIER_LIKELY (context.iter + 1 < context.endIter && **context.iter == rBrace) {
						++context.iter;
					}
					--context.currentObjectDepth;
				}
				JSONIFIER_ELSE_UNLIKELY(else) {
					context.parserPtr->template reportError<parse_errors::Missing_Object_Start>(context);
					base::skipToNextValue(context);
					return;
				}
			}
			JSONIFIER_ELSE_UNLIKELY(else) {
				return;
			}
		}
	};

	template<concepts::map_t value_type, typename context_type, parse_options options, bool minifiedOrInsideRepeated>
	struct parse_partial_impl<value_type, context_type, options, minifiedOrInsideRepeated> {
		using base = derailleur<options, context_type>;

		JSONIFIER_INLINE static void impl(value_type& value, context_type& context) noexcept {
			if JSONIFIER_LIKELY (context.iter + 1 < context.endIter) {
				if JSONIFIER_LIKELY (**context.iter == lBrace) {
					++context.iter;
					++context.currentObjectDepth;
					if JSONIFIER_LIKELY (**context.iter != rBrace) {
						static thread_local typename value_type::key_type key{};
						parse<options, minifiedOrInsideRepeated>::impl(key, context);

						if JSONIFIER_LIKELY ((context.iter < context.endIter) && **context.iter == colon) {
							++context.iter;
							parse<options, minifiedOrInsideRepeated>::impl(value[key], context);
						}
						JSONIFIER_ELSE_UNLIKELY(else) {
							context.parserPtr->template reportError<parse_errors::Missing_Colon>(context);
							base::skipToNextValue(context);
							return;
						}

						while ((context.iter < context.endIter) && **context.iter != rBrace) {
							if JSONIFIER_LIKELY (**context.iter == comma) {
								++context.iter;
								parse<options, minifiedOrInsideRepeated>::impl(key, context);

								if JSONIFIER_LIKELY ((context.iter < context.endIter) && **context.iter == colon) {
									++context.iter;
									parse<options, minifiedOrInsideRepeated>::impl(value[key], context);
								} else {
									context.parserPtr->template reportError<parse_errors::Missing_Colon>(context);
									base::skipToNextValue(context);
									return;
								}
							} else {
								context.parserPtr->template reportError<parse_errors::Missing_Comma>(context);
								base::skipToNextValue(context);
								return;
							}
						}
						++context.iter;
						--context.currentObjectDepth;
					} else {
						++context.iter;
						--context.currentObjectDepth;
					}
				}
				JSONIFIER_ELSE_UNLIKELY(else) {
					context.parserPtr->template reportError<parse_errors::Missing_Object_Start>(context);
					base::skipToNextValue(context);
					return;
				}
			}
			JSONIFIER_ELSE_UNLIKELY(else) {
				return;
			}
		}
	};

	template<concepts::vector_t value_type, typename context_type, parse_options options, bool minifiedOrInsideRepeated>
	struct parse_partial_impl<value_type, context_type, options, minifiedOrInsideRepeated> {
		using base = derailleur<options, context_type>;

		JSONIFIER_INLINE static void impl(value_type& value, context_type& context) noexcept {
			if JSONIFIER_LIKELY (context.iter + 1 < context.endIter) {
				if JSONIFIER_LIKELY ((context.iter < context.endIter) && **context.iter == lBracket) {
					++context.currentArrayDepth;
					++context.iter;
					if JSONIFIER_LIKELY ((context.iter < context.endIter) && **context.iter != rBracket) {
						if JSONIFIER_LIKELY (const size_t size = value.size(); size > 0) {
							auto iterNew = value.begin();

							for (size_t i = 0; i < size; ++i, ++iterNew) {
								parse<options, minifiedOrInsideRepeated>::impl(*(iterNew), context);

								if JSONIFIER_LIKELY (**context.iter == rBracket) {
									++context.iter;
									return (value.size() == (i) + 1) ? noop() : value.resize((i) + 1);
								} else {
									++context.iter;
								}
							}
						}

						while (context.iter < context.endIter) {
							parse<options, minifiedOrInsideRepeated>::impl(value.emplace_back(), context);
							if JSONIFIER_LIKELY (**context.iter == rBracket) {
								++context.iter;
								--context.currentArrayDepth;
								return;
							} else {
								++context.iter;
							}
						}
					} else {
						++context.iter;
						--context.currentArrayDepth;
					}
				}
				JSONIFIER_ELSE_UNLIKELY(else) {
					context.parserPtr->template reportError<parse_errors::Missing_Array_Start>(context);
					base::skipToNextValue(context);
				}
			}
			JSONIFIER_ELSE_UNLIKELY(else) {
				return;
			}
		}
	};

	template<concepts::raw_array_t value_type, typename context_type, parse_options options, bool minifiedOrInsideRepeated>
	struct parse_partial_impl<value_type, context_type, options, minifiedOrInsideRepeated> {
		using base = derailleur<options, context_type>;

		JSONIFIER_INLINE static void impl(value_type& value, context_type& context) noexcept {
			if JSONIFIER_LIKELY (context.iter + 1 < context.endIter) {
				if JSONIFIER_LIKELY (**context.iter == lBracket) {
					++context.currentArrayDepth;
					++context.iter;
					if JSONIFIER_LIKELY (**context.iter != rBracket) {
						auto iterNew = std::begin(value);

						for (size_t i = 0; i < value.size(); ++i) {
							parse<options, minifiedOrInsideRepeated>::impl(*(iterNew++), context);

							if JSONIFIER_LIKELY ((context.iter < context.endIter) && **context.iter == comma) {
								++context.iter;
							}
							JSONIFIER_ELSE_UNLIKELY(else) {
								if JSONIFIER_LIKELY ((context.iter < context.endIter) && **context.iter == rBracket) {
									++context.iter;
									--context.currentArrayDepth;
									return;
								}
								JSONIFIER_ELSE_UNLIKELY(else) {
									return;
								}
							}
						}
					} else {
						++context.iter;
						--context.currentArrayDepth;
					}
				}
				JSONIFIER_ELSE_UNLIKELY(else) {
					context.parserPtr->template reportError<parse_errors::Missing_Array_Start>(context);
					base::skipToNextValue(context);
				}
			}
			JSONIFIER_ELSE_UNLIKELY(else) {
				return;
			}
		}
	};

	template<concepts::tuple_t value_type, typename context_type, parse_options options, bool minifiedOrInsideRepeated>
	struct parse_partial_impl<value_type, context_type, options, minifiedOrInsideRepeated> {
		using base = derailleur<options, context_type>;

		JSONIFIER_INLINE static void impl(value_type& value, context_type& context) noexcept {
			static constexpr auto memberCount = tuple_size_v<value_type>;
			if JSONIFIER_LIKELY (context.iter + 1 < context.endIter) {
				if JSONIFIER_LIKELY (**context.iter == lBracket) {
					++context.iter;
					++context.currentArrayDepth;
					if JSONIFIER_LIKELY (**context.iter != rBracket) {
						if constexpr (memberCount > 0) {
							parse<options, minifiedOrInsideRepeated>::impl(get<0>(value), context);
							parseObjects<memberCount, 1>(value, context);
						}
					}
					++context.iter;
					--context.currentArrayDepth;
				}
				JSONIFIER_ELSE_UNLIKELY(else) {
					context.parserPtr->template reportError<parse_errors::Missing_Array_Start>(context);
					base::skipToNextValue(context);
					return;
				}
			}
			JSONIFIER_ELSE_UNLIKELY(else) {
				return;
			}
		}

		template<size_t memberCount, size_t index> JSONIFIER_INLINE static void parseObjects(value_type& value, context_type& context) {
			if constexpr (index < memberCount) {
				if JSONIFIER_LIKELY ((context.iter < context.endIter) && **context.iter != rBracket) {
					if JSONIFIER_LIKELY (**context.iter == comma) {
						++context.iter;
						parse<options, minifiedOrInsideRepeated>::impl(get<index>(value), context);
						return parseObjects<memberCount, index + 1>(value, context);
					} else {
						context.parserPtr->template reportError<parse_errors::Missing_Comma>(context);
						base::skipToNextValue(context);
						return;
					}
				} else {
					++context.iter;
				}
			} else {
				++context.iter;
			}
		}
	};

	template<concepts::string_t value_type, typename context_type, parse_options options, bool minifiedOrInsideRepeated>
	struct parse_partial_impl<value_type, context_type, options, minifiedOrInsideRepeated> {
		using base = derailleur<options, context_type>;

		JSONIFIER_INLINE static void impl(value_type& value, context_type& context) noexcept {
			base::parseString(value, context);
		}
	};

	template<concepts::char_t value_type, typename context_type, parse_options options, bool minifiedOrInsideRepeated>
	struct parse_partial_impl<value_type, context_type, options, minifiedOrInsideRepeated> {
		using base = derailleur<options, context_type>;

		JSONIFIER_INLINE static void impl(value_type& value, context_type& context) noexcept {
			value = static_cast<value_type>(*(*context.iter + 1));
			++context.iter;
		}
	};

	template<concepts::enum_t value_type, typename context_type, parse_options options, bool minifiedOrInsideRepeated>
	struct parse_partial_impl<value_type, context_type, options, minifiedOrInsideRepeated> {
		using base = derailleur<options, context_type>;

		JSONIFIER_INLINE static void impl(value_type& value, context_type& context) noexcept {
			size_t newValue{};
			if JSONIFIER_LIKELY (parseNumber(newValue, context.iter, context.endIter)) {
				value = static_cast<value_type>(newValue);
				return;
			}
			JSONIFIER_ELSE_UNLIKELY(else) {
				context.parserPtr->template reportError<parse_errors::Invalid_Number_Value>(context);
				base::skipToNextValue(context);
				return;
			}
		}
	};

	template<concepts::num_t value_type, typename context_type, parse_options options, bool minifiedOrInsideRepeated>
	struct parse_partial_impl<value_type, context_type, options, minifiedOrInsideRepeated> {
		using base = derailleur<options, context_type>;

		JSONIFIER_INLINE static void impl(value_type& value, context_type& context) noexcept {
			if JSONIFIER_LIKELY (parseNumber(value, *context.iter, *context.endIter)) {
				++context.iter;
				return;
			}
			JSONIFIER_ELSE_UNLIKELY(else) {
				context.parserPtr->template reportError<parse_errors::Invalid_Number_Value>(context);
				base::skipToNextValue(context);
				return;
			}
		}
	};

	template<concepts::bool_t value_type, typename context_type, parse_options options, bool minifiedOrInsideRepeated>
	struct parse_partial_impl<value_type, context_type, options, minifiedOrInsideRepeated> {
		using base = derailleur<options, context_type>;

		JSONIFIER_INLINE static void impl(value_type& value, context_type& context) noexcept {
			auto newPtr = *context.iter;
			if JSONIFIER_LIKELY (parseBool(value, newPtr)) {
				++context.iter;
				return;
			}
			JSONIFIER_ELSE_UNLIKELY(else) {
				context.parserPtr->template reportError<parse_errors::Invalid_Bool_Value>(context);
				base::skipToNextValue(context);
				return;
			}
		}
	};

	template<concepts::always_null_t value_type, typename context_type, parse_options options, bool minifiedOrInsideRepeated>
	struct parse_partial_impl<value_type, context_type, options, minifiedOrInsideRepeated> {
		using base = derailleur<options, context_type>;

		JSONIFIER_INLINE static void impl(value_type& value, context_type& context) noexcept {
			auto newPtr = *context.iter;
			if JSONIFIER_LIKELY (parseNull(value, newPtr)) {
				++context.iter;
				return;
			}
			JSONIFIER_ELSE_UNLIKELY(else) {
				context.parserPtr->template reportError<parse_errors::Invalid_Null_Value>(context);
				base::skipToNextValue(context);
				return;
			}
		}
	};

	template<concepts::variant_t value_type, typename context_type, parse_options options, bool minifiedOrInsideRepeated>
	struct parse_partial_impl<value_type, context_type, options, minifiedOrInsideRepeated> {
		using base = derailleur<options, context_type>;

		template<json_type type, typename variant_type, size_t currentIndex = 0>
		JSONIFIER_INLINE static void iterateVariantTypes(variant_type&& variant, context_type& context) noexcept {
			if constexpr (currentIndex < std::variant_size_v<remove_cvref_t<variant_type>>) {
				using element_type = remove_cvref_t<decltype(std::get<currentIndex>(std::declval<remove_cvref_t<variant_type>>()))>;
				if constexpr (concepts::jsonifier_object_t<element_type> && type == json_type::object) {
					parse<options, minifiedOrInsideRepeated>::impl(variant.template emplace<element_type>(element_type{}), context);
				} else if constexpr ((concepts::vector_t<element_type> || concepts::raw_array_t<element_type>) && type == json_type::array) {
					parse<options, minifiedOrInsideRepeated>::impl(variant.template emplace<element_type>(element_type{}), context);
				} else if constexpr ((concepts::string_t<element_type> || concepts::string_view_t<element_type>) && type == json_type::string) {
					parse<options, minifiedOrInsideRepeated>::impl(variant.template emplace<element_type>(element_type{}), context);
				} else if constexpr (concepts::bool_t<element_type> && type == json_type::boolean) {
					parse<options, minifiedOrInsideRepeated>::impl(variant.template emplace<element_type>(element_type{}), context);
				} else if constexpr ((concepts::num_t<element_type> || concepts::enum_t<element_type>) && type == json_type::number) {
					parse<options, minifiedOrInsideRepeated>::impl(variant.template emplace<element_type>(element_type{}), context);
				} else if constexpr (concepts::always_null_t<element_type> && type == json_type::null) {
					parse<options, minifiedOrInsideRepeated>::impl(variant.template emplace<element_type>(element_type{}), context);
				} else if constexpr (concepts::accessor_t<element_type> && type == json_type::accessor) {
					parse<options, minifiedOrInsideRepeated>::impl(variant.template emplace<element_type>(element_type{}), context);
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

	template<concepts::optional_t value_type, typename context_type, parse_options options, bool minifiedOrInsideRepeated>
	struct parse_partial_impl<value_type, context_type, options, minifiedOrInsideRepeated> {
		using base = derailleur<options, context_type>;

		JSONIFIER_INLINE static void impl(value_type& value, context_type& context) noexcept {
			if JSONIFIER_LIKELY ((context.iter < context.endIter) && **context.iter != n) {
				parse<options, minifiedOrInsideRepeated>::impl(value.emplace(), context);
			} else {
				if JSONIFIER_LIKELY (parseNull(*context.iter)) {
					++context.iter;
					return;
				}
				JSONIFIER_ELSE_UNLIKELY(else) {
					context.parserPtr->template reportError<parse_errors::Invalid_Null_Value>(context);
					base::skipToNextValue(context);
					return;
				}
			}
		}
	};

	template<concepts::shared_ptr_t value_type, typename context_type, parse_options options, bool minifiedOrInsideRepeated>
	struct parse_partial_impl<value_type, context_type, options, minifiedOrInsideRepeated> {
		using base = derailleur<options, context_type>;

		JSONIFIER_INLINE static void impl(value_type& value, context_type& context) noexcept {
			if JSONIFIER_LIKELY ((context.iter < context.endIter) && **context.iter != n) {
				using member_type = decltype(*value);
				if JSONIFIER_UNLIKELY (!value) {
					value = std::make_shared<jsonifier::internal::remove_pointer_t<remove_cvref_t<member_type>>>();
				}
				parse<options, minifiedOrInsideRepeated>::impl(*value, context);
			} else {
				if JSONIFIER_LIKELY (parseNull(*context.iter)) {
					++context.iter;
					return;
				}
				JSONIFIER_ELSE_UNLIKELY(else) {
					context.parserPtr->template reportError<parse_errors::Invalid_Null_Value>(context);
					base::skipToNextValue(context);
					return;
				}
			}
		}
	};

	template<concepts::unique_ptr_t value_type, typename context_type, parse_options options, bool minifiedOrInsideRepeated>
	struct parse_partial_impl<value_type, context_type, options, minifiedOrInsideRepeated> {
		using base = derailleur<options, context_type>;

		JSONIFIER_INLINE static void impl(value_type& value, context_type& context) noexcept {
			if JSONIFIER_LIKELY ((context.iter < context.endIter) && **context.iter != n) {
				using member_type = decltype(*value);
				if JSONIFIER_UNLIKELY (!value) {
					value = std::make_unique<jsonifier::internal::remove_pointer_t<remove_cvref_t<member_type>>>();
				}
				parse<options, minifiedOrInsideRepeated>::impl(*value, context);
			} else {
				if JSONIFIER_LIKELY (parseNull(*context.iter)) {
					++context.iter;
					return;
				}
				JSONIFIER_ELSE_UNLIKELY(else) {
					context.parserPtr->template reportError<parse_errors::Invalid_Null_Value>(context);
					base::skipToNextValue(context);
					return;
				}
			}
		}
	};

	template<concepts::pointer_t value_type, typename context_type, parse_options options, bool minifiedOrInsideRepeated>
	struct parse_partial_impl<value_type, context_type, options, minifiedOrInsideRepeated> {
		using base = derailleur<options, context_type>;

		JSONIFIER_INLINE static void impl(value_type& value, context_type& context) noexcept {
			if JSONIFIER_LIKELY ((context.iter < context.endIter) && *context.iter != n) {
				if JSONIFIER_UNLIKELY (!value) {
					value = new jsonifier::internal::remove_pointer_t<value_type>{};
				}
				parse<options, minifiedOrInsideRepeated>::impl(*value, context);
			} else {
				if JSONIFIER_LIKELY (parseNull(*context.iter)) {
					++context.iter;
					return;
				}
				JSONIFIER_ELSE_UNLIKELY(else) {
					context.parserPtr->template reportError<parse_errors::Invalid_Null_Value>(context);
					base::skipToNextValue(context);
					return;
				}
			}
		}
	};

	template<concepts::raw_json_t value_type, typename context_type, parse_options options, bool minifiedOrInsideRepeated>
	struct parse_partial_impl<value_type, context_type, options, minifiedOrInsideRepeated> {
		using base = derailleur<options, context_type>;

		JSONIFIER_INLINE static void impl(value_type& value, context_type& context) noexcept {
			auto newPtr = *context.iter;
			base::skipToNextValue(context);
			int64_t newSize = *context.iter - newPtr;
			if JSONIFIER_LIKELY (newSize > 0) {
				string newString{};
				newString.resize(static_cast<size_t>(newSize));
				std::memcpy(newString.data(), newPtr, static_cast<size_t>(newSize));
				value = value_type{ *context.parserPtr, newString };
			}
			return;
		}
	};

	template<concepts::skip_t value_type, typename context_type, parse_options options, bool minifiedOrInsideRepeated>
	struct parse_partial_impl<value_type, context_type, options, minifiedOrInsideRepeated> {
		using base = derailleur<options, context_type>;

		JSONIFIER_INLINE static void impl(value_type& value, context_type& context) noexcept {
			base::skipToNextValue(context);
		}
	};
}