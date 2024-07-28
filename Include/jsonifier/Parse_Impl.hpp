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

#include <memory>

namespace jsonifier_internal {

	template<const auto& options, const auto& tuple, size_t index, typename derived_type, typename value_type, typename iterator_type>
	JSONIFIER_INLINE void invokeParse(value_type& value, iterator_type& iter, iterator_type& end) {
		static constexpr auto& ptr = std::get<index>(tuple).ptr();
		using member_type		   = unwrap_t<decltype(value.*ptr)>;
		parse_impl<derived_type, member_type>::template impl<options>(value.*ptr, iter, end);
	}

	template<const auto& options, const auto& tuple, size_t index, typename derived_type, typename value_type, typename iterator_type> using invoke_parse_function_ptr =
		decltype(&invokeParse<options, tuple, index, derived_type, value_type, iterator_type>);

	template<const auto& options, const auto& tuple, typename derived_type, typename value_type, typename iterator_type, size_t... indices>
	constexpr auto generateArrayOfInvokeParsePtrsInternal(std::index_sequence<indices...>) {
		return std::array<invoke_parse_function_ptr<options, tuple, 0, derived_type, value_type, iterator_type>, sizeof...(indices)>{
			&invokeParse<options, tuple, indices, derived_type, value_type, iterator_type>...
		};
	}

	template<const auto& options, const auto& tuple, typename derived_type, typename value_type, typename iterator_type> constexpr auto generateArrayOfInvokeParsePtrs() {
		constexpr auto tupleSize = std::tuple_size_v<unwrap_t<decltype(tuple)>>;
		return generateArrayOfInvokeParsePtrsInternal<options, tuple, derived_type, value_type, iterator_type>(std::make_index_sequence<tupleSize>{});
	}

	template<const auto& options, const auto& tuple, typename derived_type, typename value_type, typename iterator_type, size_t... indices>
	constexpr auto generateTupleOfInvokeParsePtrArraysInternal(std::index_sequence<indices...>) {
		return std::make_tuple(generateArrayOfInvokeParsePtrs<options, std::get<indices>(tuple), derived_type, value_type, iterator_type>()...);
	}

	template<const auto& options, typename derived_type, typename value_type, typename iterator_type> constexpr auto generateTupleOfInvokeParsePtrArrays() {
		constexpr auto& tuple	 = final_tuple_data<value_type>::staticData;
		constexpr auto tupleSize = std::tuple_size_v<unwrap_t<decltype(tuple)>>;
		return generateTupleOfInvokeParsePtrArraysInternal<options, tuple, derived_type, value_type, iterator_type>(std::make_index_sequence<tupleSize>{});
	}

