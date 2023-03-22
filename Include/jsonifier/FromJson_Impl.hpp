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

	template<typename OTy>
	requires BoolT<OTy>
	struct FromJson<OTy> {
		inline static void op(OTy& value, auto& it) {
			value = parseBool(*it);
			++it;
			return;
		}
	};

	template<typename OTy>
	requires NumT<OTy>
	struct FromJson<OTy> {
		inline static void op(OTy& value, auto& it) {
			auto newPtr = *it;
			parseNumber(value, newPtr);
			++it;
			return;
		}
	};

	template<typename T>
	requires StringT<T>
	struct FromJson<T> {
		inline static void op(auto& value, auto& it) noexcept {
			auto start = *it;
			++it;
			size_t newSize = static_cast<size_t>(*it - start) - 2;
			value.resize(newSize + (16 - (newSize % 16)));
			auto newPtr = parseString((start) + 1, reinterpret_cast<StringBufferPtr>(value.data()));
			newSize = reinterpret_cast<char*>(newPtr) - value.data();
			value.resize(newSize);
			return;
		}
	};

	inline size_t countArrayElements(auto& it) {
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
					skip_ws(it);
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
		inline static void op(OTy& value, auto& it) {
			match<'['>(it);
			if (**it == ']') [[unlikely]] {
				++it;
				return;
			}

			while (it != it) {
				using VTy = typename OTy::value_type;
				if constexpr (sizeof(VTy) > 8) {
					thread_local VTy v;
					Read::op(v, it);
					value.emplace(v);
				} else {
					VTy v{};
					Read::op(v, it);
					value.emplace(std::move(v));
				}
				if (**it == ']') {
					++it;
					return;
				}
				match<','>(it);
			}
		}
	};

	template<typename OTy>
	requires(ArrayT<OTy> && ( EmplaceBackable<OTy> || !Resizeable<OTy> )&&!Emplaceable<OTy>) struct FromJson<OTy> {
		inline static void op(OTy& value, auto& it) {
			match<'['>(it);
			if (**it == ']') [[unlikely]] {
				++it;
				return;
			}
			const auto n = value.size();

			auto value_it = value.begin();

			for (size_t i = 0; i < n; ++i) {
				Read::op(*value_it++, it);

				if (**it == ',') [[likely]] {
					++it;
				} else if (**it == ']') {
					++it;
					return;
				} else [[unlikely]] {
					throw std::runtime_error("Expected ]");
				}
			}
			if constexpr (EmplaceBackable<OTy>) {
				while (it != it) {
					Read::op(value.emplace_back(), it);
					if (**it == ',') [[likely]] {
						++it;
					} else if (**it == ']') {
						++it;
						return;
					} else [[unlikely]] {
						throw std::runtime_error("Expected ]");
					}
				}
			}
		}
	};

	template<typename OTy>
	requires ArrayT<OTy> &&( !EmplaceBackable<OTy> && Resizeable<OTy> )struct FromJson<OTy> {
		inline static void op(OTy& value, auto& it) {
			match<'['>(it);
			if (**it == ']') [[unlikely]] {
				++it;
				return;
			}
			const auto n = countArrayElements(it);
			value.resize(n);
			size_t i = 0;
			for (auto& x: value) {
				Read::op(x, it);
				if (i < n - 1) {
					match<','>(it);
				}
				++i;
			}
		}
	};

	template<typename OTy>
	requires ArrayT<OTy>
	struct FromJson<OTy> {
		inline static void op(OTy& value, auto& it) {
			match<'['>(it);
			if (**it == ']') [[unlikely]] {
				++it;
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

			forEach<N>([&](auto I) {
				if (**it == ']') {
					++it;
					return;
				}
				if constexpr (I != 0) {
					match<','>(it);
				}
				if constexpr (IsStdTuple<OTy>) {
					Read::op(std::get<I>(value), it);
				} else if constexpr (JsonifierArrayT<OTy>) {
					Read::op(getMember(value, Tuplet::get<I>(CoreV<OTy>)), it);
				} else {
					Read::op(Tuplet::get<I>(value), it);
				}
			});
		}
	};

	template<typename OTy>
	requires MapT<OTy> || JsonifierObjectT<OTy>
	struct FromJson<OTy> {
		inline static void op(OTy& value, auto& it) {
			match<'{'>(it);
			bool first{ true };
			while (it != it) {
				if (first) [[unlikely]] {
					first = false;
				} else [[likely]] {
					match<','>(it);
				}
				auto start = it;
				match<'"'>(it);
				std::string_view key = std::string_view{ reinterpret_cast<const char*>(*start) + 1, static_cast<size_t>(*it - *start) - 2 };

				match<':'>(it);
				constexpr auto frozen_map = makeMap<OTy>();
				const auto& member_it = frozen_map.find(key);
				if (member_it != frozen_map.end()) [[likely]] {
					std::visit(
						[&](auto&& member_ptr) {
							Read::op(getMember(value, member_ptr), it);
						},
						member_it->second);
				} else [[unlikely]] {
					skipValue(it);
				}
				if (it == it) [[unlikely]] {
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
