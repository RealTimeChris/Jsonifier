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
/// https://github.com/RealTimeChris/jsonifier
/// Feb 20, 2023
#pragma once

#include <jsonifier/Serializer.hpp>
#include <jsonifier/Parser.hpp>
#include <jsonifier/Base.hpp>
#include <algorithm>

namespace jsonifier_internal {

	template<jsonifier::concepts::vector_like buffer_type> inline void writeCharacter(char c, buffer_type&& buffer, uint64_t& index) {
		if (index >= buffer.size()) [[unlikely]] {
			buffer.resize((buffer.size() + 1) * 2);
		}

		buffer[index] = c;
		++index;
	}

	template<char c, jsonifier::concepts::vector_like buffer_type> inline void writeCharacter(buffer_type&& buffer, uint64_t& index) {
		if (index >= buffer.size()) [[unlikely]] {
			buffer.resize((buffer.size() + 1) * 2);
		}

		buffer[index] = c;
		++index;
	}

	template<jsonifier::concepts::vector_like buffer_type> inline void writeCharacterUnChecked(char c, buffer_type&& buffer, uint64_t& index) {
		buffer[index] = c;
		++index;
	}

	template<const jsonifier::string_view& str, jsonifier::concepts::vector_like buffer_type> inline void writeCharacters(buffer_type&& buffer, uint64_t& index) {
		static constexpr jsonifier::string_view s = str;
		static constexpr uint64_t n				  = s.size();

		if (index + n > buffer.size()) [[unlikely]] {
			buffer.resize(std::max(buffer.size() * 2, index + n));
		}

		std::memcpy(buffer.data() + index, s.data(), n);
		index += n;
	}

	template<string_literal str, jsonifier::concepts::vector_like buffer_type> inline void writeCharacters(buffer_type&& buffer, uint64_t& index) {
		static constexpr auto s = str;
		static constexpr auto n = s.size();

		if (index + n > buffer.size()) [[unlikely]] {
			buffer.resize(std::max(buffer.size() * 2, index + n));
		}

		std::memcpy(buffer.data() + index, s.data(), n);
		index += n;
	}

	template<jsonifier::concepts::vector_like buffer_type> inline void writeCharacters(const jsonifier::string_view str, buffer_type&& buffer, uint64_t& index) {
		const auto n = str.size();
		if (index + n > buffer.size()) [[unlikely]] {
			buffer.resize(std::max(buffer.size() * 2, index + n));
		}

		std::memcpy(buffer.data() + index, str.data(), n);
		index += n;
	}

	template<bool excludedKeys, jsonifier::concepts::null_t value_type> struct serialize_impl<excludedKeys, value_type> {
		template<jsonifier::concepts::vector_like buffer_type> inline static void op(const value_type&, buffer_type&& buffer, uint64_t& index) {
			writeCharacters<"null">(buffer, index);
		}
	};

	template<bool excludedKeys, jsonifier::concepts::bool_t value_type> struct serialize_impl<excludedKeys, value_type> {
		template<jsonifier::concepts::vector_like buffer_type> inline static void op(const value_type& value, buffer_type&& buffer, uint64_t& index) {
			if (value) {
				writeCharacters<"true">(buffer, index);
			} else {
				writeCharacters<"false">(buffer, index);
			}
		}
	};

	template<bool excludedKeys, jsonifier::concepts::num_t value_type> struct serialize_impl<excludedKeys, value_type> {
		template<jsonifier::concepts::vector_like buffer_type> inline static void op(const value_type& value, buffer_type&& buffer, uint64_t& index) {
			if (index + 32 > buffer.size()) [[unlikely]] {
				buffer.resize(std::max(buffer.size() * 2, index + 64));
			}
			auto start = jsonifier::concepts::dataPtr(buffer) + index;
			auto end   = toChars(start, value);
			index += std::distance(start, end);
		}
	};

