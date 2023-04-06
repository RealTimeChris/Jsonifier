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

#include <jsonifier/Serializer.hpp>
#include <jsonifier/Parser.hpp>
#include <jsonifier/Base.hpp>
#include <algorithm>

namespace Jsonifier {

	inline void writeCharacter(char c, VectorLike auto& buffer, auto& index) {
		if (index >= buffer.size()) [[unlikely]] {
			buffer.resize((buffer.size() + 1) * 2);
		}

		buffer[index] = c;
		++index;
	}

	inline void writeCharacterUnChecked(char c, VectorLike auto& buffer, auto& index) {
		buffer[index] = c;
		++index;
	}

	template<const StringView& str> inline void writeCharacters(VectorLike auto& buffer, auto& index) {
		static constexpr auto s = str;
		static constexpr auto n = s.size();

		if (index + n > buffer.size()) [[unlikely]] {
			buffer.resize(std::max(buffer.size() * 2, index + n));
		}

		std::memcpy(buffer.data() + index, s.data(), n);
		index += n;
	}

	template<StringLiteral str> inline void writeCharacters(VectorLike auto& buffer, auto& index) {
		static constexpr auto s = str;
		static constexpr auto n = s.size();

		if (index + n > buffer.size()) [[unlikely]] {
			buffer.resize(std::max(buffer.size() * 2, index + n));
		}

		std::memcpy(buffer.data() + index, s.data(), n);
		index += n;
	}

	inline void writeCharacters(const StringView str, VectorLike auto& buffer, auto& index) {
		const auto n = str.size();
		if (index + n > buffer.size()) [[unlikely]] {
			buffer.resize(std::max(buffer.size() * 2, index + n));
		}

		std::memcpy(buffer.data() + index, str.data(), n);
		index += n;
	}

	template<BoolT OTy, VectorLike BTy> void SerializeNoKeys::op(OTy& value, BTy& buffer, size_t& index) {
		if (value) {
			writeCharacters<"true">(buffer, index);
		} else {
			writeCharacters<"false">(buffer, index);
		}
	}

	template<NumT OTy, VectorLike BTy> void SerializeNoKeys::op(OTy& value, BTy& buffer, size_t& index) {
		if (index + 32 > buffer.size()) [[unlikely]] {
			buffer.resize((std::max)(buffer.size() * 2, index + 64));
		}
		auto start = dataPtr(buffer) + index;
		auto end = toChars(start, value);
		index += std::distance(start, end);
	}

	template<EnumT OTy, VectorLike BTy> void SerializeNoKeys::op(OTy& value, BTy& buffer, size_t& index) {
		if (index + 32 > buffer.size()) [[unlikely]] {
			buffer.resize((std::max)(buffer.size() * 2, index + 64));
		}
		auto start = dataPtr(buffer) + index;
		auto end = toChars(start, static_cast<int64_t>(value));
		index += std::distance(start, end);
	}

	template<CharT OTy, VectorLike BTy> void SerializeNoKeys::op(OTy& value, BTy& buffer, size_t& index) {
		writeCharacter('"', buffer, index);
		switch (value) {
			case '"':
				writeCharacters<"\\\"">(buffer, index);
				break;
			case '\\':
				writeCharacters<"\\\\">(buffer, index);
				break;
			case '\b':
				writeCharacters<"\\b">(buffer, index);
				break;
			case '\f':
				writeCharacters<"\\f">(buffer, index);
				break;
			case '\n':
				writeCharacters<"\\n">(buffer, index);
				break;
			case '\r':
				writeCharacters<"\\r">(buffer, index);
				break;
			case '\t':
				writeCharacters<"\\t">(buffer, index);
				break;
			default:
				writeCharacter(value, buffer, index);
		}
		writeCharacter('"', buffer, index);
	}

