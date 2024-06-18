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
#include <jsonifier/Config.hpp>
#include <algorithm>

namespace jsonifier_internal {

	template<const serialize_options_internal& options, typename derived_type, jsonifier::concepts::jsonifier_value_t value_type_new>
	struct serialize_impl<options, derived_type, value_type_new> {
		template<jsonifier::concepts::jsonifier_value_t value_type, jsonifier::concepts::buffer_like buffer_type, jsonifier::concepts::uint64_type index_type>
		JSONIFIER_INLINE static void impl(value_type&& value, buffer_type&& buffer, index_type&& index) {
			static constexpr auto numMembers = std::tuple_size_v<jsonifier::concepts::core_t<value_type_new>>;
			writeObjectEntry<numMembers, options>(buffer, index);

			if constexpr (numMembers > 0) {
				serializeObjects<numMembers, 0>(value, buffer, index);
			}

			writeObjectExit<numMembers, options>(buffer, index);
		}

		template<uint64_t n, uint64_t indexNew = 0, jsonifier::concepts::jsonifier_value_t value_type, jsonifier::concepts::buffer_like buffer_type,
			jsonifier::concepts::uint64_type index_type>
		static void serializeObjects(value_type&& value, buffer_type&& buffer, index_type&& index) {
			static constexpr auto& group = std::get<indexNew>(jsonifier::concepts::core_v<jsonifier::concepts::unwrap_t<value_type>>);

			static constexpr jsonifier::string_view key = std::get<0>(group);
			if constexpr (jsonifier::concepts::has_excluded_keys<value_type>) {
				auto& keys = value.jsonifierExcludedKeys;
				if (keys.find(static_cast<typename jsonifier::concepts::unwrap_t<decltype(keys)>::key_type>(key)) != keys.end()) {
					if constexpr (indexNew < n - 1) {
						serializeObjects<n, indexNew + 1>(value, buffer, index);
					} else {
						return;
					}
				}
			}

			static constexpr auto quotedKey = joinV < chars<"\"">, key, options.optionsReal.prettify ? chars<"\": "> : chars < "\":" >> ;
			writeCharacters<quotedKey>(buffer, index);

			static constexpr auto frozenSet = makeSet<value_type>();
			static constexpr auto memberIt	= frozenSet.find(key);
			static_assert(memberIt != frozenSet.end(), "Invalid key passed to partial write");
			std::visit(
				[&](const auto& memberPtr) -> void {
					auto& newMember	  = getMember(value, memberPtr);
					using member_type = jsonifier::concepts::unwrap_t<decltype(newMember)>;
					serialize_impl<options, derived_type, member_type>::impl(newMember, buffer, index);
					return;
				},
				*memberIt);


			if constexpr (indexNew != n - 1) {
				if constexpr (options.optionsReal.prettify) {
					if constexpr (jsonifier::concepts::buffer_like<buffer_type>) {
						if (auto k = index + options.indent + 256; k > buffer.size()) [[unlikely]] {
							buffer.resize(max(buffer.size() * 2, k));
						}
					}
					writeCharactersUnchecked<",\n">(buffer, index);
					writeCharactersUnchecked<' '>(options.indent * options.optionsReal.indentSize, buffer, index);
				} else {
					writeCharacter<','>(buffer, index);
				}
			}


			if constexpr (indexNew < n - 1) {
				serializeObjects<n, indexNew + 1>(value, buffer, index);
			}
		}
	};

	template<const serialize_options_internal& options, typename derived_type, jsonifier::concepts::jsonifier_scalar_value_t value_type_new>
	struct serialize_impl<options, derived_type, value_type_new> {
		template<jsonifier::concepts::jsonifier_scalar_value_t value_type, jsonifier::concepts::buffer_like buffer_type, jsonifier::concepts::uint64_type index_type>
		JSONIFIER_INLINE static void impl(value_type&& value, buffer_type&& buffer, index_type&& index) {
			static constexpr auto size{ std::tuple_size_v<jsonifier::concepts::core_t<value_type_new>> };
			if constexpr (size > 0) {
				auto& newMember	  = getMember(value, std::get<0>(jsonifier::concepts::core_v<value_type_new>));
				using member_type = jsonifier::concepts::unwrap_t<decltype(newMember)>;
				serialize_impl<options, derived_type, member_type>::impl(newMember, buffer, index);
			}
		}
	};

