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

	template<size_t index, const auto& options, typename value_type, typename iterator>
	JSONIFIER_ALWAYS_INLINE static bool processIndex(value_type&& value, iterator&& iter, iterator&& end) noexcept {
		if constexpr (index < std::tuple_size_v<unwrap_t<decltype(finalTupleStaticData<value_type>)>>) {
			static constexpr auto& ptr		 = std::get<index>(finalTupleStaticData<value_type>).ptr();
			static constexpr auto& key		 = std::get<index>(finalTupleStaticData<value_type>).view();
			static constexpr auto keySize	 = key.size();
			static constexpr auto keySizeNew = keySize + 2;
			if (compare<keySize>(key.data(), iter + 1)) [[likely]] {
				iter += keySizeNew;
				JSONIFIER_SKIP_WS();
				if (*iter == ':') [[likely]] {
					++iter;
					JSONIFIER_SKIP_WS();
					using member_type = unwrap_t<decltype(value.*ptr)>;
					parse_impl<options, member_type, iterator>::impl(value.*ptr, std::forward<iterator>(iter), std::forward<iterator>(end));
					JSONIFIER_SKIP_WS();
					return true;
				} else {
					static constexpr auto sourceLocation{ std::source_location::current() };
					options.parserPtr->getErrors().emplace_back(error::constructError<sourceLocation, error_classes::Parsing, parse_errors::Missing_Colon>(
						getUnderlyingPtr(iter) - options.rootIter, getUnderlyingPtr(end) - options.rootIter, options.rootIter));
					skipToNextValue<options>(iter, end);
					return false;
				}
			}
		}
		return false;
	}

	template<const auto& options, typename value_type, typename iterator, size_t... indices>
	JSONIFIER_ALWAYS_INLINE constexpr auto generateFunctionPtrsImpl(std::index_sequence<indices...>) {
		using function_type = decltype(&processIndex<0, options, value_type, iterator>);
		return std::array<function_type, sizeof...(indices)>{ &processIndex<indices, options, value_type, iterator>... };
	}

	template<const auto& options, typename value_type, typename iterator> JSONIFIER_ALWAYS_INLINE constexpr auto generateFunctionPtrs() {
		constexpr auto tupleSize = std::tuple_size_v<unwrap_t<decltype(finalTupleStaticData<value_type>)>>;
		return generateFunctionPtrsImpl<options, value_type, iterator>(std::make_index_sequence<tupleSize>{});
	}

	template<const auto& options, typename value_type, typename iterator>
	JSONIFIER_ALWAYS_INLINE bool processAndExecute(value_type&& value, iterator&& iter, iterator&& end) noexcept {
		static constexpr auto N = std::tuple_size_v<unwrap_t<decltype(finalTupleStaticData<value_type>)>>;
		const auto index		= findIndex<value_type, iterator>(std::forward<iterator>(iter), std::forward<iterator>(end));
		if (index < N) [[likely]] {
			static constexpr auto arrayOfPtrs = generateFunctionPtrs<options, value_type, iterator>();
			return arrayOfPtrs[index](std::forward<value_type>(value), std::forward<iterator>(iter), std::forward<iterator>(end));
		} else {
			return false;
		}
	}

	template<const auto& options, jsonifier::concepts::jsonifier_value_t value_type, typename iterator> struct parse_impl<options, value_type, iterator> {
		template<jsonifier::concepts::jsonifier_value_t value_type_new, typename iterator_new> 
		JSONIFIER_ALWAYS_INLINE static void impl(value_type_new&& value, iterator_new&& iter, iterator_new&& end) noexcept {
			JSONIFIER_SKIP_WS();
			if (*iter == '{') [[likely]] {
				++iter;

				static constexpr auto memberCount = std::tuple_size_v<jsonifier::concepts::core_t<value_type>>;
				if constexpr (memberCount > 0) {
					parseObjects(std::integral_constant<size_t, 0>{}, std::forward<value_type_new>(value), std::forward<iterator_new>(iter), std::forward<iterator_new>(end));
				} else {
					JSONIFIER_SKIP_WS();
					++iter;
				}
			} else {
				static constexpr auto sourceLocation{ std::source_location::current() };
				options.parserPtr->getErrors().emplace_back(error::constructError<sourceLocation, error_classes::Parsing, parse_errors::Missing_Array_Start>(
					iter - options.rootIter, end - options.rootIter, options.rootIter));
				skipToNextValue<options>(iter, end);
				return;
			}
		}

		template<jsonifier::concepts::jsonifier_value_t value_type_new, typename iterator_new> 
#if defined(JSONIFIER_GNUCXX)
		JSONIFIER_INLINE static void parseObjects(const auto first, value_type_new&& value, iterator_new&& iter, iterator_new&& end) {
#else
		JSONIFIER_ALWAYS_INLINE static void parseObjects(const auto first, value_type_new&& value, iterator_new&& iter, iterator_new&& end) {
#endif
			JSONIFIER_SKIP_WS();
			if (*iter != '}') {
				if constexpr (first != 0) {
					if (*iter == ',') [[likely]] {
						++iter;
					} else {
						static constexpr auto sourceLocation{ std::source_location::current() };
						options.parserPtr->getErrors().emplace_back(error::constructError<sourceLocation, error_classes::Parsing, parse_errors::Missing_Comma_Or_Object_End>(
							iter - options.rootIter, end - options.rootIter, options.rootIter));
						skipToNextValue<options>(iter, end);
						return;
					}
				}
				JSONIFIER_SKIP_WS();

				if constexpr (jsonifier::concepts::has_excluded_keys<value_type>) {
					const auto keySize = getKeyLength<options, value_type>(iter, end);
					JSONIFIER_SKIP_WS();
					jsonifier::string_view key{ static_cast<const char*>(iter) + 1, keySize };
					auto& keys = value.jsonifierExcludedKeys;
					if (keys.find(static_cast<typename unwrap_t<decltype(keys)>::key_type>(key)) != keys.end()) [[likely]] {
						skipToNextValue<options>(iter, end);
						return parseObjects(std::integral_constant<size_t, 1>{}, std::forward<value_type_new>(value), std::forward<iterator_new>(iter),
							std::forward<iterator_new>(end));
					}
				}

				if (!processAndExecute<options>(std::forward<value_type_new>(value), std::forward<iterator_new>(iter), std::forward<iterator_new>(end))) [[unlikely]] {
					skipToNextValue<options>(iter, end);
				}
				return parseObjects(std::integral_constant<size_t, 1>{}, std::forward<value_type_new>(value), std::forward<iterator_new>(iter), std::forward<iterator_new>(end));
			} else {
				++iter;
			}
		}
	};

	template<const auto& options, jsonifier::concepts::jsonifier_scalar_value_t value_type, typename iterator>
	struct parse_impl<options, value_type, iterator> {
		template<jsonifier::concepts::jsonifier_scalar_value_t value_type_new, typename iterator_new>
		JSONIFIER_ALWAYS_INLINE static void impl(value_type_new&& value, iterator_new&& iter, iterator_new&& end) noexcept {
			static constexpr auto size{ std::tuple_size_v<jsonifier::concepts::core_t<value_type>> };
			if constexpr (size > 0) {
				static constexpr auto newPtr = std::get<0>(jsonifier::concepts::coreV<value_type>);
				auto& newMember				 = getMember<newPtr>(value);
				using member_type			 = unwrap_t<decltype(newMember)>;
				parse_impl<options, member_type, iterator>::impl(newMember, std::forward<iterator_new>(iter), std::forward<iterator_new>(end));
			}
		}
	};

	template<const auto& options, jsonifier::concepts::array_tuple_t value_type, typename iterator>
	struct parse_impl<options, value_type, iterator> {
		template<jsonifier::concepts::array_tuple_t value_type_new, typename iterator_new>
		JSONIFIER_ALWAYS_INLINE static void impl(value_type_new&& value, iterator_new&& iter, iterator_new&& end) noexcept {
			JSONIFIER_SKIP_WS();
			if (*iter == '[') [[likely]] {
				++iter;
				JSONIFIER_SKIP_WS();

				static constexpr auto n = std::tuple_size_v<value_type>;
				parseObjects<options, n>(std::forward<value_type_new>(value), std::forward<iterator_new>(iter), std::forward<iterator_new>(end));
				if (*iter == ']') [[likely]] {
					++iter;
				} else {
					static constexpr auto sourceLocation{ std::source_location::current() };
					options.parserPtr->getErrors().emplace_back(error::constructError<sourceLocation, error_classes::Parsing, parse_errors::Missing_Array_End>(
						iter - options.rootIter, end - options.rootIter, options.rootIter));
					skipToNextValue<options>(iter, end);
					return;
				}
			} else {
				static constexpr auto sourceLocation{ std::source_location::current() };
				options.parserPtr->getErrors().emplace_back(error::constructError<sourceLocation, error_classes::Parsing, parse_errors::Missing_Array_Start>(
					iter - options.rootIter, end - options.rootIter, options.rootIter));
				skipToNextValue<options>(iter, end);
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
					return;
				}
			}

			parse_impl<options, decltype(item), iterator>::impl(item, std::forward<iterator_new>(iter), std::forward<iterator_new>(end));
			if constexpr (indexNew < n - 1) {
				parseObjects<options, n, indexNew + 1, false>(std::forward<value_type_new>(value), std::forward<iterator_new>(iter), std::forward<iterator_new>(end));
			}
		}
	};

	template<const auto& options, jsonifier::concepts::map_t value_type, typename iterator>
	struct parse_impl<options, value_type, iterator> {
		template<jsonifier::concepts::map_t value_type_new, typename iterator_new>
		JSONIFIER_ALWAYS_INLINE static void impl(value_type_new&& value, iterator_new&& iter, iterator_new&& end) noexcept {
			JSONIFIER_SKIP_WS();
			if (*iter == '{') [[likely]] {
				++iter;
			} else {
				static constexpr auto sourceLocation{ std::source_location::current() };
				options.parserPtr->getErrors().emplace_back(error::constructError<sourceLocation, error_classes::Parsing, parse_errors::Missing_Object_Start>(
					iter - options.rootIter, end - options.rootIter, options.rootIter));
				skipToNextValue<options>(iter, end);
				return;
			}
			bool first{ true };
			while (*iter != '}') {
				JSONIFIER_SKIP_WS();
				if (!first) [[likely]] {
					if (*iter == ',') [[likely]] {
						++iter;
					} else {
						static constexpr auto sourceLocation{ std::source_location::current() };
						options.parserPtr->getErrors().emplace_back(error::constructError<sourceLocation, error_classes::Parsing, parse_errors::Missing_Comma_Or_Object_End>(
							iter - options.rootIter, end - options.rootIter, options.rootIter));
						skipToNextValue<options>(iter, end);
						return;
					}
				} else {
					first = false;
				}
				JSONIFIER_SKIP_WS();

				static thread_local typename unwrap_t<value_type_new>::key_type key{};
				parse_impl<options, typename unwrap_t<value_type_new>::key_type, iterator>::impl(key, iter, end);
				JSONIFIER_SKIP_WS();

				if (*iter == ':') [[likely]] {
					++iter;
				} else {
					static constexpr auto sourceLocation{ std::source_location::current() };
					options.parserPtr->getErrors().emplace_back(error::constructError<sourceLocation, error_classes::Parsing, parse_errors::Missing_Colon>(iter - options.rootIter,
						end - options.rootIter, options.rootIter));
					skipToNextValue<options>(iter, end);
					return;
				}
				using member_type = unwrap_t<value_type>::mapped_type;
				JSONIFIER_SKIP_WS();
				parse_impl<options, member_type, iterator>::impl(value[key], iter, end);
				JSONIFIER_SKIP_WS();
			}
			++iter;
		}
	};

	template<const auto& options, jsonifier::concepts::variant_t value_type, typename iterator>
	struct parse_impl<options, value_type, iterator> {
		template<jsonifier::concepts::variant_t value_type_new, typename iterator_new>
		JSONIFIER_ALWAYS_INLINE static void impl(value_type_new&& value, iterator_new&& iter, iterator_new&& end) noexcept {
			static constexpr auto lambda = [&](auto& valueNew, auto& value, auto& buffer, auto& index) {
				using member_type = decltype(valueNew);
				return parse_impl<options, member_type, iterator>::impl(std::forward<value_type_new>(value), std::forward<iterator_new>(iter), std::forward<iterator_new>(end));
			};
			visit<lambda>(std::forward<value_type>(value), std::forward<value_type>(value), std::forward<iterator>(iter), std::forward<iterator>(end));
		}
	};

	template<const auto& options, jsonifier::concepts::optional_t value_type, typename iterator>
	struct parse_impl<options, value_type, iterator> {
		template<jsonifier::concepts::optional_t value_type_new, typename iterator_new>
		JSONIFIER_ALWAYS_INLINE static void impl(value_type_new&& value, iterator_new&& iter, iterator_new&& end) noexcept {
			JSONIFIER_SKIP_WS();
			if (*iter != 'n') [[likely]] {
				parse_impl<options, decltype(*value), iterator>::impl(value.emplace(), std::forward<iterator_new>(iter), std::forward<iterator_new>(end));
			} else {
				iter += 4;
				return;
			}
		}
	};

	JSONIFIER_ALWAYS_INLINE void noop() noexcept {};

	template<const auto& options, jsonifier::concepts::vector_t value_type, typename iterator>
	struct parse_impl<options, value_type, iterator> {
		template<jsonifier::concepts::vector_t value_type_new, typename iterator_new>
		JSONIFIER_ALWAYS_INLINE static void impl(value_type_new&& value, iterator_new&& iter, iterator_new&& end) noexcept {
			JSONIFIER_SKIP_WS();
			if (*iter == '[') [[likely]] {
				++iter;
				JSONIFIER_SKIP_WS();

				if (*iter != ']') [[likely]] {
					const auto n = value.size();
					auto iterNew = value.begin();
					for (size_t i = 0; i < n; ++i) {
						parse_impl<options, typename unwrap_t<value_type>::value_type, iterator>::impl(*(iterNew++), std::forward<iterator_new>(iter),
							std::forward<iterator_new>(end));
						JSONIFIER_SKIP_WS();

						if (*iter == ',') [[likely]] {
							++iter;
							JSONIFIER_SKIP_WS();
						} else {
							if (*iter == ']') [[likely]] {
								++iter;
								return value.size() == (i + 1) ? noop() : value.resize(i + 1);
							} else {
								static constexpr auto sourceLocation{ std::source_location::current() };
								options.parserPtr->getErrors().emplace_back(error::constructError<sourceLocation, error_classes::Parsing, parse_errors::Missing_Array_End>(
									iter - options.rootIter, end - options.rootIter, options.rootIter));
							}
							return;
						}
					}
					while (static_cast<const char*>(iter) != static_cast<const char*>(end)) {
						parse_impl<options, typename unwrap_t<value_type>::value_type, iterator>::impl(value.emplace_back(), std::forward<iterator_new>(iter),
							std::forward<iterator_new>(end));
						JSONIFIER_SKIP_WS();

						if (*iter == ',') [[likely]] {
							++iter;
							JSONIFIER_SKIP_WS();
						} else {
							if (*iter == ']') [[likely]] {
								++iter;
							} else {
								static constexpr auto sourceLocation{ std::source_location::current() };
								options.parserPtr->getErrors().emplace_back(error::constructError<sourceLocation, error_classes::Parsing, parse_errors::Missing_Array_End>(
									iter - options.rootIter, end - options.rootIter, options.rootIter));
							}
							return;
						}
					}
				} else {
					++iter;
					return;
				}
			} else {
				static constexpr auto sourceLocation{ std::source_location::current() };
				options.parserPtr->getErrors().emplace_back(error::constructError<sourceLocation, error_classes::Parsing, parse_errors::Missing_Array_Start>(
					iter - options.rootIter, end - options.rootIter, options.rootIter));
				skipToNextValue<options>(iter, end);
				return;
			}
		}
	};

	template<const auto& options, jsonifier::concepts::raw_array_t value_type, typename iterator>
	struct parse_impl<options, value_type, iterator> {
		template<jsonifier::concepts::raw_array_t value_type_new, typename iterator_new>
		JSONIFIER_ALWAYS_INLINE static void impl(value_type_new&& value, iterator_new&& iter, iterator_new&& end) noexcept {
			JSONIFIER_SKIP_WS();
			if (*iter == '[') [[likely]] {
				++iter;
				JSONIFIER_SKIP_WS();

				if (*iter != ']') [[likely]] {
					static constexpr auto n = value.size();
					auto iterNew			= value.begin();
					for (size_t i = 0; i < n; ++i) {
						parse_impl<options, decltype(value[0]), iterator>::impl(*(iterNew++), std::forward<iterator_new>(iter), std::forward<iterator_new>(end));

						JSONIFIER_SKIP_WS();
						if (*iter == ',') [[likely]] {
							++iter;
							JSONIFIER_SKIP_WS();
						} else {
							if (*iter == ']') [[likely]] {
								++iter;
							} else {
								static constexpr auto sourceLocation{ std::source_location::current() };
								options.parserPtr->getErrors().emplace_back(error::constructError<sourceLocation, error_classes::Parsing, parse_errors::Missing_Array_End>(
									iter - options.rootIter, end - options.rootIter, options.rootIter));
							}
							return;
						}
					}
				} else {
					++iter;
					JSONIFIER_SKIP_WS();
					return;
				}

			} else {
				static constexpr auto sourceLocation{ std::source_location::current() };
				options.parserPtr->getErrors().emplace_back(error::constructError<sourceLocation, error_classes::Parsing, parse_errors::Missing_Array_Start>(
					iter - options.rootIter, end - options.rootIter, options.rootIter));
				skipToNextValue<options>(iter, end);
				return;
			}
		}
	};

	template<const auto& options, jsonifier::concepts::string_t value_type, typename iterator>
	struct parse_impl<options, value_type, iterator> {
		template<jsonifier::concepts::string_t value_type_new, typename iterator_new>
		JSONIFIER_ALWAYS_INLINE static void impl(value_type_new&& value, iterator_new&& iter, iterator_new&& end) noexcept {
			JSONIFIER_SKIP_WS();
			parseString<options>(std::forward<value_type_new>(value), std::forward<iterator_new>(iter), std::forward<iterator_new>(end));
		}
	};

	template<const auto& options, jsonifier::concepts::char_type value_type, typename iterator>
	struct parse_impl<options, value_type, iterator> {
		template<jsonifier::concepts::char_type value_type_new, typename iterator_new>
		JSONIFIER_ALWAYS_INLINE static void impl(value_type_new&& value, iterator_new&& iter, iterator_new&& end) noexcept {
			JSONIFIER_SKIP_WS();
			value = static_cast<value_type>(*(static_cast<const char*>(iter) + 1));
			++iter;
		}
	};

	template<const auto& options, jsonifier::concepts::raw_json_t value_type, typename iterator>
	struct parse_impl<options, value_type, iterator> {
		template<jsonifier::concepts::raw_json_t value_type_new, typename iterator_new>
		JSONIFIER_ALWAYS_INLINE static void impl(value_type_new&& value, iterator_new&& iter, iterator_new&& end) noexcept {
			auto newPtr = static_cast<const char*>(iter);
			skipToNextValue<options>(iter, end);
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

	template<const auto& options, jsonifier::concepts::unique_ptr_t value_type, typename iterator>
	struct parse_impl<options, value_type, iterator> {
		template<jsonifier::concepts::unique_ptr_t value_type_new, typename iterator_new>
		JSONIFIER_ALWAYS_INLINE static void impl(value_type_new&& value, iterator_new&& iter, iterator_new&& end) noexcept {
			if (!value) {
				value = std::make_unique<unwrap_t<value_type>>();
			}
			parse_impl<options, decltype(*value), iterator>::impl(*std::forward<value_type_new>(value), std::forward<iterator_new>(iter), std::forward<iterator_new>(end));
		}
	};

	template<const auto& options, jsonifier::concepts::always_null_t value_type, typename iterator>
	struct parse_impl<options, value_type, iterator> {
		template<jsonifier::concepts::always_null_t value_type_new, typename iterator_new>
		JSONIFIER_ALWAYS_INLINE static void impl(value_type_new&&, iterator_new&& iter, iterator_new&& end) noexcept {
			JSONIFIER_SKIP_WS();
			if (parseNull(iter)) [[likely]] {
				return;
			} else {
				static constexpr auto sourceLocation{ std::source_location::current() };
				options.parserPtr->getErrors().emplace_back(error::constructError<sourceLocation, error_classes::Parsing, parse_errors::Invalid_Null_Value>(iter - options.rootIter,
					end - options.rootIter, options.rootIter));
				return;
			}
		}
	};

	template<const auto& options, jsonifier::concepts::enum_t value_type, typename iterator>
	struct parse_impl<options, value_type, iterator> {
		template<jsonifier::concepts::enum_t value_type_new, typename iterator_new>
		JSONIFIER_ALWAYS_INLINE static void impl(value_type_new&& value, iterator_new&& iter, iterator_new&& end) noexcept {
			size_t newValue{};
			JSONIFIER_SKIP_WS();
			if (parseNumber(newValue, std::forward<iterator_new>(iter), std::forward<iterator_new>(end))) [[likely]] {
				value = static_cast<value_type>(newValue);
				return;
			} else {
				static constexpr auto sourceLocation{ std::source_location::current() };
				options.parserPtr->getErrors().emplace_back(error::constructError<sourceLocation, error_classes::Parsing, parse_errors::Invalid_Number_Value>(
					iter - options.rootIter, end - options.rootIter, options.rootIter));
				return;
			}
		}
	};

	template<const auto& options, jsonifier::concepts::num_t value_type, typename iterator>
	struct parse_impl<options, value_type, iterator> {
		template<jsonifier::concepts::num_t value_type_new, typename iterator_new>
		JSONIFIER_ALWAYS_INLINE static void impl(value_type_new&& value, iterator_new&& iter, iterator_new&& end) noexcept {
			JSONIFIER_SKIP_WS();
			if (parseNumber(std::forward<value_type_new>(value), std::forward<iterator_new>(iter), std::forward<iterator_new>(end))) [[likely]] {
				return;
			} else {
				static constexpr auto sourceLocation{ std::source_location::current() };
				options.parserPtr->getErrors().emplace_back(error::constructError<sourceLocation, error_classes::Parsing, parse_errors::Invalid_Number_Value>(
					iter - options.rootIter, end - options.rootIter, options.rootIter));
				return;
			}
		}
	};

	template<const auto& options, jsonifier::concepts::bool_t value_type, typename iterator>
	struct parse_impl<options, value_type, iterator> {
		template<jsonifier::concepts::bool_t value_type_new, typename iterator_new>
		JSONIFIER_ALWAYS_INLINE static void impl(value_type_new&& value, iterator_new&& iter, iterator_new&& end) noexcept {
			JSONIFIER_SKIP_WS();
			if (parseBool(value, iter)) [[likely]] {
				return;
			} else {
				static constexpr auto sourceLocation{ std::source_location::current() };
				options.parserPtr->getErrors().emplace_back(error::constructError<sourceLocation, error_classes::Parsing, parse_errors::Invalid_Bool_Value>(iter - options.rootIter,
					end - options.rootIter, options.rootIter));
				return;
			}
		}
	};

}