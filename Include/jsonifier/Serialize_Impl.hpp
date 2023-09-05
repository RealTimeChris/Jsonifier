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

#include <jsonifier/Serializer.hpp>
#include <jsonifier/Parser.hpp>
#include <jsonifier/Base.hpp>
#include <algorithm>

namespace JsonifierInternal {

	template<VectorLike BufferType> inline void writeCharacter(char c, BufferType&& buffer, uint64_t& index) {
		if (index >= buffer.size()) [[unlikely]] {
			buffer.resize((buffer.size() + 1) * 2);
		}

		buffer[index] = c;
		++index;
	}

	template<VectorLike BufferType> inline void writeCharacterUnChecked(char c, BufferType&& buffer, uint64_t& index) {
		buffer[index] = c;
		++index;
	}

	template<const Jsonifier::StringView& str, VectorLike BufferType> inline void writeCharacters(BufferType&& buffer, uint64_t& index) {
		static constexpr auto s = str;
		static constexpr auto n = s.size();

		if (index + n > buffer.size()) [[unlikely]] {
			buffer.resize(std::max(buffer.size() * 2, index + n));
		}

		std::memcpy(buffer.data() + index, s.data(), n);
		index += n;
	}

	template<StringLiteral str, VectorLike BufferType> inline void writeCharacters(BufferType&& buffer, uint64_t& index) {
		static constexpr auto s = str;
		static constexpr auto n = s.size();

		if (index + n > buffer.size()) [[unlikely]] {
			buffer.resize(std::max(buffer.size() * 2, index + n));
		}

		std::memcpy(buffer.data() + index, s.data(), n);
		index += n;
	}

	template<VectorLike BufferType> inline void writeCharacters(const Jsonifier::StringView str, BufferType&& buffer, uint64_t& index) {
		const auto n = str.size();
		if (index + n > buffer.size()) [[unlikely]] {
			buffer.resize(std::max(buffer.size() * 2, index + n));
		}

		std::memcpy(buffer.data() + index, str.data(), n);
		index += n;
	}

	template<bool excludeKeys, NullT ValueType> struct SerializeImpl<excludeKeys, ValueType> {
		template<VectorLike BufferType> inline static void op(const ValueType&, BufferType&& buffer, uint64_t& index) {
			writeCharacters<"null">(buffer, index);
		}
	};

	template<bool excludeKeys, BoolT ValueType> struct SerializeImpl<excludeKeys, ValueType> {
		template<VectorLike BufferType> inline static void op(const ValueType& value, BufferType&& buffer, uint64_t& index) {
			if (value) {
				writeCharacters<"true">(buffer, index);
			} else {
				writeCharacters<"false">(buffer, index);
			}
		}
	};

	template<bool excludeKeys, NumT ValueType> struct SerializeImpl<excludeKeys, ValueType> {
		template<VectorLike BufferType> inline static void op(const ValueType& value, BufferType&& buffer, uint64_t& index) {
			if (index + 32 > buffer.size()) [[unlikely]] {
				buffer.resize(std::max(buffer.size() * 2, index + 64));
			}
			auto start = dataPtr(buffer) + index;
			auto end   = toChars(start, value);
			index += std::distance(start, end);
		}
	};

	template<bool excludeKeys, EnumT ValueType> struct SerializeImpl<excludeKeys, ValueType> {
		template<VectorLike BufferType> inline static void op(const ValueType& value, BufferType&& buffer, uint64_t& index) {
			if (index + 32 > buffer.size()) [[unlikely]] {
				buffer.resize(std::max(buffer.size() * 2, index + 64));
			}
			auto start = dataPtr(buffer) + index;
			auto end   = toChars(start, static_cast<int64_t>(value));
			index += std::distance(start, end);
		}
	};

