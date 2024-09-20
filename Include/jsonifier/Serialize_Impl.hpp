
/*
	MIT License

	Copyright (c) 2024 RealTimeChris

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

	template<const jsonifier::serialize_options& options, jsonifier::concepts::jsonifier_value_t value_type, jsonifier::concepts::buffer_like buffer_type,
		typename serialize_context_type, size_t indentNew>
	struct serialize_impl<options, value_type, buffer_type, serialize_context_type, indentNew> {
		template<typename value_type_new> JSONIFIER_INLINE static void impl(value_type_new&& value, buffer_type& buffer, serialize_context_type& serializePair) noexcept {
			static constexpr auto additionalSize = additionalStringLength<unwrap_t<value_type>, options, indentNew>;
			static constexpr auto numMembers	 = std::tuple_size_v<core_tuple_t<unwrap_t<value_type>>>;

			if (serializePair.index + additionalSize >= buffer.size()) {
				buffer.resize(serializePair.index + additionalSize * 2);
			}

			writer<options>::template writeObjectEntry<numMembers, indentNew>(buffer, serializePair);

			static constexpr auto serializeLambda = [](const auto currentIndex, const auto maxIndex, auto&& value, auto&& buffer, auto&& serializePair) {
				static constexpr auto indent{ indentNew + 1 };
				if constexpr (currentIndex < maxIndex) {
					static constexpr auto subTuple = std::get<currentIndex>(jsonifier::concepts::coreV<unwrap_t<value_type>>);
					static constexpr auto key	   = subTuple.view();

					if constexpr (jsonifier::concepts::has_excluded_keys<unwrap_t<value_type>>) {
						auto& keys = value.jsonifierExcludedKeys;
						if (keys.find(static_cast<typename std::remove_reference_t<decltype(keys)>::key_type>(key)) != keys.end()) [[likely]] {
							return;
						}
					}

					static constexpr auto memberPtr = subTuple.ptr();
					static constexpr auto unQuotedKey{ string_literal{ "\"" } + stringLiteralFromView<key.size()>(key) };
					if constexpr (options.prettify) {
						static constexpr auto quotedKey = unQuotedKey + string_literal{ "\": " };
						writer<options>::template writeCharacters<quotedKey, false>(buffer, serializePair.index);
					} else {
						static constexpr auto quotedKey = unQuotedKey + string_literal{ "\":" };
						writer<options>::template writeCharacters<quotedKey, false>(buffer, serializePair.index);
					}

					serialize_impl<options, std::remove_reference_t<decltype(value.*memberPtr)>, buffer_type, serialize_context_type, indentNew>::impl(value.*memberPtr, buffer,
						serializePair);

					if constexpr (currentIndex < maxIndex - 1) {
						if constexpr (options.prettify) {
							writer<options>::template writeCharacters<",\n", false>(buffer, serializePair.index);
							writer<options>::template writeCharacters<' ', false>(indent * options.indentSize, buffer, serializePair.index);
						} else {
							writer<options>::template writeCharacter<',', false>(buffer, serializePair.index);
						}
					}
					return;
				}
			};

			forEach<numMembers, serializeLambda>(value, buffer, serializePair);

			writer<options>::template writeObjectExit<numMembers, indentNew>(buffer, serializePair);
		}
	};

	template<const jsonifier::serialize_options& options, jsonifier::concepts::map_t value_type, jsonifier::concepts::buffer_like buffer_type, typename serialize_context_type,
		size_t indent>
	struct serialize_impl<options, value_type, buffer_type, serialize_context_type, indent> {
		template<typename value_type_new> JSONIFIER_INLINE static void impl(value_type_new&& value, buffer_type& buffer, serialize_context_type& serializePair) noexcept {
			if (value.size() > 0) [[likely]] {
				writer<options>::template writeObjectEntry<indent>(buffer, serializePair);

				auto iter = value.begin();
				serialize<options, indent>::impl(iter->first, buffer, serializePair);
				writer<options>::template writeCharacter<':'>(buffer, serializePair.index);
				if constexpr (options.prettify) {
					writer<options>::template writeCharacter<0x20u>(buffer, serializePair.index);
				}
				serialize<options, indent>::impl(iter->second, buffer, serializePair);
				++iter;
				auto endIter = value.end();
				for (; iter != endIter; ++iter) {
					writer<options>::template writeEntrySeparator<indent>(buffer, serializePair);
					serialize<options, indent>::impl(iter->first, buffer, serializePair);
					writer<options>::template writeCharacter<':'>(buffer, serializePair.index);
					if constexpr (options.prettify) {
						writer<options>::template writeCharacter<0x20u, indent>(buffer, serializePair.index);
					}
					serialize<options, indent>::impl(iter->second, buffer, serializePair);
				}
				writer<options>::template writeObjectExit<indent>(buffer, serializePair);
			} else {
				writer<options>::template writeCharacters<"{}">(buffer, serializePair.index);
			}
		}
	};

	template<const jsonifier::serialize_options& options, jsonifier::concepts::variant_t value_type, jsonifier::concepts::buffer_like buffer_type, typename serialize_context_type,
		size_t indent>
	struct serialize_impl<options, value_type, buffer_type, serialize_context_type, indent> {
		template<typename value_type_new> JSONIFIER_INLINE static void impl(value_type_new&& value, buffer_type& buffer, serialize_context_type& serializePair) noexcept {
			static constexpr auto lambda = [](auto&& valueNew, auto&& valueNewer, auto&& bufferNew, auto&& indexNew) {
				serialize<options, indent>::impl(valueNewer, bufferNew, indexNew);
			};
			visit<lambda>(value, value, buffer, serializePair);
		}
	};

	template<const jsonifier::serialize_options& options, jsonifier::concepts::optional_t value_type, jsonifier::concepts::buffer_like buffer_type, typename serialize_context_type,
		size_t indent>
	struct serialize_impl<options, value_type, buffer_type, serialize_context_type, indent> {
		template<typename value_type_new> JSONIFIER_INLINE static void impl(value_type_new&& value, buffer_type& buffer, serialize_context_type& serializePair) noexcept {
			if (value) [[likely]] {
				serialize<options, indent>::impl(*value, buffer, serializePair);
			} else {
				writer<options>::template writeCharacters<"null">(buffer, serializePair.index);
			}
		}
	};

	template<const jsonifier::serialize_options& options, jsonifier::concepts::tuple_t value_type, jsonifier::concepts::buffer_like buffer_type, typename serialize_context_type,
		size_t indent>
	struct serialize_impl<options, value_type, buffer_type, serialize_context_type, indent> {
		template<typename value_type_new> JSONIFIER_INLINE static void impl(value_type_new&& value, buffer_type& buffer, serialize_context_type& serializePair) noexcept {
			static constexpr auto size = std::tuple_size_v<std::remove_reference_t<value_type>>;
			writer<options>::template writeArrayEntry<indent>(buffer, serializePair);
			serializeObjects<0, size, indent>(value, buffer, serializePair);
			writer<options>::template writeArrayExit<indent>(buffer, serializePair);
		}

		template<size_t currentIndex, size_t maxIndex, typename value_type_new>
		JSONIFIER_INLINE static void serializeObjects(value_type_new&& value, buffer_type& buffer, serialize_context_type& serializePair) noexcept {
			if constexpr (currentIndex < maxIndex) {
				auto subTuple = std::get<currentIndex>(value);
				serialize<options, indent>::impl(subTuple, buffer, serializePair);
				if constexpr (currentIndex < maxIndex - 1) {
					if constexpr (options.prettify) {
						auto k = serializePair.index + indent + 256;
						if (k > buffer.size()) [[unlikely]] {
							buffer.resize(max(buffer.size() * 2, k));
						}
						writer<options>::template writeCharacters<",\n", indent, false>(buffer, serializePair.index);
						writer<options>::template writeCharacters<' ', indent, false>(indent * options.indentSize, buffer, serializePair.index);
					} else {
						writer<options>::template writeCharacter<','>(buffer, serializePair.index);
					}
				}
				return serializeObjects<currentIndex + 1, maxIndex, indent>(value, buffer, serializePair);
			}
		}
	};

	template<const jsonifier::serialize_options& options, jsonifier::concepts::vector_t value_type, jsonifier::concepts::buffer_like buffer_type, typename serialize_context_type,
		size_t indent>
	struct serialize_impl<options, value_type, buffer_type, serialize_context_type, indent> {
		template<typename value_type_new> JSONIFIER_INLINE static void impl(value_type_new&& value, buffer_type& buffer, serialize_context_type& serializePair) noexcept {
			const auto maxIndex = value.size();
			if (maxIndex > 0) [[likely]] {
				writer<options>::template writeArrayEntry<indent>(buffer, serializePair);
				auto iter = value.begin();
				serialize<options, indent>::impl(*iter, buffer, serializePair);
				++iter;
				for (const auto end = value.end(); iter != end; ++iter) {
					writer<options>::template writeEntrySeparator<indent>(buffer, serializePair);
					serialize<options, indent>::impl(*iter, buffer, serializePair);
				}
				writer<options>::template writeArrayExit<indent>(buffer, serializePair);
			} else {
				writer<options>::template writeCharacters<"[]">(buffer, serializePair.index);
			}
		}
	};

	template<const jsonifier::serialize_options& options, jsonifier::concepts::pointer_t value_type, jsonifier::concepts::buffer_like buffer_type, typename serialize_context_type,
		size_t indent>
	struct serialize_impl<options, value_type, buffer_type, serialize_context_type, indent> {
		template<typename value_type_new> JSONIFIER_INLINE static void impl(value_type_new&& value, buffer_type& buffer, serialize_context_type& serializePair) noexcept {
			serialize<options, indent>::impl(*value, buffer, serializePair);
		}
	};

	template<const jsonifier::serialize_options& options, jsonifier::concepts::raw_array_t value_type, jsonifier::concepts::buffer_like buffer_type,
		typename serialize_context_type, size_t indent>
	struct serialize_impl<options, value_type, buffer_type, serialize_context_type, indent> {
		template<typename value_type_new> JSONIFIER_INLINE static void impl(value_type_new&& value, buffer_type& buffer, serialize_context_type& serializePair) noexcept {
			auto maxIndex = std::size(value);
			if (maxIndex > 0) {
				writer<options>::template writeArrayEntry<indent>(buffer, serializePair);
				auto iter = std::begin(value);
				serialize<options, indent>::impl(*iter, buffer, serializePair);
				++iter;
				for (const auto end = std::end(value); iter != end; ++iter) {
					writer<options>::template writeEntrySeparator<indent>(buffer, serializePair);
					serialize<options, indent>::impl(*iter, buffer, serializePair);
				}
				writer<options>::template writeArrayExit<indent>(buffer, serializePair);
			} else {
				writer<options>::template writeCharacters<"[]">(buffer, serializePair.index);
			}
		}
	};

	template<const jsonifier::serialize_options& options, jsonifier::concepts::raw_json_t value_type, jsonifier::concepts::buffer_like buffer_type, typename serialize_context_type,
		size_t indent>
	struct serialize_impl<options, value_type, buffer_type, serialize_context_type, indent> {
		template<typename value_type_new> JSONIFIER_INLINE static void impl(value_type_new&& value, buffer_type& buffer, serialize_context_type& serializePair) noexcept {
			serialize<options, indent>::impl(static_cast<const jsonifier::string>(value), buffer, serializePair);
		}
	};

	template<const jsonifier::serialize_options& options, jsonifier::concepts::string_t value_type, jsonifier::concepts::buffer_like buffer_type, typename serialize_context_type,
		size_t indent>
	struct serialize_impl<options, value_type, buffer_type, serialize_context_type, indent> {
		template<typename value_type_new> JSONIFIER_INLINE static void impl(value_type_new&& value, buffer_type& buffer, serialize_context_type& serializePair) noexcept {
			const auto valueSize  = value.size();
			const auto bufferSize = buffer.size();
			const auto k		  = serializePair.index + 10 + (valueSize * 2);
			if (k >= bufferSize) [[unlikely]] {
				buffer.resize(bufferSize * 2 > k ? bufferSize * 2 : k);
			}
			writer<options>::template writeCharacter<'"'>(buffer, serializePair.index);
			auto newPtr = buffer.data() + serializePair.index;
			serializeStringImpl(value.data(), newPtr, valueSize);
			serializePair.index = static_cast<size_t>(newPtr - buffer.data());
			writer<options>::template writeCharacter<'"'>(buffer, serializePair.index);
		}
	};

	template<const jsonifier::serialize_options& options, jsonifier::concepts::char_t value_type, jsonifier::concepts::buffer_like buffer_type, typename serialize_context_type,
		size_t indent>
	struct serialize_impl<options, value_type, buffer_type, serialize_context_type, indent> {
		template<typename value_type_new> JSONIFIER_INLINE static void impl(value_type_new&& value, buffer_type& buffer, serialize_context_type& serializePair) noexcept {
			writer<options>::template writeCharacter<'"'>(buffer, serializePair.index);
			switch (value) {
				[[unlikely]] case '\b': {
					writer<options>::writeCharacters(buffer, serializePair.index, R"(\b)");
					break;
				}
				[[unlikely]] case '\t': {
					writer<options>::writeCharacters(buffer, serializePair.index, R"(\t)");
					break;
				}
				[[unlikely]] case '\n': {
					writer<options>::writeCharacters(buffer, serializePair.index, R"(\n)");
					break;
				}
				[[unlikely]] case '\f': {
					writer<options>::writeCharacters(buffer, serializePair.index, R"(\f)");
					break;
				}
				[[unlikely]] case '\r': {
					writer<options>::writeCharacters(buffer, serializePair.index, R"(\r)");
					break;
				}
				[[unlikely]] case '"': {
					writer<options>::writeCharacters(buffer, serializePair.index, R"(\")");
					break;
				}
				[[unlikely]] case '\\': {
					writer<options>::writeCharacters(buffer, serializePair.index, R"(\\)");
					break;
				}
					[[likely]] default : {
						writer<options>::writeCharacter(buffer, serializePair.index, value);
					}
			}
			writer<options>::template writeCharacter<'"'>(buffer, serializePair.index);
		}
	};

	template<const jsonifier::serialize_options& options, jsonifier::concepts::shared_ptr_t value_type, jsonifier::concepts::buffer_like buffer_type,
		typename serialize_context_type, size_t indent>
	struct serialize_impl<options, value_type, buffer_type, serialize_context_type, indent> {
		template<typename value_type_new> JSONIFIER_INLINE static void impl(value_type_new&& value, buffer_type& buffer, serialize_context_type& serializePair) noexcept {
			if (value) [[likely]] {
				serialize<options, indent>::impl(*value, buffer, serializePair);
			}
		}
	};

	template<const jsonifier::serialize_options& options, jsonifier::concepts::unique_ptr_t value_type, jsonifier::concepts::buffer_like buffer_type,
		typename serialize_context_type, size_t indent>
	struct serialize_impl<options, value_type, buffer_type, serialize_context_type, indent> {
		template<typename value_type_new> JSONIFIER_INLINE static void impl(value_type_new&& value, buffer_type& buffer, serialize_context_type& serializePair) noexcept {
			if (value) [[likely]] {
				serialize<options, indent>::impl(*value, buffer, serializePair);
			}
		}
	};

	template<const jsonifier::serialize_options& options, jsonifier::concepts::enum_t value_type, jsonifier::concepts::buffer_like buffer_type, typename serialize_context_type,
		size_t indent>
	struct serialize_impl<options, value_type, buffer_type, serialize_context_type, indent> {
		template<typename value_type_new> JSONIFIER_INLINE static void impl(value_type_new&& value, buffer_type& buffer, serialize_context_type& serializePair) noexcept {
			const auto k		  = serializePair.index + 32;
			const auto bufferSize = buffer.size();
			if (k >= bufferSize) [[unlikely]] {
				buffer.resize(bufferSize * 2 > k ? bufferSize * 2 : k);
			}
			int64_t valueNew{ static_cast<int64_t>(value) };
			serializePair.index = toChars(buffer.data() + serializePair.index, valueNew) - buffer.data();
		}
	};

	template<const jsonifier::serialize_options& options, jsonifier::concepts::always_null_t value_type, jsonifier::concepts::buffer_like buffer_type,
		typename serialize_context_type, size_t indent>
	struct serialize_impl<options, value_type, buffer_type, serialize_context_type, indent> {
		template<typename value_type_new> JSONIFIER_INLINE static void impl(value_type_new&&, buffer_type& buffer, serialize_context_type& serializePair) noexcept {
			writer<options>::template writeCharacters<"null">(buffer, serializePair.index);
		}
	};

	template<const jsonifier::serialize_options& options, jsonifier::concepts::bool_t value_type, jsonifier::concepts::buffer_like buffer_type, typename serialize_context_type,
		size_t indent>
	struct serialize_impl<options, value_type, buffer_type, serialize_context_type, indent> {
		template<typename value_type_new> JSONIFIER_INLINE static void impl(value_type_new&& value, buffer_type& buffer, serialize_context_type& serializePair) noexcept {
			if (value) [[likely]] {
				writer<options>::template writeCharacters<"true">(buffer, serializePair.index);
			} else {
				writer<options>::template writeCharacters<"false">(buffer, serializePair.index);
			}
		}
	};

	template<const jsonifier::serialize_options& options, jsonifier::concepts::num_t value_type, jsonifier::concepts::buffer_like buffer_type, typename serialize_context_type,
		size_t indent>
	struct serialize_impl<options, value_type, buffer_type, serialize_context_type, indent> {
		template<typename value_type_new> JSONIFIER_INLINE static void impl(value_type_new&& value, buffer_type& buffer, serialize_context_type& serializePair) noexcept {
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
