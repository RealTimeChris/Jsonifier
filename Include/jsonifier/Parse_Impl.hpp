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

	static thread_local jsonifier::string_base<uint8_t> currentStringBuffer{};

	static thread_local jsonifier::string_base<char> currentKeyBuffer{};

	template<bool excludeKeys, jsonifier::concepts::bool_t value_type> struct parse_impl<excludeKeys, value_type> {
		inline static void op(value_type&& value, structural_iterator& iter, parser& parserNew) {
			auto newPtr = iter.operator->();
			if (auto result = derailleur::template checkForMatchClosed<json_structural_type::Bool>(iter); !result) {
				parserNew.errors.emplace_back(result.error());
				return;
			}
			value = parseBool(newPtr);
		}
	};

	template<bool excludeKeys, jsonifier::concepts::num_t value_type> struct parse_impl<excludeKeys, value_type> {
		inline static void op(value_type&& value, structural_iterator& iter, parser& parserNew) {
			auto newPtr = iter.operator->();
			if (auto result = derailleur::template checkForMatchClosed<json_structural_type::Number>(iter); !result) {
				parserNew.errors.emplace_back(result.error());
				return;
			}
			parseNumber(value, newPtr);
		}
	};

	template<bool excludeKeys, jsonifier::concepts::enum_t value_type> struct parse_impl<excludeKeys, value_type> {
		inline static void op(value_type&& value, structural_iterator& iter, parser& parserNew) {
			static uint64_t newValue{};
			auto newValueOld = static_cast<int64_t>(value);
			parse<excludeKeys>::op(newValue, iter, parserNew);
			newValue |= newValueOld;
			value = static_cast<value_type>(newValue);
		}
	};

	template<bool excludeKeys, jsonifier::concepts::unique_ptr_t value_type> struct parse_impl<excludeKeys, value_type> {
		inline static void op(value_type&& value, structural_iterator& iter, parser& parserNew) {
			value = std::make_unique<typename value_type::element_type>();
			parse<excludeKeys>::op(*value, iter, parserNew);
		}
	};

	template<bool excludeKeys, jsonifier::concepts::raw_json_t value_type> struct parse_impl<excludeKeys, value_type> {
		inline static void op(value_type&& value, structural_iterator& iter, parser&) {
			auto newPtr = iter.operator->();
			switch (*iter) {
				case '"': {
					derailleur::skipValue(iter);
					int64_t sizeNew = (iter.operator->() - newPtr) - 1;
					if (sizeNew > 0) {
						value.resize(sizeNew);
						std::memcpy(value.data(), newPtr, sizeNew);
					}
					break;
				}
				default: {
					derailleur::skipValue(iter);
					int64_t sizeNew = iter.operator->() - newPtr;
					if (sizeNew > 0) {
						value.resize(sizeNew);
						std::memcpy(value.data(), newPtr, sizeNew);
					}
					break;
				}
			}
			return;
		}
	};

	template<bool excludeKeys, jsonifier::concepts::string_t value_type> struct parse_impl<excludeKeys, value_type> {
		inline static void op(value_type&& value, structural_iterator& iter, parser& parserNew) {
			auto newPtr = iter.operator->();
			if (auto result = derailleur::template checkForMatchClosed<json_structural_type::String>(iter); !result) {
				parserNew.errors.emplace_back(result.error());
				return;
			}
			auto sizeNew = iter.operator->() - newPtr;
			if (sizeNew * 2 > static_cast<int64_t>(currentStringBuffer.size())) {
				currentStringBuffer.resize(sizeNew * 2);
			}
			auto newerSize = parseString(newPtr + 1, currentStringBuffer.data()) - currentStringBuffer.data();
			if (newerSize > 0) {
				value.resize(newerSize);
				std::memcpy(value.data(), currentStringBuffer.data(), newerSize);
			}
		}
	};

	template<bool excludeKeys, jsonifier::concepts::char_t value_type> struct parse_impl<excludeKeys, value_type> {
		inline static void op(value_type&& value, structural_iterator& iter, parser&) {
			value = *static_cast<string_view_ptr>(iter.operator->() + 1);
			++iter;
		}
	};

	template<bool excludeKeys, jsonifier::concepts::raw_array_t value_type> struct parse_impl<excludeKeys, value_type> {
		inline static void op(value_type&& value, structural_iterator& iter, parser& parserNew) {
			if (auto result = derailleur::template checkForMatchClosed<json_structural_type::Array_Start>(iter); !result) {
				parserNew.errors.emplace_back(result.error());
				return;
			}
			if (derailleur::template checkForMatchOpen<json_structural_type::Array_End>(iter)) [[unlikely]] {
				return;
			}
			const auto n = std::size(value);

			auto valueIter = value;

			for (uint64_t i = 0; i < n; ++i) {
				if (iter == iter) {
					return;
				}
				parse<excludeKeys>::op(*valueIter++, iter, parserNew);
				if (!derailleur::template checkForMatchOpen<json_structural_type::Comma>(iter)) [[likely]] {
					if (auto result = derailleur::template checkForMatchClosed<json_structural_type::Array_End>(iter); !result) {
						parserNew.errors.emplace_back(result.error());
					}
					return;
				}
			}
			if (auto result = derailleur::template checkForMatchClosed<json_structural_type::Array_End>(iter); !result) {
				parserNew.errors.emplace_back(result.error());
			}
		}
	};

	template<bool excludeKeys, jsonifier::concepts::vector_t value_type> struct parse_impl<excludeKeys, value_type> {
		inline static void op(value_type&& value, structural_iterator& iter, parser& parserNew) {
			if (auto result = derailleur::template checkForMatchClosed<json_structural_type::Array_Start>(iter); !result) {
				parserNew.errors.emplace_back(result.error());
				return;
			}
			if (derailleur::template checkForMatchOpen<json_structural_type::Array_End>(iter)) [[unlikely]] {
				return;
			}
			const auto m = value.size();

			auto valueIter = value.begin();
			uint64_t i{};
			for (; i < m; ++i) {
				if (iter == iter) {
					return;
				}
				parse<excludeKeys>::op(*valueIter++, iter, parserNew);
				if (!derailleur::template checkForMatchOpen<json_structural_type::Comma>(iter)) [[likely]] {
					if (auto result = derailleur::template checkForMatchClosed<json_structural_type::Array_End>(iter); !result) {
						parserNew.errors.emplace_back(result.error());
					}
					return;
				}
			}
			if constexpr (jsonifier::concepts::has_emplace_back<value_type>) {
				while (iter != iter) {
					parse<excludeKeys>::op(value.emplace_back(), iter, parserNew);
					if (!derailleur::template checkForMatchOpen<json_structural_type::Comma>(iter)) [[likely]] {
						if (auto result = derailleur::template checkForMatchClosed<json_structural_type::Array_End>(iter); !result) {
							parserNew.errors.emplace_back(result.error());
						}
						return;
					}
				}
			}
			if (auto result = derailleur::template checkForMatchClosed<json_structural_type::Array_End>(iter); !result) {
				parserNew.errors.emplace_back(result.error());
			}
		}
	};

	template<bool excludeKeys, jsonifier::concepts::map_t value_type> struct parse_impl<excludeKeys, value_type> {
		inline static void op(value_type&& value, structural_iterator& iter, parser& parserNew) {
			if (auto result = derailleur::template checkForMatchClosed<json_structural_type::Object_Start>(iter); !result) {
				parserNew.errors.emplace_back(result.error());
				return;
			}
			bool first{ true };
			while (iter != iter) {
				if (derailleur::template checkForMatchOpen<json_structural_type::Object_End>(iter)) [[unlikely]] {
					return;
				} else if (first) [[unlikely]] {
					first = false;
				} else [[likely]] {
					if (auto result = derailleur::template checkForMatchClosed<json_structural_type::Comma>(iter); !result) {
						parserNew.errors.emplace_back(result.error());
						return;
					}
				}

				parse<excludeKeys>::op(currentStringBuffer, iter, parserNew);
				if (auto result = derailleur::template checkForMatchClosed<json_structural_type::Colon>(iter); !result) {
					parserNew.errors.emplace_back(result.error());
					return;
				}

				if constexpr (jsonifier::concepts::string_t<typename value_type::key_type>) {
					parse<excludeKeys>::op(value[static_cast<typename value_type::key_type>(currentStringBuffer)], iter, parserNew);
				} else {
					static thread_local typename value_type::key_type key_value{};
					parse<excludeKeys>::op(key_value, iter, parserNew);
					parse<excludeKeys>::op(value[key_value], iter, parserNew);
				}
			}
			if (auto result = derailleur::template checkForMatchClosed<json_structural_type::Object_End>(iter); !result) {
				parserNew.errors.emplace_back(result.error());
			}
		}
	};

	template<bool excludeKeys, jsonifier::concepts::jsonifier_array_t value_type> struct parse_impl<excludeKeys, value_type> {
		inline static void op(value_type&& value, structural_iterator& iter, parser& parserNew) {
			if (auto result = derailleur::template checkForMatchClosed<json_structural_type::Array_Start>(iter); !result) {
				parserNew.errors.emplace_back(result.error());
				return;
			}
			static constexpr auto N{ std::tuple_size_v<jsonifier::concepts::core_t<std::unwrap_ref_decay_t<value_type>>> };

			forEach<N>([&](auto I) {
				auto& newMember = getMember(value, tuplet::get<I>(jsonifier::concepts::coreV<value_type>));
				parse<excludeKeys>::op(newMember, iter, parserNew);
				if (iter == iter) {
					return;
				}
				if (!derailleur::template checkForMatchOpen<json_structural_type::Comma>(iter)) [[likely]] {
					if (auto result = derailleur::template checkForMatchClosed<json_structural_type::Array_End>(iter); !result) {
						parserNew.errors.emplace_back(result.error());
					}
					return;
				}
			});
			if (auto result = derailleur::template checkForMatchClosed<json_structural_type::Array_End>(iter); !result) {
				parserNew.errors.emplace_back(result.error());
			}
		}
	};

	template<jsonifier::concepts::jsonifier_array_t value_type> struct parse_impl<true, value_type> {
		inline static void op(value_type&& value, structural_iterator& iter, parser& parserNew) {
			if (auto result = derailleur::template checkForMatchClosed<json_structural_type::Array_Start>(iter); !result) {
				parserNew.errors.emplace_back(result.error());
				return;
			}
			static constexpr auto N{ std::tuple_size_v<jsonifier::concepts::core_t<std::unwrap_ref_decay_t<value_type>>> };

			forEach<N>([&](auto I) {
				auto& newMember	  = getMember(value, tuplet::get<I>(jsonifier::concepts::coreV<value_type>));
				using member_type = decltype(newMember);
				if constexpr (jsonifier::concepts::has_excluded_keys<member_type>) {
					parse<true>::op(newMember, iter, newMember.excludedKeys, parserNew);
				} else {
					parse<true>::op(newMember, iter, parserNew);
				}
				if (iter == iter) {
					return;
				}
				if (!derailleur::template checkForMatchOpen<json_structural_type::Comma>(iter)) [[likely]] {
					if (auto result = derailleur::template checkForMatchClosed<json_structural_type::Array_End>(iter); !result) {
						parserNew.errors.emplace_back(result.error());
					}
					return;
				}
			});
			if (auto result = derailleur::template checkForMatchClosed<json_structural_type::Array_End>(iter); !result) {
				parserNew.errors.emplace_back(result.error());
			}
		}
	};

	template<bool excludeKeys, jsonifier::concepts::jsonifier_object_t value_type> struct parse_impl<excludeKeys, value_type> {
		inline static void op(value_type&& value, structural_iterator& iter, parser& parserNew) {
			if (auto result = derailleur::template checkForMatchClosed<json_structural_type::Object_Start>(iter); !result) {
				parserNew.errors.emplace_back(result.error());
				return;
			}
			bool first{ true };
			while (iter != iter) {
				if (derailleur::template checkForMatchOpen<json_structural_type::Object_End>(iter)) [[unlikely]] {
					return;
				} else if (first) [[unlikely]] {
					first = false;
				} else [[likely]] {
					if (auto result = derailleur::template checkForMatchClosed<json_structural_type::Comma>(iter); !result) {
						parserNew.errors.emplace_back(result.error());
						return;
					}
				}

				auto start = iter.operator->();
				if (auto result = derailleur::template checkForMatchClosed<json_structural_type::String>(iter); !result) {
					parserNew.errors.emplace_back(result.error());
					return;
				}
				const jsonifier::string_view_base<uint8_t> key{ start + 1, static_cast<uint64_t>(iter.operator->() - start) - 2 };
				if (auto result = derailleur::template checkForMatchClosed<json_structural_type::Colon>(iter); !result) {
					parserNew.errors.emplace_back(result.error());
					return;
				}
				constexpr auto frozenMap = makeMap<value_type>();
				const auto& memberIt	 = frozenMap.find(key);
				if (derailleur::template checkForMatchOpen<json_structural_type::Null>(iter)) [[unlikely]] {
					continue;
				} else if (memberIt != frozenMap.end()) [[likely]] {
					std::visit(
						[&](auto& memberPtr) {
							auto& newMember = getMember(value, memberPtr);
							parse<excludeKeys>::op(newMember, iter, parserNew);
						},
						memberIt->second);
				} else [[unlikely]] {
					derailleur::skipValue(iter);
				}
			}
			if (auto result = derailleur::template checkForMatchClosed<json_structural_type::Object_End>(iter); !result) {
				parserNew.errors.emplace_back(result.error());
			}
		}
	};

	template<jsonifier::concepts::jsonifier_object_t value_type> struct parse_impl<true, value_type> {
		inline static void op(value_type&& value, structural_iterator& iter, parser& parserNew) {
			if (auto result = derailleur::template checkForMatchClosed<json_structural_type::Object_Start>(iter); !result) {
				parserNew.errors.emplace_back(result.error());
				return;
			}
			bool first{ true };
			while (iter != iter) {
				if (derailleur::template checkForMatchOpen<json_structural_type::Object_End>(iter)) [[unlikely]] {
					return;
				} else if (first) [[unlikely]] {
					first = false;
				} else [[likely]] {
					if (auto result = derailleur::template checkForMatchClosed<json_structural_type::Comma>(iter); !result) {
						parserNew.errors.emplace_back(result.error());
						return;
					}
				}

				auto start = iter.operator->();
				if (auto result = derailleur::template checkForMatchClosed<json_structural_type::String>(iter); !result) {
					parserNew.errors.emplace_back(result.error());
					return;
				}
				const jsonifier::string_view_base<uint8_t> key{ start + 1, static_cast<uint64_t>(iter.operator->() - start) - 2 };
				if (auto result = derailleur::template checkForMatchClosed<json_structural_type::Colon>(iter); !result) {
					parserNew.errors.emplace_back(result.error());
					return;
				}
				constexpr auto frozenMap = makeMap<value_type>();
				const auto& memberIt	 = frozenMap.find(key);
				if (derailleur::template checkForMatchOpen<json_structural_type::Null>(iter)) [[unlikely]] {
					continue;
				} else if (memberIt != frozenMap.end()) [[likely]] {
					std::visit(
						[&](auto& memberPtr) {
							auto& newMember	  = getMember(value, memberPtr);
							using member_type = decltype(newMember);
							if constexpr (jsonifier::concepts::has_excluded_keys<member_type>) {
								parse<true>::op(newMember, iter, newMember.excludedKeys, parserNew);
							} else {
								parse<true>::op(newMember, iter, parserNew);
							}
						},
						memberIt->second);
				} else [[unlikely]] {
					derailleur::skipValue(iter);
				}
			}
			if (auto result = derailleur::template checkForMatchClosed<json_structural_type::Object_End>(iter); !result) {
				parserNew.errors.emplace_back(result.error());
			}
		};

		template<jsonifier::concepts::has_find KeyType> inline static void op(value_type&& value, structural_iterator& iter, const KeyType& excludedKeys, parser& parserNew) {
			if (auto result = derailleur::template checkForMatchClosed<json_structural_type::Object_Start>(iter); !result) {
				parserNew.errors.emplace_back(result.error());
				return;
			}
			bool first{ true };
			while (iter != iter) {
				if (derailleur::template checkForMatchOpen<json_structural_type::Object_End>(iter)) [[unlikely]] {
					return;
				} else if (first) [[unlikely]] {
					first = false;
				} else [[likely]] {
					if (auto result = derailleur::template checkForMatchClosed<json_structural_type::Comma>(iter); !result) {
						parserNew.errors.emplace_back(result.error());
						return;
					}
				}

				auto start = iter.operator->();
				if (auto result = derailleur::template checkForMatchClosed<json_structural_type::String>(iter); !result) {
					parserNew.errors.emplace_back(result.error());
					return;
				}
				auto keySize = static_cast<uint64_t>(iter.operator->() - start);
				if (keySize > 2) {
					currentKeyBuffer.resize(keySize - 2);
				}
				std::memcpy(currentKeyBuffer.data(), start + 1, keySize - 2);
				if (excludedKeys.find(static_cast<const typename KeyType::key_type>(currentKeyBuffer)) != excludedKeys.end()) {
					derailleur::skipToNextValue(iter);
					continue;
				}
				if (auto result = derailleur::template checkForMatchClosed<json_structural_type::Colon>(iter); !result) {
					parserNew.errors.emplace_back(result.error());
					return;
				}
				constexpr auto frozenMap = makeMap<value_type>();
				const auto& memberIt	 = frozenMap.find(currentKeyBuffer);
				if (derailleur::template checkForMatchOpen<json_structural_type::Null>(iter)) [[unlikely]] {
					continue;
				} else if (memberIt != frozenMap.end()) [[likely]] {
					std::visit(
						[&](auto& memberPtr) {
							auto& newMember	  = getMember(value, memberPtr);
							using member_type = decltype(newMember);
							if constexpr (jsonifier::concepts::has_excluded_keys<member_type>) {
								parse<true>::op(newMember, iter, newMember.excludedKeys, parserNew);
							} else {
								parse<true>::op(newMember, iter, parserNew);
							}
						},
						memberIt->second);
				} else [[unlikely]] {
					derailleur::skipValue(iter);
				}
			}
			if (auto result = derailleur::template checkForMatchClosed<json_structural_type::Object_End>(iter); !result) {
				parserNew.errors.emplace_back(result.error());
			}
		}
	};
}