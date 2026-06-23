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

#include <jsonifier-incl/utilities/json_entity.hpp>
#include <jsonifier-incl/parsing/validator.hpp>
#include <jsonifier-incl/utilities/hash_map.hpp>
#include <jsonifier-incl/utilities/string.hpp>
#include <jsonifier-incl/utilities/error.hpp>
#include <jsonifier-incl/utilities/simd.hpp>

namespace jsonifier::internal {

	template<typename derived_type> class parser;

	template<typename derived_type, typename iterator_t> struct parse_context {
		constexpr parse_context() noexcept = default;

		mutable parser<derived_type>* parserPtr{};
		mutable int64_t currentObjectDepth{};
		mutable int64_t currentArrayDepth{};
		mutable iterator_t rootIter{};
		mutable iterator_t endIter{};
		mutable iterator_t iter{};
	};

	template<typename value_type> static constexpr bool areWeInsideRepeated() {
		return concepts::map_t<value_type> || concepts::vector_t<value_type> || concepts::raw_array_t<value_type>;
	}

	template<typename derived_type, typename iterator_type> struct parse_context_partial {
		constexpr parse_context_partial() noexcept = default;

		JSONIFIER_INLINE bool getState() const {
			return (currentArrayDepth > 0 || currentObjectDepth > 0);
		}

		mutable parser<derived_type>* parserPtr{};
		mutable int64_t currentObjectDepth{};
		mutable int64_t currentArrayDepth{};
		mutable iterator_type rootIter{};
		mutable iterator_type endIter{};
		mutable iterator_type iter{};
	};

	template<typename value_type>
	concept partial_reading_context_t = requires(jsonifier::internal::remove_cvref_t<value_type> value) {
		{ value.getState() } -> std::same_as<bool>;
	};

	JSONIFIER_INLINE constexpr uint64_t str_len(const char* input) noexcept {
		uint64_t return_val{};
		while (input[return_val] != '\0') {
			++return_val;
		}
		return return_val;
	}

	template<concepts::pointer_t value_type> JSONIFIER_INLINE static string_view_ptr getEndIter(value_type value) noexcept {
		return value + str_len(value);
	}

	template<concepts::pointer_t value_type> JSONIFIER_INLINE static string_view_ptr getBeginIter(value_type value) noexcept {
		return std::bit_cast<string_view_ptr>(value);
	}

	template<concepts::has_data value_type> JSONIFIER_INLINE static string_view_ptr getEndIter(value_type& value) noexcept {
		return std::bit_cast<string_view_ptr>(value.data() + value.size());
	}

	template<concepts::has_data value_type> JSONIFIER_INLINE static string_view_ptr getBeginIter(value_type& value) noexcept {
		return std::bit_cast<string_view_ptr>(value.data());
	}

	template<typename value_type, typename context_type, parse_options optionsNew, bool minified> struct parse_partial_impl;
	template<typename value_type, typename context_type, parse_options optionsNew, bool minified> struct parse_impl;

	template<parse_options options, bool minified> struct parse {
		template<typename value_type_new, typename context_type> JSONIFIER_INLINE static void impl(value_type_new&& value, context_type&& context) noexcept {
			using value_type = remove_cvref_t<value_type_new>;
			if constexpr (options.partialRead) {
				parse_partial_impl<value_type, context_type, options, minified>::impl(value, context);
				if (!context.getState()) {
					context.iter = context.endIter;
					return;
				}
			} else {
				parse_impl<value_type, context_type, options, options.minified>::impl(value, context);
			}
		}
	};

