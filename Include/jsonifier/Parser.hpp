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
/// Feb 3, 2023
#pragma once

#include <jsonifier/Validator.hpp>
#include <jsonifier/HashMap.hpp>
#include <jsonifier/String.hpp>
#include <jsonifier/Error.hpp>
#include <jsonifier/Simd.hpp>

namespace jsonifier_internal {

	JSONIFIER_FORCE_INLINE void printIterValues(auto iter, const std::source_location& title = std::source_location::current()) {
		//std::cout<< "File: " << title.file_name() << ", Line: " << title.line() << std::endl;
		//std::cout<< "Values: " << jsonifier::string_view{ iter, 32 } << std::endl;
	}

	template<typename derived_type> class parser;

	template<typename derived_type, typename iterator_t> struct parse_context {
		JSONIFIER_INLINE constexpr parse_context() noexcept = default;

		parser<derived_type>* parserPtr{};
		int64_t currentObjectDepth{};
		int64_t currentArrayDepth{};
		iterator_t rootIter{};
		iterator_t endIter{};
		iterator_t iter{};
	};

	template<typename value_type> constexpr bool areWeInsideRepeated() {
		return jsonifier::concepts::map_t<value_type> || jsonifier::concepts::vector_t<value_type> || jsonifier::concepts::raw_array_t<value_type>;
	}

	template<typename derived_type, typename iterator_type> struct parse_context_partial {
		JSONIFIER_INLINE constexpr parse_context_partial() noexcept = default;

		JSONIFIER_FORCE_INLINE bool getState() {
			return remainingMemberCount > 0 && (currentArrayDepth > 0 || currentObjectDepth > 0);
		}

		parser<derived_type>* parserPtr{};
		int64_t remainingMemberCount{};
		int64_t currentObjectDepth{};
		int64_t currentArrayDepth{};
		iterator_type rootIter{};
		iterator_type endIter{};
		iterator_type iter{};
	};

	template<typename value_type>
	concept partial_reading_context_t = requires(std::remove_cvref_t<value_type> value) {
		{ value.getState() } -> std::same_as<bool>;
	};

	template<jsonifier::concepts::pointer_t value_type> JSONIFIER_FORCE_INLINE string_view_ptr getEndIter(value_type value) noexcept {
		return reinterpret_cast<string_view_ptr>(char_comparison<'\0', decltype(*value)>::memchar(value, std::numeric_limits<size_t>::max()));
	}

	template<jsonifier::concepts::pointer_t value_type> JSONIFIER_FORCE_INLINE string_view_ptr getBeginIter(value_type value) noexcept {
		return reinterpret_cast<string_view_ptr>(value);
	}

	template<jsonifier::concepts::has_data value_type> JSONIFIER_FORCE_INLINE string_view_ptr getEndIter(value_type& value) noexcept {
		return reinterpret_cast<string_view_ptr>(value.data() + value.size());
	}

	template<jsonifier::concepts::has_data value_type> JSONIFIER_FORCE_INLINE string_view_ptr getBeginIter(value_type& value) noexcept {
		return reinterpret_cast<string_view_ptr>(value.data());
	}

	template<typename value_type, typename buffer_type, typename context_type, size_t maxDepth, size_t depth = 0, jsonifier::parse_options optionsNew = jsonifier::parse_options{},
		bool minified = false>
	struct object_val_parser;
	template<typename value_type, typename buffer_type, typename context_type, size_t maxDepth, size_t depth = 0, jsonifier::parse_options optionsNew = jsonifier::parse_options{},
		bool insideRepeated = false>
	struct object_val_parser_partial;

	template<typename value_type, typename buffer_type, typename context_type, size_t maxDepth, size_t depth = 0, jsonifier::parse_options optionsNew = jsonifier::parse_options{},
		bool minified = false>
	struct array_val_parser;
	template<typename value_type, typename buffer_type, typename context_type, size_t maxDepth, size_t depth = 0, jsonifier::parse_options optionsNew = jsonifier::parse_options{},
		bool insideRepeated = false>
	struct array_val_parser_partial;

