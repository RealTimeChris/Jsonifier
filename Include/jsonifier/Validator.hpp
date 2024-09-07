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
#include <jsonifier/StringUtils.hpp>
#include <jsonifier/Vector.hpp>
#include <jsonifier/Error.hpp>

namespace jsonifier_internal {

	static thread_local jsonifier_internal::simd_string_reader<true> section{};

	enum class validate_errors {
		Success						   = 0,
		Missing_Object_Start		   = 1,
		Imbalanced_Object_Braces	   = 2,
		Missing_Array_Start			   = 3,
		Imbalanced_Array_Brackets	   = 4,
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

		JSONIFIER_ALWAYS_INLINE validator& operator=(const validator& other) = delete;
		JSONIFIER_ALWAYS_INLINE validator(const validator& other)			 = delete;

		template<jsonifier::concepts::string_t string_type> JSONIFIER_ALWAYS_INLINE bool validateJson(string_type&& in) noexcept {
			derivedRef.errors.clear();
			derivedRef.index = 0;
			section.reset(in.data(), in.size());
			rootIter = in.data();
			endIter	 = in.data() + in.size();
			const char** iter{ section.begin() };
			if (!iter) {
				static constexpr auto sourceLocation{ std::source_location::current() };
				getErrors().emplace_back(
					error::constructError<sourceLocation, error_classes::Validating, validate_errors::No_Input>(getUnderlyingPtr(iter) - rootIter, endIter - rootIter, rootIter));
				return false;
			}
			auto result = impl(iter, derivedRef.index, *this);
			if (derivedRef.index > 0 || *iter || derivedRef.errors.size() > 0) {
				result = false;
			}
			return result;
		}

	  protected:
		derived_type& derivedRef{ initializeSelfRef() };
		mutable const char* rootIter{};
		mutable const char* endIter{};

		JSONIFIER_ALWAYS_INLINE validator() noexcept : derivedRef{ initializeSelfRef() } {};

		template<typename iterator, typename validator_type> JSONIFIER_ALWAYS_INLINE static bool impl(iterator& iter, uint64_t& depth, validator_type& validator) noexcept {
			if (*iter && **iter == '{') {
				return validate_impl<json_structural_type::Object_Start, derived_type>::impl(iter, depth, validator);
			} else {
				if (*iter && **iter == '[') {
					return validate_impl<json_structural_type::Array_Start, derived_type>::impl(iter, depth, validator);
				} else {
					if (*iter && **iter == '"') {
						return validate_impl<json_structural_type::String, derived_type>::impl(iter, validator);
					} else {
						if (*iter && numberTable[static_cast<uint8_t>(**iter)]) {
							return validate_impl<json_structural_type::Number, derived_type>::impl(iter, validator);
						} else {
							if (*iter && boolTable[static_cast<uint8_t>(**iter)]) {
								return validate_impl<json_structural_type::Bool, derived_type>::impl(iter, validator);
							} else {
								if (*iter && **iter == 'n') {
									return validate_impl<json_structural_type::Null, derived_type>::impl(iter, validator);
								} else {
									return false;
								}
							}
						}
					}
				}
			}
		}

		JSONIFIER_ALWAYS_INLINE derived_type& initializeSelfRef() noexcept {
			return *static_cast<derived_type*>(this);
		}

		JSONIFIER_ALWAYS_INLINE jsonifier::vector<error>& getErrors() noexcept {
			return derivedRef.errors;
		}

		JSONIFIER_ALWAYS_INLINE ~validator() noexcept = default;
	};

}// namespace jsonifier_internal
