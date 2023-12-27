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

	template<jsonifier::concepts::bool_t value_type_new> struct parse_impl<value_type_new> : public derailleur {
		template<jsonifier::concepts::bool_t value_type, jsonifier::concepts::is_fwd_iterator iterator_type>
		JSONIFIER_INLINE static void impl(value_type&& value, iterator_type&& iter) {
			if (parseBool(value, iter.operator->())) {
				++iter;
			} else {
				iter.template createError<json_structural_type::Jsonifier_Bool>();
				skipToNextValue(iter);
			}
		}
	};

	template<jsonifier::concepts::num_t value_type_new> struct parse_impl<value_type_new> : public derailleur {
		template<jsonifier::concepts::num_t value_type, jsonifier::concepts::is_fwd_iterator iterator_type>
		JSONIFIER_INLINE static void impl(value_type&& value, iterator_type&& iter) {
			if (parseNumber(value, iter.operator->())) [[unlikely]] {
				++iter;
			} else {
				iter.template createError<json_structural_type::Jsonifier_Number>();
				skipToNextValue(iter);
			}
		}
	};

	template<jsonifier::concepts::enum_t value_type_new> struct parse_impl<value_type_new> : public derailleur {
		template<jsonifier::concepts::enum_t value_type, jsonifier::concepts::is_fwd_iterator iterator_type>
		JSONIFIER_INLINE static void impl(value_type&& value, iterator_type&& iter) {
			uint64_t newValue{};
			auto newValueOld = static_cast<int64_t>(value);
			parse::impl(newValue, iter);
			newValue |= newValueOld;
			value = static_cast<value_type_new>(newValue);
		}
	};

	template<jsonifier::concepts::always_null_t value_type_new> struct parse_impl<value_type_new> : public derailleur {
		template<jsonifier::concepts::null_t value_type, jsonifier::concepts::is_fwd_iterator iterator_type> JSONIFIER_INLINE static void impl(value_type&&, iterator_type&& iter) {
			if (!parseNull(iter.operator->())) [[unlikely]] {
				iter.template createError<json_structural_type::Jsonifier_Null>();
				skipToNextValue(iter);
			} else {
				++iter;
			}
		}
	};

	template<jsonifier::concepts::unique_ptr_t value_type_new> struct parse_impl<value_type_new> : public derailleur {
		template<jsonifier::concepts::unique_ptr_t value_type, jsonifier::concepts::is_fwd_iterator iterator_type>
		JSONIFIER_INLINE static void impl(value_type&& value, iterator_type&& iter) {
			parse::impl(*value, iter);
		}
	};

	template<jsonifier::concepts::raw_json_t value_type_new> struct parse_impl<value_type_new> : public derailleur {
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

	template<jsonifier::concepts::string_t value_type_new> struct parse_impl<value_type_new> : public derailleur {
		template<jsonifier::concepts::string_t value_type, jsonifier::concepts::is_fwd_iterator iterator_type>
		JSONIFIER_INLINE static void impl(value_type&& value, iterator_type&& iter) {
			auto newPtr = iter.operator->();
			if (*iter == 0x22u) [[unlikely]] {
				++iter;
			} else {
				iter.template createError<json_structural_type::Jsonifier_String>();
				skipToNextValue(iter);
				return;
			}
			auto newSize = iter.operator->() - newPtr;
			if (newSize > static_cast<int64_t>(iter.getCurrentString().size())) [[unlikely]] {
				iter.getCurrentString().resize(static_cast<uint64_t>(newSize));
			}
			newPtr = parseString(newPtr + 1, iter.getCurrentString().data(), static_cast<uint64_t>(newSize));
			if (newPtr) [[likely]] {
				newSize = newPtr - iter.getCurrentString().data();
				value.resize(static_cast<uint64_t>(newSize));
				std::memcpy(value.data(), iter.getCurrentString().data(), static_cast<uint64_t>(newSize));
			} else {
				iter.template createError<json_structural_type::Jsonifier_String>();
				skipToNextValue(iter);
				return;
			}
		}
	};

	template<jsonifier::concepts::char_type value_type_new> struct parse_impl<value_type_new> : public derailleur {
		template<jsonifier::concepts::char_type value_type, jsonifier::concepts::is_fwd_iterator iterator_type>
		JSONIFIER_INLINE static void impl(value_type&& value, iterator_type&& iter) {
			value = static_cast<value_type_new>(*iter.operator->() + 1);
			++iter;
		}
	};

	template<jsonifier::concepts::raw_array_t value_type_new> struct parse_impl<value_type_new> : public derailleur {
		template<jsonifier::concepts::raw_array_t value_type, jsonifier::concepts::is_fwd_iterator iterator_type>
		JSONIFIER_INLINE static void impl(value_type&& value, iterator_type&& iter) {
			if (*iter == 0x5Bu) [[likely]] {
				++iter;
			} else [[unlikely]] {
				iter.template createError<json_structural_type::Jsonifier_Array_Start>();
				skipToNextValue(iter);
				return;
			}
			if (*iter == 0x5Du) [[unlikely]] {
				++iter;
				return;
			}
			auto n = std::size(value);

			for (uint64_t x = 0; x < n; ++x) {
				parse::impl(value[x], iter);
				if (*iter == 0x2Cu) [[likely]] {
					++iter;
				} else if (*iter == 0x5Du) {
					++iter;
					return;
				} else [[unlikely]] {
					iter.template createError<json_structural_type::Jsonifier_Array_End>();
					skipToNextValue(iter);
					return;
				}
			}
		}
	};

	template<jsonifier::concepts::vector_t value_type_new> struct parse_impl<value_type_new> : public derailleur {
		template<jsonifier::concepts::vector_t value_type, jsonifier::concepts::is_fwd_iterator iterator_type>
		JSONIFIER_INLINE static void impl(value_type&& value, iterator_type&& iter) {
			if (*iter == 0x5Bu) [[likely]] {
				++iter;
			} else [[unlikely]] {
				iter.template createError<json_structural_type::Jsonifier_Array_Start>();
				skipToNextValue(iter);
				return;
			}
			if (*iter == 0x5Du) [[unlikely]] {
				++iter;
				return;
			}
			auto oldSize = value.size();
			auto newPtr	 = value.data();
			auto endPtr	 = value.data() + oldSize;
			for (; newPtr < endPtr; ++newPtr) {
				parse::impl(*newPtr, iter);
				if (*iter == 0x2Cu) [[likely]] {
					++iter;
				} else if (*iter == 0x5Du) {
					++iter;
					return;
				} else [[unlikely]] {
					iter.template createError<json_structural_type::Jsonifier_Array_End>();
					skipToEndOfArray(iter);
					return;
				}
			}
			if constexpr (jsonifier::concepts::has_resize<value_type_new>) {
				auto newSize = value.size() + countValueElements(iter);
				value.resize(newSize);
				newPtr = value.data() + static_cast<int64_t>(oldSize);
				endPtr = value.data() + newSize;
				for (; newPtr < endPtr; ++newPtr) {
					parse::impl(*newPtr, iter);
					if (*iter == 0x2Cu) [[likely]] {
						++iter;
					} else if (*iter == 0x5Du) {
						++iter;
						return;
					} else [[unlikely]] {
						iter.template createError<json_structural_type::Jsonifier_Array_End>();
						skipToEndOfArray(iter);
						return;
					}
				}
			}
		}
	};

	template<jsonifier::concepts::map_t value_type_new> struct parse_impl<value_type_new> : public derailleur {
		template<jsonifier::concepts::map_t value_type, jsonifier::concepts::is_fwd_iterator iterator_type>
		JSONIFIER_INLINE static void impl(value_type&& value, iterator_type&& iter) {
			if (*iter == 0x7Bu) [[likely]] {
				++iter;
			} else [[unlikely]] {
				iter.template createError<json_structural_type::Jsonifier_Object_Start>();
				skipToNextValue(iter);
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
					iter.template createError<json_structural_type::Jsonifier_Array_End>();
					skipToNextValue(iter);
					return;
				}

				if (*iter == 0x6Eu) [[unlikely]] {
					++iter;
					continue;
				}

				static thread_local typename value_type_new::key_type keyNew{};
				parse::impl(keyNew, iter);
				if (*iter == 0x3Au) [[likely]] {
					++iter;
				} else [[unlikely]] {
					iter.template createError<json_structural_type::Jsonifier_Colon>();
					skipToEndOfObject(iter);
					return;
				}
				parse::impl(value[keyNew], iter);
			}
		}
	};

	template<jsonifier::concepts::variant_t value_type_new> struct parse_impl<value_type_new> : public derailleur {
		template<jsonifier::concepts::variant_t value_type, jsonifier::concepts::is_fwd_iterator iterator_type>
		JSONIFIER_INLINE static void impl(value_type&& value, iterator_type&& iter) {
			std::visit(
				[&](auto&& valueNew) {
					parse::impl(valueNew, iter);
				},
				value);
		}
	};

	template<jsonifier::concepts::jsonifier_scalar_value_t value_type_new> struct parse_impl<value_type_new> : public derailleur {
		template<jsonifier::concepts::jsonifier_scalar_value_t value_type, jsonifier::concepts::is_fwd_iterator iterator_type>
		JSONIFIER_INLINE static void impl(value_type&& value, iterator_type&& iter) {
			static constexpr auto size{ std::tuple_size_v<jsonifier::concepts::core_t<value_type_new>> };
			if constexpr (size > 0) {
				parse::impl(getMember(value, get<0>(jsonifier::concepts::coreV<value_type_new>)), iter);
			}
		}
	};

	template<jsonifier::concepts::jsonifier_value_t value_type_new> struct parse_impl<value_type_new> : public derailleur {
		template<jsonifier::concepts::jsonifier_value_t value_type, jsonifier::concepts::is_fwd_iterator iterator_type, jsonifier::concepts::has_find... key_type>
		JSONIFIER_INLINE static void impl(value_type&& value, iterator_type&& iter, key_type&... excludedKeys) {
			if (*iter == 0x7Bu) [[likely]] {
				++iter;
			} else [[unlikely]] {
				iter.template createError<json_structural_type::Jsonifier_Object_Start>();
				skipToEndOfObject(iter);
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
					iter.template createError<json_structural_type::Jsonifier_Array_End>();
					skipToEndOfObject(iter);
					return;
				}

				auto start = iter.operator->();
				if (*iter == 0x22u) [[likely]] {
					++iter;
				} else [[unlikely]] {
					iter.template createError<json_structural_type::Jsonifier_String>();
					skipToEndOfObject(iter);
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
					iter.template createError<json_structural_type::Jsonifier_Colon>();
					skipToEndOfObject(iter);
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
								parse::impl(getMember(value, std::forward<decltype(memberPtr)>(memberPtr)), iter);
							},
							std::move(memberIt->second));
					}
				} else [[unlikely]] {
					skipToNextValue(iter);
				}
			}
		}
	};

}