	template<typename derived_type, jsonifier::concepts::jsonifier_value_t value_type_new> struct parse_impl<derived_type, value_type_new> {
		template<const parse_options_internal<derived_type>& options, jsonifier::concepts::jsonifier_value_t value_type, typename iterator_type>
		JSONIFIER_INLINE static void impl(value_type&& value, iterator_type& iter, iterator_type& end) {
			if (*iter == '{') [[likely]] {
				++iter;
			} else {
				static constexpr auto sourceLocation{ std::source_location::current() };
				options.parserPtr->getErrors().emplace_back(error::constructError<sourceLocation, error_classes::Parsing, parse_errors::Missing_Array_Start>(
					iter - options.rootIter, end - options.rootIter, options.rootIter));
				skipToNextValue(iter, end);
				return;
			}
			bool isItFirst{ true };
			static constexpr auto memberCount = std::tuple_size_v<jsonifier::concepts::core_t<value_type>>;
			static constexpr auto frozenMap{ makeHashTuple<value_type_new>() };
			static constexpr auto functionPtrArray = generateTupleOfInvokeParsePtrArrays<options, derived_type, value_type, iterator_type>();
			if constexpr (memberCount > 0) {
				while (true) {
					if (*iter == '}') [[unlikely]] {
						++iter;
						return;
					}
					if (!isItFirst) {
						if (*iter == ',') [[likely]] {
							++iter;
						} else {
							static constexpr auto sourceLocation{ std::source_location::current() };
							options.parserPtr->getErrors().emplace_back(error::constructError<sourceLocation, error_classes::Parsing, parse_errors::Missing_Comma>(
								iter - options.rootIter, end - options.rootIter, options.rootIter));
							skipToNextValue(iter, end);
							return;
						}
					} else {
						isItFirst = false;
					}

					const auto keySize = getKeyLength<options, value_type>(iter, end, options.parserPtr->getErrors());

					if constexpr (jsonifier::concepts::has_excluded_keys<value_type>) {
						jsonifier::string_view key{ static_cast<const char*>(iter) + 1, keySize };
						auto& keys = value.jsonifierExcludedKeys;
						if (keys.find(static_cast<typename unwrap_t<decltype(keys)>::key_type>(key)) != keys.end()) {
							skipToNextValue(iter, end);
							return parseObjects<options, false>(value, iter, end);
						}
					}

					static constexpr auto functionLambda = [](const auto hashSubTupleIndex, auto& value, auto& iter, auto& end, const auto keySize) {
						static constexpr auto hashSubTuple			   = std::get<hashSubTupleIndex>(hash_tuple<value_type>::tuple);
						static constexpr auto subTupleFunctionPtrArray = std::get<hashSubTupleIndex>(functionPtrArray);
						auto memberIt								   = hashSubTuple.template find<subTupleFunctionPtrArray>(static_cast<const char*>(iter) + 1);
						if (memberIt != subTupleFunctionPtrArray.data() + subTupleFunctionPtrArray.size()) [[likely]] {
							if constexpr (jsonifier::concepts::json_structural_iterator_t<iterator_type>) {
								++iter;
							} else {
								iter += keySize + 2;
							}
							if (*iter == ':') [[likely]] {
								++iter;
							} else {
								static constexpr auto sourceLocation{ std::source_location::current() };
								options.parserPtr->getErrors().emplace_back(error::constructError<sourceLocation, error_classes::Parsing, parse_errors::Missing_Colon>(
									iter - options.rootIter, end - iter, options.rootIter));
								skipToNextValue(iter, end);
								return false;
							}
							(*memberIt)(value, iter, end);
						} else [[unlikely]] {
							skipToNextValue(iter, end);
							return false;
						}
						return true;
					};
					frozenMap.template find<functionLambda>(keySize, value, iter, end, keySize);
				}
			} else {
				skipToEndOfValue(iter, end);
			}
		}
	};

	template<typename derived_type, jsonifier::concepts::jsonifier_scalar_value_t value_type_new> struct parse_impl<derived_type, value_type_new> {
		template<const parse_options_internal<derived_type>& options, jsonifier::concepts::jsonifier_scalar_value_t value_type, typename iterator_type>
		JSONIFIER_INLINE static void impl(value_type&& value, iterator_type& iter, iterator_type& end) {
			static constexpr auto size{ std::tuple_size_v<jsonifier::concepts::core_t<value_type_new>> };
			if constexpr (size > 0) {
				static constexpr auto newPtr = std::get<0>(jsonifier::concepts::coreV<value_type_new>);
				auto& newMember				 = getMember<newPtr>(value);
				using member_type			 = unwrap_t<decltype(newMember)>;
				parse_impl<derived_type, member_type>::template impl<options>(newMember, iter, end);
			}
		}
	};