	template<const serialize_options_internal& options, typename derived_type, jsonifier::concepts::map_t value_type_new>
	struct serialize_impl<options, derived_type, value_type_new> {
		template<jsonifier::concepts::map_t value_type, jsonifier::concepts::buffer_like buffer_type, jsonifier::concepts::uint64_type index_type>
		JSONIFIER_INLINE static void impl(value_type&& value, buffer_type&& buffer, index_type&& index) {
			using member_type = jsonifier::concepts::unwrap_t<decltype(value[std::declval<typename jsonifier::concepts::unwrap_t<value_type_new>::key_type>()])>;
			writeObjectEntry(buffer, index, value.size());

			if (value.size() > 0) [[likely]] {
				auto iter = value.begin();
				serialize_impl<options, derived_type, member_type>::impl(iter->first, buffer, index);
				writeCharacter<json_structural_type::Colon>(buffer, index);
				if constexpr (options.optionsReal.prettify) {
					writeCharacter<0x20u>(buffer, index);
				}
				serialize_impl<options, derived_type, member_type>::impl(iter->second, buffer, index);
				++iter;
				auto endIter = value.end();
				for (; iter != endIter; ++iter) {
					writeEntrySeparator<options>(buffer, index);
					serialize_impl<options, derived_type, member_type>::impl(iter->first, buffer, index);
					writeCharacter<json_structural_type::Colon>(buffer, index);
					if constexpr (options.optionsReal.prettify) {
						writeCharacter<0x20u>(buffer, index);
					}
					serialize_impl<options, derived_type, member_type>::impl(iter->second, buffer, index);
				}
			}
			writeObjectExit<options>(buffer, index, value.size());
		}
	};

	template<const serialize_options_internal& options, typename derived_type, jsonifier::concepts::variant_t value_type_new>
	struct serialize_impl<options, derived_type, value_type_new> {
		template<jsonifier::concepts::variant_t value_type, jsonifier::concepts::buffer_like buffer_type, jsonifier::concepts::uint64_type index_type>
		JSONIFIER_INLINE static void impl(value_type&& value, buffer_type&& buffer, index_type&& index) {
			std::visit(
				[&](auto&& valueNew) {
					using member_type = decltype(valueNew);
					serialize_impl<options, derived_type, member_type>::impl(valueNew, buffer, index);
				},
				value);
		}
	};

	template<const serialize_options_internal& options, typename derived_type, jsonifier::concepts::optional_t value_type_new>
	struct serialize_impl<options, derived_type, value_type_new> {
		template<jsonifier::concepts::optional_t value_type, jsonifier::concepts::buffer_like buffer_type, jsonifier::concepts::uint64_type index_type>
		JSONIFIER_INLINE static void impl(value_type&& value, buffer_type&& buffer, index_type&& index) {
			if (value) {
				using member_type = typename jsonifier::concepts::unwrap_t<value_type_new>::value_type;
				serialize_impl<options, derived_type, member_type>::impl(*value, buffer, index);
			} else {
				writeCharacters<"null">(buffer, index);
			}
		}
	};

	template<const serialize_options_internal& options, typename derived_type, jsonifier::concepts::array_tuple_t value_type_new>
	struct serialize_impl<options, derived_type, value_type_new> {
		template<jsonifier::concepts::array_tuple_t value_type, jsonifier::concepts::buffer_like buffer_type, jsonifier::concepts::uint64_type index_type>
		JSONIFIER_INLINE static void impl(value_type&& value, buffer_type&& buffer, index_type&& index) {
			static constexpr auto size = std::tuple_size_v<jsonifier::concepts::unwrap_t<value_type>>;
			writeArrayEntry<options>(buffer, index, size);
			serializeObjects<size, 0>(value, buffer, index);
			writeArrayExit<options>(buffer, index, size);
		}

		template<uint64_t n, uint64_t indexNew = 0, bool areWeFirst = true, jsonifier::concepts::array_tuple_t value_type, jsonifier::concepts::buffer_like buffer_type,
			jsonifier::concepts::uint64_type index_type>
		static void serializeObjects(value_type&& value, buffer_type&& buffer, index_type&& index) {
			auto& item = std::get<indexNew>(value);

			if constexpr (indexNew > 0 && !areWeFirst) {
				writeEntrySeparator<options>(buffer, index);
			}
			using member_type = jsonifier::concepts::unwrap_t<decltype(item)>;
			serialize_impl<options, derived_type, member_type>::impl(item, buffer, index);
			if constexpr (indexNew < n - 1) {
				serializeObjects<n, indexNew + 1, false>(value, buffer, index);
			}
		}
	};

