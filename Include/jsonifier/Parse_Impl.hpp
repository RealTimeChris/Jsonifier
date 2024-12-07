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

#define JSONIFIER_SKIP_KEY_VALUE() \
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
		return; \
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

	template<jsonifier::parse_options options, bool minified> struct index_processor_parse {
		template<size_t index, typename buffer_type, typename value_type, typename parse_context_type>
		JSONIFIER_ALWAYS_INLINE static bool processIndex(value_type& value, parse_context_type& context) noexcept {
			if constexpr (index < tuple_size_v<core_tuple_t<value_type>>) {
				static constexpr auto ptr			= get<index>(coreTupleV<value_type>).ptr();
				static constexpr auto key			= get<index>(coreTupleV<value_type>).view();
				static constexpr auto stringLiteral = stringLiteralFromView<key.size()>(key);
				static constexpr auto keySize		= key.size();
				static constexpr auto keySizeNew	= keySize + 1;

				if JSONIFIER_LIKELY (((context.iter + keySize) < context.endIter) && string_literal_comparitor<decltype(stringLiteral), stringLiteral>::impl(context.iter)) {
					context.iter += keySizeNew;
					if constexpr (!minified) {
						JSONIFIER_SKIP_WS();
					}
					if JSONIFIER_LIKELY ((context.iter < context.endIter) && *context.iter == colon) {
						++context.iter;
						if constexpr (!minified) {
							JSONIFIER_SKIP_WS();
						}
						if constexpr (jsonifier::concepts::has_excluded_keys<value_type>) {
							auto& keys = value.jsonifierExcludedKeys;
							if JSONIFIER_LIKELY (keys.find(static_cast<typename std::remove_cvref_t<decltype(keys)>::key_type>(key)) != keys.end()) {
								derailleur<options, parse_context_type>::template skipToNextValue<value_type>(context);
								return true;
							}
						}
						parse<minified, options>::template impl<buffer_type>(value.*ptr, context);
						return true;
					}
					JSONIFIER_ELSE_UNLIKELY(else) {
						context.parserPtr->template reportError<parse_errors::Missing_Colon>(context);
						derailleur<options, parse_context_type>::template skipToNextValue<value_type>(context);
					}
				} else {
					derailleur<options, parse_context_type>::template skipKey<value_type>(context);
					++context.iter;
					derailleur<options, parse_context_type>::template skipToNextValue<value_type>(context);
				}
			}
			return false;
		}
	};

	template<typename buffer_type, typename value_type, typename parse_context_type, jsonifier::parse_options options, bool minified, size_t... indices>
	static constexpr auto generateFunctionPtrs(std::index_sequence<indices...>) noexcept {
		using function_type = decltype(&index_processor_parse<options, minified>::template processIndex<0, buffer_type, value_type, parse_context_type>);
		return std::array<function_type, sizeof...(indices)>{
			{ &index_processor_parse<options, minified>::template processIndex<indices, buffer_type, value_type, parse_context_type>... }
		};
	}

	template<typename buffer_type, typename value_type, typename parse_context_type, jsonifier::parse_options options, bool minified, size_t... indices>
	JSONIFIER_INLINE static bool processIndexImpl(value_type& value, parse_context_type& context, size_t index, std::index_sequence<indices...>) noexcept {
		if constexpr (sizeof...(indices) <= 8) {
			return ((index == indices ? index_processor_parse<options, minified>::template processIndex<indices, buffer_type>(value, context) : false) || ...);
		} else {
			static constexpr auto parsePtrs = generateFunctionPtrs<buffer_type, value_type, parse_context_type, options, minified>(std::make_index_sequence<sizeof...(indices)>{});
			return parsePtrs[index](value, context);
		}
	}

	template<jsonifier::parse_options optionsNew, jsonifier::concepts::jsonifier_object_t value_type, typename buffer_type, typename parse_context_type>
	struct parse_impl<false, optionsNew, value_type, buffer_type, parse_context_type> : derailleur<optionsNew, parse_context_type> {
		static constexpr jsonifier::parse_options options{ optionsNew };
		using base						  = derailleur<options, parse_context_type>;
		static constexpr auto memberCount = tuple_size_v<core_tuple_t<value_type>>;
		inline static thread_local std::array<bool, memberCount> antiHashStates{ [] {
			std::array<bool, memberCount> returnValues{};
			returnValues.fill(true);
			return returnValues;
		}() };
		JSONIFIER_ALWAYS_INLINE static void impl(value_type& value, parse_context_type& context) noexcept {
			if JSONIFIER_LIKELY (context.iter + 1 < context.endIter) {
				if JSONIFIER_LIKELY (*context.iter == lBrace) {
					++context.iter;
					++context.currentObjectDepth;
					if JSONIFIER_LIKELY (*context.iter != rBrace) {
						if constexpr (memberCount > 0) {
							const auto wsStart = context.iter;
							JSONIFIER_SKIP_WS();
							size_t wsSize{ static_cast<size_t>(context.iter - wsStart) };

							if JSONIFIER_LIKELY ((context.iter < context.endIter) && *context.iter == quote) {
								++context.iter;
								if constexpr (options.knownOrder && !options.partialRead) {
									if (antiHashStates[0]) {
										static constexpr auto ptr			= get<0>(jsonifier::concepts::coreV<value_type>).ptr();
										static constexpr auto key			= get<0>(jsonifier::concepts::coreV<value_type>).view();
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
														if ((context.iter + wsSize < context.endIter) && whitespaceTable[static_cast<uint8_t>(*(context.iter + wsSize))]) {
															impl(std::integral_constant<size_t, 1>{}, std::integral_constant<bool, true>{}, value, context, wsStart, wsSize);
														} else {
															impl(std::integral_constant<size_t, 1>{}, std::integral_constant<bool, false>{}, value, context);
														}
													}
												}
												parse<false, options>::template impl<buffer_type>(value.*ptr, context);
												if ((context.iter + wsSize < context.endIter) && whitespaceTable[static_cast<uint8_t>(*(context.iter + wsSize))]) {
													impl(std::integral_constant<size_t, 1>{}, std::integral_constant<bool, true>{}, value, context, wsStart, wsSize);
												} else {
													impl(std::integral_constant<size_t, 1>{}, std::integral_constant<bool, false>{}, value, context);
												}
											}
											JSONIFIER_ELSE_UNLIKELY(else) {
												context.parserPtr->template reportError<parse_errors::Missing_Colon>(context);
												base::template skipToNextValue<value_type>(context);
											}
										} else {
											antiHashStates[0] = false;
											initialParse(value, context);
											if ((context.iter + wsSize < context.endIter) && whitespaceTable[static_cast<uint8_t>(*(context.iter + wsSize))]) {
												impl(std::integral_constant<size_t, 0>{}, std::integral_constant<bool, true>{}, value, context, wsStart, wsSize);
											} else {
												impl(std::integral_constant<size_t, 0>{}, std::integral_constant<bool, false>{}, value, context);
											}
										}
									} else {
										initialParse(value, context);
										if ((context.iter + wsSize < context.endIter) && whitespaceTable[static_cast<uint8_t>(*(context.iter + wsSize))]) {
											impl(std::integral_constant<size_t, 0>{}, std::integral_constant<bool, true>{}, value, context, wsStart, wsSize);
										} else {
											impl(std::integral_constant<size_t, 0>{}, std::integral_constant<bool, false>{}, value, context);
										}
									}
								} else {
									initialParse(value, context);
									if ((context.iter + wsSize < context.endIter) && whitespaceTable[static_cast<uint8_t>(*(context.iter + wsSize))]) {
										impl(std::integral_constant<size_t, 0>{}, std::integral_constant<bool, true>{}, value, context, wsStart, wsSize);
									} else {
										impl(std::integral_constant<size_t, 0>{}, std::integral_constant<bool, false>{}, value, context);
									}
								}
							}
							JSONIFIER_ELSE_UNLIKELY(else) {
								context.parserPtr->template reportError<parse_errors::Missing_String_Start>(context);
								base::template skipToNextValue<value_type>(context);
							}
						}
					}
					++context.iter;
					--context.currentObjectDepth;
					JSONIFIER_SKIP_WS();
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

		JSONIFIER_ALWAYS_INLINE static void initialParse(value_type& value, parse_context_type& context) {
			auto index = hash_map<value_type, std::remove_cvref_t<decltype(context.iter)>>::findIndex(context.iter, context.endIter);
			if JSONIFIER_LIKELY (index < memberCount) {
				if (processIndexImpl<buffer_type, value_type, parse_context_type, options, false>(value, context, index, std::make_index_sequence<memberCount>{})) {
					return;
				}
				JSONIFIER_ELSE_UNLIKELY(else) {
					if JSONIFIER_LIKELY ((context.iter < context.endIter) && *context.iter == ':') {
						++context.iter;
						JSONIFIER_SKIP_WS();
					}
					JSONIFIER_ELSE_UNLIKELY(else) {
						context.parserPtr->template reportError<parse_errors::Missing_Colon>(context);
						base::template skipToNextValue<value_type>(context);
						return;
					}
					base::template skipToNextValue<value_type>(context);
				}
			} else {
				JSONIFIER_SKIP_KEY_VALUE();
			}
		};

		JSONIFIER_INLINE static void impl(const auto index, const auto newLines, value_type& value, parse_context_type& context, const char* wsStart = nullptr,
			size_t wsSize = 0) {
			( void )context, ( void )value, ( void )wsStart, ( void )wsSize, ( void )newLines;
			if constexpr (index < memberCount) {
				if JSONIFIER_LIKELY ((context.iter < context.endIter) && *context.iter != rBrace) {
					if JSONIFIER_LIKELY (*context.iter == comma) {
						++context.iter;
						JSONIFIER_SKIP_MATCHING_WS();
					}
					JSONIFIER_ELSE_UNLIKELY(else) {
						context.parserPtr->template reportError<parse_errors::Missing_Comma_Or_Object_End>(context);
						base::template skipToNextValue<value_type>(context);
						return;
					}

					if JSONIFIER_LIKELY ((context.iter < context.endIter) && *context.iter == quote) {
						++context.iter;
						if constexpr (options.knownOrder && !options.partialRead) {
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
										return impl(std::integral_constant<size_t, index + 1>{}, std::integral_constant<bool, newLines>{}, value, context, wsStart, wsSize);
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
							if JSONIFIER_LIKELY (processIndexImpl<buffer_type, value_type, parse_context_type, options, false>(value, context, indexNew,
													 std::make_index_sequence<memberCount>{})) {
								return impl(std::integral_constant<size_t, index + (options.partialRead ? 1 : 0)>{}, std::integral_constant<bool, newLines>{}, value, context,
									wsStart, wsSize);
							}
							JSONIFIER_ELSE_UNLIKELY(else) {
								if JSONIFIER_LIKELY ((context.iter < context.endIter) && *context.iter == ':') {
									++context.iter;
									JSONIFIER_SKIP_WS();
								}
								JSONIFIER_ELSE_UNLIKELY(else) {
									context.parserPtr->template reportError<parse_errors::Missing_Colon>(context);
									base::template skipToNextValue<value_type>(context);
									return;
								}
								base::template skipToNextValue<value_type>(context);
								return impl(std::integral_constant<size_t, index>{}, std::integral_constant<bool, newLines>{}, value, context, wsStart, wsSize);
							}
						} else {
							JSONIFIER_SKIP_KEY_VALUE();
							return impl(std::integral_constant<size_t, index>{}, std::integral_constant<bool, newLines>{}, value, context, wsStart, wsSize);
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
		};
	};

	template<jsonifier::parse_options options, jsonifier::concepts::jsonifier_object_t value_type, typename buffer_type, typename parse_context_type>
	struct parse_impl<true, options, value_type, buffer_type, parse_context_type> : derailleur<options, parse_context_type> {
		using base						  = derailleur<options, parse_context_type>;
		static constexpr auto memberCount = tuple_size_v<core_tuple_t<value_type>>;
		inline static thread_local std::array<bool, memberCount> antiHashStates{ [] {
			std::array<bool, memberCount> returnValues{};
			returnValues.fill(true);
			return returnValues;
		}() };
		JSONIFIER_ALWAYS_INLINE static void impl(value_type& value, parse_context_type& context) noexcept {
			if JSONIFIER_LIKELY (context.iter + 1 < context.endIter) {
				if JSONIFIER_LIKELY (*context.iter == lBrace) {
					++context.iter;
					++context.currentObjectDepth;
					if JSONIFIER_LIKELY (*context.iter != rBrace) {
						if constexpr (memberCount > 0) {
							if JSONIFIER_LIKELY ((context.iter < context.endIter) && *context.iter == quote) {
								++context.iter;
								if constexpr (options.knownOrder && !options.partialRead) {
									if (antiHashStates[0]) {
										static constexpr auto ptr			= get<0>(jsonifier::concepts::coreV<value_type>).ptr();
										static constexpr auto key			= get<0>(jsonifier::concepts::coreV<value_type>).view();
										static constexpr auto stringLiteral = stringLiteralFromView<key.size()>(key);
										static constexpr auto keySize		= key.size();
										static constexpr auto keySizeNew	= keySize + 1;
										if JSONIFIER_LIKELY (((context.iter + keySize) < context.endIter) && (*(context.iter + keySize) == quote) &&
											string_literal_comparitor<decltype(stringLiteral), stringLiteral>::impl(context.iter)) {
											if constexpr (jsonifier::concepts::has_excluded_keys<value_type>) {
												auto& keys = value.jsonifierExcludedKeys;
												if JSONIFIER_LIKELY (keys.find(static_cast<typename std::remove_cvref_t<decltype(keys)>::key_type>(key)) != keys.end()) {
													base::template skipToNextValue<value_type>(context);
												}
											}
											context.iter += keySizeNew;
											if JSONIFIER_LIKELY ((context.iter < context.endIter) && *context.iter == colon) {
												++context.iter;
												parse<true, options>::template impl<buffer_type>(value.*ptr, context);
												impl(std::integral_constant<size_t, 1>{}, value, context);
											}
											JSONIFIER_ELSE_UNLIKELY(else) {
												context.parserPtr->template reportError<parse_errors::Missing_Colon>(context);
												base::template skipToNextValue<value_type>(context);
											}
										} else {
											antiHashStates[0] = false;
											initialParse(value, context);
											impl(std::integral_constant<size_t, 0>{}, value, context);
										}
									} else {
										initialParse(value, context);
										impl(std::integral_constant<size_t, 0>{}, value, context);
									}
								} else {
									initialParse(value, context);
									impl(std::integral_constant<size_t, 0>{}, value, context);
								}
							}
							JSONIFIER_ELSE_UNLIKELY(else) {
								context.parserPtr->template reportError<parse_errors::Missing_String_Start>(context);
								base::template skipToNextValue<value_type>(context);
							}
						}
					}
					++context.iter;
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

		JSONIFIER_ALWAYS_INLINE static void initialParse(value_type& value, parse_context_type& context) {
			auto index = hash_map<value_type, std::remove_cvref_t<decltype(context.iter)>>::findIndex(context.iter, context.endIter);
			if JSONIFIER_LIKELY (index < memberCount) {
				if JSONIFIER_LIKELY (processIndexImpl<buffer_type, value_type, parse_context_type, options, true>(value, context, index, std::make_index_sequence<memberCount>{})) {
					return;
				}
				JSONIFIER_ELSE_UNLIKELY(else) {
					if JSONIFIER_LIKELY ((context.iter < context.endIter) && *context.iter == ':') {
						++context.iter;
					}
					JSONIFIER_ELSE_UNLIKELY(else) {
						context.parserPtr->template reportError<parse_errors::Missing_Colon>(context);
						base::template skipToNextValue<value_type>(context);
						return;
					}
					base::template skipToNextValue<value_type>(context);
				}
			} else {
				JSONIFIER_SKIP_KEY_VALUE();
			}
		};

		JSONIFIER_INLINE static void impl(const auto index, value_type& value, parse_context_type& context) {
			( void )context, ( void )value;
			if constexpr (index < memberCount) {
				if JSONIFIER_LIKELY ((context.iter < context.endIter) && *context.iter != rBrace) {
					if JSONIFIER_LIKELY (*context.iter == comma) {
						++context.iter;
					}
					JSONIFIER_ELSE_UNLIKELY(else) {
						context.parserPtr->template reportError<parse_errors::Missing_Comma_Or_Object_End>(context);
						base::template skipToNextValue<value_type>(context);
						return;
					}

					if constexpr (jsonifier::concepts::has_excluded_keys<value_type>) {
						const auto keySize = getKeyLength<options>(context);
						jsonifier::string_view key{ static_cast<const char*>(context.iter) + 1, keySize };
						auto& keys = value.jsonifierExcludedKeys;
						if JSONIFIER_LIKELY (keys.find(static_cast<typename std::remove_cvref_t<decltype(keys)>::key_type>(key)) != keys.end()) {
							base::template skipToNextValue<value_type>(context);
							return;
						}
					}

					if JSONIFIER_LIKELY ((context.iter < context.endIter) && *context.iter == quote) {
						++context.iter;
						if constexpr (options.knownOrder && !options.partialRead) {
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
										parse<true, options>::template impl<buffer_type>(value.*ptr, context);
										return impl(std::integral_constant<size_t, index + 1>{}, value, context);
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
							if JSONIFIER_LIKELY (processIndexImpl<buffer_type, value_type, parse_context_type, options, true>(value, context, indexNew,
													 std::make_index_sequence<memberCount>{})) {
								return impl(std::integral_constant<size_t, index + (options.partialRead ? 1 : 0)>{}, value, context);
							}
							JSONIFIER_ELSE_UNLIKELY(else) {
								if JSONIFIER_LIKELY ((context.iter < context.endIter) && *context.iter == ':') {
									++context.iter;
								}
								JSONIFIER_ELSE_UNLIKELY(else) {
									context.parserPtr->template reportError<parse_errors::Missing_Colon>(context);
									base::template skipToNextValue<value_type>(context);
									return;
								}
								base::template skipToNextValue<value_type>(context);
								return impl(std::integral_constant<size_t, index>{}, value, context);
							}
						} else {
							JSONIFIER_SKIP_KEY_VALUE();
							return impl(std::integral_constant<size_t, index>{}, value, context);
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
		};
	};

	template<bool minified, jsonifier::parse_options options, jsonifier::concepts::jsonifier_scalar_value_t value_type, typename buffer_type, typename parse_context_type>
	struct parse_impl<minified, options, value_type, buffer_type, parse_context_type> : derailleur<options, parse_context_type> {
		using base = derailleur<options, parse_context_type>;
		JSONIFIER_ALWAYS_INLINE static void impl(value_type& value, parse_context_type& context) noexcept {
			static constexpr auto size{ tuple_size_v<core_tuple_t<value_type>> };
			if constexpr (size == 1) {
				static constexpr auto newPtr = get<0>(coreTupleV<value_type>);
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

		template<size_t maxIndex, size_t currentIndex, bool newLines>
		JSONIFIER_ALWAYS_INLINE static void parseObjects(value_type& value, parse_context_type& context, const auto wsStart = {}, size_t wsSize = {}) {
			if constexpr (currentIndex < maxIndex) {
				if JSONIFIER_LIKELY ((context.iter < context.endIter) && *context.iter != rBracket) {
					if JSONIFIER_LIKELY (*context.iter == comma) {
						++context.iter;
						JSONIFIER_SKIP_MATCHING_WS();
						parse<false, options>::template impl<buffer_type>(get<currentIndex>(value), context);
						return parseObjects<maxIndex, currentIndex + 1, newLines>(value, context, wsStart, wsSize);
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

		template<size_t maxIndex, size_t currentIndex> JSONIFIER_ALWAYS_INLINE static void parseObjects(value_type& value, parse_context_type& context) {
			if constexpr (currentIndex < maxIndex) {
				if JSONIFIER_LIKELY ((context.iter < context.endIter) && *context.iter != rBracket) {
					if JSONIFIER_LIKELY (*context.iter == comma) {
						++context.iter;
						parse<true, options>::template impl<buffer_type>(get<currentIndex>(value), context);
						return parseObjects<maxIndex, currentIndex + 1>(value, context);
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
			value = static_cast<value_type>(*(static_cast<const char*>(context.iter) + 1));
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
			auto newPtr = static_cast<const char*>(context.iter);
			base::template skipToNextValue<value_type>(context);
			int64_t newSize = static_cast<const char*>(context.iter) - newPtr;
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