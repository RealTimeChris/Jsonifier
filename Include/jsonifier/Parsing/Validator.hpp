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

#include <jsonifier/Utilities/TypeEntities.hpp>
#include <jsonifier/Utilities/StringUtils.hpp>
#include <jsonifier/Utilities/Error.hpp>

namespace jsonifier::internal {

	template<json_structural_type typeNew, typename derived_type> struct validate_impl;

	template<typename derived_type> class validator {
	  public:
		template<json_structural_type typeNew, typename derived_type_new> friend struct validate_impl;

		validator& operator=(const validator& other) = delete;
		validator(const validator& other)			 = delete;

		template<concepts::string_t string_type> JSONIFIER_INLINE bool validateJson(string_type&& in) noexcept {
			derivedRef.errors.clear();
			derivedRef.section.template reset<false>(in.data(), in.size());
			rootIter = in.data();
			endIter	 = in.data() + in.size();
			string_view_ptr* iter{ derivedRef.section.begin() };
			string_view_ptr* end{ derivedRef.section.end() };
			if (!iter) {
				getErrors().emplace_back(error::constructError<error_classes::Validating, validate_errors::No_Input>(0, 0, nullptr));
				return false;
			}
			auto result = impl(iter, end, *this);
			if (((static_cast<uint64_t>(*iter - rootIter) < in.size()) || derivedRef.errors.size() > 0ull)) {
				getErrors().emplace_back(error::constructError<error_classes::Validating, validate_errors::No_Input>(*iter - rootIter, endIter - rootIter, rootIter));
				result = false;
			}
			return result;
		}

	  protected:
		derived_type& derivedRef{ initializeSelfRef() };
		mutable string_view_ptr rootIter{};
		mutable string_view_ptr endIter{};

		validator() noexcept : derivedRef{ initializeSelfRef() } {};

		template<typename iterator, typename validator_type> JSONIFIER_INLINE static bool impl(iterator& iter, iterator& end, validator_type& validator) noexcept {
			if (*iter && **iter == '{') {
				return validate_impl<json_structural_type::object_start, derived_type>::impl(iter, end, validator);
			} else {
				if (*iter && **iter == '[') {
					return validate_impl<json_structural_type::array_start, derived_type>::impl(iter, end, validator);
				} else {
					if (*iter && **iter == '"') {
						return validate_impl<json_structural_type::string, derived_type>::impl(iter, validator);
					} else {
						if (*iter && numberTable[static_cast<uint8_t>(**iter)]) {
							return validate_impl<json_structural_type::number, derived_type>::impl(iter, validator);
						} else {
							if (*iter && boolTable[static_cast<uint8_t>(**iter)]) {
								return validate_impl<json_structural_type::boolean, derived_type>::impl(iter, validator);
							} else {
								if (*iter && **iter == 'n') {
									return validate_impl<json_structural_type::null, derived_type>::impl(iter, validator);
								} else {
									return false;
								}
							}
						}
					}
				}
			}
		}

		JSONIFIER_INLINE auto& getStringBuffer() noexcept {
			return derivedRef.stringBuffer;
		}

		derived_type& initializeSelfRef() noexcept {
			return *static_cast<derived_type*>(this);
		}

		std::vector<error>& getErrors() noexcept {
			return derivedRef.errors;
		}

		~validator() noexcept = default;
	};

}// namespace internal
