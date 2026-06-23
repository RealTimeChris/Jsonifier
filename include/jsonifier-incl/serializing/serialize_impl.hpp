
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

#include <jsonifier-incl/serializing/serializer.hpp>
#include <jsonifier-incl/parsing/parser.hpp>
#include <jsonifier-incl/utilities/type_entities.hpp>
#include <jsonifier-incl/utilities/json_entity.hpp>
#include <expected>
#include <algorithm>
#include <assert.h>

namespace jsonifier::internal {

	template<serialize_options options, typename value_type, typename integer_sequence> struct size_collection_lambda;

	template<serialize_options options, typename value_type, uint64_t... indices> struct size_collection_lambda<options, value_type, std::integer_sequence<uint64_t, indices...>> {
		static constexpr uint64_t maxIndex{ core_tuple_size<value_type> };
		template<uint64_t index> static constexpr void impl(uint64_t& pairNew) {
			constexpr auto subTuple	 = get_because_other_lib_authors_resolve<index>(core_tuple_type<value_type>{});
			constexpr auto key		 = subTuple.name;
			constexpr auto quotedKey = string_literal{ "\"" } + key + string_literal{ "\": " };
			pairNew += quotedKey.size();
			if constexpr (index < maxIndex - 1) {
				if constexpr (options.prettify) {
					pairNew += std::size(",\n") + 1;
				} else {
					++pairNew;
				}
			}
		}

		static constexpr void impl(uint64_t& pairNew) {
			(impl<indices>(pairNew), ...);
		}
	};