	template<const serialize_options_internal& options, typename derived_type, jsonifier::concepts::vector_t value_type_new>
	struct serialize_impl<options, derived_type, value_type_new> {
		template<jsonifier::concepts::vector_t value_type, jsonifier::concepts::buffer_like buffer_type, jsonifier::concepts::uint64_type index_type>
		JSONIFIER_INLINE static void impl(value_type&& value, buffer_type&& buffer, index_type&& index) {
			auto n = value.size();
			writeArrayEntry<options>(buffer, index, n);

			if (n != 0) {
				using member_type = typename jsonifier::concepts::unwrap_t<value_type_new>::value_type;
				auto iter		  = value.begin();
				serialize_impl<options, derived_type, member_type>::impl(*iter, buffer, index);
				++iter;
				for (auto fin = value.end(); iter != fin; ++iter) {
					writeEntrySeparator<options>(buffer, index);
					serialize_impl<options, derived_type, member_type>::impl(*iter, buffer, index);
				}
			}
			writeArrayExit<options>(buffer, index, n);
		}
	};

	template<const serialize_options_internal& options, typename derived_type, jsonifier::concepts::pointer_t value_type_new>
	struct serialize_impl<options, derived_type, value_type_new> {
		template<jsonifier::concepts::pointer_t value_type, jsonifier::concepts::buffer_like buffer_type, jsonifier::concepts::uint64_type index_type>
		JSONIFIER_INLINE static void impl(value_type& value, buffer_type&& buffer, index_type&& index) {
			using member_type = jsonifier::concepts::unwrap_t<decltype(*value)>;
			serialize_impl<options, derived_type, member_type>::impl(*value, buffer, index);
		}
	};

	template<const serialize_options_internal& options, typename derived_type, jsonifier::concepts::raw_array_t value_type_new>
	struct serialize_impl<options, derived_type, value_type_new> {
		template<jsonifier::concepts::raw_array_t value_type, jsonifier::concepts::buffer_like buffer_type, jsonifier::concepts::uint64_type index_type>
		JSONIFIER_INLINE static void impl(value_type& value, buffer_type&& buffer, index_type&& index) {
			using member_type		= jsonifier::concepts::unwrap_t<decltype(value[0])>;
			static constexpr auto n = std::size(value);
			writeArrayEntry<options>(buffer, index, n);
			if constexpr (n > 0) {
				auto newPtr = value.data();
				serialize_impl<options, derived_type, member_type>::impl(*newPtr, buffer, index);
				++newPtr;
				for (uint64_t x = 1; x < n; ++x) {
					writeEntrySeparator<options>(buffer, index);
					serialize_impl<options, derived_type, member_type>::impl(*newPtr, buffer, index);
					++newPtr;
				}
			}
			writeArrayExit<options>(buffer, index, n);
		}
	};

	template<const serialize_options_internal& options, typename derived_type, jsonifier::concepts::raw_json_t value_type_new>
	struct serialize_impl<options, derived_type, value_type_new> {
		template<jsonifier::concepts::raw_json_t value_type, jsonifier::concepts::buffer_like buffer_type, jsonifier::concepts::uint64_type index_type>
		JSONIFIER_INLINE static void impl(value_type&& value, buffer_type&& buffer, index_type&& index) {
			using member_type = jsonifier::string;
			serialize_impl<options, derived_type, member_type>::impl(static_cast<const jsonifier::string>(value), buffer, index);
		}
	};

	template<const serialize_options_internal& options, typename derived_type, jsonifier::concepts::string_t value_type_new>
	struct serialize_impl<options, derived_type, value_type_new> {
		template<jsonifier::concepts::string_t value_type, jsonifier::concepts::buffer_like buffer_type, jsonifier::concepts::uint64_type index_type>
		JSONIFIER_INLINE static void impl(value_type&& value, buffer_type&& buffer, index_type&& index) {
			auto valueSize = value.size();
			auto k		   = index + 10 + (valueSize * 2);
			if (k >= buffer.size()) [[unlikely]] {
				buffer.resize(max(buffer.size() * 2, k));
			}
			writeCharacter<'"'>(buffer, index);
			auto newPtr = buffer.data() + index;
			serializeStringImpl(value.data(), newPtr, valueSize);
			index += newPtr - (buffer.data() + index);
			writeCharacter<'"'>(buffer, index);
		}
	};

