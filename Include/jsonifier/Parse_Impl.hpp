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

	template<typename value_type, typename buffer_type, typename context_type, jsonifier::parse_options options, bool minifiedOrInsideRepeated> struct parse_types_impl {
		using base						  = derailleur<options, context_type>;
		static constexpr auto memberCount = tuple_size_v<core_tuple_type<value_type>>;
		template<size_t index> JSONIFIER_INLINE static bool processIndex(value_type& value, context_type& context) {
			static constexpr auto tupleElem		= get<index>(jsonifier::core<std::remove_cvref_t<value_type>>::parseValue);
			static constexpr auto key			= tupleElem.name.operator jsonifier::string_view();
			static constexpr auto ptrNew		= tupleElem.memberPtr;
			static constexpr auto stringLiteral = stringLiteralFromView<key.size()>(key);
			static constexpr auto keySize		= key.size();
			static constexpr auto keySizeNew	= keySize + 1;
			if JSONIFIER_LIKELY (((context.iter + keySize) < context.endIter) && string_literal_comparitor<decltype(stringLiteral), stringLiteral>::impl(context.iter)) {
				context.iter += keySizeNew;
				if constexpr (!options.minified) {
					JSONIFIER_SKIP_WS();
				}
				if JSONIFIER_LIKELY ((context.iter < context.endIter) && *context.iter == colon) {
					++context.iter;
					if constexpr (!options.minified) {
						JSONIFIER_SKIP_WS();
					}
					if constexpr (jsonifier::concepts::has_excluded_keys<value_type>) {
						auto& keys = value.jsonifierExcludedKeys;
						if JSONIFIER_LIKELY (keys.find(static_cast<typename std::remove_cvref_t<decltype(keys)>::key_type>(key)) != keys.end()) {
							base::skipToNextValue(context);
							return true;
						}
					}
					using member_type = typename std::remove_cvref_t<decltype(tupleElem)>::member_type;
					parse<getJsonType<member_type>(), options, minifiedOrInsideRepeated>::template impl<buffer_type>(value.*ptrNew, context);
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

	template<typename value_type, typename buffer_type, typename context_type, jsonifier::parse_options options, bool minifiedOrInsideRepeated> struct parse_types_impl_partial {
		using base						  = derailleur<options, context_type>;
		static constexpr auto memberCount = tuple_size_v<core_tuple_type<value_type>>;
		template<size_t index> JSONIFIER_INLINE static bool processIndex(value_type& value, context_type& context) {
			static constexpr auto tupleElem		= get<index>(jsonifier::core<std::remove_cvref_t<value_type>>::parseValue);
			static constexpr auto key			= tupleElem.name.operator jsonifier::string_view();
			static constexpr auto ptrNew		= tupleElem.memberPtr;
			static constexpr auto stringLiteral = stringLiteralFromView<key.size()>(key);
			static constexpr auto keySize		= key.size();
			static constexpr auto keySizeNew	= keySize + 1;
			if JSONIFIER_LIKELY (((context.iter + 1) < context.endIter) && (*((*context.iter) + keySizeNew)) == quote &&
				string_literal_comparitor<decltype(stringLiteral), stringLiteral>::impl((*context.iter) + 1)) {
				++context.iter;
				if JSONIFIER_LIKELY ((context.iter < context.endIter) && **context.iter == colon) {
					++context.iter;
					if constexpr (jsonifier::concepts::has_excluded_keys<value_type>) {
						auto& keys = value.jsonifierExcludedKeys;
						if JSONIFIER_LIKELY (keys.find(static_cast<typename std::remove_cvref_t<decltype(keys)>::key_type>(key)) != keys.end()) {
							base::skipToNextValue(context);
							return false;
						}
					}
					using member_type = typename std::remove_cvref_t<decltype(tupleElem)>::member_type;
					parse<getJsonType<member_type>(), options, minifiedOrInsideRepeated>::template impl<buffer_type>(value.*ptrNew, context);
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

	template<template<typename, typename, typename, jsonifier::parse_options, bool> typename parsing_type, typename value_type, typename buffer_type, typename context_type,
		jsonifier::parse_options options, bool minifiedOrInsideRepeated, size_t... indices>
	static constexpr auto generateFunctionPtrs(std::index_sequence<indices...>) noexcept {
		using function_type = decltype(&parse_types_impl<value_type, buffer_type, context_type, options, minifiedOrInsideRepeated>::template processIndex<0>);
		return array<function_type, sizeof...(indices)>{
			{ &parsing_type<value_type, buffer_type, context_type, options, minifiedOrInsideRepeated>::template processIndex<indices>... }
		};
	}

	template<jsonifier::parse_options options, typename value_type, typename context_type, bool newLines = true>
	JSONIFIER_INLINE static void checkForEntryComma(context_type& context, const char* wsStart, size_t wsSize) {
		using base = derailleur<options, context_type>;
		if JSONIFIER_LIKELY (*context.iter == comma) {
			++context.iter;
			JSONIFIER_SKIP_MATCHING_WS()
		}
		JSONIFIER_ELSE_UNLIKELY(else) {
			context.parserPtr->template reportError<parse_errors::Missing_Comma>(context);
			base::skipToNextValue(context);
			return;
		}
	}

	template<jsonifier::parse_options options, typename value_type, typename context_type> JSONIFIER_INLINE static void checkForEntryComma(context_type& context) {
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

	template<jsonifier::parse_options options, typename value_type, partial_reading_context_t context_type> JSONIFIER_INLINE static void checkForEntryComma(context_type& context) {
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

	template<template<typename, typename, typename, jsonifier::parse_options, bool> typename parsing_type, typename value_type, typename buffer_type, typename context_type,
		jsonifier::parse_options options, bool minifiedOrInsideRepeated>
	static constexpr auto functionPtrs{ generateFunctionPtrs<parsing_type, value_type, buffer_type, context_type, options, minifiedOrInsideRepeated>(
		std::make_index_sequence<tuple_size_v<core_tuple_type<value_type>>>{}) };

	template<jsonifier::parse_options options, typename json_entity_type, bool minified> struct json_entity_parse;

	template<jsonifier::parse_options options, typename json_entity_type> struct json_entity_parse<options, json_entity_type, false> : public json_entity_type {
		static constexpr auto memberCount{ tuple_size_v<core_tuple_type<typename json_entity_type::class_type>> };
		inline static thread_local array<uint64_t, (memberCount > 0 ? memberCount : 1)> antiHashStates{ [] {
			array<uint64_t, (memberCount > 0 ? memberCount : 1)> returnValues{};
			for (uint64_t x = 0; x < memberCount; ++x) {
				returnValues[x] = x;
			}
			return returnValues;
		}() };

		constexpr json_entity_parse() noexcept = default;

		template<typename buffer_type, typename value_type, typename context_type>
		static void processIndexCall(value_type& value, context_type& context, const char* wsStart, size_t wsSize) {
			return processIndex<buffer_type, value_type, context_type, true>(value, context, wsStart, wsSize);
		}

		template<typename buffer_type, typename value_type, typename context_type>
		static void processIndexImpl(value_type& value, context_type& context, const char* wsStart, size_t wsSize) {
			using base = derailleur<options, context_type>;
			if constexpr (options.knownOrder) {
				if (antiHashStates[json_entity_type::index] == json_entity_type::index) {
					static constexpr auto key			= json_entity_type::name.operator jsonifier::string_view();
					static constexpr auto ptrNew		= json_entity_type::memberPtr;
					static constexpr auto stringLiteral = stringLiteralFromView<key.size()>(key);
					static constexpr auto keySize		= key.size();
					static constexpr auto keySizeNew	= keySize + 1;
					if JSONIFIER_LIKELY (((context.iter + keySize) < context.endIter) && (*(context.iter + keySize)) == quote &&
						string_literal_comparitor<decltype(stringLiteral), stringLiteral>::impl(context.iter)) {
						context.iter += keySizeNew;
						JSONIFIER_SKIP_WS();
						if JSONIFIER_LIKELY ((context.iter < context.endIter) && *context.iter == colon) {
							++context.iter;
							JSONIFIER_SKIP_WS();
							if constexpr (jsonifier::concepts::has_excluded_keys<value_type>) {
								auto& keys = value.jsonifierExcludedKeys;
								if JSONIFIER_LIKELY (keys.find(static_cast<typename std::remove_cvref_t<decltype(keys)>::key_type>(key)) != keys.end()) {
									base::skipToNextValue(context);
									return;
								}
							}
							using member_type = typename json_entity_type::member_type;
							parse<getJsonType<member_type>(), options, false>::template impl<buffer_type>(value.*ptrNew, context);
							return;
						}
						JSONIFIER_ELSE_UNLIKELY(else) {
							context.parserPtr->template reportError<parse_errors::Missing_Colon>(context);
							base::skipToNextValue(context);
						}
					}
				} else {
					if JSONIFIER_LIKELY (functionPtrs<parse_types_impl, value_type, buffer_type, context_type, options, false>[antiHashStates[json_entity_type::index]](value,
											 context)) {
						return;
					}
				}
			}
			if JSONIFIER_LIKELY (auto indexNew = hash_map<value_type, std::remove_cvref_t<decltype(context.iter)>>::findIndex(context.iter, context.endIter);
				indexNew < memberCount) {
				if JSONIFIER_LIKELY (functionPtrs<parse_types_impl, value_type, buffer_type, context_type, options, false>[indexNew](value, context)) {
					antiHashStates[json_entity_type::index] = indexNew;
					return;
				}
			}
			base::template skipKeyStarted<value_type>(context);
			++context.iter;
			JSONIFIER_SKIP_WS();
			if ((context.iter < context.endIter) && *context.iter == ':') [[likely]] {
				++context.iter;
				JSONIFIER_SKIP_WS();
			} else [[unlikely]] {
				context.parserPtr->template reportError<parse_errors::Missing_Colon>(context);
				base::skipToNextValue(context);
				return;
			}
			base::skipToNextValue(context);
			return processIndexCall<buffer_type>(value, context, wsStart, wsSize);
		}

		template<typename buffer_type, typename value_type, typename context_type, bool haveWeStarted = false>
		JSONIFIER_INLINE static void processIndex(value_type& value, context_type& context, const char* wsStart, size_t wsSize) {
			using base = derailleur<options, context_type>;
			if constexpr (memberCount > 0 && json_entity_type::index < memberCount) {
				if JSONIFIER_LIKELY (context.iter < context.endIter) {
					if (*context.iter != rBrace) {
						if constexpr (haveWeStarted || json_entity_type::index > 0) {
							checkForEntryComma<options, value_type>(context, wsStart, wsSize);
						}
						if JSONIFIER_LIKELY ((context.iter < context.endIter) && *context.iter == quote) {
							++context.iter;
							processIndexImpl<buffer_type>(value, context, wsStart, wsSize);
						}
						JSONIFIER_ELSE_UNLIKELY(else) {
							context.parserPtr->template reportError<parse_errors::Missing_String_Start>(context);
							base::template skipKeyStarted<value_type>(context);
							++context.iter;
							JSONIFIER_SKIP_WS();
							if ((context.iter < context.endIter) && *context.iter == ':') [[likely]] {
								++context.iter;
								JSONIFIER_SKIP_WS();
							} else [[unlikely]] {
								context.parserPtr->template reportError<parse_errors::Missing_Colon>(context);
								base::skipToNextValue(context);
								return;
							}
							base::skipToNextValue(context);
							;
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

	template<jsonifier::parse_options options, typename json_entity_type> struct json_entity_parse<options, json_entity_type, true> : public json_entity_type {
		static constexpr auto memberCount{ tuple_size_v<core_tuple_type<typename json_entity_type::class_type>> };
		inline static thread_local array<uint64_t, (memberCount > 0 ? memberCount : 1)> antiHashStates{ [] {
			array<uint64_t, (memberCount > 0 ? memberCount : 1)> returnValues{};
			for (uint64_t x = 0; x < memberCount; ++x) {
				returnValues[x] = x;
			}
			return returnValues;
		}() };

		constexpr json_entity_parse() noexcept = default;

		template<typename buffer_type, typename value_type, typename context_type> static void processIndexCall(value_type& value, context_type& context) {
			return processIndex<buffer_type, value_type, context_type, true>(value, context);
		}

		template<typename buffer_type, typename value_type, typename context_type> static void processIndexImpl(value_type& value, context_type& context) {
			using base = derailleur<options, context_type>;
			if constexpr (options.knownOrder) {
				if (antiHashStates[json_entity_type::index] == json_entity_type::index) {
					static constexpr auto key			= json_entity_type::name.operator jsonifier::string_view();
					static constexpr auto ptrNew		= json_entity_type::memberPtr;
					static constexpr auto stringLiteral = stringLiteralFromView<key.size()>(key);
					static constexpr auto keySize		= key.size();
					static constexpr auto keySizeNew	= keySize + 1;
					if JSONIFIER_LIKELY (((context.iter + keySize) < context.endIter) && (*(context.iter + keySize)) == quote &&
						string_literal_comparitor<decltype(stringLiteral), stringLiteral>::impl(context.iter)) {
						context.iter += keySizeNew;
						if JSONIFIER_LIKELY ((context.iter < context.endIter) && *context.iter == colon) {
							++context.iter;
							if constexpr (jsonifier::concepts::has_excluded_keys<value_type>) {
								auto& keys = value.jsonifierExcludedKeys;
								if JSONIFIER_LIKELY (keys.find(static_cast<typename std::remove_cvref_t<decltype(keys)>::key_type>(key)) != keys.end()) {
									base::skipToNextValue(context);
									return;
								}
							}
							using member_type = typename json_entity_type::member_type;
							parse<getJsonType<member_type>(), options, true>::template impl<buffer_type>(value.*ptrNew, context);
							return;
						}
						JSONIFIER_ELSE_UNLIKELY(else) {
							context.parserPtr->template reportError<parse_errors::Missing_Colon>(context);
							base::skipToNextValue(context);
						}
					}
				} else {
					if JSONIFIER_LIKELY (functionPtrs<parse_types_impl, value_type, buffer_type, context_type, options, true>[antiHashStates[json_entity_type::index]](value,
											 context)) {
						return;
					}
				}
			}
			if JSONIFIER_LIKELY (auto indexNew = hash_map<value_type, std::remove_cvref_t<decltype(context.iter)>>::findIndex(context.iter, context.endIter);
				indexNew < memberCount) {
				if JSONIFIER_LIKELY (functionPtrs<parse_types_impl, value_type, buffer_type, context_type, options, true>[indexNew](value, context)) {
					antiHashStates[json_entity_type::index] = indexNew;
					return;
				}
			}
			base::template skipKeyStarted<value_type>(context);
			++context.iter;
			if JSONIFIER_LIKELY ((context.iter < context.endIter) && *context.iter == ':') [[likely]] {
				++context.iter;
				base::skipToNextValue(context);
				return processIndexCall<buffer_type>(value, context);
			}
			JSONIFIER_ELSE_UNLIKELY(else) [[unlikely]] {
				context.parserPtr->template reportError<parse_errors::Missing_Colon>(context);
				base::skipToNextValue(context);
				return;
			}
		}

		template<typename buffer_type, typename value_type, typename context_type, bool haveWeStarted = false>
		JSONIFIER_INLINE static void processIndex(value_type& value, context_type& context) {
			using base = derailleur<options, context_type>;
			if constexpr (memberCount > 0 && json_entity_type::index < memberCount) {
				if JSONIFIER_LIKELY (context.iter < context.endIter) {
					if (*context.iter != rBrace) {
						if constexpr (haveWeStarted || json_entity_type::index > 0) {
							checkForEntryComma<options, value_type>(context);
						}
						if JSONIFIER_LIKELY ((context.iter < context.endIter) && *context.iter == quote) {
							++context.iter;
							processIndexImpl<buffer_type>(value, context);
						}
						JSONIFIER_ELSE_UNLIKELY(else) {
							context.parserPtr->template reportError<parse_errors::Missing_String_Start>(context);
							base::template skipKeyStarted<value_type>(context);
							++context.iter;
							if ((context.iter < context.endIter) && *context.iter == ':') [[likely]] {
								++context.iter;
							} else [[unlikely]] {
								context.parserPtr->template reportError<parse_errors::Missing_Colon>(context);
								base::skipToNextValue(context);
								return;
							}
							base::skipToNextValue(context);
							;
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
		template<typename buffer_type, typename... arg_types> constexpr static void iterateValues(arg_types&&... args) {
			(( void )(args), ...);
			(bases::template processIndex<buffer_type>(args...), ...);
		}
	};

	template<jsonifier::parse_options options, typename value_type, typename buffer_type, typename context_type, bool minifiedOrInsideRepeated, typename index_sequence,
		typename... value_types>
	struct get_parse_base;

	template<jsonifier::parse_options options, typename value_type, typename buffer_type, typename context_type, bool minifiedOrInsideRepeated, size_t... I>
	struct get_parse_base<options, value_type, buffer_type, context_type, minifiedOrInsideRepeated, std::index_sequence<I...>> {
		using type =
			parse_map<json_entity_parse<options, std::remove_cvref_t<decltype(get<I>(jsonifier::core<std::remove_cvref_t<value_type>>::parseValue))>, minifiedOrInsideRepeated>...>;
	};

	template<jsonifier::parse_options options, typename value_type, typename buffer_type, typename context_type, bool minifiedOrInsideRepeated> using parse_base_t =
		typename get_parse_base<options, value_type, buffer_type, context_type, minifiedOrInsideRepeated,
			std::make_index_sequence<tuple_size_v<core_tuple_type<value_type>>>>::type;

	template<typename value_type, typename buffer_type, typename context_type, jsonifier::parse_options options, auto tupleElem, bool minifiedOrInsideRepeated> struct parse_types;

	template<typename value_type, typename buffer_type, typename context_type, jsonifier::parse_options options, auto tupleElem, bool insideRepeated> struct parse_types_partial {
		using base						  = derailleur<options, context_type>;
		static constexpr auto memberCount = tuple_size_v<core_tuple_type<value_type>>;
		inline static thread_local array<uint64_t, (memberCount > 0 ? memberCount : 1)> antiHashStates{ [] {
			array<uint64_t, (memberCount > 0 ? memberCount : 1)> returnValues{};
			for (uint64_t x = 0; x < memberCount; ++x) {
				returnValues[x] = x;
			}
			return returnValues;
		}() };

		static void processIndexCall(value_type& value, context_type& context) noexcept {
			return processIndex<true>(value, context);
		}

		template<bool haveWeStarted = false> JSONIFIER_CLANG_INLINE static void processIndex(value_type& value, context_type& context) noexcept {
			( void )value, ( void )context;
			if constexpr (tupleElem.index < memberCount) {
				if (**context.iter != rBrace) {
					if JSONIFIER_LIKELY (context.iter < context.endIter) {
						if constexpr (haveWeStarted || tupleElem.index > 0) {
							checkForEntryComma<options, value_type>(context);
						}

						if JSONIFIER_LIKELY ((context.iter < context.endIter) && **context.iter == quote) {
							if (antiHashStates[tupleElem.index] != std::numeric_limits<uint64_t>::max()) {
								if JSONIFIER_LIKELY (functionPtrs<parse_types_impl_partial, value_type, buffer_type, context_type, options,
														 insideRepeated>[antiHashStates[tupleElem.index]](value, context)) {
									return;
								} else {
									if JSONIFIER_LIKELY (auto indexNew =
															 hash_map<value_type, std::remove_cvref_t<decltype(*context.iter)>>::findIndex((*context.iter) + 1, *context.endIter);
										indexNew < memberCount) {
										if JSONIFIER_LIKELY (functionPtrs<parse_types_impl_partial, value_type, buffer_type, context_type, options, insideRepeated>[indexNew](value,
																 context)) {
											antiHashStates[tupleElem.index] = indexNew;
											return;
										}
									}
									antiHashStates[tupleElem.index] = std::numeric_limits<uint64_t>::max();
								}
							}
							if JSONIFIER_LIKELY (auto indexNew =
													 hash_map<value_type, std::remove_cvref_t<decltype(*context.iter)>>::findIndex((*context.iter) + 1, *context.endIter);
								indexNew < memberCount) {
								if JSONIFIER_LIKELY (functionPtrs<parse_types_impl_partial, value_type, buffer_type, context_type, options, insideRepeated>[indexNew](value,
														 context)) {
									return;
								}
							}
							base::template skipKey<value_type>(context);
							if ((context.iter < context.endIter) && **context.iter == ':') [[likely]] {
								++context.iter;
							} else [[unlikely]] {
								context.parserPtr->template reportError<parse_errors::Missing_Colon>(context);
								base::skipToNextValue(context);
								return;
							}
							base::skipToNextValue(context);
							return processIndexCall(value, context);
						}
						JSONIFIER_ELSE_UNLIKELY(else) {
							context.parserPtr->template reportError<parse_errors::Missing_String_Start>(context);
							base::template skipKey<value_type>(context);
							if ((context.iter < context.endIter) && **context.iter == ':') [[likely]] {
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

	template<template<typename, typename, typename, jsonifier::parse_options, auto, bool> typename parse_type, typename value_type, typename buffer_type, typename context_type,
		jsonifier::parse_options options, bool minifiedOrInsideRepeated, auto... values>
	struct parser_map {
		template<typename... arg_types> JSONIFIER_CLANG_INLINE static void processIndices(arg_types&&... args) {
			(( void )(args), ...);
			if constexpr (sizeof...(values) > 0) {
				(parse_type<value_type, buffer_type, context_type, options, values, minifiedOrInsideRepeated>::processIndex(std::forward<arg_types>(args)...), ...);
			}
		}
	};

	template<template<typename, typename, typename, jsonifier::parse_options, auto, bool> typename parse_type, typename value_type, typename buffer_type, typename context_type,
		jsonifier::parse_options options, bool minifiedOrInsideRepeated, typename index_sequence, auto tuple>
	struct get_parser_base;

	template<template<typename, typename, typename, jsonifier::parse_options, auto, bool> typename parse_type, typename value_type, typename buffer_type, typename context_type,
		jsonifier::parse_options options, bool minifiedOrInsideRepeated, size_t... I, auto tuple>
	struct get_parser_base<parse_type, value_type, buffer_type, context_type, options, minifiedOrInsideRepeated, std::index_sequence<I...>, tuple> {
		using type = parser_map<parse_type, value_type, buffer_type, context_type, options, minifiedOrInsideRepeated, get<I>(tuple)...>;
	};

	template<template<typename, typename, typename, jsonifier::parse_options, auto, bool> typename parse_type, typename value_type, typename buffer_type, typename context_type,
		jsonifier::parse_options options, bool minifiedOrInsideRepeated, auto value>
	using parser_base_t = typename get_parser_base<parse_type, value_type, buffer_type, context_type, options, minifiedOrInsideRepeated,
		std::make_index_sequence<tuple_size_v<decltype(value)>>, value>::type;

	template<typename buffer_type, typename context_type, jsonifier::parse_options optionsNew> struct object_val_parser<buffer_type, context_type, optionsNew, false> {
		static constexpr jsonifier::parse_options options{ optionsNew };
		using base = derailleur<options, context_type>;

		template<typename value_type> JSONIFIER_INLINE static void parseObjectOpening(value_type& value, context_type& context) noexcept {
			if JSONIFIER_LIKELY (context.iter < context.endIter) {
				if JSONIFIER_LIKELY (*context.iter == lBrace) {
					++context.iter;
					++context.currentObjectDepth;
					string_view_ptr wsStart = context.iter;
					JSONIFIER_SKIP_WS();
					size_t wsSize = static_cast<size_t>(context.iter - wsStart);
					parse_base_t<options, value_type, buffer_type, context_type, false>::template iterateValues<buffer_type>(value, context, wsStart, wsSize);
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

		template<jsonifier::concepts::jsonifier_object_t value_type> JSONIFIER_INLINE static void impl(value_type& value, context_type& context) noexcept {
			parseObjectOpening<value_type>(value, context);
			parseObjectEnding(context);
		}

		template<jsonifier::concepts::map_t value_type> JSONIFIER_INLINE static void impl(value_type& value, context_type& context) noexcept {
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
							base::skipToNextValue(context);
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

		template<jsonifier::concepts::map_t value_type, bool newLines>
		JSONIFIER_INLINE static void parseObjects(value_type& value, context_type& context, const auto wsStart = {}, size_t wsSize = {}) {
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

	JSONIFIER_INLINE void noop() noexcept {};

	template<typename buffer_type, typename context_type, jsonifier::parse_options optionsNew> struct array_val_parser<buffer_type, context_type, optionsNew, false> {
		static constexpr jsonifier::parse_options options{ optionsNew };
		using base = derailleur<options, context_type>;

		template<jsonifier::concepts::vector_t value_type> JSONIFIER_INLINE static void impl(value_type& value, context_type& context) noexcept {
			if JSONIFIER_LIKELY (context.iter + 1 < context.endIter) {
				if JSONIFIER_LIKELY (*context.iter == lBracket) {
					++context.currentArrayDepth;
					++context.iter;
					if JSONIFIER_LIKELY (*context.iter != rBracket) {
						const auto wsStart = context.iter;
						JSONIFIER_SKIP_WS();
						size_t wsSize{ static_cast<size_t>(context.iter - wsStart) };
						if (auto size = value.size() > 0) {
							parseObjectsWithSize<value_type, true>(value, context, size, wsStart, wsSize);
						} else {
							parseObjects<value_type, true>(value, context, wsStart, wsSize);
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

		template<jsonifier::concepts::vector_t value_type, bool newLines>
		JSONIFIER_INLINE static void parseObjectsWithSize(value_type& value, context_type& context, size_t size, const auto wsStart = {}, size_t wsSize = {}) {
			size_t i	 = 0;
			auto newIter = value.begin();
			auto endIter = value.end();

			for (; newIter != endIter; ++i, ++newIter) {
				parse<getJsonType<typename value_type::value_type>(), options, false>::template impl<buffer_type>(*newIter, context);

				if JSONIFIER_LIKELY (context.iter < context.endIter) {
					if JSONIFIER_LIKELY (*context.iter == comma) {
						++context.iter;
						JSONIFIER_SKIP_MATCHING_WS()
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
			parseObjects<value_type, newLines>(value, context, wsStart, wsSize);
		}

		template<jsonifier::concepts::vector_t value_type, bool newLines>
		JSONIFIER_INLINE static void parseObjects(value_type& value, context_type& context, const auto wsStart = {}, size_t wsSize = {}) {
			parse<getJsonType<typename value_type::value_type>(), options, false>::template impl<buffer_type>(value.emplace_back(), context);

			while (context.iter < context.endIter && *context.iter == comma) {
				++context.iter;
				JSONIFIER_SKIP_MATCHING_WS()
				parse<getJsonType<typename value_type::value_type>(), options, false>::template impl<buffer_type>(value.emplace_back(), context);
			}
			if JSONIFIER_LIKELY (*context.iter == rBracket) {
				return;
			} else {
				context.parserPtr->template reportError<parse_errors::Missing_Array_End>(context);
				base::skipToNextValue(context);
				return;
			}
		}

		template<jsonifier::concepts::raw_array_t value_type> JSONIFIER_INLINE static void impl(value_type& value, context_type& context) noexcept {
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

		template<jsonifier::concepts::raw_array_t value_type, bool newLines>
		JSONIFIER_INLINE static void parseObjects(value_type& value, context_type& context, const auto wsStart = {}, size_t wsSize = {}) {
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

		template<jsonifier::concepts::tuple_t value_type> JSONIFIER_INLINE static void impl(value_type& value, context_type& context) noexcept {
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

		template<jsonifier::concepts::tuple_t value_type, size_t memberCount, size_t index> JSONIFIER_INLINE static void parseObjects(value_type& value, context_type& context) {
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

	template<typename buffer_type, typename context_type, jsonifier::parse_options optionsNew> struct object_val_parser<buffer_type, context_type, optionsNew, true> {
		static constexpr jsonifier::parse_options options{ optionsNew };
		using base = derailleur<options, context_type>;

		template<typename value_type> JSONIFIER_INLINE static void parseObjectOpening(value_type& value, context_type& context) noexcept {
			if JSONIFIER_LIKELY (context.iter < context.endIter) {
				if JSONIFIER_LIKELY (*context.iter == lBrace) {
					++context.iter;
					++context.currentObjectDepth;
					parse_base_t<options, value_type, buffer_type, context_type, true>::template iterateValues<buffer_type>(value, context);
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

		template<jsonifier::concepts::jsonifier_object_t value_type> JSONIFIER_INLINE static void impl(value_type& value, context_type& context) noexcept {
			parseObjectOpening<value_type>(value, context);
			parseObjectEnding(context);
		}

		template<jsonifier::concepts::map_t value_type> JSONIFIER_INLINE static void impl(value_type& value, context_type& context) noexcept {
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
							base::skipToNextValue(context);
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

		template<jsonifier::concepts::map_t value_type> JSONIFIER_INLINE static void parseObjects(value_type& value, context_type& context) {
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
		}
	};

	template<typename buffer_type, typename context_type, jsonifier::parse_options optionsNew> struct array_val_parser<buffer_type, context_type, optionsNew, true> {
		static constexpr jsonifier::parse_options options{ optionsNew };
		using base = derailleur<options, context_type>;

		template<jsonifier::concepts::vector_t value_type> JSONIFIER_INLINE static void impl(value_type& value, context_type& context) noexcept {
			if JSONIFIER_LIKELY (context.iter + 1 < context.endIter) {
				if JSONIFIER_LIKELY (*context.iter == lBracket) {
					++context.currentArrayDepth;
					++context.iter;

					if JSONIFIER_LIKELY (context.iter < context.endIter && *context.iter != rBracket) {
						size_t i	= 0;
						size_t size = value.size();

						for (; i < size; ++i) {
							parse<getJsonType<typename value_type::value_type>(), options, true>::template impl<buffer_type>(value[i], context);

							if JSONIFIER_LIKELY (context.iter < context.endIter) {
								if JSONIFIER_LIKELY (*context.iter == comma) {
									++context.iter;
								} else {
									if JSONIFIER_LIKELY (*context.iter == rBracket) {
										++context.iter;
										--context.currentArrayDepth;
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

						parse<getJsonType<typename value_type::value_type>(), options, true>::template impl<buffer_type>(value.emplace_back(), context);

						while (context.iter < context.endIter && *context.iter == comma) {
							++context.iter;
							parse<getJsonType<typename value_type::value_type>(), options, true>::template impl<buffer_type>(value.emplace_back(), context);
						}
						if JSONIFIER_LIKELY (*context.iter == rBracket) {
							++context.iter;
							--context.currentArrayDepth;
						} else {
							context.parserPtr->template reportError<parse_errors::Missing_Array_End>(context);
							base::skipToNextValue(context);
							return;
						}
					} else {
						++context.iter;
						--context.currentArrayDepth;
						return;
					}
				} else {
					context.parserPtr->template reportError<parse_errors::Missing_Array_Start>(context);
					base::skipToNextValue(context);
				}
			} else {
				context.parserPtr->template reportError<parse_errors::Unexpected_String_End>(context);
				base::skipToNextValue(context);
			}
		}


		template<jsonifier::concepts::raw_array_t value_type> JSONIFIER_INLINE static void impl(value_type& value, context_type& context) noexcept {
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

		template<jsonifier::concepts::tuple_t value_type> JSONIFIER_INLINE static void impl(value_type& value, context_type& context) noexcept {
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

		template<jsonifier::concepts::tuple_t value_type, size_t memberCount, size_t index> JSONIFIER_INLINE static void parseObjects(value_type& value, context_type& context) {
			if constexpr (index < memberCount) {
				if JSONIFIER_LIKELY ((context.iter < context.endIter) && *context.iter != rBracket) {
					if JSONIFIER_LIKELY (*context.iter == comma) {
						++context.iter;
						using member_type = decltype(get<index>(value));
						parse<getJsonType<member_type>(), options, true>::template impl<buffer_type>(get<index>(value), context);
						return parseObjects<value_type, memberCount, index + 1>(value, context);
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

	template<typename buffer_type, typename context_type, jsonifier::parse_options optionsNew, bool minified> struct string_val_parser {
		static constexpr jsonifier::parse_options options{ optionsNew };
		using base = derailleur<options, context_type>;

		template<jsonifier::concepts::string_t value_type> JSONIFIER_INLINE static void impl(value_type& value, context_type& context) noexcept {
			base::parseString(value, context);
			if constexpr (!minified) {
				JSONIFIER_SKIP_WS();
			}
		}

		template<jsonifier::concepts::char_t value_type> JSONIFIER_INLINE static void impl(value_type& value, context_type& context) noexcept {
			value = static_cast<value_type>(*(context.iter + 1));
			++context.iter;
			if constexpr (!minified) {
				JSONIFIER_SKIP_WS();
			}
		}
	};

	template<typename buffer_type, typename context_type, jsonifier::parse_options optionsNew, bool minified> struct number_val_parser {
		static constexpr jsonifier::parse_options options{ optionsNew };
		using base = derailleur<options, context_type>;

		template<jsonifier::concepts::enum_t value_type> JSONIFIER_INLINE static void impl(value_type& value, context_type& context) noexcept {
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

		template<jsonifier::concepts::num_t value_type> JSONIFIER_INLINE static void impl(value_type& value, context_type& context) noexcept {
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

	template<typename buffer_type, typename context_type, jsonifier::parse_options optionsNew, bool minified> struct bool_val_parser {
		static constexpr jsonifier::parse_options options{ optionsNew };
		using base = derailleur<options, context_type>;

		template<jsonifier::concepts::bool_t value_type> JSONIFIER_INLINE static void impl(value_type& value, context_type& context) noexcept {
			if JSONIFIER_LIKELY ((context.iter + 4) < context.endIter && parseBool(value, context.iter)) {
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

	template<typename buffer_type, typename context_type, jsonifier::parse_options optionsNew, bool minified> struct null_val_parser {
		static constexpr jsonifier::parse_options options{ optionsNew };
		using base = derailleur<options, context_type>;

		template<jsonifier::concepts::always_null_t value_type> JSONIFIER_INLINE static void impl(value_type&, context_type& context) noexcept {
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

	template<typename buffer_type, typename context_type, jsonifier::parse_options optionsNew, bool minified> struct accessor_val_parser {
		static constexpr jsonifier::parse_options options{ optionsNew };
		using base = derailleur<options, context_type>;

		template<jsonifier::json_type type, typename variant_type, size_t currentIndex = 0>
		JSONIFIER_INLINE static constexpr void iterateVariantTypes(variant_type&& variant, context_type& context) noexcept {
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

		template<jsonifier::concepts::variant_t value_type> JSONIFIER_INLINE static void impl(value_type& value, context_type& context) noexcept {
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

		template<jsonifier::concepts::optional_t value_type> JSONIFIER_INLINE static void impl(value_type& value, context_type& context) noexcept {
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
					base::skipToNextValue(context);
					return;
				}
			}
		}

		template<jsonifier::concepts::shared_ptr_t value_type> JSONIFIER_INLINE static void impl(value_type& value, context_type& context) noexcept {
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
					base::skipToNextValue(context);
					return;
				}
			}
		}

		template<jsonifier::concepts::unique_ptr_t value_type> JSONIFIER_INLINE static void impl(value_type& value, context_type& context) noexcept {
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
					base::skipToNextValue(context);
					return;
				}
			}
		}

		template<jsonifier::concepts::pointer_t value_type> JSONIFIER_INLINE static void impl(value_type& value, context_type& context) noexcept {
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
					base::skipToNextValue(context);
					return;
				}
			}
		}

		template<jsonifier::concepts::raw_json_t value_type> JSONIFIER_INLINE static void impl(value_type& value, context_type& context) noexcept {
			auto newPtr = context.iter;
			base::skipToNextValue(context);
			uint64_t newSize = context.iter - newPtr;
			if JSONIFIER_LIKELY (newSize > 0) {
				jsonifier::string newString{};
				newString.resize(static_cast<size_t>(newSize));
				std::memcpy(newString.data(), newPtr, static_cast<size_t>(newSize));
				value = value_type{ *context.parserPtr, newString };
			}
			return;
		}

		template<jsonifier::concepts::skip_t value_type> JSONIFIER_INLINE static void impl(value_type& value, context_type& context) noexcept {
			base::skipToNextValue(context);
		}
	};

	template<typename buffer_type, typename context_type, jsonifier::parse_options optionsNew, bool insideRepeated> struct object_val_parser_partial {
		static constexpr jsonifier::parse_options options{ optionsNew };
		using base = derailleur<options, context_type>;

		template<jsonifier::concepts::jsonifier_object_t value_type> JSONIFIER_INLINE static void impl(value_type& value, context_type& context) noexcept {
			if JSONIFIER_LIKELY (context.iter + 1 < context.endIter) {
				if JSONIFIER_LIKELY (**context.iter == lBrace) {
					++context.iter;
					++context.currentObjectDepth;
					parser_base_t<parse_types_partial, value_type, buffer_type, context_type, options, insideRepeated,
						jsonifier::core<std::remove_cvref_t<value_type>>::parseValue>::processIndices(value, context);
					base::template skipToEndOfValue<'{', '}'>(context);
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
				context.parserPtr->template reportError<parse_errors::Unexpected_String_End>(context);
				base::skipToNextValue(context);
				return;
			}
		}

		template<jsonifier::concepts::map_t value_type> JSONIFIER_INLINE static void impl(value_type& value, context_type& context) noexcept {
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
							base::skipToNextValue(context);
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

		template<jsonifier::concepts::tuple_t value_type> JSONIFIER_INLINE static void impl(value_type& value, context_type& context) noexcept {
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

		template<jsonifier::concepts::tuple_t value_type, size_t memberCount, size_t index> JSONIFIER_INLINE static void parseObjects(value_type& value, context_type& context) {
			if constexpr (index < memberCount) {
				if JSONIFIER_LIKELY ((context.iter < context.endIter) && **context.iter != rBracket) {
					if JSONIFIER_LIKELY (**context.iter == comma) {
						++context.iter;
						using member_type = decltype(get<index>(value));
						parse<getJsonType<member_type>(), options, insideRepeated>::template impl<buffer_type>(get<index>(value), context);
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

	template<typename buffer_type, typename context_type, jsonifier::parse_options optionsNew, bool insideRepeated> struct array_val_parser_partial {
		static constexpr jsonifier::parse_options options{ optionsNew };
		using base = derailleur<options, context_type>;

		template<jsonifier::concepts::vector_t value_type> JSONIFIER_INLINE static void impl(value_type& value, context_type& context) noexcept {
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

		template<jsonifier::concepts::raw_array_t value_type> JSONIFIER_INLINE static void impl(value_type& value, context_type& context) noexcept {
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

		template<jsonifier::concepts::tuple_t value_type> JSONIFIER_INLINE static void impl(value_type& value, context_type& context) noexcept {
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

		template<jsonifier::concepts::tuple_t value_type, size_t memberCount, size_t index> JSONIFIER_INLINE static void parseObjects(value_type& value, context_type& context) {
			if constexpr (index < memberCount) {
				if JSONIFIER_LIKELY ((context.iter < context.endIter) && **context.iter != rBracket) {
					if JSONIFIER_LIKELY (**context.iter == comma) {
						++context.iter;
						using member_type = decltype(get<index>(value));
						parse<getJsonType<member_type>(), options, insideRepeated>::template impl<buffer_type>(get<index>(value), context);
						return parseObjects<value_type, memberCount, index + 1>(value, context);
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

	template<typename buffer_type, typename context_type, jsonifier::parse_options optionsNew, bool insideRepeated> struct string_val_parser_partial {
		static constexpr jsonifier::parse_options options{ optionsNew };
		using base = derailleur<options, context_type>;

		template<jsonifier::concepts::string_t value_type> JSONIFIER_INLINE static void impl(value_type& value, context_type& context) noexcept {
			base::parseString(value, context);
		}

		template<jsonifier::concepts::char_t value_type> JSONIFIER_INLINE static void impl(value_type& value, context_type& context) noexcept {
			value = static_cast<value_type>(*(*context.iter + 1));
			++context.iter;
		}
	};

	template<typename buffer_type, typename context_type, jsonifier::parse_options optionsNew, bool insideRepeated> struct number_val_parser_partial {
		static constexpr jsonifier::parse_options options{ optionsNew };
		using base = derailleur<options, context_type>;

		template<jsonifier::concepts::enum_t value_type> JSONIFIER_INLINE static void impl(value_type& value, context_type& context) noexcept {
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

		template<jsonifier::concepts::num_t value_type> JSONIFIER_INLINE static void impl(value_type& value, context_type& context) noexcept {
			if JSONIFIER_LIKELY (parseNumber(value, static_cast<string_view_ptr>(*context.iter), static_cast<string_view_ptr>(*context.endIter))) {
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

	template<typename buffer_type, typename context_type, jsonifier::parse_options optionsNew, bool insideRepeated> struct bool_val_parser_partial {
		static constexpr jsonifier::parse_options options{ optionsNew };
		using base = derailleur<options, context_type>;

		template<jsonifier::concepts::bool_t value_type> JSONIFIER_INLINE static void impl(value_type& value, context_type& context) noexcept {
			auto newPtr = static_cast<string_view_ptr>(*context.iter);
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

	template<typename buffer_type, typename context_type, jsonifier::parse_options optionsNew, bool insideRepeated> struct null_val_parser_partial {
		static constexpr jsonifier::parse_options options{ optionsNew };
		using base = derailleur<options, context_type>;

		template<jsonifier::concepts::always_null_t value_type> JSONIFIER_INLINE static void impl(value_type& value, context_type& context) noexcept {
			auto newPtr = static_cast<string_view_ptr>(*context.iter);
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

	template<typename buffer_type, typename context_type, jsonifier::parse_options optionsNew, bool insideRepeated> struct accessor_val_parser_partial {
		static constexpr jsonifier::parse_options options{ optionsNew };
		using base = derailleur<options, context_type>;

		template<jsonifier::concepts::variant_t value_type> JSONIFIER_INLINE static void impl(value_type& value, context_type& context) noexcept {
			static constexpr auto lambda = [](auto&& valueNew, auto&& context) {
				using member_type = decltype(valueNew);
				return parse<getJsonType<member_type>(), options, insideRepeated>::template impl<buffer_type>(valueNew, context);
			};
			visit<lambda>(value, context);
		}

		template<jsonifier::concepts::optional_t value_type> JSONIFIER_INLINE static void impl(value_type& value, context_type& context) noexcept {
			if JSONIFIER_LIKELY ((context.iter < context.endIter) && **context.iter != n) {
				parse<getJsonType<typename value_type::value_type>(), options, insideRepeated>::template impl<buffer_type>(value.emplace(), context);
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

		template<jsonifier::concepts::shared_ptr_t value_type> JSONIFIER_INLINE static void impl(value_type& value, context_type& context) noexcept {
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
					base::skipToNextValue(context);
					return;
				}
			}
		}

		template<jsonifier::concepts::unique_ptr_t value_type> JSONIFIER_INLINE static void impl(value_type& value, context_type& context) noexcept {
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
					base::skipToNextValue(context);
					return;
				}
			}
		}

		template<jsonifier::concepts::pointer_t value_type> JSONIFIER_INLINE static void impl(value_type& value, context_type& context) noexcept {
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
					base::skipToNextValue(context);
					return;
				}
			}
		}

		template<jsonifier::concepts::raw_json_t value_type> JSONIFIER_INLINE static void impl(value_type& value, context_type& context) noexcept {
			auto newPtr = *context.iter;
			base::skipToNextValue(context);
			int64_t newSize = *context.iter - newPtr;
			if JSONIFIER_LIKELY (newSize > 0) {
				jsonifier::string newString{};
				newString.resize(static_cast<size_t>(newSize));
				std::memcpy(newString.data(), newPtr, static_cast<size_t>(newSize));
				value = value_type{ *context.parserPtr, newString };
			}
			return;
		}

		template<jsonifier::concepts::skip_t value_type> JSONIFIER_INLINE static void impl(value_type& value, context_type& context) noexcept {
			base::skipToNextValue(context);
		}
	};
}