	template<serialize_options options, typename value_type> static constexpr uint64_t getPaddingSize() noexcept {
		if constexpr (concepts::jsonifier_object_t<value_type>) {
			constexpr auto memberCount = core_tuple_size<value_type>;
			constexpr uint64_t newSize{ [] {
				uint64_t retVal{ 2 };
				size_collection_lambda<options, value_type, std::make_integer_sequence<uint64_t, memberCount>>::impl(retVal);
				return retVal;
			}() };
			return newSize;
		} else if constexpr (concepts::bool_t<value_type>) {
			return 5;
		} else if constexpr (concepts::num_t<value_type>) {
			return 24;
		} else if constexpr (concepts::vector_t<value_type>) {
			return 4 + getPaddingSize<options, typename value_type::value_type>();
		} else if constexpr (concepts::tuple_t<value_type>) {
			constexpr auto accumulatePaddingSizesImpl = []<typename tuple, uint64_t... indices>(jsonifier::internal::index_sequence<indices...>) noexcept {
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

	template<serialize_options options, typename value_type> JSONIFIER_INLINE static uint64_t computeRuntimeSize(const value_type& value, uint64_t currentIndent) noexcept {
		if constexpr (concepts::jsonifier_object_t<value_type>) {
			return getPaddingSize<options, value_type>() + (options.prettify ? (core_tuple_size<value_type> * currentIndent) : 0);
		} else if constexpr (concepts::vector_t<value_type> || concepts::raw_array_t<value_type>) {
			using elem_t			= typename value_type::value_type;
			const auto perElemFixed = options.prettify ? currentIndent + options.indentSize + 1 : 1;
			uint64_t total			= 2;
			if constexpr (concepts::vector_t<elem_t> || concepts::raw_array_t<elem_t> || concepts::map_t<elem_t> || concepts::string_t<elem_t> || concepts::optional_t<elem_t> ||
				concepts::unique_ptr_t<elem_t> || concepts::shared_ptr_t<elem_t> || concepts::pointer_t<elem_t>) {
				for (const auto& elem: value) {
					total += computeRuntimeSize<options>(elem, currentIndent + options.indentSize) + perElemFixed;
				}
			} else {
				total += value.size() * (getPaddingSize<options, elem_t>() + perElemFixed);
			}
			return total;
		} else if constexpr (concepts::map_t<value_type>) {
			using mapped_t			= typename value_type::mapped_type;
			using key_t				= typename value_type::key_type;
			const auto perPairFixed = options.prettify ? currentIndent + options.indentSize + 4 : 2;
			const auto keyPad		= getPaddingSize<options, key_t>();
			uint64_t total			= 2;
			if constexpr (concepts::vector_t<mapped_t> || concepts::raw_array_t<mapped_t> || concepts::map_t<mapped_t> || concepts::string_t<mapped_t> ||
				concepts::optional_t<mapped_t> || concepts::unique_ptr_t<mapped_t> || concepts::shared_ptr_t<mapped_t> || concepts::pointer_t<mapped_t>) {
				for (const auto& pair: value) {
					total += keyPad + computeRuntimeSize<options>(pair.second, currentIndent + options.indentSize) + perPairFixed;
				}
			} else {
				total += value.size() * (keyPad + getPaddingSize<options, mapped_t>() + perPairFixed);
			}
			return total;
		} else if constexpr (concepts::string_t<value_type>) {
			return value.size() * 2 + 2;
		} else if constexpr (concepts::optional_t<value_type>) {
			return value ? computeRuntimeSize<options>(value.value(), currentIndent) : 4;
		} else if constexpr (concepts::unique_ptr_t<value_type> || concepts::shared_ptr_t<value_type> || concepts::pointer_t<value_type>) {
			return value ? computeRuntimeSize<options>(*value, currentIndent) : 4;
		} else {
			return getPaddingSize<options, value_type>();
		}
	}

	template<serialize_options options, typename context_type> JSONIFIER_INLINE static void ensureCapacity(context_type& context, uint64_t additional) noexcept {
		context.index = static_cast<uint64_t>(context.bufferPtr - context.buffer.data());
		if (context.buffer.size() <= context.index + additional) {
			context.buffer.resize((context.index + additional) * 2);
			context.bufferPtr = context.buffer.data() + context.index;
		}
	}

	template<string_literal string> struct char_blitter {
		using int_type = convert_length_to_int_t<string.size()>;
		static constexpr uint64_t lengthToAdvance{ string.size() };
		static constexpr uint64_t lengthToCopy{ sizeof(int_type) };
		static constexpr auto value{ packValues<string>() };
	};

	template<string_literal prefix, char indentChar, uint64_t indentSize, uint64_t depth> struct JSONIFIER_ALIGN(8) indent_blitter {
		static constexpr uint64_t prefixLen{ prefix.size() };
		static constexpr uint64_t indentLen{ depth * indentSize };
		static constexpr uint64_t totalLen{ prefixLen + indentLen };
		static constexpr uint64_t paddedLen{ (totalLen + 7) & ~uint64_t{ 7 } };

		static constexpr auto build() noexcept {
			array<char, paddedLen> arr{};
			for (uint64_t i = 0; i < prefixLen; ++i) {
				arr[i] = prefix.values[i];
			}
			for (uint64_t i = 0; i < indentLen; ++i) {
				arr[prefixLen + i] = indentChar;
			}
			return arr;
		}

		static constexpr auto value{ build() };
		static constexpr uint64_t lengthToAdvance{ totalLen };
		static constexpr uint64_t lengthToCopy{ paddedLen };
	};

	template<string_literal prefix, char indentChar, uint64_t indentSize, uint64_t... depths> struct indent_table_impl {
		static constexpr auto table = array{ indent_blitter<prefix, indentChar, indentSize, depths>{}... };
	};

	template<string_literal prefix, char indentChar, uint64_t indentSize, uint64_t maxDepth> struct indent_table {
		static constexpr uint64_t entries		 = maxDepth + 1;
		static constexpr uint64_t maxIndentBytes = maxDepth * indentSize;

		static constexpr uint64_t computeTotal() noexcept {
			uint64_t total = 0;
			for (uint64_t d = 0; d <= maxDepth; ++d)
				total += prefix.size() + d * indentSize;
			return total;
		}
		static constexpr uint64_t totalLen = computeTotal();

		static constexpr void buildOffsets(array<uint64_t, entries>& arr) noexcept {
			uint64_t pos = 0;
			for (uint64_t d = 0; d <= maxDepth; ++d) {
				arr[d] = pos;
				pos += prefix.size() + d * indentSize;
			}
			return;
		}

		static constexpr void buildFlat(array<char, totalLen>& arr) noexcept {
			uint64_t pos = 0;
			for (uint64_t d = 0; d <= maxDepth; ++d) {
				for (uint64_t i = 0; i < prefix.size(); ++i)
					arr[pos++] = prefix.values[i];
				for (uint64_t i = 0; i < d * indentSize; ++i)
					arr[pos++] = indentChar;
			}
		}

		static constexpr array<char, totalLen> data{ []() -> array<char, totalLen> {
			array<char, totalLen> arr{};
			buildFlat(arr);
			return arr;
		}() };

		static constexpr array<uint64_t, entries> offsets{ []() -> array<uint64_t, entries> {
			array<uint64_t, entries> arr{};
			buildOffsets(arr);
			return arr;
		}() };

		JSONIFIER_INLINE static void blitWithOverflow(char*& bufferPtr, uint64_t totalIndent) noexcept {
			const uint64_t cappedDepth = (totalIndent / indentSize) < entries ? (totalIndent / indentSize) : maxDepth;
			const uint64_t advance	   = prefix.size() + cappedDepth * indentSize;
			std::memcpy(bufferPtr, data.data() + offsets[cappedDepth], advance);
			bufferPtr += advance;

			const uint64_t remaining = totalIndent > maxIndentBytes ? totalIndent - maxIndentBytes : 0;
			if (remaining) [[unlikely]] {
				std::memset(bufferPtr, indentChar, remaining);
				bufferPtr += remaining;
			}
		}
	};

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
				static constexpr uint64_t maxIndentDepth = 8;
				using comma_indent						 = indent_table<",\n", options.indentChar, options.indentSize, maxIndentDepth>;
				comma_indent::blitWithOverflow(context.bufferPtr, context.indent);
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
			writeObjectEntry<options, json_entity_type::name>(context);
			serialize<options>::impl(value.*json_entity_type::memberPtr, context);
			writeObjectExit<options, json_entity_type::isItLast>(context);
		}
	};

	template<typename... bases> struct serialize_map : public bases... {
		template<typename json_entity_type, typename... arg_types> JSONIFIER_INLINE static void iterateValuesImpl(arg_types&&... args) {
			json_entity_type::processIndex(internal::forward<arg_types>(args)...);
		}

		template<typename... arg_types> JSONIFIER_INLINE static constexpr void iterateValues([[maybe_unused]] arg_types&&... args) {
			((iterateValuesImpl<bases>(internal::forward<arg_types>(args)...)), ...);
		}
	};

	template<serialize_options options, typename value_type, typename context_type, typename index_sequence, typename... value_types> struct get_serialize_base;

	template<serialize_options options, typename value_type, typename context_type, uint64_t... index>
	struct get_serialize_base<options, value_type, context_type, index_sequence<index...>> {
		using type = serialize_map<json_entity_serialize<options, remove_cvref_t<decltype(get_because_other_lib_authors_resolve<index>(core<value_type>::parseValue))>>...>;
	};

	template<serialize_options options, typename value_type, typename context_type> using serialize_base_t =
		typename get_serialize_base<options, value_type, context_type, make_index_sequence<core_tuple_size<value_type>>>::type;

	template<concepts::jsonifier_object_t value_type, typename context_type, serialize_options options> struct serialize_impl<value_type, context_type, options> {
		static constexpr uint64_t maxIndentDepth = 8;
		using open_indent						 = indent_table<"{\n", options.indentChar, options.indentSize, maxIndentDepth>;
		using close_indent						 = indent_table<"\n", options.indentChar, options.indentSize, maxIndentDepth>;

		JSONIFIER_ALIGN(8) static constexpr char_blitter<"{}"> emptyObject {};

		template<typename value_type_new> inline static void impl(value_type_new&& value, context_type& context) noexcept {
			static constexpr auto memberCount{ core_tuple_size<value_type> };
			static constexpr auto paddingSize{ getPaddingSize<options, value_type>() * 4 };

			if constexpr (memberCount > 0) {
				if constexpr (options.prettify) {
					const auto additionalSize = (paddingSize + (memberCount * context.indent * 4));
					ensureCapacity<options>(context, additionalSize);
					context.indent += options.indentSize;
					open_indent::blitWithOverflow(context.bufferPtr, context.indent);
				} else {
					ensureCapacity<options>(context, paddingSize);
					*context.bufferPtr = lBrace;
					++context.bufferPtr;
				}

				serialize_base_t<options, value_type, context_type>::iterateValues(value, context);

				if constexpr (options.prettify) {
					context.indent -= options.indentSize;
					close_indent::blitWithOverflow(context.bufferPtr, context.indent);
				}
				*context.bufferPtr = rBrace;
				++context.bufferPtr;
			} else {
				ensureCapacity<options>(context, 2);
				std::memcpy(context.bufferPtr, &emptyObject.value, emptyObject.lengthToCopy);
				context.bufferPtr += emptyObject.lengthToAdvance;
			}
		}
	};

	template<concepts::map_t value_type, typename context_type, serialize_options options> struct serialize_impl<value_type, context_type, options> {
		static constexpr uint64_t maxIndentDepth = 8;
		using open_indent						 = indent_table<"{\n", options.indentChar, options.indentSize, maxIndentDepth>;
		using comma_indent						 = indent_table<",\n", options.indentChar, options.indentSize, maxIndentDepth>;
		using close_indent						 = indent_table<"\n", options.indentChar, options.indentSize, maxIndentDepth>;

		JSONIFIER_ALIGN(8) static constexpr char_blitter<": "> colonSpace {};
		JSONIFIER_ALIGN(8) static constexpr char_blitter<"{}"> emptyObject {};

		template<typename value_type_new> inline static void impl(value_type_new&& value, context_type& context) noexcept {
			const auto newSize = value.size();
			static constexpr auto paddingSize{ getPaddingSize<options, typename value_type::mapped_type>() };

			if JSONIFIER_LIKELY (newSize > 0) {
				if constexpr (options.prettify) {
					const auto additionalSize = newSize * (paddingSize + context.indent);
					ensureCapacity<options>(context, additionalSize);
					context.indent += options.indentSize;
					open_indent::blitWithOverflow(context.bufferPtr, context.indent);
				} else {
					const auto additionalSize = newSize * paddingSize;
					ensureCapacity<options>(context, additionalSize);
					*context.bufferPtr = lBrace;
					++context.bufferPtr;
				}

				auto iter = value.begin();
				serialize<options>::impl(iter->first, context);
				if constexpr (options.prettify) {
					std::memcpy(context.bufferPtr, &colonSpace.value, colonSpace.lengthToCopy);
					context.bufferPtr += colonSpace.lengthToAdvance;
				} else {
					*context.bufferPtr = colon;
					++context.bufferPtr;
				}
				serialize<options>::impl(iter->second, context);
				++iter;
				const auto end = value.end();
				for (; iter != end; ++iter) {
					if constexpr (options.prettify) {
						comma_indent::blitWithOverflow(context.bufferPtr, context.indent);
					} else {
						*context.bufferPtr = comma;
						++context.bufferPtr;
					}
					serialize<options>::impl(iter->first, context);
					if constexpr (options.prettify) {
						std::memcpy(context.bufferPtr, &colonSpace.value, colonSpace.lengthToCopy);
						context.bufferPtr += colonSpace.lengthToAdvance;
					} else {
						*context.bufferPtr = colon;
						++context.bufferPtr;
					}
					serialize<options>::impl(iter->second, context);
				}
				if constexpr (options.prettify) {
					context.indent -= options.indentSize;
					close_indent::blitWithOverflow(context.bufferPtr, context.indent);
				}
				*context.bufferPtr = rBrace;
				++context.bufferPtr;
			} else {
				std::memcpy(context.bufferPtr, &emptyObject.value, emptyObject.lengthToCopy);
				context.bufferPtr += emptyObject.lengthToAdvance;
			}
		}
	};

	template<concepts::vector_t value_type, typename context_type, serialize_options options> struct serialize_impl<value_type, context_type, options> {
		static constexpr uint64_t maxIndentDepth = 8;
		using open_indent						 = indent_table<"[\n", options.indentChar, options.indentSize, maxIndentDepth>;
		using comma_indent						 = indent_table<",\n", options.indentChar, options.indentSize, maxIndentDepth>;
		using close_indent						 = indent_table<"\n", options.indentChar, options.indentSize, maxIndentDepth>;

		JSONIFIER_ALIGN(8) static constexpr char_blitter<"[]"> emptyArray {};

		template<typename value_type_new> inline static void impl(value_type_new&& value, context_type& context) noexcept {
			const auto newSize = value.size();
			if JSONIFIER_LIKELY (newSize > 0) {
				const auto additional = computeRuntimeSize<options>(value, context.indent);
				ensureCapacity<options>(context, additional);

				if constexpr (options.prettify) {
					context.indent += options.indentSize;
					open_indent::blitWithOverflow(context.bufferPtr, context.indent);
				} else {
					*context.bufferPtr = lBracket;
					++context.bufferPtr;
				}

				auto iter = getBeginIterVec(value);
				serialize<options>::impl(iter[0], context);
				for (uint64_t index{ 1 }; index != newSize; ++index) {
					if constexpr (options.prettify) {
						comma_indent::blitWithOverflow(context.bufferPtr, context.indent);
					} else {
						*context.bufferPtr = comma;
						++context.bufferPtr;
					}
					serialize<options>::impl(iter[static_cast<int64_t>(index)], context);
				}

				if constexpr (options.prettify) {
					context.indent -= options.indentSize;
					close_indent::blitWithOverflow(context.bufferPtr, context.indent);
				}
				*context.bufferPtr = rBracket;
				++context.bufferPtr;
			} else {
				std::memcpy(context.bufferPtr, &emptyArray.value, emptyArray.lengthToCopy);
				context.bufferPtr += emptyArray.lengthToAdvance;
			}
		}
	};

	template<concepts::raw_array_t value_type, typename context_type, serialize_options options> struct serialize_impl<value_type, context_type, options> {
		static constexpr uint64_t maxIndentDepth = 8;
		using open_indent						 = indent_table<"[\n", options.indentChar, options.indentSize, maxIndentDepth>;
		using comma_indent						 = indent_table<",\n", options.indentChar, options.indentSize, maxIndentDepth>;
		using close_indent						 = indent_table<"\n", options.indentChar, options.indentSize, maxIndentDepth>;

		JSONIFIER_ALIGN(8) static constexpr char_blitter<"[]"> emptyArray {};

		template<template<typename, size_t> typename value_type_new, typename value_type_internal, size_t size>
		inline static void impl(const value_type_new<value_type_internal, size>& value, context_type& context) noexcept {
			static constexpr auto newSize = size;
			static constexpr auto paddingSize{ getPaddingSize<options, typename value_type_new<value_type_internal, size>::value_type>() };
			if constexpr (newSize > 0) {
				if constexpr (options.prettify) {
					const auto additionalSize = newSize * (paddingSize + context.indent);
					ensureCapacity<options>(context, additionalSize);
					context.indent += options.indentSize;
					open_indent::blitWithOverflow(context.bufferPtr, context.indent);
				} else {
					const auto additionalSize = newSize * paddingSize;
					ensureCapacity<options>(context, additionalSize);
					*context.bufferPtr = lBracket;
					++context.bufferPtr;
				}

				auto iter = getBeginIterVec(value);
				serialize<options>::impl(iter[0], context);
				for (uint64_t index{ 1 }; index != newSize; ++index) {
					if constexpr (options.prettify) {
						comma_indent::blitWithOverflow(context.bufferPtr, context.indent);
					} else {
						*context.bufferPtr = comma;
						++context.bufferPtr;
					}
					serialize<options>::impl(iter[index], context);
				}

				if constexpr (options.prettify) {
					context.indent -= options.indentSize;
					close_indent::blitWithOverflow(context.bufferPtr, context.indent);
				}
				*context.bufferPtr = rBracket;
				++context.bufferPtr;
			} else {
				std::memcpy(context.bufferPtr, &emptyArray.value, emptyArray.lengthToCopy);
				context.bufferPtr += emptyArray.lengthToAdvance;
			}
		}
	};

	template<concepts::tuple_t value_type, typename context_type, serialize_options options> struct serialize_impl<value_type, context_type, options> {
		static constexpr uint64_t maxIndentDepth = 8;
		using open_indent						 = indent_table<"[\n", options.indentChar, options.indentSize, maxIndentDepth>;
		using comma_indent						 = indent_table<",\n", options.indentChar, options.indentSize, maxIndentDepth>;
		using close_indent						 = indent_table<"\n", options.indentChar, options.indentSize, maxIndentDepth>;

		JSONIFIER_ALIGN(8) static constexpr char_blitter<"[]"> emptyArray {};

		template<typename value_type_new> inline static void impl(value_type_new&& value, context_type& context) noexcept {
			static constexpr auto additionalSize{ getPaddingSize<options, value_type>() };
			ensureCapacity<options>(context, additionalSize);
			static constexpr auto size = std::tuple_size_v<jsonifier::internal::remove_reference_t<value_type>>;
			if constexpr (size > 0) {
				*context.bufferPtr = lBracket;
				++context.bufferPtr;
				if constexpr (options.prettify) {
					context.indent += options.indentSize;
					open_indent::blitWithOverflow(context.bufferPtr, context.indent);
				}
				serializeObjects<0, size>(value, context);
				if constexpr (options.prettify) {
					context.indent -= options.indentSize;
					close_indent::blitWithOverflow(context.bufferPtr, context.indent);
				}
				*context.bufferPtr = rBracket;
				++context.bufferPtr;
			} else {
				std::memcpy(context.bufferPtr, &emptyArray.value, emptyArray.lengthToCopy);
				context.bufferPtr += emptyArray.lengthToAdvance;
			}
		}

		template<uint64_t currentIndex, uint64_t newSize, typename value_type_new> inline static void serializeObjects(value_type_new&& value, context_type& context) noexcept {
			if constexpr (currentIndex < newSize) {
				auto subTuple = get<currentIndex>(value);
				serialize<options>::impl(subTuple, context);
				if constexpr (currentIndex < newSize - 1) {
					if constexpr (options.prettify) {
						comma_indent::blitWithOverflow(context.bufferPtr, context.indent);
					} else {
						*context.bufferPtr = comma;
						++context.bufferPtr;
					}
				}
				return serializeObjects<currentIndex + 1, newSize>(value, context);
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
					ensureCapacity<options>(context, additionalSize);
				} else {
					const auto additionalSize = newSize * paddingSize;
					ensureCapacity<options>(context, additionalSize);
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
					context.bufferPtr = to_chars<std::remove_cvref_t<uint64_t>>::impl(context.bufferPtr, static_cast<uint64_t>(value));
				} else if constexpr (concepts::signed_t<value_type>) {
					context.bufferPtr = to_chars<std::remove_cvref_t<int64_t>>::impl(context.bufferPtr, static_cast<int64_t>(value));
				} else {
					context.bufferPtr = to_chars<std::remove_cvref_t<double>>::impl(context.bufferPtr, static_cast<double>(value));
				}
			}
		}
	};

