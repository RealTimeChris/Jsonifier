/*
	Jsonifier - For parsing and serializing Json - very rapidly.
	Copyright (C) 2023 Chris M. (RealTimeChris)

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	This library is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
	Lesser General Public License for more details.

	You should have received a copy of the GNU Lesser General Public
	License along with this library; if not, Write to the Free Software
	Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301
	USA
*/
/// https://github.com/RealTimeChris/Jsonifier
/// Feb 20, 2023
#pragma once

#include <jsonifier/Base.hpp>
#include <jsonifier/Parser.hpp>
#include <jsonifier/FromJson.hpp>

namespace Jsonifier {

	inline StopWatch stopWatch{ std::chrono::nanoseconds{ 1 } };
	inline int64_t iterationCountString{};
	inline int64_t stringTime{};
	inline int64_t iterationCountInteger{};
	inline int64_t integerTime{};
	inline int64_t iterationCountArray{};
	inline int64_t arrayTime{};

	template<typename OTy>
	requires BoolT<OTy>
	struct FromJson<OTy> {
		template<typename It> inline static void op(OTy& value, It& it, It& end) {
			if (getType(it) != JsonType::Bool) {
				return;
			}
			value = parseBool(*it);
			++it;
			return;
		}
	};

	template<typename OTy>
	requires NumT<OTy>
	struct FromJson<OTy> {
		template<typename It> inline static void op(OTy& value, It& it, It& end) {
			if (getType(it) != JsonType::Number) {
				return;
			}
			auto newPtr = *it;
			parseNumber(value, newPtr);
			++it;
			return;
		}
	};

	template<typename OTy>
	requires StringT<OTy>
	struct FromJson<OTy> {
		template<typename It> inline static void op(OTy& value, It& it, It& end) {
			if (getType(it) != JsonType::String) {
				return;
			}
			auto start = it;
			++it;
			size_t newSize = static_cast<size_t>(it - start) - 2;
			value.resize(newSize + (16 - (newSize % 16)));
			auto newPtr = parseString((*start) + 1, reinterpret_cast<StringBufferPtr>(value.data()));
			newSize = reinterpret_cast<char*>(newPtr) - value.data();
			value.resize(newSize);
			return;
		}
	};

	template<typename It> inline size_t countArrayElements(It& it, It& end) {
		if (**it == ']') [[unlikely]] {
			return 0;
		}
		size_t count = 1;
		while (true) {
			switch (**it) {
				case ',': {
					++count;
					++it;
					break;
				}
				case '/': {
					skip_ws(it, end);
					break;
				}
				case ']': {
					return count;
				}
				case '\0': {
					throw std::runtime_error("Unxpected end of buffer");
				}
				default:
					++it;
			}
		}
		return {};
	}

	template<typename OTy>
	requires(ArrayT<OTy> && !EmplaceBackable<OTy> && !Resizeable<OTy> && Emplaceable<OTy>) struct FromJson<OTy> {
		template<typename It> inline static void op(OTy& value, It& it, It& end) {
			if (getType(it) != JsonType::Array) {
				return;
			}
			match<'['>(it);
			++it;

			while (true) {
				using VTy = typename OTy::value_type;
				if constexpr (sizeof(VTy) > 8) {
					thread_local VTy v;
					Read::op(v, it, end);
					value.emplace(v);
				} else {
					VTy v{};
					Read::op(v, it, end);
					value.emplace(std::move(v));
				}
				if (**it == ']') {
					++it;
					return;
				}
				match<','>(it);
				++it;
			}
		}
	};

