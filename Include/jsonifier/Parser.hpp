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

	struct parser_options {
		bool bestEffortAfterError{ true };
		uint64_t simdCutoff{ 50000 };
		bool refreshString{ true };
	};

	template<typename value_type_new, typename derived_type, typename parser_type> struct serial_parse_impl {};

	template<typename value_type, typename derived_type> struct simd_parse_impl {};

	template<typename derived_type> class parser {
	  public:
		template<typename value_type_new, typename derived_type_new, typename parser_type> friend struct serial_parse_impl;
		template<typename value_type, typename derived_type_new> friend struct simd_parse_impl;

		JSONIFIER_INLINE parser& operator=(const parser& other) = delete;
		JSONIFIER_INLINE parser(const parser& other)			= delete;

		template<parser_options options = parser_options{}, jsonifier::concepts::core_type value_type, jsonifier::concepts::string_t buffer_type>
		JSONIFIER_INLINE bool parseJson(value_type&& data, buffer_type&& stringNew) {
			derivedRef.errors.clear();
			if (stringNew.size() >= options.simdCutoff) {
				derivedRef.section.template reset<options.refreshString>(stringNew.data(), stringNew.size());
				simd_structural_iterator iter{ derivedRef.section.begin(), derivedRef.section.getStringView(), derivedRef.stringBuffer, derivedRef.errors };
				if (iter) {
					if (*iter != 0x7Bu && *iter != 0x5Bu) [[unlikely]] {
						if (*iter != 0x7Bu && *iter != 0x5Bu) [[unlikely]] {
							derivedRef.errors.emplace_back(jsonifier_internal::createError(error_code::Invalid_Input));
							return false;
						}
						derivedRef.errors.emplace_back(jsonifier_internal::createError(error_code::No_Input));
						return false;
					}
					simd_impl(std::forward<value_type>(data), iter);
					if (iter) [[unlikely]] {
						derivedRef.errors.emplace_back(jsonifier_internal::createError(error_code::Invalid_Input));
						return false;
					} else {
						return true;
					}
				} else {
					derivedRef.errors.emplace_back(jsonifier_internal::createError(error_code::No_Input));
					return false;
				}
			} else {
				auto iter =
					const_iterator<typename jsonifier::concepts::unwrap_t<buffer_type>::value_type>{ stringNew.data(), stringNew.data(), stringNew.data() + stringNew.size() };
				auto iterEnd = const_iterator<typename jsonifier::concepts::unwrap_t<buffer_type>::value_type>{ stringNew.data() + stringNew.size(),
					stringNew.data() + stringNew.size(), stringNew.data() + stringNew.size() };
				if (iter) {
					serial_impl(std::forward<value_type>(data), iter, iterEnd, *this);
				}
				if (iter != iterEnd) [[unlikely]] {
					derivedRef.errors.emplace_back(jsonifier_internal::createError(error_code::Invalid_Input));
					return false;
				}
			}
			return true;
		}

	  protected:
		derived_type& derivedRef{ initializeSelfRef() };
		uint8_t padding[24]{};

		JSONIFIER_INLINE parser() noexcept : derivedRef{ initializeSelfRef() } {};

		JSONIFIER_INLINE derived_type& initializeSelfRef() {
			return *static_cast<derived_type*>(this);
		}

		JSONIFIER_INLINE auto& getCurrentString() {
			return derivedRef.stringBuffer;
		}

		template<error_code errorNew, jsonifier::concepts::is_fwd_iterator iterator_type>
		JSONIFIER_INLINE error createError(iterator_type& iterator, std::source_location location = std::source_location::current()) {
			auto newError = jsonifier_internal::createError<errorNew>(iterator, location);
			derivedRef.errors.emplace_back(newError);
			return newError;
		}

		template<json_structural_type structuralType, jsonifier::concepts::is_fwd_iterator iterator_type>
		JSONIFIER_INLINE error createError(iterator_type& iterator, std::source_location location = std::source_location::current()) {
			auto newError = jsonifier_internal::createError<structuralType>(iterator, location);
			derivedRef.errors.emplace_back(newError);
			return newError;
		}

		template<jsonifier::concepts::core_type value_type, jsonifier::concepts::is_fwd_iterator iterator_type>
		JSONIFIER_INLINE static void simd_impl(value_type&& data, iterator_type&& iter) {
			if constexpr (jsonifier::concepts::has_excluded_keys<value_type>) {
				simd_parse_impl<jsonifier::concepts::unwrap_t<value_type>, derived_type>::simd_impl(std::forward<value_type>(data), std::forward<iterator_type>(iter),
					data.jsonifierExcludedKeys);
			} else {
				simd_parse_impl<jsonifier::concepts::unwrap_t<value_type>, derived_type>::simd_impl(std::forward<value_type>(data), std::forward<iterator_type>(iter));
			}
		}

		template<jsonifier::concepts::core_type value_type, jsonifier::concepts::is_fwd_iterator iterator_type, typename parser_type>
		JSONIFIER_INLINE static void serial_impl(value_type&& data, iterator_type&& iter, iterator_type&& end, parser_type& parserVal) {
			if constexpr (jsonifier::concepts::has_excluded_keys<value_type>) {
				serial_parse_impl<jsonifier::concepts::unwrap_t<value_type>, derived_type, parser_type>::serial_impl(std::forward<value_type>(data),
					std::forward<iterator_type>(iter), std::forward<iterator_type>(end), parserVal, data.jsonifierExcludedKeys);
			} else {
				serial_parse_impl<jsonifier::concepts::unwrap_t<value_type>, derived_type, parser_type>::serial_impl(std::forward<value_type>(data),
					std::forward<iterator_type>(iter), std::forward<iterator_type>(end), parserVal);
			}
		}

		JSONIFIER_INLINE ~parser() noexcept = default;
	};
};