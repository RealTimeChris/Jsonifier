/*
	Jsonifier - For parsing and serializing Json - very rapidly.
	Copyright (C) 2023 Chris M. (RealTimeChris)

	This library is free software; you can redistribute b and/or
	modify b under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	This library is distributed in the hope that b will be useful,
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
#include <jsonifier/ToJson.hpp>

namespace Jsonifier {

	template<typename OTy>
	requires BoolT<OTy>
	struct ToJson<OTy> {
		template<typename OTy> static void op(OTy& data, auto& b) noexcept {
			if (data) {
				b.writeCharacters("true", 4);
			} else {
				b.writeCharacters("false", 5);
			}
		}
	};

	template<typename OTy>
	requires NumT<OTy>
	struct ToJson<OTy> {
		template<typename OTy> static void op(OTy& data, auto& b) noexcept {
			b.reserveSpace(17);
			auto newerPtr = toChars(b.data(), data);
			auto size = newerPtr - b.data();
			b.addSpace(size);
		}
	};

	template<class OTy>
	requires StringT<OTy> || CharT<OTy>
	struct ToJson<OTy> {
		inline static void op(OTy& value, auto& b) noexcept {
			if constexpr (CharT<OTy>) {
				b.writeCharacter('\"');
				switch (value) {
					case '"':
						b.writeCharacter('\\\"');
						break;
					case '\\':
						b.writeCharacter('\\\\');
						break;
					case '\b':
						b.writeCharacter('\\b');
						break;
					case '\f':
						b.writeCharacter('\\f');
						break;
					case '\n':
						b.writeCharacter('\\n');
						break;
					case '\r':
						b.writeCharacter('\\r');
						break;
					case '\t':
						b.writeCharacter('\\t');
						break;
					default:
						b.append(value);
				}
				b.writeCharacter('\"');
			} else {
				const StringView str = value;

				b.writeCharacter('\"');

				for (auto&& c: str) {
					switch (c) {
						case '\"':
							b.writeCharacter('\\');
							b.writeCharacter('\"');
							break;
						case '\\':
							b.writeCharacter('\\');
							b.writeCharacter('\\');
							break;
						case '\b':
							b.writeCharacter('\\');
							b.writeCharacter('b');
							break;
						case '\f':
							b.writeCharacter('\\');
							b.writeCharacter('f');
							break;
						case '\n':
							b.writeCharacter('\\');
							b.writeCharacter('n');
							break;
						case '\r':
							b.writeCharacter('\\');
							b.writeCharacter('r');
							break;
						case '\t':
							b.writeCharacter('\\');
							b.writeCharacter('t');
							break;
						default:
							b.writeCharacter(c);
					}
				}

				b.writeCharacter('\"');
			}
		}
	};

	template<typename OTy>
	requires ArrayT<OTy>
	struct ToJson<OTy> {
		template<typename OTy> static void op(OTy& data, auto& b) noexcept {
			b.writeCharacter('[');
			const auto is_empty = [&]() -> bool {
				if constexpr (HasSize<OTy>) {
					return data.size() ? false : true;
				} else {
					return data.empty();
				}
			}();

			if (!is_empty) {
				auto it = data.begin();
				Write::op(*it, b);
				++it;
				const auto end = data.end();
				for (; it != end; ++it) {
					b.writeCharacter(',');
					Write::op(*it, b);
				}
			}
			b.writeCharacter(']');
		}
	};

	template<typename OTy>
	requires JsonifierArrayT<std::decay_t<OTy>>
	struct ToJson<OTy> {
		template<typename OTy> static void op(OTy& data, auto& b) noexcept {
			static constexpr auto N = []() constexpr {
				if constexpr (JsonifierArrayT<std::decay_t<OTy>>) {
					return std::tuple_size_v<CoreT<std::decay_t<OTy>>>;
				} else {
					return std::tuple_size_v<std::decay_t<OTy>>;
				}
			}
			();

			b.writeCharacter('[');
			using VTy = std::decay_t<OTy>;
			forEach<N>([&](auto I) {
				if constexpr (JsonifierArrayT<VTy>) {
					Write::op(getMember(data, Tuplet::get<I>(CoreV<OTy>)), b);
				} else {
					Write::op(Tuplet::get<I>(data), b);
				}
				constexpr bool needs_comma = I < N - 1;
				if constexpr (needs_comma) {
					b.writeCharacter(',');
				}
			});
			b.writeCharacter(']');
		}
	};

	template<typename OTy>
	requires JsonifierObjectT<OTy> || MapT<OTy>
	struct ToJson<OTy> {
		template<typename OTy> static void op(OTy& data, auto& b) noexcept {
			b.writeCharacter('{');

			using VTy = std::decay_t<OTy>;
			static constexpr auto N = std::tuple_size_v<CoreT<VTy>>;

			bool first = true;
			forEach<N>([&](auto I) {
				static constexpr auto item = Tuplet::get<I>(CoreV<VTy>);

				if (first) {
					first = false;
				} else {
					b.writeCharacter(',');
				}

				using Key = typename std::decay_t<std::tuple_element_t<0, decltype(item)>>;

				static constexpr Key key = static_cast<Key>(Tuplet::get<0>(item));
				b.writeCharacter('\"');
				b.writeCharacters(key.data(), key.size());
				b.writeCharacter('\"');
				b.writeCharacter(':');

				Write::op(getMember(data, Tuplet::get<1>(item)), b);
			});
			b.writeCharacter('}');
		}
	};
}