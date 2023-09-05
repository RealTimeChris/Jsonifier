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
/// https://github.com/RealTimeChris/Jsonifier
/// Feb 20, 2023
#pragma once

#include <jsonifier/NumberUtils.hpp>
#include <jsonifier/StringUtils.hpp>
#include <jsonifier/Derailleur.hpp>
#include <jsonifier/Parser.hpp>
#include <jsonifier/Base.hpp>
#include <iterator>

namespace JsonifierInternal {

	static inline Jsonifier::StringBase<uint8_t>& getCurrentStringBuffer() noexcept {
		static thread_local Jsonifier::StringBase<uint8_t> currentStringBuffer{};
		return currentStringBuffer;
	}

	static inline Jsonifier::StringBase<char>& getCurrentKeyBuffer() noexcept {
		static thread_local Jsonifier::StringBase<char> currentStringBuffer{};
		return currentStringBuffer;
	}

	template<bool printErrors, bool excludeKeys, BoolT ValueType> struct ParseImpl<printErrors, excludeKeys, ValueType> {
		inline static void op(ValueType&& value, StructuralIterator& iter) {
			value = parseBool(*iter);
			++iter;
			return;
		}
	};

	template<bool printErrors, bool excludeKeys, NumT ValueType> struct ParseImpl<printErrors, excludeKeys, ValueType> {
		inline static void op(ValueType&& value, StructuralIterator& iter) {
			parseNumber(value, *iter);
			++iter;
			return;
		}
	};

	template<bool printErrors, bool excludeKeys, EnumT ValueType> struct ParseImpl<printErrors, excludeKeys, ValueType> {
		inline static void op(ValueType&& value, StructuralIterator& iter) {
			auto newValue	 = static_cast<int64_t>(value);
			auto newValueOld = static_cast<int64_t>(value);
			parseNumber(newValue, *iter);
			newValue |= newValueOld;
			value = static_cast<ValueType>(newValue);
			++iter;
			return;
		}
	};

	template<bool printErrors, bool excludeKeys, UniquePtrT ValueType> struct ParseImpl<printErrors, excludeKeys, ValueType> {
		inline static void op(ValueType&& value, StructuralIterator& iter) {
			value = std::make_unique<typename ValueType::element_type>();
			Parse<printErrors, excludeKeys>::op(*value, iter);
			return;
		}
	};

	template<bool printErrors, bool excludeKeys, RawJsonT ValueType> struct ParseImpl<printErrors, excludeKeys, ValueType> {
		inline static void op(ValueType&& value, StructuralIterator& iter) {
			auto newPtr = *iter;
			Derailleur<printErrors>::skipValue(iter);
			int64_t sizeNew = static_cast<int64_t>(*iter - newPtr);
			value.resize(sizeNew);
			std::memcpy(value.data(), newPtr, sizeNew);
		}
	};

	template<bool printErrors, bool excludeKeys, StringT ValueType> struct ParseImpl<printErrors, excludeKeys, ValueType> {
		inline static void op(ValueType&& value, StructuralIterator& iter) {
			auto newPtr = *iter;
			++iter;
			int64_t sizeNew = static_cast<int64_t>(*iter - newPtr);
			auto newerSize	= sizeNew + (BytesPerStep - (sizeNew % BytesPerStep));
			getCurrentStringBuffer().resize(newerSize * 2);
			auto newerPtr = parseString((newPtr) + 1, getCurrentStringBuffer().data(), newerSize);
			if (newerPtr) {
				newerSize = newerPtr - getCurrentStringBuffer().data();
				value.resize(newerSize);
				std::memcpy(value.data(), getCurrentStringBuffer().data(), newerSize);
			}
		}
	};

	template<bool printErrors, bool excludeKeys, CharT ValueType> struct ParseImpl<printErrors, excludeKeys, ValueType> {
		inline static void op(ValueType&& value, StructuralIterator& iter) {
			auto newPtr = *iter;
			value		= static_cast<ValueType>(*(newPtr + 1));
			++iter;
		}
	};

	template<bool printErrors, bool excludeKeys, RawArrayT ValueType> struct ParseImpl<printErrors, excludeKeys, ValueType> {
		inline static void op(ValueType&& value, StructuralIterator& iter) {
			if (!Derailleur<printErrors>::template checkForMatchClosed<'['>(iter)) {
				return;
			}
			if (Derailleur<printErrors>::template checkForMatchOpen<']'>(iter)) [[unlikely]] {
				return;
			}
			const auto n = std::min(value.size(), Derailleur<printErrors>::countArrayElements(iter));

			auto valueIter = value.begin();

			for (uint64_t i = 0; i < n; ++i) {
				if (iter == iter) {
					return;
				}
				Parse<printErrors, excludeKeys>::op(*valueIter++, iter);
				if (!Derailleur<printErrors>::template checkForMatchOpen<','>(iter)) [[likely]] {
					if (!Derailleur<printErrors>::template checkForMatchClosed<']'>(iter)) {
						return;
					}
					return;
				}
			}
			Derailleur<printErrors>::template checkForMatchClosed<']'>(iter);
		}
	};

