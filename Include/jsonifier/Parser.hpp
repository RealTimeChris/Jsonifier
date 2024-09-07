/*
	MIT License

	Copyright (c) 2023 RealTimeChris

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

namespace jsonifier {

	struct parse_options {
		bool validateJson{ false };
		bool minified{ false };
	};

}

namespace jsonifier_internal {

	template<typename derived_type> class parser;

	template<const auto&, typename value_type, typename derived_type> struct parse_impl;

	template<typename derived_type> class parser {
	  public:
		template<const auto&, typename value_type, typename derived_type_new> friend struct parse_impl;

		JSONIFIER_ALWAYS_INLINE parser& operator=(const parser& other) = delete;
		JSONIFIER_ALWAYS_INLINE parser(const parser& other)			   = delete;

		template<jsonifier::parse_options optionsNew = jsonifier::parse_options{}, typename value_type, typename buffer_type>
		JSONIFIER_ALWAYS_INLINE bool parseJson(value_type&& object, buffer_type&& in) noexcept {
			static constexpr auto options{ optionsNew };
			parse_context<derived_type> context{ *this };
			context.currentIter = in;
			context.rootIter	= in;
			context.endIter			= char_comparison<'\0', unwrap_t<decltype(*in)>>::memchar(in, std::numeric_limits<size_t>::max());
			if (!in || *in == '\0') {
				static constexpr auto sourceLocation{ std::source_location::current() };
				getErrors().emplace_back(error::constructError<sourceLocation, error_classes::Parsing, parse_errors::No_Input>(context.currentIter - context.rootIter,
					context.currentIter - context.rootIter, context.rootIter));
				return false;
			}
			if constexpr (options.validateJson) {
				if (!derivedRef.validateJson(in)) {
					return false;
				}
			}
			derivedRef.errors.clear();
			if (!context.currentIter || (*context.currentIter != '{' && *context.currentIter != '[')) [[unlikely]] {
				static constexpr auto sourceLocation{ std::source_location::current() };
				getErrors().emplace_back(error::constructError<sourceLocation, error_classes::Parsing, parse_errors::No_Input>(context.currentIter - context.rootIter,
					context.currentIter - context.rootIter, context.rootIter));
				return false;
			}
			parse_impl<options, value_type, derived_type>::impl(std::forward<value_type>(object), context);
			if (context.currentObjectDepth != 0) [[unlikely]] {
				static constexpr auto sourceLocation{ std::source_location::current() };
				getErrors().emplace_back(error::constructError<sourceLocation, error_classes::Parsing, parse_errors::Imbalanced_Object_Braces>(context.currentIter - context.rootIter,
					context.currentIter - context.rootIter, context.rootIter));
				return false;
			} else if (context.currentArrayDepth != 0) [[unlikely]] {
				static constexpr auto sourceLocation{ std::source_location::current() };
				getErrors().emplace_back(error::constructError<sourceLocation, error_classes::Parsing, parse_errors::Imbalanced_Array_Brackets>(context.currentIter - context.rootIter,
					context.currentIter - context.rootIter, context.rootIter));
				return false;
			}
			if (context.currentIter != context.endIter) [[unlikely]] {
				static constexpr auto sourceLocation{ std::source_location::current() };
				getErrors().emplace_back(error::constructError<sourceLocation, error_classes::Parsing, parse_errors::Unfinished_Input>(context.currentIter - context.rootIter,
					context.currentIter - context.rootIter, context.rootIter));
				return false;
			}
			return true;
		}

		template<jsonifier::parse_options optionsNew = jsonifier::parse_options{}, typename value_type, jsonifier::concepts::string_t buffer_type>
		JSONIFIER_ALWAYS_INLINE bool parseJson(value_type&& object, buffer_type&& in) noexcept {
			static constexpr auto options{ optionsNew };
			parse_context<derived_type> context{ *this };
			context.currentIter = in.data();
			context.rootIter	= in.data();
			context.endIter		= in.data() + in.size();
			if (in.size() == 0) {
				static constexpr auto sourceLocation{ std::source_location::current() };
				getErrors().emplace_back(error::constructError<sourceLocation, error_classes::Parsing, parse_errors::No_Input>(context.currentIter - context.rootIter,
					context.endIter - context.rootIter, context.rootIter));
				return false;
			}
			if constexpr (options.validateJson) {
				if (!derivedRef.validateJson(in)) {
					return false;
				}
			}
			derivedRef.errors.clear();
			if (!context.currentIter || (*context.currentIter != '{' && *context.currentIter != '[')) [[unlikely]] {
				static constexpr auto sourceLocation{ std::source_location::current() };
				getErrors().emplace_back(error::constructError<sourceLocation, error_classes::Parsing, parse_errors::No_Input>(context.currentIter - context.rootIter,
					context.endIter - context.rootIter, context.rootIter));
				return false;
			}
			parse_impl<options, value_type, derived_type>::impl(std::forward<value_type>(object), context);
			if (context.currentObjectDepth != 0) [[unlikely]] {
				static constexpr auto sourceLocation{ std::source_location::current() };
				getErrors().emplace_back(error::constructError<sourceLocation, error_classes::Parsing, parse_errors::Imbalanced_Object_Braces>(context.currentIter - context.rootIter,
					context.endIter - context.rootIter, context.rootIter));
				return false;
			} else if (context.currentArrayDepth != 0) [[unlikely]] {
				static constexpr auto sourceLocation{ std::source_location::current() };
				getErrors().emplace_back(error::constructError<sourceLocation, error_classes::Parsing, parse_errors::Imbalanced_Array_Brackets>(context.currentIter - context.rootIter,
					context.endIter - context.rootIter, context.rootIter));
				return false;
			}
			if (context.currentIter != context.endIter) [[unlikely]] {
				static constexpr auto sourceLocation{ std::source_location::current() };
				getErrors().emplace_back(error::constructError<sourceLocation, error_classes::Parsing, parse_errors::Unfinished_Input>(context.currentIter - context.rootIter,
					context.endIter - context.rootIter, context.rootIter));
				return false;
			}
			return true;
		}

		template<typename value_type, jsonifier::parse_options optionsNew = jsonifier::parse_options{}, jsonifier::concepts::string_t buffer_type>
		JSONIFIER_ALWAYS_INLINE value_type parseJson(buffer_type&& in) noexcept {
			static constexpr auto options{ optionsNew };
			parse_context<derived_type> context{ *this };
			context.currentIter = in.data();
			context.rootIter	= in.data();
			context.endIter		= in.data() + in.size();
			if (in.size() == 0) [[unlikely]] {
				static constexpr auto sourceLocation{ std::source_location::current() };
				getErrors().emplace_back(error::constructError<sourceLocation, error_classes::Parsing, parse_errors::No_Input>(context.currentIter - context.rootIter,
					context.endIter - context.rootIter, context.rootIter));
				return value_type{};
			}
			if constexpr (options.validateJson) {
				if (!derivedRef.validateJson(in)) [[unlikely]] {
					return value_type{};
				}
			}
			derivedRef.errors.clear();
			unwrap_t<value_type> object{};
			if (!context.currentIter || (*context.currentIter != '{' && *context.currentIter != '[')) [[unlikely]] {
				static constexpr auto sourceLocation{ std::source_location::current() };
				getErrors().emplace_back(error::constructError<sourceLocation, error_classes::Parsing, parse_errors::No_Input>(context.currentIter - context.rootIter,
					context.endIter - context.rootIter, context.rootIter));
				return object;
			}
			parse_impl<options, value_type, derived_type>::impl(std::forward<value_type>(object), context);
			if (context.currentObjectDepth != 0) [[unlikely]] {
				static constexpr auto sourceLocation{ std::source_location::current() };
				getErrors().emplace_back(error::constructError<sourceLocation, error_classes::Parsing, parse_errors::Imbalanced_Object_Braces>(context.currentIter - context.rootIter,
					context.endIter - context.rootIter, context.rootIter));
				return value_type{};
			} else if (context.currentArrayDepth != 0) [[unlikely]] {
				static constexpr auto sourceLocation{ std::source_location::current() };
				getErrors().emplace_back(error::constructError<sourceLocation, error_classes::Parsing, parse_errors::Imbalanced_Array_Brackets>(context.currentIter - context.rootIter,
					context.endIter - context.rootIter, context.rootIter));
				return value_type{};
			}
			if (context.currentIter != context.endIter) [[unlikely]] {
				static constexpr auto sourceLocation{ std::source_location::current() };
				getErrors().emplace_back(error::constructError<sourceLocation, error_classes::Parsing, parse_errors::Unfinished_Input>(context.currentIter - context.rootIter,
					context.endIter - context.rootIter, context.rootIter));
				return value_type{};
			}
			return object;
		}

		JSONIFIER_ALWAYS_INLINE jsonifier::vector<error>& getErrors() noexcept {
			return derivedRef.errors;
		}

	  protected:
		derived_type& derivedRef{ initializeSelfRef() };

		JSONIFIER_ALWAYS_INLINE parser() noexcept : derivedRef{ initializeSelfRef() } {};

		JSONIFIER_ALWAYS_INLINE derived_type& initializeSelfRef() noexcept {
			return *static_cast<derived_type*>(this);
		}

		JSONIFIER_ALWAYS_INLINE ~parser() noexcept = default;
	};
};