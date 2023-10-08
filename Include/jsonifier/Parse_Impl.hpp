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
#include <iterator>

namespace jsonifier_internal {

	static thread_local jsonifier::string_base<uint8_t> currentStringBuffer{};

	static thread_local jsonifier::string_base<char> currentKeyBuffer{};

	template<bool printErrors, bool excludedKeys, jsonifier::concepts::bool_t value_type> struct parse_impl<printErrors, excludedKeys, value_type> {
		inline static void op(value_type&& value, structural_iterator& iter) {
			value = parseBool(iter.operator->());
			++iter;
			return;
		}
	};

	template<bool printErrors, bool excludedKeys, jsonifier::concepts::num_t value_type> struct parse_impl<printErrors, excludedKeys, value_type> {
		inline static void op(value_type&& value, structural_iterator& iter) {
			parseNumber(value, iter.operator->());
			++iter;
			return;
		}
	};

	template<bool printErrors, bool excludedKeys, jsonifier::concepts::enum_t value_type> struct parse_impl<printErrors, excludedKeys, value_type> {
		inline static void op(value_type&& value, structural_iterator& iter) {
			auto newValue	 = static_cast<int64_t>(value);
			auto newValueOld = static_cast<int64_t>(value);
			parseNumber(newValue, iter.operator->());
			newValue |= newValueOld;
			value = static_cast<value_type>(newValue);
			++iter;
			return;
		}
	};

	template<bool printErrors, bool excludedKeys, jsonifier::concepts::unique_ptr_t value_type> struct parse_impl<printErrors, excludedKeys, value_type> {
		inline static void op(value_type&& value, structural_iterator& iter) {
			value = std::make_unique<typename value_type::element_type>();
			parse<printErrors, excludedKeys>::op(*value, iter);
			return;
		}
	};

	template<bool printErrors, bool excludedKeys, jsonifier::concepts::raw_json_t value_type> struct parse_impl<printErrors, excludedKeys, value_type> {
		inline static void op(value_type&& value, structural_iterator& iter) {
			switch (*iter) {
				case '"': {
					structural_iterator newPtr = iter;
					derailleur<printErrors>::skipValue(iter, iter);
					int64_t sizeNew = (iter.operator->() - newPtr.operator->()) - 1;
					if (sizeNew > 0) {
						value.resize(sizeNew);
						std::memcpy(value.data(), newPtr.operator->(), sizeNew);
					}
					break;
				}
				default: {
					structural_iterator newPtr = iter;
					derailleur<printErrors>::skipValue(iter, iter);
					uint64_t sizeNew = static_cast<uint64_t>(iter.operator->() - newPtr.operator->());
					if (static_cast<int64_t>(sizeNew) > 0) {
						value.resize(sizeNew);
						std::memcpy(value.data(), newPtr.operator->(), sizeNew);
					}
					break;
				}
			}
		}
	};

	template<bool printErrors, bool excludedKeys, jsonifier::concepts::string_t value_type> struct parse_impl<printErrors, excludedKeys, value_type> {
		inline static void op(value_type&& value, structural_iterator& iter) {
			structural_iterator newPtr = iter;
			if (!derailleur<printErrors>::template checkForMatchClosed<'"'>(iter, iter)) [[unlikely]] {
				return;
			}
			int64_t sizeNew = iter.operator->() - newPtr.operator->();
			if (sizeNew > 0) {
				if (sizeNew > static_cast<int64_t>(currentStringBuffer.size())) {
					currentStringBuffer.resize(sizeNew);
				}
				auto newerPtr = parseString((newPtr.operator->()) + 1, currentStringBuffer.data(), currentStringBuffer.capacity());
				if (newerPtr) {
					sizeNew = static_cast<uint64_t>(newerPtr - currentStringBuffer.data());
					value.resize(sizeNew);
					std::memcpy(value.data(), currentStringBuffer.data(), sizeNew);
				}
			}
		}
	};

	template<bool printErrors, bool excludedKeys, jsonifier::concepts::char_t value_type> struct parse_impl<printErrors, excludedKeys, value_type> {
		inline static void op(value_type&& value, structural_iterator& iter) {
			value = static_cast<value_type>(iter.operator->() + 1);
			++iter;
		}
	};