	template<StringT OTy, VectorLike BTy> void SerializeNoKeys::op(OTy& value, BTy& buffer, size_t& index) {
		const auto string = static_cast<StringView>(value);
		const auto n = string.size();

		if constexpr (Resizeable<BTy>) {
			if ((index + (4 * n)) >= buffer.size()) [[unlikely]] {
				buffer.resize(std::max(buffer.size() * 2, index + (4 * n)));
			}
		}

		writeCharacterUnChecked('"', buffer, index);

		for (auto&& c: string) {
			switch (c) {
				case '"':
					buffer[index++] = '\\';
					buffer[index++] = '\"';
					break;
				case '\\':
					buffer[index++] = '\\';
					buffer[index++] = '\\';
					break;
				case '\b':
					buffer[index++] = '\\';
					buffer[index++] = 'b';
					break;
				case '\f':
					buffer[index++] = '\\';
					buffer[index++] = 'f';
					break;
				case '\n':
					buffer[index++] = '\\';
					buffer[index++] = 'n';
					break;
				case '\r':
					buffer[index++] = '\\';
					buffer[index++] = 'r';
					break;
				case '\t':
					buffer[index++] = '\\';
					buffer[index++] = 't';
					break;
				default:
					buffer[index++] = c;
			}
		}

		writeCharacterUnChecked('"', buffer, index);
	}

	template<ArrayTupleT OTy, VectorLike BTy> void SerializeNoKeys::op(OTy& value, BTy& buffer, size_t& index) {
		static constexpr auto N = []() constexpr {
			if constexpr (JsonifierArrayT<std::decay_t<OTy>>) {
				return std::tuple_size_v<CoreT<std::decay_t<OTy>>>;
			} else {
				return std::tuple_size_v<std::decay_t<OTy>>;
			}
		}();

		writeCharacter('[', buffer, index);
		using V = std::decay_t<OTy>;
		forEach<N>([&](auto I) {
			if constexpr (JsonifierArrayT<V>) {
				SerializeNoKeys::op(getMember(value, Tuplet::get<I>(CoreV<OTy>)), buffer, index);
			} else {
				SerializeNoKeys::op(Tuplet::get<I>(value), buffer, index);
			}
			constexpr bool needsComma = I < N - 1;
			if constexpr (needsComma) {
				writeCharacter(',', buffer, index);
			}
		});
		writeCharacter(']', buffer, index);
	}

	template<RawArrayT OTy, VectorLike BTy> void SerializeNoKeys::op(OTy& value, BTy& buffer, size_t& index) {
		const auto n = value.size();
		writeCharacter('[', buffer, index);
		for (size_t x = 0; x < n; ++x) {
			SerializeNoKeys::op(value[x], buffer, index);
			const bool needsComma = x < n - 1;
			if (needsComma) {
				writeCharacter(',', buffer, index);
			}
		}
		writeCharacter(']', buffer, index);
	}

	template<VectorT OTy, VectorLike BTy> void SerializeNoKeys::op(OTy& value, BTy& buffer, size_t& index) {
		writeCharacter('[', buffer, index);
		const auto isEmpty = [&]() -> bool {
			return value.size() ? false : true;
		}();

		if (!isEmpty) {
			auto it = value.begin();
			SerializeNoKeys::op(*it, buffer, index);
			++it;
			const auto end = value.end();
			for (; it != end; ++it) {
				writeCharacter(',', buffer, index);
				SerializeNoKeys::op(*it, buffer, index);
			}
		}
		writeCharacter(']', buffer, index);
	}