	template<bool excludeKeys, CharT ValueType> struct SerializeImpl<excludeKeys, ValueType> {
		template<VectorLike BufferType> inline static void op(const ValueType& value, BufferType&& buffer, uint64_t& index) {
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
	};

	template<bool excludeKeys, StringT ValueType> struct SerializeImpl<excludeKeys, ValueType> {
		template<VectorLike BufferType> inline static void op(const ValueType& value, BufferType&& buffer, uint64_t& index) {
			const auto n = value.size();

			if constexpr (HasResize<BufferType>) {
				if ((index + (4 * n)) >= buffer.size()) [[unlikely]] {
					buffer.resize(std::max(buffer.size() * 2, index + (4 * n)));
				}
			}

			writeCharacterUnChecked('"', buffer, index);

			for (auto&& c: value) {
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
	};

	template<bool excludeKeys, RawJsonT ValueType> struct SerializeImpl<excludeKeys, ValueType> {
		template<VectorLike BufferType> inline static void op(const ValueType& value, BufferType&& buffer, uint64_t& index) {
			Jsonifier::String newValue = static_cast<const Jsonifier::String>(value);
			Serialize<excludeKeys>::op(newValue, buffer, index);
		}
	};

	template<bool excludeKeys, ArrayTupleT ValueType> struct SerializeImpl<excludeKeys, ValueType> {
		template<VectorLike BufferType> inline static void op(const ValueType& value, BufferType&& buffer, uint64_t& index) {
			static constexpr auto N = []() constexpr {
				if constexpr (JsonifierArrayT<RefUnwrap<ValueType>>) {
					return std::tuple_size_v<CoreT<RefUnwrap<ValueType>>>;
				} else {
					return std::tuple_size_v<RefUnwrap<ValueType>>;
				}
			}();

			writeCharacter('[', buffer, index);
			using V = RefUnwrap<ValueType>;
			forEach<N>([&](auto I) {
				if constexpr (JsonifierArrayT<V>) {
					auto& newMember = getMember(value, Tuplet::get<I>(CoreV<ValueType>));
					Serialize<excludeKeys>::op(newMember, buffer, index);
				} else {
					Serialize<excludeKeys>::op(Tuplet::get<I>(value), buffer, index);
				}
				constexpr bool needsComma = I < N - 1;
				if constexpr (needsComma) {
					writeCharacter(',', buffer, index);
				}
			});
			writeCharacter(']', buffer, index);
		}
	};

	template<bool excludeKeys, RawArrayT ValueType> struct SerializeImpl<excludeKeys, ValueType> {
		template<VectorLike BufferType> inline static void op(const ValueType& value, BufferType&& buffer, uint64_t& index) {
			const auto n = value.size();
			writeCharacter('[', buffer, index);
			for (uint64_t x = 0; x < n; ++x) {
				Serialize<excludeKeys>::op(value[x], buffer, index);
				const bool needsComma = x < n - 1;
				if (needsComma) {
					writeCharacter(',', buffer, index);
				}
			}
			writeCharacter(']', buffer, index);
		}
	};

	template<bool excludeKeys, ArrayT ValueType> struct SerializeImpl<excludeKeys, ValueType> {
		template<VectorLike BufferType> inline static void op(const ValueType& value, BufferType&& buffer, uint64_t& index) {
			writeCharacter('[', buffer, index);

			if (value.size()) {
				auto it = value.begin();
				Serialize<excludeKeys>::op(*it, buffer, index);
				++it;
				const auto end = value.end();
				for (; it != end; ++it) {
					writeCharacter(',', buffer, index);
					Serialize<excludeKeys>::op(*it, buffer, index);
				}
			}
			writeCharacter(']', buffer, index);
		}
	};

	inline constexpr bool needsEscaping(auto& S) {
		for (auto& c: S) {
			if (c == '"') {
				return true;
			}
		}
		return false;
	}

	template<bool excludeKeys, ObjectT ValueType> struct SerializeImpl<excludeKeys, ValueType> {
		template<VectorLike BufferType> inline static void op(const ValueType& value, BufferType&& buffer, uint64_t& index) {
			writeCharacter('{', buffer, index);
			using V					= RefUnwrap<ValueType>;
			static constexpr auto n = std::tuple_size_v<CoreT<V>>;

			bool first = true;
			forEach<n>([&](auto x) {
				static constexpr auto item = Tuplet::get<x>(CoreV<V>);
				using ItemType			   = decltype(item);
				using MPtrT				   = std::tuple_element_t<1, ItemType>;
				using ValT				   = MemberT<V, MPtrT>;

				if constexpr (NullT<ValT>) {
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

				using Key = RefUnwrap<std::tuple_element_t<0, decltype(item)>>;

				if constexpr (StringT<Key> || CharT<Key>) {
					static constexpr Jsonifier::StringView key = Tuplet::get<0>(item);
					if constexpr (needsEscaping(key)) {
						Serialize<excludeKeys>::op(key, buffer, index);
						writeCharacter(':', buffer, index);
					} else {
						static constexpr auto quoted = JoinV<Chars<"\"">, key, Chars<"\":">>;
						writeCharacters<quoted>(buffer, index);
					}
				} else {
					static constexpr auto quoted = concatArrays(concatArrays("\"", Tuplet::get<0>(item)), "\":", "");
					Serialize<excludeKeys>::op(quoted, buffer, index);
				}
				auto& newMember = getMember(value, Tuplet::get<1>(item));
				Serialize<excludeKeys>::op(newMember, buffer, index);
			});
			writeCharacter('}', buffer, index);
		}
	};

	template<ObjectT ValueType> struct SerializeImpl<true, ValueType> {
		template<VectorLike BufferType> inline static void op(const ValueType& value, BufferType&& buffer, uint64_t& index) {
			writeCharacter('{', buffer, index);
			using V					= RefUnwrap<ValueType>;
			static constexpr auto n = std::tuple_size_v<CoreT<V>>;

			bool first = true;
			forEach<n>([&](auto x) {
				static constexpr auto item = Tuplet::get<x>(CoreV<V>);
				using ItemType			   = decltype(item);
				using MPtrT				   = std::tuple_element_t<1, ItemType>;
				using ValT				   = MemberT<V, MPtrT>;

				if constexpr (NullT<ValT>) {
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

				using Key = RefUnwrap<std::tuple_element_t<0, ItemType>>;

				if constexpr (StringT<Key> || CharT<Key>) {
					static constexpr Jsonifier::StringView key = Tuplet::get<0>(item);
					if (first) {
						first = false;
					} else {
						writeCharacter(',', buffer, index);
					}
					if constexpr (needsEscaping(key)) {
						Serialize<true>::op(key, buffer, index);
						writeCharacter(':', buffer, index);
					} else {
						static constexpr auto quoted = JoinV<Chars<"\"">, key, Chars<"\":">>;
						writeCharacters<quoted>(buffer, index);
					}
				} else {
					static constexpr auto quoted = concatArrays(concatArrays("\"", Tuplet::get<0>(item)), "\":", "");
					Serialize<true>::op(quoted, buffer, index);
				}
				auto& newMember = getMember(value, Tuplet::get<1>(item));
				using MemberType = decltype(newMember);
				if constexpr (HasExcludedKeys<MemberType>) {
					Serialize<true>::op(newMember, buffer, index, newMember.excludedKeys);
				} else {
					Serialize<true>::op(newMember, buffer, index);
				}
			});
			writeCharacter('}', buffer, index);
		}

		template<VectorLike BufferType, HasFind KeyType> inline static void op(const ValueType& value, BufferType&& buffer, uint64_t& index, KeyType& excludedKeys) {
			writeCharacter('{', buffer, index);
			using V					= RefUnwrap<ValueType>;
			static constexpr auto n = std::tuple_size_v<CoreT<V>>;

			bool first = true;
			forEach<n>([&](auto x) {
				static constexpr auto item = Tuplet::get<x>(CoreV<V>);
				using ItemType			   = decltype(item);
				using MPtrT				   = std::tuple_element_t<1, ItemType>;
				using ValT				   = MemberT<V, MPtrT>;

				if constexpr (NullT<ValT>) {
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

				using Key = RefUnwrap<std::tuple_element_t<0, ItemType>>;

				if constexpr (StringT<Key> || CharT<Key>) {
					static constexpr Jsonifier::StringView key = Tuplet::get<0>(item);
					if (excludedKeys.find(static_cast<const typename KeyType::key_type>(key)) != excludedKeys.end()) {
						return;
					}
					if (first) {
						first = false;
					} else {
						writeCharacter(',', buffer, index);
					}
					if constexpr (needsEscaping(key)) {
						Serialize<true>::op(key, buffer, index);
						writeCharacter(':', buffer, index);
					} else {
						static constexpr auto quoted = JoinV<Chars<"\"">, key, Chars<"\":">>;
						writeCharacters<quoted>(buffer, index);
					}
				} else {
					static constexpr auto quoted = concatArrays(concatArrays("\"", Tuplet::get<0>(item)), "\":", "");
					Serialize<true>::op(quoted, buffer, index);
				}
				auto& newMember = getMember(value, Tuplet::get<1>(item));
				using NewMemberType = decltype(newMember);
				if constexpr (HasExcludedKeys<NewMemberType>) {
					Serialize<true>::op(newMember, buffer, index, newMember.excludedKeys);
				} else {
					Serialize<true>::op(newMember, buffer, index);
				}
			});
			writeCharacter('}', buffer, index);
		}
	};
}