	template<typename value_type, typename buffer_type, typename context_type, size_t maxDepth, size_t depth = 0, jsonifier::parse_options optionsNew = jsonifier::parse_options{},
		bool minified = false>
	struct string_val_parser;
	template<typename value_type, typename buffer_type, typename context_type, size_t maxDepth, size_t depth = 0, jsonifier::parse_options optionsNew = jsonifier::parse_options{},
		bool insideRepeated = false>
	struct string_val_parser_partial;

	template<typename value_type, typename buffer_type, typename context_type, size_t maxDepth, size_t depth = 0, jsonifier::parse_options optionsNew = jsonifier::parse_options{},
		bool minified = false>
	struct number_val_parser;
	template<typename value_type, typename buffer_type, typename context_type, size_t maxDepth, size_t depth = 0, jsonifier::parse_options optionsNew = jsonifier::parse_options{},
		bool insideRepeated = false>
	struct number_val_parser_partial;

	template<typename value_type, typename buffer_type, typename context_type, size_t maxDepth, size_t depth = 0, jsonifier::parse_options optionsNew = jsonifier::parse_options{},
		bool minified = false>
	struct bool_val_parser;
	template<typename value_type, typename buffer_type, typename context_type, size_t maxDepth, size_t depth = 0, jsonifier::parse_options optionsNew = jsonifier::parse_options{},
		bool insideRepeated = false>
	struct bool_val_parser_partial;

	template<typename value_type, typename buffer_type, typename context_type, size_t maxDepth, size_t depth = 0, jsonifier::parse_options optionsNew = jsonifier::parse_options{},
		bool minified = false>
	struct null_val_parser;
	template<typename value_type, typename buffer_type, typename context_type, size_t maxDepth, size_t depth = 0, jsonifier::parse_options optionsNew = jsonifier::parse_options{},
		bool insideRepeated = false>
	struct null_val_parser_partial;

	template<typename value_type, typename buffer_type, typename context_type, size_t maxDepth, size_t depth = 0, jsonifier::parse_options optionsNew = jsonifier::parse_options{},
		bool minified = false>

	struct accessor_val_parser;
	template<typename value_type, typename buffer_type, typename context_type, size_t maxDepth, size_t depth = 0, jsonifier::parse_options optionsNew = jsonifier::parse_options{},
		bool insideRepeated = false>
	struct accessor_val_parser_partial;

	template<typename value_type, typename buffer_type, typename context_type, size_t maxDepth, size_t depth = 0, jsonifier::parse_options optionsNew = jsonifier::parse_options{},
		bool insideRepeated = false>
	struct custom_val_parser;
	template<typename value_type, typename buffer_type, typename context_type, size_t maxDepth, size_t depth = 0, jsonifier::parse_options optionsNew = jsonifier::parse_options{},
		bool insideRepeated = false>
	struct custom_val_parser_partial;

	template<typename value_type> static constexpr auto getJsonType() {
		if constexpr (jsonifier::concepts::jsonifier_object_t<value_type> || jsonifier::concepts::map_t<value_type>) {
			return jsonifier::json_type::object;
		} else if constexpr (jsonifier::concepts::raw_array_t<value_type> || jsonifier::concepts::tuple_t<value_type> || jsonifier::concepts::vector_t<value_type>) {
			return jsonifier::json_type::array;
		} else if constexpr (jsonifier::concepts::string_t<value_type> || jsonifier::concepts::string_view_t<value_type>) {
			return jsonifier::json_type::string;
		} else if constexpr (jsonifier::concepts::bool_t<value_type>) {
			return jsonifier::json_type::boolean;
		} else if constexpr (jsonifier::concepts::num_t<value_type> || jsonifier::concepts::enum_t<value_type>) {
			return jsonifier::json_type::number;
		} else if constexpr (jsonifier::concepts::always_null_t<value_type>) {
			return jsonifier::json_type::null;
		} else if constexpr (jsonifier::concepts::accessor_t<value_type>) {
			return jsonifier::json_type::accessor;
		} else {
			return jsonifier::json_type::custom;
		}
	}