	template<StdTupleT OTy, VectorLike BTy> void SerializeNoKeys::op(OTy& value, BTy& buffer, size_t& index) {
		static constexpr auto n = []() constexpr {
			if constexpr (JsonifierArrayT<std::decay_t<OTy>>) {
				return std::tuple_size_v<CoreT<std::decay_t<OTy>>>;
			} else {
				return std::tuple_size_v<std::decay_t<OTy>>;
			}
		}();

		writeCharacter('[', buffer, index);
		using V = std::decay_t<OTy>;
		forEach<n>([&](auto x) {
			if constexpr (JsonifierArrayT<V>) {
				SerializeNoKeys::op(value.*std::get<x>(CoreV<V>), buffer, index);
			} else {
				SerializeNoKeys::op(std::get<x>(value), buffer, index);
			}
			constexpr bool needsComma = x < n - 1;
			if constexpr (needsComma) {
				writeCharacter(',', buffer, index);
			}
		});
		writeCharacter(']', buffer, index);
	}

	inline constexpr bool needsEscaping(const auto& S) {
		for (const auto& c: S) {
			if (c == '"') {
				return true;
			}
		}
		return false;
	}

	template<ObjectT OTy, VectorLike BTy> void SerializeNoKeys::op(OTy& value, BTy& buffer, size_t& index) {
		writeCharacter('{', buffer, index);
		using V = std::decay_t<OTy>;
		static constexpr auto n = std::tuple_size_v<CoreT<V>>;

		bool first = true;
		forEach<n>([&](auto x) {
			static constexpr auto item = Tuplet::get<x>(CoreV<V>);
			using ItemType = decltype(item);
			using MPtrT = std::tuple_element_t<1, ItemType>;
			using ValT = MemberT<V, MPtrT>;

			if constexpr (NullableT<ValT>) {
				auto isNull = [&]() {
					if constexpr (std::is_member_pointer_v<std::tuple_element_t<1, ItemType>>) {
						return !bool(value.*Tuplet::get<1>(item));
					} else {
						return !bool(Tuplet::get<1>(item)(value));
					}
				}();
				if (isNull)
					return;
			}

			if (first) {
				first = false;
			} else {
				writeCharacter(',', buffer, index);
			}

			using Key = typename std::decay_t<std::tuple_element_t<0, decltype(item)>>;

			if constexpr (StringT<Key> || CharT<Key>) {
				static constexpr StringView key = Tuplet::get<0>(item);
				if constexpr (needsEscaping(key)) {
					SerializeNoKeys::op(key, buffer, index);
					writeCharacter(':', buffer, index);
				} else {
					static constexpr auto quoted = JoinV<Chars<"\"">, key, Chars<"\":">>;
					writeCharacters<quoted>(buffer, index);
				}
			} else {
				static constexpr auto quoted = concatArrays(concatArrays("\"", Tuplet::get<0>(item)), "\":", "");
				SerializeNoKeys::op(quoted, buffer, index);
			}
			SerializeNoKeys::op(getMember(value, Tuplet::get<1>(item)), buffer, index);
		});
		writeCharacter('}', buffer, index);
	}

	template<BoolT OTy, VectorLike BTy> void SerializeWithKeys::op(OTy& value, BTy& buffer, size_t& index) {
		if (value) {
			writeCharacters<"true">(buffer, index);
		} else {
			writeCharacters<"false">(buffer, index);
		}
	}

	template<NumT OTy, VectorLike BTy> void SerializeWithKeys::op(OTy& value, BTy& buffer, size_t& index) {
		if (index + 32 > buffer.size()) [[unlikely]] {
			buffer.resize((std::max)(buffer.size() * 2, index + 64));
		}
		auto start = dataPtr(buffer) + index;
		auto end = toChars(start, value);
		index += std::distance(start, end);
	}

	template<EnumT OTy, VectorLike BTy> void SerializeWithKeys::op(OTy& value, BTy& buffer, size_t& index) {
		if (index + 32 > buffer.size()) [[unlikely]] {
			buffer.resize((std::max)(buffer.size() * 2, index + 64));
		}
		auto start = dataPtr(buffer) + index;
		auto end = toChars(start, static_cast<int64_t>(value));
		index += std::distance(start, end);
	}

