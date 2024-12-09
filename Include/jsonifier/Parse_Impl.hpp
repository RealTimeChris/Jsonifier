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
	derailleur<options, parse_context_type>::template skipKey<value_type>(context); \
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
		derailleur<options, parse_context_type>::template skipToNextValue<value_type>(context); \
		return RETURN; \
	} \
	derailleur<options, parse_context_type>::template skipToNextValue<value_type>(context);

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

	template<bool minified, jsonifier::parse_options options, typename parse_context_type, typename buffer_type, typename value_type> struct index_processor_parse;

	template<jsonifier::parse_options options, typename parse_context_type, typename buffer_type, typename value_type>
	struct index_processor_parse<false, options, parse_context_type, buffer_type, value_type> : derailleur<options, parse_context_type> {
		using base						  = derailleur<options, parse_context_type>;
		static constexpr auto memberCount = tuple_size_v<typename core_tuple_type<value_type>::core_type>;
		inline static thread_local std::array<bool, memberCount> antiHashStates{ [] {
			std::array<bool, memberCount> returnValues{};
			returnValues.fill(true);
			return returnValues;
		}() };

		template<size_t index> JSONIFIER_ALWAYS_INLINE static bool processIndex(value_type& value, parse_context_type& context) noexcept {
			if constexpr (index < tuple_size_v<typename core_tuple_type<value_type>::core_type>) {
				static constexpr auto ptr			= get<index>(core_tuple_type<value_type>::coreTupleV).ptr();
				static constexpr auto key			= get<index>(core_tuple_type<value_type>::coreTupleV).view();
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
						parse<false, options>::template impl<buffer_type>(value.*ptr, context);
						return true;
					}
					JSONIFIER_ELSE_UNLIKELY(else) {
						context.parserPtr->template reportError<parse_errors::Missing_Colon>(context);
						derailleur<options, parse_context_type>::template skipToNextValue<value_type>(context);
					}
				} else {
					JSONIFIER_SKIP_KEY_VALUE(false);
				}
			}
			return false;
		}

		template<size_t... indices> static constexpr auto generateFunctionPtrs(std::index_sequence<indices...>) noexcept {
			using function_type = decltype(&index_processor_parse<false, options, parse_context_type, buffer_type, value_type>::template processIndex<0>);
			return std::array<function_type, sizeof...(indices)>{
				{ &index_processor_parse<false, options, parse_context_type, buffer_type, value_type>::template processIndex<indices>... }
			};
		}

		template<size_t... indices>
		JSONIFIER_INLINE static bool processIndexImpl(value_type& value, parse_context_type& context, size_t index, std::index_sequence<indices...>) noexcept {
			if constexpr (sizeof...(indices) <= 8) {
				return ((index == indices ? index_processor_parse<false, options, parse_context_type, buffer_type, value_type>::template processIndex<indices>(value, context)
										  : false) ||
					...);
			} else {
				static constexpr auto parsePtrs = generateFunctionPtrs(std::make_index_sequence<sizeof...(indices)>{});
				return parsePtrs[index](value, context);
			}
		}

		template<size_t index> JSONIFIER_INLINE static auto processIndexLambda(value_type& value, parse_context_type& context, string_view_ptr& wsStart, size_t& wsSize) noexcept {
			( void )value, ( void )context, ( void )wsStart, ( void )wsSize;
			static constexpr bool newLines{ true };
			if constexpr (index < memberCount) {
				if JSONIFIER_LIKELY ((context.iter < context.endIter) && *context.iter != rBrace) {
					if constexpr (index > 0 || options.partialRead) {
						if JSONIFIER_LIKELY (*context.iter == comma) {
							++context.iter;
							JSONIFIER_SKIP_MATCHING_WS();
						}
						JSONIFIER_ELSE_UNLIKELY(else) {
							if constexpr (!options.partialRead) {
								context.parserPtr->template reportError<parse_errors::Missing_Comma_Or_Object_End>(context);
								base::template skipToNextValue<value_type>(context);
								return;
							}
						}
					}

					if JSONIFIER_LIKELY ((context.iter < context.endIter) && *context.iter == quote) {
						++context.iter;
						if constexpr (options.knownOrder) {
							if (antiHashStates[index]) {
								static constexpr auto ptr			= get<index>(jsonifier::concepts::coreV<value_type>).ptr();
								static constexpr auto key			= get<index>(jsonifier::concepts::coreV<value_type>).view();
								static constexpr auto stringLiteral = stringLiteralFromView<key.size()>(key);
								static constexpr auto keySize		= key.size();
								static constexpr auto keySizeNew	= keySize + 1;
								if JSONIFIER_LIKELY (((context.iter + keySize) < context.endIter) && (*(context.iter + keySize) == quote) &&
									string_literal_comparitor<decltype(stringLiteral), stringLiteral>::impl(context.iter)) {
									context.iter += keySizeNew;
									JSONIFIER_SKIP_WS();
									if JSONIFIER_LIKELY ((context.iter < context.endIter) && *context.iter == colon) {
										++context.iter;
										JSONIFIER_SKIP_WS();
										if constexpr (jsonifier::concepts::has_excluded_keys<value_type>) {
											auto& keys = value.jsonifierExcludedKeys;
											if JSONIFIER_LIKELY (keys.find(static_cast<typename std::remove_cvref_t<decltype(keys)>::key_type>(key)) != keys.end()) {
												base::template skipToNextValue<value_type>(context);
											}
										}
										parse<false, options>::template impl<buffer_type>(value.*ptr, context);
										return;
									}
									JSONIFIER_ELSE_UNLIKELY(else) {
										context.parserPtr->template reportError<parse_errors::Missing_Colon>(context);
										base::template skipToNextValue<value_type>(context);
										return;
									}
								} else {
									antiHashStates[index] = false;
								}
							}
						}
						if JSONIFIER_LIKELY (auto indexNew = hash_map<value_type, std::remove_cvref_t<decltype(context.iter)>>::findIndex(context.iter, context.endIter);
											 indexNew < memberCount) {
							if JSONIFIER_LIKELY (processIndexImpl(value, context, indexNew, std::make_index_sequence<memberCount>{})) {
								return;
							}
							JSONIFIER_ELSE_UNLIKELY(else) {
								if constexpr (options.partialRead) {
									return processIndexLambda<index>(value, context, wsStart, wsSize);
								} else {
									JSONIFIER_SKIP_KEY_VALUE();
								}
								return;
							}
						} else {
							JSONIFIER_SKIP_KEY_VALUE();
							return;
						}
					}
					JSONIFIER_ELSE_UNLIKELY(else) {
						context.parserPtr->template reportError<parse_errors::Missing_String_Start>(context);
						JSONIFIER_SKIP_KEY_VALUE();
						return;
					}
				}
				return;
			}
		}

		template<typename... arg_types, size_t... indices> JSONIFIER_INLINE static void executeIndices(std::index_sequence<indices...>, arg_types&&... args) {
			(( void )(args), ...);
			(processIndexLambda<indices>(std::forward<arg_types>(args)...), ...);
		}
	};

	template<jsonifier::parse_options options, typename parse_context_type, typename buffer_type, typename value_type>
	struct index_processor_parse<true, options, parse_context_type, buffer_type, value_type> : derailleur<options, parse_context_type> {
		using base						  = derailleur<options, parse_context_type>;
		static constexpr auto memberCount = tuple_size_v<typename core_tuple_type<value_type>::core_type>;
		inline static thread_local std::array<bool, memberCount> antiHashStates{ [] {
			std::array<bool, memberCount> returnValues{};
			returnValues.fill(true);
			return returnValues;
		}() };

		template<size_t index> JSONIFIER_ALWAYS_INLINE static bool processIndex(value_type& value, parse_context_type& context) noexcept {
			if constexpr (index < tuple_size_v<typename core_tuple_type<value_type>::core_type>) {
				static constexpr auto ptr			= get<index>(core_tuple_type<value_type>::coreTupleV).ptr();
				static constexpr auto key			= get<index>(core_tuple_type<value_type>::coreTupleV).view();
				static constexpr auto stringLiteral = stringLiteralFromView<key.size()>(key);
				static constexpr auto keySize		= key.size();
				static constexpr auto keySizeNew	= keySize + 1;

				if JSONIFIER_LIKELY (((context.iter + keySize) < context.endIter) && string_literal_comparitor<decltype(stringLiteral), stringLiteral>::impl(context.iter)) {
					context.iter += keySizeNew;
					if JSONIFIER_LIKELY ((context.iter < context.endIter) && *context.iter == colon) {
						++context.iter;
						if constexpr (jsonifier::concepts::has_excluded_keys<value_type>) {
							auto& keys = value.jsonifierExcludedKeys;
							if JSONIFIER_LIKELY (keys.find(static_cast<typename std::remove_cvref_t<decltype(keys)>::key_type>(key)) != keys.end()) {
								derailleur<options, parse_context_type>::template skipToNextValue<value_type>(context);
								return true;
							}
						}
						parse<true, options>::template impl<buffer_type>(value.*ptr, context);
						return true;
					}
					JSONIFIER_ELSE_UNLIKELY(else) {
						context.parserPtr->template reportError<parse_errors::Missing_Colon>(context);
						derailleur<options, parse_context_type>::template skipToNextValue<value_type>(context);
					}
				} else {
					JSONIFIER_SKIP_KEY_VALUE(false);
				}
			}
			return false;
		}

		template<size_t... indices> static constexpr auto generateFunctionPtrs(std::index_sequence<indices...>) noexcept {
			using function_type = decltype(&index_processor_parse<true, options, parse_context_type, buffer_type, value_type>::template processIndex<0>);
			return std::array<function_type, sizeof...(indices)>{
				{ &index_processor_parse<true, options, parse_context_type, buffer_type, value_type>::template processIndex<indices>... }
			};
		}

		template<size_t... indices>
		JSONIFIER_INLINE static bool processIndexImpl(value_type& value, parse_context_type& context, size_t index, std::index_sequence<indices...>) noexcept {
			if constexpr (sizeof...(indices) <= 8) {
				return ((index == indices ? index_processor_parse<true, options, parse_context_type, buffer_type, value_type>::template processIndex<indices>(value, context)
										  : false) ||
					...);
			} else {
				static constexpr auto parsePtrs = generateFunctionPtrs(std::make_index_sequence<sizeof...(indices)>{});
				return parsePtrs[index](value, context);
			}
		}

		template<size_t index> JSONIFIER_INLINE static auto processIndexLambda(value_type& value, parse_context_type& context) noexcept {
			( void )value, ( void )context;
			if constexpr (index < memberCount) {
				if JSONIFIER_LIKELY ((context.iter < context.endIter) && *context.iter != rBrace) {
					if constexpr (index > 0 || options.partialRead) {
						if JSONIFIER_LIKELY (*context.iter == comma) {
							++context.iter;
						}
						JSONIFIER_ELSE_UNLIKELY(else) {
							if constexpr (!options.partialRead) {
								context.parserPtr->template reportError<parse_errors::Missing_Comma_Or_Object_End>(context);
								base::template skipToNextValue<value_type>(context);
								return;
							}
						}
					}

					if JSONIFIER_LIKELY ((context.iter < context.endIter) && *context.iter == quote) {
						++context.iter;
						if constexpr (options.knownOrder) {
							if (antiHashStates[index]) {
								static constexpr auto ptr			= get<index>(jsonifier::concepts::coreV<value_type>).ptr();
								static constexpr auto key			= get<index>(jsonifier::concepts::coreV<value_type>).view();
								static constexpr auto stringLiteral = stringLiteralFromView<key.size()>(key);
								static constexpr auto keySize		= key.size();
								static constexpr auto keySizeNew	= keySize + 1;
								if JSONIFIER_LIKELY (((context.iter + keySize) < context.endIter) && (*(context.iter + keySize) == quote) &&
									string_literal_comparitor<decltype(stringLiteral), stringLiteral>::impl(context.iter)) {
									context.iter += keySizeNew;
									if JSONIFIER_LIKELY ((context.iter < context.endIter) && *context.iter == colon) {
										++context.iter;
										if constexpr (jsonifier::concepts::has_excluded_keys<value_type>) {
											auto& keys = value.jsonifierExcludedKeys;
											if JSONIFIER_LIKELY (keys.find(static_cast<typename std::remove_cvref_t<decltype(keys)>::key_type>(key)) != keys.end()) {
												base::template skipToNextValue<value_type>(context);
											}
										}
										parse<true, options>::template impl<buffer_type>(value.*ptr, context);
										return;
									}
									JSONIFIER_ELSE_UNLIKELY(else) {
										context.parserPtr->template reportError<parse_errors::Missing_Colon>(context);
										base::template skipToNextValue<value_type>(context);
										return;
									}
								} else {
									antiHashStates[index] = false;
								}
							}
						}
						if JSONIFIER_LIKELY (auto indexNew = hash_map<value_type, std::remove_cvref_t<decltype(context.iter)>>::findIndex(context.iter, context.endIter);
											 indexNew < memberCount) {
							if JSONIFIER_LIKELY (processIndexImpl(value, context, indexNew, std::make_index_sequence<memberCount>{})) {
								return;
							}
							JSONIFIER_ELSE_UNLIKELY(else) {
								if constexpr (options.partialRead) {
									return processIndexLambda<index>(value, context);
								} else {
									JSONIFIER_SKIP_KEY_VALUE();
								}
								return;
							}
						} else {
							JSONIFIER_SKIP_KEY_VALUE();
							return;
						}
					}
					JSONIFIER_ELSE_UNLIKELY(else) {
						context.parserPtr->template reportError<parse_errors::Missing_String_Start>(context);
						base::template skipToNextValue<value_type>(context);
						JSONIFIER_SKIP_KEY_VALUE();
						return;
					}
				}
				return;
			}
		}

		template<typename... arg_types, size_t... indices> JSONIFIER_INLINE static void executeIndices(std::index_sequence<indices...>, arg_types&&... args) {
			(( void )(args), ...);
			(processIndexLambda<indices>(std::forward<arg_types>(args)...), ...);
		}
	};

	template<jsonifier::parse_options optionsNew, jsonifier::concepts::jsonifier_object_t value_type, typename buffer_type, typename parse_context_type>
	struct parse_impl<false, optionsNew, value_type, buffer_type, parse_context_type> : derailleur<optionsNew, parse_context_type> {
		static constexpr jsonifier::parse_options options{ optionsNew };
		using base						  = derailleur<options, parse_context_type>;
		static constexpr auto memberCount = tuple_size_v<typename core_tuple_type<value_type>::core_type>;
		JSONIFIER_ALWAYS_INLINE static void impl(value_type& value, parse_context_type& context) noexcept {
			if JSONIFIER_LIKELY (context.iter + 1 < context.endIter) {
				if JSONIFIER_LIKELY (*context.iter == lBrace) {
					++context.iter;
					++context.currentObjectDepth;
					string_view_ptr wsStart = context.iter;
					JSONIFIER_SKIP_WS();
					size_t wsSize = static_cast<size_t>(context.iter - wsStart);
					index_processor_parse<false, options, parse_context_type, buffer_type, value_type>::executeIndices(std::make_index_sequence<memberCount>{}, value, context,
						wsStart, wsSize);
					if JSONIFIER_LIKELY (*context.iter == rBrace) {
						++context.iter;
						JSONIFIER_SKIP_WS();
					}
					JSONIFIER_ELSE_UNLIKELY(else) {
						if constexpr (!options.partialRead) {
							context.parserPtr->template reportError<parse_errors::Missing_Comma_Or_Object_End>(context);
							base::template skipToNextValue<value_type>(context);
						}
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
	};

	template<jsonifier::parse_options options, jsonifier::concepts::jsonifier_object_t value_type, typename buffer_type, typename parse_context_type>
	struct parse_impl<true, options, value_type, buffer_type, parse_context_type> : derailleur<options, parse_context_type> {
		using base						  = derailleur<options, parse_context_type>;
		static constexpr auto memberCount = tuple_size_v<typename core_tuple_type<value_type>::core_type>;
		JSONIFIER_ALWAYS_INLINE static void impl(value_type& value, parse_context_type& context) noexcept {
			if JSONIFIER_LIKELY (context.iter + 1 < context.endIter) {
				if JSONIFIER_LIKELY (*context.iter == lBrace) {
					++context.iter;
					++context.currentObjectDepth;
					index_processor_parse<true, options, parse_context_type, buffer_type, value_type>::executeIndices(std::make_index_sequence<memberCount>{}, value, context);
					if JSONIFIER_LIKELY (*context.iter == rBrace) {
						++context.iter;
					}
					JSONIFIER_ELSE_UNLIKELY(else) {
						if constexpr (!options.partialRead) {
							context.parserPtr->template reportError<parse_errors::Missing_Comma_Or_Object_End>(context);
							base::template skipToNextValue<value_type>(context);
						}
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
	};

	template<bool minified, jsonifier::parse_options options, jsonifier::concepts::jsonifier_scalar_value_t value_type, typename buffer_type, typename parse_context_type>
	struct parse_impl<minified, options, value_type, buffer_type, parse_context_type> : derailleur<options, parse_context_type> {
		using base = derailleur<options, parse_context_type>;
		JSONIFIER_ALWAYS_INLINE static void impl(value_type& value, parse_context_type& context) noexcept {
			static constexpr auto size{ tuple_size_v<typename core_tuple_type<value_type>::core_type> };
			if constexpr (size == 1) {
				static constexpr auto newPtr = get<0>(core_tuple_type<value_type>::coreTupleV);
				parse<minified, options>::template impl<buffer_type>(getMember<newPtr>(value), context);
			}
		}
	};

	template<jsonifier::parse_options options, jsonifier::concepts::tuple_t value_type, typename buffer_type, typename parse_context_type>
	struct parse_impl<false, options, value_type, buffer_type, parse_context_type> : derailleur<options, parse_context_type> {
		using base = derailleur<options, parse_context_type>;
		JSONIFIER_ALWAYS_INLINE static void impl(value_type& value, parse_context_type& context) noexcept {
			static constexpr auto memberCount = tuple_size_v<std::remove_cvref_t<value_type>>;
			if JSONIFIER_LIKELY (context.iter + 1 < context.endIter) {
				if JSONIFIER_LIKELY (*context.iter == lBracket) {
					++context.iter;
					++context.currentArrayDepth;
					if JSONIFIER_LIKELY (*context.iter != rBracket) {
						if constexpr (memberCount > 0) {
							const auto wsStart = context.iter;
							JSONIFIER_SKIP_WS();
							size_t wsSize{ static_cast<size_t>(context.iter - wsStart) };
							parse<false, options>::template impl<buffer_type>(get<0>(value), context);

							if (whitespaceTable[static_cast<uint8_t>(*(context.iter + wsSize))]) {
								parseObjects<memberCount, 1, true>(value, context, wsStart, wsSize);
							} else {
								parseObjects<memberCount, 1, false>(value, context, wsStart, wsSize);
							}
						}
					}
					++context.iter;
					JSONIFIER_SKIP_WS();
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

		template<size_t memberCount, size_t index, bool newLines>
		JSONIFIER_ALWAYS_INLINE static void parseObjects(value_type& value, parse_context_type& context, const auto wsStart = {}, size_t wsSize = {}) {
			if constexpr (index < memberCount) {
				if JSONIFIER_LIKELY ((context.iter < context.endIter) && *context.iter != rBracket) {
					if JSONIFIER_LIKELY (*context.iter == comma) {
						++context.iter;
						JSONIFIER_SKIP_MATCHING_WS();
						parse<false, options>::template impl<buffer_type>(get<index>(value), context);
						return parseObjects<memberCount, index + 1, newLines>(value, context, wsStart, wsSize);
					} else {
						context.parserPtr->template reportError<parse_errors::Missing_Comma_Or_Object_End>(context);
						base::template skipToNextValue<value_type>(context);
						return;
					}
				} else {
					++context.iter;
					JSONIFIER_SKIP_WS();
				}
			} else {
				++context.iter;
				JSONIFIER_SKIP_WS();
			}
		}
	};

	template<jsonifier::parse_options options, jsonifier::concepts::tuple_t value_type, typename buffer_type, typename parse_context_type>
	struct parse_impl<true, options, value_type, buffer_type, parse_context_type> : derailleur<options, parse_context_type> {
		using base = derailleur<options, parse_context_type>;
		JSONIFIER_ALWAYS_INLINE static void impl(value_type& value, parse_context_type& context) noexcept {
			static constexpr auto memberCount = tuple_size_v<std::remove_cvref_t<value_type>>;
			if JSONIFIER_LIKELY (context.iter + 1 < context.endIter) {
				if JSONIFIER_LIKELY (*context.iter == lBracket) {
					++context.iter;
					++context.currentArrayDepth;
					if JSONIFIER_LIKELY (*context.iter != rBracket) {
						if constexpr (memberCount > 0) {
							parse<true, options>::template impl<buffer_type>(get<0>(value), context);
							parseObjects<memberCount, 1>(value, context);
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

		template<size_t memberCount, size_t index> JSONIFIER_ALWAYS_INLINE static void parseObjects(value_type& value, parse_context_type& context) {
			if constexpr (index < memberCount) {
				if JSONIFIER_LIKELY ((context.iter < context.endIter) && *context.iter != rBracket) {
					if JSONIFIER_LIKELY (*context.iter == comma) {
						++context.iter;
						parse<true, options>::template impl<buffer_type>(get<index>(value), context);
						return parseObjects<memberCount, index + 1>(value, context);
					} else {
						context.parserPtr->template reportError<parse_errors::Missing_Comma_Or_Object_End>(context);
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

	template<jsonifier::parse_options options, jsonifier::concepts::map_t value_type, typename buffer_type, typename parse_context_type>
	struct parse_impl<false, options, value_type, buffer_type, parse_context_type> : derailleur<options, parse_context_type> {
		using base = derailleur<options, parse_context_type>;
		JSONIFIER_ALWAYS_INLINE static void impl(value_type& value, parse_context_type& context) noexcept {
			if JSONIFIER_LIKELY (context.iter + 1 < context.endIter) {
				if JSONIFIER_LIKELY (*context.iter == lBrace) {
					++context.iter;
					++context.currentObjectDepth;
					if JSONIFIER_LIKELY (*context.iter != rBrace) {
						const auto wsStart = context.iter;
						JSONIFIER_SKIP_WS();
						size_t wsSize{ static_cast<size_t>(context.iter - wsStart) };
						static thread_local typename std::remove_cvref_t<value_type>::key_type key{};
						parse<false, options>::template impl<buffer_type>(key, context);

						if JSONIFIER_LIKELY ((context.iter < context.endIter) && *context.iter == colon) {
							++context.iter;
							JSONIFIER_SKIP_WS();
							parse<false, options>::template impl<buffer_type>(value[key], context);
						}
						JSONIFIER_ELSE_UNLIKELY(else) {
							context.parserPtr->template reportError<parse_errors::Missing_Colon>(context);
							base::template skipToNextValue<value_type>(context);
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

		template<bool newLines> JSONIFIER_ALWAYS_INLINE static void parseObjects(value_type& value, parse_context_type& context, const auto wsStart = {}, size_t wsSize = {}) {
			while
				JSONIFIER_LIKELY((context.iter < context.endIter) && *context.iter != rBrace) {
					if (*context.iter == comma) {
						++context.iter;
						JSONIFIER_SKIP_MATCHING_WS();
						static thread_local typename std::remove_cvref_t<value_type>::key_type key{};
						parse<false, options>::template impl<buffer_type>(key, context);

						if JSONIFIER_LIKELY ((context.iter < context.endIter) && *context.iter == colon) {
							++context.iter;
							JSONIFIER_SKIP_WS();
							parse<false, options>::template impl<buffer_type>(value[key], context);
						} else {
							context.parserPtr->template reportError<parse_errors::Missing_Colon>(context);
							base::template skipToNextValue<value_type>(context);
							return;
						}
					} else {
						context.parserPtr->template reportError<parse_errors::Missing_Comma_Or_Object_End>(context);
						base::template skipToNextValue<value_type>(context);
						return;
					}
				}
			++context.iter;
			--context.currentObjectDepth;
			JSONIFIER_SKIP_WS();
		}
	};

	template<jsonifier::parse_options options, jsonifier::concepts::map_t value_type, typename buffer_type, typename parse_context_type>
	struct parse_impl<true, options, value_type, buffer_type, parse_context_type> : derailleur<options, parse_context_type> {
		using base = derailleur<options, parse_context_type>;
		JSONIFIER_ALWAYS_INLINE static void impl(value_type& value, parse_context_type& context) noexcept {
			if JSONIFIER_LIKELY (context.iter + 1 < context.endIter) {
				if JSONIFIER_LIKELY (*context.iter == lBrace) {
					++context.iter;
					++context.currentObjectDepth;
					if JSONIFIER_LIKELY (*context.iter != rBrace) {
						static thread_local typename std::remove_cvref_t<value_type>::key_type key{};
						parse<true, options>::template impl<buffer_type>(key, context);

						if JSONIFIER_LIKELY ((context.iter < context.endIter) && *context.iter == colon) {
							++context.iter;
							parse<true, options>::template impl<buffer_type>(value[key], context);
						}
						JSONIFIER_ELSE_UNLIKELY(else) {
							context.parserPtr->template reportError<parse_errors::Missing_Colon>(context);
							base::template skipToNextValue<value_type>(context);
							return;
						}

						while ((context.iter < context.endIter) && *context.iter != rBrace) {
							if JSONIFIER_LIKELY (*context.iter == comma) {
								++context.iter;
								parse<true, options>::template impl<buffer_type>(key, context);

								if JSONIFIER_LIKELY ((context.iter < context.endIter) && *context.iter == colon) {
									++context.iter;
									parse<true, options>::template impl<buffer_type>(value[key], context);
								} else {
									context.parserPtr->template reportError<parse_errors::Missing_Colon>(context);
									base::template skipToNextValue<value_type>(context);
									return;
								}
							} else {
								context.parserPtr->template reportError<parse_errors::Missing_Comma_Or_Object_End>(context);
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
	};

	template<bool minified, jsonifier::parse_options options, jsonifier::concepts::variant_t value_type, typename buffer_type, typename parse_context_type>
	struct parse_impl<minified, options, value_type, buffer_type, parse_context_type> : derailleur<options, parse_context_type> {
		using base = derailleur<options, parse_context_type>;
		JSONIFIER_ALWAYS_INLINE static void impl(value_type& value, parse_context_type& context) noexcept {
			static constexpr auto lambda = [](auto&& valueNew, auto&& value, auto&& context) {
				return parse<minified, options>::template impl<buffer_type>(value, context);
			};
			visit<lambda>(value, value, context);
		}
	};

	template<bool minified, jsonifier::parse_options options, jsonifier::concepts::optional_t value_type, typename buffer_type, typename parse_context_type>
	struct parse_impl<minified, options, value_type, buffer_type, parse_context_type> : derailleur<options, parse_context_type> {
		using base = derailleur<options, parse_context_type>;
		JSONIFIER_ALWAYS_INLINE static void impl(value_type& value, parse_context_type& context) noexcept {
			if JSONIFIER_LIKELY ((context.iter < context.endIter) && *context.iter != n) {
				parse<minified, options>::template impl<buffer_type>(value.emplace(), context);
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
	};

	JSONIFIER_ALWAYS_INLINE void noop() noexcept {};

	template<jsonifier::parse_options options, jsonifier::concepts::vector_t value_type, typename buffer_type, typename parse_context_type>
	struct parse_impl<false, options, value_type, buffer_type, parse_context_type> : derailleur<options, parse_context_type> {
		using base = derailleur<options, parse_context_type>;
		JSONIFIER_ALWAYS_INLINE static void impl(value_type& value, parse_context_type& context) noexcept {
			if JSONIFIER_LIKELY (context.iter + 1 < context.endIter) {
				if JSONIFIER_LIKELY (*context.iter == lBracket) {
					++context.currentArrayDepth;
					++context.iter;
					if JSONIFIER_LIKELY (*context.iter != rBracket) {
						const auto wsStart = context.iter;
						JSONIFIER_SKIP_WS();
						size_t wsSize{ static_cast<size_t>(context.iter - wsStart) };
						parseObjects<true>(value, context, wsStart, wsSize);
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

		template<bool newLines> JSONIFIER_ALWAYS_INLINE static void parseObjects(value_type& value, parse_context_type& context, const auto wsStart = {}, size_t wsSize = {}) {
			if JSONIFIER_LIKELY (const size_t size = value.size(); size > 0) {
				auto iterNew = value.begin();

				for (size_t i = 0; i < size; ++i) {
					parse<false, options>::template impl<buffer_type>(*(iterNew++), context);

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
								return (value.size() == i + 1) ? noop() : value.resize(i + 1);
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
				parse<false, options>::template impl<buffer_type>(value.emplace_back(), context);
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
	};

	template<jsonifier::parse_options options, jsonifier::concepts::vector_t value_type, typename buffer_type, typename parse_context_type>
	struct parse_impl<true, options, value_type, buffer_type, parse_context_type> : derailleur<options, parse_context_type> {
		using base = derailleur<options, parse_context_type>;
		JSONIFIER_ALWAYS_INLINE static void impl(value_type& value, parse_context_type& context) noexcept {
			if JSONIFIER_LIKELY (context.iter + 1 < context.endIter) {
				if JSONIFIER_LIKELY ((context.iter < context.endIter) && *context.iter == lBracket) {
					++context.currentArrayDepth;
					++context.iter;
					if JSONIFIER_LIKELY ((context.iter < context.endIter) && *context.iter != rBracket) {
						if JSONIFIER_LIKELY (const size_t size = value.size(); size > 0) {
							auto iterNew = value.begin();

							for (size_t i = 0; i < size; ++i) {
								parse<true, options>::template impl<buffer_type>(*(iterNew++), context);

								if JSONIFIER_LIKELY ((context.iter < context.endIter) && *context.iter == comma) {
									++context.iter;
								}
								JSONIFIER_ELSE_UNLIKELY(else) {
									if JSONIFIER_LIKELY ((context.iter < context.endIter) && *context.iter == rBracket) {
										++context.iter;
										--context.currentArrayDepth;
										return (value.size() == i + 1) ? noop() : value.resize(i + 1);
									}
									JSONIFIER_ELSE_UNLIKELY(else) {
										context.parserPtr->template reportError<parse_errors::Imbalanced_Array_Brackets>(context);
										base::template skipToNextValue<value_type>(context);
										return;
									}
								}
							}
						}

						while (true) {
							parse<true, options>::template impl<buffer_type>(value.emplace_back(), context);
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
	};

	template<jsonifier::parse_options options, jsonifier::concepts::raw_array_t value_type, typename buffer_type, typename parse_context_type>
	struct parse_impl<false, options, value_type, buffer_type, parse_context_type> : derailleur<options, parse_context_type> {
		using base = derailleur<options, parse_context_type>;
		JSONIFIER_ALWAYS_INLINE static void impl(value_type& value, parse_context_type& context) noexcept {
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

		template<bool newLines> JSONIFIER_ALWAYS_INLINE static void parseObjects(value_type& value, parse_context_type& context, const auto wsStart = {}, size_t wsSize = {}) {
			if JSONIFIER_LIKELY (const size_t n = std::size(value); n > 0) {
				auto iterNew = std::begin(value);

				for (size_t i = 0; i < n; ++i) {
					parse<false, options>::template impl<buffer_type>(*(iterNew++), context);

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
	};

	template<jsonifier::parse_options options, jsonifier::concepts::raw_array_t value_type, typename buffer_type, typename parse_context_type>
	struct parse_impl<true, options, value_type, buffer_type, parse_context_type> : derailleur<options, parse_context_type> {
		using base = derailleur<options, parse_context_type>;
		JSONIFIER_ALWAYS_INLINE static void impl(value_type& value, parse_context_type& context) noexcept {
			if JSONIFIER_LIKELY (context.iter + 1 < context.endIter) {
				if JSONIFIER_LIKELY (*context.iter == lBracket) {
					++context.currentArrayDepth;
					++context.iter;
					if JSONIFIER_LIKELY (*context.iter != rBracket) {
						auto iterNew = std::begin(value);

						for (size_t i = 0; i < value.size(); ++i) {
							parse<true, options>::template impl<buffer_type>(*(iterNew++), context);

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
	};

	template<bool minified, jsonifier::parse_options options, jsonifier::concepts::string_t value_type, typename buffer_type, typename parse_context_type>
	struct parse_impl<minified, options, value_type, buffer_type, parse_context_type> : derailleur<options, parse_context_type> {
		using base = derailleur<options, parse_context_type>;
		JSONIFIER_ALWAYS_INLINE static void impl(value_type& value, parse_context_type& context) noexcept {
			base::parseString(value, context);
			if constexpr (!minified) {
				JSONIFIER_SKIP_WS();
			}
		}
	};

	template<bool minified, jsonifier::parse_options options, jsonifier::concepts::char_t value_type, typename buffer_type, typename parse_context_type>
	struct parse_impl<minified, options, value_type, buffer_type, parse_context_type> : derailleur<options, parse_context_type> {
		using base = derailleur<options, parse_context_type>;
		JSONIFIER_ALWAYS_INLINE static void impl(value_type& value, parse_context_type& context) noexcept {
			value = static_cast<value_type>(*(static_cast<string_view_ptr>(context.iter) + 1));
			++context.iter;
			if constexpr (!minified) {
				JSONIFIER_SKIP_WS();
			}
		}
	};

	template<bool minified, jsonifier::parse_options options, jsonifier::concepts::raw_json_t value_type, typename buffer_type, typename parse_context_type>
	struct parse_impl<minified, options, value_type, buffer_type, parse_context_type> : derailleur<options, parse_context_type> {
		using base = derailleur<options, parse_context_type>;
		JSONIFIER_ALWAYS_INLINE static void impl(value_type& value, parse_context_type& context) noexcept {
			auto newPtr = static_cast<string_view_ptr>(context.iter);
			base::template skipToNextValue<value_type>(context);
			int64_t newSize = static_cast<string_view_ptr>(context.iter) - newPtr;
			if JSONIFIER_LIKELY (newSize > 0) {
				jsonifier::string newString{};
				newString.resize(static_cast<size_t>(newSize));
				std::memcpy(newString.data(), newPtr, static_cast<size_t>(newSize));
				value = value_type{ *context.parserPtr, newString };
			}
			return;
		}
	};

	template<bool minified, jsonifier::parse_options options, jsonifier::concepts::shared_ptr_t value_type, typename buffer_type, typename parse_context_type>
	struct parse_impl<minified, options, value_type, buffer_type, parse_context_type> : derailleur<options, parse_context_type> {
		using base = derailleur<options, parse_context_type>;
		JSONIFIER_ALWAYS_INLINE static void impl(value_type& value, parse_context_type& context) noexcept {
			if JSONIFIER_LIKELY ((context.iter < context.endIter) && *context.iter != n) {
				using member_type = decltype(*value);
				if JSONIFIER_UNLIKELY (!value) {
					value = std::make_shared<std::remove_pointer_t<std::remove_cvref_t<member_type>>>();
				}
				parse<minified, options>::template impl<buffer_type>(*value, context);
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
	};

	template<bool minified, jsonifier::parse_options options, jsonifier::concepts::unique_ptr_t value_type, typename buffer_type, typename parse_context_type>
	struct parse_impl<minified, options, value_type, buffer_type, parse_context_type> : derailleur<options, parse_context_type> {
		using base = derailleur<options, parse_context_type>;
		JSONIFIER_ALWAYS_INLINE static void impl(value_type& value, parse_context_type& context) noexcept {
			if JSONIFIER_LIKELY ((context.iter < context.endIter) && *context.iter != n) {
				using member_type = decltype(*value);
				if JSONIFIER_UNLIKELY (!value) {
					value = std::make_unique<std::remove_pointer_t<std::remove_cvref_t<member_type>>>();
				}
				parse<minified, options>::template impl<buffer_type>(*value, context);
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
	};

	template<bool minified, jsonifier::parse_options options, jsonifier::concepts::pointer_t value_type, typename buffer_type, typename parse_context_type>
	struct parse_impl<minified, options, value_type, buffer_type, parse_context_type> : derailleur<options, parse_context_type> {
		using base = derailleur<options, parse_context_type>;
		JSONIFIER_ALWAYS_INLINE static void impl(value_type& value, parse_context_type& context) noexcept {
			if JSONIFIER_LIKELY ((context.iter < context.endIter) && *context.iter != n) {
				if JSONIFIER_UNLIKELY (!value) {
					value = new std::remove_pointer_t<std::remove_cvref_t<value_type>>{};
				}
				parse<minified, options>::template impl<buffer_type>(*value, context);
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
	};

	template<bool minified, jsonifier::parse_options options, jsonifier::concepts::always_null_t value_type, typename buffer_type, typename parse_context_type>
	struct parse_impl<minified, options, value_type, buffer_type, parse_context_type> : derailleur<options, parse_context_type> {
		using base = derailleur<options, parse_context_type>;
		JSONIFIER_ALWAYS_INLINE static void impl(value_type&, parse_context_type& context) noexcept {
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

	template<bool minified, jsonifier::parse_options options, jsonifier::concepts::enum_t value_type, typename buffer_type, typename parse_context_type>
	struct parse_impl<minified, options, value_type, buffer_type, parse_context_type> : derailleur<options, parse_context_type> {
		using base = derailleur<options, parse_context_type>;
		JSONIFIER_ALWAYS_INLINE static void impl(value_type& value, parse_context_type& context) noexcept {
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
	};

	template<bool minified, jsonifier::parse_options options, jsonifier::concepts::num_t value_type, typename buffer_type, typename parse_context_type>
	struct parse_impl<minified, options, value_type, buffer_type, parse_context_type> : derailleur<options, parse_context_type> {
		using base = derailleur<options, parse_context_type>;
		JSONIFIER_ALWAYS_INLINE static void impl(value_type& value, parse_context_type& context) noexcept {
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

	template<bool minified, jsonifier::parse_options options, jsonifier::concepts::bool_t value_type, typename buffer_type, typename parse_context_type>
	struct parse_impl<minified, options, value_type, buffer_type, parse_context_type> : derailleur<options, parse_context_type> {
		using base = derailleur<options, parse_context_type>;
		JSONIFIER_ALWAYS_INLINE static void impl(value_type& value, parse_context_type& context) noexcept {
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
}