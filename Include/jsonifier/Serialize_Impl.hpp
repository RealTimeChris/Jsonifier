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

	template<jsonifier::concepts::always_null_t value_type_new> struct serialize_impl<value_type_new> {
		template<bool shortStringsSupport, jsonifier::concepts::always_null_t value_type, jsonifier::concepts::is_fwd_iterator iterator_type>
		JSONIFIER_INLINE static void op(value_type&&, iterator_type&& buffer) {
			buffer.writeCharacters("null");
		}
	};

	template<jsonifier::concepts::bool_t value_type_new> struct serialize_impl<value_type_new> {
		template<bool shortStringsSupport, jsonifier::concepts::bool_t value_type, jsonifier::concepts::is_fwd_iterator iterator_type>
		JSONIFIER_INLINE static void op(value_type&& value, iterator_type&& buffer) {
			value ? buffer.writeCharacters("true") : buffer.writeCharacters("false");
		}
	};

	template<jsonifier::concepts::num_t value_type_new> struct serialize_impl<value_type_new> {
		template<bool shortStringsSupport, jsonifier::concepts::num_t value_type, jsonifier::concepts::is_fwd_iterator iterator_type>
		JSONIFIER_INLINE static void op(value_type&& value, iterator_type&& buffer) {
			buffer.potentiallyResize(32);
			auto end = toChars(buffer.data() + buffer.currentSize, value);
			buffer.currentSize += end - (buffer.data() + buffer.currentSize);
		}
	};

	template<jsonifier::concepts::enum_t value_type_new> struct serialize_impl<value_type_new> {
		template<bool shortStringsSupport, jsonifier::concepts::enum_t value_type, jsonifier::concepts::is_fwd_iterator iterator_type>
		JSONIFIER_INLINE static void op(value_type&& value, iterator_type&& buffer) {
			buffer.potentiallyResize(32);
			auto end = toChars(buffer.data() + buffer.currentSize, static_cast<int64_t>(value));
			buffer.currentSize += end - (buffer.data() + buffer.currentSize);
		}
	};

	template<jsonifier::concepts::char_t value_type_new> struct serialize_impl<value_type_new> {
		template<bool shortStringsSupport, jsonifier::concepts::char_t value_type, jsonifier::concepts::is_fwd_iterator iterator_type>
		JSONIFIER_INLINE static void op(value_type&& value, iterator_type&& buffer) {
			buffer.template writeCharacter<json_structural_type, json_structural_type::String>();
			switch (value) {
				case 0x08u: {
					buffer.writeCharacters("\\b");
					break;
				}
				case 0x09u: {
					buffer.writeCharacters("\\t");
					break;
				}
				case 0x0Au: {
					buffer.writeCharacters("\\n");
					break;
				}
				case 0x0Cu: {
					buffer.writeCharacters("\\f");
					break;
				}
				case 0x0Du: {
					buffer.writeCharacters("\\r");
					break;
				}
				case 0x22u: {
					buffer.writeCharacters("\\\"");
					break;
				}
				case 0x5CU: {
					buffer.writeCharacters("\\\\");
					break;
				}
				default: {
					buffer.writeCharacter(value);
				}
			}
			buffer.template writeCharacter<json_structural_type, json_structural_type::String>();
		}
	};

	template<jsonifier::concepts::string_t value_type_new> struct serialize_impl<value_type_new> {
		template<bool shortStringsSupport, jsonifier::concepts::string_t value_type, jsonifier::concepts::is_fwd_iterator iterator_type>
		JSONIFIER_INLINE static void op(value_type&& value, iterator_type&& buffer) {
			auto n = static_cast<uint64_t>(value.size());

			buffer.potentiallyResize(buffer.currentSize + (4 * n));
			buffer.template writeCharacterUnchecked<json_structural_type, json_structural_type::String>();

			n = 0;

			auto newPtr = serializeString<shortStringsSupport>(value.data(), buffer.data() + buffer.currentSize, value.size(), n);

			buffer.currentSize += n;

			n = static_cast<uint64_t>(newPtr - value.data());
			for (; n < value.size(); ++n) {
				switch (value[n]) {
					case 0x08u: {
						buffer.writeCharactersUnchecked("\\b");
						break;
					}
					case 0x09u: {
						buffer.writeCharactersUnchecked("\\t");
						break;
					}
					case 0x0Au: {
						buffer.writeCharactersUnchecked("\\n");
						break;
					}
					case 0x0Cu: {
						buffer.writeCharactersUnchecked("\\f");
						break;
					}
					case 0x0Du: {
						buffer.writeCharactersUnchecked("\\r");
						break;
					}
					case 0x22u: {
						buffer.writeCharactersUnchecked("\\\"");
						break;
					}
					case 0x5CU: {
						buffer.writeCharactersUnchecked("\\\\");
						break;
					}
					default: {
						buffer.writeCharacterUnchecked(value[n]);
					}
				}
			}

			buffer.template writeCharacterUnchecked<json_structural_type, json_structural_type::String>();
		}
	};

	template<jsonifier::concepts::raw_json_t value_type_new> struct serialize_impl<value_type_new> {
		template<bool shortStringsSupport, jsonifier::concepts::raw_json_t value_type, jsonifier::concepts::is_fwd_iterator iterator_type>
		JSONIFIER_INLINE static void op(value_type&& value, iterator_type&& buffer) {
			serialize::template op<shortStringsSupport>(static_cast<const jsonifier::string>(value), buffer);
		}
	};

	template<jsonifier::concepts::raw_array_t value_type_new> struct serialize_impl<value_type_new> {
		template<bool shortStringsSupport, jsonifier::concepts::raw_array_t value_type, jsonifier::concepts::is_fwd_iterator iterator_type>
		JSONIFIER_INLINE static void op(value_type& value, iterator_type&& buffer) {
			static constexpr auto n = std::size(value);
			buffer.template writeCharacter<json_structural_type, json_structural_type::Array_Start>();
			if constexpr (n > 0) {
				serialize::template op<shortStringsSupport>(value[0], buffer);
				for (uint64_t x = 0; x < n; ++x) {
					buffer.template writeCharacter<json_structural_type, json_structural_type::Comma>();
					serialize::template op<shortStringsSupport>(value[x], buffer);
				}
			}

			buffer.template writeCharacter<json_structural_type, json_structural_type::Array_End>();
		}
	};

	template<jsonifier::concepts::vector_t value_type_new> struct serialize_impl<value_type_new> {
		template<bool shortStringsSupport, jsonifier::concepts::vector_t value_type, jsonifier::concepts::is_fwd_iterator iterator_type>
		JSONIFIER_INLINE static void op(value_type&& value, iterator_type&& buffer) {
			buffer.template writeCharacter<json_structural_type, json_structural_type::Array_Start>();

			if (value.size() > 0) {
				auto iter = value.begin();
				serialize::template op<shortStringsSupport>(*iter, buffer);
				++iter;
				for (; iter != value.end(); ++iter) {
					buffer.template writeCharacter<json_structural_type, json_structural_type::Comma>();
					serialize::template op<shortStringsSupport>(*iter, buffer);
				}
			}

			buffer.template writeCharacter<json_structural_type, json_structural_type::Array_End>();
		}
	};

	template<jsonifier::concepts::map_t value_type_new> struct serialize_impl<value_type_new> {
		template<bool shortStringsSupport, jsonifier::concepts::map_t value_type, jsonifier::concepts::is_fwd_iterator iterator_type>
		JSONIFIER_INLINE static void op(value_type&& value, iterator_type&& buffer) {
			buffer.template writeCharacter<json_structural_type, json_structural_type::Object_Start>();
			bool first{ true };
			auto setToFalseLambda = [&]() {
				first = false;
			};
			if (value.size() > 0) {
				for (auto& [key, valueNew]: value) {
					first ? setToFalseLambda() : buffer.template writeCharacter<json_structural_type, json_structural_type::Comma>();
					serialize::template op<shortStringsSupport>(key, buffer);
					buffer.template writeCharacter<json_structural_type, json_structural_type::Colon>();
					serialize::template op<shortStringsSupport>(valueNew, buffer);
				}
			}

			buffer.template writeCharacter<json_structural_type, json_structural_type::Object_End>();
		}
	};

	template<jsonifier::concepts::jsonifier_array_t value_type_new> struct serialize_impl<value_type_new> {
		template<bool shortStringsSupport, jsonifier::concepts::jsonifier_array_t value_type, jsonifier::concepts::is_fwd_iterator iterator_type>
		JSONIFIER_INLINE static void op(value_type&& value, iterator_type&& buffer) {
			static constexpr auto size{ std::tuple_size_v<jsonifier::concepts::core_t<value_type_new>> };

			buffer.template writeCharacter<json_structural_type, json_structural_type::Array_Start>();
			if constexpr (size > 0) {
				auto& newMember = getMember(value, get<0>(jsonifier::concepts::coreV<value_type_new>));
				serialize::template op<shortStringsSupport>(newMember, buffer);
				forEach<size - 1>([&](auto x) {
					buffer.template writeCharacter<json_structural_type, json_structural_type::Comma>();
					auto& newMember = getMember(value, get<x + 1>(jsonifier::concepts::coreV<value_type_new>));
					serialize::template op<shortStringsSupport>(newMember, buffer);
				});
			}

			buffer.template writeCharacter<json_structural_type, json_structural_type::Array_End>();
		}
	};

	template<jsonifier::concepts::jsonifier_object_t value_type_new> struct serialize_impl<value_type_new> {
		template<bool shortStringsSupport, jsonifier::concepts::jsonifier_object_t value_type, jsonifier::concepts::is_fwd_iterator iterator_type,
			jsonifier::concepts::has_find... key_type>
		JSONIFIER_INLINE static void op(value_type&& value, iterator_type&& buffer, key_type&&... excludedKeys) {
			buffer.template writeCharacter<json_structural_type, json_structural_type::Object_Start>();
			static constexpr auto n = std::tuple_size_v<jsonifier::concepts::core_t<value_type_new>>;
			bool first{ true };
			auto setToFalseLambda = [&]() {
				first = false;
			};
			forEach<n>([&](auto x) {
				static constexpr auto& item = get<x>(jsonifier::concepts::coreV<value_type_new>);
				using item_type				= jsonifier::concepts::unwrap<decltype(item)>;
				using member_ptr_t			= std::tuple_element_t<1, item_type>;
				using value_type_newer		= member_t<value_type, member_ptr_t>;

				static constexpr jsonifier::string_view key = get<0>(item);
				if constexpr ((( !std::is_void_v<key_type> ) || ...)) {
					constexpr jsonifier::string_view keyNew{ key };
					if (((excludedKeys.find(static_cast<jsonifier::concepts::unwrap<key_type...>::key_type>(keyNew)) != excludedKeys.end()) && ...)) {
						return;
					}
				}
				first ? setToFalseLambda() : buffer.template writeCharacter<json_structural_type, json_structural_type::Comma>();
				buffer.writeCharacters("\"");
				buffer.template writeCharacters<key>();
				buffer.writeCharacters("\":");

				auto& newMember = getMember(value, get<1>(item));
				serialize::template op<shortStringsSupport>(newMember, buffer);
			});
			buffer.template writeCharacter<json_structural_type, json_structural_type::Object_End>();
		}
	};
}