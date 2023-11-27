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
#include <jsonifier/Base.hpp>
#include <memory>

namespace jsonifier_internal {

	template<jsonifier::concepts::bool_t value_type_new> struct parse_impl<value_type_new> {
		template<jsonifier::concepts::bool_t value_type, jsonifier::concepts::is_fwd_iterator iterator_type>
		JSONIFIER_INLINE static void op(value_type&& value, iterator_type&& iter) {
			auto newPtr = iter.operator->();
			if (!derailleur::checkForMatchClosed<json_structural_type::Jsonifier_Bool>(iter)) [[unlikely]] {
				iter.template createError<json_structural_type::Jsonifier_Bool>();
				derailleur::skipValue(iter);
				return;
			}
			value = parseBool(newPtr);
		}
	};

	template<jsonifier::concepts::num_t value_type_new> struct parse_impl<value_type_new> {
		template<jsonifier::concepts::num_t value_type, jsonifier::concepts::is_fwd_iterator iterator_type>
		JSONIFIER_INLINE static void op(value_type&& value, iterator_type&& iter) {
			auto newPtr = iter.operator->();
			if (!derailleur::checkForMatchClosed<json_structural_type::Jsonifier_Number>(iter)) [[unlikely]] {
				iter.template createError<json_structural_type::Jsonifier_Number>();
				derailleur::skipValue(iter);
				return;
			}
			if (!parseNumber(value, newPtr)) [[unlikely]] {
				iter.template createError<json_structural_type::Jsonifier_Number>();
			}
		}
	};

	template<jsonifier::concepts::enum_t value_type_new> struct parse_impl<value_type_new> {
		template<jsonifier::concepts::enum_t value_type, jsonifier::concepts::is_fwd_iterator iterator_type>
		JSONIFIER_INLINE static void op(value_type&& value, iterator_type&& iter) {
			uint64_t newValue{};
			auto newValueOld = static_cast<int64_t>(value);
			parse::op(newValue, iter);
			newValue |= newValueOld;
			value = static_cast<value_type_new>(newValue);
		}
	};

	template<jsonifier::concepts::always_null_t value_type_new> struct parse_impl<value_type_new> {
		template<jsonifier::concepts::null_t value_type, jsonifier::concepts::is_fwd_iterator iterator_type>
		JSONIFIER_INLINE static void op(value_type&& value, iterator_type&& iter) {
			if (!derailleur::checkForMatchClosed<json_structural_type::Jsonifier_Null>(iter)) [[unlikely]] {
				iter.template createError<json_structural_type::Jsonifier_Null>();
				derailleur::skipValue(iter);
				return;
			}
		}
	};

	template<jsonifier::concepts::unique_ptr_t value_type_new> struct parse_impl<value_type_new> {
		template<jsonifier::concepts::unique_ptr_t value_type, jsonifier::concepts::is_fwd_iterator iterator_type>
		JSONIFIER_INLINE static void op(value_type&& value, iterator_type&& iter) {
			parse::op(*value, iter);
		}
	};

	template<jsonifier::concepts::raw_json_t value_type_new> struct parse_impl<value_type_new> {
		template<jsonifier::concepts::raw_json_t value_type, jsonifier::concepts::is_fwd_iterator iterator_type>
		JSONIFIER_INLINE static void op(value_type&& value, iterator_type&& iter) {
			auto newPtr = iter.operator->();
			derailleur::skipValue(iter);
			int64_t newSize = iter.operator->() - newPtr;
			if (newSize > 0) [[likely]] {
				value.resize(static_cast<uint64_t>(newSize));
				std::memcpy(value.data(), newPtr, static_cast<uint64_t>(newSize));
			}
			return;
		}
	};

	template<jsonifier::concepts::string_t value_type_new> struct parse_impl<value_type_new> {
		template<jsonifier::concepts::string_t value_type, jsonifier::concepts::is_fwd_iterator iterator_type>
		JSONIFIER_INLINE static void op(value_type&& value, iterator_type&& iter) {
			auto newPtr = iter.operator->();
			if (!derailleur::checkForMatchClosed<json_structural_type::Jsonifier_String>(iter)) [[unlikely]] {
				iter.template createError<json_structural_type::Jsonifier_String>();
				derailleur::skipValue(iter);
				return;
			}
			auto newSize = roundUpToMultiple<int64_t, BytesPerStep>((iter.operator->() - newPtr));
			if (newSize > static_cast<int64_t>(iter.size())) [[unlikely]] {
				iter.resize(static_cast<uint64_t>(newSize));
			}
			newSize = parseString(newPtr + 1, iter.data(), static_cast<uint64_t>(newSize)) - iter.data();
			if (newSize > 0) [[likely]] {
				value.resize(static_cast<uint64_t>(newSize));
				std::memcpy(value.data(), iter.data(), static_cast<uint64_t>(newSize));
			}
		}
	};

