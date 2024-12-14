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

#include <jsonifier/NumberUtils.hpp>
#include <jsonifier/StringUtils.hpp>
#include <jsonifier/Parser.hpp>

#include <memory>

namespace jsonifier_internal {

#define JSONIFIER_SKIP_KEY_VALUE(RETURN) \
	base::template skipKey<value_type>(context); \
	++context.iter; \
	if constexpr (!options.minified) { \
		JSONIFIER_SKIP_WS(); \
	} \
	if JSONIFIER_LIKELY ((context.iter < context.endIter) && *context.iter == ':') { \
		++context.iter; \
		if constexpr (!options.minified) { \
			JSONIFIER_SKIP_WS(); \
		} \
	} \
	JSONIFIER_ELSE_UNLIKELY(else) { \
		context.parserPtr->template reportError<parse_errors::Missing_Colon>(context); \
		base::template skipToNextValue<value_type>(context); \
		return RETURN; \
	} \
	base::template skipToNextValue<value_type>(context);

#define JSONIFIER_SKIP_KEY_VALUE_KEY_STARTED(RETURN) \
	base::template skipKeyStarted<value_type>(context); \
	++context.iter; \
	if constexpr (!options.minified) { \
		JSONIFIER_SKIP_WS(); \
	} \
	if JSONIFIER_LIKELY ((context.iter < context.endIter) && *context.iter == ':') { \
		++context.iter; \
		if constexpr (!options.minified) { \
			JSONIFIER_SKIP_WS(); \
		} \
	} \
	JSONIFIER_ELSE_UNLIKELY(else) { \
		context.parserPtr->template reportError<parse_errors::Missing_Colon>(context); \
		base::template skipToNextValue<value_type>(context); \
		return RETURN; \
	} \
	base::template skipToNextValue<value_type>(context);

	static constexpr auto falseV{ "false" };
	static constexpr auto trueV{ "true" };
	static constexpr auto nullV{ "null" };
	static constexpr char newline{ '\n' };
	static constexpr char lBracket{ '[' };
	static constexpr char rBracket{ ']' };
	static constexpr char lBrace{ '{' };
	static constexpr char rBrace{ '}' };
	static constexpr char colon{ ':' };
	static constexpr char comma{ ',' };
	static constexpr char quote{ '"' };
	static constexpr char n{ 'n' };

	template<typename class_type_new, typename parse_context_type> struct json_entity_parse_base {
		JSONIFIER_ALWAYS_INLINE virtual bool processIndex(class_type_new& value, parse_context_type& context) = 0;
		JSONIFIER_ALWAYS_INLINE virtual ~json_entity_parse_base() noexcept										  = default;
	};

	template<jsonifier::parse_options options, jsonifier::string_view nameNew, jsonifier::json_type typeNew, auto ptrNew, typename parse_context_type, typename buffer_type,
		bool minifiedOrInsideRepeated>
	struct json_entity_parse_final : public json_entity_parse_base<jsonifier_internal::remove_member_pointer_t<decltype(ptrNew)>, parse_context_type> {
		using value_type = jsonifier_internal::remove_member_pointer_t<decltype(ptrNew)>;
		static constexpr jsonifier::string_view name{ nameNew };
		static constexpr jsonifier::json_type type{ typeNew };
		static constexpr auto memberPtr{ ptrNew };
		JSONIFIER_ALWAYS_INLINE virtual bool processIndex(value_type& value, parse_context_type& context) {
			static constexpr auto key			= name;
			static constexpr auto stringLiteral = stringLiteralFromView<key.size()>(key);
			static constexpr auto keySize		= key.size();
			static constexpr auto keySizeNew	= keySize + 1;

			if JSONIFIER_LIKELY (((context.iter + keySize) < context.endIter) && string_literal_comparitor<decltype(stringLiteral), stringLiteral>::impl(context.iter)) {
				context.iter += keySizeNew;
				JSONIFIER_SKIP_WS();
				if JSONIFIER_LIKELY ((context.iter < context.endIter) && *context.iter == colon) {
					++context.iter;
					JSONIFIER_SKIP_WS();
					if constexpr (jsonifier::concepts::has_excluded_keys<value_type>) {
						auto& keys = value.jsonifierExcludedKeys;
						if JSONIFIER_LIKELY (keys.find(static_cast<typename std::remove_cvref_t<decltype(keys)>::key_type>(key)) != keys.end()) {
							derailleur<options, parse_context_type>::template skipToNextValue<value_type>(context);
							return true;
						}
					}
					using member_type = jsonifier_internal::remove_class_pointer_t<decltype(ptrNew)>;
					parse<getJsonType<member_type>(), options, minifiedOrInsideRepeated>::template impl<buffer_type>(value.*memberPtr, context);
					return true;
				}
				JSONIFIER_ELSE_UNLIKELY(else) {
					context.parserPtr->template reportError<parse_errors::Missing_Colon>(context);
					derailleur<options, parse_context_type>::template skipToNextValue<value_type>(context);
				}
			}
			return false;
		};
		constexpr json_entity_parse_final() noexcept = default;
	};

	template<jsonifier::parse_options options, jsonifier::string_view nameNew, jsonifier::json_type typeNew, auto ptrNew, typename parse_context_type, typename buffer_type,
		bool minifiedOrInsideRepeated>
	struct json_entity_parse_final_partial : public json_entity_parse_base<jsonifier_internal::remove_member_pointer_t<decltype(ptrNew)>, parse_context_type> {
		using value_type = jsonifier_internal::remove_member_pointer_t<decltype(ptrNew)>;
		static constexpr jsonifier::string_view name{ nameNew };
		static constexpr jsonifier::json_type type{ typeNew };
		static constexpr auto memberPtr{ ptrNew };
		JSONIFIER_ALWAYS_INLINE virtual bool processIndex(value_type& value, parse_context_type& context) {
			static constexpr auto key			= name;
			static constexpr auto stringLiteral = stringLiteralFromView<key.size()>(key);
			static constexpr auto keySize		= key.size() + 1;
			if JSONIFIER_LIKELY (((context.iter + 1) < context.endIter) && (*((*context.iter) + keySize)) == quote &&
				string_literal_comparitor<decltype(stringLiteral), stringLiteral>::impl((*context.iter) + 1)) {
				++context.iter;
				if JSONIFIER_LIKELY ((context.iter < context.endIter) && **context.iter == colon) {
					++context.iter;
					if constexpr (jsonifier::concepts::has_excluded_keys<value_type>) {
						auto& keys = value.jsonifierExcludedKeys;
						if JSONIFIER_LIKELY (keys.find(static_cast<typename std::remove_cvref_t<decltype(keys)>::key_type>(key)) != keys.end()) {
							derailleur<options, parse_context_type>::template skipToNextValue<value_type>(context);
							return false;
						}
					}
					using member_type = jsonifier_internal::remove_class_pointer_t<decltype(ptrNew)>;
					parse<getJsonType<member_type>(), options, minifiedOrInsideRepeated>::template impl<buffer_type>(value.*memberPtr, context);
					return true;
				}
				JSONIFIER_ELSE_UNLIKELY(else) {
					context.parserPtr->template reportError<parse_errors::Missing_Colon>(context);
					derailleur<options, parse_context_type>::template skipToNextValue<value_type>(context);
				}
			}
			return false;
		}
		constexpr json_entity_parse_final_partial() noexcept = default;
	};

	template<typename class_type_new, typename parse_context_type, typename buffer_type, typename... bases> struct json_parse_map : public bases... {
		template<typename current_type, typename... rest_types> constexpr auto generateBasePtrs(size_t currentIndex,
			std::array<jsonifier_internal::json_entity_parse_base<class_type_new, parse_context_type>*, sizeof...(bases)> arrayOfPtrsNew = {}) {
			if constexpr (sizeof...(rest_types) >= 1) {
				arrayOfPtrsNew[currentIndex] = static_cast<current_type*>(this);
				++currentIndex;
				return generateBasePtrs<rest_types...>(currentIndex, arrayOfPtrsNew);
			} else {
				arrayOfPtrsNew[currentIndex] = static_cast<current_type*>(this);
				++currentIndex;
				return arrayOfPtrsNew;
			}
		}
		std::array<jsonifier_internal::json_entity_parse_base<class_type_new, parse_context_type>*, sizeof...(bases)> arrayOfPtrs{ generateBasePtrs<bases...>(0) };
	};

	template<template<auto, auto, auto, auto, typename, typename, bool> typename json_entity_parse_type, jsonifier::parse_options options, size_t index, typename class_type,
		typename parse_context_type, typename buffer_type, auto tuple, bool minifiedOrInsideRepeated>
	struct construct_json_entity_parse_final {
		static constexpr auto originalTupleValue{ jsonifier_internal::get<index>(tuple) };
		using type =
			json_entity_parse_type<options, originalTupleValue.name, originalTupleValue.type, originalTupleValue.memberPtr, parse_context_type, buffer_type, minifiedOrInsideRepeated>;
	};

	template<template<auto, auto, auto, auto, typename, typename, bool> typename json_entity_parse_type, jsonifier::parse_options options, size_t index, typename class_type,
		typename parse_context_type, typename buffer_type, auto tuple, bool minifiedOrInsideRepeated>
	using construct_json_entity_parse_final_t =
		construct_json_entity_parse_final<json_entity_parse_type, options, index, class_type, parse_context_type, buffer_type, tuple, minifiedOrInsideRepeated>::type;

	template<template<auto, auto, auto, auto, typename, typename, bool> typename json_entity_parse_type, jsonifier::parse_options options, typename class_type,
		typename parse_context_type, typename buffer_type, auto tuple, bool minifiedOrInsideRepeated, typename index_sequence>
	struct get_json_parse_map;

	template<template<auto, auto, auto, auto, typename, typename, bool> typename json_entity_parse_type, jsonifier::parse_options options, typename class_type,
		typename parse_context_type, typename buffer_type, auto tuple, bool minifiedOrInsideRepeated, size_t... I>
	struct get_json_parse_map<json_entity_parse_type, options, class_type, parse_context_type, buffer_type, tuple, minifiedOrInsideRepeated, std::index_sequence<I...>> {
		using type = json_parse_map<class_type, parse_context_type, buffer_type,
			construct_json_entity_parse_final_t<json_entity_parse_type, options, I, class_type, parse_context_type, buffer_type, tuple, minifiedOrInsideRepeated>...>;
	};

