
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

	template<const serialize_options_internal& options, typename derived_type, jsonifier::concepts::jsonifier_value_t value_type_new>
	struct serialize_impl<options, derived_type, value_type_new> {
		template<jsonifier::concepts::jsonifier_value_t value_type, jsonifier::concepts::buffer_like buffer_type, typename serialize_pair_t>
		JSONIFIER_ALWAYS_INLINE static void impl(value_type&& value, buffer_type&& buffer, serialize_pair_t&& serializePair) noexcept {
			static constexpr auto numMembers = std::tuple_size_v<core_tuple_t<value_type>>;
			writer<options>::template writeObjectEntry<numMembers>(std::forward<buffer_type>(buffer), std::forward<serialize_pair_t>(serializePair));
			serializeObjects<0, numMembers>(std::forward<value_type>(value), std::forward<buffer_type>(buffer), std::forward<serialize_pair_t>(serializePair));
			writer<options>::template writeObjectExit<numMembers>(std::forward<buffer_type>(buffer), std::forward<serialize_pair_t>(serializePair));
		}

		template<size_t currentIndex, size_t maxIndex, jsonifier::concepts::jsonifier_value_t value_type, jsonifier::concepts::buffer_like buffer_type, typename serialize_pair_t>
		JSONIFIER_INLINE static void serializeObjects(value_type&& value, buffer_type&& buffer, serialize_pair_t&& serializePair) noexcept {
			if constexpr (currentIndex < maxIndex) {
				static constexpr auto subTuple = std::get<currentIndex>(coreTupleV<value_type>);
				static constexpr auto key		= subTuple.view();
				if constexpr (jsonifier::concepts::has_excluded_keys<value_type>) {
					auto& keys = value.jsonifierExcludedKeys;
					if (keys.find(static_cast<typename std::remove_reference_t<decltype(keys)>::key_type>(key)) != keys.end()) [[likely]] {
						return serializeObjects<currentIndex + 1, maxIndex>(std::forward<value_type>(value), std::forward<buffer_type>(buffer),
							std::forward<serialize_pair_t>(serializePair));
					}
				}
				static constexpr auto memberPtr = subTuple.ptr();
				static constexpr auto unQuotedKey{ string_literal{ "\"" } + stringLiteralFromView<key.size()>(key) };
				if constexpr (options.optionsReal.prettify) {
					static constexpr auto quotedKey = unQuotedKey + string_literal{ "\": " };
					writer<options>::template writeCharacters<quotedKey>(buffer, serializePair.index);
				} else {
					static constexpr auto quotedKey = unQuotedKey + string_literal{ "\":" };
					writer<options>::template writeCharacters<quotedKey>(buffer, serializePair.index);
				}

				using member_type = std::remove_reference_t<decltype(value.*memberPtr)>;
				serialize_impl<options, derived_type, member_type>::impl(value.*memberPtr, std::forward<buffer_type>(buffer), std::forward<serialize_pair_t>(serializePair));
				if constexpr (currentIndex < maxIndex - 1) {
					if constexpr (options.optionsReal.prettify) {
						auto k = serializePair.index + serializePair.indent + 256;
						if (k > buffer.size()) [[unlikely]] {
							buffer.resize(max(buffer.size() * 2, k));
						}
						writer<options>::template writeCharacters<",\n", false>(buffer, serializePair.index);
						writer<options>::template writeCharacters<' ', false>(serializePair.indent * options.optionsReal.indentSize, buffer, serializePair.index);
					} else {
						writer<options>::template writeCharacter<','>(buffer, serializePair.index);
					}
				}
				return serializeObjects<currentIndex + 1, maxIndex>(std::forward<value_type>(value), std::forward<buffer_type>(buffer),
					std::forward<serialize_pair_t>(serializePair));
			}
		}
	};

	template<const serialize_options_internal& options, typename derived_type, jsonifier::concepts::map_t value_type_new>
	struct serialize_impl<options, derived_type, value_type_new> {
		template<jsonifier::concepts::map_t value_type, jsonifier::concepts::buffer_like buffer_type, typename serialize_pair_t>
		JSONIFIER_ALWAYS_INLINE static void impl(value_type&& value, buffer_type&& buffer, serialize_pair_t&& serializePair) noexcept {
			using member_type = unwrap_t<decltype(value.at(std::declval<typename std::remove_reference_t<value_type>::key_type>()))>;
			if (value.size() > 0) [[likely]] {
				writer<options>::writeObjectEntry(std::forward<buffer_type>(buffer), std::forward<serialize_pair_t>(serializePair));

				auto iter = value.begin();
				serialize_impl<options, derived_type, typename std::remove_reference_t<value_type>::key_type>::impl(iter->first, std::forward<buffer_type>(buffer),
					std::forward<serialize_pair_t>(serializePair));
				writer<options>::template writeCharacter<':'>(std::forward<buffer_type>(buffer), serializePair.index);
				if constexpr (options.optionsReal.prettify) {
					writer<options>::template writeCharacter<0x20u>(std::forward<buffer_type>(buffer), serializePair.index);
				}
				serialize_impl<options, derived_type, member_type>::impl(iter->second, std::forward<buffer_type>(buffer), std::forward<serialize_pair_t>(serializePair));
				++iter;
				auto endIter = value.end();
				for (; iter != endIter; ++iter) {
					writer<options>::writeEntrySeparator(std::forward<buffer_type>(buffer), std::forward<serialize_pair_t>(serializePair));
					serialize_impl<options, derived_type, typename std::remove_reference_t<value_type>::key_type>::impl(iter->first, std::forward<buffer_type>(buffer),
						std::forward<serialize_pair_t>(serializePair));
					writer<options>::template writeCharacter<':'>(std::forward<buffer_type>(buffer), serializePair.index);
					if constexpr (options.optionsReal.prettify) {
						writer<options>::template writeCharacter<0x20u>(std::forward<buffer_type>(buffer), serializePair.index);
					}
					serialize_impl<options, derived_type, member_type>::impl(iter->second, std::forward<buffer_type>(buffer), std::forward<serialize_pair_t>(serializePair));
				}
				writer<options>::writeObjectExit(std::forward<buffer_type>(buffer), std::forward<serialize_pair_t>(serializePair));
			} else {
				writer<options>::template writeCharacters<"{}">(std::forward<buffer_type>(buffer), serializePair.index);
			}
		}
	};

	template<const serialize_options_internal& options, typename derived_type, jsonifier::concepts::variant_t value_type_new>
	struct serialize_impl<options, derived_type, value_type_new> {
		template<jsonifier::concepts::variant_t value_type, jsonifier::concepts::buffer_like buffer_type, typename serialize_pair_t>
		JSONIFIER_ALWAYS_INLINE static void impl(value_type&& value, buffer_type&& buffer, serialize_pair_t&& serializePair) noexcept {
			static constexpr auto lambda = [&](auto&& valueNew, auto&& valueNewer, auto&& bufferNew, auto&& indexNew) {
				using member_type = decltype(valueNew);
				serialize_impl<options, derived_type, member_type>::impl(valueNew, std::forward<buffer_type>(buffer), std::forward<serialize_pair_t>(serializePair));
			};
			visit<lambda>(std::forward<value_type>(value), std::forward<value_type>(value), std::forward<buffer_type>(buffer), std::forward<serialize_pair_t>(serializePair));
		}
	};

	template<const serialize_options_internal& options, typename derived_type, jsonifier::concepts::optional_t value_type_new>
	struct serialize_impl<options, derived_type, value_type_new> {
		template<jsonifier::concepts::optional_t value_type, jsonifier::concepts::buffer_like buffer_type, typename serialize_pair_t>
		JSONIFIER_ALWAYS_INLINE static void impl(value_type&& value, buffer_type&& buffer, serialize_pair_t&& serializePair) noexcept {
			if (value) [[likely]] {
				using member_type = typename std::remove_reference_t<value_type>::value_type;
				serialize_impl<options, derived_type, member_type>::impl(*value, std::forward<buffer_type>(buffer), std::forward<serialize_pair_t>(serializePair));
			} else {
				writer<options>::template writeCharacters<"null">(std::forward<buffer_type>(buffer), serializePair.index);
			}
		}
	};

	template<const serialize_options_internal& options, typename derived_type, jsonifier::concepts::tuple_t value_type_new>
	struct serialize_impl<options, derived_type, value_type_new> {
		template<jsonifier::concepts::tuple_t value_type, jsonifier::concepts::buffer_like buffer_type, typename serialize_pair_t>
		JSONIFIER_ALWAYS_INLINE static void impl(value_type&& value, buffer_type&& buffer, serialize_pair_t&& serializePair) noexcept {
			static constexpr auto size = std::tuple_size_v<std::remove_reference_t<value_type>>;
			writer<options>::writeArrayEntry(std::forward<buffer_type>(buffer), std::forward<serialize_pair_t>(serializePair));
			serializeObjects<0, size>(std::forward<value_type>(value), std::forward<buffer_type>(buffer), std::forward<serialize_pair_t>(serializePair));
			writer<options>::writeArrayExit(std::forward<buffer_type>(buffer), std::forward<serialize_pair_t>(serializePair));
		}

		template<size_t currentIndex, size_t maxIndex, jsonifier::concepts::tuple_t value_type, jsonifier::concepts::buffer_like buffer_type, typename serialize_pair_t>
		JSONIFIER_INLINE static void serializeObjects(value_type&& value, buffer_type&& buffer, serialize_pair_t&& serializePair) noexcept {
			if constexpr (currentIndex < maxIndex) {
				auto subTuple	  = std::get<currentIndex>(value);
				using member_type = std::remove_reference_t<decltype(subTuple)>;
				serialize_impl<options, derived_type, member_type>::impl(subTuple, std::forward<buffer_type>(buffer), std::forward<serialize_pair_t>(serializePair));
				if constexpr (currentIndex < maxIndex - 1) {
					if constexpr (options.optionsReal.prettify) {
						auto k = serializePair.index + serializePair.indent + 256;
						if (k > buffer.size()) [[unlikely]] {
							buffer.resize(max(buffer.size() * 2, k));
						}
						writer<options>::template writeCharacters<",\n", false>(buffer, serializePair.index);
						writer<options>::template writeCharacters<' ', false>(serializePair.indent * options.optionsReal.indentSize, buffer, serializePair.index);
					} else {
						writer<options>::template writeCharacter<','>(buffer, serializePair.index);
					}
				}
				return serializeObjects<currentIndex + 1, maxIndex>(std::forward<value_type>(value), std::forward<buffer_type>(buffer),
					std::forward<serialize_pair_t>(serializePair));
			}
		}
	};

	template<const serialize_options_internal& options, typename derived_type, jsonifier::concepts::array_tuple_t value_type_new>
	struct serialize_impl<options, derived_type, value_type_new> {
		template<jsonifier::concepts::array_tuple_t value_type, jsonifier::concepts::buffer_like buffer_type, typename serialize_pair_t>
		JSONIFIER_ALWAYS_INLINE static void impl(value_type&& value, buffer_type&& buffer, serialize_pair_t&& serializePair) noexcept {
			static constexpr auto size = std::tuple_size_v<std::remove_reference_t<value_type>>;
			writer<options>::writeArrayEntry(std::forward<buffer_type>(buffer), std::forward<serialize_pair_t>(serializePair));
			serializeObjects<0, size>(std::forward<value_type>(value), std::forward<buffer_type>(buffer), std::forward<serialize_pair_t>(serializePair));
			writer<options>::writeArrayExit(std::forward<buffer_type>(buffer), std::forward<serialize_pair_t>(serializePair));
		}

		template<size_t currentIndex, size_t maxIndex, jsonifier::concepts::array_tuple_t value_type, jsonifier::concepts::buffer_like buffer_type, typename serialize_pair_t>
		JSONIFIER_INLINE static void serializeObjects(value_type&& value, buffer_type&& buffer, serialize_pair_t&& serializePair) noexcept {
			if constexpr (currentIndex < maxIndex) {
				auto subTuple	  = std::get<currentIndex>(value);
				using member_type = std::remove_reference_t<decltype(subTuple)>;
				serialize_impl<options, derived_type, member_type>::impl(subTuple, std::forward<buffer_type>(buffer), std::forward<serialize_pair_t>(serializePair));
				if constexpr (currentIndex < maxIndex - 1) {
					if constexpr (options.optionsReal.prettify) {
						auto k = serializePair.index + serializePair.indent + 256;
						if (k > buffer.size()) [[unlikely]] {
							buffer.resize(max(buffer.size() * 2, k));
						}
						writer<options>::template writeCharacters<",\n", false>(buffer, serializePair.index);
						writer<options>::template writeCharacters<' ', false>(serializePair.indent * options.optionsReal.indentSize, buffer, serializePair.index);
					} else {
						writer<options>::template writeCharacter<','>(buffer, serializePair.index);
					}
				}
				return serializeObjects<currentIndex + 1, maxIndex>(std::forward<value_type>(value), std::forward<buffer_type>(buffer),
					std::forward<serialize_pair_t>(serializePair));
			}
		}
	};

	template<const serialize_options_internal& options, typename derived_type, jsonifier::concepts::vector_t value_type_new>
	struct serialize_impl<options, derived_type, value_type_new> {
		template<jsonifier::concepts::vector_t value_type, jsonifier::concepts::buffer_like buffer_type, typename serialize_pair_t>
		JSONIFIER_ALWAYS_INLINE static void impl(value_type&& value, buffer_type&& buffer, serialize_pair_t&& serializePair) noexcept {
			const auto maxIndex = value.size();
			if (maxIndex > 0) [[likely]] {
				writer<options>::writeArrayEntry(std::forward<buffer_type>(buffer), std::forward<serialize_pair_t>(serializePair));
				using member_type = typename std::remove_reference_t<value_type>::value_type;
				auto iter		  = value.begin();
				serialize_impl<options, derived_type, member_type>::impl(*iter, std::forward<buffer_type>(buffer), std::forward<serialize_pair_t>(serializePair));
				++iter;
				for (const auto end = value.end(); iter != end; ++iter) {
					writer<options>::writeEntrySeparator(std::forward<buffer_type>(buffer), std::forward<serialize_pair_t>(serializePair));
					serialize_impl<options, derived_type, member_type>::impl(*iter, std::forward<buffer_type>(buffer), std::forward<serialize_pair_t>(serializePair));
				}
				writer<options>::writeArrayExit(std::forward<buffer_type>(buffer), std::forward<serialize_pair_t>(serializePair));
			} else {
				writer<options>::template writeCharacters<"[]">(std::forward<buffer_type>(buffer), serializePair.index);
			}
		}
	};

	template<const serialize_options_internal& options, typename derived_type, jsonifier::concepts::pointer_t value_type_new>
	struct serialize_impl<options, derived_type, value_type_new> {
		template<jsonifier::concepts::pointer_t value_type, jsonifier::concepts::buffer_like buffer_type, typename serialize_pair_t>
		JSONIFIER_ALWAYS_INLINE static void impl(value_type&& value, buffer_type&& buffer, serialize_pair_t&& serializePair) noexcept {
			using member_type = std::remove_reference_t<decltype(*value)>;
			serialize_impl<options, derived_type, member_type>::impl(std::forward<member_type>(*value), std::forward<buffer_type>(buffer),
				std::forward<serialize_pair_t>(serializePair));
		}
	};

	template<const serialize_options_internal& options, typename derived_type, jsonifier::concepts::raw_array_t value_type_new>
	struct serialize_impl<options, derived_type, value_type_new> {
		template<jsonifier::concepts::raw_array_t value_type, jsonifier::concepts::buffer_like buffer_type, typename serialize_pair_t>
		JSONIFIER_ALWAYS_INLINE static void impl(value_type&& value, buffer_type&& buffer, serialize_pair_t&& serializePair) noexcept {
			static constexpr auto maxIndex = std::size(value);
			if constexpr (maxIndex > 0) {
				writer<options>::writeArrayEntry(std::forward<buffer_type>(buffer), std::forward<serialize_pair_t>(serializePair));

				using member_type = typename std::remove_reference_t<value_type>::value_type;
				auto iter		  = std::begin(value);
				serialize_impl<options, derived_type, member_type>::impl(*iter, std::forward<buffer_type>(buffer), std::forward<serialize_pair_t>(serializePair));
				++iter;
				for (const auto end = std::end(value); iter != end; ++iter) {
					writer<options>::writeEntrySeparator(std::forward<buffer_type>(buffer), std::forward<serialize_pair_t>(serializePair));
					serialize_impl<options, derived_type, member_type>::impl(*iter, std::forward<buffer_type>(buffer), std::forward<serialize_pair_t>(serializePair));
				}
				writer<options>::writeArrayExit(std::forward<buffer_type>(buffer), std::forward<serialize_pair_t>(serializePair));
			} else {
				writer<options>::template writeCharacters<"[]">(std::forward<buffer_type>(buffer), serializePair.index);
			}
		}
	};

	template<const serialize_options_internal& options, typename derived_type, jsonifier::concepts::raw_json_t value_type_new>
	struct serialize_impl<options, derived_type, value_type_new> {
		template<jsonifier::concepts::raw_json_t value_type, jsonifier::concepts::buffer_like buffer_type, typename serialize_pair_t>
		JSONIFIER_ALWAYS_INLINE static void impl(value_type&& value, buffer_type&& buffer, serialize_pair_t&& serializePair) noexcept {
			using member_type = jsonifier::string;
			serialize_impl<options, derived_type, member_type>::impl(static_cast<const jsonifier::string>(value), std::forward<buffer_type>(buffer),
				std::forward<serialize_pair_t>(serializePair));
		}
	};

	template<const serialize_options_internal& options, typename derived_type, jsonifier::concepts::string_t value_type_new>
	struct serialize_impl<options, derived_type, value_type_new> {
		template<jsonifier::concepts::string_t value_type, jsonifier::concepts::buffer_like buffer_type, typename serialize_pair_t>
		JSONIFIER_ALWAYS_INLINE static void impl(value_type&& value, buffer_type&& buffer, serialize_pair_t&& serializePair) noexcept {
			const auto valueSize  = value.size();
			const auto bufferSize = buffer.size();
			const auto k		  = serializePair.index + 10 + (valueSize * 2);
			if (k >= bufferSize) [[unlikely]] {
				buffer.resize(bufferSize * 2 > k ? bufferSize * 2 : k);
			}
			writer<options>::template writeCharacter<'"'>(std::forward<buffer_type>(buffer), serializePair.index);
			auto newPtr = buffer.data() + serializePair.index;
			serializeStringImpl(value.data(), newPtr, valueSize);
			serializePair.index = static_cast<size_t>(newPtr - buffer.data());
			writer<options>::template writeCharacter<'"'>(std::forward<buffer_type>(buffer), serializePair.index);
		}
	};

	template<const serialize_options_internal& options, typename derived_type, jsonifier::concepts::char_t value_type_new>
	struct serialize_impl<options, derived_type, value_type_new> {
		template<jsonifier::concepts::char_t value_type, jsonifier::concepts::buffer_like buffer_type, typename serialize_pair_t>
		JSONIFIER_ALWAYS_INLINE static void impl(value_type&& value, buffer_type&& buffer, serialize_pair_t&& serializePair) noexcept {
			writer<options>::template writeCharacter<'"'>(std::forward<buffer_type>(buffer), serializePair.index);
			switch (value) {
				[[unlikely]] case '\b': {
					writer<options>::writeCharacters(std::forward<buffer_type>(buffer), serializePair.index, R"(\b)");
					break;
				}
				[[unlikely]] case '\t': {
					writer<options>::writeCharacters(std::forward<buffer_type>(buffer), serializePair.index, R"(\t)");
					break;
				}
				[[unlikely]] case '\n': {
					writer<options>::writeCharacters(std::forward<buffer_type>(buffer), serializePair.index, R"(\n)");
					break;
				}
				[[unlikely]] case '\f': {
					writer<options>::writeCharacters(std::forward<buffer_type>(buffer), serializePair.index, R"(\f)");
					break;
				}
				[[unlikely]] case '\r': {
					writer<options>::writeCharacters(std::forward<buffer_type>(buffer), serializePair.index, R"(\r)");
					break;
				}
				[[unlikely]] case '"': {
					writer<options>::writeCharacters(std::forward<buffer_type>(buffer), serializePair.index, R"(\")");
					break;
				}
				[[unlikely]] case '\\': {
					writer<options>::writeCharacters(std::forward<buffer_type>(buffer), serializePair.index, R"(\\)");
					break;
				}
				[[likely]] default: { writer<options>::writeCharacter(std::forward<buffer_type>(buffer), serializePair.index, std::forward<value_type>(value)); }
			}
			writer<options>::template writeCharacter<'"'>(std::forward<buffer_type>(buffer), serializePair.index);
		}
	};

	template<const serialize_options_internal& options, typename derived_type, jsonifier::concepts::unique_ptr_t value_type_new>
	struct serialize_impl<options, derived_type, value_type_new> {
		template<jsonifier::concepts::unique_ptr_t value_type, jsonifier::concepts::buffer_like buffer_type, typename serialize_pair_t>
		JSONIFIER_ALWAYS_INLINE static void impl(value_type&& value, buffer_type&& buffer, serialize_pair_t&& serializePair) noexcept {
			using member_type = std::remove_reference_t<decltype(*value)>;
			serialize_impl<options, derived_type, member_type>::impl(std::forward<member_type>(*value), std::forward<buffer_type>(buffer),
				std::forward<serialize_pair_t>(serializePair));
		}
	};

	template<const serialize_options_internal& options, typename derived_type, jsonifier::concepts::enum_t value_type_new>
	struct serialize_impl<options, derived_type, value_type_new> {
		template<jsonifier::concepts::enum_t value_type, jsonifier::concepts::buffer_like buffer_type, typename serialize_pair_t>
		JSONIFIER_ALWAYS_INLINE static void impl(value_type&& value, buffer_type&& buffer, serialize_pair_t&& serializePair) noexcept {
			const auto k		  = serializePair.index + 32;
			const auto bufferSize = buffer.size();
			if (k >= bufferSize) [[unlikely]] {
				buffer.resize(bufferSize * 2 > k ? bufferSize * 2 : k);
			}
			int64_t valueNew{ static_cast<int64_t>(value) };
			serializePair.index = toChars(buffer.data() + serializePair.index, valueNew) - buffer.data();
		}
	};

	template<const serialize_options_internal& options, typename derived_type, jsonifier::concepts::always_null_t value_type_new>
	struct serialize_impl<options, derived_type, value_type_new> {
		template<jsonifier::concepts::always_null_t value_type, jsonifier::concepts::buffer_like buffer_type, typename serialize_pair_t>
		JSONIFIER_ALWAYS_INLINE static void impl(value_type&&, buffer_type&& buffer, serialize_pair_t&& serializePair) noexcept {
			writer<options>::template writeCharacters<"null">(std::forward<buffer_type>(buffer), serializePair.index);
		}
	};

	template<const serialize_options_internal& options, typename derived_type, jsonifier::concepts::bool_t value_type_new>
	struct serialize_impl<options, derived_type, value_type_new> {
		template<jsonifier::concepts::bool_t value_type, jsonifier::concepts::buffer_like buffer_type, typename serialize_pair_t>
		JSONIFIER_ALWAYS_INLINE static void impl(value_type&& value, buffer_type&& buffer, serialize_pair_t&& serializePair) noexcept {
			if (value) [[likely]] {
				writer<options>::template writeCharacters<"true">(std::forward<buffer_type>(buffer), serializePair.index);
			} else {
				writer<options>::template writeCharacters<"false">(std::forward<buffer_type>(buffer), serializePair.index);
			}
		}
	};

	template<const serialize_options_internal& options, typename derived_type, jsonifier::concepts::num_t value_type_new>
	struct serialize_impl<options, derived_type, value_type_new> {
		template<jsonifier::concepts::num_t value_type, jsonifier::concepts::buffer_like buffer_type, typename serialize_pair_t>
		JSONIFIER_ALWAYS_INLINE static void impl(value_type&& value, buffer_type&& buffer, serialize_pair_t&& serializePair) noexcept {
			const auto bufferSize = buffer.size();
			const auto newIndex	  = serializePair.index + 64;
			if (newIndex > bufferSize) [[unlikely]] {
				buffer.resize(bufferSize * 2 > newIndex ? bufferSize * 2 : newIndex);
			}
			if constexpr (jsonifier::concepts::unsigned_type<value_type>) {
				serializePair.index = static_cast<size_t>(toChars(buffer.data() + serializePair.index, static_cast<uint64_t>(value)) - buffer.data());
			} else if constexpr (jsonifier::concepts::signed_type<value_type>) {
				serializePair.index = static_cast<size_t>(toChars(buffer.data() + serializePair.index, static_cast<int64_t>(value)) - buffer.data());
			} else {
				serializePair.index = static_cast<size_t>(toChars(buffer.data() + serializePair.index, static_cast<double>(value)) - buffer.data());
			}
		}
	};

}
