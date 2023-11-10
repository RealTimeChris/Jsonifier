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

namespace jsonifier_internal {

	template<typename value_type> struct parse_impl : public derailleur {};

	struct parse {
		template<jsonifier::concepts::core_type value_type, jsonifier::concepts::is_fwd_iterator iterator_type>
		JSONIFIER_INLINE static void impl(value_type&& data, iterator_type&& iter) {
			if constexpr (jsonifier::concepts::has_excluded_keys<value_type>) {
				parse_impl<jsonifier::concepts::unwrap_t<value_type>>::impl(std::forward<value_type>(data), std::forward<iterator_type>(iter), data.jsonifierExcludedKeys);
			} else {
				parse_impl<jsonifier::concepts::unwrap_t<value_type>>::impl(std::forward<value_type>(data), std::forward<iterator_type>(iter));
			}
		}
	};

	template<typename derived_type> class parser {
	  public:
		template<typename value_type> friend struct parse_impl;

		JSONIFIER_INLINE parser& operator=(const parser& other) = delete;
		JSONIFIER_INLINE parser(const parser& other)			= delete;

		template<bool refreshString = true, jsonifier::concepts::core_type value_type, jsonifier::concepts::string_t buffer_type>
		JSONIFIER_INLINE bool parseJson(value_type&& data, buffer_type&& stringNew) {
			derivedRef.errors.clear();
			derivedRef.section.template reset<refreshString>(stringNew.data(), stringNew.size());
			simd_structural_iterator iter{ derivedRef.section.begin(), derivedRef.section.getStringView(), derivedRef.stringBuffer, derivedRef.errors };
			if (!iter || (*iter != 0x7Bu && *iter != 0x5Bu)) [[unlikely]] {
				derivedRef.errors.emplace_back(createError(error_code::No_Input));
				return false;
			}
			parse::impl(std::forward<value_type>(data), iter);
			if (iter) [[unlikely]] {
				derivedRef.errors.emplace_back(createError(error_code::Invalid_Input));
				return false;
			}
			return true;
		}

	  protected:
		derived_type& derivedRef{ initializeSelfRef() };

		JSONIFIER_INLINE parser() noexcept : derivedRef{ initializeSelfRef() } {};

		JSONIFIER_INLINE derived_type& initializeSelfRef() {
			return *static_cast<derived_type*>(this);
		}

		JSONIFIER_INLINE ~parser() noexcept = default;
	};
};