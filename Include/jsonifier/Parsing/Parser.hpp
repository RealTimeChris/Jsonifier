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

#include <jsonifier/Utilities/JsonEntity.hpp>
#include <jsonifier/Parsing/Validator.hpp>
#include <jsonifier/Utilities/HashMap.hpp>
#include <jsonifier/Utilities/String.hpp>
#include <jsonifier/Utilities/Error.hpp>
#include <jsonifier/Utilities/Simd.hpp>

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
			return remainingMemberCount > 0 && (currentArrayDepth > 0 || currentObjectDepth > 0);
		}

		mutable parser<derived_type>* parserPtr{};
		mutable int64_t remainingMemberCount{};
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

	template<concepts::pointer_t value_type> JSONIFIER_INLINE static string_view_ptr getEndIter(value_type value) noexcept {
		return reinterpret_cast<string_view_ptr>(char_comparison<'\0', decltype(*value)>::memchar(value, std::numeric_limits<size_t>::max()));
	}

	template<concepts::pointer_t value_type> JSONIFIER_INLINE static string_view_ptr getBeginIter(value_type value) noexcept {
		return reinterpret_cast<string_view_ptr>(value);
	}

	template<concepts::has_data value_type> JSONIFIER_INLINE static string_view_ptr getEndIter(value_type& value) noexcept {
		return reinterpret_cast<string_view_ptr>(value.data() + value.size());
	}

	template<concepts::has_data value_type> JSONIFIER_INLINE static string_view_ptr getBeginIter(value_type& value) noexcept {
		return reinterpret_cast<string_view_ptr>(value.data());
	}

	template<typename value_type, typename context_type, parse_options optionsNew, bool minified> struct parse_partial_impl;
	template<typename value_type, typename context_type, parse_options optionsNew, bool minified> struct parse_impl;

	template<typename value_type, size_t currentIndex = 0> static constexpr size_t countTotalNonRepeatedMembers(size_t currentCount = 1) {
		if constexpr (currentIndex < core_tuple_size<value_type>) {
			constexpr auto newSubTuple = internal::get<currentIndex>(core<jsonifier::internal::remove_cvref_t<value_type>>::parseValue);
			using member_type		   = typename jsonifier::internal::remove_cvref_t<decltype(newSubTuple)>::member_type;
			if constexpr (concepts::jsonifier_object_t<member_type>) {
				currentCount += countTotalNonRepeatedMembers<member_type>();
			}
			++currentCount;
			return countTotalNonRepeatedMembers<value_type, currentIndex + 1>(currentCount);
		} else {
			return currentCount;
		}
	}

	template<parse_options options, bool minifiedOrInsideRepeated> struct parse {
		template<typename value_type_new, typename context_type> JSONIFIER_INLINE static void impl(value_type_new&& value, context_type&& context) noexcept {
			using value_type = remove_cvref_t<value_type_new>;
			if constexpr (options.partialRead) {
				if constexpr (concepts::map_t<value_type> || concepts::jsonifier_object_t<value_type>) {
					if constexpr (concepts::map_t<value_type> || minifiedOrInsideRepeated) {
						parse_partial_impl<value_type, context_type, options, true>::impl(value, context);
					} else {
						parse_partial_impl<value_type, context_type, options, false>::impl(value, context);
					}
				} else if constexpr (jsonifier::concepts::raw_array_t<value_type> || concepts::vector_t<value_type>) {
					parse_partial_impl<value_type, context_type, options, true>::impl(value, context);
				} else {
					parse_partial_impl<value_type, context_type, options, minifiedOrInsideRepeated>::impl(value, context);
				}
				if constexpr (!minifiedOrInsideRepeated) {
					--context.remainingMemberCount;
				}
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

		template<parse_options options = parse_options{}, typename value_type, typename buffer_type>
		JSONIFIER_INLINE bool parseJson(value_type&& object, buffer_type&& in) noexcept {
			if constexpr (options.partialRead) {
				static constexpr parse_options optionsNew{ options };
				constexpr parse_context_partial<derived_type, string_view_ptr*> context{ constEval(parse_context_partial<derived_type, string_view_ptr*>{}) };
				auto rootIter = getBeginIter(in);
				auto endIter  = getEndIter(in);
				derivedRef.section.template reset<options.minified>(rootIter, static_cast<size_t>(endIter - rootIter));
				context.rootIter			 = derivedRef.section.begin();
				context.iter				 = derivedRef.section.begin();
				context.endIter				 = derivedRef.section.end();
				context.remainingMemberCount = static_cast<int64_t>(countTotalNonRepeatedMembers<value_type>());
				context.parserPtr			 = this;
				auto newSize				 = static_cast<uint64_t>((context.endIter - context.iter) / 2);
				if (derivedRef.stringBuffer.size() < newSize) {
					derivedRef.stringBuffer.resize(newSize);
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
				if JSONIFIER_UNLIKELY (!context.iter) {
					reportError<parse_errors::No_Input>(context);
					return false;
				}
				parse<optionsNew, options.minified>::impl(object, context);
				return (context.currentObjectDepth != 0) ? (reportError<parse_errors::Imbalanced_Object_Braces>(context), false)
					: (context.currentArrayDepth != 0)	 ? (reportError<parse_errors::Imbalanced_Array_Brackets>(context), false)
					: (context.iter < context.endIter)	 ? (reportError<parse_errors::Unfinished_Input>(context), false)
					: derivedRef.errors.size() > 0		 ? false
														 : true;
			}
		}

		template<typename value_type, parse_options options = parse_options{}, typename buffer_type>
		JSONIFIER_INLINE bool parseManyJson(value_type&& object, buffer_type&& in) noexcept {
			static constexpr parse_options optionsNew{ .validateJson = options.validateJson, .partialRead = false, .knownOrder = options.knownOrder, .minified = options.minified };
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
			if JSONIFIER_UNLIKELY (!context.iter) {
				reportError<parse_errors::No_Input>(context);
				return false;
			}
			while (context.iter < context.endIter) {
				parse<optionsNew, options.minified>::impl(object.emplace_back(), context);
			}
			return (context.currentObjectDepth != 0)						  ? (reportError<parse_errors::Imbalanced_Object_Braces>(context), false)
				: (context.currentArrayDepth != 0)							  ? (reportError<parse_errors::Imbalanced_Array_Brackets>(context), false)
				: (context.iter < context.endIter && !optionsNew.partialRead) ? (reportError<parse_errors::Unfinished_Input>(context), false)
				: derivedRef.errors.size() > 0								  ? false
																			  : true;
		}

		template<typename value_type, parse_options options = parse_options{}, concepts::string_t buffer_type> JSONIFIER_INLINE value_type parseJson(buffer_type&& in) noexcept {
			if constexpr (options.partialRead) {
				static constexpr parse_options optionsNew{ options };
				constexpr parse_context_partial<derived_type, string_view_ptr*> context{ constEval(parse_context_partial<derived_type, string_view_ptr*>{}) };
				auto rootIter = getBeginIter(in);
				auto endIter  = getEndIter(in);
				derivedRef.section.template reset<options.minified>(rootIter, static_cast<size_t>(endIter - rootIter));
				context.rootIter			 = derivedRef.section.begin();
				context.iter				 = derivedRef.section.begin();
				context.endIter				 = derivedRef.section.end();
				context.remainingMemberCount = countTotalNonRepeatedMembers<value_type>();
				context.parserPtr			 = this;
				auto newSize				 = static_cast<uint64_t>((context.endIter - context.iter) / 2);
				if (derivedRef.stringBuffer.size() < newSize) {
					derivedRef.stringBuffer.resize(newSize);
				}
				if constexpr (options.validateJson) {
					if (!derivedRef.validateJson(in)) {
						return jsonifier::internal::remove_cvref_t<value_type>{};
					}
				}
				derivedRef.errors.clear();
				if JSONIFIER_UNLIKELY (!context.iter) {
					reportError<parse_errors::No_Input>(context);
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
				if JSONIFIER_UNLIKELY (!context.iter) {
					reportError<parse_errors::No_Input>(context);
					return jsonifier::internal::remove_cvref_t<value_type>{};
				}
				value_type object{};
				parse<optionsNew, options.minified>::impl(object, context);
				return (context.currentObjectDepth != 0) ? (reportError<parse_errors::Imbalanced_Object_Braces>(context), jsonifier::internal::remove_cvref_t<value_type>{})
					: (context.currentArrayDepth != 0)	 ? (reportError<parse_errors::Imbalanced_Array_Brackets>(context), jsonifier::internal::remove_cvref_t<value_type>{})
					: (context.iter < context.endIter && !optionsNew.partialRead)
					? (reportError<parse_errors::Unfinished_Input>(context), jsonifier::internal::remove_cvref_t<value_type>{})
					: derivedRef.errors.size() > 0 ? jsonifier::internal::remove_cvref_t<value_type>{}
												   : object;
			}
		}

		template<auto parseError, typename context_type>
		void reportError(context_type& context, const std::source_location& sourceLocation = std::source_location::current()) noexcept {
			derivedRef.errors.emplace_back(error::constructError<error_classes::Parsing, parseError>(getUnderlyingPtr(context.iter) - getUnderlyingPtr(context.rootIter),
				getUnderlyingPtr(context.endIter) - getUnderlyingPtr(context.rootIter), getUnderlyingPtr(context.rootIter), sourceLocation));
		}

	  protected:
		derived_type& derivedRef{ initializeSelfRef() };

		parser() noexcept : derivedRef{ initializeSelfRef() } {};

		JSONIFIER_INLINE auto& getStringBuffer() noexcept {
			return derivedRef.stringBuffer;
		}

		derived_type& initializeSelfRef() noexcept {
			return *static_cast<derived_type*>(this);
		}

		~parser() noexcept = default;
	};
};