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
		template<jsonifier::concepts::bool_t value_type, jsonifier::concepts::is_fwd_iterator iterator_type, jsonifier::concepts::is_parser parser_type>
		inline static void op(value_type&& value, iterator_type&& iter, parser_type&& parserNew) {
			auto newPtr = iter.operator->();
			if (!derailleur::template checkForMatchClosed<json_structural_type::Bool>(iter)) [[unlikely]] {
				parserNew.getErrors().emplace_back(createError<json_structural_type::Bool>(iter));
				derailleur::skipValue(iter);
				return;
			}
			value = parseBool(newPtr);
		}
	};

	template<jsonifier::concepts::num_t value_type_new> struct parse_impl<value_type_new> {
		template<jsonifier::concepts::num_t value_type, jsonifier::concepts::is_fwd_iterator iterator_type, jsonifier::concepts::is_parser parser_type>
		inline static void op(value_type&& value, iterator_type&& iter, parser_type&& parserNew) {
			auto newPtr = iter.operator->();
			if (!derailleur::template checkForMatchClosed<json_structural_type::Number>(iter)) [[unlikely]] {
				parserNew.getErrors().emplace_back(createError<json_structural_type::Number>(iter));
				derailleur::skipValue(iter);
				return;
			}
			if (!parseNumber(value, newPtr)) {
				parserNew.getErrors().emplace_back(createError<json_structural_type::Number>(iter));
			}
		}
	};

	template<jsonifier::concepts::enum_t value_type_new> struct parse_impl<value_type_new> {
		template<jsonifier::concepts::enum_t value_type, jsonifier::concepts::is_fwd_iterator iterator_type, jsonifier::concepts::is_parser parser_type>
		inline static void op(value_type&& value, iterator_type&& iter, parser_type&& parserNew) {
			uint64_t newValue{};
			auto newValueOld = static_cast<int64_t>(value);
			parse::op(newValue, iter, parserNew);
			newValue |= newValueOld;
			value = static_cast<value_type_new>(newValue);
		}
	};

	template<jsonifier::concepts::null_t value_type_new> struct parse_impl<value_type_new> {
		template<jsonifier::concepts::null_t value_type, jsonifier::concepts::is_fwd_iterator iterator_type, jsonifier::concepts::is_parser parser_type>
		inline static void op(value_type&& value, iterator_type&& iter, parser_type&& parserNew) {
			if (!derailleur::template checkForMatchClosed<json_structural_type::Null>(iter)) [[unlikely]] {
				parserNew.getErrors().emplace_back(createError<json_structural_type::Null>(iter));
				derailleur::skipValue(iter);
				return;
			}
		}
	};

	template<jsonifier::concepts::unique_ptr_t value_type_new> struct parse_impl<value_type_new> {
		template<jsonifier::concepts::unique_ptr_t value_type, jsonifier::concepts::is_fwd_iterator iterator_type, jsonifier::concepts::is_parser parser_type>
		inline static void op(value_type&& value, iterator_type&& iter, parser_type&& parserNew) {
			value = std::make_unique<typename value_type_new::element_type>();
			if constexpr (jsonifier::concepts::has_excluded_keys<typename value_type_new::element_type>) {
				parse::op(*value, iter, parserNew, value->jsonifierExcludedKeys);
			} else {
				parse::op(*value, iter, parserNew);
			}
		}
	};

	template<jsonifier::concepts::raw_json_t value_type_new> struct parse_impl<value_type_new> {
		template<jsonifier::concepts::raw_json_t value_type, jsonifier::concepts::is_fwd_iterator iterator_type, jsonifier::concepts::is_parser parser_type>
		inline static void op(value_type&& value, iterator_type&& iter, parser_type&&) {
			auto newPtr = iter.operator->();
			switch (*iter) {
				case 0x22u: {
					derailleur::skipValue(iter);
					int64_t sizeNew = (iter.operator->() - newPtr) - 1;
					if (sizeNew > 0) {
						value.resize(static_cast<uint64_t>(sizeNew));
						std::memcpy(value.data(), newPtr, static_cast<uint64_t>(sizeNew));
					}
					break;
				}
				default: {
					derailleur::skipValue(iter);
					int64_t sizeNew = iter.operator->() - newPtr;
					if (sizeNew > 0) {
						value.resize(static_cast<uint64_t>(sizeNew));
						std::memcpy(value.data(), newPtr, static_cast<uint64_t>(sizeNew));
					}
					break;
				}
			}
			return;
		}
	};

	template<jsonifier::concepts::string_t value_type_new> struct parse_impl<value_type_new> {
		template<jsonifier::concepts::string_t value_type, jsonifier::concepts::is_fwd_iterator iterator_type, jsonifier::concepts::is_parser parser_type>
		inline static void op(value_type&& value, iterator_type&& iter, parser_type&& parserNew) {
			auto newPtr = iter.operator->();
			if (!derailleur::template checkForMatchClosed<json_structural_type::String>(iter)) [[unlikely]] {
				parserNew.getErrors().emplace_back(createError<json_structural_type::String>(iter));
				derailleur::skipValue(iter);
				return;
			}
			auto sizeNew = static_cast<int64_t>(roundUpToMultiple<BytesPerStep>(static_cast<uint64_t>(iter.operator->() - newPtr)) * 2ull);
			if (sizeNew > static_cast<int64_t>(parserNew.getCurrentStringBuffer().size())) {
				parserNew.getCurrentStringBuffer().resize(static_cast<uint64_t>(sizeNew));
			}
			sizeNew = parseString(newPtr + 1, parserNew.getCurrentStringBuffer().data()) - parserNew.getCurrentStringBuffer().data();
			if (sizeNew > 0) {
				value.resize(static_cast<uint64_t>(sizeNew));
				std::memcpy(value.data(), parserNew.getCurrentStringBuffer().data(), static_cast<uint64_t>(sizeNew));
			}
		}
	};

	template<jsonifier::concepts::char_t value_type_new> struct parse_impl<value_type_new> {
		template<jsonifier::concepts::char_t value_type, jsonifier::concepts::is_fwd_iterator iterator_type, jsonifier::concepts::is_parser parser_type>
		inline static void op(value_type&& value, iterator_type&& iter, parser_type&&) {
			value = static_cast<value_type_new>(*static_cast<string_view_ptr>(iter.operator->() + 1));
			++iter;
		}
	};

	template<jsonifier::concepts::raw_array_t value_type_new> struct parse_impl<value_type_new> {
		template<jsonifier::concepts::raw_array_t value_type, jsonifier::concepts::is_fwd_iterator iterator_type, jsonifier::concepts::is_parser parser_type>
		inline static void op(value_type&& value, iterator_type&& iter, parser_type&& parserNew) {
			if (!derailleur::template checkForMatchClosed<json_structural_type::Array_Start>(iter)) [[unlikely]] {
				parserNew.getErrors().emplace_back(createError<json_structural_type::Array_Start>(iter));
				derailleur::skipValue(iter);
				return;
			}
			if (derailleur::template checkForMatchOpen<json_structural_type::Array_End>(iter)) [[unlikely]] {
				return;
			}
			constexpr auto n = std::size(value);

			auto valueIter = value;

			for (uint64_t x = 0; x < n; ++x) {
				using member_type = jsonifier::concepts::unwrap<decltype(*valueIter)>;
				if constexpr (jsonifier::concepts::has_excluded_keys<member_type>) {
					parse::op(*valueIter++, iter, parserNew, valueIter->jsonifierExcludedKeys);
				} else {
					parse::op(*valueIter++, iter, parserNew);
				}
				if (!derailleur::template checkForMatchOpen<json_structural_type::Comma>(iter)) [[likely]] {
					if (!derailleur::template checkForMatchClosed<json_structural_type::Array_End>(iter)) [[unlikely]] {
						parserNew.getErrors().emplace_back(createError<json_structural_type::Array_End>(iter));
						derailleur::skipValue(iter);
					}
					return;
				}
			}
		}
	};

	template<jsonifier::concepts::vector_t value_type_new> struct parse_impl<value_type_new> {
		template<jsonifier::concepts::vector_t value_type, jsonifier::concepts::is_fwd_iterator iterator_type, jsonifier::concepts::is_parser parser_type>
		inline static void op(value_type&& value, iterator_type&& iter, parser_type&& parserNew) {
			if (!derailleur::template checkForMatchClosed<json_structural_type::Array_Start>(iter)) [[unlikely]] {
				parserNew.getErrors().emplace_back(createError<json_structural_type::Array_Start>(iter));
				derailleur::skipValue(iter);
				return;
			}
			if (derailleur::template checkForMatchOpen<json_structural_type::Array_End>(iter)) [[unlikely]] {
				return;
			}
			const auto m = value.size();

			auto valueIter	  = value.begin();
			using member_type = jsonifier::concepts::unwrap<decltype(*valueIter)>;
			for (uint64_t x = 0; x < m; ++x) {
				if constexpr (jsonifier::concepts::has_excluded_keys<member_type>) {
					parse::op(*valueIter++, iter, parserNew, valueIter->jsonifierExcludedKeys);
				} else {
					parse::op(*valueIter++, iter, parserNew);
				}
				if (!derailleur::template checkForMatchOpen<json_structural_type::Comma>(iter)) [[likely]] {
					if (!derailleur::template checkForMatchClosed<json_structural_type::Array_End>(iter)) [[unlikely]] {
						parserNew.getErrors().emplace_back(createError<json_structural_type::Array_End>(iter));
						derailleur::skipValue(iter);
					}
					return;
				}
			}
			if constexpr (jsonifier::concepts::has_emplace_back<value_type_new>) {
				while (iter != iter) {
					if constexpr (jsonifier::concepts::has_excluded_keys<member_type>) {
						auto& newReference = value.emplace_back();
						parse::op(newReference, iter, parserNew, newReference.jsonifierExcludedKeys);
					} else {
						parse::op(value.emplace_back(), iter, parserNew);
					}
					if (!derailleur::template checkForMatchOpen<json_structural_type::Comma>(iter)) [[likely]] {
						if (!derailleur::template checkForMatchClosed<json_structural_type::Array_End>(iter)) [[unlikely]] {
							parserNew.getErrors().emplace_back(createError<json_structural_type::Array_End>(iter));
							derailleur::skipValue(iter);
						}
						return;
					}
				}
			}
		}
	};

	template<jsonifier::concepts::map_t value_type_new> struct parse_impl<value_type_new> {
		template<jsonifier::concepts::map_t value_type, jsonifier::concepts::is_fwd_iterator iterator_type, jsonifier::concepts::is_parser parser_type>
		inline static void op(value_type&& value, iterator_type&& iter, parser_type&& parserNew) {
			if (!derailleur::template checkForMatchClosed<json_structural_type::Object_Start>(iter)) [[unlikely]] {
				parserNew.getErrors().emplace_back(createError<json_structural_type::Object_Start>(iter));
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
						parserNew.getErrors().emplace_back(createError<json_structural_type::Comma>(iter));
						derailleur::skipToEndOfObject(iter);
						return;
					}
				}
				if (derailleur::template checkForMatchOpen<json_structural_type::Null>(iter)) [[unlikely]] {
					continue;
				}

				using member_type = typename value_type_new::mapped_type;
				if constexpr (jsonifier::concepts::string_t<typename value_type_new::key_type>) {
					parse::op(parserNew.getCurrentStringBuffer(), iter, parserNew);
					if (!derailleur::template checkForMatchClosed<json_structural_type::Colon>(iter)) [[unlikely]] {
						parserNew.getErrors().emplace_back(createError<json_structural_type::Colon>(iter));
						derailleur::skipToEndOfObject(iter);
						return;
					}
					if constexpr (jsonifier::concepts::has_excluded_keys<member_type>) {
						parse::op(value[static_cast<typename value_type_new::key_type>(parserNew.getCurrentStringBuffer())], iter, parserNew,
							value[static_cast<typename value_type_new::key_type>(parserNew.getCurrentStringBuffer())].jsonifierExcludedKeys);
					} else {
						parse::op(value[static_cast<typename value_type_new::key_type>(parserNew.getCurrentStringBuffer())], iter, parserNew);
					}

				} else {
					typename value_type_new::key_type keyNew{};
					parse::op(keyNew, iter, parserNew);
					if (!derailleur::template checkForMatchClosed<json_structural_type::Colon>(iter)) [[unlikely]] {
						parserNew.getErrors().emplace_back(createError<json_structural_type::Colon>(iter));
						derailleur::skipToEndOfObject(iter);
						return;
					}
					if constexpr (jsonifier::concepts::has_excluded_keys<member_type>) {
						parse::op(value[keyNew], iter, parserNew, value[keyNew].jsonifierExcludedKeys);
					} else {
						parse::op(value[keyNew], iter, parserNew);
					}
				}
			}
		}
	};

	template<jsonifier::concepts::jsonifier_array_t value_type_new> struct parse_impl<value_type_new> {
		template<jsonifier::concepts::jsonifier_array_t value_type, jsonifier::concepts::is_fwd_iterator iterator_type, jsonifier::concepts::is_parser parser_type>
		inline static void op(value_type&& value, iterator_type&& iter, parser_type&& parserNew) {
			if (!derailleur::template checkForMatchClosed<json_structural_type::Array_Start>(iter)) [[unlikely]] {
				parserNew.getErrors().emplace_back(createError<json_structural_type::Array_Start>(iter));
				derailleur::skipToEndOfArray(iter);
				return;
			}
			static constexpr auto size{ std::tuple_size_v<jsonifier::concepts::core_t<value_type_new>> };

			forEach<size>([&](auto I) {
				auto& newMember	  = getMember(value, get<I>(jsonifier::concepts::coreV<value_type_new>));
				using member_type = jsonifier::concepts::unwrap<decltype(newMember)>;
				if constexpr (jsonifier::concepts::has_excluded_keys<member_type>) {
					parse::op(newMember, iter, parserNew, newMember.jsonifierExcludedKeys);
				} else {
					parse::op(newMember, iter, parserNew);
				}
				if (iter == iter) {
					return;
				}
				if (!derailleur::template checkForMatchOpen<json_structural_type::Comma>(iter)) [[likely]] {
					if (!derailleur::template checkForMatchClosed<json_structural_type::Array_End>(iter)) [[unlikely]] {
						parserNew.getErrors().emplace_back(createError<json_structural_type::Array_End>(iter));
						derailleur::skipToEndOfArray(iter);
					}
					return;
				}
			});
		}
	};

	template<jsonifier::concepts::jsonifier_object_t value_type_new> struct parse_impl<value_type_new> {
		template<jsonifier::concepts::jsonifier_object_t value_type, jsonifier::concepts::is_fwd_iterator iterator_type, jsonifier::concepts::is_parser parser_type>
		inline static void op(value_type&& value, iterator_type&& iter, parser_type&& parserNew) {
			if (!derailleur::template checkForMatchClosed<json_structural_type::Object_Start>(iter)) [[unlikely]] {
				parserNew.getErrors().emplace_back(createError<json_structural_type::Object_Start>(iter));
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
					parserNew.getErrors().emplace_back(createError<json_structural_type::Comma>(iter));
					derailleur::skipToEndOfObject(iter);
					return;
				}

				auto start = iter.operator->();
				if (!derailleur::template checkForMatchClosed<json_structural_type::String>(iter)) [[unlikely]] {
					parserNew.getErrors().emplace_back(createError<json_structural_type::String>(iter));
					derailleur::skipToEndOfObject(iter);
					return;
				}
				const jsonifier::string_view_base<uint8_t> key{ start + 1, static_cast<uint64_t>(iter.operator->() - (start + 2)) };
				if (!derailleur::template checkForMatchClosed<json_structural_type::Colon>(iter)) [[unlikely]] {
					parserNew.getErrors().emplace_back(createError<json_structural_type::Colon>(iter));
					derailleur::skipToEndOfObject(iter);
					return;
				}
				static constexpr auto frozenMap = makeMap<value_type_new>();
				const auto& memberIt			= frozenMap.find(key);
				if (derailleur::template checkForMatchOpen<json_structural_type::Null>(iter)) [[unlikely]] {
					continue;
				} else if (memberIt != frozenMap.end()) {
					if constexpr (std::variant_size_v<decltype(memberIt->second)> > 0) {
						std::visit(
							[&](auto& memberPtr) {
								auto& newMember	  = getMember(value, memberPtr);
								using member_type = jsonifier::concepts::unwrap<decltype(newMember)>;
								if constexpr (jsonifier::concepts::has_excluded_keys<member_type>) {
									parse::op(newMember, iter, parserNew, newMember.jsonifierExcludedKeys);
								} else {
									parse::op(newMember, iter, parserNew);
								}
							},
							memberIt->second);
					}

				} else [[unlikely]] {
					derailleur::skipValue(iter);
				}
			}
		}

		template<jsonifier::concepts::jsonifier_object_t value_type, jsonifier::concepts::is_fwd_iterator iterator_type, jsonifier::concepts::is_parser parser_type,
			jsonifier::concepts::has_find key_type>
		inline static void op(value_type&& value, iterator_type&& iter, parser_type&& parserNew, key_type&& excludedKeys) {
			if (!derailleur::template checkForMatchClosed<json_structural_type::Object_Start>(iter)) [[unlikely]] {
				parserNew.getErrors().emplace_back(createError<json_structural_type::Object_Start>(iter));
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
					parserNew.getErrors().emplace_back(createError<json_structural_type::Comma>(iter));
					derailleur::skipToEndOfObject(iter);
					return;
				}

				auto start = iter.operator->();
				if (!derailleur::template checkForMatchClosed<json_structural_type::String>(iter)) [[unlikely]] {
					parserNew.getErrors().emplace_back(createError<json_structural_type::String>(iter));
					derailleur::skipToEndOfObject(iter);
					return;
				}
				const jsonifier::string_view_base<uint8_t> key{ start + 1, static_cast<uint64_t>(iter.operator->() - (start + 2)) };
				if (excludedKeys.find(static_cast<const typename jsonifier::concepts::unwrap<key_type>::key_type>(key)) != excludedKeys.end()) {
					derailleur::skipToNextValue(iter);
					continue;
				}
				if (!derailleur::template checkForMatchClosed<json_structural_type::Colon>(iter)) [[unlikely]] {
					parserNew.getErrors().emplace_back(createError<json_structural_type::Colon>(iter));
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
								auto& newMember	  = getMember(value, memberPtr);
								using member_type = jsonifier::concepts::unwrap<decltype(newMember)>;
								if constexpr (jsonifier::concepts::has_excluded_keys<member_type>) {
									parse::op(newMember, iter, parserNew, newMember.jsonifierExcludedKeys);
								} else {
									parse::op(newMember, iter, parserNew);
								}
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