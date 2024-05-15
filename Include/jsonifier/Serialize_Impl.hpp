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

	template<jsonifier::serialize_options options, typename derived_type, jsonifier::concepts::jsonifier_value_t value_type_new>
	struct serialize_impl<options, derived_type, value_type_new> {
		template<jsonifier::concepts::jsonifier_value_t value_type, jsonifier::concepts::buffer_like buffer_type, jsonifier::concepts::uint64_type index_type,
			jsonifier::concepts::has_find... key_type>
		JSONIFIER_INLINE static void impl(value_type&& value, buffer_type&& buffer, index_type&& index, prettify_arguments<options.prettifyOptions.maxDepth>* prettifyArgs,
			key_type&&... excludedKeys) {
			static constexpr auto n = std::tuple_size_v<jsonifier::concepts::core_wrapper_type<value_type_new>>;
			writeCharacter<json_structural_type::Object_Start>(buffer, index);
			if constexpr (options.prettify) {
				++prettifyArgs->indent;
				prettifyArgs->state[prettifyArgs->indent] = json_structural_type::Object_Start;
				if (prettifyArgs->indent >= static_cast<int64_t>(options.prettifyOptions.maxDepth)) [[unlikely]] {
					return;
				}
				if constexpr (n != 0) {
					writeCharacter<0x0Au>(buffer, index);
					if constexpr (options.prettifyOptions.tabs) {
						writeCharacters<'\t'>(buffer, index, prettifyArgs->indent);
					} else {
						writeCharacters<' '>(buffer, index, static_cast<int64_t>(prettifyArgs->indent * options.prettifyOptions.indentSize));
					}
				}
			}
			if constexpr (n > 0) {
				serializeObjects<n, 0, true>(value, buffer, index, prettifyArgs, excludedKeys...);
			}
			if constexpr (options.prettify) {
				--prettifyArgs->indent;
				if (prettifyArgs->indent < 0) {
					return;
				}
				if (*(buffer.data() + (index - 1)) != 0x7Bu) {
					writeCharacter<0x0Au>(buffer, index);
					if constexpr (options.prettifyOptions.tabs) {
						writeCharacters<'\t'>(buffer, index, prettifyArgs->indent);
					} else {
						writeCharacters<' '>(buffer, index, static_cast<int64_t>(prettifyArgs->indent * options.prettifyOptions.indentSize));
					}
				}
			}
			writeCharacter<json_structural_type::Object_End>(buffer, index);
		}

		template<uint64_t n, uint64_t indexNew = 0, bool areWeFirst = true, jsonifier::concepts::jsonifier_value_t value_type, jsonifier::concepts::buffer_like buffer_type,
			jsonifier::concepts::uint64_type index_type, jsonifier::concepts::has_find... key_type>
		static void serializeObjects(value_type&& value, buffer_type&& buffer, index_type&& index, prettify_arguments<options.prettifyOptions.maxDepth>* prettifyArgs,
			key_type&&... excludedKeys) {
			static constexpr auto& item = get<indexNew>(jsonifier::concepts::core_wrapper_value<jsonifier::concepts::unwrap_t<value_type>>);

			static constexpr jsonifier::string_view key = get<0>(item);
			if constexpr ((( !std::is_void_v<key_type> ) || ...)) {
				if (((excludedKeys.find(static_cast<typename jsonifier::concepts::unwrap_t<key_type...>::key_type>(key)) != excludedKeys.end()) && ...)) [[unlikely]] {
					if constexpr (indexNew < n - 1 && areWeFirst) {
						serializeObjects<n, indexNew + 1, true>(value, buffer, index, prettifyArgs, excludedKeys...);
					} else if constexpr (indexNew < n - 1) {
						serializeObjects<n, indexNew + 1, false>(value, buffer, index, prettifyArgs, excludedKeys...);
					}
					return;
				}
			}
			if constexpr (indexNew > 0 && !areWeFirst) {
				writeCharacter<json_structural_type::Comma>(buffer, index);
				if constexpr (options.prettify) {
					if constexpr (options.prettifyOptions.newLinesInArray) {
						writeCharacter<0x0Au>(buffer, index);
						if constexpr (options.prettifyOptions.tabs) {
							writeCharacters<'\t'>(buffer, index, prettifyArgs->indent);
						} else {
							writeCharacters<' '>(buffer, index, static_cast<int64_t>(prettifyArgs->indent * options.prettifyOptions.indentSize));
						}
					} else {
						if (prettifyArgs->state[prettifyArgs->indent] == json_structural_type::Object_Start) {
							writeCharacter<0x0Au>(buffer, index);
							if constexpr (options.prettifyOptions.tabs) {
								writeCharacters<'\t'>(buffer, index, prettifyArgs->indent);
							} else {
								writeCharacters<' '>(buffer, index, static_cast<int64_t>(prettifyArgs->indent * options.prettifyOptions.indentSize));
							}
						} else {
							writeCharacter<0x20u>(buffer, index);
						}
					}
				}
			}
			writeCharacters(buffer, index, "\"");
			writeCharacters<key>(buffer, index);
			writeCharacters(buffer, index, "\":");
			if constexpr (options.prettify) {
				writeCharacter<0x20u>(buffer, index);
			}

			serializer<derived_type>::template impl<options>(getMember(value, get<1>(item)), buffer, index, prettifyArgs);
			if constexpr (indexNew < n - 1) {
				serializeObjects<n, indexNew + 1, false>(value, buffer, index, prettifyArgs, excludedKeys...);
			}
		}
	};

	template<jsonifier::serialize_options options, typename derived_type, jsonifier::concepts::jsonifier_scalar_value_t value_type_new>
	struct serialize_impl<options, derived_type, value_type_new> {
		template<jsonifier::concepts::jsonifier_scalar_value_t value_type, jsonifier::concepts::buffer_like buffer_type, jsonifier::concepts::uint64_type index_type>
		JSONIFIER_INLINE static void impl(value_type&& value, buffer_type&& buffer, index_type&& index, prettify_arguments<options.prettifyOptions.maxDepth>* prettifyArgs) {
			static constexpr auto size{ std::tuple_size_v<jsonifier::concepts::core_wrapper_type<value_type_new>> };
			if constexpr (size > 0) {
				serializer<derived_type>::template impl<options>(getMember(value, get<0>(jsonifier::concepts::core_wrapper_value<value_type_new>)), buffer, index, prettifyArgs);
			}
		}
	};

	template<jsonifier::serialize_options options, typename derived_type, jsonifier::concepts::map_t value_type_new> struct serialize_impl<options, derived_type, value_type_new> {
		template<jsonifier::concepts::map_t value_type, jsonifier::concepts::buffer_like buffer_type, jsonifier::concepts::uint64_type index_type>
		JSONIFIER_INLINE static void impl(value_type&& value, buffer_type&& buffer, index_type&& index, prettify_arguments<options.prettifyOptions.maxDepth>* prettifyArgs) {
			writeCharacter<json_structural_type::Object_Start>(buffer, index);
			if constexpr (options.prettify) {
				++prettifyArgs->indent;
				prettifyArgs->state[prettifyArgs->indent] = json_structural_type::Object_Start;
				if (prettifyArgs->indent >= static_cast<int64_t>(options.prettifyOptions.maxDepth)) [[unlikely]] {
					return;
				}
				if (value.size() != 0) {
					writeCharacter<0x0Au>(buffer, index);
					if constexpr (options.prettifyOptions.tabs) {
						writeCharacters<'\t'>(buffer, index, prettifyArgs->indent);
					} else {
						writeCharacters<' '>(buffer, index, static_cast<int64_t>(prettifyArgs->indent * options.prettifyOptions.indentSize));
					}
				}
			}

			if (value.size() > 0) [[likely]] {
				auto iter = value.begin();
				serializer<derived_type>::template impl<options>(iter->first, buffer, index, prettifyArgs);
				writeCharacter<json_structural_type::Colon>(buffer, index);
				if constexpr (options.prettify) {
					writeCharacter<0x20u>(buffer, index);
				}
				serializer<derived_type>::template impl<options>(iter->second, buffer, index, prettifyArgs);
				++iter;
				auto endIter = value.end();
				for (; iter != endIter; ++iter) {
					writeCharacter<json_structural_type::Comma>(buffer, index);
					if constexpr (options.prettify) {
						if constexpr (options.prettifyOptions.newLinesInArray) {
							writeCharacter<0x0Au>(buffer, index);
							if constexpr (options.prettifyOptions.tabs) {
								writeCharacters<'\t'>(buffer, index, prettifyArgs->indent);
							} else {
								writeCharacters<' '>(buffer, index, static_cast<int64_t>(prettifyArgs->indent * options.prettifyOptions.indentSize));
							}
						} else {
							if (prettifyArgs->state[prettifyArgs->indent] == json_structural_type::Object_Start) {
								writeCharacter<0x0Au>(buffer, index);
								if constexpr (options.prettifyOptions.tabs) {
									writeCharacters<'\t'>(buffer, index, prettifyArgs->indent);
								} else {
									writeCharacters<' '>(buffer, index, static_cast<int64_t>(prettifyArgs->indent * options.prettifyOptions.indentSize));
								}
							} else {
								writeCharacter<0x20u>(buffer, index);
							}
						}
					}
					serializer<derived_type>::template impl<options>(iter->first, buffer, index, prettifyArgs);
					writeCharacter<json_structural_type::Colon>(buffer, index);
					if constexpr (options.prettify) {
						writeCharacter<0x20u>(buffer, index);
					}
					serializer<derived_type>::template impl<options>(iter->second, buffer, index, prettifyArgs);
				}
			}
			if constexpr (options.prettify) {
				--prettifyArgs->indent;
				if (prettifyArgs->indent < 0) {
					return;
				}
				if (*(buffer.data() + (index - 1)) != 0x7Bu) {
					writeCharacter<0x0Au>(buffer, index);
					if constexpr (options.prettifyOptions.tabs) {
						writeCharacters<'\t'>(buffer, index, prettifyArgs->indent);
					} else {
						writeCharacters<' '>(buffer, index, static_cast<int64_t>(prettifyArgs->indent * options.prettifyOptions.indentSize));
					}
				}
			}
			writeCharacter<json_structural_type::Object_End>(buffer, index);
		}
	};

	template<jsonifier::serialize_options options, typename derived_type, jsonifier::concepts::variant_t value_type_new>
	struct serialize_impl<options, derived_type, value_type_new> {
		template<jsonifier::concepts::variant_t value_type, jsonifier::concepts::buffer_like buffer_type, jsonifier::concepts::uint64_type index_type>
		JSONIFIER_INLINE static void impl(value_type&& value, buffer_type&& buffer, index_type&& index, prettify_arguments<options.prettifyOptions.maxDepth>* prettifyArgs) {
			std::visit(
				[&](auto&& valueNew) {
					serializer<derived_type>::template impl<options>(valueNew, buffer, index, prettifyArgs);
				},
				value);
		}
	};

	template<jsonifier::serialize_options options, typename derived_type, jsonifier::concepts::optional_t value_type_new>
	struct serialize_impl<options, derived_type, value_type_new> {
		template<jsonifier::concepts::optional_t value_type, jsonifier::concepts::buffer_like buffer_type, jsonifier::concepts::uint64_type index_type>
		JSONIFIER_INLINE static void impl(value_type&& value, buffer_type&& buffer, index_type&& index, prettify_arguments<options.prettifyOptions.maxDepth>* prettifyArgs) {
			if (value.has_value()) {
				serializer<derived_type>::template impl<options>(value.value(), buffer, index, prettifyArgs);
			} else {
				serializer<derived_type>::template impl<options>(nullptr, buffer, index, prettifyArgs);
			}
		}
	};

	template<jsonifier::serialize_options options, typename derived_type, jsonifier::concepts::array_tuple_t value_type_new>
	struct serialize_impl<options, derived_type, value_type_new> {
		template<jsonifier::concepts::array_tuple_t value_type, jsonifier::concepts::buffer_like buffer_type, jsonifier::concepts::uint64_type index_type>
		JSONIFIER_INLINE static void impl(value_type&& value, buffer_type&& buffer, index_type&& index, prettify_arguments<options.prettifyOptions.maxDepth>* prettifyArgs) {
			static constexpr auto size = std::tuple_size_v<jsonifier::concepts::unwrap_t<value_type>>;
			writeCharacter<json_structural_type::Array_Start>(buffer, index);
			if constexpr (options.prettify) {
				++prettifyArgs->indent;
				prettifyArgs->state[prettifyArgs->indent] = json_structural_type::Array_Start;
				if (prettifyArgs->indent >= static_cast<int64_t>(options.prettifyOptions.maxDepth)) [[unlikely]] {
					return;
				}
				if constexpr (options.prettifyOptions.newLinesInArray) {
					if (value.size() != 0) {
						writeCharacter<0x0Au>(buffer, index);
						if constexpr (options.prettifyOptions.tabs) {
							writeCharacters<'\t'>(buffer, index, prettifyArgs->indent);
						} else {
							writeCharacters<' '>(buffer, index, static_cast<int64_t>(prettifyArgs->indent * options.prettifyOptions.indentSize));
						}
					}
				} else {
					writeCharacter<0x20u>(buffer, index);
				}
			}
			serializeObjects<size, 0>(value, buffer, index, prettifyArgs);
			if constexpr (options.prettify) {
				--prettifyArgs->indent;
				if (prettifyArgs->indent < 0) {
					return;
				}
				if (*(buffer.data() + (index - 1)) != 0x5Bu) {
					if constexpr (options.prettifyOptions.newLinesInArray) {
						writeCharacter<0x0Au>(buffer, index);
						if constexpr (options.prettifyOptions.tabs) {
							writeCharacters<'\t'>(buffer, index, prettifyArgs->indent);
						} else {
							writeCharacters<' '>(buffer, index, static_cast<int64_t>(prettifyArgs->indent * options.prettifyOptions.indentSize));
						}
					} else {
						writeCharacter<0x20u>(buffer, index);
					}
				}
			}
			writeCharacter<json_structural_type::Array_End>(buffer, index);
		}

		template<uint64_t n, uint64_t indexNew = 0, bool areWeFirst = true, jsonifier::concepts::array_tuple_t value_type, jsonifier::concepts::buffer_like buffer_type,
			jsonifier::concepts::uint64_type index_type>
		static void serializeObjects(value_type&& value, buffer_type&& buffer, index_type&& index, prettify_arguments<options.prettifyOptions.maxDepth>* prettifyArgs) {
			auto& item = std::get<indexNew>(value);

			if constexpr (indexNew > 0 && !areWeFirst) {
				writeCharacter<json_structural_type::Comma>(buffer, index);
				if constexpr (options.prettify) {
					if constexpr (options.prettifyOptions.newLinesInArray) {
						writeCharacter<0x0Au>(buffer, index);
						if constexpr (options.prettifyOptions.tabs) {
							writeCharacters<'\t'>(buffer, index, prettifyArgs->indent);
						} else {
							writeCharacters<' '>(buffer, index, static_cast<int64_t>(prettifyArgs->indent * options.prettifyOptions.indentSize));
						}
					} else {
						if (prettifyArgs->state[prettifyArgs->indent] == json_structural_type::Object_Start) {
							writeCharacter<0x0Au>(buffer, index);
							if constexpr (options.prettifyOptions.tabs) {
								writeCharacters<'\t'>(buffer, index, prettifyArgs->indent);
							} else {
								writeCharacters<' '>(buffer, index, static_cast<int64_t>(prettifyArgs->indent * options.prettifyOptions.indentSize));
							}
						} else {
							writeCharacter<0x20u>(buffer, index);
						}
					}
				}
			}

			serializer<derived_type>::template impl<options>(item, buffer, index, prettifyArgs);
			if constexpr (indexNew < n - 1) {
				serializeObjects<n, indexNew + 1, false>(value, buffer, index, prettifyArgs);
			}
		}
	};

	template<jsonifier::serialize_options options, typename derived_type, jsonifier::concepts::vector_t value_type_new>
	struct serialize_impl<options, derived_type, value_type_new> {
		template<jsonifier::concepts::vector_t value_type, jsonifier::concepts::buffer_like buffer_type, jsonifier::concepts::uint64_type index_type>
		JSONIFIER_INLINE static void impl(value_type&& value, buffer_type&& buffer, index_type&& index, prettify_arguments<options.prettifyOptions.maxDepth>* prettifyArgs) {
			auto n = value.size();
			writeCharacter<json_structural_type::Array_Start>(buffer, index);
			if constexpr (options.prettify) {
				++prettifyArgs->indent;
				prettifyArgs->state[prettifyArgs->indent] = json_structural_type::Array_Start;
				if (prettifyArgs->indent >= static_cast<int64_t>(options.prettifyOptions.maxDepth)) [[unlikely]] {
					return;
				}
				if constexpr (options.prettifyOptions.newLinesInArray) {
					if (value.size() != 0) {
						writeCharacter<0x0Au>(buffer, index);
						if constexpr (options.prettifyOptions.tabs) {
							writeCharacters<'\t'>(buffer, index, prettifyArgs->indent);
						} else {
							writeCharacters<' '>(buffer, index, static_cast<int64_t>(prettifyArgs->indent * options.prettifyOptions.indentSize));
						}
					}
				} else {
					writeCharacter<0x20u>(buffer, index);
				}
			}
			if (n > 0) {
				auto newPtr = value.begin();
				serializer<derived_type>::template impl<options>(*newPtr, buffer, index, prettifyArgs);
				++newPtr;
				auto endPtr = value.begin() + value.size();
				for (; newPtr < endPtr; ++newPtr) {
					writeCharacter<json_structural_type::Comma>(buffer, index);
					if constexpr (options.prettify) {
						if constexpr (options.prettifyOptions.newLinesInArray) {
							writeCharacter<0x0Au>(buffer, index);
							if constexpr (options.prettifyOptions.tabs) {
								writeCharacters<'\t'>(buffer, index, prettifyArgs->indent);
							} else {
								writeCharacters<' '>(buffer, index, static_cast<int64_t>(prettifyArgs->indent * options.prettifyOptions.indentSize));
							}
						} else {
							if (prettifyArgs->state[prettifyArgs->indent] == json_structural_type::Object_Start) {
								writeCharacter<0x0Au>(buffer, index);
								if constexpr (options.prettifyOptions.tabs) {
									writeCharacters<'\t'>(buffer, index, prettifyArgs->indent);
								} else {
									writeCharacters<' '>(buffer, index, static_cast<int64_t>(prettifyArgs->indent * options.prettifyOptions.indentSize));
								}
							} else {
								writeCharacter<0x20u>(buffer, index);
							}
						}
					}
					serializer<derived_type>::template impl<options>(*newPtr, buffer, index, prettifyArgs);
				}
			}
			if constexpr (options.prettify) {
				--prettifyArgs->indent;
				if (prettifyArgs->indent < 0) {
					return;
				}
				if (*(buffer.data() + (index - 1)) != 0x5Bu) {
					if constexpr (options.prettifyOptions.newLinesInArray) {
						writeCharacter<0x0Au>(buffer, index);
						if constexpr (options.prettifyOptions.tabs) {
							writeCharacters<'\t'>(buffer, index, prettifyArgs->indent);
						} else {
							writeCharacters<' '>(buffer, index, static_cast<int64_t>(prettifyArgs->indent * options.prettifyOptions.indentSize));
						}
					} else {
						writeCharacter<0x20u>(buffer, index);
					}
				}
			}
			writeCharacter<json_structural_type::Array_End>(buffer, index);
		}
	};

	template<jsonifier::serialize_options options, typename derived_type, jsonifier::concepts::pointer_t value_type_new>
	struct serialize_impl<options, derived_type, value_type_new> {
		template<jsonifier::concepts::pointer_t value_type, jsonifier::concepts::buffer_like buffer_type, jsonifier::concepts::uint64_type index_type>
		JSONIFIER_INLINE static void impl(value_type& value, buffer_type&& buffer, index_type&& index, prettify_arguments<options.prettifyOptions.maxDepth>* prettifyArgs) {
			serializer<derived_type>::template impl<options>(*value, buffer, index, prettifyArgs);
		}
	};

	template<jsonifier::serialize_options options, typename derived_type, jsonifier::concepts::raw_array_t value_type_new>
	struct serialize_impl<options, derived_type, value_type_new> {
		template<jsonifier::concepts::raw_array_t value_type, jsonifier::concepts::buffer_like buffer_type, jsonifier::concepts::uint64_type index_type>
		JSONIFIER_INLINE static void impl(value_type& value, buffer_type&& buffer, index_type&& index, prettify_arguments<options.prettifyOptions.maxDepth>* prettifyArgs) {
			static constexpr auto n = std::size(value);
			writeCharacter<json_structural_type::Array_Start>(buffer, index);
			if constexpr (options.prettify) {
				++prettifyArgs->indent;
				prettifyArgs->state[prettifyArgs->indent] = json_structural_type::Array_Start;
				if (prettifyArgs->indent >= static_cast<int64_t>(options.prettifyOptions.maxDepth)) [[unlikely]] {
					return;
				}
				if constexpr (options.prettifyOptions.newLinesInArray) {
					if (value.size() != 0) {
						writeCharacter<0x0Au>(buffer, index);
						if constexpr (options.prettifyOptions.tabs) {
							writeCharacters<'\t'>(buffer, index, prettifyArgs->indent);
						} else {
							writeCharacters<' '>(buffer, index, static_cast<int64_t>(prettifyArgs->indent * options.prettifyOptions.indentSize));
						}
					}
				} else {
					writeCharacter<0x20u>(buffer, index);
				}
			}
			if constexpr (n > 0) {
				auto newPtr = value.data();
				serializer<derived_type>::template impl<options>(*newPtr, buffer, index, prettifyArgs);
				++newPtr;
				for (uint64_t x = 1; x < n; ++x) {
					writeCharacter<json_structural_type::Comma>(buffer, index);
					if constexpr (options.prettify) {
						if constexpr (options.prettifyOptions.newLinesInArray) {
							writeCharacter<0x0Au>(buffer, index);
							if constexpr (options.prettifyOptions.tabs) {
								writeCharacters<'\t'>(buffer, index, prettifyArgs->indent);
							} else {
								writeCharacters<' '>(buffer, index, static_cast<int64_t>(prettifyArgs->indent * options.prettifyOptions.indentSize));
							}
						} else {
							if (prettifyArgs->state[prettifyArgs->indent] == json_structural_type::Object_Start) {
								writeCharacter<0x0Au>(buffer, index);
								if constexpr (options.prettifyOptions.tabs) {
									writeCharacters<'\t'>(buffer, index, prettifyArgs->indent);
								} else {
									writeCharacters<' '>(buffer, index, static_cast<int64_t>(prettifyArgs->indent * options.prettifyOptions.indentSize));
								}
							} else {
								writeCharacter<0x20u>(buffer, index);
							}
						}
					}
					serializer<derived_type>::template impl<options>(*newPtr, buffer, index, prettifyArgs);
					++newPtr;
				}
			}
			if constexpr (options.prettify) {
				--prettifyArgs->indent;
				if (prettifyArgs->indent < 0) {
					return;
				}
				if (*(buffer.data() + (index - 1)) != 0x5Bu) {
					if constexpr (options.prettifyOptions.newLinesInArray) {
						writeCharacter<0x0Au>(buffer, index);
						if constexpr (options.prettifyOptions.tabs) {
							writeCharacters<'\t'>(buffer, index, prettifyArgs->indent);
						} else {
							writeCharacters<' '>(buffer, index, static_cast<int64_t>(prettifyArgs->indent * options.prettifyOptions.indentSize));
						}
					} else {
						writeCharacter<0x20u>(buffer, index);
					}
				}
			}
			writeCharacter<json_structural_type::Array_End>(buffer, index);
		}
	};

	template<jsonifier::serialize_options options, typename derived_type, jsonifier::concepts::raw_json_t value_type_new>
	struct serialize_impl<options, derived_type, value_type_new> {
		template<jsonifier::concepts::raw_json_t value_type, jsonifier::concepts::buffer_like buffer_type, jsonifier::concepts::uint64_type index_type>
		JSONIFIER_INLINE static void impl(value_type&& value, buffer_type&& buffer, index_type&& index, prettify_arguments<options.prettifyOptions.maxDepth>* prettifyArgs) {
			serializer<derived_type>::template impl<options>(static_cast<const jsonifier::string>(value), buffer, index, prettifyArgs);
		}
	};

	template<jsonifier::serialize_options options, typename derived_type, jsonifier::concepts::string_t value_type_new>
	struct serialize_impl<options, derived_type, value_type_new> {
		template<jsonifier::concepts::string_t value_type, jsonifier::concepts::buffer_like buffer_type, jsonifier::concepts::uint64_type index_type>
		JSONIFIER_INLINE static void impl(value_type&& value, buffer_type&& buffer, index_type&& index, prettify_arguments<options.prettifyOptions.maxDepth>*) {
			const auto valueSize = value.size();
			const auto k		 = index + 10 + (valueSize * 2);
			if (k >= buffer.size()) [[unlikely]] {
				buffer.resize(max(buffer.size() * 2, k));
			}
			writeCharacter<json_structural_type::String>(buffer, index);
			auto newPtr = buffer.data() + index;
			serializeStringImpl(value.data(), newPtr, valueSize);
			index += newPtr - (buffer.data() + index);
			writeCharacter<json_structural_type::String>(buffer, index);
		}
	};

	template<jsonifier::serialize_options options, typename derived_type, jsonifier::concepts::char_t value_type_new> struct serialize_impl<options, derived_type, value_type_new> {
		template<jsonifier::concepts::char_t value_type, jsonifier::concepts::buffer_like buffer_type, jsonifier::concepts::uint64_type index_type>
		JSONIFIER_INLINE static void impl(value_type&& value, buffer_type&& buffer, index_type&& index, prettify_arguments<options.prettifyOptions.maxDepth>*) {
			writeCharacter<json_structural_type::String>(buffer, index);
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
			writeCharacter<json_structural_type::String>(buffer, index);
		}
	};

	template<jsonifier::serialize_options options, typename derived_type, jsonifier::concepts::unique_ptr_t value_type_new>
	struct serialize_impl<options, derived_type, value_type_new> {
		template<jsonifier::concepts::unique_ptr_t value_type, jsonifier::concepts::buffer_like buffer_type, jsonifier::concepts::uint64_type index_type>
		JSONIFIER_INLINE static void impl(value_type&& value, buffer_type&& buffer, index_type&& index, prettify_arguments<options.prettifyOptions.maxDepth>* prettifyArgs) {
			serializer<derived_type>::template impl<options>(*value, buffer, index, prettifyArgs);
		}
	};

	template<jsonifier::serialize_options options, typename derived_type, jsonifier::concepts::enum_t value_type_new> struct serialize_impl<options, derived_type, value_type_new> {
		template<jsonifier::concepts::enum_t value_type, jsonifier::concepts::buffer_like buffer_type, jsonifier::concepts::uint64_type index_type>
		JSONIFIER_INLINE static void impl(value_type&& value, buffer_type&& buffer, index_type&& index, prettify_arguments<options.prettifyOptions.maxDepth>*) {
			const auto k = index + 32;
			if (k >= buffer.size()) [[unlikely]] {
				buffer.resize(max(buffer.size() * 2, k));
			}
			int64_t valueNew{ static_cast<int64_t>(value) };
			index = toChars(buffer.data() + index, valueNew) - buffer.data();
		}
	};

	template<jsonifier::serialize_options options, typename derived_type, jsonifier::concepts::num_t value_type_new> struct serialize_impl<options, derived_type, value_type_new> {
		template<jsonifier::concepts::num_t value_type, jsonifier::concepts::buffer_like buffer_type, jsonifier::concepts::uint64_type index_type>
		JSONIFIER_INLINE static void impl(value_type&& value, buffer_type&& buffer, index_type&& index, prettify_arguments<options.prettifyOptions.maxDepth>*) {
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

	template<jsonifier::serialize_options options, typename derived_type, jsonifier::concepts::always_null_t value_type_new>
	struct serialize_impl<options, derived_type, value_type_new> {
		template<jsonifier::concepts::always_null_t value_type, jsonifier::concepts::buffer_like buffer_type, jsonifier::concepts::uint64_type index_type>
		JSONIFIER_INLINE static void impl(value_type&&, buffer_type&& buffer, index_type&& index, prettify_arguments<options.prettifyOptions.maxDepth>*) {
			writeCharacters(buffer, index, "null");
		}
	};

	template<jsonifier::serialize_options options, typename derived_type, jsonifier::concepts::bool_t value_type_new> struct serialize_impl<options, derived_type, value_type_new> {
		template<jsonifier::concepts::bool_t value_type, jsonifier::concepts::buffer_like buffer_type, jsonifier::concepts::uint64_type index_type>
		JSONIFIER_INLINE static void impl(value_type&& value, buffer_type&& buffer, index_type&& index, prettify_arguments<options.prettifyOptions.maxDepth>*) {
			value ? writeCharacters(buffer, index, "true") : writeCharacters(buffer, index, "false");
		}
	};

}