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
#include <jsonifier/Derailleur.hpp>
#include <jsonifier/Parser.hpp>
#include <jsonifier/ISADetection.hpp>
#include <memory>

namespace jsonifier_internal {

	template<typename derived_type, jsonifier::concepts::jsonifier_value_t value_type_new> struct parse_impl<derived_type, value_type_new> {
		template<const parse_options_internal<derived_type>& options, jsonifier::concepts::jsonifier_value_t value_type, typename iterator_type>
		JSONIFIER_INLINE static void impl(value_type&& value, iterator_type&& iter, iterator_type&& end) {
			if (*iter != '{') [[unlikely]] {
				static constexpr auto sourceLocation{ std::source_location::current() };
				options.parserPtr->getErrors().emplace_back(createError<sourceLocation, error_classes::Parsing>(iter - options.rootIter, static_cast<uint64_t>(end - iter),
					options.rootIter, parse_errors::Missing_Object_Start));
				skipToNextValue(iter, end);
				return;
			} else {
				++iter;
			}

			static constexpr auto memberCount = std::tuple_size_v<jsonifier::concepts::core_t<value_type>>;
			if constexpr (memberCount > 0) {
				static constexpr decltype(auto) frozenMap{ makeMap<value_type>() };

				bool first = true;
				while (iter != end) {
					if (*iter == '}') {
						++iter;
						return;
					} else if (first) {
						first = false;
					} else {
						if (*iter != ',') [[unlikely]] {
							static constexpr auto sourceLocation{ std::source_location::current() };
							options.parserPtr->getErrors().emplace_back(createError<sourceLocation, error_classes::Parsing>(iter - options.rootIter,
								static_cast<uint64_t>(end - iter), options.rootIter, parse_errors::Missing_Comma));
							skipToNextValue(iter, end);
							return;
						} else {
							++iter;
						}
					}

					const auto key = parseKey<options, value_type>(iter, end, options.parserPtr->getErrors());

					if constexpr (jsonifier::concepts::has_excluded_keys<value_type>) {
						auto& keys = value.jsonifierExcludedKeys;
						if (keys.find(static_cast<typename jsonifier::concepts::unwrap_t<decltype(keys)>::key_type>(key)) != keys.end()) {
							skipToNextValue(iter, end);
							continue;
						}
					}

					if (const auto& memberIt = frozenMap.find(key); memberIt != frozenMap.end()) [[likely]] {
						if (*iter != ':') [[unlikely]] {
							static constexpr auto sourceLocation{ std::source_location::current() };
							options.parserPtr->getErrors().emplace_back(createError<sourceLocation, error_classes::Parsing>(iter - options.rootIter,
								static_cast<uint64_t>(end - iter), options.rootIter, parse_errors::Missing_Colon));
							skipToNextValue(iter, end);
							return;
						} else {
							++iter;
						}

						std::visit(
							[&](auto&& memberPtr) {
								using member_type = jsonifier::concepts::unwrap_t<decltype(getMember(value, memberPtr))>;
								parse_impl<derived_type, member_type>::template impl<options>(getMember(value, memberPtr), iter, end);
							},
							memberIt->second);

					} else [[unlikely]] {
						skipToNextValue(iter, end);
					}
					
				}
			} else {
				skipToEndOfValue(iter, end);
			}
		}
	};

	template<typename derived_type, jsonifier::concepts::jsonifier_scalar_value_t value_type_new> struct parse_impl<derived_type, value_type_new> {
		template<const parse_options_internal<derived_type>& options, jsonifier::concepts::jsonifier_scalar_value_t value_type, typename iterator_type>
		JSONIFIER_INLINE static void impl(value_type&& value, iterator_type&& iter, iterator_type&& end) {
			static constexpr auto size{ std::tuple_size_v<jsonifier::concepts::core_t<value_type_new>> };
			if constexpr (size > 0) {
				parse_impl<derived_type, decltype(getMember(value, get<0>(jsonifier::concepts::core_v<value_type_new>)))>::template impl<options>(
					getMember(value, get<0>(jsonifier::concepts::core_v<value_type_new>)), iter, end);
			}
		}
	};

	template<typename derived_type, jsonifier::concepts::skip_value_t value_type_new> struct parse_impl<derived_type, value_type_new> {
		template<const parse_options_internal<derived_type>& options, jsonifier::concepts::skip_value_t value_type, typename iterator_type>
		JSONIFIER_INLINE static void impl(value_type&& value, iterator_type&& iter, iterator_type&& end) {
			skipToNextValue(iter, end);
		}
	};