	template<const serialize_options_internal& options, typename derived_type, jsonifier::concepts::char_t value_type_new>
	struct serialize_impl<options, derived_type, value_type_new> {
		template<jsonifier::concepts::char_t value_type, jsonifier::concepts::buffer_like buffer_type, jsonifier::concepts::uint64_type index_type>
		JSONIFIER_INLINE static void impl(value_type&& value, buffer_type&& buffer, index_type&& index) {
			writeCharacter<json_structural_type::String>(buffer, index);
			switch (value) {
				[[unlikely]] case '\b': {
					writeCharacters(buffer, index, "\\b");
					break;
				}
				[[unlikely]] case '\t': {
					writeCharacters(buffer, index, "\\t");
					break;
				}
				[[unlikely]] case '\n': {
					writeCharacters(buffer, index, "\\n");
					break;
				}
				[[unlikely]] case 0x0Cu: {
					writeCharacters(buffer, index, "\\f");
					break;
				}
				[[unlikely]] case '\r': {
					writeCharacters(buffer, index, "\\r");
					break;
				}
				[[unlikely]] case '"': {
					writeCharacters(buffer, index, "\\\"");
					break;
				}
				[[unlikely]] case 0x5CU: {
					writeCharacters(buffer, index, "\\\\");
					break;
				}
				[[likely]] default: { writeCharacter(buffer, index, value); }
			}
			writeCharacter<json_structural_type::String>(buffer, index);
		}
	};

	template<const serialize_options_internal& options, typename derived_type, jsonifier::concepts::unique_ptr_t value_type_new>
	struct serialize_impl<options, derived_type, value_type_new> {
		template<jsonifier::concepts::unique_ptr_t value_type, jsonifier::concepts::buffer_like buffer_type, jsonifier::concepts::uint64_type index_type>
		JSONIFIER_INLINE static void impl(value_type&& value, buffer_type&& buffer, index_type&& index) {
			using member_type = jsonifier::concepts::unwrap_t<decltype(*value)>;
			serialize_impl<options, derived_type, member_type>::impl(*value, buffer, index);
		}
	};

	template<const serialize_options_internal& options, typename derived_type, jsonifier::concepts::enum_t value_type_new>
	struct serialize_impl<options, derived_type, value_type_new> {
		template<jsonifier::concepts::enum_t value_type, jsonifier::concepts::buffer_like buffer_type, jsonifier::concepts::uint64_type index_type>
		JSONIFIER_INLINE static void impl(value_type&& value, buffer_type&& buffer, index_type&& index) {
			auto k = index + 32;
			if (k >= buffer.size()) [[unlikely]] {
				buffer.resize(max(buffer.size() * 2, k));
			}
			int64_t valueNew{ static_cast<int64_t>(value) };
			index = toChars(buffer.data() + index, valueNew) - buffer.data();
		}
	};

	template<const serialize_options_internal& options, typename derived_type, jsonifier::concepts::always_null_t value_type_new>
	struct serialize_impl<options, derived_type, value_type_new> {
		template<jsonifier::concepts::always_null_t value_type, jsonifier::concepts::buffer_like buffer_type, jsonifier::concepts::uint64_type index_type>
		JSONIFIER_INLINE static void impl(value_type&&, buffer_type&& buffer, index_type&& index) {
			writeCharacters<"null">(buffer, index);
		}
	};

	template<const serialize_options_internal& options, typename derived_type, jsonifier::concepts::bool_t value_type_new>
	struct serialize_impl<options, derived_type, value_type_new> {
		template<jsonifier::concepts::bool_t value_type, jsonifier::concepts::buffer_like buffer_type, jsonifier::concepts::uint64_type index_type>
		JSONIFIER_INLINE static void impl(value_type&& value, buffer_type&& buffer, index_type&& index) {
			if (value) {
				writeCharacters<"true">(buffer, index);
			} else {
				writeCharacters<"false">(buffer, index);
			}
		}
	};

	template<const serialize_options_internal& options, typename derived_type, jsonifier::concepts::num_t value_type_new>
	struct serialize_impl<options, derived_type, value_type_new> {
		template<jsonifier::concepts::num_t value_type, jsonifier::concepts::buffer_like buffer_type, jsonifier::concepts::uint64_type index_type>
		JSONIFIER_INLINE static void impl(value_type&& value, buffer_type&& buffer, index_type&& index) {
			auto k = index + 32;
			if (k >= buffer.size()) [[unlikely]] {
				buffer.resize(max(buffer.size() * 2, k));
			}
			index = static_cast<uint64_t>(toChars(buffer.data() + index, value) - buffer.data());
		}
	};

}