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
#include <jsonifier/TypeEntities.hpp>
#include <algorithm>
#include <assert.h>

namespace jsonifier_internal {

	template<const auto& options, const auto& tuple, size_t index, typename derived_type, typename value_type, typename buffer_type, typename index_type>
	JSONIFIER_INLINE void invokeSerialize(value_type& value, buffer_type& buffer, index_type& indexVal) {
		static constexpr auto ptr = std::get<index>(tuple).ptr();
		using member_type		  = unwrap_t<decltype(value.*ptr)>;
		serialize_impl<derived_type, member_type>::template impl<options>(value.*ptr, buffer, indexVal);
	}

	template<const auto& options, const auto& tuple, size_t index, typename derived_type, typename value_type, typename buffer_type, typename index_type>
	using invoke_serialize_function_ptr = decltype(&invokeSerialize<options, tuple, index, derived_type, value_type, buffer_type, index_type>);

	template<const auto& options, const auto& tuple, typename derived_type, typename value_type, typename buffer_type, typename index_type, size_t... indices>
	constexpr auto generateArrayOfInvokeSerializePtrsInternal(std::index_sequence<indices...>) {
		return std::array<invoke_serialize_function_ptr<options, tuple, 0, derived_type, value_type, buffer_type, index_type>, sizeof...(indices)>{
			&invokeSerialize<options, tuple, indices, derived_type, value_type, buffer_type, index_type>...
		};
	}

	template<const auto& options, const auto& tuple, typename derived_type, typename value_type, typename buffer_type, typename index_type>
	constexpr auto generateArrayOfInvokeSerializePtrs() {
		constexpr auto tupleSize = std::tuple_size_v<std::decay_t<decltype(tuple)>>;
		return generateArrayOfInvokeSerializePtrsInternal<options, tuple, derived_type, value_type, buffer_type, index_type>(std::make_index_sequence<tupleSize>{});
	}

	template<const auto& options, const auto& tuple, typename derived_type, typename value_type, typename buffer_type, typename index_type, size_t... indices>
	constexpr auto generateTupleOfInvokeSerializePtrArraysInternal(std::index_sequence<indices...>) {
		return std::make_tuple(generateArrayOfInvokeSerializePtrs<options, std::get<indices>(tuple).tuple, derived_type, value_type, buffer_type, index_type>()...);
	}

	template<const auto& options, typename derived_type, typename value_type, typename buffer_type, typename index_type> constexpr auto generateTupleOfInvokeSerializePtrArrays() {
		constexpr auto& tuple	 = hash_tuple<value_type>::tuple;
		constexpr auto tupleSize = std::tuple_size_v<std::decay_t<decltype(tuple)>>;
		return generateTupleOfInvokeSerializePtrArraysInternal<options, tuple, derived_type, value_type, buffer_type, index_type>(std::make_index_sequence<tupleSize>{});
	}

