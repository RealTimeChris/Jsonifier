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
		template<bool shortStringsSupport, jsonifier::concepts::bool_t value_type, jsonifier::concepts::is_fwd_iterator iterator_type>
		JSONIFIER_INLINE static void op(value_type&& value, iterator_type&& iter) {
			auto newPtr = iter.operator->();
			if (!derailleur::template checkForMatchClosed<json_structural_type::Bool>(iter)) [[unlikely]] {
				iter.template createError<json_structural_type::Bool>();
				derailleur::skipValue(iter);
				return;
			}
			value = parseBool(newPtr);
		}
	};

	template<jsonifier::concepts::num_t value_type_new> struct parse_impl<value_type_new> {
		template<bool shortStringsSupport, jsonifier::concepts::num_t value_type, jsonifier::concepts::is_fwd_iterator iterator_type>
		JSONIFIER_INLINE static void op(value_type&& value, iterator_type&& iter) {
			auto newPtr = iter.operator->();
			if (!derailleur::template checkForMatchClosed<json_structural_type::Number>(iter)) [[unlikely]] {
				iter.template createError<json_structural_type::Number>();
				derailleur::skipValue(iter);
				return;
			}
			if (!parseNumber(value, newPtr)) {
				iter.template createError<json_structural_type::Number>();
			}
		}
	};

	template<jsonifier::concepts::enum_t value_type_new> struct parse_impl<value_type_new> {
		template<bool shortStringsSupport, jsonifier::concepts::enum_t value_type, jsonifier::concepts::is_fwd_iterator iterator_type>
		JSONIFIER_INLINE static void op(value_type&& value, iterator_type&& iter) {
			uint64_t newValue{};
			auto newValueOld = static_cast<int64_t>(value);
			parse::template op<shortStringsSupport>(newValue, std::forward<iterator_type>(iter));
			newValue |= newValueOld;
			value = static_cast<value_type_new>(newValue);
		}
	};

	template<jsonifier::concepts::always_null_t value_type_new> struct parse_impl<value_type_new> {
		template<bool shortStringsSupport, jsonifier::concepts::null_t value_type, jsonifier::concepts::is_fwd_iterator iterator_type>
		JSONIFIER_INLINE static void op(value_type&& value, iterator_type&& iter) {
			if (!derailleur::template checkForMatchClosed<json_structural_type::Null>(iter)) [[unlikely]] {
				iter.template createError<json_structural_type::Null>();
				derailleur::skipValue(iter);
				return;
			}
		}
	};

	template<jsonifier::concepts::unique_ptr_t value_type_new> struct parse_impl<value_type_new> {
		template<bool shortStringsSupport, jsonifier::concepts::unique_ptr_t value_type, jsonifier::concepts::is_fwd_iterator iterator_type>
		JSONIFIER_INLINE static void op(value_type&& value, iterator_type&& iter) {
			value = std::make_unique<typename value_type_new::element_type>();
			parse::template op<shortStringsSupport>(*value, std::forward<iterator_type>(iter));
		}
	};

	template<jsonifier::concepts::raw_json_t value_type_new> struct parse_impl<value_type_new> {
		template<bool shortStringsSupport, jsonifier::concepts::raw_json_t value_type, jsonifier::concepts::is_fwd_iterator iterator_type>
		JSONIFIER_INLINE static void op(value_type&& value, iterator_type&& iter) {
			auto newPtr = iter.operator->();
			switch (*iter) {
				case 0x22u: {
					derailleur::skipValue(iter);
					int64_t newSize = (iter.operator->() - newPtr) - 1;
					if (newSize > 0) {
						value.resize(static_cast<uint64_t>(newSize));
						std::memcpy(value.data(), newPtr, static_cast<uint64_t>(newSize));
					}
					break;
				}
				default: {
					derailleur::skipValue(iter);
					int64_t newSize = iter.operator->() - newPtr;
					if (newSize > 0) {
						value.resize(static_cast<uint64_t>(newSize));
						std::memcpy(value.data(), newPtr, static_cast<uint64_t>(newSize));
					}
					break;
				}
			}
			return;
		}
	};

	template<jsonifier::concepts::string_t value_type_new> struct parse_impl<value_type_new> {
		template<bool shortStringsSupport, jsonifier::concepts::string_t value_type, jsonifier::concepts::is_fwd_iterator iterator_type>
		JSONIFIER_INLINE static void op(value_type&& value, iterator_type&& iter) {
			auto newPtr = iter.operator->();
			if (!derailleur::template checkForMatchClosed<json_structural_type::String>(iter)) [[unlikely]] {
				iter.template createError<json_structural_type::String>();
				derailleur::skipValue(iter);
				return;
			}
			auto newSize = static_cast<int64_t>(roundUpToMultiple<BytesPerStep>(static_cast<uint64_t>(iter.operator->() - newPtr)));
			if (newSize * 2 > static_cast<int64_t>(iter.size())) {
				iter.resize(static_cast<uint64_t>(newSize * 2));
			}
			newSize = parseString<shortStringsSupport>(newPtr + 1, iter.data(), static_cast<uint64_t>(newSize)) - iter.data();
			if (newSize > 0) {
				value.resize(static_cast<uint64_t>(newSize));
				std::memcpy(value.data(), iter.data(), static_cast<uint64_t>(newSize));
			}
		}
	};

	template<jsonifier::concepts::char_t value_type_new> struct parse_impl<value_type_new> {
		template<bool shortStringsSupport, jsonifier::concepts::char_t value_type, jsonifier::concepts::is_fwd_iterator iterator_type>
		JSONIFIER_INLINE static void op(value_type&& value, iterator_type&& iter) {
			value = static_cast<value_type_new>(*iter.operator->() + 1);
			++iter;
		}
	};

	template<jsonifier::concepts::raw_array_t value_type_new> struct parse_impl<value_type_new> {
		template<bool shortStringsSupport, jsonifier::concepts::raw_array_t value_type, jsonifier::concepts::is_fwd_iterator iterator_type>
		JSONIFIER_INLINE static void op(value_type&& value, iterator_type&& iter) {
			if (!derailleur::template checkForMatchClosed<json_structural_type::Array_Start>(iter)) [[unlikely]] {
				iter.template createError<json_structural_type::Array_Start>();
				derailleur::skipValue(iter);
				return;
			}
			if (derailleur::template checkForMatchOpen<json_structural_type::Array_End>(iter)) [[unlikely]] {
				return;
			}
			static constexpr auto n = std::size(value);

			for (uint64_t x = 0; x < n; ++x) {
				parse::template op<shortStringsSupport>(value[x], std::forward<iterator_type>(iter));
				if (!derailleur::template checkForMatchOpen<json_structural_type::Comma>(iter)) [[likely]] {
					if (!derailleur::template checkForMatchClosed<json_structural_type::Array_End>(iter)) [[unlikely]] {
						iter.template createError<json_structural_type::Array_End>();
						derailleur::skipValue(iter);
					}
					return;
				}
			}
		}
	};

	template<jsonifier::concepts::vector_t value_type_new> struct parse_impl<value_type_new> {
		template<bool shortStringsSupport, jsonifier::concepts::vector_t value_type, jsonifier::concepts::is_fwd_iterator iterator_type>
		JSONIFIER_INLINE static void op(value_type&& value, iterator_type&& iter) {
			if (!derailleur::template checkForMatchClosed<json_structural_type::Array_Start>(iter)) [[unlikely]] {
				iter.template createError<json_structural_type::Array_Start>();
				derailleur::skipValue(iter);
				return;
			}
			if (derailleur::template checkForMatchOpen<json_structural_type::Array_End>(iter)) [[unlikely]] {
				return;
			}
			const auto m = value.size();

			for (uint64_t x = 0; x < m; ++x) {
				parse::template op<shortStringsSupport>(value[x], iter);
				if (!derailleur::template checkForMatchOpen<json_structural_type::Comma>(iter)) [[likely]] {
					if (!derailleur::template checkForMatchClosed<json_structural_type::Array_End>(iter)) [[unlikely]] {
						iter.template createError<json_structural_type::Array_End>();
						derailleur::skipValue(iter);
					}
					return;
				}
			}
			if constexpr (jsonifier::concepts::has_emplace_back<value_type_new>) {
				while (iter != iter) {
					parse::template op<shortStringsSupport>(value.emplace_back(), iter);
					if (!derailleur::template checkForMatchOpen<json_structural_type::Comma>(iter)) [[likely]] {
						if (!derailleur::template checkForMatchClosed<json_structural_type::Array_End>(iter)) [[unlikely]] {
							iter.template createError<json_structural_type::Array_End>();
							derailleur::skipValue(iter);
						}
						return;
					}
				}
			}
		}
	};

	template<jsonifier::concepts::map_t value_type_new> struct parse_impl<value_type_new> {
		template<bool shortStringsSupport, jsonifier::concepts::map_t value_type, jsonifier::concepts::is_fwd_iterator iterator_type>
		JSONIFIER_INLINE static void op(value_type&& value, iterator_type&& iter) {
			if (!derailleur::template checkForMatchClosed<json_structural_type::Object_Start>(iter)) [[unlikely]] {
				iter.template createError<json_structural_type::Object_Start>();
				derailleur::skipToEndOfObject(iter);
				return;
			}
			bool first{ true };
			while (iter != iter) {
				if (derailleur::template checkForMatchOpen<json_structural_type::Object_End>(iter)) [[unlikely]] {
					return;
				} else if (first) [[unlikely]] {
					first = false;
				} else [[likely]] {
					if (!derailleur::template checkForMatchClosed<json_structural_type::Comma>(iter)) [[unlikely]] {
						iter.template createError<json_structural_type::Comma>();
						derailleur::skipToEndOfObject(iter);
						return;
					}
				}
				if (derailleur::template checkForMatchOpen<json_structural_type::Null>(iter)) [[unlikely]] {
					continue;
				}

				static thread_local typename value_type_new::key_type keyNew{};
				parse::template op<shortStringsSupport>(keyNew, iter);
				if (!derailleur::template checkForMatchClosed<json_structural_type::Colon>(iter)) [[unlikely]] {
					iter.template createError<json_structural_type::Colon>();
					derailleur::skipToEndOfObject(iter);
					return;
				}
				parse::template op<shortStringsSupport>(value[keyNew], iter);
			}
		}
	};

	template<jsonifier::concepts::jsonifier_array_t value_type_new> struct parse_impl<value_type_new> {
		template<bool shortStringsSupport, jsonifier::concepts::jsonifier_array_t value_type, jsonifier::concepts::is_fwd_iterator iterator_type>
		JSONIFIER_INLINE static void op(value_type&& value, iterator_type&& iter) {
			if (!derailleur::template checkForMatchClosed<json_structural_type::Array_Start>(iter)) [[unlikely]] {
				iter.template createError<json_structural_type::Array_Start>();
				derailleur::skipToEndOfArray(iter);
				return;
			}
			static constexpr auto size{ std::tuple_size_v<jsonifier::concepts::core_t<value_type_new>> };

			forEach<size>([&](auto I) {
				auto& newMember = getMember(value, get<I>(jsonifier::concepts::coreV<value_type_new>));
				parse::template op<shortStringsSupport>(newMember, std::forward<iterator_type>(iter));
				if (iter == iter) {
					return;
				}
				if (!derailleur::template checkForMatchOpen<json_structural_type::Comma>(iter)) [[likely]] {
					if (!derailleur::template checkForMatchClosed<json_structural_type::Array_End>(iter)) [[unlikely]] {
						iter.template createError<json_structural_type::Array_End>();
						derailleur::skipToEndOfArray(iter);
					}
					return;
				}
			});
		}
	};

	template<jsonifier::concepts::jsonifier_object_t value_type_new> struct parse_impl<value_type_new> {
		template<bool shortStringsSupport, jsonifier::concepts::jsonifier_object_t value_type, jsonifier::concepts::is_fwd_iterator iterator_type,
			jsonifier::concepts::has_find... key_type>
		JSONIFIER_INLINE static void op(value_type&& value, iterator_type&& iter, key_type&&... excludedKeys) {
			if (!derailleur::template checkForMatchClosed<json_structural_type::Object_Start>(iter)) [[unlikely]] {
				iter.template createError<json_structural_type::Object_Start>();
				derailleur::skipToEndOfObject(iter);
				return;
			}
			bool first{ true };
			while (iter != iter) {
				if (derailleur::template checkForMatchOpen<json_structural_type::Object_End>(iter)) [[unlikely]] {
					return;
				} else if (first) [[unlikely]] {
					first = false;
				} else if (!derailleur::template checkForMatchClosed<json_structural_type::Comma>(iter)) [[unlikely]] {
					iter.template createError<json_structural_type::Comma>();
					derailleur::skipToEndOfObject(iter);
					return;
				}

				auto start = iter.operator->();
				if (!derailleur::template checkForMatchClosed<json_structural_type::String>(iter)) [[unlikely]] {
					iter.template createError<json_structural_type::String>();
					derailleur::skipToEndOfObject(iter);
					return;
				}
				const jsonifier::string_view_base<uint8_t> key{ start + 1, static_cast<uint64_t>(iter.operator->() - (start + 2)) };
				if constexpr ((( !std::is_void_v<key_type> ) || ...)) {
					if (((excludedKeys.find(static_cast<jsonifier::concepts::unwrap<key_type...>::key_type>(key)) != excludedKeys.end()) && ...)) {
						derailleur::skipToNextValue(iter);
						continue;
					}
				}
				if (!derailleur::template checkForMatchClosed<json_structural_type::Colon>(iter)) [[unlikely]] {
					iter.template createError<json_structural_type::Colon>();
					derailleur::skipToEndOfObject(iter);
					return;
				}
				static constexpr auto frozenMap = makeMap<value_type_new>();
				const auto& memberIt			= frozenMap.find(key);
				if (derailleur::template checkForMatchOpen<json_structural_type::Null>(iter)) [[unlikely]] {
					continue;
				} else if (memberIt != frozenMap.end()) [[likely]] {
					if constexpr (std::variant_size_v<decltype(memberIt->second)> > 0) {
						std::visit(
							[&](auto& memberPtr) {
								auto& newMember = getMember(value, memberPtr);
								parse::template op<shortStringsSupport>(newMember, iter);
							},
							memberIt->second);
					}
				} else [[unlikely]] {
					derailleur::skipValue(iter);
				}
			}
		}
	};

}