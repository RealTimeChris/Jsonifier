
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
#include <expected>
#include <algorithm>
#include <assert.h>

namespace jsonifier_internal {

	static constexpr uint64_t falseV{ 435728179558 };
	static constexpr uint64_t trueV{ 434025983730 };
	static constexpr uint32_t nullV{ 1819047278 };

	struct serialize_size_values {
		size_t newLineCount{};
		size_t ctIndex{};
	};

	template<class value_type> constexpr size_t getPaddingSize() noexcept {
		if constexpr (jsonifier::concepts::bool_t<value_type>) {
			return 8;
		} else if constexpr (jsonifier::concepts::num_t<value_type>) {
			return 64;
		} else if constexpr (jsonifier::concepts::vector_t<value_type>) {
			return 4;
		} else if constexpr (jsonifier::concepts::map_t<value_type>) {
			return 8;
		} else if constexpr (jsonifier::concepts::string_t<value_type>) {
			return 2;
		} else if constexpr (jsonifier::concepts::char_t<value_type>) {
			return 5;
		} else if constexpr (jsonifier::concepts::optional_t<value_type>) {
			return getPaddingSize<typename std::remove_cvref_t<value_type>::value_type>();
		} else if constexpr (jsonifier::concepts::always_null_t<value_type>) {
			return 8;
		} else {
			return {};
		}
	}

	template<jsonifier::serialize_options options, typename value_type, typename size_collect_context_type> struct size_collect_impl;

