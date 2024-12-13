
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

	template<typename value_type> constexpr size_t getTotalMemberCount(size_t currentCount = 0) {
		if constexpr (jsonifier::concepts::jsonifier_object_t<value_type>) {
			constexpr auto numMembers = tuple_size_v<typename core_tuple_type<value_type>::core_type>;
			size_t count{ numMembers };
			constexpr auto newVal			   = [](auto& currentCountNew) constexpr {
				   constexpr auto sizeCollectLambda = [](const auto currentIndex, const auto maxIndex, auto& currentCountNew) {
					   if constexpr (currentIndex < maxIndex) {
						   constexpr auto subTuple = get<currentIndex>(jsonifier::concepts::coreV<value_type>);
						   using member_type = typename std::remove_cvref_t<decltype(get<currentIndex>(jsonifier::concepts::coreV<value_type>))>::member_type;
						   if constexpr (jsonifier::concepts::jsonifier_object_t<member_type>) {
							   currentCountNew += getTotalMemberCount<member_type>(currentCountNew);
						   } else {
							   currentCountNew += tuple_size_v<typename core_tuple_type<member_type>::core_type>;						   
						   }
					   }
				   };

				   forEach<numMembers, make_static<sizeCollectLambda>::value>(currentCountNew);

				   return;
			};
			newVal(count);

			return count;
		} 
	}

	template<jsonifier::serialize_options options, typename value_type> constexpr size_t getPaddingSize() noexcept {
		if constexpr (jsonifier::concepts::jsonifier_object_t<value_type>) {
			constexpr auto numMembers = tuple_size_v<typename core_tuple_type<value_type>::core_type>;
			constexpr auto newSize	  = []() constexpr {
				   size_t pair{};
				   constexpr auto sizeCollectLambda = [](const auto currentIndex, const auto maxIndex, auto& pairNew) {
					   if constexpr (currentIndex < maxIndex) {
						   constexpr auto subTuple	  = get<currentIndex>(jsonifier::concepts::coreV<value_type>);
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

				   forEach<numMembers, make_static<sizeCollectLambda>::value>(pair);

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
				return (jsonifier_internal::getPaddingSize<options, std::tuple_element_t<indices, tuple>>() + ... + 0);
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

	template<size_t maxIndex, jsonifier::serialize_options options> struct index_processor_serialize {
		template<size_t currentIndex, typename value_type, typename buffer_type, typename index_type, typename indent_type>
		JSONIFIER_ALWAYS_INLINE static auto processIndexLambda(const value_type& value, buffer_type& buffer, index_type& index, indent_type& indent) noexcept {
			if constexpr (currentIndex < maxIndex) {
				static constexpr auto subTuple	 = get<currentIndex>(jsonifier::concepts::coreV<value_type>);
				static constexpr auto numMembers = tuple_size_v<typename core_tuple_type<value_type>::core_type>;
				static constexpr auto key		 = subTuple.view();
				auto* dataPtr					 = buffer.data();
				if constexpr (jsonifier::concepts::has_excluded_keys<value_type>) {
					auto& keys = value.jsonifierExcludedKeys;
					if JSONIFIER_LIKELY (keys.find(static_cast<typename std::remove_reference_t<decltype(keys)>::key_type>(key)) != keys.end()) {
						return;
					}
				}
				static constexpr auto memberPtr	  = subTuple.ptr();
				static constexpr auto unQuotedKey = string_literal{ "\"" } + stringLiteralFromView<key.size()>(key);
				if constexpr (options.prettify) {
					static constexpr auto quotedKey	   = unQuotedKey + string_literal{ "\": " };
					static constexpr auto size		   = quotedKey.size();
					static constexpr auto quotedKeyPtr = quotedKey.data();
					std::memcpy(dataPtr + index, quotedKeyPtr, size);
					index += size;
				} else {
					static constexpr auto quotedKey	   = unQuotedKey + string_literal{ "\":" };
					static constexpr auto size		   = quotedKey.size();
					static constexpr auto quotedKeyPtr = quotedKey.data();
					std::memcpy(dataPtr + index, quotedKeyPtr, size);
					index += size;
				}

				serialize<options>::impl(value.*memberPtr, buffer, index, indent);
				if constexpr (currentIndex < numMembers - 1) {
					if constexpr (options.prettify) {
						dataPtr = buffer.data();
						static constexpr auto packedValues{ ",\n" };
						std::memcpy(dataPtr + index, packedValues, 2);
						index += 2;
						std::memset(dataPtr + index, ' ', indent * options.indentSize);
						index += indent;
					} else {
						buffer[index] = comma;
						++index;
					}
				}
			}
			return;
		}

		template<typename... arg_types, size_t... indices> JSONIFIER_INLINE static void executeIndices(std::index_sequence<indices...>, arg_types&&... args) {
			(processIndexLambda<indices>(std::forward<arg_types>(args)...), ...);
		}
	};

	template<jsonifier::serialize_options options, jsonifier::concepts::jsonifier_object_t value_type, jsonifier::concepts::buffer_like buffer_type, typename index_type,
		typename indent_type>
	struct serialize_impl<options, value_type, buffer_type, index_type, indent_type> {
		static constexpr char packedValues01[]{ "{\n" };
		static constexpr char packedValues02[]{ "{}" };

		template<typename value_type_new> JSONIFIER_ALWAYS_INLINE static void impl(value_type_new&& value, buffer_type& buffer, index_type& index, indent_type& indent) noexcept {
			static constexpr auto numMembers{ tuple_size_v<typename core_tuple_type<value_type>::core_type> };
			static constexpr auto paddingSize{ getPaddingSize<options, std::remove_cvref_t<value_type>>() };
			auto* dataPtr = buffer.data();
			if constexpr (numMembers > 0) {
				if constexpr (options.prettify) {
					const auto additionalSize = (paddingSize + (numMembers * indent));
					if (buffer.size() <= index + additionalSize) {
						buffer.resize((index + additionalSize) * 2);
						dataPtr = buffer.data();
					}
					indent += options.indentSize;
					std::memcpy(dataPtr + index, packedValues01, 2);
					index += 2;
					std::memset(dataPtr + index, options.indentChar, indent);
					index += indent;
				} else {
					if (buffer.size() <= index + paddingSize) {
						buffer.resize((index + paddingSize) * 2);
						dataPtr = buffer.data();
					}
					buffer[index] = lBrace;
					++index;
				}

				index_processor_serialize<numMembers, options>::executeIndices(std::make_index_sequence<numMembers>{}, value, buffer, index, indent);

				if constexpr (options.prettify) {
					dataPtr = buffer.data();
					indent -= options.indentSize;
					buffer[index] = newline;
					++index;
					std::memset(dataPtr + index, options.indentChar, indent);
					index += indent;
				}
				buffer[index] = rBrace;
				++index;
			} else {
				std::memcpy(dataPtr + index, packedValues02, 2);
				index += 2;
			}
		}
	};

	template<jsonifier::serialize_options options, jsonifier::concepts::jsonifier_scalar_value_t value_type, jsonifier::concepts::buffer_like buffer_type, typename index_type,
		typename indent_type>
	struct serialize_impl<options, value_type, buffer_type, index_type, indent_type> {
		template<typename value_type_new> JSONIFIER_ALWAYS_INLINE static void impl(value_type_new&& value, buffer_type& buffer, index_type& index, indent_type& indent) noexcept {
			static constexpr auto size{ tuple_size_v<typename core_tuple_type<value_type>::core_type> };
			if constexpr (size == 1) {
				static constexpr auto newPtr = get<0>(core_tuple_type<value_type>::coreTupleV);
				serialize<options>::impl(getMember<newPtr>(value), buffer, index, indent);
			}
		}
	};

	template<jsonifier::serialize_options options, jsonifier::concepts::map_t value_type, jsonifier::concepts::buffer_like buffer_type, typename index_type, typename indent_type>
	struct serialize_impl<options, value_type, buffer_type, index_type, indent_type> {
		static constexpr char packedValues01[]{ "{\n" };
		static constexpr char packedValues02[]{ ": " };
		static constexpr char packedValues03[]{ ",\n" };
		static constexpr char packedValues04[]{ "{}" };
		template<typename value_type_new> JSONIFIER_ALWAYS_INLINE static void impl(value_type_new&& value, buffer_type& buffer, index_type& index, indent_type& indent) noexcept {
			const auto newSize = value.size();
			static constexpr auto paddingSize{ getPaddingSize<options, typename std::remove_cvref_t<value_type>::mapped_type>() };
			auto* dataPtr = buffer.data();
			if JSONIFIER_LIKELY (newSize > 0) {
				if constexpr (options.prettify) {
					const auto additionalSize = newSize * (paddingSize + indent);
					if (buffer.size() <= index + additionalSize) {
						buffer.resize((index + additionalSize) * 2);
						dataPtr = buffer.data();
					}
					indent += options.indentSize;
					std::memcpy(dataPtr + index, packedValues01, 2);
					index += 2;
					std::memset(dataPtr + index, options.indentChar, indent);
					index += indent;
				} else {
					const auto additionalSize = newSize * paddingSize;
					if (buffer.size() <= index + additionalSize) {
						buffer.resize((index + additionalSize) * 2);
						dataPtr = buffer.data();
					}
					buffer[index] = lBrace;
					++index;
				}
				auto iter = value.begin();
				serialize<options>::impl(iter->first, buffer, index, indent);
				if constexpr (options.prettify) {
					dataPtr = buffer.data();
					std::memcpy(dataPtr + index, packedValues02, 2);
					index += 2;
				} else {
					buffer[index] = colon;
					++index;
				}
				serialize<options>::impl(iter->second, buffer, index, indent);
				++iter;
				const auto end = value.end();
				for (; iter != end; ++iter) {
					if constexpr (options.prettify) {
						dataPtr = buffer.data();
						std::memcpy(dataPtr + index, packedValues03, 2);
						index += 2;
						std::memset(dataPtr + index, options.indentChar, indent);
						index += indent;
					} else {
						buffer[index] = comma;
						++index;
					}
					serialize<options>::impl(iter->first, buffer, index, indent);
					if constexpr (options.prettify) {
						dataPtr = buffer.data();
						std::memcpy(dataPtr + index, packedValues02, 2);
						index += 2;
					} else {
						buffer[index] = colon;
						++index;
					}
					serialize<options>::impl(iter->second, buffer, index, indent);
				}
				if constexpr (options.prettify) {
					dataPtr = buffer.data();
					indent -= options.indentSize;
					buffer[index] = newline;
					++index;
					std::memset(dataPtr + index, options.indentChar, indent);
					index += indent;
				}
				buffer[index] = rBrace;
				++index;
			} else {
				std::memcpy(dataPtr + index, packedValues04, 2);
				index += 2;
			}
		}
	};

	template<jsonifier::serialize_options options, jsonifier::concepts::variant_t value_type, jsonifier::concepts::buffer_like buffer_type, typename index_type,
		typename indent_type>
	struct serialize_impl<options, value_type, buffer_type, index_type, indent_type> {
		template<typename value_type_new> JSONIFIER_ALWAYS_INLINE static void impl(value_type_new&& value, buffer_type& buffer, index_type& index, indent_type& indent) noexcept {
			static constexpr auto lambda = [](auto&& valueNewer, auto&& bufferNew, auto&& indexNew) {
				serialize<options>::impl(valueNewer, bufferNew, indexNew);
			};
			visit<lambda>(value, buffer, index, indent);
		}
	};

	template<jsonifier::serialize_options options, jsonifier::concepts::optional_t value_type, jsonifier::concepts::buffer_like buffer_type, typename index_type,
		typename indent_type>
	struct serialize_impl<options, value_type, buffer_type, index_type, indent_type> {
		template<typename value_type_new> JSONIFIER_ALWAYS_INLINE static void impl(value_type_new&& value, buffer_type& buffer, index_type& index, indent_type& indent) noexcept {
			if JSONIFIER_LIKELY (value) {
				serialize<options>::impl(*value, buffer, index, indent);
			} else {
				std::memcpy(buffer.data() + index, nullV, 4);
				index += 4;
			}
		}
	};

	template<jsonifier::serialize_options options, jsonifier::concepts::tuple_t value_type, jsonifier::concepts::buffer_like buffer_type, typename index_type, typename indent_type>
	struct serialize_impl<options, value_type, buffer_type, index_type, indent_type> {
		static constexpr char packedValues01[]{ "[]" };
		template<typename value_type_new> JSONIFIER_ALWAYS_INLINE static void impl(value_type_new&& value, buffer_type& buffer, index_type& index, indent_type& indent) noexcept {
			static constexpr auto additionalSize{ getPaddingSize<options, std::remove_cvref_t<value_type>>() };
			if (buffer.size() <= index + additionalSize) {
				buffer.resize((index + additionalSize) * 2);
			}
			static constexpr auto size = tuple_size_v<std::remove_reference_t<value_type>>;
			if constexpr (size > 0) {
				buffer[index] = lBracket;
				++index;
				if constexpr (options.prettify) {
					indent += options.indentSize;
					buffer[index] = newline;
					++index;
					std::memset(buffer.data() + index, options.indentChar, indent);
					index += indent;
				}
				serializeObjects<0, size>(value, buffer, index, indent);
				if constexpr (options.prettify) {
					indent -= options.indentSize;
					buffer[index] = newline;
					++index;
					std::memset(buffer.data() + index, options.indentChar, indent);
					index += indent;
				}
				buffer[index] = rBracket;
				++index;
			} else {
				std::memcpy(buffer.data() + index, packedValues01, 2);
				index += 2;
			}
		}

		template<size_t currentIndex, size_t newSize, typename value_type_new>
		JSONIFIER_ALWAYS_INLINE static void serializeObjects(value_type_new&& value, buffer_type& buffer, index_type& index, indent_type& indent) noexcept {
			if constexpr (currentIndex < newSize) {
				auto subTuple = get<currentIndex>(value);
				serialize<options>::impl(subTuple, buffer, index, indent);
				if constexpr (currentIndex < newSize - 1) {
					if constexpr (options.prettify) {
						std::memcpy(buffer.data() + index, ",\n", 2);
						index += 2;
						std::memset(buffer.data() + index, options.indentChar, indent * options.indentSize);
						index += indent;
					} else {
						buffer[index] = comma;
						++index;
					}
				}
				return serializeObjects<currentIndex + 1, newSize>(value, buffer, index, indent);
			}
		}
	};

	template<jsonifier::serialize_options options, jsonifier::concepts::vector_t value_type, jsonifier::concepts::buffer_like buffer_type, typename index_type,
		typename indent_type>
	struct serialize_impl<options, value_type, buffer_type, index_type, indent_type> {
		static constexpr char packedValues01[]{ "[\n" };
		static constexpr char packedValues02[]{ ",\n" };
		static constexpr char packedValues03[]{ "[]" };
		template<typename value_type_new> JSONIFIER_ALWAYS_INLINE static void impl(value_type_new&& value, buffer_type& buffer, index_type& index, indent_type& indent) noexcept {
			const auto newSize = value.size();
			static constexpr auto paddingSize{ getPaddingSize<options, typename std::remove_cvref_t<value_type>::value_type>() };
			auto* dataPtr = buffer.data();
			if JSONIFIER_LIKELY (newSize > 0) {
				if constexpr (options.prettify) {
					const auto additionalSize = newSize * (paddingSize + indent);
					if (buffer.size() <= index + additionalSize) {
						buffer.resize((index + additionalSize) * 2);
						dataPtr = buffer.data();
					}
					indent += options.indentSize;
					std::memcpy(dataPtr + index, packedValues01, 2);
					index += 2;
					std::memset(dataPtr + index, options.indentChar, indent);
					index += indent;
				} else {
					const auto additionalSize = newSize * paddingSize;
					if (buffer.size() <= index + additionalSize) {
						buffer.resize((index + additionalSize) * 2);
					}
					buffer[index] = lBracket;
					++index;
				}
				auto iter = std::begin(value);
				serialize<options>::impl(*iter, buffer, index, indent);
				++iter;
				const auto end = std::end(value);
				for (; iter != end; ++iter) {
					if constexpr (options.prettify) {
						dataPtr = buffer.data();
						std::memcpy(dataPtr + index, packedValues02, 2);
						index += 2;
						std::memset(dataPtr + index, options.indentChar, indent);
						index += indent;
					} else {
						buffer[index] = comma;
						++index;
					}
					serialize<options>::impl(*iter, buffer, index, indent);
				}
				if constexpr (options.prettify) {
					dataPtr = buffer.data();
					indent -= options.indentSize;
					buffer[index] = newline;
					++index;
					std::memset(dataPtr + index, options.indentChar, indent);
					index += indent;
				}
				buffer[index] = rBracket;
				++index;
			} else {
				std::memcpy(dataPtr + index, packedValues03, 2);
				index += 2;
			}
		}
	};

	template<jsonifier::serialize_options options, jsonifier::concepts::pointer_t value_type, jsonifier::concepts::buffer_like buffer_type, typename index_type,
		typename indent_type>
	struct serialize_impl<options, value_type, buffer_type, index_type, indent_type> {
		template<typename value_type_new> JSONIFIER_ALWAYS_INLINE static void impl(value_type_new&& value, buffer_type& buffer, index_type& index, indent_type& indent) noexcept {
			if (value) {
				serialize<options>::impl(*value, buffer, index, indent);
			} else {
				std::memcpy(buffer.data() + index, nullV, 4);
				index += 4;
			}
		}
	};

	template<jsonifier::serialize_options options, jsonifier::concepts::raw_array_t value_type, jsonifier::concepts::buffer_like buffer_type, typename index_type,
		typename indent_type>
	struct serialize_impl<options, value_type, buffer_type, index_type, indent_type> {
		static constexpr char packedValues01[]{ "[\n" };
		static constexpr char packedValues02[]{ ",\n" };
		static constexpr char packedValues03[]{ "[]" };
		template<template<typename, size_t> typename value_type_new, typename value_type_internal, size_t size>
		JSONIFIER_ALWAYS_INLINE static void impl(const value_type_new<value_type_internal, size>& value, buffer_type& buffer, index_type& index, indent_type& indent) noexcept {
			constexpr auto newSize = size;
			static constexpr auto paddingSize{ getPaddingSize<options, typename std::remove_cvref_t<value_type>::value_type>() };
			if constexpr (newSize > 0) {
				if constexpr (options.prettify) {
					const auto additionalSize = newSize * (paddingSize + indent);
					if (buffer.size() <= index + additionalSize) {
						buffer.resize((index + additionalSize) * 2);
					}
					indent += options.indentSize;
					std::memcpy(buffer.data() + index, packedValues01, 2);
					index += 2;
					std::memset(buffer.data() + index, options.indentChar, indent);
					index += indent;
				} else {
					const auto additionalSize = newSize * paddingSize;
					if (buffer.size() <= index + additionalSize) {
						buffer.resize((index + additionalSize) * 2);
					}
					buffer[index] = lBracket;
					++index;
				}
				auto iter = std::begin(value);
				serialize<options>::impl(*iter, buffer, index, indent);
				++iter;
				const auto end = std::end(value);
				for (; iter != end; ++iter) {
					if constexpr (options.prettify) {
						std::memcpy(buffer.data() + index, packedValues02, 2);
						index += 2;
						std::memset(buffer.data() + index, options.indentChar, indent);
						index += indent;
					} else {
						buffer[index] = comma;
						++index;
					}
					serialize<options>::impl(*iter, buffer, index, indent);
				}
				if constexpr (options.prettify) {
					indent -= options.indentSize;
					buffer[index] = newline;
					++index;
					std::memset(buffer.data() + index, options.indentChar, indent);
					index += indent;
				}
				buffer[index] = rBracket;
				++index;
			} else {
				std::memcpy(buffer.data() + index, packedValues03, 2);
				index += 2;
			}
		}
	};

	template<jsonifier::serialize_options options, jsonifier::concepts::raw_json_t value_type, jsonifier::concepts::buffer_like buffer_type, typename index_type,
		typename indent_type>
	struct serialize_impl<options, value_type, buffer_type, index_type, indent_type> {
		template<typename value_type_new> JSONIFIER_ALWAYS_INLINE static void impl(value_type_new&& value, buffer_type& buffer, index_type& index, indent_type& indent) noexcept {
			serialize<options>::impl(value.rawJson(), buffer, index, indent);
		}
	};

	template<jsonifier::serialize_options options, jsonifier::concepts::string_t value_type, jsonifier::concepts::buffer_like buffer_type, typename index_type,
		typename indent_type>
	struct serialize_impl<options, value_type, buffer_type, index_type, indent_type> {
		static constexpr char packedValues01[]{ "\"\"" };
		template<typename value_type_new> JSONIFIER_ALWAYS_INLINE static void impl(value_type_new&& value, buffer_type& buffer, index_type& index, indent_type& indent) noexcept {
			const auto newSize = value.size();
			static constexpr auto paddingSize{ getPaddingSize<options, typename std::remove_cvref_t<value_type>::value_type>() };
			if (newSize > 0) {
				if constexpr (options.prettify) {
					const auto additionalSize = newSize * (paddingSize + indent);
					if (buffer.size() <= index + additionalSize) {
						buffer.resize((index + additionalSize) * 2);
					}
				} else {
					const auto additionalSize = newSize * paddingSize;
					if (buffer.size() <= index + additionalSize) {
						buffer.resize((index + additionalSize) * 2);
					}
				}
				buffer[index] = quote;
				++index;
				auto newPtr	  = string_serializer<options, decltype(value.data()), decltype(buffer.data() + index)>::impl(value.data(), buffer.data() + index, value.size());
				index		  = static_cast<size_t>(newPtr - buffer.data());
				buffer[index] = quote;
				++index;
			} else {
				std::memcpy(buffer.data() + index, packedValues01, 2);
				index += 2;
			}
		}
	};

	template<jsonifier::serialize_options options, jsonifier::concepts::char_t value_type, jsonifier::concepts::buffer_like buffer_type, typename index_type, typename indent_type>
	struct serialize_impl<options, value_type, buffer_type, index_type, indent_type> {
		template<typename value_type_new> JSONIFIER_ALWAYS_INLINE static void impl(value_type_new&& value, buffer_type& buffer, index_type& index, indent_type& indent) noexcept {
			buffer[index] = quote;
			++index;
			switch (value) {
				[[unlikely]] case '\b': {
					std::memcpy(buffer.data() + index, R"(\b)", 2);
					index += 2;
					break;
				}
				[[unlikely]] case '\t': {
					std::memcpy(buffer.data() + index, R"(\t)", 2);
					index += 2;
					break;
				}
				[[unlikely]] case '\n': {
					std::memcpy(buffer.data() + index, R"(\n)", 2);
					index += 2;
					break;
				}
				[[unlikely]] case '\f': {
					std::memcpy(buffer.data() + index, R"(\f)", 2);
					index += 2;
					break;
				}
				[[unlikely]] case '\r': {
					std::memcpy(buffer.data() + index, R"(\r)", 2);
					index += 2;
					break;
				}
				[[unlikely]] case '"': {
					std::memcpy(buffer.data() + index, R"(\")", 2);
					index += 2;
					break;
				}
				[[unlikely]] case '\\': {
					std::memcpy(buffer.data() + index, R"(\\)", 2);
					index += 2;
					break;
				}
				[[likely]] default: {
					buffer[index] = value;
					++index;
				}
			}
			buffer[index] = quote;
			++index;
		}
	};

	template<jsonifier::serialize_options options, jsonifier::concepts::shared_ptr_t value_type, jsonifier::concepts::buffer_like buffer_type, typename index_type,
		typename indent_type>
	struct serialize_impl<options, value_type, buffer_type, index_type, indent_type> {
		template<typename value_type_new> JSONIFIER_ALWAYS_INLINE static void impl(value_type_new&& value, buffer_type& buffer, index_type& index, indent_type& indent) noexcept {
			if JSONIFIER_LIKELY (value) {
				serialize<options>::impl(*value, buffer, index, indent);
			} else {
				std::memcpy(buffer.data() + index, nullV, 4);
				index += 4;
			}
		}
	};

	template<jsonifier::serialize_options options, jsonifier::concepts::unique_ptr_t value_type, jsonifier::concepts::buffer_like buffer_type, typename index_type,
		typename indent_type>
	struct serialize_impl<options, value_type, buffer_type, index_type, indent_type> {
		template<typename value_type_new> JSONIFIER_ALWAYS_INLINE static void impl(value_type_new&& value, buffer_type& buffer, index_type& index, indent_type& indent) noexcept {
			if JSONIFIER_LIKELY (value) {
				serialize<options>::impl(*value, buffer, index, indent);
			} else {
				std::memcpy(buffer.data() + index, nullV, 4);
				index += 4;
			}
		}
	};

	template<jsonifier::serialize_options options, jsonifier::concepts::enum_t value_type, jsonifier::concepts::buffer_like buffer_type, typename index_type, typename indent_type>
	struct serialize_impl<options, value_type, buffer_type, index_type, indent_type> {
		template<typename value_type_new> JSONIFIER_ALWAYS_INLINE static void impl(value_type_new&& value, buffer_type& buffer, index_type& index, indent_type& indent) noexcept {
			int64_t valueNew{ static_cast<int64_t>(value) };
			serialize<options>::impl(valueNew, buffer, index, indent);
		}
	};

	template<jsonifier::serialize_options options, jsonifier::concepts::always_null_t value_type, jsonifier::concepts::buffer_like buffer_type, typename index_type,
		typename indent_type>
	struct serialize_impl<options, value_type, buffer_type, index_type, indent_type> {
		template<typename value_type_new> JSONIFIER_ALWAYS_INLINE static void impl(value_type_new&&, buffer_type& buffer, index_type& index, indent_type&) noexcept {
			std::memcpy(buffer.data() + index, nullV, 4);
			index += 4;
		}
	};

	template<jsonifier::serialize_options options, jsonifier::concepts::bool_t value_type, jsonifier::concepts::buffer_like buffer_type, typename index_type, typename indent_type>
	struct serialize_impl<options, value_type, buffer_type, index_type, indent_type> {
		template<typename value_type_new> JSONIFIER_ALWAYS_INLINE static void impl(value_type_new&& value, buffer_type& buffer, index_type& index, indent_type&) noexcept {
			const uint64_t state = falseVInt - (value * trueVInt);
			std::memcpy(buffer.data() + index, &state, 5);
			index += 5 - value;
		}
	};

	template<jsonifier::serialize_options options, jsonifier::concepts::num_t value_type, jsonifier::concepts::buffer_like buffer_type, typename index_type, typename indent_type>
	struct serialize_impl<options, value_type, buffer_type, index_type, indent_type> {
		template<typename value_type_new> JSONIFIER_ALWAYS_INLINE static void impl(value_type_new&& value, buffer_type& buffer, index_type& index, indent_type&) noexcept {
			auto* dataPtr = buffer.data();
			if constexpr (sizeof(value_type) == 8) {
				index = static_cast<size_t>(toChars<std::remove_cvref_t<value_type>>(dataPtr + index, value) - dataPtr);
			} else {
				if constexpr (jsonifier::concepts::unsigned_t<value_type>) {
					index = static_cast<size_t>(toChars<uint64_t>(dataPtr + index, static_cast<uint64_t>(value)) - dataPtr);
				} else if constexpr (jsonifier::concepts::signed_t<value_type>) {
					index = static_cast<size_t>(toChars<int64_t>(dataPtr + index, static_cast<int64_t>(value)) - dataPtr);
				} else {
					index = static_cast<size_t>(toChars<float>(dataPtr + index, value) - dataPtr);
				}
			}
		}
	};
}