
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

	template<typename derived_type, jsonifier::concepts::jsonifier_value_t value_type_new> struct serialize_impl<derived_type, value_type_new> {
		using value_type = unwrap_t<value_type_new>;
		template<const serialize_options_internal& options, jsonifier::concepts::jsonifier_value_t value_type, jsonifier::concepts::buffer_like buffer_type,
			jsonifier::concepts::uint64_type index_type>
		JSONIFIER_INLINE static void impl(value_type&& value, buffer_type&& buffer, index_type&& index) {
			static constexpr auto numMembers = std::tuple_size_v<unwrap_t<decltype(final_tuple_static_data<value_type>)>>;
			writeObjectEntry<numMembers, options>(std::forward<buffer_type>(buffer), std::forward<index_type>(index));
			static constexpr auto serializeLambda = [](const auto currentIndex, auto&& valueNew, auto&& bufferNew, auto&& indexNew) {
				static constexpr auto subTuple{ std::get<currentIndex>(final_tuple_static_data<value_type>) };
				static constexpr auto subTupleSize{ std::tuple_size_v<unwrap_t<decltype(subTuple)>> };
				static constexpr auto isItLast{ (currentIndex == 0) };
				static constexpr auto serializeLambdaNew = [](const auto currentIndexNewer, auto&& valueNewer, auto&& bufferNewer, auto&& indexNewer) {
					static constexpr auto key = std::get<currentIndexNewer>(subTuple).view();
					if constexpr (jsonifier::concepts::has_excluded_keys<value_type>) {
						auto& keys = valueNewer.jsonifierExcludedKeys;
						if (keys.find(static_cast<typename unwrap_t<decltype(keys)>::key_type>(key)) != keys.end()) [[unlikely]] {
							if constexpr (currentIndexNewer > 1) {
								serializeSubObjects<options, subTuple, isItLast, currentIndexNewer - 1>(std::forward<value_type>(valueNewer),
									std::forward<buffer_type>(bufferNewer), std::forward<index_type>(indexNewer));
							} else {
								return;
							}
						}
					}
					static constexpr auto memberPtr = std::get<currentIndexNewer>(subTuple).ptr();
					static constexpr auto quotedKey = joinV < chars<"\"">, key, options.optionsReal.prettify ? chars<"\": "> : chars < "\":" >> ;
					writeCharacters<quotedKey>(bufferNewer, indexNewer);
					using member_type = unwrap_t<decltype(valueNewer.*memberPtr)>;
					serialize_impl<derived_type, member_type>::template impl<options>(std::forward<member_type>(valueNewer.*memberPtr), std::forward<buffer_type>(bufferNewer),
						std::forward<index_type>(indexNewer));
					if constexpr (!isItLast || currentIndexNewer > 0) {
						if constexpr (options.optionsReal.prettify) {
							if constexpr (jsonifier::concepts::buffer_like<buffer_type>) {
								if (auto k = indexNewer + options.indent + 256; k > bufferNewer.size()) [[unlikely]] {
									bufferNewer.resize(max(bufferNewer.size() * 2, k));
								}
							}
							writeCharacters<",\n", false>(bufferNewer, indexNewer);
							writeCharacters<' ', false>(options.indent * options.optionsReal.indentSize, bufferNewer, indexNewer);
						} else {
							writeCharacter<','>(bufferNewer, indexNewer);
						}
					}
				};
				forEach<subTupleSize, serializeLambdaNew>(std::forward<value_type>(valueNew), std::forward<buffer_type>(bufferNew), std::forward<index_type>(indexNew));
			};
			forEach<numMembers, serializeLambda>(std::forward<value_type>(value), std::forward<buffer_type>(buffer), std::forward<index_type>(index));

			writeObjectExit<numMembers, options>(std::forward<buffer_type>(buffer), std::forward<index_type>(index));
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
				serialize_impl<derived_type, member_type>::template impl<options>(newMember, std::forward<buffer_type>(buffer), std::forward<index_type>(index));
			}
		}
	};

	template<typename derived_type, jsonifier::concepts::map_t value_type_new> struct serialize_impl<derived_type, value_type_new> {
		template<const serialize_options_internal& options, jsonifier::concepts::map_t value_type, jsonifier::concepts::buffer_like buffer_type,
			jsonifier::concepts::uint64_type index_type>
		JSONIFIER_INLINE static void impl(value_type&& value, buffer_type&& buffer, index_type&& index) {
			using member_type = unwrap_t<decltype(value[std::declval<typename unwrap_t<value_type_new>::key_type>()])>;
			if (value.size() > 0) [[likely]] {
				writeObjectEntry(std::forward<buffer_type>(buffer), std::forward<index_type>(index));

				if (value.size() > 0) [[likely]] {
					auto iter = value.begin();
					serialize_impl<derived_type, member_type>::template impl<options>(iter->first, std::forward<buffer_type>(buffer), std::forward<index_type>(index));
					writeCharacter<json_structural_type::Colon>(std::forward<buffer_type>(buffer), std::forward<index_type>(index));
					if constexpr (options.optionsReal.prettify) {
						writeCharacter<0x20u>(std::forward<buffer_type>(buffer), std::forward<index_type>(index));
					}
					serialize_impl<derived_type, member_type>::template impl<options>(iter->second, std::forward<buffer_type>(buffer), std::forward<index_type>(index));
					++iter;
					auto endIter = value.end();
					for (; iter != endIter; ++iter) {
						writeEntrySeparator<options>(std::forward<buffer_type>(buffer), std::forward<index_type>(index));
						serialize_impl<derived_type, member_type>::template impl<options>(iter->first, std::forward<buffer_type>(buffer), std::forward<index_type>(index));
						writeCharacter<json_structural_type::Colon>(std::forward<buffer_type>(buffer), std::forward<index_type>(index));
						if constexpr (options.optionsReal.prettify) {
							writeCharacter<0x20u>(std::forward<buffer_type>(buffer), std::forward<index_type>(index));
						}
						serialize_impl<derived_type, member_type>::template impl<options>(iter->second, std::forward<buffer_type>(buffer), std::forward<index_type>(index));
					}
				}
				writeObjectExit<options>(std::forward<buffer_type>(buffer), std::forward<index_type>(index), std::forward<value_type>(value).size());
			} else {
				writeCharacters<"{}">(std::forward<buffer_type>(buffer), std::forward<index_type>(index));
			}
		}
	};

	template<typename derived_type, jsonifier::concepts::variant_t value_type_new> struct serialize_impl<derived_type, value_type_new> {
		template<const serialize_options_internal& options, jsonifier::concepts::variant_t value_type, jsonifier::concepts::buffer_like buffer_type,
			jsonifier::concepts::uint64_type index_type>
		JSONIFIER_INLINE static void impl(value_type&& value, buffer_type&& buffer, index_type&& index) {
			static constexpr auto lambda = [&](auto&& valueNew, auto&& value, auto&& buffer, auto&& index) {
				using member_type = decltype(valueNew);
				serialize_impl<derived_type, member_type>::template impl<options>(valueNew, std::forward<buffer_type>(buffer), std::forward<index_type>(index));
			};
			visit<lambda>(std::forward<value_type>(value), std::forward<value_type>(value), std::forward<buffer_type>(buffer), std::forward<index_type>(index));
		}
	};

	template<typename derived_type, jsonifier::concepts::optional_t value_type_new> struct serialize_impl<derived_type, value_type_new> {
		template<const serialize_options_internal& options, jsonifier::concepts::optional_t value_type, jsonifier::concepts::buffer_like buffer_type,
			jsonifier::concepts::uint64_type index_type>
		JSONIFIER_INLINE static void impl(value_type&& value, buffer_type&& buffer, index_type&& index) {
			if (value) [[likely]] {
				using member_type = typename unwrap_t<value_type_new>::value_type;
				serialize_impl<derived_type, member_type>::template impl<options>(std::forward<member_type>(*value), std::forward<buffer_type>(buffer),
					std::forward<index_type>(index));
			} else {
				writeCharacters<"null">(std::forward<buffer_type>(buffer), std::forward<index_type>(index));
			}
		}
	};

	template<typename derived_type, jsonifier::concepts::array_tuple_t value_type_new> struct serialize_impl<derived_type, value_type_new> {
		template<const serialize_options_internal& options, jsonifier::concepts::array_tuple_t value_type, jsonifier::concepts::buffer_like buffer_type,
			jsonifier::concepts::uint64_type index_type>
		JSONIFIER_INLINE static void impl(value_type&& value, buffer_type&& buffer, index_type&& index) {
			static constexpr auto size = std::tuple_size_v<unwrap_t<value_type>>;
			writeArrayEntry<options>(std::forward<buffer_type>(buffer), std::forward<index_type>(index));
			static constexpr auto lambda = [](const auto currentIndex, auto&& valueNew, auto&& bufferNew, auto&& indexNew) {
				auto& item = std::get<currentIndex>(valueNew);

				if constexpr (currentIndex > 0) {
					writeEntrySeparator<options>(std::forward<buffer_type>(bufferNew), std::forward<index_type>(indexNew));
				}

				using member_type = unwrap_t<decltype(item)>;
				serialize_impl<derived_type, member_type>::template impl<options>(std::forward<member_type>(item), std::forward<buffer_type>(bufferNew),
					std::forward<index_type>(indexNew));
			};
			forEach<size, lambda>(std::forward<value_type>(value), std::forward<buffer_type>(buffer), std::forward<index_type>(index));
			writeArrayExit<options>(std::forward<buffer_type>(buffer), std::forward<index_type>(index));
		}
	};

	template<typename derived_type, jsonifier::concepts::vector_t value_type_new> struct serialize_impl<derived_type, value_type_new> {
		template<const serialize_options_internal& options, jsonifier::concepts::vector_t value_type, jsonifier::concepts::buffer_like buffer_type,
			jsonifier::concepts::uint64_type index_type>
		JSONIFIER_INLINE static void impl(value_type&& value, buffer_type&& buffer, index_type&& index) {
			const auto maxIndex = value.size();
			if (maxIndex > 0) [[likely]] {
				writeArrayEntry<options>(std::forward<buffer_type>(buffer), std::forward<index_type>(index));
				using member_type = typename unwrap_t<value_type_new>::value_type;
				auto iter		  = std::begin(value);
				serialize_impl<derived_type, member_type>::template impl<options>(std::forward<member_type>(*iter), std::forward<buffer_type>(buffer),
					std::forward<index_type>(index));
				++iter;
				for (const auto end = std::end(value); iter != end; ++iter) {
					writeEntrySeparator<options>(std::forward<buffer_type>(buffer), std::forward<index_type>(index));
					serialize_impl<derived_type, member_type>::template impl<options>(std::forward<member_type>(*iter), std::forward<buffer_type>(buffer),
						std::forward<index_type>(index));
				}
				writeArrayExit<options>(std::forward<buffer_type>(buffer), std::forward<index_type>(index));
			} else {
				writeCharacters<"[]">(std::forward<buffer_type>(buffer), std::forward<index_type>(index));
			}
		}
	};

	template<typename derived_type, jsonifier::concepts::pointer_t value_type_new> struct serialize_impl<derived_type, value_type_new> {
		template<const serialize_options_internal& options, jsonifier::concepts::pointer_t value_type, jsonifier::concepts::buffer_like buffer_type,
			jsonifier::concepts::uint64_type index_type>
		JSONIFIER_INLINE static void impl(value_type&& value, buffer_type&& buffer, index_type&& index) {
			using member_type = unwrap_t<decltype(*value)>;
			serialize_impl<derived_type, member_type>::template impl<options>(std::forward<member_type>(*value), std::forward<buffer_type>(buffer),
				std::forward<index_type>(index));
		}
	};

	template<typename derived_type, jsonifier::concepts::raw_array_t value_type_new> struct serialize_impl<derived_type, value_type_new> {
		template<const serialize_options_internal& options, jsonifier::concepts::raw_array_t value_type, jsonifier::concepts::buffer_like buffer_type,
			jsonifier::concepts::uint64_type index_type>
		JSONIFIER_INLINE static void impl(value_type&& value, buffer_type&& buffer, index_type&& index) {
			const auto maxIndex = value.size();
			if (maxIndex > 0) [[likely]] {
				writeArrayEntry<options>(std::forward<buffer_type>(buffer), std::forward<index_type>(index));

				using member_type = typename unwrap_t<value_type_new>::value_type;
				auto iter		  = std::begin(value);
				serialize_impl<derived_type, member_type>::template impl<options>(std::forward<member_type>(*iter), std::forward<buffer_type>(buffer),
					std::forward<index_type>(index));
				++iter;
				for (const auto end = std::end(value); iter != end; ++iter) {
					writeEntrySeparator<options>(std::forward<buffer_type>(buffer), std::forward<index_type>(index));
					serialize_impl<derived_type, member_type>::template impl<options>(std::forward<member_type>(*iter), std::forward<buffer_type>(buffer),
						std::forward<index_type>(index));
				}
				writeArrayExit<options>(std::forward<buffer_type>(buffer), std::forward<index_type>(index));
			} else {
				writeCharacters<"[]">(std::forward<buffer_type>(buffer), std::forward<index_type>(index));
			}
		}
	};

	template<typename derived_type, jsonifier::concepts::raw_json_t value_type_new> struct serialize_impl<derived_type, value_type_new> {
		template<const serialize_options_internal& options, jsonifier::concepts::raw_json_t value_type, jsonifier::concepts::buffer_like buffer_type,
			jsonifier::concepts::uint64_type index_type>
		JSONIFIER_INLINE static void impl(value_type&& value, buffer_type&& buffer, index_type&& index) {
			using member_type = jsonifier::string;
			serialize_impl<derived_type, member_type>::template impl<options>(static_cast<const jsonifier::string>(value), std::forward<buffer_type>(buffer),
				std::forward<index_type>(index));
		}
	};

	template<typename derived_type, jsonifier::concepts::string_t value_type_new> struct serialize_impl<derived_type, value_type_new> {
		template<const serialize_options_internal& options, jsonifier::concepts::string_t value_type, jsonifier::concepts::buffer_like buffer_type,
			jsonifier::concepts::uint64_type index_type>
		JSONIFIER_INLINE static void impl(value_type&& value, buffer_type&& buffer, index_type&& index) {
			const auto valueSize  = value.size();
			const auto bufferSize = buffer.size();
			const auto k		  = index + 10 + (valueSize * 2);
			if (k >= bufferSize) [[unlikely]] {
				buffer.resize(bufferSize * 2 > k ? bufferSize * 2 : k);
			}
			writeCharacter<'"'>(std::forward<buffer_type>(buffer), std::forward<index_type>(index));
			auto newPtr = buffer.data() + index;
			serializeStringImpl(value.data(), newPtr, valueSize);
			index = newPtr - buffer.data();
			writeCharacter<'"'>(std::forward<buffer_type>(buffer), std::forward<index_type>(index));
		}
	};

	template<typename derived_type, jsonifier::concepts::char_t value_type_new> struct serialize_impl<derived_type, value_type_new> {
		template<const serialize_options_internal& options, jsonifier::concepts::char_t value_type, jsonifier::concepts::buffer_like buffer_type,
			jsonifier::concepts::uint64_type index_type>
		JSONIFIER_INLINE static void impl(value_type&& value, buffer_type&& buffer, index_type&& index) {
			writeCharacter<json_structural_type::String>(std::forward<buffer_type>(buffer), std::forward<index_type>(index));
			switch (value) {
				[[unlikely]] case '\b': {
					writeCharacters(std::forward<buffer_type>(buffer), std::forward<index_type>(index), R"(\b)");
					break;
				}
				[[unlikely]] case '\t': {
					writeCharacters(std::forward<buffer_type>(buffer), std::forward<index_type>(index), R"(\t)");
					break;
				}
				[[unlikely]] case '\n': {
					writeCharacters(std::forward<buffer_type>(buffer), std::forward<index_type>(index), R"(\n)");
					break;
				}
				[[unlikely]] case '\f': {
					writeCharacters(std::forward<buffer_type>(buffer), std::forward<index_type>(index), R"(\f)");
					break;
				}
				[[unlikely]] case '\r': {
					writeCharacters(std::forward<buffer_type>(buffer), std::forward<index_type>(index), R"(\r)");
					break;
				}
				[[unlikely]] case '"': {
					writeCharacters(std::forward<buffer_type>(buffer), std::forward<index_type>(index), R"(\")");
					break;
				}
				[[unlikely]] case '\\': {
					writeCharacters(std::forward<buffer_type>(buffer), std::forward<index_type>(index), R"(\\)");
					break;
				}
				[[likely]] default: { writeCharacter(std::forward<buffer_type>(buffer), std::forward<index_type>(index), std::forward<value_type>(value)); }
			}
			writeCharacter<json_structural_type::String>(std::forward<buffer_type>(buffer), std::forward<index_type>(index));
		}
	};

	template<typename derived_type, jsonifier::concepts::unique_ptr_t value_type_new> struct serialize_impl<derived_type, value_type_new> {
		template<const serialize_options_internal& options, jsonifier::concepts::unique_ptr_t value_type, jsonifier::concepts::buffer_like buffer_type,
			jsonifier::concepts::uint64_type index_type>
		JSONIFIER_INLINE static void impl(value_type&& value, buffer_type&& buffer, index_type&& index) {
			using member_type = unwrap_t<decltype(*value)>;
			serialize_impl<derived_type, member_type>::template impl<options>(std::forward<member_type>(*value), std::forward<buffer_type>(buffer),
				std::forward<index_type>(index));
		}
	};

	template<typename derived_type, jsonifier::concepts::enum_t value_type_new> struct serialize_impl<derived_type, value_type_new> {
		template<const serialize_options_internal& options, jsonifier::concepts::enum_t value_type, jsonifier::concepts::buffer_like buffer_type,
			jsonifier::concepts::uint64_type index_type>
		JSONIFIER_INLINE static void impl(value_type&& value, buffer_type&& buffer, index_type&& index) {
			const auto k		  = index + 32;
			const auto bufferSize = buffer.size();
			if (k >= bufferSize) [[unlikely]] {
				buffer.resize(bufferSize * 2 > k ? bufferSize * 2 : k);
			}
			int64_t valueNew{ static_cast<int64_t>(value) };
			index = toChars(buffer.data() + index, valueNew) - buffer.data();
		}
	};

	template<typename derived_type, jsonifier::concepts::always_null_t value_type_new> struct serialize_impl<derived_type, value_type_new> {
		template<const serialize_options_internal& options, jsonifier::concepts::always_null_t value_type, jsonifier::concepts::buffer_like buffer_type,
			jsonifier::concepts::uint64_type index_type>
		JSONIFIER_INLINE static void impl(value_type&&, buffer_type&& buffer, index_type&& index) {
			writeCharacters<"null">(std::forward<buffer_type>(buffer), std::forward<index_type>(index));
		}
	};

	template<typename derived_type, jsonifier::concepts::bool_t value_type_new> struct serialize_impl<derived_type, value_type_new> {
		template<const serialize_options_internal& options, jsonifier::concepts::bool_t value_type, jsonifier::concepts::buffer_like buffer_type,
			jsonifier::concepts::uint64_type index_type>
		JSONIFIER_INLINE static void impl(value_type&& value, buffer_type&& buffer, index_type&& index) {
			if (value) {
				writeCharacters<"true">(std::forward<buffer_type>(buffer), std::forward<index_type>(index));
			} else {
				writeCharacters<"false">(std::forward<buffer_type>(buffer), std::forward<index_type>(index));
			}
		}
	};

	template<typename derived_type, jsonifier::concepts::num_t value_type_new> struct serialize_impl<derived_type, value_type_new> {
		template<const serialize_options_internal& options, jsonifier::concepts::num_t value_type, jsonifier::concepts::buffer_like buffer_type,
			jsonifier::concepts::uint64_type index_type>
		JSONIFIER_INLINE static void impl(value_type&& value, buffer_type&& buffer, index_type&& index) {
			const auto bufferSize = buffer.size();
			const auto newIndex	  = index + 64;
			if (newIndex > bufferSize) [[unlikely]] {
				buffer.resize(bufferSize * 2 > newIndex ? bufferSize * 2 : newIndex);
			}
			index = static_cast<size_t>(toChars(buffer.data() + index, std::forward<value_type>(value)) - buffer.data());
		}
	};

}