	template<typename derived_type, jsonifier::concepts::jsonifier_value_t value_type_new> struct serialize_impl<derived_type, value_type_new> {
		template<const serialize_options_internal& options, jsonifier::concepts::jsonifier_value_t value_type, jsonifier::concepts::buffer_like buffer_type,
			jsonifier::concepts::uint64_type index_type>
		JSONIFIER_INLINE static void impl(value_type&& value, buffer_type&& buffer, index_type&& index) {
			static constexpr auto numMembers = std::tuple_size_v<final_tuple_t<unwrap_t<value_type_new>>>;
			writeObjectEntry<numMembers, options>(buffer, index);

			static constexpr auto frozenMap = makeHashTuple<value_type>();
			if constexpr (numMembers > 0) {
				serializeObjects<options, frozenMap.tuple, 0, numMembers>(value, buffer, index, std::make_index_sequence<numMembers>{});
			}

			writeObjectExit<numMembers, options>(buffer, index);
		}

		template<const serialize_options_internal& options, const auto& frozenMap, size_t currentIndex, size_t maxIndex, size_t... indices,
			jsonifier::concepts::jsonifier_value_t value_type, jsonifier::concepts::buffer_like buffer_type, jsonifier::concepts::uint64_type index_type>
		static void serializeObjects(value_type&& value, buffer_type&& buffer, index_type&& index, std::index_sequence<indices...>) {
			if constexpr (currentIndex < maxIndex) {
				serializeSubObjects<options, std::get<currentIndex>(frozenMap), currentIndex != maxIndex - 1, 0,
					std::tuple_size_v<unwrap_t<decltype(std::get<currentIndex>(frozenMap).tuple)>>>(value, buffer, index);
				serializeObjects<options, frozenMap, currentIndex + 1, maxIndex>(value, buffer, index, std::make_index_sequence<sizeof...(indices)>{});
			}
		}

		template<const serialize_options_internal& options, const auto& frozenMap, bool addEntrySeparator = false, size_t indexNew = 0, size_t n,
			jsonifier::concepts::jsonifier_value_t value_type, jsonifier::concepts::buffer_like buffer_type, jsonifier::concepts::uint64_type index_type>
		static void serializeSubObjects(value_type&& value, buffer_type&& buffer, index_type&& index) {
			if constexpr (indexNew < n) {
				static constexpr auto& group = std::get<indexNew>(frozenMap.tuple);

				static constexpr jsonifier::string_view key = group.view();
				if constexpr (jsonifier::concepts::has_excluded_keys<value_type>) {
					auto& keys = value.jsonifierExcludedKeys;
					if (keys.find(static_cast<typename unwrap_t<decltype(keys)>::key_type>(key)) != keys.end()) {
						if constexpr (indexNew < n - 1) {
							serializeObjects<options, frozenMap, n, indexNew + 1>(value, buffer, index);
						} else {
							return;
						}
					}
				}

				static constexpr auto quotedKey = joinV < chars<"\"">, key, options.optionsReal.prettify ? chars<"\": "> : chars < "\":" >> ;
				writeCharacters<quotedKey>(buffer, index);
				static constexpr auto arrayOfInvokeSerializePtrs = generateTupleOfInvokeSerializePtrArrays<options, derived_type, value_type, buffer_type, index_type>();

				static constexpr auto subTupleFunctionPtrArray = std::get<frozenMap.subTupleIndex>(arrayOfInvokeSerializePtrs);
				static constexpr auto iterNew				   = frozenMap.template find<subTupleFunctionPtrArray>(key.data(), key.size());
				static_assert(iterNew != subTupleFunctionPtrArray.data() + subTupleFunctionPtrArray.size());
				(*iterNew)(value, buffer, index);

				if constexpr (addEntrySeparator) {
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
					serializeSubObjects<options, frozenMap, addEntrySeparator, indexNew + 1, n>(value, buffer, index);
				}
			}
		}
	};

	template<typename derived_type, jsonifier::concepts::jsonifier_scalar_value_t value_type_new> struct serialize_impl<derived_type, value_type_new> {
		template<const serialize_options_internal& options, jsonifier::concepts::jsonifier_scalar_value_t value_type, jsonifier::concepts::buffer_like buffer_type,
			jsonifier::concepts::uint64_type index_type>
		JSONIFIER_INLINE static void impl(value_type&& value, buffer_type&& buffer, index_type&& index) {
			static constexpr auto size{ std::tuple_size_v<jsonifier::concepts::core_t<value_type_new>> };
			if constexpr (size > 0) {
				static constexpr auto newPtr = std::get<0>(jsonifier::concepts::coreV<value_type_new>);
				auto& newMember				 = getMember<newPtr>(value);
				using member_type			 = unwrap_t<decltype(newMember)>;
				serialize_impl<derived_type, member_type>::template impl<options>(newMember, buffer, index);
			}
		}
	};

