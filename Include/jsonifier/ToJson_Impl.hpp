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
#include <algorithm>

namespace Jsonifier {

	template<class T>
	concept Accessible = requires(T container) {
							 { container[size_t{}] };
						 };

	template<class T>
	concept VectorLike = Resizeable<T> && Accessible<T> && HasData<T>;

	inline void dump(const char c, auto& b, auto& ix) noexcept {
		if (ix == b.size()) [[unlikely]] {
			b.resize(b.size() * 2);
		}

		b[ix] = c;
		++ix;
	}

	inline void dump(const char c, char*& b) noexcept {
		*b = c;
		++b;
	}

	template<char c> inline void dump(auto& b, auto& ix) noexcept {
		if (ix >= b.size()) [[unlikely]] {
			b.resize((b.size() + 1) * 2);
		}
		using V = std::decay_t<decltype(b[0])>;
		if constexpr (std::same_as<V, char>) {
			b[ix] = c;
		} else {
			b[ix] = static_cast<V>(c);
		}

		++ix;
	}

	template<char c> inline void dump(auto* b, auto& ix) noexcept {
		b[ix] = c;
		++ix;
	}

	template<char c> inline void dump_unchecked(auto& b, auto& ix) noexcept {
		using V = std::decay_t<decltype(b[0])>;
		if constexpr (std::same_as<V, char>) {
			b[ix] = c;
		} else {
			b[ix] = static_cast<V>(c);
		}
		++ix;
	}

	template<char c> inline void dump_unchecked(auto* b, auto& ix) noexcept {
		b[ix] = c;
		++ix;
	}

	template<char c> inline void dump(char*& b) noexcept {
		*b = c;
		++b;
	}

	template<StringLiteral str> inline void dump(char*& b) noexcept {
		static constexpr auto s = str.string_view();
		for (auto& c: s) {
			*b = c;
			++b;
		}
	}

	template<char c> inline void dumpn(size_t n, char*& b) noexcept {
		std::fill_n(b, n, c);
	}

	template<StringLiteral str> inline void dump(auto& b, auto& ix) noexcept {
		static constexpr auto s = str.string_view();
		static constexpr auto n = s.size();

		if (ix + n > b.size()) [[unlikely]] {
			b.resize((std::max)(b.size() * 2, ix + n));
		}

		std::memcpy(b.data() + ix, s.data(), n);
		ix += n;
	}

	template<StringLiteral str> inline void dump(auto* b, auto& ix) noexcept {
		static constexpr auto s = str.string_view();
		static constexpr auto n = s.size();

		std::memcpy(b + ix, s.data(), n);
		ix += n;
	}

	template<char c> inline void dumpn(size_t n, auto& b, auto& ix) noexcept {
		if (ix + n > b.size()) [[unlikely]] {
			b.resize((std::max)(b.size() * 2, ix + n));
		}

		std::fill_n(b.data() + ix, n, c);
		ix += n;
	}

	template<const std::string_view& str> inline void dump(auto& b, auto& ix) noexcept {
		static constexpr auto s = str;
		static constexpr auto n = s.size();

		if (ix + n > b.size()) [[unlikely]] {
			b.resize((std::max)(b.size() * 2, ix + n));
		}

		std::memcpy(b.data() + ix, s.data(), n);
		ix += n;
	}

	template<const std::string_view& str> inline void dump(auto* b, auto& ix) noexcept {
		static constexpr auto s = str;
		static constexpr auto n = s.size();

		std::memcpy(b + ix, s.data(), n);
		ix += n;
	}

	inline void dump(const std::string_view str, auto& b, auto& ix) noexcept {
		const auto n = str.size();
		if (ix + n > b.size()) [[unlikely]] {
			b.resize((std::max)(b.size() * 2, ix + n));
		}

		std::memcpy(b.data() + ix, str.data(), n);
		ix += n;
	}

	inline void dump(const std::string_view str, auto* b, auto& ix) noexcept {
		const auto n = str.size();

		std::memcpy(b + ix, str.data(), n);
		ix += n;
	}

	inline void dump(const std::string_view str, char*& b) noexcept {
		for (auto& c: str) {
			*b = c;
			++b;
		}
	}

	template<std::byte c, class B> inline void dump(B&& b) noexcept {
		using value_t = RangeValueT<std::decay_t<B>>;
		if constexpr (std::same_as<value_t, std::byte>) {
			b.emplace_back(c);
		} else {
			static constexpr std::byte chr = c;
			static_assert(sizeof(value_t) == sizeof(std::byte));
			b.push_back(*reinterpret_cast<value_t*>(const_cast<std::byte*>(&chr)));
		}
	}