	template<bool excludedKeys, jsonifier::concepts::enum_t value_type> struct serialize_impl<excludedKeys, value_type> {
		template<jsonifier::concepts::vector_like buffer_type> inline static void op(const value_type& value, buffer_type&& buffer, uint64_t& index) {
			if (index + 32 > buffer.size()) [[unlikely]] {
				buffer.resize(std::max(buffer.size() * 2, index + 64));
			}
			auto start = jsonifier::concepts::dataPtr(buffer) + index;
			auto end   = toChars(start, static_cast<int64_t>(value));
			index += std::distance(start, end);
		}
	};

	template<bool excludedKeys, jsonifier::concepts::char_t value_type> struct serialize_impl<excludedKeys, value_type> {
		template<jsonifier::concepts::vector_like buffer_type> inline static void op(const value_type& value, buffer_type&& buffer, uint64_t& index) {
			writeCharacter<'"'>(buffer, index);
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
			writeCharacter<'"'>(buffer, index);
		}
	};

	template<bool excludedKeys, jsonifier::concepts::string_t value_type> struct serialize_impl<excludedKeys, value_type> {
		template<jsonifier::concepts::vector_like buffer_type> inline static void op(const value_type& value, buffer_type&& buffer, uint64_t& index) {
			const auto n = value.size();

			if constexpr (jsonifier::concepts::has_resize<buffer_type>) {
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

	template<bool excludedKeys, jsonifier::concepts::raw_json_t value_type> struct serialize_impl<excludedKeys, value_type> {
		template<jsonifier::concepts::vector_like buffer_type> inline static void op(const value_type& value, buffer_type&& buffer, uint64_t& index) {
			jsonifier::string newValue = static_cast<const jsonifier::string>(value);
			serialize<excludedKeys>::op(newValue, buffer, index);
		}
	};

	template<bool excludedKeys, jsonifier::concepts::array_tuple_t value_type> struct serialize_impl<excludedKeys, value_type> {
		template<jsonifier::concepts::vector_like buffer_type> inline static void op(const value_type& value, buffer_type&& buffer, uint64_t& index) {
			static constexpr auto N = []() constexpr {
				if constexpr (jsonifier::concepts::jsonifier_array_t<std::decay_t<value_type>>) {
					return std::tuple_size_v<jsonifier::concepts::core_t<std::decay_t<value_type>>>;
				} else {
					return std::tuple_size_v<std::decay_t<value_type>>;
				}
			}();

			writeCharacter<'['>(buffer, index);
			using V = std::decay_t<value_type>;
			forEach<N>([&](auto I) {
				if constexpr (jsonifier::concepts::jsonifier_array_t<V>) {
					auto& newMember = getMember(value, tuplet::get<I>(jsonifier::concepts::coreV<value_type>));
					serialize<excludedKeys>::op(newMember, buffer, index);
				} else {
					serialize<excludedKeys>::op(tuplet::get<I>(value), buffer, index);
				}
				constexpr bool needsComma = I < N - 1;
				if constexpr (needsComma) {
					writeCharacter<','>(buffer, index);
				}
			});
			writeCharacter<']'>(buffer, index);
		}
	};

	template<jsonifier::concepts::array_tuple_t value_type> struct serialize_impl<true, value_type> {
		template<jsonifier::concepts::vector_like buffer_type> inline static void op(const value_type& value, buffer_type&& buffer, uint64_t& index) {
			static constexpr auto N = []() constexpr {
				if constexpr (jsonifier::concepts::jsonifier_array_t<std::decay_t<value_type>>) {
					return std::tuple_size_v<jsonifier::concepts::core_t<std::decay_t<value_type>>>;
				} else {
					return std::tuple_size_v<std::decay_t<value_type>>;
				}
			}();

			writeCharacter<'['>(buffer, index);
			using V = std::decay_t<value_type>;
			forEach<N>([&](auto I) {
				if constexpr (jsonifier::concepts::jsonifier_array_t<V>) {
					auto& newMember	 = getMember(value, tuplet::get<I>(jsonifier::concepts::coreV<value_type>));
					using MemberType = decltype(newMember);
					if constexpr (jsonifier::concepts::has_excluded_keys<MemberType>) {
						serialize<true>::op(newMember, buffer, index, newMember.excludedKeys);
					} else {
						serialize<true>::op(newMember, buffer, index);
					}
				} else {
					auto& newMember	 = tuplet::get<I>(value);
					using MemberType = decltype(newMember);
					if constexpr (jsonifier::concepts::has_excluded_keys<MemberType>) {
						serialize<true>::op(newMember, buffer, index, newMember.excludedKeys);
					} else {
						serialize<true>::op(newMember, buffer, index);
					}
				}
				constexpr bool needsComma = I < N - 1;
				if constexpr (needsComma) {
					writeCharacter<','>(buffer, index);
				}
			});
			writeCharacter<']'>(buffer, index);
		}
	};

	template<bool excludedKeys, jsonifier::concepts::raw_array_t value_type> struct serialize_impl<excludedKeys, value_type> {
		template<jsonifier::concepts::vector_like buffer_type> inline static void op(const value_type& value, buffer_type&& buffer, uint64_t& index) {
			const auto n = value.size();
			writeCharacter<'['>(buffer, index);
			for (uint64_t x = 0; x < n; ++x) {
				serialize<excludedKeys>::op(value[x], buffer, index);
				const bool needsComma = x < n - 1;
				if (needsComma) {
					writeCharacter<','>(buffer, index);
				}
			}
			writeCharacter<']'>(buffer, index);
		}
	};

	template<jsonifier::concepts::raw_array_t value_type> struct serialize_impl<true, value_type> {
		template<jsonifier::concepts::vector_like buffer_type> inline static void op(const value_type& value, buffer_type&& buffer, uint64_t& index) {
			const auto n = value.size();
			writeCharacter<'['>(buffer, index);
			for (uint64_t x = 0; x < n; ++x) {
				auto& newMember	 = value[x];
				using MemberType = decltype(newMember);
				if constexpr (jsonifier::concepts::has_excluded_keys<MemberType>) {
					serialize<true>::op(newMember, buffer, index, newMember.excludedKeys);
				} else {
					serialize<true>::op(newMember, buffer, index);
				}
				const bool needsComma = x < n - 1;
				if (needsComma) {
					writeCharacter<','>(buffer, index);
				}
			}
			writeCharacter<']'>(buffer, index);
		}
	};

	template<bool excludedKeys, jsonifier::concepts::vector_t value_type> struct serialize_impl<excludedKeys, value_type> {
		template<jsonifier::concepts::vector_like buffer_type> inline static void op(const value_type& value, buffer_type&& buffer, uint64_t& index) {
			writeCharacter<'['>(buffer, index);

			if (value.size()) {
				bool first = true;
				for (auto iter = value.begin(); iter != value.end(); ++iter) {
					if (first) {
						first = false;
					} else {
						writeCharacter<','>(buffer, index);
					}
					serialize<excludedKeys>::op(*iter, buffer, index);
				}
			}
			writeCharacter<']'>(buffer, index);
		}
	};

	template<jsonifier::concepts::vector_t value_type> struct serialize_impl<true, value_type> {
		template<jsonifier::concepts::vector_like buffer_type> inline static void op(const value_type& value, buffer_type&& buffer, uint64_t& index) {
			writeCharacter<'['>(buffer, index);

			if (value.size()) {
				bool first = true;
				for (auto iter = value.begin(); iter != value.end(); ++iter) {
					if (first) {
						first = false;
					} else {
						writeCharacter<','>(buffer, index);
					}
					auto& newMember	 = *iter;
					using MemberType = decltype(newMember);
					if constexpr (jsonifier::concepts::has_excluded_keys<MemberType>) {
						serialize<true>::op(newMember, buffer, index, newMember.excludedKeys);
					} else {
						serialize<true>::op(newMember, buffer, index);
					}
				}
			}
			writeCharacter<']'>(buffer, index);
		}
	};

	constexpr bool needsEscaping(auto& S) {
		for (auto& c: S) {
			if (c == '"') {
				return true;
			}
		}
		return false;
	}

	template<bool excludedKeys, jsonifier::concepts::object_t value_type> struct serialize_impl<excludedKeys, value_type> {
		template<jsonifier::concepts::vector_like buffer_type> inline static void op(const value_type& value, buffer_type&& buffer, uint64_t& index) {
			writeCharacter<'{'>(buffer, index);
			using V					= std::decay_t<value_type>;
			static constexpr auto n = std::tuple_size_v<jsonifier::concepts::core_t<V>>;

			bool first = true;
			forEach<n>([&](auto x) {
				static constexpr auto item = tuplet::get<x>(jsonifier::concepts::coreV<V>);
				using ItemType			   = decltype(item);
				using MPtrT				   = std::tuple_element_t<1, ItemType>;
				using ValT				   = member_t<V, MPtrT>;

				if constexpr (jsonifier::concepts::null_t<ValT>) {
					auto isNull = [&]() {
						if constexpr (std::is_member_pointer_v<std::tuple_element_t<1, ItemType>>) {
							return !bool(value.*tuplet::get<1>(item));
						} else {
							return !bool(tuplet::get<1>(item)(value));
						}
					}();
					if (isNull) {
						return;
					}
				}

				if (first) {
					first = false;
				} else {
					writeCharacter<','>(buffer, index);
				}

				using key = std::decay_t<std::tuple_element_t<0, decltype(item)>>;

				if constexpr (jsonifier::concepts::string_t<key> || jsonifier::concepts::char_t<key>) {
					static constexpr jsonifier::string_view key = tuplet::get<0>(item);
					if constexpr (needsEscaping(key)) {
						serialize<excludedKeys>::op(key, buffer, index);
						writeCharacter<':'>(buffer, index);
					} else {
						static constexpr auto quoted = JoinV<Chars<"\"">, key, Chars<"\":">>;
						writeCharacters<quoted>(buffer, index);
					}
				} else {
					static constexpr auto quoted = concatArrays(concatArrays("\"", tuplet::get<0>(item)), "\":", "");
					serialize<excludedKeys>::op(quoted, buffer, index);
				}
				auto& newMember = getMember(value, tuplet::get<1>(item));
				serialize<excludedKeys>::op(newMember, buffer, index);
			});
			writeCharacter<'}'>(buffer, index);
		}
	};

	template<jsonifier::concepts::object_t value_type> struct serialize_impl<true, value_type> {
		template<jsonifier::concepts::vector_like buffer_type> inline static void op(const value_type& value, buffer_type&& buffer, uint64_t& index) {
			writeCharacter<'{'>(buffer, index);
			using V					= std::decay_t<value_type>;
			static constexpr auto n = std::tuple_size_v<jsonifier::concepts::core_t<V>>;

			bool first = true;
			forEach<n>([&](auto x) {
				static constexpr auto item = tuplet::get<x>(jsonifier::concepts::coreV<V>);
				using ItemType			   = decltype(item);
				using MPtrT				   = std::tuple_element_t<1, ItemType>;
				using ValT				   = member_t<V, MPtrT>;

				if constexpr (jsonifier::concepts::null_t<ValT>) {
					auto isNull = [&]() {
						if constexpr (std::is_member_pointer_v<std::tuple_element_t<1, ItemType>>) {
							return !bool(value.*tuplet::get<1>(item));
						} else {
							return !bool(tuplet::get<1>(item)(value));
						}
					}();
					if (isNull) {
						return;
					}
				}

				using key = std::decay_t<std::tuple_element_t<0, ItemType>>;

				if constexpr (jsonifier::concepts::string_t<key> || jsonifier::concepts::char_t<key>) {
					static constexpr jsonifier::string_view key = tuplet::get<0>(item);
					if (first) {
						first = false;
					} else {
						writeCharacter<','>(buffer, index);
					}
					if constexpr (needsEscaping(key)) {
						serialize<true>::op(key, buffer, index);
						writeCharacter<':'>(buffer, index);
					} else {
						static constexpr auto quoted = JoinV<Chars<"\"">, key, Chars<"\":">>;
						writeCharacters<quoted>(buffer, index);
					}
				} else {
					static constexpr auto quoted = concatArrays(concatArrays("\"", tuplet::get<0>(item)), "\":", "");
					auto& newMember				 = quoted;
					using MemberType			 = decltype(newMember);
					if constexpr (jsonifier::concepts::has_excluded_keys<MemberType>) {
						serialize<true>::op(newMember, buffer, index, newMember.excludedKeys);
					} else {
						serialize<true>::op(newMember, buffer, index);
					}
				}
				auto& newMember	 = getMember(value, tuplet::get<1>(item));
				using MemberType = decltype(newMember);
				if constexpr (jsonifier::concepts::has_excluded_keys<MemberType>) {
					serialize<true>::op(newMember, buffer, index, newMember.excludedKeys);
				} else {
					serialize<true>::op(newMember, buffer, index);
				}
			});
			writeCharacter<'}'>(buffer, index);
		}

		template<jsonifier::concepts::vector_like buffer_type, jsonifier::concepts::has_find KeyType>
		inline static void op(const value_type& value, buffer_type&& buffer, uint64_t& index, const KeyType& excludedKeys) {
			writeCharacter<'{'>(buffer, index);
			using V					= std::decay_t<value_type>;
			static constexpr auto n = std::tuple_size_v<jsonifier::concepts::core_t<V>>;

			bool first = true;
			forEach<n>([&](auto x) {
				static constexpr auto item = tuplet::get<x>(jsonifier::concepts::coreV<V>);
				using ItemType			   = decltype(item);
				using MPtrT				   = std::tuple_element_t<1, ItemType>;
				using ValT				   = member_t<V, MPtrT>;

				if constexpr (jsonifier::concepts::null_t<ValT>) {
					auto isNull = [&]() {
						if constexpr (std::is_member_pointer_v<std::tuple_element_t<1, ItemType>>) {
							return !bool(value.*tuplet::get<1>(item));
						} else {
							return !bool(tuplet::get<1>(item)(value));
						}
					}();
					if (isNull) {
						return;
					}
				}

				using key = std::decay_t<std::tuple_element_t<0, ItemType>>;

				if constexpr (jsonifier::concepts::string_t<key> || jsonifier::concepts::char_t<key>) {
					static constexpr jsonifier::string_view key = tuplet::get<0>(item);
					if (excludedKeys.find(static_cast<const typename KeyType::key_type>(key)) != excludedKeys.end()) {
						return;
					}
					if (first) {
						first = false;
					} else {
						writeCharacter<','>(buffer, index);
					}
					if constexpr (needsEscaping(key)) {
						serialize<true>::op(key, buffer, index);
						writeCharacter<':'>(buffer, index);
					} else {
						static constexpr auto quoted = JoinV<Chars<"\"">, key, Chars<"\":">>;
						writeCharacters<quoted>(buffer, index);
					}
				} else {
					static constexpr auto quoted = concatArrays(concatArrays("\"", tuplet::get<0>(item)), "\":", "");
					auto& newMember				 = quoted;
					using MemberType			 = decltype(newMember);
					if constexpr (jsonifier::concepts::has_excluded_keys<MemberType>) {
						serialize<true>::op(newMember, buffer, index, newMember.excludedKeys);
					} else {
						serialize<true>::op(newMember, buffer, index);
					}
				}
				auto& newMember	 = getMember(value, tuplet::get<1>(item));
				using MemberType = decltype(newMember);
				if constexpr (jsonifier::concepts::has_excluded_keys<MemberType>) {
					serialize<true>::op(newMember, buffer, index, newMember.excludedKeys);
				} else {
					serialize<true>::op(newMember, buffer, index);
				}
			});
			writeCharacter<'}'>(buffer, index);
		}
	};
}
