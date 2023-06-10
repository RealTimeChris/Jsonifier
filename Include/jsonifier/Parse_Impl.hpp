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

#include <jsonifier/NumberParsing.hpp>
#include <jsonifier/StringParsing.hpp>
#include <jsonifier/Derailleur.hpp>
#include <jsonifier/Parser.hpp>
#include <jsonifier/Base.hpp>
#include <iterator>

namespace JsonifierInternal {

	inline static Jsonifier::String& getCurrentStringBuffer() noexcept {
		static thread_local Jsonifier::String currentStringBuffer{};
		return currentStringBuffer;
	}

	template<bool printErrors, BoolT ValueType> inline void ParseNoKeys::op(ValueType& value, StructuralIterator& iter) {
		value = parseBool(*iter);
		++iter;
		return;
	}

	template<bool printErrors, NumT ValueType> inline void ParseNoKeys::op(ValueType& value, StructuralIterator& iter) {
		parseNumber(value, *iter);
		++iter;
		return;
	}

	template<bool printErrors, EnumT ValueType> inline void ParseNoKeys::op(ValueType& value, StructuralIterator& iter) {
		auto newValue = std::to_underlying(value);
		auto newValueOld = std::to_underlying(value);
		parseNumber(newValue, *iter);
		newValue |= newValueOld;
		value = static_cast<ValueType>(newValue);
		++iter;
		return;
	}

	template<bool printErrors, UniquePtrT ValueType> inline void ParseNoKeys::op(ValueType& value, StructuralIterator& iter) {
		value = std::make_unique<typename ValueType::element_type>();
		ParseNoKeys::op<printErrors>(*value, iter);
		return;
	}

	template<bool printErrors, RawJsonT ValueType> inline void ParseNoKeys::op(ValueType& value, StructuralIterator& iter) {
		auto newPtr = *iter;
		Derailleur<printErrors>::skipValue(iter);
		int64_t sizeNew = static_cast<int64_t>(*iter - newPtr);
		value.resize(sizeNew);
		std::memcpy(value.data(), newPtr, sizeNew);
	}

	template<bool printErrors, StringT ValueType> inline void ParseNoKeys::op(ValueType& value, StructuralIterator& iter) {
		auto newPtr = *iter;
		++iter;
		int64_t sizeNew = static_cast<int64_t>(*iter - newPtr);
		auto newerSize = sizeNew + (BytesPerStep - (sizeNew % BytesPerStep));
		getCurrentStringBuffer().resize(newerSize * 2);
		auto newerPtr = parseString((newPtr) + 1, reinterpret_cast<StringBufferPtr>(getCurrentStringBuffer().data()));
		if (newPtr) {
			newerSize = reinterpret_cast<typename ValueType::value_type*>(newerPtr) - getCurrentStringBuffer().data();
			value.resize(newerSize);
			std::memcpy(value.data(), getCurrentStringBuffer().data(), newerSize);
		}
	}

