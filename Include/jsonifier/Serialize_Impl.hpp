
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

	static constexpr uint64_t falseVInt{ 435728179558 };
	static constexpr uint64_t trueVInt{ 434025983730 };

	template<jsonifier::serialize_options options, typename value_type> constexpr size_t getPaddingSize() noexcept {
		if constexpr (jsonifier::concepts::jsonifier_object_t<value_type>) {
			constexpr auto numMembers = tuple_size_v<core_tuple_type<value_type>>;
			constexpr auto newSize	  = []() constexpr {
				   size_t pair{};
				   constexpr auto sizeCollectLambda = [](const auto currentIndex, const auto maxIndex, auto& pairNew) {
					   if constexpr (currentIndex < maxIndex) {
						   constexpr auto subTuple	  = get<currentIndex>(jsonifier::core<std::remove_cvref_t<value_type>>::parseValue);
						   constexpr auto key		  = subTuple.view();
						   constexpr auto unQuotedKey = string_literal{ "\"" } + stringLiteralFromView<key.size()>(key);
						   constexpr auto quotedKey	  = unQuotedKey + string_literal{ "\": " };
						   pairNew += quotedKey.size();
						   if constexpr (currentIndex < maxIndex - 1) {
							   if constexpr (options.prettify) {
								   pairNew += std::size(",\n") + 1;
							   } else {
								   ++pairNew;
							   }
						   }
					   }
				   };

				   forEach<numMembers>(sizeCollectLambda, pair);

				   ++pair;
				   ++pair;

				   return pair;
			}();

			return newSize;
		} else if constexpr (jsonifier::concepts::bool_t<value_type>) {
			return 8;
		} else if constexpr (jsonifier::concepts::num_t<value_type>) {
			return 64;
		} else if constexpr (jsonifier::concepts::vector_t<value_type>) {
			return 4 + getPaddingSize<options, typename std::remove_cvref_t<value_type>::value_type>();
		} else if constexpr (jsonifier::concepts::tuple_t<value_type>) {
			constexpr auto accumulatePaddingSizesImpl = []<typename tuple, size_t... indices>(std::index_sequence<indices...>) noexcept {
				return (getPaddingSize<options, std::tuple_element_t<indices, tuple>>() + ... + 0);
			};
			constexpr auto accumulatePaddingSizes = []<typename tuple>(auto&& accumulatePaddingSizesImpl) {
				return accumulatePaddingSizesImpl.template operator()<tuple>(std::make_index_sequence<tuple_size_v<tuple>>{});
			};
			return accumulatePaddingSizes.template operator()<std::remove_cvref_t<value_type>>(accumulatePaddingSizesImpl);
		} else if constexpr (jsonifier::concepts::raw_array_t<value_type>) {
			return 4 + getPaddingSize<options, typename std::remove_cvref_t<value_type>::value_type>();
		} else if constexpr (jsonifier::concepts::pointer_t<value_type>) {
			return getPaddingSize<options, decltype(*std::remove_cvref_t<value_type>{})>();
		} else if constexpr (jsonifier::concepts::unique_ptr_t<value_type>) {
			return getPaddingSize<options, decltype(*std::remove_cvref_t<value_type>{})>();
		} else if constexpr (jsonifier::concepts::shared_ptr_t<value_type>) {
			return getPaddingSize<options, decltype(*std::remove_cvref_t<value_type>{})>();
		} else if constexpr (jsonifier::concepts::map_t<value_type>) {
			return 12 + getPaddingSize<options, typename std::remove_cvref_t<value_type>::mapped_type>() +
				getPaddingSize<options, typename std::remove_cvref_t<value_type>::key_type>();
		} else if constexpr (jsonifier::concepts::string_t<value_type>) {
			return 2;
		} else if constexpr (jsonifier::concepts::char_t<value_type>) {
			return 5;
		} else if constexpr (jsonifier::concepts::optional_t<value_type>) {
			return getPaddingSize<options, typename std::remove_cvref_t<value_type>::value_type>();
		} else if constexpr (jsonifier::concepts::always_null_t<value_type>) {
			return 8;
		} else {
			return {};
		}
	}

	JSONIFIER_ALWAYS_INLINE void writeValues(const char* src, char* dst, size_t length, size_t& index) {
		std::memcpy(dst, src, length);
		index += length;
	}

	template<jsonifier::serialize_options options, jsonifier::string_view key, typename context_type>
	JSONIFIER_ALWAYS_INLINE void writeObjectEntry(context_type& context) {
		static constexpr auto unQuotedKey = string_literal{ "\"" } + stringLiteralFromView<key.size()>(key);
		auto* dataPtr					  = context.buffer.data();
		if constexpr (options.prettify) {
			static constexpr auto quotedKey	   = unQuotedKey + string_literal{ "\": " };
			static constexpr auto size		   = quotedKey.size();
			static constexpr auto quotedKeyPtr = quotedKey.data();
			writeValues(quotedKeyPtr, dataPtr + context.index, size, context.index);
		} else {
			static constexpr auto quotedKey	   = unQuotedKey + string_literal{ "\":" };
			static constexpr auto size		   = quotedKey.size();
			static constexpr auto quotedKeyPtr = quotedKey.data();
			writeValues(quotedKeyPtr, dataPtr + context.index, size, context.index);
		}
	}

	template<jsonifier::serialize_options options, bool isItLast, typename context_type> JSONIFIER_ALWAYS_INLINE void writeObjectExit(context_type& context) {
		if constexpr (!isItLast) {
			if constexpr (options.prettify) {
				auto* dataPtr = context.buffer.data();
				static constexpr auto packedValues{ ",\n" };
				writeValues(packedValues, dataPtr + context.index, 2, context.index);
				std::memset(dataPtr + context.index, ' ', context.indent * options.indentSize);
				context.index += context.indent;
			} else {
				context.buffer[context.index] = comma;
				++context.index;
			}
		}
	}

	template<typename value_type, typename context_type, jsonifier::serialize_options options, auto tupleElem> struct serialize_types {
		JSONIFIER_ALWAYS_INLINE static void processIndex(const value_type& value, context_type& context) {
			static constexpr auto subTuple = tupleElem;
			static constexpr auto key	   = subTuple.view();
			if constexpr (jsonifier::concepts::has_excluded_keys<value_type>) {
				auto& keys = value.jsonifierExcludedKeys;
				if JSONIFIER_LIKELY (keys.find(static_cast<typename std::remove_reference_t<decltype(keys)>::key_type>(key)) != keys.end()) {
					return;
				}
			}
			writeObjectEntry<options, key>(context);
			serialize<options>::impl(subTuple.accessor(value), context);
			writeObjectExit<options, subTuple.isItLast>(context);
		}
	};

	template<typename value_type, typename context_type, jsonifier::serialize_options options, auto... values> struct serializer_map {
		JSONIFIER_CLANG_MACOS_ALWAYS_INLINE static void processIndicesForceInline(const value_type& value, context_type& context) {
			if constexpr (sizeof...(values) > 0) {
				(serialize_types<value_type, context_type, options, values>::processIndex(value, context), ...);
			}
		}
		JSONIFIER_INLINE static void processIndices(const value_type& value, context_type& context) {
			if constexpr (sizeof...(values) > 0) {
				(serialize_types<value_type, context_type, options, values>::processIndex(value, context), ...);
			}
		}
	};

	template<typename value_type, typename context_type, jsonifier::serialize_options options, typename index_sequence, auto tuple> struct get_serializer_base;

	template<typename value_type, typename context_type, jsonifier::serialize_options options, size_t... I, auto tuple>
	struct get_serializer_base<value_type, context_type, options, std::index_sequence<I...>, tuple> {
		using type = serializer_map<value_type, context_type, options, get<I>(tuple)...>;
	};

	template<typename value_type, typename context_type, jsonifier::serialize_options options, auto value> using serializer_base_t =
		typename get_serializer_base<value_type, context_type, options, tag_range<tuple_size_v<decltype(value)>>, value>::type;

	template<jsonifier::serialize_options options, typename context_type> struct object_val_serializer {
		static constexpr char packedValues01[]{ "{\n" };
		static constexpr char packedValues02[]{ ": " };
		static constexpr char packedValues03[]{ ",\n" };
		static constexpr char packedValues04[]{ "{}" };

		template<jsonifier::concepts::jsonifier_object_t value_type> JSONIFIER_ALWAYS_INLINE static void impl(value_type&& value, context_type& context) noexcept {
			static constexpr auto numMembers{ tuple_size_v<core_tuple_type<value_type>> };
			static constexpr auto paddingSize{ getPaddingSize<options, std::remove_cvref_t<value_type>>() };
			auto* dataPtr = context.buffer.data();
			if constexpr (numMembers > 0) {
				if constexpr (options.prettify) {
					const auto additionalSize = (paddingSize + (numMembers * context.indent));
					if (context.buffer.size() <= context.index + additionalSize) {
						context.buffer.resize((context.index + additionalSize) * 2);
						dataPtr = context.buffer.data();
					}
					context.indent += options.indentSize;
					std::memcpy(dataPtr + context.index, packedValues01, 2);
					context.index += 2;
					std::memset(dataPtr + context.index, options.indentChar, context.indent);
					context.index += context.indent;
				} else {
					if (context.buffer.size() <= context.index + paddingSize) {
						context.buffer.resize((context.index + paddingSize) * 2);
						dataPtr = context.buffer.data();
					}
					context.buffer[context.index] = lBrace;
					++context.index;
				}

				if constexpr (numMembers <= forceInlineLimitSerialize) {
					serializer_base_t<value_type, context_type, options, jsonifier::core<std::remove_cvref_t<value_type>>::parseValue>::processIndicesForceInline(value, context);
				} else {
					serializer_base_t<value_type, context_type, options, jsonifier::core<std::remove_cvref_t<value_type>>::parseValue>::processIndices(value, context);
				}

				if constexpr (options.prettify) {
					dataPtr = context.buffer.data();
					context.indent -= options.indentSize;
					context.buffer[context.index] = newline;
					++context.index;
					std::memset(dataPtr + context.index, options.indentChar, context.indent);
					context.index += context.indent;
				}
				context.buffer[context.index] = rBrace;
				++context.index;
			} else {
				std::memcpy(dataPtr + context.index, packedValues02, 2);
				context.index += 2;
			}
		}

		template<jsonifier::concepts::map_t value_type> JSONIFIER_ALWAYS_INLINE static void impl(value_type&& value, context_type& context) noexcept {
			const auto newSize = value.size();
			static constexpr auto paddingSize{ getPaddingSize<options, typename std::remove_cvref_t<value_type>::mapped_type>() };
			auto* dataPtr = context.buffer.data();
			if JSONIFIER_LIKELY (newSize > 0) {
				if constexpr (options.prettify) {
					const auto additionalSize = newSize * (paddingSize + context.indent);
					if (context.buffer.size() <= context.index + additionalSize) {
						context.buffer.resize((context.index + additionalSize) * 2);
						dataPtr = context.buffer.data();
					}
					context.indent += options.indentSize;
					std::memcpy(dataPtr + context.index, packedValues01, 2);
					context.index += 2;
					std::memset(dataPtr + context.index, options.indentChar, context.indent);
					context.index += context.indent;
				} else {
					const auto additionalSize = newSize * paddingSize;
					if (context.buffer.size() <= context.index + additionalSize) {
						context.buffer.resize((context.index + additionalSize) * 2);
						dataPtr = context.buffer.data();
					}
					context.buffer[context.index] = lBrace;
					++context.index;
				}
				auto iter = value.begin();
				serialize<options>::impl(iter->first, context);
				if constexpr (options.prettify) {
					dataPtr = context.buffer.data();
					std::memcpy(dataPtr + context.index, packedValues02, 2);
					context.index += 2;
				} else {
					context.buffer[context.index] = colon;
					++context.index;
				}
				serialize<options>::impl(iter->second, context);
				++iter;
				const auto end = value.end();
				for (; iter != end; ++iter) {
					if constexpr (options.prettify) {
						dataPtr = context.buffer.data();
						std::memcpy(dataPtr + context.index, packedValues03, 2);
						context.index += 2;
						std::memset(dataPtr + context.index, options.indentChar, context.indent);
						context.index += context.indent;
					} else {
						context.buffer[context.index] = comma;
						++context.index;
					}
					serialize<options>::impl(iter->first, context);
					if constexpr (options.prettify) {
						dataPtr = context.buffer.data();
						std::memcpy(dataPtr + context.index, packedValues02, 2);
						context.index += 2;
					} else {
						context.buffer[context.index] = colon;
						++context.index;
					}
					serialize<options>::impl(iter->second, context);
				}
				if constexpr (options.prettify) {
					dataPtr = context.buffer.data();
					context.indent -= options.indentSize;
					context.buffer[context.index] = newline;
					++context.index;
					std::memset(dataPtr + context.index, options.indentChar, context.indent);
					context.index += context.indent;
				}
				context.buffer[context.index] = rBrace;
				++context.index;
			} else {
				std::memcpy(dataPtr + context.index, packedValues04, 2);
				context.index += 2;
			}
		}

		template<jsonifier::concepts::tuple_t value_type> JSONIFIER_ALWAYS_INLINE static void impl(value_type&& value, context_type& context) noexcept {
			static constexpr auto additionalSize{ getPaddingSize<options, std::remove_cvref_t<value_type>>() };
			if (context.buffer.size() <= context.index + additionalSize) {
				context.buffer.resize((context.index + additionalSize) * 2);
			}
			static constexpr auto size = tuple_size_v<std::remove_reference_t<value_type>>;
			if constexpr (size > 0) {
				context.buffer[context.index] = lBracket;
				++context.index;
				if constexpr (options.prettify) {
					context.indent += options.indentSize;
					context.buffer[context.index] = newline;
					++context.index;
					std::memset(context.buffer.data() + context.index, options.indentChar, context.indent);
					context.index += context.indent;
				}
				serializeObjects<0, size>(value, context);
				if constexpr (options.prettify) {
					context.indent -= options.indentSize;
					context.buffer[context.index] = newline;
					++context.index;
					std::memset(context.buffer.data() + context.index, options.indentChar, context.indent);
					context.index += context.indent;
				}
				context.buffer[context.index] = rBracket;
				++context.index;
			} else {
				std::memcpy(context.buffer.data() + context.index, packedValues01, 2);
				context.index += 2;
			}
		}

		template<size_t currentIndex, size_t newSize, jsonifier::concepts::tuple_t value_type>
		JSONIFIER_ALWAYS_INLINE static void serializeObjects(value_type&& value, context_type& context) noexcept {
			if constexpr (currentIndex < newSize) {
				auto subTuple = get<currentIndex>(value);
				serialize<options>::impl(subTuple, context);
				if constexpr (currentIndex < newSize - 1) {
					if constexpr (options.prettify) {
						std::memcpy(context.buffer.data() + context.index, ",\n", 2);
						context.index += 2;
						std::memset(context.buffer.data() + context.index, options.indentChar, context.indent * options.indentSize);
						context.index += context.indent;
					} else {
						context.buffer[context.index] = comma;
						++context.index;
					}
				}
				return serializeObjects<currentIndex + 1, newSize>(value, context);
			}
		}
	};

	template<jsonifier::serialize_options options, typename context_type> struct array_val_serializer {
		static constexpr char packedValues01[]{ "[\n" };
		static constexpr char packedValues02[]{ ",\n" };
		static constexpr char packedValues03[]{ "[]" };
		template<jsonifier::concepts::vector_t value_type> JSONIFIER_ALWAYS_INLINE static void impl(value_type&& value, context_type& context) noexcept {
			const auto newSize = value.size();
			static constexpr auto paddingSize{ getPaddingSize<options, typename std::remove_cvref_t<value_type>::value_type>() };
			auto* dataPtr = context.buffer.data();
			if JSONIFIER_LIKELY (newSize > 0) {
				if constexpr (options.prettify) {
					const auto additionalSize = newSize * (paddingSize + context.indent);
					if (context.buffer.size() <= context.index + additionalSize) {
						context.buffer.resize((context.index + additionalSize) * 2);
						dataPtr = context.buffer.data();
					}
					context.indent += options.indentSize;
					std::memcpy(dataPtr + context.index, packedValues01, 2);
					context.index += 2;
					std::memset(dataPtr + context.index, options.indentChar, context.indent);
					context.index += context.indent;
				} else {
					const auto additionalSize = newSize * paddingSize;
					if (context.buffer.size() <= context.index + additionalSize) {
						context.buffer.resize((context.index + additionalSize) * 2);
					}
					context.buffer[context.index] = lBracket;
					++context.index;
				}
				auto iter = std::begin(value);
				serialize<options>::impl(*iter, context);
				++iter;
				const auto end = std::end(value);
				for (; iter != end; ++iter) {
					if constexpr (options.prettify) {
						dataPtr = context.buffer.data();
						std::memcpy(dataPtr + context.index, packedValues02, 2);
						context.index += 2;
						std::memset(dataPtr + context.index, options.indentChar, context.indent);
						context.index += context.indent;
					} else {
						context.buffer[context.index] = comma;
						++context.index;
					}
					serialize<options>::impl(*iter, context);
				}
				if constexpr (options.prettify) {
					dataPtr = context.buffer.data();
					context.indent -= options.indentSize;
					context.buffer[context.index] = newline;
					++context.index;
					std::memset(dataPtr + context.index, options.indentChar, context.indent);
					context.index += context.indent;
				}
				context.buffer[context.index] = rBracket;
				++context.index;
			} else {
				std::memcpy(dataPtr + context.index, packedValues03, 2);
				context.index += 2;
			}
		}

		template<template<typename, size_t> typename value_type, typename value_type_internal, size_t size>
		JSONIFIER_ALWAYS_INLINE static void impl(const value_type<value_type_internal, size>& value, context_type& context) noexcept {
			constexpr auto newSize = size;
			static constexpr auto paddingSize{ getPaddingSize<options, typename std::remove_cvref_t<value_type<value_type_internal, size>>::value_type>() };
			if constexpr (newSize > 0) {
				if constexpr (options.prettify) {
					const auto additionalSize = newSize * (paddingSize + context.indent);
					if (context.buffer.size() <= context.index + additionalSize) {
						context.buffer.resize((context.index + additionalSize) * 2);
					}
					context.indent += options.indentSize;
					std::memcpy(context.buffer.data() + context.index, packedValues01, 2);
					context.index += 2;
					std::memset(context.buffer.data() + context.index, options.indentChar, context.indent);
					context.index += context.indent;
				} else {
					const auto additionalSize = newSize * paddingSize;
					if (context.buffer.size() <= context.index + additionalSize) {
						context.buffer.resize((context.index + additionalSize) * 2);
					}
					context.buffer[context.index] = lBracket;
					++context.index;
				}
				auto iter = std::begin(value);
				serialize<options>::impl(*iter, context);
				++iter;
				const auto end = std::end(value);
				for (; iter != end; ++iter) {
					if constexpr (options.prettify) {
						std::memcpy(context.buffer.data() + context.index, packedValues02, 2);
						context.index += 2;
						std::memset(context.buffer.data() + context.index, options.indentChar, context.indent);
						context.index += context.indent;
					} else {
						context.buffer[context.index] = comma;
						++context.index;
					}
					serialize<options>::impl(*iter, context);
				}
				if constexpr (options.prettify) {
					context.indent -= options.indentSize;
					context.buffer[context.index] = newline;
					++context.index;
					std::memset(context.buffer.data() + context.index, options.indentChar, context.indent);
					context.index += context.indent;
				}
				context.buffer[context.index] = rBracket;
				++context.index;
			} else {
				std::memcpy(context.buffer.data() + context.index, packedValues03, 2);
				context.index += 2;
			}
		}
	};

	template<jsonifier::serialize_options options, typename context_type> struct string_val_serializer {
		static constexpr char packedValues01[]{ "\"\"" };

		template<jsonifier::concepts::string_t value_type> JSONIFIER_ALWAYS_INLINE static void impl(value_type&& value, context_type& context) noexcept {
			const auto newSize = value.size();
			static constexpr auto paddingSize{ getPaddingSize<options, typename std::remove_cvref_t<value_type>::value_type>() };
			if (newSize > 0) {
				if constexpr (options.prettify) {
					const auto additionalSize = newSize * (paddingSize + context.indent);
					if (context.buffer.size() <= context.index + additionalSize) {
						context.buffer.resize((context.index + additionalSize) * 2);
					}
				} else {
					const auto additionalSize = newSize * paddingSize;
					if (context.buffer.size() <= context.index + additionalSize) {
						context.buffer.resize((context.index + additionalSize) * 2);
					}
				}
				context.buffer[context.index] = quote;
				++context.index;
				auto newPtr					  = string_serializer<options, decltype(value.data()), decltype(context.buffer.data() + context.index)>::impl(value.data(),
									  context.buffer.data() + context.index, value.size());
				context.index				  = static_cast<size_t>(newPtr - context.buffer.data());
				context.buffer[context.index] = quote;
				++context.index;
			} else {
				std::memcpy(context.buffer.data() + context.index, packedValues01, 2);
				context.index += 2;
			}
		}

		template<jsonifier::concepts::char_t value_type> JSONIFIER_ALWAYS_INLINE static void impl(value_type&& value, context_type& context) noexcept {
			context.buffer[context.index] = quote;
			++context.index;
			switch (value) {
				[[unlikely]] case '\b': {
					std::memcpy(context.buffer.data() + context.index, R"(\b)", 2);
					context.index += 2;
					break;
				}
				[[unlikely]] case '\t': {
					std::memcpy(context.buffer.data() + context.index, R"(\t)", 2);
					context.index += 2;
					break;
				}
				[[unlikely]] case '\n': {
					std::memcpy(context.buffer.data() + context.index, R"(\n)", 2);
					context.index += 2;
					break;
				}
				[[unlikely]] case '\f': {
					std::memcpy(context.buffer.data() + context.index, R"(\f)", 2);
					context.index += 2;
					break;
				}
				[[unlikely]] case '\r': {
					std::memcpy(context.buffer.data() + context.index, R"(\r)", 2);
					context.index += 2;
					break;
				}
				[[unlikely]] case '"': {
					std::memcpy(context.buffer.data() + context.index, R"(\")", 2);
					context.index += 2;
					break;
				}
				[[unlikely]] case '\\': {
					std::memcpy(context.buffer.data() + context.index, R"(\\)", 2);
					context.index += 2;
					break;
				}
				[[likely]] default: {
					context.buffer[context.index] = value;
					++context.index;
				}
			}
			context.buffer[context.index] = quote;
			++context.index;
		}
	};

	template<jsonifier::serialize_options options, typename context_type> struct number_val_serializer {
		template<jsonifier::concepts::enum_t value_type> JSONIFIER_ALWAYS_INLINE static void impl(value_type&& value, context_type& context) noexcept {
			int64_t valueNew{ static_cast<int64_t>(value) };
			serialize<options>::impl(valueNew, context);
		}

		template<jsonifier::concepts::num_t value_type> JSONIFIER_ALWAYS_INLINE static void impl(value_type&& value, context_type& context) noexcept {
			auto* dataPtr = context.buffer.data();
			if constexpr (sizeof(value_type) == 8) {
				context.index = static_cast<size_t>(toChars<std::remove_cvref_t<value_type>>(dataPtr + context.index, value) - dataPtr);
			} else {
				if constexpr (jsonifier::concepts::unsigned_t<value_type>) {
					context.index = static_cast<size_t>(toChars<uint64_t>(dataPtr + context.index, static_cast<uint64_t>(value)) - dataPtr);
				} else if constexpr (jsonifier::concepts::signed_t<value_type>) {
					context.index = static_cast<size_t>(toChars<int64_t>(dataPtr + context.index, static_cast<int64_t>(value)) - dataPtr);
				} else {
					context.index = static_cast<size_t>(toChars<float>(dataPtr + context.index, value) - dataPtr);
				}
			}
		}
	};

	template<jsonifier::serialize_options options, typename context_type> struct bool_val_serializer {
		template<jsonifier::concepts::bool_t value_type> JSONIFIER_ALWAYS_INLINE static void impl(value_type&& value, context_type& context) noexcept {
			const uint64_t state = falseVInt - (value * trueVInt);
			std::memcpy(context.buffer.data() + context.index, &state, 5);
			context.index += 5 - value;
		}
	};

	template<jsonifier::serialize_options options, typename context_type> struct null_val_serializer {
		template<jsonifier::concepts::always_null_t value_type> JSONIFIER_ALWAYS_INLINE static void impl(value_type&&, context_type& context) noexcept {
			std::memcpy(context.buffer.data() + context.index, nullV, 4);
			context.index += 4;
		}
	};

	template<jsonifier::serialize_options options, typename context_type> struct accessor_val_serializer {
		template<jsonifier::concepts::pointer_t value_type> JSONIFIER_ALWAYS_INLINE static void impl(value_type&& value, context_type& context) noexcept {
			if (value) {
				serialize<options>::impl(*value, context);
			} else {
				std::memcpy(context.buffer.data() + context.index, nullV, 4);
				context.index += 4;
			}
		}

		template<jsonifier::concepts::raw_json_t value_type> JSONIFIER_ALWAYS_INLINE static void impl(value_type&& value, context_type& context) noexcept {
			serialize<options>::impl(value.rawJson(), context);
		}

		template<jsonifier::concepts::skip_t value_type> JSONIFIER_ALWAYS_INLINE static void impl(value_type&& value, context_type& context) noexcept {
			std::memcpy(context.buffer.data() + context.index, nullV, 4);
			context.index += 4;
		}

		template<jsonifier::concepts::unique_ptr_t value_type> JSONIFIER_ALWAYS_INLINE static void impl(value_type&& value, context_type& context) noexcept {
			if JSONIFIER_LIKELY (value) {
				serialize<options>::impl(*value, context);
			} else {
				std::memcpy(context.buffer.data() + context.index, nullV, 4);
				context.index += 4;
			}
		}

		template<jsonifier::concepts::shared_ptr_t value_type> JSONIFIER_ALWAYS_INLINE static void impl(value_type&& value, context_type& context) noexcept {
			if JSONIFIER_LIKELY (value) {
				serialize<options>::impl(*value, context);
			} else {
				std::memcpy(context.buffer.data() + context.index, nullV, 4);
				context.index += 4;
			}
		}

		template<jsonifier::concepts::variant_t value_type> JSONIFIER_ALWAYS_INLINE static void impl(value_type&& value, context_type& context) noexcept {
			static constexpr auto lambda = [](auto&& valueNewer, auto&& contextNew) {
				serialize<options>::impl(valueNewer, contextNew);
			};
			visit<lambda>(value, context);
		}

		template<jsonifier::concepts::optional_t value_type> JSONIFIER_ALWAYS_INLINE static void impl(value_type&& value, context_type& context) noexcept {
			if JSONIFIER_LIKELY (value) {
				serialize<options>::impl(*value, context);
			} else {
				std::memcpy(context.buffer.data() + context.index, nullV, 4);
				context.index += 4;
			}
		}
	};
}