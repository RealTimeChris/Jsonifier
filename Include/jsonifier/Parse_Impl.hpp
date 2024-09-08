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

	template<const auto& options> struct index_processor {
		template<size_t index, typename value_type, typename iterator>
		JSONIFIER_ALWAYS_INLINE static bool processIndex(value_type&& value, iterator&& iter, iterator&& end) noexcept {
			if constexpr (index < std::tuple_size_v<jsonifier::concepts::core_t<value_type>>) {
				static constexpr auto ptr			= std::get<index>(jsonifier::concepts::coreV<value_type>).ptr();
				static constexpr auto key			= std::get<index>(jsonifier::concepts::coreV<value_type>).view();
				static constexpr auto stringLiteral = stringLiteralFromView<key.size()>(key);
				static constexpr auto keySize		= key.size();
				static constexpr auto keySizeNew	= keySize + 1;
				if (comparison<keySize, unwrap_t<decltype(*stringLiteral.data())>, unwrap_t<decltype(*iter)>>::compare(stringLiteral.data(), iter)) [[likely]] {
					iter += keySizeNew;
					JSONIFIER_SKIP_WS();
					if (*iter == ':') [[likely]] {
						++iter;
						JSONIFIER_SKIP_WS();
						using member_type = unwrap_t<decltype(value.*ptr)>;
						parse_impl<options, member_type, iterator>::impl(value.*ptr, iter, end);
						return true;
					} else {
						static constexpr auto sourceLocation{ std::source_location::current() };
						options.parserPtr->getErrors().emplace_back(error::constructError<sourceLocation, error_classes::Parsing, parse_errors::Missing_Colon>(
							getUnderlyingPtr(iter) - options.rootIter, getUnderlyingPtr(end) - options.rootIter, options.rootIter));
						derailleur<options>::skipToNextValue(iter, end);
						return false;
					}
				}
			}
			return false;
		}
	};

	template<const auto& options, typename value_type, typename iterator, size_t... indices>
	JSONIFIER_ALWAYS_INLINE constexpr auto generateFunctionPtrsImpl(std::index_sequence<indices...>) {
		using function_type = decltype(&index_processor<options>::template processIndex<0, value_type, iterator>);
		return std::array<function_type, sizeof...(indices)>{ &index_processor<options>::template processIndex<indices, value_type, iterator>... };
	}

	template<const auto& options, typename value_type, typename iterator> JSONIFIER_ALWAYS_INLINE constexpr auto generateFunctionPtrs() {
		constexpr auto tupleSize = std::tuple_size_v<jsonifier::concepts::core_t<value_type>>;
		return generateFunctionPtrsImpl<options, value_type, iterator>(std::make_index_sequence<tupleSize>{});
	}

	template<const auto& options, jsonifier::concepts::jsonifier_value_t value_type, typename iterator> struct parse_impl<options, value_type, iterator> {
		template<jsonifier::concepts::jsonifier_value_t value_type_new, typename iterator_new>
		JSONIFIER_ALWAYS_INLINE static void impl(value_type_new&& value, iterator_new&& iter, iterator_new&& end) noexcept {
			static constexpr auto memberCount = std::tuple_size_v<jsonifier::concepts::core_t<value_type>>;
			if (*iter == '{') [[likely]] {
				++options.currentObjectDepth;
				++iter;
				if constexpr (memberCount > 0) {
					if constexpr (!options.optionsReal.minified) {
						const auto wsStart = iter;
						JSONIFIER_SKIP_WS();
						size_t wsSize{ size_t(iter - wsStart) };
						parseObjects(std::forward<value_type_new>(value), iter, end, wsSize);
					} else {
						JSONIFIER_SKIP_WS();
						parseObjects(std::forward<value_type_new>(value), iter, end);
					}
				} else {
					++iter;
					--options.currentObjectDepth;
					JSONIFIER_SKIP_WS();
				}
			} else {
				static constexpr auto sourceLocation{ std::source_location::current() };
				options.parserPtr->getErrors().emplace_back(error::constructError<sourceLocation, error_classes::Parsing, parse_errors::Missing_Object_Start>(
					iter - options.rootIter, end - options.rootIter, options.rootIter));
				derailleur<options>::skipToNextValue(iter, end);
				return;
			}
		}

		template<bool first = true, jsonifier::concepts::jsonifier_value_t value_type_new, typename iterator_new>
#if defined(JSONIFIER_GNUCXX)
		JSONIFIER_INLINE static void parseObjects(value_type_new&& value, iterator_new&& iter, iterator_new&& end, size_t wsSize = 0) {

			if (*iter != '}') [[likely]] {
				if constexpr (!first) {
					if (*iter == ',') [[likely]] {
						++iter;
						JSONIFIER_SKIP_WS_SIZED(wsSize);
					} else {
						static constexpr auto sourceLocation{ std::source_location::current() };
						options.parserPtr->getErrors().emplace_back(error::constructError<sourceLocation, error_classes::Parsing, parse_errors::Missing_Comma_Or_Object_End>(
							iter - options.rootIter, end - options.rootIter, options.rootIter));
						derailleur<options>::skipToNextValue(iter, end);
						return;
					}
				}

#else
		JSONIFIER_ALWAYS_INLINE static void parseObjects(value_type_new&& value, iterator_new&& iter, iterator_new&& end, size_t wsSize = 0) {
			if (*iter != '}') [[likely]] {
				if constexpr (!first) {
					if (*iter == ',') [[likely]] {
						++iter;
						JSONIFIER_SKIP_WS_SIZED(wsSize);
					} else {
						static constexpr auto sourceLocation{ std::source_location::current() };
						options.parserPtr->getErrors().emplace_back(error::constructError<sourceLocation, error_classes::Parsing, parse_errors::Missing_Comma_Or_Object_End>(
							iter - options.rootIter, end - options.rootIter, options.rootIter));
						derailleur<options>::skipToNextValue(iter, end);
						return;
					}
				}
#endif
				if constexpr (jsonifier::concepts::has_excluded_keys<value_type>) {
					const auto keySize = getKeyLength<options, value_type>(iter, end);
					JSONIFIER_SKIP_WS();
					jsonifier::string_view key{ static_cast<const char*>(iter) + 1, keySize };
					auto& keys = value.jsonifierExcludedKeys;
					if (keys.find(static_cast<typename unwrap_t<decltype(keys)>::key_type>(key)) != keys.end()) [[likely]] {
						derailleur<options>::skipToNextValue(iter, end);
						return parseObjects<false>(std::forward<value_type_new>(value), iter, end, wsSize);
					}
				}

				if (*iter == '"') [[likely]] {
					++iter;
					static constexpr auto N = std::tuple_size_v<jsonifier::concepts::core_t<value_type>>;
					const auto index		= hash_map<value_type, iterator>::findIndex(iter, end);
					if (index < N) [[likely]] {
						static constexpr auto arrayOfPtrs = generateFunctionPtrs<options, value_type, iterator>();
						if (arrayOfPtrs[index](std::forward<value_type>(value), std::forward<iterator>(iter), std::forward<iterator>(end))) [[likely]] {
							return parseObjects<false>(std::forward<value_type_new>(value), std::forward<iterator>(iter), std::forward<iterator>(end), wsSize);
						} else {
							derailleur<options>::skipToNextValue(iter, end);
						}
						return;
					} else {
						derailleur<options>::skipKey(iter, end);
						++iter;
						derailleur<options>::skipToNextValue(iter, end);
						return parseObjects<false>(std::forward<value_type_new>(value), std::forward<iterator>(iter), std::forward<iterator>(end), wsSize);
					}
				} else {
					static constexpr auto sourceLocation{ std::source_location::current() };
					options.parserPtr->getErrors().emplace_back(error::constructError<sourceLocation, error_classes::Parsing, parse_errors::Missing_String_Start>(
						iter - options.rootIter, end - options.rootIter, options.rootIter));
					derailleur<options>::skipToNextValue(iter, end);
					return;
				}
			} else {
				++iter;
				--options.currentObjectDepth;
				JSONIFIER_SKIP_WS();
			}
		}
	};

	template<const auto& options, jsonifier::concepts::jsonifier_scalar_value_t value_type, typename iterator> struct parse_impl<options, value_type, iterator> {
		template<jsonifier::concepts::jsonifier_scalar_value_t value_type_new, typename iterator_new>
		JSONIFIER_ALWAYS_INLINE static void impl(value_type_new&& value, iterator_new&& iter, iterator_new&& end) noexcept {
			static constexpr auto size{ std::tuple_size_v<jsonifier::concepts::core_t<value_type>> };
			if constexpr (size == 1) {
				static constexpr auto newPtr = std::get<0>(jsonifier::concepts::coreV<value_type>);
				using member_type			 = unwrap_t<decltype(getMember<newPtr>(value))>;
				parse_impl<options, member_type, iterator>::impl(getMember<newPtr>(value), iter, end);
			}
		}
	};

	template<const auto& options, jsonifier::concepts::tuple_t value_type, typename iterator> struct parse_impl<options, value_type, iterator> {
		template<jsonifier::concepts::tuple_t value_type_new, typename iterator_new>
		JSONIFIER_ALWAYS_INLINE static void impl(value_type_new&& value, iterator_new&& iter, iterator_new&& end) noexcept {
			if (*iter == '[') [[likely]] {
				++options.currentArrayDepth;
				++iter;
				JSONIFIER_SKIP_WS();

				static constexpr auto n = std::tuple_size_v<unwrap_t<value_type>>;
				parseObjects<n, 0, true>(std::forward<value_type_new>(value), iter, end);
				if (*iter == ']') [[likely]] {
					--options.currentArrayDepth;
					++iter;
					JSONIFIER_SKIP_WS();
				} else {
					static constexpr auto sourceLocation{ std::source_location::current() };
					options.parserPtr->getErrors().emplace_back(error::constructError<sourceLocation, error_classes::Parsing, parse_errors::Imbalanced_Array_Brackets>(
						iter - options.rootIter, end - options.rootIter, options.rootIter));
					derailleur<options>::skipToNextValue(iter, end);
					return;
				}
			} else {
				static constexpr auto sourceLocation{ std::source_location::current() };
				options.parserPtr->getErrors().emplace_back(error::constructError<sourceLocation, error_classes::Parsing, parse_errors::Missing_Array_Start>(
					iter - options.rootIter, end - options.rootIter, options.rootIter));
				derailleur<options>::skipToNextValue(iter, end);
				return;
			}
		}

		template<size_t n, size_t indexNew = 0, bool isItFirst = true, jsonifier::concepts::tuple_t value_type_new, typename iterator_new>
		JSONIFIER_ALWAYS_INLINE static void parseObjects(value_type_new&& value, iterator_new&& iter, iterator_new&& end) noexcept {
			auto& item = std::get<indexNew>(value);

			if constexpr (!isItFirst) {
				if (*iter == ',') [[likely]] {
					++iter;
					JSONIFIER_SKIP_WS();
				} else {
					static constexpr auto sourceLocation{ std::source_location::current() };
					options.parserPtr->getErrors().emplace_back(error::constructError<sourceLocation, error_classes::Parsing, parse_errors::Missing_Comma>(iter - options.rootIter,
						end - options.rootIter, options.rootIter));
					derailleur<options>::skipToNextValue(iter, end);
					return;
				}
			}

			parse_impl<options, decltype(item), iterator>::impl(item, iter, end);
			if constexpr (indexNew < n - 1) {
				parseObjects<n, indexNew + 1, false>(std::forward<value_type_new>(value), iter, end);
			}
		}
	};

	template<const auto& options, jsonifier::concepts::array_tuple_t value_type, typename iterator> struct parse_impl<options, value_type, iterator> {
		template<jsonifier::concepts::array_tuple_t value_type_new, typename iterator_new>
		JSONIFIER_ALWAYS_INLINE static void impl(value_type_new&& value, iterator_new&& iter, iterator_new&& end) noexcept {
			if (*iter == '[') [[likely]] {
				++options.currentArrayDepth;
				++iter;
				JSONIFIER_SKIP_WS();

				static constexpr auto n = std::tuple_size_v<unwrap_t<value_type>>;
				parseObjects<n, 0, true>(std::forward<value_type_new>(value), iter, end);
				if (*iter == ']') [[likely]] {
					--options.currentArrayDepth;
					++iter;
					JSONIFIER_SKIP_WS();
				} else {
					static constexpr auto sourceLocation{ std::source_location::current() };
					options.parserPtr->getErrors().emplace_back(error::constructError<sourceLocation, error_classes::Parsing, parse_errors::Imbalanced_Array_Brackets>(
						iter - options.rootIter, end - options.rootIter, options.rootIter));
					derailleur<options>::skipToNextValue(iter, end);
					return;
				}
			} else {
				static constexpr auto sourceLocation{ std::source_location::current() };
				options.parserPtr->getErrors().emplace_back(error::constructError<sourceLocation, error_classes::Parsing, parse_errors::Missing_Array_Start>(
					iter - options.rootIter, end - options.rootIter, options.rootIter));
				derailleur<options>::skipToNextValue(iter, end);
				return;
			}
		}

		template<size_t n, size_t indexNew = 0, bool isItFirst = true, jsonifier::concepts::array_tuple_t value_type_new, typename iterator_new>
		JSONIFIER_ALWAYS_INLINE static void parseObjects(value_type_new&& value, iterator_new&& iter, iterator_new&& end) noexcept {
			auto& item = std::get<indexNew>(value);

			if constexpr (!isItFirst) {
				if (*iter == ',') [[likely]] {
					++iter;
					JSONIFIER_SKIP_WS();
				} else {
					static constexpr auto sourceLocation{ std::source_location::current() };
					options.parserPtr->getErrors().emplace_back(error::constructError<sourceLocation, error_classes::Parsing, parse_errors::Missing_Comma>(iter - options.rootIter,
						end - options.rootIter, options.rootIter));
					derailleur<options>::skipToNextValue(iter, end);
					return;
				}
			}

			parse_impl<options, decltype(item), iterator>::impl(item, iter, end);
			if constexpr (indexNew < n - 1) {
				parseObjects<n, indexNew + 1, false>(std::forward<value_type_new>(value), iter, end);
			}
		}
	};

	template<const auto& options, jsonifier::concepts::map_t value_type, typename iterator> struct parse_impl<options, value_type, iterator> {
		template<jsonifier::concepts::map_t value_type_new, typename iterator_new>
		JSONIFIER_ALWAYS_INLINE static void impl(value_type_new&& value, iterator_new&& iter, iterator_new&& end) noexcept {
			if (*iter == '{') [[likely]] {
				++iter;
				++options.currentObjectDepth;
				size_t wsSize{};
				if constexpr (!options.optionsReal.minified) {
					const auto wsStart = iter;
					JSONIFIER_SKIP_WS();
					wsSize = size_t(iter - wsStart);
				}
				bool first{ true };
				while (*iter != '}') [[likely]] {
					if (!first) [[likely]] {
						if (*iter == ',') [[likely]] {
							++iter;
							JSONIFIER_SKIP_WS_SIZED(wsSize);
						} else {
							static constexpr auto sourceLocation{ std::source_location::current() };
							options.parserPtr->getErrors().emplace_back(error::constructError<sourceLocation, error_classes::Parsing, parse_errors::Missing_Comma_Or_Object_End>(
								iter - options.rootIter, end - options.rootIter, options.rootIter));
							derailleur<options>::skipToNextValue(iter, end);
							return;
						}
					} else {
						first = false;
					}
					static thread_local typename unwrap_t<value_type_new>::key_type key{};
					parse_impl<options, typename unwrap_t<value_type_new>::key_type, iterator>::impl(key, iter, end);

					if (*iter == ':') [[likely]] {
						++iter;
						JSONIFIER_SKIP_WS();
						using member_type = unwrap_t<value_type>::mapped_type;
						parse_impl<options, member_type, iterator>::impl(value[key], iter, end);
					} else {
						static constexpr auto sourceLocation{ std::source_location::current() };
						options.parserPtr->getErrors().emplace_back(error::constructError<sourceLocation, error_classes::Parsing, parse_errors::Missing_Colon>(
							iter - options.rootIter, end - options.rootIter, options.rootIter));
						derailleur<options>::skipToNextValue(iter, end);
						return;
					}
				}
				++iter;
				--options.currentObjectDepth;
				JSONIFIER_SKIP_WS();
			} else {
				static constexpr auto sourceLocation{ std::source_location::current() };
				options.parserPtr->getErrors().emplace_back(error::constructError<sourceLocation, error_classes::Parsing, parse_errors::Missing_Object_Start>(
					iter - options.rootIter, end - options.rootIter, options.rootIter));
				derailleur<options>::skipToNextValue(iter, end);
				return;
			}
		}
	};

	template<const auto& options, jsonifier::concepts::variant_t value_type, typename iterator> struct parse_impl<options, value_type, iterator> {
		template<jsonifier::concepts::variant_t value_type_new, typename iterator_new>
		JSONIFIER_ALWAYS_INLINE static void impl(value_type_new&& value, iterator_new&& iter, iterator_new&& end) noexcept {
			static constexpr auto lambda = [&](auto& valueNew, auto& value, auto& buffer, auto& index) {
				using member_type = decltype(valueNew);
				return parse_impl<options, member_type, iterator>::impl(std::forward<value_type_new>(value), iter, end);
			};
			visit<lambda>(std::forward<value_type>(value), std::forward<value_type>(value), iter, end);
		}
	};

	template<const auto& options, jsonifier::concepts::optional_t value_type, typename iterator> struct parse_impl<options, value_type, iterator> {
		template<jsonifier::concepts::optional_t value_type_new, typename iterator_new>
		JSONIFIER_ALWAYS_INLINE static void impl(value_type_new&& value, iterator_new&& iter, iterator_new&& end) noexcept {
			if (*iter != 'n') [[likely]] {
				parse_impl<options, decltype(*value), iterator>::impl(value.emplace(), iter, end);
			} else {
				iter += 4;
				JSONIFIER_SKIP_WS();
				return;
			}
		}
	};

	JSONIFIER_ALWAYS_INLINE void noop() noexcept {};

	template<const auto& options, jsonifier::concepts::vector_t value_type, typename iterator> struct parse_impl<options, value_type, iterator> {
		template<jsonifier::concepts::vector_t value_type_new, typename iterator_new>
		JSONIFIER_ALWAYS_INLINE static void impl(value_type_new&& value, iterator_new&& iter, iterator_new&& end) noexcept {
			if (*iter == '[') [[likely]] {
				++options.currentArrayDepth;
				++iter;
				size_t wsSize{};
				if constexpr (!options.optionsReal.minified) {
					const auto wsStart = iter;
					JSONIFIER_SKIP_WS();
					wsSize = size_t(iter - wsStart);
				}
				if (*iter != ']') [[likely]] {
					const auto n = value.size();
					auto iterNew = value.begin();
					for (size_t i = 0; i < n; ++i) {
						parse_impl<options, typename unwrap_t<value_type>::value_type, iterator>::impl(*(iterNew++), iter, end);
						if (*iter == ',') [[likely]] {
							++iter;
							JSONIFIER_SKIP_WS_SIZED(wsSize);
						} else {
							if (*iter == ']') [[likely]] {
								++iter;
								JSONIFIER_SKIP_WS();
								--options.currentArrayDepth;
								return value.size() == (i + 1) ? noop() : value.resize(i + 1);
							} else {
								static constexpr auto sourceLocation{ std::source_location::current() };
								options.parserPtr->getErrors().emplace_back(error::constructError<sourceLocation, error_classes::Parsing, parse_errors::Imbalanced_Array_Brackets>(
									iter - options.rootIter, end - options.rootIter, options.rootIter));
								derailleur<options>::skipToNextValue(iter, end);
							}
							return;
						}
					}
					while (static_cast<const char*>(iter) != static_cast<const char*>(end)) {
						parse_impl<options, typename unwrap_t<value_type>::value_type, iterator>::impl(value.emplace_back(), iter, end);

						if (*iter == ',') [[likely]] {
							++iter;
							JSONIFIER_SKIP_WS_SIZED(wsSize);
						} else {
							if (*iter == ']') [[likely]] {
								++iter;
								JSONIFIER_SKIP_WS();
								--options.currentArrayDepth;
								return;
							} else {
								static constexpr auto sourceLocation{ std::source_location::current() };
								options.parserPtr->getErrors().emplace_back(error::constructError<sourceLocation, error_classes::Parsing, parse_errors::Imbalanced_Array_Brackets>(
									iter - options.rootIter, end - options.rootIter, options.rootIter));
								derailleur<options>::skipToNextValue(iter, end);
							}
							return;
						}
					}
					++iter;
					JSONIFIER_SKIP_WS();
					--options.currentArrayDepth;
				} else {
					++iter;
					JSONIFIER_SKIP_WS();
					--options.currentArrayDepth;
					return;
				}

			} else {
				static constexpr auto sourceLocation{ std::source_location::current() };
				options.parserPtr->getErrors().emplace_back(error::constructError<sourceLocation, error_classes::Parsing, parse_errors::Missing_Array_Start>(
					iter - options.rootIter, end - options.rootIter, options.rootIter));
				derailleur<options>::skipToNextValue(iter, end);
				return;
			}
		}
	};

	template<const auto& options, jsonifier::concepts::raw_array_t value_type, typename iterator> struct parse_impl<options, value_type, iterator> {
		template<jsonifier::concepts::raw_array_t value_type_new, typename iterator_new>
		JSONIFIER_ALWAYS_INLINE static void impl(value_type_new&& value, iterator_new&& iter, iterator_new&& end) noexcept {
			if (*iter == '[') [[likely]] {
				++options.currentArrayDepth;
				++iter;
				size_t wsSize{};
				if constexpr (!options.optionsReal.minified) {
					const auto wsStart = iter;
					JSONIFIER_SKIP_WS();
					wsSize = size_t(iter - wsStart);
				}
				if (*iter != ']') [[likely]] {
					static constexpr auto n = value.size();
					auto iterNew			= value.begin();
					for (size_t i = 0; i < n; ++i) {
						parse_impl<options, decltype(value[0]), iterator>::impl(*(iterNew++), iter, end);
						if (*iter == ',') [[likely]] {
							++iter;
							JSONIFIER_SKIP_WS_SIZED(wsSize);
						} else {
							if (*iter == ']') [[likely]] {
								++iter;
								JSONIFIER_SKIP_WS();
								--options.currentArrayDepth;
							} else {
								static constexpr auto sourceLocation{ std::source_location::current() };
								options.parserPtr->getErrors().emplace_back(error::constructError<sourceLocation, error_classes::Parsing, parse_errors::Imbalanced_Array_Brackets>(
									iter - options.rootIter, end - options.rootIter, options.rootIter));
								derailleur<options>::skipToNextValue(iter, end);
							}
							return;
						}
					}
					++iter;
					JSONIFIER_SKIP_WS();
					--options.currentArrayDepth;
				} else {
					++iter;
					JSONIFIER_SKIP_WS();
					--options.currentArrayDepth;
					return;
				}

			} else {
				static constexpr auto sourceLocation{ std::source_location::current() };
				options.parserPtr->getErrors().emplace_back(error::constructError<sourceLocation, error_classes::Parsing, parse_errors::Missing_Array_Start>(
					iter - options.rootIter, end - options.rootIter, options.rootIter));
				derailleur<options>::skipToNextValue(iter, end);
				return;
			}
		}
	};

	template<const auto& options, jsonifier::concepts::string_t value_type, typename iterator> struct parse_impl<options, value_type, iterator> {
		template<jsonifier::concepts::string_t value_type_new, typename iterator_new>
		JSONIFIER_ALWAYS_INLINE static void impl(value_type_new&& value, iterator_new&& iter, iterator_new&& end) noexcept {
			derailleur<options>::parseString(std::forward<value_type_new>(value), iter, end);
			JSONIFIER_SKIP_WS();
		}
	};

	template<const auto& options, jsonifier::concepts::char_type value_type, typename iterator> struct parse_impl<options, value_type, iterator> {
		template<jsonifier::concepts::char_type value_type_new, typename iterator_new>
		JSONIFIER_ALWAYS_INLINE static void impl(value_type_new&& value, iterator_new&& iter, iterator_new&& end) noexcept {
			value = static_cast<value_type>(*(static_cast<const char*>(iter) + 1));
			++iter;
			JSONIFIER_SKIP_WS();
		}
	};

	template<const auto& options, jsonifier::concepts::raw_json_t value_type, typename iterator> struct parse_impl<options, value_type, iterator> {
		template<jsonifier::concepts::raw_json_t value_type_new, typename iterator_new>
		JSONIFIER_ALWAYS_INLINE static void impl(value_type_new&& value, iterator_new&& iter, iterator_new&& end) noexcept {
			auto newPtr = static_cast<const char*>(iter);
			derailleur<options>::skipToNextValue(iter, end);
			int64_t newSize = static_cast<const char*>(iter) - newPtr;
			if (newSize > 0) [[likely]] {
				jsonifier::string newString{};
				newString.resize(static_cast<size_t>(newSize));
				std::memcpy(newString.data(), newPtr, static_cast<size_t>(newSize));
				value = newString;
			}
			return;
		}
	};

	template<const auto& options, jsonifier::concepts::unique_ptr_t value_type, typename iterator> struct parse_impl<options, value_type, iterator> {
		template<jsonifier::concepts::unique_ptr_t value_type_new, typename iterator_new>
		JSONIFIER_ALWAYS_INLINE static void impl(value_type_new&& value, iterator_new&& iter, iterator_new&& end) noexcept {
			using member_type = decltype(*value);
			if (!value) {
				value = std::make_unique<std::remove_pointer_t<unwrap_t<member_type>>>();
			}
			parse_impl<options, decltype(*value), iterator>::impl(*std::forward<value_type_new>(value), iter, end);
		}
	};

	template<const auto& options, jsonifier::concepts::pointer_t value_type, typename iterator> struct parse_impl<options, value_type, iterator> {
		template<jsonifier::concepts::pointer_t value_type_new, typename iterator_new>
		JSONIFIER_ALWAYS_INLINE static void impl(value_type_new&& value, iterator_new&& iter, iterator_new&& end) noexcept {
			if (!value) {
				value = new std::remove_pointer_t<unwrap_t<value_type>>{};
			}
			parse_impl<options, decltype(*value), iterator>::impl(*value, iter, end);
		}
	};

	template<const auto& options, jsonifier::concepts::always_null_t value_type, typename iterator> struct parse_impl<options, value_type, iterator> {
		template<jsonifier::concepts::always_null_t value_type_new, typename iterator_new>
		JSONIFIER_ALWAYS_INLINE static void impl(value_type_new&&, iterator_new&& iter, iterator_new&& end) noexcept {
			if (parseNull(iter)) [[likely]] {
				JSONIFIER_SKIP_WS();
				return;
			} else {
				static constexpr auto sourceLocation{ std::source_location::current() };
				options.parserPtr->getErrors().emplace_back(error::constructError<sourceLocation, error_classes::Parsing, parse_errors::Invalid_Null_Value>(iter - options.rootIter,
					end - options.rootIter, options.rootIter));
				derailleur<options>::skipToNextValue(iter, end);
				return;
			}
		}
	};

	template<const auto& options, jsonifier::concepts::enum_t value_type, typename iterator> struct parse_impl<options, value_type, iterator> {
		template<jsonifier::concepts::enum_t value_type_new, typename iterator_new>
		JSONIFIER_ALWAYS_INLINE static void impl(value_type_new&& value, iterator_new&& iter, iterator_new&& end) noexcept {
			size_t newValue{};
			if (parseNumber(newValue, iter, end)) [[likely]] {
				value = static_cast<value_type>(newValue);
				JSONIFIER_SKIP_WS();
				return;
			} else {
				static constexpr auto sourceLocation{ std::source_location::current() };
				options.parserPtr->getErrors().emplace_back(error::constructError<sourceLocation, error_classes::Parsing, parse_errors::Invalid_Number_Value>(
					iter - options.rootIter, end - options.rootIter, options.rootIter));
				derailleur<options>::skipToNextValue(iter, end);
				return;
			}
		}
	};

	template<const auto& options, jsonifier::concepts::num_t value_type, typename iterator> struct parse_impl<options, value_type, iterator> {
		template<jsonifier::concepts::num_t value_type_new, typename iterator_new>
		JSONIFIER_ALWAYS_INLINE static void impl(value_type_new&& value, iterator_new&& iter, iterator_new&& end) noexcept {
			if (parseNumber(std::forward<value_type_new>(value), iter, end)) [[likely]] {
				JSONIFIER_SKIP_WS();
				return;
			} else {
				static constexpr auto sourceLocation{ std::source_location::current() };
				options.parserPtr->getErrors().emplace_back(error::constructError<sourceLocation, error_classes::Parsing, parse_errors::Invalid_Number_Value>(
					iter - options.rootIter, end - options.rootIter, options.rootIter));
				derailleur<options>::skipToNextValue(iter, end);
				return;
			}
		}
	};

	template<const auto& options, jsonifier::concepts::bool_t value_type, typename iterator> struct parse_impl<options, value_type, iterator> {
		template<jsonifier::concepts::bool_t value_type_new, typename iterator_new>
		JSONIFIER_ALWAYS_INLINE static void impl(value_type_new&& value, iterator_new&& iter, iterator_new&& end) noexcept {
			if (parseBool(value, iter)) [[likely]] {
				JSONIFIER_SKIP_WS();
				return;
			} else {
				static constexpr auto sourceLocation{ std::source_location::current() };
				options.parserPtr->getErrors().emplace_back(error::constructError<sourceLocation, error_classes::Parsing, parse_errors::Invalid_Bool_Value>(iter - options.rootIter,
					end - options.rootIter, options.rootIter));
				derailleur<options>::skipToNextValue(iter, end);
				return;
			}
		}
	};
}