	template<typename value_type, size_t currentIndex = 0> constexpr size_t countTotalNonRepeatedMembers(size_t currentCount = 1) {
		if constexpr (currentIndex < tuple_size_v<core_tuple_type<value_type>>) {
			constexpr auto newSubTuple = jsonifier_internal::get<currentIndex>(jsonifier::core<std::remove_cvref_t<value_type>>::parseValue);
			using member_type		   = typename std::remove_cvref_t<decltype(newSubTuple)>::member_type;
			if constexpr (jsonifier::concepts::jsonifier_object_t<member_type>) {
				currentCount += countTotalNonRepeatedMembers<member_type>();
			}
			++currentCount;
			return countTotalNonRepeatedMembers<std::remove_cvref_t<value_type>, currentIndex + 1>(currentCount);
		} else {
			return currentCount;
		}
	}

	template<jsonifier ::json_type type, size_t maxDepth, size_t depth, jsonifier::parse_options options = jsonifier::parse_options{}, bool values = false,
		typename containing_type = void>
	struct parse;

	template<jsonifier::json_type type, size_t maxDepth, size_t depth, jsonifier::parse_options options, bool insideRepeated, typename containing_type> struct parse {
		template<typename buffer_type, typename value_type, typename context_type> JSONIFIER_INLINE static void impl(value_type&& value, context_type&& context) noexcept {
			if constexpr (depth < maxDepth) {
				if constexpr (options.partialRead) {
					if constexpr (type == jsonifier::json_type::object) {
						if constexpr (jsonifier::concepts::map_t<value_type> || insideRepeated) {
							object_val_parser_partial<std::remove_cvref_t<value_type>, buffer_type, context_type, maxDepth, depth, options, true>::impl(value, context);
						} else {
							object_val_parser_partial<std::remove_cvref_t<value_type>, buffer_type, context_type, maxDepth, depth, options, false>::impl(value, context);
						}
					} else if constexpr (type == jsonifier::json_type::array) {
						array_val_parser_partial<std::remove_cvref_t<value_type>, buffer_type, context_type, maxDepth, depth, options, true>::impl(value, context);
					} else if constexpr (type == jsonifier::json_type::string) {
						string_val_parser_partial<std::remove_cvref_t<value_type>, buffer_type, context_type, maxDepth, depth, options, insideRepeated>::impl(value, context);
					} else if constexpr (type == jsonifier::json_type::number) {
						number_val_parser_partial<std::remove_cvref_t<value_type>, buffer_type, context_type, maxDepth, depth, options, insideRepeated>::impl(value, context);
					} else if constexpr (type == jsonifier::json_type::boolean) {
						bool_val_parser_partial<std::remove_cvref_t<value_type>, buffer_type, context_type, maxDepth, depth, options, insideRepeated>::impl(value, context);
					} else if constexpr (type == jsonifier::json_type::null) {
						null_val_parser_partial<std::remove_cvref_t<value_type>, buffer_type, context_type, maxDepth, depth, options, insideRepeated>::impl(value, context);
					} else if constexpr (type == jsonifier::json_type::custom) {
						custom_val_parser_partial<std::remove_cvref_t<value_type>, buffer_type, context_type, maxDepth, depth, options, insideRepeated>::impl(value, context);
					} else {
						accessor_val_parser_partial<std::remove_cvref_t<value_type>, buffer_type, context_type, maxDepth, depth, options, insideRepeated>::impl(value, context);
					}
					if constexpr (!insideRepeated) {
						--context.remainingMemberCount;
					}
					if (!context.getState()) {
						context.iter = context.endIter;
						return;
					}
				} else {
					if constexpr (type == jsonifier::json_type::object) {
						object_val_parser<std::remove_cvref_t<value_type>, buffer_type, context_type, maxDepth, depth, options, insideRepeated>::impl(value, context);
					} else if constexpr (type == jsonifier::json_type::array) {
						array_val_parser<std::remove_cvref_t<value_type>, buffer_type, context_type, maxDepth, depth, options, insideRepeated>::impl(value, context);
					} else if constexpr (type == jsonifier::json_type::string) {
						string_val_parser<std::remove_cvref_t<value_type>, buffer_type, context_type, maxDepth, depth, options, insideRepeated>::impl(value, context);
					} else if constexpr (type == jsonifier::json_type::number) {
						number_val_parser<std::remove_cvref_t<value_type>, buffer_type, context_type, maxDepth, depth, options, insideRepeated>::impl(value, context);
					} else if constexpr (type == jsonifier::json_type::boolean) {
						bool_val_parser<std::remove_cvref_t<value_type>, buffer_type, context_type, maxDepth, depth, options, insideRepeated>::impl(value, context);
					} else if constexpr (type == jsonifier::json_type::null) {
						null_val_parser<std::remove_cvref_t<value_type>, buffer_type, context_type, maxDepth, depth, options, insideRepeated>::impl(value, context);
					} else if constexpr (type == jsonifier::json_type::custom) {
						custom_val_parser<std::remove_cvref_t<value_type>, buffer_type, context_type, maxDepth, depth, options, insideRepeated>::impl(value, context);
					} else {
						accessor_val_parser<std::remove_cvref_t<value_type>, buffer_type, context_type, maxDepth, depth, options, insideRepeated>::impl(value, context);
					}
				}
			}
		}

		template<typename buffer_type, typename value_type, typename context_type>
		JSONIFIER_FORCE_INLINE static void implForceInline(value_type&& value, context_type&& context) noexcept {
			if constexpr (depth < maxDepth) {
				if constexpr (options.partialRead) {
					if constexpr (type == jsonifier::json_type::object) {
						if constexpr (insideRepeated) {
							if constexpr (jsonifier::concepts::jsonifier_object_t<value_type>) {
								object_val_parser_partial<std::remove_cvref_t<value_type>, buffer_type, context_type, maxDepth, depth, options, true>::implForceInline(value,
									context);
							} else {
								object_val_parser_partial<std::remove_cvref_t<value_type>, buffer_type, context_type, maxDepth, depth, options, true>::impl(value, context);
							}
						} else {
							if constexpr (jsonifier::concepts::jsonifier_object_t<value_type>) {
								object_val_parser_partial<std::remove_cvref_t<value_type>, buffer_type, context_type, maxDepth, depth, options, false>::implForceInline(value,
									context);
							} else {
								object_val_parser_partial<std::remove_cvref_t<value_type>, buffer_type, context_type, maxDepth, depth, options, false>::impl(value, context);
							}
						}
					} else if constexpr (type == jsonifier::json_type::array) {
						array_val_parser_partial<std::remove_cvref_t<value_type>, buffer_type, context_type, maxDepth, depth, options, true>::impl(value, context);
					} else if constexpr (type == jsonifier::json_type::string) {
						string_val_parser_partial<std::remove_cvref_t<value_type>, buffer_type, context_type, maxDepth, depth, options, insideRepeated>::impl(value, context);
					} else if constexpr (type == jsonifier::json_type::number) {
						number_val_parser_partial<std::remove_cvref_t<value_type>, buffer_type, context_type, maxDepth, depth, options, insideRepeated>::impl(value, context);
					} else if constexpr (type == jsonifier::json_type::boolean) {
						bool_val_parser_partial<std::remove_cvref_t<value_type>, buffer_type, context_type, maxDepth, depth, options, insideRepeated>::impl(value, context);
					} else if constexpr (type == jsonifier::json_type::null) {
						null_val_parser_partial<std::remove_cvref_t<value_type>, buffer_type, context_type, maxDepth, depth, options, insideRepeated>::impl(value, context);
					} else if constexpr (type == jsonifier::json_type::custom) {
						custom_val_parser_partial<std::remove_cvref_t<value_type>, buffer_type, context_type, maxDepth, depth, options, insideRepeated>::impl(value, context);
					} else {
						accessor_val_parser_partial<std::remove_cvref_t<value_type>, buffer_type, context_type, maxDepth, depth, options, insideRepeated>::impl(value, context);
					}
					if constexpr (!insideRepeated) {
						--context.remainingMemberCount;
					}
					if (!context.getState()) {
						context.iter = context.endIter;
						return;
					}
				} else {
					if constexpr (type == jsonifier::json_type::object) {
						if constexpr (jsonifier::concepts::jsonifier_object_t<value_type>) {
							object_val_parser<std::remove_cvref_t<value_type>, buffer_type, context_type, maxDepth, depth, options, insideRepeated>::implForceInline(value,
								context);
						} else {
							object_val_parser<std::remove_cvref_t<value_type>, buffer_type, context_type, maxDepth, depth, options, insideRepeated>::impl(value, context);
						}
					} else if constexpr (type == jsonifier::json_type::array) {
						array_val_parser<std::remove_cvref_t<value_type>, buffer_type, context_type, maxDepth, depth, options, insideRepeated>::impl(value, context);
					} else if constexpr (type == jsonifier::json_type::string) {
						string_val_parser<std::remove_cvref_t<value_type>, buffer_type, context_type, maxDepth, depth, options, insideRepeated>::impl(value, context);
					} else if constexpr (type == jsonifier::json_type::number) {
						number_val_parser<std::remove_cvref_t<value_type>, buffer_type, context_type, maxDepth, depth, options, insideRepeated>::impl(value, context);
					} else if constexpr (type == jsonifier::json_type::boolean) {
						bool_val_parser<std::remove_cvref_t<value_type>, buffer_type, context_type, maxDepth, depth, options, insideRepeated>::impl(value, context);
					} else if constexpr (type == jsonifier::json_type::null) {
						null_val_parser<std::remove_cvref_t<value_type>, buffer_type, context_type, maxDepth, depth, options, insideRepeated>::impl(value, context);
					} else if constexpr (type == jsonifier::json_type::custom) {
						custom_val_parser<std::remove_cvref_t<value_type>, buffer_type, context_type, maxDepth, depth, options, insideRepeated>::impl(value, context);
					} else {
						accessor_val_parser<std::remove_cvref_t<value_type>, buffer_type, context_type, maxDepth, depth, options, insideRepeated>::impl(value, context);
					}
				}
			}
		}
	};