	template<std::byte c, class B> inline void dump(B&& b, auto& ix) noexcept {
		if (ix == b.size()) [[unlikely]] {
			b.resize(b.size() * 2);
		}

		using value_t = RangeValueT<std::decay_t<B>>;
		if constexpr (std::same_as<value_t, std::byte>) {
			b[ix] = c;
		} else {
			static constexpr std::byte chr = c;
			static_assert(sizeof(value_t) == sizeof(std::byte));
			b[ix] = *reinterpret_cast<value_t*>(const_cast<std::byte*>(&chr));
		}
		++ix;
	}

	template<class B> inline void dump(std::byte c, B&& b) noexcept {
		using value_t = RangeValueT<std::decay_t<B>>;
		if constexpr (std::same_as<value_t, std::byte>) {
			b.emplace_back(c);
		} else {
			static_assert(sizeof(value_t) == sizeof(std::byte));
			b.push_back(*reinterpret_cast<value_t*>(const_cast<std::byte*>(&c)));
		}
	}

	template<class B> inline void dump(std::byte c, auto&& b, auto& ix) noexcept {
		if (ix == b.size()) [[unlikely]] {
			b.resize(b.size() * 2);
		}

		using value_t = RangeValueT<std::decay_t<B>>;
		if constexpr (std::same_as<value_t, std::byte>) {
			b[ix] = c;
		} else {
			static_assert(sizeof(value_t) == sizeof(std::byte));
			b[ix] = *reinterpret_cast<value_t*>(const_cast<std::byte*>(&c));
		}
		++ix;
	}

	template<class B> inline void dump(const std::span<const std::byte> bytes, B&& b) noexcept {
		const auto n = bytes.size();
		const auto b_start = b.size();
		b.resize(b.size() + n);
		std::memcpy(b.data() + b_start, bytes.data(), n);
	}

	template<class B> inline void dump(const std::span<const std::byte> bytes, B&& b, auto& ix) noexcept {
		const auto n = bytes.size();
		if (ix + n > b.size()) [[unlikely]] {
			b.resize((std::max)(b.size() * 2, ix + n));
		}

		std::memcpy(b.data() + ix, bytes.data(), n);
		ix += n;
	}

	template<size_t N, class B> inline void dump(const std::array<uint8_t, N>& bytes, B&& b, auto& ix) noexcept {
		if (ix + N > b.size()) [[unlikely]] {
			b.resize((std::max)(b.size() * 2, ix + N));
		}

		std::memcpy(b.data() + ix, bytes.data(), N);
		ix += N;
	}

	template<BoolT T> struct ToJson<T> {
		template<class... Args> inline static void op(const bool value, Args&&... args) noexcept {
			if (value) {
				dump<"true">(std::forward<Args>(args)...);
			} else {
				dump<"false">(std::forward<Args>(args)...);
			}
		}
	};

	template<class T> inline constexpr auto sized_integer_conversion() noexcept {
		if constexpr (std::is_signed_v<T>) {
			if constexpr (sizeof(T) <= 32) {
				return int32_t{};
			} else if constexpr (sizeof(T) <= 64) {
				return int64_t{};
			} else {
				static_assert(FalseV<T>, "type is not supported");
			}
		} else {
			if constexpr (sizeof(T) <= 32) {
				return uint32_t{};
			} else if constexpr (sizeof(T) <= 64) {
				return uint64_t{};
			} else {
				static_assert(FalseV<T>, "type is not supported");
			}
		}
	}

	struct write_chars {
		template<class B> static void op(NumT auto& value, B&& b, auto& ix) noexcept {
			using V = std::decay_t<decltype(value)>;

			if constexpr (std::_Is_any_of_v<V, float, double, int32_t, uint32_t, int64_t, uint64_t>) {
			} else if constexpr (std::integral<V>) {
				using X = std::decay_t<decltype(sized_integer_conversion<V>())>;
				auto start = data_ptr(b) + ix;
				auto end = toChars(start, static_cast<X>(value));
				ix += std::distance(start, end);
			} else {
				static_assert(FalseV<V>, "type is not supported");
			}
		}
	};

	template<NumT T> struct ToJson<T> {
		template<class B> inline static void op(auto& value, B&& b, auto& ix) noexcept {
			if constexpr (Resizeable<B>) {
				if (ix + 64 > b.size()) [[unlikely]] {
					auto newValue = ix + 64;
					auto newValue01 = b.size() * 2;
					b.resize(newValue > newValue01 ? newValue : newValue01);
				}
			}
			auto start = data_ptr(b) + ix;
			auto end = toChars(start, value);
			ix += std::distance(start, end);
		}
	};