	template<typename OTy>
	requires(ArrayT<OTy> && ( EmplaceBackable<OTy> || !Resizeable<OTy> )&&!Emplaceable<OTy>) struct FromJson<OTy> {
		template<typename It> inline static void op(OTy& value, It& it, It& end) {
			if (getType(it) != JsonType::Array) {
				return;
			}
			match<'['>(it);
			++it;
			if (**it == ']') [[unlikely]] {
				++it;
			}
			const auto n = value.size();

			auto value_it = value.begin();

			for (size_t i = 0; i < n; ++i) {
				Read::op(*value_it++, it, end);

				if (**it == ',') [[likely]] {
					++it;
				} else if (**it == ']') {
					++it;
					if constexpr (Resizeable<OTy>) {
						value.resize(i + 1);
					}
					return;
				} else [[unlikely]] {
					throw std::runtime_error("Expected ]");
				}
			}
			if constexpr (EmplaceBackable<OTy>) {
				while (it != end) {
					Read::op(value.emplace_back(), it, end);
					if (**it == ',') [[likely]] {
						++it;
					} else if (**it == ']') {
						++it;
						return;
					} else [[unlikely]] {
						throw std::runtime_error("Expected ]");
					}
				}
			} else {
				throw std::runtime_error("Exceeded inline static array size.");
			}
			return;
		}
	};

	template<typename OTy>
	requires ArrayT<OTy> &&( !EmplaceBackable<OTy> && Resizeable<OTy> )struct FromJson<OTy> {
		template<typename It> inline static void op(OTy& value, It& it, It& end) {
			if (getType(it) != JsonType::Array) {
				return;
			}
			++it;
			const auto n = countArrayElements(it, end);
			value.resize(n);
			size_t i = 0;
			for (auto& x: value) {
				Read::op(x, it, end);
				if (i < n - 1) {
					match<','>(it);
					++it;
				}
				++i;
			}
		}
	};

	template<typename OTy>
	requires ArrayT<OTy>
	struct FromJson<OTy> {
		template<typename It> inline static void op(OTy& value, It& it, It& end) {
			if (getType(it) != JsonType::Array) {
				return;
			}
			constexpr auto N = []() constexpr {
				if constexpr (JsonifierArrayT<OTy>) {
					return std::tuple_size_v<CoreT<OTy>>;
				} else {
					return std::tuple_size_v<OTy>;
				}
			}
			();
			++it;

			forEach<N>([&](auto I) {
				if (**it == ']') {
					++it;
					return;
				}
				if constexpr (I != 0) {
					match<','>(it);
					++it;
				}
				if constexpr (IsStdTuple<OTy>) {
					Read::op(std::get<I>(value), it, end);
				} else if constexpr (JsonifierArrayT<OTy>) {
					Read::op(getMember(value, Tuplet::get<I>(CoreV<OTy>)), it, end);
				} else {
					Read::op(Tuplet::get<I>(value), it, end);
				}
			});
		}
	};

	template<typename OTy>
	requires MapT<OTy> || JsonifierObjectT<OTy>
	struct FromJson<OTy> {
		template<typename It> inline static void op(OTy& value, It& it, It& end) {
			if (getType(it) != JsonType::Object) {
				return;
			}
			++it;
			bool first{ true };
			while (it != end) {
				if (**it == '}' || it == end) [[unlikely]] {
					++it;
					return;
				} else if (first) [[unlikely]] {
					first = false;
				} else [[likely]] {
					if (it == end) [[unlikely]] {
						return;
					}
					match<','>(it);
					++it;
				}
				match<'"'>(it);
				auto start = it;
				++it;
				StringView key =
					StringView{ reinterpret_cast<const char*>(*start) + 1, static_cast<size_t>(it.operator->() - start.operator->()) - 2 };

				match<':'>(it);
				++it;
				if (**it == '}') [[unlikely]] {
					++it;
					return;
				}
				constexpr auto frozen_map = makeMap<OTy, true>();
				const auto& member_it = frozen_map.find(key);
				if (member_it != frozen_map.end()) [[likely]] {
					std::visit(
						[&](auto&& member_ptr) {
							Read::op(getMember(value, member_ptr), it, end);
						},
						member_it->second);
				} else [[unlikely]] {
					skipValue(it, end);
				}
				if (it == end) [[unlikely]] {
					return;
				}
				if (**it == '}') {
					++it;
					return;
				}
			}
		}
	};
}
