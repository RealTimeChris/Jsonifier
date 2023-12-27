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
#include <jsonifier/ISADetection.hpp>
#include <algorithm>

namespace jsonifier_internal {

	template<typename derived_type, jsonifier::concepts::jsonifier_value_t value_type_new> struct serialize_impl<derived_type, value_type_new> {
		template<jsonifier::concepts::jsonifier_value_t value_type, jsonifier::concepts::buffer_like buffer_type, jsonifier::concepts::uint64_type index_type,
			jsonifier::concepts::has_find... key_type>
		JSONIFIER_INLINE static void impl(value_type&& value, buffer_type&& buffer, index_type&& index, key_type&&... excludedKeys) {
			static constexpr auto n = std::tuple_size_v<jsonifier::concepts::core_t<value_type_new>>;
			writeCharacter<Object_Start>(buffer, index);
			if constexpr (n > 0) {
				serializeObjects<n, 0, true>(value, buffer, index, excludedKeys...);
			}
			writeCharacter<Object_End>(buffer, index);
		}

		template<uint64_t n, uint64_t indexNew = 0, bool areWeFirst = true, jsonifier::concepts::jsonifier_value_t value_type, jsonifier::concepts::buffer_like buffer_type,
			jsonifier::concepts::uint64_type index_type, jsonifier::concepts::has_find... key_type>
		static void serializeObjects(value_type&& value, buffer_type&& buffer, index_type&& index, key_type&&... excludedKeys) {
			static constexpr auto& item = get<indexNew>(jsonifier::concepts::coreV<jsonifier::concepts::unwrap_t<value_type>>);

			static constexpr jsonifier::string_view key = get<0>(item);
			if constexpr ((( !std::is_void_v<key_type> ) || ...)) {
				if (((excludedKeys.find(static_cast<jsonifier::concepts::unwrap_t<key_type...>::key_type>(key)) != excludedKeys.end()) && ...)) [[unlikely]] {
					if constexpr (indexNew < n - 1 && areWeFirst) {
						serializeObjects<n, indexNew + 1, true>(value, buffer, index, excludedKeys...);
					} else if constexpr (indexNew < n - 1) {
						serializeObjects<n, indexNew + 1, false>(value, buffer, index, excludedKeys...);
					}
					return;
				}
			}
			if constexpr (indexNew > 0 && !areWeFirst) {
				writeCharacter<Comma>(buffer, index);
			}
			writeCharacters(buffer, index, "\"");
			writeCharacters<key>(buffer, index);
			writeCharacters(buffer, index, "\":");

			serializer<derived_type>::impl(getMember(value, get<1>(item)), buffer, index);
			if constexpr (indexNew < n - 1) {
				serializeObjects<n, indexNew + 1, false>(value, buffer, index, excludedKeys...);
			}
		}
	};

	template<typename derived_type, jsonifier::concepts::jsonifier_scalar_value_t value_type_new> struct serialize_impl<derived_type, value_type_new> {
		template<jsonifier::concepts::jsonifier_scalar_value_t value_type, jsonifier::concepts::buffer_like buffer_type, jsonifier::concepts::uint64_type index_type>
		JSONIFIER_INLINE static void impl(value_type&& value, buffer_type&& buffer, index_type&& index) {
			static constexpr auto size{ std::tuple_size_v<jsonifier::concepts::core_t<value_type_new>> };
			if constexpr (size > 0) {
				serializer<derived_type>::impl(getMember(value, get<0>(jsonifier::concepts::coreV<value_type_new>)), buffer, index);
			}
		}
	};