	template<bool printErrors, bool excludeKeys, ArrayT ValueType> struct ParseImpl<printErrors, excludeKeys, ValueType> {
		inline static void op(ValueType&& value, StructuralIterator& iter) {
			if (!Derailleur<printErrors>::template checkForMatchClosed<'['>(iter)) {
				return;
			}
			if (Derailleur<printErrors>::template checkForMatchOpen<']'>(iter)) [[unlikely]] {
				return;
			}
			const auto m = value.size();

			auto valueIter = value.begin();
			uint64_t i{};
			for (; i < m; ++i) {
				if (iter == iter) {
					return;
				}
				Parse<printErrors, excludeKeys>::op(*valueIter++, iter);
				if (!Derailleur<printErrors>::template checkForMatchOpen<','>(iter)) [[likely]] {
					if (!Derailleur<printErrors>::template checkForMatchClosed<']'>(iter)) {
						return;
					}
					return;
				}
			}
			while (iter != iter) {
				Parse<printErrors, excludeKeys>::op(value.emplace_back(), iter);
				if (iter == iter) {
					return;
				}
				if (!Derailleur<printErrors>::template checkForMatchOpen<','>(iter)) [[likely]] {
					if (!Derailleur<printErrors>::template checkForMatchClosed<']'>(iter)) {
						return;
					}
					return;
				}
			}
			Derailleur<printErrors>::template checkForMatchClosed<']'>(iter);
		}
	};

	template<bool printErrors, bool excludeKeys, ObjectT ValueType> struct ParseImpl<printErrors, excludeKeys, ValueType> {
		inline static void op(ValueType&& value, StructuralIterator& iter) {
			if (!Derailleur<printErrors>::template checkForMatchClosed<'{'>(iter)) {
				return;
			}
			bool first{ true };
			while (iter != iter) {
				if (Derailleur<printErrors>::template checkForMatchOpen<'}'>(iter)) [[unlikely]] {
					return;
				} else if (first) [[unlikely]] {
					first = false;
				} else [[likely]] {
					if (!Derailleur<printErrors>::template checkForMatchClosed<','>(iter)) {
						return;
					}
				}

				if constexpr (JsonifierObjectT<ValueType>) {
					StructuralIterator start = iter;
					if (!Derailleur<printErrors>::template checkForMatchClosed<'"'>(iter)) {
						return;
					}
					const Jsonifier::StringViewBase<uint8_t> key{ *start + 1, static_cast<uint64_t>(*iter - *start) - 2 };
					if (!Derailleur<printErrors>::template checkForMatchClosed<':'>(iter)) {
						return;
					}
					constexpr auto frozenMap = makeMap<ValueType>();
					const auto& memberIt	 = frozenMap.find(key);
					if (Derailleur<printErrors>::template checkForMatchOpen<'n'>(iter)) [[unlikely]] {
						continue;
					} else if (memberIt != frozenMap.end()) [[likely]] {
						std::visit(
							[&](auto& memberPtr) {
								auto& newMember = getMember(value, memberPtr);
								Parse<printErrors, true>::op(newMember, iter);
							},
							memberIt->second);
					} else [[unlikely]] {
						Derailleur<printErrors>::skipValue(iter);
					}
				} else {
					Parse<printErrors, true>::op(getCurrentStringBuffer(), iter);
					if (!Derailleur<printErrors>::template checkForMatchClosed<':'>(iter)) {
						return;
					}

					if constexpr (StringT<typename ValueType::key_type>) {
						Parse<printErrors, true>::op(value[static_cast<typename ValueType::key_type>(getCurrentStringBuffer())], iter);
					} else {
						static thread_local typename ValueType::key_type key_value{};
						Parse<printErrors, true>::op(key_value, iter);
						Parse<printErrors, true>::op(value[key_value], iter);
					}
				}
			}
		}
	};