	template<typename derived_type, jsonifier::concepts::map_t value_type_new> struct serialize_impl<derived_type, value_type_new> {
		template<const serialize_options_internal& options, jsonifier::concepts::map_t value_type, jsonifier::concepts::buffer_like buffer_type,
			jsonifier::concepts::uint64_type index_type>
		JSONIFIER_INLINE static void impl(value_type&& value, buffer_type&& buffer, index_type&& index) {
			using member_type = unwrap_t<decltype(value[std::declval<typename unwrap_t<value_type_new>::key_type>()])>;
			writeObjectEntry(buffer, index, value.size());

			if (value.size() > 0) [[likely]] {
				auto iter = value.begin();
				serialize_impl<derived_type, member_type>::template impl<options>(iter->first, buffer, index);
				writeCharacter<json_structural_type::Colon>(buffer, index);
				if constexpr (options.optionsReal.prettify) {
					writeCharacter<0x20u>(buffer, index);
				}
				serialize_impl<derived_type, member_type>::template impl<options>(iter->second, buffer, index);
				++iter;
				auto endIter = value.end();
				for (; iter != endIter; ++iter) {
					writeEntrySeparator<options>(buffer, index);
					serialize_impl<derived_type, member_type>::template impl<options>(iter->first, buffer, index);
					writeCharacter<json_structural_type::Colon>(buffer, index);
					if constexpr (options.optionsReal.prettify) {
						writeCharacter<0x20u>(buffer, index);
					}
					serialize_impl<derived_type, member_type>::template impl<options>(iter->second, buffer, index);
				}
			}
			writeObjectExit<options>(buffer, index, value.size());
		}
	};

	template<typename derived_type, jsonifier::concepts::variant_t value_type_new> struct serialize_impl<derived_type, value_type_new> {
		template<const serialize_options_internal& options, jsonifier::concepts::variant_t value_type, jsonifier::concepts::buffer_like buffer_type,
			jsonifier::concepts::uint64_type index_type>
		JSONIFIER_INLINE static void impl(value_type&& value, buffer_type&& buffer, index_type&& index) {
			visit(
				[&](auto&& valueNew) {
					using member_type = decltype(valueNew);
					serialize_impl<derived_type, member_type>::template impl<options>(valueNew, buffer, index);
				},
				value);
		}
	};

	template<typename derived_type, jsonifier::concepts::optional_t value_type_new> struct serialize_impl<derived_type, value_type_new> {
		template<const serialize_options_internal& options, jsonifier::concepts::optional_t value_type, jsonifier::concepts::buffer_like buffer_type,
			jsonifier::concepts::uint64_type index_type>
		JSONIFIER_INLINE static void impl(value_type&& value, buffer_type&& buffer, index_type&& index) {
			if (value) {
				using member_type = typename unwrap_t<value_type_new>::value_type;
				serialize_impl<derived_type, member_type>::template impl<options>(*value, buffer, index);
			} else {
				writeCharacters<"null">(buffer, index);
			}
		}
	};

	template<typename derived_type, jsonifier::concepts::array_tuple_t value_type_new> struct serialize_impl<derived_type, value_type_new> {
		template<const serialize_options_internal& options, jsonifier::concepts::array_tuple_t value_type, jsonifier::concepts::buffer_like buffer_type,
			jsonifier::concepts::uint64_type index_type>
		JSONIFIER_INLINE static void impl(value_type&& value, buffer_type&& buffer, index_type&& index) {
			static constexpr auto size = std::tuple_size_v<unwrap_t<value_type>>;
			writeArrayEntry<options>(buffer, index, size);
			serializeObjects<size, 0>(value, buffer, index);
			writeArrayExit<options>(buffer, index, size);
		}

		template<const serialize_options_internal& options, size_t n, size_t indexNew = 0, bool areWeFirst = true, jsonifier::concepts::array_tuple_t value_type,
			jsonifier::concepts::buffer_like buffer_type, jsonifier::concepts::uint64_type index_type>
		static void serializeObjects(value_type&& value, buffer_type&& buffer, index_type&& index) {
			auto& item = std::get<indexNew>(value);

			if constexpr (indexNew > 0 && !areWeFirst) {
				writeEntrySeparator<options>(buffer, index);
			}
			using member_type = unwrap_t<decltype(item)>;
			serialize_impl<derived_type, member_type>::template impl<options>(item, buffer, index);
			if constexpr (indexNew < n - 1) {
				serializeObjects<options, n, indexNew + 1, false>(value, buffer, index);
			}
		}
	};