	template<typename derived_type, jsonifier::concepts::map_t value_type_new> struct serialize_impl<derived_type, value_type_new> {
		template<jsonifier::concepts::map_t value_type, jsonifier::concepts::buffer_like buffer_type, jsonifier::concepts::uint64_type index_type>
		JSONIFIER_INLINE static void impl(value_type&& value, buffer_type&& buffer, index_type&& index) {
			writeCharacter<Object_Start>(buffer, index);

			if (value.size() > 0) [[likely]] {
				auto iter = value.begin();
				serializer<derived_type>::impl(iter->first, buffer, index);
				writeCharacter<Colon>(buffer, index);
				serializer<derived_type>::impl(iter->second, buffer, index);
				++iter;
				auto endIter = value.end();
				for (; iter != endIter; ++iter) {
					writeCharacter<Comma>(buffer, index);
					serializer<derived_type>::impl(iter->first, buffer, index);
					writeCharacter<Colon>(buffer, index);
					serializer<derived_type>::impl(iter->second, buffer, index);
				}
			}

			writeCharacter<Object_End>(buffer, index);
		}
	};

	template<typename derived_type, jsonifier::concepts::variant_t value_type_new> struct serialize_impl<derived_type, value_type_new> {
		template<jsonifier::concepts::variant_t value_type, jsonifier::concepts::buffer_like buffer_type, jsonifier::concepts::uint64_type index_type>
		JSONIFIER_INLINE static void impl(value_type&& value, buffer_type&& buffer, index_type&& index) {
			std::visit(
				[&](auto&& valueNew) {
					serializer<derived_type>::impl(valueNew, buffer, index);
				},
				value);
		}
	};

	template<typename derived_type, jsonifier::concepts::optional_t value_type_new> struct serialize_impl<derived_type, value_type_new> {
		template<jsonifier::concepts::optional_t value_type, jsonifier::concepts::buffer_like buffer_type, jsonifier::concepts::uint64_type index_type>
		JSONIFIER_INLINE static void impl(value_type&& value, buffer_type&& buffer, index_type&& index) {
			if (value.has_value()) {
				serializer<derived_type>::impl(value.value(), buffer, index);
			} else {
				serializer<derived_type>::impl(nullptr, buffer, index);
			}
		}
	};

	template<typename derived_type, jsonifier::concepts::array_tuple_t value_type_new> struct serialize_impl<derived_type, value_type_new> {
		template<jsonifier::concepts::array_tuple_t value_type, jsonifier::concepts::buffer_like buffer_type, jsonifier::concepts::uint64_type index_type>
		JSONIFIER_INLINE static void impl(value_type&& value, buffer_type&& buffer, index_type&& index) {
			static constexpr auto size = std::tuple_size_v<jsonifier::concepts::unwrap_t<value_type>>;
			writeCharacter<Array_Start>(buffer, index);
			serializeObjects<size, 0>(value, buffer, index);
			writeCharacter<Array_End>(buffer, index);
		}

		template<uint64_t n, uint64_t indexNew = 0, bool areWeFirst = true, jsonifier::concepts::array_tuple_t value_type, jsonifier::concepts::buffer_like buffer_type,
			jsonifier::concepts::uint64_type index_type>
		static void serializeObjects(value_type&& value, buffer_type&& buffer, index_type&& index) {
			auto& item = std::get<indexNew>(value);

			if constexpr (indexNew > 0 && !areWeFirst) {
				writeCharacter<Comma>(buffer, index);
			}

			serializer<derived_type>::impl(item, buffer, index);
			if constexpr (indexNew < n - 1) {
				serializeObjects<n, indexNew + 1, false>(value, buffer, index);
			}
		}
	};

	template<typename derived_type, jsonifier::concepts::vector_t value_type_new> struct serialize_impl<derived_type, value_type_new> {
		template<jsonifier::concepts::vector_t value_type, jsonifier::concepts::buffer_like buffer_type, jsonifier::concepts::uint64_type index_type>
		JSONIFIER_INLINE static void impl(value_type&& value, buffer_type&& buffer, index_type&& index) {
			auto n = value.size();
			writeCharacter<Array_Start>(buffer, index);
			if (n > 0) {
				auto newPtr = value.data();
				serializer<derived_type>::impl(*newPtr, buffer, index);
				++newPtr;
				auto endPtr = value.data() + value.size();
				for (; newPtr < endPtr; ++newPtr) {
					writeCharacter<Comma>(buffer, index);
					serializer<derived_type>::impl(*newPtr, buffer, index);
				}
			}
			writeCharacter<Array_End>(buffer, index);
		}
	};

