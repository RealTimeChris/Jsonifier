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

	template<jsonifier::concepts::buffer_like buffer_type> inline void writeCharacter(char c, buffer_type& buffer, uint64_t& index) {
		if (index + 1 >= buffer.size()) {
			buffer.resize(buffer.size() == 0 ? index + 1 : (buffer.size() + index) * 2);
		}

		buffer[index++] = c;
	}

	template<json_structural_type c, jsonifier::concepts::buffer_like buffer_type> inline void writeCharacter(buffer_type& buffer, uint64_t& index) {
		if (index + 1 >= buffer.size()) {
			buffer.resize(buffer.size() == 0 ? index + 1 : (buffer.size() + index) * 2);
		}

		buffer[index++] = static_cast<typename jsonifier::concepts::unwrap<buffer_type>::value_type>(c);
	}

	template<json_structural_type c, jsonifier::concepts::buffer_like buffer_type> inline void writeCharacterUnChecked(buffer_type& buffer, uint64_t& index) {
		buffer[index++] = static_cast<typename jsonifier::concepts::unwrap<buffer_type>::value_type>(c);
	}

	template<const jsonifier::string_view& str, jsonifier::concepts::buffer_like buffer_type> inline void writeCharacters(buffer_type& buffer, uint64_t& index) {
		const auto n = str.size();
		if (index + n >= buffer.size()) {
			buffer.resize((std::max)(buffer.size() * 2, index + n));
		}

		std::memcpy(buffer.data() + index, str.data(), n);
		index += n;
	}

	template<jsonifier::concepts::buffer_like buffer_type> inline void writeCharacters(const jsonifier::string_view str, buffer_type& buffer, uint64_t& index) {
		const auto n = str.size();
		if (index + n >= buffer.size()) {
			buffer.resize((std::max)(buffer.size() * 2, index + n));
		}

		std::memcpy(buffer.data() + index, str.data(), n);
		index += n;
	}

	template<jsonifier::concepts::null_t value_type> struct serialize_impl<value_type> {
		template<jsonifier::concepts::buffer_like buffer_type> inline static void op(const value_type&, buffer_type& buffer, uint64_t& index) {
			writeCharacters("null", buffer, index);
		}
	};

	template<jsonifier::concepts::bool_t value_type> struct serialize_impl<value_type> {
		template<jsonifier::concepts::buffer_like buffer_type> inline static void op(const value_type& value, buffer_type& buffer, uint64_t& index) {
			if (value) {
				writeCharacters("true", buffer, index);
			} else {
				writeCharacters("false", buffer, index);
			}
		}
	};

	template<jsonifier::concepts::num_t value_type> struct serialize_impl<value_type> {
		template<jsonifier::concepts::buffer_like buffer_type> inline static void op(const value_type& value, buffer_type& buffer, uint64_t& index) {
			if (index + 32 >= buffer.size()) {
				buffer.resize((buffer.size() + index + 32) * 4);
			}
			auto start = buffer.data() + index;
			auto end   = toChars(start, value);
			index += end - start;
		}
	};

	template<jsonifier::concepts::enum_t value_type> struct serialize_impl<value_type> {
		template<jsonifier::concepts::buffer_like buffer_type> inline static void op(const value_type& value, buffer_type& buffer, uint64_t& index) {
			if (index + 32 >= buffer.size()) {
				buffer.resize((buffer.size() + index + 32) * 4);
			}
			auto start = buffer.data() + index;
			auto end   = toChars(start, static_cast<int64_t>(value));
			index += end - start;
		}
	};

	template<jsonifier::concepts::char_t value_type> struct serialize_impl<value_type> {
		template<jsonifier::concepts::buffer_like buffer_type> inline static void op(const value_type& value, buffer_type& buffer, uint64_t& index) {
			writeCharacter<json_structural_type::String>(buffer, index);
			switch (value) {
				case 0x22u:
					writeCharacters(R"(\")", buffer, index);
					break;
				case 0x5CU:
					writeCharacters(R"(\\)", buffer, index);
					break;
				case 0x08u:
					writeCharacters(R"(\b)", buffer, index);
					break;
				case 0x0Cu:
					writeCharacters(R"(\f)", buffer, index);
					break;
				case 0x0Au:
					writeCharacters(R"(\n)", buffer, index);
					break;
				case 0x0Du:
					writeCharacters(R"(\r)", buffer, index);
					break;
				case 0x09u:
					writeCharacters(R"(\t)", buffer, index);
					break;
				default:
					writeCharacter(value, buffer, index);
			}
			writeCharacter<json_structural_type::String>(buffer, index);
		}
	};

	template<jsonifier::concepts::string_t value_type> struct serialize_impl<value_type> {
		template<jsonifier::concepts::buffer_like buffer_type> inline static void op(const value_type& value, buffer_type& buffer, uint64_t& index) {
			auto n = static_cast<uint64_t>(value.size());

			if constexpr (jsonifier::concepts::has_resize<buffer_type>) {
				const auto k = index + (4 * n);
				if (k >= buffer.size()) [[unlikely]] {
					buffer.resize((std::max)(buffer.size() * 2, k));
				}
			}
			writeCharacterUnChecked<json_structural_type::String>(buffer, index);

			n = 0;

			auto newPtr = serializeString(value.data(), buffer.data() + index, value.size(), n);

			index += n;

			n = static_cast<uint64_t>(newPtr - value.data());
			for (auto iterBegin = value.begin() + static_cast<int64_t>(n); iterBegin < value.end(); ++iterBegin) {
				switch (*iterBegin) {
					case 0x22u:
						writeCharacters(R"(\")", buffer, index);
						break;
					case 0x5CU:
						writeCharacters(R"(\\)", buffer, index);
						break;
					case 0x08u:
						writeCharacters(R"(\b)", buffer, index);
						break;
					case 0x0Cu:
						writeCharacters(R"(\f)", buffer, index);
						break;
					case 0x0Au:
						writeCharacters(R"(\n)", buffer, index);
						break;
					case 0x0Du:
						writeCharacters(R"(\r)", buffer, index);
						break;
					case 0x09u:
						writeCharacters(R"(\t)", buffer, index);
						break;
					default:
						buffer[index++] = static_cast<uint8_t>(*iterBegin);
				}
			}

			writeCharacterUnChecked<json_structural_type::String>(buffer, index);
		}
	};

	template<jsonifier::concepts::raw_json_t value_type> struct serialize_impl<value_type> {
		template<jsonifier::concepts::buffer_like buffer_type> inline static void op(const value_type& value, buffer_type& buffer, uint64_t& index) {
			jsonifier::string newValue = static_cast<const jsonifier::string>(value);
			serialize::op(newValue, buffer, index);
		}
	};

	template<jsonifier::concepts::raw_array_t value_type> struct serialize_impl<value_type> {
		template<jsonifier::concepts::buffer_like buffer_type> inline static void op(const value_type& value, buffer_type& buffer, uint64_t& index) {
			const auto n = std::size(value);
			writeCharacter<json_structural_type::Array_Start>(buffer, index);
			for (uint64_t x = 0; x < n; ++x) {
				using member_type = decltype(value[0]);
				if constexpr (jsonifier::concepts::has_excluded_keys<member_type>) {
					serialize::op(value[x], buffer, index, value[x].jsonifierExcludedKeys);
				} else {
					serialize::op(value[x], buffer, index);
				}
				const bool needsComma = x < n - 1;
				if (needsComma) {
					writeCharacter<json_structural_type::Comma>(buffer, index);
				}
			}
			writeCharacter<json_structural_type::Array_End>(buffer, index);
		}
	};

	template<jsonifier::concepts::vector_t value_type> struct serialize_impl<value_type> {
		template<jsonifier::concepts::buffer_like buffer_type> inline static void op(const value_type& value, buffer_type& buffer, uint64_t& index) {
			writeCharacter<json_structural_type::Array_Start>(buffer, index);

			if (value.size()) {
				bool first = true;
				for (auto iter = value.begin(); iter != value.end(); ++iter) {
					if (first) {
						first = false;
					} else {
						writeCharacter<json_structural_type::Comma>(buffer, index);
					}
					if constexpr (jsonifier::concepts::has_excluded_keys<typename value_type::value_type>) {
						serialize::op(*iter, buffer, index, iter->jsonifierExcludedKeys);
					} else {
						serialize::op(*iter, buffer, index);
					}
				}
			}
			writeCharacter<json_structural_type::Array_End>(buffer, index);
		}
	};

	template<jsonifier::concepts::map_t value_type> struct serialize_impl<value_type> {
		template<jsonifier::concepts::buffer_like buffer_type> inline static void op(const value_type& value, buffer_type& buffer, uint64_t& index) {
			writeCharacter<json_structural_type::Object_Start>(buffer, index);
			uint64_t currentIndex{};
			for (auto& [key, values]: value) {
				using member_type = decltype(values);
				writeCharacter<json_structural_type::Colon>(buffer, index);
				if constexpr (jsonifier::concepts::has_excluded_keys<member_type>) {
					serialize::op(values, buffer, index, values.jsonifierExcludedKeys);
				} else {
					serialize::op(values, buffer, index);
				}
				const bool needsComma = currentIndex < value.size() - 1;
				if (needsComma) {
					writeCharacter<json_structural_type::Comma>(buffer, index);
				}
				++currentIndex;
			};
			writeCharacter<json_structural_type::Object_End>(buffer, index);
		}
	};

	template<jsonifier::concepts::jsonifier_array_t value_type> struct serialize_impl<value_type> {
		template<jsonifier::concepts::buffer_like buffer_type> inline static void op(const value_type& value, buffer_type& buffer, uint64_t& index) {
			static constexpr auto size{ std::tuple_size_v<jsonifier::concepts::core_t<value_type>> };

			writeCharacter<json_structural_type::Array_Start>(buffer, index);
			forEach<size>([&](auto x) {
				auto& newMember	  = getMember(value, get<x>(jsonifier::concepts::coreV<value_type>));
				using member_type = decltype(newMember);
				if constexpr (jsonifier::concepts::has_excluded_keys<member_type>) {
					serialize::op(newMember, buffer, index, newMember.jsonifierExcludedKeys);
				} else {
					serialize::op(newMember, buffer, index);
				}
				constexpr bool needsComma = x < size - 1;
				if constexpr (needsComma) {
					writeCharacter<json_structural_type::Comma>(buffer, index);
				}
			});
			writeCharacter<json_structural_type::Array_End>(buffer, index);
		}
	};

	constexpr bool needsEscaping(auto& S) {
		for (auto& c: S) {
			if (c == 0x22u) {
				return true;
			}
		}
		return false;
	}

	template<jsonifier::concepts::jsonifier_object_t value_type> struct serialize_impl<value_type> {
		template<jsonifier::concepts::buffer_like buffer_type> inline static void op(const value_type& value, buffer_type& buffer, uint64_t& index) {
			writeCharacter<json_structural_type::Object_Start>(buffer, index);
			static constexpr auto n = std::tuple_size_v<jsonifier::concepts::core_t<value_type>>;

			bool first = true;
			forEach<n>([&](auto x) {
				static constexpr auto item = get<x>(jsonifier::concepts::coreV<value_type>);
				using item_type			   = decltype(item);
				using member_ptr_t		   = std::tuple_element_t<1, item_type>;
				using value_type_new	   = member_t<value_type, member_ptr_t>;

				if constexpr (jsonifier::concepts::null_t<value_type_new>) {
					auto isNull = [&]() {
						if constexpr (std::is_member_pointer_v<std::tuple_element_t<1, item_type>>) {
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

				static constexpr jsonifier::string_view key = get<0>(item);
				writeCharacters(R"(")", buffer, index);
				writeCharacters<key>(buffer, index);
				writeCharacters(R"(":)", buffer, index);

				auto& newMember	  = getMember(value, get<1>(item));
				using member_type = decltype(newMember);
				if constexpr (jsonifier::concepts::has_excluded_keys<member_type>) {
					serialize::op(newMember, buffer, index, newMember.jsonifierExcludedKeys);
				} else {
					serialize::op(newMember, buffer, index);
				}
			});
			writeCharacter<json_structural_type::Object_End>(buffer, index);
		}

		template<jsonifier::concepts::buffer_like buffer_type, jsonifier::concepts::has_find key_type>
		inline static void op(const value_type& value, buffer_type& buffer, uint64_t& index, const key_type& excludedKeys) {
			writeCharacter<json_structural_type::Object_Start>(buffer, index);
			static constexpr auto n = std::tuple_size_v<jsonifier::concepts::core_t<value_type>>;

			bool first = true;
			forEach<n>([&](auto x) {
				static constexpr auto item = get<x>(jsonifier::concepts::coreV<value_type>);
				using item_type			   = decltype(item);
				using member_ptr_t		   = std::tuple_element_t<1, item_type>;
				using value_type_new	   = member_t<value_type, member_ptr_t>;

				if constexpr (jsonifier::concepts::null_t<value_type_new>) {
					auto isNull = [&]() {
						if constexpr (std::is_member_pointer_v<std::tuple_element_t<1, item_type>>) {
							return !bool(value.*get<1>(item));
						} else {
							return !bool(get<1>(item)(value));
						}
					}();
					if (isNull()) {
						return;
					}
				}

				static constexpr jsonifier::string_view key = get<0>(item);
				if (excludedKeys.find(static_cast<const typename key_type::key_type>(key)) != excludedKeys.end()) {
					return;
				}
				if (first) {
					first = false;
				} else {
					writeCharacter<json_structural_type::Comma>(buffer, index);
				}
				writeCharacters(R"(")", buffer, index);
				writeCharacters<key>(buffer, index);
				writeCharacters(R"(":)", buffer, index);

				auto& newMember	  = getMember(value, get<1>(item));
				using member_type = decltype(newMember);
				if constexpr (jsonifier::concepts::has_excluded_keys<member_type>) {
					serialize::op(newMember, buffer, index, newMember.jsonifierExcludedKeys);
				} else {
					serialize::op(newMember, buffer, index);
				}
			});
			writeCharacter<json_structural_type::Object_End>(buffer, index);
		}
	};
}