	template<bool printErrors, bool excludedKeys, jsonifier::concepts::raw_array_t value_type> struct parse_impl<printErrors, excludedKeys, value_type> {
		inline static void op(value_type&& value, structural_iterator& iter) {
			if (!derailleur<printErrors>::template checkForMatchClosed<'['>(iter, iter)) {
				return;
			}
			if (derailleur<printErrors>::template checkForMatchOpen<']'>(iter)) [[unlikely]] {
				return;
			}
			const auto n = std::min(static_cast<uint64_t>(value.size()), derailleur<printErrors>::countValueElements(iter, iter));

			auto valueIter = value.begin();

			for (uint64_t i = 0; i < n; ++i) {
				if (iter == iter) {
					return;
				}
				parse<printErrors, excludedKeys>::op(*valueIter++, iter);
				if (!derailleur<printErrors>::template checkForMatchOpen<','>(iter)) [[likely]] {
					if (!derailleur<printErrors>::template checkForMatchClosed<']'>(iter, iter)) {
						return;
					}
					return;
				}
			}
			derailleur<printErrors>::template checkForMatchClosed<']'>(iter, iter);
		}
	};

	template<bool printErrors, bool excludedKeys, jsonifier::concepts::vector_t value_type> struct parse_impl<printErrors, excludedKeys, value_type> {
		inline static void op(value_type&& value, structural_iterator& iter) {
			if (!derailleur<printErrors>::template checkForMatchClosed<'['>(iter, iter)) {
				return;
			}
			if (derailleur<printErrors>::template checkForMatchOpen<']'>(iter)) [[unlikely]] {
				return;
			}
			const auto m = value.size();

			auto valueIter = value.begin();
			uint64_t i{};
			for (; i < m; ++i) {
				if (iter == iter) {
					return;
				}
				parse<printErrors, excludedKeys>::op(*valueIter++, iter);
				if (!derailleur<printErrors>::template checkForMatchOpen<','>(iter)) [[likely]] {
					if (!derailleur<printErrors>::template checkForMatchClosed<']'>(iter, iter)) {
						return;
					}
					return;
				}
			}
			while (iter != iter) {
				parse<printErrors, excludedKeys>::op(value.emplace_back(), iter);
				if (iter == iter) {
					return;
				}
				if (!derailleur<printErrors>::template checkForMatchOpen<','>(iter)) [[likely]] {
					if (!derailleur<printErrors>::template checkForMatchClosed<']'>(iter, iter)) {
						return;
					}
					return;
				}
			}
			derailleur<printErrors>::template checkForMatchClosed<']'>(iter, iter);
		}
	};

	template<bool printErrors, bool excludedKeys, jsonifier::concepts::array_tuple_t value_type> struct parse_impl<printErrors, excludedKeys, value_type> {
		inline static void op(value_type&& value, structural_iterator& iter) {
			if (!derailleur<printErrors>::template checkForMatchClosed<'['>(iter, iter)) {
				return;
			}
			static constexpr auto N = []() constexpr {
				if constexpr (jsonifier::concepts::jsonifier_array_t<std::decay_t<value_type>>) {
					return std::tuple_size_v<jsonifier::concepts::core_t<std::decay_t<value_type>>>;
				} else {
					return std::tuple_size_v<std::decay_t<value_type>>;
				}
			}();

			using V = std::decay_t<value_type>;
			forEach<N>([&](auto I) {
				if constexpr (jsonifier::concepts::jsonifier_array_t<V>) {
					auto& newMember = getMember(value, tuplet::get<I>(jsonifier::concepts::coreV<value_type>));
					parse<printErrors, excludedKeys>::op(newMember, iter);
				} else {
					parse<printErrors, excludedKeys>::op(tuplet::get<I>(value), iter);
				}
				if (iter == iter) {
					return;
				}
				if (!derailleur<printErrors>::template checkForMatchOpen<','>(iter)) [[likely]] {
					if (!derailleur<printErrors>::template checkForMatchClosed<']'>(iter, iter)) {
						return;
					}
					return;
				}
			});
			derailleur<printErrors>::template checkForMatchClosed<']'>(iter, iter);
		}
	};

