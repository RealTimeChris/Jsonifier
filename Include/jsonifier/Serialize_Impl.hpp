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

	inline void writeCharacter(char c, VectorLike auto& buffer, size_t& index) {
		if (index >= buffer.size()) [[unlikely]] {
			buffer.resize((buffer.size() + 1) * 2);
		}

		buffer[index] = c;
		++index;
	}

	inline void writeCharacterUnChecked(char c, VectorLike auto& buffer, size_t& index) {
		buffer[index] = c;
		++index;
	}

	template<const Jsonifier::StringView& str> inline void writeCharacters(VectorLike auto& buffer, size_t& index) {
		static constexpr auto s = str;
		static constexpr auto n = s.size();

		if (index + n > buffer.size()) [[unlikely]] {
			auto newSize01 = buffer.size();
			auto newSize02 = index + n;
			newSize01 = newSize01 > newSize02 ? newSize01 : newSize02;
			buffer.resize(newSize01);
		}

		std::memcpy(buffer.data() + index, s.data(), n);
		index += n;
	}

	template<StringLiteral str> inline void writeCharacters(VectorLike auto& buffer, size_t& index) {
		static constexpr auto s = str;
		static constexpr auto n = s.size();

		if (index + n > buffer.size()) [[unlikely]] {
			auto newSize01 = buffer.size() * 2;
			auto newSize02 = index + n;
			newSize01 = newSize01 > newSize02 ? newSize01 : newSize02;
			buffer.resize(newSize01);
		}

		std::memcpy(buffer.data() + index, s.data(), n);
		index += n;
	}

	inline void writeCharacters(const Jsonifier::StringView str, VectorLike auto& buffer, size_t& index) {
		const auto n = str.size();
		if (index + n > buffer.size()) [[unlikely]] {
			auto newSize01 = buffer.size() * 2;
			auto newSize02 = index + n;
			newSize01 = newSize01 > newSize02 ? newSize01 : newSize02;
			buffer.resize(newSize01);
		}

		std::memcpy(buffer.data() + index, str.data(), n);
		index += n;
	}

	template<NullT ValueType, VectorLike BufferType> inline void SerializeNoKeys::op(ValueType&, BufferType& buffer, size_t& index) {
		writeCharacters<"null">(buffer, index);
	}

	template<BoolT ValueType, VectorLike BufferType> inline void SerializeNoKeys::op(ValueType& value, BufferType& buffer, size_t& index) {
		if (value) {
			writeCharacters<"true">(buffer, index);
		} else {
			writeCharacters<"false">(buffer, index);
		}
	}

	template<NumT ValueType, VectorLike BufferType> inline void SerializeNoKeys::op(ValueType& value, BufferType& buffer, size_t& index) {
		if (index + 32 > buffer.size()) [[unlikely]] {
			auto newSize01 = buffer.size() * 2;
			auto newSize02 = index + 64;
			newSize01 = newSize01 > newSize02 ? newSize01 : newSize02;
			buffer.resize(newSize01);
		}
		auto start = dataPtr(buffer) + index;
		auto end = toChars(start, value);
		index += std::distance(start, end);
	}

	template<EnumT ValueType, VectorLike BufferType> inline void SerializeNoKeys::op(ValueType& value, BufferType& buffer, size_t& index) {
		if (index + 32 > buffer.size()) [[unlikely]] {
			auto newSize01 = buffer.size() * 2;
			auto newSize02 = index + 64;
			newSize01 = newSize01 > newSize02 ? newSize01 : newSize02;
			buffer.resize(newSize01);
		}
		auto start = dataPtr(buffer) + index;
		auto end = toChars(start, static_cast<int64_t>(value));
		index += std::distance(start, end);
	}

	template<CharT ValueType, VectorLike BufferType> inline void SerializeNoKeys::op(ValueType& value, BufferType& buffer, size_t& index) {
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

	template<StringT ValueType, VectorLike BufferType> inline void SerializeNoKeys::op(ValueType& value, BufferType& buffer, size_t& index) {
		const auto n = value.size();

		if constexpr (HasResize<BufferType>) {
			if ((index + (4 * n)) >= buffer.size()) [[unlikely]] {
				auto newSize01 = buffer.size() * 2;
				auto newSize02 = index + (4 * n);
				newSize01 = newSize01 > newSize02 ? newSize01 : newSize02;
				buffer.resize(newSize01);
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

	template<RawJsonT ValueType, VectorLike BufferType> inline void SerializeNoKeys::op(ValueType& value, BufferType& buffer, size_t& index) {
		Jsonifier::String newValue = static_cast<Jsonifier::String>(value);
		SerializeNoKeys::op(newValue, buffer, index);
	}

	template<ArrayTupleT ValueType, VectorLike BufferType> inline void SerializeNoKeys::op(ValueType& value, BufferType& buffer, size_t& index) {
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
				SerializeNoKeys::op(getMember(value, Tuplet::get<I>(CoreV<ValueType>)), buffer, index);
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

	template<RawArrayT ValueType, VectorLike BufferType> inline void SerializeNoKeys::op(ValueType& value, BufferType& buffer, size_t& index) {
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

	template<ArrayT ValueType, VectorLike BufferType> inline void SerializeNoKeys::op(ValueType& value, BufferType& buffer, size_t& index) {
		writeCharacter('[', buffer, index);

		if (value.size()) {
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

	constexpr bool needsEscaping(const auto& S) {
		for (const auto& c: S) {
			if (c == '"') {
				return true;
			}
		}
		return false;
	}

	template<ObjectT ValueType, VectorLike BufferType> inline void SerializeNoKeys::op(ValueType& value, BufferType& buffer, size_t& index) {
		writeCharacter('{', buffer, index);
		using V = RefUnwrap<ValueType>;
		static constexpr auto n = std::tuple_size_v<CoreT<V>>;

		bool first = true;
		forEach<n>([&](auto x) {
			static constexpr auto item = Tuplet::get<x>(CoreV<V>);
			using ItemType = decltype(item);
			using MPtrT = std::tuple_element_t<1, ItemType>;
			using ValT = MemberT<V, MPtrT>;

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
					SerializeNoKeys::op(key, buffer, index);
					writeCharacter(':', buffer, index);
				} else {
					static constexpr auto quoted = JoinV<Chars<"\"">, key, Chars<"\":">>;
					writeCharacters<quoted>(buffer, index);
				}
			} else {
				static constexpr auto newValue = JsonifierInternal::Tuplet::get<0>(item);
				using ValueTypeNew = decltype(newValue);
				static auto constexpr result01 = JsonifierInternal::concatArrays<typename ValueTypeNew::value_type, newValue>("\"");
				using ResultType01 = decltype(result01);
				static constexpr auto result02 = JsonifierInternal::concatArrays<typename ResultType01::value_type, result01.size(), result01>("\":");
				SerializeNoKeys::op(result02, buffer, index);
			}
			SerializeNoKeys::op(getMember(value, Tuplet::get<1>(item)), buffer, index);
		});
		writeCharacter('}', buffer, index);
	}

	template<NullT ValueType, VectorLike BufferType> inline void SerializeWithKeys::op(ValueType&, BufferType& buffer, size_t& index) {
		writeCharacters<"null">(buffer, index);
	}

	template<BoolT ValueType, VectorLike BufferType> inline void SerializeWithKeys::op(ValueType& value, BufferType& buffer, size_t& index) {
		if (value) {
			writeCharacters<"true">(buffer, index);
		} else {
			writeCharacters<"false">(buffer, index);
		}
	}

	template<NumT ValueType, VectorLike BufferType> inline void SerializeWithKeys::op(ValueType& value, BufferType& buffer, size_t& index) {
		if (index + 32 > buffer.size()) [[unlikely]] {
			auto newSize01 = buffer.size();
			auto newSize02 = index + 64;
			newSize01 = newSize01 > newSize02 ? newSize01 : newSize02;
			buffer.resize(newSize01);
		}
		auto start = dataPtr(buffer) + index;
		auto end = toChars(start, value);
		index += std::distance(start, end);
	}

	template<EnumT ValueType, VectorLike BufferType> inline void SerializeWithKeys::op(ValueType& value, BufferType& buffer, size_t& index) {
		if (index + 32 > buffer.size()) [[unlikely]] {
			auto newSize01 = buffer.size();
			auto newSize02 = index + 64;
			newSize01 = newSize01 > newSize02 ? newSize01 : newSize02;
			buffer.resize(newSize01);
		}
		auto start = dataPtr(buffer) + index;
		auto end = toChars(start, static_cast<int64_t>(value));
		index += std::distance(start, end);
	}

	template<CharT ValueType, VectorLike BufferType> inline void SerializeWithKeys::op(ValueType& value, BufferType& buffer, size_t& index) {
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

	template<StringT ValueType, VectorLike BufferType> inline void SerializeWithKeys::op(ValueType& value, BufferType& buffer, size_t& index) {
		const auto n = value.size();

		if constexpr (HasResize<BufferType>) {
			if ((index + (4 * n)) >= buffer.size()) [[unlikely]] {
				auto newSize01 = buffer.size() * 2;
				auto newSize02 = index + (4 * n);
				newSize01 = newSize01 > newSize02 ? newSize01 : newSize02;
				buffer.resize(newSize01);
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

	template<RawJsonT ValueType, VectorLike BufferType> inline void SerializeWithKeys::op(ValueType& value, BufferType& buffer, size_t& index) {
		Jsonifier::String newValue = static_cast<Jsonifier::String>(value);
		SerializeWithKeys::op(newValue, buffer, index);
	}

	template<ArrayTupleT ValueType, VectorLike BufferType> inline void SerializeWithKeys::op(ValueType& value, BufferType& buffer, size_t& index) {
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
				auto newMember = getMember(value, Tuplet::get<I>(CoreV<ValueType>));
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

	template<RawArrayT ValueType, VectorLike BufferType> inline void SerializeWithKeys::op(ValueType& value, BufferType& buffer, size_t& index) {
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

	template<ArrayT ValueType, VectorLike BufferType> inline void SerializeWithKeys::op(ValueType& value, BufferType& buffer, size_t& index) {
		writeCharacter('[', buffer, index);

		if (value.size()) {
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
				newMember = *it;
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

	template<ObjectT ValueType, VectorLike BufferType, HasFind KeyType>
	inline void SerializeWithKeys::op(ValueType& value, BufferType& buffer, size_t& index, const KeyType& excludedKeys) {
		writeCharacter('{', buffer, index);
		using V = RefUnwrap<ValueType>;
		static constexpr auto n = std::tuple_size_v<CoreT<V>>;

		bool first = true;
		forEach<n>([&](auto x) {
			static constexpr auto item = Tuplet::get<x>(CoreV<V>);
			using ItemType = decltype(item);
			using MPtrT = std::tuple_element_t<1, ItemType>;
			using ValT = MemberT<V, MPtrT>;

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
					SerializeWithKeys::op(key, buffer, index);
					writeCharacter(':', buffer, index);
				} else {
					static constexpr auto quoted = JoinV<Chars<"\"">, key, Chars<"\":">>;
					writeCharacters<quoted>(buffer, index);
				}
			} else {
				static constexpr auto newValue = JsonifierInternal::Tuplet::get<0>(item);
				using ValueTypeNew = decltype(newValue);
				static auto constexpr result01 = JsonifierInternal::concatArrays<typename ValueTypeNew::value_type, newValue>("\"");
				using ResultType01 = decltype(result01);
				static constexpr auto result02 = JsonifierInternal::concatArrays<typename ResultType01::value_type, result01.size(), result01>("\":");
				SerializeWithKeys::op(result02, buffer, index);
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

	template<ObjectT ValueType, VectorLike BufferType> inline void SerializeWithKeys::op(ValueType& value, BufferType& buffer, size_t& index) {
		writeCharacter('{', buffer, index);
		using V = RefUnwrap<ValueType>;
		static constexpr auto n = std::tuple_size_v<CoreT<V>>;

		bool first = true;
		forEach<n>([&](auto x) {
			static constexpr auto item = Tuplet::get<x>(CoreV<V>);
			using ItemType = decltype(item);
			using MPtrT = std::tuple_element_t<1, ItemType>;
			using ValT = MemberT<V, MPtrT>;

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
					SerializeWithKeys::op(key, buffer, index);
					writeCharacter(':', buffer, index);
				} else {
					static constexpr auto quoted = JoinV<Chars<"\"">, key, Chars<"\":">>;
					writeCharacters<quoted>(buffer, index);
				}
			} else {
				static constexpr auto newValue = JsonifierInternal::Tuplet::get<0>(item);
				using ValueTypeNew = decltype(newValue);
				static auto constexpr result01 = JsonifierInternal::concatArrays<typename ValueTypeNew::value_type, newValue>("\"");
				using ResultType01 = decltype(result01);
				static constexpr auto result02 = JsonifierInternal::concatArrays<typename ResultType01::value_type, result01.size(), result01>("\":");
				SerializeWithKeys::op(result02, buffer, index);
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