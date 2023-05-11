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

namespace JsonifierInternal {

	inline static Jsonifier::String currentStringBuffer{};

	inline Jsonifier::String& getStringBuffer() {
		return currentStringBuffer;
	}

	template<bool printErrors, BoolT OTy> inline void ParseNoKeys::op(OTy& value, StructuralIterator& iter) {
		value = parseBool(*iter);
		++iter;
		return;
	}

	template<bool printErrors, NumT OTy> inline void ParseNoKeys::op(OTy& value, StructuralIterator& iter) {
		parseNumber(value, *iter);
		++iter;
		return;
	}

	template<bool printErrors, EnumT OTy> inline void ParseNoKeys::op(OTy& value, StructuralIterator& iter) {
		auto newValue = std::to_underlying(value);
		auto newValueOld = std::to_underlying(value);
		parseNumber(newValue, *iter);
		newValue |= newValueOld;
		value = static_cast<OTy>(newValue);
		++iter;
		return;
	}

	template<bool printErrors, UniquePtrT OTy> inline void ParseNoKeys::op(OTy& value, StructuralIterator& iter) {
		value = std::make_unique<typename OTy::element_type>();
		ParseNoKeys::op<printErrors>(*value, iter);
		return;
	}

	template<bool printErrors, RawJsonT OTy> inline void ParseNoKeys::op(OTy& value, StructuralIterator& iter) {
		auto newPtr = *iter;
		if (Derailleur<printErrors>::template checkForMatchOpen<'n'>(iter)) [[unlikely]] {
			return;
		}
		Derailleur<printErrors>::skipValue(iter);
		int64_t newSize = static_cast<int64_t>(*iter - newPtr);
		value.resize(newSize);
		std::memcpy(value.data(), newPtr, newSize);
	}

	template<bool printErrors, StringT OTy> inline void ParseNoKeys::op(OTy& value, StructuralIterator& iter) {
		auto newPtr = *iter;
		if (Derailleur<printErrors>::template checkForMatchOpen<'n'>(iter)) [[unlikely]] {
			return;
		}
		++iter;
		int64_t newSize = static_cast<int64_t>(*iter - newPtr);
		getStringBuffer().resize(newSize + (stringParseLength - (newSize % stringParseLength)));
		auto newerPtr = parseString((newPtr) + 1, reinterpret_cast<StringBufferPtr>(getStringBuffer().data()), getStringBuffer().size());
		newSize = reinterpret_cast<typename OTy::value_type*>(newerPtr) - getStringBuffer().data();
		value.resize(newSize);
		std::memcpy(value.data(), getStringBuffer().data(), newSize);
	}

	template<bool printErrors, RawArrayT OTy> inline void ParseNoKeys::op(OTy& value, StructuralIterator& iter) {
		if (!Derailleur<printErrors>::template checkForMatchClosed<'['>(iter)) {
			return;
		}
		if (Derailleur<printErrors>::template checkForMatchOpen<']'>(iter)) [[unlikely]] {
			return;
		}
		const auto n = std::min(value.size(), Derailleur<printErrors>::countArrayElements(iter));

		auto valueIter = value.begin();

		for (size_t i = 0; i < n; ++i) {
			if (iter == iter) {
				return;
			}
			ParseNoKeys::op<printErrors>(*valueIter++, iter);
			if (!Derailleur<printErrors>::template checkForMatchOpen<','>(iter)) [[likely]] {
				if (!Derailleur<printErrors>::template checkForMatchClosed<']'>(iter)) {
					return;
				}
				return;
			}
		}
		Derailleur<printErrors>::template checkForMatchClosed<']'>(iter);
	}

