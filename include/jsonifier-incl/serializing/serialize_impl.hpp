
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
#include <jsonifier-incl/utilities/utility.hpp>
#include <jsonifier-incl/utilities/json_entity.hpp>

namespace jsonifier::internal {

	template<serialize_options options, typename value_type, typename integer_sequence> struct size_collection_lambda;

	template<serialize_options options, typename value_type, uint64_t... indices> struct size_collection_lambda<options, value_type, integer_sequence<indices...>> {
		static constexpr uint64_t maxIndex{ coreTupleSize<value_type> };
		template<uint64_t index> static constexpr void impl(uint64_t& pairNew) {
			constexpr auto subTuple	 = getBecauseOtherLibAuthorsResolve<index>(core_tuple_type<value_type>{});
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
			constexpr auto memberCount = coreTupleSize<value_type>;
			constexpr uint64_t newSize{ [] {
				uint64_t retVal{ 2 };
				size_collection_lambda<options, value_type, make_integer_sequence<memberCount>>::impl(retVal);
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
			constexpr auto accumulatePaddingSizesImpl = []<typename tuple, uint64_t... indices>(jsonifier::internal::integer_sequence<indices...>) noexcept {
				return (getPaddingSize<options, std::tuple_element_t<indices, tuple>>() + ... + 0);
			};
			constexpr auto accumulatePaddingSizes = []<typename tuple>(auto&& accumulatePaddingSizesImplNew) {
				return accumulatePaddingSizesImplNew.template operator()<tuple>(jsonifier::internal::make_integer_sequence<tuple_size_v<tuple>>{});
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
			return getPaddingSize<options, value_type>() + (options.prettify ? (coreTupleSize<value_type> * currentIndent) : 0);
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

	template<typename value_type> consteval uint64_t getValueSize(value_type value) {
		if constexpr (concepts::integral_types<value_type>) {
			return sizeof(value_type);
		} else {
			return value.size();
		}
	}

	template<string_literal string> struct char_blitter {
		using int_type = convert_length_to_int_t<string.size()>;
		static constexpr uint64_t lengthToAdvance{ string.size() };
		static constexpr uint64_t lengthToCopy{ getValueSize(pack_values<string>::value) };
		static constexpr auto value{ pack_values<string>::value };
	};

	template<string_literal prefix, char indentChar, uint64_t indentSize> struct indent_table {
		static constexpr uint64_t maxDepth{ 8 };
		static constexpr uint64_t maxIndentBytes{ maxDepth * indentSize };
		static constexpr uint64_t totalLen{ prefix.size() + maxIndentBytes };
		static constexpr uint64_t paddedLen{ (totalLen + 7) & ~uint64_t{ 7 } };
		static constexpr array<char, paddedLen> data{ []() -> array<char, paddedLen> {
			array<char, paddedLen> arr{};
			for (uint64_t i = 0; i < prefix.size(); ++i)
				arr[i] = prefix.values[i];
			for (uint64_t i = 0; i < maxIndentBytes; ++i)
				arr[prefix.size() + i] = indentChar;
			return arr;
		}() };

		JSONIFIER_INLINE static void blitWithOverflow(char*& bufferPtr, uint64_t totalIndent) noexcept {
			const uint64_t capped  = totalIndent < maxIndentBytes ? totalIndent : maxIndentBytes;
			const uint64_t advance = prefix.size() + capped;
			const uint64_t copyLen = (advance + 7) & ~uint64_t{ 7 };
			std::memcpy(bufferPtr, data.data(), copyLen);
			bufferPtr += advance;
			const uint64_t remaining = totalIndent - capped;
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
				
				using comma_indent						 = indent_table<",\n", options.indentChar, options.indentSize>;
				comma_indent::blitWithOverflow(context.bufferPtr, context.indent);
			} else {
				*context.bufferPtr = ',';
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
			serialize<options>::impl(getMember<json_entity_type::memberPtr>(value), context);
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

	template<serialize_options options, typename value_type, typename context_type, typename integer_sequence, typename... value_types> struct get_serialize_base;

	template<serialize_options options, typename value_type, typename context_type, uint64_t... index>
	struct get_serialize_base<options, value_type, context_type, integer_sequence<index...>> {
		using type = serialize_map<json_entity_serialize<options, remove_cvref_t<decltype(getBecauseOtherLibAuthorsResolve<index>(core<value_type>::parseValue))>>...>;
	};

	template<serialize_options options, typename value_type, typename context_type> using serialize_base_t =
		typename get_serialize_base<options, value_type, context_type, make_integer_sequence<coreTupleSize<value_type>>>::type;

	template<concepts::jsonifier_object_t value_type, typename context_type, serialize_options options> struct serialize_impl<value_type, context_type, options> {
		using open_indent						 = indent_table<"{\n", options.indentChar, options.indentSize>;
		using close_indent						 = indent_table<"\n", options.indentChar, options.indentSize>;

		JSONIFIER_ALIGN(8) static constexpr char_blitter<"{}"> emptyObject {};

		template<typename value_type_new> inline static void impl(value_type_new&& value, context_type& context) noexcept {
			static constexpr auto memberCount{ coreTupleSize<value_type> };
			static constexpr auto paddingSize{ getPaddingSize<options, value_type>() * 4 };

			if constexpr (memberCount > 0) {
				if constexpr (options.prettify) {
					const auto additionalSize = (paddingSize + (memberCount * context.indent * 4));
					ensureCapacity<options>(context, additionalSize);
					context.indent += options.indentSize;
					open_indent::blitWithOverflow(context.bufferPtr, context.indent);
				} else {
					ensureCapacity<options>(context, paddingSize);
					*context.bufferPtr = '{';
					++context.bufferPtr;
				}

				serialize_base_t<options, value_type, context_type>::iterateValues(value, context);

				if constexpr (options.prettify) {
					context.indent -= options.indentSize;
					close_indent::blitWithOverflow(context.bufferPtr, context.indent);
				}
				*context.bufferPtr = '}';
				++context.bufferPtr;
			} else {
				ensureCapacity<options>(context, 2);
				std::memcpy(context.bufferPtr, &emptyObject.value, emptyObject.lengthToCopy);
				context.bufferPtr += emptyObject.lengthToAdvance;
			}
		}
	};

	template<concepts::map_t value_type, typename context_type, serialize_options options> struct serialize_impl<value_type, context_type, options> {		
		using open_indent						 = indent_table<"{\n", options.indentChar, options.indentSize>;
		using comma_indent						 = indent_table<",\n", options.indentChar, options.indentSize>;
		using close_indent						 = indent_table<"\n", options.indentChar, options.indentSize>;

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
					*context.bufferPtr = '{';
					++context.bufferPtr;
				}

				auto iter = value.begin();
				serialize<options>::impl(iter->first, context);
				if constexpr (options.prettify) {
					std::memcpy(context.bufferPtr, &colonSpace.value, colonSpace.lengthToCopy);
					context.bufferPtr += colonSpace.lengthToAdvance;
				} else {
					*context.bufferPtr = ':';
					++context.bufferPtr;
				}
				serialize<options>::impl(iter->second, context);
				++iter;
				const auto end = value.end();
				for (; iter != end; ++iter) {
					if constexpr (options.prettify) {
						comma_indent::blitWithOverflow(context.bufferPtr, context.indent);
					} else {
						*context.bufferPtr = ',';
						++context.bufferPtr;
					}
					serialize<options>::impl(iter->first, context);
					if constexpr (options.prettify) {
						std::memcpy(context.bufferPtr, &colonSpace.value, colonSpace.lengthToCopy);
						context.bufferPtr += colonSpace.lengthToAdvance;
					} else {
						*context.bufferPtr = ':';
						++context.bufferPtr;
					}
					serialize<options>::impl(iter->second, context);
				}
				if constexpr (options.prettify) {
					context.indent -= options.indentSize;
					close_indent::blitWithOverflow(context.bufferPtr, context.indent);
				}
				*context.bufferPtr = '}';
				++context.bufferPtr;
			} else {
				std::memcpy(context.bufferPtr, &emptyObject.value, emptyObject.lengthToCopy);
				context.bufferPtr += emptyObject.lengthToAdvance;
			}
		}
	};

	template<concepts::vector_t value_type, typename context_type, serialize_options options> struct serialize_impl<value_type, context_type, options> {		
		using open_indent						 = indent_table<"[\n", options.indentChar, options.indentSize>;
		using comma_indent						 = indent_table<",\n", options.indentChar, options.indentSize>;
		using close_indent						 = indent_table<"\n", options.indentChar, options.indentSize>;

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
					*context.bufferPtr = '[';
					++context.bufferPtr;
				}

				auto iter = getBeginIterVec(value);
				serialize<options>::impl(iter[0], context);
				for (uint64_t index{ 1 }; index != newSize; ++index) {
					if constexpr (options.prettify) {
						comma_indent::blitWithOverflow(context.bufferPtr, context.indent);
					} else {
						*context.bufferPtr = ',';
						++context.bufferPtr;
					}
					serialize<options>::impl(iter[static_cast<int64_t>(index)], context);
				}

				if constexpr (options.prettify) {
					context.indent -= options.indentSize;
					close_indent::blitWithOverflow(context.bufferPtr, context.indent);
				}
				*context.bufferPtr = ']';
				++context.bufferPtr;
			} else {
				std::memcpy(context.bufferPtr, &emptyArray.value, emptyArray.lengthToCopy);
				context.bufferPtr += emptyArray.lengthToAdvance;
			}
		}
	};

	template<concepts::raw_array_t value_type, typename context_type, serialize_options options> struct serialize_impl<value_type, context_type, options> {		
		using open_indent						 = indent_table<"[\n", options.indentChar, options.indentSize>;
		using comma_indent						 = indent_table<",\n", options.indentChar, options.indentSize>;
		using close_indent						 = indent_table<"\n", options.indentChar, options.indentSize>;

		JSONIFIER_ALIGN(8) static constexpr char_blitter<"[]"> emptyArray {};

		template<template<typename, auto> typename value_type_new, typename value_type_internal, auto size>
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
					*context.bufferPtr = '[';
					++context.bufferPtr;
				}

				auto iter = getBeginIterVec(value);
				serialize<options>::impl(iter[0], context);
				if constexpr (newSize > 1) {
					for (uint64_t index{ 1 }; index != newSize; ++index) {
						if constexpr (options.prettify) {
							comma_indent::blitWithOverflow(context.bufferPtr, context.indent);
						} else {
							*context.bufferPtr = ',';
							++context.bufferPtr;
						}
						serialize<options>::impl(iter[index], context);
					}
				}

				if constexpr (options.prettify) {
					context.indent -= options.indentSize;
					close_indent::blitWithOverflow(context.bufferPtr, context.indent);
				}
				*context.bufferPtr = ']';
				++context.bufferPtr;
			} else {
				std::memcpy(context.bufferPtr, &emptyArray.value, emptyArray.lengthToCopy);
				context.bufferPtr += emptyArray.lengthToAdvance;
			}
		}
	};

	template<concepts::tuple_t value_type, typename context_type, serialize_options options> struct serialize_impl<value_type, context_type, options> {		
		using open_indent						 = indent_table<"[\n", options.indentChar, options.indentSize>;
		using comma_indent						 = indent_table<",\n", options.indentChar, options.indentSize>;
		using close_indent						 = indent_table<"\n", options.indentChar, options.indentSize>;
		JSONIFIER_ALIGN(8) static constexpr char_blitter<"[]"> emptyArray {};
		static constexpr auto memberCount = tuple_size_v<value_type>;
		template<auto... values> struct tuple_member_serializer {
			template<uint64_t index, typename value_type_new> JSONIFIER_INLINE static void impl(value_type_new&& value, context_type& context) noexcept {
				if constexpr (options.prettify) {
					comma_indent::blitWithOverflow(context.bufferPtr, context.indent);
				} else {
					*context.bufferPtr = ',';
					++context.bufferPtr;
				}
				serialize<options>::impl(get<index>(value), context);
			}
		};
		template<typename value_type_new> inline static void impl(value_type_new&& value, context_type& context) noexcept {
			static constexpr auto additionalSize{ getPaddingSize<options, value_type>() };
			ensureCapacity<options>(context, additionalSize);
			if constexpr (memberCount > 0) {
				*context.bufferPtr = '[';
				++context.bufferPtr;
				if constexpr (options.prettify) {
					context.indent += options.indentSize;
					open_indent::blitWithOverflow(context.bufferPtr, context.indent);
				}
				serialize<options>::impl(get<0>(value), context);
				if constexpr (memberCount > 1) {
					functor_runner<tuple_member_serializer, offset_sequence<make_integer_sequence<memberCount - 1>, 1>>::impl(value, context);
				}
				if constexpr (options.prettify) {
					context.indent -= options.indentSize;
					close_indent::blitWithOverflow(context.bufferPtr, context.indent);
				}
				*context.bufferPtr = ']';
				++context.bufferPtr;
			} else {
				std::memcpy(context.bufferPtr, &emptyArray.value, emptyArray.lengthToCopy);
				context.bufferPtr += emptyArray.lengthToAdvance;
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
				*context.bufferPtr = '"';
				++context.bufferPtr;
				context.bufferPtr  = string_serializer<options, decltype(value.data()), decltype(context.bufferPtr)>::impl(value.data(), context.bufferPtr, value.size());
				*context.bufferPtr = '"';
				++context.bufferPtr;
			} else {
				std::memcpy(context.bufferPtr, packedValues01, 2);
				context.bufferPtr += 2;
			}
		}
	};

	template<concepts::char_t value_type, typename context_type, serialize_options options> struct serialize_impl<value_type, context_type, options> {
		template<typename value_type_new> JSONIFIER_INLINE static void impl(value_type_new&& value, context_type& context) noexcept {
			*context.bufferPtr = '"';
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
			*context.bufferPtr = '"';
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
				if constexpr (concepts::uint_types<std::remove_cvref_t<value_type_new>>) {
					context.bufferPtr = to_chars<std::remove_cvref_t<uint64_t>>::impl(context.bufferPtr, static_cast<uint64_t>(value));
				} else if constexpr (concepts::int_types<value_type>) {
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
					return 7377296907481120768ULL;
				}
			}() };
			static constexpr uint64_t trueVInt{ [] {
				if constexpr (std::endian::native == std::endian::little) {
					return 434025983730ULL;
				} else {
					return 17433142848793870336ULL;
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
