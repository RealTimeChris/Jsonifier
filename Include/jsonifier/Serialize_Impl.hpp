/*
	Jsonifier - For parsing and serializing Json - very rapidly.
	Copyright (C) 2023 Chris M. (RealTimeChris)

	This library is free software; you can redistribute buffer and/or
	modify buffer under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	This library is distributed in the hope that buffer will be useful,
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
#include <jsonifier/Serialize.hpp>
#include <algorithm>

namespace Jsonifier {

	template<typename OTy>
	concept Accessible = requires(OTy container) {
							 { container[size_t{}] };
						 };

	template<typename OTy>
	concept VectorLike = Resizeable<OTy> && Accessible<OTy> && HasData<OTy>;

	template<char c, VectorLike BTy> inline void writeCharacters(BTy& b, size_t& ix) noexcept {
		if (ix + 3 >= b.size()) [[unlikely]] {
			b.resize((ix + 1) * 2);
		}
		b[ix++] = c;
	}

	template<char c, VectorLike BTy> inline void writeCharactersUnChecked(BTy& b, size_t& ix) noexcept {
		b[ix++] = c;
	}

	template<StringLiteral str, VectorLike BTy> inline void writeCharacters(BTy& b, size_t& ix) noexcept {
		static constexpr auto s = str.sv();
		static constexpr auto n = s.size();
		if (ix >= b.size()) [[unlikely]] {
			b.resize((ix + 1) * 2);
		}
		std::memcpy(b.data() + ix, s.data(), n);
		ix += n;
	}

	template<const std::string_view& str, VectorLike BTy> inline void writeCharacters(BTy& b, size_t& ix) noexcept {
		if (ix + str.size() >= b.size()) [[unlikely]] {
			b.resize((ix + str.size()) * 2);
		}

		std::memcpy(b.data() + ix, str.data(), str.size());
		ix += str.size();
	}

	template<BoolT OTy> struct SerializeImpl<OTy> {
		template<VectorLike BTy> inline static void op(OTy& value, BTy& buffer, size_t& ix) noexcept {
			if (value) {
				writeCharacters<"true">(buffer, ix);
			} else {
				writeCharacters<"false">(buffer, ix);
			}
		}
	};

	template<NumT OTy> struct SerializeImpl<OTy> {
		template<VectorLike BTy> inline static void op(OTy& value, BTy& buffer, size_t& ix) noexcept {
			if (ix + 64 > buffer.size()) [[unlikely]] {
				buffer.resize(buffer.size() + 64);
			}
			auto start = dataPtr(buffer) + ix;
			auto end = toChars(start, value);
			ix += std::distance(start, end);
		}
	};

	template<IsEnum OTy> struct SerializeImpl<OTy> {
		template<VectorLike BTy> inline static void op(OTy& value, BTy& buffer, size_t& ix) noexcept {
			if (ix + 64 > buffer.size()) [[unlikely]] {
				buffer.resize(buffer.size() + 64);
			}
			auto start = dataPtr(buffer) + ix;
			auto end = toChars(start, static_cast<int64_t>(value));
			ix += std::distance(start, end);
		}
	};

	template<typename OTy>
		requires StringT<OTy> || CharT<OTy>
	struct SerializeImpl<OTy> {
		template<VectorLike BTy> inline static void op(OTy& value, BTy& buffer, size_t& ix) noexcept {
			if constexpr (CharT<OTy>) {
				writeCharacters<'"'>(buffer, ix);
				switch (value) {
					case '"':
						writeCharacters<"\\\"">(buffer, ix);
						break;
					case '\\':
						writeCharacters<"\\\\">(buffer, ix);
						break;
					case '\b':
						writeCharacters<"\\b">(buffer, ix);
						break;
					case '\f':
						writeCharacters<"\\f">(buffer, ix);
						break;
					case '\n':
						writeCharacters<"\\n">(buffer, ix);
						break;
					case '\r':
						writeCharacters<"\\r">(buffer, ix);
						break;
					case '\t':
						writeCharacters<"\\t">(buffer, ix);
						break;
					default:
						writeCharacters(value, buffer, ix);
				}
				writeCharacters<'"'>(buffer, ix);
			} else {
				const auto n = value.size();

				if constexpr (Resizeable<BTy>) {
					if ((ix + (4 * n)) >= buffer.size()) [[unlikely]] {
						buffer.resize(ix + (4 * n));
					}
				}

				writeCharactersUnChecked<'"'>(buffer, ix);

				for (auto&& c: value) {
					switch (c) {
						case '"':
							buffer[ix++] = '\\';
							buffer[ix++] = '\"';
							break;
						case '\\':
							buffer[ix++] = '\\';
							buffer[ix++] = '\\';
							break;
						case '\b':
							buffer[ix++] = '\\';
							buffer[ix++] = 'b';
							break;
						case '\f':
							buffer[ix++] = '\\';
							buffer[ix++] = 'f';
							break;
						case '\n':
							buffer[ix++] = '\\';
							buffer[ix++] = 'n';
							break;
						case '\r':
							buffer[ix++] = '\\';
							buffer[ix++] = 'r';
							break;
						case '\t':
							buffer[ix++] = '\\';
							buffer[ix++] = 't';
							break;
						default:
							buffer[ix++] = c;
					}
				}

				writeCharactersUnChecked<'"'>(buffer, ix);
			}
		}
	};

	template<ArrayT OTy> struct SerializeImpl<OTy> {
		template<VectorLike BTy> inline static void op(OTy& value, BTy& buffer, size_t& ix) noexcept {
			writeCharacters<'['>(buffer, ix);
			const auto is_empty = [&]() -> bool {
				if constexpr (HasSize<OTy>) {
					return value.size() ? false : true;
				} else {
					return value.empty();
				}
			}();

			if (!is_empty) {
				auto it = value.begin();
				Serialize::op(*it, buffer, ix);
				++it;
				const auto end = value.end();
				for (; it != end; ++it) {
					writeCharacters<','>(buffer, ix);
					Serialize::op(*it, buffer, ix);
				}
			}
			writeCharacters<']'>(buffer, ix);
		}
	};

	template<MapT OTy> struct SerializeImpl<OTy> {
		template<VectorLike BTy> inline static void op(OTy& value, BTy& buffer, size_t& ix) noexcept {
			writeCharacters<'{'>(buffer, ix);
			if (!value.empty()) {
				auto it = value.cbegin();
				auto write_pair = [&] {
					using Key = typename OTy::key_type;
					if constexpr (StringT<Key> || CharT<Key>) {
						Serialize::op(it->first, buffer, ix);
					} else {
						writeCharacters<'"'>(buffer, ix);
						Serialize::op(it->first, buffer, ix);
						writeCharacters<'"'>(buffer, ix);
					}
					writeCharacters<':'>(buffer, ix);
					Serialize::op(it->second, buffer, ix);
				};
				write_pair();
				++it;

				const auto end = value.cend();
				for (; it != end; ++it) {
					using Value = typename OTy::mapped_type;
					if constexpr (NullableT<Value>) {
						if (!bool(it->second))
							continue;
					}
					writeCharacters<','>(buffer, ix);
					write_pair();
				}
			}
			writeCharacters<'}'>(buffer, ix);
		}
	};

	template<typename OTy>
		requires JsonifierArrayT<std::decay_t<OTy>> || TupleT<std::decay_t<OTy>>
	struct SerializeImpl<OTy> {
		template<VectorLike BTy> inline static void op(OTy& value, BTy& buffer, size_t& ix) noexcept {
			static constexpr auto N = []() constexpr {
				if constexpr (JsonifierArrayT<std::decay_t<OTy>>) {
					return std::tuple_size_v<CoreT<std::decay_t<OTy>>>;
				} else {
					return std::tuple_size_v<std::decay_t<OTy>>;
				}
			}();

			writeCharacters<'['>(buffer, ix);
			using V = std::decay_t<OTy>;
			forEach<N>([&](auto I) {
				if constexpr (JsonifierArrayT<V>) {
					Serialize::op(getMember(value, Tuplet::get<I>(CoreV<OTy>)), buffer, ix);
				} else {
					Serialize::op(Tuplet::get<I>(value), buffer, ix);
				}
				constexpr bool needs_comma = I < N - 1;
				if constexpr (needs_comma) {
					writeCharacters<','>(buffer, ix);
				}
			});
			writeCharacters<']'>(buffer, ix);
		}
	};

	template<typename OTy>
		requires IsStdTuple<std::decay_t<OTy>>
	struct SerializeImpl<OTy> {
		template<VectorLike BTy> inline static void op(OTy& value, BTy& buffer, size_t& ix) noexcept {
			static constexpr auto N = []() constexpr {
				if constexpr (JsonifierArrayT<std::decay_t<OTy>>) {
					return std::tuple_size_v<CoreT<std::decay_t<OTy>>>;
				} else {
					return std::tuple_size_v<std::decay_t<OTy>>;
				}
			}();

			writeCharacters<'['>(buffer, ix);
			using V = std::decay_t<OTy>;
			forEach<N>([&](auto I) {
				if constexpr (JsonifierArrayT<V>) {
					Serialize::op(value.*std::get<I>(CoreV<V>), buffer, ix);
				} else {
					Serialize::op(std::get<I>(value), buffer, ix);
				}
				constexpr bool needs_comma = I < N - 1;
				if constexpr (needs_comma) {
					writeCharacters<','>(buffer, ix);
				}
			});
			writeCharacters<']'>(buffer, ix);
		}
	};

	inline constexpr bool needsEscaping(const auto& S) noexcept {
		for (const auto& c: S) {
			if (c == '"') {
				return true;
			}
		}
		return false;
	}

	template<typename OTy>
		requires JsonifierObjectT<OTy>
	struct SerializeImpl<OTy> {
		template<VectorLike BTy> inline static void op(OTy& value, BTy& buffer, size_t& ix) noexcept {
			writeCharacters<'{'>(buffer, ix);
			using V = std::decay_t<OTy>;
			static constexpr auto N = std::tuple_size_v<CoreT<V>>;

			bool first = true;
			forEach<N>([&](auto I) {
				static constexpr auto item = Tuplet::get<I>(CoreV<V>);
				using ItemType = decltype(item);
				using MPtrT = std::tuple_element_t<1, ItemType>;
				using ValT = MemberT<V, MPtrT>;

				if constexpr (NullableT<ValT>) {
					auto is_null = [&]() {
						if constexpr (std::is_member_pointer_v<std::tuple_element_t<1, ItemType>>) {
							return !bool(value.*Tuplet::get<1>(item));
						} else {
							return !bool(Tuplet::get<1>(item)(value));
						}
					}();
					if (is_null)
						return;
				}

				if (first) {
					first = false;
				} else {
					writeCharacters<','>(buffer, ix);
				}

				using Key = typename std::decay_t<std::tuple_element_t<0, ItemType>>;

				if constexpr (StringT<Key> || CharT<Key>) {
					static constexpr Key key = Tuplet::get<0>(item);
					if constexpr (needsEscaping(key)) {
						Serialize::op(key.sv(), buffer, ix);
						writeCharacters<':'>(buffer, ix);
					} else {
						static constexpr auto quoted = JoinV<Chars<"\"">, key, Chars<"\":">>;
						writeCharacters<quoted>(buffer, ix);
					}
				} else {
					static constexpr auto quoted = concatArrays(concatArrays("\"", Tuplet::get<0>(item)), "\":", "");
					Serialize::op(quoted, buffer, ix);
				}

				Serialize::op(getMember(value, Tuplet::get<1>(item)), buffer, ix);
			});
			writeCharacters<'}'>(buffer, ix);
		}
	};
}