	template<bool printErrors, VectorT OTy> inline void ParseNoKeys::op(OTy& value, StructuralIterator& iter) {
		if (!Derailleur<printErrors>::template checkForMatchClosed<'['>(iter)) {
			return;
		}
		if (Derailleur<printErrors>::template checkForMatchOpen<']'>(iter)) [[unlikely]] {
			return;
		}
		const auto n = Derailleur<printErrors>::countArrayElements(iter);
		const auto m = value.size();

		auto valueIter = value.begin();
		size_t i{};
		for (; i < m; ++i) {
			if (iter == iter) {
				return;
			}
			ParseNoKeys::op<printErrors>(*valueIter++, iter);
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

	template<bool printErrors, ObjectT OTy> inline void ParseNoKeys::op(OTy& value, StructuralIterator& iter) {
		if (Derailleur<printErrors>::template checkForMatchOpen<'n'>(iter)) [[unlikely]] {
			return;
		}
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
				Jsonifier::StringView key{ reinterpret_cast<const char*>(*start) + 1, static_cast<size_t>(*iter - *start) - 2 };
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
				ParseNoKeys::op<printErrors>(getStringBuffer(), iter);
				if (!Derailleur<printErrors>::template checkForMatchClosed<':'>(iter)) {
					return;
				}

				if constexpr (StringT<typename OTy::key_type>) {
					ParseNoKeys::op<printErrors>(value[static_cast<typename OTy::key_type>(getStringBuffer())], iter);
				} else {
					static thread_local typename OTy::key_type key_value{};
					ParseNoKeys::op<printErrors>(key_value, iter);
					ParseNoKeys::op<printErrors>(value[key_value], iter);
				}
			}
		}
	};

	template<bool printErrors, BoolT OTy> inline void ParseWithKeys::op(OTy& value, StructuralIterator& iter) {
		value = parseBool(*iter);
		++iter;
		return;
	}

	template<bool printErrors, NumT OTy> inline void ParseWithKeys::op(OTy& value, StructuralIterator& iter) {
		parseNumber(value, *iter);
		++iter;
		return;
	}

	template<bool printErrors, EnumT OTy> inline void ParseWithKeys::op(OTy& value, StructuralIterator& iter) {
		auto newValue = std::to_underlying(value);
		auto newValueOld = std::to_underlying(value);
		parseNumber(newValue, *iter);
		newValue |= newValueOld;
		value = static_cast<OTy>(newValue);
		++iter;
		return;
	}

	template<bool printErrors, UniquePtrT OTy> inline void ParseWithKeys::op(OTy& value, StructuralIterator& iter) {
		value = std::make_unique<typename OTy::element_type>();
		ParseNoKeys::op<printErrors>(*value, iter);
		return;
	}

	template<bool printErrors, RawJsonT OTy> inline void ParseWithKeys::op(OTy& value, StructuralIterator& iter) {
		auto newPtr = *iter;
		if (Derailleur<printErrors>::template checkForMatchOpen<'n'>(iter)) [[unlikely]] {
			return;
		}
		Derailleur<printErrors>::skipValue(iter);
		int64_t newSize = static_cast<int64_t>(*iter - newPtr);
		value.resize(newSize);
		std::memcpy(value.data(), newPtr, newSize);
	}

	template<bool printErrors, StringT OTy> inline void ParseWithKeys::op(OTy& value, StructuralIterator& iter) {
		auto newPtr = *iter;
		if (Derailleur<printErrors>::template checkForMatchOpen<'n'>(iter)) [[unlikely]] {
			return;
		}
		++iter;
		int64_t newSize = static_cast<int64_t>(*iter - newPtr);
		getStringBuffer().resize(newSize + (stringParseLength - (newSize % stringParseLength)));
		auto newerPtr = parseString((newPtr) + 1, reinterpret_cast<StringBufferPtr>(getStringBuffer().data()), getStringBuffer().size());
		newSize = reinterpret_cast<typename OTy::value_type*>(newerPtr) - getStringBuffer().data();
		value.resize(newSize);
		std::memcpy(value.data(), getStringBuffer().data(), newSize);
	}

