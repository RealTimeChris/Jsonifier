
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

	struct serialize_size_values {
		size_t newLineCount{};
		size_t rtIndex{};
		size_t ctIndex{};
	};

	template<typename value_type>
	concept sizeable_type = jsonifier::concepts::bool_t<value_type> || jsonifier::concepts::num_t<value_type> || jsonifier::concepts::always_null_t<value_type>;

	template<typename value_type> static constexpr auto getPaddingSize() {
		if constexpr (jsonifier::concepts::bool_t<value_type>) {
			return 8;
		} else if constexpr (jsonifier::concepts::num_t<value_type>) {
			return 64;
		} else if constexpr (jsonifier::concepts::always_null_t<value_type>) {
			return 8;
		} else {
			return 0;
		}
	}

	template<jsonifier::serialize_options options, typename value_type, typename size_collect_context_type> struct size_collect_impl;

	template<jsonifier::serialize_options options, jsonifier::concepts::jsonifier_value_t value_type, typename size_collect_context_type>
	struct size_collect_impl<options, value_type, size_collect_context_type> {
		JSONIFIER_ALWAYS_INLINE static size_t impl(size_t indent) noexcept {
			constexpr auto numMembers = std::tuple_size_v<core_tuple_t<value_type>>;
			constexpr auto newSize	  = []() constexpr {
				   serialize_size_values pair{};
				   constexpr auto sizeCollectLambda = [](const auto currentIndex, const auto maxIndex, auto& pairNew) {
					   if constexpr (currentIndex < maxIndex) {
						   constexpr auto subTuple = std::get<currentIndex>(jsonifier::concepts::coreV<value_type>);
						   constexpr auto key	   = subTuple.view();
						   using member_type	   = typename std::remove_cvref_t<decltype(subTuple)>::member_type;
						   pairNew.ctIndex += getPaddingSize<member_type>();
						   constexpr auto unQuotedKey = string_literal{ "\"" } + stringLiteralFromView<key.size()>(key);
						   constexpr auto quotedKey	  = unQuotedKey + string_literal{ "\": " };
						   pairNew.ctIndex += quotedKey.size();
						   if constexpr (currentIndex < maxIndex - 1) {
							   if constexpr (options.prettify) {
								   ++pairNew.newLineCount;
								   pairNew.ctIndex += std::size(",\n") + 1;
							   } else {
								   ++pairNew.ctIndex;
							   }
						   }
					   }
				   };

				   forEach<numMembers, make_static<sizeCollectLambda>::value>(pair);

				   ++pair.ctIndex;
				   ++pair.ctIndex;

				   return pair;
			}();

			return (indent * newSize.newLineCount) + newSize.ctIndex;
		}
	};

	template<jsonifier::serialize_options options, jsonifier::concepts::vector_t value_type, typename size_collect_context_type>
	struct size_collect_impl<options, value_type, size_collect_context_type> {
		JSONIFIER_ALWAYS_INLINE static size_t impl(size_t size, size_t indent) noexcept {
			static constexpr auto newSize = []() constexpr {
				serialize_size_values pair{};
				++pair.ctIndex;
				++pair.ctIndex;
				return pair;
			}();
			if constexpr (sizeable_type<typename std::remove_cvref_t<value_type>::value_type>) {
				if constexpr (options.prettify) {
					if constexpr (options.newLinesInArray) {
						return (indent * size) + newSize.ctIndex + (size * 2) + (size * getPaddingSize<typename std::remove_cvref_t<value_type>::value_type>());
					} else {
						return indent + size + newSize.ctIndex + (size * getPaddingSize<typename std::remove_cvref_t<value_type>::value_type>());
					}
				} else {
					return size + newSize.ctIndex + (size * getPaddingSize<typename std::remove_cvref_t<value_type>::value_type>());
				}
			} else {
				if constexpr (options.prettify) {
					if constexpr (options.newLinesInArray) {
						return (indent * size) + newSize.ctIndex + (size * 2);
					} else {
						return indent + size + newSize.ctIndex;
					}
				} else {
					return size + newSize.ctIndex;
				}
			}
		}
	};

	template<jsonifier::serialize_options options, jsonifier::concepts::raw_array_t value_type, typename size_collect_context_type>
	struct size_collect_impl<options, value_type, size_collect_context_type> {
		JSONIFIER_ALWAYS_INLINE static size_t impl(size_t indent, size_t size) noexcept {
			static constexpr auto newSize = []() constexpr {
				serialize_size_values pair{};
				++pair.ctIndex;
				++pair.ctIndex;
				return pair;
			}();
			if constexpr (options.prettify) {
				if constexpr (options.newLinesInArray) {
					return (indent * size) + newSize.ctIndex + (size * 2);
				} else {
					return indent + size + newSize.ctIndex;
				}
			} else {
				return size + newSize.ctIndex;
			}
		}
	};

	template<jsonifier::serialize_options options, jsonifier::concepts::map_t value_type, typename size_collect_context_type>
	struct size_collect_impl<options, value_type, size_collect_context_type> {
		JSONIFIER_ALWAYS_INLINE static size_t impl(size_t size, size_t indent) noexcept {
			static constexpr auto newSize = []() constexpr {
				serialize_size_values pair{};
				if constexpr (options.prettify) {
					pair.ctIndex += std::size(",\n") + 1;
				} else {
					++pair.ctIndex;
				}

				++pair.ctIndex;
				++pair.ctIndex;

				return pair;
			}();

			return (indent * size) + (size * newSize.ctIndex);
		}
	};

	template<jsonifier::serialize_options options, size_t maxIndex> struct index_processor_serialize {
		template<size_t currentIndex, typename value_type, typename buffer_type, typename serialize_context_type>
		JSONIFIER_ALWAYS_INLINE static void processIndex(const value_type& value, buffer_type& buffer, serialize_context_type& serializePair) noexcept {
			if constexpr (currentIndex < maxIndex) {
				static constexpr auto subTuple = std::get<currentIndex>(jsonifier::concepts::coreV<value_type>);
				static constexpr auto key	   = subTuple.view();
				if constexpr (jsonifier::concepts::has_excluded_keys<value_type>) {
					auto& keys = value.jsonifierExcludedKeys;
					if JSONIFIER_LIKELY ((keys.find(static_cast<typename std::remove_reference_t<decltype(keys)>::key_type>(key)) != keys.end())) {
						return;
					}
				}
				static constexpr auto memberPtr = subTuple.ptr();
				static constexpr auto unQuotedKey{ string_literal{ "\"" } + stringLiteralFromView<key.size()>(key) };
				if constexpr (options.prettify) {
					static constexpr auto quotedKey = unQuotedKey + string_literal{ "\": " };
					static constexpr auto size		= quotedKey.size();
					static constexpr auto newValue{ packValues<quotedKey>() };
					std::memcpy(&buffer[serializePair.index], &newValue, size);
					serializePair.index += size;
				} else {
					static constexpr auto quotedKey = unQuotedKey + string_literal{ "\":" };
					static constexpr auto size		= quotedKey.size();
					static constexpr auto newValue{ packValues<quotedKey>() };
					std::memcpy(&buffer[serializePair.index], &newValue, size);
					serializePair.index += size;
				}

				serialize<setCheckedForSize(options, false)>::impl(value.*memberPtr, buffer, serializePair);
				if constexpr (currentIndex < maxIndex - 1) {
					if constexpr (options.prettify) {
						static constexpr auto newValue{ packValues<",\n">() };
						std::memcpy(&buffer[serializePair.index], &newValue, 2);
						serializePair.index += 2;
						std::memset(&buffer[serializePair.index], ' ', serializePair.indent * options.indentSize);
						serializePair.index += serializePair.indent;
					} else {
						buffer[serializePair.index] = ',';
						++serializePair.index;
					}
				}
				return;
			}
		}
	};

	template<jsonifier::serialize_options options, typename value_type, typename buffer_type, typename serialize_context_type, size_t... indices>
	constexpr auto generateFunctionPtrsImpl(std::index_sequence<indices...>) {
		using function_type = decltype(&index_processor_serialize<options, sizeof...(indices)>::template processIndex<0, value_type, buffer_type, serialize_context_type>);
		return std::array<function_type, sizeof...(indices)>{
			{ &index_processor_serialize<options, sizeof...(indices)>::template processIndex<indices, value_type, buffer_type, serialize_context_type>... }
		};
	}

	template<jsonifier::serialize_options options, typename value_type, typename buffer_type, typename serialize_context_type> constexpr auto generateFunctionPtrs() {
		constexpr auto tupleSize = std::tuple_size_v<core_tuple_t<value_type>>;
		return generateFunctionPtrsImpl<options, value_type, buffer_type, serialize_context_type>(std::make_index_sequence<tupleSize>{});
	}

	template<jsonifier::serialize_options options, jsonifier::concepts::jsonifier_value_t value_type, jsonifier::concepts::buffer_like buffer_type, typename serialize_context_type>
	struct serialize_impl<options, value_type, buffer_type, serialize_context_type> {
		static constexpr auto newValue01{ packValues<"{\n">() };
		static constexpr auto newValue02{ packValues<"{}">() };
		static constexpr auto functionPtrs{ generateFunctionPtrs<options, value_type, buffer_type, serialize_context_type>() };
		template<typename value_type_new> JSONIFIER_ALWAYS_INLINE static void impl(value_type_new&& value, buffer_type& buffer, serialize_context_type& serializePair) noexcept {
			static constexpr auto numMembers{ std::tuple_size_v<core_tuple_t<value_type>> };
			if constexpr (options.checkedForSize) {
				auto additionalSize = size_collect_impl<options, value_type, std::remove_cvref_t<serialize_context_type>>::impl(serializePair.indent);
				if (buffer.size() < serializePair.index + additionalSize) {
					buffer.resize((serializePair.index + additionalSize) * 2);
				}
			}
			if constexpr (numMembers > 0) {
				if constexpr (options.prettify) {
					serializePair.indent += options.indentSize;
					std::memcpy(&buffer[serializePair.index], &newValue01, 2);
					serializePair.index += 2;
					std::memset(&buffer[serializePair.index], options.indentChar, serializePair.indent);
					serializePair.index += serializePair.indent;
				} else {
					buffer[serializePair.index] = '{';
					++serializePair.index;
				}
				for (size_t x = 0; x < numMembers; ++x) {
					functionPtrs[x](std::forward<value_type_new>(value), buffer, serializePair);
				}
				if constexpr (options.prettify && numMembers > 0) {
					serializePair.indent -= options.indentSize;
					buffer[serializePair.index] = '\n';
					++serializePair.index;
					std::memset(&buffer[serializePair.index], options.indentChar, serializePair.indent);
					serializePair.index += serializePair.indent;
				}
				buffer[serializePair.index] = '}';
				++serializePair.index;
			} else {
				std::memcpy(&buffer[serializePair.index], &newValue02, 2);
				serializePair.index += 2;
			}
		}
	};

	template<jsonifier::serialize_options options, jsonifier::concepts::jsonifier_scalar_value_t value_type, jsonifier::concepts::buffer_like buffer_type,
		typename serialize_context_type>
	struct serialize_impl<options, value_type, buffer_type, serialize_context_type> {
		template<typename value_type_new> JSONIFIER_ALWAYS_INLINE static void impl(value_type_new&& value, buffer_type& buffer, serialize_context_type& serializePair) noexcept {
			static constexpr auto size{ std::tuple_size_v<core_tuple_t<value_type>> };
			if constexpr (size == 1) {
				static constexpr auto newPtr = std::get<0>(coreTupleV<value_type>);
				serialize<options>::impl(getMember<newPtr>(value), buffer, serializePair);
			}
		}
	};

	template<jsonifier::serialize_options options, jsonifier::concepts::map_t value_type, jsonifier::concepts::buffer_like buffer_type, typename serialize_context_type>
	struct serialize_impl<options, value_type, buffer_type, serialize_context_type> {
		static constexpr auto newValue01{ packValues<"{\n">() };
		static constexpr auto newValue02{ packValues<": ">() };
		static constexpr auto newValue03{ packValues<",\n">() };
		static constexpr auto newValue04{ packValues<": ">() };
		static constexpr auto newValue05{ packValues<"{}">() };
		template<typename value_type_new> JSONIFIER_ALWAYS_INLINE static void impl(value_type_new&& value, buffer_type& buffer, serialize_context_type& serializePair) noexcept {
			const auto maxIndex = value.size();
			if constexpr (options.checkedForSize) {
				auto additionalSize = size_collect_impl<options, value_type, std::remove_cvref_t<serialize_context_type>>::impl(maxIndex, serializePair.indent);
				if (buffer.size() < serializePair.index + additionalSize) {
					buffer.resize((serializePair.index + additionalSize) * 2);
				}
			}
			if JSONIFIER_LIKELY ((maxIndex > 0)) {
				if constexpr (options.prettify) {
					serializePair.indent += options.indentSize;
					std::memcpy(&buffer[serializePair.index], &newValue01, 2);
					serializePair.index += 2;
					std::memset(&buffer[serializePair.index], options.indentChar, serializePair.indent);
					serializePair.index += serializePair.indent;
				} else {
					buffer[serializePair.index] = '{';
					++serializePair.index;
				}
				auto iter = value.begin();
				serialize<options>::impl(iter->first, buffer, serializePair);
				if constexpr (options.prettify) {
					std::memcpy(&buffer[serializePair.index], &newValue02, 2);
					serializePair.index += 2;
				} else {
					buffer[serializePair.index] = ':';
					++serializePair.index;
				}
				serialize<options>::impl(iter->second, buffer, serializePair);
				++iter;
				const auto end = value.end();
				for (; iter != end; ++iter) {
					if constexpr (options.prettify) {
						std::memcpy(&buffer[serializePair.index], &newValue03, 2);
						serializePair.index += 2;
						std::memset(&buffer[serializePair.index], options.indentChar, serializePair.indent);
						serializePair.index += serializePair.indent;
					} else {
						buffer[serializePair.index] = ',';
						++serializePair.index;
					}
					serialize<options>::impl(iter->first, buffer, serializePair);
					if constexpr (options.prettify) {
						std::memcpy(&buffer[serializePair.index], &newValue04, 2);
						serializePair.index += 2;
					} else {
						buffer[serializePair.index] = ':';
						++serializePair.index;
					}
					serialize<options>::impl(iter->second, buffer, serializePair);
				}
				if constexpr (options.prettify) {
					serializePair.indent -= options.indentSize;
					buffer[serializePair.index] = '\n';
					++serializePair.index;
					std::memset(&buffer[serializePair.index], options.indentChar, serializePair.indent);
					serializePair.index += serializePair.indent;
				}
				buffer[serializePair.index] = '}';
				++serializePair.index;
			} else {
				std::memcpy(&buffer[serializePair.index], &newValue05, 2);
				serializePair.index += 2;
			}
		}
	};

	template<jsonifier::serialize_options options, jsonifier::concepts::variant_t value_type, jsonifier::concepts::buffer_like buffer_type, typename serialize_context_type>
	struct serialize_impl<options, value_type, buffer_type, serialize_context_type> {
		template<typename value_type_new> JSONIFIER_ALWAYS_INLINE static void impl(value_type_new&& value, buffer_type& buffer, serialize_context_type& serializePair) noexcept {
			static constexpr auto lambda = [](auto&& valueNewer, auto&& bufferNew, auto&& indexNew) {
				serialize<options>::impl(valueNewer, bufferNew, indexNew);
			};
			visit<lambda>(value, buffer, serializePair);
		}
	};

	template<jsonifier::serialize_options options, jsonifier::concepts::optional_t value_type, jsonifier::concepts::buffer_like buffer_type, typename serialize_context_type>
	struct serialize_impl<options, value_type, buffer_type, serialize_context_type> {
		static constexpr uint32_t nullV{ 1819047278 };
		template<typename value_type_new> JSONIFIER_ALWAYS_INLINE static void impl(value_type_new&& value, buffer_type& buffer, serialize_context_type& serializePair) noexcept {
			if JSONIFIER_LIKELY ((value)) {
				serialize<options>::impl(*value, buffer, serializePair);
			} else {
				std::memcpy(&buffer[serializePair.index], &nullV, 4);
				serializePair.index += 4;
			}
		}
	};

	template<jsonifier::serialize_options options, jsonifier::concepts::tuple_t value_type, jsonifier::concepts::buffer_like buffer_type, typename serialize_context_type>
	struct serialize_impl<options, value_type, buffer_type, serialize_context_type> {
		template<typename value_type_new> JSONIFIER_ALWAYS_INLINE static void impl(value_type_new&& value, buffer_type& buffer, serialize_context_type& serializePair) noexcept {
			static constexpr auto size	= std::tuple_size_v<std::remove_reference_t<value_type>>;
			buffer[serializePair.index] = '[';
			++serializePair.index;
			if constexpr (options.prettify) {
				serializePair.indent += options.indentSize;
				buffer[serializePair.index] = '\n';
				++serializePair.index;
				std::memset(&buffer[serializePair.index], options.indentChar, serializePair.indent);
				serializePair.index += serializePair.indent;
			}
			serializeObjects<0, size>(value, buffer, serializePair);
			if constexpr (options.prettify) {
				serializePair.indent -= options.indentSize;
				buffer[serializePair.index] = '\n';
				++serializePair.index;
				std::memset(&buffer[serializePair.index], options.indentChar, serializePair.indent);
				serializePair.index += serializePair.indent;
			}
			buffer[serializePair.index] = ']';
			++serializePair.index;
		}

		template<size_t currentIndex, size_t maxIndex, typename value_type_new>
		JSONIFIER_ALWAYS_INLINE static void serializeObjects(value_type_new&& value, buffer_type& buffer, serialize_context_type& serializePair) noexcept {
			if constexpr (currentIndex < maxIndex) {
				auto subTuple = std::get<currentIndex>(value);
				serialize<options>::impl(subTuple, buffer, serializePair);
				if constexpr (currentIndex < maxIndex - 1) {
					if constexpr (options.prettify) {
						static constexpr auto size = std::size(",\n") - 1;
						std::memcpy(&buffer[serializePair.index], ",\n", size);
						serializePair.index += size;
						std::memset(&buffer[serializePair.index], options.indentChar, serializePair.indent * options.indentSize);
						serializePair.index += serializePair.indent;
						;
					} else {
						buffer[serializePair.index] = ',';
						++serializePair.index;
					}
				}
				return serializeObjects<currentIndex + 1, maxIndex>(value, buffer, serializePair);
			}
		}
	};

	template<jsonifier::serialize_options options, jsonifier::concepts::vector_t value_type, jsonifier::concepts::buffer_like buffer_type, typename serialize_context_type>
	struct serialize_impl<options, value_type, buffer_type, serialize_context_type> {
		static constexpr auto newValue01{ packValues<"[\n">() };
		static constexpr auto newValue02{ packValues<",\n">() };
		static constexpr auto newValue03{ packValues<"[]">() };
		template<typename value_type_new> JSONIFIER_ALWAYS_INLINE static void impl(value_type_new&& value, buffer_type& buffer, serialize_context_type& serializePair) noexcept {
			const auto maxIndex = value.size();
			if constexpr (options.checkedForSize) {
				auto additionalSize = size_collect_impl<options, value_type, std::remove_cvref_t<serialize_context_type>>::impl(maxIndex, serializePair.indent);
				if (buffer.size() < serializePair.index + additionalSize) {
					buffer.resize((serializePair.index + additionalSize) * 2);
				}
			}
			if JSONIFIER_LIKELY ((maxIndex > 0)) {
				if constexpr (options.prettify) {
					serializePair.indent += options.indentSize;
					std::memcpy(&buffer[serializePair.index], &newValue01, 2);
					serializePair.index += 2;
					std::memset(&buffer[serializePair.index], options.indentChar, serializePair.indent);
					serializePair.index += serializePair.indent;
				} else {
					buffer[serializePair.index] = '[';
					++serializePair.index;
				}
				auto iter = value.begin();
				serialize<setCheckedForSize(options, !sizeable_type<typename std::remove_cvref_t<value_type>::value_type>)>::impl(*iter, buffer, serializePair);
				++iter;
				const auto end = value.end();
				for (; iter != end; ++iter) {
					if constexpr (options.prettify) {
						std::memcpy(&buffer[serializePair.index], &newValue02, 2);
						serializePair.index += 2;
						std::memset(&buffer[serializePair.index], options.indentChar, serializePair.indent);
						serializePair.index += serializePair.indent;
					} else {
						buffer[serializePair.index] = ',';
						++serializePair.index;
					}
					serialize<setCheckedForSize(options, !sizeable_type<typename std::remove_cvref_t<value_type>::value_type>)>::impl(*iter, buffer, serializePair);
				}
				if constexpr (options.prettify) {
					serializePair.indent -= options.indentSize;
					buffer[serializePair.index] = '\n';
					++serializePair.index;
					std::memset(&buffer[serializePair.index], options.indentChar, serializePair.indent);
					serializePair.index += serializePair.indent;
				}
				buffer[serializePair.index] = ']';
				++serializePair.index;
			} else {
				std::memcpy(&buffer[serializePair.index], &newValue03, 2);
				serializePair.index += 2;
			}
		}
	};

	template<jsonifier::serialize_options options, jsonifier::concepts::pointer_t value_type, jsonifier::concepts::buffer_like buffer_type, typename serialize_context_type>
	struct serialize_impl<options, value_type, buffer_type, serialize_context_type> {
		static constexpr uint32_t nullV{ 1819047278 };
		template<typename value_type_new> JSONIFIER_ALWAYS_INLINE static void impl(value_type_new&& value, buffer_type& buffer, serialize_context_type& serializePair) noexcept {
			if (value) {
				serialize<options>::impl(*value, buffer, serializePair);
			} else {
				std::memcpy(&buffer[serializePair.index], &nullV, 4);
				serializePair.index += 4;
			}
		}
	};

	template<jsonifier::serialize_options options, jsonifier::concepts::raw_array_t value_type, jsonifier::concepts::buffer_like buffer_type, typename serialize_context_type>
	struct serialize_impl<options, value_type, buffer_type, serialize_context_type> {
		static constexpr auto newValue01{ packValues<"[\n">() };
		static constexpr auto newValue02{ packValues<",\n">() };
		static constexpr auto newValue03{ packValues<"[]">() };
		template<template<typename, size_t> typename value_type_new, typename value_type_internal, size_t size>
		JSONIFIER_ALWAYS_INLINE static void impl(value_type_new<value_type_internal, size>& value, buffer_type& buffer, serialize_context_type& serializePair) noexcept {
			const auto maxIndex = std::size(value);
			if constexpr (options.checkedForSize) {
				auto additionalSize = size_collect_impl<options, value_type, std::remove_cvref_t<serialize_context_type>>::impl(serializePair.indent, maxIndex);
				if (buffer.size() < serializePair.index + additionalSize) {
					buffer.resize((serializePair.index + additionalSize) * 2);
				}
			}
			if JSONIFIER_LIKELY ((maxIndex > 0)) {
				if constexpr (options.prettify) {
					serializePair.indent += options.indentSize;
					std::memcpy(&buffer[serializePair.index], &newValue01, 2);
					serializePair.index += 2;
					std::memset(&buffer[serializePair.index], options.indentChar, serializePair.indent);
					serializePair.index += serializePair.indent;
				} else {
					buffer[serializePair.index] = '[';
					++serializePair.index;
				}
				auto iter = std::begin(value);
				serialize<options>::impl(*iter, buffer, serializePair);
				++iter;
				const auto end = std::end(value);
				for (; iter != end; ++iter) {
					if constexpr (options.prettify) {
						std::memcpy(&buffer[serializePair.index], &newValue02, 2);
						serializePair.index += 2;
						std::memset(&buffer[serializePair.index], options.indentChar, serializePair.indent);
						serializePair.index += serializePair.indent;
					} else {
						buffer[serializePair.index] = ',';
						++serializePair.index;
					}
					serialize<options>::impl(*iter, buffer, serializePair);
				}
				if constexpr (options.prettify) {
					serializePair.indent -= options.indentSize;
					buffer[serializePair.index] = '\n';
					++serializePair.index;
					std::memset(&buffer[serializePair.index], options.indentChar, serializePair.indent);
					serializePair.index += serializePair.indent;
				}
				buffer[serializePair.index] = ']';
				++serializePair.index;
			} else {
				std::memcpy(&buffer[serializePair.index], &newValue03, 2);
				serializePair.index += 2;
			}
		}
	};

	template<jsonifier::serialize_options options, jsonifier::concepts::raw_json_t value_type, jsonifier::concepts::buffer_like buffer_type, typename serialize_context_type>
	struct serialize_impl<options, value_type, buffer_type, serialize_context_type> {
		template<typename value_type_new> JSONIFIER_ALWAYS_INLINE static void impl(value_type_new&& value, buffer_type& buffer, serialize_context_type& serializePair) noexcept {
			serialize<options>::impl(static_cast<const jsonifier::string>(value), buffer, serializePair);
		}
	};

	template<jsonifier::serialize_options options, jsonifier::concepts::string_t value_type, jsonifier::concepts::buffer_like buffer_type, typename serialize_context_type>
	struct serialize_impl<options, value_type, buffer_type, serialize_context_type> {
		template<typename value_type_new> JSONIFIER_ALWAYS_INLINE static void impl(value_type_new&& value, buffer_type& buffer, serialize_context_type& serializePair) noexcept {
			if constexpr (options.checkedForSize) {
				auto additionalSize = value.size() + 2 + serializePair.indent;
				if (buffer.size() < serializePair.index + additionalSize) {
					buffer.resize((serializePair.index + additionalSize) * 2);
				}
			}
			buffer[serializePair.index] = '"';
			++serializePair.index;
			auto newPtr = &buffer[serializePair.index];
			serializeStringImpl(value.data(), newPtr, value.size());
			serializePair.index			= static_cast<size_t>(newPtr - buffer.data());
			buffer[serializePair.index] = '"';
			++serializePair.index;
		}
	};

	template<jsonifier::serialize_options options, jsonifier::concepts::char_t value_type, jsonifier::concepts::buffer_like buffer_type, typename serialize_context_type>
	struct serialize_impl<options, value_type, buffer_type, serialize_context_type> {
		template<typename value_type_new> JSONIFIER_ALWAYS_INLINE static void impl(value_type_new&& value, buffer_type& buffer, serialize_context_type& serializePair) noexcept {
			if constexpr (options.checkedForSize) {
				auto additionalSize = 3 + serializePair.indent;
				if (buffer.size() < serializePair.index + additionalSize) {
					buffer.resize((serializePair.index + additionalSize) * 2);
				}
			}
			buffer[serializePair.index] = '"';
			++serializePair.index;
			switch (value) {
				[[unlikely]] case '\b': {
					static constexpr auto size = std::size(R"(\b)") - 1;
					std::memcpy(&buffer[serializePair.index], R"(\b)", size);
					serializePair.index += size;
					break;
				}
				[[unlikely]] case '\t': {
					static constexpr auto size = std::size(R"(\t)") - 1;
					std::memcpy(&buffer[serializePair.index], R"(\t)", size);
					serializePair.index += size;
					break;
				}
				[[unlikely]] case '\n': {
					static constexpr auto size = std::size(R"(\n)") - 1;
					std::memcpy(&buffer[serializePair.index], R"(\n)", size);
					serializePair.index += size;
					break;
				}
				[[unlikely]] case '\f': {
					static constexpr auto size = std::size(R"(\f)") - 1;
					std::memcpy(&buffer[serializePair.index], R"(\f)", size);
					serializePair.index += size;
					break;
				}
				[[unlikely]] case '\r': {
					static constexpr auto size = std::size(R"(\r)") - 1;
					std::memcpy(&buffer[serializePair.index], R"(\r)", size);
					serializePair.index += size;
					break;
				}
				[[unlikely]] case '"': {
					static constexpr auto size = std::size(R"(\")") - 1;
					std::memcpy(&buffer[serializePair.index], R"(\")", size);
					serializePair.index += size;
					break;
				}
				[[unlikely]] case '\\': {
					static constexpr auto size = std::size(R"(\\)") - 1;
					std::memcpy(&buffer[serializePair.index], R"(\\)", size);
					serializePair.index += size;
					break;
				}
				[[likely]] default: {
					buffer[serializePair.index] = value;
					++serializePair.index;
				}
			}
			buffer[serializePair.index] = '"';
			++serializePair.index;
		}
	};

	template<jsonifier::serialize_options options, jsonifier::concepts::shared_ptr_t value_type, jsonifier::concepts::buffer_like buffer_type, typename serialize_context_type>
	struct serialize_impl<options, value_type, buffer_type, serialize_context_type> {
		static constexpr uint32_t nullV{ 1819047278 };
		template<typename value_type_new> JSONIFIER_ALWAYS_INLINE static void impl(value_type_new&& value, buffer_type& buffer, serialize_context_type& serializePair) noexcept {
			if JSONIFIER_LIKELY ((value)) {
				serialize<options>::impl(*value, buffer, serializePair);
			} else {
				std::memcpy(&buffer[serializePair.index], &nullV, 4);
				serializePair.index += 4;
			}
		}
	};

	template<jsonifier::serialize_options options, jsonifier::concepts::unique_ptr_t value_type, jsonifier::concepts::buffer_like buffer_type, typename serialize_context_type>
	struct serialize_impl<options, value_type, buffer_type, serialize_context_type> {
		static constexpr uint32_t nullV{ 1819047278 };
		template<typename value_type_new> JSONIFIER_ALWAYS_INLINE static void impl(value_type_new&& value, buffer_type& buffer, serialize_context_type& serializePair) noexcept {
			if JSONIFIER_LIKELY ((value)) {
				serialize<options>::impl(*value, buffer, serializePair);
			} else {
				std::memcpy(&buffer[serializePair.index], &nullV, 4);
				serializePair.index += 4;
			}
		}
	};

	template<jsonifier::serialize_options options, jsonifier::concepts::enum_t value_type, jsonifier::concepts::buffer_like buffer_type, typename serialize_context_type>
	struct serialize_impl<options, value_type, buffer_type, serialize_context_type> {
		template<typename value_type_new> JSONIFIER_ALWAYS_INLINE static void impl(value_type_new&& value, buffer_type& buffer, serialize_context_type& serializePair) noexcept {
			int64_t valueNew{ static_cast<int64_t>(value) };
			serialize<options>::impl(valueNew, buffer, serializePair);
		}
	};

	template<jsonifier::serialize_options options, jsonifier::concepts::always_null_t value_type, jsonifier::concepts::buffer_like buffer_type, typename serialize_context_type>
	struct serialize_impl<options, value_type, buffer_type, serialize_context_type> {
		static constexpr uint32_t nullV{ 1819047278 };
		template<typename value_type_new> JSONIFIER_ALWAYS_INLINE static void impl(value_type_new&&, buffer_type& buffer, serialize_context_type& serializePair) noexcept {
			if constexpr (options.checkedForSize) {
				auto additionalSize = 4 + serializePair.indent;
				if (buffer.size() < serializePair.index + additionalSize) {
					buffer.resize((serializePair.index + additionalSize) * 2);
				}
			}
			std::memcpy(&buffer[serializePair.index], &nullV, 4);
			serializePair.index += 4;
		}
	};

	template<jsonifier::serialize_options options, jsonifier::concepts::bool_t value_type, jsonifier::concepts::buffer_like buffer_type, typename serialize_context_type>
	struct serialize_impl<options, value_type, buffer_type, serialize_context_type> {
		static constexpr uint64_t falseV{ 435728179558 };
		static constexpr uint64_t trueV{ 434025983730 };
		template<typename value_type_new> JSONIFIER_ALWAYS_INLINE static void impl(value_type_new&& value, buffer_type& buffer, serialize_context_type& serializePair) noexcept {
			if constexpr (options.checkedForSize) {
				auto additionalSize = 5 + serializePair.indent;
				if (buffer.size() < serializePair.index + additionalSize) {
					buffer.resize((serializePair.index + additionalSize) * 2);
				}
			}
			const uint64_t state = falseV - (value * trueV);
			std::memcpy(&buffer[serializePair.index], &state, 8);
			serializePair.index += 5 - value;
		}
	};

	template<jsonifier::serialize_options options, jsonifier::concepts::num_t value_type, jsonifier::concepts::buffer_like buffer_type, typename serialize_context_type>
	struct serialize_impl<options, value_type, buffer_type, serialize_context_type> {
		template<typename value_type_new> JSONIFIER_ALWAYS_INLINE static void impl(value_type_new&& value, buffer_type& buffer, serialize_context_type& serializePair) noexcept {
			if constexpr (options.checkedForSize) {
				auto additionalSize = 64 + serializePair.indent;
				if (buffer.size() < serializePair.index + additionalSize) {
					buffer.resize((serializePair.index + additionalSize) * 2);
				}
			}
			if constexpr (sizeof(value_type) == 8) {
				serializePair.index = static_cast<size_t>(to_chars<std::remove_cvref_t<value_type>>::impl(&buffer[serializePair.index], value) - buffer.data());
			} else {
				if constexpr (jsonifier::concepts::unsigned_type<value_type>) {
					serializePair.index = static_cast<size_t>(to_chars<uint64_t>::impl(&buffer[serializePair.index], static_cast<uint64_t>(value)) - buffer.data());
				} else if constexpr (jsonifier::concepts::signed_type<value_type>) {
					serializePair.index = static_cast<size_t>(to_chars<int64_t>::impl(&buffer[serializePair.index], static_cast<int64_t>(value)) - buffer.data());
				} else {
					serializePair.index = static_cast<size_t>(to_chars<double>::impl(&buffer[serializePair.index], static_cast<double>(value)) - buffer.data());
				}
			}
		}
	};

}