	template<typename derived_type_new> class parser {
	  public:
		using derived_type = derived_type_new;

		parser& operator=(const parser& other) = delete;
		parser(const parser& other)			   = delete;

		template<jsonifier::parse_options options = jsonifier::parse_options{}, typename value_type, typename buffer_type>
		JSONIFIER_FORCE_INLINE bool parseJson(value_type&& object, buffer_type&& in) noexcept {
			if constexpr (options.partialRead) {
				static constexpr jsonifier::parse_options optionsNew{ options };
				parse_context_partial<derived_type, const char**> context{};
				auto rootIter = getBeginIter(in);
				auto endIter  = getEndIter(in);
				section.reset<options.minified>(rootIter, static_cast<size_t>(endIter - rootIter));
				context.rootIter			 = section.begin();
				context.iter				 = section.begin();
				context.endIter				 = section.end();
				context.remainingMemberCount = static_cast<int64_t>(countTotalNonRepeatedMembers<value_type>());
				context.parserPtr			 = this;
				auto newSize				 = static_cast<uint64_t>((context.endIter - context.iter) / 2);
				if (stringBuffer.size() < newSize) {
					stringBuffer.resize(newSize);
				}
				if constexpr (options.validateJson) {
					if (!derivedRef.validateJson(in)) {
						return false;
					}
				}
				derivedRef.errors.clear();
				if JSONIFIER_UNLIKELY (!context.iter) {
					reportError<parse_errors::No_Input>(context);
					return false;
				}
				if constexpr (is_core_type<value_type>) {
					parse<core_type<value_type>::type, maxFieldDepth<value_type>(), 0, optionsNew, areWeInsideRepeated<value_type>()>::template impl<buffer_type>(object, context);
				} else {
					parse<getJsonType<value_type>(), maxFieldDepth<value_type>(), 0, optionsNew, areWeInsideRepeated<value_type>()>::template impl<buffer_type>(object, context);
				}
				return derivedRef.errors.size() > 0 ? false : true;
			} else {
				static constexpr jsonifier::parse_options optionsNew{ options };
				parse_context<derived_type, const char*> context{};
				context.rootIter  = getBeginIter(in);
				context.iter	  = context.rootIter;
				context.endIter	  = getEndIter(in);
				context.parserPtr = this;
				auto newSize	  = static_cast<uint64_t>((context.endIter - context.iter) / 2);
				if (stringBuffer.size() < newSize) {
					stringBuffer.resize(newSize);
				}
				if constexpr (options.validateJson) {
					if (!derivedRef.validateJson(in)) {
						return false;
					}
				}
				derivedRef.errors.clear();
				if JSONIFIER_UNLIKELY (!context.iter) {
					reportError<parse_errors::No_Input>(context);
					return false;
				}
				if constexpr (is_core_type<value_type>) {
					parse<core_type<value_type>::type, maxFieldDepth<value_type>(), 0, optionsNew, options.minified>::template impl<buffer_type>(object, context);
				} else {
					parse<getJsonType<value_type>(), maxFieldDepth<value_type>(), 0, optionsNew, options.minified>::template impl<buffer_type>(object, context);
				}
				return (context.currentObjectDepth != 0) ? (reportError<parse_errors::Imbalanced_Object_Braces>(context), false)
					: (context.currentArrayDepth != 0)	 ? (reportError<parse_errors::Imbalanced_Array_Brackets>(context), false)
					: (context.iter < context.endIter)	 ? (reportError<parse_errors::Unfinished_Input>(context), false)
					: derivedRef.errors.size() > 0		 ? false
														 : true;
			}
		}

		template<typename value_type, jsonifier::parse_options options = jsonifier::parse_options{}, typename buffer_type>
		JSONIFIER_FORCE_INLINE bool parseManyJson(value_type&& object, buffer_type&& in) noexcept {
			static constexpr jsonifier::parse_options optionsNew{ .validateJson = options.validateJson,
				.partialRead													= false,
				.knownOrder														= options.knownOrder,
				.minified														= options.minified };
			parse_context<derived_type, const char*> context{};
			context.rootIter  = getBeginIter(in);
			context.iter	  = context.rootIter;
			context.endIter	  = getEndIter(in);
			context.parserPtr = this;
			auto newSize	  = static_cast<uint64_t>((context.endIter - context.iter) / 2);
			if (stringBuffer.size() < newSize) {
				stringBuffer.resize(newSize);
			}
			if constexpr (options.validateJson) {
				if (!derivedRef.validateJson(in)) {
					return false;
				}
			}
			derivedRef.errors.clear();
			if JSONIFIER_UNLIKELY (!context.iter) {
				reportError<parse_errors::No_Input>(context);
				return false;
			}
			while (context.iter < context.endIter) {
				if constexpr (is_core_type<value_type>) {
					parse<core_type<value_type>::type, maxFieldDepth<value_type>(), 0, optionsNew, options.minified>::template impl<buffer_type>(object.emplace_back(), context);
				} else {
					parse<getJsonType<value_type>(), maxFieldDepth<value_type>(), 0, optionsNew, options.minified>::template impl<buffer_type>(object.emplace_back(), context);
				}
			}
			return (context.currentObjectDepth != 0)						  ? (reportError<parse_errors::Imbalanced_Object_Braces>(context), false)
				: (context.currentArrayDepth != 0)							  ? (reportError<parse_errors::Imbalanced_Array_Brackets>(context), false)
				: (context.iter < context.endIter && !optionsNew.partialRead) ? (reportError<parse_errors::Unfinished_Input>(context), false)
				: derivedRef.errors.size() > 0								  ? false
																			  : true;
		}

		template<typename value_type, jsonifier::parse_options options = jsonifier::parse_options{}, jsonifier::concepts::string_t buffer_type>
		JSONIFIER_FORCE_INLINE value_type parseJson(buffer_type&& in) noexcept {
			if constexpr (options.partialRead) {
				static constexpr jsonifier::parse_options optionsNew{ options };
				parse_context_partial<derived_type, const char**> context{};
				auto rootIter = getBeginIter(in);
				auto endIter  = getEndIter(in);
				section.reset<options.minified>(rootIter, static_cast<size_t>(endIter - rootIter));
				context.rootIter			 = section.begin();
				context.iter				 = section.begin();
				context.endIter				 = section.end();
				context.remainingMemberCount = countTotalNonRepeatedMembers<value_type>();
				context.parserPtr			 = this;
				auto newSize				 = static_cast<uint64_t>((context.endIter - context.iter) / 2);
				if (stringBuffer.size() < newSize) {
					stringBuffer.resize(newSize);
				}
				if constexpr (options.validateJson) {
					if (!derivedRef.validateJson(in)) {
						return std::remove_cvref_t<value_type>{};
					}
				}
				derivedRef.errors.clear();
				if JSONIFIER_UNLIKELY (!context.iter) {
					reportError<parse_errors::No_Input>(context);
					return std::remove_cvref_t<value_type>{};
				}
				value_type object{};
				if constexpr (is_core_type<value_type>) {
					parse<core_type<value_type>::type, maxFieldDepth<value_type>(), 0, optionsNew, areWeInsideRepeated<value_type>()>::template impl<buffer_type>(object, context);
				} else {
					parse<getJsonType<value_type>(), maxFieldDepth<value_type>(), 0, optionsNew, areWeInsideRepeated<value_type>()>::template impl<buffer_type>(object, context);
				}
				return derivedRef.errors.size() > 0 ? std::remove_cvref_t<value_type>{} : object;
			} else {
				static constexpr jsonifier::parse_options optionsNew{ options };
				parse_context<derived_type, const char*> context{};
				context.rootIter  = getBeginIter(in);
				context.iter	  = context.rootIter;
				context.endIter	  = getEndIter(in);
				context.parserPtr = this;
				auto newSize	  = static_cast<uint64_t>((context.endIter - context.iter) / 2);
				if (stringBuffer.size() < newSize) {
					stringBuffer.resize(newSize);
				}
				if constexpr (options.validateJson) {
					if (!derivedRef.validateJson(in)) {
						return std::remove_cvref_t<value_type>{};
					}
				}
				derivedRef.errors.clear();
				if JSONIFIER_UNLIKELY (!context.iter) {
					reportError<parse_errors::No_Input>(context);
					return std::remove_cvref_t<value_type>{};
				}
				value_type object{};
				if constexpr (is_core_type<value_type>) {
					parse<core_type<value_type>::type, maxFieldDepth<value_type>(), 0, optionsNew, options.minified>::template impl<buffer_type>(object, context);
				} else {
					parse<getJsonType<value_type>(), maxFieldDepth<value_type>(), 0, optionsNew, options.minified>::template impl<buffer_type>(object, context);
				}
				return (context.currentObjectDepth != 0) ? (reportError<parse_errors::Imbalanced_Object_Braces>(context), std::remove_cvref_t<value_type>{})
					: (context.currentArrayDepth != 0)	 ? (reportError<parse_errors::Imbalanced_Array_Brackets>(context), std::remove_cvref_t<value_type>{})
					: (context.iter < context.endIter && !optionsNew.partialRead) ? (reportError<parse_errors::Unfinished_Input>(context), std::remove_cvref_t<value_type>{})
					: derivedRef.errors.size() > 0								  ? std::remove_cvref_t<value_type>{}
																				  : object;
			}
		}

		template<auto parseError, typename context_type>
		JSONIFIER_INLINE void reportError(context_type& context, const std::source_location& sourceLocation = std::source_location::current()) noexcept {
			derivedRef.errors.emplace_back(error::constructError<error_classes::Parsing, parseError>(getUnderlyingPtr(context.iter) - getUnderlyingPtr(context.rootIter),
				getUnderlyingPtr(context.endIter) - getUnderlyingPtr(context.rootIter), getUnderlyingPtr(context.rootIter), sourceLocation));
		}

	  protected:
		derived_type& derivedRef{ initializeSelfRef() };

		JSONIFIER_INLINE parser() noexcept : derivedRef{ initializeSelfRef() } {};

		JSONIFIER_INLINE derived_type& initializeSelfRef() noexcept {
			return *static_cast<derived_type*>(this);
		}

		JSONIFIER_INLINE ~parser() noexcept = default;
	};
};