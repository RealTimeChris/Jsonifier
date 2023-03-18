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
		template<typename It> inline static void op(OTy& value, It& it, It& end) {
			value = parseBool(*it);
			++it;
			return;
		}
	};

	template<typename OTy>
		requires NumT<OTy>
	struct FromJson<OTy> {
		template<typename It> inline static void op(OTy& value, It& it, It& end) {
			auto newPtr = *it;
			parseNumber(value, newPtr);
			++it;
			return;
		}
	};

	template<typename T>
		requires StringT<T>
	struct FromJson<T> {
		template<class It> inline static void op(auto& value, It&& it, It&& end) noexcept {
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
		requires(ArrayT<OTy> && !EmplaceBackable<OTy> && !Resizeable<OTy> && Emplaceable<OTy>)
	struct FromJson<OTy> {
		template<typename It> inline static void op(OTy& value, It& it, It& end) {
			match<'['>(it);
			if (**it == ']') [[unlikely]] {
				++it;
				return;
			}

			while (it != end) {
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
			}
		}
	};

	template<typename OTy>
		requires(ArrayT<OTy> && ( EmplaceBackable<OTy> || !Resizeable<OTy> ) && !Emplaceable<OTy>)
	struct FromJson<OTy> {
		template<typename It> inline static void op(OTy& value, It& it, It& end) {
			match<'['>(it);
			if (**it == ']') [[unlikely]] {
				++it;
				return;
			}
			const auto n = value.size();

			auto value_it = value.begin();

			for (size_t i = 0; i < n; ++i) {
				Read::op(*value_it++, it, end);

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
			}
		}
	};

	template<typename OTy>
		requires ArrayT<OTy> && ( !EmplaceBackable<OTy> && Resizeable<OTy> )
	struct FromJson<OTy> {
		template<typename It> inline static void op(OTy& value, It& it, It& end) {
			match<'['>(it);
			if (**it == ']') [[unlikely]] {
				++it;
				return;
			}
			const auto n = countArrayElements(it, end);
			value.resize(n);
			size_t i = 0;
			for (auto& x: value) {
				Read::op(x, it, end);
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
		template<typename It> inline static void op(OTy& value, It& it, It& end) {
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
			}();

			forEach<N>([&](auto I) {
				if (**it == ']') {
					++it;
					return;
				}
				if constexpr (I != 0) {
					match<','>(it);
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
			match<'{'>(it);
			bool first{ true };
			while (it != end) {
				if (first) [[unlikely]] {
					first = false;
				} else [[likely]] {
					match<','>(it);
				}
				auto start = it;
				match<'"'>(it);
				StringView key = StringView{ reinterpret_cast<const char*>(*start) + 1, static_cast<size_t>(*it - *start) - 2 };

				match<':'>(it);
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
