
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

#include <jsonifier/Serializing/Serializer.hpp>
#include <jsonifier/Parsing/Parser.hpp>
#include <jsonifier/Utilities/TypeEntities.hpp>
#include <jsonifier/Utilities/JsonEntity.hpp>
#include <expected>
#include <algorithm>
#include <assert.h>

namespace jsonifier::internal {

	template<serialize_options options, typename value_type> static constexpr size_t getPaddingSize() noexcept {
		if constexpr (concepts::jsonifier_object_t<value_type>) {
			constexpr auto memberCount = core_tuple_size<value_type>;
			constexpr auto newSize	   = []() constexpr {
				size_t pair{};
				constexpr auto sizeCollectLambda = [](const auto currentIndex, const auto maxIndex, auto& pairNew) {
					if constexpr (currentIndex < maxIndex) {
						constexpr auto subTuple	 = get<currentIndex>(core_tuple_type<value_type>{});
						constexpr auto key		 = subTuple.name;
						constexpr auto quotedKey = string_literal{ "\"" } + key + string_literal{ "\": " };
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
		} else if constexpr (concepts::bool_t<value_type>) {
			return 5;
		} else if constexpr (concepts::num_t<value_type>) {
			return 24;
		} else if constexpr (concepts::vector_t<value_type>) {
			return 4 + getPaddingSize<options, typename value_type::value_type>();
		} else if constexpr (concepts::tuple_t<value_type>) {
			constexpr auto accumulatePaddingSizesImpl = []<typename tuple, size_t... indices>(jsonifier::internal::index_sequence<indices...>) noexcept {
				return (getPaddingSize<options, std::tuple_element_t<indices, tuple>>() + ... + 0);
			};
			constexpr auto accumulatePaddingSizes = []<typename tuple>(auto&& accumulatePaddingSizesImplNew) {
				return accumulatePaddingSizesImplNew.template operator()<tuple>(jsonifier::internal::make_index_sequence<tuple_size_v<tuple>>{});
			};
			return accumulatePaddingSizes.template operator()<value_type>(accumulatePaddingSizesImpl);
		} else if constexpr (concepts::raw_array_t<value_type>) {
			return 4 + getPaddingSize<options, typename value_type::value_type>();
		} else if constexpr (concepts::pointer_t<value_type>) {
			return getPaddingSize<options, decltype(*value_type{})>();
		} else if constexpr (concepts::unique_ptr_t<value_type>) {
			return getPaddingSize<options, decltype(*value_type{})>();
		} else if constexpr (concepts::shared_ptr_t<value_type>) {
			return getPaddingSize<options, decltype(*value_type{})>();
		} else if constexpr (concepts::map_t<value_type>) {
			return 12 + getPaddingSize<options, typename value_type::mapped_type>() + getPaddingSize<options, typename value_type::key_type>();
		} else if constexpr (concepts::string_t<value_type>) {
			return 2;
		} else if constexpr (concepts::char_t<value_type>) {
			return 2;
		} else if constexpr (concepts::optional_t<value_type>) {
			return getPaddingSize<options, typename value_type::value_type>();
		} else if constexpr (concepts::always_null_t<value_type>) {
			return 4;
		} else {
			return {};
		}
	}

	template<serialize_options options, string_literal key, typename context_type> JSONIFIER_INLINE static void writeObjectEntry(context_type& context) {
		static constexpr auto unQuotedKey = string_literal{ "\"" } + key;
		if constexpr (options.prettify) {
			static constexpr auto quotedKey	   = unQuotedKey + string_literal{ "\": " };
			static constexpr auto size		   = quotedKey.size();
			static constexpr auto quotedKeyPtr = quotedKey.data();
			std::memcpy(context.bufferPtr, quotedKeyPtr, size);
			context.bufferPtr += size;
		} else {
			static constexpr auto quotedKey	   = unQuotedKey + string_literal{ "\":" };
			static constexpr auto size		   = quotedKey.size();
			static constexpr auto quotedKeyPtr = quotedKey.data();
			std::memcpy(context.bufferPtr, quotedKeyPtr, size);
			context.bufferPtr += size;
		}
	}

	template<serialize_options options, bool isItLast, typename context_type> JSONIFIER_INLINE static void writeObjectExit(context_type& context) {
		if constexpr (!isItLast) {
			if constexpr (options.prettify) {
				JSONIFIER_ALIGN(8) static constexpr char packedValues[]{ ",\n" };
				std::memcpy(context.bufferPtr, packedValues, 2);
				context.bufferPtr += 2;
				std::memset(context.bufferPtr, ' ', context.indent * options.indentSize);
				context.bufferPtr += context.indent;
			} else {
				*context.bufferPtr = comma;
				++context.bufferPtr;
			}
		}
	}

	template<serialize_options options, typename json_entity_type> struct json_entity_serialize : public json_entity_type {
		constexpr json_entity_serialize() noexcept = default;

		template<typename value_type, typename context_type> JSONIFIER_INLINE static void processIndex(value_type& value, context_type& context) {
			if constexpr (concepts::has_excluded_keys<value_type>) {
				auto& keys = value.jsonifierExcludedKeys;
				if JSONIFIER_LIKELY (keys.find(static_cast<typename jsonifier::internal::remove_reference_t<decltype(keys)>::key_type>(json_entity_type::name)) != keys.end()) {
					return;
				}
			}

			/// @brief Writes the object entry and serializes the member.
			writeObjectEntry<options, json_entity_type::name>(context);
			serialize<options>::impl(value.*json_entity_type::memberPtr, context);
			writeObjectExit<options, json_entity_type::isItLast>(context);
		}
	};

	template<typename... bases> struct serialize_map : public bases... {
		template<typename json_entity_type, typename... arg_types> JSONIFIER_INLINE static void iterateValuesImpl(arg_types&&... args) {
			json_entity_type::processIndex(internal::forward<arg_types>(args)...);
		}

		template<typename... arg_types> static constexpr void iterateValues(arg_types&&... args) {
			(( void )(args), ...);
			((iterateValuesImpl<bases>(internal::forward<arg_types>(args)...)), ...);
		}
	};

	template<serialize_options options, typename value_type, typename context_type, typename index_sequence, typename... value_types> struct get_serialize_base;

	template<serialize_options options, typename value_type, typename context_type, size_t... index>
	struct get_serialize_base<options, value_type, context_type, index_sequence<index...>> {
		using type = serialize_map<json_entity_serialize<options, remove_cvref_t<decltype(get<index>(core<value_type>::parseValue))>>...>;
	};

	template<serialize_options options, typename value_type, typename context_type> using serialize_base_t =
		typename get_serialize_base<options, value_type, context_type, make_index_sequence<core_tuple_size<value_type>>>::type;

	template<concepts::jsonifier_object_t value_type, typename context_type, serialize_options options> struct serialize_impl<value_type, context_type, options> {
		JSONIFIER_ALIGN(8) static constexpr char packedValues01[] { "{\n" };
		JSONIFIER_ALIGN(8) static constexpr char packedValues02[] { ": " };
		JSONIFIER_ALIGN(8) static constexpr char packedValues03[] { ",\n" };
		JSONIFIER_ALIGN(8) static constexpr char packedValues04[] { "{}" };
		template<typename value_type_new> JSONIFIER_INLINE static void impl(value_type_new&& value, context_type& context) noexcept {
			static constexpr auto memberCount{ core_tuple_size<value_type> };
			static constexpr auto paddingSize{ getPaddingSize<options, value_type>() * 4 };

			if constexpr (memberCount > 0) {
				if constexpr (options.prettify) {
					const auto additionalSize = (paddingSize + (memberCount * context.indent * 4));
					context.index			  = static_cast<size_t>(context.bufferPtr - context.buffer.data());
					if (context.buffer.size() <= context.index + additionalSize) {
						context.buffer.resize((context.index + additionalSize) * 4);
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
						context.buffer.resize((context.index + paddingSize) * 4);
						context.bufferPtr = context.buffer.data() + context.index;
					}
					*context.bufferPtr = lBrace;
					++context.bufferPtr;
				}

				serialize_base_t<options, value_type, context_type>::iterateValues(value, context);

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

	template<concepts::map_t value_type, typename context_type, serialize_options options> struct serialize_impl<value_type, context_type, options> {
		JSONIFIER_ALIGN(8) static constexpr char packedValues01[] { "{\n" };
		JSONIFIER_ALIGN(8) static constexpr char packedValues02[] { ": " };
		JSONIFIER_ALIGN(8) static constexpr char packedValues03[] { ",\n" };
		JSONIFIER_ALIGN(8) static constexpr char packedValues04[] { "{}" };
		template<typename value_type_new> JSONIFIER_INLINE static void impl(value_type_new&& value, context_type& context) noexcept {
			const auto newSize = value.size();
			static constexpr auto paddingSize{ getPaddingSize<options, typename value_type::mapped_type>() };

			if JSONIFIER_LIKELY (newSize > 0) {
				if constexpr (options.prettify) {
					const auto additionalSize = newSize * (paddingSize + context.indent);
					context.index			  = static_cast<size_t>(context.bufferPtr - context.buffer.data());
					if (context.buffer.size() <= context.index + additionalSize) {
						context.buffer.resize((context.index + additionalSize) * 4);
						context.bufferPtr = context.buffer.data() + context.index;
					}
					context.indent += options.indentSize;
					std::memcpy(context.bufferPtr, packedValues01, 2);
					context.bufferPtr += 2;
					std::memset(context.bufferPtr, options.indentChar, context.indent);
					context.bufferPtr += context.indent;
				} else {
					const auto additionalSize = newSize * paddingSize;
					context.index			  = static_cast<size_t>(context.bufferPtr - context.buffer.data());
					if (context.buffer.size() <= context.index + additionalSize) {
						context.buffer.resize((context.index + additionalSize) * 4);
						context.bufferPtr = context.buffer.data() + context.index;
					}
					*context.bufferPtr = lBrace;
					++context.bufferPtr;
				}
				auto iter = value.begin();
				serialize<options>::impl(iter->first, context);
				if constexpr (options.prettify) {
					std::memcpy(context.bufferPtr, packedValues02, 2);
					context.bufferPtr += 2;
				} else {
					*context.bufferPtr = colon;
					++context.bufferPtr;
				}
				serialize<options>::impl(iter->second, context);
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
					serialize<options>::impl(iter->first, context);
					if constexpr (options.prettify) {
						std::memcpy(context.bufferPtr, packedValues02, 2);
						context.bufferPtr += 2;
					} else {
						*context.bufferPtr = colon;
						++context.bufferPtr;
					}
					serialize<options>::impl(iter->second, context);
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

	template<concepts::tuple_t value_type, typename context_type, serialize_options options> struct serialize_impl<value_type, context_type, options> {
		JSONIFIER_ALIGN(8) static constexpr char packedValues01[] { "[\n" };
		JSONIFIER_ALIGN(8) static constexpr char packedValues02[] { ",\n" };
		JSONIFIER_ALIGN(8) static constexpr char packedValues03[] { "[]" };
		template<typename value_type_new> JSONIFIER_INLINE static void impl(value_type_new&& value, context_type& context) noexcept {
			static constexpr auto additionalSize{ getPaddingSize<options, value_type>() };
			context.index = static_cast<size_t>(context.bufferPtr - context.buffer.data());
			if (context.buffer.size() <= context.index + additionalSize) {
				context.buffer.resize((context.index + additionalSize) * 4);
				context.bufferPtr = context.buffer.data() + context.index;
			}
			static constexpr auto size = tuple_size_v<jsonifier::internal::remove_reference_t<value_type>>;
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
				std::memcpy(context.bufferPtr, packedValues03, 2);
				context.bufferPtr += 2;
			}
		}

		template<size_t currentIndex, size_t newSize, typename value_type_new> static void serializeObjects(value_type_new&& value, context_type& context) noexcept {
			if constexpr (currentIndex < newSize) {
				auto subTuple = get<currentIndex>(value);
				serialize<options>::impl(subTuple, context);
				if constexpr (currentIndex < newSize - 1) {
					if constexpr (options.prettify) {
						std::memcpy(context.bufferPtr, packedValues02, 2);
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

	template<concepts::vector_t value_type, typename context_type, serialize_options options> struct serialize_impl<value_type, context_type, options> {
		JSONIFIER_ALIGN(8) static constexpr char packedValues01[] { "[\n" };
		JSONIFIER_ALIGN(8) static constexpr char packedValues02[] { ",\n" };
		JSONIFIER_ALIGN(8) static constexpr char packedValues03[] { "[]" };
		template<typename value_type_new> JSONIFIER_INLINE static void impl(value_type_new&& value, context_type& context) noexcept {
			const auto newSize = value.size();
			static constexpr auto paddingSize{ getPaddingSize<options, typename value_type::value_type>() };
			if JSONIFIER_LIKELY (newSize > 0) {
				if constexpr (options.prettify) {
					const auto additionalSize = newSize * (paddingSize + context.indent) * 4;
					context.index			  = static_cast<size_t>(context.bufferPtr - context.buffer.data());
					if (context.buffer.size() <= context.index + additionalSize) {
						context.buffer.resize((context.index + additionalSize) * 4);
						context.bufferPtr = context.buffer.data() + context.index;
					}
					context.indent += options.indentSize;
					std::memcpy(context.bufferPtr, packedValues01, 2);
					context.bufferPtr += 2;
					std::memset(context.bufferPtr, options.indentChar, context.indent);
					context.bufferPtr += context.indent;
				} else {
					const auto additionalSize = newSize * paddingSize;
					context.index			  = static_cast<size_t>(context.bufferPtr - context.buffer.data());
					if (context.buffer.size() <= context.index + additionalSize) {
						context.buffer.resize((context.index + additionalSize) * 4);
						context.bufferPtr = context.buffer.data() + context.index;
					}
					*context.bufferPtr = lBracket;
					++context.bufferPtr;
				}
				auto iter = getBeginIterVec(value);
				serialize<options>::impl(iter[0], context);
				for (int64_t index{ 1 }; index != static_cast<int64_t>(newSize); ++index) {
					if constexpr (options.prettify) {
						std::memcpy(context.bufferPtr, packedValues02, 2);
						context.bufferPtr += 2;
						std::memset(context.bufferPtr, options.indentChar, context.indent);
						context.bufferPtr += context.indent;
					} else {
						*context.bufferPtr = comma;
						++context.bufferPtr;
					}
					serialize<options>::impl(iter[index], context);
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

	template<concepts::raw_array_t value_type, typename context_type, serialize_options options> struct serialize_impl<value_type, context_type, options> {
		JSONIFIER_ALIGN(8) static constexpr char packedValues01[] { "[\n" };
		JSONIFIER_ALIGN(8) static constexpr char packedValues02[] { ",\n" };
		JSONIFIER_ALIGN(8) static constexpr char packedValues03[] { "[]" };
		template<template<typename, size_t> typename value_type_new, typename value_type_internal, size_t size>
		JSONIFIER_INLINE static void impl(const value_type_new<value_type_internal, size>& value, context_type& context) noexcept {
			static constexpr auto newSize = size;
			static constexpr auto paddingSize{ getPaddingSize<options, typename value_type_new<value_type_internal, size>::value_type>() };
			if constexpr (newSize > 0) {
				if constexpr (options.prettify) {
					const auto additionalSize = newSize * (paddingSize + context.indent);
					context.index			  = static_cast<size_t>(context.bufferPtr - context.buffer.data());
					if (context.buffer.size() <= context.index + additionalSize) {
						context.buffer.resize((context.index + additionalSize) * 4);
						context.bufferPtr = context.buffer.data() + context.index;
					}
					context.indent += options.indentSize;
					std::memcpy(context.bufferPtr, packedValues01, 2);
					context.bufferPtr += 2;
					std::memset(context.bufferPtr, options.indentChar, context.indent);
					context.bufferPtr += context.indent;
				} else {
					const auto additionalSize = newSize * paddingSize;
					context.index			  = static_cast<size_t>(context.bufferPtr - context.buffer.data());
					if (context.buffer.size() <= context.index + additionalSize) {
						context.buffer.resize((context.index + additionalSize) * 4);
						context.bufferPtr = context.buffer.data() + context.index;
					}
					*context.bufferPtr = lBracket;
					++context.bufferPtr;
				}
				auto iter = getBeginIterVec(value);
				serialize<options>::impl(iter[0], context);
				for (int64_t index{ 1 }; index != newSize; ++index) {
					if constexpr (options.prettify) {
						std::memcpy(context.bufferPtr, packedValues02, 2);
						context.bufferPtr += 2;
						std::memset(context.bufferPtr, options.indentChar, context.indent);
						context.bufferPtr += context.indent;
					} else {
						*context.bufferPtr = comma;
						++context.bufferPtr;
					}
					serialize<options>::impl(iter[index], context);
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

	template<concepts::string_t value_type, typename context_type, serialize_options options> struct serialize_impl<value_type, context_type, options> {
		JSONIFIER_ALIGN(8) static constexpr char packedValues01[] { "\"\"" };
		template<typename value_type_new> JSONIFIER_INLINE static void impl(value_type_new&& value, context_type& context) noexcept {
			const auto newSize = value.size();
			static constexpr auto paddingSize{ getPaddingSize<options, typename value_type::value_type>() };
			if (newSize > 0) {
				if constexpr (options.prettify) {
					const auto additionalSize = newSize * (paddingSize + context.indent);
					context.index			  = static_cast<size_t>(context.bufferPtr - context.buffer.data());
					if (context.buffer.size() <= context.index + additionalSize) {
						context.buffer.resize((context.index + additionalSize) * 4);
						context.bufferPtr = context.buffer.data() + context.index;
					}
				} else {
					const auto additionalSize = newSize * paddingSize;
					context.index			  = static_cast<size_t>(context.bufferPtr - context.buffer.data());
					if (context.buffer.size() <= context.index + additionalSize) {
						context.buffer.resize((context.index + additionalSize) * 4);
						context.bufferPtr = context.buffer.data() + context.index;
					}
				}
				*context.bufferPtr = quote;
				++context.bufferPtr;
				context.bufferPtr  = string_serializer<options, decltype(value.data()), decltype(context.bufferPtr)>::impl(value.data(), context.bufferPtr, value.size());
				*context.bufferPtr = quote;
				++context.bufferPtr;
			} else {
				std::memcpy(context.bufferPtr, packedValues01, 2);
				context.bufferPtr += 2;
			}
		}
	};

	template<concepts::char_t value_type, typename context_type, serialize_options options> struct serialize_impl<value_type, context_type, options> {
		template<typename value_type_new> JSONIFIER_INLINE static void impl(value_type_new&& value, context_type& context) noexcept {
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
				[[unlikely]] case newline: {
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
				[[unlikely]] case quote: {
					std::memcpy(context.bufferPtr, R"(\")", 2);
					context.bufferPtr += 2;
					break;
				}
				[[unlikely]] case backslash: {
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

	template<concepts::enum_t value_type, typename context_type, serialize_options options> struct serialize_impl<value_type, context_type, options> {
		template<typename value_type_new> JSONIFIER_INLINE static void impl(value_type_new&& value, context_type& context) noexcept {
			int64_t valueNew{ static_cast<int64_t>(value) };
			serialize<options>::impl(valueNew, context);
		}
	};

	template<concepts::num_t value_type, typename context_type, serialize_options options> struct serialize_impl<value_type, context_type, options> {
		template<typename value_type_new> JSONIFIER_INLINE static void impl(value_type_new&& value, context_type& context) noexcept {
			if constexpr (sizeof(value_type) == 8) {
				context.bufferPtr = to_chars<std::remove_cvref_t<value_type_new>>::impl(context.bufferPtr, value);
			} else {
				if constexpr (concepts::unsigned_t<std::remove_cvref_t<value_type_new>>) {
					context.bufferPtr = to_chars<std::remove_cvref_t<value_type_new>>::impl(context.bufferPtr, static_cast<uint64_t>(value));
				} else if constexpr (concepts::signed_t<value_type>) {
					context.bufferPtr = to_chars<std::remove_cvref_t<value_type_new>>::impl(context.bufferPtr, static_cast<int64_t>(value));
				} else {
					context.bufferPtr = to_chars<std::remove_cvref_t<value_type_new>>::impl(context.bufferPtr, static_cast<double>(value));
				}
			}
		}
	};

	template<concepts::bool_t value_type, typename context_type, serialize_options options> struct serialize_impl<value_type, context_type, options> {
		template<typename value_type_new> JSONIFIER_INLINE static void impl(value_type_new&& value, context_type& context) noexcept {
			static constexpr uint64_t falseVInt{ 435728179558 };
			static constexpr uint64_t trueVInt{ 434025983730 };
			const uint64_t state = falseVInt - (value * trueVInt);
			std::memcpy(context.bufferPtr, &state, 5);
			context.bufferPtr += 5 - value;
		}
	};

	template<concepts::always_null_t value_type, typename context_type, serialize_options options> struct serialize_impl<value_type, context_type, options> {
		template<typename value_type_new> JSONIFIER_INLINE static void impl(value_type_new&&, context_type& context) noexcept {
			JSONIFIER_ALIGN(4) static constexpr char nullV[]{ "null" };
			std::memcpy(context.bufferPtr, nullV, 4);
			context.bufferPtr += 4;
		}
	};

	template<concepts::pointer_t value_type, typename context_type, serialize_options options> struct serialize_impl<value_type, context_type, options> {
		template<typename value_type_new> JSONIFIER_INLINE static void impl(value_type_new&& value, context_type& context) noexcept {
			if (value) {
				serialize<options>::impl(*value, context);
			} else {
				JSONIFIER_ALIGN(4) static constexpr char nullV[]{ "null" };
				std::memcpy(context.bufferPtr, nullV, 4);
				context.bufferPtr += 4;
			}
		}
	};

	template<concepts::raw_json_t value_type, typename context_type, serialize_options options> struct serialize_impl<value_type, context_type, options> {
		template<typename value_type_new> JSONIFIER_INLINE static void impl(value_type_new&& value, context_type& context) noexcept {
			const auto rawJson = value.rawJson();
			const auto size	   = rawJson.size();
			if (context.buffer.size() <= context.index + size) {
				context.buffer.resize((context.index + size) * 4);
				context.bufferPtr = context.buffer.data() + context.index;
			}
			std::memcpy(context.bufferPtr, rawJson.data(), size);
			context.bufferPtr += size;
		}
	};

	template<concepts::skip_t value_type, typename context_type, serialize_options options> struct serialize_impl<value_type, context_type, options> {
		template<typename value_type_new> JSONIFIER_INLINE static void impl(value_type_new&& value, context_type& context) noexcept {
			JSONIFIER_ALIGN(4) static constexpr char nullV[]{ "null" };
			std::memcpy(context.bufferPtr, nullV, 4);
			context.bufferPtr += 4;
		}
	};

	template<concepts::unique_ptr_t value_type, typename context_type, serialize_options options> struct serialize_impl<value_type, context_type, options> {
		template<typename value_type_new> JSONIFIER_INLINE static void impl(value_type_new&& value, context_type& context) noexcept {
			if JSONIFIER_LIKELY (value) {
				serialize<options>::impl(*value, context);
			} else {
				JSONIFIER_ALIGN(4) static constexpr char nullV[]{ "null" };
				std::memcpy(context.bufferPtr, nullV, 4);
				context.bufferPtr += 4;
			}
		}
	};

	template<concepts::shared_ptr_t value_type, typename context_type, serialize_options options> struct serialize_impl<value_type, context_type, options> {
		template<typename value_type_new> JSONIFIER_INLINE static void impl(value_type_new&& value, context_type& context) noexcept {
			if JSONIFIER_LIKELY (value) {
				serialize<options>::impl(*value, context);
			} else {
				JSONIFIER_ALIGN(4) static constexpr char nullV[]{ "null" };
				std::memcpy(context.bufferPtr, nullV, 4);
				context.bufferPtr += 4;
			}
		}
	};

	template<concepts::variant_t value_type, typename context_type, serialize_options options> struct serialize_impl<value_type, context_type, options> {
		template<typename value_type_new> JSONIFIER_INLINE static void impl(value_type_new&& value, context_type& context) noexcept {
			static constexpr auto lambda = [](auto&& valueNewer, auto&& contextNew) {
				serialize<options>::impl(valueNewer, contextNew);
			};
			visit<lambda>(value, context);
		}
	};


	template<concepts::optional_t value_type, typename context_type, serialize_options options> struct serialize_impl<value_type, context_type, options> {
		template<typename value_type_new> JSONIFIER_INLINE static void impl(value_type_new&& value, context_type& context) noexcept {
			if JSONIFIER_LIKELY (value) {
				serialize<options>::impl(value.value(), context);
			} else {
				JSONIFIER_ALIGN(4) static constexpr char nullV[]{ "null" };
				std::memcpy(context.bufferPtr, nullV, 4);
				context.bufferPtr += 4;
			}
		}
	};
}