	template<typename derived_type, jsonifier::concepts::array_tuple_t value_type_new> struct parse_impl<derived_type, value_type_new> {
		template<const parse_options_internal<derived_type>& options, jsonifier::concepts::array_tuple_t value_type, typename iterator_type>
		JSONIFIER_INLINE static void impl(value_type&& value, iterator_type&& iter, iterator_type&& end) {
			if (*iter == '[') [[likely]] {
				++iter;
			} else {
				static constexpr auto sourceLocation{ std::source_location::current() };
				options.parserPtr->getErrors().emplace_back(
					createError<sourceLocation, error_classes::Parsing>(iter - options.rootIter, end - options.rootIter, options.rootIter, parse_errors::Missing_Array_Start));
				skipToNextValue(iter, end);
				return;
			}
			static constexpr auto n = std::tuple_size_v<value_type_new>;
			parseObjects<options, n>(value, iter, end);
			if (*iter == ']') [[unlikely]] {
				++iter;
			} else {
				static constexpr auto sourceLocation{ std::source_location::current() };
				options.parserPtr->getErrors().emplace_back(
					createError<sourceLocation, error_classes::Parsing>(iter - options.rootIter, end - options.rootIter, options.rootIter, parse_errors::Missing_Array_End));
				skipToNextValue(iter, end);
				return;
			}
		}

		template<const parse_options_internal<derived_type>& options, uint64_t n, uint64_t indexNew = 0, bool isItFirst = true, jsonifier::concepts::array_tuple_t value_type, typename iterator_type>
		JSONIFIER_INLINE static void parseObjects(value_type&& value, iterator_type&& iter, iterator_type&& end) {
			auto& item = std::get<indexNew>(value);

			if constexpr (!isItFirst) {
				if (*iter == ',') [[likely]] {
					++iter;
				} else {
					static constexpr auto sourceLocation{ std::source_location::current() };
					options.parserPtr->getErrors().emplace_back(
						createError<sourceLocation, error_classes::Parsing>(iter - options.rootIter, end - options.rootIter, options.rootIter, parse_errors::Missing_Comma));
					return;
				}
			}

			parse_impl<derived_type, decltype(item)>::template impl<options>(item, iter, end);
			if constexpr (indexNew < n - 1) {
				parseObjects<n, indexNew + 1, false>(value, iter, end);
			}
		}
	};

	template<typename derived_type, jsonifier::concepts::map_t value_type_new> struct parse_impl<derived_type, value_type_new> {
		template<const parse_options_internal<derived_type>& options, jsonifier::concepts::map_t value_type, typename iterator_type>
		JSONIFIER_INLINE static void impl(value_type&& value, iterator_type&& iter, iterator_type&& end) {			
			if (*iter == '{') [[likely]] {
				++iter;
			} else {
				static constexpr auto sourceLocation{ std::source_location::current() };
				options.parserPtr->getErrors().emplace_back(
					createError<sourceLocation, error_classes::Parsing>(iter - options.rootIter, end - options.rootIter, options.rootIter, parse_errors::Missing_Object_Start));
				skipToNextValue(iter, end);
				return;
			}			

			bool first{ true };
			while (iter != end) {
				if (*iter == '}') [[unlikely]] {
					++iter;
					return;
				} else if (first) {
					first = false;
				} else {
					if (*iter == ',') [[likely]] {
						++iter;
					} else {
						static constexpr auto sourceLocation{ std::source_location::current() };
						options.parserPtr->getErrors().emplace_back(createError<sourceLocation, error_classes::Parsing>(iter - options.rootIter, end - options.rootIter,
							options.rootIter, parse_errors::Missing_Comma_Or_Object_End));
						skipToNextValue(iter, end);
						return;
					}
				}
				
				auto key = iter.parseKey();
				
				if (*iter == ':') [[likely]] {
					++iter;
				} else {
					static constexpr auto sourceLocation{ std::source_location::current() };
					options.parserPtr->getErrors().emplace_back(
						createError<sourceLocation, error_classes::Parsing>(iter - options.rootIter, end - options.rootIter, options.rootIter, parse_errors::Missing_Colon));
					skipToNextValue(iter, end);
					return;
				}
				parse_impl<derived_type, uint64_t>::template impl<options>(value[static_cast<typename value_type_new::key_type>(key)], iter, end);
				
			}
		}
	};

	template<typename derived_type, jsonifier::concepts::variant_t value_type_new> struct parse_impl<derived_type, value_type_new> {
		template<const parse_options_internal<derived_type>& options, jsonifier::concepts::variant_t value_type, typename iterator_type>
		JSONIFIER_INLINE static void impl(value_type&& value, iterator_type&& iter, iterator_type&& end) {
			std::visit(
				[&](auto& valueNew) {
					return parse_impl<derived_type, decltype(valueNew)>::template impl<options>(valueNew, iter, end);
				},
				value);
		}
	};