	template<typename derived_type_new> class parser {
	  public:
		friend class jsonifier::raw_json_data;
		template<const auto options, typename context_type> friend struct derailleur;

		using derived_type = derived_type_new;

		parser& operator=(const parser& other) = delete;
		parser(const parser& other)			   = delete;

		template<parse_options options = parse_options{}, typename value_type, typename buffer_type> inline bool parseJson(value_type&& object, buffer_type&& in) noexcept {
			if constexpr (options.partialRead) {
				static constexpr parse_options optionsNew{ options };
				constexpr parse_context_partial<derived_type, string_view_ptr*> context{ constEval(parse_context_partial<derived_type, string_view_ptr*>{}) };
				auto rootIter = getBeginIter(in);
				auto endIter  = getEndIter(in);
				derivedRef.section.template reset<options.minified>(rootIter, static_cast<uint64_t>(endIter - rootIter));
				context.rootIter  = derivedRef.section.begin();
				context.iter	  = derivedRef.section.begin();
				context.endIter	  = derivedRef.section.end();
				context.parserPtr = this;
				auto newSize	  = static_cast<uint64_t>((context.endIter - context.iter) / 2);
				if (derivedRef.stringBuffer.size() < newSize) {
					derivedRef.stringBuffer.resize(newSize);
				}
				if constexpr (options.validateJson) {
					if (!derivedRef.validateJson(in)) {
						return false;
					}
				}
				derivedRef.errors.clear();
				if JSONIFIER_UNLIKELY (!context.iter || context.iter == context.endIter) {
					reportError<parse_status::No_Input>(context);
					return false;
				}
				parse<optionsNew, areWeInsideRepeated<value_type>()>::impl(object, context);
				return derivedRef.errors.size() > 0 ? false : true;
			} else {
				static constexpr parse_options optionsNew{ options };
				constexpr parse_context<derived_type, string_view_ptr> context{ constEval(parse_context<derived_type, string_view_ptr>{}) };
				context.rootIter  = getBeginIter(in);
				context.iter	  = context.rootIter;
				context.endIter	  = getEndIter(in);
				context.parserPtr = this;
				auto newSize	  = static_cast<uint64_t>((context.endIter - context.iter) / 2);
				if (derivedRef.stringBuffer.size() < newSize) {
					derivedRef.stringBuffer.resize(newSize);
				}
				if constexpr (options.validateJson) {
					if (!derivedRef.validateJson(in)) {
						return false;
					}
				}
				derivedRef.errors.clear();
				if JSONIFIER_UNLIKELY (!context.iter || context.iter == context.endIter) {
					reportError<parse_status::No_Input>(context);
					return false;
				}
				parse<optionsNew, options.minified>::impl(object, context);
				return (context.currentObjectDepth != 0) ? (reportError<parse_status::Missing_Object_End>(context), false)
					: (context.currentArrayDepth != 0)	 ? (reportError<parse_status::Missing_Array_End>(context), false)
					: (context.iter < context.endIter)	 ? (reportError<parse_status::Unfinished_Input>(context), false)
					: derivedRef.errors.size() > 0		 ? false
														 : true;
			}
		}

		template<typename value_type, parse_options options = parse_options{}, typename buffer_type> inline bool parseManyJson(value_type&& object, buffer_type&& in) noexcept {
			static constexpr parse_options optionsNew{ options };
			constexpr parse_context_partial<derived_type, string_view_ptr*> context{ constEval(parse_context_partial<derived_type, string_view_ptr*>{}) };
			auto rootIter = getBeginIter(in);
			auto endIter  = getEndIter(in);
			derivedRef.section.template reset<options.minified>(rootIter, static_cast<uint64_t>(endIter - rootIter));
			context.rootIter  = derivedRef.section.begin();
			context.iter	  = derivedRef.section.begin();
			context.endIter	  = derivedRef.section.end();
			context.parserPtr = this;
			auto newSize	  = static_cast<uint64_t>((endIter - rootIter) / 2);
			if (derivedRef.stringBuffer.size() < newSize) {
				derivedRef.stringBuffer.resize(newSize);
			}
			if constexpr (options.validateJson) {
				if (!derivedRef.validateJson(in)) {
					return false;
				}
			}
			derivedRef.errors.clear();
			if JSONIFIER_UNLIKELY (context.iter >= context.endIter) {
				reportError<parse_status::No_Input>(context);
				return false;
			}
			while (context.iter < context.endIter) {
				auto before = context.iter;
				parse<optionsNew, options.minified>::impl(object.emplace_back(), context);
				if JSONIFIER_UNLIKELY (context.iter == before) {
					object.pop_back();
					reportError<parse_status::Unfinished_Input>(context);
					break;
				}
			}
			return (context.currentObjectDepth != 0)						  ? (reportError<parse_status::Missing_Object_End>(context), false)
				: (context.currentArrayDepth != 0)							  ? (reportError<parse_status::Missing_Array_End>(context), false)
				: (context.iter < context.endIter && !optionsNew.partialRead) ? (reportError<parse_status::Unfinished_Input>(context), false)
				: derivedRef.errors.size() > 0								  ? false
																			  : true;
		}

		template<typename value_type, parse_options options = parse_options{}, concepts::string_t buffer_type> inline value_type parseJson(buffer_type&& in) noexcept {
			if constexpr (options.partialRead) {
				static constexpr parse_options optionsNew{ options };
				constexpr parse_context_partial<derived_type, string_view_ptr*> context{ constEval(parse_context_partial<derived_type, string_view_ptr*>{}) };
				auto rootIter = getBeginIter(in);
				auto endIter  = getEndIter(in);
				derivedRef.section.template reset<options.minified>(rootIter, static_cast<uint64_t>(endIter - rootIter));
				context.rootIter  = derivedRef.section.begin();
				context.iter	  = derivedRef.section.begin();
				context.endIter	  = derivedRef.section.end();
				context.parserPtr = this;
				auto newSize	  = static_cast<uint64_t>((context.endIter - context.iter) / 2);
				if (derivedRef.stringBuffer.size() < newSize) {
					derivedRef.stringBuffer.resize(newSize);
				}
				if constexpr (options.validateJson) {
					if (!derivedRef.validateJson(in)) {
						return jsonifier::internal::remove_cvref_t<value_type>{};
					}
				}
				derivedRef.errors.clear();
				if JSONIFIER_UNLIKELY (!context.iter || context.iter == context.endIter) {
					reportError<parse_status::No_Input>(context);
					return jsonifier::internal::remove_cvref_t<value_type>{};
				}
				value_type object{};
				parse<optionsNew, areWeInsideRepeated<value_type>()>::impl(object, context);
				return derivedRef.errors.size() > 0 ? jsonifier::internal::remove_cvref_t<value_type>{} : object;
			} else {
				static constexpr parse_options optionsNew{ options };
				constexpr parse_context<derived_type, string_view_ptr> context{ constEval(parse_context<derived_type, string_view_ptr>{}) };
				context.rootIter  = getBeginIter(in);
				context.iter	  = context.rootIter;
				context.endIter	  = getEndIter(in);
				context.parserPtr = this;
				auto newSize	  = static_cast<uint64_t>((context.endIter - context.iter) / 2);
				if (derivedRef.stringBuffer.size() < newSize) {
					derivedRef.stringBuffer.resize(newSize);
				}
				if constexpr (options.validateJson) {
					if (!derivedRef.validateJson(in)) {
						return jsonifier::internal::remove_cvref_t<value_type>{};
					}
				}
				derivedRef.errors.clear();
				if JSONIFIER_UNLIKELY (!context.iter || context.iter == context.endIter) {
					reportError<parse_status::No_Input>(context);
					return jsonifier::internal::remove_cvref_t<value_type>{};
				}
				value_type object{};
				parse<optionsNew, options.minified>::impl(object, context);
				return (context.currentObjectDepth != 0) ? (reportError<parse_status::Missing_Object_End>(context), jsonifier::internal::remove_cvref_t<value_type>{})
					: (context.currentArrayDepth != 0)	 ? (reportError<parse_status::Missing_Array_End>(context), jsonifier::internal::remove_cvref_t<value_type>{})
					: (context.iter < context.endIter && !optionsNew.partialRead)
					? (reportError<parse_status::Unfinished_Input>(context), jsonifier::internal::remove_cvref_t<value_type>{})
					: derivedRef.errors.size() > 0 ? jsonifier::internal::remove_cvref_t<value_type>{}
												   : object;
			}
		}

		template<auto parseError, typename context_type>
		void reportError(context_type& context, const std::source_location& sourceLocation = std::source_location::current()) noexcept {
			derivedRef.errors.emplace_back(error::constructError<status_classes::Parsing, parseError>(getUnderlyingPtr(context.iter) - getUnderlyingPtr(context.rootIter),
				getUnderlyingPtr(context.endIter) - getUnderlyingPtr(context.rootIter), getUnderlyingPtr(context.rootIter), sourceLocation));
		}

		const std::vector<internal::error>& getErrors() const noexcept {
			return derivedRef.getErrors();
		}

		JSONIFIER_INLINE auto& getStringBuffer() noexcept {
			return derivedRef.stringBuffer;
		}

	  protected:
		derived_type& derivedRef{ initializeSelfRef() };

		parser() noexcept : derivedRef{ initializeSelfRef() } {
		}

		derived_type& initializeSelfRef() noexcept {
			return *static_cast<derived_type*>(this);
		}

		~parser() noexcept = default;
	};
};