	template<typename derived_type, jsonifier::concepts::array_tuple_t value_type_new> struct parse_impl<derived_type, value_type_new> {
		template<const parse_options_internal<derived_type>& options, jsonifier::concepts::array_tuple_t value_type, typename iterator_type>
		JSONIFIER_INLINE static void impl(value_type&& value, iterator_type& iter, iterator_type& end) {
			if (*iter == '[') [[likely]] {
				++iter;
			} else {
				static constexpr auto sourceLocation{ std::source_location::current() };
				options.parserPtr->getErrors().emplace_back(error::constructError<sourceLocation, error_classes::Parsing, parse_errors::Missing_Array_Start>(
					iter - options.rootIter, end - options.rootIter, options.rootIter));
				skipToNextValue(iter, end);
				return;
			}
			static constexpr auto n = std::tuple_size_v<value_type_new>;
			parseObjects<options, n>(value, iter, end);
			if (*iter == ']') [[likely]] {
				++iter;
			} else {
				static constexpr auto sourceLocation{ std::source_location::current() };
				options.parserPtr->getErrors().emplace_back(error::constructError<sourceLocation, error_classes::Parsing, parse_errors::Missing_Array_End>(iter - options.rootIter,
					end - options.rootIter, options.rootIter));
				skipToNextValue(iter, end);
				return;
			}
		}

		template<const parse_options_internal<derived_type>& options, size_t n, size_t indexNew = 0, bool isItFirst = true, jsonifier::concepts::array_tuple_t value_type,
			typename iterator_type>
		JSONIFIER_INLINE static void parseObjects(value_type&& value, iterator_type& iter, iterator_type& end) {
			auto& item = std::get<indexNew>(value);

			if constexpr (!isItFirst) {
				if (*iter == ',') [[likely]] {
					++iter;
				} else {
					static constexpr auto sourceLocation{ std::source_location::current() };
					options.parserPtr->getErrors().emplace_back(error::constructError<sourceLocation, error_classes::Parsing, parse_errors::Missing_Comma>(iter - options.rootIter,
						end - options.rootIter, options.rootIter));
					return;
				}
			}

			parse_impl<derived_type, decltype(item)>::template impl<options>(item, iter, end);
			if constexpr (indexNew < n - 1) {
				parseObjects<options, n, indexNew + 1, false>(value, iter, end);
			}
		}
	};

