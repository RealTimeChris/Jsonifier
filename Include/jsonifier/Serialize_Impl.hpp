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

	template<jsonifier::concepts::vector_like buffer_type> jsonifier_inline void writeCharacter(char c, buffer_type& buffer, uint64_t& index) {
		if (index + 1 >= buffer.size()) {
			buffer.resize(buffer.size() == 0 ? index + 1 : (buffer.size() + index) * 2);
		}

		buffer[index++] = c;
		buffer[index]	= '\0';
	}

	template<json_structural_type c, jsonifier::concepts::vector_like buffer_type> jsonifier_inline void writeCharacter(buffer_type& buffer, uint64_t& index) {
		if (index + 1 >= buffer.size()) {
			buffer.resize(buffer.size() == 0 ? index + 1 : (buffer.size() + index) * 2);
		}

		buffer[index++] = static_cast<typename jsonifier::concepts::unwrap<buffer_type>::value_type>(c);
		buffer[index]	= '\0';
	}

	template<json_structural_type c, jsonifier::concepts::vector_like buffer_type> jsonifier_inline void writeCharacterUnChecked(buffer_type& buffer, uint64_t& index) {
		buffer[index++] = static_cast<typename jsonifier::concepts::unwrap<buffer_type>::value_type>(c);
		buffer[index]	= '\0';
	}

	template<const jsonifier::string_view& str, jsonifier::concepts::vector_like buffer_type> jsonifier_inline void writeCharacters(buffer_type& buffer, uint64_t& index) {
		static jsonifier_constexpr jsonifier::string_view s = str;
		static jsonifier_constexpr uint64_t n				= s.size();
		if (index + n >= buffer.size()) {
			buffer.resize((std::max)(buffer.size() * 2, index + n));
		}

		std::memcpy(buffer.data() + index, s.data(), n);
		index += n;
	}

	template<jsonifier::concepts::vector_like buffer_type> jsonifier_inline void writeCharacters(const jsonifier::string_view str, buffer_type& buffer, uint64_t& index) {
		const auto n = str.size();
		if (index + n >= buffer.size()) {
			buffer.resize((std::max)(buffer.size() * 2, index + n));
		}

		std::memcpy(buffer.data() + index, str.data(), n);
		index += n;
	}

	template<bool excludeKeys, jsonifier::concepts::null_t value_type> struct serialize_impl<excludeKeys, value_type> {
		template<jsonifier::concepts::vector_like buffer_type> jsonifier_inline static void op(const value_type&, buffer_type& buffer, uint64_t& index) {
			static jsonifier_constexpr jsonifier::string_view stringViewNew{ "null" };
			writeCharacters<stringViewNew>(buffer, index);
		}
	};

	template<bool excludeKeys, jsonifier::concepts::bool_t value_type> struct serialize_impl<excludeKeys, value_type> {
		template<jsonifier::concepts::vector_like buffer_type> jsonifier_inline static void op(const value_type& value, buffer_type& buffer, uint64_t& index) {
			if (value) {
				static jsonifier_constexpr jsonifier::string_view stringViewNew{ "true" };
				writeCharacters<stringViewNew>(buffer, index);
			} else {
				static jsonifier_constexpr jsonifier::string_view stringViewNew{ "false" };
				writeCharacters<stringViewNew>(buffer, index);
			}
		}
	};

	template<bool excludeKeys, jsonifier::concepts::num_t value_type> struct serialize_impl<excludeKeys, value_type> {
		template<jsonifier::concepts::vector_like buffer_type> jsonifier_inline static void op(const value_type& value, buffer_type& buffer, uint64_t& index) {
			if (index + 32 >= buffer.size()) {
				buffer.resize((buffer.size() + index + 32) * 4);
			}
			auto start = buffer.data() + index;
			auto end   = toChars(start, value);
			index += std::distance(start, end);
		}
	};

	template<bool excludeKeys, jsonifier::concepts::enum_t value_type> struct serialize_impl<excludeKeys, value_type> {
		template<jsonifier::concepts::vector_like buffer_type> jsonifier_inline static void op(const value_type& value, buffer_type& buffer, uint64_t& index) {
			if (index + 32 >= buffer.size()) {
				buffer.resize((buffer.size() + index + 32) * 4);
			}
			auto start = buffer.data() + index;
			auto end   = toChars(start, static_cast<int64_t>(value));
			index += std::distance(start, end);
		}
	};

	template<bool excludeKeys, jsonifier::concepts::char_t value_type> struct serialize_impl<excludeKeys, value_type> {
		template<jsonifier::concepts::vector_like buffer_type> jsonifier_inline static void op(const value_type& value, buffer_type& buffer, uint64_t& index) {
			writeCharacter<json_structural_type::String>(buffer, index);
			switch (value) {
				case 0x22u: {
					static jsonifier_constexpr jsonifier::string_view stringViewNew{ "\\\"" };
					writeCharacters<stringViewNew>(buffer, index);
					break;
				}
				case 0x5Cu: {
					static jsonifier_constexpr jsonifier::string_view stringViewNew{ "\\\\" };
					writeCharacters<stringViewNew>(buffer, index);
					break;
				}
				case 0x08u: {
					static jsonifier_constexpr jsonifier::string_view stringViewNew{ "\\b" };
					writeCharacters<stringViewNew>(buffer, index);
					break;
				}
				case 0x0Cu: {
					static jsonifier_constexpr jsonifier::string_view stringViewNew{ "\\f" };
					writeCharacters<stringViewNew>(buffer, index);
					break;
				}
				case 0x0Au: {
					static jsonifier_constexpr jsonifier::string_view stringViewNew{ "\\n" };
					writeCharacters<stringViewNew>(buffer, index);
					break;
				}
				case 0x0Du: {
					static jsonifier_constexpr jsonifier::string_view stringViewNew{ "\\r" };
					writeCharacters<stringViewNew>(buffer, index);
					break;
				}
				case 0x09u: {
					static jsonifier_constexpr jsonifier::string_view stringViewNew{ "\\t" };
					writeCharacters<stringViewNew>(buffer, index);
					break;
				}
				default:
					writeCharacter(value, buffer, index);
			}
			writeCharacter<json_structural_type::String>(buffer, index);
		}
	};

	template<bool excludeKeys, jsonifier::concepts::string_t value_type> struct serialize_impl<excludeKeys, value_type> {
		template<jsonifier::concepts::vector_like buffer_type> jsonifier_inline static void op(const value_type& value, buffer_type& buffer, uint64_t& index) {
			const auto n = value.size();

			if jsonifier_constexpr (jsonifier::concepts::has_resize<buffer_type>) {
				const auto k = index + (4 * n);
				if (k >= buffer.size()) [[unlikely]] {
					buffer.resize((std::max)(buffer.size() * 2, k));
				}
			}
			writeCharacterUnChecked<json_structural_type::String>(buffer, index);
			uint64_t indexNew{};

			auto newPtr = serializeString(value.data(), buffer.data() + index, value.size(), indexNew);

			index += indexNew;

			indexNew = static_cast<uint64_t>(newPtr - value.data());
			for (auto iterBegin = value.begin() + static_cast<int64_t>(indexNew); iterBegin < value.end(); ++iterBegin) {
				switch (*iterBegin) {
					case 0x22u:
						std::memcpy(buffer.data() + index, R"(\")", 2);
						index += 2;
						break;
					case 0x5CU:
						std::memcpy(buffer.data() + index, R"(\\)", 2);
						index += 2;
						break;
					case 0x08u:
						std::memcpy(buffer.data() + index, R"(\b)", 2);
						index += 2;
						break;
					case 0x0Cu:
						std::memcpy(buffer.data() + index, R"(\f)", 2);
						index += 2;
						break;
					case 0x0Au:
						std::memcpy(buffer.data() + index, R"(\n)", 2);
						index += 2;
						break;
					case 0x0Du:
						std::memcpy(buffer.data() + index, R"(\r)", 2);
						index += 2;
						break;
					case 0x09u:
						std::memcpy(buffer.data() + index, R"(\t)", 2);
						index += 2;
						break;
					default:
						buffer[index++] = static_cast<uint8_t>(*iterBegin);
				}
			}

			writeCharacterUnChecked<json_structural_type::String>(buffer, index);
		}
	};

	template<bool excludeKeys, jsonifier::concepts::raw_json_t value_type> struct serialize_impl<excludeKeys, value_type> {
		template<jsonifier::concepts::vector_like buffer_type> jsonifier_inline static void op(const value_type& value, buffer_type& buffer, uint64_t& index) {
			jsonifier::string newValue = static_cast<const jsonifier::string>(value);
			serialize<excludeKeys>::op(newValue, buffer, index);
		}
	};

	template<bool excludeKeys, jsonifier::concepts::raw_array_t value_type> struct serialize_impl<excludeKeys, value_type> {
		template<jsonifier::concepts::vector_like buffer_type> jsonifier_inline static void op(const value_type& value, buffer_type& buffer, uint64_t& index) {
			const auto n = std::size(value);
			writeCharacter<json_structural_type::Array_Start>(buffer, index);
			for (uint64_t x = 0; x < n; ++x) {
				serialize<excludeKeys>::op(value[x], buffer, index);
				const bool needsComma = x < n - 1;
				if (needsComma) {
					writeCharacter<json_structural_type::Comma>(buffer, index);
				}
			}
			writeCharacter<json_structural_type::Array_End>(buffer, index);
		}
	};

	template<bool excludeKeys, jsonifier::concepts::vector_t value_type> struct serialize_impl<excludeKeys, value_type> {
		template<jsonifier::concepts::vector_like buffer_type> jsonifier_inline static void op(const value_type& value, buffer_type& buffer, uint64_t& index) {
			writeCharacter<json_structural_type::Array_Start>(buffer, index);

			if (value.size()) {
				bool first = true;
				for (auto iter = value.begin(); iter != value.end(); ++iter) {
					if (first) {
						first = false;
					} else {
						writeCharacter<json_structural_type::Comma>(buffer, index);
					}
					serialize<excludeKeys>::op(*iter, buffer, index);
				}
			}
			writeCharacter<json_structural_type::Array_End>(buffer, index);
		}
	};

	template<bool excludeKeys, jsonifier::concepts::map_t value_type> struct serialize_impl<excludeKeys, value_type> {
		template<jsonifier::concepts::vector_like buffer_type> jsonifier_inline static void op(const value_type& value, buffer_type& buffer, uint64_t& index) {
			writeCharacter<json_structural_type::Object_Start>(buffer, index);
			uint64_t currentIndex{};
			for (auto& [key, valueNew]: value) {
				serialize<excludeKeys>::op(key, buffer, index);
				writeCharacter<json_structural_type::Colon>(buffer, index);
				serialize<excludeKeys>::op(valueNew, buffer, index);
				const bool needsComma = currentIndex < value.size() - 1;
				if (needsComma) {
					writeCharacter<json_structural_type::Comma>(buffer, index);
				}
				++currentIndex;
			};
			writeCharacter<json_structural_type::Object_End>(buffer, index);
		}
	};

	template<bool excludeKeys, jsonifier::concepts::jsonifier_array_t value_type> struct serialize_impl<excludeKeys, value_type> {
		template<jsonifier::concepts::vector_like buffer_type> jsonifier_inline static void op(const value_type& value, buffer_type& buffer, uint64_t& index) {
			static jsonifier_constexpr auto size{ std::tuple_size_v<jsonifier::concepts::core_t<value_type>> };

			writeCharacter<json_structural_type::Array_Start>(buffer, index);
			forEach<size>([&](auto I) {
				auto& newMember = getMember(value, get<I>(jsonifier::concepts::coreV<value_type>));
				serialize<excludeKeys>::op(newMember, buffer, index);
				jsonifier_constexpr bool needsComma = I < size - 1;
				if jsonifier_constexpr (needsComma) {
					writeCharacter<json_structural_type::Comma>(buffer, index);
				}
			});
			writeCharacter<json_structural_type::Array_End>(buffer, index);
		}
	};

	jsonifier_constexpr bool needsEscaping(auto& S) {
		for (auto& c: S) {
			if (c == 0x22u) {
				return true;
			}
		}
		return false;
	}

	template<bool excludeKeys, jsonifier::concepts::jsonifier_object_t value_type> struct serialize_impl<excludeKeys, value_type> {
		template<jsonifier::concepts::vector_like buffer_type> jsonifier_inline static void op(const value_type& value, buffer_type& buffer, uint64_t& index) {
			writeCharacter<json_structural_type::Object_Start>(buffer, index);
			static jsonifier_constexpr auto n = std::tuple_size_v<jsonifier::concepts::core_t<value_type>>;

			bool first = true;
			forEach<n>([&](auto x) {
				static jsonifier_constexpr auto item = get<x>(jsonifier::concepts::coreV<value_type>);
				using item_type						 = decltype(item);
				using member_ptr_t					 = std::tuple_element_t<1, item_type>;
				using value_type_new				 = member_t<value_type, member_ptr_t>;

				if jsonifier_constexpr (jsonifier::concepts::null_t<value_type_new>) {
					auto isNull = [&]() {
						if jsonifier_constexpr (std::is_member_pointer_v<std::tuple_element_t<1, item_type>>) {
							return !bool(value.*get<1>(item));
						} else {
							return !bool(get<1>(item)(value));
						}
					}();
					if (isNull()) {
						return;
					}
				}

				if (first) {
					first = false;
				} else {
					writeCharacter<json_structural_type::Comma>(buffer, index);
				}

				using key_type = std::tuple_element_t<0, decltype(item)>;

				if jsonifier_constexpr (jsonifier::concepts::string_t<key_type> || jsonifier::concepts::char_t<key_type>) {
					static jsonifier_constexpr jsonifier::string_view key = get<0>(item);
					if jsonifier_constexpr (needsEscaping(key)) {
						serialize<excludeKeys>::op(key, buffer, index);
						writeCharacter<json_structural_type::Colon>(buffer, index);
					} else {
						static jsonifier_constexpr jsonifier::string_view string01{ "\"" };
						static jsonifier_constexpr jsonifier::string_view string02{ "\":" };
						static jsonifier_constexpr auto quoted = JoinV<string01, key, string02>;
						writeCharacters<quoted>(buffer, index);
					}
				}
				auto& newMember = getMember(value, get<1>(item));
				serialize<excludeKeys>::op(newMember, buffer, index);
			});
			writeCharacter<json_structural_type::Object_End>(buffer, index);
		}
	};

	template<jsonifier::concepts::jsonifier_object_t value_type> struct serialize_impl<true, value_type> {
		template<jsonifier::concepts::vector_like buffer_type> jsonifier_inline static void op(const value_type& value, buffer_type& buffer, uint64_t& index) {
			writeCharacter<json_structural_type::Object_Start>(buffer, index);
			static jsonifier_constexpr auto n = std::tuple_size_v<jsonifier::concepts::core_t<value_type>>;

			bool first = true;
			forEach<n>([&](auto x) {
				static jsonifier_constexpr auto item = get<x>(jsonifier::concepts::coreV<value_type>);
				using item_type						 = decltype(item);
				using member_ptr_t					 = std::tuple_element_t<1, item_type>;
				using value_type_new				 = member_t<value_type, member_ptr_t>;

				if jsonifier_constexpr (jsonifier::concepts::null_t<value_type_new>) {
					auto isNull = [&]() {
						if jsonifier_constexpr (std::is_member_pointer_v<std::tuple_element_t<1, item_type>>) {
							return !bool(value.*get<1>(item));
						} else {
							return !bool(get<1>(item)(value));
						}
					}();
					if (isNull()) {
						return;
					}
				}

				using key_type = std::tuple_element_t<0, item_type>;

				if jsonifier_constexpr (jsonifier::concepts::string_t<key_type> || jsonifier::concepts::char_t<key_type>) {
					static jsonifier_constexpr jsonifier::string_view key = get<0>(item);
					if (first) {
						first = false;
					} else {
						writeCharacter<json_structural_type::Comma>(buffer, index);
					}
					if jsonifier_constexpr (needsEscaping(key)) {
						serialize<true>::op(key, buffer, index);
						writeCharacter<json_structural_type::Colon>(buffer, index);
					} else {
						static jsonifier_constexpr jsonifier::string_view string01{ "\"" };
						static jsonifier_constexpr jsonifier::string_view string02{ "\":" };
						static jsonifier_constexpr auto quoted = JoinV<string01, key, string02>;
						writeCharacters<quoted>(buffer, index);
					}
				}
				auto& newMember	 = getMember(value, get<1>(item));
				using MemberType = decltype(newMember);
				if jsonifier_constexpr (jsonifier::concepts::has_excluded_keys<MemberType>) {
					serialize<true>::op(newMember, buffer, index, newMember.excludedKeys);
				} else {
					serialize<true>::op(newMember, buffer, index);
				}
			});
			writeCharacter<json_structural_type::Object_End>(buffer, index);
		}

		template<jsonifier::concepts::vector_like buffer_type, jsonifier::concepts::has_find KeyType>
		jsonifier_inline static void op(const value_type& value, buffer_type& buffer, uint64_t& index, const KeyType& excludedKeys) {
			writeCharacter<json_structural_type::Object_Start>(buffer, index);
			static jsonifier_constexpr auto n = std::tuple_size_v<jsonifier::concepts::core_t<value_type>>;

			bool first = true;
			forEach<n>([&](auto x) {
				static jsonifier_constexpr auto item = get<x>(jsonifier::concepts::coreV<value_type>);
				using item_type						 = decltype(item);
				using member_ptr_t					 = std::tuple_element_t<1, item_type>;
				using value_type_new				 = member_t<value_type, member_ptr_t>;

				if jsonifier_constexpr (jsonifier::concepts::null_t<value_type_new>) {
					auto isNull = [&]() {
						if jsonifier_constexpr (std::is_member_pointer_v<std::tuple_element_t<1, item_type>>) {
							return !bool(value.*get<1>(item));
						} else {
							return !bool(get<1>(item)(value));
						}
					}();
					if (isNull()) {
						return;
					}
				}

				using key_type = std::tuple_element_t<0, item_type>;

				if jsonifier_constexpr (jsonifier::concepts::string_t<key_type> || jsonifier::concepts::char_t<key_type>) {
					static jsonifier_constexpr jsonifier::string_view key = get<0>(item);
					if (excludedKeys.find(static_cast<const typename KeyType::key_type>(key)) != excludedKeys.end()) {
						return;
					}
					if (first) {
						first = false;
					} else {
						writeCharacter<json_structural_type::Comma>(buffer, index);
					}
					if jsonifier_constexpr (needsEscaping(key)) {
						serialize<true>::op(key, buffer, index);
						writeCharacter<json_structural_type::Colon>(buffer, index);
					} else {
						static jsonifier_constexpr jsonifier::string_view string01{ "\"" };
						static jsonifier_constexpr jsonifier::string_view string02{ "\":" };
						static jsonifier_constexpr auto quoted = JoinV<string01, key, string02>;
						writeCharacters<quoted>(buffer, index);
					}
				}
				auto& newMember	 = getMember(value, get<1>(item));
				using MemberType = decltype(newMember);
				if jsonifier_constexpr (jsonifier::concepts::has_excluded_keys<MemberType>) {
					serialize<true>::op(newMember, buffer, index, newMember.excludedKeys);
				} else {
					serialize<true>::op(newMember, buffer, index);
				}
			});
			writeCharacter<json_structural_type::Object_End>(buffer, index);
		}
	};
}