	template<concepts::bool_t value_type, typename context_type, serialize_options options> struct serialize_impl<value_type, context_type, options> {
		template<typename value_type_new> JSONIFIER_INLINE static void impl(value_type_new&& value, context_type& context) noexcept {
			static constexpr uint64_t falseVInt{ [] {
				if constexpr (std::endian::native == std::endian::little) {
					return 435728179558ULL;
				} else {
					return 439722333440ULL;
				}
			}() };
			static constexpr uint64_t trueVInt{ [] {
				if constexpr (std::endian::native == std::endian::little) {
					return 434025983730ULL;
				} else {
					return -59434803200ULL;
				}
			}() };
			const uint64_t state = falseVInt - (value * trueVInt);
			std::memcpy(context.bufferPtr, &state, 5);
			context.bufferPtr += 5 - value;
		}
	};

	template<concepts::always_null_t value_type, typename context_type, serialize_options options> struct serialize_impl<value_type, context_type, options> {
		template<typename value_type_new> JSONIFIER_INLINE static void impl(value_type_new&&, context_type& context) noexcept {
			JSONIFIER_ALIGN(4) static constexpr char_blitter<"null"> nullV{};
			std::memcpy(context.bufferPtr, &nullV.value, nullV.lengthToCopy);
			context.bufferPtr += nullV.lengthToAdvance;
		}
	};