	template<CharT OTy, VectorLike BTy> void SerializeWithKeys::op(OTy& value, BTy& buffer, size_t& index) {
		writeCharacter('"', buffer, index);
		switch (value) {
			case '"':
				writeCharacters<"\\\"">(buffer, index);
				break;
			case '\\':
				writeCharacters<"\\\\">(buffer, index);
				break;
			case '\b':
				writeCharacters<"\\b">(buffer, index);
				break;
			case '\f':
				writeCharacters<"\\f">(buffer, index);
				break;
			case '\n':
				writeCharacters<"\\n">(buffer, index);
				break;
			case '\r':
				writeCharacters<"\\r">(buffer, index);
				break;
			case '\t':
				writeCharacters<"\\t">(buffer, index);
				break;
			default:
				writeCharacter(value, buffer, index);
		}
		writeCharacter('"', buffer, index);
	}

	template<StringT OTy, VectorLike BTy> void SerializeWithKeys::op(OTy& value, BTy& buffer, size_t& index) {
		const auto string = static_cast<StringView>(value);
		const auto n = string.size();

		if constexpr (Resizeable<BTy>) {
			if ((index + (4 * n)) >= buffer.size()) [[unlikely]] {
				buffer.resize(std::max(buffer.size() * 2, index + (4 * n)));
			}
		}

		writeCharacterUnChecked('"', buffer, index);

		for (auto&& c: string) {
			switch (c) {
				case '"':
					buffer[index++] = '\\';
					buffer[index++] = '\"';
					break;
				case '\\':
					buffer[index++] = '\\';
					buffer[index++] = '\\';
					break;
				case '\b':
					buffer[index++] = '\\';
					buffer[index++] = 'b';
					break;
				case '\f':
					buffer[index++] = '\\';
					buffer[index++] = 'f';
					break;
				case '\n':
					buffer[index++] = '\\';
					buffer[index++] = 'n';
					break;
				case '\r':
					buffer[index++] = '\\';
					buffer[index++] = 'r';
					break;
				case '\t':
					buffer[index++] = '\\';
					buffer[index++] = 't';
					break;
				default:
					buffer[index++] = c;
			}
		}

		writeCharacterUnChecked('"', buffer, index);
	}

	template<ArrayTupleT OTy, VectorLike BTy> void SerializeWithKeys::op(OTy& value, BTy& buffer, size_t& index) {
		static constexpr auto N = []() constexpr {
			if constexpr (JsonifierArrayT<std::decay_t<OTy>>) {
				return std::tuple_size_v<CoreT<std::decay_t<OTy>>>;
			} else {
				return std::tuple_size_v<std::decay_t<OTy>>;
			}
		}();

		writeCharacter('[', buffer, index);
		using V = std::decay_t<OTy>;
		forEach<N>([&](auto I) {
			if constexpr (JsonifierArrayT<V>) {
				auto newMember = getMember(value, Tuplet::get<I>(CoreV<OTy>));
				using MemberType = decltype(newMember);
				if constexpr (HasExcludedKeys<MemberType>) {
					SerializeWithKeys::op(newMember, buffer, index, newMember.excludedKeys);
				} else {
					SerializeWithKeys::op(newMember, buffer, index);
				}
			} else {
				auto newMember = Tuplet::get<I>(value);
				using MemberType = decltype(newMember);
				if constexpr (HasExcludedKeys<MemberType>) {
					SerializeWithKeys::op(newMember, buffer, index, newMember.excludedKeys);
				} else {
					SerializeWithKeys::op(newMember, buffer, index);
				}
			}
			constexpr bool needsComma = I < N - 1;
			if constexpr (needsComma) {
				writeCharacter(',', buffer, index);
			}
		});
		writeCharacter(']', buffer, index);
	}

