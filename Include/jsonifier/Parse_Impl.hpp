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
#include <jsonifier/Base02.hpp>
#include <memory>

namespace jsonifier_internal {

	template<bool excludeKeys, jsonifier::concepts::bool_t value_type> struct parse_impl<excludeKeys, value_type> {
		jsonifier_inline static void op(value_type&& value, structural_iterator& iter, structural_iterator& end, parser& parserNew) {
			auto newPtr = iter.operator->();
			if (!derailleur::template checkForMatchClosed<json_structural_type::Bool>(iter)) {
				parserNew.errors.emplace_back(createError<json_structural_type::Bool>(iter));
				derailleur::skipToNextValue(iter, end);
				return;
			}
			value = parseBool(newPtr);
		}
	};

	template<bool excludeKeys, jsonifier::concepts::num_t value_type> struct parse_impl<excludeKeys, value_type> {
		jsonifier_inline static void op(value_type&& value, structural_iterator& iter, structural_iterator& end, parser& parserNew) {
			auto newPtr = iter.operator->();
			if (!derailleur::template checkForMatchClosed<json_structural_type::Number>(iter)) {
				parserNew.errors.emplace_back(createError<json_structural_type::Number>(iter));
				derailleur::skipToNextValue(iter, end);
				return;
			}
			parseNumber(value, newPtr);
		}
	};

	template<bool excludeKeys, jsonifier::concepts::enum_t value_type> struct parse_impl<excludeKeys, value_type> {
		jsonifier_inline static void op(value_type&& value, structural_iterator& iter, structural_iterator& end, parser& parserNew) {
			uint64_t newValue{};
			auto newValueOld = static_cast<int64_t>(value);
			parse<excludeKeys>::op(newValue, iter, end, parserNew);
			newValue |= newValueOld;
			value = static_cast<value_type>(newValue);
		}
	};

	template<bool excludeKeys, jsonifier::concepts::unique_ptr_t value_type> struct parse_impl<excludeKeys, value_type> {
		jsonifier_inline static void op(value_type&& value, structural_iterator& iter, structural_iterator& end, parser& parserNew) {
			value = std::make_unique<typename value_type::element_type>();
			parse<excludeKeys>::op(*value, iter, end, parserNew);
		}
	};

