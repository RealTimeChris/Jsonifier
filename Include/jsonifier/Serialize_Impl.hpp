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

	template<json_structural_type c, jsonifier::concepts::is_fwd_iterator buffer_iterator_type> inline bool writeCharacter(buffer_iterator_type&& buffer) {
		buffer.potentiallyResize(1);
		buffer += c;
		return true;
	}

	template<jsonifier::concepts::is_fwd_iterator buffer_iterator_type> inline bool writeCharacter(uint8_t c, buffer_iterator_type&& buffer) {
		buffer.potentiallyResize(1);
		buffer += c;
		return true;
	}

	template<const jsonifier::string_view& str, jsonifier::concepts::is_fwd_iterator buffer_iterator_type> inline void writeCharacters(buffer_iterator_type&& buffer) {
		const auto n = str.size();
		buffer.potentiallyResize(n);
		buffer.writeData(str.data(), n);
	}

	template<uint64_t size, jsonifier::concepts::is_fwd_iterator buffer_iterator_type> inline void writeCharacters(const char (&str)[size], buffer_iterator_type&& buffer) {
		auto n = size - 1;
		buffer.potentiallyResize(n);
		buffer.writeData(str, n);
	}

	template<json_structural_type c, jsonifier::concepts::is_fwd_iterator buffer_iterator_type> inline void writeCharacterUnChecked(buffer_iterator_type&& buffer) {
		buffer += c;
	}

	template<uint64_t size, jsonifier::concepts::is_fwd_iterator buffer_iterator_type>
	inline void writeCharactersUnchecked(const char (&str)[size], buffer_iterator_type&& buffer) {
		buffer.writeData(str, size - 1);
	}

	template<jsonifier::concepts::null_t value_type_new> struct serialize_impl<value_type_new> {
		template<jsonifier::concepts::null_t value_type, jsonifier::concepts::is_fwd_iterator buffer_iterator_type>
		inline static void op(value_type&&, buffer_iterator_type&& buffer) {
			writeCharacters("null", buffer);
		}
	};

	template<jsonifier::concepts::bool_t value_type_new> struct serialize_impl<value_type_new> {
		template<jsonifier::concepts::bool_t value_type, jsonifier::concepts::is_fwd_iterator buffer_iterator_type>
		inline static void op(value_type&& value, buffer_iterator_type&& buffer) {
			if (value) {
				writeCharacters("true", buffer);
			} else {
				writeCharacters("false", buffer);
			}
		}
	};

	template<jsonifier::concepts::num_t value_type_new> struct serialize_impl<value_type_new> {
		template<jsonifier::concepts::num_t value_type, jsonifier::concepts::is_fwd_iterator buffer_iterator_type>
		inline static void op(value_type&& value, buffer_iterator_type&& buffer) {
			buffer.potentiallyResize(32);
			auto start = buffer.data() + buffer.currentSize;
			auto end   = toChars(start, value);
			buffer.currentSize += end - start;
		}
	};

	template<jsonifier::concepts::enum_t value_type_new> struct serialize_impl<value_type_new> {
		template<jsonifier::concepts::enum_t value_type, jsonifier::concepts::is_fwd_iterator buffer_iterator_type>
		inline static void op(value_type&& value, buffer_iterator_type&& buffer) {
			buffer.potentiallyResize(32);
			auto start = buffer.data() + buffer.currentSize;
			auto end   = toChars(start, static_cast<int64_t>(value));
			buffer.currentSize += end - start;
		}
	};

	template<jsonifier::concepts::char_t value_type_new> struct serialize_impl<value_type_new> {
		template<jsonifier::concepts::char_t value_type, jsonifier::concepts::is_fwd_iterator buffer_iterator_type>
		inline static void op(value_type&& value, buffer_iterator_type&& buffer) {
			writeCharacter<json_structural_type::String>(buffer);
			switch (value) {
				case 0x22u: {
					writeCharacters("\\\"", buffer);
					break;
				}
				case 0x5CU: {
					writeCharacters("\\\\", buffer);
					break;
				}
				case 0x08u: {
					writeCharacters("\\b", buffer);
					break;
				}
				case 0x0Cu: {
					writeCharacters("\\f", buffer);
					break;
				}
				case 0x0Au: {
					writeCharacters("\\n", buffer);
					break;
				}
				case 0x0Du: {
					writeCharacters("\\r", buffer);
					break;
				}
				case 0x09u: {
					writeCharacters("\\t", buffer);
					break;
				}
				default: {
					buffer += value;
				}
			}
			writeCharacter<json_structural_type::String>(buffer);
		}
	};

	template<jsonifier::concepts::string_t value_type_new> struct serialize_impl<value_type_new> {
		template<jsonifier::concepts::string_t value_type, jsonifier::concepts::is_fwd_iterator buffer_iterator_type>
		inline static void op(value_type&& value, buffer_iterator_type&& buffer) {
			auto n = static_cast<uint64_t>(value.size());

			buffer.potentiallyResize(buffer.currentSize + (4 * n));
			writeCharacterUnChecked<json_structural_type::String>(buffer);

			n = 0;

			auto newPtr = serializeString(value.data(), buffer.data() + buffer.currentSize, value.size(), n);

			buffer.currentSize += n;

			n = static_cast<uint64_t>(newPtr - value.data());
			for (auto iterBegin = value.begin() + static_cast<int64_t>(n); iterBegin < value.end(); ++iterBegin) {
				switch (*iterBegin) {
					case 0x22u: {
						writeCharactersUnchecked("\\\"", buffer);
						break;
					}
					case 0x5CU: {
						writeCharactersUnchecked("\\\\", buffer);
						break;
					}
					case 0x08u: {
						writeCharactersUnchecked("\\b", buffer);
						break;
					}
					case 0x0Cu: {
						writeCharactersUnchecked("\\f", buffer);
						break;
					}
					case 0x0Au: {
						writeCharactersUnchecked("\\n", buffer);
						break;
					}
					case 0x0Du: {
						writeCharactersUnchecked("\\r", buffer);
						break;
					}
					case 0x09u: {
						writeCharactersUnchecked("\\t", buffer);
						break;
					}
					default: {
						buffer += *iterBegin;
					}
				}
			}

			writeCharacterUnChecked<json_structural_type::String>(buffer);
		}
	};

	template<jsonifier::concepts::raw_json_t value_type_new> struct serialize_impl<value_type_new> {
		template<jsonifier::concepts::raw_json_t value_type, jsonifier::concepts::is_fwd_iterator buffer_iterator_type>
		inline static void op(value_type&& value, buffer_iterator_type&& buffer) {
			jsonifier::string newValue = static_cast<const jsonifier::string>(value);
			serialize::op(newValue, buffer);
		}
	};

	template<jsonifier::concepts::raw_array_t value_type_new> struct serialize_impl<value_type_new> {
		template<jsonifier::concepts::raw_array_t value_type, jsonifier::concepts::is_fwd_iterator buffer_iterator_type>
		inline static void op(value_type&& value, buffer_iterator_type&& buffer) {
			const auto n = std::size(value);
			writeCharacter<json_structural_type::Array_Start>(buffer);
			for (uint64_t x = 0; x < n; ++x) {
				using member_type = decltype(value[0]);
				if constexpr (jsonifier::concepts::has_excluded_keys<member_type>) {
					serialize::op(value[x], buffer, value[x].jsonifierExcludedKeys);
				} else {
					serialize::op(value[x], buffer);
				}
				const bool needsComma = x < n - 1;
				if (needsComma) {
					writeCharacter<json_structural_type::Comma>(buffer);
				}
			}
			writeCharacter<json_structural_type::Array_End>(buffer);
		}
	};

	template<jsonifier::concepts::vector_t value_type_new> struct serialize_impl<value_type_new> {
		template<jsonifier::concepts::vector_t value_type, jsonifier::concepts::is_fwd_iterator buffer_iterator_type>
		inline static void op(value_type&& value, buffer_iterator_type&& buffer) {
			writeCharacter<json_structural_type::Array_Start>(buffer);

			if (value.size()) {
				bool first = true;
				for (auto iter = value.begin(); iter != value.end(); ++iter) {
					if (first) {
						first = false;
					} else {
						writeCharacter<json_structural_type::Comma>(buffer);
					}
					if constexpr (jsonifier::concepts::has_excluded_keys<typename value_type_new::value_type>) {
						serialize::op(*iter, buffer, iter->jsonifierExcludedKeys);
					} else {
						serialize::op(*iter, buffer);
					}
				}
			}
			writeCharacter<json_structural_type::Array_End>(buffer);
		}
	};

	template<jsonifier::concepts::map_t value_type_new> struct serialize_impl<value_type_new> {
		template<jsonifier::concepts::map_t value_type, jsonifier::concepts::is_fwd_iterator buffer_iterator_type>
		inline static void op(value_type&& value, buffer_iterator_type&& buffer) {
			writeCharacter<json_structural_type::Object_Start>(buffer);
			uint64_t currentIndex{};
			for (const auto& [key, values]: value) {
				using member_type = jsonifier::concepts::unwrap<decltype(values)>;
				writeCharacter<json_structural_type::Colon>(buffer);
				if constexpr (jsonifier::concepts::has_excluded_keys<member_type>) {
					serialize::op(values, buffer, values.jsonifierExcludedKeys);
				} else {
					serialize::op(values, buffer);
				}
				const bool needsComma = currentIndex < value.size() - 1;
				if (needsComma) {
					writeCharacter<json_structural_type::Comma>(buffer);
				}
				++currentIndex;
			};
			writeCharacter<json_structural_type::Object_End>(buffer);
		}
	};

	template<jsonifier::concepts::jsonifier_array_t value_type_new> struct serialize_impl<value_type_new> {
		template<jsonifier::concepts::jsonifier_array_t value_type, jsonifier::concepts::is_fwd_iterator buffer_iterator_type>
		inline static void op(value_type&& value, buffer_iterator_type&& buffer) {
			static constexpr auto size{ std::tuple_size_v<jsonifier::concepts::core_t<value_type_new>> };

			writeCharacter<json_structural_type::Array_Start>(buffer);
			forEach<size>([&](auto x) {
				auto& newMember	  = getMember(value, get<x>(jsonifier::concepts::coreV<value_type_new>));
				using member_type = jsonifier::concepts::unwrap<decltype(newMember)>;
				if constexpr (jsonifier::concepts::has_excluded_keys<member_type>) {
					serialize::op(newMember, buffer, newMember.jsonifierExcludedKeys);
				} else {
					serialize::op(newMember, buffer);
				}
				constexpr bool needsComma = x < size - 1;
				if constexpr (needsComma) {
					writeCharacter<json_structural_type::Comma>(buffer);
				}
			});
			writeCharacter<json_structural_type::Array_End>(buffer);
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

	template<jsonifier::concepts::jsonifier_object_t value_type_new> struct serialize_impl<value_type_new> {
		template<jsonifier::concepts::jsonifier_object_t value_type, jsonifier::concepts::is_fwd_iterator buffer_iterator_type>
		inline static void op(value_type&& value, buffer_iterator_type&& buffer) {
			writeCharacter<json_structural_type::Object_Start>(buffer);
			static constexpr auto n = std::tuple_size_v<jsonifier::concepts::core_t<value_type_new>>;

			bool first = true;
			forEach<n>([&](auto x) {
				static constexpr auto item = get<x>(jsonifier::concepts::coreV<value_type_new>);
				using item_type			   = jsonifier::concepts::unwrap<decltype(item)>;
				using member_ptr_t		   = std::tuple_element_t<1, item_type>;
				using value_type_newer	   = member_t<value_type_new, member_ptr_t>;

				if constexpr (jsonifier::concepts::null_t<value_type_newer>) {
					auto isNull = [&]() -> bool {
						if constexpr (std::is_member_pointer_v<std::tuple_element_t<1, item_type>>) {
							return !bool(value.*get<1>(item));
						} else {
							return !bool(get<1>(item)(value));
						}
					};
					if (isNull()) {
						return;
					}
				}

				first ? first = false : writeCharacter<json_structural_type::Comma>(buffer);

				static constexpr jsonifier::string_view key = get<0>(item);
				writeCharacters("\"", buffer);
				writeCharacters<key>(buffer);
				writeCharacters("\":", buffer);

				auto& newMember	  = getMember(value, get<1>(item));
				using member_type = jsonifier::concepts::unwrap<decltype(newMember)>;
				if constexpr (jsonifier::concepts::has_excluded_keys<member_type>) {
					serialize::op(newMember, buffer, newMember.jsonifierExcludedKeys);
				} else {
					serialize::op(newMember, buffer);
				}
			});
			writeCharacter<json_structural_type::Object_End>(buffer);
		}

		template<jsonifier::concepts::jsonifier_object_t value_type, jsonifier::concepts::is_fwd_iterator buffer_iterator_type, jsonifier::concepts::has_find key_type>
		inline static void op(value_type&& value, buffer_iterator_type&& buffer, key_type&& excludedKeys) {
			writeCharacter<json_structural_type::Object_Start>(buffer);
			static constexpr auto n = std::tuple_size_v<jsonifier::concepts::core_t<value_type_new>>;

			bool first = true;
			forEach<n>([&](auto x) {
				static constexpr auto item = get<x>(jsonifier::concepts::coreV<value_type_new>);
				using item_type			   = jsonifier::concepts::unwrap<decltype(item)>;
				using member_ptr_t		   = std::tuple_element_t<1, item_type>;
				using value_type_newer	   = member_t<value_type, member_ptr_t>;

				if constexpr (jsonifier::concepts::null_t<value_type_newer>) {
					auto isNull = [&]() -> bool {
						if constexpr (std::is_member_pointer_v<std::tuple_element_t<1, item_type>>) {
							return !bool(value.*get<1>(item));
						} else {
							return !bool(get<1>(item)(value));
						}
					};
					if (isNull()) {
						return;
					}
				}

				static constexpr jsonifier::string_view key = get<0>(item);
				if (excludedKeys.find(static_cast<const typename jsonifier::concepts::unwrap<key_type>::key_type>(key)) != excludedKeys.end()) {
					return;
				}
				first ? first = false : writeCharacter<json_structural_type::Comma>(buffer);
				writeCharacters("\"", buffer);
				writeCharacters<key>(buffer);
				writeCharacters("\":", buffer);

				auto& newMember	  = getMember(value, get<1>(item));
				using member_type = jsonifier::concepts::unwrap<decltype(newMember)>;
				if constexpr (jsonifier::concepts::has_excluded_keys<member_type>) {
					serialize::op(newMember, buffer, newMember.jsonifierExcludedKeys);
				} else {
					serialize::op(newMember, buffer);
				}
			});
			writeCharacter<json_structural_type::Object_End>(buffer);
		}
	};
}