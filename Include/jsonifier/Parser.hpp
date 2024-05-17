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

#include <jsonifier/SimdStructuralIterator.hpp>
#include <jsonifier/Validator.hpp>
#include <jsonifier/HashMap.hpp>
#include <jsonifier/String.hpp>
#include <jsonifier/Error.hpp>
#include <jsonifier/Simd.hpp>

namespace jsonifier {

	struct parse_options {
		bool refreshString{ true };
		bool minified{ false };
	};

}

namespace jsonifier_internal {

	enum class parse_errors {
		Success						= 0,
		Missing_Object_Start		= 1,
		Missing_Object_End			= 2,
		Missing_Array_Start			= 3,
		Missing_String_Start		= 4,
		Missing_Colon				= 5,
		Missing_Comma_Or_Object_End = 6,
		Missing_Comma_Or_Array_End	= 7,
		Invalid_Number_Value		= 8,
		Invalid_Null_Value			= 9,
		Invalid_Bool_Value			= 10,
		Invalid_String_Characters	= 11,
		No_Input					= 12,
	};

	template<typename derived_type, typename value_type> struct parse_impl;

	template<typename derived_type> class parser {
	  public:
		template<typename derived_type_new, typename value_type> friend struct parse_impl;

		JSONIFIER_INLINE parser& operator=(const parser& other) = delete;
		JSONIFIER_INLINE parser(const parser& other)			= delete;

		template<jsonifier::parse_options options = jsonifier::parse_options{}, typename value_type, jsonifier::concepts::string_t buffer_type>
		JSONIFIER_INLINE bool parseJson(value_type&& object, buffer_type&& in) {
			static_assert(jsonifier::concepts::printErrorFunction<jsonifier::concepts::unwrap_t<value_type>>(),
				"No specialization of core exists for the type named above - please specialize it!");
			derivedRef.errors.clear();
			derivedRef.section.template reset<options.refreshString, options.minified>(in.data(), in.size());
			simd_structural_iterator iter{ derivedRef.section.begin(), in.size(), derivedRef.errors };
			if (!iter || (*iter != 0x7Bu && *iter != 0x5Bu)) {
				iter.template createError<error_classes::Parsing>(parse_errors::No_Input);
				return false;
			}
			impl(std::forward<value_type>(object), iter);
			if constexpr (!options.minified) {
				if (iter) {
					iter.template createError<error_classes::Parsing>(parse_errors::No_Input);
					return false;
				}
			}
			return true;
		}

		template<typename value_type, jsonifier::parse_options options = jsonifier::parse_options{}, jsonifier::concepts::string_t buffer_type>
		JSONIFIER_INLINE value_type parseJson(buffer_type&& in) {
			static_assert(jsonifier::concepts::printErrorFunction<jsonifier::concepts::unwrap_t<value_type>>(),
				"No specialization of core exists for the type named above - please specialize it!");
			derivedRef.errors.clear();
			derivedRef.section.template reset<options.refreshString, options.minified>(in.data(), in.size());
			simd_structural_iterator iter{ derivedRef.section.begin(), in.size(), derivedRef.errors };
			if (!iter || (*iter != 0x7Bu && *iter != 0x5Bu)) {
				iter.template createError<error_classes::Parsing>(parse_errors::No_Input);
				return value_type{};
			}
			value_type object{};
			impl(std::forward<value_type>(object), iter);
			if constexpr (!options.minified) {
				if (iter) {
					iter.template createError<error_classes::Parsing>(parse_errors::No_Input);
					return {};
				}
			}
			return object;
		}

	  protected:
		derived_type& derivedRef{ initializeSelfRef() };

		JSONIFIER_INLINE parser() noexcept : derivedRef{ initializeSelfRef() } {};

		JSONIFIER_INLINE derived_type& initializeSelfRef() {
			return *static_cast<derived_type*>(this);
		}

		template<typename value_type, jsonifier::concepts::is_fwd_iterator iterator_type> JSONIFIER_INLINE static void impl(value_type&& object, iterator_type&& iter) {
			if constexpr (jsonifier::concepts::has_excluded_keys<value_type>) {
				parse_impl<derived_type, jsonifier::concepts::unwrap_t<value_type>>::impl(std::forward<value_type>(object), std::forward<iterator_type>(iter),
					object.jsonifierExcludedKeys);
			} else {
				parse_impl<derived_type, jsonifier::concepts::unwrap_t<value_type>>::impl(std::forward<value_type>(object), std::forward<iterator_type>(iter));
			}
		}

		JSONIFIER_INLINE ~parser() noexcept = default;
	};
};