	template<bool printErrors, RawArrayT ValueType> inline void ParseNoKeys::op(ValueType& value, StructuralIterator& iter) {
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

	template<bool printErrors, VectorT ValueType> inline void ParseNoKeys::op(ValueType& value, StructuralIterator& iter) {
		if (!Derailleur<printErrors>::template checkForMatchClosed<'['>(iter)) {
			return;
		}
		if (Derailleur<printErrors>::template checkForMatchOpen<']'>(iter)) [[unlikely]] {
			return;
		}
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

	template<bool printErrors, ObjectT ValueType> inline void ParseNoKeys::op(ValueType& value, StructuralIterator& iter) {
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
					continue;
				}
			}

			if constexpr (JsonifierObjectT<ValueType>) {
				StructuralIterator start = iter;
				if (!Derailleur<printErrors>::template checkForMatchClosed<'"'>(iter)) {
					continue;
				}
				const Jsonifier::StringView key{ reinterpret_cast<const char*>(*start) + 1, static_cast<size_t>(*iter - *start) - 2 };
				if (!Derailleur<printErrors>::template checkForMatchClosed<':'>(iter)) {
					continue;
				}
				constexpr auto frozenMap = makeMap<ValueType>();
				const auto& memberIt = frozenMap.find(key);
				if (Derailleur<printErrors>::template checkForMatchOpen<'n'>(iter)) [[unlikely]] {
					continue;
				} else if (memberIt != frozenMap.end()) [[likely]] {
					std::visit(
						[&](auto& memberPtr) {
							ParseNoKeys::op<printErrors>(getMember(value, memberPtr), iter);
						},
						memberIt->second);
				} else [[unlikely]] {
					Derailleur<printErrors>::skipValue(iter);
				}
			} else {
				ParseNoKeys::op<printErrors>(getCurrentStringBuffer(), iter);
				if (!Derailleur<printErrors>::template checkForMatchClosed<':'>(iter)) {
					continue;
				}

				if constexpr (StringT<typename ValueType::key_type>) {
					ParseNoKeys::op<printErrors>(value[static_cast<typename ValueType::key_type>(getCurrentStringBuffer())], iter);
				} else {
					static thread_local typename ValueType::key_type key_value{};
					ParseNoKeys::op<printErrors>(key_value, iter);
					ParseNoKeys::op<printErrors>(value[key_value], iter);
				}
			}
		}
	};

	template<bool printErrors, BoolT ValueType> inline void ParseWithKeys::op(ValueType& value, StructuralIterator& iter) {
		value = parseBool(*iter);
		++iter;
		return;
	}

	template<bool printErrors, NumT ValueType> inline void ParseWithKeys::op(ValueType& value, StructuralIterator& iter) {
		parseNumber(value, *iter);
		++iter;
		return;
	}

	template<bool printErrors, EnumT ValueType> inline void ParseWithKeys::op(ValueType& value, StructuralIterator& iter) {
		auto newValue = std::to_underlying(value);
		auto newValueOld = std::to_underlying(value);
		parseNumber(newValue, *iter);
		newValue |= newValueOld;
		value = static_cast<ValueType>(newValue);
		++iter;
		return;
	}

	template<bool printErrors, UniquePtrT ValueType> inline void ParseWithKeys::op(ValueType& value, StructuralIterator& iter) {
		value = std::make_unique<typename ValueType::element_type>();
		ParseNoKeys::op<printErrors>(*value, iter);
		return;
	}

	template<bool printErrors, RawJsonT ValueType> inline void ParseWithKeys::op(ValueType& value, StructuralIterator& iter) {
		auto newPtr = *iter;
		Derailleur<printErrors>::skipValue(iter);
		int64_t sizeNew = static_cast<int64_t>(*iter - newPtr);
		value.resize(sizeNew);
		std::memcpy(value.data(), newPtr, sizeNew);
	}

	template<bool printErrors, StringT ValueType> inline void ParseWithKeys::op(ValueType& value, StructuralIterator& iter) {
		auto newPtr = *iter;
		++iter;
		int64_t sizeNew = static_cast<int64_t>(*iter - newPtr);
		auto newerSize = sizeNew + (BytesPerStep - (sizeNew % BytesPerStep));
		getCurrentStringBuffer().resize(newerSize * 2);
		auto newerPtr = parseString((newPtr) + 1, reinterpret_cast<StringBufferPtr>(getCurrentStringBuffer().data()));
		if (newPtr) {
			newerSize = reinterpret_cast<typename ValueType::value_type*>(newerPtr) - getCurrentStringBuffer().data();
			value.resize(newerSize);
			std::memcpy(value.data(), getCurrentStringBuffer().data(), newerSize);
		}
	}

	template<bool printErrors, RawArrayT ValueType> inline void ParseWithKeys::op(ValueType& value, StructuralIterator& iter) {
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

	template<bool printErrors, VectorT ValueType> inline void ParseWithKeys::op(ValueType& value, StructuralIterator& iter) {
		if (!Derailleur<printErrors>::template checkForMatchClosed<'['>(iter)) {
			return;
		}
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

	template<bool printErrors, ObjectT ValueType> inline void ParseWithKeys::op(ValueType& value, StructuralIterator& iter) {
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
					continue;
				}
			}

			if constexpr (JsonifierObjectT<ValueType>) {
				StructuralIterator start = iter;
				if (!Derailleur<printErrors>::template checkForMatchClosed<'"'>(iter)) {
					continue;
				}
				const Jsonifier::StringView key{ reinterpret_cast<const char*>(*start) + 1, static_cast<size_t>(*iter - *start) - 2 };
				if (!Derailleur<printErrors>::template checkForMatchClosed<':'>(iter)) {
					continue;
				}
				constexpr auto frozenMap = makeMap<ValueType>();
				const auto& memberIt = frozenMap.find(key);
				if (Derailleur<printErrors>::template checkForMatchOpen<'n'>(iter)) [[unlikely]] {
					continue;
				} else if (memberIt != frozenMap.end()) [[likely]] {
					std::visit(
						[&](auto& memberPtr) {
							ParseWithKeys::op<printErrors>(getMember(value, memberPtr), iter);
						},
						memberIt->second);
				} else [[unlikely]] {
					Derailleur<printErrors>::skipValue(iter);
				}
			} else {
				ParseWithKeys::op<printErrors>(getCurrentStringBuffer(), iter);
				if (!Derailleur<printErrors>::template checkForMatchClosed<':'>(iter)) {
					continue;
				}

				if constexpr (StringT<typename ValueType::key_type>) {
					ParseWithKeys::op<printErrors>(value[static_cast<typename ValueType::key_type>(getCurrentStringBuffer())], iter);
				} else {
					static thread_local typename ValueType::key_type key_value{};
					ParseWithKeys::op<printErrors>(key_value, iter);
					ParseWithKeys::op<printErrors>(value[key_value], iter);
				}
			}
		}
	};

	template<bool printErrors, ObjectT ValueType, HasFind KeyType>
	inline void ParseWithKeys::op(ValueType& value, StructuralIterator& iter, const KeyType& excludedKeys) {
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
				const Jsonifier::StringView key{ reinterpret_cast<const char*>(*start) + 1, static_cast<size_t>(*iter - *start) - 2 };
				if (!Derailleur<printErrors>::template checkForMatchClosed<':'>(iter)) {
					return;
				}
				if (excludedKeys.find(static_cast<typename KeyType::key_type>(key)) != excludedKeys.end()) {
					Derailleur<printErrors>::skipValue(iter);
					continue;
				}
				constexpr auto frozenMap = makeMap<ValueType>();
				const auto& memberIt = frozenMap.find(key);
				if (Derailleur<printErrors>::template checkForMatchOpen<'n'>(iter)) [[unlikely]] {
					continue;
				} else if (memberIt != frozenMap.end()) [[likely]] {
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
				ParseNoKeys::op<printErrors>(getCurrentStringBuffer(), iter);
				if (!Derailleur<printErrors>::template checkForMatchClosed<':'>(iter)) {
					return;
				}

				if constexpr (StringT<typename ValueType::key_type>) {
					ParseNoKeys::op<printErrors>(value[static_cast<typename ValueType::key_type>(getCurrentStringBuffer())], iter);
				} else {
					static thread_local typename ValueType::key_type key_value{};
					ParseNoKeys::op<printErrors>(key_value, iter);
					ParseNoKeys::op<printErrors>(value[key_value], iter);
				}
			}
		}
	};


}
