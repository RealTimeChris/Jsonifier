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

	template<typename value_type, typename = void> struct json_comparator_impl;

	struct json_comparator {
		template<typename value_type_new> inline static bool impl(value_type_new&& lhs, value_type_new&& rhs) noexcept {
			using value_type = remove_cvref_t<value_type_new>;
			return json_comparator_impl<value_type>::impl(internal::forward<value_type_new>(lhs), internal::forward<value_type_new>(rhs));
		}
	};

	template<typename derived_type> class comparator {
	  public:
		comparator& operator=(const comparator& other) = delete;
		comparator(const comparator& other)			   = delete;

		template<typename value_type> inline bool compareJson(value_type&& lhs, value_type&& rhs) noexcept {
			return json_comparator::impl(lhs, rhs);
		}

		template<auto parseError, typename context_type>
		void reportError(context_type& context, const std::source_location& sourceLocation = std::source_location::current()) noexcept {
			derivedRef.errors.emplace_back(error::constructError<status_classes::Parsing, parseError>(getUnderlyingPtr(context.iter) - getUnderlyingPtr(context.rootIter),
				getUnderlyingPtr(context.endIter) - getUnderlyingPtr(context.rootIter), getUnderlyingPtr(context.rootIter), sourceLocation));
		}

	  protected:
		derived_type& derivedRef{ initializeSelfRef() };
		comparator() noexcept : derivedRef{ initializeSelfRef() } {}
		derived_type& initializeSelfRef() noexcept {
			return *static_cast<derived_type*>(this);
		}
		~comparator() noexcept = default;
	};
};