	template<typename derived_type, jsonifier::concepts::pointer_t value_type_new> struct serialize_impl<derived_type, value_type_new> {
		template<jsonifier::concepts::pointer_t value_type, jsonifier::concepts::buffer_like buffer_type, jsonifier::concepts::uint64_type index_type>
		JSONIFIER_INLINE static void impl(value_type& value, buffer_type&& buffer, index_type&& index) {
			serializer<derived_type>::impl(*value, buffer, index);
		}
	};

	template<typename derived_type, jsonifier::concepts::raw_array_t value_type_new> struct serialize_impl<derived_type, value_type_new> {
		template<jsonifier::concepts::raw_array_t value_type, jsonifier::concepts::buffer_like buffer_type, jsonifier::concepts::uint64_type index_type>
		JSONIFIER_INLINE static void impl(value_type& value, buffer_type&& buffer, index_type&& index) {
			static constexpr auto n = std::size(value);
			writeCharacter<Array_Start>(buffer, index);
			if constexpr (n > 0) {
				auto newPtr = value.data();
				serializer<derived_type>::impl(*newPtr, buffer, index);
				++newPtr;
				for (uint64_t x = 1; x < n; ++x) {
					writeCharacter<Comma>(buffer, index);
					serializer<derived_type>::impl(*newPtr, buffer, index);
					++newPtr;
				}
			}
			writeCharacter<Array_End>(buffer, index);
		}
	};

	template<typename derived_type, jsonifier::concepts::raw_json_t value_type_new> struct serialize_impl<derived_type, value_type_new> {
		template<jsonifier::concepts::raw_json_t value_type, jsonifier::concepts::buffer_like buffer_type, jsonifier::concepts::uint64_type index_type>
		JSONIFIER_INLINE static void impl(value_type&& value, buffer_type&& buffer, index_type&& index) {
			serializer<derived_type>::impl(static_cast<const jsonifier::string>(value), buffer, index);
		}
	};

	template<typename derived_type, jsonifier::concepts::string_t value_type_new> struct serialize_impl<derived_type, value_type_new> {
		template<jsonifier::concepts::string_t value_type, jsonifier::concepts::buffer_like buffer_type, jsonifier::concepts::uint64_type index_type>
		JSONIFIER_INLINE static void impl(value_type&& value, buffer_type&& buffer, index_type&& index) {
			const auto valueSize = value.size();
			const auto k		 = index + 10 + (valueSize * 2);
			if (k >= buffer.size()) [[unlikely]] {
				buffer.resize(max(buffer.size() * 2, k));
			}
			writeCharacter<String>(buffer, index);
			auto newPtr = buffer.data() + index;
			serializeStringImpl(value.data(), newPtr, valueSize);
			index += newPtr - (buffer.data() + index);
			writeCharacter<String>(buffer, index);
		}
	};

	template<typename derived_type, jsonifier::concepts::char_t value_type_new> struct serialize_impl<derived_type, value_type_new> {
		template<jsonifier::concepts::char_t value_type, jsonifier::concepts::buffer_like buffer_type, jsonifier::concepts::uint64_type index_type>
		JSONIFIER_INLINE static void impl(value_type&& value, buffer_type&& buffer, index_type&& index) {
			writeCharacter<String>(buffer, index);
			switch (value) {
				[[unlikely]] case 0x08u: {
					writeCharacters(buffer, index, "\\b");
					break;
				}
				[[unlikely]] case 0x09u: {
					writeCharacters(buffer, index, "\\t");
					break;
				}
				[[unlikely]] case 0x0Au: {
					writeCharacters(buffer, index, "\\n");
					break;
				}
				[[unlikely]] case 0x0Cu: {
					writeCharacters(buffer, index, "\\f");
					break;
				}
				[[unlikely]] case 0x0Du: {
					writeCharacters(buffer, index, "\\r");
					break;
				}
				[[unlikely]] case 0x22u: {
					writeCharacters(buffer, index, "\\\"");
					break;
				}
				[[unlikely]] case 0x5CU: {
					writeCharacters(buffer, index, "\\\\");
					break;
				}
					[[likely]] default : {
						writeCharacter(buffer, index, value);
					}
			}
			writeCharacter<String>(buffer, index);
		}
	};

