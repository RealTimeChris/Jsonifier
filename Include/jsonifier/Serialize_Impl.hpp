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

	template<jsonifier::concepts::buffer_like buffer_type, jsonifier::concepts::uint64_type index_type, typename char_type>
	JSONIFIER_INLINE void writeCharacter(buffer_type&& buffer, index_type&& index, const char_type c) noexcept {
		const auto k = index + 1;
		if (k >= buffer.size()) [[unlikely]] {
			buffer.resize(max(buffer.size() * 2, k));
		}

		buffer[index] = static_cast<typename jsonifier::concepts::unwrap<buffer_type>::value_type>(c);
		++index;
	}

	template<jsonifier::concepts::buffer_like buffer_type, jsonifier::concepts::uint64_type index_type, typename char_type>
	JSONIFIER_INLINE void writeCharacterUnchecked(buffer_type&& buffer, index_type&& index, const char_type c) noexcept {
		buffer[index] = static_cast<typename jsonifier::concepts::unwrap<buffer_type>::value_type>(c);
		++index;
	}

	template<json_structural_type c, jsonifier::concepts::buffer_like buffer_type, jsonifier::concepts::uint64_type index_type>
	JSONIFIER_INLINE void writeCharacter(buffer_type&& buffer, index_type&& index) noexcept {
		const auto k = index + 1;
		if (k >= buffer.size()) [[unlikely]] {
			buffer.resize(max(buffer.size() * 2, k));
		}

		buffer[index] = static_cast<typename jsonifier::concepts::unwrap<buffer_type>::value_type>(c);
		++index;
	}

	template<jsonifier::concepts::buffer_like buffer_type, jsonifier::concepts::uint64_type index_type, uint64_t size, typename char_type>
	JSONIFIER_INLINE void writeCharacters(buffer_type&& buffer, index_type&& index, const char_type (&str)[size]) noexcept {
		static constexpr auto sizeNew = size - 1;
		auto k						  = index + sizeNew;
		if (k >= buffer.size()) [[unlikely]] {
			buffer.resize(max(buffer.size() * 2, k));
		}
		std::memcpy(buffer.data() + std::forward<index_type>(index), str, sizeNew);
		index += sizeNew;
	}

	template<const jsonifier::string_view& str, jsonifier::concepts::buffer_like buffer_type, jsonifier::concepts::uint64_type index_type>
	JSONIFIER_INLINE void writeCharacters(buffer_type&& buffer, index_type&& index) noexcept {
		const auto sizeNew = str.size();
		const auto k	   = index + sizeNew;
		if (k >= buffer.size()) [[unlikely]] {
			buffer.resize(max(buffer.size() * 2, k));
		}
		std::memcpy(buffer.data() + std::forward<index_type>(index), str.data(), sizeNew);
		index += sizeNew;
	}

	template<jsonifier::concepts::always_null_t value_type_new> struct serialize_impl<value_type_new> {
		template<jsonifier::concepts::always_null_t value_type, jsonifier::concepts::buffer_like buffer_type, jsonifier::concepts::uint64_type index_type>
		JSONIFIER_INLINE static void op(value_type&&, buffer_type&& buffer, index_type&& index) {
			writeCharacters(buffer, std::forward<index_type>(index), "null");
		}
	};

	template<jsonifier::concepts::unique_ptr_t value_type_new> struct serialize_impl<value_type_new> {
		template<jsonifier::concepts::unique_ptr_t value_type, jsonifier::concepts::buffer_like buffer_type, jsonifier::concepts::uint64_type index_type>
		JSONIFIER_INLINE static void op(value_type&& value, buffer_type&& buffer, index_type&& index) {
			serialize::op(*value, buffer, index);
		}
	};

	template<jsonifier::concepts::bool_t value_type_new> struct serialize_impl<value_type_new> {
		template<jsonifier::concepts::bool_t value_type, jsonifier::concepts::buffer_like buffer_type, jsonifier::concepts::uint64_type index_type>
		JSONIFIER_INLINE static void op(value_type&& value, buffer_type&& buffer, index_type&& index) {
			value ? writeCharacters(buffer, std::forward<index_type>(index), "true") : writeCharacters(buffer, std::forward<index_type>(index), "false");
		}
	};

	template<jsonifier::concepts::num_t value_type_new> struct serialize_impl<value_type_new> {
		template<jsonifier::concepts::num_t value_type, jsonifier::concepts::buffer_like buffer_type, jsonifier::concepts::uint64_type index_type>
		JSONIFIER_INLINE static void op(value_type&& value, buffer_type&& buffer, index_type&& index) {
			const auto k = index + 32;
			if (k >= buffer.size()) [[unlikely]] {
				buffer.resize(max(buffer.size() * 2, k));
			}
			if constexpr (jsonifier::concepts::unsigned_t<value_type> && sizeof(value) < 8) {
				uint64_t newValue{ static_cast<uint64_t>(value) };
				auto end = toChars(buffer.data() + std::forward<index_type>(index), newValue);
				index += end - (buffer.data() + index);
			} else if constexpr (jsonifier::concepts::signed_t<value_type> && sizeof(value) < 8) {
				int64_t newValue{ static_cast<int64_t>(value) };
				auto end = toChars(buffer.data() + std::forward<index_type>(index), newValue);
				index += end - (buffer.data() + index);
			} else {
				auto end = toChars(buffer.data() + std::forward<index_type>(index), value);
				index += end - (buffer.data() + index);
			}
		}
	};

	template<jsonifier::concepts::enum_t value_type_new> struct serialize_impl<value_type_new> {
		template<jsonifier::concepts::enum_t value_type, jsonifier::concepts::buffer_like buffer_type, jsonifier::concepts::uint64_type index_type>
		JSONIFIER_INLINE static void op(value_type&& value, buffer_type&& buffer, index_type&& index) {
			const auto k = index + 32;
			if (k >= buffer.size()) [[unlikely]] {
				buffer.resize(max(buffer.size() * 2, k));
			}
			auto end = toChars(buffer.data() + std::forward<index_type>(index), static_cast<int64_t>(value));
			index += end - (buffer.data() + index);
		}
	};

	template<jsonifier::concepts::char_t value_type_new> struct serialize_impl<value_type_new> {
		template<jsonifier::concepts::char_t value_type, jsonifier::concepts::buffer_like buffer_type, jsonifier::concepts::uint64_type index_type>
		JSONIFIER_INLINE static void op(value_type&& value, buffer_type&& buffer, index_type&& index) {
			writeCharacter<Jsonifier_String>(buffer, index);
			switch (value) {
				[[unlikely]] case 0x08u : {
					writeCharacters(buffer, std::forward<index_type>(index), "\\b");
					break;
				}
				[[unlikely]] case 0x09u : {
					writeCharacters(buffer, std::forward<index_type>(index), "\\t");
					break;
				}
				[[unlikely]] case 0x0Au : {
					writeCharacters(buffer, std::forward<index_type>(index), "\\n");
					break;
				}
				[[unlikely]] case 0x0Cu : {
					writeCharacters(buffer, std::forward<index_type>(index), "\\f");
					break;
				}
				[[unlikely]] case 0x0Du : {
					writeCharacters(buffer, std::forward<index_type>(index), "\\r");
					break;
				}
				[[unlikely]] case 0x22u : {
					writeCharacters(buffer, std::forward<index_type>(index), "\\\"");
					break;
				}
				[[unlikely]] case 0x5CU : {
					writeCharacters(buffer, std::forward<index_type>(index), "\\\\");
					break;
				}
					[[likely]] default : {
						writeCharacter(buffer, std::forward<index_type>(index), value);
					}
			}
			writeCharacter<Jsonifier_String>(buffer, index);
		}
	};

	template<jsonifier::concepts::string_t value_type_new> struct serialize_impl<value_type_new> {
		template<jsonifier::concepts::string_t value_type, jsonifier::concepts::buffer_like buffer_type, jsonifier::concepts::uint64_type index_type>
		JSONIFIER_INLINE static void op(value_type&& value, buffer_type&& buffer, index_type&& index) {
			const auto k = index + 10 + value.size() * 2;
			if (k >= buffer.size()) [[unlikely]] {
				buffer.resize(max(buffer.size() * 2, k));
			}
			writeCharacter<Jsonifier_String>(buffer, index);

			serializeString(value.data(), buffer.data() + std::forward<index_type>(index), value.size(), index);

			writeCharacter<Jsonifier_String>(buffer, index);
		}
	};

	template<jsonifier::concepts::raw_json_t value_type_new> struct serialize_impl<value_type_new> {
		template<jsonifier::concepts::raw_json_t value_type, jsonifier::concepts::buffer_like buffer_type, jsonifier::concepts::uint64_type index_type>
		JSONIFIER_INLINE static void op(value_type&& value, buffer_type&& buffer, index_type&& index) {
			serialize_impl<jsonifier::string>::op(static_cast<const jsonifier::string>(value), buffer, index);
		}
	};

	template<jsonifier::concepts::raw_array_t value_type_new> struct serialize_impl<value_type_new> {
		template<jsonifier::concepts::raw_array_t value_type, jsonifier::concepts::buffer_like buffer_type, jsonifier::concepts::uint64_type index_type>
		JSONIFIER_INLINE static void op(value_type& value, buffer_type&& buffer, index_type&& index) {
			static constexpr auto n = std::size(value);
			writeCharacter<Jsonifier_Array_Start>(buffer, index);
			if constexpr (n > 0) {
				serialize::op(value[0], buffer, index);
				for (uint64_t x = 1; x < n; ++x) {
					writeCharacter<Jsonifier_Comma>(buffer, index);
					serialize::op(value[x], buffer, index);
				}
			}

			writeCharacter<Jsonifier_Array_End>(buffer, index);
		}
	};

	template<jsonifier::concepts::vector_t value_type_new> struct serialize_impl<value_type_new> {
		template<jsonifier::concepts::vector_t value_type, jsonifier::concepts::buffer_like buffer_type, jsonifier::concepts::uint64_type index_type>
		JSONIFIER_INLINE static void op(value_type&& value, buffer_type&& buffer, index_type&& index) {
			auto n = value.size();
			writeCharacter<Jsonifier_Array_Start>(buffer, index);
			if (n > 0) [[likely]] {
				serialize::op(value[0], buffer, index);
				for (uint64_t x = 1; x < n; ++x) {
					writeCharacter<Jsonifier_Comma>(buffer, index);
					serialize::op(value[x], buffer, index);
				}
			}

			writeCharacter<Jsonifier_Array_End>(buffer, index);
		}
	};

	template<jsonifier::concepts::map_t value_type_new> struct serialize_impl<value_type_new> {
		template<jsonifier::concepts::map_t value_type, jsonifier::concepts::buffer_like buffer_type, jsonifier::concepts::uint64_type index_type>
		JSONIFIER_INLINE static void op(value_type&& value, buffer_type&& buffer, index_type&& index) {
			writeCharacter<Jsonifier_Object_Start>(buffer, index);

			if (value.size() > 0) [[likely]] {
				auto iter = value.begin();
				serialize::op(iter->first, buffer, index);
				writeCharacter<Jsonifier_Colon>(buffer, index);
				serialize::op(iter->second, buffer, index);
				++iter;
				for (; iter != value.end(); ++iter) {
					writeCharacter<Jsonifier_Comma>(buffer, index);
					serialize::op(iter->first, buffer, index);
					writeCharacter<Jsonifier_Colon>(buffer, index);
					serialize::op(iter->second, buffer, index);
				}
			}

			writeCharacter<Jsonifier_Object_End>(buffer, index);
		}
	};

	template<jsonifier::concepts::jsonifier_scalar_value_t value_type_new> struct serialize_impl<value_type_new> {
		template<jsonifier::concepts::jsonifier_scalar_value_t value_type, jsonifier::concepts::buffer_like buffer_type, jsonifier::concepts::uint64_type index_type>
		JSONIFIER_INLINE static void op(value_type&& value, buffer_type&& buffer, index_type&& index) {
			static constexpr auto size{ std::tuple_size_v<jsonifier::concepts::core_t<value_type_new>> };
			if constexpr (size > 0) {
				serialize::op(getMember(value, get<0>(jsonifier::concepts::coreV<value_type_new>)), buffer, index);
			}
		}
	};

	template<uint64_t n, uint64_t indexNew = 0, bool areWeFirst = true, jsonifier::concepts::jsonifier_value_t value_type, jsonifier::concepts::buffer_like buffer_type,
		jsonifier::concepts::uint64_type index_type, jsonifier::concepts::has_find... key_type>
	void serializeObjects(value_type&& value, buffer_type&& buffer, index_type&& index, key_type&&... excludedKeys) {
		static constexpr auto& item = get<indexNew>(jsonifier::concepts::coreV<jsonifier::concepts::unwrap<value_type>>);

		static constexpr jsonifier::string_view key = get<0>(item);
		if constexpr ((( !std::is_void_v<key_type> ) || ...)) {
			constexpr jsonifier::string_view keyNew = key;
			if (((excludedKeys.find(static_cast<jsonifier::concepts::unwrap<key_type...>::key_type>(keyNew)) != excludedKeys.end()) && ...)) [[unlikely]] {
				if constexpr (indexNew < n - 1 && areWeFirst) {
					serializeObjects<n, indexNew + 1, true>(value, buffer, index, excludedKeys...);
				} else if constexpr (indexNew < n - 1) {
					serializeObjects<n, indexNew + 1, false>(value, buffer, index, excludedKeys...);
				}
				return;
			}
		}
		if constexpr (indexNew > 0 && !areWeFirst) {
			writeCharacter<Jsonifier_Comma>(buffer, index);
		}
		writeCharacters(buffer, std::forward<index_type>(index), "\"");
		writeCharacters<key>(buffer, index);
		writeCharacters(buffer, std::forward<index_type>(index), "\":");

		serialize::op(getMember(value, get<1>(item)), buffer, index);
		if constexpr (indexNew < n - 1) {
			serializeObjects<n, indexNew + 1, false>(value, buffer, index, excludedKeys...);
		}
	}

	template<jsonifier::concepts::jsonifier_value_t value_type_new> struct serialize_impl<value_type_new> {
		template<jsonifier::concepts::jsonifier_value_t value_type, jsonifier::concepts::buffer_like buffer_type, jsonifier::concepts::uint64_type index_type,
			jsonifier::concepts::has_find... key_type>
		JSONIFIER_INLINE static void op(value_type&& value, buffer_type&& buffer, index_type&& index, key_type&&... excludedKeys) {
			static constexpr auto n = std::tuple_size_v<jsonifier::concepts::core_t<value_type_new>>;
			writeCharacter<Jsonifier_Object_Start>(buffer, index);
			serializeObjects<n, 0, true>(value, buffer, index, excludedKeys...);
			writeCharacter<Jsonifier_Object_End>(buffer, index);
		}
	};
}