	template<typename derived_type, jsonifier::concepts::vector_t value_type_new> struct serialize_impl<derived_type, value_type_new> {
		template<const serialize_options_internal& options, jsonifier::concepts::vector_t value_type, jsonifier::concepts::buffer_like buffer_type,
			jsonifier::concepts::uint64_type index_type>
		JSONIFIER_INLINE static void impl(value_type&& value, buffer_type&& buffer, index_type&& index) {
			auto n = value.size();
			writeArrayEntry<options>(buffer, index, n);

			if (n != 0) {
				using member_type = typename unwrap_t<value_type_new>::value_type;
				auto iter		  = value.begin();
				serialize_impl<derived_type, member_type>::template impl<options>(*iter, buffer, index);
				++iter;
				for (auto fin = value.end(); iter != fin; ++iter) {
					writeEntrySeparator<options>(buffer, index);
					serialize_impl<derived_type, member_type>::template impl<options>(*iter, buffer, index);
				}
			}
			writeArrayExit<options>(buffer, index, n);
		}
	};

	template<typename derived_type, jsonifier::concepts::pointer_t value_type_new> struct serialize_impl<derived_type, value_type_new> {
		template<const serialize_options_internal& options, jsonifier::concepts::pointer_t value_type, jsonifier::concepts::buffer_like buffer_type,
			jsonifier::concepts::uint64_type index_type>
		JSONIFIER_INLINE static void impl(value_type& value, buffer_type&& buffer, index_type&& index) {
			using member_type = unwrap_t<decltype(*value)>;
			serialize_impl<derived_type, member_type>::template impl<options>(*value, buffer, index);
		}
	};

	template<typename derived_type, jsonifier::concepts::raw_array_t value_type_new> struct serialize_impl<derived_type, value_type_new> {
		template<const serialize_options_internal& options, jsonifier::concepts::raw_array_t value_type, jsonifier::concepts::buffer_like buffer_type,
			jsonifier::concepts::uint64_type index_type>
		JSONIFIER_INLINE static void impl(value_type& value, buffer_type&& buffer, index_type&& index) {
			using member_type		= unwrap_t<decltype(value[0])>;
			static constexpr auto n = std::size(value);
			writeArrayEntry<options>(buffer, index, n);
			if constexpr (n > 0) {
				auto newPtr = value.data();
				serialize_impl<derived_type, member_type>::template impl<options>(*newPtr, buffer, index);
				++newPtr;
				for (size_t x = 1; x < n; ++x) {
					writeEntrySeparator<options>(buffer, index);
					serialize_impl<derived_type, member_type>::template impl<options>(*newPtr, buffer, index);
					++newPtr;
				}
			}
			writeArrayExit<options>(buffer, index, n);
		}
	};

	template<typename derived_type, jsonifier::concepts::raw_json_t value_type_new> struct serialize_impl<derived_type, value_type_new> {
		template<const serialize_options_internal& options, jsonifier::concepts::raw_json_t value_type, jsonifier::concepts::buffer_like buffer_type,
			jsonifier::concepts::uint64_type index_type>
		JSONIFIER_INLINE static void impl(value_type&& value, buffer_type&& buffer, index_type&& index) {
			using member_type = jsonifier::string;
			serialize_impl<derived_type, member_type>::template impl<options>(static_cast<const jsonifier::string>(value), buffer, index);
		}
	};