	template<bool printErrors, ObjectT ValueType> struct ParseImpl<printErrors, true, ValueType> {
		inline static void op(ValueType&& value, StructuralIterator& iter) {
			if (!Derailleur<printErrors>::template checkForMatchClosed<'{'>(iter)) {
				return;
			}
			bool first{ true };
			while (iter != iter) {
				if (Derailleur<printErrors>::template checkForMatchOpen<'}'>(iter)) [[unlikely]] {
					return;
				} else if (first) [[unlikely]] {
					first = false;
				} else [[likely]] {
					if (!Derailleur<printErrors>::template checkForMatchClosed<','>(iter)) {
						return;
					}
				}

				if constexpr (JsonifierObjectT<ValueType>) {
					StructuralIterator start = iter;
					if (!Derailleur<printErrors>::template checkForMatchClosed<'"'>(iter)) {
						return;
					}
					const Jsonifier::StringViewBase<uint8_t> key{ *start + 1, static_cast<uint64_t>(*iter - *start) - 2 };
					if (!Derailleur<printErrors>::template checkForMatchClosed<':'>(iter)) {
						return;
					}
					constexpr auto frozenMap = makeMap<ValueType>();
					const auto& memberIt	 = frozenMap.find(key);
					if (Derailleur<printErrors>::template checkForMatchOpen<'n'>(iter)) [[unlikely]] {
						continue;
					} else if (memberIt != frozenMap.end()) [[likely]] {
						std::visit(
							[&](auto& memberPtr) {
								auto& newMember		= getMember(value, memberPtr);
								using newMemberType = decltype(newMember);
								if constexpr (HasExcludedKeys<newMemberType>) {
									Parse<printErrors, true>::op(newMember, iter, newMember.excludedKeys);
								} else {
									Parse<printErrors, true>::op(newMember, iter);
								}
							},
							memberIt->second);
					} else [[unlikely]] {
						Derailleur<printErrors>::skipValue(iter);
					}
				} else {
					Parse<printErrors, true>::op(getCurrentStringBuffer(), iter);
					if (!Derailleur<printErrors>::template checkForMatchClosed<':'>(iter)) {
						return;
					}

					if constexpr (StringT<typename ValueType::key_type>) {
						Parse<printErrors, true>::op(value[static_cast<typename ValueType::key_type>(getCurrentStringBuffer())], iter);
					} else {
						static thread_local typename ValueType::key_type key_value{};
						Parse<printErrors, true>::op(key_value, iter);
						Parse<printErrors, true>::op(value[key_value], iter);
					}
				}
			}
		};

		template<HasFind KeyType> inline static void op(ValueType&& value, StructuralIterator& iter, const KeyType& excludedKeys) {
			if (!Derailleur<printErrors>::template checkForMatchClosed<'{'>(iter)) {
				return;
			}
			bool first{ true };
			while (iter != iter) {
				if (Derailleur<printErrors>::template checkForMatchOpen<'}'>(iter)) [[unlikely]] {
					return;
				} else if (first) [[unlikely]] {
					first = false;
				} else [[likely]] {
					if (!Derailleur<printErrors>::template checkForMatchClosed<','>(iter)) {
						return;
					}
				}

				if constexpr (JsonifierObjectT<ValueType>) {
					StructuralIterator start = iter;
					if (!Derailleur<printErrors>::template checkForMatchClosed<'"'>(iter)) {
						return;
					}
					const Jsonifier::StringViewBase<uint8_t> key{ *start + 1, static_cast<uint64_t>(*iter - *start) - 2 };
					if (!Derailleur<printErrors>::template checkForMatchClosed<':'>(iter)) {
						return;
					}
					getCurrentKeyBuffer().resize(static_cast<uint64_t>(*iter - *start) - 2);
					std::memcpy(getCurrentKeyBuffer().data(), *start + 1, static_cast<uint64_t>(*iter - *start) - 2);
					if (excludedKeys.find(static_cast<typename KeyType::key_type>(getCurrentKeyBuffer())) != excludedKeys.end()) {
						Derailleur<printErrors>::skipValue(iter);
						continue;
					}
					constexpr auto frozenMap = makeMap<ValueType>();
					const auto& memberIt	 = frozenMap.find(key);
					if (Derailleur<printErrors>::template checkForMatchOpen<'n'>(iter)) [[unlikely]] {
						continue;
					} else if (memberIt != frozenMap.end()) [[likely]] {
						std::visit(
							[&](auto& memberPtr) {
								auto& newMember		= getMember(value, memberPtr);
								using newMemberType = decltype(newMember);
								if constexpr (HasExcludedKeys<newMemberType>) {
									Parse<printErrors, true>::op(newMember, iter, newMember.excludedKeys);
								} else {
									Parse<printErrors, true>::op(newMember, iter);
								}
							},
							memberIt->second);
					} else [[unlikely]] {
						Derailleur<printErrors>::skipValue(iter);
					}
				} else {
					Parse<printErrors, true>::op(getCurrentStringBuffer(), iter);
					if (!Derailleur<printErrors>::template checkForMatchClosed<':'>(iter)) {
						return;
					}

					if constexpr (StringT<typename ValueType::key_type>) {
						Parse<printErrors, true>::op(value[static_cast<typename ValueType::key_type>(getCurrentStringBuffer())], iter);
					} else {
						static thread_local typename ValueType::key_type key_value{};
						Parse<printErrors, true>::op(key_value, iter);
						Parse<printErrors, true>::op(value[key_value], iter);
					}
				}
			}
		};
	};
}