	template<typename derived_type, jsonifier::concepts::optional_t value_type_new> struct parse_impl<derived_type, value_type_new> {
		template<const parse_options_internal<derived_type>& options, jsonifier::concepts::optional_t value_type, typename iterator_type>
		JSONIFIER_INLINE static void impl(value_type&& value, iterator_type&& iter, iterator_type&& end) {
			if (parseNull(iter)) {
				return;
			}
			parse_impl<derived_type, decltype(*value)>::template impl<options>(value.emplace(), iter, end);
		}
	};

	template<typename derived_type, jsonifier::concepts::vector_t value_type_new> struct parse_impl<derived_type, value_type_new> {
		template<const parse_options_internal<derived_type>& options, jsonifier::concepts::vector_t value_type, typename iterator_type>
		JSONIFIER_INLINE static void impl(value_type&& value, iterator_type&& iter, iterator_type&& end) {			
			if (*iter == '[') [[likely]] {
				++iter;
			} else {
				static constexpr auto sourceLocation{ std::source_location::current() };
				options.parserPtr->getErrors().emplace_back(
					createError<sourceLocation, error_classes::Parsing>(iter - options.rootIter, end - options.rootIter, options.rootIter, parse_errors::Missing_Array_Start));
				skipToNextValue(iter, end);
				return;
			}
			
			if (*iter == ']') [[unlikely]] {
				++iter;
				return;
			}

			auto n		 = value.size();
			auto iterNew = value.begin();
			for (size_t i = 0; i < n; ++i) {
				
				parse_impl<derived_type, typename jsonifier::concepts::unwrap_t<value_type_new>::value_type>::template impl<options>(*(iterNew++), iter, end);
				
				if (*iter == ',') {
					++iter;
				} else if (*iter == ']') {
					++iter;
					return;
				} else [[unlikely]] {
					static constexpr auto sourceLocation{ std::source_location::current() };
					options.parserPtr->getErrors().emplace_back(
						createError<sourceLocation, error_classes::Parsing>(iter - options.rootIter, end - options.rootIter, options.rootIter, parse_errors::Missing_Array_End));
					skipToNextValue(iter, end);
					return;
				}
			}
			while (iter != end) {
				
				parse_impl<derived_type, typename jsonifier::concepts::unwrap_t<value_type_new>::value_type>::template impl<options>(value.emplace_back(), iter, end);
				
				if (*iter == ',') {
					++iter;
				} else if (*iter == ']') {
					++iter;
					return;
				} else [[unlikely]] {
					static constexpr auto sourceLocation{ std::source_location::current() };
					options.parserPtr->getErrors().emplace_back(
						createError<sourceLocation, error_classes::Parsing>(iter - options.rootIter, end - options.rootIter, options.rootIter, parse_errors::Missing_Array_End));
					skipToNextValue(iter, end);
					return;
				}
			}
		}
	};

	template<typename derived_type, jsonifier::concepts::raw_array_t value_type_new> struct parse_impl<derived_type, value_type_new> {
		template<const parse_options_internal<derived_type>& options, jsonifier::concepts::raw_array_t value_type, typename iterator_type>
		JSONIFIER_INLINE static void impl(value_type&& value, iterator_type&& iter, iterator_type&& end) {
			
			if (*iter == '[') [[likely]] {
				++iter;
			} else {
				static constexpr auto sourceLocation{ std::source_location::current() };
				options.parserPtr->getErrors().emplace_back(
					createError<sourceLocation, error_classes::Parsing>(iter - options.rootIter, end - options.rootIter, options.rootIter, parse_errors::Missing_Array_Start));
				skipToNextValue(iter, end);
				return;
			}
			
			if (*iter == ']') [[unlikely]] {
				++iter;
				return;
			}

			static constexpr auto n = value.size();
			auto iterNew			= value.begin();
			for (size_t i = 0; i < n; ++i) {
				
				parse_impl<derived_type, decltype(value[0])>::template impl<options>(*(iterNew++), iter, end);
				
				if (*iter == ',') [[likely]] {
					++iter;
				} else {
					if (*iter == ']') [[unlikely]] {
						++iter;
					} else {
						static constexpr auto sourceLocation{ std::source_location::current() };
						options.parserPtr->getErrors().emplace_back(createError<sourceLocation, error_classes::Parsing>(iter - options.rootIter, end - options.rootIter,
							options.rootIter, parse_errors::Missing_Array_End));
					}
					return;
				}
			}
		}
	};