	template<typename derived_type, jsonifier::concepts::unique_ptr_t value_type_new> struct serialize_impl<derived_type, value_type_new> {
		template<jsonifier::concepts::unique_ptr_t value_type, jsonifier::concepts::buffer_like buffer_type, jsonifier::concepts::uint64_type index_type>
		JSONIFIER_INLINE static void impl(value_type&& value, buffer_type&& buffer, index_type&& index) {
			serializer<derived_type>::impl(*value, buffer, index);
		}
	};

	template<typename derived_type, jsonifier::concepts::enum_t value_type_new> struct serialize_impl<derived_type, value_type_new> {
		template<jsonifier::concepts::enum_t value_type, jsonifier::concepts::buffer_like buffer_type, jsonifier::concepts::uint64_type index_type>
		JSONIFIER_INLINE static void impl(value_type&& value, buffer_type&& buffer, index_type&& index) {
			const auto k = index + 32;
			if (k >= buffer.size()) [[unlikely]] {
				buffer.resize(max(buffer.size() * 2, k));
			}
			int64_t valueNew{ static_cast<int64_t>(value) };
			auto end = toChars(buffer.data() + index, valueNew);
			index	 = end - buffer.data();
		}
	};

	template<typename derived_type, jsonifier::concepts::num_t value_type_new> struct serialize_impl<derived_type, value_type_new> {
		template<jsonifier::concepts::num_t value_type, jsonifier::concepts::buffer_like buffer_type, jsonifier::concepts::uint64_type index_type>
		JSONIFIER_INLINE static void impl(value_type&& value, buffer_type&& buffer, index_type&& index) {
			const auto k = index + 32;
			if (k >= buffer.size()) [[unlikely]] {
				buffer.resize(max(buffer.size() * 2, k));
			}
			if constexpr (jsonifier::concepts::unsigned_type<value_type_new>) {
				index = static_cast<jsonifier::concepts::unwrap_t<index_type>>(toChars(buffer.data() + index, static_cast<uint64_t>(value)) - buffer.data());
			} else if constexpr (jsonifier::concepts::signed_type<value_type_new>) {
				index = static_cast<jsonifier::concepts::unwrap_t<index_type>>(toChars(buffer.data() + index, static_cast<int64_t>(value)) - buffer.data());
			} else if constexpr (jsonifier::concepts::float_t<value_type>) {
				index = static_cast<jsonifier::concepts::unwrap_t<index_type>>(toChars(buffer.data() + index, value) - buffer.data());
			}
		}
	};

	template<typename derived_type, jsonifier::concepts::always_null_t value_type_new> struct serialize_impl<derived_type, value_type_new> {
		template<jsonifier::concepts::always_null_t value_type, jsonifier::concepts::buffer_like buffer_type, jsonifier::concepts::uint64_type index_type>
		JSONIFIER_INLINE static void impl(value_type&&, buffer_type&& buffer, index_type&& index) {
			writeCharacters(buffer, index, "null");
		}
	};

	template<typename derived_type, jsonifier::concepts::bool_t value_type_new> struct serialize_impl<derived_type, value_type_new> {
		template<jsonifier::concepts::bool_t value_type, jsonifier::concepts::buffer_like buffer_type, jsonifier::concepts::uint64_type index_type>
		JSONIFIER_INLINE static void impl(value_type&& value, buffer_type&& buffer, index_type&& index) {
			value ? writeCharacters(buffer, index, "true") : writeCharacters(buffer, index, "false");
		}
	};

}