	template<typename derived_type, jsonifier::concepts::string_t value_type_new> struct serialize_impl<derived_type, value_type_new> {
		template<const serialize_options_internal& options, jsonifier::concepts::string_t value_type, jsonifier::concepts::buffer_like buffer_type,
			jsonifier::concepts::uint64_type index_type>
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

	template<typename derived_type, jsonifier::concepts::char_t value_type_new> struct serialize_impl<derived_type, value_type_new> {
		template<const serialize_options_internal& options, jsonifier::concepts::char_t value_type, jsonifier::concepts::buffer_like buffer_type,
			jsonifier::concepts::uint64_type index_type>
		JSONIFIER_INLINE static void impl(value_type&& value, buffer_type&& buffer, index_type&& index) {
			writeCharacter<json_structural_type::String>(buffer, index);
			switch (value) {
				[[unlikely]] case '\b': {
					writeCharacters(buffer, index, R"(\b)");
					break;
				}
				[[unlikely]] case '\t': {
					writeCharacters(buffer, index, R"(\t)");
					break;
				}
				[[unlikely]] case '\n': {
					writeCharacters(buffer, index, R"(\n)");
					break;
				}
				[[unlikely]] case '\f': {
					writeCharacters(buffer, index, R"(\f)");
					break;
				}
				[[unlikely]] case '\r': {
					writeCharacters(buffer, index, R"(\r)");
					break;
				}
				[[unlikely]] case '"': {
					writeCharacters(buffer, index, R"(\")");
					break;
				}
				[[unlikely]] case '\\': {
					writeCharacters(buffer, index, R"(\\)");
					break;
				}
					[[likely]] default : {
						writeCharacter(buffer, index, value);
					}
			}
			writeCharacter<json_structural_type::String>(buffer, index);
		}
	};

	template<typename derived_type, jsonifier::concepts::unique_ptr_t value_type_new> struct serialize_impl<derived_type, value_type_new> {
		template<const serialize_options_internal& options, jsonifier::concepts::unique_ptr_t value_type, jsonifier::concepts::buffer_like buffer_type,
			jsonifier::concepts::uint64_type index_type>
		JSONIFIER_INLINE static void impl(value_type&& value, buffer_type&& buffer, index_type&& index) {
			using member_type = unwrap_t<decltype(*value)>;
			serialize_impl<derived_type, member_type>::template impl<options>(*value, buffer, index);
		}
	};

	template<typename derived_type, jsonifier::concepts::enum_t value_type_new> struct serialize_impl<derived_type, value_type_new> {
		template<const serialize_options_internal& options, jsonifier::concepts::enum_t value_type, jsonifier::concepts::buffer_like buffer_type,
			jsonifier::concepts::uint64_type index_type>
		JSONIFIER_INLINE static void impl(value_type&& value, buffer_type&& buffer, index_type&& index) {
			auto k = index + 32;
			if (k >= buffer.size()) [[unlikely]] {
				buffer.resize(max(buffer.size() * 2, k));
			}
			int64_t valueNew{ static_cast<int64_t>(value) };
			index = toChars(buffer.data() + index, valueNew) - buffer.data();
		}
	};

	template<typename derived_type, jsonifier::concepts::always_null_t value_type_new> struct serialize_impl<derived_type, value_type_new> {
		template<const serialize_options_internal& options, jsonifier::concepts::always_null_t value_type, jsonifier::concepts::buffer_like buffer_type,
			jsonifier::concepts::uint64_type index_type>
		JSONIFIER_INLINE static void impl(value_type&&, buffer_type&& buffer, index_type&& index) {
			writeCharacters<"null">(buffer, index);
		}
	};

	template<typename derived_type, jsonifier::concepts::bool_t value_type_new> struct serialize_impl<derived_type, value_type_new> {
		template<const serialize_options_internal& options, jsonifier::concepts::bool_t value_type, jsonifier::concepts::buffer_like buffer_type,
			jsonifier::concepts::uint64_type index_type>
		JSONIFIER_INLINE static void impl(value_type&& value, buffer_type&& buffer, index_type&& index) {
			if (value) {
				writeCharacters<"true">(buffer, index);
			} else {
				writeCharacters<"false">(buffer, index);
			}
		}
	};

	template<typename derived_type, jsonifier::concepts::num_t value_type_new> struct serialize_impl<derived_type, value_type_new> {
		template<const serialize_options_internal& options, jsonifier::concepts::num_t value_type, jsonifier::concepts::buffer_like buffer_type,
			jsonifier::concepts::uint64_type index_type>
		JSONIFIER_INLINE static void impl(value_type&& value, buffer_type&& buffer, index_type&& index) {
			auto k = index + 32;
			if (k >= buffer.size()) [[unlikely]] {
				buffer.resize(max(buffer.size() * 2, k));
			}
			index = static_cast<size_t>(toChars(buffer.data() + index, value) - buffer.data());
		}
	};

}