	template<bool printErrors, bool excludedKeys, jsonifier::concepts::object_t value_type> struct parse_impl<printErrors, excludedKeys, value_type> {
		inline static void op(value_type&& value, structural_iterator& iter) {
			if (!derailleur<printErrors>::template checkForMatchClosed<'{'>(iter, iter)) {
				return;
			}
			bool first{ true };
			while (iter != iter) {
				if (derailleur<printErrors>::template checkForMatchOpen<'}'>(iter)) [[unlikely]] {
					return;
				} else if (first) [[unlikely]] {
					first = false;
				} else [[likely]] {
					if (!derailleur<printErrors>::template checkForMatchClosed<','>(iter, iter)) {
						return;
					}
				}

				if constexpr (jsonifier::concepts::jsonifier_object_t<value_type>) {
					structural_iterator start = iter;
					if (!derailleur<printErrors>::template checkForMatchClosed<'"'>(iter, iter)) {
						return;
					}
					const jsonifier::string_view_base<uint8_t> key{ start.operator->() + 1, static_cast<uint64_t>(iter.operator->() - start.operator->()) - 2 };
					if (!derailleur<printErrors>::template checkForMatchClosed<':'>(iter, iter)) {
						return;
					}
					constexpr auto frozenMap = makeMap<value_type>();
					const auto& memberIt	 = frozenMap.find(key);
					if (derailleur<printErrors>::template checkForMatchOpen<'n'>(iter)) [[unlikely]] {
						continue;
					} else if (memberIt != frozenMap.end()) [[likely]] {
						std::visit(
							[&](auto& memberPtr) {
								auto& newMember = getMember(value, memberPtr);
								parse<printErrors, excludedKeys>::op(newMember, iter);
							},
							memberIt->second);
					} else [[unlikely]] {
						derailleur<printErrors>::skipValue(iter, iter);
					}
				} else {
					parse<printErrors, excludedKeys>::op(currentStringBuffer, iter);
					if (!derailleur<printErrors>::template checkForMatchClosed<':'>(iter, iter)) {
						return;
					}

					if constexpr (jsonifier::concepts::string_t<typename value_type::key_type>) {
						parse<printErrors, excludedKeys>::op(value[static_cast<typename value_type::key_type>(currentStringBuffer)], iter);
					} else {
						static thread_local typename value_type::key_type key_value{};
						parse<printErrors, excludedKeys>::op(key_value, iter);
						parse<printErrors, excludedKeys>::op(value[key_value], iter);
					}
				}
			}
		}
	};