	template<jsonifier::concepts::char_type value_type_new> struct parse_impl<value_type_new> {
		template<jsonifier::concepts::char_type value_type, jsonifier::concepts::is_fwd_iterator iterator_type>
		JSONIFIER_INLINE static void op(value_type&& value, iterator_type&& iter) {
			value = static_cast<value_type_new>(*iter.operator->() + 1);
			++iter;
		}
	};

	template<jsonifier::concepts::raw_array_t value_type_new> struct parse_impl<value_type_new> {
		template<jsonifier::concepts::raw_array_t value_type, jsonifier::concepts::is_fwd_iterator iterator_type>
		JSONIFIER_INLINE static void op(value_type&& value, iterator_type&& iter) {
			if (!derailleur::checkForMatchClosed<json_structural_type::Jsonifier_Array_Start>(iter)) [[unlikely]] {
				iter.template createError<json_structural_type::Jsonifier_Array_Start>();
				derailleur::skipValue(iter);
				return;
			}
			if (derailleur::checkForMatchOpen<json_structural_type::Jsonifier_Array_End>(iter)) [[unlikely]] {
				return;
			}
			auto n = std::size(value);

			for (uint64_t x = 0; x < n; ++x) {
				parse::op(value[x], iter);
				if (!derailleur::checkForMatchOpen<json_structural_type::Jsonifier_Comma>(iter)) [[unlikely]] {
					if (!derailleur::checkForMatchClosed<json_structural_type::Jsonifier_Array_End>(iter)) [[unlikely]] {
						iter.template createError<json_structural_type::Jsonifier_Array_End>();
						derailleur::skipValue(iter);
					}
					return;
				}
			}
		}
	};

	template<jsonifier::concepts::vector_t value_type_new> struct parse_impl<value_type_new> {
		template<jsonifier::concepts::vector_t value_type, jsonifier::concepts::is_fwd_iterator iterator_type>
		JSONIFIER_INLINE static void op(value_type&& value, iterator_type&& iter) {
			if (!derailleur::checkForMatchClosed<json_structural_type::Jsonifier_Array_Start>(iter)) [[unlikely]] {
				iter.template createError<json_structural_type::Jsonifier_Array_Start>();
				derailleur::skipValue(iter);
				return;
			}
			if (derailleur::checkForMatchOpen<json_structural_type::Jsonifier_Array_End>(iter)) [[unlikely]] {
				return;
			}
			auto newPtr = value.begin();
			auto endPtr = value.end();
			for (; newPtr < endPtr; ++newPtr) {
				parse::op(*newPtr, iter);
				if (!derailleur::checkForMatchOpen<json_structural_type::Jsonifier_Comma>(iter)) [[unlikely]] {
					if (!derailleur::checkForMatchClosed<json_structural_type::Jsonifier_Array_End>(iter)) [[unlikely]] {
						iter.template createError<json_structural_type::Jsonifier_Array_End>();
						derailleur::skipValue(iter);
					}
					return;
				}
			}
			if constexpr (jsonifier::concepts::has_resize<value_type_new>) {
				auto oldSize = value.size();
				value.resize(value.size() + derailleur::countValueElements(iter));
				newPtr = value.begin() + static_cast<int64_t>(oldSize);
				endPtr = value.end();
				for (; newPtr < endPtr; ++newPtr) {
					parse::op(*newPtr, iter);
					if (!derailleur::checkForMatchOpen<json_structural_type::Jsonifier_Comma>(iter)) [[unlikely]] {
						if (!derailleur::checkForMatchClosed<json_structural_type::Jsonifier_Array_End>(iter)) [[unlikely]] {
							iter.template createError<json_structural_type::Jsonifier_Array_End>();
							derailleur::skipValue(iter);
						}
						return;
					}
				}
			}
		}
	};