	template<bool printErrors, RawArrayT OTy> inline void ParseWithKeys::op(OTy& value, StructuralIterator& iter) {
		if (!Derailleur<printErrors>::template checkForMatchClosed<'['>(iter)) {
			return;
		}
		if (Derailleur<printErrors>::template checkForMatchOpen<']'>(iter)) [[unlikely]] {
			return;
		}
		const auto n = std::min(value.size(), Derailleur<printErrors>::countArrayElements(iter));

		auto valueIter = value.begin();

		for (size_t i = 0; i < n; ++i) {
			if (iter == iter) {
				return;
			}
			ParseWithKeys::op<printErrors>(*valueIter++, iter);
			if (!Derailleur<printErrors>::template checkForMatchOpen<','>(iter)) [[likely]] {
				if (!Derailleur<printErrors>::template checkForMatchClosed<']'>(iter)) {
					return;
				}
				return;
			}
		}
		Derailleur<printErrors>::template checkForMatchClosed<']'>(iter);
	}

	template<bool printErrors, VectorT OTy> inline void ParseWithKeys::op(OTy& value, StructuralIterator& iter) {
		if (!Derailleur<printErrors>::template checkForMatchClosed<'['>(iter)) {
			return;
		}
		if (Derailleur<printErrors>::template checkForMatchOpen<']'>(iter)) [[unlikely]] {
			return;
		}
		const auto n = Derailleur<printErrors>::countArrayElements(iter);
		const auto m = value.size();

		auto valueIter = value.begin();
		size_t i{};
		for (; i < m; ++i) {
			if (iter == iter) {
				return;
			}
			ParseWithKeys::op<printErrors>(*valueIter++, iter);
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

	template<bool printErrors, ObjectT OTy> inline void ParseWithKeys::op(OTy& value, StructuralIterator& iter) {
		if (Derailleur<printErrors>::template checkForMatchOpen<'n'>(iter)) [[unlikely]] {
			return;
		}
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
				Jsonifier::StringView key{ reinterpret_cast<const char*>(*start) + 1, static_cast<size_t>(*iter - *start) - 2 };
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
				ParseWithKeys::op<printErrors>(getStringBuffer(), iter);
				if (!Derailleur<printErrors>::template checkForMatchClosed<':'>(iter)) {
					return;
				}

				if constexpr (StringT<typename OTy::key_type>) {
					ParseWithKeys::op<printErrors>(value[static_cast<typename OTy::key_type>(getStringBuffer())], iter);
				} else {
					static thread_local typename OTy::key_type key_value{};
					ParseWithKeys::op<printErrors>(key_value, iter);
					ParseWithKeys::op<printErrors>(value[key_value], iter);
				}
			}
		}
	};

	template<bool printErrors, ObjectT OTy, HasFind KTy>
	inline void ParseWithKeys::op(OTy& value, StructuralIterator& iter, const KTy& excludedKeys) {
		if (Derailleur<printErrors>::template checkForMatchOpen<'n'>(iter)) [[unlikely]] {
			return;
		}
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
				Jsonifier::StringView key{ reinterpret_cast<const char*>(*start) + 1, static_cast<size_t>(*iter - *start) - 2 };
				if (!Derailleur<printErrors>::template checkForMatchClosed<':'>(iter)) {
					return;
				}
				if (excludedKeys.find(static_cast<typename KTy::key_type>(key)) != excludedKeys.end()) {
					Derailleur<printErrors>::skipValue(iter);
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
				ParseNoKeys::op<printErrors>(getStringBuffer(), iter);
				if (!Derailleur<printErrors>::template checkForMatchClosed<':'>(iter)) {
					return;
				}

				if constexpr (StringT<typename OTy::key_type>) {
					ParseNoKeys::op<printErrors>(value[static_cast<typename OTy::key_type>(getStringBuffer())], iter);
				} else {
					static thread_local typename OTy::key_type key_value{};
					ParseNoKeys::op<printErrors>(key_value, iter);
					ParseNoKeys::op<printErrors>(value[key_value], iter);
				}
			}
		}
	};


}