	template<concepts::pointer_t value_type, typename context_type, serialize_options options> struct serialize_impl<value_type, context_type, options> {
		template<typename value_type_new> JSONIFIER_INLINE static void impl(value_type_new&& value, context_type& context) noexcept {
			if (value) {
				serialize<options>::impl(*value, context);
			} else {
				JSONIFIER_ALIGN(4) static constexpr char_blitter<"null"> nullV{};
				std::memcpy(context.bufferPtr, &nullV.value, nullV.lengthToCopy);
				context.bufferPtr += nullV.lengthToAdvance;
			}
		}
	};

	template<concepts::raw_json_t value_type, typename context_type, serialize_options options> struct serialize_impl<value_type, context_type, options> {
		template<typename value_type_new> JSONIFIER_INLINE static void impl(value_type_new&& value, context_type& context) noexcept {
			const auto rawJson = value.rawJson();
			const auto size	   = rawJson.size();
			ensureCapacity<options>(context, size);
			std::memcpy(context.bufferPtr, rawJson.data(), size);
			context.bufferPtr += size;
		}
	};

	template<concepts::skip_t value_type, typename context_type, serialize_options options> struct serialize_impl<value_type, context_type, options> {
		template<typename value_type_new> JSONIFIER_INLINE static void impl(value_type_new&&, context_type& context) noexcept {
			JSONIFIER_ALIGN(4) static constexpr char_blitter<"null"> nullV{};
			std::memcpy(context.bufferPtr, &nullV.value, nullV.lengthToCopy);
			context.bufferPtr += nullV.lengthToAdvance;
		}
	};