	template<typename derived_type, jsonifier::concepts::map_t value_type_new> struct parse_impl<derived_type, value_type_new> {
		template<const parse_options_internal<derived_type>& options, jsonifier::concepts::map_t value_type, typename iterator_type>
		JSONIFIER_INLINE static void impl(value_type&& value, iterator_type& iter, iterator_type& end) {
			if (*iter == '{') [[likely]] {
				++iter;
			} else {
				static constexpr auto sourceLocation{ std::source_location::current() };
				options.parserPtr->getErrors().emplace_back(error::constructError<sourceLocation, error_classes::Parsing, parse_errors::Missing_Object_Start>(
					iter - options.rootIter, end - options.rootIter, options.rootIter));
				skipToNextValue(iter, end);
				return;
			}

			bool first{ true };
			while (static_cast<const char*>(iter) != static_cast<const char*>(end)) {
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
						options.parserPtr->getErrors().emplace_back(error::constructError<sourceLocation, error_classes::Parsing, parse_errors::Missing_Comma_Or_Object_End>(
							iter - options.rootIter, end - options.rootIter, options.rootIter));
						skipToNextValue(iter, end);
						return;
					}
				}

				static thread_local typename value_type_new::key_type key{};
				parse_impl<derived_type, typename value_type_new::key_type>::template impl<options>(key, iter, end);

				if (*iter == ':') [[likely]] {
					++iter;
				} else {
					static constexpr auto sourceLocation{ std::source_location::current() };
					options.parserPtr->getErrors().emplace_back(error::constructError<sourceLocation, error_classes::Parsing, parse_errors::Missing_Colon>(iter - options.rootIter,
						end - options.rootIter, options.rootIter));
					skipToNextValue(iter, end);
					return;
				}
				parse_impl<derived_type, typename value_type_new::mapped_type>::template impl<options>(value[key], iter, end);
			}
		}
	};

	template<typename derived_type, jsonifier::concepts::variant_t value_type_new> struct parse_impl<derived_type, value_type_new> {
		template<const parse_options_internal<derived_type>& options, jsonifier::concepts::variant_t value_type, typename iterator_type>
		JSONIFIER_INLINE static void impl(value_type&& value, iterator_type& iter, iterator_type& end) {
			visit(
				[&](auto& valueNew) {
					return parse_impl<derived_type, decltype(valueNew)>::template impl<options>(valueNew, iter, end);
				},
				value);
		}
	};

	template<typename derived_type, jsonifier::concepts::optional_t value_type_new> struct parse_impl<derived_type, value_type_new> {
		template<const parse_options_internal<derived_type>& options, jsonifier::concepts::optional_t value_type, typename iterator_type>
		JSONIFIER_INLINE static void impl(value_type&& value, iterator_type& iter, iterator_type& end) {
			if (parseNull(iter)) {
				return;
			}
			parse_impl<derived_type, decltype(*value)>::template impl<options>(value.emplace(), iter, end);
		}
	};

	template<typename derived_type, jsonifier::concepts::vector_t value_type_new> struct parse_impl<derived_type, value_type_new> {
		template<const parse_options_internal<derived_type>& options, jsonifier::concepts::vector_t value_type, typename iterator_type>
		JSONIFIER_INLINE static void impl(value_type&& value, iterator_type& iter, iterator_type& end) {
			if (*iter == '[') [[likely]] {
				++iter;
			} else {
				static constexpr auto sourceLocation{ std::source_location::current() };
				options.parserPtr->getErrors().emplace_back(error::constructError<sourceLocation, error_classes::Parsing, parse_errors::Missing_Array_Start>(
					iter - options.rootIter, end - options.rootIter, options.rootIter));
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
				parse_impl<derived_type, typename unwrap_t<value_type_new>::value_type>::template impl<options>(*(iterNew++), iter, end);

				if (*iter == ',') [[likely]] {
					++iter;
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
			while (static_cast<const char*>(iter) != static_cast<const char*>(end)) {
				parse_impl<derived_type, typename unwrap_t<value_type_new>::value_type>::template impl<options>(value.emplace_back(), iter, end);

				if (*iter == ',') [[likely]] {
					++iter;
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
		}
	};

	template<typename derived_type, jsonifier::concepts::raw_array_t value_type_new> struct parse_impl<derived_type, value_type_new> {
		template<const parse_options_internal<derived_type>& options, jsonifier::concepts::raw_array_t value_type, typename iterator_type>
		JSONIFIER_INLINE static void impl(value_type&& value, iterator_type& iter, iterator_type& end) {
			if (*iter == '[') [[likely]] {
				++iter;
			} else {
				static constexpr auto sourceLocation{ std::source_location::current() };
				options.parserPtr->getErrors().emplace_back(error::constructError<sourceLocation, error_classes::Parsing, parse_errors::Missing_Array_Start>(
					iter - options.rootIter, end - options.rootIter, options.rootIter));
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
		}
	};

	template<typename derived_type, jsonifier::concepts::string_t value_type_new> struct parse_impl<derived_type, value_type_new> {
		template<const parse_options_internal<derived_type>& options, jsonifier::concepts::string_t value_type, typename iterator_type>
		JSONIFIER_INLINE static void impl(value_type&& value, iterator_type& iter, iterator_type& end) {
			parseString<options>(value, iter, end, options.parserPtr->getErrors());
		}
	};

	template<typename derived_type, jsonifier::concepts::char_type value_type_new> struct parse_impl<derived_type, value_type_new> {
		template<const parse_options_internal<derived_type>& options, jsonifier::concepts::char_type value_type, typename iterator_type>
		JSONIFIER_INLINE static void impl(value_type&& value, iterator_type& iter, iterator_type& end) {
			value = static_cast<value_type_new>(*iter.operator->() + 1);
			++iter;
		}
	};

	template<typename derived_type, jsonifier::concepts::raw_json_t value_type_new> struct parse_impl<derived_type, value_type_new> {
		template<const parse_options_internal<derived_type>& options, jsonifier::concepts::raw_json_t value_type, typename iterator_type>
		JSONIFIER_INLINE static void impl(value_type&& value, iterator_type& iter, iterator_type& end) {
			auto newPtr = iter.operator->();
			iter.skipToNextValue();
			int64_t newSize = iter.operator->() - newPtr;
			if (newSize > 0) [[likely]] {
				jsonifier::string newString{};
				newString.resize(static_cast<size_t>(newSize));
				std::memcpy(newString.data(), newPtr, static_cast<size_t>(newSize));
				value = newString;
			}
			return;
		}
	};

	template<typename derived_type, jsonifier::concepts::unique_ptr_t value_type_new> struct parse_impl<derived_type, value_type_new> {
		template<const parse_options_internal<derived_type>& options, jsonifier::concepts::unique_ptr_t value_type, typename iterator_type>
		JSONIFIER_INLINE static void impl(value_type&& value, iterator_type& iter, iterator_type& end) {
			parse_impl<derived_type, decltype(*value)>::template impl<options>(*value, iter, end);
		}
	};

	template<typename derived_type, jsonifier::concepts::always_null_t value_type_new> struct parse_impl<derived_type, value_type_new> {
		template<const parse_options_internal<derived_type>& options, jsonifier::concepts::null_t value_type, typename iterator_type>
		JSONIFIER_INLINE static void impl(value_type&&, iterator_type& iter, iterator_type& end) {
			if (!parseNull(iter)) {
				static constexpr auto sourceLocation{ std::source_location::current() };
				options.parserPtr->getErrors().emplace_back(error::constructError<sourceLocation, error_classes::Parsing, parse_errors::Invalid_Null_Value>(iter - options.rootIter,
					end - options.rootIter, options.rootIter));
				return;
			}
		}
	};

	template<typename derived_type, jsonifier::concepts::enum_t value_type_new> struct parse_impl<derived_type, value_type_new> {
		template<const parse_options_internal<derived_type>& options, jsonifier::concepts::enum_t value_type, typename iterator_type>
		JSONIFIER_INLINE static void impl(value_type&& value, iterator_type& iter, iterator_type& end) {
			size_t newValue{};
			parse_impl<derived_type, size_t>::template impl<options>(newValue, iter, end);
			value = static_cast<value_type_new>(newValue);
		}
	};

	template<typename derived_type, jsonifier::concepts::num_t value_type_new> struct parse_impl<derived_type, value_type_new> {
		template<const parse_options_internal<derived_type>& options, jsonifier::concepts::num_t value_type, typename iterator_type>
		JSONIFIER_INLINE static void impl(value_type&& value, iterator_type& iter, iterator_type& end) {
			parseNumber<options>(value, iter, end, options.parserPtr->getErrors());
		}
	};

	template<typename derived_type, jsonifier::concepts::bool_t value_type_new> struct parse_impl<derived_type, value_type_new> {
		template<const parse_options_internal<derived_type>& options, jsonifier::concepts::bool_t value_type, typename iterator_type>
		JSONIFIER_INLINE static void impl(value_type&& value, iterator_type& iter, iterator_type& end) {
			if (!parseBool(value, iter)) {
				if (!options.optionsReal.minified) {
					++iter;
				}
				static constexpr auto sourceLocation{ std::source_location::current() };
				options.parserPtr->getErrors().emplace_back(error::constructError<sourceLocation, error_classes::Parsing, parse_errors::Invalid_Bool_Value>(iter - options.rootIter,
					end - options.rootIter, options.rootIter));
				return;
			}
		}
	};

}