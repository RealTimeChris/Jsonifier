
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
#include <jsonifier/JsonEntity.hpp>
#include <expected>
#include <algorithm>
#include <assert.h>

DEFINE_ENTITY_HAS_VALUE_CONCEPT(jsonEntity, forceInline)

namespace jsonifier_internal {

	static constexpr uint64_t falseVInt{ 435728179558 };
	static constexpr uint64_t trueVInt{ 434025983730 };

	template<jsonifier::serialize_options options, typename value_type> constexpr size_t getPaddingSize() noexcept {
		if constexpr (jsonifier::concepts::jsonifier_object_t<value_type>) {
			constexpr auto memberCount = tuple_size_v<core_tuple_type<value_type>>;
			constexpr auto newSize	   = []() constexpr {
				size_t pair{};
				constexpr auto sizeCollectLambda = [](const auto currentIndex, const auto maxIndex, auto& pairNew) {
					if constexpr (currentIndex < maxIndex) {
						constexpr auto subTuple	   = jsonifier_internal::get<currentIndex>(jsonifier::core<std::remove_cvref_t<value_type>>::parseValue);
						constexpr auto key		   = subTuple.name;
						constexpr auto unQuotedKey = string_literal{ "\"" } + key;
						constexpr auto quotedKey   = unQuotedKey + string_literal{ "\": " };
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

				forEach<memberCount>(sizeCollectLambda, pair);

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

	JSONIFIER_FORCE_INLINE void writeValues(const char* src, char*& dst, size_t length) {
		std::memcpy(dst, src, length);
		dst += length;
	}

	template<jsonifier::serialize_options options, jsonifier::string_view key, typename context_type> JSONIFIER_FORCE_INLINE void writeObjectEntry(context_type& context) {
		static constexpr auto unQuotedKey = string_literal{ "\"" } + stringLiteralFromView<key.size()>(key);
		if constexpr (options.prettify) {
			static constexpr auto quotedKey	   = unQuotedKey + string_literal{ "\": " };
			static constexpr auto size		   = quotedKey.size();
			static constexpr auto quotedKeyPtr = quotedKey.data();
			writeValues(quotedKeyPtr, context.bufferPtr, size);
		} else {
			static constexpr auto quotedKey	   = unQuotedKey + string_literal{ "\":" };
			static constexpr auto size		   = quotedKey.size();
			static constexpr auto quotedKeyPtr = quotedKey.data();
			writeValues(quotedKeyPtr, context.bufferPtr, size);
		}
	}

	template<jsonifier::serialize_options options, bool isItLast, typename context_type> JSONIFIER_FORCE_INLINE void writeObjectExit(context_type& context) {
		if constexpr (!isItLast) {
			if constexpr (options.prettify) {
				static constexpr auto packedValues{ ",\n" };
				writeValues(packedValues, context.bufferPtr, 2);
				std::memset(context.bufferPtr, ' ', context.indent * options.indentSize);
				context.bufferPtr += context.indent;
			} else {
				*context.bufferPtr = comma;
				++context.bufferPtr;
			}
		}
	}

	template<jsonifier::serialize_options options, size_t depth, size_t maxDepth, typename... bases> struct serialize_entities : public bases... {

		template<typename json_entity_type, typename value_type, typename context_type>
		JSONIFIER_INLINE void processIndexCall(value_type& value, context_type& context) const noexcept {
			return processIndex<json_entity_type, value_type, context_type>(value, context);
		}

		template<typename json_entity_type, typename value_type, typename context_type>
		JSONIFIER_FORCE_INLINE static void processIndex(const value_type& value, context_type& context) noexcept {
			static constexpr json_entity_type jsonEntity{};
			static constexpr auto key		= jsonEntity.name.template view<jsonifier::string_view>();
			static constexpr auto memberPtr = jsonEntity.memberPtr;
			if constexpr (jsonifier::concepts::has_excluded_keys<value_type>) {
				auto& keys = value.jsonifierExcludedKeys;
				if JSONIFIER_LIKELY (keys.find(static_cast<typename std::remove_reference_t<decltype(keys)>::key_type>(key)) != keys.end()) {
					return;
				}
			}
			jsonifier_internal::writeObjectEntry<options, key>(context);
			serialize<options, jsonEntity, depth, maxDepth>::impl(value.*memberPtr, context);
			jsonifier_internal::writeObjectExit<options, jsonEntity.isItLast>(context);
		}

		template<typename... arg_types> JSONIFIER_FORCE_INLINE void iterateValues(
			arg_types&&... args) const {
			((processIndexCall<bases>(jsonifier_internal::forward<arg_types>(args)...)), ...);
		}
	};

	template<auto options, typename serialize_entity_pre, size_t depth, size_t maxDepth, size_t index> struct construct_serialize_entity {
		using serialize_entity_pre_type = std::remove_cvref_t<serialize_entity_pre>;

		using type = json_entity<serialize_entity_pre_type::memberPtr, serialize_entity_pre_type::type, serialize_entity_pre_type::name, index,
			getForceInline<jsonifier::core<std::remove_cvref_t<typename serialize_entity_pre_type::class_type>>>()>;
	};

	template<auto options, typename value_type, size_t depth, size_t maxDepth, typename index_sequence> struct get_serialize_entities;

	template<auto options, typename value_type, size_t depth, size_t maxDepth, size_t... I>
	struct get_serialize_entities<options, value_type, depth, maxDepth, std::index_sequence<I...>> {
		using type = serialize_entities<options, depth, maxDepth,
			typename construct_serialize_entity<options, decltype(jsonifier_internal::get<I>(jsonifier::core<std::remove_cvref_t<value_type>>::parseValue)), depth, maxDepth,
				I>::type...>;
	};

	template<auto options, typename value_type, size_t depth, size_t maxDepth> using serialize_entities_t =
		typename get_serialize_entities<options, value_type, depth, maxDepth, jsonifier_internal::tag_range<jsonifier_internal::tuple_size_v<raw_core_type<value_type>>>>::type;

	template<jsonifier::concepts::jsonifier_object_t value_type, typename context_type, jsonifier::serialize_options options, auto jsonEntity, size_t depth, size_t maxDepth>
	struct object_val_serializer<value_type, context_type, options, jsonEntity, depth, maxDepth> {
		static constexpr char packedValues01[]{ "{\n" };
		static constexpr char packedValues02[]{ ": " };
		static constexpr char packedValues03[]{ ",\n" };
		static constexpr char packedValues04[]{ "{}" };

		template<jsonifier::concepts::jsonifier_object_t value_type_new> JSONIFIER_FORCE_INLINE static void implForceInline(value_type_new&& value, context_type& context) noexcept {
			static constexpr auto memberCount{ tuple_size_v<core_tuple_type<value_type>> };
			static constexpr auto paddingSize{ getPaddingSize<options, std::remove_cvref_t<value_type>>() };

			if constexpr (memberCount > 0) {
				if constexpr (options.prettify) {
					const auto additionalSize = (paddingSize + (memberCount * context.indent));
					context.index			  = context.bufferPtr - context.buffer.data();
					if (context.buffer.size() <= context.index + additionalSize) {
						context.buffer.resize((context.index + additionalSize) * 2);
						context.bufferPtr = context.buffer.data() + context.index;
					}
					context.indent += options.indentSize;
					std::memcpy(context.bufferPtr, packedValues01, 2);
					context.bufferPtr += 2;
					std::memset(context.bufferPtr, options.indentChar, context.indent);
					context.bufferPtr += context.indent;
				} else {
					context.index = static_cast<size_t>(context.bufferPtr - context.buffer.data());
					if (context.buffer.size() <= context.index + paddingSize) {
						context.buffer.resize((context.index + paddingSize) * 2);
						context.bufferPtr = context.buffer.data() + context.index;
					}
					*context.bufferPtr = lBrace;
					++context.bufferPtr;
				}

				static constexpr jsonifier_internal::serialize_entities_t<options, value_type, depth, maxDepth> values{};
				values.iterateValues(value, context);

				if constexpr (options.prettify) {
					context.indent -= options.indentSize;
					*context.bufferPtr = newline;
					++context.bufferPtr;
					std::memset(context.bufferPtr, options.indentChar, context.indent);
					context.bufferPtr += context.indent;
				}
				*context.bufferPtr = rBrace;
				++context.bufferPtr;
			} else {
				std::memcpy(context.bufferPtr, packedValues02, 2);
				context.bufferPtr += 2;
			}
		}

		template<jsonifier::concepts::jsonifier_object_t value_type_new> JSONIFIER_INLINE static void impl(value_type_new&& value, context_type& context) noexcept {
			static constexpr auto memberCount{ tuple_size_v<core_tuple_type<value_type>> };
			static constexpr auto paddingSize{ getPaddingSize<options, std::remove_cvref_t<value_type>>() };

			if constexpr (memberCount > 0) {
				if constexpr (options.prettify) {
					const auto additionalSize = (paddingSize + (memberCount * context.indent));
					context.index			  = context.bufferPtr - context.buffer.data();
					if (context.buffer.size() <= context.index + additionalSize) {
						context.buffer.resize((context.index + additionalSize) * 2);
						context.bufferPtr = context.buffer.data() + context.index;
					}
					context.indent += options.indentSize;
					std::memcpy(context.bufferPtr, packedValues01, 2);
					context.bufferPtr += 2;
					std::memset(context.bufferPtr, options.indentChar, context.indent);
					context.bufferPtr += context.indent;
				} else {
					context.index = static_cast<size_t>(context.bufferPtr - context.buffer.data());
					if (context.buffer.size() <= context.index + paddingSize) {
						context.buffer.resize((context.index + paddingSize) * 2);
						context.bufferPtr = context.buffer.data() + context.index;
					}
					*context.bufferPtr = lBrace;
					++context.bufferPtr;
				}

				static constexpr jsonifier_internal::serialize_entities_t<options, value_type, depth, maxDepth> values{};
				values.iterateValues(value, context);

				if constexpr (options.prettify) {
					context.indent -= options.indentSize;
					*context.bufferPtr = newline;
					++context.bufferPtr;
					std::memset(context.bufferPtr, options.indentChar, context.indent);
					context.bufferPtr += context.indent;
				}
				*context.bufferPtr = rBrace;
				++context.bufferPtr;
			} else {
				std::memcpy(context.bufferPtr, packedValues02, 2);
				context.bufferPtr += 2;
			}
		}
	};

	template<jsonifier::concepts::map_t value_type, typename context_type, jsonifier::serialize_options options, auto jsonEntity, size_t depth, size_t maxDepth>
	struct object_val_serializer<value_type, context_type, options, jsonEntity, depth, maxDepth> {
		static constexpr char packedValues01[]{ "{\n" };
		static constexpr char packedValues02[]{ ": " };
		static constexpr char packedValues03[]{ ",\n" };
		static constexpr char packedValues04[]{ "{}" };
		template<jsonifier::concepts::map_t value_type_new> JSONIFIER_FORCE_INLINE static void impl(value_type_new&& value, context_type& context) noexcept {
			const auto newSize = value.size();
			static constexpr auto paddingSize{ getPaddingSize<options, typename std::remove_cvref_t<value_type>::mapped_type>() };

			if JSONIFIER_LIKELY (newSize > 0) {
				if constexpr (options.prettify) {
					const auto additionalSize = newSize * (paddingSize + context.indent);
					context.index			  = context.bufferPtr - context.buffer.data();
					if (context.buffer.size() <= context.index + additionalSize) {
						context.buffer.resize((context.index + additionalSize) * 2);
						context.bufferPtr = context.buffer.data() + context.index;
					}
					context.indent += options.indentSize;
					std::memcpy(context.bufferPtr, packedValues01, 2);
					context.bufferPtr += 2;
					std::memset(context.bufferPtr, options.indentChar, context.indent);
					context.bufferPtr += context.indent;
				} else {
					const auto additionalSize = newSize * paddingSize;
					context.index			  = context.bufferPtr - context.buffer.data();
					if (context.buffer.size() <= context.index + additionalSize) {
						context.buffer.resize((context.index + additionalSize) * 2);
						context.bufferPtr = context.buffer.data() + context.index;
					}
					*context.bufferPtr = lBrace;
					++context.bufferPtr;
				}
				auto iter = value.begin();
				serialize<options, jsonEntity, depth, maxDepth>::impl(iter->first, context);
				if constexpr (options.prettify) {
					std::memcpy(context.bufferPtr, packedValues02, 2);
					context.bufferPtr += 2;
				} else {
					*context.bufferPtr = colon;
					++context.bufferPtr;
				}
				serialize<options, jsonEntity, depth, maxDepth>::impl(iter->second, context);
				++iter;
				const auto end = value.end();
				for (; iter != end; ++iter) {
					if constexpr (options.prettify) {
						std::memcpy(context.bufferPtr, packedValues03, 2);
						context.bufferPtr += 2;
						std::memset(context.bufferPtr, options.indentChar, context.indent);
						context.bufferPtr += context.indent;
					} else {
						*context.bufferPtr = comma;
						++context.bufferPtr;
					}
					serialize<options, jsonEntity, depth, maxDepth>::impl(iter->first, context);
					if constexpr (options.prettify) {
						std::memcpy(context.bufferPtr, packedValues02, 2);
						context.bufferPtr += 2;
					} else {
						*context.bufferPtr = colon;
						++context.bufferPtr;
					}
					serialize<options, jsonEntity, depth, maxDepth>::impl(iter->second, context);
				}
				if constexpr (options.prettify) {
					context.indent -= options.indentSize;
					*context.bufferPtr = newline;
					++context.bufferPtr;
					std::memset(context.bufferPtr, options.indentChar, context.indent);
					context.bufferPtr += context.indent;
				}
				*context.bufferPtr = rBrace;
				++context.bufferPtr;
			} else {
				std::memcpy(context.bufferPtr, packedValues04, 2);
				context.bufferPtr += 2;
			}
		}
	};

	template<jsonifier::concepts::tuple_t value_type, typename context_type, jsonifier::serialize_options options, auto jsonEntity, size_t depth, size_t maxDepth>
	struct object_val_serializer<value_type, context_type, options, jsonEntity, depth, maxDepth> {
		static constexpr char packedValues01[]{ "{\n" };
		static constexpr char packedValues02[]{ ": " };
		static constexpr char packedValues03[]{ ",\n" };
		static constexpr char packedValues04[]{ "{}" };
		template<jsonifier::concepts::tuple_t value_type_new> JSONIFIER_FORCE_INLINE static void impl(value_type_new&& value, context_type& context) noexcept {
			static constexpr auto additionalSize{ getPaddingSize<options, std::remove_cvref_t<value_type>>() };
			context.index = static_cast<size_t>(context.bufferPtr - context.buffer.data());
			if (context.buffer.size() <= context.index + additionalSize) {
				context.buffer.resize((context.index + additionalSize) * 2);
				context.bufferPtr = context.buffer.data() + context.index;
			}
			static constexpr auto size = tuple_size_v<std::remove_reference_t<value_type>>;
			if constexpr (size > 0) {
				*context.bufferPtr = lBracket;
				++context.bufferPtr;
				if constexpr (options.prettify) {
					context.indent += options.indentSize;
					*context.bufferPtr = newline;
					++context.bufferPtr;
					std::memset(context.bufferPtr, options.indentChar, context.indent);
					context.bufferPtr += context.indent;
				}
				serializeObjects<0, size>(value, context);
				if constexpr (options.prettify) {
					context.indent -= options.indentSize;
					*context.bufferPtr = newline;
					++context.bufferPtr;
					std::memset(context.bufferPtr, options.indentChar, context.indent);
					context.bufferPtr += context.indent;
				}
				*context.bufferPtr = rBracket;
				++context.bufferPtr;
			} else {
				std::memcpy(context.bufferPtr, packedValues01, 2);
				context.bufferPtr += 2;
			}
		}

		template<size_t currentIndex, size_t newSize, jsonifier::concepts::tuple_t value_type_new>
		JSONIFIER_FORCE_INLINE static void serializeObjects(value_type_new&& value, context_type& context) noexcept {
			if constexpr (currentIndex < newSize) {
				auto subTuple	  = jsonifier_internal::get<currentIndex>(value);
				serialize<options, jsonEntity, depth, maxDepth>::impl(subTuple, context);
				if constexpr (currentIndex < newSize - 1) {
					if constexpr (options.prettify) {
						std::memcpy(context.bufferPtr, ",\n", 2);
						context.bufferPtr += 2;
						std::memset(context.bufferPtr, options.indentChar, context.indent * options.indentSize);
						context.bufferPtr += context.indent;
					} else {
						*context.bufferPtr = comma;
						++context.bufferPtr;
					}
				}
				return serializeObjects<currentIndex + 1, newSize>(value, context);
			}
		}
	};

	template<jsonifier::concepts::vector_t value_type, typename context_type, jsonifier::serialize_options options, auto jsonEntity, size_t depth, size_t maxDepth>
	struct array_val_serializer<value_type, context_type, options, jsonEntity, depth, maxDepth> {
		static constexpr char packedValues01[]{ "[\n" };
		static constexpr char packedValues02[]{ ",\n" };
		static constexpr char packedValues03[]{ "[]" };
		template<jsonifier::concepts::vector_t value_type_new> JSONIFIER_FORCE_INLINE static void impl(value_type_new&& value, context_type& context) noexcept {
			const auto newSize = value.size();
			static constexpr auto paddingSize{ getPaddingSize<options, typename std::remove_cvref_t<value_type>::value_type>() };
			if JSONIFIER_LIKELY (newSize > 0) {
				if constexpr (options.prettify) {
					const auto additionalSize = newSize * (paddingSize + context.indent);
					context.index			  = context.bufferPtr - context.buffer.data();
					if (context.buffer.size() <= context.index + additionalSize) {
						context.buffer.resize((context.index + additionalSize) * 2);
						context.bufferPtr = context.buffer.data() + context.index;
					}
					context.indent += options.indentSize;
					std::memcpy(context.bufferPtr, packedValues01, 2);
					context.bufferPtr += 2;
					std::memset(context.bufferPtr, options.indentChar, context.indent);
					context.bufferPtr += context.indent;
				} else {
					const auto additionalSize = newSize * paddingSize;
					context.index			  = context.bufferPtr - context.buffer.data();
					if (context.buffer.size() <= context.index + additionalSize) {
						context.buffer.resize((context.index + additionalSize) * 2);
						context.bufferPtr = context.buffer.data() + context.index;
					}
					*context.bufferPtr = lBracket;
					++context.bufferPtr;
				}
				auto iter = getBeginIterVec(value);
				const int64_t vecSize{ static_cast<int64_t>(value.size()) };
				serialize<options, jsonEntity, depth, maxDepth>::impl(iter[0], context);
				for (int64_t index{ 1 }; index != vecSize; ++index) {
					if constexpr (options.prettify) {
						std::memcpy(context.bufferPtr, packedValues02, 2);
						context.bufferPtr += 2;
						std::memset(context.bufferPtr, options.indentChar, context.indent);
						context.bufferPtr += context.indent;
					} else {
						*context.bufferPtr = comma;
						++context.bufferPtr;
					}
					serialize<options, jsonEntity, depth, maxDepth>::impl(iter[index], context);
				}
				if constexpr (options.prettify) {
					context.indent -= options.indentSize;
					*context.bufferPtr = newline;
					++context.bufferPtr;
					std::memset(context.bufferPtr, options.indentChar, context.indent);
					context.bufferPtr += context.indent;
				}
				*context.bufferPtr = rBracket;
				++context.bufferPtr;
			} else {
				std::memcpy(context.bufferPtr, packedValues03, 2);
				context.bufferPtr += 2;
			}
		}
	};

	template<jsonifier::concepts::raw_array_t value_type, typename context_type, jsonifier::serialize_options options, auto jsonEntity, size_t depth, size_t maxDepth>
	struct array_val_serializer<value_type, context_type, options, jsonEntity, depth, maxDepth> {
		static constexpr char packedValues01[]{ "[\n" };
		static constexpr char packedValues02[]{ ",\n" };
		static constexpr char packedValues03[]{ "[]" };
		template<template<typename, size_t> typename value_type_new, typename value_type_internal, size_t size>
		JSONIFIER_FORCE_INLINE static void impl(const value_type_new<value_type_internal, size>& value, context_type& context) noexcept {
			constexpr auto newSize = size;
			static constexpr auto paddingSize{ getPaddingSize<options, typename std::remove_cvref_t<value_type_new<value_type_internal, size>>::value_type>() };
			if constexpr (newSize > 0) {
				if constexpr (options.prettify) {
					const auto additionalSize = newSize * (paddingSize + context.indent);
					context.index			  = context.bufferPtr - context.buffer.data();
					if (context.buffer.size() <= context.index + additionalSize) {
						context.buffer.resize((context.index + additionalSize) * 2);
						context.bufferPtr = context.buffer.data() + context.index;
					}
					context.indent += options.indentSize;
					std::memcpy(context.bufferPtr, packedValues01, 2);
					context.bufferPtr += 2;
					std::memset(context.bufferPtr, options.indentChar, context.indent);
					context.bufferPtr += context.indent;
				} else {
					const auto additionalSize = newSize * paddingSize;
					context.index			  = context.bufferPtr - context.buffer.data();
					if (context.buffer.size() <= context.index + additionalSize) {
						context.buffer.resize((context.index + additionalSize) * 2);
						context.bufferPtr = context.buffer.data() + context.index;
					}
					*context.bufferPtr = lBracket;
					++context.bufferPtr;
				}
				auto iter = getBeginIterVec(value);
				static constexpr int64_t vecSize{ static_cast<int64_t>(value.size()) };
				serialize<options, jsonEntity, depth, maxDepth>::impl(iter[0], context);
				for (int64_t index{ 1 }; index != vecSize; ++index) {
					if constexpr (options.prettify) {
						std::memcpy(context.bufferPtr, packedValues02, 2);
						context.bufferPtr += 2;
						std::memset(context.bufferPtr, options.indentChar, context.indent);
						context.bufferPtr += context.indent;
					} else {
						*context.bufferPtr = comma;
						++context.bufferPtr;
					}
					serialize<options, jsonEntity, depth, maxDepth>::impl(iter[index], context);
				}
				if constexpr (options.prettify) {
					context.indent -= options.indentSize;
					*context.bufferPtr = newline;
					++context.bufferPtr;
					std::memset(context.bufferPtr, options.indentChar, context.indent);
					context.bufferPtr += context.indent;
				}
				*context.bufferPtr = rBracket;
				++context.bufferPtr;
			} else {
				std::memcpy(context.bufferPtr, packedValues03, 2);
				context.bufferPtr += 2;
			}
		}
	};

	template<jsonifier::concepts::string_t value_type, typename context_type, jsonifier::serialize_options options, auto jsonEntity, size_t depth, size_t maxDepth>
	struct string_val_serializer<value_type, context_type, options, jsonEntity, depth, maxDepth> {
		static constexpr char packedValues01[]{ "\"\"" };

		template<jsonifier::concepts::string_t value_type_new> JSONIFIER_FORCE_INLINE static void impl(value_type_new&& value, context_type& context) noexcept {
			const auto newSize = value.size();
			static constexpr auto paddingSize{ getPaddingSize<options, typename std::remove_cvref_t<value_type>::value_type>() };
			if (newSize > 0) {
				if constexpr (options.prettify) {
					const auto additionalSize = newSize * (paddingSize + context.indent);
					context.index			  = context.bufferPtr - context.buffer.data();
					if (context.buffer.size() <= context.index + additionalSize) {
						context.buffer.resize((context.index + additionalSize) * 2);
						context.bufferPtr = context.buffer.data() + context.index;
					}
				} else {
					const auto additionalSize = newSize * paddingSize;
					context.index			  = context.bufferPtr - context.buffer.data();
					if (context.buffer.size() <= context.index + additionalSize) {
						context.buffer.resize((context.index + additionalSize) * 2);
						context.bufferPtr = context.buffer.data() + context.index;
					}
				}
				*context.bufferPtr = quote;
				++context.bufferPtr;
				auto newPtr		   = string_serializer<options, decltype(value.data()), decltype(context.bufferPtr)>::impl(value.data(), context.bufferPtr, value.size());
				context.bufferPtr  = newPtr;
				*context.bufferPtr = quote;
				++context.bufferPtr;
			} else {
				std::memcpy(context.bufferPtr, packedValues01, 2);
				context.bufferPtr += 2;
			}
		}
	};

	template<jsonifier::concepts::char_t value_type, typename context_type, jsonifier::serialize_options options, auto jsonEntity, size_t depth, size_t maxDepth>
	struct string_val_serializer<value_type, context_type, options, jsonEntity, depth, maxDepth> {
		template<jsonifier::concepts::char_t value_type_new> JSONIFIER_FORCE_INLINE static void impl(value_type_new&& value, context_type& context) noexcept {
			*context.bufferPtr = quote;
			++context.bufferPtr;
			switch (value) {
				[[unlikely]] case '\b': {
					std::memcpy(context.bufferPtr, R"(\b)", 2);
					context.bufferPtr += 2;
					break;
				}
				[[unlikely]] case '\t': {
					std::memcpy(context.bufferPtr, R"(\t)", 2);
					context.bufferPtr += 2;
					break;
				}
				[[unlikely]] case '\n': {
					std::memcpy(context.bufferPtr, R"(\n)", 2);
					context.bufferPtr += 2;
					break;
				}
				[[unlikely]] case '\f': {
					std::memcpy(context.bufferPtr, R"(\f)", 2);
					context.bufferPtr += 2;
					break;
				}
				[[unlikely]] case '\r': {
					std::memcpy(context.bufferPtr, R"(\r)", 2);
					context.bufferPtr += 2;
					break;
				}
				[[unlikely]] case '"': {
					std::memcpy(context.bufferPtr, R"(\")", 2);
					context.bufferPtr += 2;
					break;
				}
				[[unlikely]] case '\\': {
					std::memcpy(context.bufferPtr, R"(\\)", 2);
					context.bufferPtr += 2;
					break;
				}
				[[likely]] default: {
					*context.bufferPtr = value;
					++context.bufferPtr;
				}
			}
			*context.bufferPtr = quote;
			++context.bufferPtr;
		}
	};

	template<jsonifier::concepts::enum_t value_type, typename context_type, jsonifier::serialize_options options, auto jsonEntity, size_t depth, size_t maxDepth>
	struct number_val_serializer<value_type, context_type, options, jsonEntity, depth, maxDepth> {
		template<jsonifier::concepts::enum_t value_type_new> JSONIFIER_FORCE_INLINE static void impl(value_type_new&& value, context_type& context) noexcept {
			int64_t valueNew{ static_cast<int64_t>(value) };
			serialize<options, jsonEntity, depth, maxDepth>::impl(valueNew, context);
		}
	};

	template<jsonifier::concepts::num_t value_type, typename context_type, jsonifier::serialize_options options, auto jsonEntity, size_t depth, size_t maxDepth>
	struct number_val_serializer<value_type, context_type, options, jsonEntity, depth, maxDepth> {
		template<jsonifier::concepts::num_t value_type_new> JSONIFIER_FORCE_INLINE static void impl(value_type_new&& value, context_type& context) noexcept {
			if constexpr (sizeof(value_type) == 8) {
				context.bufferPtr = toChars<value_type>(context.bufferPtr, value);
			} else {
				if constexpr (jsonifier::concepts::unsigned_t<value_type>) {
					context.bufferPtr = toChars<uint64_t>(context.bufferPtr, static_cast<uint64_t>(value));
				} else if constexpr (jsonifier::concepts::signed_t<value_type>) {
					context.bufferPtr = toChars<int64_t>(context.bufferPtr, static_cast<int64_t>(value));
				} else {
					context.bufferPtr = toChars<float>(context.bufferPtr, static_cast<double>(value));
				}
			}
		}
	};

	template<jsonifier::concepts::bool_t value_type, typename context_type, jsonifier::serialize_options options, auto jsonEntity, size_t depth, size_t maxDepth>
	struct bool_val_serializer<value_type, context_type, options, jsonEntity, depth, maxDepth> {
		template<jsonifier::concepts::bool_t value_type_new> JSONIFIER_FORCE_INLINE static void impl(value_type_new&& value, context_type& context) noexcept {
			const uint64_t state = falseVInt - (value * trueVInt);
			std::memcpy(context.bufferPtr, &state, 5);
			context.bufferPtr += 5 - value;
		}
	};

	template<jsonifier::concepts::always_null_t value_type, typename context_type, jsonifier::serialize_options options, auto jsonEntity, size_t depth, size_t maxDepth>
	struct null_val_serializer<value_type, context_type, options, jsonEntity, depth, maxDepth> {
		template<jsonifier::concepts::always_null_t value_type_new> JSONIFIER_FORCE_INLINE static void impl(value_type_new&&, context_type& context) noexcept {
			std::memcpy(context.bufferPtr, nullV, 4);
			context.bufferPtr += 4;
		}
	};

	template<jsonifier::concepts::pointer_t value_type, typename context_type, jsonifier::serialize_options options, auto jsonEntity, size_t depth, size_t maxDepth>
	struct accessor_val_serializer<value_type, context_type, options, jsonEntity, depth, maxDepth> {
		template<jsonifier::concepts::pointer_t value_type_new> JSONIFIER_FORCE_INLINE static void impl(value_type_new&& value, context_type& context) noexcept {
			if (value) {
				serialize<options, jsonEntity, depth, maxDepth>::impl(*value, context);
			} else {
				std::memcpy(context.bufferPtr, nullV, 4);
				context.bufferPtr += 4;
			}
		}
	};

	template<jsonifier::concepts::raw_json_t value_type, typename context_type, jsonifier::serialize_options options, auto jsonEntity, size_t depth, size_t maxDepth>
	struct accessor_val_serializer<value_type, context_type, options, jsonEntity, depth, maxDepth> {
		template<jsonifier::concepts::raw_json_t value_type_new> JSONIFIER_FORCE_INLINE static void impl(value_type_new&& value, context_type& context) noexcept {
			serialize<options, jsonEntity, depth, maxDepth>::impl(value.rawJson(), context);
		}
	};

	template<jsonifier::concepts::skip_t value_type, typename context_type, jsonifier::serialize_options options, auto jsonEntity, size_t depth, size_t maxDepth>
	struct accessor_val_serializer<value_type, context_type, options, jsonEntity, depth, maxDepth> {
		template<jsonifier::concepts::skip_t value_type_new> JSONIFIER_FORCE_INLINE static void impl(value_type_new&& value, context_type& context) noexcept {
			std::memcpy(context.bufferPtr, nullV, 4);
			context.bufferPtr += 4;
		}
	};

	template<jsonifier::concepts::unique_ptr_t value_type, typename context_type, jsonifier::serialize_options options, auto jsonEntity, size_t depth, size_t maxDepth>
	struct accessor_val_serializer<value_type, context_type, options, jsonEntity, depth, maxDepth> {
		template<jsonifier::concepts::unique_ptr_t value_type_new> JSONIFIER_FORCE_INLINE static void impl(value_type_new&& value, context_type& context) noexcept {
			if JSONIFIER_LIKELY (value) {
				serialize<options, jsonEntity, depth, maxDepth>::impl(*value, context);
			} else {
				std::memcpy(context.bufferPtr, nullV, 4);
				context.bufferPtr += 4;
			}
		}
	};

	template<jsonifier::concepts::shared_ptr_t value_type, typename context_type, jsonifier::serialize_options options, auto jsonEntity, size_t depth, size_t maxDepth>
	struct accessor_val_serializer<value_type, context_type, options, jsonEntity, depth, maxDepth> {
		template<jsonifier::concepts::shared_ptr_t value_type_new> JSONIFIER_FORCE_INLINE static void impl(value_type_new&& value, context_type& context) noexcept {
			if JSONIFIER_LIKELY (value) {
				serialize<options, jsonEntity, depth, maxDepth>::impl(*value, context);
			} else {
				std::memcpy(context.bufferPtr, nullV, 4);
				context.bufferPtr += 4;
			}
		}
	};

	template<jsonifier::concepts::variant_t value_type, typename context_type, jsonifier::serialize_options options, auto jsonEntity, size_t depth, size_t maxDepth>
	struct accessor_val_serializer<value_type, context_type, options, jsonEntity, depth, maxDepth> {
		template<jsonifier::concepts::variant_t value_type_new> JSONIFIER_FORCE_INLINE static void impl(value_type_new&& value, context_type& context) noexcept {
			static constexpr auto lambda = [](auto&& valueNewer, auto&& contextNew) {
				serialize<options, jsonEntity, depth, maxDepth>::impl(valueNewer, contextNew);
			};
			visit<lambda>(value, context);
		}
	};


	template<jsonifier::concepts::optional_t value_type, typename context_type, jsonifier::serialize_options options, auto jsonEntity, size_t depth, size_t maxDepth>
	struct accessor_val_serializer<value_type, context_type, options, jsonEntity, depth, maxDepth> {
		template<jsonifier::concepts::optional_t value_type_new> JSONIFIER_FORCE_INLINE static void impl(value_type_new&& value, context_type& context) noexcept {
			if JSONIFIER_LIKELY (value) {
				serialize<options, jsonEntity, depth, maxDepth>::impl(value.value(), context);
			} else {
				std::memcpy(context.bufferPtr, nullV, 4);
				context.bufferPtr += 4;
			}
		}
	};
}