	template<class T>
		requires StringT<T> || CharT<T>
	struct ToJson<T> {
		template<class B> inline static void op(auto& value, B&& b, auto& ix) noexcept {
			if constexpr (CharT<T>) {
				dump<'"'>(b, ix);
				switch (value) {
					case '"':
						dump<"\\\"">(b, ix);
						break;
					case '\\':
						dump<"\\\\">(b, ix);
						break;
					case '\b':
						dump<"\\b">(b, ix);
						break;
					case '\f':
						dump<"\\f">(b, ix);
						break;
					case '\n':
						dump<"\\n">(b, ix);
						break;
					case '\r':
						dump<"\\r">(b, ix);
						break;
					case '\t':
						dump<"\\t">(b, ix);
						break;
					default:
						dump(value, b, ix);
				}
				dump<'"'>(b, ix);
			} else {
				const std::string_view str = value;
				const auto n = str.size();

				if constexpr (Resizeable<B>) {
					if ((ix + 4 * n) >= b.size()) [[unlikely]] {
						b.resize((std::max)(b.size() * 2, ix + 4 * n));
					}
				}

				dump_unchecked<'"'>(b, ix);

				using V = std::decay_t<decltype(b[0])>;
				if constexpr (std::same_as<V, std::byte>) {
					for (auto&& c: str) {
						switch (c) {
							case '"':
								b[ix++] = static_cast<std::byte>('\\');
								b[ix++] = static_cast<std::byte>('\"');
								break;
							case '\\':
								b[ix++] = static_cast<std::byte>('\\');
								b[ix++] = static_cast<std::byte>('\\');
								break;
							case '\b':
								b[ix++] = static_cast<std::byte>('\\');
								b[ix++] = static_cast<std::byte>('b');
								break;
							case '\f':
								b[ix++] = static_cast<std::byte>('\\');
								b[ix++] = static_cast<std::byte>('f');
								break;
							case '\n':
								b[ix++] = static_cast<std::byte>('\\');
								b[ix++] = static_cast<std::byte>('n');
								break;
							case '\r':
								b[ix++] = static_cast<std::byte>('\\');
								b[ix++] = static_cast<std::byte>('r');
								break;
							case '\t':
								b[ix++] = static_cast<std::byte>('\\');
								b[ix++] = static_cast<std::byte>('t');
								break;
							default:
								b[ix++] = static_cast<std::byte>(c);
						}
					}
				} else {
					for (auto&& c: str) {
						switch (c) {
							case '"':
								b[ix++] = '\\';
								b[ix++] = '\"';
								break;
							case '\\':
								b[ix++] = '\\';
								b[ix++] = '\\';
								break;
							case '\b':
								b[ix++] = '\\';
								b[ix++] = 'b';
								break;
							case '\f':
								b[ix++] = '\\';
								b[ix++] = 'f';
								break;
							case '\n':
								b[ix++] = '\\';
								b[ix++] = 'n';
								break;
							case '\r':
								b[ix++] = '\\';
								b[ix++] = 'r';
								break;
							case '\t':
								b[ix++] = '\\';
								b[ix++] = 't';
								break;
							default:
								b[ix++] = c;
						}
					}
				}

				dump_unchecked<'"'>(b, ix);
			}
		}
	};

	template<ArrayT T> struct ToJson<T> {
		template<class... Args> inline static void op(auto& value, Args&&... args) noexcept {
			dump<'['>(std::forward<Args>(args)...);
			const auto is_empty = [&]() -> bool {
				if constexpr (HasSize<T>) {
					return value.size() ? false : true;
				} else {
					return value.empty();
				}
			}();

			if (!is_empty) {
				auto it = value.begin();
				Write::op(*it, std::forward<Args>(args)...);
				++it;
				const auto end = value.end();
				for (; it != end; ++it) {
					dump<','>(std::forward<Args>(args)...);
					Write::op(*it, std::forward<Args>(args)...);
				}
			}
			dump<']'>(std::forward<Args>(args)...);
		}
	};

	template<MapT T> struct ToJson<T> {
		template<class... Args> inline static void op(auto& value, Args&&... args) noexcept {
			dump<'{'>(std::forward<Args>(args)...);
			if (!value.empty()) {
				auto it = value.cbegin();
				auto write_pair = [&] {
					using Key = decltype(it->first);
					if constexpr (StringT<Key> || CharT<Key>) {
						Write::op(it->first, std::forward<Args>(args)...);
					} else {
						dump<'"'>(std::forward<Args>(args)...);
						Write::op(it->first, std::forward<Args>(args)...);
						dump<'"'>(std::forward<Args>(args)...);
					}
					dump<':'>(std::forward<Args>(args)...);
					Write::op(it->second, std::forward<Args>(args)...);
				};
				write_pair();
				++it;

				const auto end = value.cend();
				for (; it != end; ++it) {
					using Value = std::decay_t<decltype(it->second)>;
					if constexpr (NullableT<Value>) {
						if (!bool(it->second))
							continue;
					}
					dump<','>(std::forward<Args>(args)...);
					write_pair();
				}
			}
			dump<'}'>(std::forward<Args>(args)...);
		}
	};

