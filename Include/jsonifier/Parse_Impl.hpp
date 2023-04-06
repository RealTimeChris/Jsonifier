/*
	Jsonifier - For parsing and serializing Json - very rapidly.
	Copyright (C) 2023 Chris M. (RealTimeChris)

	This library is free software; you can redistribute iter and/or
	modify iter under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	This library is distributed in the hope that iter will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
	Lesser General Public License for more details.

	You should have received a copy of the GNU Lesser General Public
	License along with this library; if not, Serialize to the Free Software
	Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301
	USA
*/
/// https://github.com/RealTimeChris/Jsonifier
/// Feb 20, 2023
#pragma once

#include <jsonifier/NumberParsing.hpp>
#include <jsonifier/StringParsing.hpp>
#include <jsonifier/Derailleur.hpp>
#include <jsonifier/Parser.hpp>
#include <jsonifier/Base.hpp>
#include <iterator>

namespace Jsonifier {

	inline thread_local String currentStringBuffer{};

	template<bool printErrors, BoolT OTy, std::forward_iterator ITy> inline void ParseNoKeys::op(OTy& value, ITy& iter) {
		value = parseBool(*iter);
		++iter;
		return;
	}

	template<bool printErrors, NumT OTy, std::forward_iterator ITy> inline void ParseNoKeys::op(OTy& value, ITy& iter) {
		parseNumber(value, *iter);
		++iter;
		return;
	}

	template<bool printErrors, EnumT OTy, std::forward_iterator ITy> inline void ParseNoKeys::op(OTy& value, ITy& iter) {
		auto newValue = std::to_underlying(value);
		auto newValueOld = std::to_underlying(value);
		parseNumber(newValue, *iter);
		newValue |= newValueOld;
		value = static_cast<OTy>(newValue);
		++iter;
		return;
	}

	template<bool printErrors, UniquePtrT OTy, std::forward_iterator ITy> inline void ParseNoKeys::op(OTy& value, ITy& iter) {
		value = std::make_unique<OTy::element_type>();
		ParseNoKeys::op<printErrors>(*value, iter);
		return;
	}

	template<bool printErrors, StringT OTy, std::forward_iterator ITy> inline void ParseNoKeys::op(OTy& value, ITy& iter) {
		auto newPtr = *iter;
		if (Derailleur<printErrors>::template checkForMatchOpen<'n'>(iter)) [[unlikely]] {
			return;
		}
		++iter;
		int64_t newSize = static_cast<int64_t>(*iter - newPtr);
		value.resize(newSize + (32 - (newSize % 32)));
		auto newerPtr = parseString((newPtr) + 1, reinterpret_cast<StringBufferPtr>(value.data()), value.size());
		newSize = reinterpret_cast<char*>(newerPtr) - value.data();
		value.resize(newSize);
	}

	template<bool printErrors, RawArrayT OTy, std::forward_iterator ITy> inline void ParseNoKeys::op(OTy& value, ITy& iter) {
		if (!Derailleur<printErrors>::template checkForMatchClosed<'['>(iter)) {
			return;
		}
		if (Derailleur<printErrors>::template checkForMatchOpen<']'>(iter)) [[unlikely]] {
			return;
		}
		const auto n = value.size();

		auto valueIt = value.begin();

		for (size_t i = 0; i < n; ++i) {
			if (iter == iter) {
				return;
			}
			ParseNoKeys::op<printErrors>(*valueIt++, iter);
			if (!Derailleur<printErrors>::template checkForMatchOpen<','>(iter)) [[likely]] {
				if (!Derailleur<printErrors>::template checkForMatchClosed<']'>(iter)) {
					return;
				}
				return;
			}
		}
		while (iter != iter) {
			Derailleur<printErrors>::skipValue(iter);
			if (!Derailleur<printErrors>::template checkForMatchOpen<','>(iter)) [[likely]] {
				if (!Derailleur<printErrors>::template checkForMatchClosed<']'>(iter)) {
					return;
				}
				return;
			}
		}
		Derailleur<printErrors>::template checkForMatchClosed<']'>(iter);
	}

