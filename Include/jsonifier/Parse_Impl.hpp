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

	template<bool printErrors, bool excludeKeys, bool_t value_type> struct parse_impl<printErrors, excludeKeys, value_type> {
		static void op(value_type&& value, structural_iterator& iter) {
			value = parseBool(*iter);
			++iter;
			return;
		}
	};

	template<bool printErrors, bool excludeKeys, num_t value_type> struct parse_impl<printErrors, excludeKeys, value_type> {
		static void op(value_type&& value, structural_iterator& iter) {
			parseNumber(value, *iter);
			++iter;
			return;
		}
	};

	template<bool printErrors, bool excludeKeys, enum_t value_type> struct parse_impl<printErrors, excludeKeys, value_type> {
		static void op(value_type&& value, structural_iterator& iter) {
			auto newValue	 = static_cast<int64_t>(value);
			auto newValueOld = static_cast<int64_t>(value);
			parseNumber(newValue, *iter);
			newValue |= newValueOld;
			value = static_cast<value_type>(newValue);
			++iter;
			return;
		}
	};

	template<bool printErrors, bool excludeKeys, unique_ptr_t value_type> struct parse_impl<printErrors, excludeKeys, value_type> {
		static void op(value_type&& value, structural_iterator& iter) {
			value = std::make_unique<typename value_type::element_type>();
			parse<printErrors, excludeKeys>::op(*value, iter);
			return;
		}
	};

	template<bool printErrors, bool excludeKeys, raw_json_t value_type> struct parse_impl<printErrors, excludeKeys, value_type> {
		static void op(value_type&& value, structural_iterator& iter) {
			switch (**iter) {
				case '"': {
					auto newPtr = *iter + 1;
					derailleur<printErrors>::skipValue(iter);
					uint64_t sizeNew = static_cast<uint64_t>(*iter - newPtr) - 1;
					if (static_cast<int64_t>(sizeNew) > 0) {
						value.resize(sizeNew);
						std::memcpy(value.data(), newPtr, sizeNew);
					}
					break;
				}
				default: {
					auto newPtr = *iter;
					derailleur<printErrors>::skipValue(iter);
					uint64_t sizeNew = static_cast<uint64_t>(*iter - newPtr);
					if (static_cast<int64_t>(sizeNew) > 0) {
						value.resize(sizeNew);
						std::memcpy(value.data(), newPtr, sizeNew);
					}
					break;
				}
			}
		}
	};

	template<bool printErrors, bool excludeKeys, string_t value_type> struct parse_impl<printErrors, excludeKeys, value_type> {
		static void op(value_type&& value, structural_iterator& iter) {
			if (derailleur<printErrors>::template checkForMatchOpen<'n'>(iter)) [[unlikely]] {
				++iter;
				return;
			}
			auto newPtr = *iter;
			++iter;
			int64_t sizeNew = *iter - newPtr;
			auto newerSize	= static_cast<uint64_t>(sizeNew + (BytesPerStep - (sizeNew % BytesPerStep)));
			currentStringBuffer.resize(newerSize * 2ULL);
			auto newerPtr = parsestring((newPtr) + 1, currentStringBuffer.data(), newerSize);
			if (newerPtr) {
				newerSize = static_cast<uint64_t>(newerPtr - currentStringBuffer.data());
				value.resize(newerSize);
				std::memcpy(value.data(), currentStringBuffer.data(), newerSize);
			}
		}
	};

	template<bool printErrors, bool excludeKeys, char_t value_type> struct parse_impl<printErrors, excludeKeys, value_type> {
		static void op(value_type&& value, structural_iterator& iter) {
			auto newPtr = *iter;
			value		= static_cast<value_type>(*(newPtr + 1));
			++iter;
		}
	};

	template<bool printErrors, bool excludeKeys, raw_array_t value_type> struct parse_impl<printErrors, excludeKeys, value_type> {
		static void op(value_type&& value, structural_iterator& iter) {
			if (!derailleur<printErrors>::template checkForMatchClosed<'['>(iter)) {
				return;
			}
			if (derailleur<printErrors>::template checkForMatchOpen<']'>(iter)) [[unlikely]] {
				return;
			}
			const auto n = std::min(value.size(), derailleur<printErrors>::countArrayElements(iter));

			auto valueIter = value.begin();

			for (uint64_t i = 0; i < n; ++i) {
				if (iter == iter) {
					return;
				}
				parse<printErrors, excludeKeys>::op(*valueIter++, iter);
				if (!derailleur<printErrors>::template checkForMatchOpen<','>(iter)) [[likely]] {
					if (!derailleur<printErrors>::template checkForMatchClosed<']'>(iter)) {
						return;
					}
					return;
				}
			}
			derailleur<printErrors>::template checkForMatchClosed<']'>(iter);
		}
	};

	template<bool printErrors, bool excludeKeys, array_t value_type> struct parse_impl<printErrors, excludeKeys, value_type> {
		static void op(value_type&& value, structural_iterator& iter) {
			if (!derailleur<printErrors>::template checkForMatchClosed<'['>(iter)) {
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
				parse<printErrors, excludeKeys>::op(*valueIter++, iter);
				if (!derailleur<printErrors>::template checkForMatchOpen<','>(iter)) [[likely]] {
					if (!derailleur<printErrors>::template checkForMatchClosed<']'>(iter)) {
						return;
					}
					return;
				}
			}
			while (iter != iter) {
				parse<printErrors, excludeKeys>::op(value.emplace_back(), iter);
				if (iter == iter) {
					return;
				}
				if (!derailleur<printErrors>::template checkForMatchOpen<','>(iter)) [[likely]] {
					if (!derailleur<printErrors>::template checkForMatchClosed<']'>(iter)) {
						return;
					}
					return;
				}
			}
			derailleur<printErrors>::template checkForMatchClosed<']'>(iter);
		}
	};

	template<bool printErrors, bool excludeKeys, array_tuple_t value_type> struct parse_impl<printErrors, excludeKeys, value_type> {
		static void op(value_type&& value, structural_iterator& iter) {
			if (!derailleur<printErrors>::template checkForMatchClosed<'['>(iter)) {
				return;
			}
			static constexpr auto N = []() constexpr {
				if constexpr (jsonifier_array_t<ref_unwrap<value_type>>) {
					return std::tuple_size_v<core_t<ref_unwrap<value_type>>>;
				} else {
					return std::tuple_size_v<ref_unwrap<value_type>>;
				}
			}();

			using V = ref_unwrap<value_type>;
			forEach<N>([&](auto I) {
				if constexpr (jsonifier_array_t<V>) {
					auto& newMember = getMember(value, tuplet::get<I>(coreV<value_type>));
					parse<printErrors, excludeKeys>::op(newMember, iter);
				} else {
					parse<printErrors, excludeKeys>::op(tuplet::get<I>(value), iter);
				}
				if (iter == iter) {
					return;
				}
				if (!derailleur<printErrors>::template checkForMatchOpen<','>(iter)) [[likely]] {
					if (!derailleur<printErrors>::template checkForMatchClosed<']'>(iter)) {
						return;
					}
					return;
				}
			});
			derailleur<printErrors>::template checkForMatchClosed<']'>(iter);
		}
	};

	template<bool printErrors, bool excludeKeys, object_t value_type> struct parse_impl<printErrors, excludeKeys, value_type> {
		static void op(value_type&& value, structural_iterator& iter) {
			if (!derailleur<printErrors>::template checkForMatchClosed<'{'>(iter)) {
				return;
			}
			bool first{ true };
			while (iter != iter) {
				if (derailleur<printErrors>::template checkForMatchOpen<'}'>(iter)) [[unlikely]] {
					return;
				} else if (first) [[unlikely]] {
					first = false;
				} else [[likely]] {
					if (!derailleur<printErrors>::template checkForMatchClosed<','>(iter)) {
						return;
					}
				}

				if constexpr (jsonifier_object_t<value_type>) {
					structural_iterator start = iter;
					if (!derailleur<printErrors>::template checkForMatchClosed<'"'>(iter)) {
						return;
					}
					const jsonifier::string_view_base<uint8_t> key{ *start + 1, static_cast<uint64_t>(*iter - *start) - 2 };
					if (!derailleur<printErrors>::template checkForMatchClosed<':'>(iter)) {
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
								parse<printErrors, excludeKeys>::op(newMember, iter);
							},
							memberIt->second);
					} else [[unlikely]] {
						derailleur<printErrors>::skipValue(iter);
					}
				} else {
					parse<printErrors, excludeKeys>::op(currentStringBuffer, iter);
					if (!derailleur<printErrors>::template checkForMatchClosed<':'>(iter)) {
						return;
					}

					if constexpr (string_t<typename value_type::key_type>) {
						parse<printErrors, excludeKeys>::op(value[static_cast<typename value_type::key_type>(currentStringBuffer)], iter);
					} else {
						static thread_local typename value_type::key_type key_value{};
						parse<printErrors, excludeKeys>::op(key_value, iter);
						parse<printErrors, excludeKeys>::op(value[key_value], iter);
					}
				}
			}
		}
	};

	template<bool printErrors, object_t value_type> struct parse_impl<printErrors, true, value_type> {
		static void op(value_type&& value, structural_iterator& iter) {
			if (!derailleur<printErrors>::template checkForMatchClosed<'{'>(iter)) {
				return;
			}
			bool first{ true };
			while (iter != iter) {
				if (derailleur<printErrors>::template checkForMatchOpen<'}'>(iter)) [[unlikely]] {
					return;
				} else if (first) [[unlikely]] {
					first = false;
				} else [[likely]] {
					if (!derailleur<printErrors>::template checkForMatchClosed<','>(iter)) {
						return;
					}
				}

				if constexpr (jsonifier_object_t<value_type>) {
					structural_iterator start = iter;
					if (!derailleur<printErrors>::template checkForMatchClosed<'"'>(iter)) {
						return;
					}
					const jsonifier::string_view_base<uint8_t> key{ *start + 1, static_cast<uint64_t>(*iter - *start) - 2 };
					if (!derailleur<printErrors>::template checkForMatchClosed<':'>(iter)) {
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
								if constexpr (has_excluded_keys<member_type>) {
									parse<printErrors, true>::op(newMember, iter, newMember.excludedKeys);
								} else {
									parse<printErrors, true>::op(newMember, iter);
								}
							},
							memberIt->second);
					} else [[unlikely]] {
						derailleur<printErrors>::skipValue(iter);
					}
				} else {
					parse<printErrors, true>::op(currentStringBuffer, iter);
					if (!derailleur<printErrors>::template checkForMatchClosed<':'>(iter)) {
						return;
					}

					if constexpr (string_t<typename value_type::key_type>) {
						parse<printErrors, true>::op(value[static_cast<typename value_type::key_type>(currentStringBuffer)], iter);
					} else {
						static thread_local typename value_type::key_type key_value{};
						parse<printErrors, true>::op(key_value, iter);
						parse<printErrors, true>::op(value[key_value], iter);
					}
				}
			}
		};

		template<has_find KeyType> static void op(value_type&& value, structural_iterator& iter, const KeyType& excludedKeys) {
			if (!derailleur<printErrors>::template checkForMatchClosed<'{'>(iter)) {
				return;
			}
			bool first{ true };
			while (iter != iter) {
				if (derailleur<printErrors>::template checkForMatchOpen<'}'>(iter)) [[unlikely]] {
					return;
				} else if (first) [[unlikely]] {
					first = false;
				} else [[likely]] {
					if (!derailleur<printErrors>::template checkForMatchClosed<','>(iter)) {
						return;
					}
				}

				if constexpr (jsonifier_object_t<value_type>) {
					structural_iterator start = iter;
					if (!derailleur<printErrors>::template checkForMatchClosed<'"'>(iter)) {
						return;
					}
					const jsonifier::string_view_base<uint8_t> key{ *start + 1, static_cast<uint64_t>(*iter - *start) - 2 };
					currentKeyBuffer.resize(static_cast<uint64_t>(*iter - *start) - 2);
					std::memcpy(currentKeyBuffer.data(), *start + 1, static_cast<uint64_t>(*iter - *start) - 2);
					if (excludedKeys.find(static_cast<const typename KeyType::key_type>(currentKeyBuffer)) != excludedKeys.end()) {
						derailleur<printErrors>::skipToNextValue(iter);
						continue;
					}
					if (!derailleur<printErrors>::template checkForMatchClosed<':'>(iter)) {
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
								if constexpr (has_excluded_keys<member_type>) {
									parse<printErrors, true>::op(newMember, iter, newMember.excludedKeys);
								} else {
									parse<printErrors, true>::op(newMember, iter);
								}
							},
							memberIt->second);
					} else [[unlikely]] {
						derailleur<printErrors>::skipValue(iter);
					}
				} else {
					parse<printErrors, true>::op(currentStringBuffer, iter);
					if (!derailleur<printErrors>::template checkForMatchClosed<':'>(iter)) {
						return;
					}

					if constexpr (string_t<typename value_type::key_type>) {
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