	template<class T>
		requires JsonifierArrayT<std::decay_t<T>> || TupleT<std::decay_t<T>>
	struct ToJson<T> {
		template<class... Args> inline static void op(auto& value, Args&&... args) noexcept {
			static constexpr auto N = []() constexpr {
				if constexpr (JsonifierArrayT<std::decay_t<T>>) {
					return std::tuple_size_v<CoreT<std::decay_t<T>>>;
				} else {
					return std::tuple_size_v<std::decay_t<T>>;
				}
			}();

			dump<'['>(std::forward<Args>(args)...);
			using V = std::decay_t<T>;
			forEach<N>([&](auto I) {
				if constexpr (JsonifierArrayT<V>) {
					Write::op(get_member(value, Tuplet::get<I>(CoreV<T>)), std::forward<Args>(args)...);
				} else {
					Write::op(Tuplet::get<I>(value), std::forward<Args>(args)...);
				}
				constexpr bool needs_comma = I < N - 1;
				if constexpr (needs_comma) {
					dump<','>(std::forward<Args>(args)...);
				}
			});
			dump<']'>(std::forward<Args>(args)...);
		}
	};

	template<class T>
		requires IsStdTuple<std::decay_t<T>>
	struct ToJson<T> {
		template<class... Args> inline static void op(auto& value, Args&&... args) noexcept {
			static constexpr auto N = []() constexpr {
				if constexpr (JsonifierArrayT<std::decay_t<T>>) {
					return std::tuple_size_v<CoreT<std::decay_t<T>>>;
				} else {
					return std::tuple_size_v<std::decay_t<T>>;
				}
			}();

			dump<'['>(std::forward<Args>(args)...);
			using V = std::decay_t<T>;
			forEach<N>([&](auto I) {
				if constexpr (JsonifierArrayT<V>) {
					Write::op(value.*std::get<I>(CoreV<V>), std::forward<Args>(args)...);
				} else {
					Write::op(std::get<I>(value), std::forward<Args>(args)...);
				}
				constexpr bool needs_comma = I < N - 1;
				if constexpr (needs_comma) {
					dump<','>(std::forward<Args>(args)...);
				}
			});
			dump<']'>(std::forward<Args>(args)...);
		}
	};

	template<const std::string_view& S> inline constexpr auto arrayFromSv() noexcept {
		constexpr auto s = S;
		constexpr auto N = s.size();
		std::array<char, N> arr;
		std::copy_n(s.data(), N, arr.data());
		return arr;
	}

	inline constexpr bool needs_escaping(const auto& S) noexcept {
		for (const auto& c: S) {
			if (c == '"') {
				return true;
			}
		}
		return false;
	}

	template<class T>
		requires JsonifierObjectT<T>
	struct ToJson<T> {
		inline static void op(auto& value, auto&& b, auto& ix) noexcept {
			dump<'{'>(b, ix);
			using V = std::decay_t<T>;
			static constexpr auto N = std::tuple_size_v<CoreT<V>>;

			bool first = true;
			forEach<N>([&](auto I) {
				static constexpr auto item = Tuplet::get<I>(CoreV<V>);
				using mptr_t = std::tuple_element_t<1, decltype(item)>;
				using val_t = MemberT<V, mptr_t>;

				if constexpr (NullableT<val_t>) {
					auto is_null = [&]() {
						if constexpr (std::is_member_pointer_v<std::tuple_element_t<1, decltype(item)>>) {
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
					dump<','>(b, ix);
				}

				using Key = typename std::decay_t<std::tuple_element_t<0, decltype(item)>>;

				if constexpr (StringT<Key> || CharT<Key>) {
					static constexpr std::string_view key = Tuplet::get<0>(item);
					if constexpr (needs_escaping(key)) {
						Write::op(key, b, ix);
						dump<':'>(b, ix);
					} else {
						static constexpr auto quoted = JoinV<Chars<"\"">, key, Chars<"\":">>;
						dump<quoted>(b, ix);
					}
				} else {
					static constexpr auto quoted = concat_arrays(concat_arrays("\"", Tuplet::get<0>(item)), "\":", "");
					Write::op(quoted, b, ix);
				}

				Write::op(getMember(value, Tuplet::get<1>(item)), b, ix);
			});
			dump<'}'>(b, ix);
		}
	};
}