	template<template<auto, auto, auto, auto, typename, typename, bool> typename json_entity_parse_type, jsonifier::parse_options options, typename class_type,
		typename parse_context_type, typename buffer_type, auto tuple, bool minifiedOrInsideRepeated>
	using json_parse_map_t = typename get_json_parse_map<json_entity_parse_type, options, class_type, parse_context_type, buffer_type, tuple, minifiedOrInsideRepeated,
		jsonifier_internal::tag_range<jsonifier_internal::tuple_size_v<typename jsonifier_internal::core_tuple_type<class_type>::core_type>>>::type;

	template<typename parse_context_type, typename buffer_type, typename value_type, jsonifier::parse_options options, bool... values> struct index_processor_parse;

	template<jsonifier::parse_options options, auto tupleElem, typename value_type, typename context_type>
	struct parse_index_processor_impl {
		JSONIFIER_ALWAYS_INLINE static void impl(const value_type& value,context_type& context) {
		}
	};

	template<jsonifier::parse_options options, bool minified, typename value_type, typename context_type, typename... value_holder_types>
	struct parser_core;

	template<jsonifier::parse_options options, bool minified, typename value_type, typename context_type, typename value_holder_type,
		typename... value_holder_types>
	struct parser_core<options, minified, value_type, context_type, value_holder_type, value_holder_types...> {
		JSONIFIER_INLINE static void processIndices(const value_type& value,context_type& context) {
			parse_index_processor_impl<options, value_holder_type::value, value_type, context_type>::impl(value, context);
			parser_core<options, minified, value_type, context_type, value_holder_types...>::processIndices(value, context);
		}
	};

	template<jsonifier::parse_options options, bool minified, typename value_type, typename context_type, typename value_holder_type>
	struct parser_core<options, minified, value_type, context_type, value_holder_type> {
		JSONIFIER_ALWAYS_INLINE static void processIndices(const value_type& value,context_type& context) {
			parse_index_processor_impl<options, value_holder_type::value, value_type, context_type>::impl(value, context);
		}
	};

	template<size_t currentIndex, typename value_type, auto valueNew> struct value_holder {
		static constexpr auto value{ valueNew };
	};

	template<jsonifier::parse_options options, bool minified, typename value_type, typename context_type, typename index_sequence, auto tuple>
	struct get_parser_core;

	template<jsonifier::parse_options options, bool minified, typename value_type, typename context_type, size_t... I, auto tuple>
	struct get_parser_core<options, minified, value_type, context_type, std::index_sequence<I...>, tuple> {
		using type = parser_core<options, minified, value_type, context_type, value_holder<I, value_type, jsonifier_internal::get<I>(tuple)>...>;
	};

	template<jsonifier::parse_options options, bool minified, typename value_type, typename context_type, auto value> using parser_core_t =
		typename get_parser_core<options, minified, value_type, context_type, jsonifier_internal::tag_range<jsonifier_internal::tuple_size_v<decltype(value)>>,
			value>::type;

	template<typename parse_context_type, typename buffer_type, typename value_type, jsonifier::parse_options options>
	struct index_processor_parse<parse_context_type, buffer_type, value_type, options, false> : derailleur<options, parse_context_type> {
		using base						  = derailleur<options, parse_context_type>;
		static constexpr auto memberCount = tuple_size_v<typename core_tuple_type<value_type>::core_type>;
		inline static thread_local array<bool, memberCount> antiHashStates{ [] {
			array<bool, memberCount> returnValues{};
			returnValues.fill(true);
			return returnValues;
		}() };

		template<bool haveWeStarted = false> JSONIFIER_ALWAYS_INLINE static auto processIndexLambda(value_type& value, parse_context_type& context, string_view_ptr& wsStart,
			size_t& wsSize, size_t index = 0) noexcept {
			( void )value, ( void )context, ( void )wsStart, ( void )wsSize;
			static constexpr bool newLines{ true };
			if constexpr (memberCount > 0) {
				if (index < memberCount) {
					if JSONIFIER_LIKELY (context.iter < context.endIter) {
						if (*context.iter != rBrace) {
							if (haveWeStarted || index > 0) {
								if JSONIFIER_LIKELY (*context.iter == comma) {
									++context.iter;
									JSONIFIER_SKIP_MATCHING_WS();
								}
								JSONIFIER_ELSE_UNLIKELY(else) {
									context.parserPtr->template reportError<parse_errors::Missing_Comma>(context);
									base::template skipToNextValue<value_type>(context);
									return;
								}
							}
							static constexpr json_parse_map_t<json_entity_parse_final, options, value_type, parse_context_type, buffer_type, jsonifier::concepts::coreV<value_type>,
								false>
								jsonMapAntiHash{};
							static constexpr json_parse_map_t<json_entity_parse_final, options, value_type, parse_context_type, buffer_type,
								jsonifier_internal::core_tuple_type<value_type>::coreTupleV, false>
								jsonMap{};
							if JSONIFIER_LIKELY ((context.iter < context.endIter) && *context.iter == quote) {
								++context.iter;
								if constexpr (options.knownOrder) {
									if (antiHashStates[index]) {
										if (jsonMapAntiHash.arrayOfPtrs[index]->processIndex(value, context)) {
											return processIndexLambda<true>(value, context, wsStart, wsSize, index + 1);
										} else {
											if JSONIFIER_LIKELY (auto indexNew =
																	 hash_map<value_type, std::remove_cvref_t<decltype(context.iter)>>::findIndex(context.iter, context.endIter);
												indexNew < memberCount) {
												if JSONIFIER_LIKELY (jsonMap.arrayOfPtrs[indexNew]->processIndex(value, context)) {
													return processIndexLambda<true>(value, context, wsStart, wsSize, index + 1);
												}
											}
											antiHashStates[index] = false;
										}
									}
								}
								if JSONIFIER_LIKELY (auto indexNew = hash_map<value_type, std::remove_cvref_t<decltype(context.iter)>>::findIndex(context.iter, context.endIter);
									indexNew < memberCount) {
									if JSONIFIER_LIKELY (jsonMap.arrayOfPtrs[indexNew]->processIndex(value, context)) {
										return processIndexLambda<true>(value, context, wsStart, wsSize, index + 1);
									}
								}
								base::template skipKeyStarted<value_type>(context);
								++context.iter;
								if constexpr (!options.minified) {
									while ((context.iter < context.endIter) && whitespaceTable[static_cast<uint8_t>(*context.iter)]) {
										++context.iter;
									};
								}
								if ((context.iter < context.endIter) && *context.iter == ':') [[likely]] {
									++context.iter;
									if constexpr (!options.minified) {
										while ((context.iter < context.endIter) && whitespaceTable[static_cast<uint8_t>(*context.iter)]) {
											++context.iter;
										};
									}
								} else [[unlikely]] {
									context.parserPtr->template reportError<parse_errors::Missing_Colon>(context);
									base::template skipToNextValue<value_type>(context);
									return;
								}
								base::template skipToNextValue<value_type>(context);
								;
								return processIndexLambda<true>(value, context, wsStart, wsSize, index);
							}
							JSONIFIER_ELSE_UNLIKELY(else) {
								context.parserPtr->template reportError<parse_errors::Missing_String_Start>(context);
								JSONIFIER_SKIP_KEY_VALUE();
								return;
							}
						} else {
							return;
						}
					}
					JSONIFIER_ELSE_UNLIKELY(else) {
						context.parserPtr->template reportError<parse_errors::Unexpected_String_End>(context);
						base::template skipToNextValue<value_type>(context);
						return;
					}
				}
				return;
			}
		}
	};

	template<typename buffer_type, typename parse_context_type, jsonifier::parse_options optionsNew> struct object_val_parser<buffer_type, parse_context_type, optionsNew, false> {
		static constexpr jsonifier::parse_options options{ optionsNew };
		using base = derailleur<options, parse_context_type>;

		template<jsonifier::concepts::jsonifier_object_t value_type> JSONIFIER_ALWAYS_INLINE static void impl(value_type& value, parse_context_type& context) noexcept {
			if JSONIFIER_LIKELY (context.iter + 1 < context.endIter) {
				if JSONIFIER_LIKELY (*context.iter == lBrace) {
					++context.iter;
					++context.currentObjectDepth;
					string_view_ptr wsStart = context.iter;
					JSONIFIER_SKIP_WS();
					size_t wsSize = static_cast<size_t>(context.iter - wsStart);
					index_processor_parse<parse_context_type, buffer_type, value_type, options, false>::template processIndexLambda<false>(value, context, wsStart, wsSize);
					if JSONIFIER_LIKELY (*context.iter == rBrace) {
						++context.iter;
						JSONIFIER_SKIP_WS();
					}
					JSONIFIER_ELSE_UNLIKELY(else) {
						base::template skipToEndOfValue<'{', '}'>(context);
						JSONIFIER_SKIP_WS();
					}
					--context.currentObjectDepth;
				}
				JSONIFIER_ELSE_UNLIKELY(else) {
					context.parserPtr->template reportError<parse_errors::Missing_Object_Start>(context);
					base::template skipToNextValue<value_type>(context);
					return;
				}
			}
			JSONIFIER_ELSE_UNLIKELY(else) {
				context.parserPtr->template reportError<parse_errors::Unexpected_String_End>(context);
				base::template skipToNextValue<value_type>(context);
				return;
			}
		}

		template<jsonifier::concepts::map_t value_type> JSONIFIER_ALWAYS_INLINE static void impl(value_type& value, parse_context_type& context) noexcept {
			static thread_local typename std::remove_cvref_t<value_type>::key_type key{};
			if JSONIFIER_LIKELY (context.iter + 1 < context.endIter) {
				if JSONIFIER_LIKELY (*context.iter == lBrace) {
					++context.iter;
					++context.currentObjectDepth;
					if JSONIFIER_LIKELY (*context.iter != rBrace) {
						const auto wsStart = context.iter;
						JSONIFIER_SKIP_WS();
						size_t wsSize{ static_cast<size_t>(context.iter - wsStart) };
						parse<getJsonType<typename value_type::key_type>(), options, false>::template impl<buffer_type>(key, context);

						if JSONIFIER_LIKELY ((context.iter < context.endIter) && *context.iter == colon) {
							++context.iter;
							JSONIFIER_SKIP_WS();
							parse<getJsonType<typename value_type::mapped_type>(), options, false>::template impl<buffer_type>(value[key], context);
						}
						JSONIFIER_ELSE_UNLIKELY(else) {
							context.parserPtr->template reportError<parse_errors::Missing_Colon>(context);
							base::template skipToNextValue<value_type>(context);
							return;
						}
						if (whitespaceTable[static_cast<uint8_t>(*(context.iter + wsSize))]) {
							return parseObjects<value_type, true>(value, context, wsStart, wsSize);
						} else {
							return parseObjects<value_type, false>(value, context, wsStart, wsSize);
						}
					} else {
						++context.iter;
						JSONIFIER_SKIP_WS();
						--context.currentObjectDepth;
					}
				}
				JSONIFIER_ELSE_UNLIKELY(else) {
					context.parserPtr->template reportError<parse_errors::Missing_Object_Start>(context);
					base::template skipToNextValue<value_type>(context);
					return;
				}
			}
			JSONIFIER_ELSE_UNLIKELY(else) {
				context.parserPtr->template reportError<parse_errors::Unexpected_String_End>(context);
				base::template skipToNextValue<value_type>(context);
				return;
			}
		}

		template<jsonifier::concepts::map_t value_type, bool newLines>
		JSONIFIER_ALWAYS_INLINE static void parseObjects(value_type& value, parse_context_type& context, const auto wsStart = {}, size_t wsSize = {}) {
			static thread_local typename std::remove_cvref_t<value_type>::key_type key{};
			while
				JSONIFIER_LIKELY((context.iter < context.endIter) && *context.iter != rBrace) {
					if (*context.iter == comma) {
						++context.iter;
						JSONIFIER_SKIP_MATCHING_WS();
						parse<getJsonType<typename value_type::key_type>(), options, false>::template impl<buffer_type>(key, context);

						if JSONIFIER_LIKELY ((context.iter < context.endIter) && *context.iter == colon) {
							++context.iter;
							JSONIFIER_SKIP_WS();
							parse<getJsonType<typename value_type::mapped_type>(), options, false>::template impl<buffer_type>(value[key], context);
						} else {
							context.parserPtr->template reportError<parse_errors::Missing_Colon>(context);
							base::template skipToNextValue<value_type>(context);
							return;
						}
					} else {
						context.parserPtr->template reportError<parse_errors::Missing_Comma>(context);
						base::template skipToNextValue<value_type>(context);
						return;
					}
				}
			++context.iter;
			--context.currentObjectDepth;
			JSONIFIER_SKIP_WS();
		}
	};