	template<concepts::unique_ptr_t value_type, typename context_type, serialize_options options> struct serialize_impl<value_type, context_type, options> {
		template<typename value_type_new> JSONIFIER_INLINE static void impl(value_type_new&& value, context_type& context) noexcept {
			if JSONIFIER_LIKELY (value) {
				serialize<options>::impl(*value, context);
			} else {
				JSONIFIER_ALIGN(4) static constexpr char_blitter<"null"> nullV{};
				std::memcpy(context.bufferPtr, &nullV.value, nullV.lengthToCopy);
				context.bufferPtr += nullV.lengthToAdvance;
			}
		}
	};

	template<concepts::shared_ptr_t value_type, typename context_type, serialize_options options> struct serialize_impl<value_type, context_type, options> {
		template<typename value_type_new> JSONIFIER_INLINE static void impl(value_type_new&& value, context_type& context) noexcept {
			if JSONIFIER_LIKELY (value) {
				serialize<options>::impl(*value, context);
			} else {
				JSONIFIER_ALIGN(4) static constexpr char_blitter<"null"> nullV{};
				std::memcpy(context.bufferPtr, &nullV.value, nullV.lengthToCopy);
				context.bufferPtr += nullV.lengthToAdvance;
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
				JSONIFIER_ALIGN(4) static constexpr char_blitter<"null"> nullV{};
				std::memcpy(context.bufferPtr, &nullV.value, nullV.lengthToCopy);
				context.bufferPtr += nullV.lengthToAdvance;
			}
		}
	};
}