	template<RawArrayT OTy, VectorLike BTy> void SerializeWithKeys::op(OTy& value, BTy& buffer, size_t& index) {
		const auto n = value.size();
		writeCharacter('[', buffer, index);
		for (size_t x = 0; x < n; ++x) {
			auto newMember = value[x];
			using MemberType = decltype(newMember);
			if constexpr (HasExcludedKeys<MemberType>) {
				SerializeWithKeys::op(newMember, buffer, index, newMember.excludedKeys);
			} else {
				SerializeWithKeys::op(newMember, buffer, index);
			}
			const bool needsComma = x < n - 1;
			if (needsComma) {
				writeCharacter(',', buffer, index);
			}
		}
		writeCharacter(']', buffer, index);
	}

	template<VectorT OTy, VectorLike BTy> void SerializeWithKeys::op(OTy& value, BTy& buffer, size_t& index) {
		writeCharacter('[', buffer, index);
		const auto isEmpty = [&]() -> bool {
			if constexpr (HasSize<OTy>) {
				return value.size() ? false : true;
			} else {
				return value.empty();
			}
		}();

		if (!isEmpty) {
			auto it = value.begin();
			auto newMember = *it;
			using MemberType = decltype(newMember);
			if constexpr (HasExcludedKeys<MemberType>) {
				SerializeWithKeys::op(newMember, buffer, index, newMember.excludedKeys);
			} else {
				SerializeWithKeys::op(newMember, buffer, index);
			}
			++it;
			const auto end = value.end();
			for (; it != end; ++it) {
				writeCharacter(',', buffer, index);
				auto newMember = *it;
				using MemberType = decltype(newMember);
				if constexpr (HasExcludedKeys<MemberType>) {
					SerializeWithKeys::op(newMember, buffer, index, newMember.excludedKeys);
				} else {
					SerializeWithKeys::op(newMember, buffer, index);
				}
			}
		}
		writeCharacter(']', buffer, index);
	}

	template<StdTupleT OTy, VectorLike BTy> void SerializeWithKeys::op(OTy& value, BTy& buffer, size_t& index) {
		static constexpr auto n = []() constexpr {
			if constexpr (JsonifierArrayT<std::decay_t<OTy>>) {
				return std::tuple_size_v<CoreT<std::decay_t<OTy>>>;
			} else {
				return std::tuple_size_v<std::decay_t<OTy>>;
			}
		}();

		writeCharacter('[', buffer, index);
		using V = std::decay_t<OTy>;
		forEach<n>([&](auto x) {
			if constexpr (JsonifierArrayT<V>) {
				auto newMember = value.*std::get<x>(CoreV<V>);
				using MemberType = decltype(newMember);
				if constexpr (HasExcludedKeys<MemberType>) {
					SerializeWithKeys::op(newMember, buffer, index, newMember.excludedKeys);
				} else {
					SerializeWithKeys::op(newMember, buffer, index);
				}
			} else {
				auto newMember = std::get<x>(value);
				using MemberType = decltype(newMember);
				if constexpr (HasExcludedKeys<MemberType>) {
					SerializeWithKeys::op(newMember, buffer, index, newMember.excludedKeys);
				} else {
					SerializeWithKeys::op(newMember, buffer, index);
				}
			}
			constexpr bool needsComma = x < n - 1;
			if constexpr (needsComma) {
				writeCharacter(',', buffer, index);
			}
		});
		writeCharacter(']', buffer, index);
	}

