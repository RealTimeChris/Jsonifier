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

	template<typename value_type, typename buffer_type, typename context_type, jsonifier::parse_options options, bool minifiedOrInsideRepeated> struct parse_types_impl {
		using base						  = derailleur<options, context_type>;
		static constexpr auto memberCount = tuple_size_v<core_tuple_type<value_type>>;
		template<size_t index> JSONIFIER_ALWAYS_INLINE static bool processIndex(value_type& value, context_type& context) {
			static constexpr auto tupleElem		= get<index>(jsonifier::core<std::remove_cvref_t<value_type>>::parseValue);
			static constexpr auto key			= tupleElem.view();
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
							base::template skipToNextValue<value_type>(context);
							return true;
						}
					}
					using member_type = typename std::remove_cvref_t<decltype(tupleElem)>::member_type;
					parse<getJsonType<member_type>(), options, minifiedOrInsideRepeated>::template impl<buffer_type>(value.*ptrNew, context);
					return true;
				}
				JSONIFIER_ELSE_UNLIKELY(else) {
					context.parserPtr->template reportError<parse_errors::Missing_Colon>(context);
					base::template skipToNextValue<value_type>(context);
				}
			}
			return false;
		}
	};

	template<typename value_type, typename buffer_type, typename context_type, jsonifier::parse_options options, bool minifiedOrInsideRepeated> struct parse_types_impl_partial {
		using base						  = derailleur<options, context_type>;
		static constexpr auto memberCount = tuple_size_v<core_tuple_type<value_type>>;
		template<size_t index> JSONIFIER_ALWAYS_INLINE static bool processIndex(value_type& value, context_type& context) {
			static constexpr auto tupleElem		= get<index>(jsonifier::core<std::remove_cvref_t<value_type>>::parseValue);
			static constexpr auto key			= tupleElem.view();
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
							base::template skipToNextValue<value_type>(context);
							return false;
						}
					}
					using member_type = typename std::remove_cvref_t<decltype(tupleElem)>::member_type;
					parse<getJsonType<member_type>(), options, minifiedOrInsideRepeated>::template impl<buffer_type>(value.*ptrNew, context);
					return true;
				}
				JSONIFIER_ELSE_UNLIKELY(else) {
					context.parserPtr->template reportError<parse_errors::Missing_Colon>(context);
					base::template skipToNextValue<value_type>(context);
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

	template<template<typename, typename, typename, jsonifier::parse_options, bool> typename parsing_type, typename value_type, typename buffer_type, typename context_type,
		jsonifier::parse_options options, bool minifiedOrInsideRepeated>
	static constexpr auto functionPtrs{ generateFunctionPtrs<parsing_type, value_type, buffer_type, context_type, options, minifiedOrInsideRepeated>(
		std::make_index_sequence<tuple_size_v<core_tuple_type<value_type>>>{}) };

	template<typename value_type, typename buffer_type, typename context_type, jsonifier::parse_options options, auto tupleElem, bool minifiedOrInsideRepeated> struct parse_types;

	template<jsonifier::parse_options options, typename value_type, typename context_type, bool newLines = true>
	JSONIFIER_ALWAYS_INLINE static void checkForEntryComma(context_type& context, const char* wsStart, size_t wsSize) {
		using base = derailleur<options, context_type>;
		if JSONIFIER_LIKELY (*context.iter == comma) {
			++context.iter;
			JSONIFIER_SKIP_MATCHING_WS()
		}
		JSONIFIER_ELSE_UNLIKELY(else) {
			context.parserPtr->template reportError<parse_errors::Missing_Comma>(context);
			base::template skipToNextValue<value_type>(context);
			return;
		}
	}

	template<jsonifier::parse_options options, typename value_type, typename context_type>
	JSONIFIER_ALWAYS_INLINE static void checkForEntryComma(context_type& context) {
		using base = derailleur<options, context_type>;
		if JSONIFIER_LIKELY (*context.iter == comma) {
			++context.iter;
		}
		JSONIFIER_ELSE_UNLIKELY(else) {
			context.parserPtr->template reportError<parse_errors::Missing_Comma>(context);
			base::template skipToNextValue<value_type>(context);
			return;
		}
	}

	template<jsonifier::parse_options options, typename value_type, partial_reading_context_t context_type>
	JSONIFIER_ALWAYS_INLINE static void checkForEntryComma(context_type& context) {
		using base = derailleur<options, context_type>;
		if JSONIFIER_LIKELY (**context.iter == comma) {
			++context.iter;
		}
		JSONIFIER_ELSE_UNLIKELY(else) {
			context.parserPtr->template reportError<parse_errors::Missing_Comma>(context);
			base::template skipToNextValue<value_type>(context);
			return;
		}
	}

	template<typename value_type, typename buffer_type, typename context_type, jsonifier::parse_options options, auto tupleElem>
	struct parse_types<value_type, buffer_type, context_type, options, tupleElem, false> {
		using base						  = derailleur<options, context_type>;
		static constexpr auto memberCount = tuple_size_v<core_tuple_type<value_type>>;
		inline static thread_local array<int64_t, (static_cast<int64_t>(memberCount) > 0 ? memberCount : 1)> antiHashStates{ [] {
			array<int64_t, (static_cast<int64_t>(memberCount) > 0 ? memberCount : 1)> returnValues{};
			for (int64_t x = 0; x < static_cast<int64_t>(memberCount); ++x) {
				returnValues[static_cast<uint64_t>(x)] = static_cast<int64_t>(x);
			}
			return returnValues;
		}() };

		JSONIFIER_INLINE static void processIndexCall(value_type& value, context_type& context, string_view_ptr& wsStart, size_t wsSize) noexcept {
			return processIndex<true>(value, context, wsStart, wsSize);
		}

		template<bool haveWeStarted = false>
		JSONIFIER_CLANG_ALWAYS_INLINE static void processIndex(value_type& value, context_type& context, string_view_ptr& wsStart, size_t wsSize) noexcept {
			( void )value, ( void )context, ( void )wsStart, ( void )wsSize;
			if constexpr (memberCount > 0 && tupleElem.index < memberCount) {
				if JSONIFIER_LIKELY (context.iter < context.endIter) {
					if (*context.iter != rBrace) {
						if constexpr (haveWeStarted || tupleElem.index > 0) {
							checkForEntryComma<options, value_type>(context, wsStart, wsSize);
						}
						if JSONIFIER_LIKELY ((context.iter < context.endIter) && *context.iter == quote) {
							++context.iter;
							if constexpr (options.knownOrder) {
								if (antiHashStates[tupleElem.index] != -1) {
									static constexpr auto key			= tupleElem.view();
									static constexpr auto ptrNew		= tupleElem.memberPtr;
									static constexpr auto stringLiteral = stringLiteralFromView<key.size()>(key);
									static constexpr auto keySize		= key.size();
									static constexpr auto keySizeNew	= keySize + 1;

									if JSONIFIER_LIKELY (((context.iter + keySize) < context.endIter) && (*(context.iter + keySize) == quote) &&
										string_literal_comparitor<decltype(stringLiteral), stringLiteral>::impl(context.iter)) {
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
													base::template skipToNextValue<value_type>(context);
													return;
												}
											}
											using member_type = typename std::remove_cvref_t<decltype(tupleElem)>::member_type;
											parse<getJsonType<member_type>(), options, false>::template impl<buffer_type>(value.*ptrNew, context);
											return;
										}
										JSONIFIER_ELSE_UNLIKELY(else) {
											context.parserPtr->template reportError<parse_errors::Missing_Colon>(context);
											base::template skipToNextValue<value_type>(context);
										}
									}
									antiHashStates[tupleElem.index] = -1;
								}
							}
							if JSONIFIER_LIKELY (auto indexNew = hash_map<value_type, std::remove_cvref_t<decltype(context.iter)>>::findIndex(context.iter, context.endIter);
													indexNew < memberCount) {
								if JSONIFIER_LIKELY (functionPtrs<parse_types_impl, value_type, buffer_type, context_type, options, false>[indexNew](value, context)) {
									return;
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
							return processIndexCall(value, context, wsStart, wsSize);
						}
						JSONIFIER_ELSE_UNLIKELY(else) {
							context.parserPtr->template reportError<parse_errors::Missing_String_Start>(context);
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
	};

	template<typename value_type, typename buffer_type, typename context_type, jsonifier::parse_options options, auto tupleElem>
	struct parse_types<value_type, buffer_type, context_type, options, tupleElem, true> {
		using base						  = derailleur<options, context_type>;
		static constexpr auto memberCount = tuple_size_v<core_tuple_type<value_type>>;
		inline static thread_local array<int64_t, (static_cast<int64_t>(memberCount) > 0 ? memberCount : 1)> antiHashStates{ [] {
			array<int64_t, (static_cast<int64_t>(memberCount) > 0 ? memberCount : 1)> returnValues{};
			for (int64_t x = 0; x < static_cast<int64_t>(memberCount); ++x) {
				returnValues[static_cast<uint64_t>(x)] = static_cast<int64_t>(x);
			}
			return returnValues;
		}() };

		JSONIFIER_INLINE static void processIndexCall(value_type& value, context_type& context) noexcept {
			return processIndex<true>(value, context);
		}

		template<bool haveWeStarted = false> JSONIFIER_CLANG_ALWAYS_INLINE static void processIndex(value_type& value, context_type& context) noexcept {
			( void )value, ( void )context;
			if constexpr (memberCount > 0 && tupleElem.index < memberCount) {
				if JSONIFIER_LIKELY (context.iter < context.endIter) {
					if (*context.iter != rBrace) {
						if constexpr (haveWeStarted || tupleElem.index > 0) {
							checkForEntryComma<options, value_type>(context);
						}
						if JSONIFIER_LIKELY ((context.iter < context.endIter) && *context.iter == quote) {
							++context.iter;
							if constexpr (options.knownOrder) {
								if (antiHashStates[tupleElem.index] != -1) {
									static constexpr auto key			= tupleElem.view();
									static constexpr auto ptrNew		= tupleElem.memberPtr;
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
													return;
												}
											}
											using member_type = typename std::remove_cvref_t<decltype(tupleElem)>::member_type;
											parse<getJsonType<member_type>(), options, true>::template impl<buffer_type>(value.*ptrNew, context);
											return;
										}
										JSONIFIER_ELSE_UNLIKELY(else) {
											context.parserPtr->template reportError<parse_errors::Missing_Colon>(context);
											base::template skipToNextValue<value_type>(context);
										}
									}
									antiHashStates[tupleElem.index] = -1;
								}
							}
							if JSONIFIER_LIKELY (auto indexNew = hash_map<value_type, std::remove_cvref_t<decltype(context.iter)>>::findIndex(context.iter, context.endIter);
													indexNew < memberCount) {
								if JSONIFIER_LIKELY (functionPtrs<parse_types_impl, value_type, buffer_type, context_type, options, true>[indexNew](value, context)) {
									return;
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
							return processIndexCall(value, context);
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
		}
	};

	template<typename value_type, typename buffer_type, typename context_type, jsonifier::parse_options options, auto tupleElem, bool insideRepeated> struct parse_types_partial {
		using base						  = derailleur<options, context_type>;
		static constexpr auto memberCount = tuple_size_v<core_tuple_type<value_type>>;
		inline static thread_local array<int64_t, (static_cast<int64_t>(memberCount) > 0 ? memberCount : 1)> antiHashStates{ [] {
			array<int64_t, (static_cast<int64_t>(memberCount) > 0 ? memberCount : 1)> returnValues{};
			for (int64_t x = 0; x < static_cast<int64_t>(memberCount); ++x) {
				returnValues[static_cast<uint64_t>(x)] = static_cast<int64_t>(x);
			}
			return returnValues;
		}() };

		JSONIFIER_INLINE static void processIndexCall(value_type& value, context_type& context) noexcept {
			return processIndex<true>(value, context);
		}

		template<bool haveWeStarted = false> JSONIFIER_CLANG_ALWAYS_INLINE static void processIndex(value_type& value, context_type& context) noexcept {
			( void )value, ( void )context;
			if constexpr (tupleElem.index < memberCount) {
				if (**context.iter != rBrace) {
					if JSONIFIER_LIKELY (context.iter < context.endIter) {
						if constexpr (haveWeStarted || tupleElem.index > 0) {
							checkForEntryComma<options, value_type>(context);
						}

						if JSONIFIER_LIKELY ((context.iter < context.endIter) && **context.iter == quote) {
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
								base::template skipToNextValue<value_type>(context);
								return;
							}
							base::template skipToNextValue<value_type>(context);
							return processIndexCall(value, context);
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
			}
		}
	};

	template<template<typename, typename, typename, jsonifier::parse_options, auto, bool> typename parse_type, typename value_type, typename buffer_type, typename context_type,
		jsonifier::parse_options options, bool minifiedOrInsideRepeated, auto... values>
	struct parser_map {
		template<typename... arg_types> JSONIFIER_INLINE static void processIndices(arg_types&&... args) {
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
		using type = parser_map<parse_type, value_type, buffer_type, context_type, options, minifiedOrInsideRepeated, jsonifier_internal::get<I>(tuple)...>;
	};

	template<template<typename, typename, typename, jsonifier::parse_options, auto, bool> typename parse_type, typename value_type, typename buffer_type, typename context_type,
		jsonifier::parse_options options, bool minifiedOrInsideRepeated, auto value>
	using parser_base_t = typename get_parser_base<parse_type, value_type, buffer_type, context_type, options, minifiedOrInsideRepeated,
		jsonifier_internal::tag_range<jsonifier_internal::tuple_size_v<decltype(value)>>, value>::type;

	template<typename buffer_type, typename context_type, jsonifier::parse_options optionsNew> struct object_val_parser<buffer_type, context_type, optionsNew, false> {
		static constexpr jsonifier::parse_options options{ optionsNew };
		using base = derailleur<options, context_type>;

		template<jsonifier::concepts::jsonifier_object_t value_type> JSONIFIER_ALWAYS_INLINE static void impl(value_type& value, context_type& context) noexcept {
			if JSONIFIER_LIKELY (context.iter + 1 < context.endIter) {
				if JSONIFIER_LIKELY (*context.iter == lBrace) {
					++context.iter;
					++context.currentObjectDepth;
					string_view_ptr wsStart = context.iter;
					JSONIFIER_SKIP_WS();
					size_t wsSize = static_cast<size_t>(context.iter - wsStart);
					parser_base_t<parse_types, value_type, buffer_type, context_type, options, false, jsonifier::core<std::remove_cvref_t<value_type>>::parseValue>::processIndices(
						value, context, wsStart, wsSize);
					if JSONIFIER_LIKELY (context.iter < context.endIter && *context.iter == rBrace) {
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

		template<jsonifier::concepts::map_t value_type> JSONIFIER_ALWAYS_INLINE static void impl(value_type& value, context_type& context) noexcept {
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
		JSONIFIER_ALWAYS_INLINE static void parseObjects(value_type& value, context_type& context, const auto wsStart = {}, size_t wsSize = {}) {
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

	template<typename buffer_type, typename context_type, jsonifier::parse_options optionsNew> struct array_val_parser<buffer_type, context_type, optionsNew, false> {
		static constexpr jsonifier::parse_options options{ optionsNew };
		using base = derailleur<options, context_type>;

		template<jsonifier::concepts::vector_t value_type> JSONIFIER_ALWAYS_INLINE static void impl(value_type& value, context_type& context) noexcept {
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
		JSONIFIER_ALWAYS_INLINE static void parseObjects(value_type& value, context_type& context, const auto wsStart = {}, size_t wsSize = {}) {
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

		template<jsonifier::concepts::raw_array_t value_type> JSONIFIER_ALWAYS_INLINE static void impl(value_type& value, context_type& context) noexcept {
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
		JSONIFIER_ALWAYS_INLINE static void parseObjects(value_type& value, context_type& context, const auto wsStart = {}, size_t wsSize = {}) {
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

		template<jsonifier::concepts::tuple_t value_type> JSONIFIER_ALWAYS_INLINE static void impl(value_type& value, context_type& context) noexcept {
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
		JSONIFIER_ALWAYS_INLINE static void parseObjects(value_type& value, context_type& context) {
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

	template<typename buffer_type, typename context_type, jsonifier::parse_options optionsNew> struct object_val_parser<buffer_type, context_type, optionsNew, true> {
		static constexpr jsonifier::parse_options options{ optionsNew };
		using base = derailleur<options, context_type>;

		template<jsonifier::concepts::jsonifier_object_t value_type> JSONIFIER_ALWAYS_INLINE static void impl(value_type& value, context_type& context) noexcept {
			if JSONIFIER_LIKELY (context.iter + 1 < context.endIter) {
				if JSONIFIER_LIKELY (*context.iter == lBrace) {
					++context.iter;
					++context.currentObjectDepth;
					parser_base_t<parse_types, value_type, buffer_type, context_type, options, true, jsonifier::core<std::remove_cvref_t<value_type>>::parseValue>::processIndices(
						value, context);
					if JSONIFIER_LIKELY (context.iter < context.endIter && *context.iter == rBrace) {
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

		template<jsonifier::concepts::map_t value_type> JSONIFIER_ALWAYS_INLINE static void impl(value_type& value, context_type& context) noexcept {
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

		template<jsonifier::concepts::map_t value_type> JSONIFIER_ALWAYS_INLINE static void parseObjects(value_type& value, context_type& context) {
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

	template<typename buffer_type, typename context_type, jsonifier::parse_options optionsNew> struct array_val_parser<buffer_type, context_type, optionsNew, true> {
		static constexpr jsonifier::parse_options options{ optionsNew };
		using base = derailleur<options, context_type>;

		template<jsonifier::concepts::vector_t value_type> JSONIFIER_ALWAYS_INLINE static void impl(value_type& value, context_type& context) noexcept {
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

		template<jsonifier::concepts::raw_array_t value_type> JSONIFIER_ALWAYS_INLINE static void impl(value_type& value, context_type& context) noexcept {
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

		template<jsonifier::concepts::tuple_t value_type> JSONIFIER_ALWAYS_INLINE static void impl(value_type& value, context_type& context) noexcept {
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
		JSONIFIER_ALWAYS_INLINE static void parseObjects(value_type& value, context_type& context) {
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

	template<typename buffer_type, typename context_type, jsonifier::parse_options optionsNew, bool minified> struct string_val_parser {
		static constexpr jsonifier::parse_options options{ optionsNew };
		using base = derailleur<options, context_type>;

		template<jsonifier::concepts::string_t value_type> JSONIFIER_ALWAYS_INLINE static void impl(value_type& value, context_type& context) noexcept {
			base::parseString(value, context);
			if constexpr (!minified) {
				JSONIFIER_SKIP_WS();
			}
		}

		template<jsonifier::concepts::char_t value_type> JSONIFIER_ALWAYS_INLINE static void impl(value_type& value, context_type& context) noexcept {
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

		template<jsonifier::concepts::enum_t value_type> JSONIFIER_ALWAYS_INLINE static void impl(value_type& value, context_type& context) noexcept {
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

		template<jsonifier::concepts::num_t value_type> JSONIFIER_ALWAYS_INLINE static void impl(value_type& value, context_type& context) noexcept {
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

	template<typename buffer_type, typename context_type, jsonifier::parse_options optionsNew, bool minified> struct bool_val_parser {
		static constexpr jsonifier::parse_options options{ optionsNew };
		using base = derailleur<options, context_type>;

		template<jsonifier::concepts::bool_t value_type> JSONIFIER_ALWAYS_INLINE static void impl(value_type& value, context_type& context) noexcept {
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

	template<typename buffer_type, typename context_type, jsonifier::parse_options optionsNew, bool minified> struct null_val_parser {
		static constexpr jsonifier::parse_options options{ optionsNew };
		using base = derailleur<options, context_type>;

		template<jsonifier::concepts::always_null_t value_type> JSONIFIER_ALWAYS_INLINE static void impl(value_type&, context_type& context) noexcept {
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

	template<typename buffer_type, typename context_type, jsonifier::parse_options optionsNew, bool minified> struct accessor_val_parser {
		static constexpr jsonifier::parse_options options{ optionsNew };
		using base = derailleur<options, context_type>;

		template<jsonifier::json_type type, typename variant_type, size_t currentIndex = 0>
		JSONIFIER_ALWAYS_INLINE static constexpr void iterateVariantTypes(variant_type&& variant, context_type& context) noexcept {
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

		template<jsonifier::concepts::variant_t value_type> JSONIFIER_ALWAYS_INLINE static void impl(value_type& value, context_type& context) noexcept {
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

		template<jsonifier::concepts::optional_t value_type> JSONIFIER_ALWAYS_INLINE static void impl(value_type& value, context_type& context) noexcept {
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

		template<jsonifier::concepts::shared_ptr_t value_type> JSONIFIER_ALWAYS_INLINE static void impl(value_type& value, context_type& context) noexcept {
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

		template<jsonifier::concepts::unique_ptr_t value_type> JSONIFIER_ALWAYS_INLINE static void impl(value_type& value, context_type& context) noexcept {
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

		template<jsonifier::concepts::pointer_t value_type> JSONIFIER_ALWAYS_INLINE static void impl(value_type& value, context_type& context) noexcept {
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

		template<jsonifier::concepts::raw_json_t value_type> JSONIFIER_ALWAYS_INLINE static void impl(value_type& value, context_type& context) noexcept {
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

		template<jsonifier::concepts::skip_t value_type> JSONIFIER_ALWAYS_INLINE static void impl(value_type& value, context_type& context) noexcept {
			base::template skipToNextValue<value_type>(context);
		}
	};

	template<typename buffer_type, typename context_type, jsonifier::parse_options optionsNew, bool insideRepeated> struct object_val_parser_partial {
		static constexpr jsonifier::parse_options options{ optionsNew };
		using base = derailleur<options, context_type>;

		template<jsonifier::concepts::jsonifier_object_t value_type> JSONIFIER_ALWAYS_INLINE static void impl(value_type& value, context_type& context) noexcept {
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

		template<jsonifier::concepts::map_t value_type> JSONIFIER_ALWAYS_INLINE static void impl(value_type& value, context_type& context) noexcept {
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

		template<jsonifier::concepts::tuple_t value_type> JSONIFIER_ALWAYS_INLINE static void impl(value_type& value, context_type& context) noexcept {
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
		JSONIFIER_ALWAYS_INLINE static void parseObjects(value_type& value, context_type& context) {
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

	template<typename buffer_type, typename context_type, jsonifier::parse_options optionsNew, bool insideRepeated> struct array_val_parser_partial {
		static constexpr jsonifier::parse_options options{ optionsNew };
		using base = derailleur<options, context_type>;

		template<jsonifier::concepts::vector_t value_type> JSONIFIER_ALWAYS_INLINE static void impl(value_type& value, context_type& context) noexcept {
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

		template<jsonifier::concepts::raw_array_t value_type> JSONIFIER_ALWAYS_INLINE static void impl(value_type& value, context_type& context) noexcept {
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

		template<jsonifier::concepts::tuple_t value_type> JSONIFIER_ALWAYS_INLINE static void impl(value_type& value, context_type& context) noexcept {
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
		JSONIFIER_ALWAYS_INLINE static void parseObjects(value_type& value, context_type& context) {
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

	template<typename buffer_type, typename context_type, jsonifier::parse_options optionsNew, bool insideRepeated> struct string_val_parser_partial {
		static constexpr jsonifier::parse_options options{ optionsNew };
		using base = derailleur<options, context_type>;

		template<jsonifier::concepts::string_t value_type> JSONIFIER_ALWAYS_INLINE static void impl(value_type& value, context_type& context) noexcept {
			base::parseString(value, context);
		}

		template<jsonifier::concepts::char_t value_type> JSONIFIER_ALWAYS_INLINE static void impl(value_type& value, context_type& context) noexcept {
			value = static_cast<value_type>(*(*context.iter + 1));
			++context.iter;
		}
	};

	template<typename buffer_type, typename context_type, jsonifier::parse_options optionsNew, bool insideRepeated> struct number_val_parser_partial {
		static constexpr jsonifier::parse_options options{ optionsNew };
		using base = derailleur<options, context_type>;

		template<jsonifier::concepts::enum_t value_type> JSONIFIER_ALWAYS_INLINE static void impl(value_type& value, context_type& context) noexcept {
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

		template<jsonifier::concepts::num_t value_type> JSONIFIER_ALWAYS_INLINE static void impl(value_type& value, context_type& context) noexcept {
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

	template<typename buffer_type, typename context_type, jsonifier::parse_options optionsNew, bool insideRepeated> struct bool_val_parser_partial {
		static constexpr jsonifier::parse_options options{ optionsNew };
		using base = derailleur<options, context_type>;

		template<jsonifier::concepts::bool_t value_type> JSONIFIER_ALWAYS_INLINE static void impl(value_type& value, context_type& context) noexcept {
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

	template<typename buffer_type, typename context_type, jsonifier::parse_options optionsNew, bool insideRepeated> struct null_val_parser_partial {
		static constexpr jsonifier::parse_options options{ optionsNew };
		using base = derailleur<options, context_type>;

		template<jsonifier::concepts::always_null_t value_type> JSONIFIER_ALWAYS_INLINE static void impl(value_type& value, context_type& context) noexcept {
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

	template<typename buffer_type, typename context_type, jsonifier::parse_options optionsNew, bool insideRepeated> struct accessor_val_parser_partial {
		static constexpr jsonifier::parse_options options{ optionsNew };
		using base = derailleur<options, context_type>;

		template<jsonifier::concepts::variant_t value_type> JSONIFIER_ALWAYS_INLINE static void impl(value_type& value, context_type& context) noexcept {
			static constexpr auto lambda = [](auto&& valueNew, auto&& context) {
				using member_type = decltype(valueNew);
				return parse<getJsonType<member_type>(), options, insideRepeated>::template impl<buffer_type>(valueNew, context);
			};
			visit<lambda>(value, context);
		}

		template<jsonifier::concepts::optional_t value_type> JSONIFIER_ALWAYS_INLINE static void impl(value_type& value, context_type& context) noexcept {
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

		template<jsonifier::concepts::shared_ptr_t value_type> JSONIFIER_ALWAYS_INLINE static void impl(value_type& value, context_type& context) noexcept {
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

		template<jsonifier::concepts::unique_ptr_t value_type> JSONIFIER_ALWAYS_INLINE static void impl(value_type& value, context_type& context) noexcept {
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

		template<jsonifier::concepts::pointer_t value_type> JSONIFIER_ALWAYS_INLINE static void impl(value_type& value, context_type& context) noexcept {
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

		template<jsonifier::concepts::raw_json_t value_type> JSONIFIER_ALWAYS_INLINE static void impl(value_type& value, context_type& context) noexcept {
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

		template<jsonifier::concepts::skip_t value_type> JSONIFIER_ALWAYS_INLINE static void impl(value_type& value, context_type& context) noexcept {
			base::template skipToNextValue<value_type>(context);
		}
	};
}