	template<jsonifier::serialize_options options, jsonifier::concepts::jsonifier_value_t value_type, typename size_collect_context_type>
	struct size_collect_impl<options, value_type, size_collect_context_type> {
		constexpr static size_t impl() noexcept {
			constexpr auto numMembers = std::tuple_size_v<core_tuple_t<value_type>>;
			constexpr auto newSize	  = []() constexpr {
				   serialize_size_values pair{};
				   constexpr auto sizeCollectLambda = [](const auto currentIndex, const auto newSize, auto& pairNew) {
					   if constexpr (currentIndex < newSize) {
						   constexpr auto subTuple = std::get<currentIndex>(jsonifier::concepts::coreV<value_type>);
						   constexpr auto key	   = subTuple.view();
						   using member_type	   = typename std::remove_cvref_t<decltype(subTuple)>::member_type;
						   pairNew.ctIndex += getPaddingSize<member_type>();
						   constexpr auto unQuotedKey = string_literal{ "\"" } + stringLiteralFromView<key.size()>(key);
						   constexpr auto quotedKey	  = unQuotedKey + string_literal{ "\": " };
						   pairNew.ctIndex += quotedKey.size();
						   if constexpr (currentIndex < newSize - 1) {
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

			return newSize.ctIndex;
		}
	};

#define if_1(n, numMembers) \
	if constexpr (n < numMembers) { \
		functionPtrsSerialize<options, value_type, buffer_type, serialize_context_type>[n](std::forward<value_type_new>(value), buffer, serializePair); \
	}

#define if_10(n, numMembers) \
if_1(n + 0, numMembers) \
if_1(n + 1, numMembers) \
if_1(n + 2, numMembers) \
if_1(n + 3, numMembers) \
if_1(n + 4, numMembers) \
if_1(n + 5, numMembers) \
if_1(n + 6, numMembers) \
if_1(n + 7, numMembers) \
if_1(n + 8, numMembers) \
if_1(n + 9, numMembers)

	template<jsonifier::serialize_options options, jsonifier::concepts::vector_t value_type, typename size_collect_context_type>
	struct size_collect_impl<options, value_type, size_collect_context_type> {
		JSONIFIER_ALWAYS_INLINE static size_t impl(size_t size) noexcept {
			static constexpr auto paddingSize{ getPaddingSize<typename std::remove_cvref_t<value_type>::value_type>() };
			return size * paddingSize;
		}
	};

	template<jsonifier::serialize_options options, jsonifier::concepts::raw_array_t value_type, typename size_collect_context_type>
	struct size_collect_impl<options, value_type, size_collect_context_type> {
		JSONIFIER_ALWAYS_INLINE static size_t impl(size_t size) noexcept {
			static constexpr auto paddingSize{ getPaddingSize<typename std::remove_cvref_t<value_type>::value_type>() };
			return size * paddingSize;
		}
	};

	template<jsonifier::serialize_options options, jsonifier::concepts::map_t value_type, typename size_collect_context_type>
	struct size_collect_impl<options, value_type, size_collect_context_type> {
		JSONIFIER_ALWAYS_INLINE static size_t impl(size_t size) noexcept {
			static constexpr auto paddingSize{ getPaddingSize<typename std::remove_cvref_t<value_type>::mapped_type>() };
			return size * paddingSize;
		}
	};

	template<jsonifier::serialize_options options, jsonifier::concepts::string_t value_type, typename size_collect_context_type>
	struct size_collect_impl<options, value_type, size_collect_context_type> {
		JSONIFIER_ALWAYS_INLINE static size_t impl(size_t size) noexcept {
			return (size * 2);
		}
	};

	template<jsonifier::serialize_options options, size_t newSize> struct index_processor_serialize {
		template<size_t currentIndex, typename value_type, typename buffer_type, typename serialize_context_type>
		JSONIFIER_NON_GCC_ALWAYS_INLINE static void processIndex(const value_type& value, buffer_type& buffer, serialize_context_type& serializePair) noexcept {
			if constexpr (currentIndex < newSize) {
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
					static constexpr auto packedValues{ packValues<quotedKey>() };
					std::memcpy(&buffer[serializePair.index], &packedValues, size);
					serializePair.index += size;
				} else {
					static constexpr auto quotedKey = unQuotedKey + string_literal{ "\":" };
					static constexpr auto size		= quotedKey.size();
					static constexpr auto packedValues{ packValues<quotedKey>() };
					std::memcpy(&buffer[serializePair.index], &packedValues, size);
					serializePair.index += size;
				}

				serialize<options>::impl(value.*memberPtr, buffer, serializePair);
				if constexpr (currentIndex < newSize - 1) {
					if constexpr (options.prettify) {
						static constexpr auto packedValues{ packValues<",\n">() };
						std::memcpy(&buffer[serializePair.index], &packedValues, 2);
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
	constexpr auto generateFunctionPtrsImpl(std::index_sequence<indices...>) noexcept {
		using function_type = decltype(&index_processor_serialize<options, sizeof...(indices)>::template processIndex<0, value_type, buffer_type, serialize_context_type>);
		return std::array<function_type, sizeof...(indices)>{
			{ &index_processor_serialize<options, sizeof...(indices)>::template processIndex<indices, value_type, buffer_type, serialize_context_type>... }
		};
	}

	template<jsonifier::serialize_options options, typename value_type, typename buffer_type, typename serialize_context_type> constexpr auto generateFunctionPtrs() noexcept {
		constexpr auto tupleSize = std::tuple_size_v<core_tuple_t<value_type>>;
		return generateFunctionPtrsImpl<options, value_type, buffer_type, serialize_context_type>(std::make_index_sequence<tupleSize>{});
	}

	template<jsonifier::serialize_options options, typename value_type, typename buffer_type, typename serialize_context_type>
	static constexpr auto functionPtrsSerialize{ generateFunctionPtrs<options, value_type, buffer_type, serialize_context_type>() };

	template<jsonifier::serialize_options options, jsonifier::concepts::jsonifier_value_t value_type, jsonifier::concepts::buffer_like buffer_type, typename serialize_context_type>
	struct serialize_impl<options, value_type, buffer_type, serialize_context_type> {
		static constexpr auto packedValues01{ packValues<"{\n">() };
		static constexpr auto packedValues02{ packValues<"{}">() };

		template<typename value_type_new> JSONIFIER_ALWAYS_INLINE static void impl(value_type_new&& value, buffer_type& buffer, serialize_context_type& serializePair) noexcept {
			static constexpr auto numMembers{ std::tuple_size_v<core_tuple_t<value_type>> };
			if constexpr (numMembers > 0) {
				static constexpr auto additionalSize = size_collect_impl<options, value_type, std::remove_cvref_t<serialize_context_type>>::impl();
				if (buffer.size() <= serializePair.index + additionalSize) {
					buffer.resize((serializePair.index + additionalSize) * 2);
				}
				if constexpr (options.prettify) {
					serializePair.indent += options.indentSize;
					std::memcpy(&buffer[serializePair.index], &packedValues01, 2);
					serializePair.index += 2;
					std::memset(&buffer[serializePair.index], options.indentChar, serializePair.indent);
					serializePair.index += serializePair.indent;
				} else {
					buffer[serializePair.index] = '{';
					++serializePair.index;
				}

				if_10(0, numMembers);
				if_10(10, numMembers);
				if_10(20, numMembers);
				if_10(30, numMembers);
				if_10(40, numMembers);
				if_10(50, numMembers);
				if_10(60, numMembers);
				if_10(70, numMembers);
				if_10(80, numMembers);
				if_10(90, numMembers);
				if_10(100, numMembers);
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
				std::memcpy(&buffer[serializePair.index], &packedValues02, 2);
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
		static constexpr auto packedValues01{ packValues<"{\n">() };
		static constexpr auto packedValues02{ packValues<": ">() };
		static constexpr auto packedValues03{ packValues<",\n">() };
		static constexpr auto packedValues04{ packValues<"{}">() };
		template<typename value_type_new> JSONIFIER_ALWAYS_INLINE static void impl(value_type_new&& value, buffer_type& buffer, serialize_context_type& serializePair) noexcept {
			const auto newSize = value.size();
			if JSONIFIER_LIKELY ((newSize > 0)) {
				const auto additionalSize = size_collect_impl<options, value_type, std::remove_cvref_t<serialize_context_type>>::impl(newSize);
				if (buffer.size() <= serializePair.index + additionalSize) {
					buffer.resize((serializePair.index + additionalSize) * 2);
				}
				if constexpr (options.prettify) {
					serializePair.indent += options.indentSize;
					std::memcpy(&buffer[serializePair.index], &packedValues01, 2);
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
					std::memcpy(&buffer[serializePair.index], &packedValues02, 2);
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
						std::memcpy(&buffer[serializePair.index], &packedValues03, 2);
						serializePair.index += 2;
						std::memset(&buffer[serializePair.index], options.indentChar, serializePair.indent);
						serializePair.index += serializePair.indent;
					} else {
						buffer[serializePair.index] = ',';
						++serializePair.index;
					}
					serialize<options>::impl(iter->first, buffer, serializePair);
					if constexpr (options.prettify) {
						std::memcpy(&buffer[serializePair.index], &packedValues02, 2);
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
				std::memcpy(&buffer[serializePair.index], &packedValues04, 2);
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

		template<size_t currentIndex, size_t newSize, typename value_type_new>
		JSONIFIER_ALWAYS_INLINE static void serializeObjects(value_type_new&& value, buffer_type& buffer, serialize_context_type& serializePair) noexcept {
			if constexpr (currentIndex < newSize) {
				auto subTuple = std::get<currentIndex>(value);
				serialize<options>::impl(subTuple, buffer, serializePair);
				if constexpr (currentIndex < newSize - 1) {
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
				return serializeObjects<currentIndex + 1, newSize>(value, buffer, serializePair);
			}
		}
	};

	template<jsonifier::serialize_options options, jsonifier::concepts::vector_t value_type, jsonifier::concepts::buffer_like buffer_type, typename serialize_context_type>
	struct serialize_impl<options, value_type, buffer_type, serialize_context_type> {
		static constexpr auto packedValues01{ packValues<"[\n">() };
		static constexpr auto packedValues02{ packValues<",\n">() };
		static constexpr auto packedValues03{ packValues<"[]">() };
		template<typename value_type_new> JSONIFIER_ALWAYS_INLINE static void impl(value_type_new&& value, buffer_type& buffer, serialize_context_type& serializePair) noexcept {
			const auto newSize = value.size();
			if JSONIFIER_LIKELY ((newSize > 0)) {
				const auto additionalSize = size_collect_impl<options, value_type, std::remove_cvref_t<serialize_context_type>>::impl(newSize);
				if (buffer.size() <= serializePair.index + additionalSize) {
					buffer.resize((serializePair.index + additionalSize) * 2);
				}
				if constexpr (options.prettify) {
					serializePair.indent += options.indentSize;
					std::memcpy(&buffer[serializePair.index], &packedValues01, 2);
					serializePair.index += 2;
					std::memset(&buffer[serializePair.index], options.indentChar, serializePair.indent);
					serializePair.index += serializePair.indent;
				} else {
					buffer[serializePair.index] = '[';
					++serializePair.index;
				}
				auto iter = value.begin();
				serialize<options>::impl(*iter, buffer, serializePair);
				++iter;
				const auto end = value.end();
				for (; iter != end; ++iter) {
					if constexpr (options.prettify) {
						std::memcpy(&buffer[serializePair.index], &packedValues02, 2);
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
				std::memcpy(&buffer[serializePair.index], &packedValues03, 2);
				serializePair.index += 2;
			}
		}
	};

	template<jsonifier::serialize_options options, jsonifier::concepts::pointer_t value_type, jsonifier::concepts::buffer_like buffer_type, typename serialize_context_type>
	struct serialize_impl<options, value_type, buffer_type, serialize_context_type> {
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
		static constexpr auto packedValues01{ packValues<"[\n">() };
		static constexpr auto packedValues02{ packValues<",\n">() };
		static constexpr auto packedValues03{ packValues<"[]">() };
		template<template<typename, size_t> typename value_type_new, typename value_type_internal, size_t size>
		JSONIFIER_ALWAYS_INLINE static void impl(value_type_new<value_type_internal, size>& value, buffer_type& buffer, serialize_context_type& serializePair) noexcept {
			const auto newSize = std::size(value);
			if JSONIFIER_LIKELY ((newSize > 0)) {
				const auto additionalSize = size_collect_impl<options, value_type, std::remove_cvref_t<serialize_context_type>>::impl(newSize);
				if (buffer.size() <= serializePair.index + additionalSize) {
					buffer.resize((serializePair.index + additionalSize) * 2);
				}
				if constexpr (options.prettify) {
					serializePair.indent += options.indentSize;
					std::memcpy(&buffer[serializePair.index], &packedValues01, 2);
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
						std::memcpy(&buffer[serializePair.index], &packedValues02, 2);
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
				std::memcpy(&buffer[serializePair.index], &packedValues03, 2);
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
		static constexpr auto packedValues01{ packValues<"\"\"">() };
		template<typename value_type_new> JSONIFIER_ALWAYS_INLINE static void impl(value_type_new&& value, buffer_type& buffer, serialize_context_type& serializePair) noexcept {
			const auto newSize = value.size();
			if (newSize > 0) {
				const auto additionalSize = size_collect_impl<options, value_type, std::remove_cvref_t<serialize_context_type>>::impl(newSize);
				if (buffer.size() <= serializePair.index + additionalSize) {
					buffer.resize((serializePair.index + additionalSize) * 2);
				}
				buffer[serializePair.index] = '"';
				++serializePair.index;
				auto newPtr = string_serializer<decltype(value.data()), decltype(&buffer[serializePair.index])>::impl(value.data(), &buffer[serializePair.index], value.size());
				serializePair.index			= static_cast<size_t>(newPtr - buffer.data());
				buffer[serializePair.index] = '"';
				++serializePair.index;
			} else {
				std::memcpy(&buffer[serializePair.index], &packedValues01, 2);
				serializePair.index += 2;
			}
		}
	};

	template<jsonifier::serialize_options options, jsonifier::concepts::char_t value_type, jsonifier::concepts::buffer_like buffer_type, typename serialize_context_type>
	struct serialize_impl<options, value_type, buffer_type, serialize_context_type> {
		template<typename value_type_new> JSONIFIER_ALWAYS_INLINE static void impl(value_type_new&& value, buffer_type& buffer, serialize_context_type& serializePair) noexcept {
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
					[[likely]] default : {
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
		template<typename value_type_new> JSONIFIER_ALWAYS_INLINE static void impl(value_type_new&&, buffer_type& buffer, serialize_context_type& serializePair) noexcept {
			std::memcpy(&buffer[serializePair.index], &nullV, 4);
			serializePair.index += 4;
		}
	};

	template<jsonifier::serialize_options options, jsonifier::concepts::bool_t value_type, jsonifier::concepts::buffer_like buffer_type, typename serialize_context_type>
	struct serialize_impl<options, value_type, buffer_type, serialize_context_type> {
		template<typename value_type_new> JSONIFIER_ALWAYS_INLINE static void impl(value_type_new&& value, buffer_type& buffer, serialize_context_type& serializePair) noexcept {
			const uint64_t state = falseV - (value * trueV);
			std::memcpy(&buffer[serializePair.index], &state, 5);
			serializePair.index += 5 - value;
		}
	};

	template<jsonifier::serialize_options options, jsonifier::concepts::num_t value_type, jsonifier::concepts::buffer_like buffer_type, typename serialize_context_type>
	struct serialize_impl<options, value_type, buffer_type, serialize_context_type> {
		template<typename value_type_new> JSONIFIER_ALWAYS_INLINE static void impl(value_type_new&& value, buffer_type& buffer, serialize_context_type& serializePair) noexcept {
			if constexpr (sizeof(value_type) == 8) {
				serializePair.index = static_cast<size_t>(toChars<std::remove_cvref_t<value_type>>(&buffer[serializePair.index], value) - buffer.data());
			} else {
				if constexpr (jsonifier::concepts::unsigned_type<value_type>) {
					serializePair.index = static_cast<size_t>(toChars<uint64_t>(&buffer[serializePair.index], static_cast<uint64_t>(value)) - buffer.data());
				} else if constexpr (jsonifier::concepts::signed_type<value_type>) {
					serializePair.index = static_cast<size_t>(toChars<int64_t>(&buffer[serializePair.index], static_cast<int64_t>(value)) - buffer.data());
				} else {
					serializePair.index = static_cast<size_t>(toChars<float>(&buffer[serializePair.index], value) - buffer.data());
				}
			}
		}
	};

}