	template<bool excludeKeys, jsonifier::concepts::raw_json_t value_type> struct parse_impl<excludeKeys, value_type> {
		jsonifier_inline static void op(value_type&& value, structural_iterator& iter, structural_iterator& end, parser&) {
			auto newPtr = iter.operator->();
			switch (*iter) {
				case 0x22u: {
					derailleur::skipToNextValue(iter, end);
					int64_t sizeNew = (iter.operator->() - newPtr) - 1;
					if (sizeNew > 0) {
						value.resize(static_cast<uint64_t>(sizeNew));
						std::memcpy(value.data(), newPtr, static_cast<uint64_t>(sizeNew));
					}
					break;
				}
				default: {
					derailleur::skipToNextValue(iter, end);
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

	template<bool excludeKeys, jsonifier::concepts::string_t value_type> struct parse_impl<excludeKeys, value_type> {
		jsonifier_inline static void op(value_type&& value, structural_iterator& iter, structural_iterator& end, parser& parserNew) {
			auto newPtr = iter.operator->();
			if (!derailleur::template checkForMatchClosed<json_structural_type::String>(iter)) {
				parserNew.errors.emplace_back(createError<json_structural_type::String>(iter));
				derailleur::skipToNextValue(iter, end);
				return;
			}
			auto sizeNew = iter.operator->() - newPtr;
			if (sizeNew * 2 > static_cast<int64_t>(parserNew.currentStringBuffer.size())) {
				parserNew.currentStringBuffer.resize(static_cast<uint64_t>(sizeNew) * 2);
			}
			auto newerSize = parseString(newPtr + 1, parserNew.currentStringBuffer.data()) - parserNew.currentStringBuffer.data();
			if (newerSize > 0) {
				value.resize(static_cast<uint64_t>(newerSize));
				std::memcpy(value.data(), parserNew.currentStringBuffer.data(), static_cast<uint64_t>(newerSize));
			}
		}
	};

	template<bool excludeKeys, jsonifier::concepts::char_t value_type> struct parse_impl<excludeKeys, value_type> {
		jsonifier_inline static void op(value_type&& value, structural_iterator& iter, structural_iterator& end, parser&) {
			value = *static_cast<string_view_ptr>(iter.operator->() + 1);
			++iter;
		}
	};

	template<bool excludeKeys, jsonifier::concepts::raw_array_t value_type> struct parse_impl<excludeKeys, value_type> {
		jsonifier_inline static void op(value_type&& value, structural_iterator& iter, structural_iterator& end, parser& parserNew) {
			if (!derailleur::template checkForMatchClosed<json_structural_type::Array_Start>(iter)) {
				parserNew.errors.emplace_back(createError<json_structural_type::Array_Start>(iter));
				derailleur::skipToNextValue(iter, end);
				return;
			}
			if (derailleur::template checkForMatchOpen<json_structural_type::Array_End>(iter)) [[unlikely]] {
				return;
			}
			const auto n = std::size(value);

			auto valueIter = value;

			for (uint64_t i = 0; i < n; ++i) {
				parse<excludeKeys>::op(*valueIter++, iter, end, parserNew);
				if (!derailleur::template checkForMatchOpen<json_structural_type::Comma>(iter)) [[likely]] {
					if (!derailleur::template checkForMatchClosed<json_structural_type::Array_End>(iter)) {
						parserNew.errors.emplace_back(createError<json_structural_type::Array_End>(iter));
						derailleur::skipToNextValue(iter, end);
						continue;
					}
					return;
				}
			}
		}
	};

	template<bool excludeKeys, jsonifier::concepts::vector_t value_type> struct parse_impl<excludeKeys, value_type> {
		jsonifier_inline static void op(value_type&& value, structural_iterator& iter, structural_iterator& end, parser& parserNew) {
			if (!derailleur::template checkForMatchClosed<json_structural_type::Array_Start>(iter)) {
				parserNew.errors.emplace_back(createError<json_structural_type::Array_Start>(iter));
				derailleur::skipToNextValue(iter, end);
				return;
			}
			if (derailleur::template checkForMatchOpen<json_structural_type::Array_End>(iter)) [[unlikely]] {
				return;
			}
			const auto m = value.size();

			auto valueIter = value.begin();
			for (uint64_t i = 0; i < m; ++i) {
				parse<excludeKeys>::op(*valueIter++, iter, end, parserNew);
				if (!derailleur::template checkForMatchOpen<json_structural_type::Comma>(iter)) [[likely]] {
					if (!derailleur::template checkForMatchClosed<json_structural_type::Array_End>(iter)) {
						parserNew.errors.emplace_back(createError<json_structural_type::Array_End>(iter));
						derailleur::skipToNextValue(iter, end);
						continue;
					}
					return;
				}
			}
			if constexpr (jsonifier::concepts::has_emplace_back<value_type>) {
				while (iter < end) {
					parse<excludeKeys>::op(value.emplace_back(), iter, end, parserNew);
					if (!derailleur::template checkForMatchOpen<json_structural_type::Comma>(iter)) [[likely]] {
						if (!derailleur::template checkForMatchClosed<json_structural_type::Array_End>(iter)) {
							parserNew.errors.emplace_back(createError<json_structural_type::Array_End>(iter));
							derailleur::skipToNextValue(iter, end);
							continue;
						}
						return;
					}
				}
			}
		}
	};

	template<bool excludeKeys, jsonifier::concepts::map_t value_type> struct parse_impl<excludeKeys, value_type> {
		jsonifier_inline static void op(value_type&& value, structural_iterator& iter, structural_iterator& end, parser& parserNew) {
			if (!derailleur::template checkForMatchClosed<json_structural_type::Object_Start>(iter)) {
				parserNew.errors.emplace_back(createError<json_structural_type::Object_Start>(iter));
				derailleur::skipToNextValue(iter, end);
				return;
			}
			bool first{ true };
			while (iter < end) {
				if (derailleur::template checkForMatchOpen<json_structural_type::Object_End>(iter)) [[unlikely]] {
					return;
				} else if (first) [[unlikely]] {
					first = false;
				} else [[likely]] {
					if (!derailleur::template checkForMatchClosed<json_structural_type::Comma>(iter)) {
						parserNew.errors.emplace_back(createError<json_structural_type::Comma>(iter));
						derailleur::skipToNextValue(iter, end);
						continue;
					}
				}

				parse<excludeKeys>::op(parserNew.currentStringBuffer, iter, end, parserNew);
				if (!derailleur::template checkForMatchClosed<json_structural_type::Colon>(iter)) {
					parserNew.errors.emplace_back(createError<json_structural_type::Colon>(iter));
					derailleur::skipToNextValue(iter, end);
					continue;
				}

				if constexpr (jsonifier::concepts::string_t<typename value_type::key_type>) {
					parse<excludeKeys>::op(value[static_cast<typename value_type::key_type>(parserNew.currentStringBuffer)], iter, end, parserNew);
				} else {
					parse<excludeKeys>::op(parserNew.currentKeyBuffer, iter, end, parserNew);
					parse<excludeKeys>::op(value[parserNew.currentKeyBuffer], iter, end, parserNew);
				}
			}
		}
	};

	template<bool excludeKeys, jsonifier::concepts::jsonifier_array_t value_type> struct parse_impl<excludeKeys, value_type> {
		jsonifier_inline static void op(value_type&& value, structural_iterator& iter, structural_iterator& end, parser& parserNew) {
			if (!derailleur::template checkForMatchClosed<json_structural_type::Array_Start>(iter)) {
				parserNew.errors.emplace_back(createError<json_structural_type::Array_Start>(iter));
				derailleur::skipToNextValue(iter, end);
				return;
			}
			static constexpr auto size{ std::tuple_size_v<jsonifier::concepts::core_t<std::unwrap_ref_decay_t<value_type>>> };

			forEach<size>([&](auto I) {
				auto& newMember = getMember(value, tuplet::get<I>(jsonifier::concepts::coreV<value_type>));
				parse<excludeKeys>::op(newMember, iter, end, parserNew);
				if (iter < end) {
					return;
				}
				if (!derailleur::template checkForMatchOpen<json_structural_type::Comma>(iter)) [[likely]] {
					if (!derailleur::template checkForMatchClosed<json_structural_type::Array_End>(iter)) {
						parserNew.errors.emplace_back(createError<json_structural_type::Array_End>(iter));
						derailleur::skipToNextValue(iter, end);
					}
					return;
				}
			});
		}
	};

	template<jsonifier::concepts::jsonifier_array_t value_type> struct parse_impl<true, value_type> {
		jsonifier_inline static void op(value_type&& value, structural_iterator& iter, structural_iterator& end, parser& parserNew) {
			if (!derailleur::template checkForMatchClosed<json_structural_type::Array_Start>(iter)) {
				parserNew.errors.emplace_back(createError<json_structural_type::Array_Start>(iter));
				derailleur::skipToNextValue(iter, end);
				return;
			}
			static constexpr auto size{ std::tuple_size_v<jsonifier::concepts::core_t<std::unwrap_ref_decay_t<value_type>>> };

			forEach<size>([&](auto I) {
				auto& newMember	  = getMember(value, tuplet::get<I>(jsonifier::concepts::coreV<value_type>));
				using member_type = std::unwrap_ref_decay_t<decltype(newMember)>;
				if constexpr (jsonifier::concepts::has_excluded_keys<member_type>) {
					parse<true>::op(newMember, iter, end, newMember.excludedKeys, parserNew);
				} else {
					parse<true>::op(newMember, iter, end, parserNew);
				}
				if (iter < end) {
					return;
				}
				if (!derailleur::template checkForMatchOpen<json_structural_type::Comma>(iter)) [[likely]] {
					if (!derailleur::template checkForMatchClosed<json_structural_type::Array_End>(iter)) {
						parserNew.errors.emplace_back(createError<json_structural_type::Array_End>(iter));
						derailleur::skipToNextValue(iter, end);
					}
					return;
				}
			});
		}
	};

	template<bool excludeKeys, jsonifier::concepts::jsonifier_object_t value_type> struct parse_impl<excludeKeys, value_type> {
		jsonifier_inline static void op(value_type&& value, structural_iterator& iter, structural_iterator& end, parser& parserNew) {
			if (!derailleur::template checkForMatchClosed<json_structural_type::Object_Start>(iter)) {
				parserNew.errors.emplace_back(createError<json_structural_type::Object_Start>(iter));
				derailleur::skipToNextValue(iter, end);
				return;
			}
			bool first{ true };
			while (iter < end) {
				if (derailleur::template checkForMatchOpen<json_structural_type::Object_End>(iter)) [[unlikely]] {
					return;
				} else if (first) [[unlikely]] {
					first = false;
				} else [[likely]] {
					if (!derailleur::template checkForMatchClosed<json_structural_type::Comma>(iter)) {
						parserNew.errors.emplace_back(createError<json_structural_type::Comma>(iter));
						derailleur::skipToNextValue(iter, end);
						continue;
					}
				}

				auto start = iter.operator->();
				if (!derailleur::template checkForMatchClosed<json_structural_type::String>(iter)) {
					parserNew.errors.emplace_back(createError<json_structural_type::String>(iter));
					derailleur::skipToNextValue(iter, end);
					continue;
				}
				const jsonifier::string_view_base<uint8_t> key{ start + 1, static_cast<uint64_t>(iter.operator->() - start) - 2 };
				if (!derailleur::template checkForMatchClosed<json_structural_type::Colon>(iter)) {
					parserNew.errors.emplace_back(createError<json_structural_type::Colon>(iter));
					derailleur::skipToNextValue(iter, end);
					continue;
				}
				static constexpr auto frozenMap = makeMap<value_type>();
				const auto& memberIt	 = frozenMap.find(key);
				if (derailleur::template checkForMatchOpen<json_structural_type::Null>(iter)) [[unlikely]] {
					continue;
				} else if (memberIt != frozenMap.end()) [[likely]] {
					std::visit(
						[&](auto& memberPtr) {
							auto& newMember = getMember(value, memberPtr);
							parse<excludeKeys>::op(newMember, iter, end, parserNew);
						},
						memberIt->second);
				} else [[unlikely]] {
					derailleur::skipToNextValue(iter, end);
				}
			}
		}
	};

	template<jsonifier::concepts::jsonifier_object_t value_type> struct parse_impl<true, value_type> {
		jsonifier_inline static void op(value_type&& value, structural_iterator& iter, structural_iterator& end, parser& parserNew) {
			if (!derailleur::template checkForMatchClosed<json_structural_type::Object_Start>(iter)) {
				parserNew.errors.emplace_back(createError<json_structural_type::Object_Start>(iter));
				derailleur::skipToNextValue(iter, end);
				return;
			}
			bool first{ true };
			while (iter < end) {
				if (derailleur::template checkForMatchOpen<json_structural_type::Object_End>(iter)) [[unlikely]] {
					return;
				} else if (first) [[unlikely]] {
					first = false;
				} else [[likely]] {
					if (!derailleur::template checkForMatchClosed<json_structural_type::Comma>(iter)) {
						parserNew.errors.emplace_back(createError<json_structural_type::Comma>(iter));
						derailleur::skipToNextValue(iter, end);
						continue;
					}
				}

				auto start = iter.operator->();
				if (!derailleur::template checkForMatchClosed<json_structural_type::String>(iter)) {
					parserNew.errors.emplace_back(createError<json_structural_type::String>(iter));
					derailleur::skipToNextValue(iter, end);
					continue;
				}
				const jsonifier::string_view_base<uint8_t> key{ start + 1, static_cast<uint64_t>(iter.operator->() - start) - 2 };
				if (!derailleur::template checkForMatchClosed<json_structural_type::Colon>(iter)) {
					parserNew.errors.emplace_back(createError<json_structural_type::Colon>(iter));
					derailleur::skipToNextValue(iter, end);
					continue;
				}
				static constexpr auto frozenMap = makeMap<value_type>();
				const auto& memberIt	 = frozenMap.find(key);
				if (derailleur::template checkForMatchOpen<json_structural_type::Null>(iter)) [[unlikely]] {
					continue;
				} else if (memberIt != frozenMap.end()) [[likely]] {
					std::visit(
						[&](auto& memberPtr) {
							auto& newMember	  = getMember(value, memberPtr);
							using member_type = std::unwrap_ref_decay_t<decltype(newMember)>;
							if constexpr (jsonifier::concepts::has_excluded_keys<member_type>) {
								parse<true>::op(newMember, iter, end, newMember.excludedKeys, parserNew);
							} else {
								parse<true>::op(newMember, iter, end, parserNew);
							}
						},
						memberIt->second);
				} else [[unlikely]] {
					derailleur::skipToNextValue(iter, end);
				}
			}
		};

		template<jsonifier::concepts::has_find KeyType>
		jsonifier_inline static void op(value_type&& value, structural_iterator& iter, structural_iterator& end, const KeyType& excludedKeys, parser& parserNew) {
			if (!derailleur::template checkForMatchClosed<json_structural_type::Object_Start>(iter)) {
				parserNew.errors.emplace_back(createError<json_structural_type::Object_Start>(iter));
				derailleur::skipToNextValue(iter, end);
				return;
			}
			bool first{ true };
			while (iter < end) {
				if (derailleur::template checkForMatchOpen<json_structural_type::Object_End>(iter)) [[unlikely]] {
					return;
				} else if (first) [[unlikely]] {
					first = false;
				} else [[likely]] {
					if (!derailleur::template checkForMatchClosed<json_structural_type::Comma>(iter)) {
						parserNew.errors.emplace_back(createError<json_structural_type::Comma>(iter));
						derailleur::skipToNextValue(iter, end);
						continue;
					}
				}

				auto start = iter.operator->();
				if (!derailleur::template checkForMatchClosed<json_structural_type::String>(iter)) {
					parserNew.errors.emplace_back(createError<json_structural_type::String>(iter));
					derailleur::skipToNextValue(iter, end);
					continue;
				}
				auto keySize = static_cast<uint64_t>(iter.operator->() - start);
				if (keySize > 2) {
					parserNew.currentKeyBuffer.resize(keySize - 2);
				}
				std::memcpy(parserNew.currentKeyBuffer.data(), start + 1, keySize - 2);
				if (excludedKeys.find(static_cast<const typename KeyType::key_type>(parserNew.currentKeyBuffer)) != excludedKeys.end()) {
					derailleur::skipToNextValue(iter, end);
					continue;
				}
				if (!derailleur::template checkForMatchClosed<json_structural_type::Colon>(iter)) {
					parserNew.errors.emplace_back(createError<json_structural_type::Colon>(iter));
					derailleur::skipToNextValue(iter, end);
					continue;
				}
				static constexpr auto frozenMap = makeMap<value_type>();
				const auto& memberIt	 = frozenMap.find(parserNew.currentKeyBuffer);
				if (derailleur::template checkForMatchOpen<json_structural_type::Null>(iter)) [[unlikely]] {
					continue;
				} else if (memberIt != frozenMap.end()) [[likely]] {
					std::visit(
						[&](auto& memberPtr) {
							auto& newMember	  = getMember(value, memberPtr);
							using member_type = std::unwrap_ref_decay_t<decltype(newMember)>;
							if constexpr (jsonifier::concepts::has_excluded_keys<member_type>) {
								parse<true>::op(newMember, iter, end, newMember.excludedKeys, parserNew);
							} else {
								parse<true>::op(newMember, iter, end, parserNew);
							}
						},
						memberIt->second);
				} else [[unlikely]] {
					derailleur::skipToNextValue(iter, end);
				}
			}
		}
	};
}