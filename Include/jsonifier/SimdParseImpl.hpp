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

	template<jsonifier::concepts::jsonifier_value_t value_type_new, typename derived_type> struct simd_parse_impl<value_type_new, derived_type> {
		template<jsonifier::concepts::jsonifier_value_t value_type, jsonifier::concepts::is_fwd_iterator iterator_type, jsonifier::concepts::has_find... key_type>
		JSONIFIER_INLINE static void simd_impl(value_type&& value, iterator_type&& iter, key_type&... excludedKeys) {
			if (*iter == 0x7Bu) [[likely]] {
				++iter;
			} else [[unlikely]] {
				iter.template createError<json_structural_type::Object_Start>();
				skipToEndOfValue(iter);
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
				} else [[unlikely]] {
					iter.template createError<json_structural_type::Array_End>();
					skipToEndOfValue(iter);
					return;
				}

				auto start = iter.operator->();
				if (*iter == 0x22u) [[likely]] {
					++iter;
				} else [[unlikely]] {
					iter.template createError<error_code::UnQuoted_String>();
					skipToEndOfValue(iter);
					continue;
				}

				const jsonifier::string_view_base<uint8_t> key{ start + 1, static_cast<uint64_t>(iter.operator->() - (start + 2)) };
				if constexpr ((( !std::is_void_v<key_type> ) || ...)) {
					if (((excludedKeys.find(static_cast<jsonifier::concepts::unwrap_t<key_type...>::key_type>(key)) != excludedKeys.end()) & ...)) [[unlikely]] {
						++iter;
						skipToNextValue(iter);
						continue;
					}
				}
				if (*iter == 0x3Au) [[likely]] {
					++iter;
				} else [[unlikely]] {
					iter.template createError<json_structural_type::Colon>();
					skipToEndOfValue(iter);
					continue;
				}
				static constexpr auto frozenMap = makeMap<value_type_new>();
				const auto& memberIt			= frozenMap.find(key);
				if (*iter == 0x6Eu) [[unlikely]] {
					++iter;
					continue;
				} else if (memberIt != frozenMap.end()) [[likely]] {
					using member_ptr_t = decltype(memberIt->second);
					if constexpr (std::variant_size_v<member_ptr_t> > 0) {
						std::visit(
							[&](auto&& memberPtr) {
								parser<derived_type>::simd_impl(getMember(value, std::forward<decltype(memberPtr)>(memberPtr)), iter);
							},
							std::move(memberIt->second));
					}
				} else [[unlikely]] {
					skipToNextValue(iter);
				}
			}
		}
	};

	template<jsonifier::concepts::jsonifier_scalar_value_t value_type_new, typename derived_type> struct simd_parse_impl<value_type_new, derived_type> {
		template<jsonifier::concepts::jsonifier_scalar_value_t value_type, jsonifier::concepts::is_fwd_iterator iterator_type>
		JSONIFIER_INLINE static void simd_impl(value_type&& value, iterator_type&& iter) {
			static constexpr auto size{ std::tuple_size_v<jsonifier::concepts::core_t<value_type_new>> };
			if constexpr (size > 0) {
				parser<derived_type>::simd_impl(getMember(value, get<0>(jsonifier::concepts::coreV<value_type_new>)), iter);
			}
		}
	};

	template<jsonifier::concepts::array_tuple_t value_type_new, typename derived_type> struct simd_parse_impl<value_type_new, derived_type> {
		template<jsonifier::concepts::array_tuple_t value_type, jsonifier::concepts::is_fwd_iterator iterator_type>
		JSONIFIER_INLINE static void simd_impl(value_type&& value, iterator_type&& iter) {
			if (*iter == '[') [[likely]] {
				++iter;
			} else [[unlikely]] {
				iter.template createError<json_structural_type::Array_Start>();
				skipToEndOfValue(iter);
				return;
			}
			static constexpr auto n = std::tuple_size_v<value_type_new>;
			parseObjects<n>(value, iter);
			if (*iter == ']') [[unlikely]] {
				++iter;
				return;
			}
		}

		template<uint64_t n, uint64_t indexNew = 0, bool isItFirst = true, jsonifier::concepts::array_tuple_t value_type, jsonifier::concepts::is_fwd_iterator iterator_type>
		JSONIFIER_INLINE static void parseObjects(value_type&& value, iterator_type&& iter) {
			auto& item = std::get<indexNew>(value);

			if (!isItFirst) {
				if (*iter == ',') [[likely]] {
					++iter;
				}
			}

			parser<derived_type>::simd_impl(item, iter);
			if constexpr (indexNew < n - 1) {
				parseObjects<n, indexNew + 1, false>(value, iter);
			}
		}
	};

	template<jsonifier::concepts::map_t value_type_new, typename derived_type> struct simd_parse_impl<value_type_new, derived_type> {
		template<jsonifier::concepts::map_t value_type, jsonifier::concepts::is_fwd_iterator iterator_type>
		JSONIFIER_INLINE static void simd_impl(value_type&& value, iterator_type&& iter) {
			if (*iter == 0x7Bu) [[likely]] {
				++iter;
			} else [[unlikely]] {
				iter.template createError<json_structural_type::Object_Start>();
				skipToEndOfValue(iter);
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
				} else [[unlikely]] {
					iter.template createError<json_structural_type::Object_End>();
					skipToEndOfValue(iter);
					return;
				}

				if (*iter == 0x6Eu) [[unlikely]] {
					++iter;
					continue;
				}

				static thread_local typename value_type_new::key_type keyNew{};
				parser<derived_type>::simd_impl(keyNew, iter);
				if (*iter == 0x3Au) [[likely]] {
					++iter;
				} else [[unlikely]] {
					iter.template createError<json_structural_type::Colon>();
					skipToEndOfValue(iter);
					return;
				}
				parser<derived_type>::simd_impl(value[keyNew], iter);
			}
		}
	};

	template<jsonifier::concepts::variant_t value_type_new, typename derived_type> struct simd_parse_impl<value_type_new, derived_type> {
		template<jsonifier::concepts::variant_t value_type, jsonifier::concepts::is_fwd_iterator iterator_type>
		JSONIFIER_INLINE static void simd_impl(value_type&& value, iterator_type&& iter) {
			std::visit(
				[&](auto&& valueNew) {
					parser<derived_type>::simd_impl(valueNew, iter);
				},
				value);
		}
	};

	template<jsonifier::concepts::optional_t value_type_new, typename derived_type> struct simd_parse_impl<value_type_new, derived_type> {
		template<jsonifier::concepts::optional_t value_type, jsonifier::concepts::is_fwd_iterator iterator_type>
		JSONIFIER_INLINE static void simd_impl(value_type&& value, iterator_type&& iter) {
			using member_type_new = typename value_type_new::value_type;
			member_type_new newValue{};
			parser<derived_type>::simd_impl(newValue, iter);
			value = newValue;
		}
	};

	template<jsonifier::concepts::vector_t value_type_new, typename derived_type> struct simd_parse_impl<value_type_new, derived_type> {
		template<jsonifier::concepts::vector_t value_type, jsonifier::concepts::is_fwd_iterator iterator_type>
		JSONIFIER_INLINE static void simd_impl(value_type&& value, iterator_type&& iter) {
			if (*iter == 0x5Bu) [[likely]] {
				++iter;
			} else [[unlikely]] {
				iter.template createError<json_structural_type::Array_Start>();
				skipToEndOfValue(iter);
				return;
			}
			if (*iter == 0x5Du) [[unlikely]] {
				++iter;
				return;
			}
			auto oldSize = value.size();
			auto newPtr	 = value.begin();
			auto endPtr	 = value.end();
			for (; newPtr < endPtr; ++newPtr) {
				parser<derived_type>::simd_impl(*newPtr, iter);
				if (*iter == 0x2Cu) [[likely]] {
					++iter;
				} else if (*iter == 0x5Du) {
					++iter;
					return;
				} else [[unlikely]] {
					iter.template createError<json_structural_type::Array_End>();
					skipToEndOfValue(iter);
					return;
				}
			}
			if constexpr (jsonifier::concepts::has_resize<value_type_new>) {
				auto newSize = oldSize + countValueElements(iter);
				value.resize(newSize);
				newPtr = value.begin() + static_cast<int64_t>(oldSize);
				endPtr = value.end();
				for (; newPtr < endPtr; ++newPtr) {
					parser<derived_type>::simd_impl(*newPtr, iter);
					if (*iter == 0x2Cu) [[likely]] {
						++iter;
					} else if (*iter == 0x5Du) {
						++iter;
						return;
					} else [[unlikely]] {
						iter.template createError<json_structural_type::Array_End>();
						skipToEndOfValue(iter);
						return;
					}
				}
			}
		}
	};

	template<jsonifier::concepts::raw_array_t value_type_new, typename derived_type> struct simd_parse_impl<value_type_new, derived_type> {
		template<jsonifier::concepts::raw_array_t value_type, jsonifier::concepts::is_fwd_iterator iterator_type>
		JSONIFIER_INLINE static void simd_impl(value_type&& value, iterator_type&& iter) {
			if (*iter == 0x5Bu) [[likely]] {
				++iter;
			} else [[unlikely]] {
				iter.template createError<json_structural_type::Array_Start>();
				skipToEndOfValue(iter);
				return;
			}
			if (*iter == 0x5Du) [[unlikely]] {
				++iter;
				return;
			}
			auto n = std::size(value);

			for (uint64_t x = 0; x < n; ++x) {
				parser<derived_type>::simd_impl(value[x], iter);
				if (*iter == 0x2Cu) [[likely]] {
					++iter;
				} else if (*iter == 0x5Du) {
					++iter;
					return;
				} else [[unlikely]] {
					iter.template createError<json_structural_type::Array_End>();
					skipToEndOfValue(iter);
					return;
				}
			}
		}
	};

	template<jsonifier::concepts::string_t value_type_new, typename derived_type> struct simd_parse_impl<value_type_new, derived_type> {
		template<jsonifier::concepts::string_t value_type, jsonifier::concepts::is_fwd_iterator iterator_type>
		JSONIFIER_INLINE static void simd_impl(value_type&& value, iterator_type&& iter) {
			auto newPtr = iter.operator->();
			if (*iter == 0x22u) [[unlikely]] {
				++iter;
			} else {
				iter.template createError<json_structural_type::String>();
				skipToNextValue(iter);
				return;
			}
			if (newPtr) {
				auto newSize = static_cast<uint64_t>(iter.operator->() - newPtr);
				if (newSize > iter.getCurrentString().size()) [[unlikely]] {
					iter.getCurrentString().resize(static_cast<uint64_t>(newSize));
				}
				auto newerPtr  = newPtr + 1;
				auto newestPtr = parseString(newerPtr, iter.getCurrentString().data(), newSize);
				if (newestPtr) [[likely]] {
					newSize = static_cast<uint64_t>(newestPtr - iter.getCurrentString().data());
					value.resize(static_cast<uint64_t>(newSize));
					std::memcpy(value.data(), iter.getCurrentString().data(), static_cast<uint64_t>(newSize));
				} else {
					iter.template createError<error_code::Invalid_String_Characters>();
					skipToNextValue(iter);
					return;
				}
			}
		}
	};

	template<jsonifier::concepts::char_type value_type_new, typename derived_type> struct simd_parse_impl<value_type_new, derived_type> {
		template<jsonifier::concepts::char_type value_type, jsonifier::concepts::is_fwd_iterator iterator_type>
		JSONIFIER_INLINE static void simd_impl(value_type&& value, iterator_type&& iter) {
			value = static_cast<value_type_new>(*iter.operator->() + 1);
			++iter;
		}
	};

	template<jsonifier::concepts::raw_json_t value_type_new, typename derived_type> struct simd_parse_impl<value_type_new, derived_type> {
		template<jsonifier::concepts::raw_json_t value_type, jsonifier::concepts::is_fwd_iterator iterator_type>
		JSONIFIER_INLINE static void simd_impl(value_type&& value, iterator_type&& iter) {
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

	template<jsonifier::concepts::unique_ptr_t value_type_new, typename derived_type> struct simd_parse_impl<value_type_new, derived_type> {
		template<jsonifier::concepts::unique_ptr_t value_type, jsonifier::concepts::is_fwd_iterator iterator_type>
		JSONIFIER_INLINE static void simd_impl(value_type&& value, iterator_type&& iter) {
			parser<derived_type>::simd_impl(*value, iter);
		}
	};

	template<jsonifier::concepts::always_null_t value_type_new, typename derived_type> struct simd_parse_impl<value_type_new, derived_type> {
		template<jsonifier::concepts::null_t value_type, jsonifier::concepts::is_fwd_iterator iterator_type>
		JSONIFIER_INLINE static void simd_impl(value_type&&, iterator_type&& iter) {
			if (!parseNull(iter.operator->())) [[unlikely]] {
				iter.template createError<error_code::Invalid_Null_Value>();
				skipToNextValue(iter);
			} else {
				++iter;
			}
		}
	};

	template<jsonifier::concepts::enum_t value_type_new, typename derived_type> struct simd_parse_impl<value_type_new, derived_type> {
		template<jsonifier::concepts::enum_t value_type, jsonifier::concepts::is_fwd_iterator iterator_type>
		JSONIFIER_INLINE static void simd_impl(value_type&& value, iterator_type&& iter) {
			uint64_t newValue{};
			auto newValueOld = static_cast<int64_t>(value);
			parser<derived_type>::simd_impl(newValue, iter);
			newValue |= newValueOld;
			value = static_cast<value_type_new>(newValue);
		}
	};

	template<jsonifier::concepts::num_t value_type_new, typename derived_type> struct simd_parse_impl<value_type_new, derived_type> {
		template<jsonifier::concepts::num_t value_type, jsonifier::concepts::is_fwd_iterator iterator_type>
		JSONIFIER_INLINE static void simd_impl(value_type&& value, iterator_type&& iter) {
			auto newPtr = iter.operator->();
			if (parseNumber(value, newPtr)) [[unlikely]] {
				++iter;
			} else {
				iter.template createError<error_code::Invalid_Number_Value>();
				skipToNextValue(iter);
			}
		}
	};

	template<jsonifier::concepts::bool_t value_type_new, typename derived_type> struct simd_parse_impl<value_type_new, derived_type> {
		template<jsonifier::concepts::bool_t value_type, jsonifier::concepts::is_fwd_iterator iterator_type>
		JSONIFIER_INLINE static void simd_impl(value_type&& value, iterator_type&& iter) {
			if (parseBool(value, iter.operator->())) {
				++iter;
			} else {
				iter.template createError<error_code::Invalid_Bool_Value>();
				skipToNextValue(iter);
			}
		}
	};

}