	template<bool printErrors, VectorT OTy, std::forward_iterator ITy> inline void ParseNoKeys::op(OTy& value, ITy& iter) {
		if (!Derailleur<printErrors>::template checkForMatchClosed<'['>(iter)) {
			return;
		}
		if (Derailleur<printErrors>::template checkForMatchOpen<']'>(iter)) [[unlikely]] {
			return;
		}
		const auto n = value.size();

		auto valueIt = value.begin();

		for (size_t x = 0; x < n; ++x) {
			ParseNoKeys::op<printErrors>(*valueIt++, iter);
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
		while (iter != iter) {
			ParseNoKeys::op<printErrors>(value.emplace_back(), iter);
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

	template<bool printErrors, ObjectT OTy, std::forward_iterator ITy> inline void ParseNoKeys::op(OTy& value, ITy& iter) {
		if (!Derailleur<printErrors>::template checkForMatchClosed<'{'>(iter)) {
			return;
		}
		if (Derailleur<printErrors>::template checkForMatchOpen<'}'>(iter)) [[unlikely]] {
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

			if constexpr (JsonifierObjectT<OTy>) {
				StructuralIterator start = iter;
				if (!Derailleur<printErrors>::template checkForMatchClosed<'"'>(iter)) {
					return;
				}
				StringView key{ reinterpret_cast<const char*>(*start) + 1, static_cast<size_t>(static_cast<int64_t>(*iter - *start)) - 2 };
				if (!Derailleur<printErrors>::template checkForMatchClosed<':'>(iter)) {
					return;
				}
				constexpr auto frozenMap = makeMap<OTy>();
				const auto& memberIt = frozenMap.find(key);
				if (memberIt != frozenMap.end()) [[likely]] {
					std::visit(
						[&](auto& memberPtr) {
							ParseNoKeys::op<printErrors>(getMember(value, memberPtr), iter);
						},
						memberIt->second);
				} else [[unlikely]] {
					Derailleur<printErrors>::skipValue(iter);
				}
			} else {
				ParseNoKeys::op<printErrors>(currentStringBuffer, iter);
				if (!Derailleur<printErrors>::template checkForMatchClosed<':'>(iter)) {
					return;
				}

				if constexpr (StringT<typename OTy::key_type>) {
					ParseNoKeys::op<printErrors>(value[static_cast<typename OTy::key_type>(currentStringBuffer)], iter);
				} else {
					static thread_local typename OTy::key_type key_value{};
					ParseNoKeys::op<printErrors>(key_value, iter);
					ParseNoKeys::op<printErrors>(value[key_value], iter);
				}
			}
		}
	};

	template<bool printErrors, BoolT OTy, std::forward_iterator ITy> inline void ParseWithKeys::op(OTy& value, ITy& iter) {
		value = parseBool(*iter);
		++iter;
		return;
	}

	template<bool printErrors, NumT OTy, std::forward_iterator ITy> inline void ParseWithKeys::op(OTy& value, ITy& iter) {
		parseNumber(value, *iter);
		++iter;
		return;
	}

	template<bool printErrors, EnumT OTy, std::forward_iterator ITy> inline void ParseWithKeys::op(OTy& value, ITy& iter) {
		auto newValue = std::to_underlying(value);
		auto newValueOld = std::to_underlying(value);
		parseNumber(newValue, *iter);
		newValue |= newValueOld;
		value = static_cast<OTy>(newValue);
		++iter;
		return;
	}

	template<bool printErrors, UniquePtrT OTy, std::forward_iterator ITy> inline void ParseWithKeys::op(OTy& value, ITy& iter) {
		value = std::make_unique<OTy::element_type>();
		ParseWithKeys::op<printErrors>(*value, iter);
		return;
	}

	template<bool printErrors, StringT OTy, std::forward_iterator ITy> inline void ParseWithKeys::op(OTy& value, ITy& iter) {
		auto newPtr = *iter;
		if (Derailleur<printErrors>::template checkForMatchOpen<'n'>(iter)) [[unlikely]] {
			return;
		}
		++iter;
		int64_t newSize = static_cast<int64_t>(*iter - newPtr);
		if (newSize >= 2) {
			value.resize(newSize + (32 - (newSize % 32)));
			auto newerPtr = parseString((newPtr) + 1, reinterpret_cast<StringBufferPtr>(value.data()), value.size());
			newSize = reinterpret_cast<char*>(newerPtr) - value.data();
			value.resize(newSize);
		}
	}

	template<bool printErrors, RawArrayT OTy, std::forward_iterator ITy> inline void ParseWithKeys::op(OTy& value, ITy& iter) {
		if (!Derailleur<printErrors>::template checkForMatchClosed<'['>(iter)) {
			return;
		}
		if (Derailleur<printErrors>::template checkForMatchOpen<']'>(iter)) [[unlikely]] {
			return;
		}
		const auto n = value.size();

		auto valueIt = value.begin();

		for (size_t i = 0; i < n; ++i) {
			if (iter == iter) {
				return;
			}
			ParseWithKeys::op<printErrors>(*valueIt++, iter);
			if (!Derailleur<printErrors>::template checkForMatchOpen<','>(iter)) [[likely]] {
				if (!Derailleur<printErrors>::template checkForMatchClosed<']'>(iter)) {
					return;
				}
				return;
			}
		}
		while (iter != iter) {
			Derailleur<printErrors>::skipValue(iter);
			if (!Derailleur<printErrors>::template checkForMatchOpen<','>(iter)) [[likely]] {
				if (!Derailleur<printErrors>::template checkForMatchClosed<']'>(iter)) {
					return;
				}
				return;
			}
		}
		Derailleur<printErrors>::template checkForMatchClosed<']'>(iter);
	}

	template<bool printErrors, VectorT OTy, std::forward_iterator ITy> inline void ParseWithKeys::op(OTy& value, ITy& iter) {
		if (!Derailleur<printErrors>::template checkForMatchClosed<'['>(iter)) {
			return;
		}
		if (Derailleur<printErrors>::template checkForMatchOpen<']'>(iter)) [[unlikely]] {
			return;
		}
		const auto n = value.size();

		auto valueIt = value.begin();

		for (size_t x = 0; x < n; ++x) {
			ParseWithKeys::op<printErrors>(*valueIt++, iter);
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
		while (iter != iter) {
			ParseWithKeys::op<printErrors>(value.emplace_back(), iter);
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

	template<bool printErrors, ObjectT OTy, std::forward_iterator ITy> inline void ParseWithKeys::op(OTy& value, ITy& iter) {
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

			if constexpr (JsonifierObjectT<OTy>) {
				StructuralIterator start = iter;
				if (!Derailleur<printErrors>::template checkForMatchClosed<'"'>(iter)) {
					return;
				}
				StringView key{ reinterpret_cast<const char*>(*start) + 1, static_cast<size_t>(static_cast<int64_t>(*iter - *start)) - 2 };
				if (!Derailleur<printErrors>::template checkForMatchClosed<':'>(iter)) {
					return;
				}
				constexpr auto frozenMap = makeMap<OTy>();
				const auto& memberIt = frozenMap.find(key);
				if (memberIt != frozenMap.end()) [[likely]] {
					std::visit(
						[&](auto& memberPtr) {
							ParseWithKeys::op<printErrors>(getMember(value, memberPtr), iter);
						},
						memberIt->second);
				} else [[unlikely]] {
					Derailleur<printErrors>::skipValue(iter);
				}
			} else {
				ParseWithKeys::op<printErrors>(currentStringBuffer, iter);
				if (!Derailleur<printErrors>::template checkForMatchClosed<':'>(iter)) {
					return;
				}

				if constexpr (StringT<typename OTy::key_type>) {
					ParseWithKeys::op<printErrors>(value[static_cast<typename OTy::key_type>(currentStringBuffer)], iter);
				} else {
					static thread_local typename OTy::key_type key_value{};
					ParseWithKeys::op<printErrors>(key_value, iter);
					ParseWithKeys::op<printErrors>(value[key_value], iter);
				}
			}
		}
	};

	template<bool printErrors, ObjectT OTy, std::forward_iterator ITy, Findable KTy>
	inline void ParseWithKeys::op(OTy& value, ITy& iter, const KTy& excludedKeys) {
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

			if constexpr (JsonifierObjectT<OTy>) {
				StructuralIterator start = iter;
				if (!Derailleur<printErrors>::template checkForMatchClosed<'"'>(iter)) {
					return;
				}
				StringView key{ reinterpret_cast<const char*>(*start) + 1, static_cast<size_t>(static_cast<int64_t>(*iter - *start)) - 2 };
				if (!Derailleur<printErrors>::template checkForMatchClosed<':'>(iter)) {
					return;
				}
				if (excludedKeys.find(static_cast<typename KTy::key_type>(key)) != excludedKeys.end()) {
					skipValue(iter);
					continue;
				}
				constexpr auto frozenMap = makeMap<OTy>();
				const auto& memberIt = frozenMap.find(key);
				if (memberIt != frozenMap.end()) [[likely]] {
					std::visit(
						[&](auto& memberPtr) {
							auto newMember = getMember(value, memberPtr);
							using newMemberType = decltype(newMember);
							if constexpr (HasExcludedKeys<newMemberType>) {
								ParseWithKeys::op<printErrors>(getMember(value, memberPtr), iter, newMember.excludedKeys);
							} else {
								ParseWithKeys::op<printErrors>(getMember(value, memberPtr), iter);
							}
						},
						memberIt->second);
				} else [[unlikely]] {
					Derailleur<printErrors>::skipValue(iter);
				}
			} else {
				ParseNoKeys::op<printErrors>(currentStringBuffer, iter);
				if (!Derailleur<printErrors>::template checkForMatchClosed<':'>(iter)) {
					return;
				}

				if constexpr (StringT<typename OTy::key_type>) {
					ParseNoKeys::op<printErrors>(value[static_cast<typename OTy::key_type>(currentStringBuffer)], iter);
				} else {
					static thread_local typename OTy::key_type key_value{};
					ParseNoKeys::op<printErrors>(key_value, iter);
					ParseNoKeys::op<printErrors>(value[key_value], iter);
				}
			}
		}
	};


}