	template<ObjectT OTy, VectorLike BTy, Findable KTy> void SerializeWithKeys::op(OTy& value, BTy& buffer, size_t& index, const KTy& excludedKeys) {
		writeCharacter('{', buffer, index);
		using V = std::decay_t<OTy>;
		static constexpr auto n = std::tuple_size_v<CoreT<V>>;

		bool first = true;
		forEach<n>([&](auto x) {
			static constexpr auto item = Tuplet::get<x>(CoreV<V>);
			using ItemType = decltype(item);
			using MPtrT = std::tuple_element_t<1, ItemType>;
			using ValT = MemberT<V, MPtrT>;

			if constexpr (NullableT<ValT>) {
				auto isNull = [&]() {
					if constexpr (std::is_member_pointer_v<std::tuple_element_t<1, ItemType>>) {
						return !bool(value.*Tuplet::get<1>(item));
					} else {
						return !bool(Tuplet::get<1>(item)(value));
					}
				}();
				if (isNull)
					return;
			}

			using Key = typename std::decay_t<std::tuple_element_t<0, ItemType>>;

			if constexpr (StringT<Key> || CharT<Key>) {
				static constexpr StringView key = Tuplet::get<0>(item);
				if (excludedKeys.find(static_cast<const typename KTy::key_type>(key)) != excludedKeys.end()) {
					return;
				}
				if (first) {
					first = false;
				} else {
					writeCharacter(',', buffer, index);
				}
				if constexpr (needsEscaping(key)) {
					SerializeWithKeys::op(key, buffer, index);
					writeCharacter(':', buffer, index);
				} else {
					static constexpr auto quoted = JoinV<Chars<"\"">, key, Chars<"\":">>;
					writeCharacters<quoted>(buffer, index);
				}
			} else {
				static constexpr auto quoted = concatArrays(concatArrays("\"", Tuplet::get<0>(item)), "\":", "");
				using MemberType = decltype(quoted);
				SerializeWithKeys::op(quoted, buffer, index);
			}
			auto newMember = getMember(value, Tuplet::get<1>(item));
			using NewMemberType = decltype(newMember);
			if constexpr (HasExcludedKeys<NewMemberType>) {
				SerializeWithKeys::op(newMember, buffer, index, newMember.excludedKeys);
			} else {
				SerializeWithKeys::op(newMember, buffer, index);
			}
		});
		writeCharacter('}', buffer, index);
	}

	template<ObjectT OTy, VectorLike BTy> void SerializeWithKeys::op(OTy& value, BTy& buffer, size_t& index) {
		writeCharacter('{', buffer, index);
		using V = std::decay_t<OTy>;
		static constexpr auto n = std::tuple_size_v<CoreT<V>>;

		bool first = true;
		forEach<n>([&](auto x) {
			static constexpr auto item = Tuplet::get<x>(CoreV<V>);
			using ItemType = decltype(item);
			using MPtrT = std::tuple_element_t<1, ItemType>;
			using ValT = MemberT<V, MPtrT>;

			if constexpr (NullableT<ValT>) {
				auto isNull = [&]() {
					if constexpr (std::is_member_pointer_v<std::tuple_element_t<1, ItemType>>) {
						return !bool(value.*Tuplet::get<1>(item));
					} else {
						return !bool(Tuplet::get<1>(item)(value));
					}
				}();
				if (isNull)
					return;
			}

			using Key = typename std::decay_t<std::tuple_element_t<0, ItemType>>;

			if constexpr (StringT<Key> || CharT<Key>) {
				static constexpr StringView key = Tuplet::get<0>(item);
				if (first) {
					first = false;
				} else {
					writeCharacter(',', buffer, index);
				}
				if constexpr (needsEscaping(key)) {
					SerializeWithKeys::op(key, buffer, index);
					writeCharacter(':', buffer, index);
				} else {
					static constexpr auto quoted = JoinV<Chars<"\"">, key, Chars<"\":">>;
					writeCharacters<quoted>(buffer, index);
				}
			} else {
				static constexpr auto quoted = concatArrays(concatArrays("\"", Tuplet::get<0>(item)), "\":", "");
				using MemberType = decltype(quoted);
				SerializeWithKeys::op(quoted, buffer, index);
			}
			auto newMember = getMember(value, Tuplet::get<1>(item));
			using MemberType = decltype(newMember);
			if constexpr (HasExcludedKeys<MemberType>) {
				SerializeWithKeys::op(newMember, buffer, index, newMember.excludedKeys);
			} else {
				SerializeWithKeys::op(newMember, buffer, index);
			}
		});
		writeCharacter('}', buffer, index);
	}

}