	template<typename derived_type, jsonifier::concepts::string_t value_type_new> struct parse_impl<derived_type, value_type_new> {
		template<const parse_options_internal<derived_type>& options, jsonifier::concepts::string_t value_type, typename iterator_type>
		JSONIFIER_INLINE static void impl(value_type&& value, iterator_type&& iter, iterator_type&& end) {
			parseString<options>(value, iter, end, options.parserPtr->getErrors());
		}
	};

	template<typename derived_type, jsonifier::concepts::char_type value_type_new> struct parse_impl<derived_type, value_type_new> {
		template<const parse_options_internal<derived_type>& options, jsonifier::concepts::char_type value_type, typename iterator_type>
		JSONIFIER_INLINE static void impl(value_type&& value, iterator_type&& iter, iterator_type&& end) {
			value = static_cast<value_type_new>(*iter.operator->() + 1);
			++iter;
		}
	};

	template<typename derived_type, jsonifier::concepts::raw_json_t value_type_new> struct parse_impl<derived_type, value_type_new> {
		template<const parse_options_internal<derived_type>& options, jsonifier::concepts::raw_json_t value_type, typename iterator_type>
		JSONIFIER_INLINE static void impl(value_type&& value, iterator_type&& iter, iterator_type&& end) {
			auto newPtr = iter.operator->();
			iter.skipToNextValue();
			int64_t newSize = iter.operator->() - newPtr;
			if (newSize > 0) [[likely]] {
				jsonifier::string newString{};
				newString.resize(static_cast<uint64_t>(newSize));
				std::memcpy(newString.data(), newPtr, static_cast<uint64_t>(newSize));
				value = newString;
			}
			return;
		}
	};

	template<typename derived_type, jsonifier::concepts::unique_ptr_t value_type_new> struct parse_impl<derived_type, value_type_new> {
		template<const parse_options_internal<derived_type>& options, jsonifier::concepts::unique_ptr_t value_type, typename iterator_type>
		JSONIFIER_INLINE static void impl(value_type&& value, iterator_type&& iter, iterator_type&& end) {
			parse_impl<derived_type, decltype(*value)>::template impl<options>(*value, iter, end);
		}
	};

	template<typename derived_type, jsonifier::concepts::always_null_t value_type_new> struct parse_impl<derived_type, value_type_new> {
		template<const parse_options_internal<derived_type>& options, jsonifier::concepts::null_t value_type, typename iterator_type>
		JSONIFIER_INLINE static void impl(value_type&&, iterator_type&& iter, iterator_type&& end) {
			if (!parseNull(iter)) {
				static constexpr auto sourceLocation{ std::source_location::current() };
				options.parserPtr->getErrors().emplace_back(
					createError<sourceLocation, error_classes::Parsing>(iter - options.rootIter, end - options.rootIter, options.rootIter, parse_errors::Invalid_Bool_Value));
				return;
			}
		}
	};

	template<typename derived_type, jsonifier::concepts::enum_t value_type_new> struct parse_impl<derived_type, value_type_new> {
		template<const parse_options_internal<derived_type>& options, jsonifier::concepts::enum_t value_type, typename iterator_type>
		JSONIFIER_INLINE static void impl(value_type&& value, iterator_type&& iter, iterator_type&& end) {
			uint64_t newValue{};
			parse_impl<derived_type, uint64_t>::template impl<options>(newValue, iter, end);
			value = static_cast<value_type_new>(newValue);
		}
	};

	template<typename derived_type, jsonifier::concepts::num_t value_type_new> struct parse_impl<derived_type, value_type_new> {
		template<const parse_options_internal<derived_type>& options, jsonifier::concepts::num_t value_type, typename iterator_type>
		JSONIFIER_INLINE static void impl(value_type&& value, iterator_type&& iter, iterator_type&& end) {
			parseNumber<options>(value, iter, end, options.parserPtr->getErrors());
		}
	};

	template<typename derived_type, jsonifier::concepts::bool_t value_type_new> struct parse_impl<derived_type, value_type_new> {
		template<const parse_options_internal<derived_type>& options, jsonifier::concepts::bool_t value_type, typename iterator_type>
		JSONIFIER_INLINE static void impl(value_type&& value, iterator_type&& iter, iterator_type&& end) {
			if (!parseBool(value, iter)) {
				if (!options.optionsReal.minified) {
					++iter;
				}
				static constexpr auto sourceLocation{ std::source_location::current() };
				options.parserPtr->getErrors().emplace_back(
					createError<sourceLocation, error_classes::Parsing>(iter - options.rootIter, end - options.rootIter, options.rootIter, parse_errors::Invalid_Bool_Value));
				return;
			}
		}
	};

}