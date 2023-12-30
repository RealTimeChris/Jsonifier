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

	template<jsonifier::concepts::jsonifier_value_t value_type_new, typename derived_type, typename parser_type>
	struct serial_parse_impl<value_type_new, derived_type, parser_type> {
		template<jsonifier::concepts::jsonifier_value_t value_type, jsonifier::concepts::is_fwd_iterator iterator_type, jsonifier::concepts::has_find... key_type>
		JSONIFIER_INLINE static void serial_impl(value_type&& value, iterator_type&& iter, iterator_type&& end, parser_type& parserVal, key_type&... excludedKeys) {
			skipWs(iter);
			if (*iter == 0x7Bu) [[likely]] {
				++iter;
				skipWs(iter);
			} else [[unlikely]] {
				parserVal.template createError<json_structural_type::Object_Start>(iter);
				skipToNextValue(iter, end);
				return;
			}
			bool first{ true };
			while (iter != end) {
				if (*iter == 0x2Cu) [[likely]] {
					++iter;
					skipWs(iter);
				} else if (*iter == 0x7Du) {
					++iter;
					skipWs(iter);
					return;
				} else if (first) {
					first = false;
				} else [[unlikely]] {
					parserVal.template createError<json_structural_type::Object_End>(iter);
					skipToNextValue(iter, end);
					return;
				}

				auto start = iter.operator->();
				if (*iter == 0x22u) [[likely]] {
					++iter;
					skipWs(iter);
					skipString(iter, end);
				} else [[unlikely]] {
					parserVal.template createError<error_code::UnQuoted_String>(iter);
					skipToNextValue(iter, end);
					continue;
				}

				const jsonifier::string_view_base<char> key{ reinterpret_cast<const char*>(start + 1), static_cast<uint64_t>(iter.operator->() - (start + 2)) };

				if constexpr ((( !std::is_void_v<key_type> ) || ...)) {
					if (((excludedKeys.find(static_cast<jsonifier::concepts::unwrap_t<key_type...>::key_type>(key)) != excludedKeys.end()) & ...)) [[unlikely]] {
						++iter;
						skipWs(iter);
						skipToNextValue(iter, end);
						continue;
					}
				}
				if (*iter == 0x3Au) [[likely]] {
					++iter;
					skipWs(iter);

				} else [[unlikely]] {
					parserVal.template createError<json_structural_type::Colon>(iter);
					skipToNextValue(iter, end);
					continue;
				}
				static constexpr auto frozenMap = makeMap<value_type_new>();
				const auto& memberIt			= frozenMap.find(key);
				if (*iter == 0x6Eu) [[unlikely]] {
					iter += 4;
					skipWs(iter);
					continue;
				} else if (memberIt != frozenMap.end()) [[likely]] {
					using member_ptr_t = decltype(memberIt->second);
					if constexpr (std::variant_size_v<member_ptr_t> > 0) {
						std::visit(
							[&](auto&& memberPtr) {
								parser<derived_type>::serial_impl(getMember(value, std::forward<decltype(memberPtr)>(memberPtr)), iter, end, parserVal);
							},
							std::move(memberIt->second));
					}
				} else [[unlikely]] {
					skipToNextValue(iter, end);
				}
				skipWs(iter);
			}
		}
	};

	template<jsonifier::concepts::jsonifier_scalar_value_t value_type_new, typename derived_type, typename parser_type>
	struct serial_parse_impl<value_type_new, derived_type, parser_type> {
		template<jsonifier::concepts::jsonifier_scalar_value_t value_type, jsonifier::concepts::is_fwd_iterator iterator_type>
		JSONIFIER_INLINE static void serial_impl(value_type&& value, iterator_type&& iter, iterator_type&& end, parser_type& parserVal) {
			static constexpr auto size{ std::tuple_size_v<jsonifier::concepts::core_t<value_type_new>> };
			if constexpr (size > 0) {
				parser<derived_type>::serial_impl(getMember(value, get<0>(jsonifier::concepts::coreV<value_type_new>)), iter, end, parserVal);
			}
		}
	};

	template<jsonifier::concepts::optional_t value_type_new, typename derived_type, typename parser_type> struct serial_parse_impl<value_type_new, derived_type, parser_type> {
		template<jsonifier::concepts::optional_t value_type, jsonifier::concepts::is_fwd_iterator iterator_type>
		JSONIFIER_INLINE static void serial_impl(value_type&& value, iterator_type&& iter, iterator_type&& end, parser_type& parserVal) {
			using member_type_new = typename value_type_new::value_type;
			member_type_new newValue{};
			parser<derived_type>::serial_impl(newValue, iter, end, parserVal);
			value = newValue;
		}
	};

	template<jsonifier::concepts::map_t value_type_new, typename derived_type, typename parser_type> struct serial_parse_impl<value_type_new, derived_type, parser_type> {
		template<jsonifier::concepts::map_t value_type, jsonifier::concepts::is_fwd_iterator iterator_type>
		JSONIFIER_INLINE static void serial_impl(value_type&& value, iterator_type&& iter, iterator_type&& end, parser_type& parserVal) {
			if (*iter == 0x7Bu) [[likely]] {
				++iter;
				skipWs(iter);

			} else [[unlikely]] {
				parserVal.template createError<json_structural_type::Object_Start>(iter);
				skipToNextValue(iter, end);
				return;
			}
			bool first{ true };
			while (iter != end) {
				if (*iter == 0x2Cu) [[likely]] {
					++iter;
					skipWs(iter);

				} else if (*iter == 0x7Du) {
					++iter;
					skipWs(iter);
					return;
				} else if (first) {
					first = false;
				} else [[unlikely]] {
					parserVal.template createError<json_structural_type::Array_End>(iter);
					skipToNextValue(iter, end);
					return;
				}

				if (*iter == 0x6Eu) [[unlikely]] {
					++iter;
					skipWs(iter);
					continue;
				}

				static thread_local typename value_type_new::key_type keyNew{};
				parser<derived_type>::serial_impl(keyNew, iter, end, parserVal);
				if (*iter == 0x3Au) [[likely]] {
					++iter;
					skipWs(iter);

				} else [[unlikely]] {
					parserVal.template createError<json_structural_type::Colon>(iter);
					skipToNextValue(iter, end);
					return;
				}
				parser<derived_type>::serial_impl(value[keyNew], iter, end, parserVal);
			}
		}
	};

	template<jsonifier::concepts::variant_t value_type_new, typename derived_type, typename parser_type> struct serial_parse_impl<value_type_new, derived_type, parser_type> {
		template<jsonifier::concepts::variant_t value_type, jsonifier::concepts::is_fwd_iterator iterator_type>
		JSONIFIER_INLINE static void serial_impl(value_type&& value, iterator_type&& iter, iterator_type&& end, parser_type& parserVal) {
			if (*iter == 'n') {
				iter += 4;
				return;
			}
			std::visit(
				[&](auto&& valueNew) {
					parser<derived_type>::serial_impl(valueNew, iter, end, parserVal);
				},
				value);
		}
	};

	template<jsonifier::concepts::vector_t value_type_new, typename derived_type, typename parser_type> struct serial_parse_impl<value_type_new, derived_type, parser_type> {
		template<jsonifier::concepts::vector_t value_type, jsonifier::concepts::is_fwd_iterator iterator_type>
		JSONIFIER_INLINE static void serial_impl(value_type&& value, iterator_type&& iter, iterator_type&& end, parser_type& parserVal) {
			if (*iter == 0x5Bu) [[likely]] {
				++iter;
				skipWs(iter);

			} else [[unlikely]] {
				parserVal.template createError<json_structural_type::Array_Start>(iter);
				skipToNextValue(iter, end);
				return;
			}
			if (*iter == 0x5Du) [[unlikely]] {
				++iter;
				skipWs(iter);
				return;
			}
			auto oldSize = value.size();
			auto newPtr	 = value.data();
			auto endPtr	 = newPtr + oldSize;
			for (; newPtr < endPtr; ++newPtr) {
				parser<derived_type>::serial_impl(*newPtr, iter, end, parserVal);

				if (*iter == 0x2Cu) [[likely]] {
					++iter;
					skipWs(iter);
				} else if (*iter == 0x5Du) {
					++iter;
					skipWs(iter);
					return;
				} else [[unlikely]] {
					parserVal.template createError<json_structural_type::Array_End>(iter);
					skipToNextValue(iter, end);
					return;
				}
			}
			if constexpr (jsonifier::concepts::has_resize<value_type_new>) {
				auto newSize = oldSize + countValueElements<'[', ']'>(iter, end);
				value.resize(newSize);
				newPtr = value.data() + oldSize;
				endPtr = value.data() + newSize;
				for (; newPtr < endPtr; ++newPtr) {
					parser<derived_type>::serial_impl(*newPtr, iter, end, parserVal);
					skipWs(iter);
					if (*iter == 0x2Cu) [[likely]] {
						++iter;
						skipWs(iter);
					} else if (*iter == 0x5Du) {
						++iter;
						skipWs(iter);
						return;
					} else [[unlikely]] {
						parserVal.template createError<json_structural_type::Array_End>(iter);
						skipToNextValue(iter, end);
						return;
					}
				}
			}
		}
	};

	template<jsonifier::concepts::raw_array_t value_type_new, typename derived_type, typename parser_type> struct serial_parse_impl<value_type_new, derived_type, parser_type> {
		template<jsonifier::concepts::raw_array_t value_type, jsonifier::concepts::is_fwd_iterator iterator_type>
		JSONIFIER_INLINE static void serial_impl(value_type&& value, iterator_type&& iter, iterator_type&& end, parser_type& parserVal) {
			if (*iter == 0x5Bu) [[likely]] {
				++iter;
				skipWs(iter);
			} else [[unlikely]] {
				parserVal.template createError<json_structural_type::Array_Start>(iter);
				skipToNextValue(iter, end);
				return;
			}
			if (*iter == 0x5Du) [[unlikely]] {
				++iter;
				skipWs(iter);
				return;
			}
			static constexpr auto n = std::size(value);

			for (uint64_t x = 0; x < n; ++x) {
				parser<derived_type>::serial_impl(value[x], iter, end, parserVal);
				if (*iter == 0x2Cu) [[likely]] {
					++iter;
					skipWs(iter);
				} else if (*iter == 0x5Du) {
					++iter;
					skipWs(iter);
					return;
				} else [[unlikely]] {
					parserVal.template createError<json_structural_type::Array_End>(iter);
					skipToNextValue(iter, end);
					return;
				}
			}
		}
	};

	template<jsonifier::concepts::string_t value_type_new, typename derived_type, typename parser_type> struct serial_parse_impl<value_type_new, derived_type, parser_type> {
		template<jsonifier::concepts::string_t value_type, jsonifier::concepts::is_fwd_iterator iterator_type>
		JSONIFIER_INLINE static void serial_impl(value_type&& value, iterator_type&& iter, iterator_type&& end, parser_type& parserVal) {
			if (*iter == 0x22u) [[unlikely]] {
				++iter;
				skipWs(iter);
			} else {
				parserVal.template createError<json_structural_type::String>(iter);
				skipToNextValue(iter, end);
				return;
			}
			auto newSize = static_cast<uint64_t>(end.operator->() - iter.operator->());
			if (newSize > parserVal.getCurrentString().size()) [[unlikely]] {
				parserVal.getCurrentString().resize(static_cast<uint64_t>(newSize));
			}
			auto newPtr	   = iter.operator->();
			auto newerPtr  = newPtr;
			auto newestPtr = parseString(newerPtr, parserVal.getCurrentString().data(), newSize);
			iter += (newerPtr - newPtr) + 1;
			skipWs(iter);
			if (newestPtr) [[likely]] {
				newSize = static_cast<uint64_t>(newestPtr - parserVal.getCurrentString().data());
				value.resize(static_cast<uint64_t>(newSize));
				std::memcpy(value.data(), parserVal.getCurrentString().data(), static_cast<uint64_t>(newSize));
			} else {
				parserVal.template createError<error_code::Invalid_String_Characters>(iter);
				skipToNextValue(iter, end);
				return;
			}
		}
	};

	template<jsonifier::concepts::char_type value_type_new, typename derived_type, typename parser_type> struct serial_parse_impl<value_type_new, derived_type, parser_type> {
		template<jsonifier::concepts::char_type value_type, jsonifier::concepts::is_fwd_iterator iterator_type>
		JSONIFIER_INLINE static void serial_impl(value_type&& value, iterator_type&& iter, iterator_type&& end, parser_type& parserVal) {
			value = static_cast<value_type_new>(*iter.operator->() + 1);
			++iter;
			skipWs(iter);
		}
	};

	template<jsonifier::concepts::raw_json_t value_type_new, typename derived_type, typename parser_type> struct serial_parse_impl<value_type_new, derived_type, parser_type> {
		template<jsonifier::concepts::raw_json_t value_type, jsonifier::concepts::is_fwd_iterator iterator_type>
		JSONIFIER_INLINE static void serial_impl(value_type&& value, iterator_type&& iter, iterator_type&& end, parser_type&) {
			auto newPtr = iter.operator->();
			skipToNextValue(iter, end);
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

	template<jsonifier::concepts::unique_ptr_t value_type_new, typename derived_type, typename parser_type> struct serial_parse_impl<value_type_new, derived_type, parser_type> {
		template<jsonifier::concepts::unique_ptr_t value_type, jsonifier::concepts::is_fwd_iterator iterator_type>
		JSONIFIER_INLINE static void serial_impl(value_type&& value, iterator_type&& iter, iterator_type&& end, parser_type& parserVal) {
			parser<derived_type>::serial_impl(*value, iter, end, parserVal);
		}
	};

	template<jsonifier::concepts::always_null_t value_type_new, typename derived_type, typename parser_type> struct serial_parse_impl<value_type_new, derived_type, parser_type> {
		template<jsonifier::concepts::null_t value_type, jsonifier::concepts::is_fwd_iterator iterator_type>
		JSONIFIER_INLINE static void serial_impl(value_type&&, iterator_type&& iter, iterator_type&& end, parser_type& parserVal) {
			if (!parseNull(iter.operator->())) [[unlikely]] {
				parserVal.template createError<error_code::Invalid_Null_Value>(iter);
				skipToNextValue(iter, end);
			} else {
				++iter;
				skipWs(iter);
			}
		}
	};

	template<jsonifier::concepts::enum_t value_type_new, typename derived_type, typename parser_type> struct serial_parse_impl<value_type_new, derived_type, parser_type> {
		template<jsonifier::concepts::enum_t value_type, jsonifier::concepts::is_fwd_iterator iterator_type>
		JSONIFIER_INLINE static void serial_impl(value_type&& value, iterator_type&& iter, iterator_type&& end, parser_type& parserVal) {
			uint64_t newValue{};
			auto newValueOld = static_cast<int64_t>(value);
			parser<derived_type>::serial_impl(newValue, iter, end, parserVal);
			newValue |= newValueOld;
			value = static_cast<value_type_new>(newValue);
		}
	};

	template<jsonifier::concepts::num_t value_type_new, typename derived_type, typename parser_type> struct serial_parse_impl<value_type_new, derived_type, parser_type> {
		template<jsonifier::concepts::num_t value_type, jsonifier::concepts::is_fwd_iterator iterator_type>
		JSONIFIER_INLINE static void serial_impl(value_type&& value, iterator_type&& iter, iterator_type&& end, parser_type& parserVal) {
			auto newPtr = iter.operator->();
			if (parseNumber(value, newPtr)) [[likely]] {
				iter = newPtr;
				skipWs(iter);
			} else {
				parserVal.template createError<error_code::Invalid_Number_Value>(iter);
				skipToNextValue(iter, end);
			}
		}
	};

	template<jsonifier::concepts::bool_t value_type_new, typename derived_type, typename parser_type> struct serial_parse_impl<value_type_new, derived_type, parser_type> {
		template<jsonifier::concepts::bool_t value_type, jsonifier::concepts::is_fwd_iterator iterator_type>
		JSONIFIER_INLINE static void serial_impl(value_type&& value, iterator_type&& iter, iterator_type&& end, parser_type& parserVal) {
			if (auto result = parseBool(value, iter.operator->()); result > 0) [[likely]] {
				iter += static_cast<int64_t>(result);
				skipWs(iter);
			} else {
				parserVal.template createError<error_code::Invalid_Bool_Value>(iter);
				skipToNextValue(iter, end);
			}
		}
	};

}