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

#include <jsonifier/TypeEntities.hpp>
#include <jsonifier/Derailleur.hpp>
#include <jsonifier/Vector.hpp>
#include <jsonifier/Error.hpp>

namespace jsonifier_internal {

	enum class validate_errors {
		Success						   = 0,
		Missing_Object_Start		   = 1,
		Missing_Object_End			   = 2,
		Missing_Array_Start			   = 3,
		Missing_Array_End			   = 4,
		Missing_String_Start		   = 5,
		Missing_Colon				   = 6,
		Missing_Comma				   = 7,
		Invalid_Number_Value		   = 8,
		Invalid_Null_Value			   = 9,
		Invalid_Bool_Value			   = 10,
		Invalid_String_Characters	   = 11,
		Invalid_Escape_Characters	   = 12,
		Missing_Comma_Or_Closing_Brace = 13,
		No_Input					   = 14,
	};

	template<json_structural_type typeNew, typename derived_type> struct validate_impl;

	template<typename derived_type> class validator {
	  public:
		template<json_structural_type typeNew, typename derived_type_new> friend struct validate_impl;

		JSONIFIER_INLINE validator& operator=(const validator& other) = delete;
		JSONIFIER_INLINE validator(const validator& other)			  = delete;

		template<jsonifier::concepts::string_t string_type> JSONIFIER_INLINE bool validateJson(string_type&& in) noexcept {
			derivedRef.errors.clear();
			derivedRef.index = 0;
			derivedRef.section.template reset<true>(in.data(), in.size());
			simd_structural_iterator iter{ derivedRef.section.begin(), in.size(), derivedRef.errors };
			if (!iter) {
				iter.template createError<error_classes::Prettifying>(validate_errors::No_Input);
				return false;
			}
			auto result = impl(iter, derivedRef.index);
			if (derivedRef.index > 0 || iter.operator bool() || derivedRef.errors.size() > 0) {
				result = false;
			}
			return result;
		}

	  protected:
		derived_type& derivedRef{ initializeSelfRef() };

		JSONIFIER_INLINE validator() noexcept : derivedRef{ initializeSelfRef() } {};

		template<jsonifier::concepts::is_fwd_iterator iterator_type> JSONIFIER_INLINE static bool impl(iterator_type&& iter, uint64_t& depth) {
			if (*iter == 0x7Bu) {
				return validate_impl<json_structural_type::Object_Start, derived_type>::impl(iter, depth);
			} else if (*iter == 0x5Bu) {
				return validate_impl<json_structural_type::Array_Start, derived_type>::impl(iter, depth);
			} else if (*iter == 0x22u) {
				return validate_impl<json_structural_type::String, derived_type>::impl(iter);
			} else if (numberTable[static_cast<uint64_t>(*iter)]) {
				return validate_impl<json_structural_type::Number, derived_type>::impl(iter);
			} else if (boolTable[static_cast<uint64_t>(*iter)]) {
				return validate_impl<json_structural_type::Bool, derived_type>::impl(iter);
			} else if (*iter == 0x6Eu) {
				return validate_impl<json_structural_type::Null, derived_type>::impl(iter);
			} else {
				return false;
			}
		}

		JSONIFIER_INLINE derived_type& initializeSelfRef() {
			return *static_cast<derived_type*>(this);
		}

		JSONIFIER_INLINE ~validator() noexcept = default;
	};

}// namespace jsonifier_internal