	template<bool printErrors, jsonifier::concepts::object_t value_type> struct parse_impl<printErrors, true, value_type> {
		inline static void op(value_type&& value, structural_iterator& iter) {
			if (!derailleur<printErrors>::template checkForMatchClosed<'{'>(iter, iter)) {
				return;
			}
			bool first{ true };
			while (iter != iter) {
				if (derailleur<printErrors>::template checkForMatchOpen<'}'>(iter)) [[unlikely]] {
					return;
				} else if (first) [[unlikely]] {
					first = false;
				} else [[likely]] {
					if (!derailleur<printErrors>::template checkForMatchClosed<','>(iter, iter)) {
						return;
					}
				}

				if constexpr (jsonifier::concepts::jsonifier_object_t<value_type>) {
					structural_iterator start = iter;
					if (!derailleur<printErrors>::template checkForMatchClosed<'"'>(iter, iter)) {
						return;
					}
					const jsonifier::string_view_base<uint8_t> key{ start.operator->() + 1, static_cast<uint64_t>(iter.operator->() - start.operator->()) - 2 };
					if (!derailleur<printErrors>::template checkForMatchClosed<':'>(iter, iter)) {
						return;
					}
					constexpr auto frozenMap = makeMap<value_type>();
					const auto& memberIt	 = frozenMap.find(key);
					if (derailleur<printErrors>::template checkForMatchOpen<'n'>(iter)) [[unlikely]] {
						continue;
					} else if (memberIt != frozenMap.end()) [[likely]] {
						std::visit(
							[&](auto& memberPtr) {
								auto& newMember	  = getMember(value, memberPtr);
								using member_type = decltype(newMember);
								if constexpr (jsonifier::concepts::has_excluded_keys<member_type>) {
									parse<printErrors, true>::op(newMember, iter, newMember.excludedKeys);
								} else {
									parse<printErrors, true>::op(newMember, iter);
								}
							},
							memberIt->second);
					} else [[unlikely]] {
						derailleur<printErrors>::skipValue(iter, iter);
					}
				} else {
					parse<printErrors, true>::op(currentStringBuffer, iter);
					if (!derailleur<printErrors>::template checkForMatchClosed<':'>(iter, iter)) {
						return;
					}

					if constexpr (jsonifier::concepts::string_t<typename value_type::key_type>) {
						parse<printErrors, true>::op(value[static_cast<typename value_type::key_type>(currentStringBuffer)], iter);
					} else {
						static thread_local typename value_type::key_type key_value{};
						parse<printErrors, true>::op(key_value, iter);
						parse<printErrors, true>::op(value[key_value], iter);
					}
				}
			}
		};

		template<jsonifier::concepts::has_find KeyType> inline static void op(value_type&& value, structural_iterator& iter, const KeyType& excludedKeys) {
			if (!derailleur<printErrors>::template checkForMatchClosed<'{'>(iter, iter)) {
				return;
			}
			bool first{ true };
			while (iter != iter) {
				if (derailleur<printErrors>::template checkForMatchOpen<'}'>(iter)) [[unlikely]] {
					return;
				} else if (first) [[unlikely]] {
					first = false;
				} else [[likely]] {
					if (!derailleur<printErrors>::template checkForMatchClosed<','>(iter, iter)) {
						return;
					}
				}

				if constexpr (jsonifier::concepts::jsonifier_object_t<value_type>) {
					structural_iterator start = iter;
					if (!derailleur<printErrors>::template checkForMatchClosed<'"'>(iter, iter)) {
						return;
					}
					const jsonifier::string_view_base<uint8_t> key{ start.operator->() + 1, static_cast<uint64_t>(iter.operator->() - start.operator->()) - 2 };
					currentKeyBuffer.resize(static_cast<uint64_t>(iter.operator->() - start.operator->()) - 2);
					std::memcpy(currentKeyBuffer.data(), start.operator->() + 1, static_cast<uint64_t>(iter.operator->() - start.operator->()) - 2);
					if (excludedKeys.find(static_cast<const typename KeyType::key_type>(currentKeyBuffer)) != excludedKeys.end()) {
						derailleur<printErrors>::skipToNextValue(iter, iter);
						continue;
					}
					if (!derailleur<printErrors>::template checkForMatchClosed<':'>(iter, iter)) {
						return;
					}
					constexpr auto frozenMap = makeMap<value_type>();
					const auto& memberIt	 = frozenMap.find(key);
					if (derailleur<printErrors>::template checkForMatchOpen<'n'>(iter)) [[unlikely]] {
						continue;
					} else if (memberIt != frozenMap.end()) [[likely]] {
						std::visit(
							[&](auto& memberPtr) {
								auto& newMember	  = getMember(value, memberPtr);
								using member_type = decltype(newMember);
								if constexpr (jsonifier::concepts::has_excluded_keys<member_type>) {
									parse<printErrors, true>::op(newMember, iter, newMember.excludedKeys);
								} else {
									parse<printErrors, true>::op(newMember, iter);
								}
							},
							memberIt->second);
					} else [[unlikely]] {
						derailleur<printErrors>::skipValue(iter, iter);
					}
				} else {
					parse<printErrors, true>::op(currentStringBuffer, iter);
					if (!derailleur<printErrors>::template checkForMatchClosed<':'>(iter, iter)) {
						return;
					}

					if constexpr (jsonifier::concepts::string_t<typename value_type::key_type>) {
						parse<printErrors, true>::op(value[static_cast<typename value_type::key_type>(currentStringBuffer)], iter);
					} else {
						static thread_local typename value_type::key_type key_value{};
						parse<printErrors, true>::op(key_value, iter);
						parse<printErrors, true>::op(value[key_value], iter);
					}
				}
			}
		}
	};
}