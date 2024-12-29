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
#include <jsonifier/JsonEntity.hpp>
#include <jsonifier/Parser.hpp>

#include <memory>

namespace jsonifier_internal {

	static constexpr char falseV[]{ "false" };
	static constexpr char trueV[]{ "true" };
	static constexpr char nullV[]{ "null" };
	static constexpr char newline{ '\n' };
	static constexpr char lBracket{ '[' };
	static constexpr char rBracket{ ']' };
	static constexpr char lBrace{ '{' };
	static constexpr char rBrace{ '}' };
	static constexpr char colon{ ':' };
	static constexpr char comma{ ',' };
	static constexpr char quote{ '"' };
	static constexpr char n{ 'n' };

	template<jsonifier::parse_options options, typename value_type, typename context_type, bool newLines = true>
	JSONIFIER_FORCE_INLINE static void checkForEntryComma(context_type& context, const char* wsStart, size_t wsSize) noexcept {
		using base = derailleur<options, value_type, context_type>;
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

	template<jsonifier::parse_options options, typename value_type, typename context_type> JSONIFIER_FORCE_INLINE static void checkForEntryComma(context_type& context) noexcept {
		using base = derailleur<options, value_type, context_type>;
		if JSONIFIER_LIKELY (*context.iter == comma) {
			++context.iter;
		}
		JSONIFIER_ELSE_UNLIKELY(else) {
			context.parserPtr->template reportError<parse_errors::Missing_Comma>(context);
			base::skipToNextValue(context);
			return;
		}
	}

	template<jsonifier::parse_options options, typename value_type, partial_reading_context_t context_type>
	JSONIFIER_FORCE_INLINE static void checkForEntryComma(context_type& context) noexcept {
		using base = derailleur<options, value_type, context_type>;
		if JSONIFIER_LIKELY (**context.iter == comma) {
			++context.iter;
		}
		JSONIFIER_ELSE_UNLIKELY(else) {
			context.parserPtr->template reportError<parse_errors::Missing_Comma>(context);
			base::skipToNextValue(context);
			return;
		}
	}

	template<jsonifier::parse_options options, size_t depth, size_t maxDepth, typename... bases> struct parse_entities : public bases... {
		using type_list_new = type_list<bases...>;
		static constexpr auto memberCount{ sizeof...(bases) };
		inline static thread_local array<uint64_t, (memberCount > 0 ? memberCount : 1)> antiHashStates{ [] {
			array<uint64_t, (memberCount > 0 ? memberCount : 1)> returnValues{};
			for (uint64_t x = 0; x < memberCount; ++x) {
				returnValues[x] = x;
			}
			return returnValues;
		}() };

		template<size_t index, typename value_type, typename context_type> JSONIFIER_INLINE static bool processIndex(value_type& value, context_type& context) noexcept {
			return processIndexForPtrs<typename get_type_at_index<type_list_new, index>::type>(value, context);
		}

		template<typename json_entity_type, typename value_type, typename context_type>
		JSONIFIER_INLINE static bool processIndexForPtrs(value_type& value, context_type& context) noexcept {
			using base = derailleur<options, value_type, context_type>;
			static constexpr json_entity_type jsonEntity{};
			static constexpr auto stringLiteral = jsonEntity.name;
			static constexpr auto keySize		= stringLiteral.size();
			static constexpr auto key			= stringLiteral.template view<jsonifier::string_view>();
			static constexpr auto keySizeNew	= keySize + 1;
			if JSONIFIER_LIKELY ((context.iter + keySize) < context.endIter && string_literal_comparitor<decltype(stringLiteral), stringLiteral>::impl(context.iter)) {
				context.iter += keySizeNew;
				if constexpr (!options.minified) {
					JSONIFIER_SKIP_WS()
				}
				if JSONIFIER_LIKELY (context.iter < context.endIter && *context.iter == colon) {
					++context.iter;
					if constexpr (!options.minified) {
						JSONIFIER_SKIP_WS()
					}
					if constexpr (jsonifier::concepts::has_excluded_keys<value_type>) {
						auto& keys = value.jsonifierExcludedKeys;
						if JSONIFIER_LIKELY (keys.find(static_cast<typename std::remove_cvref_t<decltype(keys)>::key_type>(key)) != keys.end()) {
							base::skipToNextValue(context);
							return true;
						}
					}
					static constexpr auto ptrNew = jsonEntity.memberPtr;
					parse<options, jsonEntity, 0, maxDepth, options.minified>::impl(value.*ptrNew, context);
					return true;
				}
				JSONIFIER_ELSE_UNLIKELY(else) {
					context.parserPtr->template reportError<parse_errors::Missing_Colon>(context);
					base::skipToNextValue(context);
				}
			}
			return false;
		}

		template<size_t index, typename value_type, typename context_type> using process_index_function_type = bool (*)(value_type&, context_type&) noexcept;

		template<typename value_type, typename context_type, size_t... indices> static constexpr auto generateFunctionPtrsParse(std::index_sequence<indices...>) {
			using function_type = process_index_function_type<0, value_type, context_type>;
			return array<function_type, sizeof...(indices)>{ &parse_entities<options, depth, maxDepth, bases...>::template processIndex<indices, value_type, context_type>... };
		}

		template<typename value_type, typename context_type>
		static constexpr auto functionPtrsParse{ generateFunctionPtrsParse<value_type, context_type>(std::make_index_sequence<sizeof...(bases)>{}) };

		template<typename json_entity_type, typename value_type, typename context_type>
		JSONIFIER_INLINE void processIndexCall(value_type& value, context_type& context, const char* wsStart, size_t wsSize) const noexcept {
			return processIndex<json_entity_type, value_type, context_type, true>(value, context, wsStart, wsSize);
		}

		template<typename json_entity_type, typename value_type, typename context_type>
		JSONIFIER_INLINE void processIndexImpl(value_type& value, context_type& context, const char* wsStart, size_t wsSize) const noexcept {
			using base = derailleur<options, value_type, context_type>;
			static constexpr json_entity_type jsonEntity{};
			if constexpr (options.knownOrder) {
				if (antiHashStates[jsonEntity.index] == jsonEntity.index) {
					static constexpr auto key		 = jsonEntity.name;
					static constexpr auto keySize	 = key.size();
					static constexpr auto keySizeNew = keySize + 1;
					if JSONIFIER_LIKELY ((context.iter + keySize) < context.endIter && (*(context.iter + keySize)) == quote &&
						string_literal_comparitor<decltype(key), key>::impl(context.iter)) {
						context.iter += keySizeNew;
						JSONIFIER_SKIP_WS()
						if JSONIFIER_LIKELY (context.iter < context.endIter && *context.iter == colon) {
							++context.iter;
							JSONIFIER_SKIP_WS()
							if constexpr (jsonifier::concepts::has_excluded_keys<value_type>) {
								auto& keys = value.jsonifierExcludedKeys;
								if JSONIFIER_LIKELY (keys.find(static_cast<typename std::remove_cvref_t<decltype(keys)>::key_type>(key)) != keys.end()) {
									base::skipToNextValue(context);
									return;
								}
							}
							static constexpr auto ptrNew = jsonEntity.memberPtr;
							parse<options, jsonEntity, depth, maxDepth, options.minified>::impl(value.*ptrNew, context);
							return;
						}
						JSONIFIER_ELSE_UNLIKELY(else) {
							context.parserPtr->template reportError<parse_errors::Missing_Colon>(context);
							base::skipToNextValue(context);
						}
					}
				}
				JSONIFIER_ELSE_UNLIKELY(else) {
					if JSONIFIER_LIKELY (functionPtrsParse<value_type, context_type>[antiHashStates[jsonEntity.index]](value, context)) {
						return;
					}
				}
			}
			if JSONIFIER_LIKELY (auto indexNew = hash_map<value_type, std::remove_cvref_t<decltype(context.iter)>>::findIndex(context.iter, context.endIter);
				indexNew < memberCount) {
				if JSONIFIER_LIKELY (functionPtrsParse<value_type, context_type>[indexNew](value, context)) {
					antiHashStates[jsonEntity.index] = indexNew;
					return;
				}
			}
			base::skipKeyStarted(context);
			++context.iter;
			JSONIFIER_SKIP_WS()
			if JSONIFIER_LIKELY (context.iter < context.endIter && *context.iter == ':') {
				++context.iter;
				JSONIFIER_SKIP_WS()
			} else [[unlikely]] {
				context.parserPtr->template reportError<parse_errors::Missing_Colon>(context);
				base::skipToNextValue(context);
				return;
			}
			base::skipToNextValue(context);
			return processIndexCall<json_entity_type, value_type, context_type>(value, context, wsStart, wsSize);
		}

		template<typename json_entity_type, typename value_type, typename context_type, bool haveWeStarted = false>
		JSONIFIER_INLINE void processIndex(value_type& value, context_type& context, const char* wsStart, size_t wsSize) const noexcept {
			static constexpr json_entity_type jsonEntity{};
			using base = derailleur<options, value_type, context_type>;
			if constexpr (memberCount > 0 && jsonEntity.index < memberCount) {
				if JSONIFIER_LIKELY (context.iter < context.endIter && *context.iter != rBrace) {
					if constexpr (haveWeStarted || jsonEntity.index > 0) {
						checkForEntryComma<options, value_type>(context, wsStart, wsSize);
					}
					if JSONIFIER_LIKELY (context.iter < context.endIter && *context.iter == quote) {
						++context.iter;
						processIndexImpl<json_entity_type>(value, context, wsStart, wsSize);
					}
					JSONIFIER_ELSE_UNLIKELY(else) {
						context.parserPtr->template reportError<parse_errors::Missing_String_Start>(context);
						base::skipKeyStarted(context);
						++context.iter;
						JSONIFIER_SKIP_WS()
						if JSONIFIER_LIKELY (context.iter < context.endIter && *context.iter == ':') {
							++context.iter;
							JSONIFIER_SKIP_WS()
						} else [[unlikely]] {
							context.parserPtr->template reportError<parse_errors::Missing_Colon>(context);
							base::skipToNextValue(context);
							return;
						}
						base::skipToNextValue(context);
						return;
					}
				}
			}
			return;
		}

		template<typename json_entity_type, typename value_type, typename context_type>
		JSONIFIER_INLINE void processIndexCall(value_type& value, context_type& context) const noexcept {
			return processIndex<json_entity_type, value_type, context_type, true>(value, context);
		}

		template<typename json_entity_type, typename value_type, typename context_type>
		JSONIFIER_INLINE void processIndexImpl(value_type& value, context_type& context) const noexcept {
			using base = derailleur<options, value_type, context_type>;
			static constexpr json_entity_type jsonEntity{};
			if constexpr (options.knownOrder) {
				if (antiHashStates[jsonEntity.index] == jsonEntity.index) {
					static constexpr auto key		 = jsonEntity.name;
					static constexpr auto keySize	 = key.size();
					static constexpr auto keySizeNew = keySize + 1;
					if JSONIFIER_LIKELY ((context.iter + keySize) < context.endIter && (*(context.iter + keySize)) == quote &&
						string_literal_comparitor<decltype(key), key>::impl(context.iter)) {
						context.iter += keySizeNew;
						if JSONIFIER_LIKELY (context.iter < context.endIter && *context.iter == colon) {
							++context.iter;
							if constexpr (jsonifier::concepts::has_excluded_keys<value_type>) {
								auto& keys = value.jsonifierExcludedKeys;
								if JSONIFIER_LIKELY (keys.find(static_cast<typename std::remove_cvref_t<decltype(keys)>::key_type>(key)) != keys.end()) {
									base::skipToNextValue(context);
									return;
								}
							}
							static constexpr auto ptrNew = jsonEntity.memberPtr;
							parse<options, jsonEntity, depth, maxDepth, options.minified>::impl(value.*ptrNew, context);
							return;
						}
						JSONIFIER_ELSE_UNLIKELY(else) {
							context.parserPtr->template reportError<parse_errors::Missing_Colon>(context);
							base::skipToNextValue(context);
						}
					}
				}
				JSONIFIER_ELSE_UNLIKELY(else) {
					if JSONIFIER_LIKELY (functionPtrsParse<value_type, context_type>[antiHashStates[jsonEntity.index]](value, context)) {
						return;
					}
				}
			}
			if JSONIFIER_LIKELY (auto indexNew = hash_map<value_type, std::remove_cvref_t<decltype(context.iter)>>::findIndex(context.iter, context.endIter);
				indexNew < memberCount) {
				if JSONIFIER_LIKELY (functionPtrsParse<value_type, context_type>[indexNew](value, context)) {
					antiHashStates[jsonEntity.index] = indexNew;
					return;
				}
			}
			base::skipKeyStarted(context);
			++context.iter;
			if JSONIFIER_LIKELY (context.iter < context.endIter && *context.iter == ':') {
				++context.iter;
			} else [[unlikely]] {
				context.parserPtr->template reportError<parse_errors::Missing_Colon>(context);
				base::skipToNextValue(context);
				return;
			}
			base::skipToNextValue(context);
			return processIndexCall<json_entity_type, value_type, context_type>(value, context);
		}

		template<typename json_entity_type, typename value_type, typename context_type, bool haveWeStarted = false>
		JSONIFIER_INLINE void processIndex(value_type& value, context_type& context) const noexcept {
			static constexpr json_entity_type jsonEntity{};
			using base = derailleur<options, value_type, context_type>;
			if constexpr (memberCount > 0 && jsonEntity.index < memberCount) {
				if JSONIFIER_LIKELY (context.iter < context.endIter && *context.iter != rBrace) {
					if constexpr (haveWeStarted || jsonEntity.index > 0) {
						checkForEntryComma<options, value_type>(context);
					}
					if JSONIFIER_LIKELY (context.iter < context.endIter && *context.iter == quote) {
						++context.iter;
						processIndexImpl<json_entity_type>(value, context);
					}
					JSONIFIER_ELSE_UNLIKELY(else) {
						context.parserPtr->template reportError<parse_errors::Missing_String_Start>(context);
						base::skipKeyStarted(context);
						++context.iter;
						if JSONIFIER_LIKELY (context.iter < context.endIter && *context.iter == ':') {
							++context.iter;
						} else [[unlikely]] {
							context.parserPtr->template reportError<parse_errors::Missing_Colon>(context);
							base::skipToNextValue(context);
							return;
						}
						base::skipToNextValue(context);
						return;
					}
				}
			}
			return;
		}

		template<typename... arg_types> JSONIFIER_INLINE void iterateValues(arg_types&&... args) const {
			((processIndex<bases>(jsonifier_internal::forward<arg_types>(args)...)), ...);
		}
	};

	template<jsonifier::parse_options options, typename parse_entity_pre, size_t depth, size_t maxDepth, size_t index, bool minifiedOrInsideRepeated>
	struct construct_parse_entity {
		using parse_entity_pre_type = std::remove_cvref_t<parse_entity_pre>;

		using type = json_entity<parse_entity_pre_type::memberPtr, parse_entity_pre_type::type, parse_entity_pre_type::name, index,
			getForceInline<jsonifier::core<std::remove_cvref_t<typename parse_entity_pre_type::class_type>>>()>;
	};

	template<jsonifier::parse_options options, typename value_type, size_t depth, size_t maxDepth, bool minifiedOrInsideRepeated, typename index_sequence>
	struct get_parse_entities;

	template<jsonifier::parse_options options, typename value_type, size_t depth, size_t maxDepth, bool minifiedOrInsideRepeated, size_t... I>
	struct get_parse_entities<options, value_type, depth, maxDepth, minifiedOrInsideRepeated, std::index_sequence<I...>> {
		using type = parse_entities<options, depth, maxDepth,
			typename construct_parse_entity<options, decltype(get<I>(jsonifier::core<std::remove_cvref_t<value_type>>::parseValue)), depth, maxDepth, I,
				minifiedOrInsideRepeated>::type...>;
	};

	template<jsonifier::parse_options options, typename value_type, size_t depth, size_t maxDepth, bool minifiedOrInsideRepeated> using parse_entities_t =
		typename get_parse_entities<options, value_type, depth, maxDepth, minifiedOrInsideRepeated, tag_range<tuple_size_v<raw_core_type<value_type>>>>::type;

	template<jsonifier::concepts::jsonifier_object_t value_type, typename context_type, jsonifier::parse_options options, auto jsonEntity, size_t depth, size_t maxDepth>
	struct object_val_parser<value_type, context_type, options, jsonEntity, depth, maxDepth, false> {
		using base = derailleur<options, value_type, context_type>;

		JSONIFIER_INLINE static void parseObjectOpening(value_type& value, context_type& context) noexcept {
			if JSONIFIER_LIKELY (context.iter < context.endIter && *context.iter == lBrace) {
				++context.iter;
				++context.currentObjectDepth;
				string_view_ptr wsStart = context.iter;
				JSONIFIER_SKIP_WS()
				size_t wsSize = static_cast<size_t>(context.iter - wsStart);
				static constexpr parse_entities_t<options, value_type, depth, maxDepth, options.minified> values{};
				values.iterateValues(value, context, wsStart, wsSize);
			}
			JSONIFIER_ELSE_UNLIKELY(else) {
				context.parserPtr->template reportError<parse_errors::Missing_Object_Start>(context);
				base::skipToNextValue(context);
			}
		}

		JSONIFIER_INLINE static void parseObjectEnding(context_type& context) noexcept {
			if JSONIFIER_LIKELY (context.iter < context.endIter && *context.iter == rBrace) {
				++context.iter;
				JSONIFIER_SKIP_WS()
				--context.currentObjectDepth;
			}
			JSONIFIER_ELSE_UNLIKELY(else) {
				base::template skipToEndOfValue<'{', '}'>(context);
			}
		}

		JSONIFIER_INLINE static void impl(value_type& value, context_type& context) noexcept {
			parseObjectOpening(value, context);
			parseObjectEnding(context);
		}
	};

	template<jsonifier::concepts::map_t value_type, typename context_type, jsonifier::parse_options options, auto jsonEntity, size_t depth, size_t maxDepth>
	struct object_val_parser<value_type, context_type, options, jsonEntity, depth, maxDepth, false> {
		using base = derailleur<options, value_type, context_type>;
		JSONIFIER_INLINE static void impl(value_type& value, context_type& context) noexcept {
			static thread_local typename std::remove_cvref_t<value_type>::key_type key{};
			if JSONIFIER_LIKELY ((context.iter + 1) < context.endIter && *context.iter == lBrace) {
				++context.iter;
				++context.currentObjectDepth;
				if JSONIFIER_LIKELY (*context.iter != rBrace) {
					const auto wsStart = context.iter;
					JSONIFIER_SKIP_WS()
					size_t wsSize{ static_cast<size_t>(context.iter - wsStart) };
					parse<options, jsonEntity, depth, maxDepth, options.minified>::impl(key, context);
					if JSONIFIER_LIKELY (context.iter < context.endIter && *context.iter == colon) {
						++context.iter;
						JSONIFIER_SKIP_WS()
						parse<options, jsonEntity, depth, maxDepth, options.minified>::impl(value[key], context);
					}
					JSONIFIER_ELSE_UNLIKELY(else) {
						context.parserPtr->template reportError<parse_errors::Missing_Colon>(context);
						base::skipToNextValue(context);
						return;
					}
					if (whitespaceTable[static_cast<uint8_t>(*(context.iter + wsSize))]) {
						return parseObjects<true>(value, context, wsStart, wsSize);
					}
					JSONIFIER_ELSE_UNLIKELY(else) {
						return parseObjects<false>(value, context, wsStart, wsSize);
					}
				}
				JSONIFIER_ELSE_UNLIKELY(else) {
					++context.iter;
					JSONIFIER_SKIP_WS()
					--context.currentObjectDepth;
				}
			}
			JSONIFIER_ELSE_UNLIKELY(else) {
				context.parserPtr->template reportError<parse_errors::Missing_Object_Start>(context);
				base::skipToNextValue(context);
				return;
			}
		}

		template<bool newLines> JSONIFIER_INLINE static void parseObjects(value_type& value, context_type& context, const auto wsStart = {}, size_t wsSize = {}) noexcept {
			static thread_local typename std::remove_cvref_t<value_type>::key_type key{};
			while
				JSONIFIER_LIKELY(context.iter < context.endIter && *context.iter != rBrace) {
					if (*context.iter == comma) {
						++context.iter;
						JSONIFIER_SKIP_MATCHING_WS();
						parse<options, jsonEntity, depth, maxDepth, options.minified>::impl(key, context);

						if JSONIFIER_LIKELY (context.iter < context.endIter && *context.iter == colon) {
							++context.iter;
							JSONIFIER_SKIP_WS()
							parse<options, jsonEntity, depth, maxDepth, options.minified>::impl(value[key], context);
						}
						JSONIFIER_ELSE_UNLIKELY(else) {
							context.parserPtr->template reportError<parse_errors::Missing_Colon>(context);
							base::skipToNextValue(context);
							return;
						}
					}
					JSONIFIER_ELSE_UNLIKELY(else) {
						context.parserPtr->template reportError<parse_errors::Missing_Comma>(context);
						base::skipToNextValue(context);
						return;
					}
				}
			++context.iter;
			--context.currentObjectDepth;
			JSONIFIER_SKIP_WS()
		}
	};

	JSONIFIER_FORCE_INLINE void noop() noexcept {};

	template<typename value_type> JSONIFIER_FORCE_INLINE auto getBeginIterVec(value_type& value) {
		if constexpr (std::is_same_v<typename value_type::value_type, bool>) {
			return value.begin();
		} else {
			return value.data();
		}
	}

	template<typename value_type> JSONIFIER_FORCE_INLINE auto getEndIterVec(value_type& value) {
		if constexpr (std::is_same_v<typename value_type::value_type, bool>) {
			return value.end();
		} else {
			return value.data() + value.size();
		}
	}

	template<jsonifier::concepts::vector_t value_type, typename context_type, jsonifier::parse_options options, auto jsonEntity, size_t depth, size_t maxDepth>
	struct array_val_parser<value_type, context_type, options, jsonEntity, depth, maxDepth, false> {
		using base = derailleur<options, value_type, context_type>;

		JSONIFIER_INLINE static void impl(value_type& value, context_type& context) noexcept {
			if JSONIFIER_LIKELY (context.iter + 1 < context.endIter) {
				if JSONIFIER_LIKELY (*context.iter == lBracket) {
					++context.currentArrayDepth;
					++context.iter;
					if JSONIFIER_LIKELY (*context.iter != rBracket) {
						const auto wsStart = context.iter;
						JSONIFIER_SKIP_WS()
						size_t wsSize{ static_cast<size_t>(context.iter - wsStart) };
						if (auto size = value.size(); size > 0) {
							parseObjectsWithSize<true>(value, context, size, wsStart, wsSize);
						} else {
							parseObjects<true>(value, context, wsStart, wsSize);
						}
					}
					++context.iter;
					JSONIFIER_SKIP_WS()
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

			for (size_t i{}; i < size; ++i) {
				parse<options, jsonEntity, depth, maxDepth, options.minified>::impl(newIter[i], context);

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
			parseObjects<newLines>(value, context, wsStart, wsSize);
		}

		template<bool newLines> JSONIFIER_INLINE static void parseObjects(value_type& value, context_type& context, const auto wsStart = {}, size_t wsSize = {}) {
			parse<options, jsonEntity, depth, maxDepth, options.minified>::impl(value.emplace_back(), context);

			while (context.iter < context.endIter && *context.iter == comma) {
				++context.iter;
				JSONIFIER_SKIP_MATCHING_WS();
				parse<options, jsonEntity, depth, maxDepth, options.minified>::impl(value.emplace_back(), context);
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

	template<jsonifier::concepts::raw_array_t value_type, typename context_type, jsonifier::parse_options options, auto jsonEntity, size_t depth, size_t maxDepth>
	struct array_val_parser<value_type, context_type, options, jsonEntity, depth, maxDepth, false> {
		using base = derailleur<options, value_type, context_type>;
		JSONIFIER_FORCE_INLINE static void impl(value_type& value, context_type& context) noexcept {
			if JSONIFIER_LIKELY ((context.iter + 1) < context.endIter && *context.iter == lBracket) {
				++context.currentArrayDepth;
				++context.iter;
				if JSONIFIER_LIKELY (*context.iter != rBracket) {
					const auto wsStart = context.iter;
					JSONIFIER_SKIP_WS()
					size_t wsSize{ static_cast<size_t>(context.iter - wsStart) };
					if (whitespaceTable[static_cast<uint8_t>(*(context.iter + wsSize))]) {
						return parseObjects<true>(value, context, wsStart, wsSize);
					}
					JSONIFIER_ELSE_UNLIKELY(else) {
						return parseObjects<false>(value, context, wsStart, wsSize);
					}
				}
				JSONIFIER_ELSE_UNLIKELY(else) {
					++context.iter;
					JSONIFIER_SKIP_WS()
					--context.currentArrayDepth;
				}
			}
			JSONIFIER_ELSE_UNLIKELY(else) {
				context.parserPtr->template reportError<parse_errors::Missing_Array_Start>(context);
				base::skipToNextValue(context);
			}
		}

		template<bool newLines> JSONIFIER_INLINE static void parseObjects(value_type& value, context_type& context, const auto wsStart = {}, size_t wsSize = {}) {
			if JSONIFIER_LIKELY (const size_t n = std::size(value); n > 0) {
				auto iterNew = std::begin(value);

				for (size_t i = 0; i < n; ++i) {
					parse<options, jsonEntity, depth, maxDepth, options.minified>::impl(*(iterNew++), context);

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
				}
			}
		}
	};

	template<jsonifier::concepts::tuple_t value_type, typename context_type, jsonifier::parse_options options, auto jsonEntity, size_t depth, size_t maxDepth>
	struct array_val_parser<value_type, context_type, options, jsonEntity, depth, maxDepth, false> {
		using base = derailleur<options, value_type, context_type>;
		JSONIFIER_INLINE static void impl(value_type& value, context_type& context) noexcept {
			static constexpr auto memberCount = tuple_size_v<std::remove_cvref_t<value_type>>;
			if JSONIFIER_LIKELY ((context.iter + 1) < context.endIter && *context.iter == lBracket) {
				++context.iter;
				JSONIFIER_SKIP_WS()
				++context.currentArrayDepth;
				if JSONIFIER_LIKELY (*context.iter != rBracket) {
					if constexpr (memberCount > 0) {
						parse<options, jsonEntity, depth, maxDepth, options.minified>::impl(get<0>(value), context);
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

		template<size_t memberCount, size_t index> JSONIFIER_INLINE static void parseObjects(value_type& value, context_type& context) {
			if constexpr (index < memberCount) {
				if JSONIFIER_LIKELY (context.iter < context.endIter && *context.iter != rBracket) {
					if JSONIFIER_LIKELY (*context.iter == comma) {
						++context.iter;
						JSONIFIER_SKIP_WS()
						parse<options, jsonEntity, depth, maxDepth, options.minified>::impl(get<index>(value), context);
						return parseObjects<memberCount, index + 1>(value, context);
					}
					JSONIFIER_ELSE_UNLIKELY(else) {
						context.parserPtr->template reportError<parse_errors::Missing_Comma>(context);
						base::skipToNextValue(context);
						return;
					}
				}
				JSONIFIER_ELSE_UNLIKELY(else) {
					++context.iter;
				}
			} else {
				++context.iter;
			}
		}
	};

	template<jsonifier::concepts::jsonifier_object_t value_type, typename context_type, jsonifier::parse_options options, auto jsonEntity, size_t depth, size_t maxDepth>
	struct object_val_parser<value_type, context_type, options, jsonEntity, depth, maxDepth, true> {
		using base = derailleur<options, value_type, context_type>;

		JSONIFIER_INLINE static void parseObjectOpening(value_type& value, context_type& context) noexcept {
			if JSONIFIER_LIKELY (context.iter < context.endIter && *context.iter == lBrace) {
				++context.iter;
				++context.currentObjectDepth;
				static constexpr parse_entities_t<options, value_type, depth, maxDepth, options.minified> values{};
				values.iterateValues(value, context);
			}
			JSONIFIER_ELSE_UNLIKELY(else) {
				context.parserPtr->template reportError<parse_errors::Missing_Object_Start>(context);
				base::skipToNextValue(context);
			}
		}

		JSONIFIER_INLINE static void parseObjectEnding(context_type& context) noexcept {
			if JSONIFIER_LIKELY (context.iter < context.endIter && *context.iter == rBrace) {
				++context.iter;
				--context.currentObjectDepth;
			}
			JSONIFIER_ELSE_UNLIKELY(else) {
				base::template skipToEndOfValue<'{', '}'>(context);
			}
		}

		JSONIFIER_INLINE static void impl(value_type& value, context_type& context) noexcept {
			parseObjectOpening(value, context);
			parseObjectEnding(context);
		}
	};

	template<jsonifier::concepts::map_t value_type, typename context_type, jsonifier::parse_options options, auto jsonEntity, size_t depth, size_t maxDepth>
	struct object_val_parser<value_type, context_type, options, jsonEntity, depth, maxDepth, true> {
		using base = derailleur<options, value_type, context_type>;

		JSONIFIER_INLINE static void impl(value_type& value, context_type& context) noexcept {
			static thread_local typename std::remove_cvref_t<value_type>::key_type key{};
			if JSONIFIER_LIKELY ((context.iter + 1) < context.endIter && *context.iter == lBrace) {
				++context.iter;
				++context.currentObjectDepth;
				if JSONIFIER_LIKELY (*context.iter != rBrace) {
					parse<options, jsonEntity, depth, maxDepth, options.minified>::impl(key, context);

					if JSONIFIER_LIKELY (context.iter < context.endIter && *context.iter == colon) {
						++context.iter;
						parse<options, jsonEntity, depth, maxDepth, options.minified>::impl(value[key], context);
					}
					JSONIFIER_ELSE_UNLIKELY(else) {
						context.parserPtr->template reportError<parse_errors::Missing_Colon>(context);
						base::skipToNextValue(context);
						return;
					}
					return parseObjects(value, context);
				}
				JSONIFIER_ELSE_UNLIKELY(else) {
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

		JSONIFIER_INLINE static void parseObjects(value_type& value, context_type& context) {
			static thread_local typename std::remove_cvref_t<value_type>::key_type key{};
			while
				JSONIFIER_LIKELY(context.iter < context.endIter && *context.iter != rBrace) {
					if (*context.iter == comma) {
						++context.iter;
						parse<options, jsonEntity, depth, maxDepth, options.minified>::impl(key, context);

						if JSONIFIER_LIKELY (context.iter < context.endIter && *context.iter == colon) {
							++context.iter;
							parse<options, jsonEntity, depth, maxDepth, options.minified>::impl(value[key], context);
						}
						JSONIFIER_ELSE_UNLIKELY(else) {
							context.parserPtr->template reportError<parse_errors::Missing_Colon>(context);
							base::skipToNextValue(context);
							return;
						}
					}
					JSONIFIER_ELSE_UNLIKELY(else) {
						context.parserPtr->template reportError<parse_errors::Missing_Comma>(context);
						base::skipToNextValue(context);
						return;
					}
				}
			++context.iter;
			--context.currentObjectDepth;
		}
	};

	template<jsonifier::concepts::vector_t value_type, typename context_type, jsonifier::parse_options options, auto jsonEntity, size_t depth, size_t maxDepth>
	struct array_val_parser<value_type, context_type, options, jsonEntity, depth, maxDepth, true> {
		using base = derailleur<options, value_type, context_type>;

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

			for (size_t i{}; i < size; ++i) {
				parse<options, jsonEntity, depth, maxDepth, options.minified>::impl(newIter[i], context);

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
			parse<options, jsonEntity, depth, maxDepth, options.minified>::impl(value.emplace_back(), context);

			while (context.iter < context.endIter && *context.iter == comma) {
				++context.iter;
				parse<options, jsonEntity, depth, maxDepth, options.minified>::impl(value.emplace_back(), context);
			}
			if (context.iter < context.endIter) {
				if JSONIFIER_LIKELY (*context.iter == rBracket) {
					return;
				} else {
					context.parserPtr->template reportError<parse_errors::Missing_Array_End>(context);
					base::skipToNextValue(context);
					return;
				}
			}
		}
	};

	template<jsonifier::concepts::raw_array_t value_type, typename context_type, jsonifier::parse_options options, auto jsonEntity, size_t depth, size_t maxDepth>
	struct array_val_parser<value_type, context_type, options, jsonEntity, depth, maxDepth, true> {
		using base = derailleur<options, value_type, context_type>;
		JSONIFIER_INLINE static void impl(value_type& value, context_type& context) noexcept {
			if JSONIFIER_LIKELY (context.iter < context.endIter && *context.iter == lBracket) {
				++context.currentArrayDepth;
				++context.iter;
				if JSONIFIER_LIKELY (*context.iter != rBracket) {
					auto iterNew = std::begin(value);

					for (size_t i = 0; i < value.size(); ++i) {
						parse<options, jsonEntity, depth, maxDepth, options.minified>::impl(*(iterNew++), context);

						if JSONIFIER_LIKELY (context.iter < context.endIter && *context.iter == comma) {
							++context.iter;
						}
						JSONIFIER_ELSE_UNLIKELY(else) {
							if JSONIFIER_LIKELY (context.iter < context.endIter && *context.iter == rBracket) {
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
				}
				JSONIFIER_ELSE_UNLIKELY(else) {
					++context.iter;
					--context.currentArrayDepth;
				}
			}
			JSONIFIER_ELSE_UNLIKELY(else) {
				context.parserPtr->template reportError<parse_errors::Missing_Array_Start>(context);
				base::skipToNextValue(context);
			}
		}
	};

	template<jsonifier::concepts::tuple_t value_type, typename context_type, jsonifier::parse_options options, auto jsonEntity, size_t depth, size_t maxDepth>
	struct array_val_parser<value_type, context_type, options, jsonEntity, depth, maxDepth, true> {
		using base = derailleur<options, value_type, context_type>;

		JSONIFIER_INLINE static void impl(value_type& value, context_type& context) noexcept {
			static constexpr auto memberCount = tuple_size_v<std::remove_cvref_t<value_type>>;
			if JSONIFIER_LIKELY ((context.iter + 1) < context.endIter && *context.iter == lBracket) {
				++context.iter;
				++context.currentArrayDepth;
				if JSONIFIER_LIKELY (*context.iter != rBracket) {
					if constexpr (memberCount > 0) {
						parse<options, jsonEntity, depth, maxDepth, options.minified>::impl(get<0>(value), context);
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

		template<size_t memberCount, size_t index> JSONIFIER_INLINE static void parseObjects(value_type& value, context_type& context) {
			if constexpr (index < memberCount) {
				if JSONIFIER_LIKELY (context.iter < context.endIter && *context.iter != rBracket) {
					if JSONIFIER_LIKELY (*context.iter == comma) {
						++context.iter;
						parse<options, jsonEntity, depth, maxDepth, options.minified>::impl(get<index>(value), context);
						return parseObjects<memberCount, index + 1>(value, context);
					}
					JSONIFIER_ELSE_UNLIKELY(else) {
						context.parserPtr->template reportError<parse_errors::Missing_Comma>(context);
						base::skipToNextValue(context);
						return;
					}
				}
				JSONIFIER_ELSE_UNLIKELY(else) {
					++context.iter;
				}
			} else {
				++context.iter;
			}
		}
	};

	template<jsonifier::concepts::string_t value_type, typename context_type, jsonifier::parse_options options, auto jsonEntity, size_t depth, size_t maxDepth, bool minified>
	struct string_val_parser<value_type, context_type, options, jsonEntity, depth, maxDepth, minified> {
		using base = derailleur<options, value_type, context_type>;

		JSONIFIER_FORCE_INLINE static void impl(value_type& value, context_type& context) noexcept {
			base::parseString(value, context);
			if constexpr (!minified) {
				JSONIFIER_SKIP_WS()
			}
		}
	};

	template<jsonifier::concepts::char_t value_type, typename context_type, jsonifier::parse_options options, auto jsonEntity, size_t depth, size_t maxDepth, bool minified>
	struct string_val_parser<value_type, context_type, options, jsonEntity, depth, maxDepth, minified> {
		using base = derailleur<options, value_type, context_type>;

		JSONIFIER_FORCE_INLINE static void impl(value_type& value, context_type& context) noexcept {
			value = static_cast<value_type>(*(context.iter + 1));
			++context.iter;
			if constexpr (!minified) {
				JSONIFIER_SKIP_WS()
			}
		}
	};

	template<jsonifier::concepts::enum_t value_type, typename context_type, jsonifier::parse_options options, auto jsonEntity, size_t depth, size_t maxDepth, bool minified>
	struct number_val_parser<value_type, context_type, options, jsonEntity, depth, maxDepth, minified> {
		using base = derailleur<options, value_type, context_type>;

		JSONIFIER_FORCE_INLINE static void impl(value_type& value, context_type& context) noexcept {
			size_t newValue{};
			if JSONIFIER_LIKELY (parseNumber(newValue, context.iter, context.endIter)) {
				value = static_cast<value_type>(newValue);
				if constexpr (!minified) {
					JSONIFIER_SKIP_WS()
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

	template<jsonifier::concepts::num_t value_type, typename context_type, jsonifier::parse_options options, auto jsonEntity, size_t depth, size_t maxDepth, bool minified>
	struct number_val_parser<value_type, context_type, options, jsonEntity, depth, maxDepth, minified> {
		using base = derailleur<options, value_type, context_type>;

		JSONIFIER_FORCE_INLINE static void impl(value_type& value, context_type& context) noexcept {
			if JSONIFIER_LIKELY (parseNumber(value, context.iter, context.endIter)) {
				if constexpr (!minified) {
					JSONIFIER_SKIP_WS()
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

	template<jsonifier::concepts::bool_t value_type, typename context_type, jsonifier::parse_options options, auto jsonEntity, size_t depth, size_t maxDepth, bool minified>
	struct bool_val_parser<value_type, context_type, options, jsonEntity, depth, maxDepth, minified> {
		using base = derailleur<options, value_type, context_type>;

		JSONIFIER_FORCE_INLINE static void impl(value_type& value, context_type& context) noexcept {
			if JSONIFIER_LIKELY ((context.iter + 4) < context.endIter && parseBool(value, context.iter)) {
				if constexpr (!minified) {
					JSONIFIER_SKIP_WS()
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

	template<jsonifier::concepts::always_null_t value_type, typename context_type, jsonifier::parse_options options, auto jsonEntity, size_t depth, size_t maxDepth, bool minified>
	struct null_val_parser<value_type, context_type, options, jsonEntity, depth, maxDepth, minified> {
		using base = derailleur<options, value_type, context_type>;

		JSONIFIER_FORCE_INLINE static void impl(value_type&, context_type& context) noexcept {
			if JSONIFIER_LIKELY (parseNull(context.iter)) {
				if constexpr (!minified) {
					JSONIFIER_SKIP_WS()
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

	template<jsonifier::concepts::variant_t value_type, typename context_type, jsonifier::parse_options options, auto jsonEntity, size_t depth, size_t maxDepth, bool minified>
	struct accessor_val_parser<value_type, context_type, options, jsonEntity, depth, maxDepth, minified> {
		using base = derailleur<options, value_type, context_type>;

		template<jsonifier::json_type type, typename variant_type, size_t currentIndex = 0>
		JSONIFIER_FORCE_INLINE static constexpr void iterateVariantTypes(variant_type&& variant, context_type& context) noexcept {
			if constexpr (currentIndex < std::variant_size_v<std::remove_cvref_t<variant_type>>) {
				using element_type = std::remove_cvref_t<decltype(std::get<currentIndex>(std::declval<std::remove_cvref_t<variant_type>>()))>;
				if constexpr (jsonifier::concepts::jsonifier_object_t<element_type> && type == jsonifier::json_type::object) {
					parse<options, jsonEntity, depth, maxDepth, options.minified>::impl(variant.template emplace<element_type>(element_type{}), context);
				} else if constexpr ((jsonifier::concepts::vector_t<element_type> || jsonifier::concepts::raw_array_t<element_type>) && type == jsonifier::json_type::array) {
					parse<options, jsonEntity, depth, maxDepth, options.minified>::impl(variant.template emplace<element_type>(element_type{}), context);
				} else if constexpr ((jsonifier::concepts::string_t<element_type> || jsonifier::concepts::string_view_t<element_type>) && type == jsonifier::json_type::string) {
					parse<options, jsonEntity, depth, maxDepth, options.minified>::impl(variant.template emplace<element_type>(element_type{}), context);
				} else if constexpr (jsonifier::concepts::bool_t<element_type> && type == jsonifier::json_type::boolean) {
					parse<options, jsonEntity, depth, maxDepth, options.minified>::impl(variant.template emplace<element_type>(element_type{}), context);
				} else if constexpr ((jsonifier::concepts::num_t<element_type> || jsonifier::concepts::enum_t<element_type>) && type == jsonifier::json_type::number) {
					parse<options, jsonEntity, depth, maxDepth, options.minified>::impl(variant.template emplace<element_type>(element_type{}), context);
				} else if constexpr (jsonifier::concepts::always_null_t<element_type> && type == jsonifier::json_type::null) {
					parse<options, jsonEntity, depth, maxDepth, options.minified>::impl(variant.template emplace<element_type>(element_type{}), context);
				} else if constexpr (jsonifier::concepts::accessor_t<element_type> && type == jsonifier::json_type::accessor) {
					parse<options, jsonEntity, depth, maxDepth, options.minified>::impl(variant.template emplace<element_type>(element_type{}), context);
				} else {
					return iterateVariantTypes<type, variant_type, currentIndex + 1>(variant, context);
				}
			}
		}

		JSONIFIER_FORCE_INLINE static void impl(value_type& value, context_type& context) noexcept {
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
	};

	template<jsonifier::concepts::optional_t value_type, typename context_type, jsonifier::parse_options options, auto jsonEntity, size_t depth, size_t maxDepth, bool minified>
	struct accessor_val_parser<value_type, context_type, options, jsonEntity, depth, maxDepth, minified> {
		using base = derailleur<options, value_type, context_type>;

		JSONIFIER_FORCE_INLINE static void impl(value_type& value, context_type& context) noexcept {
			if JSONIFIER_LIKELY (context.iter < context.endIter && *context.iter != n) {
				parse<options, jsonEntity, depth, maxDepth, options.minified>::impl(value.emplace(), context);
			}
			JSONIFIER_ELSE_UNLIKELY(else) {
				if JSONIFIER_LIKELY (parseNull(context.iter)) {
					if constexpr (!minified) {
						JSONIFIER_SKIP_WS()
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

	template<jsonifier::concepts::shared_ptr_t value_type, typename context_type, jsonifier::parse_options options, auto jsonEntity, size_t depth, size_t maxDepth, bool minified>
	struct accessor_val_parser<value_type, context_type, options, jsonEntity, depth, maxDepth, minified> {
		using base = derailleur<options, value_type, context_type>;

		JSONIFIER_FORCE_INLINE static void impl(value_type& value, context_type& context) noexcept {
			if JSONIFIER_LIKELY (context.iter < context.endIter && *context.iter != n) {
				using member_type = decltype(*value);
				if JSONIFIER_UNLIKELY (!value) {
					value = std::make_shared<std::remove_pointer_t<std::remove_cvref_t<member_type>>>();
				}
				parse<options, jsonEntity, depth, maxDepth, options.minified>::impl(*value, context);
			}
			JSONIFIER_ELSE_UNLIKELY(else) {
				if JSONIFIER_LIKELY (parseNull(context.iter)) {
					if constexpr (!minified) {
						JSONIFIER_SKIP_WS()
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

	template<jsonifier::concepts::unique_ptr_t value_type, typename context_type, jsonifier::parse_options options, auto jsonEntity, size_t depth, size_t maxDepth, bool minified>
	struct accessor_val_parser<value_type, context_type, options, jsonEntity, depth, maxDepth, minified> {
		using base = derailleur<options, value_type, context_type>;

		JSONIFIER_FORCE_INLINE static void impl(value_type& value, context_type& context) noexcept {
			if JSONIFIER_LIKELY (context.iter < context.endIter && *context.iter != n) {
				using member_type = decltype(*value);
				if JSONIFIER_UNLIKELY (!value) {
					value = std::make_unique<std::remove_pointer_t<std::remove_cvref_t<member_type>>>();
				}
				parse<options, jsonEntity, depth, maxDepth, options.minified>::impl(*value, context);
			}
			JSONIFIER_ELSE_UNLIKELY(else) {
				if JSONIFIER_LIKELY (parseNull(context.iter)) {
					if constexpr (!minified) {
						JSONIFIER_SKIP_WS()
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

	template<jsonifier::concepts::pointer_t value_type, typename context_type, jsonifier::parse_options options, auto jsonEntity, size_t depth, size_t maxDepth, bool minified>
	struct accessor_val_parser<value_type, context_type, options, jsonEntity, depth, maxDepth, minified> {
		using base = derailleur<options, value_type, context_type>;

		JSONIFIER_FORCE_INLINE static void impl(value_type& value, context_type& context) noexcept {
			if JSONIFIER_LIKELY (context.iter < context.endIter && *context.iter != n) {
				if JSONIFIER_UNLIKELY (!value) {
					value = new std::remove_pointer_t<std::remove_cvref_t<value_type>>{};
				}
				parse<options, jsonEntity, depth, maxDepth, options.minified>::impl(*value, context);
			}
			JSONIFIER_ELSE_UNLIKELY(else) {
				if JSONIFIER_LIKELY (parseNull(context.iter)) {
					if constexpr (!minified) {
						JSONIFIER_SKIP_WS()
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

	template<jsonifier::concepts::raw_json_t value_type, typename context_type, jsonifier::parse_options options, auto jsonEntity, size_t depth, size_t maxDepth, bool minified>
	struct accessor_val_parser<value_type, context_type, options, jsonEntity, depth, maxDepth, minified> {
		using base = derailleur<options, value_type, context_type>;

		JSONIFIER_FORCE_INLINE static void impl(value_type& value, context_type& context) noexcept {
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
	};

	template<jsonifier::concepts::skip_t value_type, typename context_type, jsonifier::parse_options options, auto jsonEntity, size_t depth, size_t maxDepth, bool minified>
	struct accessor_val_parser<value_type, context_type, options, jsonEntity, depth, maxDepth, minified> {
		using base = derailleur<options, value_type, context_type>;

		JSONIFIER_FORCE_INLINE static void impl(value_type& value, context_type& context) noexcept {
			base::skipToNextValue(context);
		}
	};

	template<jsonifier::concepts::skip_t value_type, typename context_type, jsonifier::parse_options options, auto jsonEntity, size_t depth, size_t maxDepth, bool minified>
	struct custom_val_parser<value_type, context_type, options, jsonEntity, depth, maxDepth, minified> {
		using base = derailleur<options, value_type, context_type>;

		JSONIFIER_FORCE_INLINE static void impl(value_type& value, context_type& context) noexcept {
			base::skipToNextValue(context);
		}
	};

	template<jsonifier::parse_options options, size_t depth, size_t maxDepth, bool minifiedOrInsideRepeated, typename... bases> struct parse_entities_partial : public bases... {
		using type_list_new = type_list<bases...>;
		static constexpr auto memberCount{ sizeof...(bases) };
		inline static thread_local array<uint64_t, (memberCount > 0 ? memberCount : 1)> antiHashStates{ [] {
			array<uint64_t, (memberCount > 0 ? memberCount : 1)> returnValues{};
			for (uint64_t x = 0; x < memberCount; ++x) {
				returnValues[x] = x;
			}
			return returnValues;
		}() };

		template<size_t index, typename value_type, typename context_type> JSONIFIER_INLINE static bool processIndex(value_type& value, context_type& context) noexcept {
			return processIndexForPtrs<typename get_type_at_index<type_list_new, index>::type>(value, context);
		}

		template<typename json_entity_type, typename value_type, typename context_type>
		JSONIFIER_INLINE static bool processIndexForPtrs(value_type& value, context_type& context) noexcept {
			using base = derailleur<options, value_type, context_type>;
			static constexpr json_entity_type jsonEntity{};
			static constexpr auto stringLiteral = jsonEntity.name;
			static constexpr auto keySize		= stringLiteral.size();
			static constexpr auto key			= stringLiteral.template view<jsonifier::string_view>();
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
					static constexpr auto ptrNew = jsonEntity.memberPtr;
					parse<options, jsonEntity, depth, maxDepth, minifiedOrInsideRepeated>::impl(value.*ptrNew, context);
					return true;
				}
				JSONIFIER_ELSE_UNLIKELY(else) {
					context.parserPtr->template reportError<parse_errors::Missing_Colon>(context);
					base::skipToNextValue(context);
				}
			}
			return false;
		}

		template<size_t index, typename value_type, typename context_type> using process_index_function_type = bool (*)(value_type&, context_type&) noexcept;

		template<typename value_type, typename context_type, size_t... indices> static constexpr auto generateFunctionPtrsParse(std::index_sequence<indices...>) {
			using function_type = process_index_function_type<0, value_type, context_type>;
			return array<function_type, sizeof...(indices)>{
				&parse_entities_partial<options, depth, maxDepth, minifiedOrInsideRepeated, bases...>::template processIndex<indices, value_type, context_type>...
			};
		}

		template<typename value_type, typename context_type>
		static constexpr auto functionPtrsParse{ generateFunctionPtrsParse<value_type, context_type>(std::make_index_sequence<sizeof...(bases)>{}) };

		template<typename json_entity_type, typename value_type, typename context_type>
		JSONIFIER_INLINE void processIndexCall(value_type& value, context_type& context) const noexcept {
			return processIndex<json_entity_type, value_type, context_type, true>(value, context);
		}

		template<typename json_entity_type, typename value_type, typename context_type>
		JSONIFIER_INLINE void processIndexImpl(value_type& value, context_type& context) const noexcept {
			using base = derailleur<options, value_type, context_type>;
			if JSONIFIER_LIKELY (auto indexNew = hash_map<value_type, std::remove_cvref_t<decltype(*context.iter)>>::findIndex((*context.iter) + 1, *context.endIter);
				indexNew < memberCount) {
				if JSONIFIER_LIKELY (functionPtrsParse<value_type, context_type>[indexNew](value, context)) {
					return;
				}
			}
			base::skipKeyStarted(context);
			if JSONIFIER_LIKELY (context.iter < context.endIter && **context.iter == ':') {
				++context.iter;
			} else [[unlikely]] {
				context.parserPtr->template reportError<parse_errors::Missing_Colon>(context);
				base::skipToNextValue(context);
				return;
			}
			base::skipToNextValue(context);
			return processIndexCall<json_entity_type, value_type, context_type>(value, context);
		}

		template<typename json_entity_type, typename value_type, typename context_type, bool haveWeStarted = false>
		JSONIFIER_INLINE void processIndex(value_type& value, context_type& context) const noexcept {
			static constexpr json_entity_type jsonEntity{};
			using base = derailleur<options, value_type, context_type>;
			if constexpr (memberCount > 0 && jsonEntity.index < memberCount) {
				if JSONIFIER_LIKELY (context.iter < context.endIter && **context.iter != rBrace) {
					if constexpr (haveWeStarted || jsonEntity.index > 0) {
						checkForEntryComma<options, value_type>(context);
					}
					if JSONIFIER_LIKELY (context.iter < context.endIter && **context.iter == quote) {
						processIndexImpl<json_entity_type>(value, context);
					}
					JSONIFIER_ELSE_UNLIKELY(else) {
						context.parserPtr->template reportError<parse_errors::Missing_String_Start>(context);
						base::skipKeyStarted(context);
						++context.iter;
						if JSONIFIER_LIKELY (context.iter < context.endIter && **context.iter == ':') {
							++context.iter;
						} else [[unlikely]] {
							context.parserPtr->template reportError<parse_errors::Missing_Colon>(context);
							base::skipToNextValue(context);
							return;
						}
						base::skipToNextValue(context);
						return;
					}
				}
			}
			return;
		}

		template<typename... arg_types> JSONIFIER_INLINE void iterateValues(arg_types&&... args) const {
			((processIndex<bases>(jsonifier_internal::forward<arg_types>(args)...)), ...);
		}
	};

	template<jsonifier::parse_options options, typename parse_partial_entity_pre, size_t depth, size_t maxDepth, size_t index, bool minifiedOrInsideRepeated>
	struct construct_parse_partial_entity {
		using parse_partial_entity_pre_type = std::remove_cvref_t<parse_partial_entity_pre>;

		using type = json_entity<parse_partial_entity_pre_type::memberPtr, parse_partial_entity_pre_type::type, parse_partial_entity_pre_type::name, index,
			getForceInline<jsonifier::core<std::remove_cvref_t<typename parse_partial_entity_pre_type::class_type>>>()>;
	};

	template<jsonifier::parse_options options, typename value_type, size_t depth, size_t maxDepth, bool minifiedOrInsideRepeated, typename index_sequence>
	struct get_parse_partial_entities;

	template<jsonifier::parse_options options, typename value_type, size_t depth, size_t maxDepth, bool minifiedOrInsideRepeated, size_t... I>
	struct get_parse_partial_entities<options, value_type, depth, maxDepth, minifiedOrInsideRepeated, std::index_sequence<I...>> {
		using type = parse_entities_partial<options, depth, maxDepth, minifiedOrInsideRepeated,
			typename construct_parse_partial_entity<options, decltype(get<I>(jsonifier::core<std::remove_cvref_t<value_type>>::parseValue)), depth, maxDepth, I,
				minifiedOrInsideRepeated>::type...>;
	};

	template<jsonifier::parse_options options, typename value_type, size_t depth, size_t maxDepth, bool minifiedOrInsideRepeated> using parse_partial_entities_t =
		typename get_parse_partial_entities<options, value_type, depth, maxDepth, minifiedOrInsideRepeated, tag_range<tuple_size_v<raw_core_type<value_type>>>>::type;

	template<jsonifier::concepts::jsonifier_object_t value_type, typename context_type, jsonifier::parse_options options, auto jsonEntity, size_t depth, size_t maxDepth,
		bool minifiedOrInsideRepeated>
	struct object_val_parser_partial<value_type, context_type, options, jsonEntity, depth, maxDepth, minifiedOrInsideRepeated> {
		using base = derailleur<options, value_type, context_type>;

		JSONIFIER_INLINE static void impl(value_type& value, context_type& context) noexcept {
			if JSONIFIER_LIKELY (context.iter < context.endIter && **context.iter == lBrace) {
				++context.iter;
				++context.currentObjectDepth;
				static constexpr parse_partial_entities_t<options, value_type, depth, maxDepth, minifiedOrInsideRepeated> values{};
				values.iterateValues(value, context);
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
	};

	template<jsonifier::concepts::map_t value_type, typename context_type, jsonifier::parse_options options, auto jsonEntity, size_t depth, size_t maxDepth,
		bool minifiedOrInsideRepeated>
	struct object_val_parser_partial<value_type, context_type, options, jsonEntity, depth, maxDepth, minifiedOrInsideRepeated> {
		using base = derailleur<options, value_type, context_type>;

		JSONIFIER_INLINE static void impl(value_type& value, context_type& context) noexcept {
			if JSONIFIER_LIKELY ((context.iter + 1) < context.endIter && **context.iter == lBrace) {
				++context.iter;
				++context.currentObjectDepth;
				if JSONIFIER_LIKELY (**context.iter != rBrace) {
					static thread_local typename std::remove_cvref_t<value_type>::key_type key{};
					parse<options, jsonEntity, depth, maxDepth, minifiedOrInsideRepeated>::impl(key, context);

					if JSONIFIER_LIKELY (context.iter < context.endIter && **context.iter == colon) {
						++context.iter;
						parse<options, jsonEntity, depth, maxDepth, minifiedOrInsideRepeated>::impl(value[key], context);
					}
					JSONIFIER_ELSE_UNLIKELY(else) {
						context.parserPtr->template reportError<parse_errors::Missing_Colon>(context);
						base::skipToNextValue(context);
						return;
					}

					while (context.iter < context.endIter && **context.iter != rBrace) {
						if JSONIFIER_LIKELY (**context.iter == comma) {
							++context.iter;
							parse<options, jsonEntity, depth, maxDepth, minifiedOrInsideRepeated>::impl(key, context);

							if JSONIFIER_LIKELY (context.iter < context.endIter && **context.iter == colon) {
								++context.iter;
								parse<options, jsonEntity, depth, maxDepth, minifiedOrInsideRepeated>::impl(value[key], context);
							}
							JSONIFIER_ELSE_UNLIKELY(else) {
								context.parserPtr->template reportError<parse_errors::Missing_Colon>(context);
								base::skipToNextValue(context);
								return;
							}
						}
						JSONIFIER_ELSE_UNLIKELY(else) {
							context.parserPtr->template reportError<parse_errors::Missing_Comma>(context);
							base::skipToNextValue(context);
							return;
						}
					}
					++context.iter;
					--context.currentObjectDepth;
				}
				JSONIFIER_ELSE_UNLIKELY(else) {
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
	};

	template<jsonifier::concepts::vector_t value_type, typename context_type, jsonifier::parse_options options, auto jsonEntity, size_t depth, size_t maxDepth,
		bool minifiedOrInsideRepeated>
	struct array_val_parser_partial<value_type, context_type, options, jsonEntity, depth, maxDepth, minifiedOrInsideRepeated> {
		using base = derailleur<options, value_type, context_type>;

		JSONIFIER_INLINE static void impl(value_type& value, context_type& context) noexcept {
			if JSONIFIER_LIKELY ((context.iter + 1) < context.endIter && **context.iter == lBracket) {
				++context.currentArrayDepth;
				++context.iter;
				if JSONIFIER_LIKELY (**context.iter != rBracket) {
					if (auto size = value.size(); size > 0) {
						parseObjectsWithSize(value, context, size);
					}
					JSONIFIER_ELSE_UNLIKELY(else) {
						parseObjects(value, context);
					}
				}
				++context.iter;
				--context.currentArrayDepth;
			}
			JSONIFIER_ELSE_UNLIKELY(else) {
				context.parserPtr->template reportError<parse_errors::Missing_Array_Start>(context);
				base::skipToNextValue(context);
			}
		}

		JSONIFIER_INLINE static void parseObjectsWithSize(value_type& value, context_type& context, size_t size) noexcept {
			auto newIter = getBeginIterVec(value);

			for (size_t i{}; i < size; ++i) {
				parse<options, jsonEntity, depth, maxDepth, minifiedOrInsideRepeated>::impl(newIter[i], context);

				if JSONIFIER_LIKELY (context.iter < context.endIter) {
					if JSONIFIER_LIKELY (**context.iter == comma) {
						++context.iter;
					}
					JSONIFIER_ELSE_UNLIKELY(else) {
						if JSONIFIER_LIKELY (**context.iter == rBracket) {
							return (i + 1 < size) ? value.resize(i + 1) : noop();
						}
						JSONIFIER_ELSE_UNLIKELY(else) {
							context.parserPtr->template reportError<parse_errors::Missing_Array_End>(context);
							base::skipToNextValue(context);
							return;
						}
					}
				}
				JSONIFIER_ELSE_UNLIKELY(else) {
					return;
				}
			}
			parseObjects(value, context);
		}

		JSONIFIER_INLINE static void parseObjects(value_type& value, context_type& context) {
			parse<options, jsonEntity, depth, maxDepth, minifiedOrInsideRepeated>::impl(value.emplace_back(), context);

			while (context.iter < context.endIter && **context.iter == comma) {
				++context.iter;
				parse<options, jsonEntity, depth, maxDepth, minifiedOrInsideRepeated>::impl(value.emplace_back(), context);
			}
			if JSONIFIER_LIKELY (context.iter < context.endIter) {
				if JSONIFIER_LIKELY (**context.iter == rBracket) {
					return;
				}
				JSONIFIER_ELSE_UNLIKELY(else) {
					context.parserPtr->template reportError<parse_errors::Missing_Array_End>(context);
					base::skipToNextValue(context);
					return;
				}
			}
		}
	};

	template<jsonifier::concepts::raw_array_t value_type, typename context_type, jsonifier::parse_options options, auto jsonEntity, size_t depth, size_t maxDepth,
		bool minifiedOrInsideRepeated>
	struct array_val_parser_partial<value_type, context_type, options, jsonEntity, depth, maxDepth, minifiedOrInsideRepeated> {
		using base = derailleur<options, value_type, context_type>;

		JSONIFIER_INLINE static void impl(value_type& value, context_type& context) noexcept {
			if JSONIFIER_LIKELY ((context.iter + 1) < context.endIter && **context.iter == lBracket) {
				++context.currentArrayDepth;
				++context.iter;
				if JSONIFIER_LIKELY (**context.iter != rBracket) {
					auto iterNew = std::begin(value);

					for (size_t i = 0; i < value.size(); ++i) {
						parse<options, jsonEntity, depth, maxDepth, minifiedOrInsideRepeated>::impl(*(iterNew++), context);

						if JSONIFIER_LIKELY (context.iter < context.endIter && **context.iter == comma) {
							++context.iter;
						}
						JSONIFIER_ELSE_UNLIKELY(else) {
							if JSONIFIER_LIKELY (context.iter < context.endIter && **context.iter == rBracket) {
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
				}
				JSONIFIER_ELSE_UNLIKELY(else) {
					++context.iter;
					--context.currentArrayDepth;
				}
			}
			JSONIFIER_ELSE_UNLIKELY(else) {
				context.parserPtr->template reportError<parse_errors::Missing_Array_Start>(context);
				base::skipToNextValue(context);
			}
		}
	};

	template<jsonifier::concepts::tuple_t value_type, typename context_type, jsonifier::parse_options options, auto jsonEntity, size_t depth, size_t maxDepth,
		bool minifiedOrInsideRepeated>
	struct object_val_parser_partial<value_type, context_type, options, jsonEntity, depth, maxDepth, minifiedOrInsideRepeated> {
		using base = derailleur<options, value_type, context_type>;

		JSONIFIER_INLINE static void impl(value_type& value, context_type& context) noexcept {
			static constexpr auto memberCount = tuple_size_v<std::remove_cvref_t<value_type>>;
			if JSONIFIER_LIKELY ((context.iter + 1) < context.endIter && **context.iter == lBracket) {
				++context.iter;
				++context.currentArrayDepth;
				if JSONIFIER_LIKELY (**context.iter != rBracket) {
					if constexpr (memberCount > 0) {
						parse<options, jsonEntity, depth, maxDepth, minifiedOrInsideRepeated>::impl(get<0>(value), context);
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

		template<size_t memberCount, size_t index> JSONIFIER_INLINE static void parseObjects(value_type& value, context_type& context) {
			if constexpr (index < memberCount) {
				if JSONIFIER_LIKELY (context.iter < context.endIter && **context.iter != rBracket) {
					if JSONIFIER_LIKELY (**context.iter == comma) {
						++context.iter;
						parse<options, jsonEntity, depth, maxDepth, minifiedOrInsideRepeated>::impl(get<index>(value), context);
						return parseObjects<memberCount, index + 1>(value, context);
					}
					JSONIFIER_ELSE_UNLIKELY(else) {
						context.parserPtr->template reportError<parse_errors::Missing_Comma>(context);
						base::skipToNextValue(context);
						return;
					}
				}
				JSONIFIER_ELSE_UNLIKELY(else) {
					++context.iter;
				}
			} else {
				++context.iter;
			}
		}
	};

	template<jsonifier::concepts::string_t value_type, typename context_type, jsonifier::parse_options options, auto jsonEntity, size_t depth, size_t maxDepth,
		bool minifiedOrInsideRepeated>
	struct string_val_parser_partial<value_type, context_type, options, jsonEntity, depth, maxDepth, minifiedOrInsideRepeated> {
		using base = derailleur<options, value_type, context_type>;

		JSONIFIER_FORCE_INLINE static void impl(value_type& value, context_type& context) noexcept {
			base::parseString(value, context);
		}
	};


	template<jsonifier::concepts::char_t value_type, typename context_type, jsonifier::parse_options options, auto jsonEntity, size_t depth, size_t maxDepth,
		bool minifiedOrInsideRepeated>
	struct string_val_parser_partial<value_type, context_type, options, jsonEntity, depth, maxDepth, minifiedOrInsideRepeated> {
		using base = derailleur<options, value_type, context_type>;

		JSONIFIER_FORCE_INLINE static void impl(value_type& value, context_type& context) noexcept {
			value = static_cast<value_type>(*(*context.iter + 1));
			++context.iter;
		}
	};

	template<jsonifier::concepts::enum_t value_type, typename context_type, jsonifier::parse_options options, auto jsonEntity, size_t depth, size_t maxDepth,
		bool minifiedOrInsideRepeated>
	struct number_val_parser_partial<value_type, context_type, options, jsonEntity, depth, maxDepth, minifiedOrInsideRepeated> {
		using base = derailleur<options, value_type, context_type>;

		JSONIFIER_FORCE_INLINE static void impl(value_type& value, context_type& context) noexcept {
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

	template<jsonifier::concepts::num_t value_type, typename context_type, jsonifier::parse_options options, auto jsonEntity, size_t depth, size_t maxDepth,
		bool minifiedOrInsideRepeated>
	struct number_val_parser_partial<value_type, context_type, options, jsonEntity, depth, maxDepth, minifiedOrInsideRepeated> {
		using base = derailleur<options, value_type, context_type>;

		JSONIFIER_FORCE_INLINE static void impl(value_type& value, context_type& context) noexcept {
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

	template<jsonifier::concepts::bool_t value_type, typename context_type, jsonifier::parse_options options, auto jsonEntity, size_t depth, size_t maxDepth,
		bool minifiedOrInsideRepeated>
	struct bool_val_parser_partial<value_type, context_type, options, jsonEntity, depth, maxDepth, minifiedOrInsideRepeated> {
		using base = derailleur<options, value_type, context_type>;

		JSONIFIER_FORCE_INLINE static void impl(value_type& value, context_type& context) noexcept {
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

	template<jsonifier::concepts::always_null_t value_type, typename context_type, jsonifier::parse_options options, auto jsonEntity, size_t depth, size_t maxDepth,
		bool minifiedOrInsideRepeated>
	struct null_val_parser_partial<value_type, context_type, options, jsonEntity, depth, maxDepth, minifiedOrInsideRepeated> {
		using base = derailleur<options, value_type, context_type>;

		JSONIFIER_FORCE_INLINE static void impl(value_type& value, context_type& context) noexcept {
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

	template<jsonifier::concepts::variant_t value_type, typename context_type, jsonifier::parse_options options, auto jsonEntity, size_t depth, size_t maxDepth,
		bool minifiedOrInsideRepeated>
	struct accessor_val_parser_partial<value_type, context_type, options, jsonEntity, depth, maxDepth, minifiedOrInsideRepeated> {
		using base = derailleur<options, value_type, context_type>;

		JSONIFIER_FORCE_INLINE static void impl(value_type& value, context_type& context) noexcept {
			static constexpr auto lambda = [](auto&& valueNew, auto&& context) {
				parse<options, jsonEntity, depth, maxDepth, minifiedOrInsideRepeated>::impl(valueNew, context);
			};
			visit<lambda>(value, context);
		}
	};

	template<jsonifier::concepts::optional_t value_type, typename context_type, jsonifier::parse_options options, auto jsonEntity, size_t depth, size_t maxDepth,
		bool minifiedOrInsideRepeated>
	struct accessor_val_parser_partial<value_type, context_type, options, jsonEntity, depth, maxDepth, minifiedOrInsideRepeated> {
		using base = derailleur<options, value_type, context_type>;

		JSONIFIER_FORCE_INLINE static void impl(value_type& value, context_type& context) noexcept {
			if JSONIFIER_LIKELY (context.iter < context.endIter && **context.iter != n) {
				parse<options, jsonEntity, depth, maxDepth, minifiedOrInsideRepeated>::impl(value.emplace(), context);
			}
			JSONIFIER_ELSE_UNLIKELY(else) {
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

	template<jsonifier::concepts::shared_ptr_t value_type, typename context_type, jsonifier::parse_options options, auto jsonEntity, size_t depth, size_t maxDepth,
		bool minifiedOrInsideRepeated>
	struct accessor_val_parser_partial<value_type, context_type, options, jsonEntity, depth, maxDepth, minifiedOrInsideRepeated> {
		using base = derailleur<options, value_type, context_type>;

		JSONIFIER_FORCE_INLINE static void impl(value_type& value, context_type& context) noexcept {
			if JSONIFIER_LIKELY (context.iter < context.endIter && **context.iter != n) {
				using member_type = decltype(*value);
				if JSONIFIER_UNLIKELY (!value) {
					value = std::make_shared<std::remove_pointer_t<std::remove_cvref_t<member_type>>>();
				}
				parse<options, jsonEntity, depth, maxDepth, minifiedOrInsideRepeated>::impl(*value, context);
			}
			JSONIFIER_ELSE_UNLIKELY(else) {
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

	template<jsonifier::concepts::unique_ptr_t value_type, typename context_type, jsonifier::parse_options options, auto jsonEntity, size_t depth, size_t maxDepth,
		bool minifiedOrInsideRepeated>
	struct accessor_val_parser_partial<value_type, context_type, options, jsonEntity, depth, maxDepth, minifiedOrInsideRepeated> {
		using base = derailleur<options, value_type, context_type>;

		JSONIFIER_FORCE_INLINE static void impl(value_type& value, context_type& context) noexcept {
			if JSONIFIER_LIKELY (context.iter < context.endIter && **context.iter != n) {
				using member_type = decltype(*value);
				if JSONIFIER_UNLIKELY (!value) {
					value = std::make_unique<std::remove_pointer_t<std::remove_cvref_t<member_type>>>();
				}
				parse<options, jsonEntity, depth, maxDepth, minifiedOrInsideRepeated>::impl(*value, context);
			}
			JSONIFIER_ELSE_UNLIKELY(else) {
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


	template<jsonifier::concepts::pointer_t value_type, typename context_type, jsonifier::parse_options options, auto jsonEntity, size_t depth, size_t maxDepth,
		bool minifiedOrInsideRepeated>
	struct accessor_val_parser_partial<value_type, context_type, options, jsonEntity, depth, maxDepth, minifiedOrInsideRepeated> {
		using base = derailleur<options, value_type, context_type>;

		JSONIFIER_FORCE_INLINE static void impl(value_type& value, context_type& context) noexcept {
			if JSONIFIER_LIKELY (context.iter < context.endIter && *context.iter != n) {
				if JSONIFIER_UNLIKELY (!value) {
					value = new std::remove_pointer_t<std::remove_cvref_t<value_type>>{};
				}
				parse<options, jsonEntity, depth, maxDepth, minifiedOrInsideRepeated>::impl(*value, context);
			}
			JSONIFIER_ELSE_UNLIKELY(else) {
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

	template<jsonifier::concepts::raw_json_t value_type, typename context_type, jsonifier::parse_options options, auto jsonEntity, size_t depth, size_t maxDepth,
		bool minifiedOrInsideRepeated>
	struct accessor_val_parser_partial<value_type, context_type, options, jsonEntity, depth, maxDepth, minifiedOrInsideRepeated> {
		using base = derailleur<options, value_type, context_type>;

		JSONIFIER_FORCE_INLINE static void impl(value_type& value, context_type& context) noexcept {
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
	};

	template<jsonifier::concepts::skip_t value_type, typename context_type, jsonifier::parse_options options, auto jsonEntity, size_t depth, size_t maxDepth,
		bool minifiedOrInsideRepeated>
	struct accessor_val_parser_partial<value_type, context_type, options, jsonEntity, depth, maxDepth, minifiedOrInsideRepeated> {
		using base = derailleur<options, value_type, context_type>;

		JSONIFIER_FORCE_INLINE static void impl(value_type& value, context_type& context) noexcept {
			base::skipToNextValue(context);
		}
	};

	template<jsonifier::concepts::skip_t value_type, typename context_type, jsonifier::parse_options options, auto jsonEntity, size_t depth, size_t maxDepth,
		bool minifiedOrInsideRepeated>
	struct custom_val_parser_partial<value_type, context_type, options, jsonEntity, depth, maxDepth, minifiedOrInsideRepeated> {
		using base = derailleur<options, value_type, context_type>;

		JSONIFIER_FORCE_INLINE static void impl(value_type& value, context_type& context) noexcept {
			base::skipToNextValue(context);
		}
	};
}