	template<jsonifier::concepts::map_t value_type_new> struct parse_impl<value_type_new> {
		template<jsonifier::concepts::map_t value_type, jsonifier::concepts::is_fwd_iterator iterator_type>
		JSONIFIER_INLINE static void op(value_type&& value, iterator_type&& iter) {
			if (!derailleur::checkForMatchClosed<json_structural_type::Jsonifier_Object_Start>(iter)) [[unlikely]] {
				iter.template createError<json_structural_type::Jsonifier_Object_Start>();
				derailleur::skipToEndOfObject(iter);
				return;
			}
			bool first{ true };
			while (iter) {
				if (derailleur::checkForMatchOpen<json_structural_type::Jsonifier_Object_End>(iter)) [[unlikely]] {
					return;
				} else if (first) [[unlikely]] {
					first = false;
				} else [[likely]] {
					if (!derailleur::checkForMatchClosed<json_structural_type::Jsonifier_Comma>(iter)) [[unlikely]] {
						iter.template createError<json_structural_type::Jsonifier_Comma>();
						derailleur::skipToEndOfObject(iter);
						return;
					}
				}
				if (derailleur::checkForMatchOpen<json_structural_type::Jsonifier_Null>(iter)) [[unlikely]] {
					continue;
				}

				static thread_local typename value_type_new::key_type keyNew{};
				parse::op(keyNew, iter);
				if (!derailleur::checkForMatchClosed<json_structural_type::Jsonifier_Colon>(iter)) [[unlikely]] {
					iter.template createError<json_structural_type::Jsonifier_Colon>();
					derailleur::skipToEndOfObject(iter);
					return;
				}
				parse::op(value[keyNew], iter);
			}
		}
	};

	template<jsonifier::concepts::jsonifier_scalar_value_t value_type_new> struct parse_impl<value_type_new> {
		template<jsonifier::concepts::jsonifier_scalar_value_t value_type, jsonifier::concepts::is_fwd_iterator iterator_type>
		JSONIFIER_INLINE static void op(value_type&& value, iterator_type&& iter) {
			static constexpr auto size{ std::tuple_size_v<jsonifier::concepts::core_t<value_type_new>> };
			if constexpr (size > 0) {
				parse::op(getMember(value, get<0>(jsonifier::concepts::coreV<value_type_new>)), iter);
			}
		}
	};

	template<jsonifier::concepts::jsonifier_value_t value_type_new> struct parse_impl<value_type_new> {
		template<jsonifier::concepts::jsonifier_value_t value_type, jsonifier::concepts::is_fwd_iterator iterator_type, jsonifier::concepts::has_find... key_type>
		JSONIFIER_INLINE static void op(value_type&& value, iterator_type&& iter, key_type&... excludedKeys) {
			if (!derailleur::checkForMatchClosed<json_structural_type::Jsonifier_Object_Start>(iter)) [[unlikely]] {
				iter.template createError<json_structural_type::Jsonifier_Object_Start>();
				derailleur::skipToEndOfObject(iter);
				return;
			}
			bool first{ true };
			while (iter) {
				if (derailleur::checkForMatchOpen<json_structural_type::Jsonifier_Object_End>(iter)) [[unlikely]] {
					return;
				} else if (first) [[unlikely]] {
					first = false;
				} else if (!derailleur::checkForMatchClosed<json_structural_type::Jsonifier_Comma>(iter)) [[likely]] {
					iter.template createError<json_structural_type::Jsonifier_Comma>();
					derailleur::skipToNextValue(iter);
					continue;
				}

				auto start = iter.operator->();
				if (!derailleur::checkForMatchClosed<json_structural_type::Jsonifier_String>(iter)) [[likely]] {
					iter.template createError<json_structural_type::Jsonifier_String>();
					derailleur::skipToNextValue(iter);
					continue;
				}
				const jsonifier::string_view_base<uint8_t> key{ start + 1, static_cast<uint64_t>(iter.operator->() - (start + 2)) };
				if constexpr ((( !std::is_void_v<key_type> ) || ...)) {
					if (((excludedKeys.find(static_cast<jsonifier::concepts::unwrap<key_type...>::key_type>(key)) != excludedKeys.end()) & ...)) [[unlikely]] {
						derailleur::skipToNextValue(iter);
						continue;
					}
				}
				if (!derailleur::checkForMatchClosed<json_structural_type::Jsonifier_Colon>(iter)) [[likely]] {
					iter.template createError<json_structural_type::Jsonifier_Colon>();
					derailleur::skipToNextValue(iter);
					continue;
				}
				static constexpr auto frozenMap = makeMap<value_type_new>();
				const auto& memberIt			= frozenMap.find(key);
				if (derailleur::checkForMatchOpen<json_structural_type::Jsonifier_Null>(iter)) [[unlikely]] {
					continue;
				} else if (memberIt != frozenMap.end()) [[likely]] {
					using member_ptr_t = decltype(memberIt->second);
					if constexpr (std::variant_size_v<member_ptr_t> > 0) {
						std::visit(
							[&](auto&& memberPtr) {
								parse::op(getMember(value, std::forward<decltype(memberPtr)>(memberPtr)), iter);
							},
							std::move(memberIt->second));
					}
				} else [[unlikely]] {
					derailleur::skipValue(iter);
				}
			}
		}
	};

}