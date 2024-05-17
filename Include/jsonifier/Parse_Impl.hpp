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
		template<jsonifier::concepts::jsonifier_value_t value_type, jsonifier::concepts::is_fwd_iterator iterator_type, jsonifier::concepts::has_find... key_type>
		JSONIFIER_INLINE static void impl(value_type&& value, iterator_type&& iter, key_type&... excludedKeys) {
			if (*iter == 0x7Bu) [[likely]] {
				++iter;
			} else [[unlikely]] {
				iter.template createError<error_classes::Parsing>(parse_errors::Missing_Object_Start);
				skipToEndOfValue(iter);
				return;
			}
			if (*iter == 0x7Du) {
				++iter;
				return;
			}
			bool first{ true };
			while (iter) {
				if (*iter == 0x2Cu) [[likely]] {
					++iter;
				} else if (*iter == 0x7Du) {
					++iter;
					return;
				} else if (first) {
					first = false;
				} else {
					iter.template createError<error_classes::Parsing>(parse_errors::Missing_Comma_Or_Object_End);
					skipToEndOfValue(iter);
					return;
				}

				auto start = iter.operator->();
				if (*iter == 0x22u) [[likely]] {
					++iter;
				} else [[unlikely]] {
					iter.template createError<error_classes::Parsing>(parse_errors::Missing_String_Start);
					skipToEndOfValue(iter);
					continue;
				}

				const jsonifier::string_view_base<uint8_t> key{ start + 1, static_cast<uint64_t>(iter.operator->() - (start + 2)) };
				if constexpr ((( !std::is_void_v<key_type> ) || ...)) {
					if (((excludedKeys.find(static_cast<typename jsonifier::concepts::unwrap_t<key_type...>::key_type>(key)) != excludedKeys.end()) & ...)) [[unlikely]] {
						++iter;
						skipToNextValue(iter);
						continue;
					}
				}
				if (*iter == 0x3Au) [[likely]] {
					++iter;
				} else [[unlikely]] {
					iter.template createError<error_classes::Parsing>(parse_errors::Missing_Colon);
					skipToEndOfValue(iter);
					continue;
				}
				if (*iter == 0x6Eu) {
					skipToEndOfValue(iter);
					continue;
				}
				static constexpr auto frozenMap = makeMap<value_type_new>();
				const auto memberIt				= frozenMap.find(key);
				if (memberIt != frozenMap.end()) [[likely]] {
					using member_ptr_t = decltype(memberIt->second);
					if constexpr (std::variant_size_v<member_ptr_t> > 0) {
						std::visit(
							[&](auto&& memberPtr) {
								parser<derived_type>::impl(getMember(value, std::forward<decltype(memberPtr)>(memberPtr)), iter);
							},
							std::move(memberIt->second));
					}
				} else [[unlikely]] {
					skipToNextValue(iter);
				}
			}
		}
	};

	template<typename derived_type, jsonifier::concepts::jsonifier_scalar_value_t value_type_new> struct parse_impl<derived_type, value_type_new> {
		template<jsonifier::concepts::jsonifier_scalar_value_t value_type, jsonifier::concepts::is_fwd_iterator iterator_type>
		JSONIFIER_INLINE static void impl(value_type&& value, iterator_type&& iter) {
			static constexpr auto size{ std::tuple_size_v<jsonifier::concepts::core_wrapper_type<value_type_new>> };
			if constexpr (size > 0) {
				parser<derived_type>::impl(getMember(value, get<0>(jsonifier::concepts::core_wrapper_value<value_type_new>)), iter);
			}
		}
	};

	template<typename derived_type, jsonifier::concepts::array_tuple_t value_type_new> struct parse_impl<derived_type, value_type_new> {
		template<jsonifier::concepts::array_tuple_t value_type, jsonifier::concepts::is_fwd_iterator iterator_type>
		JSONIFIER_INLINE static void impl(value_type&& value, iterator_type&& iter) {
			if (*iter == 0x5Bu) [[likely]] {
				++iter;
			} else [[unlikely]] {
				iter.template createError<error_classes::Parsing>(parse_errors::Missing_Array_Start);
				skipToEndOfValue(iter);
				return;
			}
			static constexpr auto n = std::tuple_size_v<value_type_new>;
			parseObjects<n>(value, iter);
			if (*iter == 0x5Du) [[unlikely]] {
				++iter;
				return;
			}
		}

		template<uint64_t n, uint64_t indexNew = 0, bool isItFirst = true, jsonifier::concepts::array_tuple_t value_type, jsonifier::concepts::is_fwd_iterator iterator_type>
		JSONIFIER_INLINE static void parseObjects(value_type&& value, iterator_type&& iter) {
			auto& item = std::get<indexNew>(value);

			if (!isItFirst) {
				if (*iter == 0x2Cu) [[likely]] {
					++iter;
				}
			}

			parser<derived_type>::impl(item, iter);
			if constexpr (indexNew < n - 1) {
				parseObjects<n, indexNew + 1, false>(value, iter);
			}
		}
	};

	template<typename derived_type, jsonifier::concepts::map_t value_type_new> struct parse_impl<derived_type, value_type_new> {
		template<jsonifier::concepts::map_t value_type, jsonifier::concepts::is_fwd_iterator iterator_type>
		JSONIFIER_INLINE static void impl(value_type&& value, iterator_type&& iter) {
			if (*iter == 0x7Bu) [[likely]] {
				++iter;
			} else [[unlikely]] {
				iter.template createError<error_classes::Parsing>(parse_errors::Missing_Object_Start);
				skipToEndOfValue(iter);
				return;
			}
			bool first{ true };
			if (*iter == 0x7Du) {
				++iter;
				return;
			}
			while (iter) {
				if (*iter == 0x2Cu) [[likely]] {
					++iter;
				} else if (*iter == 0x7Du) {
					++iter;
					return;
				} else if (first) {
					first = false;
				} else [[unlikely]] {
					iter.template createError<error_classes::Parsing>(parse_errors::Missing_Comma_Or_Object_End);
					skipToEndOfValue(iter);
					return;
				}

				auto start = iter.operator->();
				if (*iter == 0x22u) [[likely]] {
					++iter;
				} else [[unlikely]] {
					iter.template createError<error_classes::Parsing>(parse_errors::Missing_String_Start);
					skipToEndOfValue(iter);
					continue;
				}

				const jsonifier::string_view_base<uint8_t> keyNew{ start + 1, static_cast<uint64_t>(iter.operator->() - (start + 2)) };
				if (*iter == 0x3Au) [[likely]] {
					++iter;
				} else [[unlikely]] {
					iter.template createError<error_classes::Parsing>(parse_errors::Missing_Colon);
					skipToEndOfValue(iter);
					continue;
				}
				if (*iter == 0x6Eu) {
					skipToEndOfValue(iter);
					continue;
				} 
				parser<derived_type>::impl(value[static_cast<typename value_type_new::key_type>(keyNew)], iter);
			}
		}
	};

	template<typename derived_type, jsonifier::concepts::variant_t value_type_new> struct parse_impl<derived_type, value_type_new> {
		template<jsonifier::concepts::variant_t value_type, jsonifier::concepts::is_fwd_iterator iterator_type>
		JSONIFIER_INLINE static void impl(value_type&& value, iterator_type&& iter) {
			std::visit(
				[&](auto&& valueNew) {
					parser<derived_type>::impl(valueNew, iter);
				},
				value);
		}
	};

	template<typename derived_type, jsonifier::concepts::optional_t value_type_new> struct parse_impl<derived_type, value_type_new> {
		template<jsonifier::concepts::optional_t value_type, jsonifier::concepts::is_fwd_iterator iterator_type>
		JSONIFIER_INLINE static void impl(value_type&& value, iterator_type&& iter) {
			parser<derived_type>::impl(value.emplace(), iter);
		}
	};

	template<typename derived_type, jsonifier::concepts::vector_t value_type_new> struct parse_impl<derived_type, value_type_new> {
		template<jsonifier::concepts::vector_t value_type, jsonifier::concepts::is_fwd_iterator iterator_type>
		JSONIFIER_INLINE static void impl(value_type&& value, iterator_type&& iter) {
			if (*iter == 0x5Bu) [[likely]] {
				++iter;
			} else [[unlikely]] {
				iter.template createError<error_classes::Parsing>(parse_errors::Missing_Array_Start);
				skipToEndOfValue(iter);
				return;
			}
			static thread_local jsonifier::vector<typename value_type_new::value_type, 1000> intermediateVector{};
			if (*iter == 0x5Du) [[unlikely]] {
				++iter;
				return;
			}

			const auto n = value.size();

			auto value_it = value.begin();

			for (size_t i = 0; i < n; ++i) {
				parser<derived_type>::impl(*(value_it++), iter);
				if (*iter == ',') [[likely]] {
					++iter;
				} else if (*iter == ']') {
					++iter;
					return;
				} else [[unlikely]] {
					iter.template createError<error_classes::Parsing>(parse_errors::Missing_Comma_Or_Array_End);
					skipToEndOfValue(iter);
					return;
				}
			}

			if constexpr (jsonifier::concepts::has_emplace_back<value_type_new>) {
				if constexpr (jsonifier::concepts::has_reserve<value_type_new> && jsonifier::concepts::has_capacity<value_type_new> &&
					requires { requires(sizeof(typename value_type_new::value_type) > 4096); }) {
					if (value.capacity() == 0) {
						value.reserve(1);
					}
					const auto capacity = value.capacity();
					for (size_t i = value.size(); i < capacity; ++i) {
						parser<derived_type>::impl(value.emplace_back(), iter);
						if (*iter == ',') [[likely]] {
							++iter;
						} else if (*iter == ']') {
							++iter;
							return;
						} else [[unlikely]] {
							iter.template createError<error_classes::Parsing>(parse_errors::Missing_Comma_Or_Array_End);
							skipToEndOfValue(iter);
							return;
						}
					}

					using value_type = typename value_type_new::value_type;

					std::vector<std::vector<value_type>> intermediate;
					intermediate.reserve(48);
					auto* active = &intermediate.emplace_back();
					active->reserve(2);
					while (iter) {
						if (active->size() == active->capacity()) {
							const auto former_capacity = active->capacity();
							active					   = &intermediate.emplace_back();
							active->reserve(2 * former_capacity);
						}
						parser<derived_type>::impl(active->emplace_back(), iter);
						if (*iter == ',') [[likely]] {
							++iter;
						} else if (*iter == ']') {
							++iter;
							break;
						} else [[unlikely]] {
							iter.template createError<error_classes::Parsing>(parse_errors::Missing_Comma_Or_Array_End);
							skipToEndOfValue(iter);
							return;
						}
					}

					const auto intermediate_size = intermediate.size();
					size_t reserve_size			 = value.size();
					for (size_t i = 0; i < intermediate_size; ++i) {
						reserve_size += intermediate[i].size();
					}

					if constexpr (std::is_trivially_copyable_v<value_type> && !std::same_as<value_type_new, std::vector<bool>>) {
						const auto original_size = value.size();
						value.resize(reserve_size);
						auto* dest = value.data() + original_size;
						for (const auto& vector: intermediate) {
							const auto vector_size = vector.size();
							std::memcpy(dest, vector.data(), vector_size * sizeof(value_type));
							dest += vector_size;
						}
					} else {
						value.reserve(reserve_size);
						for (const auto& vector: intermediate) {
							const auto inter_end = vector.end();
							for (auto inter = vector.begin(); inter < inter_end; ++inter) {
								value.emplace_back(std::move(*inter));
							}
						}
					}
				} else {
					while (iter) {
						parser<derived_type>::impl(value.emplace_back(), iter);
						if (*iter == ',') [[likely]] {
							++iter;
						} else if (*iter == ']') {
							++iter;
							return;
						} else [[unlikely]] {
							iter.template createError<error_classes::Parsing>(parse_errors::Missing_Comma_Or_Array_End);
							skipToEndOfValue(iter);
							return;
						}
					}
				}
			}
		}
	};

	template<typename derived_type, jsonifier::concepts::raw_array_t value_type_new> struct parse_impl<derived_type, value_type_new> {
		template<jsonifier::concepts::raw_array_t value_type, jsonifier::concepts::is_fwd_iterator iterator_type>
		JSONIFIER_INLINE static void impl(value_type&& value, iterator_type&& iter) {
			if (*iter == 0x5Bu) [[likely]] {
				++iter;
			} else [[unlikely]] {
				iter.template createError<error_classes::Parsing>(parse_errors::Missing_Array_Start);
				skipToEndOfValue(iter);
				return;
			}
			if (*iter == 0x5Du) [[unlikely]] {
				++iter;
				return;
			}
			static constexpr auto n = std::size(value);

			for (uint64_t x = 0; x < n; ++x) {
				parser<derived_type>::impl(value[x], iter);
				if (*iter == 0x2Cu) [[likely]] {
					++iter;
				} else if (*iter == 0x5Du) {
					++iter;
					return;
				} else [[unlikely]] {
					iter.template createError<error_classes::Parsing>(parse_errors::Missing_Comma_Or_Array_End);
					skipToEndOfValue(iter);
					return;
				}
			}
		}
	};

	template<typename derived_type, jsonifier::concepts::string_t value_type_new> struct parse_impl<derived_type, value_type_new> {
		template<jsonifier::concepts::string_t value_type, jsonifier::concepts::is_fwd_iterator iterator_type>
		JSONIFIER_INLINE static void impl(value_type&& value, iterator_type&& iter) {
			auto newPtr = iter.operator->();
			if (*iter == 0x22u) [[likely]] {
				++iter;
			} else {
				iter.template createError<error_classes::Parsing>(parse_errors::Missing_String_Start);
				skipToNextValue(iter);
				return;
			}
			static thread_local jsonifier::string_base<uint8_t, 1024 * 1024> newString{};
			auto newSize = static_cast<uint64_t>(iter.operator->() - newPtr);
			if (static_cast<int64_t>(newSize) > 0) {
				if (newSize > newString.size()) [[unlikely]] {
					newString.resize(newSize);
				}
				++newPtr;
				auto newestPtr = parseStringImpl(newPtr, newString.data(), newSize);
				if (newestPtr) [[likely]] {
					newSize = static_cast<uint64_t>(newestPtr - newString.data());
					if (value.size() != newSize) {
						value.resize(newSize);
						std::memcpy(value.data(), newString.data(), newSize);
					}
				} else {
					iter.template createError<error_classes::Parsing>(parse_errors::Invalid_String_Characters);
					skipToNextValue(iter);
					return;
				}
			}
		}
	};

	template<typename derived_type, jsonifier::concepts::char_type value_type_new> struct parse_impl<derived_type, value_type_new> {
		template<jsonifier::concepts::char_type value_type, jsonifier::concepts::is_fwd_iterator iterator_type>
		JSONIFIER_INLINE static void impl(value_type&& value, iterator_type&& iter) {
			value = static_cast<value_type_new>(*iter.operator->() + 1);
			++iter;
		}
	};

	template<typename derived_type, jsonifier::concepts::raw_json_t value_type_new> struct parse_impl<derived_type, value_type_new> {
		template<jsonifier::concepts::raw_json_t value_type, jsonifier::concepts::is_fwd_iterator iterator_type>
		JSONIFIER_INLINE static void impl(value_type&& value, iterator_type&& iter) {
			auto newPtr = iter.operator->();
			skipToNextValue(iter);
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
		template<jsonifier::concepts::unique_ptr_t value_type, jsonifier::concepts::is_fwd_iterator iterator_type>
		JSONIFIER_INLINE static void impl(value_type&& value, iterator_type&& iter) {
			parser<derived_type>::impl(*value, iter);
		}
	};

	template<typename derived_type, jsonifier::concepts::always_null_t value_type_new> struct parse_impl<derived_type, value_type_new> {
		template<jsonifier::concepts::null_t value_type, jsonifier::concepts::is_fwd_iterator iterator_type> JSONIFIER_INLINE static void impl(value_type&&, iterator_type&& iter) {
			if (parseNull(iter.operator->())) [[likely]] {
				++iter;
			} else {
				iter.template createError<error_classes::Parsing>(parse_errors::Invalid_Null_Value);
				skipToNextValue(iter);
			}
		}
	};

	template<typename derived_type, jsonifier::concepts::enum_t value_type_new> struct parse_impl<derived_type, value_type_new> {
		template<jsonifier::concepts::enum_t value_type, jsonifier::concepts::is_fwd_iterator iterator_type>
		JSONIFIER_INLINE static void impl(value_type&& value, iterator_type&& iter) {
			uint64_t newValue{};
			parser<derived_type>::impl(newValue, iter);
			value = static_cast<value_type_new>(newValue);
		}
	};

	template<typename derived_type, jsonifier::concepts::num_t value_type_new> struct parse_impl<derived_type, value_type_new> {
		template<jsonifier::concepts::num_t value_type, jsonifier::concepts::is_fwd_iterator iterator_type>
		JSONIFIER_INLINE static void impl(value_type&& value, iterator_type&& iter) {
			if constexpr (jsonifier::concepts::uint64_type<value_type_new> || jsonifier::concepts::int64_type<value_type_new> || jsonifier::concepts::float_t<value_type_new>) {
				if (parseNumber(value, iter.operator->())) [[likely]] {
					++iter;
				} else {
					iter.template createError<error_classes::Parsing>(parse_errors::Invalid_Number_Value);
					skipToNextValue(iter);
				}
			} else if constexpr (jsonifier::concepts::signed_type<value_type_new>) {
				int64_t newValue{};
				if (parseNumber(newValue, iter.operator->())) [[likely]] {
					++iter;
					value = static_cast<value_type_new>(newValue);
				} else {
					iter.template createError<error_classes::Parsing>(parse_errors::Invalid_Number_Value);
					skipToNextValue(iter);
				}
			} else if constexpr (jsonifier::concepts::unsigned_type<value_type_new>) {
				uint64_t newValue{};
				if (parseNumber(newValue, iter.operator->())) [[likely]] {
					++iter;
					value = static_cast<value_type_new>(newValue);
				} else {
					iter.template createError<error_classes::Parsing>(parse_errors::Invalid_Number_Value);
					skipToNextValue(iter);
				}
			}
		}
	};

	template<typename derived_type, jsonifier::concepts::bool_t value_type_new> struct parse_impl<derived_type, value_type_new> {
		template<jsonifier::concepts::bool_t value_type, jsonifier::concepts::is_fwd_iterator iterator_type>
		JSONIFIER_INLINE static void impl(value_type&& value, iterator_type&& iter) {
			if (parseBool(value, iter.operator->())) [[likely]] {
				++iter;
			} else {
				iter.template createError<error_classes::Parsing>(parse_errors::Invalid_Bool_Value);
				skipToNextValue(iter);
			}
		}
	};

}