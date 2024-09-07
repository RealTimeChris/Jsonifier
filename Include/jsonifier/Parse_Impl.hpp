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

	template<const jsonifier::parse_options& options, typename derived_type> struct index_processor {
		template<size_t index, typename value_type>
		JSONIFIER_ALWAYS_INLINE static bool processIndex(value_type&& value, parse_context<derived_type>& context) noexcept {
			if constexpr (index < std::tuple_size_v<core_tuple_t<value_type>>) {
				static constexpr auto ptr			= std::get<index>(coreTupleV<value_type>).ptr();
				static constexpr auto key			= std::get<index>(coreTupleV<value_type>).view();
				static constexpr auto stringLiteral = stringLiteralFromView<key.size()>(key);
				static constexpr auto keySize		= key.size();
				static constexpr auto keySizeNew	= keySize + 1;
				if (comparison<keySize, unwrap_t<decltype(*stringLiteral.data())>, unwrap_t<decltype(*context.currentIter)>>::compare(stringLiteral.data(), context.currentIter)) [[likely]] {
					context.currentIter += keySizeNew;
					JSONIFIER_SKIP_WS();
					if (*context.currentIter == ':') [[likely]] {
						++context.currentIter;
						JSONIFIER_SKIP_WS();
						using member_type = unwrap_t<decltype(value.*ptr)>;
						parse_impl<options, member_type, derived_type>::impl(value.*ptr, context);
						return true;
					} else {
						static constexpr auto sourceLocation{ std::source_location::current() };
						context.parserRef.getErrors().emplace_back(error::constructError<sourceLocation, error_classes::Parsing, parse_errors::Missing_Colon>(
							getUnderlyingPtr(context.currentIter) - context.rootIter, getUnderlyingPtr(context.endIter) - context.rootIter, context.rootIter));
						derailleur<options>::skipToNextValue(context.currentIter, context.endIter);
						return false;
					}
				}
			}
			return false;
		}
	};

	template<const jsonifier::parse_options& options, typename value_type, typename derived_type, size_t... indices>
	JSONIFIER_ALWAYS_INLINE constexpr auto generateFunctionPtrsImpl(std::index_sequence<indices...>) {
		using function_type = decltype(&index_processor<options, derived_type>::template processIndex<0, value_type>);
		return std::array<function_type, sizeof...(indices)>{ &index_processor<options, derived_type>::template processIndex<indices, value_type>... };
	}

	template<const jsonifier::parse_options& options, typename value_type, typename derived_type> JSONIFIER_ALWAYS_INLINE constexpr auto generateFunctionPtrs() {
		constexpr auto tupleSize = std::tuple_size_v<core_tuple_t<value_type>>;
		return generateFunctionPtrsImpl<options, value_type, derived_type>(std::make_index_sequence<tupleSize>{});
	}

	template<const jsonifier::parse_options& options, jsonifier::concepts::jsonifier_value_t value_type, typename derived_type> struct parse_impl<options, value_type, derived_type> {
		template<jsonifier::concepts::jsonifier_value_t value_type_new> 
		JSONIFIER_ALWAYS_INLINE static void impl(value_type_new&& value, parse_context<derived_type>& context) noexcept {
			static constexpr auto memberCount = std::tuple_size_v<core_tuple_t<value_type>>;
			if (*context.currentIter == '{') [[likely]] {
				++context.currentObjectDepth;
				++context.currentIter;
				if constexpr (memberCount > 0) {
					if constexpr (!options.minified) {
						const auto wsStart = context.currentIter;
						JSONIFIER_SKIP_WS();
						size_t wsSize{ size_t(context.currentIter - wsStart) };
						parseObjects(std::forward<value_type_new>(value), context, wsSize);
					} else {
						JSONIFIER_SKIP_WS();
						parseObjects(std::forward<value_type_new>(value), context);
					}
				} else {
					++context.currentIter;
					--context.currentObjectDepth;
					JSONIFIER_SKIP_WS();
				}
			} else {
				static constexpr auto sourceLocation{ std::source_location::current() };
				context.parserRef.getErrors().emplace_back(error::constructError<sourceLocation, error_classes::Parsing, parse_errors::Missing_Object_Start>(
					context.currentIter - context.rootIter, context.endIter - context.rootIter, context.rootIter));
				derailleur<options>::skipToNextValue(context.currentIter, context.endIter);
				return;
			}
		}

		template<bool first = true, jsonifier::concepts::jsonifier_value_t value_type_new>
#if defined(JSONIFIER_GNUCXX)
		JSONIFIER_INLINE static void parseObjects(value_type_new&& value, parse_context<derived_type>& context, size_t wsSize = 0) {

			if (*context.currentIter != '}') [[likely]] {
				if constexpr (!first) {
					if (*context.currentIter == ',') [[likely]] {
						++context.currentIter;
						if constexpr (!options.minified) {
							JSONIFIER_SKIP_WS_PRESET(wsSize);
						} else {
							JSONIFIER_SKIP_WS();
						}
					} else {
						static constexpr auto sourceLocation{ std::source_location::current() };
						context.parserRef.getErrors().emplace_back(error::constructError<sourceLocation, error_classes::Parsing, parse_errors::Missing_Comma_Or_Object_End>(
							context.currentIter - context.rootIter, context.endIter - context.rootIter, context.rootIter));
						derailleur<options>::skipToNextValue(context.currentIter, context.endIter);
						return;
					}
				}

#else
		JSONIFIER_ALWAYS_INLINE static void parseObjects(value_type_new&& value, parse_context<derived_type>& context, size_t wsSize = 0) {
			if (*context.currentIter != '}') [[likely]] {
				if constexpr (!first) {
					if (*context.currentIter == ',') [[likely]] {
						++context.currentIter;
						if constexpr (!options.minified) {
							JSONIFIER_SKIP_WS_PRESET(wsSize);
						} else {
							JSONIFIER_SKIP_WS();
						}
					} else {
						static constexpr auto sourceLocation{ std::source_location::current() };
						context.parserRef.getErrors().emplace_back(error::constructError<sourceLocation, error_classes::Parsing, parse_errors::Missing_Comma_Or_Object_End>(
							context.currentIter - context.rootIter, context.endIter - context.rootIter, context.rootIter));
						derailleur<options>::skipToNextValue(context.currentIter, context.endIter);
						return;
					}
				}
#endif
				if constexpr (jsonifier::concepts::has_excluded_keys<value_type>) {
					const auto keySize = getKeyLength<options, value_type>(context.currentIter, context.endIter);
					JSONIFIER_SKIP_WS();
					jsonifier::string_view key{ static_cast<const char*>(context.currentIter) + 1, keySize };
					auto& keys = value.jsonifierExcludedKeys;
					if (keys.find(static_cast<typename unwrap_t<decltype(keys)>::key_type>(key)) != keys.context.endIter()) [[likely]] {
						derailleur<options>::skipToNextValue(context.currentIter, context.endIter);
						return parseObjects<false>(std::forward<value_type_new>(value), context, wsSize);
					}
				}

				if (*context.currentIter == '"') [[likely]] {
					++context.currentIter;
					static constexpr auto N = std::tuple_size_v<core_tuple_t<value_type>>;
					const auto index		= hash_map<value_type, unwrap_t<decltype(context.currentIter)>>::findIndex(context.currentIter, context.endIter);
					if (index < N) [[likely]] {
						static constexpr auto arrayOfPtrs = generateFunctionPtrs<options, value_type, derived_type>();
						if (arrayOfPtrs[index](std::forward<value_type>(value), context)) [[likely]] {
							return parseObjects<false>(std::forward<value_type_new>(value), context, wsSize);
						} else {
							derailleur<options>::skipToNextValue(context.currentIter, context.endIter);
						}
						return;
					} else {
						derailleur<options>::skipKey(context.currentIter, context.endIter);
						++context.currentIter;
						derailleur<options>::skipToNextValue(context.currentIter, context.endIter);
						return parseObjects<false>(std::forward<value_type_new>(value), context, wsSize);
					}
				} else {
					static constexpr auto sourceLocation{ std::source_location::current() };
					context.parserRef.getErrors().emplace_back(error::constructError<sourceLocation, error_classes::Parsing, parse_errors::Missing_String_Start>(
						context.currentIter - context.rootIter, context.endIter - context.rootIter, context.rootIter));
					derailleur<options>::skipToNextValue(context.currentIter, context.endIter);
					return;
				}
			} else {
				++context.currentIter;
				--context.currentObjectDepth;
				JSONIFIER_SKIP_WS();
			}
		}
	};

	template<const jsonifier::parse_options& options, jsonifier::concepts::jsonifier_scalar_value_t value_type, typename derived_type> struct parse_impl<options, value_type, derived_type> {
		template<jsonifier::concepts::jsonifier_scalar_value_t value_type_new>
		JSONIFIER_ALWAYS_INLINE static void impl(value_type_new&& value, parse_context<derived_type>& context) noexcept {
			static constexpr auto size{ std::tuple_size_v<core_tuple_t<value_type>> };
			if constexpr (size == 1) {
				static constexpr auto newPtr = std::get<0>(coreTupleV<value_type>);
				using member_type			 = unwrap_t<decltype(getMember<newPtr>(value))>;
				parse_impl<options, member_type, derived_type>::impl(getMember<newPtr>(value), context);;
			}
		}
	};

	template<const jsonifier::parse_options& options, jsonifier::concepts::tuple_t value_type, typename derived_type> struct parse_impl<options, value_type, derived_type> {
		template<jsonifier::concepts::tuple_t value_type_new>
		JSONIFIER_ALWAYS_INLINE static void impl(value_type_new&& value, parse_context<derived_type>& context) noexcept {
			if (*context.currentIter == '[') [[likely]] {
				++context.currentArrayDepth;
				++context.currentIter;
				JSONIFIER_SKIP_WS();

				static constexpr auto n = std::tuple_size_v<unwrap_t<value_type>>;
				parseObjects<n, 0, true>(std::forward<value_type_new>(value), context);;
				if (*context.currentIter == ']') [[likely]] {
					--context.currentArrayDepth;
					++context.currentIter;
					JSONIFIER_SKIP_WS();
				} else {
					static constexpr auto sourceLocation{ std::source_location::current() };
					context.parserRef.getErrors().emplace_back(error::constructError<sourceLocation, error_classes::Parsing, parse_errors::Imbalanced_Array_Brackets>(
						context.currentIter - context.rootIter, context.endIter - context.rootIter, context.rootIter));
					derailleur<options>::skipToNextValue(context.currentIter, context.endIter);
					return;
				}
			} else {
				static constexpr auto sourceLocation{ std::source_location::current() };
				context.parserRef.getErrors().emplace_back(error::constructError<sourceLocation, error_classes::Parsing, parse_errors::Missing_Array_Start>(
					context.currentIter - context.rootIter, context.endIter - context.rootIter, context.rootIter));
				derailleur<options>::skipToNextValue(context.currentIter, context.endIter);
				return;
			}
		}

		template<size_t n, size_t indexNew = 0, bool isItFirst = true, jsonifier::concepts::tuple_t value_type_new>
		JSONIFIER_ALWAYS_INLINE static void parseObjects(value_type_new&& value, parse_context<derived_type>& context) noexcept {
			auto& item = std::get<indexNew>(value);

			if constexpr (!isItFirst) {
				if (*context.currentIter == ',') [[likely]] {
					++context.currentIter;
					JSONIFIER_SKIP_WS();
				} else {
					static constexpr auto sourceLocation{ std::source_location::current() };
					context.parserRef.getErrors().emplace_back(error::constructError<sourceLocation, error_classes::Parsing, parse_errors::Missing_Comma>(context.currentIter - context.rootIter,
						context.endIter - context.rootIter, context.rootIter));
					derailleur<options>::skipToNextValue(context.currentIter, context.endIter);
					return;
				}
			}

			parse_impl<options, decltype(item), derived_type>::impl(item, context);;
			if constexpr (indexNew < n - 1) {
				parseObjects<n, indexNew + 1, false>(std::forward<value_type_new>(value), context);
			}
		}
	};

	template<const jsonifier::parse_options& options, jsonifier::concepts::array_tuple_t value_type, typename derived_type> struct parse_impl<options, value_type, derived_type> {
		template<jsonifier::concepts::array_tuple_t value_type_new>
		JSONIFIER_ALWAYS_INLINE static void impl(value_type_new&& value, parse_context<derived_type>& context) noexcept {
			if (*context.currentIter == '[') [[likely]] {
				++context.currentArrayDepth;
				++context.currentIter;
				JSONIFIER_SKIP_WS();

				static constexpr auto n = std::tuple_size_v<unwrap_t<value_type>>;
				parseObjects<n, 0, true>(std::forward<value_type_new>(value), context);
				if (*context.currentIter == ']') [[likely]] {
					--context.currentArrayDepth;
					++context.currentIter;
					JSONIFIER_SKIP_WS();
				} else {
					static constexpr auto sourceLocation{ std::source_location::current() };
					context.parserRef.getErrors().emplace_back(error::constructError<sourceLocation, error_classes::Parsing, parse_errors::Imbalanced_Array_Brackets>(
						context.currentIter - context.rootIter, context.endIter - context.rootIter, context.rootIter));
					derailleur<options>::skipToNextValue(context.currentIter, context.endIter);
					return;
				}
			} else {
				static constexpr auto sourceLocation{ std::source_location::current() };
				context.parserRef.getErrors().emplace_back(error::constructError<sourceLocation, error_classes::Parsing, parse_errors::Missing_Array_Start>(
					context.currentIter - context.rootIter, context.endIter - context.rootIter, context.rootIter));
				derailleur<options>::skipToNextValue(context.currentIter, context.endIter);
				return;
			}
		}

		template<size_t n, size_t indexNew = 0, bool isItFirst = true, jsonifier::concepts::array_tuple_t value_type_new>
		JSONIFIER_ALWAYS_INLINE static void parseObjects(value_type_new&& value, parse_context<derived_type>& context) noexcept {
			auto& item = std::get<indexNew>(value);

			if constexpr (!isItFirst) {
				if (*context.currentIter == ',') [[likely]] {
					++context.currentIter;
					JSONIFIER_SKIP_WS();
				} else {
					static constexpr auto sourceLocation{ std::source_location::current() };
					context.parserRef.getErrors().emplace_back(error::constructError<sourceLocation, error_classes::Parsing, parse_errors::Missing_Comma>(context.currentIter - context.rootIter,
						context.endIter - context.rootIter, context.rootIter));
					derailleur<options>::skipToNextValue(context.currentIter, context.endIter);
					return;
				}
			}

			parse_impl<options, decltype(item), derived_type>::impl(item, context);
			if constexpr (indexNew < n - 1) {
				parseObjects<n, indexNew + 1, false>(std::forward<value_type_new>(value), context);
			}
		}
	};

	template<const jsonifier::parse_options& options, jsonifier::concepts::map_t value_type, typename derived_type> struct parse_impl<options, value_type, derived_type> {
		template<jsonifier::concepts::map_t value_type_new> JSONIFIER_ALWAYS_INLINE static void impl(value_type_new&& value, parse_context<derived_type>& context) noexcept {
			if (*context.currentIter == '{') [[likely]] {
				++context.currentObjectDepth;
				++context.currentIter;
				if constexpr (!options.minified) {
					const auto wsStart = context.currentIter;
					JSONIFIER_SKIP_WS();
					size_t wsSize{ size_t(context.currentIter - wsStart) };
					parseObjects(std::forward<value_type_new>(value), context, wsSize);
				} else {
					JSONIFIER_SKIP_WS();
					parseObjects(std::forward<value_type_new>(value), context);
				}
			} else {
				static constexpr auto sourceLocation{ std::source_location::current() };
				context.parserRef.getErrors().emplace_back(error::constructError<sourceLocation, error_classes::Parsing, parse_errors::Missing_Object_Start>(
					context.currentIter - context.rootIter, context.endIter - context.rootIter, context.rootIter));
				derailleur<options>::skipToNextValue(context.currentIter, context.endIter);
				return;
			}
		}

		template<bool first = true, jsonifier::concepts::map_t value_type_new>
#if defined(JSONIFIER_GNUCXX)
		JSONIFIER_INLINE static void parseObjects(value_type_new&& value, parse_context<derived_type>& context, size_t wsSize = 0) {

			if (*context.currentIter != '}') [[likely]] {
				if constexpr (!first) {
					if (*context.currentIter == ',') [[likely]] {
						++context.currentIter;
						if constexpr (!options.minified) {
							JSONIFIER_SKIP_WS_PRESET(wsSize);
						} else {
							JSONIFIER_SKIP_WS();
						}
					} else {
						static constexpr auto sourceLocation{ std::source_location::current() };
						context.parserRef.getErrors().emplace_back(error::constructError<sourceLocation, error_classes::Parsing, parse_errors::Missing_Comma_Or_Object_End>(
							context.currentIter - context.rootIter, context.endIter - context.rootIter, context.rootIter));
						derailleur<options>::skipToNextValue(context.currentIter, context.endIter);
						return;
					}
				}
#else
		JSONIFIER_ALWAYS_INLINE static void parseObjects(value_type_new&& value, parse_context<derived_type>& context, size_t wsSize = 0) {

			if (*context.currentIter != '}') [[likely]] {
				if constexpr (!first) {
					if (*context.currentIter == ',') [[likely]] {
						++context.currentIter;
						if constexpr (!options.minified) {
							JSONIFIER_SKIP_WS_PRESET(wsSize);
						} else {
							JSONIFIER_SKIP_WS();
						}
					} else {
						static constexpr auto sourceLocation{ std::source_location::current() };
						context.parserRef.getErrors().emplace_back(error::constructError<sourceLocation, error_classes::Parsing, parse_errors::Missing_Comma_Or_Object_End>(
							context.currentIter - context.rootIter, context.endIter - context.rootIter, context.rootIter));
						derailleur<options>::skipToNextValue(context.currentIter, context.endIter);
						return;
					}
				}
#endif
				static thread_local typename unwrap_t<value_type_new>::key_type key{};
				parse_impl<options, typename unwrap_t<value_type_new>::key_type, derived_type>::impl(key, context);

				if (*context.currentIter == ':') [[likely]] {
					++context.currentIter;
					JSONIFIER_SKIP_WS();
					using member_type = unwrap_t<value_type>::mapped_type;
					parse_impl<options, member_type, derived_type>::impl(value[key], context);
					parseObjects<false>(value, context, wsSize);
				} else {
					static constexpr auto sourceLocation{ std::source_location::current() };
					context.parserRef.getErrors().emplace_back(error::constructError<sourceLocation, error_classes::Parsing, parse_errors::Missing_String_Start>(
						context.currentIter - context.rootIter, context.endIter - context.rootIter, context.rootIter));
					derailleur<options>::skipToNextValue(context.currentIter, context.endIter);
					return;
				}
			} else {
				++context.currentIter;
				--context.currentObjectDepth;
				JSONIFIER_SKIP_WS();
			}
		}
	};

	template<const jsonifier::parse_options& options, jsonifier::concepts::variant_t value_type, typename derived_type> struct parse_impl<options, value_type, derived_type> {
		template<jsonifier::concepts::variant_t value_type_new>
		JSONIFIER_ALWAYS_INLINE static void impl(value_type_new&& value, parse_context<derived_type>& context) noexcept {
			static constexpr auto lambda = [&](auto& valueNew, auto& value, auto& buffer, auto& index) {
				using member_type = decltype(valueNew);
				return parse_impl<options, member_type, derived_type>::impl(std::forward<value_type_new>(value), context);
			};
			visit<lambda>(std::forward<value_type>(value), std::forward<value_type>(value), context);
		}
	};

	template<const jsonifier::parse_options& options, jsonifier::concepts::optional_t value_type, typename derived_type> struct parse_impl<options, value_type, derived_type> {
		template<jsonifier::concepts::optional_t value_type_new>
		JSONIFIER_ALWAYS_INLINE static void impl(value_type_new&& value, parse_context<derived_type>& context) noexcept {
			if (*context.currentIter != 'n') [[likely]] {
				parse_impl<options, decltype(*value), derived_type>::impl(value.emplace(), context);
			} else {
				context.currentIter += 4;
				JSONIFIER_SKIP_WS();
				return;
			}
		}
	};

	JSONIFIER_ALWAYS_INLINE void noop() noexcept {};

	template<const jsonifier::parse_options& options, jsonifier::concepts::vector_t value_type, typename derived_type> struct parse_impl<options, value_type, derived_type> {
		template<jsonifier::concepts::vector_t value_type_new>
		JSONIFIER_ALWAYS_INLINE static void impl(value_type_new&& value, parse_context<derived_type>& context) noexcept {
			for (size_t x = 0; x < sixtyFourBitsPerStep; ++x) {
				jsonifierPrefetchImpl(context.currentIter + (x * 64));
			}
			if (*context.currentIter == '[') [[likely]] {
				++context.currentArrayDepth;
				++context.currentIter;
				size_t wsSize{};
				if constexpr (!options.minified) {
					const auto wsStart = context.currentIter;
					JSONIFIER_SKIP_WS();
					wsSize = size_t(context.currentIter - wsStart);
				} else {
					JSONIFIER_SKIP_WS();
				}

				if (*context.currentIter != ']') [[likely]] {
					const auto n = value.size();
					auto iterNew = value.begin();
					for (size_t i = 0; i < n; ++i) {
						parse_impl<options, typename unwrap_t<value_type>::value_type, derived_type>::impl(*(iterNew++), context);

						if (*context.currentIter == ',') [[likely]] {
							++context.currentIter;
							JSONIFIER_SKIP_WS_PRESET(wsSize);
						} else {
							if (*context.currentIter == ']') [[likely]] {
								++context.currentIter;
								JSONIFIER_SKIP_WS();
								--context.currentArrayDepth;
								return value.size() == (i + 1) ? noop() : value.resize(i + 1);
							} else {
								static constexpr auto sourceLocation{ std::source_location::current() };
								context.parserRef.getErrors().emplace_back(error::constructError<sourceLocation, error_classes::Parsing, parse_errors::Imbalanced_Array_Brackets>(
									context.currentIter - context.rootIter, context.endIter - context.rootIter, context.rootIter));
								derailleur<options>::skipToNextValue(context.currentIter, context.endIter);
							}
							return;
						}
					}
					while (static_cast<const char*>(context.currentIter) != static_cast<const char*>(context.endIter)) {
						parse_impl<options, typename unwrap_t<value_type>::value_type, derived_type>::impl(value.emplace_back(), context);

						if (*context.currentIter == ',') [[likely]] {
							++context.currentIter;
							JSONIFIER_SKIP_WS_PRESET(wsSize);
						} else {
							if (*context.currentIter == ']') [[likely]] {
								++context.currentIter;
								JSONIFIER_SKIP_WS();
								--context.currentArrayDepth;
							} else {
								static constexpr auto sourceLocation{ std::source_location::current() };
								context.parserRef.getErrors().emplace_back(error::constructError<sourceLocation, error_classes::Parsing, parse_errors::Imbalanced_Array_Brackets>(
									context.currentIter - context.rootIter, context.endIter - context.rootIter, context.rootIter));
								derailleur<options>::skipToNextValue(context.currentIter, context.endIter);
							}
							return;
						}
					}
					++context.currentIter;
					JSONIFIER_SKIP_WS();
				} else {
					++context.currentIter;
					JSONIFIER_SKIP_WS();
					--context.currentArrayDepth;
					return;
				}
			} else {
				static constexpr auto sourceLocation{ std::source_location::current() };
				context.parserRef.getErrors().emplace_back(error::constructError<sourceLocation, error_classes::Parsing, parse_errors::Missing_Array_Start>(
					context.currentIter - context.rootIter, context.endIter - context.rootIter, context.rootIter));
				derailleur<options>::skipToNextValue(context.currentIter, context.endIter);
				return;
			}
		}
	};

	template<const jsonifier::parse_options& options, jsonifier::concepts::raw_array_t value_type, typename derived_type> struct parse_impl<options, value_type, derived_type> {
		template<jsonifier::concepts::raw_array_t value_type_new>
		JSONIFIER_ALWAYS_INLINE static void impl(value_type_new&& value, parse_context<derived_type>& context) noexcept {
			if (*context.currentIter == '[') [[likely]] {
				++context.currentArrayDepth;
				++context.currentIter;
				JSONIFIER_SKIP_WS();

				if (*context.currentIter != ']') [[likely]] {
					static constexpr auto n = value.size();
					auto iterNew			= value.begin();
					for (size_t i = 0; i < n; ++i) {
						parse_impl<options, decltype(value[0]), derived_type>::impl(*(iterNew++), context);
						if (*context.currentIter == ',') [[likely]] {
							++context.currentIter;
							JSONIFIER_SKIP_WS();
						} else {
							if (*context.currentIter == ']') [[likely]] {
								++context.currentIter;
								JSONIFIER_SKIP_WS();
								--context.currentArrayDepth;
							} else {
								static constexpr auto sourceLocation{ std::source_location::current() };
								context.parserRef.getErrors().emplace_back(error::constructError<sourceLocation, error_classes::Parsing, parse_errors::Imbalanced_Array_Brackets>(
									context.currentIter - context.rootIter, context.endIter - context.rootIter, context.rootIter));
								derailleur<options>::skipToNextValue(context.currentIter, context.endIter);
							}
							return;
						}
					}
					++context.currentIter;
					JSONIFIER_SKIP_WS();
				} else {
					++context.currentIter;
					JSONIFIER_SKIP_WS();
					--context.currentArrayDepth;
					return;
				}

			} else {
				static constexpr auto sourceLocation{ std::source_location::current() };
				context.parserRef.getErrors().emplace_back(error::constructError<sourceLocation, error_classes::Parsing, parse_errors::Missing_Array_Start>(
					context.currentIter - context.rootIter, context.endIter - context.rootIter, context.rootIter));
				derailleur<options>::skipToNextValue(context.currentIter, context.endIter);
				return;
			}
		}
	};

	template<const jsonifier::parse_options& options, jsonifier::concepts::string_t value_type, typename derived_type> struct parse_impl<options, value_type, derived_type> {
		template<jsonifier::concepts::string_t value_type_new> JSONIFIER_ALWAYS_INLINE static void impl(value_type_new&& value, parse_context<derived_type>& context) noexcept {
			if (derailleur<options>::parseString(std::forward<value_type_new>(value), context.currentIter, context.endIter)) [[likely]] {
				JSONIFIER_SKIP_WS();
				return;
			} else {
				static constexpr auto sourceLocation{ std::source_location::current() };
				context.parserRef.getErrors().emplace_back(error::constructError<sourceLocation, error_classes::Parsing, parse_errors::Invalid_String_Characters>(
					context.currentIter - context.rootIter, context.endIter - context.rootIter, context.rootIter));
				derailleur<options>::skipToNextValue(context.currentIter, context.endIter);
				return;
			}
		}
	};

	template<const jsonifier::parse_options& options, jsonifier::concepts::char_type value_type, typename derived_type> struct parse_impl<options, value_type, derived_type> {
		template<jsonifier::concepts::char_type value_type_new>
		JSONIFIER_ALWAYS_INLINE static void impl(value_type_new&& value, parse_context<derived_type>& context) noexcept {
			value = static_cast<value_type>(*(static_cast<const char*>(context.currentIter) + 1));
			++context.currentIter;
			JSONIFIER_SKIP_WS();
		}
	};

	template<const jsonifier::parse_options& options, jsonifier::concepts::raw_json_t value_type, typename derived_type> struct parse_impl<options, value_type, derived_type> {
		template<jsonifier::concepts::raw_json_t value_type_new>
		JSONIFIER_ALWAYS_INLINE static void impl(value_type_new&& value, parse_context<derived_type>& context) noexcept {
			auto newPtr = static_cast<const char*>(context.currentIter);
			derailleur<options>::skipToNextValue(context.currentIter, context.endIter);
			int64_t newSize = static_cast<const char*>(context.currentIter) - newPtr;
			if (newSize > 0) [[likely]] {
				jsonifier::string newString{};
				newString.resize(static_cast<size_t>(newSize));
				std::memcpy(newString.data(), newPtr, static_cast<size_t>(newSize));
				value = newString;
			}
			return;
		}
	};

	template<const jsonifier::parse_options& options, jsonifier::concepts::unique_ptr_t value_type, typename derived_type> struct parse_impl<options, value_type, derived_type> {
		template<jsonifier::concepts::unique_ptr_t value_type_new>
		JSONIFIER_ALWAYS_INLINE static void impl(value_type_new&& value, parse_context<derived_type>& context) noexcept {
			using member_type = decltype(*value);
			if (!value) {
				value = std::make_unique<std::remove_pointer_t<unwrap_t<member_type>>>();
			}
			parse_impl<options, decltype(*value), derived_type>::impl(*std::forward<value_type_new>(value), context);
		}
	};

	template<const jsonifier::parse_options& options, jsonifier::concepts::pointer_t value_type, typename derived_type> struct parse_impl<options, value_type, derived_type> {
		template<jsonifier::concepts::pointer_t value_type_new>
		JSONIFIER_ALWAYS_INLINE static void impl(value_type_new&& value, parse_context<derived_type>& context) noexcept {
			if (!value) {
				value = new std::remove_pointer_t<unwrap_t<value_type>>{};
			}
			parse_impl<options, decltype(*value), derived_type>::impl(*value, context);
		}
	};

	template<const jsonifier::parse_options& options, jsonifier::concepts::always_null_t value_type, typename derived_type> struct parse_impl<options, value_type, derived_type> {
		template<jsonifier::concepts::always_null_t value_type_new>
		JSONIFIER_ALWAYS_INLINE static void impl(value_type_new&&, parse_context<derived_type>& context) noexcept {
			if (parseNull(context.currentIter)) [[likely]] {
				JSONIFIER_SKIP_WS();
				return;
			} else {
				static constexpr auto sourceLocation{ std::source_location::current() };
				context.parserRef.getErrors().emplace_back(error::constructError<sourceLocation, error_classes::Parsing, parse_errors::Invalid_Null_Value>(context.currentIter - context.rootIter,
					context.endIter - context.rootIter, context.rootIter));
				derailleur<options>::skipToNextValue(context.currentIter, context.endIter);
				return;
			}
		}
	};

	template<const jsonifier::parse_options& options, jsonifier::concepts::enum_t value_type, typename derived_type> struct parse_impl<options, value_type, derived_type> {
		template<jsonifier::concepts::enum_t value_type_new>
		JSONIFIER_ALWAYS_INLINE static void impl(value_type_new&& value, parse_context<derived_type>& context) noexcept {
			size_t newValue{};
			if (parseNumber(newValue, context.currentIter, context.endIter)) [[likely]] {
				value = static_cast<value_type>(newValue);
				JSONIFIER_SKIP_WS();
				return;
			} else {
				static constexpr auto sourceLocation{ std::source_location::current() };
				context.parserRef.getErrors().emplace_back(error::constructError<sourceLocation, error_classes::Parsing, parse_errors::Invalid_Number_Value>(
					context.currentIter - context.rootIter, context.endIter - context.rootIter, context.rootIter));
				derailleur<options>::skipToNextValue(context.currentIter, context.endIter);
				return;
			}
		}
	};

	template<const jsonifier::parse_options& options, jsonifier::concepts::num_t value_type, typename derived_type> struct parse_impl<options, value_type, derived_type> {
		template<jsonifier::concepts::num_t value_type_new>
		JSONIFIER_ALWAYS_INLINE static void impl(value_type_new&& value, parse_context<derived_type>& context) noexcept {
			if (parseNumber(std::forward<value_type_new>(value), context.currentIter, context.endIter)) [[likely]] {
				JSONIFIER_SKIP_WS();
				return;
			} else {
				static constexpr auto sourceLocation{ std::source_location::current() };
				context.parserRef.getErrors().emplace_back(error::constructError<sourceLocation, error_classes::Parsing, parse_errors::Invalid_Number_Value>(
					context.currentIter - context.rootIter, context.endIter - context.rootIter, context.rootIter));
				derailleur<options>::skipToNextValue(context.currentIter, context.endIter);
				return;
			}
		}
	};

	template<const jsonifier::parse_options& options, jsonifier::concepts::bool_t value_type, typename derived_type> struct parse_impl<options, value_type, derived_type> {
		template<jsonifier::concepts::bool_t value_type_new>
		JSONIFIER_ALWAYS_INLINE static void impl(value_type_new&& value, parse_context<derived_type>& context) noexcept {
			if (parseBool(value, context.currentIter)) [[likely]] {
				JSONIFIER_SKIP_WS();
				return;
			} else {
				static constexpr auto sourceLocation{ std::source_location::current() };
				context.parserRef.getErrors().emplace_back(error::constructError<sourceLocation, error_classes::Parsing, parse_errors::Invalid_Bool_Value>(context.currentIter - context.rootIter,
					context.endIter - context.rootIter, context.rootIter));
				derailleur<options>::skipToNextValue(context.currentIter, context.endIter);
				return;
			}
		}
	};
}