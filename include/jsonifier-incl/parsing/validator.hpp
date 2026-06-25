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

#include <jsonifier-incl/utilities/type_entities.hpp>
#include <jsonifier-incl/utilities/string_utils.hpp>
#include <jsonifier-incl/utilities/error.hpp>

namespace jsonifier::internal {

	template<json_structural_type typeNew, typename derived_type> struct validate_impl;

	template<typename derived_type> class validator {
	  public:
		template<json_structural_type typeNew, typename derived_type_new> friend struct validate_impl;

		validator& operator=(const validator& other) = delete;
		validator(const validator& other)			 = delete;

		template<concepts::string_t string_type> inline bool validateJson(string_type&& in) noexcept {
			derivedRef.errors.clear();
			derivedRef.section.template reset<false>(in.data(), in.size());
			rootIter = in.data();
			endIter	 = in.data() + in.size();
			uint32_t* iter{ derivedRef.section.begin() };
			uint32_t* end{ derivedRef.section.end() };
			if (iter == end) {
				getErrors().emplace_back(error::constructError<status_classes::Validating, validate_status::No_Input>(0, 0, nullptr));
				return false;
			}
			auto result = impl(iter, end, *this);
			if ((iter < end && static_cast<uint64_t>(*iter) < in.size()) || derivedRef.errors.size() > 0ull) {
				getErrors().emplace_back(error::constructError<status_classes::Validating, validate_status::No_Input>((iter < end ? *iter : 0), endIter - rootIter, rootIter));
				result = false;
			}
			return result;
		}

	  protected:
		derived_type& derivedRef{ initializeSelfRef() };
		mutable string_view_ptr rootIter{};
		mutable string_view_ptr endIter{};

		validator() noexcept : derivedRef{ initializeSelfRef() } {
		}

		template<typename iterator, typename validator_type> inline static bool impl(iterator& iter, iterator& end, validator_type& validator) noexcept {
			if (iter < end && validator.rootIter[*iter] == '{') {
				return validate_impl<json_structural_type::object_start, derived_type>::impl(iter, end, validator);
			} else if (iter < end && validator.rootIter[*iter] == '[') {
				return validate_impl<json_structural_type::array_start, derived_type>::impl(iter, end, validator);
			} else if (iter < end && validator.rootIter[*iter] == '"') {
				return validate_impl<json_structural_type::string, derived_type>::impl(iter, end, validator);
			} else if (iter < end && numberTable[static_cast<uint8_t>(validator.rootIter[*iter])]) {
				return validate_impl<json_structural_type::number, derived_type>::impl(iter, end, validator);
			} else if (iter < end && boolTable[static_cast<uint8_t>(validator.rootIter[*iter])]) {
				return validate_impl<json_structural_type::boolean, derived_type>::impl(iter, end, validator);
			} else if (iter < end && validator.rootIter[*iter] == 'n') {
				return validate_impl<json_structural_type::null, derived_type>::impl(iter, end, validator);
			} else {
				return false;
			}
		}

		inline auto& getStringBuffer() noexcept {
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