	JSONIFIER_ALWAYS_INLINE void noop() noexcept {};

	template<typename buffer_type, typename parse_context_type, jsonifier::parse_options optionsNew> struct array_val_parser<buffer_type, parse_context_type, optionsNew, false> {
		static constexpr jsonifier::parse_options options{ optionsNew };
		using base = derailleur<options, parse_context_type>;

		template<jsonifier::concepts::vector_t value_type> JSONIFIER_ALWAYS_INLINE static void impl(value_type& value, parse_context_type& context) noexcept {
			if JSONIFIER_LIKELY (context.iter + 1 < context.endIter) {
				if JSONIFIER_LIKELY (*context.iter == lBracket) {
					++context.currentArrayDepth;
					++context.iter;
					if JSONIFIER_LIKELY (*context.iter != rBracket) {
						const auto wsStart = context.iter;
						JSONIFIER_SKIP_WS();
						size_t wsSize{ static_cast<size_t>(context.iter - wsStart) };
						parseObjects<value_type, true>(value, context, wsStart, wsSize);
					} else {
						++context.iter;
						JSONIFIER_SKIP_WS();
						--context.currentArrayDepth;
					}
				}
				JSONIFIER_ELSE_UNLIKELY(else) {
					context.parserPtr->template reportError<parse_errors::Missing_Array_Start>(context);
					base::template skipToNextValue<value_type>(context);
				}
			}
			JSONIFIER_ELSE_UNLIKELY(else) {
				context.parserPtr->template reportError<parse_errors::Unexpected_String_End>(context);
				base::template skipToNextValue<value_type>(context);
				return;
			}
		}

		template<jsonifier::concepts::vector_t value_type, bool newLines>
		JSONIFIER_ALWAYS_INLINE static void parseObjects(value_type& value, parse_context_type& context, const auto wsStart = {}, size_t wsSize = {}) {
			if JSONIFIER_LIKELY (const size_t size = value.size(); size > 0) {
				auto iterNew = value.begin();

				for (size_t i = 0; i < size; ++i, ++iterNew) {
					parse<getJsonType<typename value_type::value_type>(), options, false>::template impl<buffer_type>(*(iterNew), context);

					if JSONIFIER_LIKELY (context.iter < context.endIter) {
						if JSONIFIER_LIKELY (*context.iter == comma) {
							++context.iter;
							JSONIFIER_SKIP_MATCHING_WS()
						}
						JSONIFIER_ELSE_UNLIKELY(else) {
							if JSONIFIER_LIKELY (*context.iter == rBracket) {
								++context.iter;
								JSONIFIER_SKIP_WS()
								--context.currentArrayDepth;
								return (value.size() == (i) + 1) ? noop() : value.resize((i) + 1);
							}
							JSONIFIER_ELSE_UNLIKELY(else) {
								context.parserPtr->template reportError<parse_errors::Imbalanced_Array_Brackets>(context);
								base::template skipToNextValue<value_type>(context);
								return;
							}
						}
					}
					JSONIFIER_ELSE_UNLIKELY(else) {
						context.parserPtr->template reportError<parse_errors::Unexpected_String_End>(context);
						base::template skipToNextValue<value_type>(context);
						return;
					}
				}
			}

			while (true) {
				parse<getJsonType<typename value_type::value_type>(), options, false>::template impl<buffer_type>(value.emplace_back(), context);
				if JSONIFIER_LIKELY (context.iter < context.endIter) {
					if JSONIFIER_LIKELY (*context.iter == comma) {
						++context.iter;
						JSONIFIER_SKIP_MATCHING_WS();
					}
					JSONIFIER_ELSE_UNLIKELY(else) {
						if JSONIFIER_LIKELY (*context.iter == rBracket) {
							++context.iter;
							JSONIFIER_SKIP_WS();
							--context.currentArrayDepth;
							return;
						}
						JSONIFIER_ELSE_UNLIKELY(else) {
							context.parserPtr->template reportError<parse_errors::Imbalanced_Array_Brackets>(context);
							base::template skipToNextValue<value_type>(context);
							return;
						}
					}
				}
				JSONIFIER_ELSE_UNLIKELY(else) {
					context.parserPtr->template reportError<parse_errors::Unexpected_String_End>(context);
					base::template skipToNextValue<value_type>(context);
					return;
				}
			}
		}

		template<jsonifier::concepts::raw_array_t value_type> JSONIFIER_ALWAYS_INLINE static void impl(value_type& value, parse_context_type& context) noexcept {
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
					base::template skipToNextValue<value_type>(context);
				}
			}
			JSONIFIER_ELSE_UNLIKELY(else) {
				context.parserPtr->template reportError<parse_errors::Unexpected_String_End>(context);
				base::template skipToNextValue<value_type>(context);
				return;
			}
		}

		template<jsonifier::concepts::raw_array_t value_type, bool newLines>
		JSONIFIER_ALWAYS_INLINE static void parseObjects(value_type& value, parse_context_type& context, const auto wsStart = {}, size_t wsSize = {}) {
			if JSONIFIER_LIKELY (const size_t n = std::size(value); n > 0) {
				auto iterNew = std::begin(value);

				for (size_t i = 0; i < n; ++i) {
					parse<getJsonType<typename value_type::value_type>(), options, false>::template impl<buffer_type>(*(iterNew++), context);

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
								context.parserPtr->template reportError<parse_errors::Imbalanced_Array_Brackets>(context);
								base::template skipToNextValue<value_type>(context);
								return;
							}
						}
					}
					JSONIFIER_ELSE_UNLIKELY(else) {
						context.parserPtr->template reportError<parse_errors::Unexpected_String_End>(context);
						base::template skipToNextValue<value_type>(context);
						return;
					}
				}
			}
		}

		template<jsonifier::concepts::tuple_t value_type> JSONIFIER_ALWAYS_INLINE static void impl(value_type& value, parse_context_type& context) noexcept {
			static constexpr auto memberCount = tuple_size_v<std::remove_cvref_t<value_type>>;
			if JSONIFIER_LIKELY (context.iter + 1 < context.endIter) {
				if JSONIFIER_LIKELY (*context.iter == lBracket) {
					++context.iter;
					JSONIFIER_SKIP_WS()
					++context.currentArrayDepth;
					if JSONIFIER_LIKELY (*context.iter != rBracket) {
						if constexpr (memberCount > 0) {
							using member_type = decltype(get<0>(value));
							parse<getJsonType<member_type>(), options, false>::template impl<buffer_type>(get<0>(value), context);
							parseObjects<value_type, memberCount, 1>(value, context);
						}
					}
					++context.iter;
					JSONIFIER_SKIP_WS()
					--context.currentArrayDepth;
				}
				JSONIFIER_ELSE_UNLIKELY(else) {
					context.parserPtr->template reportError<parse_errors::Missing_Array_Start>(context);
					base::template skipToNextValue<value_type>(context);
					return;
				}
			}
			JSONIFIER_ELSE_UNLIKELY(else) {
				context.parserPtr->template reportError<parse_errors::Unexpected_String_End>(context);
				base::template skipToNextValue<value_type>(context);
				return;
			}
		}

		template<jsonifier::concepts::tuple_t value_type, size_t memberCount, size_t index>
		JSONIFIER_ALWAYS_INLINE static void parseObjects(value_type& value, parse_context_type& context) {
			if constexpr (index < memberCount) {
				if JSONIFIER_LIKELY ((context.iter < context.endIter) && *context.iter != rBracket) {
					if JSONIFIER_LIKELY (*context.iter == comma) {
						++context.iter;
						JSONIFIER_SKIP_WS()
						using member_type = decltype(get<index>(value));
						parse<getJsonType<member_type>(), options, false>::template impl<buffer_type>(get<index>(value), context);
						return parseObjects<value_type, memberCount, index + 1>(value, context);
					} else {
						context.parserPtr->template reportError<parse_errors::Missing_Comma>(context);
						base::template skipToNextValue<value_type>(context);
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

	template<typename parse_context_type, typename buffer_type, typename value_type, jsonifier::parse_options options>
	struct index_processor_parse<parse_context_type, buffer_type, value_type, options, true> : derailleur<options, parse_context_type> {
		using base						  = derailleur<options, parse_context_type>;
		static constexpr auto memberCount = tuple_size_v<typename core_tuple_type<value_type>::core_type>;
		inline static thread_local array<bool, memberCount> antiHashStates{ [] {
			array<bool, memberCount> returnValues{};
			returnValues.fill(true);
			return returnValues;
		}() };

		template<bool haveWeStarted = false> JSONIFIER_ALWAYS_INLINE static auto processIndexLambda(value_type& value, parse_context_type& context, size_t index = 0) noexcept {
			( void )value, ( void )context;
			if constexpr (memberCount > 0) {
				if (index < memberCount) {
					if JSONIFIER_LIKELY (context.iter < context.endIter) {
						if (*context.iter != rBrace) {
							if (haveWeStarted || index > 0) {
								if JSONIFIER_LIKELY (*context.iter == comma) {
									++context.iter;
								}
								JSONIFIER_ELSE_UNLIKELY(else) {
									context.parserPtr->template reportError<parse_errors::Missing_Comma>(context);
									base::template skipToNextValue<value_type>(context);
									return;
								}
							}
							static constexpr json_parse_map_t<json_entity_parse_final, options, value_type, parse_context_type, buffer_type, jsonifier::concepts::coreV<value_type>,
								true>
								jsonMapAntiHash{};
							static constexpr json_parse_map_t<json_entity_parse_final, options, value_type, parse_context_type, buffer_type,
								jsonifier_internal::core_tuple_type<value_type>::coreTupleV, true>
								jsonMap{};
							if JSONIFIER_LIKELY ((context.iter < context.endIter) && *context.iter == quote) {
								++context.iter;
								if constexpr (options.knownOrder) {
									if (antiHashStates[index]) {
										if (jsonMapAntiHash.arrayOfPtrs[index]->processIndex(value, context)) {
											return processIndexLambda<true>(value, context, index + 1);
										} else {
											if JSONIFIER_LIKELY (auto indexNew =
																	 hash_map<value_type, std::remove_cvref_t<decltype(context.iter)>>::findIndex(context.iter, context.endIter);
												indexNew < memberCount) {
												if JSONIFIER_LIKELY (jsonMap.arrayOfPtrs[indexNew]->processIndex(value, context)) {
													return processIndexLambda<true>(value, context, index + 1);
												}
											}
											antiHashStates[index] = false;
										}
									}
								}
								if JSONIFIER_LIKELY (auto indexNew = hash_map<value_type, std::remove_cvref_t<decltype(context.iter)>>::findIndex(context.iter, context.endIter);
									indexNew < memberCount) {
									if JSONIFIER_LIKELY (jsonMap.arrayOfPtrs[indexNew]->processIndex(value, context)) {
										return processIndexLambda<true>(value, context, index + 1);
									}
								}
								base::template skipKeyStarted<value_type>(context);
								++context.iter;
								if constexpr (!options.minified) {
									while ((context.iter < context.endIter) && whitespaceTable[static_cast<uint8_t>(*context.iter)]) {
										++context.iter;
									};
								}
								if ((context.iter < context.endIter) && *context.iter == ':') [[likely]] {
									++context.iter;
									if constexpr (!options.minified) {
										while ((context.iter < context.endIter) && whitespaceTable[static_cast<uint8_t>(*context.iter)]) {
											++context.iter;
										};
									}
								} else [[unlikely]] {
									context.parserPtr->template reportError<parse_errors::Missing_Colon>(context);
									base::template skipToNextValue<value_type>(context);
									return;
								}
								base::template skipToNextValue<value_type>(context);
								;
								return processIndexLambda<true>(value, context, index);
							}
							JSONIFIER_ELSE_UNLIKELY(else) {
								context.parserPtr->template reportError<parse_errors::Missing_String_Start>(context);
								JSONIFIER_SKIP_KEY_VALUE();
								return;
							}
						} else {
							return;
						}
					}
					JSONIFIER_ELSE_UNLIKELY(else) {
						context.parserPtr->template reportError<parse_errors::Unexpected_String_End>(context);
						base::template skipToNextValue<value_type>(context);
						return;
					}
				}
				return;
			}
		}
	};

	template<typename buffer_type, typename parse_context_type, jsonifier::parse_options optionsNew> struct object_val_parser<buffer_type, parse_context_type, optionsNew, true> {
		static constexpr jsonifier::parse_options options{ optionsNew };
		using base = derailleur<options, parse_context_type>;

		template<jsonifier::concepts::jsonifier_object_t value_type> JSONIFIER_ALWAYS_INLINE static void impl(value_type& value, parse_context_type& context) noexcept {
			if JSONIFIER_LIKELY (context.iter + 1 < context.endIter) {
				if JSONIFIER_LIKELY (*context.iter == lBrace) {
					++context.iter;
					++context.currentObjectDepth;
					index_processor_parse<parse_context_type, buffer_type, value_type, options, true>::template processIndexLambda<false>(value, context);
					if JSONIFIER_LIKELY (*context.iter == rBrace) {
						++context.iter;
					}
					JSONIFIER_ELSE_UNLIKELY(else) {
						base::template skipToEndOfValue<'{', '}'>(context);
					}
					--context.currentObjectDepth;
				}
				JSONIFIER_ELSE_UNLIKELY(else) {
					context.parserPtr->template reportError<parse_errors::Missing_Object_Start>(context);
					base::template skipToNextValue<value_type>(context);
					return;
				}
			}
			JSONIFIER_ELSE_UNLIKELY(else) {
				context.parserPtr->template reportError<parse_errors::Unexpected_String_End>(context);
				base::template skipToNextValue<value_type>(context);
				return;
			}
		}

		template<jsonifier::concepts::map_t value_type> JSONIFIER_ALWAYS_INLINE static void impl(value_type& value, parse_context_type& context) noexcept {
			static thread_local typename std::remove_cvref_t<value_type>::key_type key{};
			if JSONIFIER_LIKELY (context.iter + 1 < context.endIter) {
				if JSONIFIER_LIKELY (*context.iter == lBrace) {
					++context.iter;
					++context.currentObjectDepth;
					if JSONIFIER_LIKELY (*context.iter != rBrace) {
						parse<getJsonType<typename value_type::key_type>(), options, true>::template impl<buffer_type>(key, context);

						if JSONIFIER_LIKELY ((context.iter < context.endIter) && *context.iter == colon) {
							++context.iter;
							parse<getJsonType<typename value_type::mapped_type>(), options, true>::template impl<buffer_type>(value[key], context);
						}
						JSONIFIER_ELSE_UNLIKELY(else) {
							context.parserPtr->template reportError<parse_errors::Missing_Colon>(context);
							base::template skipToNextValue<value_type>(context);
							return;
						}
						return parseObjects<value_type>(value, context);
					} else {
						++context.iter;
						--context.currentObjectDepth;
					}
				}
				JSONIFIER_ELSE_UNLIKELY(else) {
					context.parserPtr->template reportError<parse_errors::Missing_Object_Start>(context);
					base::template skipToNextValue<value_type>(context);
					return;
				}
			}
			JSONIFIER_ELSE_UNLIKELY(else) {
				context.parserPtr->template reportError<parse_errors::Unexpected_String_End>(context);
				base::template skipToNextValue<value_type>(context);
				return;
			}
		}

		template<jsonifier::concepts::map_t value_type> JSONIFIER_ALWAYS_INLINE static void parseObjects(value_type& value, parse_context_type& context) {
			static thread_local typename std::remove_cvref_t<value_type>::key_type key{};
			while
				JSONIFIER_LIKELY((context.iter < context.endIter) && *context.iter != rBrace) {
					if (*context.iter == comma) {
						++context.iter;
						parse<getJsonType<typename value_type::key_type>(), options, true>::template impl<buffer_type>(key, context);

						if JSONIFIER_LIKELY ((context.iter < context.endIter) && *context.iter == colon) {
							++context.iter;
							parse<getJsonType<typename value_type::mapped_type>(), options, true>::template impl<buffer_type>(value[key], context);
						} else {
							context.parserPtr->template reportError<parse_errors::Missing_Colon>(context);
							base::template skipToNextValue<value_type>(context);
							return;
						}
					} else {
						context.parserPtr->template reportError<parse_errors::Missing_Comma>(context);
						base::template skipToNextValue<value_type>(context);
						return;
					}
				}
			++context.iter;
			--context.currentObjectDepth;
		}
	};

	template<typename buffer_type, typename parse_context_type, jsonifier::parse_options optionsNew> struct array_val_parser<buffer_type, parse_context_type, optionsNew, true> {
		static constexpr jsonifier::parse_options options{ optionsNew };
		using base = derailleur<options, parse_context_type>;

		template<jsonifier::concepts::vector_t value_type> JSONIFIER_ALWAYS_INLINE static void impl(value_type& value, parse_context_type& context) noexcept {
			if JSONIFIER_LIKELY (context.iter + 1 < context.endIter) {
				if JSONIFIER_LIKELY ((context.iter < context.endIter) && *context.iter == lBracket) {
					++context.currentArrayDepth;
					++context.iter;
					if JSONIFIER_LIKELY ((context.iter < context.endIter) && *context.iter != rBracket) {
						if JSONIFIER_LIKELY (const size_t size = value.size(); size > 0) {
							auto iterNew = value.begin();

							for (size_t i = 0; i < size; ++i, ++iterNew) {
								parse<getJsonType<typename value_type::value_type>(), options, true>::template impl<buffer_type>(*(iterNew), context);

								if JSONIFIER_LIKELY (context.iter < context.endIter) {
									if JSONIFIER_LIKELY (*context.iter == comma) {
										++context.iter;
									}
									JSONIFIER_ELSE_UNLIKELY(else) {
										if JSONIFIER_LIKELY (*context.iter == rBracket) {
											++context.iter;
											--context.currentArrayDepth;
											return (value.size() == (i) + 1) ? noop() : value.resize((i) + 1);
										}
										JSONIFIER_ELSE_UNLIKELY(else) {
											context.parserPtr->template reportError<parse_errors::Imbalanced_Array_Brackets>(context);
											base::template skipToNextValue<value_type>(context);
											return;
										}
									}
								}
								JSONIFIER_ELSE_UNLIKELY(else) {
									context.parserPtr->template reportError<parse_errors::Unexpected_String_End>(context);
									base::template skipToNextValue<value_type>(context);
									return;
								}
							}
						}

						while (true) {
							parse<getJsonType<typename value_type::value_type>(), options, true>::template impl<buffer_type>(value.emplace_back(), context);
							if JSONIFIER_LIKELY (context.iter < context.endIter) {
								if JSONIFIER_LIKELY (*context.iter == comma) {
									++context.iter;
								}
								JSONIFIER_ELSE_UNLIKELY(else) {
									if JSONIFIER_LIKELY (*context.iter == rBracket) {
										++context.iter;
										--context.currentArrayDepth;
										return;
									}
									JSONIFIER_ELSE_UNLIKELY(else) {
										context.parserPtr->template reportError<parse_errors::Imbalanced_Array_Brackets>(context);
										base::template skipToNextValue<value_type>(context);
										return;
									}
								}
							}
							JSONIFIER_ELSE_UNLIKELY(else) {
								context.parserPtr->template reportError<parse_errors::Unexpected_String_End>(context);
								base::template skipToNextValue<value_type>(context);
								return;
							}
						}
					} else {
						++context.iter;
						--context.currentArrayDepth;
					}
				}
				JSONIFIER_ELSE_UNLIKELY(else) {
					context.parserPtr->template reportError<parse_errors::Missing_Array_Start>(context);
					base::template skipToNextValue<value_type>(context);
				}
			}
			JSONIFIER_ELSE_UNLIKELY(else) {
				context.parserPtr->template reportError<parse_errors::Unexpected_String_End>(context);
				base::template skipToNextValue<value_type>(context);
				return;
			}
		}

		template<jsonifier::concepts::raw_array_t value_type> JSONIFIER_ALWAYS_INLINE static void impl(value_type& value, parse_context_type& context) noexcept {
			if JSONIFIER_LIKELY (context.iter + 1 < context.endIter) {
				if JSONIFIER_LIKELY (*context.iter == lBracket) {
					++context.currentArrayDepth;
					++context.iter;
					if JSONIFIER_LIKELY (*context.iter != rBracket) {
						auto iterNew = std::begin(value);

						for (size_t i = 0; i < value.size(); ++i) {
							parse<getJsonType<typename value_type::value_type>(), options, true>::template impl<buffer_type>(*(iterNew++), context);

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
									context.parserPtr->template reportError<parse_errors::Imbalanced_Array_Brackets>(context);
									base::template skipToNextValue<value_type>(context);
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
					base::template skipToNextValue<value_type>(context);
				}
			}
			JSONIFIER_ELSE_UNLIKELY(else) {
				context.parserPtr->template reportError<parse_errors::Unexpected_String_End>(context);
				base::template skipToNextValue<value_type>(context);
				return;
			}
		}

		template<jsonifier::concepts::tuple_t value_type> JSONIFIER_ALWAYS_INLINE static void impl(value_type& value, parse_context_type& context) noexcept {
			static constexpr auto memberCount = tuple_size_v<std::remove_cvref_t<value_type>>;
			if JSONIFIER_LIKELY (context.iter + 1 < context.endIter) {
				if JSONIFIER_LIKELY (*context.iter == lBracket) {
					++context.iter;
					++context.currentArrayDepth;
					if JSONIFIER_LIKELY (*context.iter != rBracket) {
						if constexpr (memberCount > 0) {
							using member_type = decltype(get<0>(value));
							parse<getJsonType<member_type>(), options, true>::template impl<buffer_type>(get<0>(value), context);
							parseObjects<value_type, memberCount, 1>(value, context);
						}
					}
					++context.iter;
					--context.currentArrayDepth;
				}
				JSONIFIER_ELSE_UNLIKELY(else) {
					context.parserPtr->template reportError<parse_errors::Missing_Array_Start>(context);
					base::template skipToNextValue<value_type>(context);
					return;
				}
			}
			JSONIFIER_ELSE_UNLIKELY(else) {
				context.parserPtr->template reportError<parse_errors::Unexpected_String_End>(context);
				base::template skipToNextValue<value_type>(context);
				return;
			}
		}

		template<jsonifier::concepts::tuple_t value_type, size_t memberCount, size_t index>
		JSONIFIER_ALWAYS_INLINE static void parseObjects(value_type& value, parse_context_type& context) {
			if constexpr (index < memberCount) {
				if JSONIFIER_LIKELY ((context.iter < context.endIter) && *context.iter != rBracket) {
					if JSONIFIER_LIKELY (*context.iter == comma) {
						++context.iter;
						using member_type = decltype(get<index>(value));
						parse<getJsonType<member_type>(), options, true>::template impl<buffer_type>(get<index>(value), context);
						return parseObjects<value_type, memberCount, index + 1>(value, context);
					} else {
						context.parserPtr->template reportError<parse_errors::Missing_Comma>(context);
						base::template skipToNextValue<value_type>(context);
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

	template<typename buffer_type, typename parse_context_type, jsonifier::parse_options optionsNew, bool minified> struct string_val_parser {
		static constexpr jsonifier::parse_options options{ optionsNew };
		using base = derailleur<options, parse_context_type>;

		template<jsonifier::concepts::string_t value_type> JSONIFIER_ALWAYS_INLINE static void impl(value_type& value, parse_context_type& context) noexcept {
			base::parseString(value, context);
			if constexpr (!minified) {
				JSONIFIER_SKIP_WS();
			}
		}

		template<jsonifier::concepts::char_t value_type> JSONIFIER_ALWAYS_INLINE static void impl(value_type& value, parse_context_type& context) noexcept {
			value = static_cast<value_type>(*(context.iter + 1));
			++context.iter;
			if constexpr (!minified) {
				JSONIFIER_SKIP_WS();
			}
		}
	};

	template<typename buffer_type, typename parse_context_type, jsonifier::parse_options optionsNew, bool minified> struct number_val_parser {
		static constexpr jsonifier::parse_options options{ optionsNew };
		using base = derailleur<options, parse_context_type>;

		template<jsonifier::concepts::enum_t value_type> JSONIFIER_ALWAYS_INLINE static void impl(value_type& value, parse_context_type& context) noexcept {
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
				base::template skipToNextValue<value_type>(context);
				return;
			}
		}

		template<jsonifier::concepts::num_t value_type> JSONIFIER_ALWAYS_INLINE static void impl(value_type& value, parse_context_type& context) noexcept {
			if JSONIFIER_LIKELY (parseNumber(value, context.iter, context.endIter)) {
				if constexpr (!minified) {
					JSONIFIER_SKIP_WS();
				}
				return;
			}
			JSONIFIER_ELSE_UNLIKELY(else) {
				context.parserPtr->template reportError<parse_errors::Invalid_Number_Value>(context);
				base::template skipToNextValue<value_type>(context);
				return;
			}
		}
	};

	template<typename buffer_type, typename parse_context_type, jsonifier::parse_options optionsNew, bool minified> struct bool_val_parser {
		static constexpr jsonifier::parse_options options{ optionsNew };
		using base = derailleur<options, parse_context_type>;

		template<jsonifier::concepts::bool_t value_type> JSONIFIER_ALWAYS_INLINE static void impl(value_type& value, parse_context_type& context) noexcept {
			if JSONIFIER_LIKELY ((context.iter + 4) < context.endIter && parseBool(value, context.iter)) {
				if constexpr (!minified) {
					JSONIFIER_SKIP_WS();
				}
				return;
			}
			JSONIFIER_ELSE_UNLIKELY(else) {
				context.parserPtr->template reportError<parse_errors::Invalid_Bool_Value>(context);
				base::template skipToNextValue<value_type>(context);
				return;
			}
		}
	};

	template<typename buffer_type, typename parse_context_type, jsonifier::parse_options optionsNew, bool minified> struct null_val_parser {
		static constexpr jsonifier::parse_options options{ optionsNew };
		using base = derailleur<options, parse_context_type>;

		template<jsonifier::concepts::always_null_t value_type> JSONIFIER_ALWAYS_INLINE static void impl(value_type&, parse_context_type& context) noexcept {
			if JSONIFIER_LIKELY (parseNull(context.iter)) {
				if constexpr (!minified) {
					JSONIFIER_SKIP_WS();
				}
				return;
			}
			JSONIFIER_ELSE_UNLIKELY(else) {
				context.parserPtr->template reportError<parse_errors::Invalid_Null_Value>(context);
				base::template skipToNextValue<value_type>(context);
				return;
			}
		}
	};

	template<typename buffer_type, typename parse_context_type, jsonifier::parse_options optionsNew, bool minified> struct accessor_val_parser {
		static constexpr jsonifier::parse_options options{ optionsNew };
		using base = derailleur<options, parse_context_type>;

		template<jsonifier::json_type type, typename variant_type, size_t currentIndex = 0>
		JSONIFIER_ALWAYS_INLINE static constexpr void iterateVariantTypes(variant_type&& variant, parse_context_type& context) noexcept {
			if constexpr (currentIndex < std::variant_size_v<std::remove_cvref_t<variant_type>>) {
				using element_type = std::remove_cvref_t<decltype(std::get<currentIndex>(std::declval<std::remove_cvref_t<variant_type>>()))>;
				if constexpr (jsonifier::concepts::jsonifier_object_t<element_type> && type == jsonifier::json_type::object) {
					parse<jsonifier::json_type::object, options, minified>::template impl<buffer_type>(variant.template emplace<element_type>(element_type{}), context);
				} else if constexpr ((jsonifier::concepts::vector_t<element_type> || jsonifier::concepts::raw_array_t<element_type>) && type == jsonifier::json_type::array) {
					parse<jsonifier::json_type::array, options, minified>::template impl<buffer_type>(variant.template emplace<element_type>(element_type{}), context);
				} else if constexpr ((jsonifier::concepts::string_t<element_type> || jsonifier::concepts::string_view_t<element_type>) && type == jsonifier::json_type::string) {
					parse<jsonifier::json_type::string, options, minified>::template impl<buffer_type>(variant.template emplace<element_type>(element_type{}), context);
				} else if constexpr (jsonifier::concepts::bool_t<element_type> && type == jsonifier::json_type::boolean) {
					parse<jsonifier::json_type::boolean, options, minified>::template impl<buffer_type>(variant.template emplace<element_type>(element_type{}), context);
				} else if constexpr ((jsonifier::concepts::num_t<element_type> || jsonifier::concepts::enum_t<element_type>) && type == jsonifier::json_type::number) {
					parse<jsonifier::json_type::number, options, minified>::template impl<buffer_type>(variant.template emplace<element_type>(element_type{}), context);
				} else if constexpr (jsonifier::concepts::always_null_t<element_type> && type == jsonifier::json_type::null) {
					parse<jsonifier::json_type::null, options, minified>::template impl<buffer_type>(variant.template emplace<element_type>(element_type{}), context);
				} else if constexpr (jsonifier::concepts::accessor_t<element_type> && type == jsonifier::json_type::accessor) {
					parse<jsonifier::json_type::accessor, options, minified>::template impl<buffer_type>(variant.template emplace<element_type>(element_type{}), context);
				} else {
					return iterateVariantTypes<type, variant_type, currentIndex + 1>(variant, context);
				}
			}
		}

		template<jsonifier::concepts::variant_t value_type> JSONIFIER_ALWAYS_INLINE static void impl(value_type& value, parse_context_type& context) noexcept {
			switch (*context.iter) {
				case '{': {
					iterateVariantTypes<jsonifier::json_type::object>(value, context);
					break;
				}
				case '[': {
					iterateVariantTypes<jsonifier::json_type::array>(value, context);
					break;
				}
				case '"': {
					iterateVariantTypes<jsonifier::json_type::string>(value, context);
					break;
				}
				case 't':
					[[fallthrough]];
				case 'f': {
					iterateVariantTypes<jsonifier::json_type::boolean>(value, context);
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
					iterateVariantTypes<jsonifier::json_type::number>(value, context);
					break;
				}
				case 'n': {
					iterateVariantTypes<jsonifier::json_type::null>(value, context);
					break;
				}
				default: {
					iterateVariantTypes<jsonifier::json_type::accessor>(value, context);
					break;
				}
			}
		}

		template<jsonifier::concepts::optional_t value_type> JSONIFIER_ALWAYS_INLINE static void impl(value_type& value, parse_context_type& context) noexcept {
			if JSONIFIER_LIKELY ((context.iter < context.endIter) && *context.iter != n) {
				parse<getJsonType<typename value_type::value_type>(), options, minified>::template impl<buffer_type>(value.emplace(), context);
			} else {
				if JSONIFIER_LIKELY (parseNull(context.iter)) {
					if constexpr (!minified) {
						JSONIFIER_SKIP_WS();
					}
					return;
				}
				JSONIFIER_ELSE_UNLIKELY(else) {
					context.parserPtr->template reportError<parse_errors::Invalid_Null_Value>(context);
					base::template skipToNextValue<value_type>(context);
					return;
				}
			}
		}

		template<jsonifier::concepts::shared_ptr_t value_type> JSONIFIER_ALWAYS_INLINE static void impl(value_type& value, parse_context_type& context) noexcept {
			if JSONIFIER_LIKELY ((context.iter < context.endIter) && *context.iter != n) {
				using member_type = decltype(*value);
				if JSONIFIER_UNLIKELY (!value) {
					value = std::make_shared<std::remove_pointer_t<std::remove_cvref_t<member_type>>>();
				}
				parse<getJsonType<member_type>(), options, minified>::template impl<buffer_type>(*value, context);
			} else {
				if JSONIFIER_LIKELY (parseNull(context.iter)) {
					if constexpr (!minified) {
						JSONIFIER_SKIP_WS();
					}
					return;
				}
				JSONIFIER_ELSE_UNLIKELY(else) {
					context.parserPtr->template reportError<parse_errors::Invalid_Null_Value>(context);
					base::template skipToNextValue<value_type>(context);
					return;
				}
			}
		}

		template<jsonifier::concepts::unique_ptr_t value_type> JSONIFIER_ALWAYS_INLINE static void impl(value_type& value, parse_context_type& context) noexcept {
			if JSONIFIER_LIKELY ((context.iter < context.endIter) && *context.iter != n) {
				using member_type = decltype(*value);
				if JSONIFIER_UNLIKELY (!value) {
					value = std::make_unique<std::remove_pointer_t<std::remove_cvref_t<member_type>>>();
				}
				parse<getJsonType<member_type>(), options, minified>::template impl<buffer_type>(*value, context);
			} else {
				if JSONIFIER_LIKELY (parseNull(context.iter)) {
					if constexpr (!minified) {
						JSONIFIER_SKIP_WS();
					}
					return;
				}
				JSONIFIER_ELSE_UNLIKELY(else) {
					context.parserPtr->template reportError<parse_errors::Invalid_Null_Value>(context);
					base::template skipToNextValue<value_type>(context);
					return;
				}
			}
		}

		template<jsonifier::concepts::pointer_t value_type> JSONIFIER_ALWAYS_INLINE static void impl(value_type& value, parse_context_type& context) noexcept {
			if JSONIFIER_LIKELY ((context.iter < context.endIter) && *context.iter != n) {
				using member_type = decltype(*value);
				if JSONIFIER_UNLIKELY (!value) {
					value = new std::remove_pointer_t<std::remove_cvref_t<value_type>>{};
				}
				parse<getJsonType<member_type>(), options, minified>::template impl<buffer_type>(*value, context);
			} else {
				if JSONIFIER_LIKELY (parseNull(context.iter)) {
					if constexpr (!minified) {
						JSONIFIER_SKIP_WS();
					}
					return;
				}
				JSONIFIER_ELSE_UNLIKELY(else) {
					context.parserPtr->template reportError<parse_errors::Invalid_Null_Value>(context);
					base::template skipToNextValue<value_type>(context);
					return;
				}
			}
		}

		template<jsonifier::concepts::raw_json_t value_type> JSONIFIER_ALWAYS_INLINE static void impl(value_type& value, parse_context_type& context) noexcept {
			auto newPtr = context.iter;
			base::template skipToNextValue<value_type>(context);
			int64_t newSize = context.iter - newPtr;
			if JSONIFIER_LIKELY (newSize > 0) {
				jsonifier::string newString{};
				newString.resize(static_cast<size_t>(newSize));
				std::memcpy(newString.data(), newPtr, static_cast<size_t>(newSize));
				value = value_type{ *context.parserPtr, newString };
			}
			return;
		}

		template<jsonifier::concepts::skip_t value_type> JSONIFIER_ALWAYS_INLINE static void impl(value_type& value, parse_context_type& context) noexcept {
			base::template skipToNextValue<value_type>(context);
		}
	};

	template<typename parse_context_type, typename buffer_type, typename value_type, jsonifier::parse_options options, bool insideRepeated> struct index_processor_parse_partial {
		using base						  = derailleur<options, parse_context_type>;
		static constexpr auto memberCount = tuple_size_v<typename core_tuple_type<value_type>::core_type>;
		inline static thread_local array<bool, memberCount> antiHashStates{ [] {
			array<bool, memberCount> returnValues{};
			returnValues.fill(true);
			return returnValues;
		}() };

		template<bool haveWeStarted = false> JSONIFIER_ALWAYS_INLINE static auto processIndexLambda(value_type& value, parse_context_type& context, size_t index = 0) noexcept {
			( void )value, ( void )context;
			if (index < memberCount) {
				if (**context.iter != rBrace) {
					if JSONIFIER_LIKELY (context.iter < context.endIter) {
						if (haveWeStarted || index > 0) {
							if JSONIFIER_LIKELY (**context.iter == comma) {
								++context.iter;
							}
							JSONIFIER_ELSE_UNLIKELY(else) {
								context.parserPtr->template reportError<parse_errors::Missing_Comma>(context);
								base::template skipToNextValue<value_type>(context);
								return;
							}
						}
						static constexpr json_parse_map_t<json_entity_parse_final_partial, options, value_type, parse_context_type, buffer_type,
							core_tuple_type<value_type>::coreTupleV, insideRepeated>
							jsonMap{};
						if JSONIFIER_LIKELY ((context.iter < context.endIter) && **context.iter == quote) {
							if JSONIFIER_LIKELY (auto indexNew =
													 hash_map<value_type, std::remove_cvref_t<decltype(*context.iter)>>::findIndex((*context.iter) + 1, *context.endIter);
								indexNew < memberCount) {
								if JSONIFIER_LIKELY (jsonMap.arrayOfPtrs[indexNew]->processIndex(value, context)) {
									return processIndexLambda<true>(value, context, index + 1);
								}
							}
							base::template skipKey<value_type>(context);
							if ((context.iter < context.endIter) && **context.iter == ':') [[likely]] {
								++context.iter;
							} else [[unlikely]] {
								context.parserPtr->template reportError<parse_errors::Missing_Colon>(context);
								base::template skipToNextValue<value_type>(context);
								return;
							}
							base::template skipToNextValue<value_type>(context);
							return processIndexLambda<true>(value, context, index);
						}
						JSONIFIER_ELSE_UNLIKELY(else) {
							context.parserPtr->template reportError<parse_errors::Missing_String_Start>(context);
							base::template skipKey<value_type>(context);
							if ((context.iter < context.endIter) && **context.iter == ':') [[likely]] {
								++context.iter;
							} else [[unlikely]] {
								context.parserPtr->template reportError<parse_errors::Missing_Colon>(context);
								base::template skipToNextValue<value_type>(context);
								return;
							}
							base::template skipToNextValue<value_type>(context);
							return;
						}
					} else {
						context.parserPtr->template reportError<parse_errors::Unexpected_String_End>(context);
						base::template skipToNextValue<value_type>(context);
						return;
					}
				}
			} else {
				base::template skipToEndOfValue<'{', '}'>(context);
			}
		}

		template<typename... arg_types> JSONIFIER_INLINE static void executeIndices(arg_types&&... args) {
			(( void )(args), ...);
			processIndexLambda<0>(jsonifier_internal::forward<arg_types>(args)...);
		}
	};

	template<typename buffer_type, typename parse_context_type, jsonifier::parse_options optionsNew, bool insideRepeated> struct object_val_parser_partial {
		static constexpr jsonifier::parse_options options{ optionsNew };
		using base = derailleur<options, parse_context_type>;

		template<jsonifier::concepts::jsonifier_object_t value_type> JSONIFIER_ALWAYS_INLINE static void impl(value_type& value, parse_context_type& context) noexcept {
			if JSONIFIER_LIKELY (context.iter + 1 < context.endIter) {
				if JSONIFIER_LIKELY (**context.iter == lBrace) {
					++context.iter;
					++context.currentObjectDepth;
					index_processor_parse_partial<parse_context_type, buffer_type, value_type, options, insideRepeated>::executeIndices(value, context);
					if JSONIFIER_LIKELY (context.iter + 1 < context.endIter && **context.iter == rBrace) {
						++context.iter;
					}
					--context.currentObjectDepth;
				}
				JSONIFIER_ELSE_UNLIKELY(else) {
					context.parserPtr->template reportError<parse_errors::Missing_Object_Start>(context);
					base::template skipToNextValue<value_type>(context);
					return;
				}
			}
			JSONIFIER_ELSE_UNLIKELY(else) {
				context.parserPtr->template reportError<parse_errors::Unexpected_String_End>(context);
				base::template skipToNextValue<value_type>(context);
				return;
			}
		}

		template<jsonifier::concepts::map_t value_type> JSONIFIER_ALWAYS_INLINE static void impl(value_type& value, parse_context_type& context) noexcept {
			if JSONIFIER_LIKELY (context.iter + 1 < context.endIter) {
				if JSONIFIER_LIKELY (**context.iter == lBrace) {
					++context.iter;
					++context.currentObjectDepth;
					if JSONIFIER_LIKELY (**context.iter != rBrace) {
						static thread_local typename std::remove_cvref_t<value_type>::key_type key{};
						parse<getJsonType<typename value_type::key_type>(), options, insideRepeated>::template impl<buffer_type>(key, context);

						if JSONIFIER_LIKELY ((context.iter < context.endIter) && **context.iter == colon) {
							++context.iter;
							parse<getJsonType<typename value_type::mapped_type>(), options, insideRepeated>::template impl<buffer_type>(value[key], context);
						}
						JSONIFIER_ELSE_UNLIKELY(else) {
							context.parserPtr->template reportError<parse_errors::Missing_Colon>(context);
							base::template skipToNextValue<value_type>(context);
							return;
						}

						while ((context.iter < context.endIter) && **context.iter != rBrace) {
							if JSONIFIER_LIKELY (**context.iter == comma) {
								++context.iter;
								parse<getJsonType<typename value_type::key_type>(), options, insideRepeated>::template impl<buffer_type>(key, context);

								if JSONIFIER_LIKELY ((context.iter < context.endIter) && **context.iter == colon) {
									++context.iter;
									parse<getJsonType<typename value_type::mapped_type>(), options, insideRepeated>::template impl<buffer_type>(value[key], context);
								} else {
									context.parserPtr->template reportError<parse_errors::Missing_Colon>(context);
									base::template skipToNextValue<value_type>(context);
									return;
								}
							} else {
								context.parserPtr->template reportError<parse_errors::Missing_Comma>(context);
								base::template skipToNextValue<value_type>(context);
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
					base::template skipToNextValue<value_type>(context);
					return;
				}
			}
			JSONIFIER_ELSE_UNLIKELY(else) {
				context.parserPtr->template reportError<parse_errors::Unexpected_String_End>(context);
				base::template skipToNextValue<value_type>(context);
				return;
			}
		}

		template<jsonifier::concepts::tuple_t value_type> JSONIFIER_ALWAYS_INLINE static void impl(value_type& value, parse_context_type& context) noexcept {
			static constexpr auto memberCount = tuple_size_v<std::remove_cvref_t<value_type>>;
			if JSONIFIER_LIKELY (context.iter + 1 < context.endIter) {
				if JSONIFIER_LIKELY (**context.iter == lBracket) {
					++context.iter;
					++context.currentArrayDepth;
					if JSONIFIER_LIKELY (**context.iter != rBracket) {
						if constexpr (memberCount > 0) {
							using member_type = decltype(get<0>(value));
							parse<getJsonType<member_type>(), options, insideRepeated>::template impl<buffer_type>(get<0>(value), context);
							parseObjects<value_type, memberCount, 1>(value, context);
						}
					}
					++context.iter;
					--context.currentArrayDepth;
				}
				JSONIFIER_ELSE_UNLIKELY(else) {
					context.parserPtr->template reportError<parse_errors::Missing_Array_Start>(context);
					base::template skipToNextValue<value_type>(context);
					return;
				}
			}
			JSONIFIER_ELSE_UNLIKELY(else) {
				context.parserPtr->template reportError<parse_errors::Unexpected_String_End>(context);
				base::template skipToNextValue<value_type>(context);
				return;
			}
		}

		template<jsonifier::concepts::tuple_t value_type, size_t memberCount, size_t index>
		JSONIFIER_ALWAYS_INLINE static void parseObjects(value_type& value, parse_context_type& context) {
			if constexpr (index < memberCount) {
				if JSONIFIER_LIKELY ((context.iter < context.endIter) && **context.iter != rBracket) {
					if JSONIFIER_LIKELY (**context.iter == comma) {
						++context.iter;
						using member_type = decltype(get<index>(value));
						parse<getJsonType<member_type>(), options, insideRepeated>::template impl<buffer_type>(get<index>(value), context);
						return parseObjects<memberCount, index + 1>(value, context);
					} else {
						context.parserPtr->template reportError<parse_errors::Missing_Comma>(context);
						base::template skipToNextValue<value_type>(context);
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

	template<typename buffer_type, typename parse_context_type, jsonifier::parse_options optionsNew, bool insideRepeated> struct array_val_parser_partial {
		static constexpr jsonifier::parse_options options{ optionsNew };
		using base = derailleur<options, parse_context_type>;

		template<jsonifier::concepts::vector_t value_type> JSONIFIER_ALWAYS_INLINE static void impl(value_type& value, parse_context_type& context) noexcept {
			if JSONIFIER_LIKELY (context.iter + 1 < context.endIter) {
				if JSONIFIER_LIKELY ((context.iter < context.endIter) && **context.iter == lBracket) {
					++context.currentArrayDepth;
					++context.iter;
					if JSONIFIER_LIKELY ((context.iter < context.endIter) && **context.iter != rBracket) {
						if JSONIFIER_LIKELY (const size_t size = value.size(); size > 0) {
							auto iterNew = value.begin();

							for (size_t i = 0; i < size; ++i, ++iterNew) {
								parse<getJsonType<typename value_type::value_type>(), options, insideRepeated>::template impl<buffer_type>(*(iterNew), context);

								if JSONIFIER_LIKELY (context.iter < context.endIter) {
									if JSONIFIER_LIKELY (**context.iter == comma) {
										++context.iter;
									}
									JSONIFIER_ELSE_UNLIKELY(else) {
										if JSONIFIER_LIKELY (**context.iter == rBracket) {
											++context.iter;
											--context.currentArrayDepth;
											return (value.size() == (i) + 1) ? noop() : value.resize((i) + 1);
										}
										JSONIFIER_ELSE_UNLIKELY(else) {
											context.parserPtr->template reportError<parse_errors::Imbalanced_Array_Brackets>(context);
											base::template skipToNextValue<value_type>(context);
											return;
										}
									}
								}
								JSONIFIER_ELSE_UNLIKELY(else) {
									context.parserPtr->template reportError<parse_errors::Unexpected_String_End>(context);
									base::template skipToNextValue<value_type>(context);
									return;
								}
							}
						}

						while (true) {
							parse<getJsonType<typename value_type::value_type>(), options, insideRepeated>::template impl<buffer_type>(value.emplace_back(), context);
							if JSONIFIER_LIKELY (context.iter < context.endIter) {
								if JSONIFIER_LIKELY (**context.iter == comma) {
									++context.iter;
								}
								JSONIFIER_ELSE_UNLIKELY(else) {
									if JSONIFIER_LIKELY (**context.iter == rBracket) {
										++context.iter;
										--context.currentArrayDepth;
										return;
									}
									JSONIFIER_ELSE_UNLIKELY(else) {
										context.parserPtr->template reportError<parse_errors::Imbalanced_Array_Brackets>(context);
										base::template skipToNextValue<value_type>(context);
										return;
									}
								}
							}
							JSONIFIER_ELSE_UNLIKELY(else) {
								context.parserPtr->template reportError<parse_errors::Unexpected_String_End>(context);
								base::template skipToNextValue<value_type>(context);
								return;
							}
						}
					} else {
						++context.iter;
						--context.currentArrayDepth;
					}
				}
				JSONIFIER_ELSE_UNLIKELY(else) {
					context.parserPtr->template reportError<parse_errors::Missing_Array_Start>(context);
					base::template skipToNextValue<value_type>(context);
				}
			}
			JSONIFIER_ELSE_UNLIKELY(else) {
				context.parserPtr->template reportError<parse_errors::Unexpected_String_End>(context);
				base::template skipToNextValue<value_type>(context);
				return;
			}
		}

		template<jsonifier::concepts::raw_array_t value_type> JSONIFIER_ALWAYS_INLINE static void impl(value_type& value, parse_context_type& context) noexcept {
			if JSONIFIER_LIKELY (context.iter + 1 < context.endIter) {
				if JSONIFIER_LIKELY (**context.iter == lBracket) {
					++context.currentArrayDepth;
					++context.iter;
					if JSONIFIER_LIKELY (**context.iter != rBracket) {
						auto iterNew = std::begin(value);

						for (size_t i = 0; i < value.size(); ++i) {
							parse<getJsonType<typename value_type::value_type>(), options, insideRepeated>::template impl<buffer_type>(*(iterNew++), context);

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
									context.parserPtr->template reportError<parse_errors::Imbalanced_Array_Brackets>(context);
									base::template skipToNextValue<value_type>(context);
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
					base::template skipToNextValue<value_type>(context);
				}
			}
			JSONIFIER_ELSE_UNLIKELY(else) {
				context.parserPtr->template reportError<parse_errors::Unexpected_String_End>(context);
				base::template skipToNextValue<value_type>(context);
				return;
			}
		}

		template<jsonifier::concepts::tuple_t value_type> JSONIFIER_ALWAYS_INLINE static void impl(value_type& value, parse_context_type& context) noexcept {
			static constexpr auto memberCount = tuple_size_v<std::remove_cvref_t<value_type>>;
			if JSONIFIER_LIKELY (context.iter + 1 < context.endIter) {
				if JSONIFIER_LIKELY (**context.iter == lBracket) {
					++context.iter;
					++context.currentArrayDepth;
					if JSONIFIER_LIKELY (**context.iter != rBracket) {
						if constexpr (memberCount > 0) {
							using member_type = decltype(get<0>(value));
							parse<getJsonType<member_type>(), options, insideRepeated>::template impl<buffer_type>(get<0>(value), context);
							parseObjects<value_type, memberCount, 1>(value, context);
						}
					}
					++context.iter;
					--context.currentArrayDepth;
				}
				JSONIFIER_ELSE_UNLIKELY(else) {
					context.parserPtr->template reportError<parse_errors::Missing_Array_Start>(context);
					base::template skipToNextValue<value_type>(context);
					return;
				}
			}
			JSONIFIER_ELSE_UNLIKELY(else) {
				context.parserPtr->template reportError<parse_errors::Unexpected_String_End>(context);
				base::template skipToNextValue<value_type>(context);
				return;
			}
		}

		template<jsonifier::concepts::tuple_t value_type, size_t memberCount, size_t index>
		JSONIFIER_ALWAYS_INLINE static void parseObjects(value_type& value, parse_context_type& context) {
			if constexpr (index < memberCount) {
				if JSONIFIER_LIKELY ((context.iter < context.endIter) && **context.iter != rBracket) {
					if JSONIFIER_LIKELY (**context.iter == comma) {
						++context.iter;
						using member_type = decltype(get<index>(value));
						parse<getJsonType<member_type>(), options, insideRepeated>::template impl<buffer_type>(get<index>(value), context);
						return parseObjects<value_type, memberCount, index + 1>(value, context);
					} else {
						context.parserPtr->template reportError<parse_errors::Missing_Comma>(context);
						base::template skipToNextValue<value_type>(context);
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

	template<typename buffer_type, typename parse_context_type, jsonifier::parse_options optionsNew, bool insideRepeated> struct string_val_parser_partial {
		static constexpr jsonifier::parse_options options{ optionsNew };
		using base = derailleur<options, parse_context_type>;

		template<jsonifier::concepts::string_t value_type> JSONIFIER_ALWAYS_INLINE static void impl(value_type& value, parse_context_type& context) noexcept {
			base::parseString(value, context);
		}

		template<jsonifier::concepts::char_t value_type> JSONIFIER_ALWAYS_INLINE static void impl(value_type& value, parse_context_type& context) noexcept {
			value = static_cast<value_type>(*(*context.iter + 1));
			++context.iter;
		}
	};

	template<typename buffer_type, typename parse_context_type, jsonifier::parse_options optionsNew, bool insideRepeated> struct number_val_parser_partial {
		static constexpr jsonifier::parse_options options{ optionsNew };
		using base = derailleur<options, parse_context_type>;

		template<jsonifier::concepts::enum_t value_type> JSONIFIER_ALWAYS_INLINE static void impl(value_type& value, parse_context_type& context) noexcept {
			size_t newValue{};
			if JSONIFIER_LIKELY (parseNumber(newValue, context.iter, context.endIter)) {
				value = static_cast<value_type>(newValue);
				return;
			}
			JSONIFIER_ELSE_UNLIKELY(else) {
				context.parserPtr->template reportError<parse_errors::Invalid_Number_Value>(context);
				base::template skipToNextValue<value_type>(context);
				return;
			}
		}

		template<jsonifier::concepts::num_t value_type> JSONIFIER_ALWAYS_INLINE static void impl(value_type& value, parse_context_type& context) noexcept {
			if JSONIFIER_LIKELY (parseNumber(value, static_cast<string_view_ptr>(*context.iter), static_cast<string_view_ptr>(*context.endIter))) {
				++context.iter;
				return;
			}
			JSONIFIER_ELSE_UNLIKELY(else) {
				context.parserPtr->template reportError<parse_errors::Invalid_Number_Value>(context);
				base::template skipToNextValue<value_type>(context);
				return;
			}
		}
	};

	template<typename buffer_type, typename parse_context_type, jsonifier::parse_options optionsNew, bool insideRepeated> struct bool_val_parser_partial {
		static constexpr jsonifier::parse_options options{ optionsNew };
		using base = derailleur<options, parse_context_type>;

		template<jsonifier::concepts::bool_t value_type> JSONIFIER_ALWAYS_INLINE static void impl(value_type& value, parse_context_type& context) noexcept {
			auto newPtr = static_cast<string_view_ptr>(*context.iter);
			if JSONIFIER_LIKELY (parseBool(value, newPtr)) {
				++context.iter;
				return;
			}
			JSONIFIER_ELSE_UNLIKELY(else) {
				context.parserPtr->template reportError<parse_errors::Invalid_Bool_Value>(context);
				base::template skipToNextValue<value_type>(context);
				return;
			}
		}
	};

	template<typename buffer_type, typename parse_context_type, jsonifier::parse_options optionsNew, bool insideRepeated> struct null_val_parser_partial {
		static constexpr jsonifier::parse_options options{ optionsNew };
		using base = derailleur<options, parse_context_type>;

		template<jsonifier::concepts::always_null_t value_type> JSONIFIER_ALWAYS_INLINE static void impl(value_type& value, parse_context_type& context) noexcept {
			auto newPtr = static_cast<string_view_ptr>(*context.iter);
			if JSONIFIER_LIKELY (parseNull(value, newPtr)) {
				++context.iter;
				return;
			}
			JSONIFIER_ELSE_UNLIKELY(else) {
				context.parserPtr->template reportError<parse_errors::Invalid_Null_Value>(context);
				base::template skipToNextValue<value_type>(context);
				return;
			}
		}
	};

	template<typename buffer_type, typename parse_context_type, jsonifier::parse_options optionsNew, bool insideRepeated> struct accessor_val_parser_partial {
		static constexpr jsonifier::parse_options options{ optionsNew };
		using base = derailleur<options, parse_context_type>;

		template<jsonifier::concepts::variant_t value_type> JSONIFIER_ALWAYS_INLINE static void impl(value_type& value, parse_context_type& context) noexcept {
			static constexpr auto lambda = [](auto&& valueNew, auto&& context) {
				using member_type = decltype(valueNew);
				return parse<getJsonType<member_type>(), options, insideRepeated>::template impl<buffer_type>(valueNew, context);
			};
			visit<lambda>(value, context);
		}

		template<jsonifier::concepts::optional_t value_type> JSONIFIER_ALWAYS_INLINE static void impl(value_type& value, parse_context_type& context) noexcept {
			if JSONIFIER_LIKELY ((context.iter < context.endIter) && **context.iter != n) {
				parse<getJsonType<typename value_type::value_type>(), options, insideRepeated>::template impl<buffer_type>(value.emplace(), context);
			} else {
				if JSONIFIER_LIKELY (parseNull(*context.iter)) {
					++context.iter;
					return;
				}
				JSONIFIER_ELSE_UNLIKELY(else) {
					context.parserPtr->template reportError<parse_errors::Invalid_Null_Value>(context);
					base::template skipToNextValue<value_type>(context);
					return;
				}
			}
		}

		template<jsonifier::concepts::shared_ptr_t value_type> JSONIFIER_ALWAYS_INLINE static void impl(value_type& value, parse_context_type& context) noexcept {
			if JSONIFIER_LIKELY ((context.iter < context.endIter) && **context.iter != n) {
				using member_type = decltype(*value);
				if JSONIFIER_UNLIKELY (!value) {
					value = std::make_shared<std::remove_pointer_t<std::remove_cvref_t<member_type>>>();
				}
				parse<getJsonType<member_type>(), options, insideRepeated>::template impl<buffer_type>(*value, context);
			} else {
				if JSONIFIER_LIKELY (parseNull(*context.iter)) {
					++context.iter;
					return;
				}
				JSONIFIER_ELSE_UNLIKELY(else) {
					context.parserPtr->template reportError<parse_errors::Invalid_Null_Value>(context);
					base::template skipToNextValue<value_type>(context);
					return;
				}
			}
		}

		template<jsonifier::concepts::unique_ptr_t value_type> JSONIFIER_ALWAYS_INLINE static void impl(value_type& value, parse_context_type& context) noexcept {
			if JSONIFIER_LIKELY ((context.iter < context.endIter) && **context.iter != n) {
				using member_type = decltype(*value);
				if JSONIFIER_UNLIKELY (!value) {
					value = std::make_unique<std::remove_pointer_t<std::remove_cvref_t<member_type>>>();
				}
				parse<getJsonType<member_type>(), options, insideRepeated>::template impl<buffer_type>(*value, context);
			} else {
				if JSONIFIER_LIKELY (parseNull(*context.iter)) {
					++context.iter;
					return;
				}
				JSONIFIER_ELSE_UNLIKELY(else) {
					context.parserPtr->template reportError<parse_errors::Invalid_Null_Value>(context);
					base::template skipToNextValue<value_type>(context);
					return;
				}
			}
		}

		template<jsonifier::concepts::pointer_t value_type> JSONIFIER_ALWAYS_INLINE static void impl(value_type& value, parse_context_type& context) noexcept {
			using member_type = decltype(*value);
			if JSONIFIER_LIKELY ((context.iter < context.endIter) && *context.iter != n) {
				if JSONIFIER_UNLIKELY (!value) {
					value = new std::remove_pointer_t<std::remove_cvref_t<value_type>>{};
				}
				parse<getJsonType<member_type>(), options, insideRepeated>::template impl<buffer_type>(*value, context);
			} else {
				if JSONIFIER_LIKELY (parseNull(*context.iter)) {
					++context.iter;
					return;
				}
				JSONIFIER_ELSE_UNLIKELY(else) {
					context.parserPtr->template reportError<parse_errors::Invalid_Null_Value>(context);
					base::template skipToNextValue<value_type>(context);
					return;
				}
			}
		}

		template<jsonifier::concepts::raw_json_t value_type> JSONIFIER_ALWAYS_INLINE static void impl(value_type& value, parse_context_type& context) noexcept {
			auto newPtr = *context.iter;
			base::template skipToNextValue<value_type>(context);
			int64_t newSize = *context.iter - newPtr;
			if JSONIFIER_LIKELY (newSize > 0) {
				jsonifier::string newString{};
				newString.resize(static_cast<size_t>(newSize));
				std::memcpy(newString.data(), newPtr, static_cast<size_t>(newSize));
				value = value_type{ *context.parserPtr, newString };
			}
			return;
		}

		template<jsonifier::concepts::skip_t value_type> JSONIFIER_ALWAYS_INLINE static void impl(value_type& value, parse_context_type& context) noexcept {
			base::template skipToNextValue<value_type>(context);
		}
	};
}